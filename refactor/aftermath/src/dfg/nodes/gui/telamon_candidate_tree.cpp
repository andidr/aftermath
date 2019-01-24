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

#include "telamon_candidate_tree.h"
#include "../../../gui/widgets/TelamonCandidateTreeWidget.h"

extern "C" {
	#include <aftermath/core/trace.h>
	#include <aftermath/core/in_memory.h>
}

int am_dfg_amgui_telamon_candidate_tree_init(struct am_dfg_node* n)
{
	struct am_dfg_amgui_telamon_candidate_tree_node* t = (typeof(t))n;

	t->tree = NULL;
	t->tree_id = NULL;

	return 0;
}

void am_dfg_amgui_telamon_candidate_tree_destroy(struct am_dfg_node* n)
{
	struct am_dfg_amgui_telamon_candidate_tree_node* t = (typeof(t))n;

	free(t->tree_id);
}

int am_dfg_amgui_telamon_candidate_tree_process(struct am_dfg_node* n)
{
	struct am_dfg_amgui_telamon_candidate_tree_node* t = (typeof(t))n;
	struct am_dfg_port* proot_in = &n->ports[AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_ROOT_IN_PORT];
	struct am_dfg_port* psel_out = &n->ports[AM_DFG_AMGUI_TELAMON_CANDIDATE_TREE_NODE_SELECTIONS_OUT_PORT];
	struct am_telamon_candidate* root = NULL;

	if(am_dfg_port_activated(proot_in)) {
		if(am_dfg_port_has_data(proot_in))
			if(am_dfg_buffer_read_last(proot_in->buffer, &root))
				return 1;

		t->tree->setCandidateTree(root);
	}

	if(am_dfg_port_activated(psel_out)) {
		if(t->tree) {
			std::vector<struct am_telamon_candidate*> vec =
				t->tree->getSelection();

			if(vec.size() > 0) {
				if(am_dfg_buffer_write(psel_out->buffer,
						       vec.size(),
						       &vec[0]))
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

int am_dfg_amgui_telamon_candidate_tree_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_telamon_candidate_tree_node* t = (typeof(t))n;
	const char* tree_id;

	if(am_object_notation_eval_retrieve_string(&g->node, "tree_id",
						   &tree_id))
	{
		return 1;
	}

	if(!(t->tree_id = strdup(tree_id)))
		return 1;

	return 0;
}

int am_dfg_amgui_telamon_candidate_tree_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_telamon_candidate_tree_node* t = (typeof(t))n;
	struct am_object_notation_node_member* mtree_id;

	mtree_id = (struct am_object_notation_node_member*)
		am_object_notation_build(
			AM_OBJECT_NOTATION_BUILD_MEMBER, "tree_id",
			AM_OBJECT_NOTATION_BUILD_STRING, t->tree_id);

	if(!mtree_id)
		return 1;

	am_object_notation_node_group_add_member(g, mtree_id);

	return 0;
}
