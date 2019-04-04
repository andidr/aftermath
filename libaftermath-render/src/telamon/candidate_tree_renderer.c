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

#include <aftermath/render/telamon/candidate_tree_renderer.h>
#include <aftermath/render/tree_layout.h>
#include <aftermath/core/interval.h>
#include <aftermath/core/telamon.h>

static double am_telamon_candidate_tree_node_get_x(
	const struct am_telamon_candidate_tree_node* n)
{
	return n->upper_left.x;
}

static inline double am_telamon_candidate_tree_node_get_y(
	const struct am_telamon_candidate_tree_node* n)
{
	return n->upper_left.y;
}

static inline double am_telamon_candidate_tree_node_get_width(
	const struct am_telamon_candidate_tree_node* n)
{
	return AM_TELAMON_CANDIDATE_TREE_RENDERER_NODE_WIDTH;
}

static inline double am_telamon_candidate_tree_node_get_height(
	const struct am_telamon_candidate_tree_node* n)
{
	return AM_TELAMON_CANDIDATE_TREE_RENDERER_NODE_HEIGHT;
}

static inline double am_telamon_candidate_tree_node_set_x(
	struct am_telamon_candidate_tree_node* n,
		   double x)
{
	return n->upper_left.x = x;
}

static inline double am_telamon_candidate_tree_node_set_y(
	struct am_telamon_candidate_tree_node* n,
	double y)
{
	return n->upper_left.y = y;
}

static inline struct am_telamon_candidate_tree_node*
am_telamon_candidate_tree_node_get_nth_child(
	struct am_telamon_candidate_tree_node* tn,
	size_t n)
{
	return &tn->children[n];
}

static inline size_t
am_telamon_candidate_tree_node_get_num_children(
	const struct am_telamon_candidate_tree_node* n)
{
	return n->candidate->num_children;
}

AM_DECL_TREE_LAYOUT_FUN(
	_telamon_candidate_tree,
	struct am_telamon_candidate_tree_node,
	am_telamon_candidate_tree_node_get_x,
	am_telamon_candidate_tree_node_get_y,
	am_telamon_candidate_tree_node_set_x,
	am_telamon_candidate_tree_node_set_y,
	am_telamon_candidate_tree_node_get_width,
	am_telamon_candidate_tree_node_get_height,
	am_telamon_candidate_tree_node_get_num_children,
	am_telamon_candidate_tree_node_get_nth_child)

/* Assigns the fill and stroke color for a candidate depending on its
 * classification at time t */
static void am_telamon_candidate_tree_node_colors(
	const struct am_telamon_candidate_tree_renderer* r,
	const struct am_telamon_candidate* c,
	am_timestamp_t t,
	const struct am_rgba** fill_color,
	const struct am_rgba** stroke_color)
{
	struct am_telamon_candidate_classification cls;

	am_telamon_candidate_classify(c, t, &cls);

	switch(cls.type) {
		case AM_TELAMON_CANDIDATE_TYPE_UNKNOWN_NODE:
			*fill_color = &r->params.color.nodes.unknown.fill;
			*stroke_color = &r->params.color.nodes.unknown.stroke;
			break;
		case AM_TELAMON_CANDIDATE_TYPE_UNEXPLORED:
			if(am_telamon_candidate_is_alive(&cls)) {
				*fill_color = &r->params.color.nodes.unexplored.fill;
				*stroke_color = &r->params.color.nodes.unexplored.stroke;
			} else {
				*fill_color = &r->params.color.nodes.unexplored_deadend.fill;
				*stroke_color = &r->params.color.nodes.unexplored_deadend.stroke;
			}
			break;
		case AM_TELAMON_CANDIDATE_TYPE_INTERNAL_NODE:
			if(am_telamon_candidate_is_alive(&cls)) {
				*fill_color = &r->params.color.nodes.internal_node.fill;
				*stroke_color = &r->params.color.nodes.internal_node.stroke;
			} else {
				*fill_color = &r->params.color.nodes.internal_deadend.fill;
				*stroke_color = &r->params.color.nodes.internal_deadend.stroke;
			}
			break;
		case AM_TELAMON_CANDIDATE_TYPE_ROLLOUT_NODE:
			if(am_telamon_candidate_is_alive(&cls)) {
				*fill_color = &r->params.color.nodes.rollout_node.fill;
				*stroke_color = &r->params.color.nodes.rollout_node.stroke;
			} else {
				*fill_color = &r->params.color.nodes.rollout_deadend.fill;
				*stroke_color = &r->params.color.nodes.rollout_deadend.stroke;
			}
			break;
	}

	if(!am_telamon_candidate_is_unknown_node(&cls) &&
	   am_telamon_candidate_is_implementation(&cls))
	{
		if(am_telamon_candidate_is_alive(&cls)) {
			*fill_color = &r->params.color.nodes.implementation_node.fill;
			*stroke_color = &r->params.color.nodes.implementation_node.stroke;
		} else {
			*fill_color = &r->params.color.nodes.implementation_deadend.fill;
			*stroke_color = &r->params.color.nodes.implementation_deadend.stroke;
		}
	}
}

