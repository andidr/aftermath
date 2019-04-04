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

#include <aftermath/render/recttree/renderer.h>

/* Initializes a rect tree renderer. The parameter render_rect_cb is a function
 * that is called for each visible rectangle when invoking
 * am_recttree_renderer_render and cb_data is a pointer that is passed verbatim
 * to the callback function.
 */
void am_recttree_renderer_init(
	struct am_recttree_renderer* r,
	am_recttree_renderer_render_node_callback render_rect_cb,
	void* cb_data)
{
	r->params.zoom_factor = 1.5;
	r->params.bgcolor = AM_RGBA255(0xFF, 0xFF, 0xFF, 0xFF);

	r->width = 0;
	r->height = 0;
	r->recttree = NULL;
	r->zoom = 1.0;
	r->offset.x = 0;
	r->offset.y = 0;
	r->render_rect_cb = render_rect_cb;
	r->cb_data = cb_data;
}

void am_recttree_renderer_destroy(struct am_recttree_renderer* r)
{
}

/* Sets the rect tree associated to the renderer */
void am_recttree_renderer_set_recttree(struct am_recttree_renderer* r,
				       const struct am_recttree* t)
{
	r->recttree = t;
}

/* Zooms in at a given position. The graph coordinates at the position are
 * preserved, such that the zoom occurs at that position. */
void am_recttree_renderer_zoom_in(struct am_recttree_renderer* r,
				  const struct am_point* pos)
{
	struct am_recttree_renderer_params* p = &r->params;
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

/* Zooms out at a given position. The graph coordinates at the position are
 * preserved, such that the zoom occurs at that position. */
void am_recttree_renderer_zoom_out(struct am_recttree_renderer* r,
				   const struct am_point* pos)
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

/* Data structure passed to the rendering callback function via the rect tree
 * query function */
struct am_recttree_renderer_cb_params {
	struct am_recttree_renderer* renderer;
	cairo_t* cr;
};

/* Callback function invoked by the rect tree query function for each rectangle
 * overlapping with the query rectangle; invokes in turn the rendering callback
 * function associated with the renderer.
 *
 * Always returns 0 to indicate to the query function that the enumeration of
 * rectangles should not stop before all rectangles have been handled.
 */
static int am_recttree_renderer_node_callback(struct am_recttree_node* n,
					      void* data)
{
	struct am_recttree_renderer_cb_params* cb_params = data;
	struct am_recttree_renderer* r = cb_params->renderer;
	struct am_rect screen_rect;
	cairo_t* cr = cb_params->cr;

	screen_rect.x = am_recttree_renderer_graph_x_to_screen(
		r, n->kdnode.coordinates[0]);
	screen_rect.y = am_recttree_renderer_graph_y_to_screen(
		r, n->kdnode.coordinates[1]);

	screen_rect.width = am_recttree_renderer_graph_w_to_screen(
		r, n->hyperrect_end[0] - n->kdnode.coordinates[0]);
	screen_rect.height = am_recttree_renderer_graph_h_to_screen(
		r, n->hyperrect_end[1] - n->kdnode.coordinates[1]);

	r->render_rect_cb(cr, screen_rect, r->zoom, n, r->cb_data);

	return 0;
}

/* Performs the rendering by invoking the rendering callback function associated
 * to the renderer for each rectangle overlapping with the renderer's visible
 * area. */
void am_recttree_renderer_render(struct am_recttree_renderer* r, cairo_t* cr)
{
	struct am_recttree_renderer_cb_params cb_params = {
		.renderer = r,
		.cr = cr
	};

	double query_start[2];
	double query_end[2];

	if(r->width == 0 || r->height == 0)
		return;

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(&r->params.bgcolor));

	cairo_rectangle(cr, 0, 0, r->width, r->height);
	cairo_fill(cr);

	if(!r->recttree)
		return;

	query_start[0] = r->offset.x;
	query_start[1] = r->offset.y;

	query_end[0] = r->offset.x + am_recttree_renderer_screen_w_to_graph(r, r->width);
	query_end[1] = r->offset.y + am_recttree_renderer_screen_h_to_graph(r, r->height);

	if(r->render_rect_cb) {
		am_recttree_query_callback(r->recttree,
					   query_start, query_end,
					   am_recttree_renderer_node_callback,
					   &cb_params);
	}
}

/* Returns the first node whose bounding box includes the point p in screen
 * coordinates. If no such node exists, the function returns NULL. */
struct am_recttree_node*
am_recttree_renderer_node_at(const struct am_recttree_renderer* r,
			     const struct am_point* p)
{
	struct am_point graph_pos;

	am_recttree_renderer_screen_to_graph(r, p, &graph_pos);
	return am_recttree_node_at(r->recttree, &graph_pos.x);
}
