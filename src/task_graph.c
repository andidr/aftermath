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
#include "texec_tree.h"
#include <inttypes.h>

void dump_tcreate_edge(FILE* fp, struct frame* fcreator, int64_t time_creator, struct frame* newframe, int64_t newtime)
{
	fprintf(fp, "\t\"%"PRIx64"_%"PRId64"\" -> \"%"PRIx64"_%"PRId64"\" [style = dotted];\n",
		fcreator->addr,
		time_creator,
		newframe->addr,
		newtime);
}

void dump_write_edge(FILE* fp, struct frame* fwriter, int64_t time_writer, struct frame* freader, int64_t time_reader, int size, int max_size)
{
	double min_pen_width = 1.0;
	double max_pen_width = 5.0;

	double pen_width = min_pen_width;

	if(max_size > 0)
		pen_width = min_pen_width + (max_pen_width - min_pen_width)* ((double)size) / ((double)max_size);

	fprintf(fp, "\t\"%"PRIx64"_%"PRId64"\" -> \"%"PRIx64"_%"PRId64"\" [penwidth = %f];\n",
		fwriter->addr,
		time_writer,
		freader->addr,
		time_reader,
		pen_width);
}

void dump_node(FILE* fp, struct single_event* texec_start)
{
	fprintf(fp, "\t\"%"PRIx64"_%"PRId64"\" [colorscheme = \"pastel18\", "
		"shape=\"none\", label=<<table border=\"1\" cellspacing=\"0\">"
		"<tr><td bgcolor=\"%d\">O=%d</td><td bgcolor=\"%d\">X=%d</td></tr>"
		"<tr><td colspan=\"2\">S=%d</td></tr></table>>];\n",
		texec_start->active_frame->addr, texec_start->time,
		texec_start->active_frame->numa_node+1, texec_start->active_frame->numa_node+1,
		texec_start->event_set->numa_node+1, texec_start->event_set->numa_node+1,
		texec_start->active_frame->size);
}

int export_nodes_event_set(FILE* fp, struct multi_event_set* mes, struct event_set* es, struct filter* f, int64_t start, int64_t end)
{
	int texec_start = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START);

	if(texec_start != -1) {
		struct single_event* ts = &es->single_events[texec_start];

		for(; ts && ts < &es->single_events[es->num_single_events] && ts->time <= end; ts = ts->next_texec_start) {
			if(!filter_has_single_event(f, ts))
				continue;

			dump_node(fp, ts);
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

			if(!fr_next_exec)
				fprintf(stderr, "Warning: Could not find next texec for frame %"PRIx64"\n", ce->what->addr);
			else
				dump_write_edge(fp, ce->active_frame, ce->texec_start->time, ce->what, fr_next_exec->time, ce->size, mes->max_write_size);
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
					if(sge->prev_texec_start)
						dump_tcreate_edge(fp, sge->active_frame, sge->prev_texec_start->time, sge->what, fr_next_exec->time);
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

int add_texecs_downward(FILE* fp, struct texec_tree* tt, struct multi_event_set* mes, struct single_event* texec_start, struct single_event* parent_texec, struct comm_event* parent_write, unsigned int depth_down)
{
	if(depth_down == 0)
		return 0;

	if(parent_texec) {
		dump_tcreate_edge(fp, parent_texec->active_frame,
				  parent_texec->time,
				  texec_start->what,
				  texec_start->time);
	}

	if(parent_write) {
		dump_write_edge(fp, parent_write->active_frame,
				parent_write->texec_start->time,
				texec_start->what,
				texec_start->time,
				parent_write->size,
				mes->max_write_size);
	}

	if(texec_tree_find(tt, texec_start->active_frame, texec_start))
		return 0;

	texec_tree_add(tt, texec_start->active_frame, texec_start);

	if(texec_start->next_texec_end) {
		for(struct single_event* se = texec_start; se < se->next_texec_end; se++) {
			if(se->type == SINGLE_TYPE_TCREATE) {
				struct single_event* fr_next_exec = multi_event_set_find_next_texec_start_for_frame(mes, se->time, se->what);

				if(fr_next_exec)
					if(add_texecs_downward(fp, tt, mes, fr_next_exec, texec_start, NULL, depth_down-1))
						return 1;
			}
		}
	}

	if(texec_start->next_texec_end) {
			struct event_set* es = texec_start->event_set;
			struct single_event* texec_end = texec_start->next_texec_end;

			int comm_event = event_set_get_first_comm_in_interval(es, texec_start->time, texec_end->time);

			for(; comm_event != -1 && comm_event < es->num_comm_events; comm_event++) {
				struct comm_event* ce = &es->comm_events[comm_event];

				if(ce->time > texec_end->time)
					break;

				if(ce->type == COMM_TYPE_DATA_WRITE) {
					struct single_event* fr_next_exec = multi_event_set_find_next_texec_start_for_frame(mes, ce->time, ce->what);

					if(!fr_next_exec) {
						fprintf(stderr, "Warning: Could not find next texec for frame %"PRIx64"\n", ce->what->addr);
					} else {
						if(add_texecs_downward(fp, tt, mes, fr_next_exec, NULL, ce, depth_down-1))
							return 1;
					}
				}
			}
	}

	return 0;
}

void texec_tree_dump_element(struct texec_key* key, void* arg)
{
	FILE* fp = (FILE*)arg;
	dump_node(fp, key->texec_start);
}

int export_task_graph_selected_texec(const char* outfile, struct multi_event_set* mes, struct single_event* texec_start, unsigned int depth_down)
{
	int ret = 1;
	FILE* fp = fopen(outfile, "w+");

	if(!fp)
		goto out;

	fprintf(fp, "digraph task_graph {\n");

	struct texec_tree tt;
	texec_tree_init(&tt);

	if(add_texecs_downward(fp, &tt, mes, texec_start, NULL, NULL, depth_down))
		goto out_tt;

	texec_tree_walk_ascending(&tt, texec_tree_dump_element, fp);

	fprintf(fp, "}\n");

	ret = 0;

out_tt:
	texec_tree_destroy(&tt);
out_fp:
	fclose(fp);
out:
	return ret;
}
