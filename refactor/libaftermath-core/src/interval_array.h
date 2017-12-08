/**
 * Author: Andi Drebes <andi@drebesium.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_INTERVAL_ARRAY_H
#define AM_INTERVAL_ARRAY_H

#include <aftermath/core/base_types.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/interval.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/ptr.h>
#include <aftermath/core/ansi_extras.h>
#include <aftermath/core/contrib/linux-kernel/stddef.h>

#define ACC_PINTERVAL_PIDENT(i) (&(i))

#define ACC_PINTERVAL_OVERLAP_SMALLER_EXPR(ia, ib)	\
	((ia)->end < (ib)->start)

#define ACC_PINTERVAL_OVERLAP_GREATER_EXPR(ia, ib)	\
	((ia)->start > (ib)->end)

AM_DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(am_interval_array_,
				      _overlapping,
				      struct am_interval,
				      struct am_interval*,
				      ACC_PINTERVAL_PIDENT,
				      ACC_PINTERVAL_OVERLAP_SMALLER_EXPR,
				      ACC_PINTERVAL_OVERLAP_GREATER_EXPR)

AM_DECL_VSTRIDED_BSEARCH_LAST_SUFFIX(am_interval_array_,
				      _overlapping,
				      struct am_interval,
				      struct am_interval*,
				      ACC_PINTERVAL_PIDENT,
				      ACC_PINTERVAL_OVERLAP_SMALLER_EXPR,
				      ACC_PINTERVAL_OVERLAP_GREATER_EXPR)

/* Internal use; Used as an iterator in for_each macros */
struct am_interval_array_iterator {
	struct am_interval* start;
	struct am_interval* end;
};

/* Internal use; Returns a newly intialized interval array iterator and sets *i
 * to the first interval or NULL if no such interval exists.
 */
static inline struct am_interval_array_iterator
am_interval_array_iterator_start(struct am_typed_array_generic* arr,
				 off_t stride,
				 off_t field_offset,
				 const struct am_interval* query,
				 struct am_interval** i)
{
	struct am_interval_array_iterator it;
	struct am_interval* first_field;

	/* Address of the interval field of the first array element */
	first_field = ((struct am_interval*)((arr->elements)+field_offset));

	/* Address of the interval field of the first array element whose
	 * interval overlaps with the query interval */
	it.start = am_interval_array_bsearch_first_strided_overlapping(
		first_field,
		arr->num_elements,
		stride,
		query);

	*i = it.start;

	if(it.start) {
		/* Address of the interval field of the last array element whose
		 * interval overlaps with the query interval */
		it.end = am_interval_array_bsearch_last_strided_overlapping(
			first_field,
			arr->num_elements,
			stride,
			query);
	}

	return it;
}

/* Internal use; Returns a newly intialized interval array iterator, sets *i to
 * the first interval or NULL if no such interval exists and *uint_val to the
 * value of the unsigned integer of uint_field_bits bits at the offset
 * uint_field_offset. Uint_field_bits must be either 8, 16, 32 or 64. */
static inline struct am_interval_array_iterator
am_interval_array_iterator_start_intn(struct am_typed_array_generic* arr,
				      off_t stride,
				      off_t interval_field_offset,
				      off_t uint_field_offset,
				      unsigned int uint_field_bits,
				      const struct am_interval* query,
				      struct am_interval** i,
				      void* uint_val,
				      unsigned int uint_val_bits)
{
	struct am_interval_array_iterator it;
	void* uint_field;

	it = am_interval_array_iterator_start(arr, stride, interval_field_offset,
					      query, i);

	if(!it.start)
		return it;

	uint_field = ((void*)it.start) - interval_field_offset +
		uint_field_offset;

	am_assign_uint(uint_val, uint_val_bits, uint_field, uint_field_bits);

	return it;
}

/* Iterate over a typed array of elements that contain an interval, starting
 * with the interval of the element that first overlaps with a query interval
 * and ending with the interval of the last element that overlaps with the query
 * interval. The argument parr must be a pointer to a typed array cast to struct
 * am_typed_array_generic, pi a pointer to a struct am_interval that serves as
 * an iterator, element_size is the size in bytes of an array element,
 * field_offset is the offset of the interval field of an array element, and
 * pquery is a pointer to the query interval.
 */
