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

#ifndef AM_ARRAY_REGISTRY_H
#define AM_ARRAY_REGISTRY_H

#include "typed_array.h"

#define AM_ACC_ARRAY_REGISTRY_ENTRY_TYPE(x) ((x).type)

/* Single entry specifiying common operations for an event array type */
struct am_array_registry_entry {
	const char* type;

	void* (*allocate)(void);
	void (*free)(void* a);
	int (*init)(void* a);
	void (*destroy)(void* a);
};

AM_DECL_TYPED_ARRAY(am_array_registry, struct am_array_registry_entry)
AM_DECL_TYPED_ARRAY_BSEARCH(am_array_registry,
			    struct am_array_registry_entry,
			    char*,
			    AM_ACC_ARRAY_REGISTRY_ENTRY_TYPE,
			    strcmp)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_array_registry,
			      struct am_array_registry_entry,
			      char*,
			      AM_ACC_ARRAY_REGISTRY_ENTRY_TYPE,
			      strcmp)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_array_registry,
				   struct am_array_registry_entry,
				   char*)

struct am_array_registry_entry*
am_array_registry_find(struct am_array_registry* r, const char* type);

int
am_array_registry_add(struct am_array_registry* r,
		      const char* type,
		      void* (*allocate)(void),
		      void (*free)(void* a),
		      int (*init)(void* a),
		      void (*destroy)(void* a));

void* am_array_registry_allocate_array(struct am_array_registry* r,
				       const char* type,
				       int* type_found);
void am_array_registry_free_array(struct am_array_registry* r,
				  const char* type,
				  int* type_found,
				  void* a);
int am_array_registry_init_array(struct am_array_registry* r,
				 const char* type,
				 int* type_found,
				 void* a);
void am_array_registry_destroy_array(struct am_array_registry* r,
				     const char* type,
				     int* type_found,
				     void* a);
void am_array_registry_destroy_and_free_array(struct am_array_registry* r,
					      const char* type,
					      int* type_found,
					      void* a);
void* am_array_registry_allocate_and_init_array(struct am_array_registry* r,
						const char* type,
						int* type_found);

#endif
