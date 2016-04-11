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

#include "omp_for.h"
#include "omp_for_instance.h"
#include "omp_for_chunk_set.h"
#include "omp_for_chunk_set_part.h"
#include "omp_task.h"
#include "omp_task_instance.h"
#include "omp_task_part.h"
#include "events.h"
#include "filter.h"
#include "task.h"
#include "multi_event_set.h"
#include "trace/convert.h"
#include "uncompress.h"
#include "color.h"
#include <stdlib.h>
#include <unistd.h>

int has_nonexecuted_omp_for_chunk_sets(struct multi_event_set* mes)
{
	for(int i = 0; i < mes->num_omp_for_chunk_sets; i++)
		if(mes->omp_for_chunk_sets[i].num_chunk_set_parts == 0)
			return 1;

	return 0;
}

int has_empty_omp_for_instances(struct multi_event_set* mes)
{
	for(int i = 0; i < mes->num_omp_for_instances; i++)
		if(mes->omp_for_instances[i].num_chunk_sets == 0)
			return 1;

	return 0;
}

int has_empty_omp_for(struct multi_event_set* mes)
{
	for(int i = 0; i < mes->num_omp_fors; i++)
		if(mes->omp_fors[i].num_instances == 0)
			return 1;

	return 0;
}

int chain_omp_for_structs(struct multi_event_set* mes)
{
	struct omp_for* of;
	struct omp_for_instance* ofi;
	struct omp_for_chunk_set* ofc;
	struct omp_for_chunk_set_part* ofcp;
	struct event_set* es;

	for(int i = 0; i < mes->num_sets; i++) {
		es = &mes->sets[i];

		for(int j = 0; j < es->num_omp_for_chunk_set_parts; j++) {
			ofcp = &es->omp_for_chunk_set_parts[j];
			if((ofc = multi_event_set_find_omp_for_chunk_set_by_id(mes, ofcp->ti_ofcp->chunk_set_id)) == NULL)
				return 1;
			free(ofcp->ti_ofcp);
			ofcp->chunk_set = ofc;
			if(ofc->chunk_set_parts.next == NULL)
				INIT_LIST_HEAD(&ofc->chunk_set_parts);
			list_add(&ofcp->list, &ofc->chunk_set_parts);
			ofc->num_chunk_set_parts++;
			ofcp->color_r = ofc->color_r;
			ofcp->color_g = ofc->color_g;
			ofcp->color_b = ofc->color_b;
		}
	}

	for(int i = 0; i < mes->num_omp_for_chunk_sets; i++) {
		ofc = &mes->omp_for_chunk_sets[i];
		if((ofi = multi_event_set_find_omp_for_instance_by_id(mes, ofc->ti_ofc->for_instance_id)) == NULL)
			return 1;
		free(ofc->ti_ofc);
		ofc->for_instance = ofi;
		if(ofi->for_chunk_sets.next == NULL)
			INIT_LIST_HEAD(&ofi->for_chunk_sets);
		list_add(&ofc->list, &ofi->for_chunk_sets);
		ofi->num_chunk_sets++;
	}

	for(int i = 0; i < mes->num_omp_for_instances; i++) {
		ofi = &mes->omp_for_instances[i];
		if((of = multi_event_set_find_omp_for_by_addr(mes, ofi->ti_ofi->addr)) == NULL)
			return 1;
		free(ofi->ti_ofi);
		ofi->for_loop = of;
		if(of->for_instances.next == NULL)
			INIT_LIST_HEAD(&of->for_instances);
		list_add(&ofi->list, &of->for_instances);
		of->num_instances++;
	}

	return 0;
}

int has_artifical_task_instances(struct multi_event_set* mes)
{
	for(int i = 0; i < mes->num_omp_task_instances; i++)
		if(mes->omp_task_instances[i].ti_oti->artificial)
			return 1;

	return 0;
}

int has_nonexecuted_task_instances(struct multi_event_set* mes)
{
	for(int i = 0; i < mes->num_omp_task_instances; i++)
		if(mes->omp_task_instances[i].num_task_parts == 0)
			return 1;

	return 0;
}

