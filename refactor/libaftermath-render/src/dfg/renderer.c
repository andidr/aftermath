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

#include <aftermath/render/dfg/renderer.h>
#include <aftermath/render/cairo_extras.h>

void am_dfg_renderer_init(struct am_dfg_renderer* r)
{
	r->width = 0;
	r->height = 0;
	r->zoom = 1.0;
	r->offset.x = 0;
	r->offset.y = 0;

	r->params.zoom_factor = 1.5;
	r->params.bgcolor = AM_RGBA255(0xFF, 0xFF, 0xFF, 0xFF);

	r->params.main_rects.min_h = 30;
	r->params.main_rects.min_w = 50;
	r->params.main_rects.pad_x = 3;
	r->params.main_rects.pad_y = 3;
	r->params.main_rects.corner_radius = 7;

	r->params.main_rects.color.stroke.highlighted =
		AM_RGBA255(0xEE, 0xB7, 0x00, 0xFF);
	r->params.main_rects.color.stroke.error =
		AM_RGBA255(0xFF, 0x00, 0x00, 0xFF);
	r->params.main_rects.color.stroke.normal =
		AM_RGBA255(0x66, 0x66, 0x66, 0xFF);

	r->params.main_rects.color.bg.highlighted =
		AM_RGBA255(0xEE, 0xEE, 0xEE, 0xFF);
	r->params.main_rects.color.bg.normal =
		AM_RGBA255(0xEE, 0xEE, 0xEE, 0xFF);
	r->params.main_rects.color.bg.selected =
		AM_RGBA255(0xFF, 0x97, 0x00, 0xFF);

	r->params.ports.label.font = "Sans";
	r->params.ports.label.font_size = 10;

	r->params.ports.height = 15;
	r->params.ports.label.pad_x = 10;
	r->params.ports.rect.height = 5;
	r->params.ports.rect.width = 5;
	r->params.ports.rect.corner_radius = 1;

	r->params.connections.ctrl_distance = 20;

	r->params.ports.rect.colors.in.normal =
		AM_RGBA255(0x00, 0x80, 0x00, 0xFF);
	r->params.ports.rect.colors.in.highlighted =
		AM_RGBA255(0x66, 0xca, 0x00, 0xFF);
	r->params.ports.rect.colors.out.normal =
		AM_RGBA255(0xFF, 0x5D, 0x00, 0xFF);
	r->params.ports.rect.colors.out.highlighted =
		AM_RGBA255(0xFF, 0xBB, 0x00, 0xFF);

	r->params.names.label.font = "Sans";
	r->params.names.label.font_size = 8;
	r->params.names.label.color.normal =
		AM_RGBA255(0x00, 0x00, 0x00, 0xFF);
	r->params.names.label.color.highlighted =
		AM_RGBA255(0x00, 0x00, 0x00, 0xFF);
	r->params.names.rect.corner_radius = 2;
	r->params.names.label.pad_x = 3;
	r->params.names.label.pad_y = 1;

	r->params.names.rect.color.stroke.highlighted =
		AM_RGBA255(0xEE, 0xB7, 0x00, 0xFF);
	r->params.names.rect.color.stroke.normal =
		AM_RGBA255(0x33, 0x33, 0x33, 0xFF);
	r->params.names.rect.color.bg.normal =
		AM_RGBA255(0xDD, 0xDD, 0xDD, 0xFF);
	r->params.names.rect.color.bg.highlighted =
		AM_RGBA255(0xEE, 0xEE, 0xEE, 0xFF);

	r->params.connections.normal.color =
		AM_RGBA255(0x00, 0x00, 0x00, 0xFF);
	r->params.connections.normal.width = 1;

	r->params.connections.highlighted.color =
		AM_RGBA255(0xEE, 0xB7, 0x00, 0xFF);
	r->params.connections.highlighted.width = 1;

	r->params.connections.selected.color =
		AM_RGBA255(0xFF, 0x97, 0x00, 0xFF);
	r->params.connections.selected.width = 1;

	r->params.connections.floating.color =
		AM_RGBA255(0x00, 0x00, 0x00, 0xFF);
	r->params.connections.floating.width = 1;

	r->params.connections.error.color =
		AM_RGBA255(0xC8, 0x00, 0x00, 0xFF);
	r->params.connections.error.width = 2;

	r->highlighted_node = NULL;
	r->highlighted_port = NULL;
	r->ignore_connection.src = NULL;
	r->ignore_connection.dst = NULL;
	r->highlighted_connection.src = NULL;
	r->highlighted_connection.dst = NULL;
	r->selected_connection.src = NULL;
	r->selected_connection.dst = NULL;
	r->error_cycle = NULL;

	r->floating_connection.show = 0;
	r->params.connections.collision_px = 8;

	r->selected_node = NULL;
	r->graph = NULL;
	r->coordinate_mapping = NULL;
}

