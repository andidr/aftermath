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

#ifndef AM_DFG_RENDERER_H
#define AM_DFG_RENDERER_H

#include <cairo.h>
#include <aftermath/core/dfg_graph.h>
#include <aftermath/core/object_notation.h>
#include <aftermath/render/cairo_extras.h>
#include <aftermath/render/dfg/dfg_coordinate_mapping.h>
#include <float.h>

/* FIXME:
 * - Integer arithmetic for zoom etc */

struct am_dfg_renderer_params {
	/* Background color of the whole graph */
	struct am_rgba bgcolor;

	/* Scaling factor of the current zoom when zooming in / out */
	double zoom_factor;

	/* Parameters for nodes */
	struct {
		/* Minimal height in pixels */
		double min_h;

		/* Minimal width in pixels */
		double min_w;

		/* Horizontal padding around a node  */
		double pad_x;

		/* Vertical padding around a node  */
		double pad_y;

		/* Radius of the rounded corners */
		double corner_radius;

		struct {
			/* Colors of the frame around the rectangle */
			struct {
				/* Normal, non-highlighted, well-connected node */
				struct am_rgba normal;

				/* Color of the frame around a highlighted node */
				struct am_rgba highlighted;

				/* Color of the frame around a node that isn't well connected */
				struct am_rgba error;
			} stroke;

			struct {
				/* Background color of a node */
				struct am_rgba normal;

				/* Background color of a highlighted node */
				struct am_rgba highlighted;

				/* Background color of a selected node */
				struct am_rgba selected;
			} bg;
		} color;
	} main_rects;

	/* Parameters for ports */
	struct {
		/* Height of a port in pixels (including rectangle) */
		double height;

		/* Parameters for port labels */
		struct {
			/* Name of the font */
			const char* font;

			/* Scaling factor for the font size */
			double font_size;

			/* Horizontal padding for port labels */
			double pad_x;
		} label;

		/* Parameters for port rectangles */
		struct {
			/* Height in pixels */
			double height;

			/* Width in pixels */
			double width;

			/* Radius of the rounded corners */
			double corner_radius;

			/* Colors of port rectangles */
			struct {
				/* Input ports */
				struct {
					struct am_rgba normal;
					struct am_rgba highlighted;
				} in;

				/* Output ports */
				struct {
					struct am_rgba normal;
					struct am_rgba highlighted;
				} out;
			} colors;
		} rect;
	} ports;

	/* Parameter for properties */
	struct {
		/* Name of the font */
		const char* font;

		/* Scaling factor for the font size */
		double font_size;

		/* Font color */
		struct am_rgba color;
	} properties;

	/* Parameters for the names at the top of a node */
	struct {
		struct {
			/* Name of the font */
			const char* font;

			/* Scaling factor for the font size */
			double font_size;

			/* Horizontal padding */
			double pad_x;

			/* Vertical padding */
			double pad_y;

			struct {
				struct am_rgba normal;
				struct am_rgba highlighted;
			} color;
		} label;

		/* Rounded rectangle sourrounding a node's name */
		struct {
			/* Radius of rounded corners in pixels */
			double corner_radius;

			/* Colors */
			struct {
				/* Color of the line around the rectangle */
				struct {
					/* Color for a non-highlighted node */
					struct am_rgba normal;

					/* Color for a highlighted node */
					struct am_rgba highlighted;
				} stroke;

				/* Background color of the rectangle */
				struct {
					/* Color for a non-highlighted node */
					struct am_rgba normal;

					/* Color for a highlighted node */
					struct am_rgba highlighted;
				} bg;
			} color;
		} rect;
	} names;

	/* Parameters for curves representing the connections */
	struct {
		/* Normal, non-highlighted, non-selected connection */
		struct {
			struct am_rgba color;
			double width;
		} normal;

		/* Highlighted connection */
		struct {
			struct am_rgba color;
			double width;
		} highlighted;

		/* Selected connection */
		struct {
			struct am_rgba color;
			double width;
		} selected;

		/* Floating connection */
		struct {
			struct am_rgba color;
			double width;
		} floating;

		/* Erroneous connection (e.g., cycle) */
		struct {
			struct am_rgba color;
			double width;
		} error;

		/* Distance of control points of the connection bézier curve
		 * from the edge of a port rectangle */
		double ctrl_distance;

