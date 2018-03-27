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

#ifndef AM_DFG_NODE_HISTOGRAM_ATTRIBUTES_H
#define AM_DFG_NODE_HISTOGRAM_ATTRIBUTES_H

#include <aftermath/core/dfg_node.h>

#define AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE(TPREFIX)				\
	int am_dfg_histogram_##TPREFIX##_attributes_node_process(		\
		struct am_dfg_node* n);					\
										\
	/**									\
	 * Node that extracts the attributes of a histogram			\
	 */									\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_histogram_##TPREFIX##_attributes_node_type,		\
		"am::core::histogram1d<" #TPREFIX ">::attributes",		\
		"Histogram Attributes <" #TPREFIX ">",				\
		AM_DFG_NODE_DEFAULT_SIZE,					\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,			\
		AM_DFG_NODE_FUNCTIONS({					\
			.process = am_dfg_histogram_##TPREFIX##_attributes_node_process, \
		}),								\
		AM_DFG_NODE_PORTS(						\
			{ "in", "am::core::histogram1d<" #TPREFIX ">",		\
					AM_DFG_PORT_IN },			\
			{ "data", "am::core::histogram1d_data",		\
					AM_DFG_PORT_OUT },			\
			{ "left", "am::core::" #TPREFIX, AM_DFG_PORT_OUT },	\
			{ "right", "am::core::" #TPREFIX, AM_DFG_PORT_OUT },	\
			{ "num_bins", "am::core::uint64", AM_DFG_PORT_OUT }),	\
		AM_DFG_PORT_DEPS(),						\
		AM_DFG_NODE_PROPERTIES())

AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE( int8)
AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE(int16)
AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE(int32)
AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE(int64)

AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE( uint8)
AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE(uint16)
AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE(uint32)
AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE(uint64)

AM_DFG_DECL_HISTOGRAM_ATTRIBUTES_NODE(double)

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_dfg_histogram_int8_attributes_node_type,
	&am_dfg_histogram_int16_attributes_node_type,
	&am_dfg_histogram_int32_attributes_node_type,
	&am_dfg_histogram_int64_attributes_node_type,

	&am_dfg_histogram_uint8_attributes_node_type,
	&am_dfg_histogram_uint16_attributes_node_type,
	&am_dfg_histogram_uint32_attributes_node_type,
	&am_dfg_histogram_uint64_attributes_node_type,

	&am_dfg_histogram_double_attributes_node_type)

#endif
