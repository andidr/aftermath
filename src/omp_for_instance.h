/**
 * Copyright (C) 2015 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#ifndef OMP_FOR_INSTANCE_H
#define OMP_FOR_INSTANCE_H

#include <stdlib.h>
#include <stdint.h>
#include <search.h>
#include "omp_for_chunk_set.h"
#include "color.h"
#include "typed_list.h"

void tdestroy(void *root, void (*free_node)(void *nodep));

struct omp_trace_info_ofi {
	uint64_t addr;
	uint64_t id;
};

struct omp_for_instance {
	union {
		/* The field ti_ofi is only used when a trace is
		 * loaded and is replaced by a pointer to the
		 * associated for_loop */
		struct omp_for* for_loop;
		struct omp_trace_info_ofi* ti_ofi;
	};

	struct list_head for_chunk_sets;
	struct list_head list;

	uint32_t num_chunk_sets;
	uint32_t flags;
	uint64_t increment;
	uint64_t iter_start;
	uint64_t iter_end;
	uint32_t num_workers;

	uint64_t total_time;
	double chunk_load_balance;
	double parallelism_efficiency;
	uint64_t real_loop_time;

	double color_r;
	double color_g;
	double color_b;
};

struct omp_for_instance_tree {
	void* root;
	int num_omp_for_instances;
};

static inline void omp_for_instance_tree_init(struct omp_for_instance_tree* ofit)
{
	ofit->root = NULL;
	ofit->num_omp_for_instances = 0;
}

int compare_omp_for_instances(const void *pt1, const void *pt2);
int compare_omp_for_instancesp(const void *pt1, const void *pt2);

static inline struct omp_for_instance* omp_for_instance_tree_find(struct omp_for_instance_tree* ofit, uint64_t id)
{
	struct omp_for_instance key;
	struct omp_trace_info_ofi* key_ti = malloc(sizeof(struct omp_trace_info_ofi));;
	key.ti_ofi = key_ti;
	key.ti_ofi->id = id;
	struct omp_for_instance** pret = tfind(&key, &ofit->root, compare_omp_for_instances);

	if(pret)
		return *pret;

	return NULL;
}

static inline struct omp_for_instance* omp_for_instance_tree_add(struct omp_for_instance_tree* ofit, uint64_t id)
{
	struct omp_for_instance* key = malloc(sizeof(struct omp_for_instance));
	struct omp_trace_info_ofi* key_ti = malloc(sizeof(struct omp_trace_info_ofi));;
	struct omp_for_instance** ofi;

	if(!key || !key_ti)
		return NULL;

	key->ti_ofi = key_ti;
	key->ti_ofi->id = id;

	if((ofi = tsearch(key, &ofit->root, compare_omp_for_instances)) == NULL) {
		free(key);
		free(key_ti);
		return NULL;
	}

	ofit->num_omp_for_instances++;

	return key;
}

static inline struct omp_for_instance* omp_for_instance_tree_find_or_add(struct omp_for_instance_tree* ofit, uint64_t id)
{
	struct omp_for_instance* ret = omp_for_instance_tree_find(ofit, id);

	if(!ret)
		return omp_for_instance_tree_add(ofit, id);

	return ret;
}

void omp_for_instance_tree_walk(const void* p, const VISIT which, const int depth);
int omp_for_instance_tree_to_array(struct omp_for_instance_tree* ofit, struct omp_for_instance** arr);

static inline void omp_for_instance_tree_destroy(struct omp_for_instance_tree* ofit)
{
	tdestroy(ofit->root, free);
}

static inline int omp_for_instance_init(struct omp_for_instance* ofi, uint32_t flags, uint64_t dsk_id,
					uint64_t increment, uint64_t iter_start, uint64_t iter_end, uint64_t addr,
					uint32_t num_workers)
{
	ofi->num_chunk_sets = 0;
	ofi->flags = flags;
	ofi->increment = increment;
	ofi->iter_start = iter_start;
	ofi->iter_end = iter_end;
	ofi->num_workers = num_workers;
	ofi->ti_ofi->addr = addr;
	ofi->ti_ofi->id = dsk_id;
	ofi->for_chunk_sets.next = NULL;
	ofi->color_r = omp_for_colors[((uintptr_t)ofi >> sizeof(uintptr_t)) % NUM_OMP_FOR_COLORS][0];
	ofi->color_g = omp_for_colors[((uintptr_t)ofi >> sizeof(uintptr_t)) % NUM_OMP_FOR_COLORS][1];
	ofi->color_b = omp_for_colors[((uintptr_t)ofi >> sizeof(uintptr_t)) % NUM_OMP_FOR_COLORS][2];

	return 0;
}

#define omp_for_instance_for_each_chunk_set(ofi, ofcs) \
	typed_list_for_each(ofi, for_chunk_sets, ofcs, list)

#define omp_for_instance_for_each_chunk_set_prev(ofi, ofcs) \
	typed_list_for_each_prev(ofi, for_chunk_sets, ofcs, list)

#define omp_for_instance_for_each_chunk_set_safe(ofi, n, i) \
	typed_list_for_each_safe(ofi, for_chunk_sets, ofcs, i, list)

#define omp_for_instance_for_each_chunk_set_prev_safe(ofi, n, i) \
	typed_list_for_each_prev_safe(ofi, for_chunk_sets, ofcs, i, list)

struct omp_for_chunk_set*
omp_for_instance_find_lowest_chunk_set(const struct omp_for_instance* ofi);
uint64_t omp_for_instance_static_chunk_size(const struct omp_for_instance* ofi);

#endif