int chain_omp_task_structs(struct multi_event_set* mes)
{
	struct omp_task* ot;
	struct omp_task_instance* oti;
	struct omp_task_part* otp;
	struct event_set* es;

	for(int i = 0; i < mes->num_omp_tasks; i++) {
		ot = &mes->omp_tasks[i];
		INIT_LIST_HEAD(&ot->task_instances);
		ot->source_filename = NULL;
		ot->color_r = omp_task_colors[ot->addr % NUM_OMP_TASK_COLORS][0];
		ot->color_g = omp_task_colors[ot->addr % NUM_OMP_TASK_COLORS][1];
		ot->color_b = omp_task_colors[ot->addr % NUM_OMP_TASK_COLORS][2];
	}

	for(int i = 0; i < mes->num_omp_task_instances; i++) {
		oti = &mes->omp_task_instances[i];
		INIT_LIST_HEAD(&oti->task_parts);
		ot = &mes->omp_tasks[oti->ti_oti->task_id];
		free(oti->ti_oti);
		oti->task = ot;
		list_add(&oti->list, &ot->task_instances);
		oti->color_r = omp_task_colors[((uintptr_t)oti >> sizeof(uintptr_t)) % NUM_OMP_TASK_COLORS][0];
		oti->color_g = omp_task_colors[((uintptr_t)oti >> sizeof(uintptr_t)) % NUM_OMP_TASK_COLORS][1];
		oti->color_b = omp_task_colors[((uintptr_t)oti >> sizeof(uintptr_t)) % NUM_OMP_TASK_COLORS][2];
	}

	for(int i = 0; i < mes->num_sets; i++) {
		es = &mes->sets[i];

		for(int j = 0; j < es->num_omp_task_parts; j++) {
			otp = &es->omp_task_parts[j];
			oti = &mes->omp_task_instances[otp->ti_otp->task_instance_id];
			free(otp->ti_otp);
			otp->task_instance = oti;
			list_add(&otp->list, &oti->task_parts);
			otp->color_r = oti->color_r;
			otp->color_g = oti->color_g;
			otp->color_b = oti->color_b;
		}
	}

	return 0;
}

int trace_update_task_execution_bounds(struct event_set* es)
{
	struct single_event* ese;
	struct single_event* eee;

	struct single_event* last_ese = NULL;
	struct single_event* last_eee = NULL;

	struct state_event* se;

	int exec_start_idx = -1;
	int exec_end_idx = -1;
	int state_idx;

	/* Find execution start event */
	while((exec_start_idx = event_set_get_next_single_event(es, exec_end_idx, SINGLE_TYPE_TEXEC_START)) != -1) {
		/* Find execution end event */
		if((exec_end_idx = event_set_get_next_single_event(es, exec_start_idx, SINGLE_TYPE_TEXEC_END)) == -1)
			return 1;

		/* Get pointers to events */
		ese = &es->single_events[exec_start_idx];
		eee = &es->single_events[exec_end_idx];

		/* Check if events' frames match */
		if(ese->what != eee->what)
			return 1;

		/* Update event chain */
		ese->next_texec_end = eee;
		ese->prev_texec_end = last_eee;
		ese->prev_texec_start = last_ese;

		eee->prev_texec_start = ese;
		eee->next_texec_start = NULL;
		eee->prev_texec_end = last_eee;

		if(last_eee)
			last_eee->next_texec_start = ese;

		if(last_ese)
			last_ese->next_texec_start = ese;

		/* Update all single events before first task execution */
		if(!last_ese) {
			for(int i = 0; i < exec_start_idx; i++)
				es->single_events[i].next_texec_start = ese;
		}

		/* Update single events */
		for(int inbetween_idx = exec_start_idx+1; inbetween_idx < exec_end_idx && inbetween_idx < es->num_single_events; inbetween_idx++) {
			es->single_events[inbetween_idx].next_texec_end = eee;
			es->single_events[inbetween_idx].prev_texec_start = ese;
		}

		/* Update forward references for next_texec_start since last_ese */
		if(last_ese) {
			for(struct single_event* sge = last_ese+1; sge < ese; sge++)
				sge->next_texec_start = ese;
		}

		/* Update forward references for next_texec_end since last_eee */
		if(last_eee) {
			for(struct single_event* sge = last_eee+1; sge < eee; sge++)
				sge->next_texec_end = eee;
		}

		/* Update state events between start and end */
		if((state_idx = event_set_get_first_state_starting_in_interval(es, ese->time, eee->time)) != -1) {
			while(state_idx < es->num_state_events &&
			      es->state_events[state_idx].start < eee->time)
			{
				se = &es->state_events[state_idx];
				se->texec_start = ese;
				se->texec_end = eee;
				state_idx++;
			}
		}

		struct comm_event* ce;
		for_each_comm_event_in_interval(es, ese->time, eee->time, ce) {
			ce->texec_start = ese;
			ce->texec_end = eee;
		}

		last_ese = ese;
		last_eee = eee;
	}

	if(exec_end_idx != -1) {
		for(int i = exec_end_idx+1; i < es->num_single_events; i++) {
			es->single_events[i].prev_texec_start = last_ese;
			es->single_events[i].prev_texec_end = last_eee;
		}
	}

	return 0;
}

