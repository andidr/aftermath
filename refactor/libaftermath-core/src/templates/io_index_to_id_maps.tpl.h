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

#ifndef AM_IO_INDEX_TO_ID_MAPS_H
#define AM_IO_INDEX_TO_ID_MAPS_H

#include <aftermath/core/index_to_id_map.h>

{%- set mem = am_types.mem %}

/* Container structure for all the maps mapping array indexes to IDs */
struct am_io_index_to_id_maps {
	{% for t in am_types.filter_list_hasattrs(mem.types_list, ["index_to_id_maps"]) -%}
	{% if not t.is_pointer -%}
	{% for m in t.index_to_id_maps -%}
	struct am_index_to_id_map_u{{m.id_bits}} {{m.name}};
	{% endfor %}
	{%- endif %}
	{%- endfor %}
};

void am_io_index_to_id_maps_init(struct am_io_index_to_id_maps* maps);
void am_io_index_to_id_maps_destroy(struct am_io_index_to_id_maps* maps);

#endif
