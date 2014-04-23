/**
 * Copyright (C) 2014 Andi Drebes <andi.drebes@lip6.fr>
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

#include "address_range_tree.h"
#include "./contrib/linux-kernel/interval_tree_generic.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

#define START(node) ((node)->start)
#define END(node) ((node)->end)

INTERVAL_TREE_DEFINE(struct address_range_tree_node, rb,
		     uint64_t, __subtree_last,
		     START, END,, __address_range_tree)

void address_range_tree_init(struct address_range_tree* t)
{
	t->root = RB_ROOT;
	INIT_LIST_HEAD(&t->list_nodeps);
	task_instance_tree_init(&t->all_instances);
}

void address_range_tree_insert(struct address_range_tree* t,
			       struct address_range_tree_node* node)
{
	__address_range_tree_insert(node, &t->root);
}

void address_range_tree_remove(struct address_range_tree* t,
			       struct address_range_tree_node* node)
{
	__address_range_tree_remove(node, &t->root);
}

struct address_range_tree_node* address_range_tree_iter_first(struct address_range_tree* t,
							      uint64_t start,
							      uint64_t last)
{
	return __address_range_tree_iter_first(&t->root, start, last);
}

struct address_range_tree_node* address_range_tree_iter_next(struct address_range_tree_node* n,
							     uint64_t start,
							     uint64_t last)
{
	return __address_range_tree_iter_next(n, start, last);
}

void address_range_tree_node_init(struct address_range_tree_node* n,
				  uint64_t start,
				  uint64_t end)
{
	task_instance_rw_tree_init(&n->reader_tree);
	task_instance_rw_tree_init(&n->writer_tree);

	n->start = start;
	n->end = end;
}

struct address_range_tree_node* address_range_tree_create_insert_node(struct address_range_tree* t,
								      uint64_t range_start,
								      uint64_t range_end)
{
	struct address_range_tree_node* node;

	if(!(node = malloc(sizeof(struct address_range_tree_node))))
		return NULL;

	address_range_tree_node_init(node, range_start, range_end);
	address_range_tree_insert(t, node);

	return node;
}

void address_range_tree_node_add_inst(struct address_range_tree_node* node,
				      struct task_instance_rw_tree_node* tin,
				      struct comm_event* ce)
{
	tin->comm_event = ce;

	if(ce->type == COMM_TYPE_DATA_READ) {
		task_instance_rw_tree_insert(&node->reader_tree, tin);
		list_add_tail(&tin->list_in_deps, &tin->instance->list_in_deps);
	} else {
		task_instance_rw_tree_insert(&node->writer_tree, tin);
		list_add_tail(&tin->list_out_deps, &tin->instance->list_out_deps);
	}

	tin->address_range_node = node;
}

int address_range_tree_node_clone(struct address_range_tree_node* node,
				  struct address_range_tree_node* clone)
{
	address_range_tree_node_init(clone, node->start, node->end);
	clone->start = node->start;
	clone->end = node->end;

	if(task_instance_rw_tree_clone(&node->reader_tree, &clone->reader_tree))
		return 1;

	if(task_instance_rw_tree_clone(&node->writer_tree, &clone->writer_tree)) {
		task_instance_rw_tree_destroy(&node->reader_tree);
		return 1;
	}

	for(struct rb_node* node = rb_first(&clone->writer_tree.root);
	    node;
	    node = rb_next(node))
	{
		struct task_instance_rw_tree_node* this_node =
			rb_entry(node, struct task_instance_rw_tree_node, node);

		INIT_LIST_HEAD(&this_node->list_out_deps);
		list_add_tail(&this_node->list_out_deps, &this_node->instance->list_out_deps);

		this_node->address_range_node = clone;
	}

	for(struct rb_node* node = rb_first(&clone->reader_tree.root);
	    node;
	    node = rb_next(node))
	{
		struct task_instance_rw_tree_node* this_node =
			rb_entry(node, struct task_instance_rw_tree_node, node);

		INIT_LIST_HEAD(&this_node->list_in_deps);
		list_add_tail(&this_node->list_in_deps, &this_node->instance->list_in_deps);

		this_node->address_range_node = clone;
	}

	return 0;
}

int address_range_tree_split_node(struct address_range_tree* t,
				  struct address_range_tree_node* node,
				  uint64_t split,
				  struct address_range_tree_node** left,
				  struct address_range_tree_node** right)
{
	struct address_range_tree_node* clone;

	if(!(clone = malloc(sizeof(struct address_range_tree_node))))
		return 1;

	if(address_range_tree_node_clone(node, clone)) {
		free(clone);
		return 1;
	}

	address_range_tree_remove(t, node);

	assert(split > node->start);
	assert(split < node->end);

	node->start = split;
	clone->end = split-1;

	assert(node->end > node->start);
	assert(clone->end > clone->start);

	address_range_tree_insert(t, node);
	address_range_tree_insert(t, clone);

	*left = clone;
	*right = node;

	return 0;
}

int address_range_tree_add_address_range(struct address_range_tree* t,
					 struct task_instance* inst,
					 uint64_t range_start,
					 uint64_t range_end,
					 struct comm_event* ce)
{
	struct task_instance_rw_tree_node* tin;
	struct address_range_tree_node* split_left;
	struct address_range_tree_node* split_right;
	struct address_range_tree_node* node = address_range_tree_iter_first(t, range_start, range_end);

	if(!node) {
		assert(range_end > range_start);
		if(!(node = address_range_tree_create_insert_node(t, range_start, range_end)))
			goto out_err;
	}

	uint64_t node_start = node->start;
	uint64_t node_end = node->end;

	if(!(tin = task_instance_rw_tree_node_alloc_init(inst, ce)))
		goto out_err;

	if(range_start == node_start && range_end == node_end) {
		/* Identical ranges */
		address_range_tree_node_add_inst(node, tin, ce);
	} else if(range_start < node_start && range_end <= node_end) {
		/* Right part of range overlaps into node */
		if(range_end < node_end) {
			if(address_range_tree_split_node(t, node, range_end+1, &split_left, &split_right))
				goto out_err_tin;

			node = split_left;
		}

		address_range_tree_node_add_inst(node, tin, ce);

		/* Handle remaining part not included neither in split_left nor in split_right */
		return address_range_tree_add_address_range(t, inst, range_start, node_start - 1, ce);
	} else if(range_start >= node_start && range_end > node_end) {
		/* Left part of range overlaps into node */
		if(range_start > node_start) {
			if(address_range_tree_split_node(t, node, range_start, &split_left, &split_right))
				goto out_err_tin;

			node = split_right;
		}

		address_range_tree_node_add_inst(node, tin, ce);

		/* Handle remaining part not included neither in split_left nor in split_right */
		return address_range_tree_add_address_range(t, inst, node_end + 1, range_end, ce);
	} else if(range_start < node_start && range_end > node_end) {
		/* Node fully included in range */
		address_range_tree_node_add_inst(node, tin, ce);

		if(address_range_tree_add_address_range(t, inst, range_start, node_start - 1, ce) ||
		   address_range_tree_add_address_range(t, inst, node_end + 1, range_end, ce))
			goto out_err;
	} else if(range_start >= node_start && range_end <= node_end) {
		/* Range fully included in node */

		if(range_start > node_start) {
			if(address_range_tree_split_node(t, node, range_start, &split_left, &split_right))
				goto out_err_tin;

			node = split_right;
		}

		if(range_end < node_end) {
			if(address_range_tree_split_node(t, node, range_end+1, &split_left, &split_right))
				goto out_err_tin;

			node = split_left;
		}

		address_range_tree_node_add_inst(node, tin, ce);
	} else {
		/* Cannot happen */
		assert(0);
	}

	return 0;

