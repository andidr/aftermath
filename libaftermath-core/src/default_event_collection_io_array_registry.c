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

#include "default_event_collection_io_array_registry.h"
#include <aftermath/core/default_array_registry.h>
#include <aftermath/core/array_registry.h>
#include <aftermath/core/index_to_id_map.h>

/* AM_DECL_DEFAULT_ARRAY_REGISTRY_FUNCTIONS(am_index_to_id_map_u8) */
/* AM_DECL_DEFAULT_ARRAY_REGISTRY_FUNCTIONS(am_index_to_id_map_u16) */
/* AM_DECL_DEFAULT_ARRAY_REGISTRY_FUNCTIONS(am_index_to_id_map_u32) */
AM_DECL_DEFAULT_ARRAY_REGISTRY_FUNCTIONS(am_index_to_id_map_u64)

int
am_build_default_event_collection_io_array_registry(struct am_array_registry* r)
{
	if(AM_DEFAULT_ARRAY_REGISTRY_REGISTER(
	   	   r, am_index_to_id_map_u64, "am::openstream::task_period::instance_id"))
	{
		return 1;
	}

	return 0;
}
