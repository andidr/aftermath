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

#ifndef AM_TIMELINE_LANE_LAYER_H
#define AM_TIMELINE_LANE_LAYER_H

/* Meta render layer for timeline lanes. Simplifies rendering of timeline lanes
 * by calculating the offsets and dimensions of each lane and by calling a
 * simplified rendering function. */

#include <aftermath/render/timeline/layer.h>
#include <aftermath/core/hierarchy.h>

struct am_timeline_lane_render_layer_type;

/* Defines how collapsed lanes are rendered */
enum am_timeline_lane_render_mode {
	/* Renders only the events of the collapsed lane itself */
	AM_TIMELINE_LANE_RENDER_MODE_EXCLUSIVE,

	/* Renders the events of all child lanes and the collapsed lane */
	AM_TIMELINE_LANE_RENDER_MODE_COMBINE_SUBTREE,
};

/* Base structure for lane renderer instances */
struct am_timeline_lane_render_layer {
	struct am_timeline_render_layer super;
	enum am_timeline_lane_render_mode render_mode;
};

void am_timeline_lane_render_layer_init(
	struct am_timeline_lane_render_layer* l,
	struct am_timeline_lane_render_layer_type* t);

/* Type of the simplified rendering function. The transformation matric of the
 * cairo context is manipulated prior to the call, such that the upper left
 * corner of the lane to be rendered is at (0, 0). */
typedef void (*am_timeline_lane_render_layer_render_fun_t)(
	struct am_timeline_lane_render_layer* l,
	struct am_hierarchy_node* hn,
	struct am_interval* i,
	double lane_width,
	double lane_height,
	cairo_t* cr);

typedef struct am_timeline_lane_render_layer*(
	*am_timeline_lane_render_layer_instantiate_fun_t)(
		struct am_timeline_lane_render_layer_type* t);

typedef void (*am_timeline_lane_render_layer_destroy_fun_t)(
	struct am_timeline_lane_render_layer* l);

#define AM_TIMELINE_LANE_RENDER_LAYER_RENDER_FUN(x) \
	((am_timeline_lane_render_layer_render_fun_t)(x))

#define AM_TIMELINE_LANE_RENDER_LAYER_INSTANTIATE_FUN(x) \
	((am_timeline_lane_render_layer_instantiate_fun_t)(x))

#define AM_TIMELINE_LANE_RENDER_LAYER_DESTROY_FUN(x) \
	((am_timeline_lane_render_layer_destroy_fun_t)(x))

/* Base structure for lane renderer types */
struct am_timeline_lane_render_layer_type {
	struct am_timeline_render_layer_type super;
	am_timeline_lane_render_layer_render_fun_t render;
	am_timeline_lane_render_layer_instantiate_fun_t instantiate;
	am_timeline_lane_render_layer_destroy_fun_t destroy;
};

#define AM_TIMELINE_LANE_RENDER_LAYER_TYPE(x) \
	((struct am_timeline_lane_render_layer_type*)x)

int am_timeline_lane_render_layer_type_init(
	struct am_timeline_lane_render_layer_type* l,
	const char* name);

#endif
