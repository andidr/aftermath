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

#include "task.h"
#include <string.h>

int compare_tasksp(const void *pt1, const void *pt2)
{
	const struct task* t1 = *((struct task**)pt1);
	const struct task* t2 = *((struct task**)pt2);

	if(t1->addr < t2->addr)
		return -1;
	else if(t1->addr > t2->addr)
		return 1;

	return 0;
}

int compare_tasks(const void *pt1, const void *pt2)
{
	const struct task* t1 = pt1;
	const struct task* t2 = pt2;

	if(t1->addr < t2->addr)
		return -1;
	else if(t1->addr > t2->addr)
		return 1;

	return 0;
}

/* Ugly thread local variables needed for reentrance of twalk */
__thread struct task* curr_tt_array;
__thread int curr_tt_array_index;

void task_tree_walk(const void* p, const VISIT which, const int depth)
{
	if(which == leaf || which == postorder) {
		struct task* t = *((struct task**)p);
		memcpy(&curr_tt_array[curr_tt_array_index++], t, sizeof(struct task));
	}
}

int task_tree_to_array(struct task_tree* tt, struct task** arr)
{
	if(!(*arr = malloc(tt->num_tasks*sizeof(struct task))))
		return 1;

	curr_tt_array = *arr;
	curr_tt_array_index = 0;

	twalk(tt->root, task_tree_walk);
	return 0;
}
