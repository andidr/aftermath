/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "counter_event_set_index.h"
#include "counter_event_set.h"
#include "aux.h"

int counter_event_set_index_node_min_max_value(struct counter_event_set_index_node* root,
					       int64_t start, int64_t end,
					       int64_t* min, int64_t* max);

int counter_event_set_index_node_min_max_slope(struct counter_event_set_index_node* root,
					       int64_t start, int64_t end,
					       long double* min, long double* max);

#define NUM_SAMPLES(__leaf) \
	(((unsigned long)((__leaf)->right_event) - \
	  ((unsigned long)(__leaf)->left_event)) / \
	 sizeof(struct counter_event))

#define NTH_EVENT(__leaf, __n) (&(__leaf)->left_event[(__n)])
#define NTH_NODE(__node, __n) (&(__node)->left_child[(__n)])

/*
 * Calculates the number of parent nodes in the index for nodes child
 * nodes.
 */
static inline int num_parents(int nodes, int fan_out)
{
	return ROUND_UP(nodes, fan_out) / fan_out;
}

/*
 * Create a counter event set index for ces. Fan_out indicates the
 * maximal number of successors in the tree used by the index.
 */
int counter_event_set_index_create(struct counter_event_set_index* idx,
				   struct counter_event_set* ces,
				   int fan_out)
{
	/* count nodes */
	int num_nodes = 0;
	int remaining = ces->num_events;
	int this_level;


	idx->fan_out = fan_out;

	do {
		this_level = num_parents(remaining, fan_out);
		num_nodes += this_level;
		remaining = this_level;
	} while(remaining > 1);

	/* allocate nodes */
	if(!(idx->nodes = malloc(num_nodes*sizeof(struct counter_event_set_index_node))))
		return 1;

	/* build index */
	int curr_idx = num_nodes;

	/* leaves */
	int num_leaves = num_parents(ces->num_events, fan_out);
	curr_idx -= num_leaves;

	struct counter_event_set_index_node* curr_node = &idx->nodes[curr_idx];
	int64_t value_max = 0;
	int64_t value_min = 0;

	long double slope_min = 0;
	long double slope_max = 0;

	/* index for the left child */
	int left = 0;

	for(int i = 0; i < ces->num_events; i++) {
		if(i % idx->fan_out == 0) {
			/* set initial min and max for the next node */
			value_max = ces->events[i].value;
			value_min = ces->events[i].value;

			slope_min  = ces->events[i].slope;
			slope_max  = ces->events[i].slope;

			left = i;
		} else {
			/* Update statistics */
			if(ces->events[i].value > value_max)
				value_max = ces->events[i].value;

			if(ces->events[i].value < value_min)
				value_min = ces->events[i].value;

			if(ces->events[i].slope > slope_max)
				slope_max = ces->events[i].slope;

			if(ces->events[i].slope < slope_min)
				slope_min = ces->events[i].slope;
		}

		if(i % idx->fan_out == idx->fan_out-1 ||
		   i == ces->num_events-1)
		{
			/* Set values with final statistics for the leaf */
			curr_node->type = COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF;

			curr_node->value_min = value_min;
			curr_node->value_max = value_max;

			curr_node->slope_min = slope_min;
			curr_node->slope_max = slope_max;

			curr_node->left_event = &ces->events[left];
			curr_node->right_event = &ces->events[i];

			curr_node->right_child = NULL;
			curr_node->left_child = NULL;

			curr_node++;
		}
	}

	/* internal nodes */
	int last_level = num_leaves;
	int start;

	if(num_leaves > 1) {
		do {
			this_level = num_parents(last_level, fan_out);
			start = curr_idx;

			curr_idx -= this_level;
			curr_node = &idx->nodes[curr_idx];

			for(int i = 0; i < last_level; i++) {
				if(i % fan_out == 0) {
					/* set initial min and max for the next node */
					value_max = idx->nodes[start+i].value_max;
					value_min = idx->nodes[start+i].value_min;

					slope_max = idx->nodes[start+i].slope_max;
					slope_min = idx->nodes[start+i].slope_min;

					left = start + i;
				} else {
					/* Update statistics */
					if(idx->nodes[start+i].value_max > value_max)
						value_max = idx->nodes[start+i].value_max;

					if(idx->nodes[start+i].value_min < value_min)
						value_min = idx->nodes[start+i].value_min;

					if(idx->nodes[start+i].slope_max > slope_max)
						slope_max = idx->nodes[start+i].slope_max;

					if(idx->nodes[start+i].slope_min < slope_min)
						slope_min = idx->nodes[start+i].slope_min;
				}

				if(i % fan_out == fan_out-1 || i == last_level-1)
				{
					/* Set values with final statistics for the node */
					curr_node->type = COUNTER_EVENT_SET_INDEX_NODE_TYPE_INTERNAL;

					curr_node->value_min = value_min;
					curr_node->value_max = value_max;

					curr_node->slope_min = slope_min;
					curr_node->slope_max = slope_max;

					curr_node->left_child = &idx->nodes[left];
					curr_node->right_child = &idx->nodes[start+i];

					curr_node->left_event = curr_node->left_child->left_event;
					curr_node->right_event = curr_node->right_child->right_event;

					curr_node++;
				}
			}

			last_level = this_level;
		} while(this_level > 1);
	}

	return 0;
}

