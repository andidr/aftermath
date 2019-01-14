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

{% set mem_types = aftermath.config.getMemTypes() %}

/* Forward declarations for pointers used in the struct definitions */
{% for t in mem_types.filterByTag(aftermath.tags.Compound) -%}
{{ t.getCType() }};
{% endfor -%}
{# #}

{% for t in mem_types.topologicalSort().filterByTag(aftermath.tags.Compound) -%}
{{ aftermath.templates.StructDefinition(t) }}
{% endfor -%}

{% for t in mem_types.filterByTag(aftermath.tags.GenerateDestructor) -%}
{{ aftermath.templates.Destructor(t).getPrototype() }}
{% endfor -%}

{% for t in mem_types.filterByTag(aftermath.tags.GenerateDefaultConstructor) -%}
{{ aftermath.templates.DefaultConstructor(t).getPrototype() }}
{% endfor -%}

{# #}
#endif
