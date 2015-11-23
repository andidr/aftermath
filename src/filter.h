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

#ifndef FILTER_H
#define FILTER_H

#include "events.h"
#include "buffer.h"
#include "bitvector.h"
#include "counter_description.h"
#include "multi_event_set.h"

#define FILTER_TASK_PREALLOC 16
#define FILTER_FRAME_PREALLOC 1024
#define FILTER_COUNTER_BITS 64
#define FILTER_NUMA_NODE_BITS 64

struct filter {
	struct task** tasks;
	size_t num_tasks;
	size_t num_tasks_free;
	int filter_tasks;

	struct frame** frames;
	size_t num_frames;
	size_t num_frames_free;
	int filter_frames;

	struct bitvector counters;
	int filter_counters;

	int filter_counter_values;
	int64_t min;
	int64_t max;

	int filter_counter_slopes;
	long double min_slope;
	long double max_slope;

	int filter_task_length;
	int64_t min_task_length;
	int64_t max_task_length;

	int filter_comm_size;
	int64_t min_comm_size;
	int64_t max_comm_size;

	int filter_single_event_types;
	int64_t single_event_types;

	struct bitvector frame_numa_nodes;
	int filter_frame_numa_nodes;

	struct bitvector comm_numa_nodes;
	int filter_comm_numa_nodes;

	struct bitvector writes_to_numa_nodes;
	int filter_writes_to_numa_nodes;
	int64_t writes_to_numa_nodes_minsize;

	struct bitvector cpus;
	int filter_cpus;
};

static inline int filter_init(struct filter* f, int64_t min, int64_t max,
			      long double min_slope, long double max_slope,
			      int max_cpu)
{
	f->tasks = NULL;
	f->num_tasks = 0;
	f->num_tasks_free = 0;

	f->frames = NULL;
	f->num_frames = 0;
	f->num_frames_free = 0;

	f->filter_tasks = 0;
	f->filter_frames = 0;
	f->filter_counters = 0;
	f->filter_frame_numa_nodes = 0;
	f->filter_comm_numa_nodes = 0;
	f->filter_single_event_types = 0;

	f->filter_counter_values = 0;
	f->min = min;
	f->max = max;

	f->filter_counter_slopes = 0;
	f->min_slope = min_slope;
	f->max_slope = max_slope;

	f->filter_task_length = 0;
	f->filter_comm_size = 0;
	f->filter_cpus = 0;
	f->filter_writes_to_numa_nodes = 0;
	f->writes_to_numa_nodes_minsize = 0;

	if(bitvector_init(&f->counters, FILTER_COUNTER_BITS))
		return 1;

	if(bitvector_init(&f->frame_numa_nodes, FILTER_NUMA_NODE_BITS))
		return 1;

	if(bitvector_init(&f->comm_numa_nodes, FILTER_NUMA_NODE_BITS))
		return 1;

	if(bitvector_init(&f->writes_to_numa_nodes, FILTER_NUMA_NODE_BITS))
		return 1;

	if(bitvector_init(&f->cpus, max_cpu))
		return 1;

	return 0;
}

static inline void filter_set_task_filtering(struct filter* f, int b)
{
	f->filter_tasks = b;
}

static inline void filter_clear_tasks(struct filter* f)
{
	f->num_tasks_free += f->num_tasks;
	f->num_tasks = 0;
}

static inline int filter_add_task(struct filter* f, struct task* t)
{
	filter_set_task_filtering(f, 1);

	return add_buffer_grow((void**)&f->tasks, &t, sizeof(t),
			&f->num_tasks, &f->num_tasks_free,
			FILTER_TASK_PREALLOC);
}

static inline void filter_set_single_event_type_filtering(struct filter* f, int b)
{
	f->filter_single_event_types = b;
}

static inline void filter_clear_single_event_types(struct filter* f)
{
	f->single_event_types = 0;
}

static inline void filter_add_single_event_type(struct filter* f, enum single_event_type t)
{
	f->single_event_types |= (1 << t);
}

static inline int filter_has_single_event_type(struct filter* f, enum single_event_type t)
{
	if(!f->filter_single_event_types)
		return 1;

	return (f->single_event_types & (1 << t));
}

static inline void filter_set_cpu_filtering(struct filter* f, int b)
{
	f->filter_cpus = b;
}

static inline void filter_clear_cpus(struct filter* f)
{
	bitvector_clear(&f->cpus);
	filter_set_cpu_filtering(f, 0);
}

static inline void filter_add_cpu(struct filter* f, int cpu)
{
	filter_set_cpu_filtering(f, 1);
	bitvector_set_bit(&f->cpus, cpu);
}

static inline void filter_set_frame_filtering(struct filter* f, int b)
{
	f->filter_frames = b;
}

