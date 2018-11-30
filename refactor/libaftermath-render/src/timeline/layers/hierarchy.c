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

#include <aftermath/render/timeline/layers/hierarchy.h>
#include <aftermath/render/timeline/renderer.h>
#include <stdlib.h>
#include <math.h>

static const struct am_timeline_hierarchy_layer_params HIERARCHY_LAYER_DEFAULT_PARAMS = {
	.colors = {
		.circle = {0.8, 0.8, 0.0, 1.0},
		.connection = {0.8, 0.8, 0.0, 1.0},
		.plus_sign = {0.3, 0.3, 0.3, 1.0},
		.labels = {0.8, 0.8, 0.0, 1.0}
	},
	.circle_radius = 5,
	.column_width = 50,
	.left_margin = 3,
	.connection_width = 1,
	.label_font = {
		.size = 12,
		.top_margin = -3,
		.family = NULL
	}
};

/* Calculates the center of a circle for a given lane index and column */
static inline void circle_center(struct am_timeline_hierarchy_layer* hl,
				 unsigned int lane,
				 unsigned int col,
				 struct am_point* p)
{
	struct am_timeline_renderer* r = hl->super.renderer;

	p->x = hl->params.left_margin +
		((double)col + .5) * hl->params.column_width -
		hl->params.circle_radius;

	p->y = (lane + .5) * (double)r->lane_height -
		(r->lane_offset - floor(r->lane_offset  / r->lane_height) *
		 r->lane_height);
}

/* Parent_lane is only valid if parent_visible is true */
typedef void (*visible_node_fun_t)(struct am_timeline_hierarchy_layer* hl,
				   struct am_hierarchy_node* n,
				   unsigned int node_idx,
				   unsigned int lane,
				   unsigned int column,
				   unsigned int parent_lane,
				   int parent_visible,
				   void* data);

/* Lane only valid if visible is true, parent_lane only valid if parent_visible
 * is true */
typedef void (*visible_connection_fun_t)(struct am_timeline_hierarchy_layer* hl,
					 struct am_hierarchy_node* parent,
					 struct am_hierarchy_node* n,
					 int visible,
					 unsigned int lane,
					 unsigned int column,
					 unsigned int parent_lane,
					 int parent_visible,
					 void* data);

/* Function that recursively traverses all visible nodes and connection starting
 * with n.
 *
 * hl: the hierarchy layer that is being rendered
 * n: The current node to be rendered
 * node_idx: Index of n
 * curr_lane: The 0-indexed lane starting from the top of the timeline for n
 * parent_lane: The 0-indexed lane starting from the top of the timeline for the
 *              parent of n
 * column: 0-indexed column of n
 * parent_visible: Indicates whether the parent node is visible
 * node_cb: Callback function called for each visible node
 * connection_cb: Callback function called for each visible connection
 * data: Data to be passed verbatim to the callback functions
 */
static void foreach_visible_down(struct am_timeline_hierarchy_layer* hl,
				 struct am_hierarchy_node* n,
				 unsigned int node_idx,
				 unsigned int* curr_lane,
				 unsigned int parent_lane,
				 unsigned int column,
				 int parent_visible,
				 visible_node_fun_t node_cb,
				 visible_connection_fun_t connection_cb,
				 void* data)
{
	struct am_timeline_renderer* r = hl->super.renderer;
	struct am_hierarchy_node* child;
	unsigned int child_idx = node_idx + 1;
	unsigned int this_lane = *curr_lane;
	int child_visible;

	if(*curr_lane > r->num_visible_lanes)
		return;

	if(node_cb) {
		node_cb(hl,
			n,
			node_idx,
			this_lane,
			column,
			parent_lane,
			parent_visible,
			data);
	}

	if(am_timeline_renderer_is_leaf_lane(r, n, node_idx))
		(*curr_lane)++;

	if(!am_bitvector_test_bit(&r->collapsed_nodes, node_idx)) {
		am_hierarchy_node_for_each_child(n, child) {
			child_visible = (*curr_lane) <= r->num_visible_lanes;

			if(connection_cb) {
				connection_cb(hl,
					      n,
					      child,
					      child_visible,
					      *curr_lane,
					      column + 1,
					      this_lane,
					      1,
					      data);
			}

			foreach_visible_down(hl,
					     child,
					     child_idx,
					     curr_lane,
					     this_lane,
					     column+1,
					     1,
					     node_cb,
					     connection_cb,
					     data);

			child_idx += child->num_descendants + 1;
		}
	}
}

