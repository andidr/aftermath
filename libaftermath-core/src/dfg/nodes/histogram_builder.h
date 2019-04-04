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

#ifndef AM_DFG_NODE_HISTOGRAM_BUILDER_H
#define AM_DFG_NODE_HISTOGRAM_BUILDER_H

#include <stdint.h>
#include <aftermath/core/dfg_node.h>

#define AM_DFG_HISTOGRAM_BUILDER_DECL_INT(T, TPREFIX)				\
	struct am_dfg_histogram_builder_##TPREFIX##_node {			\
		struct am_dfg_node node;					\
										\
		size_t num_bins;						\
		size_t num_bins_u64;						\
		int auto_min_max;						\
		T min;								\
		T max;								\
	};									\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_init(			\
		struct am_dfg_node* n);					\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_process(		\
		struct am_dfg_node* n);					\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_set_property(		\
		struct am_dfg_node* n,						\
		const struct am_dfg_property* property,			\
		const void* value);						\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_get_property(		\
		const struct am_dfg_node* n,					\
		const struct am_dfg_property* property,			\
		void** value);							\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_from_object_notation(	\
		struct am_dfg_node* n, struct am_object_notation_node_group* g);\
										\
	int am_dfg_histogram_builder_##TPREFIX##_node_to_object_notation(	\
		struct am_dfg_node* n, struct am_object_notation_node_group* g);\
										\
	/**									\
	 * Node that creates a ##TPREFIX histogram from a series of ##TPREFIX \
	 * values */								\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_histogram_builder_##TPREFIX##_node_type,		\
		"am::core::statistics::histogram_builder<" #TPREFIX ">",	\
		"Histogram builder <" #TPREFIX ">",				\
		sizeof(struct am_dfg_histogram_builder_##TPREFIX##_node),	\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,			\
		AM_DFG_NODE_FUNCTIONS({					\
			.init = am_dfg_histogram_builder_##TPREFIX##_node_init, \
			.process = am_dfg_histogram_builder_##TPREFIX##_node_process, \
			.set_property = am_dfg_histogram_builder_##TPREFIX##_node_set_property, \
			.get_property = am_dfg_histogram_builder_##TPREFIX##_node_get_property, \
			.from_object_notation = am_dfg_histogram_builder_##TPREFIX##_node_from_object_notation, \
			.to_object_notation = am_dfg_histogram_builder_##TPREFIX##_node_to_object_notation, \
		}),								\
		AM_DFG_NODE_PORTS(						\
			{ "in", "am::core::" #TPREFIX,			\
				AM_DFG_PORT_IN | AM_DFG_PORT_MANDATORY },	\
			{ "out", "am::core::histogram1d<" #TPREFIX ">",	\
				AM_DFG_PORT_OUT | AM_DFG_PORT_MANDATORY }),	\
		AM_DFG_PORT_DEPS(),						\
		AM_DFG_NODE_PROPERTIES(					\
			{ "num_bins", "Number of bins", "am::core::uint64" },	\
			{ "auto_min_max",					\
				"Automatically determine min. / max. value",	\
				"am::core::bool" },				\
			{ "min", "Minimum", "am::core::" #TPREFIX },		\
			{ "max", "Maximum", "am::core::" #TPREFIX }		\
		))

AM_DFG_HISTOGRAM_BUILDER_DECL_INT( int8_t,  int8)
AM_DFG_HISTOGRAM_BUILDER_DECL_INT(int16_t, int16)
AM_DFG_HISTOGRAM_BUILDER_DECL_INT(int32_t, int32)
AM_DFG_HISTOGRAM_BUILDER_DECL_INT(int64_t, int64)

AM_DFG_HISTOGRAM_BUILDER_DECL_INT( uint8_t,  uint8)
AM_DFG_HISTOGRAM_BUILDER_DECL_INT(uint16_t, uint16)
AM_DFG_HISTOGRAM_BUILDER_DECL_INT(uint32_t, uint32)
AM_DFG_HISTOGRAM_BUILDER_DECL_INT(uint64_t, uint64)

AM_DFG_HISTOGRAM_BUILDER_DECL_INT(double, double)

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_dfg_histogram_builder_int8_node_type,
	&am_dfg_histogram_builder_int16_node_type,
	&am_dfg_histogram_builder_int32_node_type,
	&am_dfg_histogram_builder_int64_node_type,
	&am_dfg_histogram_builder_uint8_node_type,
	&am_dfg_histogram_builder_uint16_node_type,
	&am_dfg_histogram_builder_uint32_node_type,
	&am_dfg_histogram_builder_uint64_node_type,
	&am_dfg_histogram_builder_double_node_type)

#endif
