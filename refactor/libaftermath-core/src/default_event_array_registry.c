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
#include <aftermath/core/default_array_registry.h>
#include <aftermath/core/state_event_array.h>
#include <aftermath/core/counter_event_array_collection.h>
#include <aftermath/core/openstream_task_period_array.h>
#include <stdlib.h>

AM_DECL_DEFAULT_ARRAY_REGISTRY_FUNCTIONS(am_state_event_array)
AM_DECL_DEFAULT_ARRAY_REGISTRY_FUNCTIONS(am_counter_event_array_collection)
AM_DECL_DEFAULT_ARRAY_REGISTRY_FUNCTIONS(am_openstream_task_period_array)

int am_build_default_event_array_registry(struct am_array_registry* r)
{
	if(AM_DEFAULT_ARRAY_REGISTRY_REGISTER(
		   r, am_state_event_array, "am::generic::state") ||
	   AM_DEFAULT_ARRAY_REGISTRY_REGISTER(
		   r, am_counter_event_array_collection, "am::generic::counter") ||
	   AM_DEFAULT_ARRAY_REGISTRY_REGISTER(
		   r, am_openstream_task_period_array, "am::openstream::task_period"))
	{
		return 1;
	}

	return 0;
}
