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

#ifndef AM_TELAMON_CANDIDATE_TREE_RENDERER_H
#define AM_TELAMON_CANDIDATE_TREE_RENDERER_H

#include <aftermath/render/recttree/renderer.h>
#include <aftermath/core/base_types.h>
#include <aftermath/core/interval.h>

#define AM_TELAMON_CANDIDATE_TREE_RENDERER_NODE_WIDTH 100
#define AM_TELAMON_CANDIDATE_TREE_RENDERER_NODE_HEIGHT 50

/* Maximum allowed depth for generated rect trees; since the rect trees are
 * usually balanced binary trees (unless a large amount of the rectangles has
 * the same coordinates for the upper left corner), this value can be fairly
 * low. Not to be confused with the depth of the candidate tree. */
#define AM_TELAMON_CANDIDATE_TREE_RENDERER_MAX_RECTTREE_DEPTH 50

struct am_telamon_candidate_tree_renderer_params {
	struct {
		struct am_rgba background;

		struct {
			struct {
				struct am_rgba fill;
				struct am_rgba stroke;
			} internal_node;

			struct {
				struct am_rgba fill;
				struct am_rgba stroke;
			} internal_deadend;

			struct {
				struct am_rgba fill;
				struct am_rgba stroke;
			} rollout_node;

			struct {
				struct am_rgba fill;
				struct am_rgba stroke;
			} rollout_deadend;

			struct {
				struct am_rgba fill;
				struct am_rgba stroke;
			} implementation_node;

			struct {
				struct am_rgba fill;
				struct am_rgba stroke;
			} implementation_deadend;

			struct {
				struct am_rgba fill;
				struct am_rgba stroke;
			} highlighted;

			struct {
				struct am_rgba fill;
				struct am_rgba stroke;
			} unknown;
		} nodes;

		struct {
			struct am_rgba normal;
			struct am_rgba highlighted;
		} edges;
	} color;
};

/* The renderer's internal representation of a candidate; Each node is
 * represented by a rectangle and all rectangles are indexed by a rect tree. */
struct am_telamon_candidate_tree_node {
	struct am_recttree_node recttree_node;
	struct am_telamon_candidate* candidate;

	/* Upper left corner of bounding box in graph
	 * coordinates */
	struct am_point upper_left;

	/* Lower right corner of bounding box in graph
	 * coordinates */
	struct am_point lower_right;

	/* Pointer to children nodes */
	struct am_telamon_candidate_tree_node* children;

	/* If true, the node is rendered as a selected node */
	int selected:1;
};

/* The renderer's internal representation of an edge between two
 * candidates. Although edges are implicit in a candidate tree, they are stored
 * in an explicit representation for the widget for indexing by a rect tree for
 * fast rendering.  */
struct am_telamon_candidate_tree_edge {
	struct am_recttree_node recttree_node;

	/* The coordinates only define the line's bounding box
	 * in graph coordinates; The actual coordinates (in
	 * graph coordinated) for the start and end point of the
	 * line are derived from the bounding box through the
	 * mode field. */
	struct am_point bb_upper_left;
	struct am_point bb_lower_right;

	/* Instead of explicitly storing the coordinates of the
	 * start and end point in addition to the bounding box,
	 * only indicate which corners need to be connected and
	 * in which direction. */
	enum {
		/* Upper left to lower right */
		AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_UL_LR,

		/* Upper right to lower left */
		AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_UR_LL,

		/* Lower right to upper left */
		AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_LR_UL,

		/* Lower left to upper right */
		AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_LL_UR
	} mode;

	/* If true, the edge is rendered as a selected edge */
	int selected:1;

	/* Source node of this edge (where the edge starts) */
	const struct am_telamon_candidate_tree_node* src_node;

	/* Destination node of this edge (where the edge points to) */
	const struct am_telamon_candidate_tree_node* dst_node;
};

/* Renderer for Telamon candidate trees */
struct am_telamon_candidate_tree_renderer {
	struct am_recttree_renderer node_renderer;
	struct am_recttree_renderer edge_renderer;

	struct am_telamon_candidate_tree_renderer_params params;

