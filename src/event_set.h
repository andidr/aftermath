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

#ifndef EVENT_SET_H
#define EVENT_SET_H

#include "counter_event_set.h"
#include "buffer.h"
#include "events.h"

#define COUNTER_EVENT_SET_PREALLOC 5

struct event_set {
	struct state_event* state_events;
	int num_state_events;
	int num_state_events_free;

	struct comm_event* comm_events;
	int num_comm_events;
	int num_comm_events_free;

	struct single_event* single_events;
	int num_single_events;
	int num_single_events_free;

	struct counter_event_set* counter_event_sets;
	int num_counter_event_sets;
	int num_counter_event_sets_free;

	int cpu;
	uint64_t first_start;
	uint64_t last_end;
};

struct filter;
int event_set_get_enclosing_state(struct event_set* es, uint64_t time);
int event_set_get_first_state_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_first_comm_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_first_single_event_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_major_state(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, int* major_state);
void event_set_sort_comm(struct event_set* es);
int event_set_get_first_counter_event_in_interval(struct event_set* es, uint64_t counter_id, uint64_t start, uint64_t end);
int event_set_compare_cpus(const void* p1, const void* p2);

static inline int event_set_add_state_event(struct event_set* es, struct state_event* se)
{
	if(add_buffer_grow((void**)&es->state_events, se, sizeof(*se),
			&es->num_state_events, &es->num_state_events_free,
			   EVENT_PREALLOC) != 0)
	{
		return 1;
	}

	if(se->start < es->first_start)
		es->first_start = se->start;

	if(se->end > es->last_end)
		es->last_end = se->end;

	return 0;
}

static inline int event_set_add_comm_event(struct event_set* es, struct comm_event* ce)
{
	if(add_buffer_grow((void**)&es->comm_events, ce, sizeof(*ce),
			&es->num_comm_events, &es->num_comm_events_free,
			   EVENT_PREALLOC) != 0)
	{
		return 1;
	}

	if(ce->time < es->first_start)
		es->first_start = ce->time;

	if(ce->time > es->last_end)
		es->last_end = ce->time;

	return 0;
}

static inline int event_set_add_single_event(struct event_set* es, struct single_event* se)
{
	if(add_buffer_grow((void**)&es->single_events, se, sizeof(*se),
			&es->num_single_events, &es->num_single_events_free,
			   EVENT_PREALLOC) != 0)
	{
		return 1;
	}

	if(se->time < es->first_start)
		es->first_start = se->time;

	if(se->time > es->last_end)
		es->last_end = se->time;

	return 0;
}

static inline int event_set_alloc_counter_event_set(struct event_set* es, uint64_t counter_id, int counter_index)
{
	if(check_buffer_grow((void**)&es->counter_event_sets, sizeof(struct counter_event_set),
			     es->num_counter_event_sets, &es->num_counter_event_sets_free,
			     COUNTER_EVENT_SET_PREALLOC))
	{
		return 1;
	}

	es->num_counter_event_sets_free--;
	counter_event_set_init(&es->counter_event_sets[es->num_counter_event_sets++], counter_id, counter_index);
	return 0;
}

static inline struct counter_event_set* event_set_find_counter_event_set(struct event_set* es, uint64_t counter_id)
{
	for(int i = 0; i < es->num_counter_event_sets; i++)
		if(es->counter_event_sets[i].counter_id == counter_id)
			return &es->counter_event_sets[i];

	return NULL;
}

static inline struct counter_event_set* event_set_find_alloc_counter_event_set(struct event_set* es, uint64_t counter_id, int counter_index)
{
	struct counter_event_set* res;

	if(!(res = event_set_find_counter_event_set(es, counter_id)))
		if(event_set_alloc_counter_event_set(es, counter_id, counter_index) != 0)
			return NULL;

	res = event_set_find_counter_event_set(es, counter_id);

	return res;
}

static inline int event_set_add_counter_event(struct event_set* es, struct counter_event* ce)
{
	struct counter_event_set* ces;

	if(!(ces = event_set_find_alloc_counter_event_set(es, ce->counter_id, ce->counter_index)))
		return 1;

	if(add_buffer_grow((void**)&ces->events, ce, sizeof(*ce),
			&ces->num_events, &ces->num_events_free,
			   EVENT_PREALLOC) != 0)
	{
		return 1;
	}

	if(ce->time < es->first_start)
		es->first_start = ce->time;

	if(ce->time > es->last_end)
		es->last_end = ce->time;

	return 0;
}

static inline void event_set_init(struct event_set* es, int cpu)
{
	es->num_state_events = 0;
	es->num_state_events_free = 0;
	es->state_events = NULL;

	es->num_comm_events = 0;
	es->num_comm_events_free = 0;
	es->comm_events = NULL;

	es->num_single_events = 0;
	es->num_single_events_free = 0;
	es->single_events = NULL;

	es->num_counter_event_sets = 0;
	es->num_counter_event_sets_free = 0;
	es->counter_event_sets = NULL;

	es->cpu = cpu;
	es->first_start = UINT64_MAX;
	es->last_end = 0;
}

static inline void event_set_destroy(struct event_set* es)
{
	free(es->state_events);
	free(es->comm_events);
	free(es->single_events);

	for(int i = 0; i < es->num_counter_event_sets; i++)
		counter_event_set_destroy(&es->counter_event_sets[i]);

	free(es->counter_event_sets);
}

#endif