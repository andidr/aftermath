/**
 * Copyright (C) 2015 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#ifndef OMP_FOR_INSTANCE_H
#define OMP_FOR_INSTANCE_H

#define OMP_FOR_INSTANCE_PREALLOC 16

#include <stdlib.h>
#include <stdint.h>
#include "multi_event_set.h"
#include "omp_for_chunk_set.h"

struct omp_trace_info_ofi {
	uint64_t addr;
	uint64_t ofi_id;
	uint32_t artificial;

	uint32_t next_for_instance_id;
	uint32_t for_loop_id;
	uint32_t first_chunk_set_id;
};

struct omp_for_instance {
	union {
		/* The field ti_ofi is only used when a trace is
		 * loaded and is replaced by a pointer to the
		 * associated for_loop */
		struct omp_for* for_loop;
		struct omp_trace_info_ofi* ti_ofi;
	};

	struct list_head for_chunk_sets;
	struct list_head list;

	uint32_t num_chunk_sets;
	uint32_t flags;
	uint64_t increment;
	uint64_t iter_start;
	uint64_t iter_end;
	uint32_t num_workers;

	double color_r;
	double color_g;
	double color_b;
};

static inline int omp_for_instance_init(struct omp_for_instance* ofi, uint32_t flags, uint64_t dsk_id,
					uint64_t increment, uint64_t iter_start, uint64_t iter_end, uint64_t addr,
					uint32_t num_workers)
{
	if(!(ofi->ti_ofi = malloc(sizeof(struct omp_trace_info_ofi))))
		return 1;

	ofi->num_chunk_sets = 0;
	ofi->flags = flags;
	ofi->increment = increment;
	ofi->iter_start = iter_start;
	ofi->iter_end = iter_end;
	ofi->num_workers = num_workers;
	ofi->ti_ofi->addr = addr;
	ofi->ti_ofi->ofi_id = dsk_id;
	ofi->ti_ofi->artificial = 0;

	return 0;
}

static inline struct omp_for_instance* omp_for_instance_find(struct multi_event_set* mes, uint64_t dsk_id)
{
	for(int i = 0; i < mes->num_omp_for_instances; i++)
		if(mes->omp_for_instances[i].ti_ofi->ofi_id == dsk_id)
			return &mes->omp_for_instances[i];

	return NULL;
}

static inline int omp_for_instance_alloc(struct multi_event_set* mes, uint32_t flags, uint64_t dsk_id,
					 uint64_t increment, uint64_t iter_start, uint64_t iter_end,
					 uint64_t addr, uint32_t num_workers)

{
	if(check_buffer_grow((void**)&mes->omp_for_instances, sizeof(struct omp_for_instance),
			  mes->num_omp_for_instances, &mes->num_omp_for_instances_free,
			     OMP_FOR_INSTANCE_PREALLOC))
	{
		return 1;
	}

	if(omp_for_instance_init(&mes->omp_for_instances[mes->num_omp_for_instances], flags,
				 dsk_id, increment, iter_start, iter_end, addr, num_workers) == 0)
	{
		mes->num_omp_for_instances_free--;
		mes->num_omp_for_instances++;

		return 0;
	}

	return 1;
}

static inline struct omp_for_instance* omp_for_instance_alloc_ptr(struct multi_event_set* mes, uint32_t flags, uint64_t dsk_id, uint64_t increment, uint64_t iter_start, uint64_t iter_end, uint64_t addr, uint32_t num_workers)
{
	if(omp_for_instance_alloc(mes, flags, dsk_id, increment,
				  iter_start, iter_end, addr, num_workers) != 0)
	{
		return NULL;
	}

	return &mes->omp_for_instances[mes->num_omp_for_instances - 1];
}

#endif
