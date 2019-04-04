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

#ifndef AM_IN_MEMORY_DFG_NODE_TYPES_H
#define AM_IN_MEMORY_DFG_NODE_TYPES_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/core/dfg/nodes/event_mapping_common.h>
#include <aftermath/core/in_memory.h>

{% set mem_types = aftermath.config.getMemTypes() -%}

{% for t in mem_types.filterByTag(aftermath.tags.mem.dfg.DeclareEventMappingOverlappingIntervalExtractionNode) -%}
{% set tag = t.getTagInheriting(aftermath.tags.mem.dfg.DeclareEventMappingOverlappingIntervalExtractionNode) -%}
/* A node extracting all {{t.getEntity()}} instances all event mappings on input
 * overlapping with at least one of the intervals provided at the "intervals"
 * input port. If the port is disconnected, all events of the mapping will be
 * output. */
AM_DFG_DECL_EVENT_MAPPING_EXTRACT_OVERLAPPING_INTERVAL_NODE(
	{{tag.getStripNamePlural()}}, "{{tag.getPortName()}}", "{{tag.getCapitalizedHumanReadablePlural()}}", "{{t.getIdent()}}")

{% endfor -%}

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	{% for t in mem_types.filterByTag(aftermath.tags.mem.dfg.DeclareEventMappingOverlappingIntervalExtractionNode) -%}
	{% set tag = t.getTagInheriting(aftermath.tags.mem.dfg.DeclareEventMappingOverlappingIntervalExtractionNode) -%}
	&am_dfg_event_mapping_{{tag.getStripNamePlural()}}_node_type{% if not loop.last %},{% endif %}
	{% endfor -%}
)

#endif
