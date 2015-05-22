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

#include "counter_event_set.h"
#include "counter_event_set_index.h"
#include "event_set.h"

void counter_event_set_destroy(struct counter_event_set* ces)
{
	free(ces->events);

	if(ces->idx)
		counter_event_set_index_destroy(ces->idx);

	free(ces->idx);
}

int counter_event_set_create_index(struct counter_event_set* ces)
{
	if(!(ces->idx = malloc(sizeof(struct counter_event_set_index))))
		return 1;

	if(counter_event_set_index_create(ces->idx, ces, COUNTER_EVENT_SET_INDEX_DEFAULT_FAN_OUT)) {
		free(ces->idx);
		ces->idx = NULL;

		return 1;
	}

	return 0;
}

int counter_event_set_get_last_event_in_interval(struct counter_event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_events-1;
	int center_idx = 0;

	if(es->num_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->events[center_idx].time > interval_end)
			end_idx = center_idx-1;
		else if(es->events[center_idx].time < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx < es->num_events-1 && es->events[center_idx+1].time < interval_end && es->events[center_idx+1].time > interval_start)
		center_idx++;

	if(es->events[center_idx].time > interval_end || es->events[center_idx].time < interval_start)
		return -1;

	return center_idx;
}

uint64_t counter_event_set_get_value(struct counter_event_set* ces, uint64_t time)
{
	int start_idx = 0;
	int end_idx = ces->num_events-1;
	int center_idx = 0;

	if(ces->num_events == 0)
		return 0;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(ces->events[center_idx].time > time)
			end_idx = center_idx - 1;
		else if(ces->events[center_idx].time < time)
			start_idx = center_idx + 1;
		else
			return ces->events[center_idx].value;
	}

	if(ces->events[center_idx].time > time) {
		if(center_idx == 0)
			return ces->events[center_idx].value;
		else
			return (ces->events[center_idx-1].value +
				((time - ces->events[center_idx-1].time) *
				 (ces->events[center_idx].value - ces->events[center_idx-1].value)) /
				(ces->events[center_idx].time - ces->events[center_idx-1].time));
	} else {
		if(center_idx == ces->num_events-1)
			return ces->events[center_idx].value;
		else
			return (ces->events[center_idx].value +
				((time - ces->events[center_idx].time) *
				 (ces->events[center_idx+1].value - ces->events[center_idx].value)) /
				(ces->events[center_idx+1].time - ces->events[center_idx].time));
	}

	/* Should never happen */
	return 0;
}

int counter_event_set_interpolate_value(struct counter_event_set* ces, uint64_t time, int64_t* val_out)
{
	int ctr_idx = counter_event_set_get_last_event_in_interval(ces, 0, time);

	// no counter before time or last counter of the trace
	if (ctr_idx == -1 || ctr_idx == ces->num_events - 1)
		return 1;

	*val_out = counter_event_interpolate_value(&ces->events[ctr_idx],
						   &ces->events[ctr_idx+1],
						   time);

	return 0;
}

int counter_event_set_is_monotonously_increasing(struct counter_event_set* ces, struct counter_description** cd, int* cpu)
{
	int64_t value = INT64_MIN;

	for(int idx = 0; idx < ces->num_events; idx++) {
		if (ces->events[idx].value < value) {
			*cd = ces->events[idx].desc;
			*cpu = ces->events[idx].event_set->cpu;
			return 0;
		}
		value = ces->events[idx].value;
	}

	return 1;
}
