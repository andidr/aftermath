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

#ifndef AM_TIMELINE_INTERVAL_LAYER_H
#define AM_TIMELINE_INTERVAL_LAYER_H

#include <aftermath/render/timeline/layers/lane.h>
#include <aftermath/render/cairo_extras.h>
#include <aftermath/render/color_map.h>

struct am_timeline_interval_layer;

/* Renders events with an embedded interval and index as colored rectangles of a
 * width of one pixel on the time line, with one color corresponding to each
 * interval index. The color of a rectangle is determined by the interval index
 * that accounts for most of the execution time during the interval of the
 * pixel's rectangle.
 *
 * The interval render layer type is not a specific type, but defines a meta
 * type. Actual types are incarnations of the interval render layer type for a
 * specific event type (e.g., state events).
 */

struct am_timeline_render_layer_type*
am_timeline_interval_layer_instantiate_type_index_member(
	const char* name,
	const char* event_array_type_name,
	size_t element_size,
	off_t interval_offset,
	off_t index_offset,
	unsigned int index_bits);

struct am_timeline_render_layer_type*
am_timeline_interval_layer_instantiate_type_index_fun(
	const char* name,
	const char* event_array_type_name,
	size_t element_size,
	off_t interval_offset,
	size_t (*calculate_index)(struct am_timeline_interval_layer*, void*));

#define AM_TIMELINE_INTERVAL_LAYER(x) \
	((struct am_timeline_interval_layer*)x)

void
am_timeline_interval_layer_set_color_map(struct am_timeline_interval_layer* l,
					 const struct am_color_map* color_map);

int
am_timeline_interval_layer_set_max_index(struct am_timeline_interval_layer* l,
					 size_t max_idx);

void
am_timeline_interval_layer_set_extra_data(struct am_timeline_interval_layer* l,
					 void* extra_data);

void*
am_timeline_interval_layer_get_extra_data(struct am_timeline_interval_layer* l);

int am_timeline_interval_layer_get_dominant_index(
	struct am_timeline_interval_layer* il,
	struct am_hierarchy_node* hn,
	const struct am_interval* i,
	size_t* index,
	int* index_valid);

#endif
