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

#include "event_collection.h"

void am_event_array_type_entry_destroy(struct am_event_array_type_entry* e,
				       struct am_event_array_registry* r)
{
	am_event_registry_destroy_and_free_array(r, e->type, NULL, e->array);
}

/* Destroys all the elements of an event array collection without freeing the
 * event array collection itself or the space occupied by the elements. */
void am_event_array_collection_destroy_elements(struct am_event_array_collection* a,
						struct am_event_array_registry* r)
{
	for(size_t i = 0; i < a->num_elements; i++)
		am_event_array_type_entry_destroy(&a->elements[i], r);
}

/* Short-hand for conditionally setting an integer */
static inline void set_int_if_not_null(int* i, int val)
{
	if(i)
		*i = val;
}

/* Retrieve an entry from an event array registry by the event array's type. If
 * no such entry exists the function returns NULL.  */
struct am_event_array_registry_entry*
am_event_array_registry_find(struct am_event_array_registry* r,
			     enum am_event_array_type type)
{
	return am_event_array_registry_bsearch(r, type);
}

/* Returns a new, uninitialized instance of an event array. If the specified
 * type is unknown to the registry, NULL is returned and *type_found is set to
 * 0. If the type is found, but the allocation fails, NULL is returned and
 * *type_found is set to 1. Upon success, the new instance is returned and
 * *type_found is set to 1. */
void* am_event_registry_allocate_array(struct am_event_array_registry* r,
				       enum am_event_array_type type,
				       int* type_found)
{
	struct am_event_array_registry_entry* e;

	if(!(e = am_event_array_registry_find(r, type))) {
		set_int_if_not_null(type_found, 0);
		return NULL;
	}

	set_int_if_not_null(type_found, 1);

	return e->allocate();
}

/* Frees an instance of an event array. If the specified type is unknown to the
 * registry, *type_found is set to 0. */
void am_event_registry_free_array(struct am_event_array_registry* r,
				  enum am_event_array_type type,
				  int* type_found,
				  void* a)
{
	struct am_event_array_registry_entry* e;

	if(!(e = am_event_array_registry_find(r, type))) {
		set_int_if_not_null(type_found, 0);
		return;
	}

	set_int_if_not_null(type_found, 1);

	e->free(a);
}

/* Initializes an instance of an event array. If the specified type is unknown
 * to the registry, 1 is returned and *type_found is set to 0. If the type is
 * found, but the initialization fails, 1 is returned and *type_found is set to
 * 1. Upon success, 0 is returned and *type_found is set to 1. */
int am_event_registry_init_array(struct am_event_array_registry* r,
				 enum am_event_array_type type,
				 int* type_found,
				 void* a)
{
	struct am_event_array_registry_entry* e;

	if(!(e = am_event_array_registry_find(r, type))) {
		set_int_if_not_null(type_found, 0);
		return 1;
	}

	set_int_if_not_null(type_found, 1);

	return e->init(a);
}

/* Allocates and initializes an instance of an event array. If the specified
 * type is unknown to the registry, NULL is returned and *type_found is set to
 * 0. If the type is found, but the allocation or initialization fails, NULL is
 * returned and *type_found is set to 1. Upon success, the new instance is
 * returned and *type_found is set to 1. */
void* am_event_registry_allocate_and_init_array(struct am_event_array_registry* r,
						enum am_event_array_type type,
						int* type_found)
{
	void* a;

	if(!(a = am_event_registry_allocate_array(r, type, type_found)))
		return NULL;

	if(am_event_registry_init_array(r, type, type_found, a)) {
		am_event_registry_free_array(r, type, type_found, a);
		return NULL;
	}

	return a;
}

/* Destroys an instance of an event array. If the specified type is unknown to
 * the registry, *type_found is set to 0. */
void am_event_registry_destroy_array(struct am_event_array_registry* r,
				     enum am_event_array_type type,
				     int* type_found,
				     void* a)
{
	struct am_event_array_registry_entry* e;

	if(!(e = am_event_array_registry_find(r, type))) {
		set_int_if_not_null(type_found, 0);
		return;
	}

	set_int_if_not_null(type_found, 1);

	e->destroy(a);
}

/* Destroys and frees an instance of an event array. If the specified type is
 * unknown to the registry, *type_found is set to 0. */
void am_event_registry_destroy_and_free_array(struct am_event_array_registry* r,
					      enum am_event_array_type type,
					      int* type_found,
					      void* a)
{
	am_event_registry_destroy_array(r, type, type_found, a);

	if(type_found && !*type_found)
		return;

	am_event_registry_free_array(r, type, type_found, a);
}

/* Add a new type to the registry. Returns 0 on success, otherwise 1. */
int am_event_array_registry_add(struct am_event_array_registry* r,
				enum am_event_array_type type,
				void* (*allocate)(void),
				void (*free)(void* a),
				int (*init)(void* a),
				void (*destroy)(void* a))
{
	struct am_event_array_registry_entry* e;

	if(am_event_array_registry_find(r, type))
		return 1;

	if(!(e = am_event_array_registry_reserve_sorted(r, type)))
		return 1;

	e->type = type;
	e->allocate = allocate;
	e->free = free;
	e->init = init;
	e->destroy = destroy;

	return 0;
}

/* Initializes an event collection and reuses the provided string as the name,
 * i.e., without copying the name to a new buffer. */
void am_event_collection_init_nodup(struct am_event_collection* e,
				    am_event_collection_id_t id,
				    char* name)
{
	am_event_array_collection_init(&e->event_arrays);
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

/* Destroys and event collection. */
void am_event_collection_destroy(struct am_event_collection* e,
				 struct am_event_array_registry* r)
{
	am_event_array_collection_destroy_elements(&e->event_arrays, r);
	am_event_array_collection_destroy(&e->event_arrays);
	free(e->name);
}

/* Finds the event array of type t in an event collection e. If no such array
   exists, the function returns NULL. */
void* am_event_collection_find_event_array(struct am_event_collection* e,
					   enum am_event_array_type t)
{
	struct am_event_array_type_entry* ead;

	if(!(ead = am_event_array_collection_bsearch(&e->event_arrays, t)))
		return NULL;

	return ead->array;
}

/* Retrieves an event array from an event collection by type. If the collection
 * does not have a corresonding entry, a new array is allocated, initialized and
 * added to the event collection. If allocation or initialization fails, the
 * function returns NULL. */
void* am_event_collection_find_or_add_event_array(struct am_event_array_registry* r,
						  struct am_event_collection* e,
						  enum am_event_array_type t)
{
	void* ret;

	if((ret = am_event_collection_find_event_array(e, t)))
		return ret;

	if(!(ret = am_event_registry_allocate_and_init_array(r, t, NULL)))
		return NULL;

	if(am_event_collection_add_event_array(e, t, ret)) {
		am_event_registry_destroy_and_free_array(r, t, NULL, ret);
		return NULL;
	}

	return ret;
}

/* Add an event array of type t to an event collection. If there is already an
 * array of that type or if the array cannot be added, the function returns
 * 1. On success, the return value is 0. */
int am_event_collection_add_event_array(struct am_event_collection* e,
					enum am_event_array_type t,
					void* array)
{
	size_t pos;
	struct am_event_array_type_entry eate = {
		.type = t,
		.array = array
	};

	if(am_event_collection_find_event_array(e, t))
		return 1;

	pos = am_event_array_collection_insertpos(&e->event_arrays, t);

	if(am_event_array_collection_insert(&e->event_arrays, pos, eate))
		return 1;

	return 0;
}