void am_dfg_renderer_destroy(struct am_dfg_renderer* r)
{
}

void am_dfg_renderer_set_graph(struct am_dfg_renderer* r,
			       const struct am_dfg_graph* g)
{
	r->graph = g;
}

void am_dfg_renderer_set_coordinate_mapping(
	struct am_dfg_renderer* r,
	const struct am_dfg_coordinate_mapping* m)
{
	r->coordinate_mapping = m;
}

/* Calculates the dimensions and position of the main rectangle of a node n. The
 * result is returned in rect.
 */
static void
am_dfg_renderer_node_main_rect_dimensions(struct am_dfg_renderer* r,
					  cairo_t* cr,
					  const struct am_dfg_node* n,
					  const struct am_point* node_pos,
					  struct am_rect* rect)
{
	struct am_dfg_port* p;
	cairo_text_extents_t extents;
	double max_in = 0;
	double max_out = 0;
	size_t n_in = 0;
	size_t n_out = 0;
	double main_rect_width;
	double main_rect_height;
	double name_label_width;

	cairo_select_font_face(cr,
			       r->params.ports.label.font,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);

	cairo_set_font_size(cr, r->params.ports.label.font_size);

	/* Calculate width of each port label */
	am_dfg_node_for_each_port(n, p) {
		cairo_text_extents(cr, p->type->name, &extents);

		if(p->type->flags & AM_DFG_PORT_IN) {
			if(max_in < extents.width)
				max_in = extents.width;

			n_in++;
		} else {
			if(max_out < extents.width)
				max_out = extents.width;

			n_out++;
		}
	}

	main_rect_width = max_in +
		max_out +
		r->params.ports.label.pad_x +
		2 * r->params.main_rects.pad_x;

	main_rect_height = ((n_in > n_out) ? n_in : n_out) *
		r->params.ports.height +
		2 * r->params.main_rects.pad_y;

	/* Calculate size of the type label */
	cairo_select_font_face(cr,
			       r->params.names.label.font,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size(cr, r->params.names.label.font_size);

	cairo_text_extents(cr, n->type->hrname, &extents);
	name_label_width = extents.width + 2 * r->params.names.label.pad_x;

	/* Extend main rectangle if type label is larger than the required size
	 * for the main rectangle */
	if(name_label_width + 2 * r->params.main_rects.corner_radius >
	   main_rect_width)
	{
		main_rect_width = name_label_width +
			2 * r->params.main_rects.corner_radius;
	}

	/* Extend if size is smaller than the minimum */
	rect->width = (main_rect_width > r->params.main_rects.min_w) ?
		main_rect_width :
		r->params.main_rects.min_w;

	rect->height = (main_rect_height > r->params.main_rects.min_h) ?
		main_rect_height :
		r->params.main_rects.min_h;

	rect->x = node_pos->x;
	rect->y = node_pos->y;
}

/* Calculates the dimensions and position of the type label rectangle of a node
 * n. The result is returned in rect.
 */
static void
am_dfg_renderer_name_label_rect_dimensions(struct am_dfg_renderer* r,
					   cairo_t* cr,
					   const struct am_dfg_node* n,
					   const struct am_point* node_pos,
					   struct am_rect* rect)
{
	cairo_text_extents_t extents;
	cairo_font_extents_t fextents;

