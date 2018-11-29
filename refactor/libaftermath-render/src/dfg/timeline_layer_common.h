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

#ifndef AM_RENDER_DFG_TIMELINE_LAYER_COMMON_H
#define AM_RENDER_DFG_TIMELINE_LAYER_COMMON_H

#include <aftermath/core/dfg_type.h>
#include <aftermath/core/dfg_node.h>

/* Declares a DFG type "const am::render::timeline::layer::<ident_name>*" for
 * pointers to a render layer */
#define AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(layer_name, ident_name)		\
	AM_DFG_DECL_BUILTIN_TYPE(						\
		am_render_dfg_type_timeline_##layer_name##_layer,		\
		"const am::render::timeline::layer::" ident_name "*",		\
		/* The exact type doesn't really matter here, since the size of \
		 * pointers is the same for all layers */			\
		sizeof(struct am_timeline_render_layer*),			\
		NULL, NULL, NULL, NULL)

/* Declares a DFG node type that takes a list of timeline layers as an input and
 * only writes those layers to the output that are of the DFG type "const
 * am::render::timeline::layer::<ident_name>*".
 *
 * The parameter layer_name is used to complete the symbol for this DFG node
 * definition and hrname is the human-readable name for the node.
 */
#define AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(			\
	layer_name, ident_name, hrname)					\
	int am_render_dfg_timeline_##layer_name##_layer_filter_node_process(	\
		struct am_dfg_node* n);					\
										\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_render_dfg_timeline_##layer_name##_layer_filter_node_type,	\
		"am::render::timeline::layer::" ident_name "::filter",		\
		hrname,							\
		AM_DFG_NODE_DEFAULT_SIZE,					\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,			\
		AM_DFG_NODE_FUNCTIONS({					\
			.process = am_render_dfg_timeline_##layer_name##_layer_filter_node_process \
		}),								\
		AM_DFG_NODE_PORTS(						\
			{ "in",						\
			  "const am::render::timeline::layer*",		\
			  AM_DFG_PORT_IN },					\
			{ "out",						\
			  "const am::render::timeline::layer::" ident_name "*", \
			  AM_DFG_PORT_OUT }					\
		),								\
		AM_DFG_PORT_DEPS(),						\
		AM_DFG_NODE_PROPERTIES())

/* Implementation macro for
 * AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE. The additional paramter
 * layer_c_name must be the C type representing an instance of the layer.
 */
#define AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(			\
	layer_name, ident_name, layer_c_type)					\
	int am_render_dfg_timeline_##layer_name##_layer_filter_node_process(	\
		struct am_dfg_node* n)						\
	{									\
		size_t num_in_layers;						\
		struct am_timeline_render_layer** in_layers;			\
		layer_c_type** out_layers;					\
		size_t num_out_layers = 0;					\
		size_t curr_idx = 0;						\
										\
		/* No input layers -> Exit */					\
		if(!am_dfg_port_activated_and_has_data(&n->ports[0]) ||	\
		   !am_dfg_port_activated(&n->ports[1]))			\
		{								\
			return 0;						\
		}								\
										\
		num_in_layers = n->ports[0].buffer->num_samples;		\
		in_layers = n->ports[0].buffer->data;				\
										\
		/* Count number of matching layers in order to allocate the	\
		 * entire chunk for the output layers in one go afterwards */	\
		for(size_t i = 0; i < num_in_layers; i++) {			\
			if(strcmp(in_layers[i]->type->name, ident_name) == 0)	\
				num_out_layers++;				\
		}								\
										\
		if(num_out_layers == 0)					\
			return 0;						\
										\
		if(!(out_layers = am_dfg_buffer_reserve(			\
			     n->ports[1].buffer, num_out_layers)))		\
		{								\
			return 1;						\
		}								\
										\
		for(size_t i = 0; i < num_in_layers; i++) {			\
			if(strcmp(in_layers[i]->type->name, ident_name) == 0) { \
				out_layers[curr_idx++] =			\
					(layer_c_type*)in_layers[i];		\
			}							\
		}								\
										\
		return 0;							\
	}

#endif
