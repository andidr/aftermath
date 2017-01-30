/**
 * Copyright (C) 2017 Andi Drebes <andi.drebes@lip6.fr>
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

#ifndef HIERARCHY_H
#define HIERARCHY_H

#include "typed_list.h"
#include "trace/types.h"
#include <string.h>
#include <stdlib.h>

struct hierarchy {
	char* name;
	am_hierarchy_id_t id;
	struct hierarchy_node* root;
};

void hierarchy_init_nodup(struct hierarchy* h, char* name, am_hierarchy_id_t id);
int hierarchy_init(struct hierarchy* h, const char* name, am_hierarchy_id_t id);
void hierarchy_destroy(struct hierarchy* h);
void hierarchy_dump(struct hierarchy* h);

struct hierarchy_node {
	char* name;
	am_hierarchy_node_id_t id;
	size_t num_descendants;
	struct list_head children;
	struct hierarchy_node* parent;
	struct list_head siblings;
};

#define hierarchy_node_for_each_child(hnode, child) \
	typed_list_for_each(hnode, children, child, siblings)

#define hierarchy_node_for_each_child_prev(hnode, child) \
	typed_list_for_each_prev(hnode, children, child, siblings)

#define hierarchy_node_for_each_child_safe(hnode, n, i) \
	typed_list_for_each_safe(hnode, children, n, i, siblings)

#define hierarchy_node_for_each_child_prev_safe(hnode, n, i) \
	typed_list_for_each_prev_safe(hnode, children, child, i, siblings)

#define hierarchy_node_for_each_ancestor(hnode, ancestor) \
	for(ancestor = hnode->parent; ancestor; ancestor = ancestor->parent)

/*
 * Add a node to a parent node
 */
static inline void hierarchy_node_add_child(struct hierarchy_node* parent,
					    struct hierarchy_node* child)
{
	struct hierarchy_node* ancestor;

	list_add(&child->siblings, &parent->children);
	child->parent = parent;

	hierarchy_node_for_each_ancestor(child, ancestor)
		ancestor->num_descendants += child->num_descendants + 1;
}

/*
 * Detach a node from its parent
 */
static inline void hierarchy_node_unparent(struct hierarchy_node* child)
{
	struct hierarchy_node* ancestor;

	hierarchy_node_for_each_ancestor(child, ancestor)
		ancestor->num_descendants -= child->num_descendants + 1;

	list_del(&child->siblings);
	child->parent = NULL;
}

/*
 * Check if a node has at least one child.
 */
static inline int hierarchy_node_has_children(const struct hierarchy_node* hn)
{
	return !list_empty(&hn->children);
}

int hierarchy_node_init(struct hierarchy_node* hn,
			am_hierarchy_node_id_t id,
			const char* name);
void hierarchy_node_init_nodup(struct hierarchy_node* hn,
			       am_hierarchy_node_id_t id,
			       char* name);
void hierarchy_node_destroy(struct hierarchy_node* hn);
void hierarchy_node_set_name_nodup(struct hierarchy_node* hn, char* name);
int hierarchy_node_set_name(struct hierarchy_node* hn, const char* name);

#endif
