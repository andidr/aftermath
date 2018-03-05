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

#include "label.h"
#include <QLabel>

int am_dfg_amgui_label_init(struct am_dfg_node* n)
{
	struct am_dfg_amgui_label_node* l = (typeof(l))n;

	l->label = NULL;
	l->label_id = NULL;

	return 0;
}

void am_dfg_amgui_label_destroy(struct am_dfg_node* n)
{
	struct am_dfg_amgui_label_node* l = (typeof(l))n;

	free(l->label_id);
}

int am_dfg_amgui_label_process(struct am_dfg_node* n)
{
	struct am_dfg_amgui_label_node* l = (typeof(l))n;
	struct am_dfg_port* in = &n->ports[0];
	char* str;

	/* Connected and data present? */
	if(in->num_connections > 0) {
		if(in->buffer->num_samples) {
			if(am_dfg_buffer_get(in->buffer,
					     in->buffer->num_samples-1,
					     1,
					     &str))
			{
				return 1;
			}

			l->label->setText(str);
		} else {
			l->label->setText("");
		}
	}

	return 0;
}

int am_dfg_amgui_label_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_label_node* l = (typeof(l))n;
	const char* label_id;

	if(am_object_notation_eval_retrieve_string(&g->node, "label_id", &label_id))
		return 1;

	if(!(l->label_id = strdup(label_id)))
		return 1;

	return 0;
}

int am_dfg_amgui_label_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_label_node* l = (typeof(l))n;
	struct am_object_notation_node_member* mlabel_id;

	mlabel_id = (struct am_object_notation_node_member*)
		am_object_notation_build(
			AM_OBJECT_NOTATION_BUILD_MEMBER, "label_id",
			AM_OBJECT_NOTATION_BUILD_STRING, l->label_id);

	if(!mlabel_id)
		return 1;

	am_object_notation_node_group_add_member(g, mlabel_id);

	return 0;
}
