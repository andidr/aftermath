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

#include "hierarchy.h"
#include "typed_array.h"

struct am_event_array_registry;

/* Types of event arrays that can be part of an event array collection. */
enum am_event_array_type {
	AM_EVENT_ARRAY_TYPE_STATE_EVENT,
	AM_EVENT_ARRAY_TYPE_COUNTER_EVENT_ARRAY_COLLECTION,
	AM_EVENT_ARRAY_TYPE_MAX = 0
};

/* A single entry of an event array collection, associating the numerical ID
 * with the address of the array. */
struct am_event_array_type_entry {
	enum am_event_array_type type;
	void* array;
};

void am_event_array_type_entry_destroy(struct am_event_array_type_entry* e,
				       struct am_event_array_registry* r);

#define ACC_TYPE(x) (x).type

AM_DECL_TYPED_ARRAY(am_event_array_collection, struct am_event_array_type_entry)
AM_DECL_TYPED_ARRAY_BSEARCH(am_event_array_collection,
			    struct am_event_array_type_entry,
			    enum am_event_array_type,
			    ACC_TYPE)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_event_array_collection,
			      struct am_event_array_type_entry,
			      enum am_event_array_type,
			      ACC_TYPE)

void event_array_collection_destroy_elements(struct am_event_array_collection* a,
					     struct am_event_array_registry* r);

/* Single entry specifiying common operations for an event array type */
struct am_event_array_registry_entry {
	enum am_event_array_type type;

	void* (*allocate)(void);
	void (*free)(void* a);
	int (*init)(void* a);
	void (*destroy)(void* a);
};

AM_DECL_TYPED_ARRAY(am_event_array_registry,
		    struct am_event_array_registry_entry)
AM_DECL_TYPED_ARRAY_BSEARCH(am_event_array_registry,
			    struct am_event_array_registry_entry,
			    enum am_event_array_type,
			    ACC_TYPE)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_event_array_registry,
			      struct am_event_array_registry_entry,
			      enum am_event_array_type,
			      ACC_TYPE)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_event_array_registry,
				   struct am_event_array_registry_entry,
				   enum am_event_array_type)

struct am_event_array_registry_entry*
event_array_registry_find(struct am_event_array_registry* r,
			  enum am_event_array_type type);
int am_event_array_registry_add(struct am_event_array_registry* r,
				enum am_event_array_type type,
				void* (*allocate)(void),
				void (*free)(void* a),
				int (*init)(void* a),
				void (*destroy)(void* a));

void* am_event_registry_allocate_array(struct am_event_array_registry* r,
				       enum am_event_array_type type,
				       int* type_found);
void am_event_registry_free_array(struct am_event_array_registry* r,
				  enum am_event_array_type type,
				  int* type_found,
				  void* a);
int am_event_registry_init_array(struct am_event_array_registry* r,
				 enum am_event_array_type type,
				 int* type_found,
				 void* a);
void am_event_registry_destroy_array(struct am_event_array_registry* r,
				     enum am_event_array_type type,
				     int* type_found,
				     void* a);
void am_event_registry_destroy_and_free_array(struct am_event_array_registry* r,
					      enum am_event_array_type type,
					      int* type_found,
					      void* a);
void* am_event_registry_allocate_and_init_array(struct am_event_array_registry* r,
						enum am_event_array_type type,
						int* type_found);

/* Single event collection, respresenting a logical execution stream */
struct am_event_collection {
	struct am_event_array_collection event_arrays;
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
				 struct am_event_array_registry* r);

void* am_event_collection_find_event_array(struct am_event_collection* e,
					   enum am_event_array_type t);

void* am_event_collection_find_or_add_event_array(struct am_event_array_registry* r,
						  struct am_event_collection* e,
						  enum am_event_array_type t);

int am_event_collection_add_event_array(struct am_event_collection* e,
					enum am_event_array_type t,
					void* array);

#endif
