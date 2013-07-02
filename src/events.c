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
#include "filter.h"
#include "task.h"
#include "multi_event_set.h"
#include <stdlib.h>
#include <unistd.h>

int read_trace_samples(struct multi_event_set* mes, struct task_tree* tt, FILE* fp, off_t* bytes_read)
{
	struct trace_event_header dsk_eh;
	struct trace_state_event dsk_se;
	struct trace_comm_event dsk_ce;
	struct trace_single_event dsk_sge;
	struct trace_counter_description dsk_cd;
	struct trace_counter_event dsk_cre;

	struct event_set* es;
	struct state_event se;
	struct comm_event ce;
	struct single_event sge;
	struct counter_description* cd;
	struct counter_event cre;

	struct task* last_task = NULL;

	while(!feof(fp)) {
		if(bytes_read)
			*bytes_read = lseek(fileno(fp), 0, SEEK_CUR);

		if(read_uint32_convert(fp, &dsk_eh.type) != 0) {
			if(feof(fp))
				return 0;
			else
				return 1;
		}

		if(dsk_eh.type == EVENT_TYPE_COUNTER_DESCRIPTION) {
			dsk_cd.type = dsk_eh.type;

			if(read_struct_convert(fp, &dsk_cd, sizeof(dsk_cd), trace_counter_description_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			/* Counter description already read? */
			if(multi_event_set_find_counter_description(mes, dsk_cd.counter_id) != NULL)
				return 1;

			if((cd = multi_event_set_counter_description_alloc_ptr(mes, dsk_cd.counter_id, dsk_cd.name_len)) == NULL)
				return 1;

			if(fread(cd->name, dsk_cd.name_len, 1, fp) != 1)
				return 1;

			cd->name[dsk_cd.name_len] = '\0';
		} else {
			if(read_struct_convert(fp, &dsk_eh, sizeof(dsk_eh), trace_event_header_conversion_table, sizeof(dsk_eh.type)) != 0)
					return 1;

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
			} else if(dsk_eh.type == EVENT_TYPE_COUNTER) {
				memcpy(&dsk_cre, &dsk_eh, sizeof(dsk_eh));
				if(read_struct_convert(fp, &dsk_cre, sizeof(dsk_cre), trace_counter_event_conversion_table, sizeof(dsk_eh)) != 0)
					return 1;

				if(!(cd = multi_event_set_find_counter_description(mes, dsk_cre.counter_id)))
					return 1;

				es = multi_event_set_find_alloc_cpu(mes, dsk_cre.header.cpu);
				cre.active_task = dsk_cre.header.active_task;
				cre.time = dsk_cre.header.time;
				cre.counter_id = dsk_cre.counter_id;
				cre.counter_index = cd->index;
				cre.value = dsk_cre.value;

				if(event_set_add_counter_event(es, &cre) != 0)
					return 1;

				multi_event_set_check_update_counter_bounds(mes, &cre);
			}
		}
	}

	if(bytes_read)
		*bytes_read = lseek(fileno(fp), 0, SEEK_CUR);

	return 0;
}

int read_trace_sample_file(struct multi_event_set* mes, const char* file, off_t* bytes_read)
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

	if(read_trace_samples(mes, &tt, fp, bytes_read) != 0)
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
	if(bytes_read)
		*bytes_read = lseek(fileno(fp), 0, SEEK_CUR);

	fclose(fp);
out:
	return res;
}
