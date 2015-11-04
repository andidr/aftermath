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

#ifndef MULTI_EVENT_SET
#define MULTI_EVENT_SET

#include "counter_description.h"
#include "state_description.h"
#include "event_set.h"
#include "task.h"
#include "color.h"
#include "frame.h"
#include <stdlib.h>

#define GLOBAL_EVENT_PREALLOC 32
#define SET_PREALLOC 5

struct multi_event_set {
	struct event_set* sets;
	size_t num_sets;
	size_t num_sets_free;

	struct task* tasks;
	size_t num_tasks;
	size_t num_tasks_free;

	struct frame* frames;
	size_t num_frames;
	size_t num_frames_free;

	struct counter_description* counters;
	size_t num_counters;
	size_t num_counters_free;

        struct state_description* states;
        size_t num_states;
        size_t num_states_free;

	struct global_single_event* global_single_events;
	size_t num_global_single_events;
	size_t num_global_single_events_free;

	int max_numa_node_id;

	int max_write_size;
	int max_read_size;

	int min_cpu;
	int max_cpu;

	int* cpu_idx_map;
};

#define for_each_event_set(mes, es)					\
	for((es) = &(mes)->sets[0];					\
	    (es) < &(mes)->sets[(mes)->num_sets];			\
	    (es)++)

static inline int multi_event_event_set_get_min_time(struct multi_event_set* mes, int64_t* time)
{
	int64_t min = INT64_MAX;
	struct event_set* es;

	for_each_event_set(mes, es)
		if(es->first_start < min)
			min = es->first_start;

	*time = min;

	return (min < UINT64_MAX);
}

static inline int multi_event_event_set_get_max_time(struct multi_event_set* mes, int64_t* time)
{
	int64_t max = 0;
	struct event_set* es;

	for_each_event_set(mes, es)
		if(es->last_end > max)
			max = es->last_end;

	*time = max;

	return (max > 0);
}

static inline void multi_event_event_set_add_counter_offset(struct multi_event_set* mes, int counter_id, int64_t offset)
{
	struct event_set* es;

	for_each_event_set(mes, es)
		event_set_add_counter_offset(es, counter_id, offset);
}

static inline struct counter_description* multi_event_set_find_counter_description(struct multi_event_set* mes, uint64_t counter_id)
{
	for(int i = 0; i < mes->num_counters; i++)
		if(mes->counters[i].counter_id == counter_id)
			return &mes->counters[i];

	return NULL;
}

static inline int64_t multi_event_get_min_counter_value(struct multi_event_set* mes)
{
	int64_t min = INT64_MAX;

	for(int i = 0; i < mes->num_counters; i++)
		if(mes->counters[i].min < min)
			min = mes->counters[i].min;

	return min;
}

static inline int multi_event_get_max_cpu(struct multi_event_set* mes)
{
	int max_cpu = -1;
	struct event_set* es;

	for_each_event_set(mes, es)
		if(es->cpu > max_cpu)
			max_cpu = es->cpu;

	return max_cpu;
}

static inline int64_t multi_event_get_max_counter_value(struct multi_event_set* mes)
{
	int64_t max = INT64_MIN;

	for(int i = 0; i < mes->num_counters; i++)
		if(mes->counters[i].max > max)
			max = mes->counters[i].max;

	return max;
}

static inline long double multi_event_get_min_counter_slope(struct multi_event_set* mes)
{
	long double min_slope = INT64_MAX;

	for(int i = 0; i < mes->num_counters; i++)
		if(mes->counters[i].min_slope < min_slope)
			min_slope = mes->counters[i].min_slope;

	return min_slope;
}

static inline long double multi_event_get_max_counter_slope(struct multi_event_set* mes)
{
	long double max_slope = INT64_MIN;

	for(int i = 0; i < mes->num_counters; i++)
		if(mes->counters[i].max_slope > max_slope)
			max_slope = mes->counters[i].max_slope;

	return max_slope;
}

void multi_event_set_sort_by_cpu(struct multi_event_set* mes);

