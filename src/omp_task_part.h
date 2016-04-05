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

#ifndef OMP_TASK_PART_H
#define OMP_TASK_PART_H

#define OMP_TASK_PART_PREALLOC 16

#include <stdint.h>
#include "./contrib/linux-kernel/list.h"
#include "event_set.h"

struct omp_trace_info_otp {
	uint64_t task_instance_id;
};

struct omp_task_part {
	union {
		/* The field ti_ofcp is only used when a trace is
		 * loaded and is replaced by a pointer to the
		 * associated task_instance */
		struct omp_task_instance *task_instance;
		struct omp_trace_info_otp* ti_otp;
	};

	struct list_head list;

	uint32_t cpu;
	uint64_t start;
	uint64_t end;

	double color_r;
	double color_g;
	double color_b;
};

static inline int omp_task_part_init(struct omp_task_part* otp, uint64_t dsk_task_instance_id, uint32_t cpu,
				     uint64_t start, uint64_t end)
{
	if(!(otp->ti_otp = malloc(sizeof(struct omp_trace_info_otp))))
		return 1;

	otp->ti_otp->task_instance_id = dsk_task_instance_id;
	otp->cpu = cpu;
	otp->start = start;
	otp->end = end;

	return 0;
}

static inline int omp_task_part_add(struct event_set* es, struct omp_task_part* otp)
{
	if(add_buffer_grow((void **)&es->omp_task_parts, otp, sizeof(*otp),
				&es->num_omp_task_parts, &es->num_omp_task_parts_free,
				OMP_TASK_PART_PREALLOC) != 0)
	{
		return 0;
	}

	if(otp->start < es->first_start)
		es->first_start = otp->start;

	if(otp->end > es->last_end)
		es->last_end = otp->end;

	return 0;
}

static inline struct omp_task_part* omp_task_part_find(struct event_set* es, uint64_t cpu, uint64_t start)
{
	int start_idx = 0;
	int end_idx = es->num_omp_task_parts - 1;
	int center_idx = 0;

	if(es->num_omp_task_parts == 0)
		return NULL;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->omp_task_parts[center_idx].start > start)
			end_idx = center_idx - 1;
		else if(es->omp_task_parts[center_idx].start < start)
			start_idx = center_idx + 1;
		else
			break;
	}

	if(es->omp_task_parts[center_idx].start == start)
		return &es->omp_task_parts[center_idx];

	return NULL;
}

static inline int omp_task_part_has_part(struct event_set* es, struct omp_task_part* otp)
{
	return omp_task_part_find(es, otp->cpu, otp->start) != NULL;
}

#endif
