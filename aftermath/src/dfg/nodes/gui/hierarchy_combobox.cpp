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

#include "hierarchy_combobox.h"
#include "../../../gui/widgets/HierarchyComboBox.h"

extern "C" {
	#include <aftermath/core/hierarchy.h>
	#include <aftermath/core/trace.h>
}

int am_dfg_amgui_hierarchy_combobox_init(struct am_dfg_node* n)
{
	struct am_dfg_amgui_hierarchy_combobox_node* hcb = (typeof(hcb))n;

	hcb->widget = NULL;
	hcb->widget_id = NULL;

	return 0;
}

void am_dfg_amgui_hierarchy_combobox_destroy(struct am_dfg_node* n)
{
	struct am_dfg_amgui_hierarchy_combobox_node* hcb = (typeof(hcb))n;

	free(hcb->widget_id);
}

void am_dfg_amgui_hierarchy_combobox_disconnect(struct am_dfg_node* n,
						struct am_dfg_port* pi)
{
	struct am_dfg_amgui_hierarchy_combobox_node* hcb = (typeof(hcb))n;

	if(pi == &n->ports[0])
		if(hcb->widget)
			hcb->widget->setHierarchies(NULL);
}

int am_dfg_amgui_hierarchy_combobox_process(struct am_dfg_node* n)
{
	struct am_dfg_amgui_hierarchy_combobox_node* hcb = (typeof(hcb))n;
	struct am_dfg_port* ptrace = &n->ports[AM_DFG_AMGUI_HIERARCHY_COMBOBOX_NODE_TRACE];
	struct am_dfg_port* phierarchy = &n->ports[AM_DFG_AMGUI_HIERARCHY_COMBOBOX_NODE_HIERARCHY];
	struct am_hierarchy* selected = NULL;
	struct am_trace* trace = NULL;

	if(am_dfg_port_activated(ptrace)) {
		if(am_dfg_buffer_read_last(ptrace->buffer, (void*)&trace))
			return 1;

		if(hcb->widget) {
			if(trace)
				hcb->widget->setHierarchies(&trace->hierarchies);
			else
				hcb->widget->setHierarchies(NULL);
		}
	}

	if(am_dfg_port_activated(phierarchy) && hcb->widget) {
		if((selected = hcb->widget->getSelectedHierarchy()))
			if(am_dfg_buffer_write(phierarchy->buffer, 1, &selected))
				return 1;
	}

	return 0;
}

int am_dfg_amgui_hierarchy_combobox_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_hierarchy_combobox_node* hcb = (typeof(hcb))n;
	const char* widget_id;

	if(am_object_notation_eval_retrieve_string(&g->node, "widget_id",
						   &widget_id))
	{
		return 1;
	}

	if(!(hcb->widget_id = strdup(widget_id)))
		return 1;

	return 0;
}

int am_dfg_amgui_hierarchy_combobox_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_hierarchy_combobox_node* hcb = (typeof(hcb))n;

	return am_object_notation_node_group_build_add_members(
		g,
		AM_OBJECT_NOTATION_BUILD_MEMBER, "widget_id",
		  AM_OBJECT_NOTATION_BUILD_STRING, hcb->widget_id);
}
