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

#include "texec_tree.h"

int compare_texecsp(const void *pf1, const void *pf2)
{
	const struct texec_key* f1 = *((struct texec_key**)pf1);
	const struct texec_key* f2 = *((struct texec_key**)pf2);

	if(f1->frame->addr < f2->frame->addr ||
	   (f1->frame->addr == f2->frame->addr &&
	    f1->texec_start->time < f2->texec_start->time))
		return -1;

	if(f1->frame->addr > f2->frame->addr ||
	   (f1->frame->addr == f2->frame->addr &&
	    f1->texec_start->time > f2->texec_start->time))
		return 1;

	return 0;
}

int compare_texecs(const void *pt1, const void *pt2)
{
	const struct texec_key* f1 = pt1;
	const struct texec_key* f2 = pt2;

	if(f1->frame->addr < f2->frame->addr ||
	   (f1->frame->addr == f2->frame->addr &&
	    f1->texec_start->time < f2->texec_start->time))
		return -1;

	if(f1->frame->addr > f2->frame->addr ||
	   (f1->frame->addr == f2->frame->addr &&
	    f1->texec_start->time > f2->texec_start->time))
		return 1;

	return 0;
}

__thread void (*walk_fun)(struct texec_key*, void*);
__thread void* walk_arg;

void texec_tree_walk(const void* p, const VISIT which, const int depth)
{
	if(which == leaf || which == postorder) {
		struct texec_key* t = *((struct texec_key**)p);
		walk_fun(t, walk_arg);
	}
}

void texec_tree_walk_ascending(struct texec_tree* tt, void (*f)(struct texec_key*, void*), void* arg)
{
	walk_fun = f;
	walk_arg = arg;
	twalk(tt->root, texec_tree_walk);
}
