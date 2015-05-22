#include <unit_tests.h>
#include "../src/counter_event_set_index.h"
#include "../src/counter_event_set.h"

#define D ((int64_t)1000000)
#define V ((int64_t)1000)
#define S (((long double)V) / ((long double)D))

static inline int64_t interpolate(struct counter_event_set* ces, int i0, int i1, int64_t time)
{
	return counter_event_interpolate_value(&ces->events[i0], &ces->events[i1], time);
}

static inline int64_t int64_min(int64_t a, int64_t b)
{
	return (a < b) ? a : b;
}

static inline int64_t int64_max(int64_t a, int64_t b)
{
	return (a > b) ? a : b;
}

static inline int64_t int64_min3(int64_t a, int64_t b, int64_t c)
{
	return (a < b) ? int64_min(a, c) : int64_min(b, c);
}

static inline int64_t int64_max3(int64_t a, int64_t b, int64_t c)
{
	return (a > b) ? int64_max(a, c) : int64_max(b, c);
}

static inline int64_t int64_min_n(int64_t* v, int n)
{
	int64_t min = v[0];

	for(int i = 1; i < n; i++)
		if(v[i] < min)
			min = v[i];

	return min;
}

static inline int64_t int64_max_n(int64_t* v, int n)
{
	int64_t max = v[0];

	for(int i = 1; i < n; i++)
		if(v[i] > max)
			max = v[i];

	return max;
}

/*
 * Adds nsamples equal-spaced counter events with linearly increasing
 * values to a counter event set
 */
static int populate_counter_event_set_linear(struct counter_event_set* ces, int nsamples)
{
	struct counter_event ce;

	for(int i = 0; i < nsamples; i++) {
		ce.value = V*(i+1);
		ce.time = D*(i+1);

		if(counter_event_set_add_event(ces, &ce, 1))
			return 1;
	}

	return 0;
}

/*
 * Adds nsamples equal-spaced counter events with quadratically
 * increasing values to a counter event set
 */
static int populate_counter_event_set_quadratic(struct counter_event_set* ces, int nsamples)
{
	struct counter_event ce;

	for(int i = 0; i < nsamples; i++) {
		ce.value = V*(i+1)*(i+1);
		ce.time = D*(i+1);

		if(counter_event_set_add_event(ces, &ce, 1))
			return 1;
	}

	return 0;
}

/*
 * Adds nsamples equal-spaced counter events with alternating values:
 * 0, 1, 0, -1, 0, 1, 0, -1, ...
 */
static int populate_counter_event_set_alt(struct counter_event_set* ces, int nsamples)
{
	struct counter_event ce;

	for(int i = 0; i < nsamples; i++) {
		ce.value = (i%2)*V;

		if(i % 4 == 3)
			ce.value = -ce.value;
		ce.time = D*(i+1);

		if(counter_event_set_add_event(ces, &ce, 1))
			return 1;
	}

	return 0;
}

/*
 * Calculates the slope between the i-th sample and the (i+1)-th
 * sample for an event set initialized with
 * populate_counter_event_set_quadratic
 */
static long double SQ(int i)
{
	long double ild = i;
	long double ildn = i+1;

	return (ildn*ildn*V - ild*ild*V) / D;
}

/*
 * Creates an index with a single level containing one leaf only,
 * indexing a single event
 *
 *     root
 *      |
 *     [0]
 */
UNIT_TEST(construction_test_one_event)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	struct counter_event_set_index_node* root;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, 1), 0);

	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, 10), 0);

	root = idx.nodes;
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF);

	ASSERT_EQUALS(root->left_event->time, D);
	ASSERT_EQUALS(root->right_event->time, D);

	ASSERT_EQUALS(root->left_event->value, V);
	ASSERT_EQUALS(root->right_event->value, V);

	ASSERT_EQUALS_LD(root->left_event->slope, 0);
	ASSERT_EQUALS_LD(root->right_event->slope, 0);

	ASSERT_EQUALS(root->value_min, V);
	ASSERT_EQUALS(root->value_max, V);

	ASSERT_EQUALS(root->slope_min, 0);
	ASSERT_EQUALS(root->slope_max, 0);

	ASSERT_EQUALS(root->left_event, &ces.events[0]);
	ASSERT_EQUALS(root->right_event, &ces.events[0]);

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Creates an index with a single level containing one leaf only
 *
 *       root
 *      /    \
 *     [0,..,4]
 */
