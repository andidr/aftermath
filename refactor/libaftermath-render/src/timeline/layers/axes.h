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

#ifndef AM_TIMELINE_AXES_LAYER_H
#define AM_TIMELINE_AXES_LAYER_H

#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/cairo_extras.h>

/* The axes layer renders vertical lines for the horizontal and vertical axis
 * and displays the labels for the horizontal axis. */

enum am_timeline_axes_layer_entity_type {
	AM_TIMELINE_AXES_LAYER_ENTITY_AXIS,
};

enum am_timeline_axes_layer_axis_type {
	AM_TIMELINE_AXES_LAYER_AXIS_TYPE_HORIZONTAL,
	AM_TIMELINE_AXES_LAYER_AXIS_TYPE_VERTICAL
};

struct am_timeline_axes_layer_axis {
	struct am_timeline_entity super;
	enum am_timeline_axes_layer_axis_type type;
};

struct am_timeline_axes_layer_tick_params {
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

struct am_timeline_axes_layer_axis_params {
	/* Color of the axis line */
	struct am_rgba color;

	/* Width in pixels of the axis line */
	double width;
};

struct am_timeline_axes_layer_params {
	struct {
		struct am_timeline_axes_layer_axis_params vertical;
		struct am_timeline_axes_layer_axis_params horizontal;
	} axes;

	struct am_timeline_axes_layer_tick_params major_ticks;
	struct am_timeline_axes_layer_tick_params minor_ticks;

	unsigned int min_minor_tick_distance;
};


struct am_timeline_axes_layer {
	struct am_timeline_render_layer super;
	struct am_timeline_axes_layer_params params;
};

struct am_timeline_render_layer_type*
am_timeline_axes_layer_instantiate_type(void);

#endif
