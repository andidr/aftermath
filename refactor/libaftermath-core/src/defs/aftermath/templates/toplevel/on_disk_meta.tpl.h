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

#ifndef AM_ON_DISK_META_H
#define AM_ON_DISK_META_H

#include <aftermath/core/base_types.h>
#include <aftermath/core/in_memory.h>
#include <aftermath/core/io_context.h>
#include <aftermath/core/typed_array.h>
#include <aftermath/core/qsort.h>
#include <aftermath/core/bsearch.h>
#include <aftermath/core/default_array_registry.h>

{% set meta_types = aftermath.config.getMetaTypes() -%}

#define AM_META_ACC_ID(x) (x).id
{# #}
{%- for t in meta_types.filterByTag(aftermath.tags.Compound).topologicalSort() -%}
{{ aftermath.templates.StructDefinition(t) }}

/* Define typed array {{t.getName()}}_array */
AM_DECL_TYPED_ARRAY({{t.getName()}}_array, {{t.getCType()}})

/* Define binary search function for {{t.getName()}}_array using the id as a
 * key */
AM_DECL_TYPED_ARRAY_BSEARCH({{t.getName()}}_array,
			    {{t.getCType()}},
			    {{t.getFields().getFieldByName("id").getType().getCType()}},
			    AM_META_ACC_ID,
			    AM_VALCMP_EXPR)

/* Comparison function for {{t.getName()}} */
static inline int {{t.getName()}}_cmp({{t.getCType()}}* a,
				      {{t.getCType()}}* b)
{
	return (a->id > b->id) ? 1 : (a->id < b->id ? -1 : 0);
}

/* Generate {{t.getName()}}_array_qsort */
AM_DECL_QSORT_SUFFIX({{t.getName()}}_array_,
		     , /* No suffix */
		     {{t.getCType()}},
		     {{t.getName()}}_cmp)
{% endfor %}

{% for t in meta_types -%}
{% for tag in t.getAllTagsInheriting(aftermath.tags.TemplatedGenerateFunctionTag) -%}
{% if not isinstance(tag, aftermath.tags.GenerateDestructor) -%}
{{ tag.instantiateTemplate() }}
{# #}
{% endif -%}
{% endfor -%}
{% endfor %}

{% for t in meta_types.filterByTag(aftermath.tags.Compound) -%}
AM_DECL_DEFAULT_ARRAY_REGISTRY_FUNCTIONS({{t.getName()}}_array)
{% endfor %}

static inline int am_build_default_meta_array_registry(struct am_array_registry* r)
{
	{% for t in meta_types.filterByTag(aftermath.tags.Compound) -%}
	if(AM_DEFAULT_ARRAY_REGISTRY_REGISTER(r,
					      {{t.getName()}}_array,
					      "{{t.getIdent()}}"))
	{
		return 1;
	}
{# #}
	{% endfor -%}

	return 0;
}

#endif
