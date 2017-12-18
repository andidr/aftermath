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
#include <math.h>
#include <aftermath/core/ansi_extras.h>

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

/* Calculate the absolute euclidean distance between two points p1 and p2. */
static inline double
am_point_distance(const struct am_point* p1,
		  const struct am_point* p2)
{
	double xdiff = p1->x - p2->x;
	double ydiff = p1->y - p2->y;

	return sqrt(xdiff*xdiff + ydiff*ydiff);
}

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

/* Stores a rectangle describing the intersection of r and b in r if b and r
 * overlap. Returns 1 if the rectangles overlap, otherwise 0. */
static inline int
am_rectangle_intersect(struct am_rect* r,
		       const struct am_rect* b)
{
	double x1;
	double x2;
	double y1;
	double y2;

	if(b->x <= r->x + r->width && b->x + b->width >= r->x &&
	   b->y <= r->y + r->height && b->y + b->height >= r->y)
	{
		x1 = am_max_double(b->x, r->x);
		x2 = am_min_double(b->x + b->width, r->x + r->width);

		y1 = am_max_double(b->y, r->y);
		y2 = am_min_double(b->y + b->height, r->y + r->height);

		r->x = x1;
		r->width = x2 - x1;
		r->y = y1;
		r->height = y2 - y1;

		return 1;
	}

	return 0;
}

/* Returns true if the point p lies within the rect r (including values, where p
 * lies on the borders of r). Otherwise, the function returns false. */
static inline int
am_point_in_rect(const struct am_point* p,
		 const struct am_rect* r)
{
	return p->x >= r->x && p->x <= r->x + r->width &&
		p->y >= r->y && p->y <= r->y + r->height;
}

#define AM_ROUNDED_CORNER_UPPER_LEFT  (1 << 0)
#define AM_ROUNDED_CORNER_UPPER_RIGHT (1 << 1)
#define AM_ROUNDED_CORNER_LOWER_LEFT  (1 << 2)
#define AM_ROUNDED_CORNER_LOWER_RIGHT (1 << 3)

#define AM_ROUNDED_CORNERS_TOP \
	(AM_ROUNDED_CORNER_UPPER_LEFT | AM_ROUNDED_CORNER_UPPER_RIGHT)

#define AM_ROUNDED_CORNERS_BOTOM \
	(AM_ROUNDED_CORNER_LOWER_LEFT | AM_ROUNDED_CORNER_LOWER_RIGHT)

#define AM_ROUNDED_CORNERS_RIGHT \
	(AM_ROUNDED_CORNER_UPPER_RIGHT | AM_ROUNDED_CORNER_LOWER_RIGHT)

#define AM_ROUNDED_CORNERS_LEFT \
	(AM_ROUNDED_CORNER_UPPER_LEFT | AM_ROUNDED_CORNER_LOWER_LEFT)

#define AM_ROUNDED_CORNERS_ALL		 \
	(AM_ROUNDED_CORNER_UPPER_LEFT  | \
	 AM_ROUNDED_CORNER_UPPER_RIGHT | \
	 AM_ROUNDED_CORNER_LOWER_LEFT  | \
	 AM_ROUNDED_CORNER_LOWER_RIGHT)

void am_rounded_rectangle_corners(cairo_t* cr,
				  const struct am_rect* rect,
				  double r,
				  long corner_flags);

void am_rounded_rectangle(cairo_t* cr, const struct am_rect* rect, double r);

cairo_bool_t am_point_on_curve(cairo_t* cr,
			       const struct am_point* p,
			       const struct am_point* p1,
			       const struct am_point* c1,
			       const struct am_point* c2,
			       const struct am_point* p2,
			       double line_width);

#endif
