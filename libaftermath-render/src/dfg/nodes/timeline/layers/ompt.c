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

#include <aftermath/render/dfg/nodes/timeline/layers/ompt.h>
#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/renderer.h>

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_thread,
	"ompt::thread",
	struct am_timeline_ompt_thread_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_thread,
	"ompt::thread")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_parallel,
	"ompt::parallel",
	struct am_timeline_ompt_parallel_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_parallel,
	"ompt::parallel")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_task_create,
	"ompt::task_create",
	struct am_timeline_ompt_task_create_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_task_create,
	"ompt::task_create")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_task_schedule,
	"ompt::task_schedule",
	struct am_timeline_ompt_task_schedule_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_task_schedule,
	"ompt::task_schedule")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_implicit_task,
	"ompt::implicit_task",
	struct am_timeline_ompt_implicit_task_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_implicit_task,
	"ompt::implicit_task")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_sync_region_wait,
	"ompt::sync_region_wait",
	struct am_timeline_ompt_sync_region_wait_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_sync_region_wait,
	"ompt::sync_region_wait")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_mutex_released,
	"ompt::mutex_released",
	struct am_timeline_ompt_mutex_released_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_mutex_released,
	"ompt::mutex_released")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_dependences,
	"ompt::dependences",
	struct am_timeline_ompt_dependences_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_dependences,
	"ompt::dependences")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_task_dependence,
	"ompt::task_dependence",
	struct am_timeline_ompt_task_dependence_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_task_dependence,
	"ompt::task_dependence")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_work,
	"ompt::work",
	struct am_timeline_ompt_work_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_work,
	"ompt::work")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_master,
	"ompt::master",
	struct am_timeline_ompt_master_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_master,
	"ompt::master")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_sync_region,
	"ompt::sync_region",
	struct am_timeline_ompt_sync_region_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_sync_region,
	"ompt::sync_region")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_lock_init,
	"ompt::lock_init",
	struct am_timeline_ompt_lock_init_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_lock_init,
	"ompt::lock_init")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_lock_destroy,
	"ompt::lock_destroy",
	struct am_timeline_ompt_lock_destroy_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_lock_destroy,
	"ompt::lock_destroy")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_mutex_acquire,
	"ompt::mutex_acquire",
	struct am_timeline_ompt_mutex_acquire_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_mutex_acquire,
	"ompt::mutex_acquire")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_mutex_acquired,
	"ompt::mutex_acquired",
	struct am_timeline_ompt_mutex_acquired_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_mutex_acquired,
	"ompt::mutex_acquired")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_nest_lock,
	"ompt::nest_lock",
	struct am_timeline_ompt_nest_lock_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_nest_lock,
	"ompt::nest_lock")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_flush,
	"ompt::flush",
	struct am_timeline_ompt_flush_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_flush,
	"ompt::flush")

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	ompt_cancel,
	"ompt::cancel",
	struct am_timeline_ompt_cancel_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	ompt_cancel,
	"ompt::cancel")

