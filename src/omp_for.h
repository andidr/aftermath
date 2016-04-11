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

#ifndef OMP_FOR_H
#define OMP_FOR_H

#include <stdint.h>
#include <search.h>
#include "omp_for_instance.h"
#include "color.h"

void tdestroy(void *root, void (*free_node)(void *nodep));

struct omp_for {
	struct list_head for_instances;
	uint64_t addr;
	uint32_t num_instances;
	char* source_filename;

	char* symbol_name;
	int source_line;

	double color_r;
	double color_g;
	double color_b;
};

struct omp_for_tree {
	void* root;
	int num_omp_fors;
};

static inline void omp_for_tree_init(struct omp_for_tree* oft)
{
	oft->root = NULL;
	oft->num_omp_fors = 0;
}

static inline int omp_for_init(struct omp_for* of, uint64_t addr)
{
	of->addr = addr;
	of->num_instances = 0;
	of->source_line = -1;
	of->symbol_name = NULL;
	of->source_filename = NULL;
	of->for_instances.next = NULL;
	of->color_r = omp_for_colors[of->addr % NUM_OMP_FOR_COLORS][0];
	of->color_g = omp_for_colors[of->addr % NUM_OMP_FOR_COLORS][1];
	of->color_b = omp_for_colors[of->addr % NUM_OMP_FOR_COLORS][2];

	return 0;
}

int compare_omp_fors(const void *pt1, const void *pt2);
int compare_omp_forsp(const void *pt1, const void *pt2);

static inline struct omp_for* omp_for_tree_find(struct omp_for_tree* oft, uint64_t addr)
{
	struct omp_for key = { .addr = addr };
	struct omp_for** pret = tfind(&key, &oft->root, compare_omp_fors);

	if(pret)
		return *pret;

	return NULL;
}

static inline struct omp_for* omp_for_tree_add(struct omp_for_tree* oft, uint64_t addr)
{
	struct omp_for* key = malloc(sizeof(struct omp_for));
	struct omp_for** of;

	if(!key)
		return NULL;

	key->addr = addr;

	if((of = tsearch(key, &oft->root, compare_omp_fors)) == NULL) {
		free(key);
		return NULL;
	}

	oft->num_omp_fors++;

	return key;
}

static inline struct omp_for* omp_for_tree_find_or_add(struct omp_for_tree* oft, uint64_t addr)
{
	struct omp_for* ret = omp_for_tree_find(oft, addr);

	if(!ret)
		return omp_for_tree_add(oft, addr);

	return ret;
}

void omp_for_tree_walk(const void* p, const VISIT which, const int depth);
int omp_for_tree_to_array(struct omp_for_tree* oft, struct omp_for** arr);

static inline void omp_for_tree_destroy(struct omp_for_tree* oft)
{
	tdestroy(oft->root, free);
}

#endif
