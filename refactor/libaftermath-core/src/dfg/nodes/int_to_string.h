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

#ifndef AM_DFG_NODE_INT_TO_STRING_H
#define AM_DFG_NODE_INT_TO_STRING_H

#include <aftermath/core/dfg_node.h>

#define AM_DECL_INT_TO_STRING_NODE_TYPE(SUFFIX, SUFFIX_CAP)			\
	int am_dfg_##SUFFIX##_to_string_node_process(struct am_dfg_node* n);	\
										\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_##SUFFIX##_to_string_node_type,				\
		"am::core::" #SUFFIX "::to_string",				\
		#SUFFIX_CAP " -> String",					\
		AM_DFG_NODE_DEFAULT_SIZE,					\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,			\
		AM_DFG_NODE_FUNCTIONS({					\
			.process = am_dfg_##SUFFIX##_to_string_node_process	\
		}),								\
		AM_DFG_NODE_PORTS(						\
			{ "in", "am::core::" #SUFFIX, AM_DFG_PORT_IN },	\
			{ "out", "am::core::string", AM_DFG_PORT_OUT }),	\
		AM_DFG_PORT_DEPS(),						\
		AM_DFG_NODE_PROPERTIES())

AM_DECL_INT_TO_STRING_NODE_TYPE(uint8,  Uint8)
AM_DECL_INT_TO_STRING_NODE_TYPE(uint16, Uint16)
AM_DECL_INT_TO_STRING_NODE_TYPE(uint32, Uint32)
AM_DECL_INT_TO_STRING_NODE_TYPE(uint64, Uint64)

AM_DECL_INT_TO_STRING_NODE_TYPE(int8,  int8)
AM_DECL_INT_TO_STRING_NODE_TYPE(int16, int16)
AM_DECL_INT_TO_STRING_NODE_TYPE(int32, int32)
AM_DECL_INT_TO_STRING_NODE_TYPE(int64, int64)

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_dfg_uint8_to_string_node_type,
	&am_dfg_uint16_to_string_node_type,
	&am_dfg_uint32_to_string_node_type,
	&am_dfg_uint64_to_string_node_type,

	&am_dfg_int8_to_string_node_type,
	&am_dfg_int16_to_string_node_type,
	&am_dfg_int32_to_string_node_type,
	&am_dfg_int64_to_string_node_type)

#endif
