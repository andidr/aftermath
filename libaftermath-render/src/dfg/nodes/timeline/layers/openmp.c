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

#include <aftermath/render/dfg/nodes/timeline/layers/openmp.h>
#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/renderer.h>

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_for_loop_type,
	"openmp::for_loop_type",
	struct am_timeline_openmp_for_loop_type_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_for_loop_type,
	"openmp::for_loop_type")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_for_loop_instance,
	"openmp::for_loop_instance",
	struct am_timeline_openmp_for_loop_instance_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_for_loop_instance,
	"openmp::for_loop_instance")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_iteration_set,
	"openmp::iteration_set",
	struct am_timeline_openmp_iteration_set_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_iteration_set,
	"openmp::iteration_set")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_iteration_period,
	"openmp::iteration_period",
	struct am_timeline_openmp_iteration_period_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_iteration_period,
	"openmp::iteration_period")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_task_type,
	"openmp::task_type",
	struct am_timeline_openmp_task_type_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_task_type,
	"openmp::task_type")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_task_instance,
	"openmp::task_instance",
	struct am_timeline_openmp_task_instance_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_task_instance,
	"openmp::task_instance")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_task_period,
	"openmp::task_period",
	struct am_timeline_openmp_task_period_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_task_period,
	"openmp::task_period")
