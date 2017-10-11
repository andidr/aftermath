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

#include "trace.h"
#include "default_event_array_registry.h"

int am_trace_init(struct am_trace* t, const char* filename)
{
	if(!(t->filename = strdup(filename)))
		return 1;

	am_hierarchyp_array_init(&t->hierarchies);
	am_event_collection_array_init(&t->event_collections);
	am_state_description_array_init(&t->state_descriptions);
	am_counter_description_array_init(&t->counter_descriptions);
	am_event_array_registry_init(&t->event_array_registry);
	am_measurement_interval_array_init(&t->measurement_intervals);

	if(am_event_array_registry_add_default(&t->event_array_registry)) {
		am_trace_destroy(t);
		return 1;
	}

	return 0;
}

void am_trace_destroy(struct am_trace* t)
{
	am_measurement_interval_array_destroy(&t->measurement_intervals);
	am_counter_description_array_destroy_elements(&t->counter_descriptions);
	am_counter_description_array_destroy(&t->counter_descriptions);

	am_state_description_array_destroy_elements(&t->state_descriptions);
	am_state_description_array_destroy(&t->state_descriptions);

	am_event_collection_array_destroy_elements(&t->event_collections,
						&t->event_array_registry);
	am_event_collection_array_destroy(&t->event_collections);

	am_hierarchyp_array_destroy_elements(&t->hierarchies);
	am_hierarchyp_array_destroy(&t->hierarchies);

	am_event_array_registry_destroy(&t->event_array_registry);

	free(t->filename);
}
