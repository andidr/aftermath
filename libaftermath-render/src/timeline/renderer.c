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

#include <aftermath/render/timeline/renderer.h>
#include <aftermath/core/interval.h>
#include <aftermath/core/aux.h>

static inline void
am_timeline_renderer_update_rects(struct am_timeline_renderer* r);

static void
am_timeline_renderer_update_num_visible_lanes(struct am_timeline_renderer* r);

static inline void
am_timeline_renderer_update_first_visible_lane(struct am_timeline_renderer* r);

int am_timeline_renderer_init(struct am_timeline_renderer* r)
{
	INIT_LIST_HEAD(&r->layers);
	r->colors.bg = (struct am_rgba){ 0.0, 0.0, 0.0, 1.0 };

	r->lane_height = 50;
	r->lane_offset = 0;

	r->width = 0;
	r->height = 0;
	r->xdesc_height = 30;
	r->ydesc_width = 200;

	r->visible_interval.start = 0;
	r->visible_interval.end = 0;

	am_timeline_renderer_update_rects(r);

	/* Number of nodes unknown; initialize bitvector with a single chunk*/
	if(am_bitvector_init(&r->collapsed_nodes, AM_BV_BITS_PER_CHUNK))
		return 1;

	r->trace = NULL;
	r->hierarchy = NULL;
	r->num_visible_lanes = 0;
	r->num_invisible_lanes = 0;
	r->max_visible_lanes = 0;

	r->first_lane.node = NULL;
	r->first_lane.node_index = 0;

	r->lane_mode = AM_TIMELINE_RENDERER_LANE_MODE_COLLAPSE_FIRSTCHILD;

	INIT_LIST_HEAD(&r->layer_appearance_change_callbacks);

	return 0;
}

/* Recursive function for the update of the data for the first visible lane */
static unsigned int
am_timeline_renderer_update_first_visible_lane_rec(
	struct am_timeline_renderer* r,
	struct am_hierarchy_node* n,
	unsigned int node_idx,
	unsigned int* lane_num)
{
	struct am_hierarchy_node* child;

	if(*lane_num >= r->num_invisible_lanes) {
		/* Node found */
		r->first_lane.node = n;
		r->first_lane.node_index = node_idx;

		return 1;
	}

	/* Current node in invisible region at the top */
	if(am_timeline_renderer_is_leaf_lane(r, n, node_idx))
		(*lane_num)++;

	if(am_bitvector_test_bit(&r->collapsed_nodes, node_idx))
		return 0;

	am_hierarchy_node_for_each_child(n, child) {
		if(am_timeline_renderer_update_first_visible_lane_rec(r,
								      child,
								      node_idx+1,
								      lane_num))
		{
			return 1;
		}

		node_idx += child->num_descendants + 1;
	}

	return 0;
}

/* Updates the index of the first visible and last visible lanes */
static inline void
am_timeline_renderer_update_first_visible_lane(struct am_timeline_renderer* r)
{
	unsigned int lane_num = 0;

	r->first_lane.node = NULL;

	if(!r->hierarchy || !r->hierarchy->root)
		return;

	am_timeline_renderer_update_first_visible_lane_rec(r,
							   r->hierarchy->root,
							   0,
							   &lane_num);
}

/* Update rectangles for timeline regions */
static inline void
am_timeline_renderer_update_rects(struct am_timeline_renderer* r)
{
	r->rects.lanes.x = r->ydesc_width + .5;
	r->rects.lanes.y = 0.5;
	r->rects.lanes.width = (r->width > r->ydesc_width) ?
		r->width - r->ydesc_width :
		0;
	r->rects.lanes.height = (r->height > r->xdesc_height) ?
		r->height - r->xdesc_height :
		0;

	r->rects.ylegend.x = 0;
	r->rects.ylegend.y = 0;
	r->rects.ylegend.width = r->ydesc_width;
	r->rects.ylegend.height = r->rects.lanes.height;

	r->rects.xlegend.x = r->rects.lanes.x;
	r->rects.xlegend.y = r->rects.lanes.y + r->rects.lanes.height;
	r->rects.xlegend.width = r->rects.lanes.width;
	r->rects.xlegend.height = (r->height > r->xdesc_height) ?
		r->xdesc_height :
		r->height;
}

