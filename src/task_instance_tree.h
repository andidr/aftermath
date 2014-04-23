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

#ifndef TASK_INSTANCE_TREE_H
#define TASK_INSTANCE_TREE_H

#include <stdlib.h>
#include "./contrib/linux-kernel/rbtree.h"
#include "./contrib/linux-kernel/list.h"
#include "task_instance.h"

struct task_instance_tree {
	struct rb_root root;
};

void task_instance_tree_init(struct task_instance_tree* t);
int task_instance_tree_insert(struct task_instance_tree* t, struct task_instance* n);
void task_instance_tree_remove(struct task_instance_tree* t, struct task_instance* n);
void task_instance_tree_reset_reached(struct task_instance_tree* t);
struct task_instance* task_instance_tree_iter_first(struct task_instance_tree* t);
struct task_instance* task_instance_tree_find(struct task_instance_tree* t, uint64_t task_addr, int cpu, uint64_t start);
struct task_instance* task_instance_tree_iter_next(struct task_instance* n);
struct task_instance* task_instance_tree_iter_first_postorder(struct task_instance_tree* t);
struct task_instance* task_instance_tree_iter_next_postorder(struct task_instance* n);
#endif
