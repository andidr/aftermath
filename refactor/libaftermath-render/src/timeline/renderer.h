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

#ifndef AFTERMATH_RENDER_TIMELINE_RENDERER_H
#define AFTERMATH_RENDER_TIMELINE_RENDERER_H

#include <aftermath/core/typed_list.h>
#include <aftermath/core/hierarchy.h>
#include <aftermath/core/bitvector.h>
#include "../cairo_extras.h"
#include "layer.h"

enum am_timeline_renderer_lane_mode {
	/* Always use a separate lane for a node */
	AM_TIMELINE_RENDERER_LANE_MODE_ALWAYS_SEPARATE,

	/* Collapse a node's lane with the lane of its first child */
	AM_TIMELINE_RENDERER_LANE_MODE_COLLAPSE_FIRSTCHILD
};

struct am_timeline_renderer {
	/* Width in pixels of the time line */
	unsigned int width;

	/* Height in pixels of the time line */
	unsigned int height;

	/* Height in pixels of a lane displaying the activity of a hierarchy
	 * node */
	double lane_height;

	/* Lane offset in pixels, i.e., by how many pixels the lanes have been
	 * "scrolled down" */
	double lane_offset;

	/* Width in pixels of the left part of the timeline with the description
	 * for each lane */
	unsigned int ydesc_width;

	/* Height in pixels of the bottom part of the timeline with the x
	 * labels */
	unsigned int xdesc_height;

	struct {
		/* Rectangle describing the area of all lanes (i.e., without
		 * legends and axes) */
		struct am_rect lanes;

		/* Legend for vertical axis on the left of the lanes */
		struct am_rect ylegend;

		/* Legend for horizontal axis below the lanes */
		struct am_rect xlegend;
	} rects;

	struct {
		/* Background color rendered below all layers */
		struct am_rgba bg;
	} colors;

	/* List of all render layers */
	struct list_head layers;

	/* Hierarchy whose events are displayed by the time line */
	struct am_hierarchy* hierarchy;

	/* Start and end time stamp for the events to be displayed */
	struct am_interval visible_interval;

	/* Bitvector with one entry per hierarchy node of the displayed
	 * hierarchy, indicating whether the node is collapsed (corresponding
	 * bit is 1) or expanded (corresponding bit is 0). The index of the bit
	 * of a node is defined as the index of the parent node plus the number
	 * of preceding siblings and all of its descendants. The root node has
	 * the index 0.*/
	struct am_bitvector collapsed_nodes;

	/* Number of lanes to be rendered, taking into account collapsed
	 * nodes. */
	unsigned int num_visible_lanes;

	/* Number of lanes at the top */
	unsigned int num_invisible_lanes;

	/* Total number of lanes, just based on the height of the timeline and
	 * the lane height */
	unsigned int max_visible_lanes;

	struct {
		/* First node on the first lane */
		struct am_hierarchy_node* node;

		/* Index of the first node on the first lane */
		unsigned int node_index;
	} first_lane;

	/* Defines how lanes for parents and their children are rendered, e.g.,
	 * if the lane of a parent should be collapsed with the first child. */
	enum am_timeline_renderer_lane_mode lane_mode;
};

#define am_timeline_renderer_for_each_layer(r, layer) \
	am_typed_list_for_each(r, layers, layer, list)

#define am_timeline_renderer_for_each_layer_prev(r, layer) \
	am_typed_list_for_each_prev(r, layers, layer, list)

#define am_timeline_renderer_for_each_layer_safe(r, i, n) \
	am_typed_list_for_each_safe(r, layers, i, n, list)

#define am_timeline_renderer_for_each_layer_prev_safe(r, i, n) \
	am_typed_list_for_each_prev_safe(r, layers, i, n, list)

#define AM_DECL_TIMELINE_RENDERER_SETCOLOR_FUN2(color_name, member)		\
	static inline void							\
	am_timeline_renderer_set_##color_name##_color(struct am_timeline_renderer* r, \
						      double col_r,		\
						      double col_g,		\
						      double col_b,		\
						      double col_a)		\
	{									\
		r->colors.member.r = col_r;					\
		r->colors.member.g = col_g;					\
		r->colors.member.b = col_b;					\
		r->colors.member.a = col_a;					\
	}

#define AM_DECL_TIMELINE_RENDERER_SETCOLOR_FUN(member) \
	AM_DECL_TIMELINE_RENDERER_SETCOLOR_FUN2(member, member)

#define AM_DECL_TIMELINE_RENDERER_SETTER_FUN2(name, member)			\
	static inline void							\
	am_timeline_renderer_set_##name(struct am_timeline_renderer* r,	\
					typeof(r->member) name)		\
	{									\
		r->member = name;						\
	}

#define AM_DECL_TIMELINE_RENDERER_SETTER_FUN(member) \
	AM_DECL_TIMELINE_RENDERER_SETTER_FUN2(member, member)

#define AM_DECL_TIMELINE_RENDERER_GETTER_FUN2(name, member)			\
	static inline typeof(((struct am_timeline_renderer*)0)->member)	\
	am_timeline_renderer_get_##name(struct am_timeline_renderer* r)	\
	{									\
		return r->member;						\
	}

