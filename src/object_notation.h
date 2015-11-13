/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
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

#ifndef OBJECT_NOTATION_H
#define OBJECT_NOTATION_H

#include "parser.h"
#include "ansi_extras.h"
#include "contrib/linux-kernel/list.h"

/* The object notation used in aftermath is a very simple notation
 * similar to JSON, but less complex. It consists of:
 *
 * - string literals (e.g., "ABCD")
 * - integer literals (e.g., 1234) with a maximum of 64 bits
 * - lists in brackets with comma-separated elements (e.g., [1, "ABC", 2]
 * - groups consisting of a group name and a list of members in braces (e.g.,
 *   foo {message: "foo", value: 1234 })
 */

enum object_notation_node_type {
	OBJECT_NOTATION_NODE_TYPE_GROUP,
	OBJECT_NOTATION_NODE_TYPE_MEMBER,
	OBJECT_NOTATION_NODE_TYPE_LIST,
	OBJECT_NOTATION_NODE_TYPE_STRING,
	OBJECT_NOTATION_NODE_TYPE_INT
};

struct object_notation_node {
	enum object_notation_node_type type;
	struct list_head siblings;
};

struct object_notation_node_group {
	struct object_notation_node node;
	char* name;
	struct list_head members;
};

struct object_notation_node_member {
	struct object_notation_node node;
	char* name;
	struct object_notation_node* def;
};

struct object_notation_node_list {
	struct object_notation_node node;
	struct list_head items;
};

struct object_notation_node_string {
	struct object_notation_node node;
	char* value;
};

struct object_notation_node_int {
	struct object_notation_node node;
	uint64_t value;
};

#define object_notation_for_each_list_item(list_node, iter)		\
	for(iter = list_entry((list_node)->items.next,			\
			      struct object_notation_node,		\
			      siblings);				\
	    iter != list_entry(&(list_node)->items,			\
			       struct object_notation_node,		\
			       siblings);				\
	    iter = list_entry(iter->siblings.next,			\
			      struct object_notation_node,		\
			      siblings))

#define object_notation_for_each_group_member(group_node, iter)	\
	for(iter = (struct object_notation_node_member*)		\
		    list_entry((group_node)->members.next,		\
			      struct object_notation_node,		\
			      siblings);				\
	    iter != (struct object_notation_node_member*)		\
		    list_entry(&(group_node)->members,			\
			       struct object_notation_node,		\
			       siblings);				\
	    iter = (struct object_notation_node_member*)		\
		    list_entry(iter->node.siblings.next,		\
			      struct object_notation_node,		\
			      siblings))

void object_notation_node_destroy(struct object_notation_node* node);
struct object_notation_node* object_notation_parse(const char* str, size_t len);

/* Returns the first item of a list node. */
static inline struct object_notation_node* object_notation_node_list_first_member(struct object_notation_node_list* node)
{
	if(list_empty(&node->items))
		return NULL;

	return list_entry(node->items.next, struct object_notation_node, siblings);
}

/* Returns the first member of a group. The node returned by the
 * function is the member node, not the node corresponding to the
 * definition of the member. */
static inline struct object_notation_node* object_notation_node_group_first_member(struct object_notation_node_group* node)
{
	if(list_empty(&node->members))
		return NULL;

	return list_entry(node->members.next, struct object_notation_node, siblings);
}

/* Finds the member node for a given name of a group node and returns
 * its definition. */
static inline struct object_notation_node* object_notation_node_group_get_member_def(struct object_notation_node_group* node, const char* name)
{
	struct object_notation_node_member* iter;

	object_notation_for_each_group_member(node, iter) {
		if(strcmp(iter->name, name) == 0)
			return iter->def;
	}

	return NULL;
}

/* Returns the number of list items of a list node. */
static inline size_t object_notation_node_list_num_items(struct object_notation_node_list* node)
{
	struct object_notation_node* iter;
	size_t ret = 0;

	object_notation_for_each_list_item(node, iter)
		ret++;

	return ret;
}

/* Returns the number of members of a group node. */
static inline size_t object_notation_node_group_num_members(struct object_notation_node_group* node)
{
	struct object_notation_node_member* iter;
	size_t ret = 0;

	object_notation_for_each_group_member(node, iter)
		ret++;

	return ret;
}

int object_notation_node_group_has_members(struct object_notation_node_group* node, int exact, ...);

#endif