/* Returns true if the creation timestamp of c is within at least one of the
 * given intervals. */
static inline int am_telamon_candidate_tree_node_in_intervals(
	const struct am_telamon_candidate* c,
	const struct am_interval* intervals,
	size_t num_intervals)
{
	am_timestamp_t first_ts = am_telamon_candidate_first_encounter(c);

	for(size_t i = 0; i < num_intervals; i++)
		if(am_interval_contains_p(&intervals[i], first_ts))
			return 1;

	return 0;
}

static void
am_telamon_candidate_tree_renderer_render_node(
	cairo_t* cr,
	struct am_rect screen_rect,
	double zoom,
	const struct am_recttree_node* rtn,
	void* data)
{
	struct am_telamon_candidate_tree_renderer* r = data;
	const struct am_telamon_candidate_tree_node* n = (const void*)rtn;
	const struct am_rgba* fill_color;
	const struct am_rgba* stroke_color;
	struct am_rect real_rect;
	double corner_radius = 5 * r->node_renderer.zoom;

	if(r->intervals &&
	   !am_telamon_candidate_tree_node_in_intervals(
		   n->candidate, r->intervals, r->num_intervals))
	{
		return;
	}

	real_rect = screen_rect;

	if(real_rect.width < 0.1)
		real_rect.width = 0.1;

	if(real_rect.height < 0.1)
		real_rect.height = 0.1;

	am_telamon_candidate_tree_node_colors(
		r, n->candidate, r->max_interval_end, &fill_color, &stroke_color);

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(fill_color));

	/* Paint small rectangles without rounded corners */
	if(corner_radius < 1)
		cairo_rectangle(cr, AM_RECT_ARGS(real_rect));
	else
		am_rounded_rectangle(cr, &real_rect, corner_radius);

	cairo_fill_preserve(cr);

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(stroke_color));

	if(n->selected) {
		cairo_stroke_preserve(cr);

		fill_color = &r->params.color.nodes.highlighted.fill;
		stroke_color = &r->params.color.nodes.highlighted.stroke;

		cairo_set_source_rgba(cr, AM_PRGBA_ARGS(fill_color));
		cairo_fill_preserve(cr);
		cairo_set_source_rgba(cr, AM_PRGBA_ARGS(stroke_color));
	}

	cairo_stroke(cr);
}

