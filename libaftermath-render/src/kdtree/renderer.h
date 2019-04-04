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

#ifndef AM_KDTREE_RENDERER_H
#define AM_KDTREE_RENDERER_H

#include <aftermath/core/indexes/kdtree.h>
#include <aftermath/render/cairo_extras.h>

struct am_kdtree_renderer_params {
	/* Background color of the whole graph */
	struct am_rgba bgcolor;

	/* Scaling factor of the current zoom when zooming in / out */
	double zoom_factor;
};

typedef void (*am_kdtree_renderer_render_node_callback)
	(cairo_t* cr, struct am_point screen_pos,
	 double zoom,
	 const struct am_kdtree_node* n,
	 void* data);

struct am_kdtree_renderer {
	/* Parameters for rendering */
	struct am_kdtree_renderer_params params;

	/* Width in pixels of the visible portion of the graph */
	unsigned int width;

	/* Height in pixels of the visible portion of the graph */
	unsigned int height;

	/* The k-d-tree to be rendered */
	const struct am_kdtree* kdtree;

	/* callback function to render a single node */
	am_kdtree_renderer_render_node_callback render_node_cb;

	/* Current zoom (scaling factor) */
	double zoom;

	/* Scrolling offset in graph coordinates */
	struct am_point offset;

	/* Data pointer passed to render_node_cb */
	void* cb_data;
};

void am_kdtree_renderer_init(
	struct am_kdtree_renderer* r,
	am_kdtree_renderer_render_node_callback render_node_cb,
	void* cb_data);
void am_kdtree_renderer_destroy(struct am_kdtree_renderer* r);
void am_kdtree_renderer_set_kdtree(struct am_kdtree_renderer* r,
				   const struct am_kdtree* t);
void am_kdtree_renderer_zoom_in(struct am_kdtree_renderer* r,
				const struct am_point* pos);
void am_kdtree_renderer_zoom_out(struct am_kdtree_renderer* r,
				 const struct am_point* pos);
void am_kdtree_renderer_render(struct am_kdtree_renderer* r, cairo_t* cr);

/* Sets the width in pixels of the renderer. */
static inline void am_kdtree_renderer_set_width(struct am_kdtree_renderer* r,
						unsigned int w)
{
	r->width = w;
}

/* Sets the height in pixels of the renderer. */
static inline void
am_kdtree_renderer_set_height(struct am_kdtree_renderer* r, unsigned int h)
{
	r->height = h;
}

/* Translates a width of w pixels to a width in graph units */
static inline double
am_kdtree_renderer_screen_w_to_graph(
	const struct am_kdtree_renderer* r, double w)
{
	return w / r->zoom;
}

/* Translates a height of h pixels to a height in graph units */
static inline double
am_kdtree_renderer_screen_h_to_graph(
	const struct am_kdtree_renderer* r, double h)
{
	return h / r->zoom;
}

/* Translates a width of w graph units to a width in pixels */
static inline double
am_kdtree_renderer_graph_w_to_screen(
	const struct am_kdtree_renderer* r, double w)
{
	return w * r->zoom;
}

/* Translates a height of h graph units to a height in pixels */
static inline double
am_kdtree_renderer_graph_h_to_screen(
	const struct am_kdtree_renderer* r, double h)
{
	return h * r->zoom;
}

/* Translates an x pixel coordinate to a graph coordinate */
static inline double
am_kdtree_renderer_screen_x_to_graph(
	const struct am_kdtree_renderer* r, double x)
{
	return r->offset.x + x / r->zoom;
}

/* Translates an x graph coordinate to a pixel coordinate */
static inline double
am_kdtree_renderer_graph_x_to_screen(
	const struct am_kdtree_renderer* r, double x)
{
	return (x - r->offset.x) * r->zoom;
}

/* Translates a y pixel coordinate to a graph coordinate */
static inline double
am_kdtree_renderer_screen_y_to_graph(
	const struct am_kdtree_renderer* r, double y)
{
	return r->offset.y + y / r->zoom;
}

/* Translates a y graph coordinate to a pixel coordinate */
static inline double
am_kdtree_renderer_graph_y_to_screen(
	const struct am_kdtree_renderer* r, double y)
{
	return (y - r->offset.y) * r->zoom;
}

/* Translates a point in graph coordinates to screen coordinates */
static inline void
am_kdtree_renderer_graph_to_screen(const struct am_kdtree_renderer* r,
				const struct am_point* graph_pos,
				struct am_point* screen_pos)
{
	screen_pos->x = am_kdtree_renderer_graph_x_to_screen(r, graph_pos->x);
	screen_pos->y = am_kdtree_renderer_graph_y_to_screen(r, graph_pos->y);
}

/* Translates a point in screen coordinates to graph coordinates */
static inline void
am_kdtree_renderer_screen_to_graph(const struct am_kdtree_renderer* r,
				const struct am_point* screen_pos,
				struct am_point* graph_pos)
{
	graph_pos->x = am_kdtree_renderer_screen_x_to_graph(r, screen_pos->x);
	graph_pos->y = am_kdtree_renderer_screen_y_to_graph(r, screen_pos->y);
}

/* Returns the center in screen coordinates of the renderer in *center. */
static inline void am_kdtree_renderer_screen_center(
	const struct am_kdtree_renderer* r,
	struct am_point* center)
{
	center->x = r->width / 2;
	center->y = r->height / 2;
}

/* Sets the offset in graph units of the upper left corner relative to the
 * origin of the graph. */
static inline void
am_kdtree_renderer_set_offset(struct am_kdtree_renderer* r, double x, double y)
{
	r->offset.x = x;
	r->offset.y = y;
}

/* Returns the offset in graph units of the upper left corner relative to the
 * origin of the graph in *x and *y. */
static inline void
am_kdtree_renderer_get_offset(const struct am_kdtree_renderer* r,
			   double* x, double* y)
{
	*x = r->offset.x;
	*y = r->offset.y;
}

#endif
