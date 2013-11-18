/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "task_graph.h"
#include <inttypes.h>

int export_nodes_event_set(FILE* fp, struct multi_event_set* mes, struct event_set* es, struct filter* f, int64_t start, int64_t end)
{
	int texec_start = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START);

	if(texec_start != -1) {
		struct single_event* ts = &es->single_events[texec_start];

		for(; ts && ts < &es->single_events[es->num_single_events] && ts->time <= end; ts = ts->next_texec_start) {
			if(!filter_has_single_event(f, ts))
				continue;

			fprintf(fp, "\t\"%"PRIx64"_%"PRId64"\" [colorscheme = \"pastel18\", "
				"shape=\"none\", label=<<table border=\"1\" cellspacing=\"0\">"
				"<tr><td bgcolor=\"%d\">O=%d</td><td bgcolor=\"%d\">X=%d</td></tr>"
				"<tr><td colspan=\"2\">S=%d</td></tr></table>>];\n",
				ts->active_frame->addr, ts->time,
				ts->active_frame->numa_node+1, ts->active_frame->numa_node+1,
				es->numa_node+1, es->numa_node+1,
				ts->active_frame->size);
		}

		ts = &es->single_events[texec_start];
	}

	return 0;
}

int export_task_graph_event_set(FILE* fp, struct multi_event_set* mes, struct event_set* es, struct filter* f, int64_t start, int64_t end)
{
	int comm_event = event_set_get_first_comm_in_interval(es, start, end);

	for(; comm_event != -1 && comm_event < es->num_comm_events; comm_event++) {
		struct comm_event* ce = &es->comm_events[comm_event];

		if(ce->time > end)
			break;

		if(!filter_has_comm_event(f, mes, ce))
			continue;

		if(ce->type == COMM_TYPE_DATA_WRITE && filter_has_comm_event(f, mes, ce)) {
			struct single_event* fr_next_exec = multi_event_set_find_next_texec_start_for_frame(mes, ce->time, ce->what);

			if(!fr_next_exec) {
				fprintf(stderr, "Warning: Could not find next texec for frame %"PRIx64"\n", ce->what->addr);
			} else {
				fprintf(fp, "\t\"%"PRIx64"_%"PRId64"\" -> \"%"PRIx64"_%"PRId64"\";\n",
					ce->active_frame->addr,
					ce->texec_start->time,
					ce->what->addr,
					fr_next_exec->time);
			}
		}
	}

	if(filter_has_single_event_type(f, SINGLE_TYPE_TCREATE)) {
		int single_event = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TCREATE);

		for(; single_event != -1 && single_event < es->num_single_events; single_event++) {
			struct single_event* sge = &es->single_events[single_event];

			if(sge->time > end)
				break;

			if(sge->type == SINGLE_TYPE_TCREATE) {
				if(!filter_has_single_event(f, sge))
					continue;

				struct single_event* fr_next_exec = multi_event_set_find_next_texec_start_for_frame(mes, sge->time, sge->what);

				if(!fr_next_exec) {
					fprintf(stderr, "Warning: Could not find next texec for frame %"PRIx64"\n", sge->what->addr);
				} else {
					if(sge->prev_texec_start) {
						fprintf(fp, "\t\"%"PRIx64"_%"PRId64"\" -> \"%"PRIx64"_%"PRId64"\" [style = dotted];\n",
							sge->active_frame->addr,
							sge->prev_texec_start->time,
							sge->what->addr,
							fr_next_exec->time);
					}
				}
			}
		}
	}

	return 0;
}

int export_task_graph(const char* outfile, struct multi_event_set* mes, struct filter* f, int64_t start, int64_t end)
{
	int ret = 1;
	FILE* fp = fopen(outfile, "w+");

	if(!fp)
		goto out;

	if(start < 0)
		start = 0;

	if(end < 0)
		end = 0;

	fprintf(fp, "digraph task_graph {\n");

	for(struct event_set* es = &mes->sets[0]; es < &mes->sets[mes->num_sets]; es++)
		if(export_nodes_event_set(fp, mes, es, f, start, end))
			goto out_fp;

	for(struct event_set* es = &mes->sets[0]; es < &mes->sets[mes->num_sets]; es++)
		if(export_task_graph_event_set(fp, mes, es, f, start, end))
			goto out_fp;

	fprintf(fp, "}\n");

	ret = 0;

out_fp:
	fclose(fp);
out:
	return ret;
}
