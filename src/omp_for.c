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

#include "omp_for.h"
#include <string.h>

int compare_omp_forsp(const void *pof1, const void *pof2)
{
	const struct omp_for* of1 = *((struct omp_for**)pof1);
	const struct omp_for* of2 = *((struct omp_for**)pof2);

	if(of1->addr < of2->addr)
		return -1;
	else if(of1->addr > of2->addr)
		return 1;

	return 0;
}

int compare_omp_fors(const void *pt1, const void *pt2)
{
	const struct omp_for* of1 = pt1;
	const struct omp_for* of2 = pt2;

	if(of1->addr < of2->addr)
		return -1;
	else if(of1->addr > of2->addr)
		return 1;

	return 0;
}

/* Ugly thread local variables needed for reentrance of twalk */
__thread struct omp_for* curr_oft_array;
__thread int curr_oft_array_index;

void omp_for_tree_walk(const void* p, const VISIT which, const int depth)
{
	if(which == leaf || which == postorder) {
		struct omp_for* t = *((struct omp_for**)p);
		t->color_r = omp_for_colors[curr_oft_array_index % NUM_OMP_FOR_COLORS][0];
		t->color_g = omp_for_colors[curr_oft_array_index % NUM_OMP_FOR_COLORS][1];
		t->color_b = omp_for_colors[curr_oft_array_index % NUM_OMP_FOR_COLORS][2];
		memcpy(&curr_oft_array[curr_oft_array_index++], t, sizeof(struct omp_for));
	}
}

int omp_for_tree_to_array(struct omp_for_tree* oft, struct omp_for** arr)
{
	if(!(*arr = malloc(oft->num_omp_fors*sizeof(struct omp_for))))
		return 1;

	curr_oft_array = *arr;
	curr_oft_array_index = 0;

	twalk(oft->root, omp_for_tree_walk);
	return 0;
}
