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

#ifndef ADDRESS_RANGE_TREE
#define ADDRESS_RANGE_TREE

#include <stdint.h>
#include "./contrib/linux-kernel/rbtree.h"
#include "./contrib/linux-kernel/list.h"
#include "task_instance_rw_tree.h"
#include "task_instance_tree.h"
#include "multi_event_set.h"
#include "statistics.h"

struct address_range_tree {
	struct rb_root root;
	struct list_head list_nodeps;
	struct task_instance_tree all_instances;
};

struct address_range_tree_node {
	struct rb_node rb;
	struct task_instance_rw_tree reader_tree;
	struct task_instance_rw_tree writer_tree;

	uint64_t start;
	uint64_t end;
	uint64_t __subtree_last;
};

void address_range_tree_init(struct address_range_tree* t);
void address_range_tree_destroy(struct address_range_tree* t);

void address_range_tree_node_init(struct address_range_tree_node* n,
				  uint64_t start,
				  uint64_t end);

static inline uint64_t address_range_tree_node_size(struct address_range_tree_node* n)
{
	return n->end - n->start + 1;
}

void address_range_tree_insert(struct address_range_tree* t, struct address_range_tree_node* node);

void address_range_tree_remove(struct address_range_tree* t, struct address_range_tree_node* node);

struct address_range_tree_node* address_range_tree_iter_first(struct address_range_tree* t,
							      uint64_t start,
							      uint64_t last);

struct address_range_tree_node* address_range_tree_iter_next(struct address_range_tree_node* n,
							     uint64_t start,
							     uint64_t last);

int address_range_tree_from_event_set(struct address_range_tree* t, struct event_set* es);
int address_range_tree_from_multi_event_set(struct address_range_tree* t, struct multi_event_set* mes);
void address_range_tree_reset_deps(struct address_range_tree* t);
void address_range_tree_reset_depths(struct address_range_tree* t);
void address_range_tree_calculate_depths(struct address_range_tree* t, int* max_depth);
int address_range_tree_build_parallelism_histogram(struct address_range_tree* t, struct histogram* h);
void address_range_tree_dump(struct address_range_tree* t);
void address_range_tree_get_predecessors(struct address_range_tree* t, struct task_instance* inst, int max_depth, struct list_head* heads, int* num_found, int* total);
void address_range_tree_get_successors(struct address_range_tree* t, struct task_instance* inst, int max_depth, struct list_head* heads, int* num_found, int* total);

#endif
