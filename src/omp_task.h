/**
 * Copyright (C) 2016 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#ifndef OMP_TASK_H
#define OMP_TASK_H

#include <stdint.h>
#include "omp_task_instance.h"
#include "color.h"

void tdestroy(void *root, void (*free_node)(void *nodep));

struct omp_task {
	struct list_head task_instances;
	uint64_t addr;
	uint32_t num_instances;

	char* source_filename;
	char* symbol_name;
	int source_line;

	double color_r;
	double color_g;
	double color_b;
};

struct omp_task_tree {
	void* root;
	int num_omp_tasks;
};

static inline void omp_task_tree_init(struct omp_task_tree* ott)
{
	ott->root = NULL;
	ott->num_omp_tasks = 0;
}

int compare_omp_tasks(const void *pt1, const void *pt2);
int compare_omp_tasksp(const void *pt1, const void *pt2);

static inline struct omp_task* omp_task_tree_find(struct omp_task_tree* ott, uint64_t addr)
{
	struct omp_task key;
	key.addr = addr;
	struct omp_task** pret = tfind(&key, &ott->root, compare_omp_tasks);

	if(pret)
		return *pret;

	return NULL;
}

static inline struct omp_task* omp_task_tree_add(struct omp_task_tree* ott, uint64_t addr)
{
	struct omp_task* key = malloc(sizeof(struct omp_task));
	struct omp_task** ot;

	if(!key)
		return NULL;

	key->addr = addr;

	if((ot = tsearch(key, &ott->root, compare_omp_tasks)) == NULL) {
		free(key);
		return NULL;
	}

	ott->num_omp_tasks++;

	return key;
}

static inline struct omp_task* omp_task_tree_find_or_add(struct omp_task_tree* ott, uint64_t addr)
{
	struct omp_task* ret = omp_task_tree_find(ott, addr);

	if(!ret)
		return omp_task_tree_add(ott, addr);

	return ret;
}

void omp_task_tree_walk(const void* p, const VISIT which, const int depth);
int omp_task_tree_to_array(struct omp_task_tree* ott, struct omp_task** arr);

static inline void omp_task_tree_destroy(struct omp_task_tree* ott)
{
	tdestroy(ott->root, free);
}

static inline int omp_task_init(struct omp_task* ot, uint64_t addr)
{
	ot->addr = addr;
	ot->num_instances = 0;
	ot->source_line = -1;
	ot->symbol_name = NULL;
	ot->task_instances.next = NULL;
	ot->source_filename = NULL;
	ot->color_r = omp_task_colors[ot->addr % NUM_OMP_TASK_COLORS][0];
	ot->color_g = omp_task_colors[ot->addr % NUM_OMP_TASK_COLORS][1];
	ot->color_b = omp_task_colors[ot->addr % NUM_OMP_TASK_COLORS][2];

	return 0;
}

#endif