#define am_interval_array_for_each_overlapping_offs(parr, pi, element_size,	\
						    field_offset, pquery)	\
	for(struct am_interval_array_iterator __it =				\
		    am_interval_array_iterator_start(				\
			    parr,						\
			    element_size,					\
			    field_offset,					\
			    pquery,						\
			    &pi);						\
	    (pi) && AM_PTR_LEQ(pi, __it.end);					\
	    (pi) = ((void*)pi) + element_size)

/* Iterate over a typed array of elements that contain an interval, starting
 * with the interval of the element that first overlaps with a query interval
 * and ending with the interval of the last element that overlaps with the query
 * interval. The argument parr must be a pointer to a typed array cast to struct
 * am_typed_array_generic, pi a pointer to a struct am_interval that serves as
 * an iterator, field is the name of the interval field of an array element, and
 * pquery is a pointer to the query interval.
 */
#define am_interval_array_for_each_overlapping(parr, pi, field, pquery) \
	am_interval_array_for_each_overlapping_offs(			\
		parr,							\
		pi,							\
		sizeof((parr)->elements[0]),				\
		AM_OFFSETOF_PTR((parr)->elements[0], field),		\
		pquery)

/* Same as am_interval_array_for_each_overlapping_offs, but extracts an unsigned
 * integer field at each iteration from the current array element and assigns
 * the value to *puint. Uint_field_offset specifies the offset of the unsigned
 * integer field in an element and uint_field_bits indicates the size of the
 * unsigned integer in bits. The number of bits must be 8, 16, 32 or 64.
 */
#define am_interval_array_for_each_overlapping_uint_offs(parr, pi, puint,	\
							 puint_bits,		\
							 element_size,		\
							 interval_field_offset, \
							 uint_field_offset,	\
							 uint_field_bits,	\
							 pquery)		\
	for(struct am_interval_array_iterator __it =				\
		    am_interval_array_iterator_start_intn(			\
			    parr,						\
			    element_size,					\
			    interval_field_offset,				\
			    uint_field_offset,					\
			    uint_field_bits,					\
			    pquery,						\
			    &pi,						\
			    puint,						\
			    puint_bits);					\
	    (pi) && AM_PTR_LEQ(pi, __it.end);					\
	    (pi) = ((void*)pi) + element_size,					\
		    am_assign_uint(puint, puint_bits,				\
				   ((void*)pi) - interval_field_offset +	\
				   uint_field_offset, uint_field_bits))

/* Same as am_interval_array_for_each_overlapping, but extracts the unsigned
 * integer field specified by interval_field at each iteration from the current
 * array element and assigns the value to *puint.
 */
#define am_interval_array_for_each_overlapping_uint(parr, pi, puint,	\
						    interval_field,	\
						    uint_field,	\
						    pquery)		\
	am_interval_array_for_each_overlapping_uint_offs(		\
		parr,							\
		pi,							\
		puint,							\
		sizeof((parr)->elements[0]),				\
		AM_OFFSETOF_PTR((parr)->elements[0], interval_field),	\
		AM_OFFSETOF_PTR(typeof((parr)->elements[0], uint_field),\
		AM_SIZEOF_BITS((parr)->elements[0].uint_field),	\
		AM_SIZEOF_BITS(*puint),				\
		pquery)

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
#define AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING(prefix, T, member) \
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
		res = am_interval_array_bsearch_first_strided_overlapping(base, \
									a->num_elements, \
									stride, \
									query); \
										\
		return (res) ? (((void*)res)-offsetof(T, member)) : NULL;	\
	}

/* Same as AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING, but defines a
 * function that retrieves the last element that overlaps with an interval. */
#define AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_LAST_OVERLAPPING(prefix, T, member) \
	static inline T*							\
	prefix##_bsearch_last_overlapping(struct prefix* a,			\
					  const struct am_interval* query)	\
	{									\
		off_t offs = offsetof(T, member);				\
		off_t stride = sizeof(T);					\
		struct am_interval* base = (struct am_interval*)		\
			(((void*)a->elements)+offs);				\
		struct am_interval* res;					\
										\
		res = am_interval_array_bsearch_last_strided_overlapping(base, \
									 a->num_elements, \
									 stride,\
									 query);\
										\
		return (res) ? (((void*)res)-offsetof(T, member)) : NULL;	\
	}

#endif