		/* Number of pixels around a connection that still account for
		 * the connection when testing for a collision with a point */
		double collision_px;
	} connections;
};

struct am_dfg_renderer {
	/* Width in pixels of the visible portion of the graph */
	unsigned int width;

	/* Height in pixels of the visible portion of the graph */
	unsigned int height;

	struct am_dfg_renderer_params params;

	/* Current zoom */
	double zoom;

	/* Rendering offset in graph units */
	struct am_point offset;

	struct am_dfg_node* selected_node;
	struct am_dfg_node* highlighted_node;
	struct am_dfg_port* highlighted_port;
	struct am_dfg_connection selected_connection;
	struct am_dfg_connection highlighted_connection;
	struct am_dfg_path* error_cycle;

	/* Connection that is not drawn even though it exists in the graph */
	struct am_dfg_connection ignore_connection;

	/* The floating connection is a virtual connection that does not exist
	 * in the graph, but that is added to the rendered image (e.g., a
	 * connection that is about to be added to the graph). */
	struct {
		int show;
		struct am_point src;
		struct am_point dst;
	} floating_connection;

	/* Current graph that is to be rendered */
	const struct am_dfg_graph* graph;

	/* Position of each node */
	const struct am_dfg_coordinate_mapping* coordinate_mapping;
};

void am_dfg_renderer_init(struct am_dfg_renderer* r);
void am_dfg_renderer_destroy(struct am_dfg_renderer* r);
void am_dfg_renderer_set_graph(struct am_dfg_renderer* r, const struct am_dfg_graph* g);
void am_dfg_renderer_set_coordinate_mapping(
	struct am_dfg_renderer* r,
	const struct am_dfg_coordinate_mapping* m);
void am_dfg_renderer_render(struct am_dfg_renderer* r, cairo_t* cr);

/* Sets the width in pixels of the renderer. */
static inline void am_dfg_renderer_set_width(struct am_dfg_renderer* r,
					     unsigned int w)
{
	r->width = w;
}

/* Sets the height in pixels of the renderer. */
static inline void am_dfg_renderer_set_height(struct am_dfg_renderer* r,
					      unsigned int h)
{
	r->height = h;
}

struct am_dfg_node* am_dfg_renderer_node_at(struct am_dfg_renderer* r,
					    cairo_t* cr,
					    double x,
					    double y);

struct am_dfg_port* am_dfg_renderer_port_at(struct am_dfg_renderer* r,
					    cairo_t* cr,
					    double x,
					    double y);

int am_dfg_renderer_connection_at(struct am_dfg_renderer* r,
				  cairo_t* cr,
				  double x,
				  double y,
				  struct am_dfg_port** p0,
				  struct am_dfg_port** p1);

void am_dfg_renderer_port_connection_point_xy(struct am_dfg_renderer* r,
					      cairo_t* cr,
					      const struct am_dfg_port* p,
					      struct am_point* point);

/* Sets the current floating connection */
static inline void
am_dfg_renderer_set_floating_connection(struct am_dfg_renderer* r,
					double xsrc, double ysrc,
					double xdst, double ydst)
{
	r->floating_connection.src.x = xsrc;
	r->floating_connection.src.y = ysrc;
	r->floating_connection.dst.x = xdst;
	r->floating_connection.dst.y = ydst;
	r->floating_connection.show = 1;
}

/* Passes the coordinates of the current floating connection in *xsrc, *ysrc,
 * *xdst and *ydst and returns 0. If there is no floating connection, the
 * function does not modify the output parameters and returns 1.*/
static inline int
am_dfg_renderer_get_floating_connection(struct am_dfg_renderer* r,
					double* xsrc, double* ysrc,
					double* xdst, double* ydst)
{
	if(r->floating_connection.show) {
		*xsrc = r->floating_connection.src.x;
		*ysrc = r->floating_connection.src.x;
		*xdst = r->floating_connection.dst.x;
		*ydst = r->floating_connection.dst.y;
	}

	return r->floating_connection.show;
}

/* Disables rendering of the current floating connection */
static inline void
am_dfg_renderer_unset_floating_connection(struct am_dfg_renderer* r)
{
	r->floating_connection.show = 0;
}

