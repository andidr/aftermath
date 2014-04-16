#include "../../address_range_tree.h"
#include "../../task_instance_tree.h"
#include <stdio.h>
#include <inttypes.h>

static struct address_range_tree t;

void address_range_tree_dump(struct address_range_tree* t)
{
	for(struct address_range_tree_node* node = address_range_tree_iter_first(t, 0, UINT64_MAX);
	    node ;
	    node = address_range_tree_iter_next(node, 0, UINT64_MAX))
	{
		printf("found %"PRIu64" %"PRIu64"\n", node->start, node->end);
	}
}

void task_instance_tree_dump(struct task_instance_tree* t)
{
	for(struct rb_node* node = rb_first(&t->root); node; node = rb_next(node)) {
		struct task_instance_tree_node* this_node = rb_entry(node, struct task_instance_tree_node, node);

		printf("Instance of %p [%"PRIu64" ; %"PRIu64"]\n", this_node->instance->task, this_node->instance->start, this_node->instance->end);
	}
}

int main(int argc, char** argv)
{
	struct task_instance_tree ti;
	task_instance_tree_init(&ti);

	struct task_instance tinst;
	tinst.task = 0x0;
	tinst.start = 5;
	tinst.end = 100;

	struct task_instance_tree_node tin;
	tin.instance = &tinst;

	task_instance_tree_insert(&ti, &tin);
	task_instance_tree_dump(&ti);

	struct task_instance_tree ti_clone;
	task_instance_tree_init(&ti_clone);

	task_instance_tree_clone(&ti, &ti_clone);
	task_instance_tree_dump(&ti_clone);
	task_instance_tree_destroy(&ti_clone);

	struct address_range_tree_node n1;
	struct address_range_tree_node n2;
	struct address_range_tree_node n3;

	address_range_tree_node_init(&n1, 5, 10);
	address_range_tree_node_init(&n2, 25, 100);
	address_range_tree_node_init(&n3, 26, 100);

	address_range_tree_init(&t);
	address_range_tree_insert(&t, &n1);
	address_range_tree_insert(&t, &n2);
	address_range_tree_insert(&t, &n3);

	struct address_range_tree_node* node = address_range_tree_iter_first(&t, 4, 6);

	printf("found %d %d\n------\n", node->start, node->end);

	address_range_tree_dump(&t);

	return 0;
}
