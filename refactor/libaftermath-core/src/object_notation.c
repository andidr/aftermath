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

#include "object_notation.h"
#include <stdarg.h>
#include <alloca.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static inline struct am_object_notation_node*
am_object_notation_parse_group(struct am_parser* p);

static inline struct am_object_notation_node*
am_object_notation_parse_node(struct am_parser* p);

/* Destroys a group node */
static inline void
am_object_notation_node_group_destroy(struct am_object_notation_node_group* node)
{
	struct list_head* iter;
	struct list_head* n;
	free(node->name);

	/* Destroy and free each member */
	list_for_each_safe(iter, n, &node->members) {
		struct am_object_notation_node* node_iter =
			list_entry(iter, struct am_object_notation_node, siblings);
		am_object_notation_node_destroy(node_iter);
		free(node_iter);
	}
}

/* Destroys a list node */
static inline void
am_object_notation_node_list_destroy(struct am_object_notation_node_list* node)
{
	struct list_head* iter;
	struct list_head* n;

	/* Destroy and free each item */
	list_for_each_safe(iter, n, &node->items) {
		struct am_object_notation_node* node_iter =
			list_entry(iter, struct am_object_notation_node, siblings);
		am_object_notation_node_destroy(node_iter);
		free(node_iter);
	}
}

/* Destroys a member node */
static inline void
am_object_notation_node_member_destroy(struct am_object_notation_node_member* node)
{
	free(node->name);

	if(node->def)
		am_object_notation_node_destroy(node->def);

	free(node->def);
}

/* Destroys a string node */
static inline void
am_object_notation_node_string_destroy(struct am_object_notation_node_string* node)
{
	free(node->value);
}

/* Destroys a node. Depending on the node type the correct destruction
 * function is called. The node is not freed. */
void am_object_notation_node_destroy(struct am_object_notation_node* node)
{
	switch(node->type) {
		case AM_OBJECT_NOTATION_NODE_TYPE_GROUP:
			am_object_notation_node_group_destroy(
				(struct am_object_notation_node_group*)node);
			break;
		case AM_OBJECT_NOTATION_NODE_TYPE_MEMBER:
			am_object_notation_node_member_destroy(
				(struct am_object_notation_node_member*)node);
			break;
		case AM_OBJECT_NOTATION_NODE_TYPE_LIST:
			am_object_notation_node_list_destroy(
				(struct am_object_notation_node_list*)node);
			break;
		case AM_OBJECT_NOTATION_NODE_TYPE_STRING:
			am_object_notation_node_string_destroy(
				(struct am_object_notation_node_string*)node);
			break;
		case AM_OBJECT_NOTATION_NODE_TYPE_INT:
			break;
	}
}

/* Parses a list of the form [] or [E1, E2, ...]. Preceding whitespace
 * is ignored. The position of the parser will be set to the character
 * following the closing bracket. */
static inline struct am_object_notation_node*
am_object_notation_parse_list(struct am_parser* p)
{
	struct am_parser_token t;
	struct am_object_notation_node_list* node;
	struct am_object_notation_node* item;

	if(am_parser_read_next_char(p, &t, '['))
		goto out_err;

	if(!(node = am_object_notation_node_list_create()))
		goto out_err;

	/* Read items */
	do {
		am_parser_skip_ws(p);

		if(am_parser_peek_any_char(p, &t))
			goto out_err_destroy;

		if(am_parser_token_equals_char(&t, ']'))
			break;

		if(!(item = am_object_notation_parse_node(p)))
			goto out_err_destroy;

		am_object_notation_node_list_add_item(node, item);

		am_parser_skip_ws(p);

		if(am_parser_peek_any_char(p, &t))
			goto out_err_destroy;

		if(!am_parser_token_equals_char_oneof(&t, ",]"))
			goto out_err_destroy;

		if(am_parser_token_equals_char(&t, ','))
			am_parser_read_any_char(p, &t);
	} while(1);

	am_parser_skip_ws(p);

	if(am_parser_read_next_char(p, &t, ']'))
		goto out_err_destroy;

	return (struct am_object_notation_node*)node;

out_err_destroy:
	am_object_notation_node_list_destroy(node);
	free(node);
out_err:
	return NULL;
}