void am_timeline_renderer_destroy(struct am_timeline_renderer* r)
{
	struct am_timeline_render_layer* l;
	struct am_timeline_render_layer* n;

	am_timeline_renderer_for_each_layer_safe(r, l, n) {
		am_timeline_render_layer_destroy(l);
		free(l);
	}

	am_bitvector_destroy(&r->collapsed_nodes);
}

/* Adds a timeline rendering layer. Returns 0 on success, otherwise 1. */
int am_timeline_renderer_add_layer(struct am_timeline_renderer* r,
				   struct am_timeline_render_layer* l)
{
	if(l->type->renderer_changed)
		if(l->type->renderer_changed(l, r))
			return 1;

	list_add_tail(&l->list, &r->layers);
	l->renderer = r;

	return 0;
}

/* Removes a timeline rendering layer; Returns 0 on success, otherwise 1. */
int am_timeline_renderer_remove_layer(struct am_timeline_renderer* r,
				       struct am_timeline_render_layer* l)
{
	if(l->type->renderer_changed)
		if(l->type->renderer_changed(l, NULL))
			return 1;

	list_del(&l->list);

	return 0;
}

/* Renders all layers in order */
void am_timeline_renderer_render(struct am_timeline_renderer* r,
				 cairo_t* cr)
{
	struct am_timeline_render_layer* l;

	if(r->width == 0 || r->height == 0)
		return;

	cairo_set_source_rgba(cr, AM_PRGBA_ARGS(&r->colors.bg));

	cairo_rectangle(cr, 0, 0, r->width, r->height);
	cairo_fill(cr);

	am_timeline_renderer_for_each_layer(r, l)
		if(l->enabled)
			l->type->render(l, cr);
}

/* Associate a trace with the timeline. Returns 0 on success, 1 otherwise. */
int am_timeline_renderer_set_trace(struct am_timeline_renderer* r,
				   struct am_trace* t)
{
	struct am_timeline_render_layer* l;

	r->trace = t;

	am_timeline_renderer_for_each_layer(r, l) {
		if(l->type->trace_changed) {
			if(l->type->trace_changed(l, t)) {
				am_timeline_renderer_set_trace(r, NULL);
				r->trace = NULL;

				return 1;
			}
		}
	}

	return 0;
}

/* Associate a hierarchy with the timeline */
int am_timeline_renderer_set_hierarchy(struct am_timeline_renderer* r,
				       struct am_hierarchy* h)
{
	if(h && h->root) {
		if(am_bitvector_resize(&r->collapsed_nodes,
				       h->root->num_descendants + 1,
				       0))
		{
			return 1;
		}
	}

	am_bitvector_clear(&r->collapsed_nodes);

	r->hierarchy = h;

	am_timeline_renderer_update_num_visible_lanes(r);
	am_timeline_renderer_update_first_visible_lane(r);

	return 0;
}

/* Sets the width in pixels of the timeline */
void am_timeline_renderer_set_width(struct am_timeline_renderer* r,
				    unsigned int w)
{
	r->width = w;
	am_timeline_renderer_update_rects(r);
}

/* Sets the height in pixels of the timeline */
void am_timeline_renderer_set_height(struct am_timeline_renderer* r,
				     unsigned int h)
{
	r->height = h;

	am_timeline_renderer_update_rects(r);
	am_timeline_renderer_update_num_visible_lanes(r);
}

void am_timeline_renderer_set_lane_offset(struct am_timeline_renderer* r,
					  double offs)
{
	if(offs < 0)
		offs = 0;

	r->lane_offset = offs;

	am_timeline_renderer_update_num_visible_lanes(r);
	am_timeline_renderer_update_first_visible_lane(r);
}

