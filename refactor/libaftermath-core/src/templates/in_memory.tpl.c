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

#include <aftermath/core/in_memory.h>
#include <stdlib.h>
{# #}

{%- set mem = am_types.mem %}

{%- for t in mem.types_list %}
{%- if "destructor" in t.defs %}
void {{t.destructor}}({{t.c_type}}* e)
{
	{%- for field in t.fields -%}
	{%- set field_type = mem.find(field.type) -%}
	{% if field_type.destructor %}
	{{field_type.destructor}}(&e->{{field.name}});
	{%- endif %}
	{%- endfor %}
}
{% endif -%}
{% endfor -%}
