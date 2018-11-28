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

#ifndef AM_IN_MEMORY_DFG_TYPES_H
#define AM_IN_MEMORY_DFG_TYPES_H

#include <aftermath/core/in_memory.h>

{% set mem_types = aftermath.config.getMemTypes() -%}

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_trace,
	"const am::core::trace*",
	sizeof(struct am_trace*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_event_mapping,
	"const am::core::event_mapping*",
	sizeof(struct am_event_mapping*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_hierarchy,
	"const am::core::hierarchy*",
	sizeof(struct am_event_hierarchy*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_hierarchy_node,
	"const am::core::hierarchy_node*",
	sizeof(struct am_event_hierarchy_node*),
	NULL, NULL, NULL, NULL)

{% for t in mem_types.filterByTag(aftermath.tags.mem.dfg.DeclareConstPointerType) -%}
/* DFG type for references to {{t.getName()}} */
AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_{{t.getStripName()}},
	"const {{t.getIdent()}}*",
	sizeof({{t.getCType()}}*),
	NULL, NULL, NULL, NULL)

{% endfor -%}
AM_DFG_ADD_BUILTIN_TYPES(
{%- for t in mem_types.filterByTag(aftermath.tags.mem.dfg.DeclareConstPointerType) %}
	&am_dfg_type_const_{{t.getStripName()}},
{%- endfor -%}
	&am_dfg_type_const_trace,
	&am_dfg_type_const_event_mapping,
	&am_dfg_type_const_hierarchy,
	&am_dfg_type_const_hierarchy_node)

#endif
