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

#ifndef AM_TIMELINE_HIERARCHY_LAYER_H
#define AM_TIMELINE_HIERARCHY_LAYER_H

#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/cairo_extras.h>

/* The hierarchy render layer renders the hierarchy associated to a time line as
 * a tree-like structure in the left part of the time line, providing each lane
 * with the name of the associated hierarchy node. Example for a hierarchy of a
 * systems with two NUMA nodes, each with 2 CPUs with 2 SMT cores:
 *
 *      NUMA Node 0     Core 0   SMT 0
 *  O---O---------------O--------O
 *  |   |               |
 *  |   |               |        SMT 1
 *  |   |               +--------O
 *  |   |
 *  |   |               Core 1   SMT 0
 *  |   +---------------O--------O
 *  |                   |
 *  |                   |        SMT 1
 *  |                   +--------O
 *  |
 *  |   NUMA Node 0     Core 0   SMT 0
 *  +---O---------------O--------O
 *      |               |
 *      |               |        SMT 1
 *      |               +--------O
 *      |
 *      |               Core 1   SMT 0
 *      +---------------O--------O
 *                      |
 *                      |        SMT 1
 *                      +--------O
 */

struct am_timeline_hierarchy_layer_params {
	struct {
		/* Colros of the circles representing hierarchy nodes */
		struct am_rgba circle;

		/* Color of the connecting lines between nodes */
		struct am_rgba connection;

		/* Color of the plus sign of collapsed nodes */
		struct am_rgba plus_sign;

		/* Color of labels describing nodes */
		struct am_rgba labels;
	} colors;

	/* Radius of circles representing hierarchy nodes */
	double circle_radius;

	/* Number of horizontal pixels between a node and its children */
	double column_width;

	/* Margin in pixels on the left side for the entire layer */
	double left_margin;

	/* Width in pixels of connecting lines between nodes */
	double connection_width;

	/* Font used to render labels */
	struct {
		/* Font family */
		char* family;

		/* Scaling factor for the font */
		double size;

		/* Top margin in pixels for labels */
		double top_margin;
	} label_font;
};

struct am_timeline_hierarchy_layer {
	struct am_timeline_render_layer super;
	struct am_timeline_hierarchy_layer_params params;
};

enum am_timeline_hierarchy_layer_entity_type {
	AM_TIMELINE_HIERARCHY_LAYER_ENTITY_COLLAPSE_BUTTON,
};

enum am_timeline_hierarchy_layer_button_state {
	AM_TIMELINE_HIERARCHY_LAYER_BUTTON_STATE_COLLAPSED = 0,
	AM_TIMELINE_HIERARCHY_LAYER_BUTTON_STATE_EXPANDED = 1
};

/* Circle of a node */
struct am_timeline_hierarchy_layer_collapse_button {
	struct am_timeline_entity super;
	struct am_hierarchy_node* node;
	unsigned int node_idx;
	enum am_timeline_hierarchy_layer_button_state state;
};

struct am_timeline_render_layer_type*
am_timeline_hierarchy_layer_instantiate_type(void);

#endif