#define AM_DECL_TIMELINE_RENDERER_GETTER_FUN(member) \
	AM_DECL_TIMELINE_RENDERER_GETTER_FUN2(member, member)

#define AM_DECL_TIMELINE_RENDERER_SETTER_FUN_STRUCT2(name, member)		\
	static inline void							\
	am_timeline_renderer_set_##name(struct am_timeline_renderer* r,	\
					const typeof(r->member)* name)		\
	{									\
		r->member = *name;						\
	}

#define AM_DECL_TIMELINE_RENDERER_SETTER_FUN_STRUCT(member) \
	AM_DECL_TIMELINE_RENDERER_SETTER_FUN_STRUCT2(member, member)

#define AM_DECL_TIMELINE_RENDERER_GETTER_FUN_STRUCT2(name, member)	\
	static inline void						\
	am_timeline_renderer_get_##name(struct am_timeline_renderer* r, \
					typeof(r->member)* name)	\
	{								\
		*name = r->member;					\
	}

#define AM_DECL_TIMELINE_RENDERER_GETTER_FUN_STRUCT(member) \
	AM_DECL_TIMELINE_RENDERER_GETTER_FUN_STRUCT2(member, member)

int am_timeline_renderer_init(struct am_timeline_renderer* r);
void am_timeline_renderer_destroy(struct am_timeline_renderer* r);
int am_timeline_renderer_add_layer(struct am_timeline_renderer* r,
				   struct am_timeline_render_layer* l);
int am_timeline_renderer_remove_layer(struct am_timeline_renderer* r,
				      struct am_timeline_render_layer* l);
void am_timeline_renderer_render(struct am_timeline_renderer* r,
				 cairo_t* cr);

int am_timeline_renderer_set_hierarchy(struct am_timeline_renderer* r,
				       struct am_hierarchy* h);

void am_timeline_renderer_set_width(struct am_timeline_renderer* r,
				    unsigned int w);
void am_timeline_renderer_set_height(struct am_timeline_renderer* r,
				     unsigned int h);
void am_timeline_renderer_set_lane_height(struct am_timeline_renderer* r,
					  double h);

void am_timeline_renderer_set_lane_offset(struct am_timeline_renderer* r,
					  double offs);

int am_timeline_renderer_set_horizontal_axis_y(struct am_timeline_renderer* r,
						double y);

int am_timeline_renderer_set_vertical_axis_x(struct am_timeline_renderer* r,
					     double x);

int am_timeline_renderer_collapse_node_idx(struct am_timeline_renderer* r,
					   unsigned int idx);

int am_timeline_renderer_expand_node_idx(struct am_timeline_renderer* r,
					 unsigned int idx);

int am_timeline_renderer_toggle_node_idx(struct am_timeline_renderer* r,
					 unsigned int idx);

int am_timeline_renderer_identify_entities(struct am_timeline_renderer* r,
					   struct list_head* lst,
					   double x, double y);

void am_timeline_renderer_destroy_entities(struct am_timeline_renderer* r,
					   struct list_head* lst);

/* Calculates the X coordinate in pixels for a timestamp t relative to the left
 * of the rectangle for the lanes. */
static inline double
am_timeline_renderer_timestamp_to_relx(struct am_timeline_renderer* r,
				       am_timestamp_t t)
{
	am_timestamp_diff_t d;

	d = am_interval_duration(&r->visible_interval);

	if(d == 0 || r->rects.lanes.width == 0)
		return 0;

	return (double)(((long double)t - (long double)r->visible_interval.start) *
			(long double)r->rects.lanes.width) /
		(long double)d;
}

/* Calculates the X coordinate in pixels for a timestamp t. */
static inline double
am_timeline_renderer_timestamp_to_x(struct am_timeline_renderer* r,
				    am_timestamp_t t)
{
	return r->rects.lanes.x + am_timeline_renderer_timestamp_to_relx(r, t);
}

AM_DECL_TIMELINE_RENDERER_SETCOLOR_FUN2(background, bg)
AM_DECL_TIMELINE_RENDERER_GETTER_FUN(lane_offset)
AM_DECL_TIMELINE_RENDERER_GETTER_FUN(lane_mode)
AM_DECL_TIMELINE_RENDERER_SETTER_FUN_STRUCT(visible_interval)
AM_DECL_TIMELINE_RENDERER_GETTER_FUN_STRUCT(visible_interval)

typedef void (*am_timeline_renderer_lane_fun_t)(struct am_timeline_renderer* r,
						struct am_hierarchy_node* n,
						unsigned int node_idx,
						unsigned int lane,
						void* data);

void am_timeline_renderer_foreach_visible_lane(struct am_timeline_renderer* r,
					       am_timeline_renderer_lane_fun_t cb,
					       void* data);

int am_timeline_renderer_lane_extents(struct am_timeline_renderer* r,
				      struct am_rect* e,
				      unsigned int lane);

int am_timeline_renderer_is_leaf_lane(struct am_timeline_renderer* r,
				      struct am_hierarchy_node* n,
				      unsigned int node_idx);

int am_timeline_renderer_parent_on_same_lane(struct am_timeline_renderer* r,
					     struct am_hierarchy_node* n);

void am_timeline_renderer_set_lane_mode(struct am_timeline_renderer* r,
					enum am_timeline_renderer_lane_mode m);

#endif
