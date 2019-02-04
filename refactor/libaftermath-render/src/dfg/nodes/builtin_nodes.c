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

#include <aftermath/render/dfg/nodes/builtin_nodes.h>
#include <aftermath/core/dfg_builtin_node_impl.h>

#undef DEFS_NAME
#define DEFS_NAME() axes_defs
#include <aftermath/render/dfg/nodes/timeline/layers/axes.h>

#undef DEFS_NAME
#define DEFS_NAME() telamon_evaluation_defs
#include <aftermath/render/dfg/nodes/timeline/layers/telamon_evaluation.h>

#undef DEFS_NAME
#define DEFS_NAME() tfexec_defs
#include <aftermath/render/dfg/nodes/timeline/layers/tensorflow_node_execution.h>

#undef DEFS_NAME
#define DEFS_NAME() background_defs
#include <aftermath/render/dfg/nodes/timeline/layers/background.h>

#undef DEFS_NAME
#define DEFS_NAME() hierarchy_defs
#include <aftermath/render/dfg/nodes/timeline/layers/hierarchy.h>

#undef DEFS_NAME
#define DEFS_NAME() rgba_constant_defs
#include <aftermath/render/dfg/nodes/rgba_constant.h>

#undef DEFS_NAME
#define DEFS_NAME() state_defs
#include <aftermath/render/dfg/nodes/timeline/layers/state.h>

/* Final list of all lists of node types from all headers included above */
static struct am_dfg_static_node_type_def** defsets[] = {
	axes_defs,
	background_defs,
	hierarchy_defs,
	rgba_constant_defs,
	state_defs,
	telamon_evaluation_defs,
	tfexec_defs,
	NULL
};

/* Registers all DFG node types for libaftermath-render.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_render_dfg_builtin_node_types_register(
	struct am_dfg_node_type_registry* ntr,
	struct am_dfg_type_registry* tr)
{
	return am_dfg_node_type_registry_add_static(ntr, tr, defsets);
}
