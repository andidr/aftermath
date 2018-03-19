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

#ifndef AM_DFG_NODE_TRACE_H
#define AM_DFG_NODE_TRACE_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/core/in_memory.h>

struct am_dfg_node_trace {
	struct am_dfg_node n;
	struct am_trace* trace;
};

int am_dfg_trace_node_init(struct am_dfg_node* n);
int am_dfg_trace_node_process(struct am_dfg_node* n);

AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_trace_node_type,
	"trace",
	"Trace",
	sizeof(struct am_dfg_node_trace),
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
			.init = am_dfg_trace_node_init,
			.process = am_dfg_trace_node_process
	}),
	AM_DFG_NODE_PORTS(
		{ "trace", "trace", AM_DFG_PORT_OUT }),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_trace_node_type)

#endif
