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

#ifndef AM_TIMELINE_SELECTION_LAYER_H
#define AM_TIMELINE_SELECTION_LAYER_H

#include <aftermath/core/base_types.h>
#include <aftermath/render/timeline/layer.h>

struct am_timeline_selection {
	struct am_interval interval;
};

enum am_timeline_selection_layer_entity_type {
	AM_TIMELINE_SELECTION_LAYER_SELECTION,
};

enum am_timeline_selection_layer_selection_part {
	AM_TIMELINE_SELECTION_LAYER_SELECTION_START,
	AM_TIMELINE_SELECTION_LAYER_SELECTION_INTERVAL,
	AM_TIMELINE_SELECTION_LAYER_SELECTION_END
};

struct am_timeline_selection_layer_entity {
	struct am_timeline_entity super;
	enum am_timeline_selection_layer_selection_part type;
	const struct am_timeline_selection* selection;
};

struct am_timeline_selection_layer;

#define AM_TIMELINE_SELECTION_LAYER(x) \
	((struct am_timeline_selection_layer*)x)

struct am_timeline_render_layer_type*
am_timeline_selection_layer_instantiate_type(void);

const struct am_timeline_selection* am_timeline_selection_layer_add_selection(
	struct am_timeline_selection_layer* sl,
	const struct am_interval* i);

const struct am_timeline_selection* am_timeline_selection_layer_update_selection(
	struct am_timeline_selection_layer* sl,
	const struct am_timeline_selection* s,
	const struct am_interval* new_interval);

int am_timeline_selection_layer_delete_selection(
	struct am_timeline_selection_layer* sl,
	const struct am_timeline_selection* s);

size_t am_timeline_selection_layer_get_num_selections(
	const struct am_timeline_selection_layer* sl);

const struct am_timeline_selection*
am_timeline_selection_layer_get_selections(
	const struct am_timeline_selection_layer* sl);

#endif