static inline struct event_set* multi_event_set_find_cpu(struct multi_event_set* mes, int cpu)
{
	if(cpu < mes->min_cpu || cpu > mes->max_cpu)
		return NULL;

	int mapidx = cpu - mes->min_cpu;
	int idx = mes->cpu_idx_map[mapidx];

	if(idx != -1)
		return &mes->sets[idx];

	return NULL;
}

static inline int multi_event_set_find_cpu_idx(struct multi_event_set* mes, int cpu)
{
	if(cpu < mes->min_cpu || cpu > mes->max_cpu)
		return -1;

	int mapidx = cpu - mes->min_cpu;
	int idx = mes->cpu_idx_map[mapidx];

	return idx;
}

static inline void multi_event_set_update_cpu_idx_map(struct multi_event_set* mes)
{
	int num_cpus = mes->max_cpu - mes->min_cpu + 1;
	int mapidx;

	for(int i = 0; i < num_cpus; i++)
		mes->cpu_idx_map[i] = -1;

	for(int i = 0; i < mes->num_sets; i++) {
		mapidx = mes->sets[i].cpu - mes->min_cpu;
		mes->cpu_idx_map[mapidx] = i;
	}
}


static inline int multi_event_set_rebuild_cpu_idx_map(struct multi_event_set* mes)
{
	int num_cpus = mes->max_cpu - mes->min_cpu + 1;
	void* tmp = realloc(mes->cpu_idx_map, num_cpus*sizeof(mes->cpu_idx_map[0]));

	if(!tmp)
		return 1;

	mes->cpu_idx_map = tmp;
	multi_event_set_update_cpu_idx_map(mes);

	return 0;
}

static inline int multi_event_set_alloc(struct multi_event_set* mes, int cpu)
{
	if(check_buffer_grow((void**)&mes->sets, sizeof(struct event_set),
			  mes->num_sets, &mes->num_sets_free,
			     SET_PREALLOC))
	{
		return 1;
	}

	int rebuild_map = 0;

	if(cpu < mes->min_cpu || mes->min_cpu == -1) {
		mes->min_cpu = cpu;
		rebuild_map = 1;
	}

	if(cpu > mes->max_cpu || mes->max_cpu == -1) {
		mes->max_cpu = cpu;
		rebuild_map = 1;
	}

	mes->num_sets_free--;
	event_set_init(&mes->sets[mes->num_sets++], cpu);

	if(rebuild_map) {
		if(multi_event_set_rebuild_cpu_idx_map(mes))
			return 1;
	} else {
		multi_event_set_update_cpu_idx_map(mes);
	}

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
	struct event_set* es;

	for_each_event_set(mes, es)
		if(es->first_start < start)
			start = es->first_start;

	return start;
}

static inline uint64_t multi_event_set_last_event_end(struct multi_event_set* mes)
{
	uint64_t end = 0;
	struct event_set* es;

	for_each_event_set(mes, es)
		if(es->last_end > end)
			end = es->last_end;

	return end;
}

static inline struct task* multi_event_set_find_task(struct multi_event_set* mes, struct task* t)
{
	return bsearch(t, mes->tasks, mes->num_tasks, sizeof(struct task), compare_tasks);
}

static inline struct task* multi_event_set_find_task_by_addr(struct multi_event_set* mes, uint64_t addr)
{
	struct task t = { .addr = addr };
	return multi_event_set_find_task(mes, &t);
}

static inline struct frame* multi_event_set_find_frame(struct multi_event_set* mes, struct frame* f)
{
	return bsearch(f, mes->frames, mes->num_frames, sizeof(struct frame), compare_frames);
}

static inline struct frame* multi_event_set_find_frame_by_addr(struct multi_event_set* mes, uint64_t addr)
{
	struct frame f = { .addr = addr };
	return multi_event_set_find_frame(mes, &f);
}

