/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * Libaftermath-trace is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include <aftermath/trace/simple_hierarchy.h>
#include <aftermath/trace/on_disk_structs.h>
#include <aftermath/trace/on_disk_write_to_buffer.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/**
 * Intialize an empty simple hierarchy with the given name and id.
 * @return 0 on success, otherwise 1.
 */
int am_simple_hierarchy_init(struct am_simple_hierarchy* h,
			     const char* name,
			     am_hierarchy_id_t id)
{
	if(!(h->name = strdup(name)))
		return 1;

	h->id = id;
	h->root = NULL;

	return 0;
}

/**
 * Set the root node for the specified hierarchy.
 *
 * @return 0 on success, otherwise 1 (if a root node already exists)
 */
int am_simple_hierarchy_set_root(struct am_simple_hierarchy* h,
				 struct am_simple_hierarchy_node* n)
{
	if(h->root)
		return 1;

	h->root = n;
	n->parent = NULL;
	n->next_sibling = NULL;

	return 0;
}

/**
 * Dump a simple hierarchy to stdout in textual form, e.g.:
 *
 *   "<hierarchy_name>" : "<root_node_name>" {
 *   	"<node_name_1>" : { ... },
 *   	"<node_name_2>" : { ... },
 *   	"<node_name_3>" : { ... },
 *   	"<node_name_4>" : { ... },
 *   	...
 *   }
 */
void am_simple_hierarchy_dump_stdout(struct am_simple_hierarchy* h)
{
	printf("\"%s\": ", h->name);

	if(h->root)
		am_simple_hierarchy_node_dump_stdout(h->root, 0);

	puts("");
}

/**
 * Destroy a hierarchy, including all of its nodes.
 */
void am_simple_hierarchy_destroy(struct am_simple_hierarchy* h)
{
	if(h->root) {
		am_simple_hierarchy_node_destroy(h->root);
		free(h->root);
	}

	free(h->name);
}

/**
 * Write the simple hierarchy node n (including its children) to the buffer wb.
 *
 * @param h The hierarchy n belongs to
 * @param hierarchy_node_type The numerical ID for frames of type
 * am_dsk_hierarchy_node
 * @return 0 on success, otherwise 1.
 */
int am_simple_hierarchy_node_write_to_buffer(struct am_write_buffer* wb,
					     struct am_simple_hierarchy* h,
					     struct am_simple_hierarchy_node* n,
					     uint32_t hierarchy_node_type)
{
	struct am_dsk_hierarchy_node dsk_hn;
	struct am_simple_hierarchy_node* child;

	dsk_hn.hierarchy_id = h->id;
	dsk_hn.id = n->id;
	dsk_hn.parent_id = (n->parent) ? n->parent->id : 0;
	dsk_hn.name.str = n->name;
	dsk_hn.name.len = strlen(n->name);

	if(am_dsk_hierarchy_node_write_to_buffer(
		   wb, &dsk_hn, hierarchy_node_type))
	{
		return 1;
	}

	for(child = n->first_child; child; child = child->next_sibling) {
		if(am_simple_hierarchy_node_write_to_buffer(
			   wb, h, child, hierarchy_node_type))
		{
			return 1;
		}
	}

	return 0;
}

/**
 * Write the simple hierarchy h (including its nodes) to the buffer wb.
 *
 * @param hierarchy_description_type The numerical ID for frames of type
 * am_dsk_hierarchy_description
 * @param hierarchy_node_type The numerical ID for frames of type
 * am_dsk_hierarchy_node
 * @return 0 on success, otherwise 1.
 */
int am_simple_hierarchy_write_to_buffer(struct am_write_buffer* wb,
					struct am_simple_hierarchy* h,
					uint32_t hierarchy_description_type,
					uint32_t hierarchy_node_type)
{
	struct am_dsk_hierarchy_description dsk_hd;

	dsk_hd.id = h->id;
	dsk_hd.name.str = h->name;
	dsk_hd.name.len = strlen(h->name);

	if(am_dsk_hierarchy_description_write_to_buffer(
		   wb, &dsk_hd, hierarchy_description_type))
	{
		return 1;
	}

	if(am_simple_hierarchy_node_write_to_buffer(
		   wb, h, h->root, hierarchy_node_type))
	{
		return 1;
	}

	return 0;
}

/**
 * Simple data structure indicating the current parsing position
 */
struct am_simple_hierarchy_spec_parser {
	/** Pointer to the next character to be read */
	const char* curr;
};

/**
 * Simple data structure tp mark a substring
 */
struct am_simple_hierarchy_spec_token {
	/** Start of the substring */
	const char* str;

	/** Length in bytes of the substring */
	size_t len;
};

static struct am_simple_hierarchy_node*
am_simple_hierarchy_spec_build_node(
	struct am_simple_hierarchy_spec_parser* p,
	am_hierarchy_node_id_t* curr_id);

