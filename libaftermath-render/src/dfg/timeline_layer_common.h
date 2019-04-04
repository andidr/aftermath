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

#include <aftermath/core/ansi_extras.h>
#include <aftermath/core/dfg_type.h>
#include <aftermath/core/dfg_node.h>
#include <aftermath/core/dfg/types/generic.h>
#include <aftermath/render/cairo_extras.h>
#include <limits.h>

/* Declares a DFG type "const am::render::timeline::layer::<ident_name>*" for
 * pointers to a render layer */
#define AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(layer_name, ident_name)		\
	AM_DFG_DECL_BUILTIN_TYPE(						\
		am_render_dfg_type_timeline_##layer_name##_layer,		\
		"const am::render::timeline::layer::" ident_name "*",		\
		/* The exact type doesn't really matter here, since the size of \
		 * pointers is the same for all layers */			\
		sizeof(struct am_timeline_render_layer*),			\
		NULL,								\
		am_dfg_type_generic_plain_copy_samples,			\
		NULL, NULL, NULL)

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

#define AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN(SUFFIX, TIN, TOUT, REASSIGN_EXPR)	\
	/* Updates the value of type TOUT at offset tgt_offset of each layer	\
	 * read from layers_port according to the last value of type TIN read	\
	 * from in_port. The assignment is done using REASSIGN_EXPR, which takes\
	 * a pointer to the layer's value, a pointer to the input value and a	\
	 * pointer to an integer that allows the expression to indicate whether \
	 * the conversion was successful (0) or failed (1).			\
	 *									\
	 * If any layer was updated, *changed is set to 1, otherwise remains	\
	 * untouched.								\
	 *									\
	 * Returns 0 on success, otherwise 1.					\
	 */									\
	static inline int am_render_dfg_valupd_##SUFFIX(			\
		struct am_dfg_port* layers_port,				\
		struct am_dfg_port* in_port,					\
		off_t tgt_offset,						\
		int* changed)							\
	{									\
		void** layers = layers_port->buffer->data;			\
		size_t num_layers = layers_port->buffer->num_samples;		\
		TIN* v;							\
		TOUT* vout;							\
		int assign_ret;						\
										\
		if(am_dfg_port_activated(in_port) &&				\
		   in_port->buffer->num_samples > 0)				\
		{								\
			/* Skip intermediate changes, since they wouldn't be	\
			 * visible anyways; just take the last sample */	\
			if(am_dfg_buffer_last_ptr(in_port->buffer, (void**)&v)) \
				return 1;					\
										\
			for(size_t i = 0; i < num_layers; i++) {		\
				vout = AM_PTR_ADD(layers[i], tgt_offset);	\
										\
				REASSIGN_EXPR(vout, v, &assign_ret);		\
										\
				if(assign_ret != 0)				\
					return 1;				\
			}							\
										\
			*changed = 1;						\
		}								\
										\
		return 0;							\
	}

/* REASSIGN_EXPR for AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN for values of identical
 * type */
#define AM_RENDER_DFG_LAYER_VALUPD_DIRECT_REASSIGN_EXPR(PTGT, PSRC, PRETVAL)	\
	do {									\
		*(PTGT) = *(PSRC);						\
		*(PRETVAL) = 0;						\
	} while(0)

/* REASSIGN_EXPR for AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN for a uint64_t assigned
 * to a size_t */
#define AM_RENDER_DFG_LAYER_VALUPD_SIZE_T_FROM_UINT64_REASSIGN_EXPR(	\
	PTGT, PSRC, PRETVAL)						\
	do {								\
		if(am_safe_size_from_u64(PTGT, *(PSRC)))		\
			*(PRETVAL) = 1;				\
		else							\
			*(PRETVAL) = 0;				\
	} while(0)

/* REASSIGN_EXPR for AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN for a uint64_t assigned
 * to a unsigned int */
#define AM_RENDER_DFG_LAYER_VALUPD_UINT_FROM_UINT64_REASSIGN_EXPR(	\
	PTGT, PSRC, PRETVAL)						\
	do {								\
		if((*PSRC) > UINT_MAX) {				\
			*(PRETVAL) = 1;				\
		} else {						\
			(*PTGT) = (*PSRC);				\
			*(PRETVAL) = 0;				\
		}							\
	} while(0)

/* REASSIGN_EXPR for AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN for a string value
 * assigned to an already allocated string */
#define AM_RENDER_DFG_LAYER_VALUPD_STRING_REASSIGN_EXPR(PTGT, PSRC, PRETVAL)	\
	do {									\
		char* tmp = strdup(*(PSRC));					\
										\
		if(tmp) {							\
			free((*PTGT));						\
			*(PTGT) = tmp;						\
			*(PRETVAL) = 0;					\
		} else {							\
			*(PRETVAL) = 1;					\
		}								\
	} while(0)

AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN(
	rgba, struct am_rgba, struct am_rgba,
	AM_RENDER_DFG_LAYER_VALUPD_DIRECT_REASSIGN_EXPR)

AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN(
	double, double, double,
	AM_RENDER_DFG_LAYER_VALUPD_DIRECT_REASSIGN_EXPR)

AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN(
	bool, int, int,
	AM_RENDER_DFG_LAYER_VALUPD_DIRECT_REASSIGN_EXPR)

AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN(
	size_t, uint64_t, size_t,
	AM_RENDER_DFG_LAYER_VALUPD_SIZE_T_FROM_UINT64_REASSIGN_EXPR)

AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN(
	uint, uint64_t, unsigned int,
	AM_RENDER_DFG_LAYER_VALUPD_UINT_FROM_UINT64_REASSIGN_EXPR)

AM_RENDER_DFG_DECL_LAYER_VALUPD_FUN(
	string, char*, char*,
	AM_RENDER_DFG_LAYER_VALUPD_STRING_REASSIGN_EXPR)

#endif