static inline void multi_event_set_init(struct multi_event_set* mes)
{
	mes->sets = NULL;
	mes->num_sets = 0;
	mes->num_sets_free = 0;
	mes->num_tasks = 0;
	mes->num_tasks_free = 0;
	mes->tasks = NULL;
	mes->num_frames = 0;
	mes->num_frames_free = 0;
	mes->frames = NULL;
	mes->counters = NULL;
	mes->num_counters = 0;
	mes->num_counters_free = 0;
        mes->states = NULL;
	mes->num_states = 0;
	mes->num_states_free = 0;
	mes->max_numa_node_id = 0;
	mes->max_write_size = 0;
	mes->max_read_size = 0;
	mes->min_cpu = -1;
	mes->max_cpu = -1;
	mes->cpu_idx_map = NULL;
	mes->num_global_single_events = 0;
	mes->num_global_single_events_free = 0;
	mes->global_single_events = NULL;
}

static inline void multi_event_set_destroy(struct multi_event_set* mes)
{
	struct event_set* es;

	for_each_event_set(mes, es)
		event_set_destroy(es);

	for(int task = 0; task < mes->num_tasks; task++)
		task_destroy(&mes->tasks[task]);

	for(int counter = 0; counter < mes->num_counters; counter++)
		counter_description_destroy(&mes->counters[counter]);

	for(int state = 0; state < mes->num_states; state++)
		state_description_destroy(&mes->states[state]);

	free(mes->sets);
	free(mes->tasks);
	free(mes->frames);
	free(mes->cpu_idx_map);
	free(mes->global_single_events);
	free(mes->states);
}

static inline int multi_event_set_state_description_alloc(struct multi_event_set* mes, uint32_t state_id, int name_len)
{
	if(check_buffer_grow((void**)&mes->states, sizeof(struct state_description),
			  mes->num_states, &mes->num_states_free,
			     STATE_PREALLOC))
	{
		return 1;
	}

	if(state_description_init(&mes->states[mes->num_states], state_id, name_len) == 0) {
		mes->num_states_free--;
		mes->num_states++;

		return 0;
	}

	return 1;
}

static inline int multi_event_set_state_descriptions_artificial(struct multi_event_set* mes)
{
	for(int i = 0; i < mes->num_states; i++)
		if(!mes->states[i].artificial)
			return 0;

	return 1;
}

static inline int multi_event_set_state_ids_in_range(struct multi_event_set* mes, int low, int high)
{
	for(int i = 0; i < mes->num_states; i++) {
		if(mes->states[i].state_id < low ||
		   mes->states[i].state_id > high)
		{
			return 0;
		}
	}

	return 1;
}

static inline struct state_description* multi_event_set_state_description_alloc_ptr(struct multi_event_set* mes, uint32_t state_id, int name_len)
{
	if(multi_event_set_state_description_alloc(mes, state_id, name_len) != 0)
		return NULL;

	return &mes->states[mes->num_states-1];
}

static inline struct state_description* multi_event_set_find_state_description(struct multi_event_set* mes, uint32_t state_id)
{
	for(int i = 0; i < mes->num_states; i++)
		if(mes->states[i].state_id == state_id)
			return &mes->states[i];

	return NULL;
}

static inline int multi_event_set_counter_description_alloc(struct multi_event_set* mes, uint64_t counter_id, int name_len)
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

static inline struct counter_description* multi_event_set_counter_description_alloc_ptr(struct multi_event_set* mes, uint64_t counter_id, int name_len)
{
	static double counter_colors[][3] = {
		{ COL_NORM(255), COL_NORM(  0), COL_NORM(  0) },
		{ COL_NORM(  0), COL_NORM(255), COL_NORM(  0) },
		{ COL_NORM(255), COL_NORM(255), COL_NORM(  0) },
		{ COL_NORM(255), COL_NORM(  0), COL_NORM(255) }
	};

	if(multi_event_set_counter_description_alloc(mes, counter_id, name_len) != 0)
		return NULL;

	int col_idx = (mes->num_counters-1) % (sizeof(counter_colors) / sizeof(counter_colors[0]));
	mes->counters[mes->num_counters-1].color_r = counter_colors[col_idx][0];
	mes->counters[mes->num_counters-1].color_g = counter_colors[col_idx][1];
	mes->counters[mes->num_counters-1].color_b = counter_colors[col_idx][2];

	return &mes->counters[mes->num_counters-1];
}

