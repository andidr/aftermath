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

#ifndef AM_DFG_NODE_TYPE_INT_CONSTANT_H
#define AM_DFG_NODE_TYPE_INT_CONSTANT_H

#include <aftermath/core/dfg_node.h>

#define AM_DFG_INT_CONSTANT_NODE_DECL(TPREFIX, TPREFIX_CAP)			\
	struct am_dfg_##TPREFIX##_constant_node {				\
		struct am_dfg_node n;						\
		TPREFIX##_t value;						\
		size_t num_samples;						\
		uint64_t num_samples64;					\
	};									\
										\
	int am_dfg_##TPREFIX##_constant_node_init(struct am_dfg_node* n);	\
	int am_dfg_##TPREFIX##_constant_node_process(struct am_dfg_node* n);	\
	int am_dfg_##TPREFIX##_constant_node_set_property(			\
		struct am_dfg_node* n,						\
		const struct am_dfg_property* property,			\
		const void* value);						\
	int am_dfg_##TPREFIX##_constant_node_get_property(			\
		const struct am_dfg_node* n,					\
		const struct am_dfg_property* property,			\
		void** value);							\
	int am_dfg_##TPREFIX##_constant_node_from_object_notation(		\
		struct am_dfg_node* n,						\
		struct am_object_notation_node_group* g);			\
	int am_dfg_##TPREFIX##_constant_node_to_object_notation(		\
		struct am_dfg_node* n,						\
		struct am_object_notation_node_group* g);			\
										\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_##TPREFIX##_constant_node_type,				\
		"am::core::" #TPREFIX "_constant",				\
		#TPREFIX_CAP " Constant",					\
		sizeof(struct am_dfg_##TPREFIX##_constant_node),		\
		AM_DFG_DEFAULT_PORT_DEPS_NONE,					\
		AM_DFG_NODE_FUNCTIONS({					\
			.init = am_dfg_##TPREFIX##_constant_node_init,		\
			.process = am_dfg_##TPREFIX##_constant_node_process,	\
			.set_property = am_dfg_##TPREFIX##_constant_node_set_property, \
			.get_property = am_dfg_##TPREFIX##_constant_node_get_property, \
			.from_object_notation = am_dfg_##TPREFIX##_constant_node_from_object_notation, \
			.to_object_notation = am_dfg_##TPREFIX##_constant_node_to_object_notation \
		}),								\
		AM_DFG_NODE_PORTS({ "out", "am::core::" #TPREFIX, AM_DFG_PORT_OUT }), \
		AM_DFG_PORT_DEPS(),						\
		AM_DFG_NODE_PROPERTIES(					\
			{ "value", "Value", "am::core::" #TPREFIX },		\
			{ "num_samples", "Num samples", "am::core::uint64" },	\
		)								\
	)

AM_DFG_INT_CONSTANT_NODE_DECL(uint8,  Uint8)
AM_DFG_INT_CONSTANT_NODE_DECL(uint16, Uint16)
AM_DFG_INT_CONSTANT_NODE_DECL(uint32, Uint32)
AM_DFG_INT_CONSTANT_NODE_DECL(uint64, Uint64)

AM_DFG_INT_CONSTANT_NODE_DECL(int8,  Int8)
AM_DFG_INT_CONSTANT_NODE_DECL(int16, Int16)
AM_DFG_INT_CONSTANT_NODE_DECL(int32, Int32)
AM_DFG_INT_CONSTANT_NODE_DECL(int64, Int64)


AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_dfg_int8_constant_node_type,
	&am_dfg_int16_constant_node_type,
	&am_dfg_int32_constant_node_type,
	&am_dfg_int64_constant_node_type,

	&am_dfg_uint8_constant_node_type,
	&am_dfg_uint16_constant_node_type,
	&am_dfg_uint32_constant_node_type,
	&am_dfg_uint64_constant_node_type)

#endif
