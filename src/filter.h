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

struct filter {
	struct task** tasks;
	int num_tasks;
	int num_tasks_free;
	int filter_tasks;

	struct frame** frames;
	int num_frames;
	int num_frames_free;
	int filter_frames;

	struct bitvector counters;
	int filter_counters;

	int filter_counter_values;
	int64_t min;
	int64_t max;

	int filter_counter_slopes;
	long double min_slope;
	long double max_slope;
};

static inline int filter_init(struct filter* f, int64_t min, int64_t max,
			      long double min_slope, long double max_slope)
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

	f->filter_counter_values = 0;
	f->min = min;
	f->max = max;

	f->filter_counter_slopes = 0;
	f->min_slope = min_slope;
	f->max_slope = max_slope;

	if(bitvector_init(&f->counters, FILTER_COUNTER_BITS))
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

	filter_set_task_filtering(f, 0);
}

static inline int filter_add_task(struct filter* f, struct task* t)
{
	filter_set_task_filtering(f, 1);

	return add_buffer_grow((void**)&f->tasks, &t, sizeof(t),
			&f->num_tasks, &f->num_tasks_free,
			FILTER_TASK_PREALLOC);
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

void filter_sort_tasks(struct filter* f);
int filter_has_task(struct filter* f, uint64_t work_fn);

void filter_sort_frames(struct filter* f);
int filter_has_frame(struct filter* f, uint64_t addr);

static inline int filter_has_state_event(struct filter* f, struct state_event* se)
{
	return filter_has_task(f, se->active_task) &&
		filter_has_frame(f, se->active_frame);
}

static inline int filter_has_comm_event(struct filter* f, struct multi_event_set* mes, struct comm_event* ce)
{
	if(ce->type == COMM_TYPE_DATA_READ &&
	   (!filter_has_task(f, ce->active_task) ||
	    !filter_has_frame(f, ce->active_frame)))
	{
		struct event_set* dst_es = multi_event_set_find_cpu(mes, ce->dst_cpu);
		int idx = event_set_get_enclosing_state(dst_es, ce->time);

		if(idx == -1 || !filter_has_state_event(f, &dst_es->state_events[idx]))
			return 0;
	} else if((ce->type == COMM_TYPE_STEAL || ce->type == COMM_TYPE_PUSH) &&
		  (!filter_has_task(f, ce->active_task) ||
		   !filter_has_frame(f, ce->active_frame) ||
		   !filter_has_frame(f, ce->what)))
	{
		struct event_set* dst_es = multi_event_set_find_cpu(mes, ce->dst_cpu);
		int idx = event_set_get_enclosing_state(dst_es, ce->time);

		if(idx == -1 ||
		   (!filter_has_task(f, dst_es->state_events[idx].active_task) ||
		    !filter_has_frame(f, dst_es->state_events[idx].active_frame)))
			return 0;

		if(idx == -1 ||
		   !(filter_has_state_event(f, &dst_es->state_events[idx]) ||
		     filter_has_frame(f, ce->what)))
			return 0;
	}

	return 1;
}

static inline void filter_destroy(struct filter* f)
{
	free(f->tasks);
	free(f->frames);
	bitvector_destroy(&f->counters);
}

#endif