static void
am_telamon_candidate_tree_renderer_render_edge(
	cairo_t* cr,
	struct am_rect screen_rect,
	double zoom,
	const struct am_recttree_node* n,
	void* data)
{
	struct am_telamon_candidate_tree_renderer* r = data;
	struct am_point start = { 0, 0};
	struct am_point end = { 0, 0 };
	const struct am_telamon_candidate_tree_edge* e = (const void*)n;
	double line_width = 2 * r->edge_renderer.zoom;
	struct am_rgba* color;

	/* Only paint edges for node, which are visible */
	if(r->intervals) {
		if(!am_telamon_candidate_tree_node_in_intervals(
			   e->src_node->candidate,
			   r->intervals,
			   r->num_intervals) ||
		   !am_telamon_candidate_tree_node_in_intervals(
			   e->dst_node->candidate,
			   r->intervals,
			   r->num_intervals))
		{
			return;
		}
	}

	if(line_width < 0.1)
		line_width = 0.1;

	if(e->selected)
		color = &r->params.color.edges.highlighted;
	else
		color = &r->params.color.edges.normal;

	switch(e->mode) {
		case AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_UL_LR:
			start.x = screen_rect.x;
			start.y = screen_rect.y;
			end.x = screen_rect.x + screen_rect.width;
			end.y = screen_rect.y + screen_rect.height;
			break;
		case AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_LL_UR:
			start.x = screen_rect.x;
			start.y = screen_rect.y + screen_rect.height;
			end.x = screen_rect.x + screen_rect.width;
			end.y = screen_rect.y;
			break;
		case AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_UR_LL:
			start.x = screen_rect.x + screen_rect.width;
			start.y = screen_rect.y;
			end.x = screen_rect.x;
			end.y = screen_rect.y + screen_rect.height;
			break;
		case AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_LR_UL:
			start.x = screen_rect.x + screen_rect.width;
			start.y = screen_rect.y + screen_rect.height;
			end.x = screen_rect.x;
			end.y = screen_rect.y;
			break;
	}

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(color));
	cairo_set_line_width(cr, line_width);
	cairo_move_to(cr, start.x, start.y);
	cairo_line_to(cr, end.x, end.y);
	cairo_stroke(cr);
}

static void am_telamon_candidate_tree_renderer_reset_data(struct am_telamon_candidate_tree_renderer* r)
{
	free(r->nodes);
	free(r->edges);

	r->nodes = NULL;
	r->edges = NULL;
	r->root_node = NULL;

	am_recttree_renderer_set_recttree(&r->node_renderer, NULL);
	am_recttree_renderer_set_recttree(&r->edge_renderer, NULL);

	r->valid = 0;
}

void am_telamon_candidate_tree_renderer_destroy(
	struct am_telamon_candidate_tree_renderer* r)
{
	am_telamon_candidate_tree_renderer_reset_data(r);
}


static void am_telamon_candidate_tree_renderer_init_nodes(
	struct am_telamon_candidate_tree_renderer* r,
	struct am_telamon_candidate* root,
	size_t num_nodes)
{
	struct am_telamon_candidate_tree_node* n;
	struct am_telamon_candidate* c;
	size_t child_idx = 1;

	if(num_nodes == 0)
		return;

	r->nodes[0].candidate = root;
	r->root_node = &r->nodes[0];

	for(size_t i = 0; i < num_nodes; i++) {
		n = &r->nodes[i];
		n->recttree_node.hyperrect_end = &n->lower_right.x;
		n->recttree_node.kdnode.coordinates = &n->upper_left.x;
		n->selected = 0;

		c = n->candidate;

		if(c->num_children > 0)
			n->children = &r->nodes[child_idx];
		else
			n->children = NULL;

		for(size_t j = 0; j < c->num_children; j++)
			n->children[j].candidate = c->children[j];

		child_idx += c->num_children;
	}
}

void am_telamon_candidate_tree_renderer_init_edge(
	struct am_telamon_candidate_tree_edge* e,
	const struct am_telamon_candidate_tree_node* src,
	const struct am_telamon_candidate_tree_node* dst)
{
	struct am_point start;
	struct am_point end;

	e->recttree_node.kdnode.coordinates = &e->bb_upper_left.x;
	e->recttree_node.hyperrect_end = &e->bb_lower_right.x;

	start.x = (src->upper_left.x + src->lower_right.x) / 2.0;
	start.y = src->lower_right.y;

	end.x = (dst->upper_left.x + dst->lower_right.x) / 2.0;
	end.y = dst->upper_left.y;

	if(start.x <= end.x && start.y <= end.y) {
		e->bb_upper_left = start;
		e->bb_lower_right = end;
		e->mode = AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_UL_LR;
	} else if(start.x <= end.x && start.y > end.y) {
		e->bb_upper_left.x = start.x;
		e->bb_upper_left.y = end.y;
		e->bb_lower_right.x = end.x;
		e->bb_lower_right.y = start.y;
		e->mode = AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_LL_UR;
	} else if(start.x > end.x && start.y <= end.y) {
		e->bb_upper_left.x = end.x;
		e->bb_upper_left.y = start.y;
		e->bb_lower_right.x = start.x;
		e->bb_lower_right.y = end.y;
		e->mode = AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_LL_UR;
	} else if(start.x > end.x && start.y > end.y) {
		e->bb_upper_left = end;
		e->bb_lower_right = start;
		e->mode = AM_TELAMON_CANDIDATE_TREE_EDGE_LINE_MODE_LR_UL;
	}

	e->selected = 0;
	e->src_node = src;
	e->dst_node = dst;
}