	cairo_select_font_face(cr,
			       r->params.names.label.font,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size(cr, r->params.names.label.font_size);

	/* Size is padding + text width / height */
	cairo_text_extents(cr, n->type->hrname, &extents);
	cairo_font_extents(cr, &fextents);

	rect->width = extents.width + 2*r->params.names.label.pad_x;
	rect->height = fextents.height + 2*r->params.names.label.pad_y;

	rect->x = node_pos->x + r->params.main_rects.corner_radius;
	rect->y = node_pos->y - rect->height;
}

/* Calculates the coordinates of the top left corner of a port p. The result is
 * returned in point.
 */
void am_dfg_renderer_port_connection_point_xy(struct am_dfg_renderer* r,
					      cairo_t* cr,
					      const struct am_dfg_port* p,
					      struct am_point* point)
{
	struct am_dfg_node* n = p->node;
	size_t idx;
	struct am_rect mr;
	struct am_point node_pos;

	idx = am_dfg_node_get_port_index(n, p);

	am_dfg_renderer_get_node_coordinate_def(r, n, &node_pos);
	am_dfg_renderer_node_main_rect_dimensions(r, cr, n, &node_pos, &mr);

	point->y = mr.y +
		idx * r->params.ports.height +
		r->params.main_rects.pad_y +
		r->params.ports.height / 2;

	if(p->type->flags & AM_DFG_PORT_IN)
		point->x = mr.x - r->params.ports.rect.width;
	else
		point->x = mr.x + mr.width + r->params.ports.rect.width;
}

/* Draws the box with the label at the top of a node:
 *
 *    .-----------.
 *    | Label     |
 *  .-----------------------.
 *  |                       |
 */
static void am_dfg_renderer_paint_node_label(struct am_dfg_renderer* r,
					     cairo_t* cr,
					     const struct am_dfg_node* n,
					     const struct am_point* node_pos)
{
	cairo_font_extents_t fextents;
	struct am_rect rect;
	const struct am_rgba* stroke_color;
	const struct am_rgba* bg_color;
	const struct am_rgba* label_color;

	/* Calculate size of the label */
	cairo_select_font_face(cr, r->params.names.label.font,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size(cr, r->params.names.label.font_size);
	cairo_font_extents(cr, &fextents);

	am_dfg_renderer_name_label_rect_dimensions(r, cr, n, node_pos, &rect);

	if(r->highlighted_node == n) {
		stroke_color = &r->params.names.rect.color.stroke.highlighted;
		bg_color = &r->params.names.rect.color.bg.highlighted;
		label_color = &r->params.names.label.color.highlighted;
	} else {
		stroke_color = &r->params.names.rect.color.stroke.normal;
		bg_color = &r->params.names.rect.color.bg.normal;
		label_color = &r->params.names.label.color.normal;
	}

	/* Draw box around label rectangle */
	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(stroke_color));
	cairo_set_line_width(cr, 1);

	am_rounded_rectangle_corners(cr,
				     &rect,
				     r->params.names.rect.corner_radius,
				     AM_ROUNDED_CORNERS_TOP);

	cairo_stroke_preserve(cr);

	/* Draw node background */
	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(bg_color));
	cairo_fill(cr);

	/* Draw label box background */
	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(label_color));
	cairo_move_to(cr,
		      rect.x + r->params.names.label.pad_x,
		      rect.y + rect.height - fextents.descent -
		      r->params.names.label.pad_y);
	cairo_show_text(cr, n->type->hrname);
}

