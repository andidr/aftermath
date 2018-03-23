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

#ifndef AM_DFG_AMGUI_HISTOGRAM_NODE_H
#define AM_DFG_AMGUI_HISTOGRAM_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aftermath/core/dfg_node.h>
#include "../../../cxx_interoperability.h"

AM_CXX_C_FWDDECL_CLASS_STRUCT(HistogramWidget);

struct am_dfg_amgui_histogram_node {
	struct am_dfg_node node;
	AM_CXX_C_DECL_CLASS_STRUCT_PTR_FIELD(HistogramWidget, histogram_widget);
	char* histogram_id;
};

int am_dfg_amgui_histogram_init(struct am_dfg_node* n);
void am_dfg_amgui_histogram_destroy(struct am_dfg_node* n);
int am_dfg_amgui_histogram_process(struct am_dfg_node* n);
int am_dfg_amgui_histogram_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);
int am_dfg_amgui_histogram_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);
int am_dfg_amgui_histogram_property_port_data_changed(
	const struct am_dfg_node* n,
	struct am_dfg_port* p);

/**
 * Node that can be associated with a histogram widget.
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_amgui_histogram_node_type,
	"am::gui::histogram",
	"Histogram",
	sizeof(struct am_dfg_amgui_histogram_node),
	AM_DFG_DEFAULT_PORT_DEPS_NONE,
	AM_DFG_NODE_FUNCTIONS({
		.init = am_dfg_amgui_histogram_init,
		.destroy = am_dfg_amgui_histogram_destroy,
		.process = am_dfg_amgui_histogram_process,
		.from_object_notation = am_dfg_amgui_histogram_from_object_notation,
		.to_object_notation = am_dfg_amgui_histogram_to_object_notation
	}),
	AM_DFG_NODE_PORTS({ "in", "am::core::histogram1d_data", AM_DFG_PORT_IN }),
	AM_DFG_PORT_DEPS(
		AM_DFG_PORT_DEP_UPDATE_IN_PORT("in")
	),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_amgui_histogram_node_type)

#ifdef __cplusplus
}
#endif

#endif