/* Function that recursively traverses all visible nodes and connection starting
 * with the first visible node.
 *
 * hl: the hierarchy layer that is being rendered
 * n: The current node to be rendered
 * node_idx: Index of n
 * calling_child: The child node of the last level of recursion (or null if n is
 *                the first visible node). Only the remaining siblings, starting
 *                with calling_child will be considered.
 * calling_child_idx: Index of the calling child (only valid if calling_child !=
 *                    NULL)
 * curr_lane: The 0-indexed lane starting from the top of the timeline for the
 *            next visible node
 * node_visible: Indicates whether n itself is visible
 * column: 0-indexed column of n
 * node_cb: Callback function called for each visible node
 * connection_cb: Callback function called for each visible connection
 * data: Data to be passed verbatim to the callback functions
 */
static void foreach_visible_up(struct am_timeline_hierarchy_layer* hl,
			       struct am_hierarchy_node* n,
			       unsigned int node_idx,
			       struct am_hierarchy_node* calling_child,
			       unsigned int calling_child_idx,
			       unsigned int* curr_lane,
			       int node_visible,
			       unsigned int column,
			       visible_node_fun_t node_cb,
			       visible_connection_fun_t connection_cb,
			       void* data)
{
	struct am_timeline_renderer* r = hl->super.renderer;
	struct am_hierarchy_node* sibling;
	struct am_hierarchy_node* child;
	struct am_hierarchy_node* parent = n->parent;
	unsigned int parent_idx;
	unsigned int child_idx;
	unsigned int this_lane = 0;
	int parent_visible;
	int child_visible;

	/* The parent is only visible if this node is the same lane as the first
	 * visible node and if the parent is also on the same lane. */
	parent_visible = node_visible &&
		am_timeline_renderer_parent_on_same_lane(r, n);

	/* Count n itself only if it is visible */
	if(node_visible) {
		if(n->parent && connection_cb) {
			connection_cb(hl,
				      n->parent,
				      n,
				      node_visible,
				      this_lane,
				      column,
				      0,
				      parent_visible,
				      data);
		}

		if(node_cb) {
			node_cb(hl,
				n,
				node_idx,
				this_lane,
				column,
				0,
				parent_visible,
				data);
		}

		/* The first child is rendered on the same column, so don't
		 * increase lane number if node has children and is not
		 * collapsed. */
		if(am_timeline_renderer_is_leaf_lane(r, n, node_idx))
			(*curr_lane)++;
	}

	/* Process all children starting with the calling child's successor or
	 * the first child if this is the first visible node. */
	if(*curr_lane <= r->num_visible_lanes &&
	   !am_bitvector_test_bit(&r->collapsed_nodes, node_idx))
	{
		if(calling_child) {
			child_idx = calling_child_idx +
				calling_child->num_descendants + 1;
		} else {
			child_idx = node_idx + 1;
		}

		am_hierarchy_node_for_each_child_start(n, child, calling_child) {
			child_visible = (*curr_lane) < r->num_visible_lanes;

			if(connection_cb) {
				connection_cb(hl,
					      n,
					      child,
					      child_visible,
					      *curr_lane,
					      column + 1,
					      this_lane,
					      node_visible,
					      data);
			}

			foreach_visible_down(hl,
					     child,
					     child_idx,
					     curr_lane,
					     this_lane,
					     column+1,
					     node_visible,
					     node_cb,
					     connection_cb,
					     data);

			child_idx += child->num_descendants + 1;
		}
	}

	/* Render remaining siblings of n */
	if(parent) {
		/* Calculate parent index */
		parent_idx = node_idx - 1;

		am_hierarchy_node_for_each_child_prev_start(parent, sibling, n)
			parent_idx -= sibling->num_descendants + 1;

		foreach_visible_up(hl,
				   n->parent,
				   parent_idx,
				   n,
				   node_idx,
				   curr_lane,
				   parent_visible,
				   column-1,
				   node_cb,
				   connection_cb,
				   data);
	}
}

/* Calls node_cb for each visible node and connection_cb for each visible
 * connection. The data pointer is passed verbatim to the callback functions. */
static void foreach_visible(struct am_timeline_hierarchy_layer* hl,
			    visible_node_fun_t node_cb,
			    visible_connection_fun_t connection_cb,
			    void* data)
{
	struct am_timeline_renderer* r = hl->super.renderer;
	unsigned int first_node_column;
	unsigned int curr_lane = 0;

	if(!r->first_lane.node)
		return;

	first_node_column = am_hierarchy_node_depth(r->first_lane.node);

	foreach_visible_up(hl,
			   r->first_lane.node,
			   r->first_lane.node_index,
			   NULL,
			   0,
			   &curr_lane,
			   1,
			   first_node_column,
			   node_cb,
			   connection_cb,
			   data);
}

