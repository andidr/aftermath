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
 * GNU General Public License for more deta *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "omp_for_chunk_set.h"
#include <string.h>

int compare_omp_for_chunk_setsp(const void *pcs1, const void *pcs2)
{
	const struct omp_for_chunk_set* cs1 = *((struct omp_for_chunk_set**)pcs1);
	const struct omp_for_chunk_set* cs2 = *((struct omp_for_chunk_set**)pcs2);

	if(cs1->ti_ofc->id < cs2->ti_ofc->id)
		return -1;
	else if(cs1->ti_ofc->id > cs2->ti_ofc->id)
		return 1;

	return 0;
}

int compare_omp_for_chunk_sets(const void *pcs1, const void *pcs2)
{
	const struct omp_for_chunk_set* cs1 = pcs1;
	const struct omp_for_chunk_set* cs2 = pcs2;

	if(cs1->ti_ofc->id < cs2->ti_ofc->id)
		return -1;
	else if(cs1->ti_ofc->id > cs2->ti_ofc->id)
		return 1;

	return 0;
}

/* Ugly thread local variables needed for reentrance of twalk */
__thread struct omp_for_chunk_set* curr_ofcst_array;
__thread int curr_ofcst_array_index;

void omp_for_chunk_set_tree_walk(const void* p, const VISIT which, const int depth)
{
	if(which == leaf || which == postorder) {
		struct omp_for_chunk_set* t = *((struct omp_for_chunk_set**)p);
		memcpy(&curr_ofcst_array[curr_ofcst_array_index++], t, sizeof(struct omp_for_chunk_set));
	}
}

int omp_for_chunk_set_tree_to_array(struct omp_for_chunk_set_tree* ofcst, struct omp_for_chunk_set** arr)
{
	if(!(*arr = malloc(ofcst->num_omp_for_chunk_sets*sizeof(struct omp_for_chunk_set))))
		return 1;

	curr_ofcst_array = *arr;
	curr_ofcst_array_index = 0;

	twalk(ofcst->root, omp_for_chunk_set_tree_walk);
	return 0;
}