/* Parses a string literal. Preceding whitespace is ignored. The
 * string is unescaped. The position of the parser will be set to the
 * character following the last double quote. */
static inline struct am_object_notation_node*
am_object_notation_parse_string(struct am_parser* p)
{
	struct am_parser_token t;

	if(am_parser_read_next_string(p, &t))
		return NULL;

	return (struct am_object_notation_node*)
		am_object_notation_node_string_createn(t.str+1, t.len-2, 1);
}

/* Parses an integer literal. Preceding whitespace is ignored. The
 * position of the parser will be set to the character following the
 * last digit. The function does not check whether the value fits into
 * an unsigned integer. */
static inline struct am_object_notation_node*
am_object_notation_parse_int(struct am_parser* p)
{
	struct am_parser_token t;
	uint64_t val;

	if(am_parser_read_next_int(p, &t))
		return NULL;

	if(am_atou64n(t.str, t.len, &val))
		return NULL;

	return (struct am_object_notation_node*)
		am_object_notation_node_int_create(val);
}

/* Parses an group member of the form "member_name: EXPR". Preceding
 * whitespace is ignored. The position of the parser will be set to
 * the character following the member definition. */
static inline struct am_object_notation_node*
am_object_notation_parse_member(struct am_parser* p)
{
	struct am_parser_token t;
	struct am_object_notation_node_member* node;

	/* Read member name */
	if(am_parser_read_next_identifier(p, &t))
		goto out_err;

	if(!(node = am_object_notation_node_member_createn(t.str, t.len, NULL)))
		goto out_err;

	am_parser_skip_ws(p);

	if(am_parser_read_char(p, &t, ':'))
		goto out_err_destroy;

	if(!(node->def = am_object_notation_parse_node(p)))
		goto out_err_destroy;

	return (struct am_object_notation_node*)node;

out_err_destroy:
	am_object_notation_node_member_destroy(node);
	free(node);
out_err:
	return NULL;
}

/* Parses a group body of the form "{ member_name1: EXPR1,
 * member_name1: EXPR, ...}". The empty group body "{}" is
 * allowed. Preceding whitespace is ignored. The position of the
 * parser will be set to the character following the group body. */
static inline int
am_object_notation_parse_group_body(
	struct am_parser* p,
	struct am_object_notation_node_group* node)
{
	struct am_parser_token t;
	struct am_object_notation_node* member;

	if(am_parser_read_next_char(p, &t, '{'))
		return 1;

	/* Read members */
	do {
		am_parser_skip_ws(p);

		if(am_parser_peek_any_char(p, &t))
			return 1;

		if(am_parser_token_equals_char(&t, '}'))
			break;

		if(!(member = am_object_notation_parse_member(p)))
			return 1;

		am_object_notation_node_group_add_member(
			node,
			(struct am_object_notation_node_member*)member);

		am_parser_skip_ws(p);

		if(am_parser_peek_any_char(p, &t))
			return 1;

		if(!am_parser_token_equals_char_oneof(&t, ",}"))
			return 1;

		if(am_parser_token_equals_char(&t, ','))
			am_parser_read_any_char(p, &t);
	} while(1);

	am_parser_skip_ws(p);

	if(am_parser_read_next_char(p, &t, '}'))
		return 1;

	return 0;
}

/* Parses a group group of the form "group_name { member_name1: EXPR1,
 * member_name1: EXPR, ...}". Preceding whitespace is ignored. The
 * position of the parser will be set to the character following the
 * group. */
static inline struct am_object_notation_node*
am_object_notation_parse_group(struct am_parser* p)
{
	struct am_parser_token t;
	struct am_object_notation_node_group* node;

	if(am_parser_read_next_identifier(p, &t))
		goto out_err;

	if(!(node = am_object_notation_node_group_createn(t.str, t.len)))
		goto out_err;

	if(am_object_notation_parse_group_body(p, node))
		goto out_err_destroy;

	return (struct am_object_notation_node*)node;

out_err_destroy:
	am_object_notation_node_group_destroy(node);
out_err:
	return NULL;
}

