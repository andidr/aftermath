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
#include <aftermath/core/event_collection_array.h>
#include <aftermath/core/state_description_array.h>
#include <aftermath/core/counter_description_array.h>
#include <aftermath/core/event_collection.h>
#include <aftermath/core/measurement_interval_array.h>
#include <aftermath/core/openstream_task_type_array.h>

struct am_trace {
	char* filename;

	/* Minimum and maximum timestamps encountered in the trace */
	struct am_interval bounds;

	struct am_hierarchyp_array hierarchies;
	struct am_event_collection_array event_collections;
	struct am_state_description_array state_descriptions;
	struct am_counter_description_array counter_descriptions;
	struct am_event_array_registry event_array_registry;
	struct am_measurement_interval_array measurement_intervals;
	struct am_openstream_task_type_array openstream_task_types;
};

#define am_trace_for_each_event_collection(t, coll) \
	for(coll = &t->event_collections.elements[0]; \
	    coll != &t->event_collections.elements[t->event_collections.num_elements]; \
	    coll++)

int am_trace_init(struct am_trace* t, const char* filename);
void am_trace_destroy(struct am_trace* t);

#endif
