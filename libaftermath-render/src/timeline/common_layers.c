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

static struct am_timeline_render_layer_type* (*inst_functions[])(void) = {
	am_timeline_axes_layer_instantiate_type,
	am_timeline_background_layer_instantiate_type,
	am_timeline_hierarchy_layer_instantiate_type,
	am_timeline_measurement_intervals_layer_instantiate_type,
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