/* Visible node callback function for rendering */
static void render_visible_node(struct am_timeline_hierarchy_layer* hl,
				struct am_hierarchy_node* n,
				unsigned int node_idx,
				unsigned int lane,
				unsigned int column,
				unsigned int parent_lane,
				int parent_visible,
				cairo_t* cr)
{
	struct am_timeline_renderer* r = hl->super.renderer;
	struct am_rgba* psc = &hl->params.colors.plus_sign;
	struct am_rgba* cc = &hl->params.colors.circle;
	struct am_rgba* lc = &hl->params.colors.labels;
	struct am_point p;
	cairo_text_extents_t extents;

	circle_center(hl, lane, column, &p);

	/* Draw circle */
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(*cc));
	cairo_arc(cr, p.x, p.y, hl->params.circle_radius, 0, 2*M_PI);
	cairo_fill(cr);

	/* Draw plus sign for if node collapsed */
	if(am_bitvector_test_bit(&r->collapsed_nodes, node_idx) &&
	   am_hierarchy_node_has_children(n))
	{
		cairo_set_source_rgba(cr, AM_RGBA_ARGS(*psc));
		cairo_set_line_width(cr, .2 * hl->params.circle_radius);
		cairo_move_to(cr, p.x - 0.9 * hl->params.circle_radius, p.y);
		cairo_line_to(cr, p.x + 0.9 * hl->params.circle_radius, p.y);
		cairo_move_to(cr, p.x, p.y - 0.9 * hl->params.circle_radius);
		cairo_line_to(cr, p.x, p.y + 0.9 * hl->params.circle_radius);
		cairo_stroke(cr);
	}

	/* Draw label */
	cairo_set_source_rgba(cr, AM_RGBA_ARGS(*lc));
	cairo_text_extents(cr, n->name, &extents);
	cairo_move_to(cr,
		      p.x - (extents.width / 2.0),
		      p.y - hl->params.circle_radius +
		      hl->params.label_font.top_margin);
	cairo_show_text(cr, n->name);
}

/* Visible connection callback function for rendering */
void render_visible_connection(struct am_timeline_hierarchy_layer* hl,
			       struct am_hierarchy_node* parent,
			       struct am_hierarchy_node* n,
			       int visible,
			       unsigned int lane,
			       unsigned int column,
			       unsigned int parent_lane,
			       int parent_visible,
			       cairo_t* cr)
{
	struct am_timeline_renderer* r = hl->super.renderer;
	struct am_rgba* color = &hl->params.colors.connection;
	struct am_point pt;
	struct am_point pt_parent;

	cairo_set_source_rgba(cr, AM_RGBA_ARGS(*color));
	cairo_set_line_width(cr, hl->params.connection_width);

	if(visible && parent_visible) {
		circle_center(hl, lane, column, &pt);
		pt.x -= hl->params.circle_radius;
		circle_center(hl, parent_lane, column-1, &pt_parent);

		if(lane != parent_lane) {
			/* Full L-shape */
			pt_parent.y += hl->params.circle_radius;

			cairo_move_to(cr, AM_POINT_ARGS(pt));
			cairo_line_to(cr, pt_parent.x, pt.y);
			cairo_line_to(cr, AM_POINT_ARGS(pt_parent));
		} else {
			/* Straight horizontal line */
			pt_parent.x += hl->params.circle_radius;
			cairo_move_to(cr, AM_POINT_ARGS(pt));
			cairo_line_to(cr, AM_POINT_ARGS(pt_parent));
		}
	} else if(visible && !parent_visible) {
		/* Full L-shape */
		circle_center(hl, lane, column, &pt);
		pt_parent.x = pt.x - hl->params.column_width;
		pt_parent.y = 0;

		pt.x -= hl->params.circle_radius;

		cairo_move_to(cr, AM_POINT_ARGS(pt));
		cairo_line_to(cr, pt_parent.x, pt.y);
		cairo_line_to(cr, AM_POINT_ARGS(pt_parent));
	} else if(!visible && parent_visible) {
		/* Straight vertical line with defined beginning, open end */
		circle_center(hl, parent_lane, column-1, &pt_parent);

		pt_parent.y += hl->params.circle_radius;

		pt.x = pt_parent.x;
		pt.y = r->rects.ylegend.y + r->rects.ylegend.height;

		cairo_move_to(cr, AM_POINT_ARGS(pt));
		cairo_line_to(cr, AM_POINT_ARGS(pt_parent));
	} else if(!visible && !parent_visible) {
		/* Straight vertical line with open beginning and end */
		circle_center(hl, 0, column-1, &pt);

		pt.y = r->rects.ylegend.y;

		pt_parent.x = pt.x;
		pt_parent.y = r->rects.ylegend.y + r->rects.ylegend.height;

		cairo_move_to(cr, AM_POINT_ARGS(pt));
		cairo_line_to(cr, AM_POINT_ARGS(pt_parent));
	}

	cairo_stroke(cr);
}