UNIT_TEST(construction_test_one_leaf)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	struct counter_event_set_index_node* root;
	int nsamples = 5;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, nsamples), 0);

	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, 10), 0);

	root = idx.nodes;
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF);

	ASSERT_EQUALS(root->left_event->time, D);
	ASSERT_EQUALS(root->right_event->time, 5*D);

	ASSERT_EQUALS(root->left_event->value, V);
	ASSERT_EQUALS(root->right_event->value, nsamples*V);

	ASSERT_EQUALS_LD(root->left_event->slope, 0);
	ASSERT_EQUALS_LD(root->right_event->slope, S);

	ASSERT_EQUALS(root->value_min, V);
	ASSERT_EQUALS(root->value_max, nsamples*V);

	ASSERT_EQUALS_LD(root->slope_min, 0);
	ASSERT_EQUALS_LD(root->slope_max, S);

	ASSERT_EQUALS(root->left_event, &ces.events[0]);
	ASSERT_EQUALS(root->right_event, &ces.events[4]);

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Creates an index with two leaves.
 *
 *       root
 *      /    \
 *     l      r
 *   /   \  /   \
 *  [0..F-1,F..2F-1]
 */
UNIT_TEST(construction_test_two_leaves)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	struct counter_event_set_index_node* root;
	int fan_out = 10;
	int nsamples = 2*fan_out;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, nsamples), 0);

	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	root = idx.nodes;
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, COUNTER_EVENT_SET_INDEX_NODE_TYPE_INTERNAL);
	ASSERT_EQUALS(root->left_child, &idx.nodes[1]);
	ASSERT_EQUALS(root->right_child, &idx.nodes[2]);

	ASSERT_EQUALS(root->left_child->type,
		      COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF);
	ASSERT_EQUALS(root->right_child->type,
		      COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF);

	ASSERT_EQUALS(root->left_event->time, D);
	ASSERT_EQUALS(root->right_event->time, 2*fan_out*D);

	ASSERT_EQUALS(root->left_event->value, V);
	ASSERT_EQUALS(root->right_event->value, 2*fan_out*V);

	ASSERT_EQUALS_LD(root->left_event->slope, 0);
	ASSERT_EQUALS_LD(root->right_event->slope, S);

	ASSERT_EQUALS(root->value_min, V);
	ASSERT_EQUALS(root->value_max, 2*fan_out*V);

	ASSERT_EQUALS_LD(root->slope_min, 0);
	ASSERT_EQUALS_LD(root->slope_max, S);

	/* left leaf */
	ASSERT_EQUALS(root->left_child->left_event,
		      &ces.events[0]);
	ASSERT_EQUALS(root->left_child->right_event,
		      &ces.events[fan_out-1]);

	ASSERT_EQUALS(root->left_child->left_event->time, D);
	ASSERT_EQUALS(root->left_child->right_event->time,
		      fan_out*D);

	ASSERT_EQUALS(root->left_child->value_min, V);
	ASSERT_EQUALS(root->left_child->value_max, fan_out*V);

	ASSERT_EQUALS_LD(root->left_child->slope_min, 0);
	ASSERT_EQUALS_LD(root->left_child->slope_max, S);

	/* right leaf */
	ASSERT_EQUALS(root->right_child->left_event,
		      &ces.events[fan_out]);
	ASSERT_EQUALS(root->right_child->right_event,
		      &ces.events[2*fan_out-1]);

	ASSERT_EQUALS(root->right_child->left_event->time, (fan_out+1)*D);
	ASSERT_EQUALS(root->right_child->right_event->time, 2*fan_out*D);

	ASSERT_EQUALS(root->right_child->value_min, (fan_out+1)*V);
	ASSERT_EQUALS(root->right_child->value_max, 2*fan_out*V);

	ASSERT_EQUALS_LD(root->right_child->slope_min, S);
	ASSERT_EQUALS_LD(root->right_child->slope_max, S);

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Creates an index with F leaves (FF samples).
 *
 *        .-- root --.
 *      /       |     \
 *     l       ...     r
 *   /   \           /   \
 *  [0..F-1, ..., (F-1)F..FF-1]
 */
