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
#include "color.h"
#include <inttypes.h>

void dump_tcreate_edge(FILE* fp,
		       uint64_t task_addr_creator, int cpu_creator, uint64_t time_creator,
		       uint64_t task_addr_created, int cpu_created, uint64_t time_created)
{
	fprintf(fp, "\t\"%"PRIx64"_%d_%"PRId64"\" -> \"%"PRIx64"_%d_%"PRId64"\" [style = dotted];\n",
		task_addr_creator, cpu_creator, time_creator,
		task_addr_created, cpu_created, time_created);
}

void dump_write_edge(FILE* fp, struct task_instance_rw_tree_node* nwriter, int max_size)
{
	double min_pen_width = 1.0;
	double max_pen_width = 5.0;
	uint64_t size = address_range_tree_node_size(nwriter->address_range_node);
	struct task_instance_rw_tree_node* nreader = nwriter->prodcons_counterpart;

	double pen_width = min_pen_width;

	if(max_size > 0)
		pen_width = min_pen_width + (max_pen_width - min_pen_width)* ((long double)size) / ((long double)max_size);

	fprintf(fp, "\t\"%"PRIx64"_%d_%"PRId64"\" -> \"%"PRIx64"_%d_%"PRId64"\" [penwidth = %f];\n",
		nwriter->instance->task->addr,
		nwriter->instance->cpu,
		nwriter->instance->start,
		nreader->instance->task->addr,
		nreader->instance->cpu,
		nreader->instance->start,
		pen_width);
}

void dump_node(FILE* fp, struct single_event* texec_start, struct filter* f, int max_numa_node_id, uint64_t size, int highlight)
{
	char buff_col_major_read[8];
	char buff_col_major_write[8];
	char buff_col_major_both[8];
	char buff_col_exec[8];
	int major_node_both;
	int major_node_read;
	int major_node_write;

	if(!event_set_get_major_accessed_node_in_interval(texec_start->event_set, f,
							 texec_start->time-1,
							 texec_start->next_texec_end->time,
							 max_numa_node_id, &major_node_both))
	{
		major_node_both = texec_start->event_set->numa_node;
	}

	if(!event_set_get_major_read_node_in_interval(texec_start->event_set, f,
						      texec_start->time-1,
						      texec_start->next_texec_end->time,
						      max_numa_node_id, &major_node_read))
	{
		major_node_read = texec_start->event_set->numa_node;
	}

	if(!event_set_get_major_read_node_in_interval(texec_start->event_set, f,
						      texec_start->time-1,
						      texec_start->next_texec_end->time,
						      max_numa_node_id, &major_node_write))
	{
		major_node_write = texec_start->event_set->numa_node;
	}

	get_node_color_htmlrgb(major_node_both, max_numa_node_id, buff_col_major_both);
	get_node_color_htmlrgb(major_node_read, max_numa_node_id, buff_col_major_read);
	get_node_color_htmlrgb(major_node_write, max_numa_node_id, buff_col_major_write);
	get_node_color_htmlrgb(texec_start->event_set->numa_node, max_numa_node_id, buff_col_exec);

	fprintf(fp, "\t\"%"PRIx64"_%d_%"PRId64"\""
		"[shape=\"none\", label=<<table border=\"1\" cellspacing=\"0\">"
		"<tr><td bgcolor=\"%s\">A=%d</td><td bgcolor=\"%s\">X=%d</td></tr>"
		"<tr><td bgcolor=\"%s\">R=%d</td><td bgcolor=\"%s\">W=%d</td></tr>"
		"<tr><td colspan=\"2\" bgcolor=\"%s\">S=%"PRIu64"</td></tr></table>>];\n",
		texec_start->active_task->addr,
		texec_start->event_set->cpu,
		texec_start->time,
		buff_col_major_both, major_node_both,
		buff_col_exec, texec_start->event_set->numa_node,
		buff_col_major_read, major_node_read,
		buff_col_major_write, major_node_write,
		(highlight) ? "#ffff00" : "#ffffff",
		size);
}

