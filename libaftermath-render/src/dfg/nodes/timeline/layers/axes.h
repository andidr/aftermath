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

#ifndef AM_RENDER_DFG_TIMELINE_LAYER_NODE_AXES_H
#define AM_RENDER_DFG_TIMELINE_LAYER_NODE_AXES_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/render/dfg/timeline_layer_common.h>

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	axes,
	"axes",
	"Timeline Axes Layer Filter")

int am_render_dfg_timeline_axes_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_render_dfg_timeline_axes_layer_configuration_node_type,
	"am::render::timeline::layer::axes::configuration",
	"Timeline Axes Layer Configuration",
	AM_DFG_NODE_DEFAULT_SIZE,
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
		.process = am_render_dfg_timeline_axes_layer_configuration_node_process
	}),
	AM_DFG_NODE_PORTS(
		{ "layer", "const am::render::timeline::layer::axes*", AM_DFG_PORT_IN },

		{ "minimal minor tick distance", "am::core::uint64", AM_DFG_PORT_IN },

		{ "vertical axis color", "am::render::rgba", AM_DFG_PORT_IN },
		{ "vertical axis width", "am::core::double", AM_DFG_PORT_IN },

		{ "horizontal axis color", "am::render::rgba", AM_DFG_PORT_IN },
		{ "horizontal axis width", "am::core::double", AM_DFG_PORT_IN },

		{ "major labels enable", "am::core::bool", AM_DFG_PORT_IN },
		{ "major tick color", "am::render::rgba", AM_DFG_PORT_IN },
		{ "major tick height", "am::core::double", AM_DFG_PORT_IN },
		{ "major tick width", "am::core::double", AM_DFG_PORT_IN },
		{ "major label left margin", "am::core::double", AM_DFG_PORT_IN },
		{ "major label top margin", "am::core::double", AM_DFG_PORT_IN },
		{ "major label color", "am::render::rgba", AM_DFG_PORT_IN },
		{ "major label rotation degrees", "am::core::double", AM_DFG_PORT_IN },
		{ "major label significant digits", "am::core::uint64", AM_DFG_PORT_IN },
		{ "major label font scale", "am::core::double", AM_DFG_PORT_IN },
		{ "major label font", "am::core::string", AM_DFG_PORT_IN },

		{ "minor labels enable", "am::core::bool", AM_DFG_PORT_IN },
		{ "minor tick color", "am::render::rgba", AM_DFG_PORT_IN },
		{ "minor tick height", "am::core::double", AM_DFG_PORT_IN },
		{ "minor tick width", "am::core::double", AM_DFG_PORT_IN },
		{ "minor label left margin", "am::core::double", AM_DFG_PORT_IN },
		{ "minor label top margin", "am::core::double", AM_DFG_PORT_IN },
		{ "minor label color", "am::render::rgba", AM_DFG_PORT_IN },
		{ "minor label rotation degrees", "am::core::double", AM_DFG_PORT_IN },
		{ "minor label significant digits", "am::core::uint64", AM_DFG_PORT_IN },
		{ "minor label font scale", "am::core::double", AM_DFG_PORT_IN },
		{ "minor label font", "am::core::string", AM_DFG_PORT_IN }
	),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_render_dfg_timeline_axes_layer_filter_node_type,
	&am_render_dfg_timeline_axes_layer_configuration_node_type)

#endif
