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

#include "object_notation.h"
#include <stdarg.h>
#include <alloca.h>

static inline struct object_notation_node* object_notation_parse_group(struct parser* p);
static inline struct object_notation_node* object_notation_parse_node(struct parser* p);

/* Destroys a group node */
static inline void object_notation_node_group_destroy(struct object_notation_node_group* node)
{
	struct list_head* iter;
	struct list_head* n;
	free(node->name);

	/* Destroy and free each member */
	list_for_each_safe(iter, n, &node->members) {
		struct object_notation_node* node_iter =
			list_entry(iter, struct object_notation_node, siblings);
		object_notation_node_destroy(node_iter);
		free(node_iter);
	}
}

/* Destroys a list node */
static inline void object_notation_node_list_destroy(struct object_notation_node_list* node)
{
	struct list_head* iter;
	struct list_head* n;

	/* Destroy and free each item */
	list_for_each_safe(iter, n, &node->items) {
		struct object_notation_node* node_iter =
			list_entry(iter, struct object_notation_node, siblings);
		object_notation_node_destroy(node_iter);
		free(node_iter);
	}
}

/* Destroys a member node */
static inline void object_notation_node_member_destroy(struct object_notation_node_member* node)
{
	free(node->name);
	object_notation_node_destroy(node->def);
	free(node->def);
}

/* Destroys a string node */
static inline void object_notation_node_string_destroy(struct object_notation_node_string* node)
{
	free(node->value);
}

/* Destroys a node. Depending on the node type the correct destruction
 * function is called. The node is not freed. */
void object_notation_node_destroy(struct object_notation_node* node)
{
	switch(node->type) {
		case OBJECT_NOTATION_NODE_TYPE_GROUP:
			object_notation_node_group_destroy((struct object_notation_node_group*)node);
			break;
		case OBJECT_NOTATION_NODE_TYPE_MEMBER:
			object_notation_node_member_destroy((struct object_notation_node_member*)node);
			break;
		case OBJECT_NOTATION_NODE_TYPE_LIST:
			object_notation_node_list_destroy((struct object_notation_node_list*)node);
			break;
		case OBJECT_NOTATION_NODE_TYPE_STRING:
			object_notation_node_string_destroy((struct object_notation_node_string*)node);
			break;
		case OBJECT_NOTATION_NODE_TYPE_INT:
			break;
	}
}

/* Parses a list of the form [] or [E1, E2, ...]. Preceding whitespace
 * is ignored. The position of the parser will be set to the character
 * following the closing bracket. */
static inline struct object_notation_node* object_notation_parse_list(struct parser* p)
{
	struct parser_token t;
	struct object_notation_node_list* node;
	struct object_notation_node* item;

	if(parser_read_next_char(p, &t, '['))
		goto out_err;

	if(!(node = malloc(sizeof(*node)))) {
		goto out_err;
	}

	node->node.type = OBJECT_NOTATION_NODE_TYPE_LIST;
	INIT_LIST_HEAD(&node->node.siblings);
	INIT_LIST_HEAD(&node->items);

	/* Read items */
	do {
		parser_skip_ws(p);

		if(parser_peek_any_char(p, &t))
			goto out_err_destroy;

		if(parser_token_equals_char(&t, ']'))
			break;

		if(!(item = object_notation_parse_node(p)))
			goto out_err_destroy;

		list_add_tail(&item->siblings, &node->items);

		parser_skip_ws(p);

		if(parser_peek_any_char(p, &t))
			goto out_err_destroy;

		if(!parser_token_equals_char_oneof(&t, ",]"))
			goto out_err_destroy;

		if(parser_token_equals_char(&t, ','))
			parser_read_any_char(p, &t);
	} while(1);

	parser_skip_ws(p);

	if(parser_read_next_char(p, &t, ']'))
		goto out_err_destroy;

	return (struct object_notation_node*)node;

out_err_destroy:
	object_notation_node_list_destroy(node);
	free(node);
out_err:
	return NULL;
}

/* Parses a string literal. Preceding whitespace is ignored. The
 * string is unescaped. The position of the parser will be set to the
 * character following the last double quote. */
static inline struct object_notation_node* object_notation_parse_string(struct parser* p)
{
	struct parser_token t;
	struct object_notation_node_string* node;

	if(parser_read_next_string(p, &t))
		goto out_err;

	if(!(node = malloc(sizeof(*node)))) {
		goto out_err;
	}

	node->node.type = OBJECT_NOTATION_NODE_TYPE_STRING;
	INIT_LIST_HEAD(&node->node.siblings);

	if(!(node->value = unescape_stringn(t.str+1, t.len-2)))
		goto out_err_free;

	return (struct object_notation_node*)node;

out_err_free:
	free(node);
out_err:
	return NULL;
}

/* Parses an integer literal. Preceding whitespace is ignored. The
 * position of the parser will be set to the character following the
 * last digit. The function does not check whether the value fits into
 * an unsigned integer. */
static inline struct object_notation_node* object_notation_parse_int(struct parser* p)
{
	struct parser_token t;
	struct object_notation_node_int* node;

	if(parser_read_next_int(p, &t))
		return NULL;

	if(!(node = malloc(sizeof(*node))))
		return NULL;

	node->node.type = OBJECT_NOTATION_NODE_TYPE_INT;
	INIT_LIST_HEAD(&node->node.siblings);

	node->value = atou64n(t.str, t.len);

	return (struct object_notation_node*)node;
}

