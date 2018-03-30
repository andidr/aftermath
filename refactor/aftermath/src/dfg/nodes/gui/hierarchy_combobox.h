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

#ifndef AM_DFG_AMGUI_HIERARCHY_COMBOBOX_NODE_H
#define AM_DFG_AMGUI_HIERARCHY_COMBOBOX_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aftermath/core/dfg_node.h>
#include "../../../cxx_interoperability.h"

AM_CXX_C_FWDDECL_CLASS_STRUCT(HierarchyComboBox);

enum am_dfg_amgui_hierarchy_combobox_node_port_indexes {
	AM_DFG_AMGUI_HIERARCHY_COMBOBOX_NODE_TRACE = 0,
	AM_DFG_AMGUI_HIERARCHY_COMBOBOX_NODE_HIERARCHY = 1
};

struct am_dfg_amgui_hierarchy_combobox_node {
	struct am_dfg_node node;
	AM_CXX_C_DECL_CLASS_STRUCT_PTR_FIELD(HierarchyComboBox, widget);
	char* widget_id;
};

int am_dfg_amgui_hierarchy_combobox_init(struct am_dfg_node* n);
void am_dfg_amgui_hierarchy_combobox_destroy(struct am_dfg_node* n);
void am_dfg_amgui_hierarchy_combobox_disconnect(struct am_dfg_node* n,
						struct am_dfg_port* pi);
int am_dfg_amgui_hierarchy_combobox_process(struct am_dfg_node* n);
int am_dfg_amgui_hierarchy_combobox_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);
int am_dfg_amgui_hierarchy_combobox_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);

/**
 * Node that can be associated with a hierarchy combobox widget.
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_amgui_hierarchy_combobox_node_type,
	"am::gui::hierarchy_combobox",
	"Hierarchy Combobox",
	sizeof(struct am_dfg_amgui_hierarchy_combobox_node),
	AM_DFG_DEFAULT_PORT_DEPS_NONE,
	AM_DFG_NODE_FUNCTIONS({
		.init = am_dfg_amgui_hierarchy_combobox_init,
		.destroy = am_dfg_amgui_hierarchy_combobox_destroy,
		.disconnect = am_dfg_amgui_hierarchy_combobox_disconnect,
		.process = am_dfg_amgui_hierarchy_combobox_process,
		.from_object_notation = am_dfg_amgui_hierarchy_combobox_from_object_notation,
		.to_object_notation = am_dfg_amgui_hierarchy_combobox_to_object_notation
	}),
	AM_DFG_NODE_PORTS(
		{ "trace", "const am::core::trace", AM_DFG_PORT_IN },
		{ "hierarchy", "const am::core::hierarchy", AM_DFG_PORT_OUT }),
	AM_DFG_PORT_DEPS(
		AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_NEW, "trace",
				AM_DFG_PORT_DEP_PUSH_NEW, "hierarchy"),
		AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_NEW, "hierarchy",
				AM_DFG_PORT_DEP_PULL_NEW, "trace"),
		AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_OLD, "trace",
				AM_DFG_PORT_DEP_PUSH_OLD, "hierarchy"),
		AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_OLD, "hierarchy",
				AM_DFG_PORT_DEP_PUSH_OLD, "hierarchy")
	),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_amgui_hierarchy_combobox_node_type)

#ifdef __cplusplus
}
#endif

#endif