UNIT_TEST(construction_test_max_one_level)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	struct counter_event_set_index_node* root;
	int fan_out = 10;

	int nsamples = fan_out*fan_out;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, nsamples), 0);

	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	root = idx.nodes;
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, COUNTER_EVENT_SET_INDEX_NODE_TYPE_INTERNAL);
	ASSERT_EQUALS(root->left_child, &idx.nodes[1]);
	ASSERT_EQUALS(root->right_child, &idx.nodes[fan_out]);

	ASSERT_EQUALS(root->left_child->type,
		      COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF);
	ASSERT_EQUALS(root->right_child->type,
		      COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF);

	ASSERT_EQUALS(root->left_event->time, D);
	ASSERT_EQUALS(root->right_event->time, nsamples*D);

	ASSERT_EQUALS(root->left_event->value, V);
	ASSERT_EQUALS(root->right_event->value, nsamples*V);

	ASSERT_EQUALS_LD(root->left_event->slope, 0);
	ASSERT_EQUALS_LD(root->right_event->slope, S);

	ASSERT_EQUALS(root->value_min, V);
	ASSERT_EQUALS(root->value_max, nsamples*V);

	ASSERT_EQUALS_LD(root->slope_min, 0);
	ASSERT_EQUALS_LD(root->slope_max, S);

	/* Check each leaf */
	for(int i = 0; i < fan_out; i++) {
		ASSERT_EQUALS(root->left_child[i].type,
			      COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF);

		ASSERT_EQUALS(root->left_child[i].left_event->time,
			      (i*fan_out+1)*D);
		ASSERT_EQUALS(root->left_child[i].right_event->time,
			      ((i+1)*fan_out)*D);

		ASSERT_EQUALS(root->left_child[i].value_min,
			      (i*fan_out+1)*V);
		ASSERT_EQUALS(root->left_child[i].value_max,
			      ((i+1)*fan_out)*V);

		ASSERT_EQUALS_LD(root->left_child[i].slope_min, i ? S : 0);
		ASSERT_EQUALS_LD(root->left_child[i].slope_max, S);
	}

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Creates an index with FF leaves (FFF samples).
 *
 *        .-- root --.            Level 0
 *      /       |     \
 *     l       ...     r          Level !
 *   /   \           /   \
 *  [0..F-1, ..., (F-1)F..FF-1]   Level 2
 *  /     \        /        \
 * [0..FF-1, ..., (F-1)FF..FFF-1] Samples
 */
