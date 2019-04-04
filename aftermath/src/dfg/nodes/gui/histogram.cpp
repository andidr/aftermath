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

#include "histogram.h"
#include "../../../gui/widgets/HistogramWidget.h"

int am_dfg_amgui_histogram_init(struct am_dfg_node* n)
{
	struct am_dfg_amgui_histogram_node* h = (typeof(h))n;

	h->histogram_widget = NULL;
	h->histogram_id = NULL;

	return 0;
}

void am_dfg_amgui_histogram_destroy(struct am_dfg_node* n)
{
	struct am_dfg_amgui_histogram_node* h = (typeof(h))n;

	free(h->histogram_id);
}

int am_dfg_amgui_histogram_process(struct am_dfg_node* n)
{
	struct am_dfg_amgui_histogram_node* h = (typeof(h))n;
	struct am_dfg_port* pdata = &n->ports[0];
	struct am_histogram1d_data* hd;
	struct am_histogram1d_data* hdclone;

	if(!am_dfg_port_is_connected(pdata) || !h->histogram_widget)
		return 0;

	if(am_dfg_buffer_read_last(pdata->buffer, &hd))
		return 1;

	if(!(hdclone = am_histogram1d_data_clone(hd)))
		return 1;

	h->histogram_widget->setHistogram(hdclone);

	return 0;
}

int am_dfg_amgui_histogram_property_port_data_changed(
	const struct am_dfg_node* n,
	struct am_dfg_port* p)
{
	return 0;
}

int am_dfg_amgui_histogram_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_histogram_node* h = (typeof(h))n;
	const char* histogram_id;

	if(am_object_notation_eval_retrieve_string(&g->node, "histogram_id",
						   &histogram_id))
	{
		return 1;
	}

	if(!(h->histogram_id = strdup(histogram_id)))
		return 1;

	return 0;
}

int am_dfg_amgui_histogram_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_histogram_node* h = (typeof(h))n;
	struct am_object_notation_node_member* mhistogram_id;

	mhistogram_id = (struct am_object_notation_node_member*)
		am_object_notation_build(
			AM_OBJECT_NOTATION_BUILD_MEMBER, "histogram_id",
			AM_OBJECT_NOTATION_BUILD_STRING, h->histogram_id);

	if(!mhistogram_id)
		return 1;

	am_object_notation_node_group_add_member(g, mhistogram_id);

	return 0;
}