static void render(struct am_timeline_hierarchy_layer* hl, cairo_t* cr)
{
	struct am_timeline_renderer* r = hl->super.renderer;

	cairo_rectangle(cr, AM_RECT_ARGS(r->rects.ylegend));
	cairo_clip(cr);

	cairo_select_font_face(cr, hl->params.label_font.family,
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size(cr, hl->params.label_font.size);

	foreach_visible(hl,
			(visible_node_fun_t)render_visible_node,
			(visible_connection_fun_t)render_visible_connection,
			cr);

	cairo_reset_clip(cr);
}

static void destroy(struct am_timeline_hierarchy_layer* hl)
{
	free(hl->params.label_font.family);
}

static struct am_timeline_render_layer*
instantiate(struct am_timeline_render_layer_type* t)
{
	struct am_timeline_hierarchy_layer* l;

	if(!(l = malloc(sizeof(*l))))
		return NULL;

	am_timeline_render_layer_init(&l->super, t);

	l->params = HIERARCHY_LAYER_DEFAULT_PARAMS;

	if(!(l->params.label_font.family = strdup("Sans"))) {
		free(l);
		return NULL;
	}

	return (struct am_timeline_render_layer*)l;
}

struct collapse_button_callback_data {
	/* Status of the last invocation of the callback function */
	int last_status;

	/* List passed to identify_entities() */
	struct list_head* lst;

	/* Location for which item identification takes place */
	struct am_point p;
};

void identify_collapse_buttons_callback(struct am_timeline_hierarchy_layer* hl,
					struct am_hierarchy_node* n,
					unsigned int node_idx,
					unsigned int lane,
					unsigned int column,
					unsigned int parent_lane,
					int parent_visible,
					struct collapse_button_callback_data* data)
{
	struct am_timeline_renderer* r = hl->super.renderer;
	struct am_point pcircle;
	struct am_timeline_hierarchy_layer_collapse_button* btn;
	double distance;

	circle_center(hl, lane, column, &pcircle);

	distance = am_point_distance(&data->p, &pcircle);

	if(distance < hl->params.circle_radius) {
		if(!(btn = malloc(sizeof(*btn)))) {
			data->last_status = 1;
			return;
		}

		am_timeline_entity_init(&btn->super, &hl->super,
			AM_TIMELINE_HIERARCHY_LAYER_ENTITY_COLLAPSE_BUTTON);

		btn->node = n;
		btn->node_idx = node_idx;
		btn->state = (am_bitvector_test_bit(&r->collapsed_nodes, node_idx)) ?
			AM_TIMELINE_HIERARCHY_LAYER_BUTTON_STATE_COLLAPSED :
			AM_TIMELINE_HIERARCHY_LAYER_BUTTON_STATE_EXPANDED;

		am_timeline_entity_append(&btn->super, data->lst);
	}
}

static int identify_collapse_buttons(struct am_timeline_hierarchy_layer* hl,
				     struct list_head* lst,
				     double x, double y)
{
	struct collapse_button_callback_data cbdata = {
		.last_status = 0,
		.lst = lst,
		.p = { .x = x, .y = y }
	};

	foreach_visible(hl,
			(visible_node_fun_t)identify_collapse_buttons_callback,
			NULL,
			&cbdata);

	return cbdata.last_status;
}

static int identify_entities(struct am_timeline_hierarchy_layer* hl,
			     struct list_head* lst,
			     double x, double y)
{
	return identify_collapse_buttons(hl, lst, x, y);
}

static void destroy_entity(struct am_timeline_hierarchy_layer* hl,
			   struct am_timeline_entity* e)
{
	am_timeline_entity_destroy(e);
	free(e);
}

struct am_timeline_render_layer_type*
am_timeline_hierarchy_layer_instantiate_type(void)
{
	struct am_timeline_render_layer_type* t;

	if(!(t = malloc(sizeof(*t))))
		return NULL;

	if(am_timeline_render_layer_type_init(t, "hierarchy")) {
		free(t);
		return NULL;
	}

	t->render = AM_TIMELINE_RENDER_LAYER_RENDER_FUN(render);
	t->destroy = AM_TIMELINE_RENDER_LAYER_DESTROY_FUN(destroy);
	t->instantiate = AM_TIMELINE_RENDER_LAYER_INSTANTIATE_FUN(instantiate);
	t->identify_entities = AM_TIMELINE_RENDER_LAYER_IDENTIFY_ENTITIES_FUN(identify_entities);
	t->destroy_entity = AM_TIMELINE_RENDER_LAYER_DESTROY_ENTITY_FUN(destroy_entity);

	return t;
}