UNIT_TEST(construction_test_max_two_levels)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	struct counter_event_set_index_node* root;
	int fan_out = 10;

	int nsamples = fan_out*fan_out*fan_out;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, nsamples), 0);

	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	root = idx.nodes;
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, COUNTER_EVENT_SET_INDEX_NODE_TYPE_INTERNAL);
	ASSERT_EQUALS(root->left_child, &idx.nodes[1]);
	ASSERT_EQUALS(root->right_child, &idx.nodes[fan_out]);

	/* Check internal nodes on level 1 */
	for(int i = 0; i < fan_out; i++) {
		ASSERT_EQUALS(root->left_child[i].type,
			      COUNTER_EVENT_SET_INDEX_NODE_TYPE_INTERNAL);

		ASSERT_EQUALS(root->left_child[i].left_event->time,
			      (i*fan_out*fan_out+1)*D);
		ASSERT_EQUALS(root->left_child[i].right_event->time,
			      ((i+1)*fan_out*fan_out)*D);

		ASSERT_EQUALS(root->left_child[i].value_min,
			      (i*fan_out*fan_out+1)*V);
		ASSERT_EQUALS(root->left_child[i].value_max,
			      ((i+1)*fan_out*fan_out)*V);

		ASSERT_EQUALS_LD(root->left_child[i].slope_min, i ? S : 0);
		ASSERT_EQUALS_LD(root->left_child[i].slope_max, S);
	}

	/* Check internal nodes on level 2 */
	for(int i = 0; i < fan_out*fan_out; i++) {
		ASSERT_EQUALS(root->left_child[0].left_child[i].type,
			      COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF);

		ASSERT_EQUALS(root->left_child[0].left_child[i].left_event->time,
			      (i*fan_out+1)*D);

		ASSERT_EQUALS(root->left_child[0].left_child[i].right_event->time,
			      ((i+1)*fan_out)*D);

		ASSERT_EQUALS(root->left_child[0].left_child[i].value_min,
			      (i*fan_out+1)*V);

		ASSERT_EQUALS(root->left_child[0].left_child[i].value_max,
			      ((i+1)*fan_out)*V);

		ASSERT_EQUALS_LD(root->left_child[0].left_child[i].slope_min, i ? S : 0);
		ASSERT_EQUALS_LD(root->left_child[0].left_child[i].slope_max, S);
	}

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Creates an index with F^3...F^n leaves (F^4..F^(n+1) samples).
 */
UNIT_TEST(construction_test_max_n_levels)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int n = 5;
	int fan_out = 10;

	for(int i = 3; i <= n; i++) {
		int nsamples = 1;

		for(int j = 0; j < n+1; j++)
			nsamples *= fan_out;

		counter_event_set_init(&ces, NULL);

		ASSERT_EQUALS(populate_counter_event_set_linear(&ces, nsamples), 0);

		ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

		counter_event_set_index_destroy(&idx);
		counter_event_set_destroy(&ces);
	}
}
END_TEST()

/*
 * Lookup min and max in a tree for a single counter event.
 */
UNIT_TEST(lookup_test_min_max_value_one_event)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int fan_out = 10;

	int64_t min;
	int64_t max;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, 1), 0);
	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* check intervals not in global interval */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, D-1, &min, &max), 1);
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, D+1, 2*D, &min, &max), 1);

	/* Exact match */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, D, D, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, V);

	/* Interval embracing the event */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, 2*D, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, V);

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Lookup min and max slope in a tree for a single counter event.
 */
UNIT_TEST(lookup_test_min_max_slope_one_event)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int fan_out = 10;

	long double min;
	long double max;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_quadratic(&ces, 1), 0);
	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* check intervals not in global interval */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 0, D-1, &min, &max), 1);
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, D+1, 2*D, &min, &max), 1);

	/* Exact match */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, D, D, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, 0);

	/* Interval embracing the event */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 0, 2*D, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, 0);

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Lookup min and max in a tree with a single leaf.
 */
UNIT_TEST(lookup_test_min_max_value_one_leaf)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int fan_out = 10;
	int nsamples = 5;

	int64_t min;
	int64_t max;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, nsamples), 0);

	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* check intervals not in global interval */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, V, &min, &max), 1);
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 5*D+1, 6*D, &min, &max), 1);

	/* First and second event: exact match */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, D, 2*D, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, 2*V);

	/* Interval embracing all events */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, 5*D+1, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, 5*V);

	/* Perfect match on first event, between first and second event */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, D, 3*D/2, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, 3*V/2);

	/* Between first and second event, between second and third event */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 3*D/2, 5*D/2, &min, &max), 0);
	ASSERT_EQUALS(min, 3*V/2);
	ASSERT_EQUALS(max, 5*V/2);

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Lookup min and max slope in a tree with a single leaf.
 */
UNIT_TEST(lookup_test_min_max_slope_one_leaf)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int fan_out = 10;
	int nsamples = 5;

	long double min;
	long double max;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_quadratic(&ces, nsamples), 0);
	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* check intervals not in global interval */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 0, V, &min, &max), 1);
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 5*D+1, 6*D, &min, &max), 1);

	/* First and second event: exact match */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, D, 2*D, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, SQ(1));

	/* Interval embracing all events */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 0, 5*D+1, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, SQ(nsamples-1));

	/* Perfect match on first event, between first and second event */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, D, 3*D/2, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, 0);

	/* Between first and second event, between second and third event */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 3*D/2, 5*D/2, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, SQ(1));

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Lookup min and max in a tree with two leaves.
 */
