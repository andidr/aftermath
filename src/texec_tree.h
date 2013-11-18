
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

#ifndef TEXEC_TREE_H
#define TEXEC_TREE_H

#define _GNU_SOURCE
#include <stdint.h>
#include <malloc.h>
#include <search.h>
#include <sys/types.h>
#include "frame.h"
#include "events.h"

struct texec_key {
	struct frame* frame;
	struct single_event* texec_start;
};

struct texec_tree {
	void* root;
	int num_items;
};

static inline void texec_tree_init(struct texec_tree* ft)
{
	ft->root = NULL;
	ft->num_items = 0;
}

int compare_texecs(const void *pt1, const void *pt2);
int compare_texecsp(const void *pt1, const void *pt2);

static inline struct texec_key* texec_tree_find(struct texec_tree* tt, struct frame* f, struct single_event* texec_start)
{
	struct texec_key key = { .frame = f, .texec_start = texec_start };
	struct texec_key** pret = tfind(&key, &tt->root, compare_texecs);

	if(pret)
		return *pret;

	return NULL;
}

static inline struct texec_key* texec_tree_add(struct texec_tree* tt, struct frame* f, struct single_event* texec_start)
{
	struct texec_key* key = malloc(sizeof(struct texec_key));

	if(!key)
		return NULL;

	key->frame = f;
	key->texec_start = texec_start;

	if((f = tsearch(key, &tt->root, compare_texecs)) == NULL) {
		free(key);
		return NULL;
	}

	tt->num_items++;

	return key;
}

static inline struct texec_key* texec_tree_find_or_add(struct texec_tree* tt, struct frame* f, struct single_event* texec_start)
{
	struct texec_key* ret = texec_tree_find(tt, f, texec_start);

	if(!ret)
		return texec_tree_add(tt, f, texec_start);

	return ret;
}

void texec_tree_walk_ascending(struct texec_tree* tt, void (*f)(struct texec_key*, void*), void* arg);

static inline void texec_tree_destroy(struct texec_tree* ft)
{
	tdestroy(ft->root, free);
}

#endif
