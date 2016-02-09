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

#ifndef OMP_FOR_H
#define OMP_FOR_H

#define OMP_FOR_PREALLOC 8

#include <stdint.h>
#include "multi_event_set.h"
#include "omp_for_instance.h"

struct omp_trace_info_of {
	uint32_t first_for_instance_id;
};

struct omp_for {
	struct list_head for_instances;
	uint64_t addr;
	uint32_t num_instances;
	union {
		/* The field ti_of is only use at loading time and 
		 * later freed to be replaced by a pointer to 
		 * the string giving the source file's name */
		char* source_filename;
		struct omp_trace_info_of ti_of;
	};

	char* symbol_name;
	int source_line;

	double color_r;
	double color_g;
	double color_b;
};

static inline int omp_for_init(struct omp_for* of, uint64_t addr)
{
	of->addr = addr;
	of->num_instances = 0;
	of->source_line = -1;
	of->symbol_name = NULL;

	return 0;
}

static inline struct omp_for* omp_for_find(struct multi_event_set* mes, uint64_t addr)
{
	for(int i = 0; i < mes->num_omp_fors; i++)
		if(mes->omp_fors[i].addr == addr)
			return &mes->omp_fors[i];

	return NULL;
}

static inline int omp_for_alloc(struct multi_event_set* mes, uint64_t addr)
{
	if(check_buffer_grow((void**)&mes->omp_fors, sizeof(struct omp_for),
			  mes->num_omp_fors, &mes->num_omp_fors_free,
			     OMP_FOR_PREALLOC))
	{
		return 1;
	}

	if(omp_for_init(&mes->omp_fors[mes->num_omp_fors], addr) == 0) {
		mes->num_omp_fors_free--;
		mes->num_omp_fors++;

		return 0;
	}

	return 1;
}

static inline struct omp_for* omp_for_alloc_ptr(struct multi_event_set* mes, uint64_t addr)
{
	if(omp_for_alloc(mes, addr) != 0)
		return NULL;

	return &mes->omp_fors[mes->num_omp_fors - 1];
}

#endif