/* Checks if the connection from p0 to p1 is the ignored connection. */
static inline int am_dfg_renderer_connection_ignored(struct am_dfg_renderer* r,
						     struct am_dfg_port* p0,
						     struct am_dfg_port* p1)
{
	struct am_dfg_connection conn = { .src = p0, .dst = p1 };
	return am_dfg_connection_eq_nd(&conn, &r->ignore_connection);
}

/* Sets the current ignored connection (connection that is excluded from
 * rendering). */
static inline void
am_dfg_renderer_set_ignore_connection(struct am_dfg_renderer* r,
				      struct am_dfg_port* p0,
				      struct am_dfg_port* p1)
{
	r->ignore_connection.src = p0;
	r->ignore_connection.dst = p1;
}

/* Unsets the current ignored connection (connection that is excluded from
 * rendering) */
static inline void
am_dfg_renderer_unset_ignore_connection(struct am_dfg_renderer* r)
{
	r->ignore_connection.src = NULL;
	r->ignore_connection.dst = NULL;
}

/* Sets the connection to be highlighted. */
static inline void
am_dfg_renderer_set_highlighted_connection(struct am_dfg_renderer* r,
					   struct am_dfg_port* p0,
					   struct am_dfg_port* p1)
{
	r->highlighted_connection.src = p0;
	r->highlighted_connection.dst = p1;
}

/* Returns true if there is a highlighted connection */
static inline int
am_dfg_renderer_has_highlighted_connection(struct am_dfg_renderer* r)
{
	return (r->highlighted_connection.src && r->highlighted_connection.dst);
}

/* Disables the highlighted connection. */
static inline void
am_dfg_renderer_unset_highlighted_connection(struct am_dfg_renderer* r)
{
	r->highlighted_connection.src = NULL;
	r->highlighted_connection.dst = NULL;
}

/* Sets the port to be highlighted. */
static inline void
am_dfg_renderer_set_highlighted_port(struct am_dfg_renderer* r,
				     struct am_dfg_port* p)
{
	r->highlighted_port = p;
}

/* Disables the highlighted port. */
static inline void
am_dfg_renderer_unset_highlighted_port(struct am_dfg_renderer* r)
{
	r->highlighted_port = NULL;
}

/* Returns true if there is a selected connection */
static inline int
am_dfg_renderer_has_selected_connection(struct am_dfg_renderer* r)
{
	return (r->selected_connection.src && r->selected_connection.dst);
}

/* Sets the selected connection. */
static inline void
am_dfg_renderer_set_selected_connection(struct am_dfg_renderer* r,
					struct am_dfg_port* p0,
					struct am_dfg_port* p1)
{
	r->selected_connection.src = p0;
	r->selected_connection.dst = p1;
}

/* Returns the selected connection in p0 and p1. If no connection is selected,
 * p0 and p1 are set to NULL. */
static inline void
am_dfg_renderer_get_selected_connection(struct am_dfg_renderer* r,
					struct am_dfg_port** p0,
					struct am_dfg_port** p1)
{
	*p0 = r->selected_connection.src;
	*p1 = r->selected_connection.dst;
}

/* Unsets the selected connection */
static inline void
am_dfg_renderer_unset_selected_connection(struct am_dfg_renderer* r)
{
	r->selected_connection.src = NULL;
	r->selected_connection.dst = NULL;
}

/* Returns the currently selected node or NULL if no node is selected. */
static inline struct am_dfg_node*
am_dfg_renderer_get_selected_node(struct am_dfg_renderer* r)
{
	return r->selected_node;
}

/* Sets the currently selected node. */
static inline void
am_dfg_renderer_set_selected_node(struct am_dfg_renderer* r,
				  struct am_dfg_node* n)
{
	r->selected_node = n;
}

/* Unsets the currently selected node. */
static inline void
am_dfg_renderer_unset_selected_node(struct am_dfg_renderer* r)
{
	r->selected_node = NULL;
}

/* Returns true if a node is currently selected. */
static inline int
am_dfg_renderer_has_selected_node(struct am_dfg_renderer* r)
{
	return r->selected_node != NULL;
}

/* Sets the current error cycle (path whose edges are highlighted as an
 * error). The path may include edges that do not exist in the rendered
 * graph. */
static inline void
am_dfg_renderer_set_error_cycle(struct am_dfg_renderer* r,
				struct am_dfg_path* cycle)
{
	r->error_cycle = cycle;
}

/* Unsets the current error cycle */
static inline void
am_dfg_renderer_unset_error_cycle(struct am_dfg_renderer* r)
{
	r->error_cycle = NULL;
}