static void am_telamon_candidate_tree_renderer_init_edges(
	struct am_telamon_candidate_tree_renderer* r,
	size_t num_nodes)
{
	struct am_telamon_candidate_tree_node* curr_node;
	struct am_telamon_candidate_tree_node* curr_child;
	struct am_telamon_candidate_tree_edge* curr_edge;
	size_t edge_idx = 0;

	for(size_t i = 0; i < num_nodes; i++) {
		curr_node = &r->nodes[i];

		for(size_t j = 0; j < curr_node->candidate->num_children; j++) {
			curr_edge = &r->edges[edge_idx];
			curr_child = &curr_node->children[j];

			am_telamon_candidate_tree_renderer_init_edge(
				curr_edge, curr_node, curr_child);

			edge_idx++;
		}
	}
}

static void am_telamon_candidate_tree_renderer_set_nodebb_ends(
	struct am_telamon_candidate_tree_renderer* r,
	size_t num_nodes)
{
	struct am_telamon_candidate_tree_node* curr_node;

	for(size_t i = 0; i < num_nodes; i++) {
		curr_node = &r->nodes[i];

		curr_node->lower_right.x = curr_node->upper_left.x +
			AM_TELAMON_CANDIDATE_TREE_RENDERER_NODE_WIDTH;

		curr_node->lower_right.y = curr_node->upper_left.y +
			AM_TELAMON_CANDIDATE_TREE_RENDERER_NODE_HEIGHT;
	}
}

/* Set the candidate tree to be rendered by the renderer */
int am_telamon_candidate_tree_renderer_set_root(
	struct am_telamon_candidate_tree_renderer* r,
	struct am_telamon_candidate* root)
{
	size_t num_nodes;
	size_t num_edges;
	size_t tree_depth;
	size_t edge_idx = 0;
	struct am_recttree_node** rtnodes = NULL;
	struct am_recttree_node** rtedges = NULL;
	struct am_telamon_candidate_tree_node* curr_node;
	size_t max_depth = AM_TELAMON_CANDIDATE_TREE_RENDERER_MAX_RECTTREE_DEPTH;

	am_telamon_candidate_tree_renderer_reset_data(r);

	if(!root)
		return 0;

	num_nodes = am_telamon_candidate_tree_count_nodes(root);
	num_edges = num_nodes - 1;

	if(!(r->nodes = calloc(num_nodes, sizeof(*r->nodes))))
		goto out_err;

	if(!(r->edges = calloc(num_nodes, sizeof(*r->edges))))
		goto out_err_edges;

	if(!(rtnodes = calloc(num_nodes, sizeof(*rtnodes))))
		goto out_err_rtnodes;

	if(!(rtedges = calloc(num_nodes, sizeof(*rtedges))))
		goto out_err_rtedges;

	tree_depth = am_telamon_candidate_tree_depth(root);

	am_telamon_candidate_tree_renderer_init_nodes(r, root, num_nodes);

	am_recttree_init(&r->node_rect_tree, 2);
	am_recttree_init(&r->edge_rect_tree, 2);

	am_tree_layout_2d_telamon_candidate_tree(r->nodes, tree_depth, 30, 30);

	/* Tree layout only sets the upper left corner of each node rectangle;
	 * Update the lower right corners of each node */
	am_telamon_candidate_tree_renderer_set_nodebb_ends(r, num_nodes);

	am_telamon_candidate_tree_renderer_init_edges(r, num_nodes);

	/* Collect pointers to each node and edge */
	for(size_t i = 0; i < num_nodes; i++) {
		curr_node = &r->nodes[i];
		rtnodes[i] = &curr_node->recttree_node;

		for(size_t j = 0; j < curr_node->candidate->num_children; j++) {
			rtedges[edge_idx] = &r->edges[edge_idx].recttree_node;
			edge_idx++;
		}
	}

