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

#include "timeline.h"
#include "../../../gui/widgets/TimelineWidget.h"

extern "C" {
	#include <aftermath/core/dfg/types/pair_timestamp_hierarchy_node.h>
}

int am_dfg_amgui_timeline_init(struct am_dfg_node* n)
{
	struct am_dfg_amgui_timeline_node* t = (typeof(t))n;

	t->timeline = NULL;
	t->timeline_id = NULL;

	return 0;
}

void am_dfg_amgui_timeline_destroy(struct am_dfg_node* n)
{
	struct am_dfg_amgui_timeline_node* t = (typeof(t))n;

	free(t->timeline_id);
}

int am_dfg_amgui_timeline_process(struct am_dfg_node* n)
{
	struct am_dfg_amgui_timeline_node* t = (typeof(t))n;
	struct am_dfg_port* phierarchy_in = &n->ports[AM_DFG_AMGUI_TIMELINE_NODE_HIERARCHY_PORT];
	struct am_dfg_port* ptrace_in = &n->ports[AM_DFG_AMGUI_TIMELINE_NODE_TRACE_PORT];
	struct am_dfg_port* pinterval_in = &n->ports[AM_DFG_AMGUI_TIMELINE_NODE_INTERVAL_IN_PORT];
	struct am_dfg_port* players_out = &n->ports[AM_DFG_AMGUI_TIMELINE_NODE_LAYERS_OUT_PORT];
	struct am_dfg_port* pinterval_out = &n->ports[AM_DFG_AMGUI_TIMELINE_NODE_INTERVAL_OUT_PORT];
	struct am_dfg_port* pselections_out = &n->ports[AM_DFG_AMGUI_TIMELINE_NODE_SELECTIONS_OUT_PORT];
	struct am_dfg_port* pmousepos_out = &n->ports[AM_DFG_AMGUI_TIMELINE_NODE_MOUSE_POSITION_OUT_PORT];
	struct am_interval* interval;
	struct am_interval* selection_intervals;
	struct am_dfg_type_pair_timestamp_const_hierarchy_node* mousepos;
	struct am_hierarchy_node* hnode;
	struct am_interval interval_in;
	struct am_trace* trace_in = NULL;
	struct am_hierarchy* hierarchy_in = NULL;
	struct am_timeline_renderer* renderer;
	struct am_timeline_render_layer* layer;
	struct am_timeline_render_layer** layers;
	size_t num_layers;
	size_t num_selections;
	size_t i;
	void* out;
	struct am_interval bounds;

	if(am_dfg_port_activated_and_has_data(phierarchy_in)) {
		if(am_dfg_buffer_read_last(phierarchy_in->buffer, &hierarchy_in))
			return 1;

		t->timeline->setHierarchy(hierarchy_in);
	}

	if(am_dfg_port_activated_and_has_data(ptrace_in)) {
		if(am_dfg_buffer_read_last(ptrace_in->buffer, &trace_in))
			return 1;

		t->timeline->setTrace(trace_in);

		/* Bounds may be "invalid" if the trace does not contain any
		 * event with a timestamp */
		if(trace_in->bounds.start > trace_in->bounds.end) {
			bounds.start = 0;
			bounds.end = 0;
		} else {
			bounds = trace_in->bounds;
		}

		t->timeline->setVisibleInterval(&bounds);
	}

	if(am_dfg_port_activated_and_has_data(pinterval_in)) {
		if(am_dfg_buffer_read_last(pinterval_in->buffer, &interval_in))
			return 1;

		t->timeline->setVisibleInterval(&interval_in);
	}

	if(am_dfg_port_activated(players_out)) {
		renderer = t->timeline->getRenderer();
		num_layers = 0;

		/* Count layers */
		am_timeline_renderer_for_each_layer(renderer, layer)
			num_layers++;

		if(!(out = am_dfg_buffer_reserve(players_out->buffer, num_layers)))
		 	return 1;

		i = 0;
		layers = (typeof(layers))out;

		am_timeline_renderer_for_each_layer(renderer, layer)
			layers[i++] = layer;
	}

	if(am_dfg_port_activated(pinterval_out)) {
		if(!(out = am_dfg_buffer_reserve(pinterval_out->buffer, 1)))
			return 1;

		interval = (struct am_interval*)out;

		t->timeline->getVisibleInterval(interval);
	}

	if(am_dfg_port_activated(pselections_out)) {
		num_selections = t->timeline->getNumSelections();

		if(num_selections > 0) {
			if(!(out = am_dfg_buffer_reserve(pselections_out->buffer,
							 num_selections)))
			{
				return 1;
			}

			selection_intervals = (struct am_interval*)out;

			t->timeline->storeSelectionIntervals(selection_intervals,
							     num_selections);
		}
	}

	if(am_dfg_port_activated(pmousepos_out)) {
		hnode = t->timeline->getLastHierarchyNodeUnderCursor();

		/* Only write a sample, in which both the timestamp and the
		 * hierarchy node are valid */
		if(hnode) {
			if(!(out = am_dfg_buffer_reserve(pmousepos_out->buffer, 1)))
				return 1;

			mousepos = (struct am_dfg_type_pair_timestamp_const_hierarchy_node*)out;

			mousepos->timestamp = t->timeline->getLastTimestampUnderCursor();
			mousepos->node = hnode;
		}
	}

	return 0;
}

int am_dfg_amgui_timeline_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_timeline_node* t = (typeof(t))n;
	const char* timeline_id;

	if(am_object_notation_eval_retrieve_string(&g->node, "timeline_id",
						   &timeline_id))
	{
		return 1;
	}

	if(!(t->timeline_id = strdup(timeline_id)))
		return 1;

	return 0;
}

int am_dfg_amgui_timeline_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_amgui_timeline_node* t = (typeof(t))n;
	struct am_object_notation_node_member* mtimeline_id;

	mtimeline_id = (struct am_object_notation_node_member*)
		am_object_notation_build(
			AM_OBJECT_NOTATION_BUILD_MEMBER, "timeline_id",
			AM_OBJECT_NOTATION_BUILD_STRING, t->timeline_id);

	if(!mtimeline_id)
		return 1;

	am_object_notation_node_group_add_member(g, mtimeline_id);

	return 0;
}