/* Draws an entire node, composed of the main rectangle, the top label box and
 * all ports.
 *
 *      .-----------.
 *      | Label     |
 *    .-----------------------.
 *    |                       |
 *  (]| in_port0    out_port0 |[)
 *    |                       |
 *  (]| in_port1    out_port1 |[)
 *    |                       |
 *    \_______________________/
 *
 */
static void am_dfg_renderer_paint_node(struct am_dfg_renderer* r,
				       cairo_t* cr,
				       const struct am_dfg_node* n)
{
	cairo_text_extents_t textents;
	struct am_rect mr;
	size_t i_in = 0;
	size_t i_out = 0;
	double port_y;
	struct am_rect port_rect;
	struct am_point label_point;
	const struct am_rgba* port_rect_color;
	const struct am_rgba* bg_color;
	const struct am_rgba* stroke_color;
	long corner_flags;
	struct am_dfg_port* p;
	struct am_point node_pos;
	typeof(&r->params.ports.rect.colors) prc = &r->params.ports.rect.colors;
	typeof(&r->params.main_rects.color) mrc = &r->params.main_rects.color;

	am_dfg_renderer_get_node_coordinate_def(r, n, &node_pos);
	am_dfg_renderer_paint_node_label(r, cr, n, &node_pos);
	am_dfg_renderer_node_main_rect_dimensions(r, cr, n, &node_pos, &mr);

	/* Main rectangle, background */
	am_rounded_rectangle(cr, &mr, r->params.main_rects.corner_radius);

	if(n == r->highlighted_node)
		bg_color = &mrc->bg.highlighted;
	else
		bg_color = &mrc->bg.normal;

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(bg_color));
	cairo_fill(cr);

	/* Ports */
	cairo_select_font_face(cr,
			       r->params.ports.label.font,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, r->params.ports.label.font_size);

	am_dfg_node_for_each_port(n, p) {
		cairo_text_extents(cr, p->type->name, &textents);

		if(p->type->flags & AM_DFG_PORT_IN) {
			corner_flags = AM_ROUNDED_CORNERS_LEFT;

			port_y = node_pos.y +
				i_in * r->params.ports.height +
				r->params.main_rects.pad_y;

			label_point.x = node_pos.x + r->params.main_rects.pad_x;

			port_rect.x = node_pos.x - r->params.ports.rect.width;

			if(r->highlighted_port == p)
				port_rect_color = &prc->in.highlighted;
			else
				port_rect_color = &prc->in.normal;

			i_in++;
		} else {
			corner_flags = AM_ROUNDED_CORNERS_RIGHT;

			port_y = node_pos.y +
				i_out * r->params.ports.height +
				r->params.main_rects.pad_y;

			label_point.x = node_pos.x +
				mr.width -
				r->params.main_rects.pad_x -
				textents.width;

			port_rect.x = node_pos.x + mr.width;

			if(r->highlighted_port == p)
				port_rect_color = &prc->out.highlighted;
			else
				port_rect_color = &prc->out.normal;

			i_out++;
		}

		port_rect.y = port_y +
			r->params.ports.height / 2 -
			r->params.ports.rect.height / 2;

		label_point.y = port_y +
			r->params.ports.height / 2 +
			textents.height / 2;

		port_rect.height = r->params.ports.rect.height;
		port_rect.width = r->params.ports.rect.width;

		cairo_set_source_rgba(cr, AM_PRGBA_ARGS(port_rect_color));
		cairo_set_line_width(cr, 1);
		am_rounded_rectangle_corners(cr, &port_rect,
					     r->params.ports.rect.corner_radius,
					     corner_flags);
		cairo_fill(cr);

		cairo_move_to(cr, label_point.x, label_point.y);
		cairo_show_text(cr, p->type->name);
	}

	/* Main rectangle, foreground */
	am_rounded_rectangle(cr, &mr, r->params.main_rects.corner_radius);
	cairo_set_line_width(cr, 1);

	if(n == r->highlighted_node) {
		stroke_color = &mrc->stroke.highlighted;
	} else {
		if(n == r->selected_node) {
			stroke_color = &mrc->bg.selected;
		} else {
			if(am_dfg_node_is_well_connected(n))
				stroke_color = &mrc->stroke.normal;
			else
				stroke_color = &mrc->stroke.error;
		}
	}

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(stroke_color));
	cairo_stroke(cr);
}

