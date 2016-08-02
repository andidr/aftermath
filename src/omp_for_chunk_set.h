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

#ifndef OMP_FOR_CHUNK_H
#define OMP_FOR_CHUNK_H

#define OMP_FOR_CHUNK_PREALLOC 16

#include <stdint.h>
#include "multi_event_set.h"
#include "omp_for_chunk_set_part.h"

struct omp_trace_info_ofc {
	uint64_t for_id;
	uint64_t id;
	uint32_t artificial;

	uint32_t next_chunk_set_id;
	uint32_t for_instance_id;
};


struct omp_for_chunk_set {
	union {
		/* The field ti_ofc is only used when a trace is
		 * loaded and is later replaced by a pointer to the
		 * associated for instance */
		struct omp_for_instance *for_instance;
		struct omp_trace_info_ofc *ti_ofc;
	};

	struct list_head list;
	struct list_head chunk_set_parts;

	uint32_t num_chunk_set_parts;
	uint64_t iter_start;
	uint64_t iter_end;

	double color_r;
	double color_g;
	double color_b;
};

static inline int omp_for_chunk_set_init(struct omp_for_chunk_set* ofc, uint64_t dsk_for_id, uint64_t dsk_id,
				     uint64_t iter_start, uint64_t iter_end)
{
	if(!(ofc->ti_ofc = malloc(sizeof(struct omp_trace_info_ofc))))
		return 1;

	ofc->num_chunk_set_parts = 0;
	ofc->ti_ofc->for_id = dsk_for_id;
	ofc->ti_ofc->id = dsk_id;
	ofc->iter_start = iter_start;
	ofc->iter_end = iter_end;
	ofc->ti_ofc->artificial = 0;

	return 0;
}

static inline struct omp_for_chunk_set* omp_for_chunk_set_find(struct multi_event_set* mes, uint64_t dsk_id)
{
	for(int i = 0; i < mes->num_omp_for_chunk_sets; i++)
		if(mes->omp_for_chunk_sets[i].ti_ofc->id == dsk_id)
			return &mes->omp_for_chunk_sets[i];

	return NULL;
}

static inline int omp_for_chunk_set_alloc(struct multi_event_set* mes, uint64_t dsk_for_id,
				      uint64_t dsk_id, uint64_t iter_start, uint64_t iter_end)

{
	if(check_buffer_grow((void**)&mes->omp_for_chunk_sets, sizeof(struct omp_for_chunk_set),
			  mes->num_omp_for_chunk_sets, &mes->num_omp_for_chunk_sets_free,
			     OMP_FOR_CHUNK_PREALLOC))
	{
		return 1;
	}

	if(omp_for_chunk_set_init(&mes->omp_for_chunk_sets[mes->num_omp_for_chunk_sets], dsk_for_id,
			      dsk_id, iter_start, iter_end) == 0)
	{
		mes->num_omp_for_chunk_sets_free--;
		mes->num_omp_for_chunk_sets++;

		return 0;
	}

	return 1;
}

static inline struct omp_for_chunk_set* omp_for_chunk_set_alloc_ptr(struct multi_event_set* mes, uint64_t dsk_for_id,
							    uint64_t dsk_id, uint64_t iter_start, uint64_t iter_end)
{
	if(omp_for_chunk_set_alloc(mes, dsk_for_id, dsk_id, iter_start, iter_end) != 0)
		return NULL;

	return &mes->omp_for_chunk_sets[mes->num_omp_for_chunk_sets - 1];
}

static inline struct omp_for_chunk_set* omp_for_chunk_set_add_part(struct multi_event_set* mes, struct omp_for_chunk_set_part* ofcp)
{
	struct omp_for_chunk_set* ofc;

	if(!(ofc = omp_for_chunk_set_find(mes, ofcp->ti_ofcp->chunk_set_id))) {
		if(!(ofc = omp_for_chunk_set_alloc_ptr(mes, -1, ofcp->ti_ofcp->chunk_set_id, -1, -1)))
			return NULL;

		ofc->ti_ofc->artificial = 1;
	}

	ofc->num_chunk_set_parts++;

	return ofc;
}

#endif
