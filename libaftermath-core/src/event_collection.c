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

#include <aftermath/core/event_collection.h>

/* Initializes an event collection and reuses the provided string as the name,
 * i.e., without copying the name to a new buffer. */
void am_event_collection_init_nodup(struct am_event_collection* e,
				    am_event_collection_id_t id,
				    char* name)
{
	am_array_collection_init(&e->event_arrays);
	e->id = id;
	e->name = name;
}

/* Initializes an event collection. The name is copied to a new buffer and can
 * be freed safely after the call. */
int am_event_collection_init(struct am_event_collection* e,
			     am_event_collection_id_t id,
			     const char* name)
{
	char* dupname;

	if(!(dupname = strdup(name)))
		return 1;

	am_event_collection_init_nodup(e, id, dupname);

	return 0;
}

/* Destroys an event collection. */
void am_event_collection_destroy(struct am_event_collection* e,
				 struct am_array_registry* r)
{
	am_array_collection_destroy(&e->event_arrays, r);
	free(e->name);
}

/* Finds the event array of type t in an event collection e. If no such array
   exists, the function returns NULL. */
void* am_event_collection_find_event_array(struct am_event_collection* e,
					   const char* t)
{
	struct am_array_collection_entry* ead;

	if(!(ead = am_array_collection_bsearch(&e->event_arrays, t)))
		return NULL;

	return ead->array;
}

/* Retrieves an event array from an event collection by type. If the collection
 * does not have a corresonding entry, a new array is allocated, initialized and
 * added to the event collection. If allocation or initialization fails, the
 * function returns NULL. */
void* am_event_collection_find_or_add_event_array(struct am_array_registry* r,
						  struct am_event_collection* e,
						  const char* t)
{
	void* ret;

	if((ret = am_event_collection_find_event_array(e, t)))
		return ret;

	if(!(ret = am_array_registry_allocate_and_init_array(r, t, NULL)))
		return NULL;

	if(am_event_collection_add_event_array(e, t, ret)) {
		am_array_registry_destroy_and_free_array(r, t, NULL, ret);
		return NULL;
	}

	return ret;
}

/* Add an event array of type t to an event collection. If there is already an
 * array of that type or if the array cannot be added, the function returns
 * 1. On success, the return value is 0. */
int am_event_collection_add_event_array(struct am_event_collection* e,
					const char* t,
					void* array)
{
	size_t pos;
	struct am_array_collection_entry ace;

	if(am_event_collection_find_event_array(e, t))
		goto out_err;

	if(am_array_collection_entry_init(&ace, t, array))
		goto out_err;

	pos = am_array_collection_insertpos(&e->event_arrays, t);

	if(am_array_collection_insertp(&e->event_arrays, pos, &ace))
		goto out_err_free;

	return 0;

out_err_free:
	am_array_collection_entry_destroy_preserve_array(&ace);
out_err:
	return 1;
}
