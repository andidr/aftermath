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

#ifndef AM_IN_MEMORY_H
#define AM_IN_MEMORY_H

#include <aftermath/core/base_types.h>

{% set mem = am_types.mem %}

/* Forward declarations for pointers used in the struct definitions */
{% for tname in mem.types -%}
{% set t = mem.types[tname] -%}
{% if t.compound and not t.is_pointer -%}
{{t.c_type}};
{% endif %}
{%- endfor -%}
{# #}

{% for tname in am_types.topological_sort(mem.types) -%}
{% set t = mem.types[tname] -%}
{% if "type" in t.defs -%}
/* {{t.comment}} */
{{t.c_type}} {
	{%- for field in t.fields %}
	/* {{field.comment}} */
	{{mem.find(field.type).c_type}} {{field.name}};
{# #}
{%- endfor -%}
};
{# #}
{% endif -%}
{% if "destructor" in t.defs and not t.is_pointer -%}
void {{t.destructor}}({{t.c_type}}* e);
{# #}
{% endif -%}
{% endfor -%}

{# #}
#endif
