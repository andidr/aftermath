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

#ifndef AM_TRACE_H
#define AM_TRACE_H

#include <aftermath/core/hierarchy_array.h>
#include <aftermath/core/event_collection.h>
#include <aftermath/core/event_collection_array.h>
#include <aftermath/core/array_registry.h>

struct am_trace {
	char* filename;

	/* Minimum and maximum timestamps encountered in the trace */
	struct am_interval bounds;

	struct am_hierarchyp_array hierarchies;
	struct am_event_collection_array event_collections;

	struct am_array_registry array_registry;
	struct am_array_collection trace_arrays;
};

#define am_trace_for_each_event_collection(t, coll) \
	for(coll = &t->event_collections.elements[0]; \
	    coll != &t->event_collections.elements[t->event_collections.num_elements]; \
	    coll++)

int am_trace_init(struct am_trace* t, const char* filename);
void am_trace_destroy(struct am_trace* t);

/* Finds a per-trace array by type. Returns a pointer to the array or NULL if no
 * such array is associated with the trace. */
static inline void* am_trace_find_trace_array(struct am_trace* t,
					      const char* type)
{
	return am_array_collection_find(&t->trace_arrays, type);
}

void* am_trace_find_or_add_trace_array(struct am_trace* t, const char* type);

/* Iterates over all elements of the per-trace array identified by ident. At
 * each iteration, the address of the current element is assigned to iter.
 */
#define am_trace_for_each_trace_array_element(t, ident, iter)			\
	for(struct {								\
		size_t idx;							\
		struct am_typed_array_generic* arr;				\
		typeof(iter) dummy;						\
	} __it = {								\
		.idx = 0,							\
		.arr = (struct am_typed_array_generic*)			\
			  am_trace_find_trace_array(t, (ident)),		\
		.dummy = iter = ((__it.arr && __it.idx < __it.arr->num_elements) ? \
			&((typeof(iter))__it.arr->elements)[__it.idx] :	\
			NULL)							\
	  };									\
	    __it.arr && __it.idx < __it.arr->num_elements;			\
	    __it.idx++, (iter) = &((typeof(iter))__it.arr->elements)[__it.idx])

/* Helper data structure for iterating over per-event-collection arrays. */
struct am_event_collection_array_iter {
	struct am_event_collection* ecoll;
	struct am_typed_array_generic* arr;
};

/* Returns an iterator for the next per-event-collection array after the one
 * identified by prev. If prev is NULL, an iterator to the first array is
 * returned. If no further array exists, the values of the iterator are set to
 * NULL.
 *
 * If ecoll is non-NULL, the value of the event collection of the iterator is
 * assigned to *ecoll.
 *
 * If arr is non-NULL, the value of the array pointer of the iterator is
 * assigned to *arr as a generic typed array.
 */
static inline struct am_event_collection_array_iter
am_event_collection_array_iter_next_assign(
	struct am_trace* t,
	const char* ident,
	struct am_event_collection_array_iter* prev,
	struct am_event_collection** ecoll,
	struct am_typed_array_generic** arr)
{
	struct am_event_collection_array_iter ret;

	for(ret.ecoll = (prev) ? (prev->ecoll+1) : t->event_collections.elements;
	    am_event_collection_array_is_element_ptr(&t->event_collections,
						     ret.ecoll);
	    ret.ecoll++)
	{
		ret.arr = (struct am_typed_array_generic*)
			am_event_collection_find_event_array(ret.ecoll, ident);

		if(ret.arr)
			return ret;
	}

	ret.ecoll = NULL;
	ret.arr = NULL;

	if(ecoll)
		*ecoll = ret.ecoll;

	if(arr)
		*arr = ret.arr;

	return ret;
}

/* Returns an iterator for the next per-event-collection array after the one
 * identified by prev. If prev is NULL, an iterator to the first array is
 * returned. If no further array exists, the values of the iterator are set to
 * NULL.
 */
static inline struct am_event_collection_array_iter
am_event_collection_array_iter_next(
	struct am_trace* t,
	const char* ident,
	struct am_event_collection_array_iter* prev)
{
	return am_event_collection_array_iter_next_assign(
		t, ident, prev, NULL, NULL);
}

