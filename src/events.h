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

#include <stdint.h>
#include "trace/trace_file.h"
#include "task.h"
#include "frame.h"

#define EVENT_PREALLOC (5*1024)

struct single_event {
	struct event_set* event_set;
	enum single_event_type type;

	uint64_t time;

	struct task* active_task;
	struct frame* active_frame;

	struct frame* what;

	struct single_event* next_texec_start;
	struct single_event* prev_texec_start;

	struct single_event* next_texec_end;
	struct single_event* prev_texec_end;
};

struct global_single_event {
	uint64_t time;
	enum global_single_event_type type;
};

struct state_event {
	struct event_set* event_set;
	uint64_t start;
	uint64_t end;

	struct task* active_task;
	struct frame* active_frame;

	int state_id_seq;
	int state_id;

	struct single_event* texec_start;
	struct single_event* texec_end;
};

struct comm_event {
	struct event_set* event_set;
	uint64_t time;

	int dst_cpu;
	int src_cpu;

	int size;
	enum comm_event_type type;

	struct frame* what;
	struct task* active_task;
	struct frame* active_frame;

	struct single_event* texec_start;
	struct single_event* texec_end;

	uint64_t prod_ts;
};

struct counter_event {
	uint64_t time;
	int64_t value;
	long double slope;
};

/*
 * Interpolates the counter value between two samples using linear
 * interpolation
 */
static inline int64_t counter_event_interpolate_value(struct counter_event* left,
						      struct counter_event* right,
						      int64_t time)
{
	if(left->value == right->value)
		return left->value;

	long double delta = ((long double)right->value) - ((long double)left->value);
	long double slope = delta / ((long double)(right->time - left->time));
	long double ival = ((long double)left->value) + slope*((long double)time - left->time);

	return ival;
}

static inline uint64_t task_length_of_active_frame(struct state_event* se, int* valid)
{
	*valid = 0;

	/* Might be the case for main task, for example */
	if(!se->texec_start || !se->texec_start->next_texec_end)
		return 0;

	uint64_t task_start = se->texec_start->time;
	uint64_t task_end = se->texec_start->next_texec_end->time;

	if(task_end < se->start || task_start > se->end)
		return 0;

	*valid = 1;
	return task_end - task_start;
}

static inline uint64_t state_event_length_in_interval(struct state_event* se, uint64_t start, uint64_t end)
{
	if(se->start > end || se->end < start)
		return 0;

	if(se->start >= start && se->end <= end)
		return se->end - se->start;

	if(se->start <= start && se->end <= end)
		return se->end - start;

	if(se->start <= start && se->end >= end)
		return end - start;

	if(se->start <= end && se->end >= end)
		return end - se->start;

	/* Should never happen */
	return 0;
}

static inline uint64_t task_length_in_interval(struct single_event* texec_start, struct single_event* texec_end, uint64_t start, uint64_t end)
{
	if(texec_start->time > end || texec_end->time < start)
		return 0;

	if(texec_start->time >= start && texec_end->time <= end)
		return texec_end->time - texec_start->time;

	if(texec_start->time <= start && texec_end->time <= end)
		return texec_end->time - start;

	if(texec_start->time <= start && texec_end->time >= end)
		return end - start;

	if(texec_start->time <= end && texec_end->time >= end)
		return end - texec_start->time;

	/* Should never happen */
	return 0;
}

#endif
