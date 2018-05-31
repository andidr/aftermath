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

#include <aftermath/core/trace.h>
#include <aftermath/core/default_event_array_registry.h>
#include <aftermath/core/default_trace_array_registry.h>

int am_trace_init(struct am_trace* t, const char* filename)
{
	if(!(t->filename = strdup(filename)))
		return 1;

	t->bounds.start = AM_TIMESTAMP_T_MAX;
	t->bounds.end = 0;

	am_event_collection_array_init(&t->event_collections);
	am_array_registry_init(&t->event_array_registry);
	am_array_registry_init(&t->trace_array_registry);
	am_array_collection_init(&t->trace_arrays);
	am_hierarchyp_array_init(&t->hierarchies);

	if(am_build_default_event_array_registry(&t->event_array_registry) ||
	   am_build_default_trace_array_registry(&t->trace_array_registry))
	{
		am_trace_destroy(t);
		return 1;
	}

	return 0;
}

void am_trace_destroy(struct am_trace* t)
{
	am_array_collection_destroy(&t->trace_arrays, &t->trace_array_registry);
	am_array_registry_destroy(&t->event_array_registry);
	am_array_registry_destroy(&t->trace_array_registry);

	am_hierarchyp_array_destroy_elements(&t->hierarchies);
	am_hierarchyp_array_destroy(&t->hierarchies);

	free(t->filename);

	am_event_collection_array_destroy_elements(&t->event_collections,
						   &t->event_array_registry);
	am_event_collection_array_destroy(&t->event_collections);
}

/* Finds a per-trace array by type and returns a pointer to the array. If no no
 * such array is associated with the trace, the function tries to allocate and
 * initialize an array of the specified type using the trace array registry of
 * the trace. If the allocation or initialization fails, NULL is returned,
 * otherwise a pointer to the newly created array. */
void* am_trace_find_or_add_trace_array(struct am_trace* t, const char* type)
{
	void* a;

	if((a = am_trace_find_trace_array(t, type)))
		return a;

	if(!(a = am_array_registry_allocate_and_init_array(
		     &t->trace_array_registry, type, NULL)))
	{
		return NULL;
	}

	if(am_array_collection_add(&t->trace_arrays, a, type)) {
		am_array_registry_destroy_and_free_array(
			&t->trace_array_registry,
			type,
			NULL,
			a);

		return NULL;
	}

	return a;
}
