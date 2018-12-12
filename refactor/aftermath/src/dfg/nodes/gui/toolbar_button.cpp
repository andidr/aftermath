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

#include "toolbar_button.h"
#include "../../../gui/widgets/ToolbarButton.h"

int am_dfg_amgui_toolbar_button_init(struct am_dfg_node* n)
{
	struct am_dfg_amgui_toolbar_button_node* l = (typeof(l))n;

	l->button = NULL;
	l->widget_id = NULL;

	return 0;
}

void am_dfg_amgui_toolbar_button_destroy(struct am_dfg_node* n)
{
	struct am_dfg_amgui_toolbar_button_node* l = (typeof(l))n;

	free(l->widget_id);
}

int am_dfg_amgui_toolbar_button_process(struct am_dfg_node* n)
{
	struct am_dfg_amgui_toolbar_button_node* l = (typeof(l))n;
	struct am_dfg_port* pout = &n->ports[0];
	int dummy = 1;

	/* Connected and requested? */
	if(am_dfg_port_activated(pout)) {
		if(am_dfg_buffer_write(pout->buffer, 1, &dummy))
			return 1;
	}

	return 0;
}

int am_dfg_amgui_toolbar_button_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_toolbar_button_node* l = (typeof(l))n;
	const char* widget_id;

	if(am_object_notation_eval_retrieve_string(&g->node, "widget_id", &widget_id))
		return 1;

	if(!(l->widget_id = strdup(widget_id)))
		return 1;

	return 0;
}

int am_dfg_amgui_toolbar_button_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_toolbar_button_node* l = (typeof(l))n;
	struct am_object_notation_node_member* mwidget_id;

	mwidget_id = (struct am_object_notation_node_member*)
		am_object_notation_build(
			AM_OBJECT_NOTATION_BUILD_MEMBER, "widget_id",
			AM_OBJECT_NOTATION_BUILD_STRING, l->widget_id);

	if(!mwidget_id)
		return 1;

	am_object_notation_node_group_add_member(g, mwidget_id);

	return 0;
}
