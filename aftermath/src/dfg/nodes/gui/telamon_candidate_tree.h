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

#ifndef AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_H
#define AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aftermath/core/dfg_node.h>
#include "../../../cxx_interoperability.h"

AM_CXX_C_FWDDECL_CLASS_STRUCT(TelamonCandidateTreeWidget);

struct am_dfg_amgui_telamon_candidate_tree_node {
	struct am_dfg_node node;
	AM_CXX_C_DECL_CLASS_STRUCT_PTR_FIELD(TelamonCandidateTreeWidget, tree);
	char* tree_id;
};

enum am_dfg_amgui_telamon_candidate_tree_node_port_indexes {
	AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_ROOT_IN_PORT = 0,
	AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_INTERVALS_IN_PORT,
	AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_SELECTIONS_OUT_PORT,
	AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_HOVER_CANDIDATE_OUT_PORT
};

int am_dfg_amgui_telamon_candidate_tree_init(struct am_dfg_node* n);
void am_dfg_amgui_telamon_candidate_tree_destroy(struct am_dfg_node* n);
int am_dfg_amgui_telamon_candidate_tree_process(struct am_dfg_node* n);
int am_dfg_amgui_telamon_candidate_tree_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);
int am_dfg_amgui_telamon_candidate_tree_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g);

/**
 * Node that can be associated with a Telamon candidate tree widget.
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_amgui_telamon_candidate_tree_node_type,
	"am::gui::telamon::candidate_tree",
	"Telamon Candidate Tree Widget",
	sizeof(struct am_dfg_amgui_telamon_candidate_tree_node),
	AM_DFG_DEFAULT_PORT_DEPS_NONE,
	AM_DFG_NODE_FUNCTIONS({
		.init = am_dfg_amgui_telamon_candidate_tree_init,
		.destroy = am_dfg_amgui_telamon_candidate_tree_destroy,
		.process = am_dfg_amgui_telamon_candidate_tree_process,
		.from_object_notation = am_dfg_amgui_telamon_candidate_tree_from_object_notation,
		.to_object_notation = am_dfg_amgui_telamon_candidate_tree_to_object_notation
	}),
	AM_DFG_NODE_PORTS(
		{ "root", "const am::telamon::candidate*", AM_DFG_PORT_IN },
		{ "intervals", "am::core::interval", AM_DFG_PORT_IN },
		{ "selections", "const am::telamon::candidate*", AM_DFG_PORT_OUT },
		{ "hover candidate", "const am::telamon::candidate*", AM_DFG_PORT_OUT }
	),
	AM_DFG_PORT_DEPS(
		AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_NEW, "root",
				AM_DFG_PORT_DEP_PUSH_NEW, "selections"),
		AM_DFG_PORT_DEP_UPDATE_IN_PORT("root"),
		AM_DFG_PORT_DEP_UPDATE_IN_PORT("intervals"),
		AM_DFG_PORT_DEP_INDEPENDENT_OUT_PORT("selections"),
		AM_DFG_PORT_DEP_INDEPENDENT_OUT_PORT("hover candidate")),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_amgui_telamon_candidate_tree_node_type)

#ifdef __cplusplus
}
#endif

#endif
