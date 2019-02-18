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

#ifndef AM_DFG_SELECT_NTH_NODE_TYPE_H
#define AM_DFG_SELECT_NTH_NODE_TYPE_H

#include <aftermath/core/dfg_node.h>

struct am_dfg_select_nth_node {
	struct am_dfg_node super;
	const struct am_dfg_type* current_type;
	const struct am_dfg_type* any_type;
	size_t num_connections;
	int64_t N;
	int fail_if_no_input;
};

int am_dfg_select_nth_node_init(struct am_dfg_node* n);
int am_dfg_select_nth_node_process(struct am_dfg_node* n);
void am_dfg_select_nth_node_connect(struct am_dfg_node* n, struct am_dfg_port* pi);
void am_dfg_select_nth_node_disconnect(struct am_dfg_node* n, struct am_dfg_port* pi);

int am_dfg_select_nth_node_pre_connect(
	const struct am_dfg_node* n,
	const struct am_dfg_port* pi,
	const struct am_dfg_port* pother,
	size_t max_error_msg,
	char* error_msg);

int am_dfg_select_nth_node_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);

int am_dfg_select_nth_node_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);

int am_dfg_select_nth_node_set_property(
	struct am_dfg_node* n,
	const struct am_dfg_property* property,
	const void* value);

int am_dfg_select_nth_node_get_property(
	const struct am_dfg_node* n,
	const struct am_dfg_property* property,
	void** value);

/**
 * Node that selects the N-th sample from an input port. If N is negative, the
 * selected index is relative to the last index.
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_select_nth_node_type,
	"am::core::select_nth",
	"Select N-th",
	sizeof(struct am_dfg_select_nth_node),
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
		.init = am_dfg_select_nth_node_init,
		.process = am_dfg_select_nth_node_process,
		.pre_connect = am_dfg_select_nth_node_pre_connect,
		.connect = am_dfg_select_nth_node_connect,
		.disconnect = am_dfg_select_nth_node_disconnect,
		.from_object_notation = am_dfg_select_nth_node_from_object_notation,
		.to_object_notation = am_dfg_select_nth_node_to_object_notation,
		.set_property = am_dfg_select_nth_node_set_property,
		.get_property = am_dfg_select_nth_node_get_property
	}),
	AM_DFG_NODE_PORTS(
		{ "in", "am::core::any", AM_DFG_PORT_IN },
		{ "out", "am::core::any", AM_DFG_PORT_OUT }),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES(
		{ "N", "N", "am::core::int64" },
		{ "fail_if_no_input", "Fail if no input", "am::core::bool" }))

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_select_nth_node_type)

#endif