	if(am_recttree_build(&r->node_rect_tree, rtnodes, num_nodes, max_depth))
		goto out_err_all;

	if(am_recttree_build(&r->edge_rect_tree, rtedges, num_edges, max_depth))
		goto out_err_all;

	am_recttree_renderer_set_recttree(&r->node_renderer, &r->node_rect_tree);
	am_recttree_renderer_set_recttree(&r->edge_renderer, &r->edge_rect_tree);

	free(rtnodes);
	free(rtedges);

	r->valid = 1;

	return 0;

out_err_all:
	free(rtedges);
out_err_rtedges:
	free(rtnodes);
out_err_rtnodes:
	r->edges = NULL;
	free(r->edges);
out_err_edges:
	r->nodes = NULL;
	free(r->nodes);
out_err:
	return 1;
}

void am_telamon_candidate_tree_renderer_init(struct am_telamon_candidate_tree_renderer* r)
{
	struct am_telamon_candidate_tree_renderer_params* p = &r->params;

	am_recttree_renderer_init(
		&r->node_renderer,
		am_telamon_candidate_tree_renderer_render_node,
		r);

	r->node_renderer.params.bgcolor = AM_RGBA255(0, 0, 0, 0);

	am_recttree_renderer_init(
		&r->edge_renderer,
		am_telamon_candidate_tree_renderer_render_edge,
		r);

	p->color.background = AM_RGBA255(0xFF, 0xFF, 0xFF, 0xFF);
	p->color.edges.normal = AM_RGBA255(0x00, 0x00, 0x00, 0xFF);
	p->color.edges.highlighted = AM_RGBA255(127, 0x00, 127, 0xFF);

	p->color.nodes.unknown.fill = AM_RGBA255(0, 0, 0, 0);
	p->color.nodes.unknown.stroke = AM_RGBA255(0, 0, 0, 0);

	p->color.nodes.unexplored.fill = AM_RGBA255(0, 0, 0, 100);
	p->color.nodes.unexplored.stroke = AM_RGBA255(0, 0, 0, 100);

	p->color.nodes.unexplored_deadend.fill = AM_RGBA255(173, 113, 113, 255);
	p->color.nodes.unexplored_deadend.stroke = AM_RGBA255(188, 98, 98, 255);

	p->color.nodes.internal_node.fill = AM_RGBA255(129, 153, 255, 255);
	p->color.nodes.internal_node.stroke = AM_RGBA255(31, 76, 255, 255);

	p->color.nodes.rollout_node.fill = AM_RGBA255(180, 212, 255, 255);
	p->color.nodes.rollout_node.stroke = AM_RGBA255(0, 100, 234, 255);

	p->color.nodes.implementation_node.fill = AM_RGBA255(0, 140, 0, 255);
	p->color.nodes.implementation_node.stroke = AM_RGBA255(0, 80, 0, 255);

	p->color.nodes.internal_deadend.fill = AM_RGBA255(255, 132, 132, 255);
	p->color.nodes.internal_deadend.stroke = AM_RGBA255(255, 0, 0, 255);

	p->color.nodes.rollout_deadend.fill = AM_RGBA255(221, 118, 255, 255);
	p->color.nodes.rollout_deadend.stroke = AM_RGBA255(161, 0, 212, 255);

	p->color.nodes.implementation_deadend.fill = AM_RGBA255(255, 111, 0, 255);
	p->color.nodes.implementation_deadend.stroke = AM_RGBA255(168, 74, 0, 255);

	p->color.nodes.highlighted.fill = AM_RGBA255(255, 255, 0, 100);
	p->color.nodes.highlighted.stroke = AM_RGBA255(200, 200, 0, 100);

	r->valid = 0;
	r->nodes = NULL;
	r->edges = NULL;
	r->intervals = NULL;
	r->num_intervals = 0;
	r->max_interval_end = AM_TIMESTAMP_T_MAX;
}

void am_telamon_candidate_tree_renderer_render(
	struct am_telamon_candidate_tree_renderer* r,
	cairo_t* cr)
{
	am_recttree_renderer_render(&r->edge_renderer, cr);
	am_recttree_renderer_render(&r->node_renderer, cr);
}