/* Paints all the nodes of the current graph */
static void am_dfg_renderer_paint_nodes(struct am_dfg_renderer* r, cairo_t* cr)
{
	struct am_dfg_node* n;

	am_dfg_graph_for_each_node(r->graph, n)
		am_dfg_renderer_paint_node(r, cr, n);
}

/* Calculates the coordinates of the control points for the cubic bézier curve
 * of a connection from psrc to pdst. The control points are returned in pcsrc
 * and pcdst. */
static inline void
am_dfg_get_connection_controlpoints_xy(struct am_dfg_renderer* r,
				       cairo_t* cr,
				       const struct am_point* psrc,
				       const struct am_point* pdst,
				       struct am_point* pcsrc,
				       struct am_point* pcdst)
{
	pcsrc->x = psrc->x + r->params.connections.ctrl_distance;
	pcsrc->y = psrc->y;
	pcdst->x = pdst->x - r->params.connections.ctrl_distance;
	pcdst->y = pdst->y;
}

/* Paints the connection from psrc to pdst using a cubic bézier curve. */
static void am_dfg_renderer_paint_connection_xy(struct am_dfg_renderer* r,
						cairo_t* cr,
						const struct am_point* psrc,
						const struct am_point* pdst)
{
	struct am_point pcsrc;
	struct am_point pcdst;

	am_dfg_get_connection_controlpoints_xy(r, cr,
					       psrc, pdst, &pcsrc, &pcdst);

	cairo_move_to(cr, AM_PPOINT_ARGS(psrc));

	cairo_curve_to(cr,
		       AM_POINT_ARGS(pcsrc),
		       AM_POINT_ARGS(pcdst),
		       AM_PPOINT_ARGS(pdst));

	cairo_stroke(cr);
}

/* Paints the connection between a source port src and a destination port
 * dst using a specified line color and width. */
static void am_dfg_renderer_paint_connection_param(struct am_dfg_renderer* r,
						   cairo_t* cr,
						   struct am_dfg_port* src,
						   struct am_dfg_port* dst,
						   const struct am_rgba* color,
						   double width)
{
	struct am_point psrc;
	struct am_point pdst;

	am_dfg_renderer_port_connection_point_xy(r, cr, src, &psrc);
	am_dfg_renderer_port_connection_point_xy(r, cr, dst, &pdst);

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(color));
	cairo_set_line_width(cr, width);

	am_dfg_renderer_paint_connection_xy(r, cr, &psrc, &pdst);
}

/* Paints the connection between a source port src and a destination port
 * dst. */
static void am_dfg_renderer_paint_connection(struct am_dfg_renderer* r,
					     cairo_t* cr,
					     struct am_dfg_port* src,
					     struct am_dfg_port* dst)
{
	struct am_dfg_connection conn = { .src = src, .dst = dst };
	const struct am_rgba* color;
	double width;

	if(am_dfg_connection_eq_nd(&conn, &r->selected_connection)) {
		color = &r->params.connections.selected.color;
		width = r->params.connections.selected.width;
	} else {
		if(am_dfg_connection_eq_nd(&conn, &r->highlighted_connection)) {
			color = &r->params.connections.highlighted.color;
			width = r->params.connections.highlighted.width;
		} else {
			color = &r->params.connections.normal.color;
			width = r->params.connections.normal.width;
		}
	}

