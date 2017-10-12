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

#ifndef AM_OBJECT_NOTATION_H
#define AM_OBJECT_NOTATION_H

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

enum am_object_notation_node_type {
	AM_OBJECT_NOTATION_NODE_TYPE_GROUP,
	AM_OBJECT_NOTATION_NODE_TYPE_MEMBER,
	AM_OBJECT_NOTATION_NODE_TYPE_LIST,
	AM_OBJECT_NOTATION_NODE_TYPE_STRING,
	AM_OBJECT_NOTATION_NODE_TYPE_INT
};

struct am_object_notation_node {
	enum am_object_notation_node_type type;
	struct list_head siblings;
};

static inline void
am_object_notation_node_init(struct am_object_notation_node* n,
			     enum am_object_notation_node_type type)
{
	n->type = type;
	INIT_LIST_HEAD(&n->siblings);
}

struct am_object_notation_node_group {
	struct am_object_notation_node node;
	char* name;
	struct list_head members;
};

/* Initialize an already allocated group node using a non-zero-terminated string
 * for the name. Returns 0 on success, otherwise 1. */
static inline int
am_object_notation_node_group_initn(struct am_object_notation_node_group* g,
				    const char* name, size_t name_len)
{
	am_object_notation_node_init(&g->node,
				     AM_OBJECT_NOTATION_NODE_TYPE_GROUP);
	INIT_LIST_HEAD(&g->members);

	if(!(g->name = am_strdupn(name, name_len)))
		return 1;

	return 0;
}

/* Initialize an already allocated group node. Returns 0 on success, otherwise
 * 1. */
static inline int
am_object_notation_node_group_init(struct am_object_notation_node_group* g,
				   const char* name)
{
	return am_object_notation_node_group_initn(g, name, strlen(name));
}

/* Allocate and initialize a group node using a non-zero-terminated string for
 * the name. Returns a reference to the newly allocated node on success,
 * otherwise NULL.
 */
static inline struct am_object_notation_node_group*
am_object_notation_node_group_createn(const char* name, size_t name_len)
{
	struct am_object_notation_node_group* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	if(am_object_notation_node_group_initn(ret, name, name_len)) {
		free(ret);
		return NULL;
	}

	return ret;
}

/* Allocate and initialize a group node. Returns a reference to the newly
 * allocated node on success, otherwise NULL.
 */
static inline struct am_object_notation_node_group*
am_object_notation_node_group_create(const char* name)
{
	return am_object_notation_node_group_createn(name, strlen(name));
}

struct am_object_notation_node_member {
	struct am_object_notation_node node;
	char* name;
	struct am_object_notation_node* def;
};

/* Initialize an already allocated member node using a non-zero-terminated string
 * for the name. Returns 0 on success, otherwise 1. */
static inline int
am_object_notation_node_member_initn(struct am_object_notation_node_member* m,
				  const char* name, size_t name_len,
				  struct am_object_notation_node* def)
{
	am_object_notation_node_init(&m->node,
				     AM_OBJECT_NOTATION_NODE_TYPE_MEMBER);
	m->def = def;

	if(!(m->name = am_strdupn(name, name_len)))
		return 1;

	return 0;
}

/* Initialize an already allocated member node. Returns 0 on success, otherwise
 * 1. */
static inline int
am_object_notation_node_member_init(struct am_object_notation_node_member* m,
				    const char* name,
				    struct am_object_notation_node* def)
{
	return am_object_notation_node_member_initn(m, name, strlen(name), def);
}

/* Allocate and initialize a member node using a non-zero-terminated string for
 * the name. Returns a reference to the newly allocated node on success,
 * otherwise NULL.
 */
static inline struct am_object_notation_node_member*
am_object_notation_node_member_createn(const char* name, size_t name_len,
				       struct am_object_notation_node* def)
{
	struct am_object_notation_node_member* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	if(am_object_notation_node_member_initn(ret, name, name_len, def)) {
		free(ret);
		return NULL;
	}

	return ret;
}

/* Allocate and initialize a member node. Returns a reference to the newly
 * allocated node on success, otherwise NULL.
 */
static inline struct am_object_notation_node_member*
am_object_notation_node_member_create(const char* name,
				      struct am_object_notation_node* def)
{
	return am_object_notation_node_member_createn(name, strlen(name), def);
}

struct am_object_notation_node_list {
	struct am_object_notation_node node;
	struct list_head items;
};

/* Initialize an already allocated list node. Returns 0 on success, otherwise
 * 1. */
static inline void
am_object_notation_node_list_init(struct am_object_notation_node_list* l)
{
	am_object_notation_node_init(&l->node,
				     AM_OBJECT_NOTATION_NODE_TYPE_LIST);
	INIT_LIST_HEAD(&l->items);
}

