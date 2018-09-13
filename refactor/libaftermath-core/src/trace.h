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
#endif