UNIT_TEST(lookup_test_min_max_value_two_leaves)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int fan_out = 10;
	int nsamples = 2*fan_out;

	int64_t min;
	int64_t max;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, nsamples), 0);

	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* Perfect match, first node
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 *   ^     ^
	 *   S     E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, D, fan_out*D, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, fan_out*V);

	/* Perfect match, second node
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 *             ^      ^
	 *             S      E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, (fan_out+1)*D, 2*fan_out*D, &min, &max), 0);
	ASSERT_EQUALS(min, (fan_out+1)*V);
	ASSERT_EQUALS(max, 2*fan_out*V);

	/* Match inside first node
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 *     ^ ^
	 *     S E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 2*D, (fan_out-1)*D, &min, &max), 0);
	ASSERT_EQUALS(min, 2*V);
	ASSERT_EQUALS(max, (fan_out-1)*V);

	/* Start: left of first leaf
	 * End: between two leaves
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 * ^         ^
	 * S         E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, fan_out*D + D/2, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, fan_out*V + V/2);

	/* Start: between two leaves
	 * End: right of second leaf
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 *           ^          ^
	 *           S          E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, fan_out*D + D/2, 2*fan_out*D+1, &min, &max), 0);
	ASSERT_EQUALS(min, fan_out*V + V/2);
	ASSERT_EQUALS(max, 2*fan_out*V);

	/* Start: between two leaves
	 * End: between two leaves
	 *
	 *           root
	 *        /        \
	 *      L1         L2
	 *    /   \       /   \
	 *   0...F-1     F.....2F
	 *           ^ ^
	 *           S E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, fan_out*D + D/4, fan_out*D + 3*D/4, &min, &max), 0);
	ASSERT_EQUALS(min, fan_out*V + V/4);
	ASSERT_EQUALS(max, fan_out*V + 3*V/4);

	/* Start: left of first leaf
	 * End: right of second leaf
	 *
	 *           root
	 *        /        \
	 *      L1         L2
	 *    /   \       /   \
	 *   0...F-1     F.....2F
	 * ^                      ^
	 * S                      E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, 2*fan_out*D+1, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, 2*fan_out*V);

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Lookup min and max slope in a tree with two leaves.
 */
UNIT_TEST(lookup_test_min_max_slope_two_leaves)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int fan_out = 10;
	int nsamples = 2*fan_out;

	long double min;
	long double max;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_quadratic(&ces, nsamples), 0);
	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* Perfect match, first node
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 *   ^     ^
	 *   S     E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, D, fan_out*D, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, SQ(fan_out-1));

	/* Perfect match, second node
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 *             ^      ^
	 *             S      E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, (fan_out+1)*D, 2*fan_out*D, &min, &max), 0);
	ASSERT_EQUALS_LD(min, SQ(fan_out));
	ASSERT_EQUALS_LD(max, SQ(2*fan_out-1));

	/* Match inside first node
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 *     ^ ^
	 *     S E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 2*D, (fan_out-1)*D, &min, &max), 0);
	ASSERT_EQUALS_LD(min, SQ(1));
	ASSERT_EQUALS_LD(max, SQ(fan_out-2));

	/* Start: left of first leaf
	 * End: between two leaves
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 * ^         ^
	 * S         E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 0, fan_out*D + D/2, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, SQ(fan_out-1));

	/* Start: between two leaves
	 * End: right of second leaf
	 *
	 *          root
	 *        /      \
	 *      L1       L2
	 *    /   \     /   \
	 *   0...F-1   F.....2F
	 *           ^          ^
	 *           S          E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, fan_out*D + D/2, 2*fan_out*D+1, &min, &max), 0);
	ASSERT_EQUALS_LD(min, SQ(fan_out-1));
	ASSERT_EQUALS_LD(max, SQ(2*fan_out-1));

	/* Start: between two leaves
	 * End: between two leaves
	 *
	 *           root
	 *        /        \
	 *      L1         L2
	 *    /   \       /   \
	 *   0...F-1     F.....2F
	 *           ^ ^
	 *           S E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, fan_out*D + D/4, fan_out*D + 3*D/4, &min, &max), 0);
	ASSERT_EQUALS_LD(min, SQ(fan_out-1));
	ASSERT_EQUALS_LD(max, SQ(fan_out-1));

	/* Start: left of first leaf
	 * End: right of second leaf
	 *
	 *           root
	 *        /        \
	 *      L1         L2
	 *    /   \       /   \
	 *   0...F-1     F.....2F
	 * ^                      ^
	 * S                      E
	 */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 0, 2*fan_out*D+1, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, SQ(2*fan_out-1));

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Lookup min and max in a tree with n levels.
 */
