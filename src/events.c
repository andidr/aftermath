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

#include "events.h"
#include <stdlib.h>

static int comm_event_compare_time(const void* p1, const void* p2)
{
	const struct comm_event* s1 = p1;
	const struct comm_event* s2 = p2;

	if(s1->time > s2->time)
		return 1;
	else if(s1->time < s2->time)
		return -1;

	return 0;
}

void event_set_sort_comm(struct event_set* es)
{
	qsort(es->comm_events, es->num_comm_events, sizeof(struct comm_event), comm_event_compare_time);
}

static int event_set_compare_cpus(const void* p1, const void* p2)
{
	const struct event_set* s1 = p1;
	const struct event_set* s2 = p2;

	if(s1->cpu > s2->cpu)
		return 1;
	else if(s1->cpu < s2->cpu)
		return -1;

	return 0;
}

void multi_event_set_sort_by_cpu(struct multi_event_set* mes)
{
	qsort(mes->sets, mes->num_sets, sizeof(struct event_set), event_set_compare_cpus);
}

int event_set_get_first_state_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_state_events;
	int center_idx = 0;

	if(es->num_state_events == 0)
		return -1;

	while(end_idx - start_idx > 1) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->state_events[center_idx].start > interval_end)
			end_idx = center_idx;
		else if(es->state_events[center_idx].end < interval_start)
			start_idx = center_idx;
		else
			break;
	}

	while(center_idx > 0 && es->state_events[center_idx-1].start < interval_end && es->state_events[center_idx-1].end > interval_start)
		center_idx--;

	return center_idx;
}

int event_set_get_first_comm_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_comm_events;
	int center_idx = 0;

	if(es->num_comm_events == 0)
		return -1;

	while(end_idx - start_idx > 1) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->comm_events[center_idx].time > interval_end)
			end_idx = center_idx;
		else if(es->comm_events[center_idx].time < interval_start)
			start_idx = center_idx;
		else
			break;
	}

	while(center_idx > 0 && es->comm_events[center_idx-1].time < interval_end && es->comm_events[center_idx-1].time > interval_start)
		center_idx--;

	return center_idx;
}

int event_set_get_first_single_event_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_single_events;
	int center_idx = 0;

	if(es->num_single_events == 0)
		return -1;

	while(end_idx - start_idx > 1) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->single_events[center_idx].time > interval_end)
			end_idx = center_idx;
		else if(es->single_events[center_idx].time < interval_start)
			start_idx = center_idx;
		else
			break;
	}

	while(center_idx > 0 && es->single_events[center_idx-1].time < interval_end && es->single_events[center_idx-1].time > interval_start)
		center_idx--;

	return center_idx;
}

int read_trace_samples(struct multi_event_set* mes, struct task_tree* tt, FILE* fp)
{
	struct trace_event_header dsk_eh;
	struct trace_state_event dsk_se;
	struct trace_comm_event dsk_ce;
	struct trace_single_event dsk_sge;

	struct event_set* es;
	struct state_event se;
	struct comm_event ce;
	struct single_event sge;

	struct task* last_task = NULL;

	while(!feof(fp)) {
		if(read_struct_convert(fp, &dsk_eh, sizeof(dsk_eh), trace_event_header_conversion_table, 0) != 0) {
			if(feof(fp))
				return 0;
			else
				return 1;
		}

		if(!last_task || (last_task->work_fn != dsk_eh.active_task && !task_tree_find(tt, dsk_eh.active_task)))
			last_task = task_tree_add(tt, dsk_eh.active_task);

		if(dsk_eh.type == EVENT_TYPE_STATE) {
			memcpy(&dsk_se, &dsk_eh, sizeof(dsk_eh));
			if(read_struct_convert(fp, &dsk_se, sizeof(dsk_se), trace_state_event_conversion_table, sizeof(dsk_eh)) != 0)
				return 1;

			es = multi_event_set_find_alloc_cpu(mes, dsk_se.header.cpu);
			se.start = dsk_se.header.time;
			se.end = dsk_se.end_time;
			se.state = dsk_se.state;
			se.active_task = dsk_se.header.active_task;
			event_set_add_state_event(es, &se);
		} else if(dsk_eh.type == EVENT_TYPE_COMM) {
			memcpy(&dsk_ce, &dsk_eh, sizeof(dsk_eh));
			if(read_struct_convert(fp, &dsk_ce, sizeof(dsk_ce), trace_comm_event_conversion_table, sizeof(dsk_eh)) != 0)
				return 1;

			es = multi_event_set_find_alloc_cpu(mes, dsk_ce.header.cpu);
			ce.time = dsk_ce.header.time;
			ce.active_task = dsk_ce.header.active_task;
			ce.dst_cpu = dsk_ce.dst_cpu;
			ce.dst_worker = dsk_ce.dst_worker;
			ce.size = dsk_ce.size;
			ce.type = dsk_ce.type;
			ce.what = dsk_ce.what;
			event_set_add_comm_event(es, &ce);
		} else if(dsk_eh.type == EVENT_TYPE_SINGLE) {
			memcpy(&dsk_sge, &dsk_eh, sizeof(dsk_eh));
			if(read_struct_convert(fp, &dsk_sge, sizeof(dsk_sge), trace_single_event_conversion_table, sizeof(dsk_eh)) != 0)
				return 1;

			es = multi_event_set_find_alloc_cpu(mes, dsk_sge.header.cpu);
			sge.active_task = dsk_sge.header.active_task;
			sge.time = dsk_sge.header.time;
			sge.type = dsk_sge.type;
			event_set_add_single_event(es, &sge);
		}
	}

	return 0;
}

int compare_tasks(const void *pt1, const void *pt2)
{
	const struct task* t1 = pt1;
	const struct task* t2 = pt2;

	if(t1->work_fn < t2->work_fn)
		return -1;
	else if(t1->work_fn > t2->work_fn)
		return 1;

	return 0;
}

/* Ugly thread local variables needed for reentrance of twalk */
__thread struct task* curr_tt_array;
__thread int curr_tt_array_index;

void task_tree_walk(const void* p, const VISIT which, const int depth)
{
	if(which == leaf) {
		struct task* t = *((struct task**)p);
		memcpy(&curr_tt_array[curr_tt_array_index++], t, sizeof(struct task));
	}
}

int task_tree_to_array(struct task_tree* tt, struct task** arr)
{
	if(!(*arr = malloc(tt->num_tasks*sizeof(struct task))))
		return 1;

	curr_tt_array = *arr;
	curr_tt_array_index = 0;

	twalk(tt->root, task_tree_walk);
	return 0;
}

int read_trace_sample_file(struct multi_event_set* mes, const char* file)
{
	struct trace_header header;
	struct task_tree tt;

	int res = 1;
	FILE* fp;

	if(!(fp = fopen(file, "r")))
	   goto out;

	if(read_struct_convert(fp, &header, sizeof(header), trace_header_conversion_table, 0) != 0)
		goto out_fp;

	if(!trace_verify_header(&header))
		goto out_fp;

	task_tree_init(&tt);

	if(read_trace_samples(mes, &tt, fp) != 0)
		goto out_fp;

	multi_event_set_sort_by_cpu(mes);

	if(task_tree_to_array(&tt, &mes->tasks) != 0)
		goto out_tt;

	mes->num_tasks = tt.num_tasks;

	for(int i = 0; i < mes->num_sets; i++)
		event_set_sort_comm(&mes->sets[i]);

	res = 0;

out_tt:
	task_tree_destroy(&tt);
out_fp:
	fclose(fp);
out:
	printf("OUT with code = %d\n", res);
	return res;
}
