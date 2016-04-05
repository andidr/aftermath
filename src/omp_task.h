/**
 * Copyright (C) 2016 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#ifndef OMP_TASK_H
#define OMP_TASK_H

#define OMP_TASK_PREALLOC 8

#include <stdint.h>
#include "multi_event_set.h"
#include "omp_task_instance.h"

struct omp_trace_info_ot {
	uint32_t first_task_instance_id;
};

struct omp_task {
	struct list_head task_instances;
	uint64_t addr;
	uint32_t num_instances;
	union {
		/* The field ti_of is only use at loading time and 
		 * later freed to be replaced by a pointer to 
		 * the string giving the source file's name */
		char* source_filename;
		struct omp_trace_info_ot ti_ot;
	};

	char* symbol_name;
	int source_line;

	double color_r;
	double color_g;
	double color_b;
};

static inline int omp_task_init(struct omp_task* ot, uint64_t addr)
{
	ot->addr = addr;
	ot->num_instances = 0;
	ot->source_line = -1;
	ot->symbol_name = NULL;

	return 0;
}

static inline struct omp_task* omp_task_find(struct multi_event_set* mes, uint64_t addr)
{
	for(int i = 0; i < mes->num_omp_tasks; i++)
		if(mes->omp_tasks[i].addr == addr)
			return &mes->omp_tasks[i];

	return NULL;
}

static inline int omp_task_alloc(struct multi_event_set* mes, uint64_t addr)
{
	if(check_buffer_grow((void**)&mes->omp_tasks, sizeof(struct omp_task),
			  mes->num_omp_tasks, &mes->num_omp_tasks_free,
			     OMP_TASK_PREALLOC))
	{
		return 1;
	}

	if(omp_task_init(&mes->omp_tasks[mes->num_omp_tasks], addr) == 0) {
		mes->num_omp_tasks_free--;
		mes->num_omp_tasks++;

		return 0;
	}

	return 1;
}

static inline struct omp_task* omp_task_alloc_ptr(struct multi_event_set* mes, uint64_t addr)
{
	if(omp_task_alloc(mes, addr) != 0)
		return NULL;

	return &mes->omp_tasks[mes->num_omp_tasks - 1];
}

#endif
