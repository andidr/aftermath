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

#define FILTER_TASK_PREALLOC 16
#define FILTER_COUNTER_BITS 64

struct filter {
	struct task** tasks;
	int num_tasks;
	int num_tasks_free;
	int filter_tasks;

	struct bitvector counters;
	int filter_counters;
};

static inline int filter_init(struct filter* f)
{
	f->tasks = NULL;
	f->num_tasks = 0;
	f->num_tasks_free = 0;
	f->filter_tasks = 0;
	f->filter_counters = 0;

	if(bitvector_init(&f->counters, FILTER_COUNTER_BITS))
		return 1;

	return 0;
}

static inline void filter_clear_tasks(struct filter* f)
{
	f->num_tasks_free += f->num_tasks;
	f->num_tasks = 0;
	f->filter_tasks = 0;
}

static inline int filter_add_task(struct filter* f, struct task* t)
{
	f->filter_tasks = 1;

	return add_buffer_grow((void**)&f->tasks, &t, sizeof(t),
			&f->num_tasks, &f->num_tasks_free,
			FILTER_TASK_PREALLOC);
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

static inline void filter_destroy(struct filter* f)
{
	free(f->tasks);
	bitvector_destroy(&f->counters);
}

#endif
