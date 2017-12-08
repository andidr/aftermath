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

#ifndef COUNTER_EVENT_ARRAY_COLLECTION_H
#define COUNTER_EVENT_ARRAY_COLLECTION_H

#include <aftermath/core/typed_array.h>
#include <aftermath/core/counter_event_array.h>
#include <aftermath/core/in_memory.h>

#define ACC_COUNTER_ID(x) (x).counter_id

AM_DECL_TYPED_ARRAY(am_counter_event_array_collection,
		    struct am_counter_event_array)
AM_DECL_TYPED_ARRAY_BSEARCH(am_counter_event_array_collection,
			 struct am_counter_event_array,
			 am_counter_t,
			 ACC_COUNTER_ID)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_counter_event_array_collection,
			   struct am_counter_event_array,
			   am_counter_t,
			   ACC_COUNTER_ID)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_counter_event_array_collection,
				struct am_counter_event_array,
				am_counter_t)

/* Destroys all of the elements of the array, but neither frees the array
 * structure itself, nor the space occupied by the array elements. */
static inline void
am_counter_event_array_collection_destroy_elements(struct am_counter_event_array_collection* a)
{
	for(size_t i = 0; i < a->num_elements; i++)
		am_counter_event_array_destroy(&a->elements[i]);
}

/* Find a counter event array whose ID is counter_id. If no such counter event
 * array exists, NULL is returned. */
static inline struct am_counter_event_array*
am_counter_event_array_collection_find(struct am_counter_event_array_collection* a,
				       am_counter_t counter_id)
{
	struct am_counter_event_array* cea;

	if(!(cea = am_counter_event_array_collection_bsearch(a, counter_id)))
		return NULL;

	return cea;
}

/* Find a counter event array whose ID is counter_id. If no such counter event
 * array exists, a new entry in the array with the ID is created and the new
 * element is returned. In case of an error, the function returns NULL. */
static inline struct am_counter_event_array*
am_counter_event_array_collection_find_or_add(struct am_counter_event_array_collection* a,
					      am_counter_t counter_id)
{
	struct am_counter_event_array* cea;

	if((cea = am_counter_event_array_collection_find(a, counter_id)))
		return cea;

	if(!(cea = am_counter_event_array_collection_reserve_sorted(a, counter_id)))
		return NULL;

	am_counter_event_array_init(cea);
	cea->counter_id = counter_id;

	return cea;
}

#endif
