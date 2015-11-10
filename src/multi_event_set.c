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

#include "multi_event_set.h"
#include "filter.h"

void multi_event_set_sort_by_cpu(struct multi_event_set* mes)
{
	qsort(mes->sets, mes->num_sets, sizeof(struct event_set), event_set_compare_cpus);
	multi_event_set_rebuild_cpu_idx_map(mes);
}

int multi_event_set_get_max_task_duration_in_interval(struct multi_event_set* mes, struct filter* f, uint64_t start, uint64_t end, uint64_t* duration)
{
	uint64_t max = 0;
	uint64_t curr;
	struct event_set* es;

	for_each_event_set(mes, es)
		if(!f || filter_has_cpu(f, es->cpu))
			if(event_set_get_max_task_duration_in_interval(es, f, start, end, &curr))
				if(curr > max)
					max = curr;

	*duration = max;

	return (max > 0);
}

int multi_event_set_get_min_task_duration_in_interval(struct multi_event_set* mes, struct filter* f, uint64_t start, uint64_t end, uint64_t* duration)
{
	uint64_t min = UINT64_MAX;
	uint64_t curr;
	struct event_set* es;

	for_each_event_set(mes, es)
		if(!f || filter_has_cpu(f, es->cpu))
			if(event_set_get_min_task_duration_in_interval(es, f, start, end, &curr))
				if(curr < min)
					min = curr;

	*duration = min;

	return (min < UINT64_MAX);
}

int multi_event_set_get_min_max_task_duration_in_interval(struct multi_event_set* mes, struct filter* f, uint64_t start, uint64_t end, uint64_t* min, uint64_t* max)
{
	uint64_t lmin = UINT64_MAX;
	uint64_t lmax = 0;
	uint64_t curr_min;
	uint64_t curr_max;
	struct event_set* es;

	for_each_event_set(mes, es) {
		if(!f || filter_has_cpu(f, es->cpu)) {
			if(event_set_get_min_max_task_duration_in_interval(es, f, start, end, &curr_min, &curr_max)) {
				if(curr_min < lmin)
					lmin = curr_min;
				if(curr_max > lmax)
					lmax = curr_max;
			}
		}
	}

	*min = lmin;
	*max = lmax;

	return (lmin < UINT64_MAX);
}

void multi_event_set_dump_per_task_counter_values(struct multi_event_set* mes, struct filter* f, FILE* file, int* nb_errors_out)
{
	struct event_set* es;

	for_each_event_set(mes, es)
		event_set_dump_per_task_counter_values(es, f, file, nb_errors_out);
}

int multi_event_set_counters_monotonously_increasing(struct multi_event_set* mes, struct filter* f, struct counter_description** cd, int* cpu)
{
	struct event_set* es;

	for_each_event_set(mes, es) {
		if(!event_set_counters_monotonously_increasing(es, f, cd)) {
			*cpu = es->cpu;
			return 0;
		}
	}

	return 1;
}

int multi_event_set_cpus_have_counters(struct multi_event_set* mes, struct filter* f)
{
	struct event_set* es;
	struct counter_description* cd;

	for_each_counterdesc(mes, cd) {
		for_each_event_set(mes, es) {
			if(f && !filter_has_cpu(f, es->cpu))
				continue;

			if(!event_set_has_counter(es, cd))
				return 0;
		}
	}

	return 1;
}

uint32_t multi_event_set_seq_state_id(struct multi_event_set* mes, uint32_t id)
{
	for(size_t i = 0; i < mes->num_states; i++) {
		if(mes->states[i].state_id == id)
			return i;
	}
	return -1;
}
