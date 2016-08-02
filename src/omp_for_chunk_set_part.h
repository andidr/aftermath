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

#ifndef OMP_FOR_CHUNK_PART_H
#define OMP_FOR_CHUNK_PART_H

#define OMP_FOR_CHUNK_PART_PREALLOC 16

#include <stdint.h>
#include "./contrib/linux-kernel/list.h"
#include "event_set.h"

struct omp_trace_info_ofcp {
	uint64_t chunk_set_id;
};

struct omp_for_chunk_set_part {
	union {
		/* The field ti_ofcp is only used when a trace is
		 * loaded and is replaced by a pointer to the
		 * associated chunk_set */
		struct omp_for_chunk_set *chunk_set;
		struct omp_trace_info_ofcp* ti_ofcp;
	};

	struct list_head list;

	uint32_t cpu;
	uint64_t start;
	uint64_t end;

	double color_r;
	double color_g;
	double color_b;
};

static inline int omp_for_chunk_set_part_init(struct omp_for_chunk_set_part* ofcp, uint64_t dsk_chunk_set_id, uint32_t cpu,
					  uint64_t start, uint64_t end)
{
	if(!(ofcp->ti_ofcp = malloc(sizeof(struct omp_trace_info_ofcp))))
		return 1;

	ofcp->ti_ofcp->chunk_set_id = dsk_chunk_set_id;
	ofcp->cpu = cpu;
	ofcp->start = start;
	ofcp->end = end;

	return 0;
}

static inline int omp_for_chunk_set_part_add(struct event_set* es, struct omp_for_chunk_set_part* ofcp)
{
	if(add_buffer_grow((void **)&es->omp_for_chunk_set_parts, ofcp, sizeof(*ofcp),
				&es->num_omp_for_chunk_set_parts, &es->num_omp_for_chunk_set_parts_free,
				OMP_FOR_CHUNK_PART_PREALLOC) != 0)
	{
		return 0;
	}

	if(ofcp->start < es->first_start)
		es->first_start = ofcp->start;

	if(ofcp->end > es->last_end)
		es->last_end = ofcp->end;

	return 0;
}

static inline struct omp_for_chunk_set_part* omp_for_chunk_set_part_find(struct event_set* es, uint64_t cpu, uint64_t start)
{
	int start_idx = 0;
	int end_idx = es->num_omp_for_chunk_set_parts - 1;
	int center_idx = 0;

	if(es->num_omp_for_chunk_set_parts == 0)
		return NULL;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->omp_for_chunk_set_parts[center_idx].start > start)
			end_idx = center_idx - 1;
		else if(es->omp_for_chunk_set_parts[center_idx].start < start)
			start_idx = center_idx + 1;
		else
			break;
	}

	if(es->omp_for_chunk_set_parts[center_idx].start == start)
		return &es->omp_for_chunk_set_parts[center_idx];

	return NULL;
}

static inline int omp_for_chunk_set_part_has_part(struct event_set* es, struct omp_for_chunk_set_part* ofcp)
{
	return omp_for_chunk_set_part_find(es, ofcp->cpu, ofcp->start) != NULL;
}

int compare_ofcpsp(const void *po1, const void *po2);
#endif