/* Parses a node. The type of the node is detected
 * automatically. Preceding whitespace is ignored. The position of the
 * parser will be set to the character following node. */
static inline struct am_object_notation_node*
am_object_notation_parse_node(struct am_parser* p)
{
	struct am_parser_token t;

	am_parser_skip_ws(p);

	/* Determine type of the definition */
	if(am_parser_peek_any_char(p, &t))
		return NULL;

	if(t.str[0] == '"')
		return am_object_notation_parse_string(p);
	else if(t.str[0] == '[')
		return am_object_notation_parse_list(p);
	else if(isident_start_char(t.str[0]))
		return am_object_notation_parse_group(p);
	else if(isdigit(t.str[0]))
		return am_object_notation_parse_int(p);

	return NULL;
}

/* Parse an arbitrary object notation expression. Returns the root
 * node of the expression on success, otherwise NULL. */
struct am_object_notation_node*
am_object_notation_parse(const char* str, size_t len)
{
	struct am_parser p;
	am_parser_init(&p, str, len);

	return am_object_notation_parse_node(&p);
}

/* Checks whether the list of members of a group contains members
 * whose names are given as the arguments. The list of member names
 * must be terminated by NULL. The maximum number of names is 64. If
 * exact is set to true, the function only returns true if the list of
 * members is composed only of members whose names are specified.*/
