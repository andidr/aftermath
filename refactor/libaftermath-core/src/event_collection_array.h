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

#ifndef AM_EVENT_COLLECTION_ARRAY_H
#define AM_EVENT_COLLECTION_ARRAY_H

#include <aftermath/core/typed_array.h>
#include <aftermath/core/event_collection.h>

#define ACC_ID(x) (x).id

AM_DECL_TYPED_ARRAY(am_event_collection_array, struct am_event_collection)
AM_DECL_TYPED_ARRAY_BSEARCH(am_event_collection_array,
			 struct am_event_collection,
			 am_event_collection_id_t,
			 ACC_ID)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_event_collection_array,
			   struct am_event_collection,
			   am_event_collection_id_t,
			   ACC_ID)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_event_collection_array,
				struct am_event_collection,
				am_event_collection_id_t)

/* Retrieve an event collection by ID. Returns NULL if no such collection
 * exists. */
static inline struct am_event_collection*
am_event_collection_array_find(struct am_event_collection_array* a,
			       am_event_collection_id_t id)
{
	return am_event_collection_array_bsearch(a, id);
}

/* Adds an event collection to an event collection array. The name is copied to
 * a newly allocated buffer and can be freed after the call. Returns a pointer
 * to the newly allocated event collection or NULL on failure. */
static inline struct am_event_collection*
am_event_collection_array_add(struct am_event_collection_array* a,
			      am_event_collection_id_t id,
			      const char* name)
{
	struct am_event_collection* c;

	if(!(c = am_event_collection_array_reserve_sorted(a, id)))
		return NULL;

	if(am_event_collection_init(c, id, name)) {
		am_event_collection_array_removep(a, c);
		return NULL;
	}

	return c;
}

/* Adds an event collection to an event collection array. Ownership of the name
 * is transferred to the new event collection instance, i.e., it is not copied
 * to a newly allocated buffer and must not be freed after the call. Returns a
 * pointer to the newly allocated event collection or NULL on failure. */
static inline struct am_event_collection*
am_event_collection_array_add_nodup(struct am_event_collection_array* a,
				    am_event_collection_id_t id,
				    char* name)
{
	struct am_event_collection* c;

	if(!(c = am_event_collection_array_reserve_sorted(a, id)))
		return NULL;

	am_event_collection_init_nodup(c, id, name);

	return c;
}

/* Destroys all event collections of an event collection array, but frees
 * neither the space occupied by the elements, nor the array itself. */
static inline void
am_event_collection_array_destroy_elements(struct am_event_collection_array* a,
					   struct am_event_array_registry* r)
{
	for(size_t i = 0; i < a->num_elements; i++)
		am_event_collection_destroy(&a->elements[i], r);
}

#endif