/*
 * Destroys a counter event set index. All internal data structures
 * used by the index are freed, but idx remains allocated as it is
 * supposed to be allocated by the caller.
 */
void counter_event_set_index_destroy(struct counter_event_set_index* idx)
{
	free(idx->nodes);
}

/*
 * Find the minimal and maximal value within the interval [start; end]
 * within the subtree rooted by the internal node n.
 */
int counter_event_set_index_node_min_max_value_internal(struct counter_event_set_index_node* n,
							int64_t start, int64_t end,
							int64_t* min, int64_t* max)
{
	struct counter_event_set_index_node* iter;

	int64_t start_val;
	int64_t end_val;

	int64_t lmin_start;
	int64_t lmax_start;

	int64_t lmin_end;
	int64_t lmax_end;

	int64_t lmin;
	int64_t lmax;

	struct counter_event_set_index_node* node_start = n->left_child;
	struct counter_event_set_index_node* node_end;

	/* Only positive values allowed as all timestamps of counter
	 * events are positive */
	if(start < 0)
		start = 0;

	if(end < 0)
		end = 0;

	/* No overlap */
	if(start > n->right_event->time || end < n->left_event->time)
		return 1;

	/* Adjust interval */
	if(start < n->left_event->time)
		start = n->left_event->time;

	if(end > n->right_event->time)
		end = n->right_event->time;

	/* TODO: replace this with binary search */
	while(node_start->right_event->time < start)
		node_start++;

	/* Start in between two nodes? */
	if(start < node_start->left_event->time) {
		/* End also in between */
		if(end < node_start->left_event->time) {
			/* Just interpolate both and we're done */
			start_val = counter_event_interpolate_value((node_start-1)->right_event, node_start->left_event, start);
			end_val = counter_event_interpolate_value((node_start-1)->right_event, node_start->left_event, end);

			*min = (start_val < end_val) ? start_val : end_val;
			*max = (start_val < end_val) ? end_val : start_val;

			return 0;
		} else {
			/* Interpolate for the start */
			start_val = counter_event_interpolate_value((node_start-1)->right_event, node_start->left_event, start);
			counter_event_set_index_node_min_max_value_internal(n, node_start->left_event->time, end,
									    &lmin, &lmax);

			*min = (start_val < lmin) ? start_val : lmin;
			*max = (start_val > lmax) ? start_val : lmax;

			return 0;
		}
	}

	node_end = node_start;

	/* TODO: replace this with binary search */
	while(node_end->right_event->time < end)
		node_end++;

	/* End in between two nodes? */
	if(end < node_end->left_event->time) {
		/* Interpolate for the end */
		end_val = counter_event_interpolate_value((node_end-1)->right_event, node_end->left_event, end);
		counter_event_set_index_node_min_max_value_internal(n, start, (node_end-1)->right_event->time,
								    &lmin, &lmax);

		*min = (end_val < lmin) ? end_val : lmin;
		*max = (end_val > lmax) ? end_val : lmax;

		return 0;
	}

	/* Start is within the range of the start node and end is
	 * within the range of the end node
	 */

	/* Same child? */
	if(node_start == node_end) {
		return counter_event_set_index_node_min_max_value(node_start, start, end, min, max);
	} else {
		/* Get min max for start node */
		counter_event_set_index_node_min_max_value(node_start,
							   start, node_start->right_event->time,
							   &lmin_start, &lmax_start);

		/* Get min max for end node */
		counter_event_set_index_node_min_max_value(node_end,
							   node_end->left_event->time, end,
							   &lmin_end, &lmax_end);

		/* Determine min / max of both nodes */
		lmin = (lmin_start < lmin_end) ? lmin_start : lmin_end;
		lmax = (lmax_start > lmax_end) ? lmax_start : lmax_end;

		/* Check nodes in between */
		for(iter = node_start+1; iter != node_end; iter++) {
			if(iter->value_max > lmax)
				lmax = iter->value_max;

			if(iter->value_min < lmin)
				lmin = iter->value_min;
		}

		*min = lmin;
		*max = lmax;

		return 0;

	}

	/* never reached */
	return 0;
}

/*
 * Find the minimal and maximal value within the interval [start; end]
 * within the subtree rooted by the leaf n.
 */