/* Sets the height in pixels of a timeline lane */
void am_timeline_renderer_set_lane_height(struct am_timeline_renderer* r,
					  double h)
{
	if(h > 0) {
		r->lane_height = h;

		am_timeline_renderer_update_num_visible_lanes(r);
		am_timeline_renderer_update_first_visible_lane(r);
	}
}

/* Calculates the number of lanes for the sub-hierarchy rooted at n, taking into
 * account collapsed nodes. */
static void
am_timeline_renderer_num_visible_lanes_rec(struct am_timeline_renderer* r,
					   struct am_hierarchy_node* n,
					   unsigned int node_idx,
					   unsigned int* curr_lane)
{
	struct am_hierarchy_node* child;

	if(am_timeline_renderer_is_leaf_lane(r, n, node_idx))
		(*curr_lane)++;

	if(am_bitvector_test_bit(&r->collapsed_nodes, node_idx))
		return;

	am_hierarchy_node_for_each_child(n, child) {
		if(*(curr_lane) >= r->num_invisible_lanes + r->max_visible_lanes)
			return;

		am_timeline_renderer_num_visible_lanes_rec(r,
							   child,
							   node_idx+1,
							   curr_lane);

		node_idx += child->num_descendants + 1;
	}
}

/* Updates the number of lanes, taking into account the current lane offset and
 * collapsed nodes */
static void
am_timeline_renderer_update_num_visible_lanes(struct am_timeline_renderer* r)
{
	struct am_hierarchy_node* root;
	unsigned int curr_lane = 0;
	double lane_top_visible;

	r->num_invisible_lanes = r->lane_offset / r->lane_height;

	/* Number of pixels of the first visible line */
	lane_top_visible = ((r->num_invisible_lanes+1) * r->lane_height) -
		r->lane_offset;

	if(lane_top_visible > 0) {
		r->max_visible_lanes = AM_DIV_ROUND_UP(r->height - lane_top_visible,
						       r->lane_height) + 1;
	} else {
		r->max_visible_lanes = AM_DIV_ROUND_UP(r->height, r->lane_height);
	}

	if(r->hierarchy && r->hierarchy->root) {
		root = r->hierarchy->root;
		am_timeline_renderer_num_visible_lanes_rec(r, root, 0, &curr_lane);
	}

	if(curr_lane >= r->num_invisible_lanes)
		r->num_visible_lanes = curr_lane - r->num_invisible_lanes;
	else
		r->num_visible_lanes = 0;
}

/* Marks the node with the index idx as collapsed. Returns 1 if the index is out
 * of bounds, otherwise 0. */
int am_timeline_renderer_collapse_node_idx(struct am_timeline_renderer* r,
					   unsigned int idx)
{
	if(idx >= r->collapsed_nodes.max_bits)
		return 1;

	am_bitvector_set_bit(&r->collapsed_nodes, idx);
	am_timeline_renderer_update_num_visible_lanes(r);
	am_timeline_renderer_update_first_visible_lane(r);

	return 0;
}

/* Marks the node with the index idx as expanded. Returns 1 if the index is out
 * of bounds, otherwise 0. */
int am_timeline_renderer_expand_node_idx(struct am_timeline_renderer* r,
					 unsigned int idx)
{
	if(idx >= r->collapsed_nodes.max_bits)
		return 1;

	am_bitvector_set_bit(&r->collapsed_nodes, idx);
	am_timeline_renderer_update_num_visible_lanes(r);
	am_timeline_renderer_update_first_visible_lane(r);

	return 0;
}

/* Marks the node with the index idx as collapsed if it is expanded and vice
 * versa. Returns 1 if the index is out of bounds, otherwise 0. */