void export_nodes_event_set(FILE* fp, struct address_range_tree* art, struct multi_event_set* mes, struct event_set* es, struct filter* f, int64_t start, int64_t end)
{
	int texec_start = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START);

	if(texec_start != -1) {
		struct single_event* ts = &es->single_events[texec_start];

		for(; ts && ts < &es->single_events[es->num_single_events] && ts->time <= end; ts = ts->next_texec_start) {
			if(!filter_has_single_event(f, ts))
				continue;

			struct task_instance* inst = task_instance_tree_find(&art->all_instances, ts->active_task->addr, es->cpu, ts->time);

			dump_node(fp, ts, f, mes->max_numa_node_id, inst->num_read_deps, 0);
		}

		ts = &es->single_events[texec_start];
	}
}

void export_task_graph_task_instance(FILE* fp, struct task_instance* inst, struct multi_event_set* mes, struct filter* f)
{
	struct list_head* iter;

	list_for_each(iter, &inst->list_out_deps) {
		struct task_instance_rw_tree_node* tin =
			list_entry(iter, struct task_instance_rw_tree_node, list_out_deps);

		if(filter_has_comm_event(f, mes, tin->comm_event))
			dump_write_edge(fp, tin, mes->max_write_size);
	}
}

void export_task_graph_event_set(FILE* fp, struct multi_event_set* mes, struct address_range_tree* art, struct event_set* es, struct filter* f, int64_t start, int64_t end)
{
	int texec_start_idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START);

	if(texec_start_idx == -1)
		return;

	struct single_event* texec_start = &es->single_events[texec_start_idx];

	for(; texec_start && texec_start->time < end; texec_start = texec_start->next_texec_start) {
		struct task_instance* inst = task_instance_tree_find(&art->all_instances, texec_start->active_task->addr, texec_start->event_set->cpu, texec_start->time);

		if(!inst) {
			fprintf(stderr, "Could not find task instance for task 0x%"PRIx64" @ %"PRIu64" on cpu %d\n",
				texec_start->active_task->addr, texec_start->time, texec_start->event_set->cpu);
		} else {
			export_task_graph_task_instance(fp, inst, mes, f);
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
						dump_tcreate_edge(fp, sge->active_task->addr, sge->event_set->cpu, sge->prev_texec_start->time,
								  fr_next_exec->active_task->addr, fr_next_exec->event_set->cpu, fr_next_exec->time);
					}
				}
			}
		}
	}
}

int export_task_graph(const char* outfile, struct multi_event_set* mes, struct address_range_tree* art, struct filter* f, int64_t start, int64_t end)
{
	FILE* fp = fopen(outfile, "w+");

	if(!fp)
		return 1;

	if(start < 0)
		start = 0;

	if(end < 0)
		end = 0;

	fprintf(fp, "digraph task_graph {\n");

	for(struct event_set* es = &mes->sets[0]; es < &mes->sets[mes->num_sets]; es++)
		export_nodes_event_set(fp, art, mes, es, f, start, end);

	for(struct event_set* es = &mes->sets[0]; es < &mes->sets[mes->num_sets]; es++)
		export_task_graph_event_set(fp, mes, art, es, f, start, end);

	fprintf(fp, "}\n");

	fclose(fp);

	return 0;
}

void export_task_graph_task_instance_up_down(FILE* fp, struct multi_event_set* mes, struct address_range_tree* art, struct filter* f, struct task_instance* inst, unsigned int depth_down, unsigned int depth_up, int highlight)
{
	struct list_head* iter;

	if(inst->reached)
		return;

	struct event_set* es = multi_event_set_find_cpu(mes, inst->cpu);
	int texec_start_idx = event_set_get_first_single_event_in_interval_type(es, inst->start-1, inst->end, SINGLE_TYPE_TEXEC_START);
	struct single_event* texec_start = &es->single_events[texec_start_idx];

	dump_node(fp, texec_start, f, mes->max_numa_node_id, inst->num_read_deps, highlight);
	inst->reached = 1;

	if(depth_up > 0) {
		list_for_each(iter, &inst->list_in_deps) {
			struct task_instance_rw_tree_node* tin =
				list_entry(iter, struct task_instance_rw_tree_node, list_in_deps);

			if(filter_has_comm_event(f, mes, tin->comm_event)) {
				dump_write_edge(fp, tin->prodcons_counterpart, mes->max_write_size);
				export_task_graph_task_instance_up_down(fp, mes, art, f, tin->prodcons_counterpart->instance, 0, depth_up-1, 0);
			}
		}
	}

	if(depth_down > 0) {
		list_for_each(iter, &inst->list_out_deps) {
			struct task_instance_rw_tree_node* tin =
				list_entry(iter, struct task_instance_rw_tree_node, list_out_deps);

			if(filter_has_comm_event(f, mes, tin->comm_event)) {
				dump_write_edge(fp, tin, mes->max_write_size);
				export_task_graph_task_instance_up_down(fp, mes, art, f, tin->prodcons_counterpart->instance, depth_down-1, 0, 0);
			}
		}
	}

	inst->reached = 1;
}

