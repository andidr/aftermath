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

#ifndef OMP_FOR_CHUNK_H
#define OMP_FOR_CHUNK_H

#include <stdint.h>
#include <search.h>
#include "omp_for_chunk_set_part.h"
#include "color.h"

void tdestroy(void *root, void (*free_node)(void *nodep));

struct omp_trace_info_ofc {
	uint64_t for_instance_id;
	uint64_t id;
};

struct omp_for_chunk_set {
	union {
		/* The field ti_ofc is only used when a trace is
		 * loaded and is later replaced by a pointer to the
		 * associated for instance */
		struct omp_for_instance *for_instance;
		struct omp_trace_info_ofc *ti_ofc;
	};

	struct list_head list;
	struct list_head chunk_set_parts;

	uint32_t num_chunk_set_parts;
	uint64_t iter_start;
	uint64_t iter_end;

	uint64_t total_time;
	uint64_t min_start;
	uint64_t max_end;

	double color_r;
	double color_g;
	double color_b;
};

struct omp_for_chunk_set_tree {
	void* root;
	int num_omp_for_chunk_sets;
};

static inline void omp_for_chunk_set_tree_init(struct omp_for_chunk_set_tree* ofcst)
{
	ofcst->root = NULL;
	ofcst->num_omp_for_chunk_sets = 0;
}

int compare_omp_for_chunk_sets(const void *pt1, const void *pt2);
int compare_omp_for_chunk_setsp(const void *pt1, const void *pt2);

static inline struct omp_for_chunk_set* omp_for_chunk_set_tree_find(struct omp_for_chunk_set_tree* ofcst, uint64_t id)
{
	struct omp_for_chunk_set key;
	struct omp_trace_info_ofc* key_ti = malloc(sizeof(struct omp_trace_info_ofc));;
	key.ti_ofc = key_ti;
	key.ti_ofc->id = id;
	struct omp_for_chunk_set** pret = tfind(&key, &ofcst->root, compare_omp_for_chunk_sets);

	if(pret)
		return *pret;

	free(key_ti);

	return NULL;
}

static inline struct omp_for_chunk_set* omp_for_chunk_set_tree_add(struct omp_for_chunk_set_tree* ofcst, uint64_t id)
{
	struct omp_for_chunk_set* key = malloc(sizeof(struct omp_for_chunk_set));
	struct omp_trace_info_ofc* key_ti = malloc(sizeof(struct omp_trace_info_ofc));;
	struct omp_for_chunk_set** ofcs;

	if(!key || !key_ti)
		return NULL;

	key->ti_ofc = key_ti;
	key->ti_ofc->id = id;

	if((ofcs = tsearch(key, &ofcst->root, compare_omp_for_chunk_sets)) == NULL) {
		free(key);
		free(key_ti);
		return NULL;
	}

	ofcst->num_omp_for_chunk_sets++;

	return key;
}

static inline struct omp_for_chunk_set* omp_for_chunk_set_tree_find_or_add(struct omp_for_chunk_set_tree* ofcst, uint64_t id)
{
	struct omp_for_chunk_set* ret = omp_for_chunk_set_tree_find(ofcst, id);

	if(!ret)
		return omp_for_chunk_set_tree_add(ofcst, id);

	return ret;
}

void omp_for_chunk_set_tree_walk(const void* p, const VISIT which, const int depth);
int omp_for_chunk_set_tree_to_array(struct omp_for_chunk_set_tree* ofcst, struct omp_for_chunk_set** arr);

static inline void omp_for_chunk_set_tree_destroy(struct omp_for_chunk_set_tree* ofcst)
{
	tdestroy(ofcst->root, free);
}

static inline int omp_for_chunk_set_init(struct omp_for_chunk_set* ofc, uint64_t dsk_for_id, uint64_t dsk_id,
				     uint64_t iter_start, uint64_t iter_end)
{
	ofc->num_chunk_set_parts = 0;
	ofc->ti_ofc->for_instance_id = dsk_for_id;
	ofc->ti_ofc->id = dsk_id;
	ofc->iter_start = iter_start;
	ofc->iter_end = iter_end;
	ofc->chunk_set_parts.next = NULL;
	ofc->color_r = omp_for_colors[((uintptr_t)ofc >> sizeof(uintptr_t)) % NUM_OMP_FOR_COLORS][0];
	ofc->color_g = omp_for_colors[((uintptr_t)ofc >> sizeof(uintptr_t)) % NUM_OMP_FOR_COLORS][1];
	ofc->color_b = omp_for_colors[((uintptr_t)ofc >> sizeof(uintptr_t)) % NUM_OMP_FOR_COLORS][2];

	return 0;
}

#endif