int am_timeline_renderer_toggle_node_idx(struct am_timeline_renderer* r,
					 unsigned int idx)
{
	if(idx >= r->collapsed_nodes.max_bits)
		return 1;

	am_bitvector_toggle_bit(&r->collapsed_nodes, idx);
	am_timeline_renderer_update_num_visible_lanes(r);

	return 0;
}

/* Identifies the set of entities at pixel position (x, y) by traversing all
 * layers. Initializes *e with a pointer to the first entity identified by the
 * topmost layer (i.e., the layer rendered after all other layers). Entities are
 * chained using the field "list" and are appended to *lst. Returns 0 on success
 * (i.e., even if nothing failed, but no entities were identified) or 1 in case
 * of an error. */
int am_timeline_renderer_identify_entities(struct am_timeline_renderer* r,
					   struct list_head* lst,
					   double x, double y)
{
	struct am_timeline_render_layer* l;

	INIT_LIST_HEAD(lst);

	am_timeline_renderer_for_each_layer_prev(r, l) {
		if(!l->type->identify_entities)
			continue;

		if(l->type->identify_entities(l, lst, x, y)) {
			am_timeline_renderer_destroy_entities(r, lst);
			return 1;
		}
	}

	return 0;
}

/* Destroys a list of timeline entities by invoking the entity destructor of the
 * layers associated to the entities. Memory for entities is freed. */
void am_timeline_renderer_destroy_entities(struct am_timeline_renderer* r,
					   struct list_head* lst)
{
	struct am_timeline_render_layer_type* t;
	struct am_timeline_render_layer* l;
	struct am_timeline_entity* e;
	struct am_timeline_entity* i;

	am_typed_list_for_each_safe_genentry(lst, e, i, list) {
		l = e->layer;
		t = l->type;
		t->destroy_entity(l, e);
	}

	INIT_LIST_HEAD(lst);
}

/* Sets the Y position of the horizontal axis. If the position cannot be set, to
 * the exact Y coordinate, the function sets the minimum or maximum,
 * respectively and returns 1. Otherwise, 0 is returned.
 */
int am_timeline_renderer_set_horizontal_axis_y(struct am_timeline_renderer* r,
					       double y)
{
	int ret = 0;

	if(y < 1) {
		y = 1;
		ret = 1;
	} else if(y > r->height - 1) {
		y = r->height - 1;
		ret = 1;
	}

	r->rects.lanes.height = y - r->rects.lanes.y;
	r->rects.xlegend.y = y;
	r->xdesc_height = r->height - y;
	r->rects.xlegend.height = r->xdesc_height;
	r->rects.ylegend.height = y;

	am_timeline_renderer_update_num_visible_lanes(r);

	return ret;
}

/* Sets the X position of the horizontal axis. If the position cannot be set, to
 * the exact X coordinate, the function sets the minimum or maximum,
 * respectivelx and returns 1. Otherwise, 0 is returned.
 */
int am_timeline_renderer_set_vertical_axis_x(struct am_timeline_renderer* r,
					     double x)
{
	int ret = 0;

	if(x < 1) {
		x = 1;
		ret = 1;
	} else if(x > r->width - 1) {
		x = r->width - 1;
		ret = 1;
	}

	r->rects.lanes.width = r->width - x;
	r->rects.lanes.x = x;
	r->rects.ylegend.width = x;
	r->ydesc_width = r->rects.ylegend.width;
	r->rects.xlegend.x = r->rects.lanes.x;
	r->rects.xlegend.width = r->rects.lanes.width;

	am_timeline_renderer_update_num_visible_lanes(r);

	return ret;
}

/* Function that recursively traverses all visible lanes starting with n.
 *
 * r: the timeline renderer
 * n: The current node
 * node_idx: Index of n
 * curr_lane: The 0-indexed lane starting from the top of the timeline for n
 * cb: Callback function called for each visible lane
 * data: Data to be passed verbatim to the callback functions
 *
 * Returns 1 if an invocation of the callback function indicated to stop
 * iterating), otherwise 0.
 */