static inline uint64_t multi_event_set_get_free_counter_id(struct multi_event_set* mes)
{
	uint64_t id = 0;

	for(int i = 0; i < mes->num_counters; i++) {
		if(mes->counters[i].counter_id >= id)
			id = mes->counters[i].counter_id + 1;
	}

	return id;
}

static inline void multi_event_set_check_update_counter_bounds(struct multi_event_set* mes, struct counter_description* cd, struct counter_event* ce)
{
	if(ce->value < cd->min)
		cd->min = ce->value;

	if(ce->value > cd->max)
		cd->max = ce->value;

	if(ce->slope < cd->min_slope)
		cd->min_slope = ce->slope;

	if(ce->slope > cd->max_slope)
		cd->max_slope = ce->slope;
}

/* Read all trace samples from a file and store the result in mes */
int read_trace_sample_file(struct multi_event_set* mes, const char* file, off_t* bytes_read);

static inline struct single_event* multi_event_set_find_next_texec_start_for_frame(struct multi_event_set* mes, uint64_t start, struct frame* f)
{
	struct single_event* tips[mes->num_sets];
	struct single_event* min = NULL;

	int updates = 1;

	for(int i = 0; i < mes->num_sets; i++) {
		int idx = event_set_get_first_single_event_in_interval_type(&mes->sets[i], start, mes->sets[i].last_end, SINGLE_TYPE_TEXEC_START);
		tips[i] = (idx == -1) ? NULL : &mes->sets[i].single_events[idx];
	}

	while(updates != 0) {
		updates = 0;

		for(int i = 0; i < mes->num_sets; i++) {
			if(tips[i] && tips[i]->active_frame->addr == f->addr)
				if(!min || tips[i]->time < min->time)
					min = tips[i];

			if(tips[i] && (!min || tips[i]->time < min->time)) {
				tips[i] = tips[i]->next_texec_start;
				updates = 1;
			}
		}
	}


	return min;
}

static inline struct single_event* multi_event_set_find_next_tdestroy_for_frame(struct multi_event_set* mes, uint64_t start, struct frame* f)
{
	struct single_event* tips[mes->num_sets];
	struct single_event* min = NULL;

	int updates = 1;

	for(int i = 0; i < mes->num_sets; i++) {
		int idx = event_set_get_first_single_event_in_interval_type(&mes->sets[i], start, mes->sets[i].last_end, SINGLE_TYPE_TDESTROY);
		tips[i] = (idx == -1) ? NULL : &mes->sets[i].single_events[idx];
	}

	while(updates != 0) {
		updates = 0;

		for(int i = 0; i < mes->num_sets; i++) {
			if(tips[i] && tips[i]->type == SINGLE_TYPE_TDESTROY && tips[i]->what->addr == f->addr)
				if(!min || tips[i]->time < min->time)
					min = tips[i];

			if(tips[i] && (!min || tips[i]->time < min->time)) {
				if(tips[i] < &mes->sets[i].single_events[mes->sets[i].num_single_events-1])
					tips[i] = tips[i] + 1;
				else
					tips[i] = NULL;

				updates = 1;
			}
		}
	}

	return min;
}

static inline struct single_event* multi_event_set_find_prev_texec_start_for_frame(struct multi_event_set* mes, uint64_t start, struct frame* f)
{
	struct single_event* tips[mes->num_sets];
	struct single_event* max = NULL;

	int updates = 1;

	for(int i = 0; i < mes->num_sets; i++) {
		int idx = event_set_get_last_single_event_in_interval_type(&mes->sets[i], 0, start, SINGLE_TYPE_TEXEC_START);
		tips[i] = (idx == -1) ? NULL : &mes->sets[i].single_events[idx];
	}

	while(updates != 0) {
		updates = 0;

		for(int i = 0; i < mes->num_sets; i++) {
			if(tips[i] && tips[i]->active_frame->addr == f->addr)
				if(!max || tips[i]->time > max->time)
					max = tips[i];

			if(tips[i] && (!max || tips[i]->time > max->time)) {
				tips[i] = tips[i]->prev_texec_start;
				updates = 1;
			}
		}
	}


	return max;
}

