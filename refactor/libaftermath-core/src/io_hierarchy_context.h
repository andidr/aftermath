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

#ifndef AM_IO_HIERARCHY_CONTEXT_H
#define AM_IO_HIERARCHY_CONTEXT_H

/* An I/O hierarchy context is used temporarily when a trace is loaded. Its
 * purpose is to provide an interface for retrieving already loaded hierarchies
 * and hierarchy nodes by their IDs extracted from the associated on-disk data
 * structures.
 */

#include <aftermath/core/typed_rbtree.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/base_types.h>
#include <aftermath/core/hierarchy.h>

/* Represents one hierarchy with its currently loaded nodes. Mappings for the
 * hierarchy nodes are organized in a red-black tree with instances of struct
 * am_io_hierarchy_context_tree_node. */
struct am_io_hierarchy_context_tree {
	struct am_hierarchy* hierarchy;

	am_hierarchy_id_t id;

	/* Root of the red-black tree with all nodes of the hierarchy */
	struct rb_root rb_root;
};

AM_DECL_TYPED_ARRAY(am_io_hierarchy_context_tree_array,
		    struct am_io_hierarchy_context_tree)

#define IO_HIERARCHY_CONTEXT_TREE_ACC_ID(x) ((x).id)

AM_DECL_TYPED_ARRAY_BSEARCH(am_io_hierarchy_context_tree_array,
			    struct am_io_hierarchy_context_tree,
			    am_hierarchy_id_t,
			    IO_HIERARCHY_CONTEXT_TREE_ACC_ID,
			    AM_VALCMP_EXPR)

AM_DECL_TYPED_ARRAY_INSERTPOS(am_io_hierarchy_context_tree_array,
			      struct am_io_hierarchy_context_tree,
			      am_hierarchy_id_t,
			      IO_HIERARCHY_CONTEXT_TREE_ACC_ID,
			      AM_VALCMP_EXPR)

AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_io_hierarchy_context_tree_array,
				struct am_io_hierarchy_context_tree,
				am_hierarchy_id_t)

/* Mapping for a single hierarchy node from a node ID to the node's address in
 * memory. */
struct am_io_hierarchy_context_tree_node {
	struct am_hierarchy_node* hierarchy_node;
	am_hierarchy_node_id_t id;
	struct rb_node rb_node;
};


#define IO_HIERARCHY_CONTEXT_TREE_NODE_ACC_ID(x) ((x).id)
#define IO_HIERARCHY_CONTEXT_TREE_NODE_CMP_IDS(a, b) \
	(((a) < (b)) ? -1 : (((a) > (b)) ? 1 : 0))

AM_DECL_TYPED_RBTREE_OPS(am_io_hierarchy_context_tree,
			 struct am_io_hierarchy_context_tree, rb_root,
			 struct am_io_hierarchy_context_tree_node, rb_node,
			 am_hierarchy_node_id_t,
			 IO_HIERARCHY_CONTEXT_TREE_NODE_ACC_ID,
			 IO_HIERARCHY_CONTEXT_TREE_NODE_CMP_IDS)

/* I/O hierarchy context with the currently loaded hierarchies. */
struct am_io_hierarchy_context {
	struct am_io_hierarchy_context_tree_array hierarchies;
};

void am_io_hierarchy_context_init(struct am_io_hierarchy_context* ctx);
void am_io_hierarchy_context_destroy(struct am_io_hierarchy_context* ctx);
int am_io_hierarchy_context_add_hierarchy(struct am_io_hierarchy_context* hc,
					  struct am_hierarchy* h,
					  am_hierarchy_id_t id);


int am_io_hierarchy_context_add_hierarchy_node(struct am_io_hierarchy_context* hc,
					       am_hierarchy_id_t h_id,
					       struct am_hierarchy_node* hn,
					       am_hierarchy_node_id_t hn_id);

/* Retrieve a hierarchy by its hierarchy ID. If no such hierarchy has been
 * encountered, NULL is returned. Otherwise a pointer to the in-memory data
 * structure is returned. */
static inline struct am_hierarchy*
am_io_hierarchy_context_find_hierarchy(struct am_io_hierarchy_context* hc,
				       am_hierarchy_id_t id)
{
	struct am_io_hierarchy_context_tree* t;

	if((t = am_io_hierarchy_context_tree_array_bsearch(&hc->hierarchies, id)))
		return t->hierarchy;

	return NULL;
}

/* Retrieve a hierarchy node by its hierarchy ID and its node ID. If the
 * associated hierarchy hasn't been encountered before or if the node ID is
 * unknown in the set of envountered nodes for the hierarchy, NULL is
 * returned. Otherwise a pointer to the in-memory data structure is returned. */
static inline struct am_hierarchy_node*
am_io_hierarchy_context_find_hierarchy_node(struct am_io_hierarchy_context* hc,
					    am_hierarchy_id_t h_id,
					    am_hierarchy_node_id_t hn_id)
{
	struct am_io_hierarchy_context_tree* t;
	struct am_io_hierarchy_context_tree_node* n;

	if(!(t = am_io_hierarchy_context_tree_array_bsearch(&hc->hierarchies, h_id)))
		return NULL;

	if(!(n = am_io_hierarchy_context_tree_find(t, hn_id)))
		return NULL;

	return n->hierarchy_node;
}

#endif