out_err_tin:
	free(tin);
out_err:
	return 1;
}

int address_range_tree_add_task_instance(struct address_range_tree* t,
					 struct single_event* texec_start,
					 struct single_event* texec_end)
{
	struct task_instance* inst;

	if(!(inst = malloc(sizeof(struct task_instance))))
		return 1;

	task_instance_init(inst, texec_start->time, texec_end->time, texec_start->active_task, texec_start->event_set->cpu);
	task_instance_tree_insert(&t->all_instances, inst);

	struct event_set* es = texec_start->event_set;
	struct comm_event* ce;

	for_each_comm_event_in_interval(es, inst->start, inst->end, ce) {
		if(ce->type == COMM_TYPE_DATA_READ || ce->type == COMM_TYPE_DATA_WRITE) {
			if(address_range_tree_add_address_range(t, inst, ce->what->addr,
								     ce->what->addr + ce->size - 1,
								     ce))
			{
				return 1;
			}

			if(ce->type == COMM_TYPE_DATA_READ)
				inst->num_read_deps += ce->size;
		}
	}

	if(inst->num_read_deps == 0)
		list_add(&inst->list_nodeps, &t->list_nodeps);

	return 0;
}

int address_range_tree_from_event_set(struct address_range_tree* t, struct event_set* es)
{
	if(es->num_single_events == 0)
		return 0;

	struct single_event* first_sge = &es->single_events[0];

	if(first_sge->type != SINGLE_TYPE_TEXEC_START)
		first_sge = first_sge->next_texec_start;

	if(!first_sge)
		return 0;

	for(struct single_event* sge = first_sge; sge; sge = sge->next_texec_start)
		if(address_range_tree_add_task_instance(t, sge, sge->next_texec_end))
			return 1;

	return 0;
}