static inline int
am_simple_hierarchy_node_initn(struct am_simple_hierarchy_node* n,
			       const char* name,
			       size_t name_len,
			       am_hierarchy_id_t id);

/**
 * Skips all whitespace at the current position; after the call, the position is
 * on the first non-whitespace character or on the NUL character at the end of
 * the string.
 */
static inline void am_simple_hierarchy_spec_parser_skip_ws(
	struct am_simple_hierarchy_spec_parser* p)
{
	while(*p->curr && isblank(*p->curr))
		p->curr++;
}

/**
 * Reads a name in quotes at the current position of the parser and stores the
 * result in the token t.
 *
 * @return 0 on success, otherwise 1 (if no name could be read at the current
 * position).
 */
static inline int am_simple_hierarchy_spec_parse_name(
	struct am_simple_hierarchy_spec_parser* p,
	struct am_simple_hierarchy_spec_token* t)
{
	am_simple_hierarchy_spec_parser_skip_ws(p);

	if(*p->curr != '\"')
		return 1;

	p->curr++;
	t->str = p->curr;
	t->len = 0;

	while(*p->curr && *p->curr != '"') {
		p->curr++;
		t->len++;
	}

	if(*p->curr != '\"')
		return 1;

	p->curr++;

	return 0;
}

/**
 * Builds the list of children for the node passed as parent, by parsing the
 * list of children in textual representation, starting with the opening
 * brace. Consumes all characters of the list of children, including the final
 * closing brace. Leading whitespace is ignored.
 *
 *                 +----- start position
 *                 v
 *   "<node_name>" {
 *   	"<node_name_1>" : { ... },
 *   	"<node_name_2>" : { ... },
 *   	"<node_name_3>" : { ... },
 *   	"<node_name_4>" : { ... },
 *   	...
 *   }
 *    ^---- End position
 *
 * @return 0 on success, otherwise 1.
 */
static int am_simple_hierarchy_spec_build_children(
	struct am_simple_hierarchy_spec_parser* p,
	struct am_simple_hierarchy_node* parent,
	am_hierarchy_node_id_t* curr_id)
{
	struct am_simple_hierarchy_node* child;

	am_simple_hierarchy_spec_parser_skip_ws(p);

	if(*p->curr != '{')
		return 1;

	p->curr++;

	am_simple_hierarchy_spec_parser_skip_ws(p);

	/* Empty list of children? */
	if(*p->curr == '}')
		goto out;

	while(1) {
		if(!(child = am_simple_hierarchy_spec_build_node(p, curr_id)))
			return 1;

		am_simple_hierarchy_node_add_child(parent, child);

		am_simple_hierarchy_spec_parser_skip_ws(p);

		if(*p->curr != ',')
			break;

		p->curr++;
	}

	am_simple_hierarchy_spec_parser_skip_ws(p);

	if(*p->curr != '}')
		return 1;

out:
	p->curr++;

	return 0;
}

/**
 * Builds a node and all its children by parsing an entire node specification in
 * textual representation, starting with the name of the node. Consumes all
 * characters of the node specification, including the final closing
 * brace. Leading whitespace is ignored.
 *
 *   +----- start position
 *   v
 *   "<node_name>" {
 *   	"<node_name_1>" : { ... },
 *   	"<node_name_2>" : { ... },
 *   	"<node_name_3>" : { ... },
 *   	"<node_name_4>" : { ... },
 *   	...
 *   }
 *    ^---- End position
 *
 * @return The newly created node or NULL on failure.
 */
static struct am_simple_hierarchy_node*
am_simple_hierarchy_spec_build_node(
	struct am_simple_hierarchy_spec_parser* p,
	am_hierarchy_node_id_t* curr_id)
{
	struct am_simple_hierarchy_spec_token name;
	struct am_simple_hierarchy_node* n;

	if(am_simple_hierarchy_spec_parse_name(p, &name))
		goto out_err;

	if(!(n = malloc(sizeof(*n))))
		goto out_err;

	if(am_simple_hierarchy_node_initn(n, name.str, name.len, (*curr_id)++))
		goto out_free;

	if(am_simple_hierarchy_spec_build_children(p, n, curr_id))
		goto out_destroy;

	return n;

out_destroy:
	am_simple_hierarchy_node_destroy(n);
out_free:
	free(n);
out_err:
	return NULL;
}

/**
 * Build a new simple hierarchy with the given name and ID from a specification
 * string. The specification string must be of the form:
 *
 *  "node_name1" { "node_name2" { ... }, "node_name3" { ... }, ... }
 *
 * Since node names may contain spaces, quotes around each name are mandatory.
 *
 * @return a pointer to a newly created hierarchy of the specified form on
 * success or NULL on failure.
 */
