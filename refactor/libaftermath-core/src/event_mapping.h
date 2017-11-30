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
#include "ptr.h"

struct am_event_mapping_element {
	struct am_interval interval;
	struct am_event_collection* collection;
};

AM_DECL_TYPED_ARRAY(am_event_mapping_array, struct am_event_mapping_element)
AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING(am_event_mapping_array,
						       struct am_event_mapping_element,
						       interval)
AM_DECL_INTERVAL_EVENT_ARRAY_BSEARCH_LAST_OVERLAPPING(am_event_mapping_array,
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

/* Internal use; used to iterate over event the collections of an event
 * mapping. */
struct am_event_mapping_iterator {
	struct am_event_mapping_element* start;
	struct am_event_mapping_element* end;
};

/* Internal use; Initializes an event mapping iterator with the first and last
 * element overlapping with a query interval and assigns the collection of the
 * first element to *c. If none of the event collections of the event mapping
 * overlaps with the query interval *c is set to NULL. */
static inline struct am_event_mapping_iterator
am_event_mapping_iterator_start(struct am_event_mapping* m,
				const struct am_interval* query,
				struct am_event_collection** c)
{
	struct am_event_mapping_iterator it;

	it.start = am_event_mapping_array_bsearch_first_overlapping(&m->mappings,
								    query);

	if(!it.start) {
		*c = NULL;
		return it;
	}

	*c = it.start->collection;

	it.end = am_event_mapping_array_bsearch_last_overlapping(&m->mappings,
								 query);

	return it;
}

/* Internal use; Retrieves the next event collection after the current position
 * of an event mapping iterator. If the iterator has reached the end, the
 * function returns NULL. */
static inline struct am_event_collection*
am_event_mapping_iterator_next(struct am_event_mapping_iterator* it)
{
	it->start++;

	if(AM_PTR_LEQ(it->start, it->end))
		return it->start->collection;
	else
		return NULL;
}

/* Iterates over all event collections in an event mapping that overlap with a
 * query interval pquery. */
#define am_event_mapping_for_each_collection_overlapping(pmapping, query, pcoll)\
	for(struct am_event_mapping_iterator __it =				\
		    am_event_mapping_iterator_start(pmapping, query, &(pcoll)); \
	    (pcoll) && AM_PTR_LEQ(__it.start, __it.end);			\
	    (pcoll) = am_event_mapping_iterator_next(&__it))

#endif