void generic_lookup_test_min_max_value_n_levels(int n)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int64_t fan_out = 10;
	int nsamples = 1;

	int64_t min;
	int64_t max;

	counter_event_set_init(&ces, NULL);

	for(int i = 0; i < n; i++)
		nsamples *= fan_out;

	ASSERT_EQUALS(populate_counter_event_set_linear(&ces, nsamples), 0);
	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* Perfect match, n-th leaf */
	for(int64_t i = 0; i < nsamples / fan_out; i++) {
		ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, (i*fan_out+1)*D, (i+1)*fan_out*D, &min, &max), 0);
		ASSERT_EQUALS(min, (i*fan_out+1)*V);
		ASSERT_EQUALS(max, (i+1)*fan_out*V);
	}

	/* Match inside n-th leaf */
	for(int i = 0; i < nsamples / fan_out; i++) {
		ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, (i*fan_out+1)*D, ((i+1)*fan_out-1)*D, &min, &max), 0);
		ASSERT_EQUALS(min, (i*fan_out+1)*V);
		ASSERT_EQUALS(max, ((i+1)*fan_out-1)*V);
	}

	/* Start: left of first leaf
	 * End: between two leaves */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, fan_out*D + D/2, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, fan_out*V + V/2);

	/* Start: between two leaves
	 * End: right of second leaf */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, fan_out*D + D/2, 2*fan_out*D+1, &min, &max), 0);
	ASSERT_EQUALS(min, fan_out*V + V/2);
	ASSERT_EQUALS(max, 2*fan_out*V);

	/* Start: between two leaves
	 * End: between two leaves */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, fan_out*D + D/4, fan_out*D + 3*D/4, &min, &max), 0);
	ASSERT_EQUALS(min, fan_out*V + V/4);
	ASSERT_EQUALS(max, fan_out*V + 3*V/4);

	/* Start: left of first leaf
	 * End: right of second leaf */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, 2*fan_out*D+1, &min, &max), 0);
	ASSERT_EQUALS(min, V);
	ASSERT_EQUALS(max, 2*fan_out*V);

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}

/*
 * Lookup min and max in a tree with more than two levels.
 */
UNIT_TEST(lookup_test_min_max_value_n_levels)
{
	generic_lookup_test_min_max_value_n_levels(5);
	generic_lookup_test_min_max_value_n_levels(6);
}
END_TEST()

/*
 * Lookup min and max slope in a tree with n levels.
 */
