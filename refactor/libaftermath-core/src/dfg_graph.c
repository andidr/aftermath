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

#include <aftermath/core/dfg_graph.h>

void am_dfg_graph_init(struct am_dfg_graph* g, long flags)
{
	INIT_LIST_HEAD(&g->nodes);
	INIT_LIST_HEAD(&g->buffers);

	g->flags = flags;
}

/* Reset all buffers used by any node in the graph */
void am_dfg_graph_reset_buffers(const struct am_dfg_graph* g)
{
	struct am_dfg_buffer* b;

	am_dfg_graph_for_each_buffer(g, b)
		am_dfg_buffer_reset(b);
}

/* Destroy a graph. According to the flags buffers and nodes are destroyed /
 * preserved. */
void am_dfg_graph_destroy(struct am_dfg_graph* g)
{
	struct am_dfg_node* n;
	struct am_dfg_node* ntmp;
	struct am_dfg_buffer* b;
	struct am_dfg_buffer* btmp;

	if(g->flags & AM_DFG_GRAPH_DESTROY_NODES) {
		am_dfg_graph_for_each_node_safe(g, n, ntmp) {
			am_dfg_node_destroy(n);
			free(n);
		}
	}

	if(g->flags & AM_DFG_GRAPH_DESTROY_BUFFERS) {
		am_dfg_graph_for_each_buffer_safe(g, b, btmp) {
			am_dfg_buffer_destroy(b);
			free(b);
		}
	}
}

/* Add a node to the graph */
void am_dfg_graph_add_node(struct am_dfg_graph* g, struct am_dfg_node* n)
{
	list_add(&n->list, &g->nodes);
}

/*
 * Remove a node from the graph. This destroys all of the node's connections,
 * but does not destroy or free the node itself.
 */
int am_dfg_graph_remove_node(struct am_dfg_graph* g, struct am_dfg_node* n)
{
	struct am_dfg_port* p;
	struct am_dfg_port* i;
	size_t c;

	am_dfg_node_for_each_port(n, p)
		am_dfg_port_for_each_connected_port_safe(p, i, c)
			if(am_dfg_port_disconnect(p, i))
				return 1;

	list_del(&n->list);

	return 0;
}

/* Add a buffer to the graph */
static void am_dfg_graph_add_buffer(struct am_dfg_graph* g,
				    struct am_dfg_buffer* b)
{
	list_add(&b->list, &g->buffers);
}

/*
 * Connect two ports. This includes a check if the port types and directions are
 * compatible, but does not check for cycles. If the out port did not have a
 * buffer associated to it prior to the connection, a new buffer is allocated.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_graph_connectp(struct am_dfg_graph* g,
			  struct am_dfg_port* src_port,
			  struct am_dfg_port* dst_port)
{
	struct am_dfg_buffer* buffer;
	const struct am_dfg_node_type_functions* src_node_funs;
	const struct am_dfg_node_type_functions* dst_node_funs;

	/* Port directions compatible? */
	if(!(src_port->type->flags & AM_DFG_PORT_OUT) ||
	   !(dst_port->type->flags & AM_DFG_PORT_IN))
	{
		goto out_err;
	}

	/* Port data types compatible? */
	if(src_port->type->type != dst_port->type->type)
		goto out_err;

	/* Destination already connected? */
	if(dst_port->num_connections != 0)
		goto out_err;

	if(am_dfg_port_connect_onesided(src_port, dst_port))
		goto out_err;

	if(am_dfg_port_connect_onesided(dst_port, src_port))
		goto out_err_unc_dst;

	/* Assign buffer or allocate a new one */
	if(src_port->buffer) {
		dst_port->buffer = src_port->buffer;
		am_dfg_buffer_inc_ref(dst_port->buffer);

		dst_node_funs = &dst_port->node->type->functions;

		if(dst_node_funs->connect)
			dst_node_funs->connect(dst_port->node, dst_port);
	} else {
		if(!(buffer = malloc(sizeof(*buffer))))
			goto out_err_unc_src;

		am_dfg_buffer_init(buffer, src_port->type->type);
		am_dfg_graph_add_buffer(g, buffer);

		dst_port->buffer = buffer;
		src_port->buffer = buffer;

		src_node_funs = &src_port->node->type->functions;
		dst_node_funs = &dst_port->node->type->functions;

		if(src_node_funs->connect)
			src_node_funs->connect(src_port->node, src_port);

		if(dst_node_funs->connect)
			dst_node_funs->connect(dst_port->node, dst_port);
	}

	return 0;

out_err_unc_src:
	am_dfg_port_disconnect_onesided(dst_port, src_port);
out_err_unc_dst:
	am_dfg_port_disconnect_onesided(src_port, dst_port);
out_err:
	return 1;
}

