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
 * GNU General Public License for more deta *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "frame.h"
#include <string.h>

int compare_framesp(const void *pf1, const void *pf2)
{
	const struct frame* f1 = *((struct frame**)pf1);
	const struct frame* f2 = *((struct frame**)pf2);

	if(f1->addr < f2->addr)
		return -1;
	else if(f1->addr > f2->addr)
		return 1;

	return 0;
}

int compare_frames(const void *pt1, const void *pt2)
{
	const struct frame* f1 = pt1;
	const struct frame* f2 = pt2;

	if(f1->addr < f2->addr)
		return -1;
	else if(f1->addr > f2->addr)
		return 1;

	return 0;
}

/* Ugly thread local variables needed for reentrance of twalk */
__thread struct frame* curr_ft_array;
__thread int curr_ft_array_index;

void frame_tree_walk(const void* p, const VISIT which, const int depth)
{
	if(which == leaf || which == postorder) {
		struct frame* t = *((struct frame**)p);
		memcpy(&curr_ft_array[curr_ft_array_index++], t, sizeof(struct frame));
	}
}

int frame_tree_to_array(struct frame_tree* ft, struct frame** arr)
{
	if(!(*arr = malloc(ft->num_frames*sizeof(struct frame))))
		return 1;

	curr_ft_array = *arr;
	curr_ft_array_index = 0;

	twalk(ft->root, frame_tree_walk);
	return 0;
}
