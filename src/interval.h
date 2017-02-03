/**
 * Copyright (C) 2017 Andi Drebes <andi.drebes@lip6.fr>
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

#ifndef INTERVAL_H
#define INTERVAL_H

#include "trace/types.h"

/* Checks if two intervals overlap. Returns true if this is the case, otherwise
 * false. */
static inline int interval_intersect_p(const struct am_interval* a,
				       const struct am_interval* b)
{
	return !(a->end < b->start || a->start > b->end);
}

/* Calculates the overlap between two intervals. If the intervals do not
 * overlap, the function returns 1. If they do overlap, out is initialized with
 * the intersection and the function returns 0. */
static inline int interval_intersection(const struct am_interval* a,
					const struct am_interval* b,
					struct am_interval* out)
{
	if(!interval_intersect_p(a, b))
		return 1;

	out->start = (a->start <= b->start) ? b->start : a->start;
	out->end = (a->end <= b->end) ? a->end : b->end;

	return 0;
}

/* Calculates the duration of an interval. */
static inline am_timestamp_diff_t interval_duration(const struct am_interval* i)
{
	return i->end - i->start;
}

#endif