/*
 * Disconnects src_port and old_dst_port and reconnects src_port to
 * new_dst_port.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_graph_reconnectp(struct am_dfg_graph* g,
			    struct am_dfg_port* src_port,
			    struct am_dfg_port* old_dst_port,
			    struct am_dfg_port* new_dst_port)
{
	/* Prevent buffer from being freed by the disconnect */
	am_dfg_buffer_inc_ref(src_port->buffer);

	if(am_dfg_port_disconnect(src_port, old_dst_port) ||
	   am_dfg_graph_connectp(g, src_port, new_dst_port))
	{
		am_dfg_buffer_dec_ref(src_port->buffer);
		return 1;
	}

	am_dfg_buffer_dec_ref(src_port->buffer);
	return 0;
}

/*
 * Connect two nodes src and dest using the ports identified by src_port_name
 * and dst_port_name, respectively.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_dfg_graph_connectn(struct am_dfg_graph* g,
			  struct am_dfg_node* src, const char* src_port_name,
			  struct am_dfg_node* dst, const char* dst_port_name)
{
	struct am_dfg_port* src_pi;
	struct am_dfg_port* dst_pi;

	if(!(src_pi = am_dfg_node_find_port(src, src_port_name)))
		return 1;

	if(!(dst_pi = am_dfg_node_find_port(dst, dst_port_name)))
		return 1;

	return am_dfg_graph_connectp(g, src_pi, dst_pi);
}

enum {
	AM_DFG_CYCLE_UNMARKED = 0,
	AM_DFG_CYCLE_MARKED = 1,
	AM_DFG_CYCLE_VISITED = 2
};

/*
 * Checks if the subgraph rooted by n has a cycle. A hypothetical edge
 * (extra_src, extra_dst) is included in the check and any connection between
 * the ports ignore_src and ignore_dst is not taken into account. Extra_src,
 * extra_dst, ignore_src and ignore_dst may be set to NULL to check only
 * existing edges.
 *
 * Returns true if at least one cycle is found, otherwise 0.
 */
static int am_dfg_graph_has_cycle_rec(struct am_dfg_node* n,
				      const struct am_dfg_node* extra_src,
				      struct am_dfg_node* extra_dst,
				      const struct am_dfg_port* ignore_src,
				      const struct am_dfg_port* ignore_dst)
{
	struct am_dfg_node* d;
	struct am_dfg_port* p;

	/* Already checked, prune this subgraph */
	if(n->marking == AM_DFG_CYCLE_VISITED)
		return 0;

	/* All nodes marked, but not visited are on the current path, so this
	 * must be a cycle */
	if(n->marking == AM_DFG_CYCLE_MARKED)
		return 1;

	n->marking = AM_DFG_CYCLE_MARKED;

	/* Evaluate extra edge */
	if(n == extra_src) {
		if(am_dfg_graph_has_cycle_rec(extra_dst,
					      extra_src, extra_dst,
					      ignore_src, ignore_dst))
		{
			return 1;
		}
	}

	/* Check all actual, outgoing edges of the current node */
	for(size_t i = 0; i < n->type->num_ports; i++) {
		if(n->type->ports[i].flags & AM_DFG_PORT_OUT) {
			p = &n->ports[i];

			for(size_t j = 0; j < p->num_connections; j++) {
				d = p->connections[j]->node;

				/* Edge ignored? */
				if(p == ignore_src &&
				   p->connections[j] == ignore_dst)
				{
					continue;
				}

				if(am_dfg_graph_has_cycle_rec(d,
							      extra_src,
							      extra_dst,
							      ignore_src,
							      ignore_dst))
				{
					return 1;
				}
			}
		}
	}

	n->marking = AM_DFG_CYCLE_VISITED;

	return 0;
}

/*
 * Checks if the graph has a cycle. A hypothetical edge (extra_src, extra_dst)
 * is included in the check and any connection between the ports ignore_src and
 * ignore_dst is not taken into account. Extra_src, extra_dst, ignore_src and
 * ignore_dst may be set to NULL to check only existing edges.
 *
 * Returns true if at least one cycle is found, otherwise 0.
 */
int am_dfg_graph_has_cycle(const struct am_dfg_graph* g,
			   const struct am_dfg_node* extra_src,
			   struct am_dfg_node* extra_dst,
			   const struct am_dfg_port* ignore_src,
			   const struct am_dfg_port* ignore_dst)
{
	struct am_dfg_node* n;

	am_dfg_graph_for_each_node(g, n)
		n->marking = AM_DFG_CYCLE_UNMARKED;

	/* Check all roots */
	am_dfg_graph_for_each_node(g, n) {
		if(am_dfg_node_is_root_ign(n, ignore_src, ignore_dst)) {
			if(am_dfg_graph_has_cycle_rec(n, extra_src, extra_dst,
						      ignore_src, ignore_dst))
			{
				return 1;
			}
		}
	}

	/* Unmarked nodes belong to cycles that do not have any entry point. The
	 * presence of any such node indicates that there is a cycle. */
	am_dfg_graph_for_each_node(g, n)
		if(n->marking == AM_DFG_CYCLE_UNMARKED)
			return 1;

	return 0;
}
