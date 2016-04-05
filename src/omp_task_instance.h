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

#ifndef OMP_TASK_INSTANCE_H
#define OMP_TASK_INSTANCE_H

#define OMP_TASK_INSTANCE_PREALLOC 16

#include <stdlib.h>
#include <stdint.h>
#include "multi_event_set.h"
#include "omp_task_part.h"

struct omp_trace_info_oti {
	uint64_t addr;
	uint64_t oti_id;
	uint32_t artificial;

	uint32_t next_task_instance_id;
	uint32_t task_id;
	uint32_t first_task_part_id;
};

struct omp_task_instance {
	union {
		/* The field ti_oti is only used when a trace is
		 * loaded and is replaced by a pointer to the
		 * associated task */
		struct omp_task* task;
		struct omp_trace_info_oti* ti_oti;
	};

	struct list_head task_parts;
	struct list_head list;

	uint32_t num_task_parts;

	double color_r;
	double color_g;
	double color_b;
};

static inline int omp_task_instance_init(struct omp_task_instance* oti, uint64_t dsk_id, uint64_t addr)
{
	if(!(oti->ti_oti = malloc(sizeof(struct omp_trace_info_oti))))
		return 1;

	oti->num_task_parts = 0;
	oti->ti_oti->oti_id = dsk_id;
	oti->ti_oti->artificial = 0;

	return 0;
}

static inline struct omp_task_instance* omp_task_instance_find(struct multi_event_set* mes, uint64_t dsk_id)
{
	for(int i = 0; i < mes->num_omp_task_instances; i++)
		if(mes->omp_task_instances[i].ti_oti->oti_id == dsk_id)
			return &mes->omp_task_instances[i];

	return NULL;
}

static inline int omp_task_instance_alloc(struct multi_event_set* mes, uint64_t dsk_id, uint64_t addr)

{
	if(check_buffer_grow((void**)&mes->omp_task_instances, sizeof(struct omp_task_instance),
			  mes->num_omp_task_instances, &mes->num_omp_task_instances_free,
			     OMP_TASK_INSTANCE_PREALLOC))
	{
		return 1;
	}

	if(omp_task_instance_init(&mes->omp_task_instances[mes->num_omp_task_instances], dsk_id, addr) == 0)
	{
		mes->num_omp_task_instances_free--;
		mes->num_omp_task_instances++;

		return 0;
	}

	return 1;
}

static inline struct omp_task_instance* omp_task_instance_alloc_ptr(struct multi_event_set* mes, uint64_t dsk_id, uint64_t addr)
{
	if(omp_task_instance_alloc(mes, dsk_id, addr) != 0)
		return NULL;

	return &mes->omp_task_instances[mes->num_omp_task_instances - 1];
}

static inline struct omp_task_instance* omp_task_instance_add_part(struct multi_event_set* mes, struct omp_task_part* otp)
{
	struct omp_task_instance* oti;

	if(!(oti = omp_task_instance_find(mes, otp->ti_otp->task_instance_id))) {
		if(!(oti = omp_task_instance_alloc_ptr(mes, otp->ti_otp->task_instance_id, -1)))
			return NULL;

		oti->ti_oti->artificial = 1;
	}

	oti->num_task_parts++;

	return oti;
}

#endif