void export_task_graph_task_instance_tcreate_up_down(FILE* fp, struct multi_event_set* mes, struct address_range_tree* art, struct filter* f, struct task_instance* inst, unsigned int depth_down, unsigned int depth_up)
{	struct list_head* iter;

	if(inst->reached == 2)
		return;

	inst->reached = 2;

	if(filter_has_single_event_type(f, SINGLE_TYPE_TCREATE)) {
		struct event_set* es = multi_event_set_find_cpu(mes, inst->cpu);
		int single_event = event_set_get_first_single_event_in_interval_type(es, inst->start, inst->end, SINGLE_TYPE_TCREATE);

		for(; single_event != -1 && single_event < es->num_single_events; single_event++) {
			struct single_event* sge = &es->single_events[single_event];

			if(sge->time > inst->end)
				break;

			if(sge->type == SINGLE_TYPE_TCREATE) {
				if(!filter_has_single_event(f, sge))
					continue;

				struct single_event* texec_start = multi_event_set_find_next_texec_start_for_frame(mes, sge->time, sge->what);

				if(!texec_start) {
					fprintf(stderr, "Warning: Could not find next texec for frame %"PRIx64"\n", sge->what->addr);
				} else {
					struct task_instance* inst_created = task_instance_tree_find(&art->all_instances, texec_start->active_task->addr, texec_start->event_set->cpu, texec_start->time);

					if(!inst_created) {
						fprintf(stderr, "Could not find task instance for task 0x%"PRIx64" @ %"PRIu64" on cpu %d\n",
							texec_start->active_task->addr, texec_start->time, texec_start->event_set->cpu);
					} else {
						if(inst_created->reached && sge->prev_texec_start) {
							dump_tcreate_edge(fp, sge->active_task->addr, sge->event_set->cpu, sge->prev_texec_start->time,
									  texec_start->active_task->addr, texec_start->event_set->cpu, texec_start->time);
						}
					}
				}
			}
		}
	}

	if(depth_up > 0) {
		list_for_each(iter, &inst->list_in_deps) {
			struct task_instance_rw_tree_node* tin =
				list_entry(iter, struct task_instance_rw_tree_node, list_in_deps);

			if(filter_has_comm_event(f, mes, tin->comm_event))
				export_task_graph_task_instance_tcreate_up_down(fp, mes, art, f, tin->prodcons_counterpart->instance, 0, depth_up-1);
		}
	}

	if(depth_down > 0) {
		list_for_each(iter, &inst->list_out_deps) {
			struct task_instance_rw_tree_node* tin =
				list_entry(iter, struct task_instance_rw_tree_node, list_out_deps);

			if(filter_has_comm_event(f, mes, tin->comm_event))
				export_task_graph_task_instance_tcreate_up_down(fp, mes, art, f, tin->prodcons_counterpart->instance, depth_down-1, 0);
		}
	}
}

int export_task_graph_selected_texec(const char* outfile, struct multi_event_set* mes, struct address_range_tree* art, struct filter* f, struct single_event* texec_start, unsigned int depth_down, unsigned int depth_up)
{
	int ret = 1;
	FILE* fp = fopen(outfile, "w+");

	if(!fp)
		goto out;

	fprintf(fp, "digraph task_graph {\n");

	struct task_instance* inst = task_instance_tree_find(&art->all_instances, texec_start->active_task->addr, texec_start->event_set->cpu, texec_start->time);

	if(!inst) {
		fprintf(stderr, "Could not find task instance for task 0x%"PRIx64" @ %"PRIu64" on cpu %d\n",
				texec_start->active_task->addr, texec_start->time, texec_start->event_set->cpu);
		goto out_fp;
	}

	task_instance_tree_reset_reached(&art->all_instances);
	export_task_graph_task_instance_up_down(fp, mes, art, f, inst, depth_down, depth_up, 1);
	export_task_graph_task_instance_tcreate_up_down(fp, mes, art, f, inst, depth_down, depth_up);

	fprintf(fp, "}\n");

	ret = 0;

out_fp:
	fclose(fp);
out:
	return ret;
}
