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

#ifndef AM_ARRAY_COLLECTION_H
#define AM_ARRAY_COLLECTION_H

#include <aftermath/core/typed_array.h>
#include <aftermath/core/array_registry.h>

/* A single entry of an array collection, associating a type with the address of
 * the array. */
struct am_array_collection_entry {
	char* type;
	struct am_typed_array_generic* array;
};

#define AM_ACC_ARRAY_COLLECTION_TYPE(x) (x).type


int am_array_collection_entry_init(struct am_array_collection_entry* ace,
				   const char* type,
				   void* array);

void am_array_collection_entry_destroy_preserve_array(
	struct am_array_collection_entry* ace);

void am_array_collection_entry_destroy(struct am_array_collection_entry* ace,
				       struct am_array_registry* r);

AM_DECL_TYPED_ARRAY_NO_DESTRUCTOR(am_array_collection,
				  struct am_array_collection_entry)

void am_array_collection_destroy(struct am_array_collection* c,
				 struct am_array_registry* r);

AM_DECL_TYPED_ARRAY_BSEARCH(am_array_collection,
			    struct am_array_collection_entry,
			    char*,
			    AM_ACC_ARRAY_COLLECTION_TYPE,
			    strcmp)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_array_collection,
			      struct am_array_collection_entry,
			      char*,
			      AM_ACC_ARRAY_COLLECTION_TYPE,
			      strcmp)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_array_collection,
				   struct am_array_collection_entry,
				   char*)

/* Finds an array with the given type. Returns a pointer to the array on
 * success, otherwise NULL. */
static inline void* am_array_collection_find(struct am_array_collection* ac,
					     const char* type)
{
	struct am_array_collection_entry* ace;

	if(!(ace = am_array_collection_bsearch(ac, type)))
		return NULL;

	return ace->array;
}

/* Removes and destroys the array of type 'type'. If no such array exists,
 * nothing happens.
 */
static inline void am_array_collection_destroy_array(
	struct am_array_collection* ac,
	struct am_array_registry* r,
	const char* type)
{
	struct am_array_collection_entry* ace;

	if(!(ace = am_array_collection_bsearch(ac, type)))
		return;

	am_array_collection_entry_destroy(ace, r);
	am_array_collection_removep(ac, ace);
}

/* Adds a new entry to the array collection with the given array and
 * type. Returns 0 on success, otherwise 1.  */
static inline int am_array_collection_add(struct am_array_collection* ac,
					  struct am_typed_array_generic* array,
					  const char* type)
{
	char* typedup;
	struct am_array_collection_entry* ace;

	if(!(typedup = strdup(type)))
		return 1;

	if(!(ace = am_array_collection_reserve_sorted(ac, type))) {
		free(typedup);
		return 1;
	}

	ace->type = typedup;
	ace->array = array;

	return 0;
}

void* am_array_collection_find_or_add(struct am_array_collection* ac,
				      struct am_array_registry* r,
				      const char* type);

#endif
