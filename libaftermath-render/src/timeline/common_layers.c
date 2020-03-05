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

#include <aftermath/render/timeline/common_layers.h>
#include <aftermath/render/timeline/layers/axes.h>
#include <aftermath/render/timeline/layers/background.h>
#include <aftermath/render/timeline/layers/hierarchy.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/layers/measurement_intervals.h>
#include <aftermath/render/timeline/layers/selection.h>
#include <aftermath/render/timeline/renderer.h>
#include <aftermath/core/ansi_extras.h>
#include <aftermath/core/trace.h>

#include <aftermath/render/timeline/layers/lane/state_event.h>
#include <aftermath/render/timeline/layers/lane/tensorflow/node_execution.h>

#include <aftermath/render/timeline/layers/lane/openmp/openmp.h>
#include <aftermath/render/timeline/layers/lane/ompt/ompt.h>

static struct am_timeline_render_layer_type* (*inst_functions[])(void) = {
	am_timeline_axes_layer_instantiate_type,
	am_timeline_background_layer_instantiate_type,
	am_timeline_hierarchy_layer_instantiate_type,
	am_timeline_measurement_intervals_layer_instantiate_type,
	am_timeline_openmp_for_loop_type_layer_instantiate_type,
	am_timeline_openmp_for_loop_instance_layer_instantiate_type,
	am_timeline_openmp_iteration_set_layer_instantiate_type,
	am_timeline_openmp_iteration_period_layer_instantiate_type,
	am_timeline_openmp_task_type_layer_instantiate_type,
	am_timeline_openmp_task_instance_layer_instantiate_type,
	am_timeline_openmp_task_period_layer_instantiate_type,
	am_timeline_ompt_thread_layer_instantiate_type,
	am_timeline_ompt_parallel_layer_instantiate_type,
	am_timeline_ompt_task_create_layer_instantiate_type,
	am_timeline_ompt_task_schedule_layer_instantiate_type,
	am_timeline_ompt_implicit_task_layer_instantiate_type,
	am_timeline_ompt_sync_region_wait_layer_instantiate_type,
	am_timeline_ompt_mutex_released_layer_instantiate_type,
	am_timeline_ompt_dependences_layer_instantiate_type,
	am_timeline_ompt_task_dependence_layer_instantiate_type,
	am_timeline_ompt_work_layer_instantiate_type,
	am_timeline_ompt_master_layer_instantiate_type,
	am_timeline_ompt_sync_region_layer_instantiate_type,
	am_timeline_ompt_lock_init_layer_instantiate_type,
	am_timeline_ompt_lock_destroy_layer_instantiate_type,
	am_timeline_ompt_mutex_acquire_layer_instantiate_type,
	am_timeline_ompt_mutex_acquired_layer_instantiate_type,
	am_timeline_ompt_nest_lock_layer_instantiate_type,
	am_timeline_ompt_flush_layer_instantiate_type,
	am_timeline_ompt_cancel_layer_instantiate_type,
	am_timeline_state_layer_instantiate_type,
	am_timeline_selection_layer_instantiate_type,
	am_timeline_tensorflow_node_execution_layer_instantiate_type
};

int am_register_common_timeline_layer_types
(struct am_timeline_render_layer_type_registry* tr)
{
	struct am_timeline_render_layer_type* insts[AM_ARRAY_SIZE(inst_functions)];
	size_t i;

	for(i = 0; i < AM_ARRAY_SIZE(inst_functions); i++)
		if(!(insts[i] = inst_functions[i]()))
			goto out_err;

	for(i = 0; i < AM_ARRAY_SIZE(inst_functions); i++)
		am_timeline_render_layer_type_registry_add_type(tr, insts[i]);

	return 0;

out_err:
	for(size_t j = 0; j < i; j++) {
		am_timeline_render_layer_type_destroy(insts[j]);
		free(insts[j]);
	}

	return 1;
}
