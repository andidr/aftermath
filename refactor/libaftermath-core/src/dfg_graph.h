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

#ifndef AM_DFG_GRAPH_H
#define AM_DFG_GRAPH_H

#include <aftermath/core/dfg_type.h>
#include <aftermath/core/dfg_buffer.h>
#include <aftermath/core/dfg_node.h>
#include <aftermath/core/typed_list.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/typed_rbtree.h>
#include <aftermath/core/dfg_node_type_registry.h>
#include <stdio.h>

enum am_dfg_graph_flags {
	AM_DFG_GRAPH_DESTROY_NODES = (1 << 0),
	AM_DFG_GRAPH_DESTROY_BUFFERS = (1 << 1)
};

#define AM_DFG_GRAPH_DESTROY_ALL \
	(AM_DFG_GRAPH_DESTROY_NODES | AM_DFG_GRAPH_DESTROY_BUFFERS)

/* Root of the red-black tree sorted by node IDs */
struct am_dfg_node_idtree {
	struct rb_root rb_root;
};

/* A simple dataflow graph */
struct am_dfg_graph {
	struct am_dfg_node_idtree id_tree;

	/* All buffers referenced by any node in the graph */
	struct list_head buffers;

	long flags;
};

/* A path is just an array of connections */
AM_DECL_TYPED_ARRAY(am_dfg_path, struct am_dfg_connection)

#define am_dfg_path_for_each_connection(path, connection)		\
	for((connection) = &(path)->elements[0];			\
	    (connection) != &(path)->elements[(path)->num_elements];	\
	    (connection)++)

/* Appends a connection (src, dst) to the path. Returns 0 on success, otherwise
 * a value different from 0. */
static inline int am_dfg_path_append_leg(struct am_dfg_path* path,
					 const struct am_dfg_port* src,
					 const struct am_dfg_port* dst)
{
	struct am_dfg_connection c;

	c.src = (struct am_dfg_port*)src;
	c.dst = (struct am_dfg_port*)dst;

	return am_dfg_path_appendp(path, &c);
}

#define AM_DFG_NODE_ACC_ID(x) ((x).id)

AM_DECL_TYPED_RBTREE_OPS(am_dfg_node_idtree,
			 struct am_dfg_node_idtree, rb_root,
			 struct am_dfg_node, rb_node,
			 long,
			 AM_DFG_NODE_ACC_ID)

#define am_dfg_graph_for_each_node(g, n)		 \
	for(n = am_dfg_node_idtree_first(&(g)->id_tree); \
	    n;						 \
	    n = am_dfg_node_idtree_next(n))

#define am_dfg_graph_for_each_node_prev(g, n)		 \
	for(n = am_dfg_node_idtree_last(&(g)->id_tree);  \
	    n;						 \
	    n = am_dfg_node_idtree_prev(n))

/* Safe traversal of the graph. That is, the current node n can be safely
 * freed. However, n must not be removed from the graph, otherwise the graph
 * gets rebalanced, and not all of the remaining nodes are visited. Use
 * am_dfg_graph_for_each_remove_node_safe instead.
 */
#define am_dfg_graph_for_each_node_safe(g, n, i)	\
	rbtree_postorder_for_each_entry_safe(		\
		n, i, &(g)->id_tree.rb_root, rb_node)

/* Iterate over each node of the graph. The current node at each iteration must
 * be removed by the loop body. */
#define am_dfg_graph_for_each_remove_node_safe(g, n)		\
	for(n = am_dfg_node_idtree_first(&(g)->id_tree);	\
	    n;							\
	    n = am_dfg_node_idtree_first(&(g)->id_tree))

#define am_dfg_graph_for_each_buffer(g, b) \
	am_typed_list_for_each(g, buffers, b, list)

#define am_dfg_graph_for_each_buffer_prev(g, b) \
	am_typed_list_for_each_prev(g, buffers, b, list)

#define am_dfg_graph_for_each_buffer_safe(g, b, i) \
	am_typed_list_for_each_safe(g, buffers, b, i, list)

#define am_dfg_graph_for_each_buffer_prev_safe(g, b, i) \
	am_typed_list_for_each_prev_safe(g, buffers, b, i, list)

void am_dfg_graph_init(struct am_dfg_graph* g, long flags);
void am_dfg_graph_destroy(struct am_dfg_graph* g);
int am_dfg_graph_add_node(struct am_dfg_graph* g, struct am_dfg_node* n);
struct am_dfg_node*
am_dfg_graph_find_node(const struct am_dfg_graph* g, long id);
int am_dfg_graph_remove_node(struct am_dfg_graph* g, struct am_dfg_node* n);
void am_dfg_graph_remove_node_no_disconnect(struct am_dfg_graph* g,
					    struct am_dfg_node* n);
int am_dfg_graph_connectp(struct am_dfg_graph* g,
			  struct am_dfg_port* src_port,
			  struct am_dfg_port* dst_port);
int am_dfg_graph_reconnectp(struct am_dfg_graph* g,
			    struct am_dfg_port* src_port,
			    struct am_dfg_port* old_dst_port,
			    struct am_dfg_port* new_dst_port);
int am_dfg_graph_connectn(struct am_dfg_graph* g,
			  struct am_dfg_node* src, const char* src_port_name,
			  struct am_dfg_node* dst, const char* dst_port_name);
void am_dfg_graph_reset_buffers(const struct am_dfg_graph* g);
int am_dfg_graph_has_cycle(const struct am_dfg_graph* g,
			   const struct am_dfg_port* extra_src,
			   const struct am_dfg_port* extra_dst,
			   const struct am_dfg_port* ignore_src,
			   const struct am_dfg_port* ignore_dst);

int am_dfg_graph_merge(struct am_dfg_graph* dst, struct am_dfg_graph* g);
struct am_dfg_node* am_dfg_graph_lowest_id_node(const struct am_dfg_graph* g);
struct am_dfg_node* am_dfg_graph_highest_id_node(const struct am_dfg_graph* g);

struct am_object_notation_node*
am_dfg_graph_to_object_notation(const struct am_dfg_graph* g);

int am_dfg_graph_save(struct am_dfg_graph* g, const char* filename);
int am_dfg_graph_save_fp(struct am_dfg_graph* g, FILE* fp);

int am_dfg_graph_load(struct am_dfg_graph* g,
		      const char* filename,
		      struct am_dfg_node_type_registry* ntr);
int am_dfg_graph_from_object_notation(struct am_dfg_graph* g,
				      struct am_object_notation_node* n_graph,
				      struct am_dfg_node_type_registry* ntr);


#endif
