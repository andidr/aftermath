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

#ifndef AM_DFG_NODE_TYPE_MERGE_H
#define AM_DFG_NODE_TYPE_MERGE_H

#include <aftermath/core/dfg_node.h>

struct am_dfg_merge_node {
	struct am_dfg_node super;
	const struct am_dfg_type* current_type;
	const struct am_dfg_type* any_type;
	size_t num_connections;
};

int am_dfg_merge_node_init(struct am_dfg_node* n);
int am_dfg_merge_node_process(struct am_dfg_node* n);
void am_dfg_merge_node_connect(struct am_dfg_node* n, struct am_dfg_port* pi);
void am_dfg_merge_node_disconnect(struct am_dfg_node* n, struct am_dfg_port* pi);

int am_dfg_merge_node_pre_connect(
	const struct am_dfg_node* n,
	const struct am_dfg_port* pi,
	const struct am_dfg_port* pother,
	size_t max_error_msg,
	char* error_msg);

/**
 * Node that copies the samples from n input ports to one output port
 */

#define AM_DECL_MERGE_NODE_TYPE(N, ...)				\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(					\
		am_dfg_merge##N##_node_type,				\
		"am::core::merge" #N,					\
		"Merge " #N " Inputs",					\
		sizeof(struct am_dfg_merge_node),			\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,		\
		AM_DFG_NODE_FUNCTIONS({				\
			.init = am_dfg_merge_node_init,		\
			.process = am_dfg_merge_node_process,		\
			.pre_connect = am_dfg_merge_node_pre_connect,	\
			.connect = am_dfg_merge_node_connect,		\
			.disconnect = am_dfg_merge_node_disconnect	\
		}),							\
		AM_DFG_NODE_PORTS(					\
			__VA_ARGS__,					\
			{ "out", "am::core::any", AM_DFG_PORT_OUT }), \
		AM_DFG_PORT_DEPS(),					\
		AM_DFG_NODE_PROPERTIES())


AM_DECL_MERGE_NODE_TYPE(
	2,
	{ "in0", "am::core::any", AM_DFG_PORT_IN },
	{ "in1", "am::core::any", AM_DFG_PORT_IN })

AM_DECL_MERGE_NODE_TYPE(
	3,
	{ "in0", "am::core::any", AM_DFG_PORT_IN },
	{ "in1", "am::core::any", AM_DFG_PORT_IN },
	{ "in2", "am::core::any", AM_DFG_PORT_IN })

AM_DECL_MERGE_NODE_TYPE(
	4,
	{ "in0", "am::core::any", AM_DFG_PORT_IN },
	{ "in1", "am::core::any", AM_DFG_PORT_IN },
	{ "in2", "am::core::any", AM_DFG_PORT_IN },
	{ "in3", "am::core::any", AM_DFG_PORT_IN })

AM_DECL_MERGE_NODE_TYPE(
	8,
	{ "in0", "am::core::any", AM_DFG_PORT_IN },
	{ "in1", "am::core::any", AM_DFG_PORT_IN },
	{ "in2", "am::core::any", AM_DFG_PORT_IN },
	{ "in3", "am::core::any", AM_DFG_PORT_IN },
	{ "in4", "am::core::any", AM_DFG_PORT_IN },
	{ "in5", "am::core::any", AM_DFG_PORT_IN },
	{ "in6", "am::core::any", AM_DFG_PORT_IN },
	{ "in7", "am::core::any", AM_DFG_PORT_IN })

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_dfg_merge2_node_type,
	&am_dfg_merge3_node_type,
	&am_dfg_merge4_node_type,
	&am_dfg_merge8_node_type)

#endif
