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

#ifndef AM_RENDER_DFG_NODE_TIMELINE_LAYERS_TENSORFLOW_NODE_EXECUTION_H
#define AM_RENDER_DFG_NODE_TIMELINE_LAYERS_TENSORFLOW_NODE_EXECUTION_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/render/dfg/timeline_layer_common.h>

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	tensorflow_node_execution,
	"tensorflow::node_execution",
	"Timeline TensorFlow Node Execution Layer Filter")

int am_render_dfg_timeline_tensorflow_node_execution_layer_dominant_node_at_pos_node_type_process(
	struct am_dfg_node* n);

/* Node that extracts the dominant TensorFlow nodes at the pixel intervals
 * including the timestamp of the positions given as the inputs.
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_render_dfg_timeline_tensorflow_node_execution_layer_dominant_node_at_pos_node_type,
	"am::render::timeline::layer::tensorflow::node_execution::dominant_node_at_pos",
	"Dominant TensorFlow Node at Position",
	AM_DFG_NODE_DEFAULT_SIZE,
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
		.process = am_render_dfg_timeline_tensorflow_node_execution_layer_dominant_node_at_pos_node_type_process
	}),
	AM_DFG_NODE_PORTS(
		{ "layer",
		  "const am::render::timeline::layer::tensorflow::node_execution*",
		  AM_DFG_PORT_IN },
		{ "mouse position",
		  "am::core::pair<am::core::timestamp,const am::core::hierarchy_node*>",
		  AM_DFG_PORT_IN },
		{ "dominant node",
		  "const am::tensorflow::node*",
		  AM_DFG_PORT_OUT }
	),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_render_dfg_timeline_tensorflow_node_execution_layer_filter_node_type,
	&am_render_dfg_timeline_tensorflow_node_execution_layer_dominant_node_at_pos_node_type)

#endif