int counter_event_set_index_node_min_max_value_leaf(struct counter_event_set_index_node* n,
						    int64_t start, int64_t end,
						    int64_t* min, int64_t* max)
{
	int64_t start_val;
	int64_t end_val;
	int64_t lmin;
	int64_t lmax;
	struct counter_event* cre_start = n->left_event;
	struct counter_event* cre_end;

	/* Only positive values allowed as all timestamps of counter
	 * events are positive */
	if(start < 0)
		start = 0;

	if(end < 0)
		end = 0;

	/* No overlap */
	if(start > n->right_event->time || end < n->left_event->time)
		return 1;

	/* Adjust interval */
	if(start < n->left_event->time)
		start = n->left_event->time;

	if(end > n->right_event->time)
		end = n->right_event->time;

	/* TODO: replace this with binary search */
	while(cre_start->time < start)
		cre_start++;

	/* Determine value at the start of the interval */
	if(cre_start->time == start)
		start_val = cre_start->value;
	else
		start_val = counter_event_interpolate_value(cre_start-1, cre_start, start);

	cre_end = cre_start;

	/* TODO: replace this with binary search */
	while(cre_end->time < end)
		cre_end++;

	/* Determine value at the end of the interval */
	if(cre_end->time == end)
		end_val = cre_end->value;
	else
		end_val = counter_event_interpolate_value(cre_end-1, cre_end, end);

	lmin = (start_val < end_val) ? start_val : end_val;
	lmax = (start_val < end_val) ? end_val : start_val;

	/* Check values between the start and end event */
	for(struct counter_event* cre = cre_start;
	    cre != cre_end;
	    cre++)
	{
		if(cre->value < lmin)
			lmin = cre->value;

		if(cre->value > lmax)
			lmax = cre->value;
	}

	*min = lmin;
	*max = lmax;

	return 0;
}

/*
 * Find the minimal and maximal value within the interval [start; end]
 * within the subtree rooted by root.
 */
int counter_event_set_index_node_min_max_value(struct counter_event_set_index_node* root,
					       int64_t start, int64_t end,
					       int64_t* min, int64_t* max)
{
	if(root->type == COUNTER_EVENT_SET_INDEX_NODE_TYPE_INTERNAL)
		return counter_event_set_index_node_min_max_value_internal(root, start, end, min, max);
	else
		return counter_event_set_index_node_min_max_value_leaf(root, start, end, min, max);
}

/*
 * Find the minimal and maximal value within the interval [start; end]
 * using the index idx.
 */
int counter_event_set_index_min_max_value(struct counter_event_set_index* idx,
					   int64_t start, int64_t end,
					   int64_t* min, int64_t* max)
{
	struct counter_event_set_index_node* root = idx->nodes;

	/* Only positive values allowed as all timestamps of counter
	 * events are positive */
	if(start < 0)
		start = 0;

	if(end < 0)
		end = 0;

	if(start > root->right_event->time || end < root->left_event->time)
		return 1;


	return counter_event_set_index_node_min_max_value(root, start, end, min, max);
}

/*
 * Find the minimal and maximal slope within the interval [start; end]
 * within the subtree rooted by the internal node n.
 */
int counter_event_set_index_node_min_max_slope_internal(struct counter_event_set_index_node* n,
							int64_t start, int64_t end,
							long double* min, long double* max)
{
	struct counter_event_set_index_node* iter;

	long double start_val;
	long double end_val;

	long double lmin_start;
	long double lmax_start;

	long double lmin_end;
	long double lmax_end;

	long double lmin;
	long double lmax;

	struct counter_event_set_index_node* node_start = n->left_child;
	struct counter_event_set_index_node* node_end;

	/* Only positive values allowed as all timestamps of counter
	 * events are positive */
	if(start < 0)
		start = 0;

	if(end < 0)
		end = 0;

	/* No overlap */
	if(start > n->right_event->time || end < n->left_event->time)
		return 1;

	/* Adjust interval */
	if(start < n->left_event->time)
		start = n->left_event->time;

	if(end > n->right_event->time)
		end = n->right_event->time;

	/* TODO: replace this with binary search */
	while(node_start->right_event->time < start)
		node_start++;

	/* Start in between two nodes? */
	if(start < node_start->left_event->time) {
		/* End also in between */
		if(end < node_start->left_event->time) {
			/* Slope is identical for both points */
			start_val = (node_start-1)->right_event->slope;
			end_val = start_val;

			*min = (start_val < end_val) ? start_val : end_val;
			*max = (start_val < end_val) ? end_val : start_val;

			return 0;
		} else {
			/* Interpolate for the start */
			start_val = (node_start-1)->right_event->slope;
			counter_event_set_index_node_min_max_slope_internal(n, node_start->left_event->time, end,
									    &lmin, &lmax);

			*min = (start_val < lmin) ? start_val : lmin;
			*max = (start_val > lmax) ? start_val : lmax;

			return 0;
		}
	}

	node_end = node_start;

	/* TODO: replace this with binary search */
	while(node_end->right_event->time < end)
		node_end++;

	/* End in between two nodes? */
	if(end < node_end->left_event->time) {
		/* Interpolate for the end */
		end_val = (node_end-1)->right_event->slope;
		counter_event_set_index_node_min_max_slope_internal(n, start, (node_end-1)->right_event->time,
								    &lmin, &lmax);

		*min = (end_val < lmin) ? end_val : lmin;
		*max = (end_val > lmax) ? end_val : lmax;

		return 0;
	}

	/* Start is within the range of the start node and end is
	 * within the range of the end node
	 */

	/* Same child? */
	if(node_start == node_end) {
		return counter_event_set_index_node_min_max_slope(node_start, start, end, min, max);
	} else {
		/* Get min max for start node */
		counter_event_set_index_node_min_max_slope(node_start,
							   start, node_start->right_event->time,
							   &lmin_start, &lmax_start);

		/* Get min max for end node */
		counter_event_set_index_node_min_max_slope(node_end,
							   node_end->left_event->time, end,
							   &lmin_end, &lmax_end);

		/* Determine min / max of both nodes */
		lmin = (lmin_start < lmin_end) ? lmin_start : lmin_end;
		lmax = (lmax_start > lmax_end) ? lmax_start : lmax_end;

		/* Check nodes in between */
		for(iter = node_start+1; iter != node_end; iter++) {
			if(iter->slope_max > lmax)
				lmax = iter->slope_max;

			if(iter->slope_min < lmin)
				lmin = iter->slope_min;
		}

		*min = lmin;
		*max = lmax;

		return 0;
	}

	/* never reached */
	return 0;
}

