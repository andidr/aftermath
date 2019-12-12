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

#ifndef AM_TIMESTAMP_ARRAY_H
#define AM_TIMESTAMP_ARRAY_H

#include <aftermath/core/base_types.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/interval.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/ptr.h>
#include <aftermath/core/ansi_extras.h>
#include <aftermath/core/contrib/linux-kernel/stddef.h>

#define ACC_PINTERVAL_PIDENT(i) (&(i))

AM_DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(am_timestamp_array_,
				      _within,
				      am_timestamp_t,
				      struct am_interval*,
				      ACC_PINTERVAL_PIDENT,
				      am_timestamp_within_pcmp)

AM_DECL_VSTRIDED_BSEARCH_LAST_SUFFIX(am_timestamp_array_,
				     _within,
				     am_timestamp_t,
				     struct am_interval*,
				     ACC_PINTERVAL_PIDENT,
				     am_timestamp_within_pcmp)

/* Internal use; Used as an iterator in for_each macros */
struct am_timestamp_array_iterator {
	am_timestamp_t* start;
	am_timestamp_t* end;
};

/* Internal use; Returns a newly intialized timestamp array iterator and sets *t
 * to the first timestamp or NULL if no such timestamp exists.
 */
static inline struct am_timestamp_array_iterator
am_timestamp_array_iterator_start(struct am_typed_array_generic* arr,
				 off_t stride,
				 off_t field_offset,
				 const struct am_interval* query,
				 am_timestamp_t** t)
{
	struct am_timestamp_array_iterator it;
	am_timestamp_t* first_field;

	/* Address of the timestamp field of the first array element */
	first_field = (am_timestamp_t*)AM_PTR_ADD(arr->elements, field_offset);

	/* Address of the timestamp field of the first array element whose
	 * timestamp is within the query interval */
	it.start = am_timestamp_array_bsearch_first_strided_within(
		first_field,
		arr->num_elements,
		stride,
		query);

	*t = it.start;

	if(it.start) {
		/* Address of the timestamp field of the last array element
		 * whose timestamp overlaps with the query interval */
		it.end = am_timestamp_array_bsearch_last_strided_within(
			first_field,
			arr->num_elements,
			stride,
			query);
	}

	return it;
}

/* Internal use; Returns a newly intialized timestamp array iterator, sets *t to
 * the first timestamp or NULL if no such timestamp exists and *uint_val to the
 * value of the unsigned integer of uint_field_bits bits at the offset
 * uint_field_offset. Uint_field_bits must be either 8, 16, 32 or 64. */
static inline struct am_timestamp_array_iterator
am_timestamp_array_iterator_start_intn(struct am_typed_array_generic* arr,
				      off_t stride,
				      off_t interval_field_offset,
				      off_t uint_field_offset,
				      unsigned int uint_field_bits,
				      const struct am_interval* query,
				      am_timestamp_t** t,
				      void* uint_val,
				      unsigned int uint_val_bits)
{
	struct am_timestamp_array_iterator it;
	void* uint_field;

	it = am_timestamp_array_iterator_start(arr, stride, interval_field_offset,
					       query, t);

	if(!it.start)
		return it;

	uint_field = AM_PTR_ADD(it.start,
				uint_field_offset - interval_field_offset);

	am_assign_uint(uint_val, uint_val_bits, uint_field, uint_field_bits);

	return it;
}

/* Iterate over a typed array of elements that contain a timestamp, starting
 * with the timestamp of the first element whose timestamp is within a query
 * interval and ending with the timestamp of the last element whose timestamp is
 * within the query interval. The argument parr must be a pointer to a typed
 * array cast to struct am_typed_array_generic, pt a pointer to an
 * am_timestamp_t that serves as an iterator, element_size is the size in bytes
 * of an array element, field_offset is the offset of the timestamp field of an
 * array element, and pquery is a pointer to the query interval.
 */
#define am_timestamp_array_for_each_within_offs(parr, pt, element_size,		\
						field_offset, pquery)		\
	for(struct am_timestamp_array_iterator __it =				\
		    am_timestamp_array_iterator_start(				\
			    parr,						\
			    element_size,					\
			    field_offset,					\
			    pquery,						\
			    &pt);						\
	    (pt) && AM_PTR_LEQ(pt, __it.end);					\
	    (pt) = AM_PTR_ADD(pt, element_size))

/* Iterate over a typed array of elements that contain a timestamp, starting
 * with the timestamp of the first element whose timestamp is within a query
 * interval and ending with the timestamp of the last element whose timestamp is
 * within the query interval. The argument parr must be a pointer to a typed
 * array cast to struct am_typed_array_generic, pt a pointer to an
 * am_timestamp_t that serves as an iterator, field is the name of the interval
 * field of an array element, and pquery is a pointer to the query interval.
 */
#define am_timestamp_array_for_each_within(parr, pt, field, pquery)	\
	am_timestamp_array_for_each_within_offs(			\
		parr,							\
		pt,							\
		sizeof((parr)->elements[0]),				\
		AM_OFFSETOF_PTR((parr)->elements[0], field),		\
		pquery)