static int am_timeline_renderer_foreach_visible_lane_down(
	struct am_timeline_renderer* r,
	struct am_hierarchy_node* n,
	unsigned int node_idx,
	unsigned int* curr_lane,
	am_timeline_renderer_lane_fun_t cb,
	void* data)
{
	struct am_hierarchy_node* child;
	unsigned int child_idx = node_idx + 1;
	unsigned int this_lane = *curr_lane;

	if(*curr_lane > r->num_visible_lanes)
		return 0;

	if(!am_hierarchy_node_has_children(n) ||
	   am_bitvector_test_bit(&r->collapsed_nodes, node_idx))
	{
		if(cb(r, n, node_idx, this_lane, data) ==
		   AM_TIMELINE_RENDERER_LANE_CALLBACK_STATUS_STOP)
		{
			return 1;
		}
	}

	if(!am_bitvector_test_bit(&r->collapsed_nodes, node_idx)) {
		am_hierarchy_node_for_each_child(n, child) {
			if(am_timeline_renderer_foreach_visible_lane_down(
				   r,
				   child,
				   child_idx,
				   curr_lane,
				   cb,
				   data))
			{
				return 1;
			}

			child_idx += child->num_descendants + 1;
		}
	}

	if(!am_hierarchy_node_has_children(n) ||
	   am_bitvector_test_bit(&r->collapsed_nodes, node_idx))
	{
		(*curr_lane)++;
	}

	return 0;
}

/* Function that recursively traverses all visible lanes, starting with the lane
 * of the first visible node.
 *
 * r: the timeline renderer
 * n: The current node
 * node_idx: Index of n
 * calling_child: The child node of the last level of recursion (or null if n is
 *                the first visible node). Only the remaining siblings, starting
 *                with calling_child will be considered.
 * calling_child_idx: Index of the calling child (only valid if calling_child !=
 *                    NULL)
 * curr_lane: The 0-indexed lane starting from the top of the timeline for the
 *            next visible node
 * lane_visible: Indicates whether the lane of n itself is visible
 * cb: Callback function called for each visible lane
 * data: Data to be passed verbatim to the callback function
 *
 * Returns 1 if an invocation of the callback function indicated to stop
 * iterating), otherwise 0.
 */
static int am_timeline_renderer_foreach_visible_lane_up(
	struct am_timeline_renderer* r,
	struct am_hierarchy_node* n,
	unsigned int node_idx,
	struct am_hierarchy_node* calling_child,
	unsigned int calling_child_idx,
	unsigned int* curr_lane,
	int lane_visible,
	am_timeline_renderer_lane_fun_t cb,
	void* data)
{
	struct am_hierarchy_node* sibling;
	struct am_hierarchy_node* child;
	struct am_hierarchy_node* parent = n->parent;
	unsigned int parent_idx;
	unsigned int child_idx;
	int parent_lane_visible;

	/* The parent lane is only visible if identical with the lane of the
	 * first visible node. */
	parent_lane_visible = n->parent &&
		lane_visible &&
		am_hierarchy_node_is_first_child(parent, n);

	/* Count this lane only if it is visible and if this is the first
	 * invocation for this lane. */
	if(lane_visible) {
		if(!calling_child &&
		   (!am_hierarchy_node_has_children(n) ||
		    am_bitvector_test_bit(&r->collapsed_nodes, node_idx)))
		{
			if(cb(r, n, node_idx, *curr_lane, data) ==
			   AM_TIMELINE_RENDERER_LANE_CALLBACK_STATUS_STOP)
			{
				return 1;
			}
		}

		/* The first child is rendered on the same column, so don't
		 * increase lane number if node has children and is not
		 * collapsed. */
		if(!am_hierarchy_node_has_children(n) ||
		   am_bitvector_test_bit(&r->collapsed_nodes, node_idx))
		{
			(*curr_lane)++;
		}
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
			if(am_timeline_renderer_foreach_visible_lane_down(
				   r,
				   child,
				   child_idx,
				   curr_lane,
				   cb,
				   data))
			{
				return 1;
			}

			child_idx += child->num_descendants + 1;
		}
	}

	/* Render remaining siblings of n */
	if(parent) {
		/* Calculate parent index */
		parent_idx = node_idx - 1;

		am_hierarchy_node_for_each_child_prev_start(parent, sibling, n)
			parent_idx -= sibling->num_descendants + 1;

		if(am_timeline_renderer_foreach_visible_lane_up(
			   r,
			   n->parent,
			   parent_idx,
			   n,
			   node_idx,
			   curr_lane,
			   parent_lane_visible,
			   cb,
			   data))
		{
			return 1;
		}
	}

	return 0;
}

