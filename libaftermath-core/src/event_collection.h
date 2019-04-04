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

#ifndef AM_EVENT_COLLECTION_H
#define AM_EVENT_COLLECTION_H

/* An event collection groups all events of a logical execution stream (e.g., a
 * thread or a worker). Events are organized in an event array collection, which
 * represents an array of event arrays. The arrays in the array collection are
 * grouped by their type expressed as a numerical ID from enum event_array_type.
 *
 * Allocation, initialization, destruction and liberation of the arrays is
 * handled by an event array registry, which contains instances of
 * event_array_registry_entry, associating the corresponding functions with an
 * event array type.
 */

#include <aftermath/core/array_collection.h>
#include <aftermath/core/array_registry.h>
#include <aftermath/core/base_types.h>

/* Single event collection, respresenting a logical execution stream */
struct am_event_collection {
	struct am_array_collection event_arrays;
	am_event_collection_id_t id;
	char* name;
};

int am_event_collection_init(struct am_event_collection* e,
			     am_event_collection_id_t id,
			     const char* name);
void am_event_collection_init_nodup(struct am_event_collection* e,
				    am_event_collection_id_t id,
				    char* name);
void am_event_collection_destroy(struct am_event_collection* e,
				 struct am_array_registry* r);

void* am_event_collection_find_event_array(struct am_event_collection* e,
					   const char* t);

void* am_event_collection_find_or_add_event_array(struct am_array_registry* r,
						  struct am_event_collection* e,
						  const char* t);

int am_event_collection_add_event_array(struct am_event_collection* e,
					const char* t,
					void* array);

#endif