/* Allocate and initialize a list node. Returns a reference to the newly
 * allocated node on success, otherwise NULL.
 */
static inline struct am_object_notation_node_list*
am_object_notation_node_list_create(void)
{
	struct am_object_notation_node_list* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	am_object_notation_node_list_init(ret);

	return ret;
}

struct am_object_notation_node_string {
	struct am_object_notation_node node;
	char* value;
};

/* Initialize an already allocated string node using a non-zero-terminated
 * string for the value. If unescape is non-zero, the value is unescaped after
 * duplication. Returns 0 on success, otherwise 1. */
static inline int
am_object_notation_node_string_initn(struct am_object_notation_node_string* g,
				     const char* value, size_t value_len,
				     int unescape)
{
	am_object_notation_node_init(&g->node,
				     AM_OBJECT_NOTATION_NODE_TYPE_STRING);

	if(unescape) {
		if(!(g->value = am_unescape_stringn(value, value_len)))
			return 1;
	} else {
		if(!(g->value = am_strdupn(value, value_len)))
			return 1;
	}

	return 0;
}

/* Initialize an already allocated string node. If unescape is non-zero, the
 * value is unescaped after duplication. Returns 0 on success, otherwise 1. */
static inline int
am_object_notation_node_string_init(struct am_object_notation_node_string* g,
				    const char* value,
				    int unescape)
{
	return am_object_notation_node_string_initn(g, value, strlen(value),
						 unescape);
}

/* Allocate and initialize a string node using a non-zero-terminated string for
 * the value. If unescape is non-zero, the value is unescaped after
 * duplication. Returns a reference to the newly allocated node on success,
 * otherwise NULL.
 */
static inline struct am_object_notation_node_string*
am_object_notation_node_string_createn(const char* value,
				       size_t value_len,
				       int unescape)
{
	struct am_object_notation_node_string* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	if(am_object_notation_node_string_initn(ret, value, value_len,
						unescape))
	{
		free(ret);
		return NULL;
	}

	return ret;
}

/* Allocate and initialize a string node. If unescape is non-zero, the value is
 * unescaped after duplication. Returns a reference to the newly allocated node
 * on success, otherwise NULL.
 */
static inline struct am_object_notation_node_string*
am_object_notation_node_string_create(const char* value, int unescape)
{
	return am_object_notation_node_string_createn(value,
						      strlen(value),
						      unescape);
}

struct am_object_notation_node_int {
	struct am_object_notation_node node;
	uint64_t value;
};

/* Initialize an already allocated int node. Returns 0 on success, otherwise
 * 1. */
static inline void
am_object_notation_node_int_init(struct am_object_notation_node_int* i,
				 uint64_t value)
{
	am_object_notation_node_init(&i->node, AM_OBJECT_NOTATION_NODE_TYPE_INT);
	i->value = value;
}

/* Allocate and initialize a int node. Returns a reference to the newly
 * allocated node on success, otherwise NULL.
 */
static inline struct am_object_notation_node_int*
am_object_notation_node_int_create(uint64_t value)
{
	struct am_object_notation_node_int* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	am_object_notation_node_int_init(ret, value);

	return ret;
}

#define am_object_notation_for_each_list_item(list_node, iter)		\
	for(iter = list_entry((list_node)->items.next,			\
			      struct am_object_notation_node,		\
			      siblings);				\
	    iter != list_entry(&(list_node)->items,			\
			       struct am_object_notation_node,		\
			       siblings);				\
	    iter = list_entry(iter->siblings.next,			\
			      struct am_object_notation_node,		\
			      siblings))

#define am_object_notation_for_each_group_member(group_node, iter)	\
	for(iter = (struct am_object_notation_node_member*)		\
		    list_entry((group_node)->members.next,		\
			      struct am_object_notation_node,		\
			      siblings);				\
	    iter != (struct am_object_notation_node_member*)		\
		    list_entry(&(group_node)->members,			\
			       struct am_object_notation_node,		\
			       siblings);				\
	    iter = (struct am_object_notation_node_member*)		\
		    list_entry(iter->node.siblings.next,		\
			      struct am_object_notation_node,		\
			      siblings))

void am_object_notation_node_destroy(struct am_object_notation_node* node);
struct am_object_notation_node*
am_object_notation_parse(const char* str, size_t len);

/* Checks if a list does not have any items */
static inline int
am_object_notation_node_list_is_empty(
	const struct am_object_notation_node_list* node)
{
	return list_empty(&node->items);
}

/* Returns the first item of a list node. */
static inline struct am_object_notation_node*
am_object_notation_node_list_first_item(
	const struct am_object_notation_node_list* node)
{
	if(am_object_notation_node_list_is_empty(node))
		return NULL;

	return list_entry(node->items.next,
			  struct am_object_notation_node,
			  siblings);
}