/*
 * Find the minimal and maximal slope within the interval [start; end]
 * within the subtree rooted by the leaf n.
 */
int counter_event_set_index_node_min_max_slope_leaf(struct counter_event_set_index_node* n,
						    int64_t start, int64_t end,
						    long double* min, long double* max)
{
	long double start_val;
	long double end_val;
	long double lmin;
	long double lmax;
	struct counter_event* cre_start = n->left_event;
	struct counter_event* cre_end;

	/* Only positive values allowed as all timestamps of counter
	 * events are positive */
	if(start < 0)
		start = 0;

	if(end < 0)
		end = 0;

	/* No overlap */
	if(start > n->right_event->time || end < n->left_event->time)
		return 1;

	/* Adjust interval */
	if(start < n->left_event->time)
		start = n->left_event->time;

	if(end > n->right_event->time)
		end = n->right_event->time;

	/* TODO: replace this with binary search */
	while(cre_start->time < start)
		cre_start++;

	/* Determine slope at the start of the interval */
	if(cre_start->time == start)
		start_val = cre_start->slope;
	else
		start_val = (cre_start-1)->slope;

	cre_end = cre_start;

	/* TODO: replace this with binary search */
	while(cre_end->time < end)
		cre_end++;

	/* Determine slope at the end of the interval */
	if(cre_end->time == end)
		end_val = cre_end->slope;
	else
		end_val = (cre_end-1)->slope;

	lmin = (start_val < end_val) ? start_val : end_val;
	lmax = (start_val < end_val) ? end_val : start_val;

	/* Check slopes between the start and end event */
	for(struct counter_event* cre = cre_start;
	    cre != cre_end;
	    cre++)
	{
		if(cre->slope < lmin)
			lmin = cre->slope;

		if(cre->slope > lmax)
			lmax = cre->slope;
	}

	*min = lmin;
	*max = lmax;

	return 0;
}

/*
 * Find the minimal and maximal slope within the interval [start; end]
 * within the subtree rooted by root.
 */
int counter_event_set_index_node_min_max_slope(struct counter_event_set_index_node* root,
					       int64_t start, int64_t end,
					       long double* min, long double* max)
{
	if(root->type == COUNTER_EVENT_SET_INDEX_NODE_TYPE_INTERNAL)
		return counter_event_set_index_node_min_max_slope_internal(root, start, end, min, max);
	else
		return counter_event_set_index_node_min_max_slope_leaf(root, start, end, min, max);
}

/*
 * Find the minimal and maximal slope within the interval [start; end]
 * using the index idx.
 */
int counter_event_set_index_min_max_slope(struct counter_event_set_index* idx,
					  int64_t start, int64_t end,
					  long double* min, long double* max)
{
	struct counter_event_set_index_node* root = idx->nodes;

	/* Only positive values allowed as all timestamps of counter
	 * events are positive */
	if(start < 0)
		start = 0;

	if(end < 0)
		end = 0;

	if(start > root->right_event->time || end < root->left_event->time)
		return 1;


	return counter_event_set_index_node_min_max_slope(root, start, end, min, max);
}
