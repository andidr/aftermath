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

#ifndef TASK_INSTANCE_RW_TREE_H
#define TASK_INSTANCE_RW_TREE_H

#include <stdlib.h>
#include "./contrib/linux-kernel/rbtree.h"
#include "./contrib/linux-kernel/list.h"
#include "task_instance.h"
#include "events.h"

struct task_instance_rw_tree {
	struct rb_root root;
};

struct task_instance_rw_tree_node {
	struct rb_node node;
	struct task_instance* instance;
	struct list_head list_out_deps;
	struct list_head list_in_deps;
	struct address_range_tree_node* address_range_node;
	struct task_instance_rw_tree_node* prodcons_counterpart;
	struct comm_event* comm_event;

	int reached;
};

void task_instance_rw_tree_init(struct task_instance_rw_tree* t);
void task_instance_rw_tree_reset_deps(struct task_instance_rw_tree* t);
int task_instance_rw_tree_insert(struct task_instance_rw_tree* t, struct task_instance_rw_tree_node* n);
void task_instance_rw_tree_remove(struct task_instance_rw_tree* t, struct task_instance_rw_tree_node* n);
int task_instance_rw_tree_clone(struct task_instance_rw_tree* t, struct task_instance_rw_tree* clone);
void task_instance_rw_tree_destroy(struct task_instance_rw_tree* t);
void task_instance_rw_tree_node_init(struct task_instance_rw_tree_node* n, struct task_instance* inst, struct comm_event* ce);
struct task_instance_rw_tree_node* task_instance_rw_tree_node_alloc_init(struct task_instance* inst, struct comm_event* ce);
struct task_instance_rw_tree_node* task_instance_rw_tree_iter_first(struct task_instance_rw_tree* t);
struct task_instance_rw_tree_node* task_instance_rw_tree_iter_next(struct task_instance_rw_tree_node* n);
#endif
