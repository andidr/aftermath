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

#ifndef COUNTER_DESCRIPTION_H
#define COUNTER_DESCRIPTION_H

#include <stdint.h>
#include <stdlib.h>
#include <float.h>

#define COUNTER_PREALLOC 16

struct counter_description {
	uint64_t counter_id;
	int64_t min;
	int64_t max;
	long double min_slope;
	long double max_slope;
	int slope_mode;
	int index;
	char* name;

	double color_r;
	double color_g;
	double color_b;
};

static inline int counter_description_init(struct counter_description* cd, int index, uint64_t counter_id, int name_len)
{
	cd->counter_id = counter_id;
	cd->index = index;
	cd->name = malloc(name_len+1);
	cd->min = INT64_MAX;
	cd->max = INT64_MIN;
	cd->min_slope = DBL_MAX;
	cd->max_slope = DBL_MIN;
	cd->slope_mode = 0;
	cd->color_r = 1.0;
	cd->color_g = 0.0;
	cd->color_b = 0.0;

	if(cd->name == NULL)
		return 1;

	return 0;
}

static inline void counter_description_destroy(struct counter_description* cd)
{
	free(cd->name);
}

#endif
