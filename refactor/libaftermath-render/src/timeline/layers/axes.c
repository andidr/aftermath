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
#include <aftermath/render/cairo_extras.h>
#include <stdlib.h>
#include <math.h>
#include <aftermath/core/ansi_extras.h>

struct tick_params {
	/* Height of the tick line in pixels */
	double height;

	/* Width of the tick line in pixels */
	double width;

	/* Number of significant digits for the label */
	size_t significant_digits;

	/* Indicates whether a label should be drawn or not */
	int draw_label;

	/* Color of the tick line */
	struct am_rgba color;

	struct {
		/* Font family */
		char* family;

		/* Scaling factor for the font */
		double size;

		/* Top margin in pixels for labels */
		double top_margin;

		/* Color for labels */
		struct am_rgba color;

		/* Rotation in degrees */
		double rotation;
	} font;
};

struct axis_params {
	/* Color of the axis line */
	struct am_rgba color;

	/* Width in pixels of the axis line */
	double width;
};

struct axes_layer_params {
	struct {
		struct axis_params vertical;
		struct axis_params horizontal;
	} axes;

	struct tick_params major_ticks;
	struct tick_params minor_ticks;

	unsigned int min_minor_tick_distance;
};


struct axes_layer {
	struct am_timeline_render_layer super;
	struct axes_layer_params params;
};

/* Default colors: gray 0.5 and black */
static const struct axes_layer_params AXES_LAYER_DEFAULT_PARAMS = {
	.axes = {
		.vertical = {
			.color = { 0.8, 0.8, 0.0, 1.0 },
			.width = 1
		},

		.horizontal = {
			.color = { 0.8, 0.8, 0.0, 1.0 },
			.width = 1
		}
	},

	.major_ticks = {
		.height = 12,
		.width = 2,
		.significant_digits = 4,
		.color = { 0.8, 0.8, 0.0, 1.0 },
		.draw_label = 1,
		.font = {
			.size = 12,
			.family = NULL,
			.top_margin = 3,
			.color = { 0.8, 0.8, 0.0, 1.0 },
			.rotation = 0,
		}
	},

	.minor_ticks = {
		.height = 6,
		.width = 1,
		.significant_digits = 4,
		.color = { 0.8, 0.8, 0.0, 1.0 },
		.draw_label = 0,
		.font = {
			.size = 8,
			.family = NULL,
			.top_margin = 3,
			.color = { 0.8, 0.8, 0.0, 1.0 },
			.rotation = 0,
		}
	},

	.min_minor_tick_distance = 10
};

/* Calculates the distance in time units between two minor ticks based on the
 * axes layer's minimum required distance in pixels. */
static inline long double calculate_minor_tick_distance(struct axes_layer* ax)
{
	struct am_timeline_renderer* r = ax->super.renderer;
	struct am_time_offset width_ud;
	long double width_u;
	long double width_px;
	long double min_px;
	long double min_u;
	long double pow10;
	long double dist_u;

	width_px = r->rects.lanes.width;
	am_interval_duration(&r->visible_interval, &width_ud);
	width_u = width_ud.abs;

	min_px = ax->params.min_minor_tick_distance;
	min_u = (width_u * min_px) / width_px;

	pow10 = logl(min_u) / logl(10.0L);
	dist_u = powl(10.0L, roundl(pow10));

	return dist_u;
}

/* Draws a tick and its label at the correct position for a timestamp t using
 * the parameters of p. */
static void draw_tick(struct axes_layer* ax, cairo_t* cr, am_timestamp_t t,
		      struct tick_params* p)
{
	struct am_timeline_renderer* r = ax->super.renderer;
	cairo_text_extents_t extents;
	double tick_x;
	double tick_y1;
	double tick_y2;
	double label_origin_y;
	char buf[16];

	/* determine X position in pixels */
	tick_x = am_timeline_renderer_timestamp_to_x(r, t);

	/* Draw line */
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(p->color));
	cairo_set_line_width(cr, p->width);

	tick_y1 = r->rects.xlegend.y;
	tick_y2 = r->rects.xlegend.y + p->height;

	cairo_move_to(cr, tick_x, tick_y1);
	cairo_line_to(cr, tick_x, tick_y2);
	cairo_stroke(cr);

	if(!p->draw_label)
		return;

	/* Select label font */
	cairo_select_font_face(cr,
			       p->font.family,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size(cr, p->font.size);

	/* Generate label */
	am_siformat_u64(t, p->significant_digits, buf, AM_ARRAY_SIZE(buf));
	cairo_text_extents(cr, buf, &extents);

	/* Calculate origin around which the label will be rotated */
	label_origin_y = tick_y2 + extents.height / 2 + p->font.top_margin;

	cairo_set_source_rgba(cr, AM_RGBA_ARGS(p->font.color));

	cairo_save(cr);
		cairo_translate(cr, tick_x, label_origin_y);
		cairo_rotate(cr, (p->font.rotation / 360)*(2*M_PI));
		cairo_move_to(cr, -extents.width / 2, extents.height / 2);
		/* Show label */
		cairo_show_text(cr, buf);
	cairo_restore(cr);
}

