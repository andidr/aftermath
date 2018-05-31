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

#include <aftermath/render/timeline/layers/axes.h>
#include <aftermath/render/timeline/renderer.h>
#include <aftermath/core/measurement_interval_array.h>

struct measurement_intervals_layer_params {
	struct {
		/* Width of the start line in pixels */
		double line_width;

		struct {
			/* Width of the start triangle in pixels */
			double width;

			/* Height of the start triangle in pixels */
			double height;
		} triangle;

		/* Color of the start line */
		struct am_rgba line_color;

		/* Color of the start triangle */
		struct am_rgba triangle_color;
	} start;

	struct {
		/* Width of the end line in pixels */
		double line_width;

		struct {
			/* Width of the end triangle in pixels */
			double width;

			/* Height of the end triangle in pixels */
			double height;
		} triangle;

		/* Color of the end line */
		struct am_rgba line_color;

		/* Color of the end triangle */
		struct am_rgba triangle_color;
	} end;
};

struct measurement_intervals_layer {
	struct am_timeline_render_layer super;
	struct measurement_intervals_layer_params params;
};


static const struct measurement_intervals_layer_params
MEASUREMENT_INTERVALS_LAYER_DEFAULT_PARAMS = {
	.start = {
		.line_width = 3,
		.triangle = {
			.width = 25,
			.height = 15
		},
		.line_color = { 0.0, 0.8, 0.0, 1.0 },
		.triangle_color = { 0.0, 0.8, 0.0, 1.0 }
	},

	.end = {
		.line_width = 3,
		.triangle = {
			.width = 25,
			.height = 15
		},
		.line_color = { 0.8, 0.0, 0.0, 1.0 },
		.triangle_color = { 0.8, 0.0, 0.0, 1.0 }
	}
};

static void render(struct measurement_intervals_layer* mil, cairo_t* cr)
{
	struct am_timeline_renderer* r = mil->super.renderer;
	struct am_trace* t = r->trace;
	struct am_measurement_interval_array* mia;
	struct am_measurement_interval* mi;
	struct am_interval query_interval;
	struct am_time_offset triangle_duration;
	struct measurement_intervals_layer_params* p = &mil->params;
	double screen_x;
	double screen_top;
	double screen_bottom;
	double screen_start_ty;
	double screen_end_ty;

	if(!t)
		return;

	if(!(mia = am_trace_find_trace_array(t, "am::generic::measurement_interval")))
		return;

	cairo_rectangle(cr, AM_RECT_ARGS(r->rects.lanes));
	cairo_clip(cr);

	/* The query interval that we check for overlaps with measurement
	 * intervals must be a bit larger than the visible interval, since we
	 * want to be sure to render the triangles overlapping into the visible
	 * interval of measurement intervals which start right before / end
	 * right after the visible interval. */
	query_interval = r->visible_interval;

	if(am_timeline_renderer_width_to_duration(r, p->start.triangle.width,
						  &triangle_duration) !=
	   AM_ARITHMETIC_STATUS_EXACT)
	{
		triangle_duration.sign = 0;
		triangle_duration.abs = 0;
	}

	am_interval_widen_start_u(&query_interval, triangle_duration.abs);

	if(am_timeline_renderer_width_to_duration(r, p->end.triangle.width,
						  &triangle_duration) !=
	   AM_ARITHMETIC_STATUS_EXACT)
	{
		triangle_duration.sign = 0;
		triangle_duration.abs = 0;
	}

	am_interval_widen_end_u(&query_interval, triangle_duration.abs);

	mi = am_measurement_interval_array_bsearch_first_overlapping(
		mia, &query_interval);

	screen_top = r->rects.lanes.y;
	screen_bottom = r->rects.lanes.y + r->rects.lanes.height;
	screen_start_ty = screen_top + p->start.triangle.height / 2.0;
	screen_end_ty = screen_top + p->end.triangle.height / 2.0;

	while(am_measurement_interval_array_is_element_ptr(mia, mi) &&
	      mi->interval.start < query_interval.end)
	{
		/* Start line */
		screen_x = am_timeline_renderer_timestamp_to_x(
			r, mi->interval.start);

		cairo_move_to(cr, screen_x, screen_top);
		cairo_line_to(cr, screen_x, screen_bottom);

		cairo_set_line_width(cr, p->start.line_width);
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(p->start.line_color));
		cairo_stroke(cr);

		/* Start triangle */
		am_triangle(cr, screen_x, screen_start_ty,
			    p->start.triangle.width,
			    p->start.triangle.height);
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(p->start.line_color));
		cairo_fill(cr);

		/* End line */
		screen_x = am_timeline_renderer_timestamp_to_x(
			r, mi->interval.end);

		cairo_set_line_width(cr, p->end.line_width);
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(p->end.line_color));

		cairo_move_to(cr, screen_x, screen_top);
		cairo_line_to(cr, screen_x, screen_bottom);

		cairo_stroke(cr);

		/* End triangle */
		am_triangle(cr, screen_x, screen_end_ty,
			    -p->end.triangle.width,
			    p->end.triangle.height);
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(p->end.line_color));
		cairo_fill(cr);

		mi++;
	}

	cairo_reset_clip(cr);
}

static void destroy(struct measurement_intervals_layer* l)
{
}

static struct am_timeline_render_layer*
instantiate(struct am_timeline_render_layer_type* t)
{
	struct measurement_intervals_layer* l;

	if(!(l = malloc(sizeof(*l))))
		return NULL;

	am_timeline_render_layer_init(&l->super, t);

	l->params = MEASUREMENT_INTERVALS_LAYER_DEFAULT_PARAMS;

	return (struct am_timeline_render_layer*)l;
}

struct am_timeline_render_layer_type*
am_timeline_measurement_intervals_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	if(!(t = malloc(sizeof(*t))))
		return NULL;

	if(am_timeline_render_layer_type_init(t, "measurement_intervals")) {
		free(t);
		return NULL;
	}

	t->render = AM_TIMELINE_RENDER_LAYER_RENDER_FUN(render);
	t->destroy = AM_TIMELINE_RENDER_LAYER_DESTROY_FUN(destroy);
	t->instantiate = instantiate;

	return t;
}
