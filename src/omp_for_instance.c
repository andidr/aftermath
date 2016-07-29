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

#include "omp_for_instance.h"
#include "aux.h"
#include <string.h>

int compare_omp_for_instancesp(const void *pfi1, const void *pfi2)
{
	const struct omp_for_instance* ofi1 = *((struct omp_for_instance**)pfi1);
	const struct omp_for_instance* ofi2 = *((struct omp_for_instance**)pfi2);

	if(ofi1->ti_ofi->id < ofi2->ti_ofi->id)
		return -1;
	else if(ofi1->ti_ofi->id > ofi2->ti_ofi->id)
		return 1;

	return 0;
}

int compare_omp_for_instances(const void *pfi1, const void *pfi2)
{
	const struct omp_for_instance* ofi1 = pfi1;
	const struct omp_for_instance* ofi2 = pfi2;

	if(ofi1->ti_ofi->id < ofi2->ti_ofi->id)
		return -1;
	else if(ofi1->ti_ofi->id > ofi2->ti_ofi->id)
		return 1;

	return 0;
}

/* Ugly thread local variables needed for reentrance of twalk */
__thread struct omp_for_instance* curr_ofit_array;
__thread int curr_ofit_array_index;

void omp_for_instance_tree_walk(const void* p, const VISIT which, const int depth)
{
	if(which == leaf || which == postorder) {
		struct omp_for_instance* t = *((struct omp_for_instance**)p);
		memcpy(&curr_ofit_array[curr_ofit_array_index++], t, sizeof(struct omp_for_instance));
	}
}

int omp_for_instance_tree_to_array(struct omp_for_instance_tree* ofit, struct omp_for_instance** arr)
{
	if(!(*arr = malloc(ofit->num_omp_for_instances*sizeof(struct omp_for_instance))))
		return 1;

	curr_ofit_array = *arr;
	curr_ofit_array_index = 0;

	twalk(ofit->root, omp_for_instance_tree_walk);
	return 0;
}

/* Returns the chunk set with the smallest start iteration. */
struct omp_for_chunk_set*
omp_for_instance_find_lowest_chunk_set(const struct omp_for_instance* ofi)
{
	struct omp_for_chunk_set* iter;
	struct omp_for_chunk_set* lowest = NULL;

	omp_for_instance_for_each_chunk_set(ofi, iter) {
		if(ofi->flags & OMP_FOR_SIGNED_ITERATION_SPACE) {
			if(!lowest ||
			   (uint64_t)iter->iter_start <
			   (uint64_t)lowest->iter_start)
			{
				lowest = iter;
			}
		} else {
			if(!lowest ||
			   (int64_t)iter->iter_start <
			   (int64_t)lowest->iter_start)
			{
				lowest = iter;
			}
		}
	}

	return lowest;
}

/* Returns the chunk size for a loop instance with a static schedule. If the
 * loop only has a single chunk set, the reported chunk size might be lower than
 * the actual chunk size specified for the loop construct.
 */
uint64_t omp_for_instance_static_chunk_size(const struct omp_for_instance* ofi)
{
	struct omp_for_chunk_set* lowest =
		omp_for_instance_find_lowest_chunk_set(ofi);

	return lowest->iter_end - lowest->iter_start + 1;
}

/* Returns the number of chunks of a for instance. */
uint64_t omp_for_instance_num_chunks(const struct omp_for_instance* ofi)
{
	uint64_t chunk_size;
	uint64_t num_iterations;
	uint64_t num_chunks;

	if(ofi->flags & OMP_FOR_SCHEDULE_STATIC) {
		chunk_size = omp_for_instance_static_chunk_size(ofi);
		num_iterations = ofi->iter_end - ofi->iter_start + 1;
		num_chunks = DIV_ROUND_UP(num_iterations, chunk_size);
	} else {
		num_chunks = ofi->num_chunk_sets;
	}

	return num_chunks;
}
