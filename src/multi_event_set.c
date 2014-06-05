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

	for(int i = 0 ; i < mes->num_sets; i++)
		if(filter_has_cpu(f, mes->sets[i].cpu))
			if(event_set_get_max_task_duration_in_interval(&mes->sets[i], f, start, end, &curr))
				if(curr > max)
					max = curr;

	*duration = max;

	return (max > 0);
}

int multi_event_set_get_min_task_duration_in_interval(struct multi_event_set* mes, struct filter* f, uint64_t start, uint64_t end, uint64_t* duration)
{
	uint64_t min = UINT64_MAX;
	uint64_t curr;

	for(int i = 0 ; i < mes->num_sets; i++)
		if(filter_has_cpu(f, mes->sets[i].cpu))
			if(event_set_get_min_task_duration_in_interval(&mes->sets[i], f, start, end, &curr))
				if(curr < min)
					min = curr;

	*duration = min;

	return (min < UINT64_MAX);
}