static void render(struct axes_layer* ax, cairo_t* cr)
{
	struct am_timeline_renderer* r = ax->super.renderer;
	struct axis_params* vp = &ax->params.axes.vertical;
	struct axis_params* hp = &ax->params.axes.horizontal;
	struct am_rect* lr = &r->rects.lanes;
	struct am_time_offset dur;
	long double minor_time;
	long double major_time;
	long double vstart_time;
	long double start_time;

	/* Vertical line */
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(vp->color));
	cairo_set_line_width(cr, vp->width);

	cairo_move_to(cr, lr->x, lr->y);
	cairo_line_to(cr, lr->x, lr->y + lr->height);
	cairo_stroke(cr);

	/* Horizontal line */
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(hp->color));
	cairo_set_line_width(cr, hp->width);

	cairo_move_to(cr, lr->x, lr->y + lr->height);
	cairo_line_to(cr, lr->x + lr->width, lr->y + lr->height);
	cairo_stroke(cr);

	am_interval_duration(&r->visible_interval, &dur);

	if(dur.abs == 0)
		return;

	cairo_rectangle(cr, AM_RECT_ARGS(r->rects.xlegend));
	cairo_clip(cr);

	/* Calculate tick distances */
	minor_time = calculate_minor_tick_distance(ax);
	major_time = 10 * minor_time;

	vstart_time = r->visible_interval.start;
	start_time = floorl(vstart_time / major_time) * major_time;

	/* Major ticks */
	for(long double major_pos_u = start_time;
	    major_pos_u < r->visible_interval.end;
	    major_pos_u += major_time)
	{
		draw_tick(ax, cr, major_pos_u, &ax->params.major_ticks);

		/* Minor ticks */
		for(long double minor_pos_u = major_pos_u + minor_time;
		    minor_pos_u < major_pos_u + major_time &&
			    minor_pos_u < r->visible_interval.end;
		    minor_pos_u += minor_time)
		{
			draw_tick(ax, cr, minor_pos_u, &ax->params.minor_ticks);
		}
	}

	cairo_reset_clip(cr);
}

static void destroy(struct axes_layer* ax)
{
	free(ax->params.major_ticks.font.family);
	free(ax->params.minor_ticks.font.family);
}

static struct am_timeline_render_layer*
instantiate(struct am_timeline_render_layer_type* t)
{
	struct axes_layer* l;

	if(!(l = malloc(sizeof(*l))))
		goto out_err;

	am_timeline_render_layer_init(&l->super, t);

	l->params = AXES_LAYER_DEFAULT_PARAMS;

	if(!(l->params.major_ticks.font.family = strdup("Sans")))
		goto out_err_free;

	if(!(l->params.minor_ticks.font.family = strdup("Sans")))
		goto out_err_free_maj;

	return (struct am_timeline_render_layer*)l;

out_err_free_maj:
	free(l->params.major_ticks.font.family);
out_err_free:
	free(l);
out_err:
	return NULL;
}

/* Allocates, initializes and adds an axis entity to the list of entities
 * lst. Returns 0 on success, otherwise 1. */
static int add_axis_entity(struct axes_layer* ax,
			   struct list_head* lst,
			   enum am_timeline_axes_layer_axis_type type)
{
	struct am_timeline_axes_layer_axis* axis;

	if(!(axis = malloc(sizeof(*axis))))
		return 1;

	am_timeline_entity_init(&axis->super, &ax->super,
				AM_TIMELINE_AXES_LAYER_ENTITY_AXIS);

	axis->type = type;

	am_timeline_entity_append(&axis->super, lst);

	return 0;
}

static int identify_entities(struct axes_layer* ax,
			     struct list_head* lst,
			     double x, double y)
{
	struct am_timeline_renderer* r = ax->super.renderer;
	double hw_v = ax->params.axes.vertical.width / 2.0;
	double hw_h = ax->params.axes.horizontal.width / 2.0;

	/* On horizontal axis? */
	if(x >= r->rects.lanes.x &&
	   x <= r->rects.lanes.x + r->rects.lanes.width &&
	   y >= r->rects.lanes.y + r->rects.lanes.height - hw_h &&
	   y <= r->rects.lanes.y + r->rects.lanes.height + hw_h)
	{
		if(add_axis_entity(ax, lst,
				   AM_TIMELINE_AXES_LAYER_AXIS_TYPE_HORIZONTAL))
		{
			return 1;
		}
	}

	/* On vertical axis? */
	if(x >= r->rects.lanes.x - hw_v &&
	   x <= r->rects.lanes.x + hw_v &&
	   y >= r->rects.lanes.y &&
	   y <= r->rects.lanes.y + r->rects.lanes.height)
	{
		if(add_axis_entity(ax, lst,
				   AM_TIMELINE_AXES_LAYER_AXIS_TYPE_VERTICAL))
		{
			return 1;
		}
	}

	return 0;
}

static void destroy_entity(struct axes_layer* ax,
			   struct am_timeline_entity* e)
{
	am_timeline_entity_destroy(e);
	free(e);
}

struct am_timeline_render_layer_type*
am_timeline_axes_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	if(!(t = malloc(sizeof(*t))))
		return NULL;

	if(am_timeline_render_layer_type_init(t, "axes")) {
		free(t);
		return NULL;
	}

	t->render = AM_TIMELINE_RENDER_LAYER_RENDER_FUN(render);
	t->destroy = AM_TIMELINE_RENDER_LAYER_DESTROY_FUN(destroy);
	t->instantiate = AM_TIMELINE_RENDER_LAYER_INSTANTIATE_FUN(instantiate);
	t->identify_entities = AM_TIMELINE_RENDER_LAYER_IDENTIFY_ENTITIES_FUN(identify_entities);
	t->destroy_entity = AM_TIMELINE_RENDER_LAYER_DESTROY_ENTITY_FUN(destroy_entity);


	return t;
}
