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

#ifndef AM_RENDER_DFG_TIMELINE_LAYER_NODE_BACKGROUND_H
#define AM_RENDER_DFG_TIMELINE_LAYER_NODE_BACKGROUND_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/render/dfg/timeline_layer_common.h>

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	background,
	"background",
	"Timeline Background Layer Filter")

int am_render_dfg_timeline_background_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_render_dfg_timeline_background_layer_configuration_node_type,
	"am::render::timeline::layer::background::configuration",
	"Timeline Background Layer Configuration",
	AM_DFG_NODE_DEFAULT_SIZE,
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
		.process = am_render_dfg_timeline_background_layer_configuration_node_process
	}),
	AM_DFG_NODE_PORTS(
		{ "layer", "const am::render::timeline::layer::background*", AM_DFG_PORT_IN },
		{ "even index color", "am::render::rgba", AM_DFG_PORT_IN },
		{ "odd index color", "am::render::rgba", AM_DFG_PORT_IN }
	),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_render_dfg_timeline_background_layer_filter_node_type,
	&am_render_dfg_timeline_background_layer_configuration_node_type)

#endif
