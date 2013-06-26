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

#define FILTER_TASK_PREALLOC 16

struct filter {
	struct task** tasks;
	int num_tasks;
	int num_tasks_free;
};

static inline void filter_init(struct filter* f)
{
	f->tasks = NULL;
	f->num_tasks = 0;
	f->num_tasks_free = 0;
}

static inline void filter_clear_tasks(struct filter* f)
{
	f->num_tasks_free += f->num_tasks;
	f->num_tasks = 0;
}

static inline int filter_add_task(struct filter* f, struct task* t)
{
	return add_buffer_grow((void**)&f->tasks, &t, sizeof(t),
			&f->num_tasks, &f->num_tasks_free,
			FILTER_TASK_PREALLOC);
}

void filter_sort_tasks(struct filter* f);
int filter_has_task(struct filter* f, uint64_t work_fn);

static inline void filter_destroy(struct filter* f)
{
	free(f->tasks);
}

#endif