/* Calls cb for each visible lane. The data pointer is passed verbatim to the
 * callback function.
 *
 * Returns 1 if an invocation of the callback function has indicated that
 * processing should be stopped.
 */
int am_timeline_renderer_foreach_visible_lane(struct am_timeline_renderer* r,
					      am_timeline_renderer_lane_fun_t cb,
					      void* data)
{
	unsigned int curr_lane = 0;

	if(!r->first_lane.node)
		return 0;

	if(am_timeline_renderer_foreach_visible_lane_up(r,
							r->first_lane.node,
							r->first_lane.node_index,
							NULL,
							0,
							&curr_lane,
							1,
							cb,
							data))
	{
		return 1;
	}

	return 0;
}

/* Returns the coordinates of the lane-th lane in e. If the lane is invalid, the
 * function returns 1, otherwise 0. */
int am_timeline_renderer_lane_extents(struct am_timeline_renderer* r,
				      struct am_rect* e,
				      unsigned int lane)
{
	double lane_mod;

	if(lane >= r->num_visible_lanes)
		return 1;

	lane_mod = r->lane_offset - (r->num_invisible_lanes * r->lane_height);

	e->x = r->rects.lanes.x;
	e->y = ((double)(lane * r->lane_height)) - lane_mod;
	e->height = r->lane_height;
	e->width = r->rects.lanes.width;

	return 0;
}

/* Returns true if the events of n should be rendered on their own lane or if
 * the events of n and the events of *all* of its children should be rendered on
 * the same lane. In all other cases, the function returns false. */
int am_timeline_renderer_is_leaf_lane(struct am_timeline_renderer* r,
				      struct am_hierarchy_node* n,
				      unsigned int node_idx)
{
	switch(r->lane_mode) {
		case AM_TIMELINE_RENDERER_LANE_MODE_ALWAYS_SEPARATE:
			return 1;

		case AM_TIMELINE_RENDERER_LANE_MODE_COLLAPSE_FIRSTCHILD:
			return !am_hierarchy_node_has_children(n) ||
				am_bitvector_test_bit(&r->collapsed_nodes, node_idx);
	}

	return 0;
}

/* Returns true if the events of the parent of n should be rendered on the same
 * lane as n. Otherwise returns false. */
int am_timeline_renderer_parent_on_same_lane(struct am_timeline_renderer* r,
					     struct am_hierarchy_node* n)
{
	switch(r->lane_mode) {
		case AM_TIMELINE_RENDERER_LANE_MODE_ALWAYS_SEPARATE:
			return 0;

		case AM_TIMELINE_RENDERER_LANE_MODE_COLLAPSE_FIRSTCHILD:
			return n->parent &&
				am_hierarchy_node_is_first_child(n->parent, n);
	}

	return 0;
}

/* Set the lane mode of the renderer r */
void am_timeline_renderer_set_lane_mode(struct am_timeline_renderer* r,
					enum am_timeline_renderer_lane_mode m)
{
	r->lane_mode = m;

	am_timeline_renderer_update_num_visible_lanes(r);
	am_timeline_renderer_update_first_visible_lane(r);
}

