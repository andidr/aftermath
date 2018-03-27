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

#include <aftermath/core/object_notation.h>
#include <stdarg.h>
#include <alloca.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

static inline struct am_object_notation_node*
am_object_notation_parse_group(struct am_parser* p);

static inline struct am_object_notation_node*
am_object_notation_parse_node(struct am_parser* p);

/* Allocate and initialize a group node using a non-zero-terminated string for
 * the name. Returns a reference to the newly allocated node on success,
 * otherwise NULL.
 */
struct am_object_notation_node_group*
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

/* Atomically moves all members of the group src to the group dst. If at least
 * one of the members cannot be moved, the function fails and both groups remain
 * unmodified.
 *
 * Returns 0 on success, 1 otherwise.
 */
int am_object_notation_node_group_move_members(
	struct am_object_notation_node_group* dst,
	struct am_object_notation_node_group* src)
{
	struct am_object_notation_node_member* m;

	/* Check if dst already has members with the same name as in src */
	am_object_notation_group_for_each_member(src, m) {
		if(am_object_notation_node_group_get_member_def(dst, m->name))
			return 1;
	}

	list_splice_tail_init(&src->members, &dst->members);

	return 0;
}


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

/* Allocate and initialize a list node. Returns a reference to the newly
 * allocated node on success, otherwise NULL.
 */
struct am_object_notation_node_list* am_object_notation_node_list_create(void)
{
	struct am_object_notation_node_list* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	am_object_notation_node_list_init(ret);

