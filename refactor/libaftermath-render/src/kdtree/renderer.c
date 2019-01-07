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

#include <aftermath/render/kdtree/renderer.h>

/* Initializes a k-d-tree renderer. The parameter render_node_cb is a function
 * that is called for each visible point when invoking am_kdtree_renderer_render
 * and cb_data is a pointer that is passed verbatim to the callback function.
 */
void am_kdtree_renderer_init(
	struct am_kdtree_renderer* r,
	am_kdtree_renderer_render_node_callback render_node_cb,
	void* cb_data)
{
	r->params.zoom_factor = 1.5;
	r->params.bgcolor = AM_RGBA255(0xFF, 0xFF, 0xFF, 0xFF);

	r->width = 0;
	r->height = 0;
	r->kdtree = NULL;
	r->zoom = 1.0;
	r->offset.x = 0;
	r->offset.y = 0;
	r->render_node_cb = render_node_cb;
	r->cb_data = cb_data;
}

void am_kdtree_renderer_destroy(struct am_kdtree_renderer* r)
{
}

/* Sets the k-d-tree associated to the renderer */
void am_kdtree_renderer_set_kdtree(struct am_kdtree_renderer* r,
				   const struct am_kdtree* t)
{
	r->kdtree = t;
}

/* Zooms in at a given position. The graph coordinates at the position are
 * preserved, such that the zoom occurs at that position. */
void am_kdtree_renderer_zoom_in(struct am_kdtree_renderer* r,
				const struct am_point* pos)
{
	struct am_kdtree_renderer_params* p = &r->params;
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
void am_kdtree_renderer_zoom_out(struct am_kdtree_renderer* r,
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

/* Data structure passed to the rendering callback function via the k-d-tree
 * query function */
struct am_kdtree_renderer_cb_params {
	struct am_kdtree_renderer* renderer;
	cairo_t* cr;
};

/* Callback function invoked by the k-d-tree query function for each point
 * within the query rectangle; invokes in turn the rendering callback function
 * associated with the renderer.
 *
 * Always returns 0 to indicate to the query function that the enumeration of
 * points should not stop before all points have been handled.
 */
static int am_kdtree_renderer_node_callback(struct am_kdtree_node* n,
					    void* data)
{
	struct am_kdtree_renderer_cb_params* cb_params = data;
	struct am_kdtree_renderer* r = cb_params->renderer;
	struct am_point screen_pos = {
		.x = am_kdtree_renderer_graph_x_to_screen(r, n->coordinates[0]),
		.y = am_kdtree_renderer_graph_y_to_screen(r, n->coordinates[1])
	};
	cairo_t* cr = cb_params->cr;

	r->render_node_cb(cr, screen_pos, r->zoom, n, r->cb_data);

	return 0;
}

/* Performs the rendering by invoking the rendering callback function associated
 * to the renderer for each point within renderer's visible area. */
void am_kdtree_renderer_render(struct am_kdtree_renderer* r, cairo_t* cr)
{
	struct am_kdtree_renderer_cb_params cb_params = {
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

	if(!r->kdtree)
		return;

	query_start[0] = r->offset.x;
	query_start[1] = r->offset.y;

	query_end[0] = r->offset.x + am_kdtree_renderer_screen_w_to_graph(r, r->width);
	query_end[1] = r->offset.y + am_kdtree_renderer_screen_h_to_graph(r, r->height);

	if(r->render_node_cb) {
		am_kdtree_query_callback(r->kdtree,
					 query_start, query_end,
					 am_kdtree_renderer_node_callback,
					 &cb_params);
	}
}