/* Sets *lane to the lane index corresponding to the on-screen position y.
 *
 * Returns 0 on success, otherwise 1 (e.g., if y past the last lane).
 */
int am_timeline_renderer_lane_at_y(struct am_timeline_renderer* r,
				   double y,
				   unsigned int* lane)
{
	/* Y coordinate if all lanes were visible */
	double abs_y;

	/* Lane index if all lanes were visible */
	unsigned int abs_lane;

	/* Real lane index on visible lanes */
	unsigned int rel_lane;

	abs_y = (y - r->rects.lanes.y) + r->lane_offset;

	/* Way above the lane rect? */
	if(abs_y < 0)
		return 1;

	abs_lane = (unsigned int)(abs_y / r->lane_height);

	/* On invisible lane? */
	if(abs_lane < r->num_invisible_lanes)
		return 1;

	rel_lane = abs_lane - r->num_invisible_lanes;

	/* Below last visible lane? */
	if(rel_lane >= r->num_visible_lanes)
		return 1;

	*lane = rel_lane;

	return 0;
}

/* Data for am_timeline_renderer_hierarchy_node_at_y_cb, associating the node
 * with a lane */
struct node_at_callback_data {
	unsigned int lane;
	struct am_hierarchy_node* node;
};

/* Callback function for am_timeline_renderer_hierarchy_node_at_y */
enum am_timeline_renderer_lane_callback_status
am_timeline_renderer_hierarchy_node_at_y_cb(
	struct am_timeline_renderer* r,
	struct am_hierarchy_node* n,
	unsigned int node_idx,
	unsigned int lane,
	void* data)
{
	struct node_at_callback_data* d = data;

	if(lane == d->lane) {
		d->node = n;
		return AM_TIMELINE_RENDERER_LANE_CALLBACK_STATUS_STOP;
	}

	return AM_TIMELINE_RENDERER_LANE_CALLBACK_STATUS_CONTINUE;
}

/* Returns the hierarchy node associated to the lane at position (x, y). If */
struct am_hierarchy_node*
am_timeline_renderer_hierarchy_node_at_y(struct am_timeline_renderer* r,
					 double y)
{
	struct node_at_callback_data data = {
		.node = NULL
	};

	if(am_timeline_renderer_lane_at_y(r, y, &data.lane))
		return NULL;

	am_timeline_renderer_foreach_visible_lane(
		r,
		am_timeline_renderer_hierarchy_node_at_y_cb,
		&data);

	return data.node;
}

/* Registers a callback function cbfe for layer appearance changes with the
 * renderer r. The object pointed to by cbfe must remain live until unregistered
 * or until the renderer is destroyed. */
void am_timeline_renderer_register_layer_appearance_change_callback(
	struct am_timeline_renderer* r,
	struct am_timeline_renderer_layer_appearance_change_callback* cbfe)
{
	list_add_tail(&cbfe->list, &r->layer_appearance_change_callbacks);
}

/* Unregisters a callback function cbfe for layer appearance changes from the
 * renderer r. */
void am_timeline_renderer_unregister_layer_appearance_change_callback(
	struct am_timeline_renderer* r,
	struct am_timeline_renderer_layer_appearance_change_callback* cbfe)
{
	list_del(&r->layer_appearance_change_callbacks);
}

/* Notifies the renderer r that the layer l has changed its appearance. Invokes
 * all callback function registered with
 * am_timeline_renderer_register_layer_appearance_change_callback.
 */
void am_timeline_renderer_indicate_layer_appearance_change(
	struct am_timeline_renderer* r,
	struct am_timeline_render_layer* l)
{
	struct am_timeline_renderer_layer_appearance_change_callback* cbfe;

	am_typed_list_for_each_genentry(
		&r->layer_appearance_change_callbacks,
		cbfe, list)
	{
		cbfe->callback(r, l, cbfe->data);
	}
}