int address_range_tree_node_match_prodcons(struct address_range_tree_node* n)
{
	struct task_instance_rw_tree_node* prod;
	struct task_instance_rw_tree_node* cons;
	struct rb_node* prod_rb = rb_first(&n->writer_tree.root);
	struct rb_node* cons_rb = rb_first(&n->reader_tree.root);

	if(!prod_rb || !cons_rb)
		return 1;

	while(prod_rb && cons_rb) {
		prod = rb_entry(prod_rb, struct task_instance_rw_tree_node, node);
		cons = rb_entry(cons_rb, struct task_instance_rw_tree_node, node);

		if(prod->instance->start > cons->instance->start)
			return 1;

		prod->prodcons_counterpart = cons;
		cons->prodcons_counterpart = prod;

		prod_rb = rb_next(prod_rb);
		cons_rb = rb_next(cons_rb);
	}

	if(prod_rb || cons_rb)
		return 1;

	return 0;
}

int address_range_tree_match_prodcons(struct address_range_tree* t)
{
	for(struct address_range_tree_node* node = address_range_tree_iter_first(t, 0, UINT64_MAX);
	    node;
	    node = address_range_tree_iter_next(node, 0, UINT64_MAX))
	{
		if(address_range_tree_node_match_prodcons(node))
			return 1;
	}

	return 0;
}

int address_range_tree_from_multi_event_set(struct address_range_tree* t, struct multi_event_set* mes)
{
	for(int i = 0; i < mes->num_sets; i++)
		if(address_range_tree_from_event_set(t, &mes->sets[i]))
			return 1;

	if(address_range_tree_match_prodcons(t))
		return 1;

	return 0;
}

void address_range_tree_reset_deps(struct address_range_tree* t)
{
	for(struct address_range_tree_node* node = address_range_tree_iter_first(t, 0, UINT64_MAX);
	    node;
	    node = address_range_tree_iter_next(node, 0, UINT64_MAX))
	{
		task_instance_rw_tree_reset_deps(&node->reader_tree);
		task_instance_rw_tree_reset_deps(&node->writer_tree);
	}
}

void address_range_tree_reset_depths(struct address_range_tree* t)
{
	for(struct task_instance* inst = task_instance_tree_iter_first(&t->all_instances);
	    inst;
	    inst = task_instance_tree_iter_next(inst))
	{
		inst->depth = 0;
		inst->reached = 0;
	}
}

void address_range_tree_sim_texec_rec(struct task_instance* inst, int* max_depth)
{
	struct list_head* iter;

	inst->reached = 1;

	if(*max_depth < inst->depth)
		*max_depth = inst->depth;

	list_for_each(iter, &inst->list_out_deps) {
		struct task_instance_rw_tree_node* tin =
			list_entry(iter, struct task_instance_rw_tree_node, list_out_deps);

		struct task_instance_rw_tree_node* tin_reader = tin->prodcons_counterpart;
		struct task_instance* reader_inst = tin_reader->instance;
		uint64_t write_size = address_range_tree_node_size(tin->address_range_node);

		tin->reached = 1;
		tin_reader->reached = 1;

		assert(tin->address_range_node == tin_reader->address_range_node);

		reader_inst->remaining_read_deps -= write_size;

		if(reader_inst->depth < inst->depth + 1)
			reader_inst->depth = inst->depth + 1;

		if(reader_inst->remaining_read_deps == 0)
			address_range_tree_sim_texec_rec(reader_inst, max_depth);
	}
}

void address_range_tree_calculate_depths(struct address_range_tree* t, int* max_depth)
{
	address_range_tree_reset_deps(t);
	address_range_tree_reset_depths(t);

	*max_depth = 0;
	struct list_head* iter;
	list_for_each(iter, &t->list_nodeps) {
		struct task_instance* inst =
			list_entry(iter, struct task_instance, list_nodeps);
		address_range_tree_sim_texec_rec(inst, max_depth);
	}
}