int am_object_notation_node_group_has_members(
	struct am_object_notation_node_group* node,
	int exact,
	...)
{
	va_list vl;
	const char* name;
	struct am_object_notation_node_member* iter;
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

	if(exact && am_object_notation_node_group_num_members(node) != num_names)
		return 0;

	am_object_notation_for_each_group_member(node, iter) {
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

/* Generate a textual representation of a group node and save it to a file. The
 * indent represent the current indentation used as a prefix for the next
 * character, while next_indent specifies by how many tabs the next line will be
 * indented. Returns 0 on success, otherwise 1.*/
int am_object_notation_node_group_save(
	struct am_object_notation_node_group* node,
	FILE* fp, int indent, int next_indent)
{
	struct am_object_notation_node_member* iter;

	if(am_fprintf_prefix(fp, "\t", indent, "%s {\n", node->name) < 0)
		return 1;

	am_object_notation_for_each_group_member(node, iter) {
		if(am_object_notation_node_member_save(iter, fp, next_indent+1, next_indent+2))
			return 1;

		if(!am_object_notation_node_group_is_last_member(node, iter)) {
			if(fputs(",\n", fp) < 0)
				return 1;
		} else {
			if(fputs("\n", fp) < 0)
				return 1;
		}
	}

	if(am_fputs_prefix("}", "\t", next_indent, fp) < 0)
		return 1;

	return 0;
}

/* Generate a textual representation of a member node and save it to a file. The
 * indent represent the current indentation used as a prefix for the next
 * character, while next_indent specifies by how many tabs the next line will be
 * indented. Returns 0 on success, otherwise 1.*/
int am_object_notation_node_member_save(
	struct am_object_notation_node_member* node,
	FILE* fp, int indent, int next_indent)
{
	if(am_fprintf_prefix(fp, "\t", indent, "%s: ", node->name) < 0)
		return 1;

	if(am_object_notation_save_fp_indent(node->def, fp, 0, next_indent))
		return 1;

	return 0;
}

/* Generate a textual representation of a list node and save it to a file. The
 * indent represent the current indentation used as a prefix for the next
 * character, while next_indent specifies by how many tabs the next line will be
 * indented. Returns 0 on success, otherwise 1.*/
int am_object_notation_node_list_save(
	struct am_object_notation_node_list* node,
	FILE* fp, int indent, int next_indent)
{
	struct am_object_notation_node* iter;

	if(am_object_notation_node_list_is_empty(node)) {
		if(am_fputs_prefix("[]", "\t", indent, fp) < 0)
			return 1;

		return 0;
	}

	if(am_fputs_prefix("[\n", "\t", indent, fp) < 0)
		return 1;

	am_object_notation_for_each_list_item(node, iter) {
		if(am_object_notation_save_fp_indent(
			   iter, fp,
			   next_indent, next_indent+1))
		{
			return 1;
		}

		if(!am_object_notation_node_list_is_last_item(node, iter))
			if(fputs(",\n", fp) < 0)
				return 0;
	}

	if(fputs("\n", fp) < 0)
		return 1;

	if(am_fputs_prefix("]", "\t", next_indent, fp) < 0)
		return 1;

	return 0;
}

/* Generate a textual representation of a string node and save it to a file. The
 * indent represent the current indentation used as a prefix for the first
 * character. Returns 0 on success, otherwise 1.*/
int am_object_notation_node_string_save(
	struct am_object_notation_node_string* node,
	FILE* fp, int indent)
{
	char* escaped;

	if(!(escaped = am_escape_string(node->value)))
		return 1;

	if(am_fprintf_prefix(fp, "\t", indent, "\"%s\"", escaped) < 0) {
		free(escaped);
		return 1;
	}

	free(escaped);

	return 0;
}

/* Generate a textual representation of an integer node and save it to a
 * file. The indent represent the current indentation used as a prefix for the
 * first character. Returns 0 on success, otherwise 1.*/
int am_object_notation_node_int_save(
	struct am_object_notation_node_int* node,
	FILE* fp, int indent)
{
	return am_fprintf_prefix(fp, "\t", indent, "%"PRId64, node->value) < 0;
}

/* Generate a textual representation of a node and save it to a file. The indent
 * represent the current indentation used as a prefix for the next character,
 * while next_indent specifies by how many tabs the next line will be
 * indented. Returns 0 on success, otherwise 1.*/
int am_object_notation_save_fp_indent(struct am_object_notation_node* node,
				   FILE* fp, int indent, int next_indent)
{
	switch(node->type) {
		case AM_OBJECT_NOTATION_NODE_TYPE_GROUP:
			return am_object_notation_node_group_save(
				(struct am_object_notation_node_group*)node,
				fp, indent, next_indent);
		case AM_OBJECT_NOTATION_NODE_TYPE_MEMBER:
			return am_object_notation_node_member_save(
				(struct am_object_notation_node_member*)node,
				fp, indent, next_indent);
		case AM_OBJECT_NOTATION_NODE_TYPE_LIST:
			return am_object_notation_node_list_save(
				(struct am_object_notation_node_list*)node,
				fp, indent, next_indent);
		case AM_OBJECT_NOTATION_NODE_TYPE_STRING:
			return am_object_notation_node_string_save(
				(struct am_object_notation_node_string*)node,
				fp, indent);
		case AM_OBJECT_NOTATION_NODE_TYPE_INT:
			return am_object_notation_node_int_save(
				(struct am_object_notation_node_int*)node,
				fp, indent);
	}

	return 1;
}

/* Generate a textual representation of a node and save it to a file. Returns 0
 * on success, otherwise 1.*/
int am_object_notation_save_fp(struct am_object_notation_node* node, FILE* fp)
{
	return am_object_notation_save_fp_indent(node, fp, 0, 0);
}

/* Generate a textual representation of a node and save it to a file. The file
 * will be overwritten or created if necessary. Returns 0 on success, otherwise
 * 1.*/
int am_object_notation_save(
	struct am_object_notation_node* node,
	const char* filename)
{
	FILE* fp = fopen(filename, "w+");
	int ret = 0;

	if(!fp)
		return 1;

	ret = am_object_notation_save_fp(node, fp);
	fclose(fp);

	return ret;
}

/*
 * Load the an object notation node from a file. Returns a newly created node or
 * NULL if an error occurred.
 */
struct am_object_notation_node* am_object_notation_load(const char* filename)
{
	int fd;
	char* str;
	off_t size;
	struct am_object_notation_node* ret = NULL;

	if((size = am_file_size(filename)) == -1)
		goto out;

	if((fd = open(filename, O_RDONLY)) == -1)
		goto out;

	if((str = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		goto out_fd;

	ret = am_object_notation_parse(str, size);

	munmap(str, size);

out_fd:
	close(fd);
out:
	return ret;
}
