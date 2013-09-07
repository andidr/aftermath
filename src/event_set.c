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

#include "event_set.h"
#include "filter.h"
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

int event_set_compare_cpus(const void* p1, const void* p2)
{
	const struct event_set* s1 = p1;
	const struct event_set* s2 = p2;

	if(s1->cpu > s2->cpu)
		return 1;
	else if(s1->cpu < s2->cpu)
		return -1;

	return 0;
}

int event_set_get_first_state_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_state_events-1;
	int center_idx = 0;

	if(es->num_state_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->state_events[center_idx].start > interval_end)
			end_idx = center_idx-1;
		else if(es->state_events[center_idx].end < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx > 0 && es->state_events[center_idx-1].start < interval_end && es->state_events[center_idx-1].end > interval_start)
		center_idx--;

	if(es->state_events[center_idx].start > interval_end || es->state_events[center_idx].end < interval_start)
		return -1;

	return center_idx;
}

int event_set_get_first_state_starting_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int idx = event_set_get_first_state_in_interval(es, interval_start, interval_end);

	if(idx == -1)
		return -1;

	if(es->state_events[idx].start < interval_start) {
		while(idx+1 < es->num_state_events &&
		      es->state_events[idx].start < interval_start)
		{
			idx++;
		}
	}

	if(es->state_events[idx].start < interval_start ||
	   es->state_events[idx].start > interval_end)
		return -1;

	return idx;
}

int event_set_get_first_state_starting_in_interval_type(struct event_set* es, uint64_t start, uint64_t end, enum worker_state type)
{
	int idx = event_set_get_first_state_starting_in_interval(es, start, end);

	if(idx == -1)
		return -1;

	if(es->state_events[idx].state != type) {
		if((idx = event_set_get_next_state_event(es, idx, type)) != -1)
			if(es->state_events[idx].start > end)
				return -1;
	}

	return idx;
}

int event_set_get_counter_event_set(struct event_set* es, int counter_idx)
{
	for(int i = 0; i < es->num_counter_event_sets; i++)
		if(es->counter_event_sets[i].counter_index == counter_idx)
			return i;

	return -1;
}

int event_set_get_next_state_event(struct event_set* es, int curr_idx, enum worker_state state)
{
	int idx = curr_idx+1;

	while(idx < es->num_state_events) {
		if(es->state_events[idx].state == state)
			return idx;

		idx++;
	}

	return -1;
}

int event_set_get_major_state(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, int* major_state)
{
	int idx_start = event_set_get_first_state_in_interval(es, start, end);
	uint64_t state_durations[WORKER_STATE_MAX];
	uint64_t max = 0;

	if(idx_start == -1)
		return 0;

	memset(state_durations, 0, sizeof(state_durations));

	for(int i = idx_start; i < es->num_state_events && es->state_events[i].start < end; i++) {
		if(!f || filter_has_state_event(f, &es->state_events[i])) {
			state_durations[es->state_events[i].state] +=
				state_event_length_in_interval(&es->state_events[i], start, end);
		}
	}

	for(int state = 0; state < WORKER_STATE_MAX; state++) {
		if(state_durations[state] > max) {
			max = state_durations[state];
			*major_state = state;
		}
	}

	return max > 0;
}

int event_set_get_enclosing_state(struct event_set* es, uint64_t time)
{
	int idx = event_set_get_first_state_in_interval(es, time, time);

	if(idx == -1)
		return -1;

	if(es->state_events[idx].start > time || es->state_events[idx].end < time)
		return -1;

	return idx;
}

int event_set_get_first_comm_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_comm_events-1;
	int center_idx = 0;

	if(es->num_comm_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->comm_events[center_idx].time > interval_end)
			end_idx = center_idx-1;
		else if(es->comm_events[center_idx].time < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx > 0 && es->comm_events[center_idx-1].time < interval_end && es->comm_events[center_idx-1].time > interval_start)
		center_idx--;

	return center_idx;
}

int event_set_get_next_single_event(struct event_set* es, int start_idx, enum single_event_type type)
{
	int idx = start_idx+1;

	while(idx < es->num_single_events) {
		if(es->single_events[idx].type == type)
			return idx;

		idx++;
	}

	return -1;
}

int event_set_get_first_single_event_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_single_events-1;
	int center_idx = 0;

	if(es->num_single_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->single_events[center_idx].time > interval_end)
			end_idx = center_idx-1;
		else if(es->single_events[center_idx].time < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx > 0 && es->single_events[center_idx-1].time < interval_end && es->single_events[center_idx-1].time > interval_start)
		center_idx--;

	return center_idx;
}

int event_set_get_first_single_event_in_interval_type(struct event_set* es, uint64_t start, uint64_t end, enum single_event_type type)
{
	int idx = event_set_get_first_single_event_in_interval(es, start, end);

	if(idx == -1)
		return -1;

	while(es->single_events[idx].type != type && idx+1 < es->num_single_events)
		idx++;

	if(es->single_events[idx].type != type)
		return -1;

	return idx;
}

struct single_event* event_set_find_first_tcreate(struct event_set* es, uint64_t frame_addr)
{
	for(int i = 0; i < es->num_single_events; i++)
		if(es->single_events[i].type == SINGLE_TYPE_TCREATE &&
		   es->single_events[i].what == frame_addr)
			return &es->single_events[i];

	return NULL;
}

struct comm_event* event_set_find_first_write(struct event_set* es, uint64_t frame_addr)
{
	struct comm_event* ret = NULL;
	struct comm_event* curr;

	for(int i = 0; i < es->num_comm_events; i++) {
		curr = &es->comm_events[i];

		if(curr->type == COMM_TYPE_DATA_READ &&
		   curr->active_frame == frame_addr)
		{
			if(!ret || curr->prod_ts < ret->prod_ts)
				ret = curr;
		}
	}

	return ret;
}