/* Returns the last item of a list node. */
static inline struct am_object_notation_node*
am_object_notation_node_list_last_item(
	const struct am_object_notation_node_list* node)
{
	if(am_object_notation_node_list_is_empty(node))
		return NULL;

	return list_entry(node->items.prev,
			  struct am_object_notation_node,
			  siblings);
}

/* Checks if an item is the first item in a list */
static inline int
am_object_notation_node_list_is_first_item(
	const struct am_object_notation_node_list* node,
	const struct am_object_notation_node* item)
{
	return (item && item == am_object_notation_node_list_first_item(node));
}

/* Checks if an item is the last item in a list */
static inline int
am_object_notation_node_list_is_last_item(
	const struct am_object_notation_node_list* node,
	const struct am_object_notation_node* item)
{
	return (item && item == am_object_notation_node_list_last_item(node));
}

/* Returns the first member of a group. The node returned by the
 * function is the member node, not the node corresponding to the
 * definition of the member. */
static inline struct am_object_notation_node*
am_object_notation_node_group_first_member(
	const struct am_object_notation_node_group* node)
{
	if(list_empty(&node->members))
		return NULL;

	return list_entry(node->members.next,
			  struct am_object_notation_node,
			  siblings);
}

/* Returns the last member of a group. The node returned by the
 * function is the member node, not the node corresponding to the
 * definition of the member. */
static inline struct am_object_notation_node*
am_object_notation_node_group_last_member(const struct am_object_notation_node_group* node)
{
	if(list_empty(&node->members))
		return NULL;

	return list_entry(node->members.prev,
			  struct am_object_notation_node,
			  siblings);
}

/* Checks if a member is the first member in a group */
static inline int
am_object_notation_node_group_is_first_member(
	const struct am_object_notation_node_group* node,
	const struct am_object_notation_node_member* member)
{
	return (member && (struct am_object_notation_node*)member ==
		am_object_notation_node_group_first_member(node));
}

/* Checks if a member is the last member in a group */
static inline int
am_object_notation_node_group_is_last_member(
	const struct am_object_notation_node_group* node,
	const struct am_object_notation_node_member* member)
{
	return (member && (struct am_object_notation_node*)member ==
		am_object_notation_node_group_last_member(node));
}

/* Finds the member node for a given name of a group node and returns
 * its definition. */
static inline struct am_object_notation_node*
am_object_notation_node_group_get_member_def(
	const struct am_object_notation_node_group* node,
	const char* name)
{
	struct am_object_notation_node_member* iter;

	am_object_notation_for_each_group_member(node, iter) {
		if(strcmp(iter->name, name) == 0)
			return iter->def;
	}

	return NULL;
}

/* Returns the number of list items of a list node. */
static inline size_t
am_object_notation_node_list_num_items(struct am_object_notation_node_list* node)
{
	struct am_object_notation_node* iter;
	size_t ret = 0;

	am_object_notation_for_each_list_item(node, iter)
		ret++;

	return ret;
}

/* Add a member node m to a group g */
static inline void
am_object_notation_node_group_add_member(
	struct am_object_notation_node_group* g,
	struct am_object_notation_node_member* m)
{
	list_add_tail(&m->node.siblings, &g->members);
}

/* Add an item n to a list l */
static inline void
am_object_notation_node_list_add_item(struct am_object_notation_node_list* l,
				      struct am_object_notation_node* n)
{
	list_add_tail(&n->siblings, &l->items);
}

/* Returns the number of members of a group node. */
static inline size_t
am_object_notation_node_group_num_members(struct am_object_notation_node_group* node)
{
	struct am_object_notation_node_member* iter;
	size_t ret = 0;

	am_object_notation_for_each_group_member(node, iter)
		ret++;

	return ret;
}

int am_object_notation_node_group_has_members(
	struct am_object_notation_node_group* node,
	int exact,
	...);

int am_object_notation_node_group_save(
	struct am_object_notation_node_group* node,
	FILE* fp, int indent, int next_indent);
int am_object_notation_node_member_save(
	struct am_object_notation_node_member* node,
	FILE* fp, int indent, int next_indent);
int am_object_notation_node_list_save(
	struct am_object_notation_node_list* node,
	FILE* fp, int indent, int next_indent);
int am_object_notation_node_string_save(
	struct am_object_notation_node_string* node,
	FILE* fp, int indent);
int am_object_notation_node_int_save(struct am_object_notation_node_int* node,
				     FILE* fp, int indent);
int am_object_notation_save_fp_indent(struct am_object_notation_node* node,
				      FILE* fp, int indent, int next_indent);
int am_object_notation_save(
	struct am_object_notation_node* node,
	const char* filename);
int am_object_notation_save_fp(struct am_object_notation_node* node, FILE* fp);

struct am_object_notation_node* am_object_notation_load(const char* filename);

#endif