	/* Renderer-private array of nodes enriching candidates with
	 * placement */
	struct am_telamon_candidate_tree_node* nodes;

	/* Pointer to the node associated with the root of the candidate tree */
	struct am_telamon_candidate_tree_node* root_node;

	/* Renderer-private array of edges between the nodes, enriched with
	 * placement information */
	struct am_telamon_candidate_tree_edge* edges;

	/* Rendering indexes */
	struct am_recttree node_rect_tree;
	struct am_recttree edge_rect_tree;

	/* Indicates whether the renderer has been initialized and is currently
	 * associated with a candidate tree */
	int valid;

	/* If set, limits rendering of nodes whose creation time is within one
	 * of the specified intervals */
	const struct am_interval* intervals;
	size_t num_intervals;
	am_timestamp_t max_interval_end;
};

void am_telamon_candidate_tree_renderer_init(
	struct am_telamon_candidate_tree_renderer* r);

void am_telamon_candidate_tree_renderer_destroy(
	struct am_telamon_candidate_tree_renderer* r);

void am_telamon_candidate_tree_renderer_render(
	struct am_telamon_candidate_tree_renderer* r,
	cairo_t* cr);

int am_telamon_candidate_tree_renderer_set_root(
	struct am_telamon_candidate_tree_renderer* r,
	struct am_telamon_candidate* root);

struct am_telamon_candidate*
am_telamon_candidate_tree_renderer_candidate_at(
	struct am_telamon_candidate_tree_renderer* r,
	const struct am_point* p);

int am_telamon_candidate_tree_renderer_select(
	struct am_telamon_candidate_tree_renderer* r,
	struct am_telamon_candidate* c);

int am_telamon_candidate_tree_renderer_unselect(
	struct am_telamon_candidate_tree_renderer* r,
	struct am_telamon_candidate* c);

/* Sets the width in pixels of the renderer. */
static inline void am_telamon_candidate_tree_renderer_set_width(
	struct am_telamon_candidate_tree_renderer* r,
	unsigned int w)
{
	am_recttree_renderer_set_width(&r->node_renderer, w);
	am_recttree_renderer_set_width(&r->edge_renderer, w);
}

/* Sets the height in pixels of the renderer. */
static inline void
am_telamon_candidate_tree_renderer_set_height(
	struct am_telamon_candidate_tree_renderer* r,
	unsigned int h)
{
	am_recttree_renderer_set_height(&r->node_renderer, h);
	am_recttree_renderer_set_height(&r->edge_renderer, h);
}

/* Translates a point in graph coordinates to screen coordinates */
static inline void
am_telamon_candidate_tree_renderer_graph_to_screen(
	const struct am_telamon_candidate_tree_renderer* r,
	const struct am_point* graph_pos,
	struct am_point* screen_pos)
{
	/* Any of the renderers is fine */
	am_recttree_renderer_graph_to_screen(
		&r->node_renderer, graph_pos, screen_pos);
}

/* Translates a point in screen coordinates to graph coordinates */
static inline void
am_telamon_candidate_tree_renderer_screen_to_graph(
	const struct am_telamon_candidate_tree_renderer* r,
	const struct am_point* screen_pos,
	struct am_point* graph_pos)
{
	/* Any of the renderers is fine */
	am_recttree_renderer_screen_to_graph(
		&r->node_renderer, screen_pos, graph_pos);
}

/* Sets the offset in graph units of the upper left corner relative to the
 * origin of the graph. */
static inline void
am_telamon_candidate_tree_renderer_set_offset(
	struct am_telamon_candidate_tree_renderer* r,
	double x,
	double y)
{
	am_recttree_renderer_set_offset(&r->node_renderer, x, y);
	am_recttree_renderer_set_offset(&r->edge_renderer, x, y);
}

/* Returns the offset in graph units of the upper left corner relative to the
 * origin of the graph in *x and *y. */
static inline void
am_telamon_candidate_tree_renderer_get_offset(
	const struct am_telamon_candidate_tree_renderer* r,
	double* x,
	double* y)
{
	/* Any of the renderers is fine */
	am_recttree_renderer_get_offset(&r->node_renderer, x, y);
}

/* Zooms in at a given position. The graph coordinates at the position are
 * preserved, such that the zoom occurs at that position. */
