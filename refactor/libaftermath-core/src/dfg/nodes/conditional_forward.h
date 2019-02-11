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

#ifndef AM_DFG_NODE_TYPE_CONDITIONAL_FORWARD_H
#define AM_DFG_NODE_TYPE_CONDITIONAL_FORWARD_H

#include <aftermath/core/dfg_node.h>

struct am_dfg_conditional_forward_node {
	struct am_dfg_node super;
	const struct am_dfg_type* current_type;
	const struct am_dfg_type* any_type;
	size_t num_connections;
};

int am_dfg_conditional_forward_node_process(struct am_dfg_node* n);
int am_dfg_conditional_forward_node_init(struct am_dfg_node* n);
void am_dfg_conditional_forward_node_connect(
	struct am_dfg_node* n, struct am_dfg_port* pi);
void am_dfg_conditional_forward_node_disconnect(
	struct am_dfg_node* n, struct am_dfg_port* pi);
int am_dfg_conditional_forward_node_pre_connect(
	const struct am_dfg_node* n,
	const struct am_dfg_port* pi,
	const struct am_dfg_port* pother,
	size_t max_error_msg,
	char* error_msg);

#endif
