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

#include <aftermath/core/default_event_array_registry.h>
#include <aftermath/core/state_event_array.h>
#include <aftermath/core/counter_event_array_collection.h>
#include <stdlib.h>

static void* def_am_state_event_array_allocate(void)
{
	return malloc(sizeof(struct am_state_event_array));
}

static void def_am_state_event_array_free(void* a)
{
	free(a);
}

static int def_am_state_event_array_init(void* a)
{
	am_state_event_array_init(a);
	return 0;
}

static void def_am_state_event_array_destroy(void* a)
{
	am_state_event_array_destroy(a);
}

static void* def_am_counter_event_array_collection_allocate(void)
{
	return malloc(sizeof(struct am_counter_event_array_collection));
}

static void def_am_counter_event_array_collection_free(void* a)
{
	free(a);
}

static int def_am_counter_event_array_collection_init(void* a)
{
	am_counter_event_array_collection_init(a);
	return 0;
}

static void def_am_counter_event_array_collection_destroy(void* a)
{
	am_counter_event_array_collection_destroy_elements(a);
	am_counter_event_array_collection_destroy(a);
}

int am_event_array_registry_add_default(struct am_event_array_registry* r)
{
	if(am_event_array_registry_add(r,
				       AM_EVENT_ARRAY_TYPE_STATE_EVENT,
				       def_am_state_event_array_allocate,
				       def_am_state_event_array_free,
				       def_am_state_event_array_init,
				       def_am_state_event_array_destroy))
	{
		return 1;
	}

	if(am_event_array_registry_add(r,
				      AM_EVENT_ARRAY_TYPE_COUNTER_EVENT_ARRAY_COLLECTION,
				      def_am_counter_event_array_collection_allocate,
				      def_am_counter_event_array_collection_free,
				      def_am_counter_event_array_collection_init,
				      def_am_counter_event_array_collection_destroy))
	{
		return 1;
	}

	return 0;
}