/* Same as am_timestamp_array_for_each_within_offs, but extracts an unsigned
 * integer field at each iteration from the current array element and assigns
 * the value to *puint. Uint_field_offset specifies the offset of the unsigned
 * integer field in an element and uint_field_bits indicates the size of the
 * unsigned integer in bits. The number of bits must be 8, 16, 32 or 64.
 */
#define am_timestamp_array_for_each_within_uint_offs(parr, pt, puint,		\
						     puint_bits,		\
						     element_size,		\
						     timestamp_field_offset,	\
						     uint_field_offset,		\
						     uint_field_bits,		\
						     pquery)			\
	for(struct am_timestamp_array_iterator __it =				\
		    am_timestamp_array_iterator_start_intn(			\
			    parr,						\
			    element_size,					\
			    timestamp_field_offset,				\
			    uint_field_offset,					\
			    uint_field_bits,					\
			    pquery,						\
			    &pt,						\
			    puint,						\
			    puint_bits);					\
	    (pt) && AM_PTR_LEQ(pt, __it.end);					\
	    (pt) = AM_PTR_ADD(pt, element_size),				\
		    am_assign_uint(puint, puint_bits,				\
				   AM_PTR_ADD(pt, uint_field_offset -		\
					      timestamp_field_offset),		\
				   uint_field_bits))

/* Same as am_timestamp_array_for_each_within, but extracts the unsigned integer
 * field specified by interval_field at each iteration from the current array
 * element and assigns the value to *puint.
 */
#define am_timestamp_array_for_each_overlapping_uint(parr, pt, puint,	\
						    timestamp_field,	\
						    uint_field,		\
						    pquery)		\
	am_timestamp_array_for_each_overlapping_uint_offs(		\
		parr,							\
		pt,							\
		puint,							\
		sizeof((parr)->elements[0]),				\
		AM_OFFSETOF_PTR((parr)->elements[0], timestamp_field),	\
		AM_OFFSETOF_PTR(typeof((parr)->elements[0], uint_field),\
		AM_SIZEOF_BITS((parr)->elements[0].uint_field),		\
		AM_SIZEOF_BITS(*puint),					\
		pquery)

/* This macro provides a generic way to implement a binary search function for
 * arrays of an event type T with a timestamp. The declared function returns the
 * first instance of type T whose timestamp is within the query interval. If no
 * such instance exists, the function returns NULL.
 *
 * Example:
 *
 *   struct augmented_timestamp {
 *   	...
 *   	am_timestamp_t t;
 *   	int a;
 *   	int b;
 *   	struct some_other_struct sos;
 *   	...
 *   };
 *
 *   DECL_TYPED_ARRAY(augts, struct augmented_timestamp)
 *
 *   DECL_DISCRETE_EVENT_ARRAY_BSEARCH_FIRST_WITHIN(augts,
 *						    struct augmented_timestamp,
 *						    t)
 *
 * The example declares the function:
 *
 *   static inline struct augmented_timestamp*
 *   augts_array_bsearch_first_overlapping(struct augts_array* a,
 *                                         const struct am_interval* query);
 *
 * that returns the first instance of an augmented_timestamp structure whose
 * timestamp is within the interval query.
 */
#define AM_DECL_DISCRETE_EVENT_ARRAY_BSEARCH_FIRST_WITHIN(prefix, T, member)	\
	static inline T*							\
	prefix##_bsearch_first_within(struct prefix* a,				\
				      const struct am_interval* query)		\
	{									\
		off_t offs = offsetof(T, member);				\
		off_t stride = sizeof(T);					\
		struct am_interval* base =					\
			(struct am_interval*)AM_PTR_ADD(a->elements, offs);	\
		struct am_interval* res;					\
										\
		res = am_timestamp_array_bsearch_first_strided_within(base,	\
								      a->num_elements, \
								      stride,	\
								      query);	\
										\
		return (res) ? (T*)AM_PTR_SUB(res, offsetof(T, member)) : NULL; \
	}

/* Same as AM_DECL_DISCRETE_EVENT_ARRAY_BSEARCH_FIRST_WITHIN, but defines a
 * function that retrieves the last element whose timestamp is within with an
 * interval. */
#define AM_DECL_DISCRETE_EVENT_ARRAY_BSEARCH_LAST_WITHIN(prefix, T, member)	\
	static inline T*							\
	prefix##_bsearch_last_within(struct prefix* a,				\
				     const struct am_interval* query)		\
	{									\
		off_t offs = offsetof(T, member);				\
		off_t stride = sizeof(T);					\
		struct am_interval* base =					\
			(struct am_interval*)AM_PTR_ADD(a->elements, offs);	\
		struct am_interval* res;					\
										\
		res = am_timestamp_array_bsearch_last_strided_within(base,	\
								     a->num_elements, \
								     stride,	\
								     query);	\
										\
		return (res) ? (T*)AM_PTR_SUB(res, offsetof(T, member)) : NULL; \
	}

#endif