void generic_lookup_test_min_max_slope_n_levels(int n)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int64_t fan_out = 10;
	int nsamples = 1;

	long double min;
	long double max;

	counter_event_set_init(&ces, NULL);

	for(int i = 0; i < n; i++)
		nsamples *= fan_out;

	ASSERT_EQUALS(populate_counter_event_set_quadratic(&ces, nsamples), 0);
	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* Perfect match, n-th leaf */
	for(int64_t i = 0; i < nsamples / fan_out; i++) {
		ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, (i*fan_out+1)*D, (i+1)*fan_out*D, &min, &max), 0);
		ASSERT_EQUALS_LD(min, i ? SQ(i*fan_out) : 0);
		ASSERT_EQUALS_LD(max, SQ((i+1)*fan_out-1));
	}

	/* Match inside n-th leaf */
	for(int i = 0; i < nsamples / fan_out; i++) {
		ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, (i*fan_out+1)*D, ((i+1)*fan_out-1)*D, &min, &max), 0);
		ASSERT_EQUALS_LD(min, i ? SQ(i*fan_out) : 0);
		ASSERT_EQUALS_LD(max, SQ((i+1)*fan_out-2));
	}

	/* Start: left of first leaf
	 * End: between two leaves */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 0, fan_out*D + D/2, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, SQ(fan_out-1));

	/* Start: between two leaves
	 * End: right of second leaf */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, fan_out*D + D/2, 2*fan_out*D+1, &min, &max), 0);
	ASSERT_EQUALS_LD(min, SQ(fan_out-1));
	ASSERT_EQUALS_LD(max, SQ(2*fan_out-1));

	/* Start: between two leaves
	 * End: between two leaves */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, fan_out*D + D/4, fan_out*D + 3*D/4, &min, &max), 0);
	ASSERT_EQUALS_LD(min, SQ(fan_out-1));
	ASSERT_EQUALS_LD(max, SQ(fan_out-1));

	/* Start: left of first leaf
	 * End: right of second leaf */
	ASSERT_EQUALS(counter_event_set_index_min_max_slope(&idx, 0, 2*fan_out*D+1, &min, &max), 0);
	ASSERT_EQUALS_LD(min, 0);
	ASSERT_EQUALS_LD(max, SQ(2*fan_out-1));

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}

/*
 * Lookup min and max in a tree with more than two levels.
 */
UNIT_TEST(lookup_test_min_max_slope_n_levels)
{
	for(int i = 2; i <= 5; i++)
		generic_lookup_test_min_max_slope_n_levels(i);
}
END_TEST()

/*
 * Lookup min and max in a tree with a single leaf with alternating
 * samples (0, 1, 0, -1, 0, ...).
 */
UNIT_TEST(lookup_test_min_max_value_alt_one_leaf)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int fan_out = 10;
	int nsamples = 5;

	int64_t min;
	int64_t max;

	counter_event_set_init(&ces, NULL);

	ASSERT_EQUALS(populate_counter_event_set_alt(&ces, nsamples), 0);

	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* check intervals not in global interval */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, V, &min, &max), 1);
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 5*D+1, 6*D, &min, &max), 1);

	/* First and second event: exact match */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, D, 2*D, &min, &max), 0);
	ASSERT_EQUALS(min, 0);
	ASSERT_EQUALS(max, V);

	/* Interval embracing all events */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, 0, 5*D+1, &min, &max), 0);
	ASSERT_EQUALS(min, -V);
	ASSERT_EQUALS(max, V);

	/* Perfect match on first event, between first and second event */
	ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, D, 3*D/2, &min, &max), 0);
	ASSERT_EQUALS(min, 0);
	ASSERT_EQUALS(max, interpolate(&ces, 0, 1, 3*D/2));

	/* Query for timestamps around a sample */
	for(int64_t i = 1; i < nsamples-1; i++) {
		ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, (i+1)*D-D/2, (i+1)*D+D/2, &min, &max), 0);
		int64_t vleft = interpolate(&ces, i-1, i, (i+1)*D-D/2);
		int64_t vright = interpolate(&ces, i, i+1, (i+1)*D+D/2);

		ASSERT_EQUALS(min, int64_min3(vleft, vright, ces.events[i].value));
		ASSERT_EQUALS(max, int64_max3(vleft, vright, ces.events[i].value));
	}

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}
END_TEST()

/*
 * Lookup min and max in a tree with n levels for an event set with
 * alternating samples (0, 1, 0, -1, 0, ...).
 */