struct am_simple_hierarchy*
am_simple_hierarchy_build(const char* name,
			  am_hierarchy_id_t id,
			  const char* spec)
{
	struct am_simple_hierarchy* h;
	struct am_simple_hierarchy_node* root;
	struct am_simple_hierarchy_spec_parser p = { .curr = spec };
	am_hierarchy_node_id_t curr_id = 1;

	if(!(h = malloc(sizeof(*h))))
		goto out_err;

	if(am_simple_hierarchy_init(h, name, id))
		goto out_err_free;

	if(!(root = am_simple_hierarchy_spec_build_node(&p, &curr_id)))
		goto out_err_destroy;

	if(am_simple_hierarchy_set_root(h, root))
		goto out_err_destroy;

	return h;

out_err_destroy:
	am_simple_hierarchy_destroy(h);
out_err_free:
	free(h);
out_err:
	return NULL;
}

/**
 * Retrieve the node from the hierarchy whose path from the root is [ path0,
 * ...]. Path0 must be the name of the root and the last argument must be
 * NULL. E.g., am_simple_hierarchy_get_node(h, "root", "a0", "aa1", NULL) yields
 * a reference to the node aa1 in:
 *
 *   "root" {
 *   	"a0" {
 *		"aa0" {
 *		},
 *		"aa1" {
 *		}
 *   	},
 *   	"a1" {
 *   	}
 *   }
 *
 * @return a pointer to the requested node or NULL if no such node exists.
 */
struct am_simple_hierarchy_node*
am_simple_hierarchy_get_node(struct am_simple_hierarchy* h,
			     const char* path0, ...)
{
	struct am_simple_hierarchy_node* node = h->root;
	const char* curr;
	va_list vl;

	if(!node || strcmp(node->name, path0) != 0)
		return NULL;

	va_start(vl, path0);

	while((curr = va_arg(vl, const char*))) {
		for(node = node->first_child; node; node = node->next_sibling) {
			if(strcmp(node->name, curr) == 0)
				break;
		}
	}

	va_end(vl);

	return node;
}

/**
 * Initialize a simple hierarchy node with the given name (specified as a
 * substring of name_len bytes, starting at address name) and id.
 *
 * @return 0 on success, otherwise 1.
 */
static inline int
am_simple_hierarchy_node_initn(struct am_simple_hierarchy_node* n,
			       const char* name,
			       size_t name_len,
			       am_hierarchy_id_t id)
{
	if(!(n->name = malloc(name_len + 1)))
		return 1;

	strncpy(n->name, name, name_len);
	n->name[name_len] = '\0';

	n->id = id;
	n->parent = NULL;
	n->first_child = NULL;
	n->next_sibling = NULL;

	return 0;
}

/**
 * Same as @see am_simple_hierarchy_node_initn, but using an ordinary
 * zero-terminated string for the name.
 */
int am_simple_hierarchy_node_init(struct am_simple_hierarchy_node* n,
				  const char* name,
				  am_hierarchy_id_t id)
{
	return am_simple_hierarchy_node_initn(n, name, strlen(name), id);
}

/**
 * Make n the parent of c.
 */
void am_simple_hierarchy_node_add_child(struct am_simple_hierarchy_node* n,
					struct am_simple_hierarchy_node* c)
{
	c->next_sibling = (n->first_child) ? n->first_child : NULL;
	c->parent = n;
	n->first_child = c;
}

/**
 * Remove the first child of n.
 */
void am_simple_hierarchy_node_remove_first_child(
	struct am_simple_hierarchy_node* n)
{
	if(!n->first_child)
		return;

	n->first_child = n->first_child->next_sibling;
}


/**
 * Dump a textutal representation of a single node and all of its children to
 * stdout.
 */
void am_simple_hierarchy_node_dump_stdout(struct am_simple_hierarchy_node* n,
					  size_t indent)
{
	struct am_simple_hierarchy_node* child;

	for(size_t i = 0; i < indent; i++)
		putchar('\t');

	printf("\"%s\" {", n->name);

	if(n->first_child)
		puts("");

	for(child = n->first_child; child; child = child->next_sibling) {
		am_simple_hierarchy_node_dump_stdout(child, indent+1);

		if(child->next_sibling)
			puts(",");
	}

	if(n->first_child) {
		puts("");

		for(size_t i = 0; i < indent; i++)
			putchar('\t');
	}

	printf("}");
}

/**
 * Destroy a simple hierarchy node and all of its descendants recursively
 */
void am_simple_hierarchy_node_destroy(struct am_simple_hierarchy_node* n)
{
	struct am_simple_hierarchy_node* child;
	struct am_simple_hierarchy_node* nchild;

	for(child = n->first_child,
		    nchild = (child) ? child->next_sibling : NULL;
	    child;
	    child = nchild,
		    nchild = (child) ? child->next_sibling : NULL)
	{
		am_simple_hierarchy_node_destroy(child);
		free(child);
	}

	free(n->name);
}
