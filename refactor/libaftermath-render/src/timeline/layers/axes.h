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

#ifndef AM_TIMELINE_AXES_LAYER_H
#define AM_TIMELINE_AXES_LAYER_H

#include "../layer.h"

/* The axes layer renders vertical lines for the horizontal and vertical axis
 * and displays the labels for the horizontal axis. */

enum am_timeline_axes_layer_entity_type {
	AM_TIMELINE_AXES_LAYER_ENTITY_AXIS,
};

enum am_timeline_axes_layer_axis_type {
	AM_TIMELINE_AXES_LAYER_AXIS_TYPE_HORIZONTAL,
	AM_TIMELINE_AXES_LAYER_AXIS_TYPE_VERTICAL
};

struct am_timeline_axes_layer_axis {
	struct am_timeline_entity super;
	enum am_timeline_axes_layer_axis_type type;
};

struct am_timeline_render_layer_type*
am_timeline_axes_layer_instantiate_type(void);

#endif
