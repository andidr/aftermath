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

#ifndef AM_TIMELINE_DISCRETE_LAYER_H
#define AM_TIMELINE_DISCRETE_LAYER_H

#include <aftermath/core/statistics/discrete.h>
#include <aftermath/render/timeline/layers/lane.h>

struct am_timeline_discrete_layer;

typedef void (*am_timeline_discrete_layer_stats_subtree_fun_t)(
	struct am_timeline_lane_render_layer*,
	struct am_discrete_stats_by_index*,
	struct am_hierarchy_node*,
	const struct am_interval*);

typedef size_t (*am_timeline_discrete_layer_calculate_index_fun_t)(
	struct am_timeline_discrete_layer*, void*);

typedef void (*am_timeline_discrete_layer_event_render_fun_t)(
	struct am_timeline_discrete_layer* l,
	const struct am_hierarchy_node* hn,
	const struct am_interval* i,
	double lane_width,
	double lane_height,
	cairo_t* cr,
	unsigned int px,
	const struct am_discrete_stats_by_index* px_stats);

/* Renders discrete events (i.e., events that occur at a specific point in time,
 * but that do not have a duration) with an embedded timestamp and an embedded
 * subtype encoded as an index.
 *
 * The interval render layer type is not a specific type, but defines a meta
 * type. Actual types are incarnations of the interval render layer type for a
 * specific event type.
 */

struct am_timeline_render_layer_type*
am_timeline_discrete_layer_instantiate_type_index_member(
	const char* name,
	const char* event_array_type_name,
	size_t element_size,
	off_t timestamp_offset,
	off_t index_offset,
	unsigned int index_bits,
	am_timeline_discrete_layer_event_render_fun_t render_events);

struct am_timeline_render_layer_type*
am_timeline_discrete_layer_instantiate_type_index_fun(
	const char* name,
	const char* event_array_type_name,
	size_t element_size,
	off_t timestamp_offset,
	am_timeline_discrete_layer_calculate_index_fun_t calculate_index,
	am_timeline_discrete_layer_event_render_fun_t render_events);

struct am_timeline_render_layer_type*
am_timeline_discrete_layer_instantiate_type_stats_fun(
	const char* name,
	am_timeline_discrete_layer_stats_subtree_fun_t stats_subtree,
	am_timeline_discrete_layer_event_render_fun_t render_events);

#define AM_TIMELINE_DISCRETE_LAYER(x) \
	((struct am_timeline_discrete_layer*)x)

int
am_timeline_discrete_layer_set_max_index(struct am_timeline_discrete_layer* l,
					 size_t max_idx);

void
am_timeline_discrete_layer_set_extra_data(struct am_timeline_discrete_layer* l,
					  void* extra_data);

void*
am_timeline_discrete_layer_get_extra_data(struct am_timeline_discrete_layer* l);

int am_timeline_discrete_layer_get_dominant_index(
	struct am_timeline_discrete_layer* il,
	struct am_hierarchy_node* hn,
	const struct am_interval* i,
	size_t* index,
	int* index_valid);

#endif
