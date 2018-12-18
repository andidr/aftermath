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

#include <aftermath/core/dfg_builtin_node_types.h>
#include <aftermath/core/dfg_builtin_node_impl.h>

#undef DEFS_NAME
#define DEFS_NAME() in_memory_defs
#include <aftermath/core/in_memory_dfg_node_types.h>

#undef DEFS_NAME
#define DEFS_NAME() bool_constant_defs
#include <aftermath/core/dfg/nodes/bool_constant.h>

#undef DEFS_NAME
#define DEFS_NAME() count_defs
#include <aftermath/core/dfg/nodes/count.h>

#undef DEFS_NAME
#define DEFS_NAME() double_constant_defs
#include <aftermath/core/dfg/nodes/double_constant.h>

#undef DEFS_NAME
#define DEFS_NAME() hierarchy_attributes_defs
#include <aftermath/core/dfg/nodes/hierarchy_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() hierarchy_node_attributes_defs
#include <aftermath/core/dfg/nodes/hierarchy_node_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() hierarchy_node_selector_defs
#include <aftermath/core/dfg/nodes/hierarchy_node_selector.h>

#undef DEFS_NAME
#define DEFS_NAME() hierarchy_selector_defs
#include <aftermath/core/dfg/nodes/hierarchy_selector.h>

#undef DEFS_NAME
#define DEFS_NAME() histogram_attributes_defs
#include <aftermath/core/dfg/nodes/histogram_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() histogram_builder_defs
#include <aftermath/core/dfg/nodes/histogram_builder.h>

#undef DEFS_NAME
#define DEFS_NAME() int_constant_defs
#include <aftermath/core/dfg/nodes/int_constant.h>

#undef DEFS_NAME
#define DEFS_NAME() int_to_string_defs
#include <aftermath/core/dfg/nodes/int_to_string.h>

#undef DEFS_NAME
#define DEFS_NAME() interval_attributes_defs
#include <aftermath/core/dfg/nodes/interval_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() interval_duration_defs
#include <aftermath/core/dfg/nodes/interval_duration.h>

#undef DEFS_NAME
#define DEFS_NAME() duration_to_string_defs
#include <aftermath/core/dfg/nodes/duration_to_string.h>

#undef DEFS_NAME
#define DEFS_NAME() duration_to_uint64_defs
#include <aftermath/core/dfg/nodes/duration_to_uint64.h>

#undef DEFS_NAME
#define DEFS_NAME() merge_defs
#include <aftermath/core/dfg/nodes/merge.h>

#undef DEFS_NAME
#define DEFS_NAME() pair_timestamp_hierarchy_node_attributes_defs
#include <aftermath/core/dfg/nodes/pair_timestamp_hierarchy_node_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() state_description_attributes_defs
#include <aftermath/core/dfg/nodes/state_description_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() state_event_attributes_defs
#include <aftermath/core/dfg/nodes/state_event_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() string_constant_defs
#include <aftermath/core/dfg/nodes/string_constant.h>

#undef DEFS_NAME
#define DEFS_NAME() string_concat_defs
#include <aftermath/core/dfg/nodes/string_concat.h>

#undef DEFS_NAME
#define DEFS_NAME() string_format_defs
#include <aftermath/core/dfg/nodes/string_format.h>

#undef DEFS_NAME
#define DEFS_NAME() timestamp_to_string_defs
#include <aftermath/core/dfg/nodes/timestamp_to_string.h>

#undef DEFS_NAME
#define DEFS_NAME() timestamp_to_uint64_defs
#include <aftermath/core/dfg/nodes/timestamp_to_uint64.h>

#undef DEFS_NAME
#define DEFS_NAME() tensorflow_node_attributes_defs
#include <aftermath/core/dfg/nodes/tensorflow_node_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() tensorflow_node_execution_attributes_defs
#include <aftermath/core/dfg/nodes/tensorflow_node_execution_attributes.h>

#undef DEFS_NAME
#define DEFS_NAME() trace_defs
#include <aftermath/core/dfg/nodes/trace.h>

/* Final list of all lists of node types from all headers included above */
static struct am_dfg_static_node_type_def** defsets[] = {
	in_memory_defs,
	bool_constant_defs,
	count_defs,
	double_constant_defs,
	duration_to_string_defs,
	duration_to_uint64_defs,
	hierarchy_attributes_defs,
	hierarchy_node_attributes_defs,
	hierarchy_node_selector_defs,
	hierarchy_selector_defs,
	histogram_attributes_defs,
	histogram_builder_defs,
	int_constant_defs,
	int_to_string_defs,
	interval_attributes_defs,
	interval_duration_defs,
	merge_defs,
	pair_timestamp_hierarchy_node_attributes_defs,
	state_description_attributes_defs,
	state_event_attributes_defs,
	string_concat_defs,
	string_constant_defs,
	string_format_defs,
	timestamp_to_string_defs,
	timestamp_to_uint64_defs,
	tensorflow_node_attributes_defs,
	tensorflow_node_execution_attributes_defs,
	trace_defs,
	NULL
};

/* Register the builtin node types at the node type registry ntr using the type
 * registry tr. Returns 0 on success, otherwise 1. */
int am_dfg_builtin_node_types_register(struct am_dfg_node_type_registry* ntr,
				       struct am_dfg_type_registry* tr)
{
	return am_dfg_node_type_registry_add_static(ntr, tr, defsets);
}
