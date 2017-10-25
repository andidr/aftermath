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

#include "renderer.h"
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

	r->hierarchy = NULL;
	r->num_visible_lanes = 0;
	r->num_invisible_lanes = 0;
	r->max_visible_lanes = 0;

	r->first_lane.node = NULL;
	r->first_lane.node_index = 0;

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
	if(!am_hierarchy_node_has_children(n) ||
	   am_bitvector_test_bit(&r->collapsed_nodes, node_idx))
	{
		(*lane_num)++;
		return 0;
	}

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
	list_add_tail(&l->list, &r->layers);
	l->renderer = r;

	return 0;
}

/* Removes a timeline rendering layer; Returns 0 on success, otherwise 1. */
int am_timeline_renderer_remove_layer(struct am_timeline_renderer* r,
				       struct am_timeline_render_layer* l)
{
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
		l->type->render(l, cr);
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

	if(am_bitvector_test_bit(&r->collapsed_nodes, node_idx) ||
	   !am_hierarchy_node_has_children(n))
	{
		(*curr_lane)++;
		return;
	}

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
