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

#ifndef AM_DFG_NODE_TYPE_CONDITIONAL_FORWARD_ELEMENTWISE_H
#define AM_DFG_NODE_TYPE_CONDITIONAL_FORWARD_ELEMENTWISE_H

#include <aftermath/core/dfg/nodes/conditional_forward.h>

int am_dfg_conditional_forward_elementwise_node_process(struct am_dfg_node* n);

AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_conditional_forward_elementwise_node_type,
	"am::core::filter::conditional_forward::elementwise",
	"Conditional Forward (elementwise)",
	sizeof(struct am_dfg_conditional_forward_node),
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
		.init = am_dfg_conditional_forward_node_init,
		.process = am_dfg_conditional_forward_elementwise_node_process,
		.pre_connect = am_dfg_conditional_forward_node_pre_connect,
		.connect = am_dfg_conditional_forward_node_connect,
		.disconnect = am_dfg_conditional_forward_node_disconnect
	}),
	AM_DFG_NODE_PORTS(
		{ "forward if true", "am::core::any", AM_DFG_PORT_IN },
		{ "control", "am::core::bool", AM_DFG_PORT_IN },
		{ "out", "am::core::any", AM_DFG_PORT_OUT }),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_conditional_forward_elementwise_node_type)

#endif