/* Returns an iterator to the first per-event-collection array identified by
 * ident. If no such array exists, the values of the iterator are set to
 * NULL.
 *
 * If ecoll is non-NULL, the value of the event collection of the iterator is
 * assigned to *ecoll.
 *
 * If arr is non-NULL, the value of the array pointer of the iterator is
 * assigned to *arr as a generic typed array.
 */
static inline struct am_event_collection_array_iter
am_event_collection_array_iter_first_assign(
	struct am_trace* t,
	const char* ident,
	struct am_event_collection** ecoll,
	struct am_typed_array_generic** arr)
{
	return am_event_collection_array_iter_next_assign(
		t, ident, NULL, ecoll, arr);
}

/* Returns an iterator to the first per-event-collection array identified by
 * ident. If no such array exists, the values of the iterator are set to
 * NULL.
 */
static inline struct am_event_collection_array_iter
am_event_collection_array_iter_first(struct am_trace* t, const char* ident)
{
	return am_event_collection_array_iter_first_assign(t, ident, NULL, NULL);
}

/* Returns true if iter is a valid event collection array iterator that may be
 * dereferenced, otherwise false.
 */
static inline int am_event_collection_array_iter_valid(
	struct am_event_collection_array_iter* iter)
{
	return iter->ecoll && iter->arr;
}

/* Iterates over all event collection arrays whose array identifier is ident. At
 * each iteration, iter is assigned a pointer to the array.
 */
#define am_trace_for_each_event_collection_array(t, ident, iter)	\
	for(struct am_event_collection_array_iter __it =		\
		    am_event_collection_array_iter_first_assign(	\
			    t, ident, NULL,				\
			    (struct am_typed_array_generic**)&iter);	\
	    am_event_collection_array_iter_valid(&__it);		\
	    __it = am_event_collection_array_iter_next_assign(		\
		    t, ident, &__it, NULL,				\
		    (struct am_typed_array_generic**)&iter))

/* Iterates over all event collection arrays whose array identifier is ident
 * using a struct am_event_collection_array_iter.
 */
#define am_trace_iter_each_event_collection_array(t, ident, iter)	\
	for((iter) = am_event_collection_array_iter_first(t, ident);	\
	    am_event_collection_array_iter_valid(&(iter));		\
	    (iter) = am_event_collection_array_iter_next(t, ident, &(iter)))

/* Helper data structure for iterating over per-event-collection sub-arrays. */
struct am_event_collection_subarray_iter {
	struct am_event_collection* ecoll;
	struct am_typed_array_generic* arr;
	struct am_typed_array_generic* subarr;
};

/* Returns an iterator for the next per-event-collection sub-array after the one
 * identified by prev. If prev is NULL, an iterator to the first sub-array is
 * returned. If no further sub-array exists, the values of the iterator are set
 * to NULL.
 *
 * If ecoll is non-NULL, the value of the event collection of the iterator is
 * assigned to *ecoll.
 *
 * If arr is non-NULL, the value of the array pointer of the iterator is
 * assigned to *arr as a generic typed array.
 *
 * If subarr is non-NULL, the value of the sub-array pointer of the iterator is
 * assigned to *subarr as a generic typed array.
 */
static inline struct am_event_collection_subarray_iter
am_event_collection_subarray_iter_next_assign(
	struct am_trace* t,
	const char* ident,
	size_t subarr_size,
	struct am_event_collection_subarray_iter* prev,
	struct am_event_collection** ecoll,
	struct am_typed_array_generic** arr,
	struct am_typed_array_generic** subarr)
{
	struct am_event_collection_subarray_iter ret;

	if(!prev) {
		ret.ecoll = NULL;
		ret.arr = NULL;
		ret.subarr = NULL;
	} else {
		ret = *prev;
	}

	for(ret.ecoll = (!ret.ecoll) ? t->event_collections.elements : ret.ecoll;
	    am_event_collection_array_is_element_ptr(&t->event_collections,
						     ret.ecoll);
	    ret.ecoll++, ret.arr = NULL, ret.subarr = NULL)
	{
		/* Find the per-event-collection array; If we resume a previous
		 * iterator, just skip to the subarr portion
		 */
		if(!ret.arr) {
			ret.arr = (struct am_typed_array_generic*)
				am_event_collection_find_event_array(ret.ecoll,
								     ident);

			/* If this event collection does not have a
			 * per-event-collection array of the specified type,
			 * move on to the next collection */
			if(!ret.arr)
				continue;
		}

		ret.subarr = (struct am_typed_array_generic*)
			((!ret.subarr) ?
			 ret.arr->elements :
			 AM_PTR_ADD(ret.subarr, subarr_size));

		if(AM_PTR_LESS(ret.subarr,
			       AM_PTR_ADD(ret.arr->elements,
					  ret.arr->num_elements * subarr_size)))
		{
			return ret;
		}
	}

