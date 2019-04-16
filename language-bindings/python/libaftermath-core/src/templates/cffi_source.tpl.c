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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <aftermath/core/in_memory.h>
#include <aftermath/core/on_disk.h>
#include <aftermath/core/typed_array.h>

/* Arbitrary limit for the number of frame types */
#define AM_MAX_FRAME_TYPES 128

/* Loads a trace from disk. Returns the trace or NULL on error. */
struct am_trace* am_py_trace_load(const char* filename)
{
	struct am_trace* trace = NULL;
	struct am_io_context ioctx;
	struct am_frame_type_registry frame_types;

	if(am_frame_type_registry_init(&frame_types, AM_MAX_FRAME_TYPES))
		goto out_err;

	if(am_dsk_register_frame_types(&frame_types))
		goto out_err_tr;

	if(am_io_context_init(&ioctx, &frame_types))
		goto out_err_tr;

	if(am_io_context_open(&ioctx, filename, AM_IO_READ))
		goto out_err_ctx;

	if(am_dsk_load_trace(&ioctx, &trace))
		goto out_err_ctx;

out_err_ctx:
	am_io_context_destroy(&ioctx);
out_err_tr:
	am_frame_type_registry_destroy(&frame_types);
out_err:
	return trace;
}

void am_py_trace_destroy_and_free(struct am_trace* t)
{
	puts("trace destroyed");

	if(!t)
		return;

	am_trace_destroy(t);
	free(t);
}

size_t am_py_generic_array_get_num_elements(struct am_typed_array_generic* a)
{
	return a->num_elements;
}

struct am_typed_array_generic*
am_py_trace_find_trace_array(struct am_trace* t, const char* type)
{
	return (struct am_typed_array_generic*)
		am_trace_find_trace_array(t, type);
}

{% for (array_name, element_type) in array_element_types.items() %}
{{element_type}}* am_py_{{array_name}}_get_element(struct am_typed_array_generic* a, size_t i)
{
	return &(({{element_type}}*)a->elements)[i];
}
{% endfor %}

struct am_typed_array_generic*
am_py_event_collection_find_event_array(struct am_event_collection* ecoll,
					const char* ident)
{
	return am_event_collection_find_event_array(ecoll, ident);
}

struct am_typed_array_generic*
am_py_trace_get_event_collections(struct am_trace* t)
{
	return (struct am_typed_array_generic*)&t->event_collections;
}

size_t am_py_trace_get_num_event_collections(struct am_trace* t)
{
	return t->event_collections.num_elements;
}

struct am_event_collection*
am_py_event_collection_array_get_element(struct am_typed_array_generic* a,
					 size_t i)
{
	struct am_event_collection_array* ecoll_array;

	ecoll_array = (struct am_event_collection_array*)a;

	return &ecoll_array->elements[i];
}
