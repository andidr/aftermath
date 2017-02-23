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

#ifndef INTERVAL_ARRAY_H
#define INTERVAL_ARRAY_H

#include "refactor/autogen/base_types.h"
#include "bsearch.h"
#include "interval.h"
#include "typed_array.h"
#include "contrib/linux-kernel/stddef.h"

#define ACC_PINTERVAL_PIDENT(i) (&(i))

#define ACC_PINTERVAL_OVERLAP_SMALLER_EXPR(ia, ib)	\
	((ia)->end < (ib)->start)

#define ACC_PINTERVAL_OVERLAP_GREATER_EXPR(ia, ib)	\
	((ia)->start > (ib)->end)

DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(interval_array_,
				   _overlapping,
				   struct am_interval,
				   struct am_interval*,
				   ACC_PINTERVAL_PIDENT,
				   ACC_PINTERVAL_OVERLAP_SMALLER_EXPR,
				   ACC_PINTERVAL_OVERLAP_GREATER_EXPR)

/* This macro provides a generic way to implement a binary search function for
 * arrays of an event type T with an interval. The declared function returns the
 * first instance of type T whose interval overlaps with the query interval. If
 * no such instance exists, the function returns NULL.
 *
 * Example:
 *
 *   struct augmented_interval {
 *   	...
 *   	struct am_interval i;
 *   	int a;
 *   	int b;
 *   	struct some_other_struct sos;
 *   	...
 *   };
 *
 *   DECL_TYPED_ARRAY(augint, struct augmented_interval)
 *
 *   DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING(augint,
 *						struct augmented_interval,
 *						i)
 *
 * The example declares the function:
 *
 *   static inline struct augmented_interval*
 *   augint_array_bsearch_first_overlapping(struct augint_array* a,
 *                                          const struct am_interval* query);
 *
 * that returns the first instance of an augmented_interval structure in a whose
 * interval overlaps with the interval query.
 */
#define DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING(prefix, T, member)	\
	static inline T*							\
	prefix##_bsearch_first_overlapping(struct prefix* a,			\
					 const struct am_interval* query)	\
	{									\
		off_t offs = offsetof(T, member);				\
		off_t stride = sizeof(T);					\
		struct am_interval* base = (struct am_interval*)		\
			(((void*)a->elements)+offs);				\
		struct am_interval* res;					\
										\
		res = interval_array_bsearch_first_strided_overlapping(base,	\
									a->num_elements, \
									stride, \
									query); \
										\
		return (res) ? (((void*)res)-offsetof(T, member)) : NULL;	\
	}

#endif
