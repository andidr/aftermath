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

enum port_indexes {
	TRACE_PORT = 0,
	HIERARCHY_PORT = 1,
	INTERVAL_IN_PORT = 2,
	INTERVAL_OUT_PORT = 3
};

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
	struct am_dfg_port* phierarchy_in = &n->ports[HIERARCHY_PORT];
	struct am_dfg_port* ptrace_in = &n->ports[TRACE_PORT];
	struct am_dfg_port* pinterval_in = &n->ports[INTERVAL_IN_PORT];
	struct am_dfg_port* pinterval_out = &n->ports[INTERVAL_OUT_PORT];
	struct am_interval* interval;
	struct am_interval interval_in;
	struct am_trace* trace_in = NULL;
	struct am_hierarchy* hierarchy_in = NULL;
	void* out;

	if(am_dfg_port_activated_and_has_data(phierarchy_in)) {
		if(am_dfg_buffer_read_last(phierarchy_in->buffer, &hierarchy_in))
			return 1;

		t->timeline->setHierarchy(hierarchy_in);
	}

	if(am_dfg_port_activated_and_has_data(ptrace_in)) {
		if(am_dfg_buffer_read_last(ptrace_in->buffer, &trace_in))
			return 1;

		t->timeline->setTrace(trace_in);
		t->timeline->setVisibleInterval(&trace_in->bounds);
	}

	if(am_dfg_port_activated_and_has_data(pinterval_in)) {
		if(am_dfg_buffer_read_last(pinterval_in->buffer, &interval_in))
			return 1;

		t->timeline->setVisibleInterval(&interval_in);
	}

	if(am_dfg_port_activated(pinterval_out)) {
		if(!(out = am_dfg_buffer_reserve(pinterval_out->buffer, 1)))
			return 1;

		interval = (struct am_interval*)out;

		t->timeline->getVisibleInterval(interval);
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
