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

#ifndef AM_BASIC_STATISTICS_H
#define AM_BASIC_STATISTICS_H

#include <aftermath/core/dfg_node.h>

#define AM_DFG_MINMAX_NODE_DECL(FUN, FUN_CAP, TPREFIX, TPREFIX_CAP)		\
	int am_dfg_##TPREFIX##_##FUN##_node_process(struct am_dfg_node* n);	\
										\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_##TPREFIX##_##FUN##_node_type,				\
		"am::core::statistics::" #TPREFIX "::" #FUN,			\
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

AM_DFG_MINMAX_NODE_DECL(max, Maximum,  uint8,  Uint8)
AM_DFG_MINMAX_NODE_DECL(max, Maximum, uint16, Uint16)
AM_DFG_MINMAX_NODE_DECL(max, Maximum, uint32, Uint32)
AM_DFG_MINMAX_NODE_DECL(max, Maximum, uint64, Uint64)

AM_DFG_MINMAX_NODE_DECL(max, Maximum, timestamp, Timestamp)
AM_DFG_MINMAX_NODE_DECL(max, Maximum, double, Double)

AM_DFG_MINMAX_NODE_DECL(max, Maximum,  int8,  Int8)
AM_DFG_MINMAX_NODE_DECL(max, Maximum, int16, Int16)
AM_DFG_MINMAX_NODE_DECL(max, Maximum, int32, Int32)
AM_DFG_MINMAX_NODE_DECL(max, Maximum, int64, Int64)

AM_DFG_MINMAX_NODE_DECL(min, Minimum,  uint8,  Uint8)
AM_DFG_MINMAX_NODE_DECL(min, Minimum, uint16, Uint16)
AM_DFG_MINMAX_NODE_DECL(min, Minimum, uint32, Uint32)
AM_DFG_MINMAX_NODE_DECL(min, Minimum, uint64, Uint64)

AM_DFG_MINMAX_NODE_DECL(min, Minimum, timestamp, Timestamp)
AM_DFG_MINMAX_NODE_DECL(min, Minimum, double, Double)

AM_DFG_MINMAX_NODE_DECL(min, Minimum,  int8,  Int8)
AM_DFG_MINMAX_NODE_DECL(min, Minimum, int16, Int16)
AM_DFG_MINMAX_NODE_DECL(min, Minimum, int32, Int32)
AM_DFG_MINMAX_NODE_DECL(min, Minimum, int64, Int64)

#define AM_DFG_AVG_NODE_DECL(TPREFIX, TPREFIX_CAP)				\
	int am_dfg_##TPREFIX##_avg_node_process(struct am_dfg_node* n);	\
										\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_##TPREFIX##_avg_node_type,				\
		"am::core::statistics::" #TPREFIX "::average",			\
		#TPREFIX_CAP " Average",					\
		AM_DFG_NODE_DEFAULT_SIZE,					\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,			\
		AM_DFG_NODE_FUNCTIONS({					\
			.process = am_dfg_##TPREFIX##_avg_node_process,	\
		}),								\
		AM_DFG_NODE_PORTS(						\
			{ "in", "am::core::" #TPREFIX, AM_DFG_PORT_IN },	\
			{ "out", "am::core::" #TPREFIX, AM_DFG_PORT_OUT },	\
		),								\
		AM_DFG_PORT_DEPS(),						\
		AM_DFG_NODE_PROPERTIES())

AM_DFG_AVG_NODE_DECL( int8,  Int8)
AM_DFG_AVG_NODE_DECL(int16, Int16)
AM_DFG_AVG_NODE_DECL(int32, Int32)
AM_DFG_AVG_NODE_DECL(int64, Int64)

AM_DFG_AVG_NODE_DECL( uint8,  Uint8)
AM_DFG_AVG_NODE_DECL(uint16, Uint16)
AM_DFG_AVG_NODE_DECL(uint32, Uint32)
AM_DFG_AVG_NODE_DECL(uint64, Uint64)

AM_DFG_AVG_NODE_DECL(timestamp, Timestamp)
AM_DFG_AVG_NODE_DECL(double, Double)

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_dfg_uint8_min_node_type,
	&am_dfg_uint16_min_node_type,
	&am_dfg_uint32_min_node_type,
	&am_dfg_uint64_min_node_type,
	&am_dfg_timestamp_min_node_type,
	&am_dfg_double_min_node_type,
	&am_dfg_int8_min_node_type,
	&am_dfg_int16_min_node_type,
	&am_dfg_int32_min_node_type,
	&am_dfg_int64_min_node_type,

	&am_dfg_uint8_max_node_type,
	&am_dfg_uint16_max_node_type,
	&am_dfg_uint32_max_node_type,
	&am_dfg_uint64_max_node_type,
	&am_dfg_timestamp_max_node_type,
	&am_dfg_double_max_node_type,
	&am_dfg_int8_max_node_type,
	&am_dfg_int16_max_node_type,
	&am_dfg_int32_max_node_type,
	&am_dfg_int64_max_node_type,

	&am_dfg_uint8_avg_node_type,
	&am_dfg_uint16_avg_node_type,
	&am_dfg_uint32_avg_node_type,
	&am_dfg_uint64_avg_node_type,
	&am_dfg_timestamp_avg_node_type,
	&am_dfg_double_avg_node_type,
	&am_dfg_int8_avg_node_type,
	&am_dfg_int16_avg_node_type,
	&am_dfg_int32_avg_node_type,
	&am_dfg_int64_avg_node_type)

#endif
