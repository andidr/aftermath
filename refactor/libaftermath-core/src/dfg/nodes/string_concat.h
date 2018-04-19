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

#ifndef AM_DFG_NODE_STRING_CONCAT_H
#define AM_DFG_NODE_STRING_CONCAT_H

#include <aftermath/core/dfg_node.h>

struct am_dfg_string_concat_node {
	struct am_dfg_node node;

	/* Separator string inserted between the input strings */
	char* separator;

	/* strlen(separator) */
	size_t separator_len;
};

int am_dfg_string_concat_node_init(struct am_dfg_node* n);
void am_dfg_string_concat_node_destroy(struct am_dfg_node* n);
int am_dfg_string_concat_node_process(struct am_dfg_node* n);

int am_dfg_string_concat_node_set_property(
	struct am_dfg_node* n,
	const struct am_dfg_property* property,
	const void* value);

int am_dfg_string_concat_node_get_property(
	const struct am_dfg_node* n,
	const struct am_dfg_property* property,
	void** value);

int am_dfg_string_concat_node_from_object_notation(
	struct am_dfg_node* n, struct am_object_notation_node_group* g);

int am_dfg_string_concat_node_to_object_notation(
	struct am_dfg_node* n, struct am_object_notation_node_group* g);
/**
 * Node that converts timestamps into their string representations
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_string_concat_node_type,
	"am::core::string_concat",
	"String Concatenator",
	sizeof(struct am_dfg_string_concat_node),
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
		.init = am_dfg_string_concat_node_init,
		.destroy = am_dfg_string_concat_node_destroy,
		.process = am_dfg_string_concat_node_process,
		.set_property = am_dfg_string_concat_node_set_property,
		.get_property = am_dfg_string_concat_node_get_property,
		.from_object_notation = am_dfg_string_concat_node_from_object_notation,
		.to_object_notation = am_dfg_string_concat_node_to_object_notation,
	}),
	AM_DFG_NODE_PORTS(
		{ "in", "am::core::string",
				AM_DFG_PORT_IN | AM_DFG_PORT_MANDATORY },
		{ "out", "am::core::string",
				AM_DFG_PORT_OUT | AM_DFG_PORT_MANDATORY }),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES(
		{ "separator", "Separator", "am::core::string" }))

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_string_concat_node_type)

#endif