/* Returns the candidate tree node corresponding to the candidate c. If no such
 * node exists in the candidate tree currently assigned to r, the function
 * returns NULL. */
struct am_telamon_candidate_tree_node*
am_telamon_candidate_tree_renderer_lookup_node(
	struct am_telamon_candidate_tree_renderer* r,
	struct am_telamon_candidate* c)
{
	size_t max_depth = 50;
	struct am_telamon_candidate* stack_path[max_depth];
	struct am_telamon_candidate** lookup_path;
	struct am_telamon_candidate** heap_path = NULL;
	void* tmp;
	size_t lu_idx = 0;
	size_t idx;
	size_t path_size;
	struct am_telamon_candidate_tree_node* curr_node;
	struct am_telamon_candidate_tree_node* ret = NULL;
	struct am_telamon_candidate* curr_cand;

	/* */
	size_t extend = 50;

	if(!r->root_node)
		return NULL;

	if(!c->parent) {
		if(r->root_node->candidate == c)
			return r->root_node;
		else
			return NULL;
	}

	lookup_path = stack_path;
	curr_cand = c;

	/* Work way up to the root and memorize all candidates on the path */
	while(curr_cand) {
		/* If current depth exceeds space in path buffer realloc. If the
		 * current buffer is on the stack, allocate a new one on the
		 * heap, otherwise extend heap buffer. */
		if(lu_idx > max_depth-1) {
			if(am_size_inc_safe(&max_depth, extend))
				goto out_err_free;

			if(am_size_mul_safe(
				   &path_size, max_depth, sizeof(heap_path[0])))
			{
				goto out_err_free;
			}

			if(!heap_path) {
				/* Allocate path on heap */
				if(!(heap_path = am_alloc_array_safe(
					     max_depth, path_size)))
				{
					goto out_err_free;
				}

				memcpy(heap_path, lookup_path, path_size);
			} else {
				if(!(tmp = realloc(heap_path, path_size)))
					goto out_err_free;

				heap_path = tmp;
			}

			lookup_path = heap_path;
		}

		lookup_path[lu_idx++] = curr_cand;
		curr_cand = curr_cand->parent;
	}

	curr_node = r->root_node;

	/* Work the way down from the root to the target candidate and look up
	 * nodes */
	for(size_t i = lu_idx-1; i >= 1; i--) {
		idx = am_telamon_candidate_child_idx(lookup_path[i],
						     lookup_path[i-1]);

		curr_node = &curr_node->children[idx];
	}

	ret = curr_node;

out_err_free:
	free(heap_path);

	return ret;
}

/* Marks a candidate as selected. Returns 0 on success, otherwise 1. */
int am_telamon_candidate_tree_renderer_select(
	struct am_telamon_candidate_tree_renderer* r,
	struct am_telamon_candidate* c)
{
	struct am_telamon_candidate_tree_node* n;

	if((n = am_telamon_candidate_tree_renderer_lookup_node(r, c))) {
		n->selected = 1;
		return 0;
	} else {
		return 1;
	}
}

/* Marks a candidate as not selected. Returns 0 on success, otherwise 1. */
int am_telamon_candidate_tree_renderer_unselect(
	struct am_telamon_candidate_tree_renderer* r,
	struct am_telamon_candidate* c)
{
	struct am_telamon_candidate_tree_node* n;

	if((n = am_telamon_candidate_tree_renderer_lookup_node(r, c))) {
		n->selected = 0;
		return 0;
	} else {
		return 1;
	}
}

/* Returns the candidate at the screen position p. If no candidate is found, the
 * function returns NULL. */
struct am_telamon_candidate*
am_telamon_candidate_tree_renderer_candidate_at(
	struct am_telamon_candidate_tree_renderer* r,
	const struct am_point* p)
{
	struct am_recttree_node* rtn;
	struct am_telamon_candidate_tree_node* ctn;

	if(!r->valid)
		return NULL;

	if(!(rtn = am_recttree_renderer_node_at(&r->node_renderer, p)))
		return NULL;

	ctn = (struct am_telamon_candidate_tree_node*)rtn;

	return ctn->candidate;
}
