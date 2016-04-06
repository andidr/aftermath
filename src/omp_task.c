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

#include "omp_task.h"
#include <string.h>

int compare_omp_tasksp(const void *pt1, const void *pt2)
{
	const struct omp_task* t1 = *((struct omp_task**)pt1);
	const struct omp_task* t2 = *((struct omp_task**)pt2);

	if(t1->addr < t2->addr)
		return -1;
	else if(t1->addr > t2->addr)
		return 1;

	return 0;
}

int compare_omp_tasks(const void *pt1, const void *pt2)
{
	const struct omp_task* t1 = pt1;
	const struct omp_task* t2 = pt2;

	if(t1->addr < t2->addr)
		return -1;
	else if(t1->addr > t2->addr)
		return 1;

	return 0;
}

/* Ugly thread local variables needed for reentrance of twalk */
__thread struct omp_task* curr_ott_array;
__thread int curr_ott_array_index;

void omp_task_tree_walk(const void* p, const VISIT which, const int depth)
{
	if(which == leaf || which == postorder) {
		struct omp_task* t = *((struct omp_task**)p);
		memcpy(&curr_ott_array[curr_ott_array_index++], t, sizeof(struct omp_task));
	}
}

int omp_task_tree_to_array(struct omp_task_tree* ott, struct omp_task** arr)
{
	if(!(*arr = malloc(ott->num_omp_tasks*sizeof(struct omp_task))))
		return 1;

	curr_ott_array = *arr;
	curr_ott_array_index = 0;

	twalk(ott->root, omp_task_tree_walk);
	return 0;
}
