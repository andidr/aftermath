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

#ifndef AM_EVENT_COLLECTION_MAPPING_H
#define AM_EVENT_COLLECTION_MAPPING_H

#include <aftermath/core/array_collection.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/base_types.h>

struct am_event_collection;
struct am_array_registry;

/* An event collection array mapping associates an array collection with an
 * event collection via the collection's ID. This is useful for arrays that
 * contain data for collections that is only used when the trace is loaded and
 * which is dicarded afterwards. */

struct am_event_collection_array_mapping_entry {
	am_event_collection_id_t collection_id;
	struct am_array_collection arrays;
};

#define AM_EVENT_COLLECTION_ARRAY_MAPPING_ENTRY_ACC_COLLECTION_ID(x) \
	(x).collection_id

AM_DECL_TYPED_ARRAY_NO_DESTRUCTOR(am_event_collection_array_mapping,
				  struct am_event_collection_array_mapping_entry)

AM_DECL_TYPED_ARRAY_BSEARCH(am_event_collection_array_mapping,
			    struct am_event_collection_array_mapping_entry,
			    am_event_collection_id_t,
			    AM_EVENT_COLLECTION_ARRAY_MAPPING_ENTRY_ACC_COLLECTION_ID,
			    AM_VALCMP_PTR)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_event_collection_array_mapping,
			      struct am_event_collection_array_mapping_entry,
			      am_event_collection_id_t,
			      AM_EVENT_COLLECTION_ARRAY_MAPPING_ENTRY_ACC_COLLECTION_ID,
			      AM_VALCMP_PTR)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_event_collection_array_mapping,
				   struct am_event_collection_array_mapping_entry,
				   am_event_collection_id_t)

void am_event_collection_array_mapping_destroy(
	struct am_event_collection_array_mapping* m,
	struct am_array_registry* r);

struct am_array_collection* am_event_collection_array_mapping_find(
	struct am_event_collection_array_mapping* m,
	struct am_event_collection* ecoll);

struct am_array_collection* am_event_collection_array_mapping_find_or_add(
	struct am_event_collection_array_mapping* m,
	struct am_event_collection* ecoll);


#endif