/* Parses an group member of the form "member_name: EXPR". Preceding
 * whitespace is ignored. The position of the parser will be set to
 * the character following the member definition. */
static inline struct object_notation_node* object_notation_parse_member(struct parser* p)
{
	struct parser_token t;
	struct object_notation_node_member* node;

	/* Read member name */
	if(parser_read_next_identifier(p, &t))
		goto out_err;

	if(!(node = malloc(sizeof(*node)))) {
		goto out_err;
	}

	node->node.type = OBJECT_NOTATION_NODE_TYPE_MEMBER;
	INIT_LIST_HEAD(&node->node.siblings);

	if(!(node->name = strdupn(t.str, t.len)))
		goto out_err_free;

	parser_skip_ws(p);

	if(parser_read_char(p, &t, ':'))
		goto out_err_free2;

	if(!(node->def = object_notation_parse_node(p)))
		goto out_err_free2;

	return (struct object_notation_node*)node;

out_err_free2:
	free(node->name);
out_err_free:
	free(node);
out_err:
	return NULL;
}

/* Parses a group body of the form "{ member_name1: EXPR1,
 * member_name1: EXPR, ...}". The empty group body "{}" is
 * allowed. Preceding whitespace is ignored. The position of the
 * parser will be set to the character following the group body. */
static inline int object_notation_parse_group_body(struct parser* p, struct object_notation_node_group* node)
{
	struct parser_token t;
	struct object_notation_node* member;

	if(parser_read_next_char(p, &t, '{'))
		return 1;

	/* Read members */
	do {
		parser_skip_ws(p);

		if(parser_peek_any_char(p, &t))
			return 1;

		if(parser_token_equals_char(&t, '}'))
			break;

		if(!(member = object_notation_parse_member(p)))
			return 1;

		list_add_tail(&member->siblings, &node->members);

		parser_skip_ws(p);

		if(parser_peek_any_char(p, &t))
			return 1;

		if(!parser_token_equals_char_oneof(&t, ",}"))
			return 1;

		if(parser_token_equals_char(&t, ','))
			parser_read_any_char(p, &t);
	} while(1);

	parser_skip_ws(p);

	if(parser_read_next_char(p, &t, '}'))
		return 1;

	return 0;
}

/* Parses a group group of the form "group_name { member_name1: EXPR1,
 * member_name1: EXPR, ...}". Preceding whitespace is ignored. The
 * position of the parser will be set to the character following the
 * group. */
static inline struct object_notation_node* object_notation_parse_group(struct parser* p)
{
	struct parser_token t;
	struct object_notation_node_group* node;

	if(parser_read_next_identifier(p, &t))
		goto out_err;

	if(!(node = malloc(sizeof(*node))))
		goto out_err;

	if(!(node->name = strdupn(t.str, t.len)))
		goto out_err_free;

	node->node.type = OBJECT_NOTATION_NODE_TYPE_GROUP;
	INIT_LIST_HEAD(&node->node.siblings);
	INIT_LIST_HEAD(&node->members);

	if(object_notation_parse_group_body(p, node))
		goto out_err_destroy;

	return (struct object_notation_node*)node;

out_err_destroy:
	object_notation_node_group_destroy(node);
out_err_free:
	free(node);
out_err:
	return NULL;
}

/* Parses a node. The type of the node is detected
 * automatically. Preceding whitespace is ignored. The position of the
 * parser will be set to the character following node. */
static inline struct object_notation_node* object_notation_parse_node(struct parser* p)
{
	struct parser_token t;

	parser_skip_ws(p);

	/* Determine type of the definition */
	if(parser_peek_any_char(p, &t))
		return NULL;

	if(t.str[0] == '"')
		return object_notation_parse_string(p);
	else if(t.str[0] == '[')
		return object_notation_parse_list(p);
	else if(isident_start_char(t.str[0]))
		return object_notation_parse_group(p);
	else if(isdigit(t.str[0]))
		return object_notation_parse_int(p);

	return NULL;
}

/* Parse an arbitrary object notation expression. Returns the root
 * node of the expression on success, otherwise NULL. */
struct object_notation_node* object_notation_parse(const char* str, size_t len)
{
	struct parser p;
	parser_init(&p, str, len);

	return object_notation_parse_node(&p);
}

/* Checks whether the list of members of a group contains members
 * whose names are given as the arguments. The list of member names
 * must be terminated by NULL. The maximum number of names is 64. If
 * exact is set to true, the function only returns true if the list of
 * members is composed only of members whose names are specified.*/
int object_notation_node_group_has_members(struct object_notation_node_group* node, int exact, ...)
{
	va_list vl;
	const char* name;
	struct object_notation_node_member* iter;
	size_t num_names = 0;
	uint64_t notfound = 0;
	size_t i;

	va_start(vl, exact);
	while((name = va_arg(vl, const char*))) {
		notfound |= (1 << num_names);
		num_names++;
	}
	va_end(vl);

	if(num_names > 64)
		return 0;

	if(exact && object_notation_node_group_num_members(node) != num_names)
		return 0;

	object_notation_for_each_group_member(node, iter) {
		i = 0;

		va_start(vl, exact);
		while((name = va_arg(vl, const char*))) {
			if(notfound & (1 << i)) {
				if(strcmp(iter->name, name) == 0) {
					notfound &= ~(1 << i);
					break;
				}
			}
			i++;
		}
		va_end(vl);
	}

	if(notfound)
		return 0;

	return 1;
}
