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
#include <stdio.h>

enum am_dfg_graph_flags {
	AM_DFG_GRAPH_DESTROY_NODES = (1 << 0),
	AM_DFG_GRAPH_DESTROY_BUFFERS = (1 << 1)
};

/* A simple dataflow graph */
struct am_dfg_graph {
	/* *All nodes included in the graph */
	struct list_head nodes;

	/* All buffers referenced by any node in the graph */
	struct list_head buffers;

	long flags;
};

#define am_dfg_graph_for_each_node(g, n) \
	am_typed_list_for_each(g, nodes, n, list)

#define am_dfg_graph_for_each_node_prev(g, n) \
	am_typed_list_for_each_prev(g, nodes, n, list)

#define am_dfg_graph_for_each_node_safe(g, n, i) \
	am_typed_list_for_each_safe(g, nodes, n, i, list)

#define am_dfg_graph_for_each_node_prev_safe(g, n, i) \
	am_typed_list_for_each_prev_safe(g, nodes, n, i, list)

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
void am_dfg_graph_add_node(struct am_dfg_graph* g, struct am_dfg_node* n);
int am_dfg_graph_remove_node(struct am_dfg_graph* g, struct am_dfg_node* n);
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
			   const struct am_dfg_node* extra_src,
			   struct am_dfg_node* extra_dst,
			   const struct am_dfg_port* ignore_src,
			   const struct am_dfg_port* ignore_dst);

struct am_object_notation_node*
am_dfg_graph_to_object_notation(const struct am_dfg_graph* g);

int am_dfg_graph_save(struct am_dfg_graph* g, const char* filename);
int am_dfg_graph_save_fp(struct am_dfg_graph* g, FILE* fp);

#endif