int address_range_tree_build_parallelism_histogram(struct address_range_tree* t, struct histogram* h)
{
	int max_depth;

	address_range_tree_calculate_depths(t, &max_depth);

	if(histogram_init(h, max_depth+1, 0, max_depth))
		return 1;

	for(struct task_instance* inst = task_instance_tree_iter_first(&t->all_instances);
	    inst;
	    inst = task_instance_tree_iter_next(inst))
	{
		h->values[inst->depth] += 1;
		h->num_hist++;

		if(h->max_hist < h->values[inst->depth])
			h->max_hist = h->values[inst->depth];
	}

	if(h->max_hist > 0)
		for(int i = 0; i < h->num_bins; i++)
			h->values[i] /= h->max_hist;

	return 0;
}

void __address_range_tree_destroy(struct address_range_tree* t, struct rb_node* node)
{
	if(!node)
		return;

	struct address_range_tree_node* this_node =
		rb_entry(node, struct address_range_tree_node, rb);

	if(node->rb_left)
		__address_range_tree_destroy(t, node->rb_left);

	if(node->rb_right)
		__address_range_tree_destroy(t, node->rb_right);

	task_instance_rw_tree_destroy(&this_node->reader_tree);
	task_instance_rw_tree_destroy(&this_node->writer_tree);

	free(this_node);
}

void address_range_tree_destroy(struct address_range_tree* t)
{
	__address_range_tree_destroy(t, t->root.rb_node);

	for(struct task_instance* inst = task_instance_tree_iter_first_postorder(&t->all_instances);
	    inst;
	    inst = task_instance_tree_iter_next_postorder(inst))
	{
		free(inst);
	}
}

void address_range_tree_dump(struct address_range_tree* t)
{
	for(struct address_range_tree_node* node =
		    address_range_tree_iter_first(t, 0, UINT64_MAX);
	    node;
	    node = address_range_tree_iter_next(node, 0, UINT64_MAX))
	{
		printf("Found %"PRIx64"-%"PRIx64" (%"PRId64" bytes)\n",
		       node->start, node->end, address_range_tree_node_size(node));
		printf("\tReaders:\n");

		for(struct task_instance_rw_tree_node* inst_node =
			    task_instance_rw_tree_iter_first(&node->reader_tree);
		    inst_node;
		    inst_node = task_instance_rw_tree_iter_next(inst_node))
		{
			printf("\t\t%s @ %"PRIu64"\n",
			       inst_node->instance->task->symbol_name,
			       inst_node->instance->start);
		}

		printf("\tWriters:\n");

		for(struct task_instance_rw_tree_node* inst_node =
			    task_instance_rw_tree_iter_first(&node->writer_tree);
		    inst_node;
		    inst_node = task_instance_rw_tree_iter_next(inst_node))
		{
			printf("\t\t%s @ %"PRIu64"\n",
			       inst_node->instance->task->symbol_name,
			       inst_node->instance->start);
		}
		printf("\n");
	}

	printf("Task instances:\n");

	for(struct task_instance* inst = task_instance_tree_iter_first(&t->all_instances);
	    inst;
	    inst = task_instance_tree_iter_next(inst))
	{
		printf("\t%s @ %"PRIu64" (reached = %d, depth %d, rem = %"PRIu64")\n",
		       inst->task->symbol_name,
		       inst->start,
		       inst->reached,
		       inst->depth,
		       inst->remaining_read_deps);

		struct list_head* iter2;
		list_for_each(iter2, &inst->list_out_deps) {
			struct task_instance_rw_tree_node* tin =
				list_entry(iter2, struct task_instance_rw_tree_node, list_out_deps);

			printf("\t\tWrites %"PRId64" bytes to 0x%"PRIx64" (reached = %d)\n",
			       address_range_tree_node_size(tin->address_range_node),
			       tin->address_range_node->start,
			       tin->reached);
		}

		list_for_each(iter2, &inst->list_in_deps) {
			struct task_instance_rw_tree_node* tin =
				list_entry(iter2, struct task_instance_rw_tree_node, list_in_deps);

			printf("\t\tReads %"PRId64" bytes from 0x%"PRIx64" - 0x%"PRIx64" (reached = %d)\n",
			       address_range_tree_node_size(tin->address_range_node),
			       tin->address_range_node->start,
			       tin->address_range_node->end,
			       tin->reached);
		}
	}
}
