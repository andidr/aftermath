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

#ifndef EVENTS_H
#define EVENTS_H

#include <malloc.h>
#include <string.h>
#include <stdint.h>

#define SET_PREALLOC 5
#define EVENT_PREALLOC (5*1024)

struct state_event {
	uint64_t start;
	uint64_t end;
	int state;
};

struct comm_event {
	uint64_t time;
	int dst_cpu;
	int dst_worker;
	int size;
};

struct single_event {
	uint64_t time;
	int type;
};

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

	int cpu;
	uint64_t first_start;
	uint64_t last_end;
};

struct multi_event_set {
	struct event_set* sets;
	int num_sets;
	int num_sets_free;
};


int event_set_get_first_state_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_first_comm_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_first_single_event_in_interval(struct event_set* es, uint64_t start, uint64_t end);
void event_set_sort_comm(struct event_set* es);

static inline int event_set_add_state_event(struct event_set* es, struct state_event* se)
{
	void* ptr;

	if(es->num_state_events_free == 0) {
		if(!(ptr = realloc(es->state_events, (es->num_state_events+EVENT_PREALLOC)*sizeof(struct state_event)))) {
			return 1;
		} else {
			es->num_state_events_free = SET_PREALLOC;
			es->state_events = ptr;
		}
	}

	memcpy(&es->state_events[es->num_state_events++], se, sizeof(*se));
	es->num_state_events_free--;

	if(se->start < es->first_start)
		es->first_start = se->start;

	if(se->end > es->last_end)
		es->last_end = se->end;

	return 0;
}

static inline int event_set_add_comm_event(struct event_set* es, struct comm_event* ce)
{
	void* ptr;

	if(es->num_comm_events_free == 0) {
		if(!(ptr = realloc(es->comm_events, (es->num_comm_events+EVENT_PREALLOC)*sizeof(struct comm_event)))) {
			return 1;
		} else {
			es->num_comm_events_free = SET_PREALLOC;
			es->comm_events = ptr;
		}
	}

	memcpy(&es->comm_events[es->num_comm_events++], ce, sizeof(*ce));
	es->num_comm_events_free--;

	if(ce->time < es->first_start)
		es->first_start = ce->time;

	if(ce->time > es->last_end)
		es->last_end = ce->time;

	return 0;
}

static inline int event_set_add_single_event(struct event_set* es, struct single_event* se)
{
	void* ptr;

	if(es->num_single_events_free == 0) {
		if(!(ptr = realloc(es->single_events, (es->num_single_events+EVENT_PREALLOC)*sizeof(struct single_event)))) {
			return 1;
		} else {
			es->num_single_events_free = SET_PREALLOC;
			es->single_events = ptr;
		}
	}

	memcpy(&es->single_events[es->num_single_events++], se, sizeof(*se));
	es->num_single_events_free--;

	if(se->time < es->first_start)
		es->first_start = se->time;

	if(se->time > es->last_end)
		es->last_end = se->time;

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

	es->cpu = cpu;
	es->first_start = UINT64_MAX;
	es->last_end = 0;
}

static inline void event_set_destroy(struct event_set* es)
{
	free(es->state_events);
	free(es->comm_events);
	free(es->single_events);
}

void multi_event_set_sort_by_cpu(struct multi_event_set* mes);

static inline struct event_set* multi_event_set_find_cpu(struct multi_event_set* mes, int cpu)
{
	for(int i = 0; i < mes->num_sets; i++)
		if(mes->sets[i].cpu == cpu)
			return &mes->sets[i];

	return NULL;
}

static inline int multi_event_set_find_cpu_idx(struct multi_event_set* mes, int cpu)
{
	for(int i = 0; i < mes->num_sets; i++)
		if(mes->sets[i].cpu == cpu)
			return i;

	return -1;
}

static inline int multi_event_set_alloc(struct multi_event_set* mes, int cpu)
{
	void* ptr;

	if(mes->num_sets_free == 0) {
		if(!(ptr = realloc(mes->sets, (mes->num_sets+SET_PREALLOC)*sizeof(struct event_set)))) {
			return 1;
		} else {
			mes->num_sets_free = SET_PREALLOC;
			mes->sets = ptr;
		}
	}

	mes->num_sets_free--;
	event_set_init(&mes->sets[mes->num_sets++], cpu);
	return 0;
}

static inline struct event_set* multi_event_set_find_alloc_cpu(struct multi_event_set* mes, int cpu)
{
	struct event_set* res = multi_event_set_find_cpu(mes, cpu);

	if(!res)
		if(multi_event_set_alloc(mes, cpu))
			return NULL;

	return multi_event_set_find_cpu(mes, cpu);
}

static inline uint64_t multi_event_set_first_event_start(struct multi_event_set* mes)
{
	uint64_t start = UINT64_MAX;

	for(int i = 0; i < mes->num_sets; i++)
		if(mes->sets[i].first_start < start)
			start = mes->sets[i].first_start;

	return start;
}

static inline uint64_t multi_event_set_last_event_end(struct multi_event_set* mes)
{
	uint64_t end = 0;

	for(int i = 0; i < mes->num_sets; i++)
		if(mes->sets[i].last_end > end)
			end = mes->sets[i].last_end;

	return end;
}

static inline void multi_event_set_destroy(struct multi_event_set* mes)
{
	for(int set = 0; set < mes->num_sets; set++)
		event_set_destroy(&mes->sets[set]);

	free(mes->sets);
}

#endif
