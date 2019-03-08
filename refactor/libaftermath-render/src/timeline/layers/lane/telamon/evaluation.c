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

#include "evaluation.h"
#include <aftermath/core/telamon_evaluation_array.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/renderer.h>

struct am_color_map telamon_evaluation_colors = AM_STATIC_COLOR_MAP({
		AM_RGBA255_EL(117, 195, 255, 255)
	});

static int telamon_evaluation_renderer_trace_changed(
	struct am_timeline_render_layer* l,
	struct am_trace* t)
{
	struct am_timeline_interval_layer* il = (typeof(il))l;

	am_timeline_interval_layer_set_extra_data(il, NULL);

	am_timeline_interval_layer_set_color_map(AM_TIMELINE_INTERVAL_LAYER(l),
						 &telamon_evaluation_colors);

	/* Currently, only one color is used to indicate that an evaluation
	 * takes place */
	return am_timeline_interval_layer_set_max_index(
		AM_TIMELINE_INTERVAL_LAYER(l), 0);
}

static int telamon_evaluation_renderer_renderer_changed(
	struct am_timeline_render_layer* l,
	struct am_timeline_renderer* r)
{
	if(r->trace)
		return telamon_evaluation_renderer_trace_changed(l, r->trace);
	else
		return 0;
}

static size_t
am_timeline_telamon_evaluation_layer_calculate_index(
	struct am_timeline_interval_layer* l,
	void* arg)
{
	/* Currently, only one color is used to indicate that an evaluation
	 * takes place */
	return 0;
}

struct am_timeline_render_layer_type*
am_timeline_telamon_evaluation_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	t = am_timeline_interval_layer_instantiate_type_index_fun(
		"telamon::evaluation",
		"am::telamon::evaluation",
		sizeof(struct am_telamon_evaluation),
		offsetof(struct am_telamon_evaluation, interval),
		am_timeline_telamon_evaluation_layer_calculate_index);

	t->trace_changed = telamon_evaluation_renderer_trace_changed;
	t->renderer_changed = telamon_evaluation_renderer_renderer_changed;

	return t;
}
