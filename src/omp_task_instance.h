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

#ifndef OMP_TASK_INSTANCE_H
#define OMP_TASK_INSTANCE_H

#include <stdlib.h>
#include <stdint.h>
#include "omp_task_part.h"
#include "color.h"

void tdestroy(void *root, void (*free_node)(void *nodep));

struct omp_trace_info_oti {
	uint64_t addr;
	uint64_t id;
};

struct omp_task_instance {
	union {
		/* The field ti_oti is only used when a trace is
		 * loaded and is replaced by a pointer to the
		 * associated task */
		struct omp_task* task;
		struct omp_trace_info_oti* ti_oti;
	};

	struct list_head task_parts;
	struct list_head list;

	uint32_t num_task_parts;

	double color_r;
	double color_g;
	double color_b;
};

struct omp_task_instance_tree {
	void* root;
	int num_omp_task_instances;
};

static inline void omp_task_instance_tree_init(struct omp_task_instance_tree* otit)
{
	otit->root = NULL;
	otit->num_omp_task_instances = 0;
}

int compare_omp_task_instances(const void *pt1, const void *pt2);
int compare_omp_task_instancesp(const void *pt1, const void *pt2);

static inline struct omp_task_instance* omp_task_instance_tree_find(struct omp_task_instance_tree* otit, uint64_t id)
{
	struct omp_task_instance key;
	struct omp_trace_info_oti* key_ti = malloc(sizeof(struct omp_trace_info_oti));;
	key.ti_oti = key_ti;
	key.ti_oti->id = id;
	struct omp_task_instance** pret = tfind(&key, &otit->root, compare_omp_task_instances);

	if(pret)
		return *pret;

	free(key_ti);

	return NULL;
}

static inline struct omp_task_instance* omp_task_instance_tree_add(struct omp_task_instance_tree* otit, uint64_t id)
{
	struct omp_task_instance* key = malloc(sizeof(struct omp_task_instance));
	struct omp_trace_info_oti* key_ti = malloc(sizeof(struct omp_trace_info_oti));;
	struct omp_task_instance** oft;

	if(!key || !key_ti)
		return NULL;

	key->ti_oti = key_ti;
	key->ti_oti->id = id;

	if((oft = tsearch(key, &otit->root, compare_omp_task_instances)) == NULL) {
		free(key);
		free(key_ti);
		return NULL;
	}

	otit->num_omp_task_instances++;

	return key;
}

static inline struct omp_task_instance* omp_task_instance_tree_find_or_add(struct omp_task_instance_tree* otit, uint64_t id)
{
	struct omp_task_instance* ret = omp_task_instance_tree_find(otit, id);

	if(!ret)
		return omp_task_instance_tree_add(otit, id);

	return ret;
}

void omp_task_instance_tree_walk(const void* p, const VISIT which, const int depth);
int omp_task_instance_tree_to_array(struct omp_task_instance_tree* otit, struct omp_task_instance** arr);

static inline void omp_task_instance_tree_destroy(struct omp_task_instance_tree* otit)
{
	tdestroy(otit->root, free);
}

static inline int omp_task_instance_init(struct omp_task_instance* oti, uint64_t dsk_id, uint64_t addr)
{
	oti->num_task_parts = 0;
	oti->ti_oti->id = dsk_id;
	oti->ti_oti->addr = addr;
	oti->task_parts.next = NULL;
	oti->color_r = omp_task_colors[((uintptr_t)oti >> sizeof(uintptr_t)) % NUM_OMP_TASK_COLORS][0];
	oti->color_g = omp_task_colors[((uintptr_t)oti >> sizeof(uintptr_t)) % NUM_OMP_TASK_COLORS][1];
	oti->color_b = omp_task_colors[((uintptr_t)oti >> sizeof(uintptr_t)) % NUM_OMP_TASK_COLORS][2];

	return 0;
}

#endif
