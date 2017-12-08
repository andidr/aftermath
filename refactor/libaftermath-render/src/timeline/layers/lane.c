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

#include <aftermath/render/timeline/layers/lane.h>
#include <aftermath/render/timeline/renderer.h>

static struct am_timeline_lane_render_layer*
instantiate(struct am_timeline_render_layer_type* t)
{
	struct am_timeline_lane_render_layer_type* tl;

	tl = AM_TIMELINE_LANE_RENDER_LAYER_TYPE(t);

	return tl->instantiate(tl);
}

/* Initialization function for a render layer. To be called from the
 * type-specific initialization function. */
void am_timeline_lane_render_layer_init(struct am_timeline_lane_render_layer* l,
					struct am_timeline_lane_render_layer_type* t)
{
	l->render_mode = AM_TIMELINE_LANE_RENDER_MODE_COMBINE_SUBTREE;
	am_timeline_render_layer_init(&l->super, &t->super);
}

void
destroy(struct am_timeline_lane_render_layer* l)
{
	struct am_timeline_lane_render_layer_type* tl;

	tl = AM_TIMELINE_LANE_RENDER_LAYER_TYPE(l->super.type);
	tl->destroy(l);
}

struct render_lane_data {
	struct am_timeline_lane_render_layer* layer;
	cairo_t* cr;
};

/* Callback function to render a single lane */
static void render_lane(struct am_timeline_renderer* r,
			struct am_hierarchy_node* n,
			unsigned int node_idx,
			unsigned int lane,
			struct render_lane_data* data)
{
	struct am_rect lane_rect;
	struct am_timeline_lane_render_layer_type* t =
		(struct am_timeline_lane_render_layer_type*)data->layer->super.type;

	if(am_timeline_renderer_lane_extents(r, &lane_rect, lane))
		return;

	if(!am_rectangle_intersect(&lane_rect, &r->rects.lanes))
		return;

	cairo_save(data->cr);
	cairo_rectangle(data->cr, AM_RECT_ARGS(lane_rect));
	cairo_clip(data->cr);
	cairo_translate(data->cr, lane_rect.x, lane_rect.y);

	t->render(data->layer,
		  n,
		  &r->visible_interval,
		  r->rects.lanes.width,
		  r->lane_height,
		  data->cr);

	cairo_restore(data->cr);
}

/* Invokes the rendering function for all visible lanes */
static void render(struct am_timeline_lane_render_layer* l, cairo_t* cr)
{
	struct am_timeline_renderer* r = l->super.renderer;
	struct render_lane_data data = {
		.layer = l,
		.cr = cr
	};

	am_timeline_renderer_foreach_visible_lane(
		r,
		(am_timeline_renderer_lane_fun_t)render_lane,
		&data);
}

int
am_timeline_lane_render_layer_type_init(
	struct am_timeline_lane_render_layer_type* l,
	const char* name)
{
	if(am_timeline_render_layer_type_init(&l->super, name))
		return 1;

	l->super.render = AM_TIMELINE_RENDER_LAYER_RENDER_FUN(render);
	l->super.destroy = AM_TIMELINE_RENDER_LAYER_DESTROY_FUN(destroy);
	l->super.instantiate = AM_TIMELINE_RENDER_LAYER_INSTANTIATE_FUN(instantiate);

	l->render = NULL;
	l->instantiate = NULL;
	l->destroy = NULL;

	return 0;
}