static inline struct single_event* multi_event_set_find_prev_tcreate_for_frame(struct multi_event_set* mes, uint64_t start, struct frame* f)
{
	struct single_event* tips[mes->num_sets];
	struct single_event* max = NULL;

	int updates = 1;

	for(int i = 0; i < mes->num_sets; i++) {
		int idx = event_set_get_last_single_event_in_interval_type(&mes->sets[i], 0, start, SINGLE_TYPE_TCREATE);
		tips[i] = (idx == -1) ? NULL : &mes->sets[i].single_events[idx];
	}

	while(updates != 0) {
		updates = 0;

		for(int i = 0; i < mes->num_sets; i++) {
			if(tips[i] && tips[i]->type == SINGLE_TYPE_TCREATE && tips[i]->what->addr == f->addr)
				if(!max || tips[i]->time > max->time)
					max = tips[i];

			if(tips[i] && (!max || tips[i]->time > max->time)) {
				if(tips[i] > tips[i]->event_set->single_events) {
					tips[i]--;
					updates = 1;
				}
			}
		}
	}


	return max;
}

static inline struct comm_event* multi_event_set_find_prev_write_to_frame(struct multi_event_set* mes, uint64_t start, struct frame* f)
{
	struct comm_event* tips[mes->num_sets];
	struct comm_event* max = NULL;

	int updates = 1;

	for(int i = 0; i < mes->num_sets; i++) {
		int idx = event_set_get_last_comm_event_in_interval_type(&mes->sets[i], 0, start, COMM_TYPE_DATA_WRITE);
		tips[i] = (idx == -1) ? NULL : &mes->sets[i].comm_events[idx];
	}

	while(updates != 0) {
		updates = 0;

		for(int i = 0; i < mes->num_sets; i++) {
			if(tips[i] && tips[i]->type == COMM_TYPE_DATA_WRITE && tips[i]->what->addr == f->addr)
				if(!max || tips[i]->time > max->time)
					max = tips[i];

			if(tips[i] && (!max || tips[i]->time > max->time)) {
				tips[i] = tips[i] > &mes->sets[i].comm_events[0] ? tips[i]-1 : NULL;
				updates = 1;
			}
		}
	}


	return max;
}

static inline int multi_event_set_get_prev_ready_time(struct multi_event_set* mes, uint64_t start, struct frame* f, int64_t* time_out, int* cpu_out)
{
	struct comm_event* prev_write = multi_event_set_find_prev_write_to_frame(mes, start, f);

	if(prev_write) {
		*time_out = prev_write->time;
		*cpu_out = prev_write->event_set->cpu;
	} else {
		struct single_event* prev_tcreate = multi_event_set_find_prev_tcreate_for_frame(mes, start, f);

		if(prev_tcreate) {
			*time_out = prev_tcreate->time;
			*cpu_out = prev_tcreate->event_set->cpu;
		} else {
			return 1;
		}
	}

	return 0;
}

static inline int multi_event_set_add_global_single_event(struct multi_event_set* mes, struct global_single_event* gse)
{
	return add_buffer_grow((void**)&mes->global_single_events, gse, sizeof(*gse),
			       &mes->num_global_single_events, &mes->num_global_single_events_free,
			       GLOBAL_EVENT_PREALLOC);
}

struct filter;

int multi_event_set_get_min_task_duration_in_interval(struct multi_event_set* mes, struct filter* f, uint64_t start, uint64_t end, uint64_t* duration);
int multi_event_set_get_max_task_duration_in_interval(struct multi_event_set* mes, struct filter* f, uint64_t start, uint64_t end, uint64_t* duration);
int multi_event_set_get_min_max_task_duration_in_interval(struct multi_event_set* mes, struct filter* f, uint64_t start, uint64_t end, uint64_t* min, uint64_t* max);

void multi_event_set_dump_per_task_counter_values(struct multi_event_set* mes, struct filter* f, FILE* file, int* nb_errors_out);

int multi_event_set_counters_monotonously_increasing(struct multi_event_set* mes, struct filter*f, struct counter_description** cd, int* cpu);

int multi_event_set_cpus_have_counters(struct multi_event_set* mes, struct filter* f);
uint32_t multi_event_set_seq_state_id(struct multi_event_set* mes, uint32_t id);

#endif
