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

#include "array_collection.h"

int am_array_collection_entry_init(struct am_array_collection_entry* ace,
				   const char* type,
				   void* array)
{
	if(!(ace->type = strdup(type)))
		return 1;

	ace->array = array;

	return 0;
}

/* Destroys an array collection entry without destroying the array */
void am_array_collection_entry_destroy_preserve_array(
	struct am_array_collection_entry* ace)
{
	free(ace->type);
}

void am_array_collection_entry_destroy(struct am_array_collection_entry* ace,
				       struct am_array_registry* r)
{
	am_array_registry_destroy_and_free_array(
		r, ace->type, NULL, ace->array);

	am_array_collection_entry_destroy_preserve_array(ace);
}

AM_DECL_TYPED_ARRAY_DEFAULT_DESTRUCTOR(am_array_collection,
				       am_array_collection_default_destroy)

void am_array_collection_destroy(struct am_array_collection* c,
				 struct am_array_registry* r)
{
	for(size_t i = 0; i < c->num_elements; i++)
		am_array_collection_entry_destroy(&c->elements[i], r);

	am_array_collection_default_destroy(c);
}

/* Finds an array with the given type. If no such type exists, a new array is
 * created and added to the collection. Returns a pointer to the array on
 * success, otherwise NULL. */
void* am_array_collection_find_or_add(struct am_array_collection* ac,
				      struct am_array_registry* r,
				      const char* type)
{
	void* ret;

	if((ret = am_array_collection_find(ac, type)))
		return ret;

	if((ret = am_array_registry_allocate_and_init_array(r, type, NULL))) {
		if(am_array_collection_add(ac, ret, type)) {
			am_array_registry_destroy_and_free_array(
				r, type, NULL, ret);
		}

		return ret;
	}

	return NULL;
}
