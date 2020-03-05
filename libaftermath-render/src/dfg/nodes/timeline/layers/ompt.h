/**
 * Author: Andi Drebes <andi@drebesium.org>
 * Author: Igor Wodiany <igor.wodiany@manchester.ac.uk>
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

#ifndef AM_RENDER_DFG_NODE_TIMELINE_LAYERS_OMPT_H
#define AM_RENDER_DFG_NODE_TIMELINE_LAYERS_OMPT_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/render/dfg/timeline_layer_common.h>

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_thread,
	"ompt::thread",
	"Timeline OMPT Thread Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_thread,
	"ompt::thread",
	"Timeline OMPT Thread Layer Configuration")

int am_render_dfg_timeline_ompt_thread_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_parallel,
	"ompt::parallel",
	"Timeline OMPT Parallel Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_parallel,
	"ompt::parallel",
	"Timeline OMPT Parallel Layer Configuration")

int am_render_dfg_timeline_ompt_parallel_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_task_create,
	"ompt::task_create",
	"Timeline OMPT Task Create Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_task_create,
	"ompt::task_create",
	"Timeline OMPT Task Create Layer Configuration")

int am_render_dfg_timeline_ompt_task_create_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_task_schedule,
	"ompt::task_schedule",
	"Timeline OMPT Task Schedule Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_task_schedule,
	"ompt::task_schedule",
	"Timeline OMPT Task Schedule Layer Configuration")

int am_render_dfg_timeline_ompt_task_schedule_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_implicit_task,
	"ompt::implicit_task",
	"Timeline OMPT Implicit Task Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_implicit_task,
	"ompt::implicit_task",
	"Timeline OMPT Implicit Task Layer Configuration")

int am_render_dfg_timeline_ompt_implicit_task_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_sync_region_wait,
	"ompt::sync_region_wait",
	"Timeline OMPT Sync Region Wait Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_sync_region_wait,
	"ompt::sync_region_wait",
	"Timeline OMPT Sync Region Wait Layer Configuration")

int am_render_dfg_timeline_ompt_sync_region_wait_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_mutex_released,
	"ompt::mutex_released",
	"Timeline OMPT Mutex Released Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_mutex_released,
	"ompt::mutex_released",
	"Timeline OMPT Mutex Released Layer Configuration")

int am_render_dfg_timeline_ompt_mutex_released_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_dependences,
	"ompt::dependences",
	"Timeline OMPT Dependences Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_dependences,
	"ompt::dependences",
	"Timeline OMPT Dependences Layer Configuration")

int am_render_dfg_timeline_ompt_dependences_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_task_dependence,
	"ompt::task_dependence",
	"Timeline OMPT Task Dependence Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_task_dependence,
	"ompt::task_dependence",
	"Timeline OMPT Task Dependence Layer Configuration")

int am_render_dfg_timeline_ompt_task_dependence_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_work,
	"ompt::work",
	"Timeline OMPT Worksharing Region Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_work,
	"ompt::work",
	"Timeline OMPT Worksharing Region Layer Configuration")

int am_render_dfg_timeline_ompt_work_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_master,
	"ompt::master",
	"Timeline OMPT Master Region Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_master,
	"ompt::master",
	"Timeline OMPT Master Region Layer Configuration")

int am_render_dfg_timeline_ompt_master_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_sync_region,
	"ompt::sync_region",
	"Timeline OMPT Sync Region Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_sync_region,
	"ompt::sync_region",
	"Timeline OMPT Sync Region Layer Configuration")

int am_render_dfg_timeline_ompt_sync_region_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_lock_init,
	"ompt::lock_init",
	"Timeline OMPT Mutex Lock Init Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_lock_init,
	"ompt::lock_init",
	"Timeline OMPT Mutex Lock Init Configuration")

int am_render_dfg_timeline_ompt_lock_init_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_lock_destroy,
	"ompt::lock_destroy",
	"Timeline OMPT Mutex Lock Destroy Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_lock_destroy,
	"ompt::lock_destroy",
	"Timeline OMPT Mutex Lock Destroy Configuration")

int am_render_dfg_timeline_ompt_lock_destroy_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_mutex_acquire,
	"ompt::mutex_acquire",
	"Timeline OMPT Mutex Acquire Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_mutex_acquire,
	"ompt::mutex_acquire",
	"Timeline OMPT Mutex Acquire Layer Configuration")

int am_render_dfg_timeline_ompt_mutex_acquire_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_mutex_acquired,
	"ompt::mutex_acquired",
	"Timeline OMPT Mutex Acquired Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_mutex_acquired,
	"ompt::mutex_acquired",
	"Timeline OMPT Mutex Acquired Layer Configuration")

int am_render_dfg_timeline_ompt_mutex_acquired_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_nest_lock,
	"ompt::nest_lock",
	"Timeline OMPT Nest Lock Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_nest_lock,
	"ompt::nest_lock",
	"Timeline OMPT Nest Lock Layer Configuration")

int am_render_dfg_timeline_ompt_nest_lock_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_flush,
	"ompt::flush",
	"Timeline OMPT Flush Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_flush,
	"ompt::flush",
	"Timeline OMPT Flush Layer Configuration")

int am_render_dfg_timeline_ompt_flush_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_RENDER_DFG_DECL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_cancel,
	"ompt::cancel",
	"Timeline OMPT Cancel Layer Filter")

AM_RENDER_DFG_DECL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_cancel,
	"ompt::cancel",
	"Timeline OMPT Cancel Layer Configuration")

int am_render_dfg_timeline_ompt_cancel_layer_configuration_node_process(
	struct am_dfg_node* n);

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_render_dfg_timeline_ompt_thread_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_thread_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_parallel_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_parallel_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_task_create_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_task_create_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_task_schedule_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_task_schedule_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_implicit_task_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_implicit_task_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_sync_region_wait_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_sync_region_wait_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_mutex_released_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_mutex_released_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_dependences_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_dependences_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_task_dependence_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_task_dependence_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_work_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_work_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_master_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_master_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_sync_region_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_sync_region_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_lock_init_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_lock_init_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_lock_destroy_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_lock_destroy_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_mutex_acquire_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_mutex_acquire_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_mutex_acquired_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_mutex_acquired_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_nest_lock_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_nest_lock_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_flush_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_flush_layer_configuration_node_type,
	&am_render_dfg_timeline_ompt_cancel_layer_filter_node_type,
	&am_render_dfg_timeline_ompt_cancel_layer_configuration_node_type)

#endif