static inline void filter_clear_frames(struct filter* f)
{
	f->num_frames_free += f->num_frames;
	f->num_frames = 0;
	filter_set_frame_filtering(f, 0);
}

static inline int filter_add_frame(struct filter* f, struct frame* r)
{
	filter_set_frame_filtering(f, 1);

	return add_buffer_grow((void**)&f->frames, &r, sizeof(r),
			&f->num_frames, &f->num_frames_free,
			FILTER_FRAME_PREALLOC);
}

static inline void filter_add_counter(struct filter* f, struct counter_description* c)
{
	f->filter_counters = 1;
	bitvector_set_bit(&f->counters, c->index);
}

static inline void filter_clear_counters(struct filter* f)
{
	f->filter_counters = 0;
	bitvector_clear(&f->counters);
}

static inline int filter_has_counter(struct filter* f, struct counter_description* cd)
{
	return !f->filter_counters || bitvector_test_bit(&f->counters, cd->index);
}

static inline void filter_add_frame_numa_node(struct filter* f, int node_id)
{
	f->filter_frame_numa_nodes = 1;
	bitvector_set_bit(&f->frame_numa_nodes, node_id);
}

static inline void filter_clear_frame_numa_nodes(struct filter* f)
{
	f->filter_frame_numa_nodes = 0;
	bitvector_clear(&f->frame_numa_nodes);
}

static inline int filter_has_frame_numa_node(struct filter* f, int node_id)
{
	return !f->filter_frame_numa_nodes || bitvector_test_bit(&f->frame_numa_nodes, node_id);
}

static inline void filter_set_frame_numa_node_filtering(struct filter* f, int b)
{
	f->filter_frame_numa_nodes = b;
}

static inline void filter_add_comm_numa_node(struct filter* f, int node_id)
{
	f->filter_comm_numa_nodes = 1;
	bitvector_set_bit(&f->comm_numa_nodes, node_id);
}

static inline void filter_clear_comm_numa_nodes(struct filter* f)
{
	f->filter_comm_numa_nodes = 0;
	bitvector_clear(&f->comm_numa_nodes);
}

static inline int filter_has_comm_numa_node(struct filter* f, int node_id)
{
	return !f->filter_comm_numa_nodes || bitvector_test_bit(&f->comm_numa_nodes, node_id);
}

static inline void filter_set_comm_numa_node_filtering(struct filter* f, int b)
{
	f->filter_comm_numa_nodes = b;
}

static inline void filter_set_writes_to_numa_nodes_minsize(struct filter* f, int64_t minsize)
{
	f->writes_to_numa_nodes_minsize = minsize;
}


static inline void filter_clear_writes_to_numa_nodes_nodes(struct filter* f)
{
	f->filter_comm_numa_nodes = 0;
	bitvector_clear(&f->writes_to_numa_nodes);
}

static inline void filter_add_writes_to_numa_nodes_node(struct filter* f, int node_id)
{
	f->filter_writes_to_numa_nodes = 1;
	bitvector_set_bit(&f->writes_to_numa_nodes, node_id);
}

static inline void filter_writes_to_numa_nodes_nodes(struct filter* f)
{
	f->filter_writes_to_numa_nodes = 0;
	bitvector_clear(&f->writes_to_numa_nodes);
}

static inline int filter_has_writes_to_numa_nodes_node(struct filter* f, int node_id)
{
	return !f->filter_writes_to_numa_nodes || bitvector_test_bit(&f->writes_to_numa_nodes, node_id);
}

static inline void filter_set_writes_to_numa_nodes_filtering(struct filter* f, int b)
{
	f->filter_writes_to_numa_nodes = b;
}

void filter_sort_tasks(struct filter* f);
int filter_has_task(struct filter* f, struct task* t);

void filter_sort_frames(struct filter* f);
int filter_has_frame(struct filter* f, struct frame* fr);

static inline int filter_has_cpu(struct filter* f, int cpu)
{
	if(!f->filter_cpus)
		return 1;

	return bitvector_test_bit(&f->cpus, cpu);
}

static inline int filter_has_task_duration(struct filter* f, uint64_t duration)
{
	if(!f->filter_task_length)
		return 1;

	return (duration >= f->min_task_length &&
		duration <= f->max_task_length);
}

static inline int filter_has_comm_size(struct filter* f, uint64_t size)
{
	if(!f->filter_comm_size)
		return 1;

	return (size >= f->min_comm_size &&
		size <= f->max_comm_size);
}

