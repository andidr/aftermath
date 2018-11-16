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

#ifndef AM_SIMPLE_HIERARCHY_H
#define AM_SIMPLE_HIERARCHY_H

#include <aftermath/trace/base_types.h>
#include <aftermath/trace/write_buffer.h>
#include <aftermath/trace/on_disk_default_type_ids.h>

struct am_simple_hierarchy_node;

/**
 * Minimal representation of an Aftermath hierarchy for tracing (i.e., without
 * data structures needed for a trace loaded into memory event mappings, etc. --
 * in contrast to struct am_hierarchy of libaftermath-core)
 */
struct am_simple_hierarchy {
	char* name;

	/* Trace-wide unique ID of this heirarchy */
	am_hierarchy_id_t id;

	/* Pointer to the root node; NULL if hierarchy is empty */
	struct am_simple_hierarchy_node* root;
};

int am_simple_hierarchy_init(struct am_simple_hierarchy* h,
			     const char* name,
			     am_hierarchy_id_t id);
int am_simple_hierarchy_set_root(struct am_simple_hierarchy* h,
				 struct am_simple_hierarchy_node* r);
struct am_simple_hierarchy*
am_simple_hierarchy_build(const char* name,
			  am_hierarchy_id_t id,
			  const char* spec);
void am_simple_hierarchy_destroy(struct am_simple_hierarchy* h);
void am_simple_hierarchy_dump_stdout(struct am_simple_hierarchy* h);

struct am_simple_hierarchy_node*
am_simple_hierarchy_get_node(struct am_simple_hierarchy* h,
			     const char* path0, ...);

int am_simple_hierarchy_write_to_buffer(struct am_write_buffer* wb,
					struct am_simple_hierarchy* h,
					uint32_t hierarchy_description_type,
					uint32_t hierarchy_node_type);

/* Same as am_simple_hierarchy_write_to_buffer, but uses the default on-disk
 * type IDs for am_dsk_hierarchy_description and am_dsk_hierarchy_node. */
static inline int
am_simple_hierarchy_write_to_buffer_defid(struct am_write_buffer* wb,
					  struct am_simple_hierarchy* h)
{
	return am_simple_hierarchy_write_to_buffer(
		wb,
		h,
		am_default_on_disk_type_ids.am_dsk_hierarchy_description,
		am_default_on_disk_type_ids.am_dsk_hierarchy_node);
}

int am_simple_hierarchy_node_write_to_buffer(struct am_write_buffer* wb,
					     struct am_simple_hierarchy* h,
					     struct am_simple_hierarchy_node* n,
					     uint32_t hierarchy_node_type);

/* Same as am_simple_hierarchy_node_write_to_buffer, but uses the default
 * on-disk type ID for am_dsk_hierarchy_node. */
static inline int
am_simple_hierarchy_node_write_to_buffer_defid(
	struct am_write_buffer* wb,
	struct am_simple_hierarchy* h,
	struct am_simple_hierarchy_node* n)
{
	return am_simple_hierarchy_node_write_to_buffer(
		wb, h, n,
		am_default_on_disk_type_ids.am_dsk_hierarchy_node);
}

/**
 * Minimal representation of an Aftermath hierarchy node for tracing (i.e.,
 * without data structures needed for a trace loaded into memory event mappings,
 * etc. -- in contrast to struct am_hierarchy_node of libaftermath-core)
 */
struct am_simple_hierarchy_node {
	/* Human-redable name for this node */
	char* name;

	/* Hierarchy-wide unique ID for this node */
	am_hierarchy_node_id_t id;

	/* Pointer to the parent of this node; NULL for the root node */
	struct am_simple_hierarchy_node* parent;

	/* Pointer to the first child of this node */
	struct am_simple_hierarchy_node* first_child;

	/* singly-linked list of all siblings belonging to the parent, not in
	 * any particular order */
	struct am_simple_hierarchy_node* next_sibling;
};

int am_simple_hierarchy_node_init(struct am_simple_hierarchy_node* n,
				  const char* name,
				  am_hierarchy_id_t id);
void am_simple_hierarchy_node_add_child(struct am_simple_hierarchy_node* n,
					struct am_simple_hierarchy_node* c);
void am_simple_hierarchy_node_remove_first_child(
	struct am_simple_hierarchy_node* n);
void am_simple_hierarchy_node_dump_stdout(struct am_simple_hierarchy_node* n,
					  size_t indent);
void am_simple_hierarchy_node_destroy(struct am_simple_hierarchy_node* n);

#endif
