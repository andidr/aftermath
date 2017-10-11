/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * Libaftermath-trace is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_BASE_TYPES_H
#define AM_BASE_TYPES_H

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

{% for t in am_types.base.types_list|sort -%}
{% if "type" in t.defs %}
{%- if t.comment %}
/* {{t.comment}} */
{%- endif %}
typedef {{t.c_def}} {{t.c_type}};

{%- if re.match("u?int[0-9]+_t", t.c_def) %}
{%- set nbits = re.match("(u?)int([0-9]+)_t", t.c_def) -%}
{%- set signmod = "d" -%}

{%- if nbits.group(1) == "u" -%}
  {% set signmod = "u" -%}
{% endif %}
#define {{t.c_type|upper}}_FMT PRI{{signmod}}{{nbits.group(2)}}
{%- endif %}
{% endif -%}
{% endfor -%}

{# #}
static inline void am_free_charp(char** pstr)
{
	free(*pstr);
}

#endif
