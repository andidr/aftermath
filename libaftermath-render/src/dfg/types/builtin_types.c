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

#include <aftermath/core/dfg_builtin_type_impl.h>
#include <aftermath/core/dfg/types/generic.h>
#include <aftermath/render/cairo_extras.h>
#include <aftermath/render/dfg/types/builtin_types.h>
#include <aftermath/render/dfg/timeline_layer_common.h>
#include <aftermath/render/timeline/layer.h>

AM_DFG_DECL_BUILTIN_TYPE(
	am_render_dfg_type_timeline_layer,
	"const am::render::timeline::layer*",
	sizeof(struct am_timeline_render_layer*),
	NULL,
	am_dfg_type_generic_plain_copy_samples,
	NULL, NULL, NULL)

#undef DEFS_NAME
#define DEFS_NAME() am_render_dfg_type_rgba_defs
#include <aftermath/render/dfg/types/rgba.h>

AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(axes, "axes")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(background, "background")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(hierarchy, "hierarchy")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(measurement_intervals, "measurement_intervals")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(openmp_for_loop_type, "openmp::for_loop_type")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(openmp_for_loop_instance, "openmp::for_loop_instance")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(openmp_iteration_set, "openmp::iteration_set")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(openmp_iteration_period, "openmp::iteration_period")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(openmp_task_type, "openmp::task_type")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(openmp_task_instance, "openmp::task_instance")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(openmp_task_period, "openmp::task_period")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(selection, "selection")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(state, "state")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(telamon_evaluation, "telamon::evaluation")
AM_RENDER_DFG_DECL_TIMELINE_LAYER_TYPE(tensorflow_node_execution, "tensorflow::node_execution")

static struct am_dfg_static_type_def* builtin_defs[] = {
	&am_render_dfg_type_timeline_layer,
	&am_render_dfg_type_timeline_axes_layer,
	&am_render_dfg_type_timeline_background_layer,
	&am_render_dfg_type_timeline_hierarchy_layer,
	&am_render_dfg_type_timeline_measurement_intervals_layer,
	&am_render_dfg_type_timeline_openmp_for_loop_type_layer,
	&am_render_dfg_type_timeline_openmp_for_loop_instance_layer,
	&am_render_dfg_type_timeline_openmp_iteration_set_layer,
	&am_render_dfg_type_timeline_openmp_iteration_period_layer,
	&am_render_dfg_type_timeline_openmp_task_type_layer,
	&am_render_dfg_type_timeline_openmp_task_instance_layer,
	&am_render_dfg_type_timeline_openmp_task_period_layer,
	&am_render_dfg_type_timeline_selection_layer,
	&am_render_dfg_type_timeline_state_layer,
	&am_render_dfg_type_timeline_telamon_evaluation_layer,
	&am_render_dfg_type_timeline_tensorflow_node_execution_layer,
	NULL
};

static struct am_dfg_static_type_def** defsets[] = {
	builtin_defs,
	am_render_dfg_type_rgba_defs,
	NULL
};

/* Registers all builtin types for libaftermath-render.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_render_dfg_builtin_types_register(struct am_dfg_type_registry* tr)
{
	return am_dfg_type_registry_add_static(tr, defsets);
}