	am_dfg_renderer_paint_connection_param(r, cr, src, dst, color, width);
}

/* Paints the connection from a port src to a port dst iff this is not the
 * currently ignored connection. */
static void am_dfg_renderer_paint_connection_ni(struct am_dfg_renderer* r,
						cairo_t* cr,
						struct am_dfg_port* src,
						struct am_dfg_port* dst)
{
	if(!am_dfg_renderer_connection_ignored(r, src, dst))
		am_dfg_renderer_paint_connection(r, cr, src, dst);
}

/* Paints all the connections of the current graph */
static void am_dfg_renderer_paint_connections(struct am_dfg_renderer* r,
					      cairo_t* cr)
{
	struct am_dfg_node* n;
	struct am_dfg_port* p;
	struct am_dfg_port* src;

	am_dfg_graph_for_each_node(r->graph, n) {
		am_dfg_node_for_each_port(n, p) {
			if((p->type->flags & AM_DFG_PORT_IN) &&
			   p->num_connections != 0)
			{
				src = p->connections[0];
				am_dfg_renderer_paint_connection_ni(r, cr, src, p);
			}
		}
	}
}

/* Paints all the connections of the current error cycle if non-NULL */
static void am_dfg_renderer_paint_error_cycle(struct am_dfg_renderer* r,
					      cairo_t* cr)
{
	struct am_dfg_connection* c;

	if(!r->error_cycle)
		return;

	am_dfg_path_for_each_connection(r->error_cycle, c) {
		am_dfg_renderer_paint_connection_param(
			r, cr, c->src, c->dst,
			&r->params.connections.error.color,
			r->params.connections.error.width);
	}
}

/* Paints the current floating connection. */
static void am_dfg_renderer_paint_floating_connection(struct am_dfg_renderer* r,
						      cairo_t* cr)
{
	const struct am_rgba* col = &r->params.connections.floating.color;

	if(!r->floating_connection.show)
		return;

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(col));
	cairo_set_line_width(cr, r->params.connections.floating.width);

	am_dfg_renderer_paint_connection_xy(r, cr,
					    &r->floating_connection.src,
					    &r->floating_connection.dst);
}

/* Renders the entire graph. */
void am_dfg_renderer_render(struct am_dfg_renderer* r, cairo_t* cr)
{
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(r->params.bgcolor));
	cairo_rectangle(cr, 0, 0, r->width, r->height);
	cairo_fill(cr);

	if(!r->graph)
		return;

	cairo_scale(cr, r->zoom, r->zoom);
	cairo_translate(cr, -r->offset.x, -r->offset.y);

	am_dfg_renderer_paint_connections(r, cr);
	am_dfg_renderer_paint_error_cycle(r, cr);
	am_dfg_renderer_paint_nodes(r, cr);
	am_dfg_renderer_paint_floating_connection(r, cr);

	cairo_identity_matrix(cr);
}

/* Identifies the topmost node under the cursor at graph position (x, y). If no
 * such node exists, the function returns NULL, otherwise a pointer to the node.
 */
struct am_dfg_node* am_dfg_renderer_node_at(struct am_dfg_renderer* r,
					    cairo_t* cr,
					    double x,
					    double y)
{
	struct am_dfg_node* n;
	struct am_dfg_node* ret = NULL;
	struct am_rect mr;
	struct am_rect nl;
	struct am_point node_pos;
	struct am_point p = { .x = x, .y = y };

	if(!r->graph)
		goto out_null;

	cairo_scale(cr, r->zoom, r->zoom);
	cairo_translate(cr, -r->offset.x, -r->offset.y);

	/* Traversal must be in reverse order as the last node in the list is
	 * rendered on top of the others */

	/* FIXME: Implement something faster than traversal of the entire set of
	 * nodes. */
	am_dfg_graph_for_each_node_prev(r->graph, n) {
		am_dfg_renderer_get_node_coordinate_def(r, n, &node_pos);
		am_dfg_renderer_node_main_rect_dimensions(r, cr, n, &node_pos, &mr);

		if(am_point_in_rect(&p, &mr)) {
			ret = n;
			goto out;
		}

		am_dfg_renderer_name_label_rect_dimensions(
			r, cr, n, &node_pos, &nl);

		if(am_point_in_rect(&p, &nl)) {
			ret = n;
			goto out;
		}
	}

out:
	cairo_identity_matrix(cr);
out_null:
	return ret;
}

