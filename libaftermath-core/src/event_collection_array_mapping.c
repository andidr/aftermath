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

#include "event_collection_array_mapping.h"

#include <aftermath/core/array_registry.h>
#include <aftermath/core/array_collection.h>
#include <aftermath/core/event_collection.h>

AM_DECL_TYPED_ARRAY_DEFAULT_DESTRUCTOR(
	am_event_collection_array_mapping,
	am_event_collection_array_mapping_default_destroy)

/* Destroys a single mapping entry, including the arrays of the array collection
 * of the entry. Array destructors are invoked through the array registry r. */
static inline void am_event_collection_array_mapping_entry_destroy(
	struct am_event_collection_array_mapping_entry* me,
	struct am_array_registry* r)
{
	am_array_collection_destroy(&me->arrays, r);
}

/* Destroys an entire mapping, including all of its mapping entries and the
 * arrays associated to the entries. Array destructors are invoked through the
 * array registry r. */
void am_event_collection_array_mapping_destroy(
	struct am_event_collection_array_mapping* m,
	struct am_array_registry* r)
{
	for(size_t i = 0; i < m->num_elements; i++)
		am_event_collection_array_mapping_entry_destroy(&m->elements[i], r);

	am_event_collection_array_mapping_default_destroy(m);
}

/* Finds the array collection associated to the event collection whose ID is
 * identical to ecoll->id. Returns NULL if no array collection is currently
 * associated with the event collection. */
struct am_array_collection* am_event_collection_array_mapping_find(
	struct am_event_collection_array_mapping* m,
	struct am_event_collection* ecoll)
{
	struct am_event_collection_array_mapping_entry* e;

	if(!(e = am_event_collection_array_mapping_bsearch(m, ecoll->id)))
		return NULL;

	return &e->arrays;
}

/* Finds the array collection associated to the event collection whose ID is
 * identical to ecoll->id. If no array collection is currently associated with
 * the event collection, a new, empty array collection is created. If the
 * creation fails, the function returns NULL.
 */
struct am_array_collection* am_event_collection_array_mapping_find_or_add(
	struct am_event_collection_array_mapping* m,
	struct am_event_collection* ecoll)
{
	struct am_array_collection* ret;
	struct am_event_collection_array_mapping_entry* e;

	if((ret = am_event_collection_array_mapping_find(m, ecoll)))
		return ret;

	if(!(e = am_event_collection_array_mapping_reserve_sorted(m, ecoll->id)))
		return NULL;

	e->collection_id = ecoll->id;
	am_array_collection_init(&e->arrays);

	return &e->arrays;
}
