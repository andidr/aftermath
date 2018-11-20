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

#include <aftermath/core/dfg_builtin_types.h>
#include <aftermath/core/dfg_builtin_type_impl.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_bool
#include <aftermath/core/dfg/types/bool.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_duration
#include <aftermath/core/dfg/types/duration.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_histogram
#include <aftermath/core/dfg/types/histogram.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_histogram_data
#include <aftermath/core/dfg/types/histogram_data.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_int
#include <aftermath/core/dfg/types/int.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_pair_timestamp_const_hierarchy_node
#include <aftermath/core/dfg/types/pair_timestamp_hierarchy_node.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_interval
#include <aftermath/core/dfg/types/interval.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_string
#include <aftermath/core/dfg/types/string.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_timestamp
#include <aftermath/core/dfg/types/timestamp.h>

#undef DEFS_NAME
#define DEFS_NAME() am_dfg_type_set_in_memory
#include <aftermath/core/dfg/types/in_memory.h>


static struct am_dfg_static_type_def** defsets[] = {
	am_dfg_type_set_bool,
	am_dfg_type_set_duration,
	am_dfg_type_set_histogram,
	am_dfg_type_set_histogram_data,
	am_dfg_type_set_int,
	am_dfg_type_set_interval,
	am_dfg_type_set_pair_timestamp_const_hierarchy_node,
	am_dfg_type_set_string,
	am_dfg_type_set_timestamp,
	am_dfg_type_set_in_memory,
	NULL
};

/* Registers the all builtin DFG types at the registry tr. Returns 0 on success,
 * otherwise 1. */
int am_dfg_builtin_types_register(struct am_dfg_type_registry* tr)
{
	return am_dfg_type_registry_add_static(tr, defsets);
}