/* Identifies the topmost port under the cursor at position (x, y). If no such
 * port exists, the function returns NULL, otherwise a pointer to the port.
 */
struct am_dfg_port* am_dfg_renderer_port_at(struct am_dfg_renderer* r,
					    cairo_t* cr,
					    double x,
					    double y)
{
	struct am_dfg_node* n;
	struct am_dfg_port* ret = NULL;
	struct am_rect mr;
	struct am_rect port_rect;
	size_t i_in, i_out;
	struct am_point node_pos;
	typeof(&r->params.ports) pparm = &r->params.ports;

	if(!r->graph)
		goto out_null;

	cairo_scale(cr, r->zoom, r->zoom);
	cairo_translate(cr, -r->offset.x, -r->offset.y);

	port_rect.width = pparm->rect.width;
	port_rect.height = pparm->rect.height;

	/* Traversal must be in reverse order as the last node in the list is
	 * rendered on top of the others */

	/* FIXME: Implement something faster than traversal of the entire set of
	 * nodes. */
	am_dfg_graph_for_each_node_prev(r->graph, n) {
		am_dfg_renderer_get_node_coordinate_def(r, n, &node_pos);
		am_dfg_renderer_node_main_rect_dimensions(r, cr, n, &node_pos, &mr);

		/* Check if we're close enough to the main rectangle to be on a
		 * port */
		if(x >= mr.x - pparm->rect.width &&
		   x <= mr.x + mr.width + pparm->rect.width &&
		   y >= mr.y && y <= mr.y + mr.height)
		{
			i_in = 0;
			i_out = 0;

			for(size_t i = 0; i < n->type->num_ports; i++) {
				if(n->type->ports[i].flags & AM_DFG_PORT_IN) {
					port_rect.y = mr.y +
						i_in * pparm->height +
						r->params.main_rects.pad_y +
						pparm->height / 2 -
						pparm->rect.height / 2;

					port_rect.x = mr.x -
						pparm->rect.width;
					i_in++;
				} else {
					port_rect.y = mr.y +
						i_out * pparm->height +
						r->params.main_rects.pad_y +
						pparm->height / 2 -
						pparm->rect.height / 2;

					port_rect.x = mr.x + mr.width;
					i_out++;
				}

				if(x >= port_rect.x &&
				   x <= port_rect.x + pparm->rect.width &&
				   y >= port_rect.y &&
				   y <= port_rect.y + pparm->rect.height)
				{
					ret = &n->ports[i];
					goto out;
				}
			}
		}
	}

out:
	cairo_identity_matrix(cr);
out_null:
	return ret;
}

/* Identifies the topmost connection at (x, y) with a tolerance of
 * r->params.connections.collision_px around each curve representing a
 * connection. If no such connection exists, the function returns 0. Otherwise,
 * the function returns 0 and the ports linked by the connection are passed in
 * *psrc and *pdst. */
int am_dfg_renderer_connection_at(struct am_dfg_renderer* r,
				  cairo_t* cr,
				  double x,
				  double y,
				  struct am_dfg_port** psrc,
				  struct am_dfg_port** pdst)
{
	struct am_dfg_node* n;
	struct am_rect mr;
	struct am_point src_point;
	struct am_point dst_point;
	struct am_point csrc_point;
	struct am_point cdst_point;
	struct am_point node_pos;
	struct am_point p = { .x = x, .y = y };
	int ret = 0;

