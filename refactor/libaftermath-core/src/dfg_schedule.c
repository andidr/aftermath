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

#include <aftermath/core/dfg_schedule.h>

static int am_dfg_schedule_process_node_rec(struct am_dfg_node* n);

/*
 * For all outgoing edges of a node n, decrease the number of outstanding
 * dependences of the reader and schedule the reader if all dependencies have
 * been satisfied.
 *
 * Returns 0 on success, otherwise 1.
 */
static int am_dfg_schedule_process_readers(struct am_dfg_node* n)
{
	struct am_dfg_node* reader;
	struct am_dfg_port* inp;
	size_t i, j;

	for(i = 0; i < n->type->num_ports; i++) {
		if(n->type->ports[i].flags & AM_DFG_PORT_OUT) {
			am_dfg_port_for_each_connected_port_safe(&n->ports[i],
								 inp, j)
			{
				reader = inp->node;

				if(--reader->num_deps_remaining == 0 &&
				   !reader->marking)
					if(am_dfg_schedule_process_node_rec(reader))
						return 1;
			}
		}
	}

	return 0;
}

/*
 * Recursively schedule the graph rooted at the node n. Returns 0 on success, 1
 * otherwise.
 */
static int am_dfg_schedule_process_node_rec(struct am_dfg_node* n)
{
	if(n->type->functions.process)
		if(n->type->functions.process(n))
			return 1;

	n->marking = 1;

	if(am_dfg_schedule_process_readers(n))
		return 1;

	return 0;
}

/*
 * Schedule an entire, cycle-free graph. This scheduler checks if all mandatory
 * ports are connected. The caller must have made sure that there are no cycles
 * as the function but does not carry perform this test.
 *
 * Returns 0 on success (i.e., if all nodes have been executed without errors),
 * otherwise 1.
 */
int am_dfg_schedule(const struct am_dfg_graph* g)
{
	struct am_dfg_node* n;

	/* Check if all mandatory ports are connected */
	am_dfg_graph_for_each_node(g, n) {
		if(!am_dfg_node_is_well_connected(n))
			return 1;

		am_dfg_node_reset_sched_data(n);
		n->marking = 0;
	}

	/* Delete data from previous runs */
	am_dfg_graph_reset_buffers(g);

	/* Schedule all roots */
	am_dfg_graph_for_each_node(g, n) {
		if(n->num_deps_remaining == 0 && !n->marking) {
			if(am_dfg_schedule_process_node_rec(n)) {
				am_dfg_graph_reset_buffers(g);
				return 1;
			}
		}
	}

	am_dfg_graph_reset_buffers(g);

	return 0;
}
