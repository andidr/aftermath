/**
 * Author: Andi Drebes <andi@drebesium.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_HIERARCHY_H
#define AM_HIERARCHY_H

#include "typed_list.h"
#include "base_types.h"
#include "event_mapping.h"
#include <string.h>
#include <stdlib.h>

/* Hierarchies in Aftermath represent any topological information about hardware
 * or software that can be represented as a tree. Each node of the tree can be
 * associated to the events of logical execution streams through an event
 * mapping. A mapping consists of a set of entries composed of an interval and a
 * pointer to an event collection, associating the events of the event
 * collection with the hierarchy node for the duration of the interval.
 */

struct am_hierarchy {
	char* name;
	am_hierarchy_id_t id;
	struct am_hierarchy_node* root;
};

void am_hierarchy_init_nodup(struct am_hierarchy* h, char* name, am_hierarchy_id_t id);
int am_hierarchy_init(struct am_hierarchy* h, const char* name, am_hierarchy_id_t id);
void am_hierarchy_destroy(struct am_hierarchy* h);
void am_hierarchy_dump(struct am_hierarchy* h);

struct am_hierarchy_node {
	char* name;
	am_hierarchy_node_id_t id;
	size_t num_descendants;
	struct list_head children;
	struct am_hierarchy_node* parent;
	struct list_head siblings;
	struct am_event_mapping event_mapping;
};

#define am_hierarchy_node_for_each_child(hnode, child) \
	am_typed_list_for_each(hnode, children, child, siblings)

#define am_hierarchy_node_for_each_child_start(hnode, child, start) \
	am_typed_list_for_each_start(hnode, children, child, siblings, start)

#define am_hierarchy_node_for_each_child_prev(hnode, child) \
	am_typed_list_for_each_prev(hnode, children, child, siblings)

#define am_hierarchy_node_for_each_child_prev_start(hnode, child, start) \
	am_typed_list_for_each_prev_start(hnode, children, child, siblings, start)

#define am_hierarchy_node_for_each_child_safe(hnode, n, i) \
	am_typed_list_for_each_safe(hnode, children, n, i, siblings)

#define am_hierarchy_node_for_each_child_prev_safe(hnode, n, i) \
	am_typed_list_for_each_prev_safe(hnode, children, child, i, siblings)

#define am_hierarchy_node_for_each_ancestor(hnode, ancestor) \
	for(ancestor = hnode->parent; ancestor; ancestor = ancestor->parent)

/*
 * Add a node to a parent node
 */
static inline void am_hierarchy_node_add_child(struct am_hierarchy_node* parent,
					       struct am_hierarchy_node* child)
{
	struct am_hierarchy_node* ancestor;

	list_add_tail(&child->siblings, &parent->children);
	child->parent = parent;

	am_hierarchy_node_for_each_ancestor(child, ancestor)
		ancestor->num_descendants += child->num_descendants + 1;
}

/*
 * Detach a node from its parent
 */
static inline void am_hierarchy_node_unparent(struct am_hierarchy_node* child)
{
	struct am_hierarchy_node* ancestor;

	am_hierarchy_node_for_each_ancestor(child, ancestor)
		ancestor->num_descendants -= child->num_descendants + 1;

	list_del(&child->siblings);
	child->parent = NULL;
}

/*
 * Check if a node has at least one child.
 */
static inline int
am_hierarchy_node_has_children(const struct am_hierarchy_node* hn)
{
	return !list_empty(&hn->children);
}

int am_hierarchy_node_init(struct am_hierarchy_node* hn,
			   am_hierarchy_node_id_t id,
			   const char* name);
void am_hierarchy_node_init_nodup(struct am_hierarchy_node* hn,
				  am_hierarchy_node_id_t id,
				  char* name);
void am_hierarchy_node_destroy(struct am_hierarchy_node* hn);
void am_hierarchy_node_set_name_nodup(struct am_hierarchy_node* hn, char* name);
int am_hierarchy_node_set_name(struct am_hierarchy_node* hn, const char* name);
unsigned int am_hierarchy_node_depth(struct am_hierarchy_node* n);

#endif