static inline int filter_has_state_event(struct filter* f, struct state_event* se)
{
	uint64_t duration = 0;
	int valid = 1;

	if(f->filter_task_length) {
		if(se->active_task->addr != 0)
			duration = task_length_of_active_frame(se, &valid);

		if(!valid || !filter_has_task_duration(f, duration))
			return 0;
	}

	return filter_has_cpu(f, se->event_set->cpu) &&
		filter_has_task(f, se->active_task) &&
		filter_has_frame(f, se->active_frame) &&
		(!f->filter_writes_to_numa_nodes ||
		 event_set_has_write_to_numa_nodes_in_interval(se->event_set, &f->writes_to_numa_nodes, se->start, se->end, f->writes_to_numa_nodes_minsize) ||
		 (se->texec_start && se->texec_end &&
		  event_set_has_write_to_numa_nodes_in_interval(se->event_set, &f->writes_to_numa_nodes, se->texec_start->time, se->texec_end->time, f->writes_to_numa_nodes_minsize)));
}

static inline int filter_has_comm_event(struct filter* f, struct multi_event_set* mes, struct comm_event* ce)
{
	struct event_set* dst_cpu_es;
	struct event_set* src_cpu_es;
	int dst_cpu_idx;
	int src_cpu_idx;

	if(!filter_has_comm_size(f, ce->size))
		return 0;

	if(!filter_has_cpu(f, ce->event_set->cpu))
		return 0;

	if(!filter_has_comm_numa_node(f, ce->what->numa_node))
		return 0;

	if(ce->type == COMM_TYPE_DATA_WRITE) {
		if(!filter_has_writes_to_numa_nodes_node(f, ce->what->numa_node))
			return 0;

		if(f->filter_writes_to_numa_nodes && ce->size < f->writes_to_numa_nodes_minsize)
			return 0;
	}

	/* Duration of active task in filter? */
	if(ce->texec_start) {
		int64_t duration = ce->texec_end->time - ce->texec_start->time;
		if(!filter_has_task_duration(f, duration))
			return 0;
	}

	/* Active task *and* frame included in filter? */
	if(filter_has_task(f, ce->active_task) &&
	   filter_has_frame(f, ce->active_frame))
	{
		return 1;
	}

	/* Was the transferred entity a frame included in the filter? */
	if((ce->type == COMM_TYPE_STEAL ||
	    ce->type == COMM_TYPE_PUSH))
	{
		if(filter_has_frame(f, ce->what))
			return 1;
	}

	/* Was the destination worker executing a
	 * task and frame included in the filter?
	 */
	if(ce->type != COMM_TYPE_DATA_WRITE) {
		dst_cpu_es = multi_event_set_find_cpu(mes, ce->dst_cpu);
		dst_cpu_idx = event_set_get_enclosing_state(dst_cpu_es, ce->time);
	} else {
		dst_cpu_idx = -1;
	}

	if(dst_cpu_idx != -1 && !filter_has_cpu(f, dst_cpu_es->cpu))
		return 0;

	src_cpu_es = multi_event_set_find_cpu(mes, ce->src_cpu);
	src_cpu_idx = event_set_get_enclosing_state(src_cpu_es, ce->time);

	if((dst_cpu_idx != -1 && filter_has_state_event(f, &dst_cpu_es->state_events[dst_cpu_idx])) ||
	   (src_cpu_idx != -1 && filter_has_state_event(f, &src_cpu_es->state_events[src_cpu_idx])))
	{
		return 1;
	}

	return 0;
}

static inline int filter_has_single_event(struct filter* f, struct single_event* se)
{
	if(!filter_has_single_event_type(f, se->type) ||
	   !filter_has_task(f, se->active_task) ||
	   !filter_has_frame(f, se->active_frame) ||
	   !filter_has_cpu(f, se->event_set->cpu))
		return 0;

	switch(se->type) {
		case SINGLE_TYPE_TCREATE:
		case SINGLE_TYPE_TEXEC_START:
		case SINGLE_TYPE_TEXEC_END:
		case SINGLE_TYPE_TDESTROY:
			return filter_has_frame(f, se->what);
	}

	return 0;
}

static inline void filter_destroy(struct filter* f)
{
	free(f->tasks);
	free(f->frames);
	bitvector_destroy(&f->counters);
	bitvector_destroy(&f->frame_numa_nodes);
	bitvector_destroy(&f->comm_numa_nodes);
	bitvector_destroy(&f->cpus);
}

static inline void filter_set_task_length_filtering(struct filter* f, int b)
{
	f->filter_task_length = b;
}

static inline void filter_set_task_length_filtering_range(struct filter* f, int64_t min, int64_t max)
{
	f->min_task_length = min;
	f->max_task_length = max;
}

static inline void filter_set_comm_size_filtering(struct filter* f, int b)
{
	f->filter_comm_size = b;
}

static inline void filter_set_comm_size_filtering_range(struct filter* f, int64_t min, int64_t max)
{
	f->min_comm_size = min;
	f->max_comm_size = max;
}

#endif
