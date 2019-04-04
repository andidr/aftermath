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

#include "array_registry.h"

/* Short-hand for conditionally setting an integer */
static inline void set_int_if_not_null(int* i, int val)
{
	if(i)
		*i = val;
}

/* Retrieve an entry from an array registry by the array's type. If no such
 * entry exists the function returns NULL.  */
struct am_array_registry_entry*
am_array_registry_find(struct am_array_registry* r, const char* type)
{
	return am_array_registry_bsearch(r, type);
}

/* Returns a new, uninitialized instance of an array. If the specified type is
 * unknown to the registry, NULL is returned and *type_found is set to 0. If the
 * type is found, but the allocation fails, NULL is returned and *type_found is
 * set to 1. Upon success, the new instance is returned and *type_found is set
 * to 1. */
void* am_array_registry_allocate_array(struct am_array_registry* r,
				       const char* type,
				       int* type_found)
{
	struct am_array_registry_entry* e;

	if(!(e = am_array_registry_find(r, type))) {
		set_int_if_not_null(type_found, 0);
		return NULL;
	}

	set_int_if_not_null(type_found, 1);

	return e->allocate();
}

/* Frees an instance of an array. If the specified type is unknown to the
 * registry, *type_found is set to 0. */
void am_array_registry_free_array(struct am_array_registry* r,
				  const char* type,
				  int* type_found,
				  void* a)
{
	struct am_array_registry_entry* e;

	if(!(e = am_array_registry_find(r, type))) {
		set_int_if_not_null(type_found, 0);
		return;
	}

	set_int_if_not_null(type_found, 1);

	e->free(a);
}

/* Initializes an instance of an array. If the specified type is unknown to the
 * registry, 1 is returned and *type_found is set to 0. If the type is found,
 * but the initialization fails, 1 is returned and *type_found is set to 1. Upon
 * success, 0 is returned and *type_found is set to 1. */
int am_array_registry_init_array(struct am_array_registry* r,
				 const char* type,
				 int* type_found,
				 void* a)
{
	struct am_array_registry_entry* e;

	if(!(e = am_array_registry_find(r, type))) {
		set_int_if_not_null(type_found, 0);
		return 1;
	}

	set_int_if_not_null(type_found, 1);

	return e->init(a);
}

/* Allocates and initializes an instance of an array. If the specified type is
 * unknown to the registry, NULL is returned and *type_found is set to 0. If the
 * type is found, but the allocation or initialization fails, NULL is returned
 * and *type_found is set to 1. Upon success, the new instance is returned and
 * *type_found is set to 1. */
void* am_array_registry_allocate_and_init_array(struct am_array_registry* r,
								  const char* type,
								  int* type_found)
{
	void* a;

	if(!(a = am_array_registry_allocate_array(r, type, type_found)))
		return NULL;

	if(am_array_registry_init_array(r, type, type_found, a)) {
		am_array_registry_free_array(r, type, type_found, a);
		return NULL;
	}

	return a;
}

/* Destroys an instance of an array. If the specified type is unknown to the
 * registry, *type_found is set to 0. */
void am_array_registry_destroy_array(struct am_array_registry* r,
				     const char* type,
				     int* type_found,
				     void* a)
{
	struct am_array_registry_entry* e;

	if(!(e = am_array_registry_find(r, type))) {
		set_int_if_not_null(type_found, 0);
		return;
	}

	set_int_if_not_null(type_found, 1);

	e->destroy(a);
}

/* Destroys and frees an instance of an array. If the specified type is unknown
 * to the registry, *type_found is set to 0. */
void am_array_registry_destroy_and_free_array(struct am_array_registry* r,
					      const char* type,
					      int* type_found,
					      void* a)
{
	am_array_registry_destroy_array(r, type, type_found, a);

	if(type_found && !*type_found)
		return;

	am_array_registry_free_array(r, type, type_found, a);
}

/* Add a new type to the registry. Returns 0 on success, otherwise 1. */
int am_array_registry_add(struct am_array_registry* r,
			  const char* type,
			  void* (*allocate)(void),
			  void (*free)(void* a),
			  int (*init)(void* a),
			  void (*destroy)(void* a))
{
	struct am_array_registry_entry* e;

	if(am_array_registry_find(r, type))
		return 1;

	if(!(e = am_array_registry_reserve_sorted(r, type)))
		return 1;

	e->type = type;
	e->allocate = allocate;
	e->free = free;
	e->init = init;
	e->destroy = destroy;

	return 0;
}