int read_trace_samples(struct multi_event_set* mes, struct task_tree* tt, struct frame_tree* ft, 
		       struct omp_for_chunk_set_tree* ofcst, struct omp_for_instance_tree* ofit, struct omp_for_tree* oft,
		       FILE* fp, off_t* bytes_read)
{
	struct trace_event_header dsk_eh;
	struct trace_state_event dsk_se;
	struct trace_comm_event dsk_ce;
	struct trace_single_event dsk_sge;
	struct trace_counter_description dsk_cd;
	struct trace_counter_event dsk_cre;
	struct trace_frame_info dsk_fi;
	struct trace_cpu_info dsk_ci;
	struct trace_global_single_event dsk_gse;
	struct trace_state_description dsk_sd;
	struct trace_omp_for_instance dsk_ofi;
	struct trace_omp_for_chunk_set dsk_ofc;
	struct trace_omp_for_chunk_set_part dsk_ofcp;
	struct trace_omp_task_instance dsk_oti;
	struct trace_omp_task_instance_part dsk_otp;

	struct event_set* es;
	struct state_event se;
	struct comm_event ce;
	struct single_event sge;
	struct counter_description* cd;
	struct counter_event cre;
	struct global_single_event gse;
	struct state_description* sd;
	struct omp_for* of;
	struct omp_for_instance* ofi;
	struct omp_for_chunk_set* ofc;
	struct omp_for_chunk_set_part ofcp;
	struct omp_task* ot;
	struct omp_task_instance* oti;
	struct omp_task_part otp;
	struct omp_task_part* otp_p;

	struct task* last_task = NULL;
	struct frame* last_frame = NULL;
	struct frame* last_what = NULL;

	while(!feof(fp)) {
		if(read_uint32_convert(fp, &dsk_eh.type) != 0) {
			if(feof(fp))
				return 0;
			else
				return 1;
		}

		(*bytes_read) += sizeof(uint32_t);

		if(dsk_eh.type == EVENT_TYPE_COUNTER_DESCRIPTION) {
			dsk_cd.type = dsk_eh.type;

			if(read_struct_convert(fp, &dsk_cd, sizeof(dsk_cd), trace_counter_description_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			(*bytes_read) += sizeof(dsk_cd) - sizeof(dsk_eh.type);

			/* Counter description already read? */
			if(multi_event_set_find_counter_description(mes, dsk_cd.counter_id) != NULL)
				return 1;

			if((cd = multi_event_set_counter_description_alloc_ptr(mes, dsk_cd.counter_id, dsk_cd.name_len)) == NULL)
				return 1;

			if(fread(cd->name, dsk_cd.name_len, 1, fp) != 1)
				return 1;

			cd->name[dsk_cd.name_len] = '\0';
		} else if(dsk_eh.type == EVENT_TYPE_GLOBAL_SINGLE_EVENT) {
			dsk_gse.type = dsk_eh.type;

			if(read_struct_convert(fp, &dsk_gse, sizeof(dsk_gse), trace_global_single_event_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			gse.time = dsk_gse.time;
			gse.type = dsk_gse.single_type;

			(*bytes_read) += sizeof(dsk_gse) - sizeof(dsk_eh.type);

			if(multi_event_set_add_global_single_event(mes, &gse))
				return 1;
		} else if (dsk_eh.type == EVENT_TYPE_STATE_DESCRIPTION) {
			dsk_sd.type = dsk_eh.type;

			if(read_struct_convert(fp, &dsk_sd, sizeof(dsk_sd), trace_state_description_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			(*bytes_read) += sizeof(dsk_sd) - sizeof(dsk_eh.type);

			/* State description already read? */
			if((sd = multi_event_set_find_state_description(mes, dsk_sd.state_id)) != NULL) {
				if(!sd->artificial)
					return 1;

				if(state_description_realloc_name(sd, dsk_sd.name_len))
					return 1;
			} else if((sd = multi_event_set_state_description_alloc_ptr(mes, dsk_sd.state_id, dsk_sd.name_len)) == NULL)
				return 1;

			if(fread(sd->name, dsk_sd.name_len, 1, fp) != 1)
				return 1;

			sd->name[dsk_sd.name_len] = '\0';

			sd->color_r = state_colors[sd->state_id % NUM_STATE_COLORS][0];
			sd->color_g = state_colors[sd->state_id % NUM_STATE_COLORS][1];
			sd->color_b = state_colors[sd->state_id % NUM_STATE_COLORS][2];

			sd->artificial = 0;
		} else if(dsk_eh.type == EVENT_TYPE_OMP_FOR) {
			if(read_struct_convert(fp, &dsk_ofi, sizeof(dsk_ofi), trace_omp_for_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			(*bytes_read) += sizeof(dsk_ofi) - sizeof(dsk_eh.type);

			if((of = omp_for_tree_find_or_add(oft, dsk_ofi.addr)) == NULL)
					return 1;

			omp_for_init(of, dsk_ofi.addr);


			if((ofi = omp_for_instance_tree_find_or_add(ofit, dsk_ofi.id)) == NULL)
					return 1;

			omp_for_instance_init(ofi, dsk_ofi.flags, dsk_ofi.id,
					      dsk_ofi.increment, dsk_ofi.lower_bound,
					      dsk_ofi.upper_bound, dsk_ofi.addr, dsk_ofi.num_workers);

		} else if(dsk_eh.type == EVENT_TYPE_OMP_FOR_CHUNK_SET) {
			if(read_struct_convert(fp, &dsk_ofc, sizeof(dsk_ofc), trace_omp_for_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			(*bytes_read) += sizeof(dsk_ofc) - sizeof(dsk_eh.type);

			if((ofc = omp_for_chunk_set_tree_find_or_add(ofcst, dsk_ofc.id)) == NULL)
					return 1;

			omp_for_chunk_set_init(ofc, dsk_ofc.for_id, dsk_ofc.id,
					       dsk_ofc.first_lower, dsk_ofc.first_upper);

		} else if(dsk_eh.type == EVENT_TYPE_OMP_FOR_CHUNK_SET_PART) {
			if(read_struct_convert(fp, &dsk_ofcp, sizeof(dsk_ofcp), trace_omp_for_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			(*bytes_read) += sizeof(dsk_ofcp) - sizeof(dsk_eh.type);

			if(!(es = multi_event_set_find_alloc_cpu(mes, dsk_ofcp.cpu)))
				return 1;

			omp_for_chunk_set_part_init(&ofcp, dsk_ofcp.chunk_set_id, dsk_ofcp.cpu,
						    dsk_ofcp.start, dsk_ofcp.end);

			if(omp_for_chunk_set_part_add(es, &ofcp))
				return 1;

		} else if(dsk_eh.type == EVENT_TYPE_OMP_TASK_INSTANCE) {
			if(read_struct_convert(fp, &dsk_oti, sizeof(dsk_oti), trace_omp_task_instance_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			(*bytes_read) += sizeof(dsk_oti) - sizeof(dsk_eh.type);

			if(!(ot = omp_task_find(mes, dsk_oti.addr)))
				if(!(ot = omp_task_alloc_ptr(mes, dsk_oti.addr)))
					return 1;

			if((oti = omp_task_instance_find(mes, dsk_oti.id))) {
				if(oti->ti_oti->artificial != 1)
					return 1;

				oti->ti_oti->artificial = 0;
				oti->ti_oti->oti_id = dsk_oti.id;
			} else if(!(oti = omp_task_instance_alloc_ptr(mes, dsk_oti.id, dsk_oti.addr)))
			{
				return 1;
			}

			oti->ti_oti->next_task_instance_id = ot->ti_ot.first_task_instance_id;
			ot->ti_ot.first_task_instance_id = oti - &mes->omp_task_instances[0];

			ot->num_instances++;

			oti->ti_oti->task_id = ot - &mes->omp_tasks[0];

		} else if(dsk_eh.type == EVENT_TYPE_OMP_TASK_INSTANCE_PART) {
			if(read_struct_convert(fp, &dsk_otp, sizeof(dsk_otp), trace_omp_task_instance_part_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			(*bytes_read) += sizeof(dsk_otp) - sizeof(dsk_eh.type);

			if(!(es = multi_event_set_find_alloc_cpu(mes, dsk_otp.cpu)))
				return 1;

			otp.ti_otp = malloc(sizeof(struct omp_trace_info_otp));
			otp.cpu = dsk_otp.cpu;
			otp.start = dsk_otp.start;
			otp.end = dsk_otp.end;
			otp.ti_otp->task_instance_id = dsk_otp.task_instance_id;

			if(omp_task_part_add(es, &otp))
				return 1;

			if(!(otp_p = omp_task_part_find(es, dsk_otp.cpu, dsk_otp.start)))
				return 1;

			if(!(oti = omp_task_instance_add_part(mes, &otp)))
				return 1;

			otp_p->ti_otp->task_instance_id = oti - &mes->omp_task_instances[0];

		} else {
			if(read_struct_convert(fp, &dsk_eh, sizeof(dsk_eh), trace_event_header_conversion_table, sizeof(dsk_eh.type)) != 0)
				return 1;

			(*bytes_read) += sizeof(dsk_eh) - sizeof(dsk_eh.type);

			if(!last_task || last_task->addr != dsk_eh.active_task)
				last_task = task_tree_find_or_add(tt, dsk_eh.active_task);

			if(!last_frame || last_frame->addr != dsk_eh.active_frame)
				last_frame = frame_tree_find_or_add(ft, dsk_eh.active_frame);

			if(dsk_eh.type == EVENT_TYPE_STATE) {
				memcpy(&dsk_se, &dsk_eh, sizeof(dsk_eh));
				if(read_struct_convert(fp, &dsk_se, sizeof(dsk_se), trace_state_event_conversion_table, sizeof(dsk_eh)) != 0)
					return 1;

				(*bytes_read) += sizeof(dsk_se) - sizeof(dsk_eh);

				if(!(es = multi_event_set_find_alloc_cpu(mes, dsk_se.header.cpu)))
					return 1;

				se.start = dsk_se.header.time;
				se.end = dsk_se.end_time;
				se.state_id = dsk_se.state;

				if((se.state_id_seq = multi_event_set_seq_state_id(mes, dsk_se.state)) == -1) {
					if((sd = multi_event_set_state_description_alloc_ptr(mes, dsk_se.state, strlen(STATE_UNKNOWN_NAME))) == NULL)
						return 1;

					sd->color_r = state_colors[sd->state_id % NUM_STATE_COLORS][0];
					sd->color_g = state_colors[sd->state_id % NUM_STATE_COLORS][1];
					sd->color_b = state_colors[sd->state_id % NUM_STATE_COLORS][2];
					sd->artificial = 1;

					strcpy(sd->name, STATE_UNKNOWN_NAME);

					se.state_id_seq = multi_event_set_seq_state_id(mes, dsk_se.state);
				}

				se.active_task = task_tree_find(tt, dsk_se.header.active_task);
				se.active_frame = frame_tree_find(ft, dsk_se.header.active_frame);
				se.texec_start = NULL;
				se.texec_end = NULL;
				event_set_add_state_event(es, &se);
			} else if(dsk_eh.type == EVENT_TYPE_COMM) {
				memcpy(&dsk_ce, &dsk_eh, sizeof(dsk_eh));
				if(read_struct_convert(fp, &dsk_ce, sizeof(dsk_ce), trace_comm_event_conversion_table, sizeof(dsk_eh)) != 0)
					return 1;

				(*bytes_read) += sizeof(dsk_ce) - sizeof(dsk_eh);

				if(!(es = multi_event_set_find_alloc_cpu(mes, dsk_ce.header.cpu)))
					return 1;

				ce.time = dsk_ce.header.time;
				ce.active_task = task_tree_find(tt, dsk_ce.header.active_task);
				ce.active_frame = frame_tree_find(ft, dsk_ce.header.active_frame);

				switch(dsk_ce.type) {
					case COMM_TYPE_DATA_WRITE:
					case COMM_TYPE_PUSH:
						ce.dst_cpu = dsk_ce.src_or_dst_cpu;
						ce.src_cpu = dsk_ce.header.cpu;
						break;

					case COMM_TYPE_DATA_READ:
					case COMM_TYPE_STEAL:
						ce.dst_cpu = dsk_ce.header.cpu;
						ce.src_cpu = dsk_ce.src_or_dst_cpu;
						break;
				}

				ce.size = dsk_ce.size;
				ce.type = dsk_ce.type;
				ce.texec_start = NULL;
				ce.texec_end = NULL;
				ce.prod_ts = dsk_ce.prod_ts;

				if(dsk_ce.type == COMM_TYPE_DATA_WRITE)
					if(dsk_ce.size > mes->max_write_size)
						mes->max_write_size = dsk_ce.size;

				if(dsk_ce.type == COMM_TYPE_DATA_READ)
					if(dsk_ce.size > mes->max_read_size)
						mes->max_read_size = dsk_ce.size;

				if(!last_what || last_what->addr != dsk_ce.what)
					last_what = frame_tree_find_or_add(ft, dsk_ce.what);

				ce.what = last_what;

				if(dsk_ce.type == COMM_TYPE_STEAL)
					last_what->num_steals++;
				else if(dsk_ce.type == COMM_TYPE_PUSH)
					last_what->num_pushes++;

				event_set_add_comm_event(es, &ce);
			} else if(dsk_eh.type == EVENT_TYPE_SINGLE) {
				memcpy(&dsk_sge, &dsk_eh, sizeof(dsk_eh));
				if(read_struct_convert(fp, &dsk_sge, sizeof(dsk_sge), trace_single_event_conversion_table, sizeof(dsk_eh)) != 0)
					return 1;

				(*bytes_read) += sizeof(dsk_sge) - sizeof(dsk_eh);

				if(!last_what || last_what->addr != dsk_sge.what)
					last_what = frame_tree_find_or_add(ft, dsk_sge.what);

				if(!(es = multi_event_set_find_alloc_cpu(mes, dsk_sge.header.cpu)))
					return 1;

				sge.active_task = task_tree_find(tt, dsk_sge.header.active_task);
				sge.active_frame = frame_tree_find(ft, dsk_sge.header.active_frame);
				sge.time = dsk_sge.header.time;
				sge.what = last_what;
				sge.type = dsk_sge.type;
				sge.next_texec_end = NULL;
				sge.prev_texec_end = NULL;
				sge.prev_texec_start = NULL;
				sge.next_texec_start = NULL;

				event_set_add_single_event(es, &sge);
			} else if(dsk_eh.type == EVENT_TYPE_COUNTER) {
				memcpy(&dsk_cre, &dsk_eh, sizeof(dsk_eh));
				if(read_struct_convert(fp, &dsk_cre, sizeof(dsk_cre), trace_counter_event_conversion_table, sizeof(dsk_eh)) != 0)
					return 1;

				(*bytes_read) += sizeof(dsk_cre) - sizeof(dsk_eh);

				if(!(cd = multi_event_set_find_counter_description(mes, dsk_cre.counter_id)))
					return 1;

				if(!(es = multi_event_set_find_alloc_cpu(mes, dsk_cre.header.cpu)))
					return 1;

				cre.time = dsk_cre.header.time;
				cre.value = dsk_cre.value;

				if(event_set_add_counter_event(es, &cre, cd, 1) != 0)
					return 1;

				multi_event_set_check_update_counter_bounds(mes, cd, &cre);
			} else if(dsk_eh.type == EVENT_TYPE_FRAME_INFO) {
				memcpy(&dsk_fi, &dsk_eh, sizeof(dsk_eh));

				if(read_struct_convert(fp, &dsk_fi, sizeof(dsk_fi), trace_frame_info_conversion_table, sizeof(dsk_eh)) != 0)
					return 1;

				(*bytes_read) += sizeof(dsk_fi) - sizeof(dsk_eh);

				if(!last_frame || last_frame->addr != dsk_fi.addr)
					last_frame = frame_tree_find_or_add(ft, dsk_fi.addr);

				last_frame->numa_node = dsk_fi.numa_node;
				last_frame->size = dsk_fi.size;

				if(mes->max_numa_node_id < dsk_fi.numa_node)
					mes->max_numa_node_id = dsk_fi.numa_node;
			} else if(dsk_eh.type == EVENT_TYPE_CPU_INFO) {
				memcpy(&dsk_ci, &dsk_eh, sizeof(dsk_eh));

				if(read_struct_convert(fp, &dsk_ci, sizeof(dsk_ci), trace_cpu_info_conversion_table, sizeof(dsk_eh)) != 0)
					return 1;

				(*bytes_read) += sizeof(dsk_ci) - sizeof(dsk_eh);

				if(!(es = multi_event_set_find_alloc_cpu(mes, dsk_ci.header.cpu)))
					return 1;

				es->numa_node = dsk_ci.numa_node;

				if(mes->max_numa_node_id < dsk_ci.numa_node)
					mes->max_numa_node_id = dsk_ci.numa_node;
			}
		}
	}

	return 0;
}

int read_trace_sample_file(struct multi_event_set* mes, const char* file, off_t* bytes_read)
{
	struct trace_header header;
	struct task_tree tt;
	struct frame_tree ft;
	struct omp_for_chunk_set_tree ofcst;
	struct omp_for_instance_tree ofit;
	struct omp_for_tree oft;
	enum compression_type compression_type;
	struct uncompress_pipe pipe;
	struct event_set* es;
	int status;

	int res = 1;
	FILE* fp;

	(*bytes_read) = 0;

	if(uncompress_detect_type(file, &compression_type))
		goto out;

	if(compression_type == COMPRESSION_TYPE_UNKNOWN)
		goto out;

	if(compression_type == COMPRESSION_TYPE_UNCOMPRESSED) {
		if(!(fp = fopen(file, "r")))
			goto out;
	} else {
		if(uncompress_pipe_open(&pipe, compression_type, file))
			goto out;

		fp = pipe.stdout;
	}

	if(read_struct_convert(fp, &header, sizeof(header), trace_header_conversion_table, 0) != 0)
		goto out_fp;

	(*bytes_read) += sizeof(header);

	if(!trace_verify_header(&header))
		goto out_fp;

	task_tree_init(&tt);
	frame_tree_init(&ft);
	omp_for_tree_init(&oft);
	omp_for_instance_tree_init(&ofit);
	omp_for_chunk_set_tree_init(&ofcst);

	if(read_trace_samples(mes, &tt, &ft, &ofcst, &ofit, &oft, fp, bytes_read) != 0)
		goto out_trees;

	if(omp_for_tree_to_array(&oft, &mes->omp_fors) != 0)
		goto out_trees;

	mes->num_omp_fors = oft.num_omp_fors;

	if(omp_for_instance_tree_to_array(&ofit, &mes->omp_for_instances) != 0)
		goto out_trees;

	mes->num_omp_for_instances = ofit.num_omp_for_instances;

	if(omp_for_chunk_set_tree_to_array(&ofcst, &mes->omp_for_chunk_sets) != 0)
		goto out_trees;

	mes->num_omp_for_chunk_sets = ofcst.num_omp_for_chunk_sets;

	if(chain_omp_for_structs(mes))
		goto out_trees;

	if(has_empty_omp_for(mes))
		goto out_trees;

	if(has_empty_omp_for_instances(mes))
		goto out_trees;

	if(has_nonexecuted_omp_for_chunk_sets(mes))
		goto out_trees;

	if(has_artifical_task_instances(mes))
		goto out_trees;

	if(has_nonexecuted_task_instances(mes))
		goto out_trees;

	if(chain_omp_task_structs(mes))
		goto out_trees;

	if(multi_event_set_state_descriptions_artificial(mes) &&
	   multi_event_set_state_ids_in_range(mes, 0, OPENSTREAM_WORKER_STATE_MAX-1))
		set_openstream_state_descriptions(mes);

	multi_event_set_sort_by_cpu(mes);

	if(task_tree_to_array(&tt, &mes->tasks) != 0)
		goto out_trees;

	mes->num_tasks = tt.num_tasks;

	if(frame_tree_to_array(&ft, &mes->frames) != 0)
		goto out_trees;

	mes->num_frames = ft.num_frames;

	for_each_event_set(mes, es) {
		event_set_sort_comm(es);

		if(trace_update_task_execution_bounds(es) != 0)
			goto out_trees;
	}

	/* Update references to frames, tasks and event sets
	 * This is necessary, as old references frames and tasks point to
	 * elements of the frame / task tree and not to the updated array
	 * of frames / tasks.
	 * References to event sets must be updated as the address of the array
	 * containing these data structures might have changed due to a call
	 * to realloc in add_buffer_grow()
	 */
	for_each_event_set(mes, es) {
		/* Update single events */
		for(struct single_event* se = &es->single_events[0];
		    se < &es->single_events[es->num_single_events];
		    se++)
		{
			se->active_task = multi_event_set_find_task_by_addr(mes, se->active_task->addr);
			se->active_frame = multi_event_set_find_frame_by_addr(mes, se->active_frame->addr);
			se->what = multi_event_set_find_frame_by_addr(mes, se->what->addr);
			se->event_set = es;

			/* Update frame's references to first texec start if necessary */
			if(se->type == SINGLE_TYPE_TEXEC_START) {
				if(!se->active_frame->first_texec_start ||
				   se->active_frame->first_texec_start->time > se->time)
				{
					se->active_frame->first_texec_start = se;
				}
			}

			/* Update frame's references to first tcreate if necessary */
			if(se->type == SINGLE_TYPE_TCREATE) {
				if(!se->what->first_tcreate ||
				   se->what->first_tcreate->time > se->time)
				{
					se->what->first_tcreate = se;
				}
			}
		}

		/* Update state events */
		for(struct state_event* se = &es->state_events[0];
		    se < &es->state_events[es->num_state_events];
		    se++)
		{
			se->active_task = multi_event_set_find_task_by_addr(mes, se->active_task->addr);
			se->active_frame = multi_event_set_find_frame_by_addr(mes, se->active_frame->addr);
			se->event_set = es;
		}

		/* Update communication events */
		for(struct comm_event* ce = &es->comm_events[0];
		    ce < &es->comm_events[es->num_comm_events];
		    ce++)
		{
			ce->active_task = multi_event_set_find_task_by_addr(mes, ce->active_task->addr);
			ce->active_frame = multi_event_set_find_frame_by_addr(mes, ce->active_frame->addr);
			ce->what = multi_event_set_find_frame_by_addr(mes, ce->what->addr);
			ce->event_set = es;
		}
	}

	/* Second round for communication events; update frame references to
	 * write events
	 */
	for_each_event_set(mes, es) {
		for(struct comm_event* ce = &es->comm_events[0];
		    ce < &es->comm_events[es->num_comm_events];
		    ce++)
		{
			if(ce->type == COMM_TYPE_DATA_WRITE) {
				/* Update first write to the frame */
				if(!ce->what->first_write ||
				   ce->what->first_write->time > ce->time)
				{
					ce->what->first_write = ce;
				}

				/* Update first maximal write to the frame */
				if(ce->what->first_texec_start &&
				   ce->time < ce->what->first_texec_start->time)
				{
					if(!ce->what->first_max_write ||
					   ce->what->first_max_write->size < ce->size ||
					   (ce->what->first_max_write->size == ce->size &&
					    ce->what->first_max_write->time > ce->time))
					{
						ce->what->first_max_write = ce;
					}
				}
			}
		}
	}

	/* Create indexes for counters */
	for_each_event_set(mes, es) {
		for(struct counter_event_set* ces = &es->counter_event_sets[0];
		    ces < &es->counter_event_sets[es->num_counter_event_sets];
		    ces++)
		{
			if(counter_event_set_create_index(ces))
				goto out_trees;
		}
	}

	/* Create indexes for state events */
	for_each_event_set(mes, es) {
		event_set_init_state_index(es, mes->num_states);
		event_set_update_state_index(es, NULL);
	}

	res = 0;

out_trees:
	task_tree_destroy(&tt);
	frame_tree_destroy(&ft);
	omp_for_tree_destroy(&oft);
	omp_for_instance_tree_destroy(&ofit);
	omp_for_chunk_set_tree_destroy(&ofcst);
out_fp:
	if(compression_type == COMPRESSION_TYPE_UNCOMPRESSED) {
		fclose(fp);
	} else {
		uncompress_pipe_close(&pipe, &status);

		if(status != 0) {
			task_tree_destroy(&tt);
			frame_tree_destroy(&ft);
			res = 1;
		}
	}
out:
	return res;
}
