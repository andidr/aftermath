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

#ifndef AM_DFG_NODE_MATH_H
#define AM_DFG_NODE_MATH_H

#include <aftermath/core/dfg_node.h>

#define AM_DFG_ADDSUB_NODE_DECL(FUN, FUN_CAP, TPREFIX, TPREFIX_CAP)		\
	int am_dfg_##TPREFIX##_##FUN##_node_process(struct am_dfg_node* n);	\
										\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_##TPREFIX##_##FUN##_node_type,				\
		"am::core::arithmetic::" #TPREFIX "::" #FUN,			\
		#TPREFIX_CAP " " #FUN_CAP,					\
		AM_DFG_NODE_DEFAULT_SIZE,					\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,			\
		AM_DFG_NODE_FUNCTIONS({					\
			.process = am_dfg_##TPREFIX##_##FUN##_node_process,	\
		}),								\
		AM_DFG_NODE_PORTS(						\
			{ "in", "am::core::" #TPREFIX, AM_DFG_PORT_IN },	\
			{ "out", "am::core::" #TPREFIX, AM_DFG_PORT_OUT },	\
		),								\
		AM_DFG_PORT_DEPS(),						\
		AM_DFG_NODE_PROPERTIES())

AM_DFG_ADDSUB_NODE_DECL(add, Addition,  uint8,  Uint8)
AM_DFG_ADDSUB_NODE_DECL(add, Addition, uint16, Uint16)
AM_DFG_ADDSUB_NODE_DECL(add, Addition, uint32, Uint32)
AM_DFG_ADDSUB_NODE_DECL(add, Addition, uint64, Uint64)

AM_DFG_ADDSUB_NODE_DECL(add, Addition, timestamp, Timestamp)
AM_DFG_ADDSUB_NODE_DECL(add, Addition, double, Double)

AM_DFG_ADDSUB_NODE_DECL(add, Addition,  int8,  Int8)
AM_DFG_ADDSUB_NODE_DECL(add, Addition, int16, Int16)
AM_DFG_ADDSUB_NODE_DECL(add, Addition, int32, Int32)
AM_DFG_ADDSUB_NODE_DECL(add, Addition, int64, Int64)

AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction,  uint8,  Uint8)
AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction, uint16, Uint16)
AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction, uint32, Uint32)
AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction, uint64, Uint64)

AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction, timestamp, Timestamp)
AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction, double, Double)

AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction,  int8,  Int8)
AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction, int16, Int16)
AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction, int32, Int32)
AM_DFG_ADDSUB_NODE_DECL(sub, Subtraction, int64, Int64)

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_dfg_uint8_add_node_type,
	&am_dfg_uint16_add_node_type,
	&am_dfg_uint32_add_node_type,
	&am_dfg_uint64_add_node_type,
	&am_dfg_timestamp_add_node_type,
	&am_dfg_double_add_node_type,
	&am_dfg_int8_add_node_type,
	&am_dfg_int16_add_node_type,
	&am_dfg_int32_add_node_type,
	&am_dfg_int64_add_node_type,

	&am_dfg_uint8_sub_node_type,
	&am_dfg_uint16_sub_node_type,
	&am_dfg_uint32_sub_node_type,
	&am_dfg_uint64_sub_node_type,
	&am_dfg_timestamp_sub_node_type,
	&am_dfg_double_sub_node_type,
	&am_dfg_int8_sub_node_type,
	&am_dfg_int16_sub_node_type,
	&am_dfg_int32_sub_node_type,
	&am_dfg_int64_sub_node_type)

#endif
