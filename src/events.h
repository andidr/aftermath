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

#include "buffer.h"
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "trace_file.h"
#define _GNU_SOURCE
#define __USE_GNU 1
#include <search.h>
#include <sys/types.h>

#define SET_PREALLOC 5
#define EVENT_PREALLOC (5*1024)
#define COUNTER_PREALLOC 16

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

struct counter_event {
	uint64_t time;
	uint64_t active_task;
	uint64_t counter_id;
	int64_t value;
	int counter_index;
};

struct counter_event_set {
	struct counter_event* events;
	int num_events;
	int num_events_free;
	uint64_t counter_id;
	int counter_index;
};

struct counter_description {
	uint64_t counter_id;
	int64_t min;
	int64_t max;
	int index;
	char* name;
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

	struct counter_event_set* counter_event_sets;
	int num_counter_event_sets;
	int num_counter_event_sets_free;

	int cpu;
	uint64_t first_start;
	uint64_t last_end;
};

struct task {
	uint64_t work_fn;
	char* source_filename;
	char* symbol_name;
	int source_line;
};

struct multi_event_set {
	struct event_set* sets;
	int num_sets;
	int num_sets_free;

	struct task* tasks;
	int num_tasks;
	int num_tasks_free;

	struct counter_description* counters;
	int num_counters;
	int num_counters_free;
};

struct task_tree {
	void* root;
	int num_tasks;
};

struct filter;
int event_set_get_enclosing_state(struct event_set* es, uint64_t time);
int event_set_get_first_state_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_first_comm_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_first_single_event_in_interval(struct event_set* es, uint64_t start, uint64_t end);
int event_set_get_major_state(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, int* major_state);
void event_set_sort_comm(struct event_set* es);
int event_set_get_first_counter_event_in_interval(struct event_set* es, uint64_t counter_id, uint64_t start, uint64_t end);

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

static inline struct counter_description* multi_event_set_find_counter_description(struct multi_event_set* mes, uint64_t counter_id)
{
	for(int i = 0; i < mes->num_counters; i++)
		if(mes->counters[i].counter_id == counter_id)
			return &mes->counters[i];

	return NULL;
}

static inline struct counter_description* multi_event_set_find_counter_description_by_index(struct multi_event_set* mes, int counter_index)
{
	if(counter_index >= 0 && counter_index < mes->num_counters)
		return &mes->counters[counter_index];

	return NULL;
}

int counter_event_set_get_event_outside_interval(struct counter_event_set* es, uint64_t counter_id, uint64_t interval_start, uint64_t interval_end);

static inline void counter_event_set_destroy(struct counter_event_set* ces)
{
	free(ces->events);
}

static inline void counter_event_set_init(struct counter_event_set* ces, uint64_t counter_id, int counter_index)
{
	ces->events = NULL;
	ces->num_events = 0;
	ces->num_events_free = 0;
	ces->counter_id = counter_id;
	ces->counter_index = counter_index;
}

static inline int event_set_alloc_counter_event_set(struct event_set* es, uint64_t counter_id, int counter_index)
{
	if(check_buffer_grow((void**)&es->counter_event_sets, sizeof(struct counter_event_set),
			  es->num_counter_event_sets, &es->num_counter_event_sets_free,
			     SET_PREALLOC))
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

static inline int event_set_add_counter_event(struct multi_event_set* mes, struct event_set* es, struct counter_event* ce)
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

	struct counter_description* cd = multi_event_set_find_counter_description(mes, ce->counter_id);

	if(ce->value < cd->min)
		cd->min = ce->value;

	if(ce->value > cd->max)
		cd->max = ce->value;

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
	if(check_buffer_grow((void**)&mes->sets, sizeof(struct event_set),
			  mes->num_sets, &mes->num_sets_free,
			     SET_PREALLOC))
	{
		return 1;
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

int compare_tasks(const void *pt1, const void *pt2);

static inline struct task* multi_event_set_find_task(struct multi_event_set* mes, struct task* t)
{
	return bsearch(t, mes->tasks, mes->num_tasks, sizeof(struct task), compare_tasks);
}

static inline struct task* multi_event_set_find_task_by_work_fn(struct multi_event_set* mes, uint64_t work_fn)
{
	struct task t = { .work_fn = work_fn };
	return multi_event_set_find_task(mes, &t);
}

static inline void task_destroy(struct task* t)
{
	free(t->source_filename);
	free(t->symbol_name);
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
	struct task** t;

	if(!key)
		return NULL;

	key->work_fn = work_fn;

	if((t = tsearch(key, &tt->root, compare_tasks)) == NULL) {
		free(key);
		return NULL;
	}

	(*t)->source_filename = NULL;
	(*t)->symbol_name = NULL;
	(*t)->source_line = -1;

	tt->num_tasks++;

	return key;
}

void task_tree_walk(const void* p, const VISIT which, const int depth);
int task_tree_to_array(struct task_tree* tt, struct task** arr);

static inline void task_tree_destroy(struct task_tree* tt)
{
	tdestroy(tt->root, free);
}

static inline int counter_description_init(struct counter_description* cd, int index, uint64_t counter_id, int name_len)
{
	cd->counter_id = counter_id;
	cd->index = index;
	cd->name = malloc(name_len+1);
	cd->min = INT64_MAX;
	cd->max = INT64_MIN;

	if(cd->name == NULL)
		return 1;

	return 0;
}

static inline void counter_description_destroy(struct counter_description* cd)
{
	free(cd->name);
}

static inline int counter_description_alloc(struct multi_event_set* mes, uint64_t counter_id, int name_len)
{
	if(check_buffer_grow((void**)&mes->counters, sizeof(struct counter_description),
			  mes->num_counters, &mes->num_counters_free,
			     COUNTER_PREALLOC))
	{
		return 1;
	}

	if(counter_description_init(&mes->counters[mes->num_counters], mes->num_counters, counter_id, name_len) == 0) {
		mes->num_counters_free--;
		mes->num_counters++;

		return 0;
	}

	return 1;
}

static inline struct counter_description* counter_description_alloc_ptr(struct multi_event_set* mes, uint64_t counter_id, int name_len)
{
	if(counter_description_alloc(mes, counter_id, name_len) != 0)
		return NULL;

	return &mes->counters[mes->num_counters-1];
}

static inline void multi_event_set_destroy(struct multi_event_set* mes)
{
	for(int set = 0; set < mes->num_sets; set++)
		event_set_destroy(&mes->sets[set]);

	for(int task = 0; task < mes->num_tasks; task++)
		task_destroy(&mes->tasks[task]);

	for(int counter = 0; counter < mes->num_counters; counter++)
		counter_description_destroy(&mes->counters[counter]);

	free(mes->sets);
	free(mes->tasks);
}

/* Read all trace samples from a file and store the result in mes */
int read_trace_sample_file(struct multi_event_set* mes, const char* file, off_t* bytes_read);

#endif