/* Translates a width of w pixels to a width in graph units */
static inline double
am_dfg_renderer_screen_w_to_graph(const struct am_dfg_renderer* r, double w)
{
	return w / r->zoom;
}

/* Translates a height of h pixels to a height in graph units */
static inline double
am_dfg_renderer_screen_h_to_graph(const struct am_dfg_renderer* r, double h)
{
	return h / r->zoom;
}

/* Translates a width of w graph units to a width in pixels */
static inline double
am_dfg_renderer_graph_w_to_screen(const struct am_dfg_renderer* r, double w)
{
	return w * r->zoom;
}

/* Translates a height of h graph units to a height in pixels */
static inline double
am_dfg_renderer_graph_h_to_screen(const struct am_dfg_renderer* r, double h)
{
	return h * r->zoom;
}

/* Translates an x pixel coordinate to a graph coordinate */
static inline double
am_dfg_renderer_screen_x_to_graph(const struct am_dfg_renderer* r, double x)
{
	return r->offset.x + x / r->zoom;
}

/* Translates an x graph coordinate to a pixel coordinate */
static inline double
am_dfg_renderer_graph_x_to_screen(const struct am_dfg_renderer* r, double x)
{
	return (x - r->offset.x) * r->zoom;
}

/* Translates a y pixel coordinate to a graph coordinate */
static inline double
am_dfg_renderer_screen_y_to_graph(const struct am_dfg_renderer* r, double y)
{
	return r->offset.y + y / r->zoom;
}

/* Translates a y graph coordinate to a pixel coordinate */
static inline double
am_dfg_renderer_graph_y_to_screen(const struct am_dfg_renderer* r, double y)
{
	return (y - r->offset.y) * r->zoom;
}

/* Translates a point in graph coordinates to screen coordinates */
static inline void
am_dfg_renderer_graph_to_screen(const struct am_dfg_renderer* r,
				const struct am_point* graph_pos,
				struct am_point* screen_pos)
{
	screen_pos->x = am_dfg_renderer_graph_x_to_screen(r, graph_pos->x);
	screen_pos->y = am_dfg_renderer_graph_y_to_screen(r, graph_pos->y);
}

/* Translates a point in screen coordinates to graph coordinates */
static inline void
am_dfg_renderer_screen_to_graph(const struct am_dfg_renderer* r,
				const struct am_point* screen_pos,
				struct am_point* graph_pos)
{
	graph_pos->x = am_dfg_renderer_screen_x_to_graph(r, screen_pos->x);
	graph_pos->y = am_dfg_renderer_screen_y_to_graph(r, screen_pos->y);
}

/* Sets the offset in graph units of the upper left corner relative to the
 * origin of the graph. */
static inline
void am_dfg_renderer_set_offset(struct am_dfg_renderer* r, double x, double y)
{
	r->offset.x = x;
	r->offset.y = y;
}

/* Returns the offset in graph units of the upper left corner relative to the
 * origin of the graph in *x and *y. */
static inline void
am_dfg_renderer_get_offset(const struct am_dfg_renderer* r,
			   double* x, double* y)
{
	*x = r->offset.x;
	*y = r->offset.y;
}

int am_dfg_renderer_set_node_coordinate(struct am_dfg_renderer* r,
					struct am_dfg_node* n,
					double x,
					double y);

int am_dfg_renderer_get_node_coordinate(struct am_dfg_renderer* r,
					const struct am_dfg_node* n,
					struct am_point* p);

int am_dfg_renderer_get_node_coordinate_def(struct am_dfg_renderer* r,
					    const struct am_dfg_node* n,
					    struct am_point* p);

void am_dfg_renderer_remove_node_coordinate(struct am_dfg_renderer* r,
					    const struct am_dfg_node* n);

void am_dfg_renderer_zoom_in(struct am_dfg_renderer* r,
			     const struct am_point* pos);
void am_dfg_renderer_zoom_out(struct am_dfg_renderer* r,
			     const struct am_point* pos);

int
am_dfg_renderer_node_coordinate_from_object_notation(
	struct am_dfg_renderer* r,
	struct am_object_notation_node_list* lst);

int
am_dfg_renderer_node_coordinates_from_object_notation(
	struct am_dfg_renderer* r,
	struct am_object_notation_node_list* lst);

#endif
