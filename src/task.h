/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
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

#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <malloc.h>
#define _GNU_SOURCE
#define __USE_GNU 1
#include <search.h>
#include <sys/types.h>

struct task {
	uint64_t work_fn;
	char* source_filename;
	char* symbol_name;
	int source_line;
};

struct task_tree {
	void* root;
	int num_tasks;
};

static inline void task_destroy(struct task* t)
{
	free(t->source_filename);
	free(t->symbol_name);
}

static inline void task_tree_init(struct task_tree* tt)
{
	tt->root = NULL;
	tt->num_tasks = 0;
}

int compare_tasks(const void *pt1, const void *pt2);
int compare_tasksp(const void *pt1, const void *pt2);

static inline struct task* task_tree_find(struct task_tree* tt, uint64_t work_fn)
{
	struct task key = { .work_fn = work_fn };
	return tfind(&key, &tt->root, compare_tasks);
}

static inline struct task* task_tree_add(struct task_tree* tt, uint64_t work_fn)
{
	struct task* key = malloc(sizeof(struct task));
	struct task** t;

	if(!key)
		return NULL;

	key->work_fn = work_fn;

	if((t = tsearch(key, &tt->root, compare_tasks)) == NULL) {
		free(key);
		return NULL;
	}

	(*t)->source_filename = NULL;
	(*t)->symbol_name = NULL;
	(*t)->source_line = -1;

	tt->num_tasks++;

	return key;
}

void task_tree_walk(const void* p, const VISIT which, const int depth);
int task_tree_to_array(struct task_tree* tt, struct task** arr);

static inline void task_tree_destroy(struct task_tree* tt)
{
	tdestroy(tt->root, free);
}

#endif
