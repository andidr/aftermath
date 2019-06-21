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

#ifndef AM_RENDER_DFG_NODE_TIMELINE_LAYERS_STATE_H
#define AM_RENDER_DFG_NODE_TIMELINE_LAYERS_STATE_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/render/dfg/timeline_layer_common.h>

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	state,
	"state",
	"Timeline State Layer Filter")

int am_render_dfg_timeline_state_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	state,
	"state",
	"Timeline State Layer Configuration")

int am_render_dfg_timeline_state_layer_dominant_state_at_pos_node_type_process(
	struct am_dfg_node* n);

/* Node that extracts the dominant state at the pixel intervals including the
 * timestamp of the positions given as the inputs.
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_render_dfg_timeline_state_layer_dominant_state_at_pos_node_type,
	"am::render::timeline::layer::state::dominant_state_at_pos",
	"Dominant State at Position",
	AM_DFG_NODE_DEFAULT_SIZE,
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
		.process = am_render_dfg_timeline_state_layer_dominant_state_at_pos_node_type_process
	}),
	AM_DFG_NODE_PORTS(
		{ "layer",
		  "const am::render::timeline::layer::state*",
		  AM_DFG_PORT_IN },
		{ "mouse position",
		  "am::core::pair<am::core::timestamp,const am::core::hierarchy_node*>",
		  AM_DFG_PORT_IN },
		{ "dominant state",
		  "const am::core::state_description*",
		  AM_DFG_PORT_OUT }
	),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_render_dfg_timeline_state_layer_filter_node_type,
	&am_render_dfg_timeline_state_layer_configuration_node_type,
	&am_render_dfg_timeline_state_layer_dominant_state_at_pos_node_type)

#endif