	cairo_scale(cr, r->zoom, r->zoom);
	cairo_translate(cr, -r->offset.x, -r->offset.y);

	/* Traversal must be in reverse order as the last node in the list is
	 * rendered on top of the others */

	/* FIXME: Implement something faster than traversal of the entire set of
	 * nodes. */
	am_dfg_graph_for_each_node_prev(r->graph, n) {
		am_dfg_renderer_get_node_coordinate_def(r, n, &node_pos);
		am_dfg_renderer_node_main_rect_dimensions(r, cr, n, &node_pos, &mr);

		for(size_t i = 0; i < n->type->num_ports; i++) {
			if((n->type->ports[i].flags & AM_DFG_PORT_IN) &&
			   n->ports[i].num_connections != 0)
			{
				am_dfg_renderer_port_connection_point_xy(
					r, cr,
					n->ports[i].connections[0],
					&src_point);

				am_dfg_renderer_port_connection_point_xy(
					r, cr,
					&n->ports[i], &dst_point);

				am_dfg_get_connection_controlpoints_xy(
					r, cr,
					&src_point, &dst_point,
					&csrc_point, &cdst_point);

				if(am_point_on_curve(
					   cr, &p,
					   &src_point,
					   &csrc_point,
					   &cdst_point,
					   &dst_point,
					   r->params.connections.collision_px))
				{
					*psrc = n->ports[i].connections[0];
					*pdst = &n->ports[i];

					ret = 1;
					goto out;
				}
			}
		}
	}

out:
	cairo_identity_matrix(cr);
	return ret;
}

/* Retrieves the coordinates for the node n and returns the position in *p. If
 * the node does not have a mapping, the function return 1, otherwise 0. */
int am_dfg_renderer_get_node_coordinate(struct am_dfg_renderer* r,
					const struct am_dfg_node* n,
					struct am_point* p)
{
	return am_dfg_coordinate_mapping_get_coordinates(
		r->coordinate_mapping, n->id, p);
}

/* Same as am_dfg_renderer_get_node_coordinate, but returns the default position
 * (0, 0) if the node's position is unknown. */
int am_dfg_renderer_get_node_coordinate_def(struct am_dfg_renderer* r,
					    const struct am_dfg_node* n,
					    struct am_point* p)
{
	if(!r->coordinate_mapping ||
	   am_dfg_coordinate_mapping_get_coordinates(
		   r->coordinate_mapping, n->id, p))
	{
		p->x = 0;
		p->y = 0;

		return 1;
	}

	return 0;
}


/* Zooms in, such that the graph coordinates at pixel position pos remain
 * identical */
void am_dfg_renderer_zoom_in(struct am_dfg_renderer* r, const struct am_point* pos)
{
	struct am_dfg_renderer_params* p = &r->params;
	double dx;
	double dy;

	dx = (p->zoom_factor - 1) / (r->zoom * p->zoom_factor) * pos->x;
	dy = (p->zoom_factor - 1) / (r->zoom * p->zoom_factor) * pos->y;

	r->offset.x += dx;
	r->offset.y += dy;

	r->zoom *= p->zoom_factor;

	if(r->zoom == 0.0)
		r->zoom = DBL_MIN;
}

/* Zooms out, such that the graph coordinates at pixel position pos remain
 * identical */
void am_dfg_renderer_zoom_out(struct am_dfg_renderer* r, const struct am_point* pos)
{
	double zf = r->params.zoom_factor;
	double dx;
	double dy;

	dx = (1.0 / zf - 1) / (r->zoom * 1.0 / zf) * pos->x;
	dy = (1.0 / zf - 1) / (r->zoom * 1.0 / zf) * pos->y;

	r->offset.x += dx;
	r->offset.y += dy;

	r->zoom /= zf;

	if(r->zoom == 0.0)
		r->zoom = DBL_MIN;
}
