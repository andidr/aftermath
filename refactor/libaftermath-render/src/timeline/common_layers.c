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
#include <aftermath/render/timeline/renderer.h>
#include <aftermath/core/ansi_extras.h>
#include <aftermath/core/trace.h>
#include <aftermath/core/state_description_array.h>

struct am_color_map state_colors = AM_STATIC_COLOR_MAP({
		AM_RGBA255_EL(117, 195, 255, 255),
		AM_RGBA255_EL(  0,   0, 255, 255),
		AM_RGBA255_EL(255, 255, 255, 255),
		AM_RGBA255_EL(255,   0,   0, 255),
		AM_RGBA255_EL(255,   0, 174, 255),
		AM_RGBA255_EL(179,   0,   0, 255),
		AM_RGBA255_EL(  0, 255,   0, 255),
		AM_RGBA255_EL(255, 255,   0, 255),
		AM_RGBA255_EL(235,   0,   0, 255)
	});

static int state_renderer_trace_changed(struct am_timeline_render_layer* l,
					struct am_trace* t)
{
	struct am_state_description_array* sda;

	if(!t || !(sda = am_trace_find_trace_array(
			   t, "am::generic::state_description")))
	{
		return 0;
	}

	am_timeline_interval_layer_set_color_map(AM_TIMELINE_INTERVAL_LAYER(l),
						 &state_colors);

	if(sda->num_elements > 0) {
		return am_timeline_interval_layer_set_max_index(
			AM_TIMELINE_INTERVAL_LAYER(l),
			sda->num_elements-1);
	}

	return 0;
}

static int state_renderer_renderer_changed(struct am_timeline_render_layer* l,
					   struct am_timeline_renderer* r)
{
	if(r->trace)
		return state_renderer_trace_changed(l, r->trace);
	else
		return 0;
}

static struct am_timeline_render_layer_type*
am_timeline_state_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type(
		"state",
		"am::generic::state",
		sizeof(struct am_state_event),
		offsetof(struct am_state_event, interval),
		offsetof(struct am_state_event, state_idx),
		AM_SIZEOF_BITS(((struct am_state_event*)NULL)->state_idx));

	t->trace_changed = state_renderer_trace_changed;
	t->renderer_changed = state_renderer_renderer_changed;

	return t;
}

static struct am_timeline_render_layer_type* (*inst_functions[])(void) = {
	am_timeline_axes_layer_instantiate_type,
	am_timeline_background_layer_instantiate_type,
	am_timeline_hierarchy_layer_instantiate_type,
	am_timeline_measurement_intervals_layer_instantiate_type,
	am_timeline_state_layer_instantiate_type
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
