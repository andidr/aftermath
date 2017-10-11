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

#include "event_mapping.h"

void am_event_mapping_init(struct am_event_mapping* m)
{
	am_event_mapping_array_init(&m->mappings);
}

void am_event_mapping_destroy(struct am_event_mapping* m)
{
	am_event_mapping_array_destroy(&m->mappings);
}

/* Append a mapping for a single interval to the mappings. Returns 0 on success,
 * otherwise 1. */
int am_event_mapping_append(struct am_event_mapping* m,
			    const struct am_interval* interval,
			    struct am_event_collection* c)
{
	struct am_event_mapping_element e = {
		.interval = *interval,
		.collection = c
	};

	return am_event_mapping_array_append(&m->mappings, e);
}

/* Append a mapping for a single interval to the mappings. The mapping is only
 * appended, if its interval starts after the interval of the last
 * mapping. Returns 0 on success, otherwise 1. */
int am_event_mapping_append_safe(struct am_event_mapping* m,
				 const struct am_interval* interval,
				 struct am_event_collection* c)
{
	struct am_event_mapping_element* last;

	if(m->mappings.num_elements > 0) {
		last = &m->mappings.elements[m->mappings.num_elements-1];

		if(interval->start < last->interval.start)
			return 1;
	}

	return am_event_mapping_append(m, interval, c);
}
