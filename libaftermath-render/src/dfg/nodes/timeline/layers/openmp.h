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

#ifndef AM_RENDER_DFG_NODE_TIMELINE_LAYERS_OPENMP_H
#define AM_RENDER_DFG_NODE_TIMELINE_LAYERS_OPENMP_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/render/dfg/timeline_layer_common.h>

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_for_loop_type,
	"openmp::for_loop_type",
	"Timeline OpenMP Loop Type Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_for_loop_type,
	"openmp::for_loop_type",
	"Timeline OpenMP Loop Type Layer Configuration")

int am_render_dfg_timeline_openmp_for_loop_type_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_for_loop_instance,
	"openmp::for_loop_instance",
	"Timeline OpenMP Loop Instance Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_for_loop_instance,
	"openmp::for_loop_instance",
	"Timeline OpenMP Loop Instance Layer Configuration")

int am_render_dfg_timeline_openmp_for_loop_instance_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_iteration_set,
	"openmp::iteration_set",
	"Timeline OpenMP Iteration Set Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_iteration_set,
	"openmp::iteration_set",
	"Timeline OpenMP Iteration Set Layer Configuration")

int am_render_dfg_timeline_openmp_iteration_set_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_iteration_period,
	"openmp::iteration_period",
	"Timeline OpenMP Iteration Period Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_iteration_period,
	"openmp::iteration_period",
	"Timeline OpenMP Iteration Period Layer Configuration")

int am_render_dfg_timeline_openmp_iteration_period_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_task_type,
	"openmp::task_type",
	"Timeline OpenMP Task Type Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_task_type,
	"openmp::task_type",
	"Timeline OpenMP Task Type Layer Configuration")

int am_render_dfg_timeline_openmp_task_type_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_task_instance,
	"openmp::task_instance",
	"Timeline OpenMP Task Instance Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_task_instance,
	"openmp::task_instance",
	"Timeline OpenMP Task Instance Layer Configuration")

int am_render_dfg_timeline_openmp_task_instance_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	openmp_task_period,
	"openmp::task_period",
	"Timeline OpenMP Task Period Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	openmp_task_period,
	"openmp::task_period",
	"Timeline OpenMP Task Period Layer Configuration")

int am_render_dfg_timeline_openmp_task_period_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_render_dfg_timeline_openmp_for_loop_type_layer_filter_node_type,
	&am_render_dfg_timeline_openmp_for_loop_type_layer_configuration_node_type,
	&am_render_dfg_timeline_openmp_for_loop_instance_layer_filter_node_type,
	&am_render_dfg_timeline_openmp_for_loop_instance_layer_configuration_node_type,
	&am_render_dfg_timeline_openmp_iteration_set_layer_filter_node_type,
	&am_render_dfg_timeline_openmp_iteration_set_layer_configuration_node_type,
	&am_render_dfg_timeline_openmp_iteration_period_layer_filter_node_type,
	&am_render_dfg_timeline_openmp_iteration_period_layer_configuration_node_type,
	&am_render_dfg_timeline_openmp_task_type_layer_filter_node_type,
	&am_render_dfg_timeline_openmp_task_type_layer_configuration_node_type,
	&am_render_dfg_timeline_openmp_task_instance_layer_filter_node_type,
	&am_render_dfg_timeline_openmp_task_instance_layer_configuration_node_type,
	&am_render_dfg_timeline_openmp_task_period_layer_filter_node_type,
	&am_render_dfg_timeline_openmp_task_period_layer_configuration_node_type)

#endif
