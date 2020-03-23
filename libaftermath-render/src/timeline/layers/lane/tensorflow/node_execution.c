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

#include "node_execution.h"
#include <aftermath/core/tensorflow_node_array.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/renderer.h>

struct am_color_map tensorflow_node_colors = AM_STATIC_COLOR_MAP({
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

static int tensorflow_node_execution_renderer_trace_changed(
	struct am_timeline_render_layer* l,
	struct am_trace* t)
{
	struct am_tensorflow_node_array* na = NULL;
	struct am_timeline_interval_layer* il = (typeof(il))l;
	size_t max_index = 0;

	if(t && (na = am_trace_find_trace_array(t, "am::tensorflow::node")))
		max_index = na->num_elements-1;

	am_timeline_interval_layer_set_extra_data(il, na);

	am_timeline_interval_layer_set_color_map(AM_TIMELINE_INTERVAL_LAYER(l),
						 &tensorflow_node_colors);

	return am_timeline_interval_layer_set_max_index(
		AM_TIMELINE_INTERVAL_LAYER(l),
		max_index);
}

static int tensorflow_node_execution_renderer_renderer_changed(
	struct am_timeline_render_layer* l,
	struct am_timeline_renderer* r)
{
	if(r->trace)
		return tensorflow_node_execution_renderer_trace_changed(l, r->trace);
	else
		return 0;
}

static size_t
am_timeline_tensorflow_node_execution_layer_calculate_index(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	struct am_tensorflow_node_execution* ne = arg;
	struct am_tensorflow_node_array* narr;

	narr = am_timeline_interval_layer_get_extra_data(l);

	return am_tensorflow_node_array_index(narr, ne->node);
}

struct am_timeline_render_layer_type*
am_timeline_tensorflow_node_execution_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"tensorflow::node_execution",
		"am::tensorflow::node_execution",
		sizeof(struct am_tensorflow_node_execution),
		offsetof(struct am_tensorflow_node_execution, interval),
		am_timeline_tensorflow_node_execution_layer_calculate_index);

	t->trace_changed = tensorflow_node_execution_renderer_trace_changed;
	t->renderer_changed = tensorflow_node_execution_renderer_renderer_changed;

	return t;
}