void generic_lookup_test_min_max_value_alt_n_levels(int n)
{
	struct counter_event_set ces;
	struct counter_event_set_index idx;
	int64_t fan_out = 10;
	int nsamples = 1;

	int64_t min;
	int64_t max;

	counter_event_set_init(&ces, NULL);

	for(int i = 0; i < n; i++)
		nsamples *= fan_out;

	ASSERT_EQUALS(populate_counter_event_set_alt(&ces, nsamples), 0);
	ASSERT_EQUALS(counter_event_set_index_create(&idx, &ces, fan_out), 0);

	/* Perfect match on two consecutive samples */
	for(int64_t i = 0; i < nsamples-1; i++) {
		ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, (i+1)*D, (i+2)*D, &min, &max), 0);
		ASSERT_EQUALS(min, int64_min(ces.events[i].value, ces.events[i+1].value));
		ASSERT_EQUALS(max, int64_max(ces.events[i].value, ces.events[i+1].value));
	}

	/* Start and end timestamp matching the timestamp of two
	 * samples, distance-1 samples in between. E.g., for i = 2 and
	 * distance = 3 we check:
	 *
	 * [0   1   2   3   4   5]
	 *      ^           ^
	 *      S           E
	 */
	for(int distance = 1; distance < 5; distance++) {
		for(int64_t i = 0; i < nsamples-distance; i++) {
			ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, (i+1)*D, (i+distance+1)*D, &min, &max), 0);

			/* Array containing all values to check */
			int64_t vals[distance+1];

			for(int j = 0; j <= distance; j++)
				vals[j] = ces.events[i+j].value;

			ASSERT_EQUALS(min, int64_min_n(vals, distance+1));
			ASSERT_EQUALS(max, int64_max_n(vals, distance+1));
		}
	}

	/* Start and end timestamp between two samples, distance
	 * samples in between. E.g., for i = 2 and distance = 3 we
	 * check:
	 *
	 * [0   1   2   3   4   5]
	 *        ^           ^
	 *        S           E
	 */
	for(int distance = 1; distance < 5; distance++) {
		for(int64_t i = 1; i < nsamples-distance; i++) {
			ASSERT_EQUALS(counter_event_set_index_min_max_value(&idx, (i+1)*D-D/2, (i+distance)*D+D/2, &min, &max), 0);

			/* Array containing left interpolated value,
			   all values of samples in between and the
			   right interpolated value */
			int64_t vals[distance+2];

			vals[0] = interpolate(&ces, i-1, i, (i+1)*D-D/2);
			vals[distance+1] = interpolate(&ces, i+distance-1, i+distance, (i+distance)*D+D/2);

			/* Values in between */
			for(int j = 0; j < distance; j++)
				vals[1+j] = ces.events[i+j].value;

			ASSERT_EQUALS(min, int64_min_n(vals, distance+2));
			ASSERT_EQUALS(max, int64_max_n(vals, distance+2));
		}
	}

	counter_event_set_index_destroy(&idx);
	counter_event_set_destroy(&ces);
}

/*
 * Lookup min and max in a tree with more than two levels.
 */
UNIT_TEST(lookup_test_min_max_value_alt_n_levels)
{
	for(int i = 0; i <= 5; i++)
		generic_lookup_test_min_max_value_alt_n_levels(i);
}
END_TEST()

UNIT_TEST_SUITE(counter_event_set_index_test)
	ADD_TEST(construction_test_one_event);
	ADD_TEST(construction_test_one_leaf);
	ADD_TEST(construction_test_two_leaves);
	ADD_TEST(construction_test_max_one_level);
	ADD_TEST(construction_test_max_two_levels);
	ADD_TEST(construction_test_max_n_levels);

	ADD_TEST(lookup_test_min_max_value_one_event);
	ADD_TEST(lookup_test_min_max_value_one_leaf);
	ADD_TEST(lookup_test_min_max_value_two_leaves);
	ADD_TEST(lookup_test_min_max_value_n_levels);

	ADD_TEST(lookup_test_min_max_slope_one_event);
	ADD_TEST(lookup_test_min_max_slope_one_leaf);
	ADD_TEST(lookup_test_min_max_slope_two_leaves);
	ADD_TEST(lookup_test_min_max_slope_n_levels);

	ADD_TEST(lookup_test_min_max_value_alt_one_leaf);
	ADD_TEST(lookup_test_min_max_value_alt_n_levels);
END_TEST_SUITE()
