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
{% set mem_types = aftermath.config.getMemTypes() -%}

#include <aftermath/core/in_memory_dfg_node_types.h>
{% for t in mem_types.filterByTag(aftermath.tags.mem.dfg.DeclareEventMappingOverlappingIntervalExtractionNode) -%}
{% set tag = t.getTagInheriting(aftermath.tags.mem.dfg.DeclareEventMappingOverlappingIntervalExtractionNode) -%}
#include {{tag.getIncludeFile()}}
{% endfor %}

{% for t in mem_types.filterByTag(aftermath.tags.mem.dfg.DeclareEventMappingOverlappingIntervalExtractionNode) -%}
{% set tag = t.getTagInheriting(aftermath.tags.mem.dfg.DeclareEventMappingOverlappingIntervalExtractionNode) -%}
AM_DFG_IMPL_EVENT_MAPPING_EXTRACT_OVERLAPPING_INTERVAL_NODE(
	{{tag.getStripNamePlural()}},
	{{t.getName()}},
	{{tag.getArrayStructName()}},
	"{{t.getIdent()}}")

{% endfor %}
