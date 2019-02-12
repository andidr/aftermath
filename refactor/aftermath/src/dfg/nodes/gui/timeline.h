/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AM_DFG_AMGUI_TIMELINE_NODE_H
#define AM_DFG_AMGUI_TIMELINE_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aftermath/core/dfg_node.h>
#include "../../../cxx_interoperability.h"

AM_CXX_C_FWDDECL_CLASS_STRUCT(TimelineWidget);

struct am_dfg_amgui_timeline_node {
	struct am_dfg_node node;
	AM_CXX_C_DECL_CLASS_STRUCT_PTR_FIELD(TimelineWidget, timeline);
	char* timeline_id;
	uint64_t xdesc_height_u64;
	int xdesc_height_init;
	uint64_t ydesc_width_u64;
	int ydesc_width_init;
};

enum am_dfg_amgui_timeline_node_port_indexes {
	AM_DFG_AMGUI_TIMELINE_NODE_TRACE_PORT = 0,
	AM_DFG_AMGUI_TIMELINE_NODE_HIERARCHY_PORT = 1,
	AM_DFG_AMGUI_TIMELINE_NODE_INTERVAL_IN_PORT = 2,
	AM_DFG_AMGUI_TIMELINE_NODE_LAYERS_OUT_PORT = 3,
	AM_DFG_AMGUI_TIMELINE_NODE_INTERVAL_OUT_PORT = 4,
	AM_DFG_AMGUI_TIMELINE_NODE_SELECTIONS_OUT_PORT = 5,
	AM_DFG_AMGUI_TIMELINE_NODE_MOUSE_POSITION_OUT_PORT = 6,
	AM_DFG_AMGUI_TIMELINE_NODE_MOUSE_CLICK_OUT_PORT = 7
};

int am_dfg_amgui_timeline_init(struct am_dfg_node* n);
void am_dfg_amgui_timeline_destroy(struct am_dfg_node* n);
int am_dfg_amgui_timeline_process(struct am_dfg_node* n);
int am_dfg_amgui_timeline_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);
int am_dfg_amgui_timeline_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);

int am_dfg_amgui_timeline_set_property(
	struct am_dfg_node* n,
	const struct am_dfg_property* property,
	const void* value);

int am_dfg_amgui_timeline_get_property(
	const struct am_dfg_node* n,
	const struct am_dfg_property* property,
	void** value);

int am_dfg_amgui_timeline_set_initial_properties(struct am_dfg_node* n);

/**
 * Node that can be associated with a timeline widget.
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_amgui_timeline_node_type,
	"am::gui::timeline",
	"Timeline",
	sizeof(struct am_dfg_amgui_timeline_node),
	AM_DFG_DEFAULT_PORT_DEPS_NONE,
	AM_DFG_NODE_FUNCTIONS({
		.init = am_dfg_amgui_timeline_init,
		.destroy = am_dfg_amgui_timeline_destroy,
		.process = am_dfg_amgui_timeline_process,
		.from_object_notation = am_dfg_amgui_timeline_from_object_notation,
		.to_object_notation = am_dfg_amgui_timeline_to_object_notation,
		.set_property = am_dfg_amgui_timeline_set_property,
		.get_property = am_dfg_amgui_timeline_get_property
	}),
	AM_DFG_NODE_PORTS(
		{ "trace", "const am::core::trace*", AM_DFG_PORT_IN },
		{ "hierarchy", "const am::core::hierarchy*", AM_DFG_PORT_IN },
		{ "interval in", "am::core::interval", AM_DFG_PORT_IN },
		{ "layers", "const am::render::timeline::layer*", AM_DFG_PORT_OUT },
		{ "interval out", "am::core::interval", AM_DFG_PORT_OUT },
		{ "selections", "am::core::interval", AM_DFG_PORT_OUT },
		{ "mouse position", "am::core::pair<am::core::timestamp,const am::core::hierarchy_node*>", AM_DFG_PORT_OUT },
		{ "mouse click", "am::core::pair<am::core::timestamp,const am::core::hierarchy_node*>", AM_DFG_PORT_OUT }),
	AM_DFG_PORT_DEPS(
		AM_DFG_PORT_DEP_UPDATE_IN_PORT("interval in"),
		AM_DFG_PORT_DEP_UPDATE_IN_PORT("hierarchy"),
		AM_DFG_PORT_DEP_INDEPENDENT_OUT_PORT("interval out"),
		AM_DFG_PORT_DEP_INDEPENDENT_OUT_PORT("selections"),
		AM_DFG_PORT_DEP_INDEPENDENT_OUT_PORT("mouse position"),
		AM_DFG_PORT_DEP_INDEPENDENT_OUT_PORT("mouse click"),
		AM_DFG_PORT_DEP_INDEPENDENT_OUT_PORT("layers"),
		AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_NEW, "interval in",
				AM_DFG_PORT_DEP_PUSH_NEW, "interval out")
	),
	AM_DFG_NODE_PROPERTIES(
		{ "xdesc_height", "X legend height", "am::core::uint64" },
		{ "ydesc_width", "Y legend width", "am::core::uint64" }))

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_amgui_timeline_node_type)

#ifdef __cplusplus
}
#endif

#endif
