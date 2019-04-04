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

#include <aftermath/render/timeline/layers/background.h>
#include <aftermath/render/timeline/renderer.h>
#include <stdlib.h>

/* Default colors: gray 0.5 and transparent */
static const struct am_timeline_background_layer_params BACKGROUND_LAYER_DEFAULT_PARAMS = {
	.colors = {
		.even = {0.5, 0.5, 0.5, 0.5},
		.odd = {0.0, 0.0, 0.0, 0.0}
	}
};

static void render(struct am_timeline_background_layer* bg, cairo_t* cr)
{
	struct am_timeline_renderer* r = bg->layer.renderer;
	struct am_rgba* colors[2];
	double y;
	double lane_mod;

	if(!r->first_lane.node)
		return;

	lane_mod = r->lane_offset - (r->num_invisible_lanes * r->lane_height);

	if(r->num_invisible_lanes % 2) {
		colors[0] = &bg->params.colors.odd;
		colors[1] = &bg->params.colors.even;
	} else {
		colors[0] = &bg->params.colors.even;
		colors[1] = &bg->params.colors.odd;
	}

	cairo_rectangle(cr, 0, 0, r->width, r->rects.lanes.height);
	cairo_clip(cr);

	for(unsigned int i = 0; i < r->num_visible_lanes; i++) {
		y = i * r->lane_height - lane_mod;
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(*colors[i % 2]));
		cairo_rectangle(cr, 0, y, r->width, r->lane_height);
		cairo_fill(cr);
	}

	cairo_reset_clip(cr);
}

static void destroy(struct am_timeline_background_layer* bg)
{
}

static struct am_timeline_render_layer*
instantiate(struct am_timeline_render_layer_type* t)
{
	struct am_timeline_background_layer* l;

	if(!(l = malloc(sizeof(*l))))
		return NULL;

	am_timeline_render_layer_init(&l->layer, t);

	l->params = BACKGROUND_LAYER_DEFAULT_PARAMS;

	return (struct am_timeline_render_layer*)l;
}

struct am_timeline_render_layer_type*
am_timeline_background_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	if(!(t = malloc(sizeof(*t))))
		return NULL;

	if(am_timeline_render_layer_type_init(t, "background")) {
		free(t);
		return NULL;
	}

	t->render = AM_TIMELINE_RENDER_LAYER_RENDER_FUN(render);
	t->destroy = AM_TIMELINE_RENDER_LAYER_DESTROY_FUN(destroy);
	t->instantiate = instantiate;

	return t;
}