	ret.ecoll = NULL;
	ret.arr = NULL;
	ret.subarr = NULL;

	if(ecoll)
		*ecoll = ret.ecoll;

	if(arr)
		*arr = ret.arr;

	if(subarr)
		*subarr = ret.subarr;

	return ret;
}

/* Returns an iterator for the next per-event-collection sub-array after the one
 * identified by prev. If prev is NULL, an iterator to the first sub-array is
 * returned. If no further sub-array exists, the values of the iterator are set
 * to NULL.
 */
static inline struct am_event_collection_subarray_iter
am_event_collection_subarray_iter_next(
	struct am_trace* t,
	const char* ident,
	size_t subarr_size,
	struct am_event_collection_subarray_iter* prev)
{
	return am_event_collection_subarray_iter_next_assign(
		t, ident, subarr_size, prev, NULL, NULL, NULL);
}

/* Returns an iterator to the first per-event-collection sub-array identified by
 * ident. If no such sub-array exists, the values of the iterator are set to
 * NULL.
 *
 * If ecoll is non-NULL, the value of the event collection of the iterator is
 * assigned to *ecoll.
 *
 * If arr is non-NULL, the value of the array pointer of the iterator is
 * assigned to *arr as a generic typed array.
 *
 * If subarr is non-NULL, the value of the sub-array pointer of the iterator is
 * assigned to *subarr as a generic typed array.
 */
static inline struct am_event_collection_subarray_iter
am_event_collection_subarray_iter_first_assign(
	struct am_trace* t,
	const char* ident,
	size_t subarr_size,
	struct am_event_collection** ecoll,
	struct am_typed_array_generic** arr,
	struct am_typed_array_generic** subarr)
{
	return am_event_collection_subarray_iter_next_assign(
		t, ident, subarr_size, NULL, ecoll, arr, subarr);
}

/* Returns an iterator to the first per-event-collection sub-array identified by
 * ident. If no such sub-array exists, the values of the iterator are set to
 * NULL.
 */
static inline struct am_event_collection_subarray_iter
am_event_collection_subarray_iter_first(struct am_trace* t,
					const char* ident,
					size_t subarr_size)
{
	return am_event_collection_subarray_iter_first_assign(
		t, ident, subarr_size, NULL, NULL, NULL);
}

/* Returns true if iter is a valid event collection subarray iterator that may
 * be dereferenced, otherwise false.
 */
static inline int am_event_collection_subarray_iter_valid(
	struct am_event_collection_subarray_iter* iter)
{
	return iter->ecoll && iter->arr && iter->subarr;
}

/* Iterates over all event collection sub-arrays belonging to a
 * per-event-collection array whose identifier is ident. At each iteration, iter
 * is assigned a pointer to the sub-array.
 */
#define am_trace_for_each_event_collection_subarray(t, ident, subarr_type, iter)\
	for(struct am_event_collection_subarray_iter __it =			\
		    am_event_collection_subarray_iter_first_assign(		\
			    t, ident, sizeof(subarr_type), NULL, NULL,		\
			    (struct am_typed_array_generic**)&iter);		\
	    am_event_collection_subarray_iter_valid(&__it);			\
	    __it = am_event_collection_subarray_iter_next_assign(		\
			t, ident, sizeof(subarr_type), &__it, NULL, NULL,	\
				(struct am_typed_array_generic**)&iter))

/* Iterates over all event collection sub-arrays belonging to a
 * per-event-collection array whose identifier is ident using a struct
 * am_event_collection_subarray_iter.
 */
#define am_trace_iter_each_event_collection_subarray(t, ident, iter)	\
	for((iter) = am_event_collection_subarray_iter_first(t, ident); \
	    am_event_collection_subarray_iter_valid(&(iter));		\
	    (iter) = am_event_collection_subarray_iter_next(t, ident, &(iter)))

#endif
