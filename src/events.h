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
#include <stdlib.h>
#include "trace_file.h"
#define _GNU_SOURCE
#define __USE_GNU 1
#include <search.h>

#define SET_PREALLOC 5
#define EVENT_PREALLOC (5*1024)

struct state_event {
	uint64_t start;
	uint64_t end;
	uint64_t active_task;
	int state;
};

struct comm_event {
	uint64_t time;
	int dst_cpu;
	int dst_worker;
	int size;
	enum comm_event_type type;
	uint64_t active_task;
	uint64_t what;
};

struct single_event {
	uint64_t time;
	enum single_event_type type;
	uint64_t active_task;
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

struct task {
	uint64_t work_fn;
};

struct multi_event_set {
	struct event_set* sets;
	int num_sets;
	int num_sets_free;

	struct task* tasks;
	int num_tasks;
	int num_tasks_free;
};

struct task_tree {
	void* root;
	int num_tasks;
};

int event_set_get_first_state_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_first_comm_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_first_single_event_in_interval(struct event_set* es, uint64_t start, uint64_t end);
void event_set_sort_comm(struct event_set* es);

static inline int check_buffer_grow(void** buffer, int elem_size, int used, int* free, int prealloc_elems)
{
	void* ptr;

	if(*free == 0) {
		if(!(ptr = realloc(*buffer, (used+prealloc_elems)*elem_size))) {
			return 1;
		} else {
			*free = prealloc_elems;
			*buffer = ptr;
		}
	}

	return 0;
}

static inline int add_buffer_grow(void** buffer, void* elem, int elem_size, int* used, int* free, int prealloc_elems)
{
	if(check_buffer_grow(buffer, elem_size, *used, free, prealloc_elems))
		return 1;

	void* addr = ((char*)(*buffer))+(elem_size*(*used));

	memcpy(addr, elem, elem_size);
	(*free)--;
	(*used)++;

	return 0;
}

static inline int event_set_add_state_event(struct event_set* es, struct state_event* se)
{
	add_buffer_grow((void**)&es->state_events, se, sizeof(*se),
			&es->num_state_events, &es->num_state_events_free,
			EVENT_PREALLOC);

	if(se->start < es->first_start)
		es->first_start = se->start;

	if(se->end > es->last_end)
		es->last_end = se->end;

	return 0;
}

static inline int event_set_add_comm_event(struct event_set* es, struct comm_event* ce)
{
	add_buffer_grow((void**)&es->comm_events, ce, sizeof(*ce),
			&es->num_comm_events, &es->num_comm_events_free,
			EVENT_PREALLOC);

	if(ce->time < es->first_start)
		es->first_start = ce->time;

	if(ce->time > es->last_end)
		es->last_end = ce->time;

	return 0;
}

static inline int event_set_add_single_event(struct event_set* es, struct single_event* se)
{
	add_buffer_grow((void**)&es->single_events, se, sizeof(*se),
			&es->num_single_events, &es->num_single_events_free,
			EVENT_PREALLOC);

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
	check_buffer_grow((void**)&mes->sets, sizeof(struct event_set),
			  mes->num_sets, &mes->num_sets_free,
			  SET_PREALLOC);

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

int compare_tasks(const void *pt1, const void *pt2);

static inline struct task* multi_event_set_find_task(struct multi_event_set* mes, struct task* t)
{
	return bsearch(t, mes->tasks, mes->num_tasks, sizeof(struct task), compare_tasks);
}

static inline void multi_event_set_destroy(struct multi_event_set* mes)
{
	for(int set = 0; set < mes->num_sets; set++)
		event_set_destroy(&mes->sets[set]);

	free(mes->sets);
	free(mes->tasks);
}

static inline void task_tree_init(struct task_tree* tt)
{
	tt->root = NULL;
	tt->num_tasks = 0;
}

static inline struct task* task_tree_find(struct task_tree* tt, uint64_t work_fn)
{
	struct task key = { .work_fn = work_fn };
	return tfind(&key, &tt->root, compare_tasks);
}

static inline struct task* task_tree_add(struct task_tree* tt, uint64_t work_fn)
{
	struct task* key = malloc(sizeof(struct task));

	if(!key)
		return NULL;

	key->work_fn = work_fn;

	if(tsearch(key, &tt->root, compare_tasks) == NULL) {
		free(key);
		return NULL;
	}

	tt->num_tasks++;

	return key;
}

void task_tree_walk(const void* p, const VISIT which, const int depth);
int task_tree_to_array(struct task_tree* tt, struct task** arr);

static inline void task_tree_destroy(struct task_tree* tt)
{
	tdestroy(tt->root, free);
}

/* Read all trace samples from a file and store the result in mes */
int read_trace_sample_file(struct multi_event_set* mes, const char* file);

#endif
