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

#include <aftermath/core/parser.h>
#include <aftermath/core/ansi_extras.h>
#include <aftermath/core/typed_list.h>
#include <aftermath/core/contrib/linux-kernel/list.h>

/* The object notation used in aftermath is a very simple notation
 * similar to JSON, but less complex. It consists of:
 *
 * - string literals (e.g., "ABCD")
 * - 64-bit unsigned integer literals (e.g., 1234u64)
 * - 64-bit signed integer literals (e.g., 1234i64)
 * - double literals (non-scientific notation with a mandatory dot)
 * - lists in brackets with comma-separated elements (e.g., [1, "ABC", 2]
 * - groups consisting of a group name and a list of members in braces (e.g.,
 *   foo {message: "foo", value: 1234 })
 */

enum am_object_notation_node_type {
	AM_OBJECT_NOTATION_NODE_TYPE_GROUP,
	AM_OBJECT_NOTATION_NODE_TYPE_MEMBER,
	AM_OBJECT_NOTATION_NODE_TYPE_LIST,
	AM_OBJECT_NOTATION_NODE_TYPE_STRING,
	AM_OBJECT_NOTATION_NODE_TYPE_INT64,
	AM_OBJECT_NOTATION_NODE_TYPE_UINT64,
	AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE
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

/* Checks if a node is of a composite data type. Returns 1 if this is the case,
 * otherwise 0. */
static inline int
am_object_notation_node_is_composite(struct am_object_notation_node* n)
{
	switch(n->type) {
		case AM_OBJECT_NOTATION_NODE_TYPE_GROUP:
		case AM_OBJECT_NOTATION_NODE_TYPE_MEMBER:
		case AM_OBJECT_NOTATION_NODE_TYPE_LIST:
			return 1;
		case AM_OBJECT_NOTATION_NODE_TYPE_STRING:
		case AM_OBJECT_NOTATION_NODE_TYPE_INT64:
		case AM_OBJECT_NOTATION_NODE_TYPE_UINT64:
		case AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE:
			return 0;
	}

	/* Cannot happen */
	return 0;
}

struct am_object_notation_node_group {
	struct am_object_notation_node node;
	char* name;
	struct list_head members;
};

/* Iterates over all members of a group node g. M must be the name of a pointer
 * that serves as an iterator */
#define am_object_notation_group_for_each_member(g, m) \
	am_typed_list_for_each(g, members, m, node.siblings)

/* Initializes a group n and sets its name to NULL. */
static inline void am_object_notation_node_group_init_no_name(
	struct am_object_notation_node_group* g)
{
	am_object_notation_node_init(&g->node,
				     AM_OBJECT_NOTATION_NODE_TYPE_GROUP);
	INIT_LIST_HEAD(&g->members);
	g->name = NULL;
}

/* Initialize an already allocated group node using a non-zero-terminated string
 * for the name. Returns 0 on success, otherwise 1. */
static inline int
am_object_notation_node_group_initn(struct am_object_notation_node_group* g,
				    const char* name, size_t name_len)
{
	am_object_notation_node_group_init_no_name(g);

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

struct am_object_notation_node_group*
am_object_notation_node_group_createn(const char* name, size_t name_len);

/* Allocate and initialize a group node. Returns a reference to the newly
 * allocated node on success, otherwise NULL.
 */
static inline struct am_object_notation_node_group*
am_object_notation_node_group_create(const char* name)
{
	return am_object_notation_node_group_createn(name, strlen(name));
}

int am_object_notation_node_group_move_members(
	struct am_object_notation_node_group* dst,
	struct am_object_notation_node_group* src);

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

struct am_object_notation_node_member*
am_object_notation_node_member_createn(const char* name, size_t name_len,
				       struct am_object_notation_node* def);

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

struct am_object_notation_node_list* am_object_notation_node_list_create(void);

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

struct am_object_notation_node_string*
am_object_notation_node_string_createn(const char* value,
				       size_t value_len,
				       int unescape);

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

struct am_object_notation_node_int64 {
	struct am_object_notation_node node;
	int64_t value;
};

/* Initialize an already allocated int node. Returns 0 on success, otherwise
 * 1. */
static inline void
am_object_notation_node_int64_init(struct am_object_notation_node_int64* i,
				   int64_t value)
{
	am_object_notation_node_init(&i->node,
				     AM_OBJECT_NOTATION_NODE_TYPE_INT64);
	i->value = value;
}

struct am_object_notation_node_int64*
am_object_notation_node_int_create(int64_t value);

struct am_object_notation_node_uint64 {
	struct am_object_notation_node node;
	uint64_t value;
};

/* Initialize an already allocated int node. Returns 0 on success, otherwise
 * 1. */
static inline void
am_object_notation_node_uint64_init(struct am_object_notation_node_uint64* i,
				    uint64_t value)
{
	am_object_notation_node_init(&i->node,
				     AM_OBJECT_NOTATION_NODE_TYPE_UINT64);
	i->value = value;
}

struct am_object_notation_node_int64*
am_object_notation_node_int64_create(int64_t value);

struct am_object_notation_node_uint64*
am_object_notation_node_uint64_create(uint64_t value);

struct am_object_notation_node_double {
	struct am_object_notation_node node;
	double value;
};

/* Initialize an already allocated double node. Returns 0 on success, otherwise
 * 1. */
static inline void
am_object_notation_node_double_init(struct am_object_notation_node_double* i,
				    double value)
{
	am_object_notation_node_init(&i->node, AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE);
	i->value = value;
}

struct am_object_notation_node_double*
am_object_notation_node_double_create(double value);


#define am_object_notation_for_each_list_item_typed(list_node, iter, type)	\
	for(iter = (type*)list_entry((list_node)->items.next,			\
			      struct am_object_notation_node,			\
			      siblings);					\
	    iter != (type*)list_entry(&(list_node)->items,			\
				      struct am_object_notation_node,		\
				      siblings);				\
	    iter = (type*)list_entry(((struct am_object_notation_node*)iter)->	\
				     siblings.next,				\
				     struct am_object_notation_node,		\
				     siblings))

#define am_object_notation_for_each_list_item(list_node, iter)		\
	am_object_notation_for_each_list_item_typed(			\
		list_node, iter, struct am_object_notation_node)

#define am_object_notation_for_each_list_item_int64(list_node, iter)	\
	am_object_notation_for_each_list_item_typed(			\
		list_node, iter, struct am_object_notation_node_int64)

#define am_object_notation_for_each_list_item_uint64(list_node, iter)	\
	am_object_notation_for_each_list_item_typed(			\
		list_node, iter, struct am_object_notation_node_uint64)

#define am_object_notation_for_each_list_item_double(list_node, iter)	\
	am_object_notation_for_each_list_item_typed(			\
		list_node, iter, struct am_object_notation_node_double)

#define am_object_notation_for_each_list_item_string(list_node, iter)	\
	am_object_notation_for_each_list_item_typed(			\
		list_node, iter, struct am_object_notation_node_string)

#define am_object_notation_for_each_list_item_group(list_node, iter)	\
	am_object_notation_for_each_list_item_typed(			\
		list_node, iter, struct am_object_notation_node_group)

/* Returns true if all of the items of a list l are of the specified type. */
static inline int
am_object_notation_node_list_items_same_type(
	const struct am_object_notation_node_list* l,
	enum am_object_notation_node_type type)
{
	struct am_object_notation_node* iter;

	am_object_notation_for_each_list_item(l, iter)
		if(iter->type != type)
			return 0;

	return 1;
}

/* Defines a function am_object_notation_is_<name>_list(l), which checks if a
 * list l is composed exclusively of nodes of the given type. */
#define AM_OBJECT_NOTATION_DEFINE_TYPED_LIST_CHECK_FUN(name, type)		\
	static inline int							\
	am_object_notation_is_##name##_list(					\
		const struct am_object_notation_node_list* l)			\
	{									\
		return am_object_notation_node_list_items_same_type(l, type);	\
	}

AM_OBJECT_NOTATION_DEFINE_TYPED_LIST_CHECK_FUN(
	group, AM_OBJECT_NOTATION_NODE_TYPE_GROUP);

AM_OBJECT_NOTATION_DEFINE_TYPED_LIST_CHECK_FUN(
	string, AM_OBJECT_NOTATION_NODE_TYPE_STRING);

AM_OBJECT_NOTATION_DEFINE_TYPED_LIST_CHECK_FUN(
	int64, AM_OBJECT_NOTATION_NODE_TYPE_INT64);

AM_OBJECT_NOTATION_DEFINE_TYPED_LIST_CHECK_FUN(
	uint64, AM_OBJECT_NOTATION_NODE_TYPE_UINT64);

AM_OBJECT_NOTATION_DEFINE_TYPED_LIST_CHECK_FUN(
	double, AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE);


/* Checks if at least one item in a list is a composite data type. Returns 1 if
 * this is the case, otherwise 0.*/
static inline int
am_object_notation_node_list_has_composite_item(
	const struct am_object_notation_node_list* l)
{
	struct am_object_notation_node* iter;

	am_object_notation_for_each_list_item(l, iter) {
		if(am_object_notation_node_is_composite(iter))
			return 1;
	}

	return 0;
}

/* Returns the n-th item in a list. If the list has less than n items, NULL is
 * returned.
 */
static inline struct am_object_notation_node*
am_object_notation_node_list_nth_member(
	const struct am_object_notation_node_list* l,
	size_t n)
{
	struct am_object_notation_node* iter;
	size_t i = 0;

	am_object_notation_for_each_list_item(l, iter) {
		if(i == n)
			return iter;

		i++;
	}

	return NULL;
}

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

/* Returns the next sibling of a node contained in a list. */
static inline struct am_object_notation_node*
am_object_notation_node_list_next_item(
	const struct am_object_notation_node_list* list,
	const struct am_object_notation_node* item)
{
	if(am_object_notation_node_list_is_empty(list))
		return NULL;

	if(item->siblings.next == &list->items)
		return NULL;

	return list_entry(item->siblings.next, struct am_object_notation_node,
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

int am_object_notation_node_group_has_at_least_members(
	struct am_object_notation_node_group* node,
	...);

int am_object_notation_node_group_has_at_most_members(
	struct am_object_notation_node_group* node,
	...);

int am_object_notation_node_group_has_exactly_members(
	struct am_object_notation_node_group* node,
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
int am_object_notation_node_int64_save(
	struct am_object_notation_node_int64* node,
	FILE* fp, int indent);
int am_object_notation_node_uint64_save(
	struct am_object_notation_node_uint64* node,
	FILE* fp, int indent);
int am_object_notation_save_fp_indent(struct am_object_notation_node* node,
				      FILE* fp, int indent, int next_indent);
int am_object_notation_save(
	struct am_object_notation_node* node,
	const char* filename);
int am_object_notation_save_fp(struct am_object_notation_node* node, FILE* fp);

struct am_object_notation_node* am_object_notation_load(const char* filename);

enum am_object_notation_build_verb {
	AM_OBJECT_NOTATION_BUILD_GROUP,
	AM_OBJECT_NOTATION_BUILD_LIST,
	AM_OBJECT_NOTATION_BUILD_INT64,
	AM_OBJECT_NOTATION_BUILD_UINT64,
	AM_OBJECT_NOTATION_BUILD_DOUBLE,
	AM_OBJECT_NOTATION_BUILD_STRING,
	AM_OBJECT_NOTATION_BUILD_MEMBER,
	AM_OBJECT_NOTATION_BUILD_END,
};

struct am_object_notation_node* __am_object_notation_build(int dummy, ...);

/* Builds an object notation node based on its parameters. This includes
 * composite nodes (lists and groups) with an arbitrary nesting. The parameter
 * list must have a specific format, which is a mix of the control verbs defined
 * by enum am_object_notation_build_verb and actual values (e.g., integers or
 * strings).
 *
 * Examples:
 *
 * - Integer node with value 123:
 *   am_object_notation_build(AM_OBJECT_NOTATION_BUILD_UINT64, 123);
 *
 * - String node with value "FOO":
 *   am_object_notation_build(AM_OBJECT_NOTATION_BUILD_STRING, "FOO");
 *
 * - Group with name "testgroup" and two integer members "i0" and "i1" with
 *   values 11 and 2987:
 *   am_object_notation_build(AM_OBJECT_NOTATION_BUILD_GROUP, "testgroup",
 *                            AM_OBJECT_NOTATION_BUILD_MEMBER, "i0", AM_OBJECT_NOTATION_BUILD_UINT64, 11,
 *                            AM_OBJECT_NOTATION_BUILD_MEMBER, "i1", AM_OBJECT_NOTATION_BUILD_UINT64, 2987,
 *                         AM_OBJECT_NOTATION_BUILD_END);
 *
 * - Group with a list of integers and a list of strings:
 *   am_object_notation_build(AM_OBJECT_NOTATION_BUILD_GROUP, "testgroup",
 *                           AM_OBJECT_NOTATION_BUILD_MEMBER, "int_list",
 *                             AM_OBJECT_NOTATION_BUILD_LIST,
 *                               AM_OBJECT_NOTATION_BUILD_UINT64, 1,
 *                               AM_OBJECT_NOTATION_BUILD_UINT64, 2,
 *                               AM_OBJECT_NOTATION_BUILD_UINT64, 3,
 *                               AM_OBJECT_NOTATION_BUILD_UINT64, 4,
 *                             AM_OBJECT_NOTATION_BUILD_END,
 *                           AM_OBJECT_NOTATION_BUILD_MEMBER, "string_list",
 *                             AM_OBJECT_NOTATION_BUILD_LIST,
 *                               AM_OBJECT_NOTATION_BUILD_STRING, "TEST",
 *                               AM_OBJECT_NOTATION_BUILD_STRING, "FOO",
 *                               AM_OBJECT_NOTATION_BUILD_STRING, "BAR",
 *                               AM_OBJECT_NOTATION_BUILD_STRING, "BAZ",
 *                             AM_OBJECT_NOTATION_BUILD_END,
 *                         AM_OBJECT_NOTATION_BUILD_END);
 *
 * On success, the function returns a pointer to the created and newly allocated
 * node, otherwise NULL.
 */
#define am_object_notation_build(...) __am_object_notation_build(0, __VA_ARGS__)

int __am_object_notation_node_group_build_add_members(
	struct am_object_notation_node_group* g, ...);

/* Builds a set of members and adds these members to a group g atomically. If
 * the construction of at least one member fails, nothing is added to the
 * group. On success, all members are added.
 *
 * Each member is defined by a name and a set of build verbs that create the
 * member, e.g., the following code adds three members a, b, and lst to a group
 * g, where a and b are integers and lst is a list of strings:
 *
 *   am_object_notation_node_group_build_add_members(g,
 *     AM_OBJECT_NOTATION_BUILD_MEMBER, "a",
 *       AM_OBJECT_NOTATION_BUILD_UINT64, 1,
 *     AM_OBJECT_NOTATION_BUILD_MEMBER, "b",
 *       AM_OBJECT_NOTATION_BUILD_UINT64, 2,
 *     AM_OBJECT_NOTATION_BUILD_MEMBER, "lst",
 *       AM_OBJECT_NOTATION_BUILD_LIST
 *         AM_OBJECT_NOTATION_BUILD_STRING, "aaa",
 *         AM_OBJECT_NOTATION_BUILD_STRING, "bbb",
 *         AM_OBJECT_NOTATION_BUILD_STRING, "ccc",
 *       AM_OBJECT_NOTATION_BUILD_END);
 *
 * Returns 0 on success, otherwise 1.
 */
#define am_object_notation_node_group_build_add_members(g, ...) \
	__am_object_notation_node_group_build_add_members(		\
		g,							\
		__VA_ARGS__,						\
		AM_OBJECT_NOTATION_BUILD_END)

struct am_object_notation_node*
am_object_notation_eval(const struct am_object_notation_node* n,
			const char* expr);

/* Defines a function for a non-composite type that casts the result of an
 * expression to the correct type and assigns the nodes value to a memory
 * location passed as an argument.
 *
 * For example,
 *
 *   AM_OBJECT_NOTATION_DECL_EVAL_RETRIEVE_FUN(
 *		int, AM_OBJECT_NOTATION_NODE_TYPE_INT, int64_t)
 *
 * defines a new function
 *
 *  static inline int
 *  am_object_notation_eval_retrieve_int(struct am_object_notation_node* n,
 *                                  const char* expr,
 *                                  int64_t* out)
 *
 * The parameter n is the root node on which the expression expr will be
 * evaluated. If the expression is not valid on the root node or if the
 * resulting node is not an integer node, the function returns 1. Otherwise, the
 * function return 0 and writes the integer value of the node at the address
 * specified by out.
 *
 * Data at the output address remains unchanged in case of an error and will
 * only be overwritten on success.
 */
#define AM_OBJECT_NOTATION_DECL_EVAL_RETRIEVE_FUN(type_name, type_cst, ctype)	\
	static inline int							\
	am_object_notation_eval_retrieve_##type_name(				\
		const struct am_object_notation_node* n,			\
		const char* expr, ctype* out)					\
	{									\
		struct am_object_notation_node* res;				\
		struct am_object_notation_node_##type_name* res_cast;		\
										\
		if(!(res = am_object_notation_eval(n, expr)))			\
			return 1;						\
										\
		if(res->type != type_cst)					\
			return 1;						\
										\
		res_cast = (struct am_object_notation_node_##type_name*)res;	\
		*out = res_cast->value;					\
										\
		return 0;							\
	}

AM_OBJECT_NOTATION_DECL_EVAL_RETRIEVE_FUN(
	int64, AM_OBJECT_NOTATION_NODE_TYPE_INT64, int64_t)
AM_OBJECT_NOTATION_DECL_EVAL_RETRIEVE_FUN(
	uint64, AM_OBJECT_NOTATION_NODE_TYPE_UINT64, uint64_t)
AM_OBJECT_NOTATION_DECL_EVAL_RETRIEVE_FUN(
	double, AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE, double)
AM_OBJECT_NOTATION_DECL_EVAL_RETRIEVE_FUN(
	string, AM_OBJECT_NOTATION_NODE_TYPE_STRING, const char*)

#endif
