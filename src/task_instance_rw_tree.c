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

#include "task_instance_rw_tree.h"

void task_instance_rw_tree_init(struct task_instance_rw_tree* t)
{
	t->root = RB_ROOT;
}

int task_instance_rw_tree_insert(struct task_instance_rw_tree* t, struct task_instance_rw_tree_node* n)
{
	struct rb_node** new = &t->root.rb_node;
	struct rb_node* parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct task_instance_rw_tree_node* this = container_of(*new, struct task_instance_rw_tree_node, node);
		parent = *new;

		if (n->instance->start < this->instance->start)
			new = &((*new)->rb_left);
		else if (n->instance->start > this->instance->start)
			new = &((*new)->rb_right);
		else
			return 1;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&n->node, parent, new);
	rb_insert_color(&n->node, &t->root);

	return 0;
}

void task_instance_rw_tree_remove(struct task_instance_rw_tree* t, struct task_instance_rw_tree_node* n)
{
	rb_erase(&n->node, &t->root);
}

int task_instance_rw_tree_clone(struct task_instance_rw_tree* t, struct task_instance_rw_tree* cloned_tree)
{
	task_instance_rw_tree_init(cloned_tree);

	for(struct rb_node* node = rb_first(&t->root); node; node = rb_next(node)) {
		struct task_instance_rw_tree_node* this_node = rb_entry(node, struct task_instance_rw_tree_node, node);
		struct task_instance_rw_tree_node* cloned_node = task_instance_rw_tree_node_alloc_init(this_node->instance, this_node->comm_event);

		if(!cloned_node) {
			task_instance_rw_tree_destroy(cloned_tree);
			return 1;
		}

		task_instance_rw_tree_insert(cloned_tree, cloned_node);
	}

	return 0;
}

void task_instance_rw_tree_reset_deps(struct task_instance_rw_tree* t)
{
	for(struct rb_node* node = rb_first(&t->root); node; node = rb_next(node)) {
		struct task_instance_rw_tree_node* this_node = rb_entry(node, struct task_instance_rw_tree_node, node);
		this_node->instance->remaining_read_deps = this_node->instance->num_read_deps;
		this_node->reached = 0;
	}
}

struct task_instance_rw_tree_node* task_instance_rw_tree_iter_first(struct task_instance_rw_tree* t)
{
	struct rb_node* node = rb_first(&t->root);

	if(!node)
		return NULL;

	struct task_instance_rw_tree_node* this_node = rb_entry(node, struct task_instance_rw_tree_node, node);
	return this_node;
}

struct task_instance_rw_tree_node* task_instance_rw_tree_iter_next(struct task_instance_rw_tree_node* n)
{
	struct rb_node* node = rb_next(&n->node);

	if(!node)
		return NULL;

	struct task_instance_rw_tree_node* this_node = rb_entry(node, struct task_instance_rw_tree_node, node);
	return this_node;
}

void __task_instance_rw_tree_destroy(struct task_instance_rw_tree* t, struct rb_node* node)
{
	if(!node)
		return;

	struct task_instance_rw_tree_node* this_node = rb_entry(node, struct task_instance_rw_tree_node, node);

	if(node->rb_left)
		__task_instance_rw_tree_destroy(t, node->rb_left);

	if(node->rb_right)
		__task_instance_rw_tree_destroy(t, node->rb_right);

	free(this_node);
}

void task_instance_rw_tree_destroy(struct task_instance_rw_tree* t)
{
	__task_instance_rw_tree_destroy(t, t->root.rb_node);
}

void task_instance_rw_tree_node_init(struct task_instance_rw_tree_node* n, struct task_instance* inst, struct comm_event* ce)
{
	n->instance = inst;
	n->address_range_node = NULL;
	n->prodcons_counterpart = NULL;
	n->comm_event = ce;

	INIT_LIST_HEAD(&n->list_out_deps);
	INIT_LIST_HEAD(&n->list_in_deps);
}

struct task_instance_rw_tree_node* task_instance_rw_tree_node_alloc_init(struct task_instance* inst, struct comm_event* ce)
{
	struct task_instance_rw_tree_node* node;

	if(!(node = malloc(sizeof(struct task_instance_rw_tree_node))))
		return NULL;

	task_instance_rw_tree_node_init(node, inst, ce);

	return node;
}