	return ret;
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

/* Allocate and initialize a member node using a non-zero-terminated string for
 * the name. Returns a reference to the newly allocated node on success,
 * otherwise NULL.
 */
struct am_object_notation_node_member*
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

/* Destroys a member node */
static inline void
am_object_notation_node_member_destroy(struct am_object_notation_node_member* node)
{
	free(node->name);

	if(node->def)
		am_object_notation_node_destroy(node->def);

	free(node->def);
}

/* Allocate and initialize a string node using a non-zero-terminated string for
 * the value. If unescape is non-zero, the value is unescaped after
 * duplication. Returns a reference to the newly allocated node on success,
 * otherwise NULL.
 */
struct am_object_notation_node_string*
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
		case AM_OBJECT_NOTATION_NODE_TYPE_INT64:
		case AM_OBJECT_NOTATION_NODE_TYPE_UINT64:
		case AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE:
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

/* Parses a signed 64-bit integer literal. Preceding whitespace is ignored. The
 * position of the parser will be set to the character following the
 * literal. The function does not check whether the value fits into a 64-bit
 * signed integer (i.e., if it overflows). */
static inline struct am_object_notation_node*
am_object_notation_parse_int64(struct am_parser* p)
{
	struct am_parser_token t;
	int64_t val;

	if(am_parser_read_next_int64(p, &t))
		return NULL;

	/* Skip the trailing "i64". Subtraction is safe, since t is guaranteed
	 * to include the suffix. */
	if(am_atoi64n(t.str, t.len - 3, &val))
		return NULL;

	return (struct am_object_notation_node*)
		am_object_notation_node_int64_create(val);
}

/* Parses an unsigned 64-bit integer literal. Preceding whitespace is
 * ignored. The position of the parser will be set to the character following
 * the literal. The function does not check whether the value fits into an
 * unsigned 64-bit signed integer (i.e., if it overflows). */
static inline struct am_object_notation_node*
am_object_notation_parse_uint64(struct am_parser* p)
{
	struct am_parser_token t;
	uint64_t val;

	if(am_parser_read_next_uint64(p, &t))
		return NULL;

	/* Skip the trailing "u64". Subtraction is safe, since t is guaranteed
	 * to include the suffix. */
	if(am_atou64n(t.str, t.len - 3, &val))
		return NULL;

	return (struct am_object_notation_node*)
		am_object_notation_node_uint64_create(val);
}

/* Parses a double literal. Preceding whitespace is ignored. The
 * position of the parser will be set to the character following the
 * last digit. The function does not check whether the value fits into
 * a double. */
static inline struct am_object_notation_node*
am_object_notation_parse_double(struct am_parser* p)
{
	struct am_parser_token t;
	double val;

	if(am_parser_read_next_double(p, &t))
		return NULL;

	if(am_atodbln(t.str, t.len, &val))
		return NULL;

	return (struct am_object_notation_node*)
		am_object_notation_node_double_create(val);
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

	if(am_parser_read_next_group_name(p, &t))
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
	else if(t.str[0] == '.')
		return am_object_notation_parse_double(p);
	else if(isdigit(t.str[0]) || t.str[0] == '-') {
		if(!am_parser_peek_double(p, &t))
			return am_object_notation_parse_double(p);
		if(!am_parser_peek_int64(p, &t))
			return am_object_notation_parse_int64(p);
		else
			return am_object_notation_parse_uint64(p);
	}

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

/* Same as am_object_notation_node_group_has_at_least_members, but used argument
 * list. */
int am_object_notation_node_group_has_at_least_membersv(
	struct am_object_notation_node_group* node,
	va_list arg)
{
	const char* name;
	struct am_object_notation_node_member* iter;
	int found;

	/* FIXME: use something faster */
	while((name = va_arg(arg, const char*))) {
		found = 0;

		am_object_notation_for_each_group_member(node, iter) {
			if(strcmp(iter->name, name) == 0) {
				found = 1;
				break;
			}
		}

		if(!found)
			return 0;
	}

	return 1;
}

/* Checks whether the list of members of a group contains at least the members
 * whose names given as the arguments. The list of member names must be
 * terminated by NULL.
 *
 * Returns 1 if the condition is met, otherwise 0.
 */
int am_object_notation_node_group_has_at_least_members(
	struct am_object_notation_node_group* node,
	...)
{
	va_list vl;
	int ret;

	va_start(vl, node);
	ret = am_object_notation_node_group_has_at_least_membersv(node, vl);
	va_end(vl);

	return ret;
}

/* Checks whether the list of members of a group contains at most the members
 * whose names given as the arguments, i.e., the group does not contain members
 * whose names are not specified as arguments. The list of member names must be
 * terminated by NULL.
 *
 * Returns 1 if the condition is met, otherwise 0.
 */
int am_object_notation_node_group_has_at_most_members(
	struct am_object_notation_node_group* node,
	...)
{
	va_list vl;
	const char* name;
	struct am_object_notation_node_member* iter;

	/* FIXME: use something faster */
	am_object_notation_for_each_group_member(node, iter) {
		va_start(vl, node);

		while((name = va_arg(vl, const char*)))
			if(strcmp(iter->name, name) != 0)
				return 0;

		va_end(vl);
	}

	return 1;
}

/* Checks whether a group is composed of exactly the members whose names are
 * given as the arguments. The list of member names must be terminated by
 * NULL.
 *
 * Returns 1 if the condition is met, otherwise 0.
 */
int am_object_notation_node_group_has_exactly_members(
	struct am_object_notation_node_group* node,
	...)
{
	va_list vl;
	int ret;
	size_t num_names = 0;
	size_t num_members;

	/* Count number of arguments */
	va_start(vl, node);
	while((va_arg(vl, const char*)))
		num_names++;
	va_end(vl);

	num_members = am_object_notation_node_group_num_members(node);

	va_start(vl, node);
	ret = am_object_notation_node_group_has_at_least_membersv(node, vl);
	va_end(vl);

	return ret && num_members == num_names;
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
	const char* list_start = "[";
	const char* item_sep = ", ";
	int has_composite;
	int item_indent = 0;
	int item_next_indent = 1;

	if(am_object_notation_node_list_is_empty(node)) {
		if(am_fputs_prefix("[]", "\t", indent, fp) < 0)
			return 1;

		return 0;
	}

	has_composite = am_object_notation_node_list_has_composite_item(node);

	if(has_composite) {
		list_start = "[\n";
		item_sep = ",\n";
		item_indent = next_indent;
	}	item_next_indent = next_indent+1;

	if(am_fputs_prefix(list_start, "\t", indent, fp) < 0)
		return 1;

	am_object_notation_for_each_list_item(node, iter) {
		if(am_object_notation_save_fp_indent(iter, fp, item_indent,
						     item_next_indent))
		{
			return 1;
		}

		if(!am_object_notation_node_list_is_last_item(node, iter))
			if(fputs(item_sep, fp) < 0)
				return 0;
	}

	if(has_composite) {
		if(fputs("\n", fp) < 0)
			return 1;

		if(am_fputs_prefix("]", "\t", next_indent, fp) < 0)
			return 1;
	} else {
		if(fputs("]", fp) < 0)
			return 1;
	}

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

/* Generate a textual representation of a 64-bit signed integer node and save it
 * to a file. The indent represent the current indentation used as a prefix for
 * the first character. Returns 0 on success, otherwise 1.*/
int am_object_notation_node_int64_save(
	struct am_object_notation_node_int64* node,
	FILE* fp, int indent)
{
	int ret;
	ret = am_fprintf_prefix(fp, "\t", indent, "%" PRId64 "i64", node->value);
	return ret < 0;
}

/* Generate a textual representation of an unsigned 64-bit integer node and save
 * it to a file. The indent represent the current indentation used as a prefix
 * for the first character. Returns 0 on success, otherwise 1.*/
int am_object_notation_node_uint64_save(
	struct am_object_notation_node_uint64* node,
	FILE* fp, int indent)
{
	int ret;
	ret = am_fprintf_prefix(fp, "\t", indent, "%" PRIu64 "u64", node->value);
	return ret < 0;
}

/* Allocate and initialize a 64-bit signed integer node. Returns a reference to
 * the newly allocated node on success, otherwise NULL.
 */
struct am_object_notation_node_int64*
am_object_notation_node_int64_create(int64_t value)
{
	struct am_object_notation_node_int64* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	am_object_notation_node_int64_init(ret, value);

	return ret;
}

/* Allocate and initialize a 64-bit unsigned integer node. Returns a reference
 * to the newly allocated node on success, otherwise NULL.
 */
struct am_object_notation_node_uint64*
am_object_notation_node_uint64_create(uint64_t value)
{
	struct am_object_notation_node_uint64* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	am_object_notation_node_uint64_init(ret, value);

	return ret;
}

/* Generate a textual representation of a double node and save it to a file. The
 * indent represent the current indentation used as a prefix for the first
 * character. Returns 0 on success, otherwise 1.*/
int am_object_notation_node_double_save(
	struct am_object_notation_node_double* node,
	FILE* fp, int indent)
{
	return am_fprintf_prefix(fp, "\t", indent, "%f", node->value) < 0;
}

/* Allocate and initialize a double node. Returns a reference to the newly
 * allocated node on success, otherwise NULL.
 */
struct am_object_notation_node_double*
am_object_notation_node_double_create(double value)
{
	struct am_object_notation_node_double* ret;

	if(!(ret = malloc(sizeof(*ret))))
		return NULL;

	am_object_notation_node_double_init(ret, value);

	return ret;
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
		case AM_OBJECT_NOTATION_NODE_TYPE_INT64:
			return am_object_notation_node_int64_save(
				(struct am_object_notation_node_int64*)node,
				fp, indent);
		case AM_OBJECT_NOTATION_NODE_TYPE_UINT64:
			return am_object_notation_node_uint64_save(
				(struct am_object_notation_node_uint64*)node,
				fp, indent);
		case AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE:
			return am_object_notation_node_double_save(
				(struct am_object_notation_node_double*)node,
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

static struct am_object_notation_node*
am_object_notation_vbuild(enum am_object_notation_build_verb type, va_list vl);

/* Build a string node. The next parameter pointed to by vl must be a pointer to
 * the string value of the node (const char*). Returns a newly allocated and
 * created string node on success, otherwise NULL.
 */
static struct am_object_notation_node*
am_object_notation_string_vbuild(va_list vl)
{
	const char* val = va_arg(vl, const char*);
	return (struct am_object_notation_node*)
		am_object_notation_node_string_create(val, 0);
}

/* Build a signed 64-bit integer node. The next parameter pointed to by vl must
 * be the integer value of the node (int64_t). Returns a newly allocated and
 * created integer node on success, otherwise NULL.
 */
static struct am_object_notation_node*
am_object_notation_int64_vbuild(va_list vl)
{
	int64_t val = va_arg(vl, int64_t);
	return (struct am_object_notation_node*)
		am_object_notation_node_int64_create(val);
}

/* Build an unsigned 64-bit integer node. The next parameter pointed to by vl
 * must be the integer value of the node (uint64_t). Returns a newly allocated
 * and created integer node on success, otherwise NULL.
 */
static struct am_object_notation_node*
am_object_notation_uint64_vbuild(va_list vl)
{
	uint64_t val = va_arg(vl, uint64_t);
	return (struct am_object_notation_node*)
		am_object_notation_node_uint64_create(val);
}

/* Build a double node. The next parameter pointed to by vl must be the double
 * value of the node. Returns a newly allocated and created double node on
 * success, otherwise NULL.
 */
static struct am_object_notation_node*
am_object_notation_double_vbuild(va_list vl)
{
	double val = va_arg(vl, double);
	return (struct am_object_notation_node*)
		am_object_notation_node_double_create(val);
}

/* Build a member node. The argument list vl must point to the remaining verbs /
 * values in the entire parameter list. Returns a newly allocated and created
 * node on success, otherwise NULL.
 */
static struct am_object_notation_node*
am_object_notation_member_vbuild(va_list vl)
{
	const char* name;
	enum am_object_notation_build_verb type;
	struct am_object_notation_node_member* member;
	struct am_object_notation_node* def;

	name = va_arg(vl, const char*);
	type = va_arg(vl, enum am_object_notation_build_verb);

	if(!(def = am_object_notation_vbuild(type, vl)))
		goto out_err;

	if(!(member = am_object_notation_node_member_create(name, def)))
		goto out_err_destroy;

	return (struct am_object_notation_node*)member;

out_err_destroy:
	am_object_notation_node_destroy(def);
	free(def);
out_err:
	return NULL;
}

/* Build a list node. The argument list vl must point to the remaining verbs /
 * values in the entire parameter list. Returns a newly allocated and created
 * node on success, otherwise NULL.
 */
static struct am_object_notation_node*
am_object_notation_list_vbuild(va_list vl)
{
	enum am_object_notation_build_verb verb;
	struct am_object_notation_node_list* list = NULL;
	struct am_object_notation_node* item;

	if(!(list = am_object_notation_node_list_create()))
		goto out_err;

	while((verb = va_arg(vl, enum am_object_notation_build_verb)) !=
	      AM_OBJECT_NOTATION_BUILD_END)
	{
		if(!(item = am_object_notation_vbuild(verb, vl)))
			goto out_err_destroy;

		am_object_notation_node_list_add_item(list, item);
	}

	return (struct am_object_notation_node*)list;

out_err_destroy:
	am_object_notation_node_list_destroy(list);
	free(list);
out_err:
	return NULL;
}

/* Builds all the members of a group and adds them to the group. Returns 0 on
 * success, 1 otherwise.
 */
static int
am_object_notation_group_vbuild_members(
	struct am_object_notation_node_group* group,
	va_list vl)
{
	struct am_object_notation_node_member* member;
	enum am_object_notation_build_verb verb;

	while((verb = va_arg(vl, enum am_object_notation_build_verb)) !=
	      AM_OBJECT_NOTATION_BUILD_END)
	{
		if(!(member = (struct am_object_notation_node_member*)
		     am_object_notation_vbuild(verb, vl)))
		{
			return 1;
		}

		am_object_notation_node_group_add_member(group, member);
	}

	return 0;
}


/* Build a group node. The argument list vl must point to the remaining verbs /
 * values in the entire parameter list. Returns a newly allocated and created
 * node on success, otherwise NULL.
 */
static struct am_object_notation_node*
am_object_notation_group_vbuild(va_list vl)
{
	struct am_object_notation_node_group* group = NULL;
	const char* name;

	name = va_arg(vl, const char*);

	if(!(group = am_object_notation_node_group_create(name)))
		goto out_err;

	if(am_object_notation_group_vbuild_members(group, vl))
		goto out_err_destroy;

	return (struct am_object_notation_node*)group;

out_err_destroy:
	am_object_notation_node_group_destroy(group);
	free(group);
out_err:
	return NULL;
}

/* Build an object node of a given type. The argument list vl must point to the
 * remaining verbs / values in the entire parameter list. Returns a newly
 * allocated and created node on success, otherwise NULL.
 */
static struct am_object_notation_node*
am_object_notation_vbuild(enum am_object_notation_build_verb type, va_list vl)
{
	switch(type) {
		case AM_OBJECT_NOTATION_BUILD_GROUP:
			return am_object_notation_group_vbuild(vl);
		case AM_OBJECT_NOTATION_BUILD_LIST:
			return am_object_notation_list_vbuild(vl);
		case AM_OBJECT_NOTATION_BUILD_INT64:
			return am_object_notation_int64_vbuild(vl);
		case AM_OBJECT_NOTATION_BUILD_UINT64:
			return am_object_notation_uint64_vbuild(vl);
		case AM_OBJECT_NOTATION_BUILD_DOUBLE:
			return am_object_notation_double_vbuild(vl);
		case AM_OBJECT_NOTATION_BUILD_STRING:
			return am_object_notation_string_vbuild(vl);
		case AM_OBJECT_NOTATION_BUILD_MEMBER:
			return am_object_notation_member_vbuild(vl);
		case AM_OBJECT_NOTATION_BUILD_END:
			return NULL;
	}

	return NULL;
}

/* @see am_object_notation_build */
struct am_object_notation_node* __am_object_notation_build(int dummy, ...)
{
	struct am_object_notation_node* ret;
	enum am_object_notation_build_verb type;
	va_list vl;

	va_start(vl, dummy);
	type = va_arg(vl, enum am_object_notation_build_verb);
	ret = am_object_notation_vbuild(type, vl);
	va_end(vl);

	return ret;
}

/* Same as am_object_notation_node_group_build_add_members, but takes an
 * argument list and expecting a AM_OBJECT_NOTATION_BUILD_END verb as the last
 * parameter.
 */
int __am_object_notation_node_group_build_add_members(
	struct am_object_notation_node_group* g, ...)
{
	struct am_object_notation_node_group gtmp;
	va_list vl;
	int ret;

	am_object_notation_node_group_init_no_name(&gtmp);

	va_start(vl, g);
	ret = am_object_notation_group_vbuild_members(&gtmp, vl);
	va_end(vl);

	if(ret)
		goto out;

	ret = am_object_notation_node_group_move_members(g, &gtmp);

out:
	am_object_notation_node_group_destroy(&gtmp);

	return ret;
}

static struct am_object_notation_node*
am_object_notation_eval_dot(const struct am_object_notation_node* n,
			    struct am_parser* p);

/* Evaluates the next sub-expression of a parser on an object notation
 * group. Returns the node resulting from the evaluation of the entire remaining
 * expression or NULL in case of an error. */
static struct am_object_notation_node*
am_object_notation_eval_group(const struct am_object_notation_node_group* g,
			      struct am_parser* p)
{
	struct am_parser_token t;
	struct am_object_notation_node_member* member;

	if(am_parser_read_next_identifier(p, &t))
		return NULL;

	am_object_notation_for_each_group_member(g, member) {
		if(am_strn1eq(t.str, t.len, member->name))
			return am_object_notation_eval_dot(member->def, p);
	}

	return NULL;
}

/* Evaluates the next sub-expression of a parser on an object notation
 * list. Returns the node resulting from the evaluation of the entire remaining
 * expression or NULL in case of an error. */
static struct am_object_notation_node*
am_object_notation_eval_list(const  struct am_object_notation_node_list* l,
			     struct am_parser* p)
{
	struct am_parser_token t;
	struct am_object_notation_node* item;
	uint64_t idx;

	if(am_parser_read_next_char(p, &t, '['))
		return NULL;

	if(am_parser_read_next_uint64(p, &t))
		return NULL;

	if(am_atou64n(t.str, t.len, &idx))
		return NULL;

	if(am_parser_read_next_char(p, &t, ']'))
		return NULL;

	if(!(item = am_object_notation_node_list_nth_member(l, idx)))
		return NULL;

	return am_object_notation_eval_dot(item, p);
}

/* Evaluates the remaining expression of a parser on an object node. Returns the
 * address of the node resulting from the evaluation or NULL in case of an
 * error.*/
static struct am_object_notation_node*
__am_object_notation_eval(const struct am_object_notation_node* n,
			  struct am_parser* p)
{
	switch(n->type) {
		case AM_OBJECT_NOTATION_NODE_TYPE_GROUP:
			return am_object_notation_eval_group(
				(const struct am_object_notation_node_group*)n, p);
		case AM_OBJECT_NOTATION_NODE_TYPE_LIST:
			return am_object_notation_eval_list(
				(const struct am_object_notation_node_list*)n, p);
		case AM_OBJECT_NOTATION_NODE_TYPE_STRING:
		case AM_OBJECT_NOTATION_NODE_TYPE_INT64:
		case AM_OBJECT_NOTATION_NODE_TYPE_UINT64:
		case AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE:
		case AM_OBJECT_NOTATION_NODE_TYPE_MEMBER:
			return NULL;
	}

	/* Cannot happen */
	return NULL;
}

/* Same as __am_object_notation_eval, but consumes a leading dot if present. */
static struct am_object_notation_node*
am_object_notation_eval_dot(const struct am_object_notation_node* n,
			    struct am_parser* p)
{
	struct am_parser_token t;
	int expect_dot = 0;

	if(am_parser_reached_end(p))
		return (struct am_object_notation_node*)n;

	switch(n->type) {
		case AM_OBJECT_NOTATION_NODE_TYPE_GROUP:
			expect_dot = 1;
			break;
		case AM_OBJECT_NOTATION_NODE_TYPE_LIST:
		case AM_OBJECT_NOTATION_NODE_TYPE_STRING:
		case AM_OBJECT_NOTATION_NODE_TYPE_INT64:
		case AM_OBJECT_NOTATION_NODE_TYPE_UINT64:
		case AM_OBJECT_NOTATION_NODE_TYPE_DOUBLE:
		case AM_OBJECT_NOTATION_NODE_TYPE_MEMBER:
			expect_dot = 0;
			break;
	}

	if(expect_dot)
		if(am_parser_read_next_char(p, &t, '.'))
			return NULL;

	return __am_object_notation_eval(n, p);
}


/* Evaluates a selection string on an object notation node. An expression can be
 * composed of a concatenation separated with points of the following basic
 * expressions:
 *
 * - A member expression (only valid on groups), composed of the name of the
 *   group member
 *
 * - A list index expression (only valid on lists), represented by an integer in
 *   brackets
 *
 * Example:
 *
 *  person {
 *     name: "foo",
 *     age: 30,
 *     favorite_meals: [ "pizza", "spaghetti", "hamburger", "chicken" ]
 *  }
 *
 * To select the name or age of the person, one would evaluate the expression
 * "name" or "age", respectively. To get the first favorite meal, one would
 * evaluate "favorite_meals[0]", for the second favorite meal
 * "favorite_meal[1]", and so on.
 */
struct am_object_notation_node*
am_object_notation_eval(const struct am_object_notation_node* n,
			const char* expr)
{
	struct am_parser p;

	am_parser_init(&p, expr, strlen(expr));

	return __am_object_notation_eval(n, &p);
}