static inline void am_telamon_candidate_tree_renderer_zoom_in(
	struct am_telamon_candidate_tree_renderer* r,
	const struct am_point* pos)
{
	am_recttree_renderer_zoom_in(&r->node_renderer, pos);
	am_recttree_renderer_zoom_in(&r->edge_renderer, pos);
}

/* Zooms out at a given position. The graph coordinates at the position are
 * preserved, such that the zoom occurs at that position. */
static inline void am_telamon_candidate_tree_renderer_zoom_out(
	struct am_telamon_candidate_tree_renderer* r,
	const struct am_point* pos)
{
	am_recttree_renderer_zoom_out(&r->node_renderer, pos);
	am_recttree_renderer_zoom_out(&r->edge_renderer, pos);
}

/* Translates a width of w pixels to a width in graph units */
static inline double
am_telamon_candidate_tree_renderer_screen_w_to_graph(
	const struct am_telamon_candidate_tree_renderer* r, double w)
{
	return am_recttree_renderer_screen_w_to_graph(&r->node_renderer, w);
}

/* Translates a height of h pixels to a height in graph units */
static inline double
am_telamon_candidate_tree_renderer_screen_h_to_graph(
	const struct am_telamon_candidate_tree_renderer* r, double h)
{
	return am_recttree_renderer_screen_h_to_graph(&r->node_renderer, h);
}

/* Translates a width of w graph units to a width in pixels */
static inline double
am_telamon_candidate_tree_renderer_graph_w_to_screen(
	const struct am_telamon_candidate_tree_renderer* r, double w)
{
	return am_recttree_renderer_graph_w_to_screen(&r->node_renderer, w);
}

/* Translates a height of h graph units to a height in pixels */
static inline double
am_telamon_candidate_tree_renderer_graph_h_to_screen(
	const struct am_telamon_candidate_tree_renderer* r, double h)
{
	return am_recttree_renderer_graph_h_to_screen(&r->node_renderer, h);
}

/* Translates an x pixel coordinate to a graph coordinate */
static inline double
am_telamon_candidate_tree_renderer_screen_x_to_graph(
	const struct am_telamon_candidate_tree_renderer* r, double x)
{
	return am_recttree_renderer_screen_x_to_graph(&r->node_renderer, x);
}

/* Translates an x graph coordinate to a pixel coordinate */
static inline double
am_telamon_candidate_tree_renderer_graph_x_to_screen(
	const struct am_telamon_candidate_tree_renderer* r, double x)
{
	return am_recttree_renderer_graph_x_to_screen(&r->node_renderer, x);
}

/* Translates a y pixel coordinate to a graph coordinate */
static inline double
am_telamon_candidate_tree_renderer_screen_y_to_graph(
	const struct am_telamon_candidate_tree_renderer* r, double y)
{
	return am_recttree_renderer_screen_y_to_graph(&r->node_renderer, y);
}

/* Translates a y graph coordinate to a pixel coordinate */
static inline double
am_telamon_candidate_tree_renderer_graph_y_to_screen(
	const struct am_telamon_candidate_tree_renderer* r, double y)
{
	return am_recttree_renderer_graph_y_to_screen(&r->node_renderer, y);
}

/* Limits rendering to candidates whose creation timestamp is within at least
 * one of the specified intervals */
static inline void
am_telamon_candidate_tree_renderer_set_intervals(
	struct am_telamon_candidate_tree_renderer* r,
	const struct am_interval* intervals,
	size_t num_intervals)
{
	r->num_intervals = num_intervals;
	r->intervals = intervals;
	r->max_interval_end = 0;

	for(size_t i = 0; i < num_intervals; i++) {
		if(intervals[i].end > r->max_interval_end)
			r->max_interval_end = intervals[i].end;
	}
}

/* Resets the interval rendering filter, such that all candidates are rendered */
static inline void
am_telamon_candidate_tree_renderer_reset_intervals(
	struct am_telamon_candidate_tree_renderer* r)
{
	r->intervals = NULL;
	r->num_intervals = 0;
	r->max_interval_end = AM_TIMESTAMP_T_MAX;
}

#endif
