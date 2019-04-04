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
#include <aftermath/core/parser.h>

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

/* Draws a triangle with a base parallel to the vertical axis of the specified
 * height and width. If width is positive, the triangle points to the right; if
 * negative, the triangle points to the left. The coordinates x and y indicate
 * the middle of the triangle's base. */
static inline void
am_triangle(cairo_t* cr, double x, double y, double width, double height)
{
	cairo_move_to(cr, x, y - height/2.0);
	cairo_line_to(cr, x, y + height/2.0);
	cairo_line_to(cr, x+width, y);
	cairo_line_to(cr, x, y - height/2.0);
}

/* Prints the string representation of rgba into out, using up to max_len
 * bytes. If the output was truncated, the function returns 1, otherwise 0. */
static inline int
am_rgba_to_string(const struct am_rgba* rgba, char* out, size_t max_len)
{
	if((size_t)snprintf(out, max_len, "rgba(%d,%d,%d,%.4f)",
			    (int)(rgba->r * 255.0),
			    (int)(rgba->g * 255.0),
			    (int)(rgba->b * 255.0),
			    rgba->a) >= max_len)
	{
		return 1;
	}

	return 0;
}

/* Allocates a new string of sufficient size and prints the string
 * representation of rgba into it. Returns a pointer to the newly allocated
 * string on success, otherwise NULL. */
static inline char* am_rgba_to_string_alloc(const struct am_rgba* rgba)
{
	/* Maximum lengths is for rgba(RRR,GGG,BBB,A.AAAA), which is 24 + 1
	 * bytes*/
	char* ret;

	if(!(ret = (char*)malloc(25)))
		return NULL;

	if(am_rgba_to_string(rgba, ret, 25)) {
		free(ret);
		return NULL;
	}

	return ret;
}

/* Initializes struct am_rgba from a string representation.
 *
 * Returns 0 on success, otherwise 1.
 */
static inline int am_rgba_from_string(struct am_rgba* rgba, const char* str)
{
	struct am_parser p;
	struct am_parser_token t;
	uint64_t uval_rgb[3];
	double dval_alpha;

	am_parser_init(&p, str, strlen(str));

	if(am_parser_read_next_identifier(&p, &t))
		return 1;

	if(!am_parser_token_equals_str(&t, "rgba"))
		return 1;

	if(am_parser_read_next_char(&p, &t, '('))
		return 1;

	for(size_t i = 0; i < 3; i++) {
		if(i != 0) {
			if(am_parser_read_next_char(&p, &t, ','))
				return 1;
		}

		am_parser_skip_ws(&p);

		if(am_parser_read_any_uint(&p, &t))
			return 1;

		if(am_safe_atou64n(t.str, t.len, &uval_rgb[i]))
			return 1;

		if(uval_rgb[i] > 255)
			return 1;
	}

	if(am_parser_read_next_char(&p, &t, ','))
		return 1;

	if(am_parser_read_next_double(&p, &t))
		return 1;

	if(am_safe_atodbln(t.str, t.len, &dval_alpha))
		return 1;

	if(dval_alpha > 1.0)
		return 1;

	if(am_parser_read_next_char(&p, &t, ')'))
		return 1;

	am_parser_skip_ws(&p);

	if(!am_parser_reached_end(&p))
		return 1;

	*rgba = AM_RGBA255(uval_rgb[0], uval_rgb[1], uval_rgb[2], 0);
	rgba->a = dval_alpha;

	return 0;
}

#endif
