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

#ifndef AM_CAIRO_EXTRAS_H
#define AM_CAIRO_EXTRAS_H

#include <cairo.h>
#include <stddef.h>

void am_striped_rectangle(cairo_t* cr,
			  double rect_left, double rect_top,
			  double rect_width, double rect_height,
			  double* dash, size_t num_dash);

/* Structure for RGB color with an alpha channel. */
struct am_rgba {
	double r;
	double g;
	double b;
	double a;
};

/* Defines a static initializer for struct am_rgba from a set of RGB values
 * ranging from 0 to 255 without an a cast to struct am_rgba. */
#define AM_RGBA255_EL(r, g, b, a)	\
	{				\
		((double)r) / 255.0,	\
		((double)g) / 255.0,	\
		((double)b) / 255.0,	\
		((double)a) / 255.0	\
	}

#define AM_RGBA255(r, g, b, a)	\
	(struct am_rgba)AM_RGBA255_EL(r, g, b, a)

/* Builds four function arguments from an RGBA struct for a call of a cairo
 * function. */
#define AM_RGBA_ARGS(rgba) \
	(rgba).r, (rgba).g, (rgba).b, (rgba).a

/* Builds four function arguments from a pointer to an RGBA struct for a call of
 * a cairo function. */
#define AM_PRGBA_ARGS(rgba) \
	(rgba)->r, (rgba)->g, (rgba)->b, (rgba)->a

/* Structure for a 2D point */
struct am_point {
	double x;
	double y;
};

/* Builds two function arguments from a point struct for a call of a cairo
 * function. */
#define AM_POINT_ARGS(p) (p).x, (p).y

/* Builds two function arguments from a pointer to a point struct for a call of
 * a cairo function. */
#define AM_PPOINT_ARGS(p) (p)->x, (p)->y

/* Structure representing rectangles. */
struct am_rect {
	double x;
	double y;
	double width;
	double height;
};

/* Builds four function arguments from a rectangle struct for a call of a cairo
 * function. */
#define AM_RECT_ARGS(r) (r).x, (r).y, (r).width, (r).height

/* Builds four function arguments from a pointer to a rectangle struct for a
 * call of a cairo function. */
#define AM_PRECT_ARGS(r) (r)->x, (r)->y, (r)->width, (r)->height

#endif