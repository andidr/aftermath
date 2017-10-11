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

#ifndef AM_EVENT_MAPPING_H
#define AM_EVENT_MAPPING_H

/* An event mapping associates event collections to an entity (such as a
 * hierarchy node). Each mapping is a series of mapping elements, one for each
 * interval. */

#include "base_types.h"
#include "typed_array.h"
#include "interval_array.h"

struct am_event_mapping_element {
	struct am_interval interval;
	struct am_event_collection* collection;
};

AM_DECL_TYPED_ARRAY(am_event_mapping_array, struct am_event_mapping_element)
AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING(am_event_mapping_array,
						       struct am_event_mapping_element,
						       interval)

struct am_event_mapping {
	struct am_event_mapping_array mappings;
};

void am_event_mapping_init(struct am_event_mapping* m);
void am_event_mapping_destroy(struct am_event_mapping* m);
int am_event_mapping_append(struct am_event_mapping* m,
			    const struct am_interval* interval,
			    struct am_event_collection* c);
int am_event_mapping_append_safe(struct am_event_mapping* m,
				 const struct am_interval* interval,
				 struct am_event_collection* c);

#endif

