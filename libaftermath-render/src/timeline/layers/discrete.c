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

#include <aftermath/render/timeline/layers/discrete.h>
#include <aftermath/render/timeline/renderer.h>

struct am_timeline_discrete_layer {
	struct am_timeline_lane_render_layer super;
	struct am_discrete_stats_by_index statistics;
	int statistics_init;
	void* extra_data;
};

struct am_timeline_discrete_layer_type {
	struct am_timeline_lane_render_layer_type super;
	char* event_array_type_name;
	size_t element_size;
	off_t timestamp_offset;
	off_t index_offset;
	unsigned int index_bits;

	am_timeline_discrete_layer_stats_subtree_fun_t stats_subtree;
	am_timeline_discrete_layer_calculate_index_fun_t calculate_index;
	am_timeline_discrete_layer_event_render_fun_t render_events;
};

/* Sets the maximum interval index. This must be equal or greater than the
 * maximum index that can appear when processing elements.
 *
 * Returns 0 on success, otherwise 1.
 */
int
am_timeline_discrete_layer_set_max_index(struct am_timeline_discrete_layer* l,
					 size_t max_idx)
{
	if(l->statistics_init)
		am_discrete_stats_by_index_destroy(&l->statistics);

	l->statistics_init = 0;

	if(am_discrete_stats_by_index_init(&l->statistics, max_idx))
		return 1;

	l->statistics_init = 1;

	return 0;
}

/* Associates private data with the timeline interval layer l. */
void
am_timeline_discrete_layer_set_extra_data(struct am_timeline_discrete_layer* l,
					  void* extra_data)
{
	l->extra_data = extra_data;
}

/* Returns the private data associated with the timeline interval layer l. */
void*
am_timeline_discrete_layer_get_extra_data(struct am_timeline_discrete_layer* l)
{
	return l->extra_data;
}

/* Calculates the statistics for an interval i, starting with the hierarchy node
 * hn. If the layer's render mode is
 * AM_TIMELINE_LANE_RENDER_MODE_COMBINE_SUBTREE, the function recurses on the
 * children of hn.
 */
static void am_timeline_discrete_layer_default_stats_subtree(
	struct am_timeline_lane_render_layer* rl,
	struct am_discrete_stats_by_index* stats,
	struct am_hierarchy_node* hn,
	const struct am_interval* i)
{
	struct am_typed_array_generic* ea;
	struct am_event_mapping* m = &hn->event_mapping;
	struct am_event_collection* ec;
	struct am_hierarchy_node* child;
	struct am_timeline_discrete_layer* dl = (typeof(dl))rl;
	struct am_timeline_render_layer* l = AM_TIMELINE_RENDER_LAYER(dl);
	struct am_timeline_discrete_layer_type* dlt = (typeof(dlt))l->type;

	am_event_mapping_for_each_collection_overlapping(m, i, ec) {
		ea = am_event_collection_find_event_array(
			ec, dlt->event_array_type_name);

		if(!ea)
			continue;

		if(dlt->calculate_index) {
			am_discrete_stats_by_index_fun_collect(
				stats,
				i,
				ea,
				dlt->element_size,
				dlt->timestamp_offset,
				(size_t (*) (void*, void*))dlt->calculate_index,
				dl);
		} else {
			am_discrete_stats_by_index_collect(
				stats,
				i,
				ea,
				dlt->element_size,
				dlt->timestamp_offset,
				dlt->index_offset,
				dlt->index_bits);
		}
	}

	if(dl->super.render_mode ==
	   AM_TIMELINE_LANE_RENDER_MODE_COMBINE_SUBTREE)
	{
		am_hierarchy_node_for_each_child(hn, child) {
			am_timeline_discrete_layer_default_stats_subtree(
				rl, stats, child, i);
		}
	}
}

/* Render function of the layer */
static void render(struct am_timeline_discrete_layer* dl,
		   struct am_hierarchy_node* hn,
		   struct am_interval* i,
		   double lane_width,
		   double lane_height,
		   cairo_t* cr)
{
	struct am_timeline_discrete_layer_type* dlt;
	struct am_interval i_px;
	struct am_timeline_renderer* r;
	size_t idx;
	size_t last_idx = 0;
	int last_valid = 0;
	int valid;

	dlt = (struct am_timeline_discrete_layer_type*)dl->super.super.type;

	if(!dl->statistics_init)
		return;

	r = AM_TIMELINE_RENDER_LAYER(dl)->renderer;

	/* Process horizontal pixels of the lane */
	for(unsigned int px = 0; px < ceil(lane_width); px++) {
		am_timeline_renderer_relx_to_timestamp(r, px, &i_px.start);
		am_timeline_renderer_relx_to_timestamp(r, px+1, &i_px.end);

		/* Intervals are always inclusive; Exclude the last timestamp
		 * from the current interval, since it will already be included
		 * in the interval for the next pixel. */
		if(i_px.end > i_px.start+1)
			i_px.end--;

		am_discrete_stats_by_index_reset(&dl->statistics);

		dlt->stats_subtree(&dl->super, &dl->statistics, hn, &i_px);

		valid = am_discrete_stats_by_index_max(&dl->statistics, &idx);

		if(valid) {
			/* Invoke actual rendering function */
			dlt->render_events(dl, hn, &i_px,
					   lane_width, lane_height,
					   cr,
					   px,
					   &dl->statistics);
		}
	}
}

static void destroy(struct am_timeline_discrete_layer* l)
{
	if(l->statistics_init)
		am_discrete_stats_by_index_destroy(&l->statistics);
}

static struct am_timeline_discrete_layer*
instantiate(struct am_timeline_discrete_layer_type* t)
{
	struct am_timeline_discrete_layer* l;

	if(!(l = malloc(sizeof(*l))))
		return NULL;

	l->statistics_init = 0;
	l->extra_data = NULL;

	am_timeline_lane_render_layer_init(&l->super, &t->super);

	return l;
}

/* Common type instantiation function used by
 * am_timeline_discrete_layer_instantiate_type_default_stats_common and
 * am_timeline_discrete_layer_instantiate_type_stats_fun */
struct am_timeline_discrete_layer_type*
am_timeline_discrete_layer_instantiate_type_common(
	const char* name,
	am_timeline_discrete_layer_event_render_fun_t render_events)
{
	struct am_timeline_discrete_layer_type* t;

	if(!(t = calloc(1, sizeof(*t))))
		goto out_err;

	if(am_timeline_lane_render_layer_type_init(&t->super, name))
		goto out_err_free;

	t->super.destroy = AM_TIMELINE_LANE_RENDER_LAYER_DESTROY_FUN(destroy);
	t->super.render = AM_TIMELINE_LANE_RENDER_LAYER_RENDER_FUN(render);
	t->super.instantiate =
		AM_TIMELINE_LANE_RENDER_LAYER_INSTANTIATE_FUN(instantiate);
	t->render_events = render_events;

	return t;

out_err_free:
	free(t);
out_err:
	return NULL;
}


/* Common type instantiation function used by
 * am_timeline_discrete_layer_instantiate_type_index_member and
 * am_timeline_discrete_layer_instantiate_type_index_fun */
struct am_timeline_discrete_layer_type*
am_timeline_discrete_layer_instantiate_type_default_stats_common(
	const char* name,
	const char* event_array_type_name,
	size_t element_size,
	off_t timestamp_offset,
	am_timeline_discrete_layer_event_render_fun_t render_events)
{
	struct am_timeline_discrete_layer_type* t;

	if(!(t = am_timeline_discrete_layer_instantiate_type_common(
		     name, render_events)))
	{
		goto out_err;
	}

	/* FIXME: Never freed */
	if(!(t->event_array_type_name = strdup(event_array_type_name)))
		goto out_err_destroy;

	t->element_size = element_size;
	t->timestamp_offset = timestamp_offset;
	t->stats_subtree = am_timeline_discrete_layer_default_stats_subtree;

	return t;

out_err_destroy:
	am_timeline_render_layer_type_destroy(AM_TIMELINE_RENDER_LAYER_TYPE(t));
	free(t);
out_err:
	return NULL;
}

/* Instatiate a discrete layer type. Name is the name of the instantiated type,
 * event_array_type_name the name of the array type whose events are rendered
 * for each event collection, element_size defines the size in bytes of each
 * array element, timestamp_offset is the offset in bytes of the timestamp
 * member to extract from each array element, index_offset the offset of the
 * index (e.g., discrete event subtype) and index_bits specifies the size of
 * each index field in bits.
 */
struct am_timeline_render_layer_type*
am_timeline_discrete_layer_instantiate_type_index_member(
	const char* name,
	const char* event_array_type_name,
	size_t element_size,
	off_t timestamp_offset,
	off_t index_offset,
	unsigned int index_bits,
	am_timeline_discrete_layer_event_render_fun_t render_events)
{
	struct am_timeline_discrete_layer_type* t;

	if(!(t = am_timeline_discrete_layer_instantiate_type_default_stats_common(
		     name, event_array_type_name, element_size, timestamp_offset,
		     render_events)))
	{
		return NULL;
	}

	t->index_offset = index_offset;
	t->index_bits = index_bits;

	return AM_TIMELINE_RENDER_LAYER_TYPE(t);
}

/* Instatiate a discrete layer type. Name is the name of the instantiated type,
 * event_array_type_name the name of the array type whose events are rendered
 * for each event collection, element_size defines the size in bytes of each
 * array element and timestamp_offset is the offset in bytes of the timestamp
 * member to extract from each array element.
 *
 * Calculate_index is invoked for each event instance considered by the renderer
 * in order to obtain an index.
 */
struct am_timeline_render_layer_type*
am_timeline_discrete_layer_instantiate_type_index_fun(
	const char* name,
	const char* event_array_type_name,
	size_t element_size,
	off_t timestamp_offset,
	am_timeline_discrete_layer_calculate_index_fun_t calculate_index,
	am_timeline_discrete_layer_event_render_fun_t render_events)
{
	struct am_timeline_discrete_layer_type* t;

	if(!(t = am_timeline_discrete_layer_instantiate_type_default_stats_common(
		     name, event_array_type_name, element_size, timestamp_offset,
		     render_events)))
	{
		return NULL;
	}

	t->calculate_index = calculate_index;

	return AM_TIMELINE_RENDER_LAYER_TYPE(t);
}

/* Instatiate a discrete layer type with a custom statistics function. Name is
 * the name of the instantiated type. Stats_subtree is invoked for each visible
 * lane.
 */
struct am_timeline_render_layer_type*
am_timeline_discrete_layer_instantiate_type_stats_fun(
	const char* name,
	am_timeline_discrete_layer_stats_subtree_fun_t stats_subtree,
	am_timeline_discrete_layer_event_render_fun_t render_events)
{
	struct am_timeline_discrete_layer_type* t;

	if(!(t = am_timeline_discrete_layer_instantiate_type_common(
		     name, render_events)))
	{
		return NULL;
	}

	t->stats_subtree = stats_subtree;
	t->calculate_index = NULL;

	return AM_TIMELINE_RENDER_LAYER_TYPE(t);
}

/* Calculates the index with the highest count in the interval i for the
 * hierarchy node hn. The dominant index is returned in *index. However, if no
 * event is within i, *index_valid is 0, otherwise 1.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_timeline_discrete_layer_get_dominant_index(
	struct am_timeline_discrete_layer* il,
	struct am_hierarchy_node* hn,
	const struct am_interval* i,
	size_t* index,
	int* index_valid)
{
	struct am_timeline_discrete_layer_type* ilt;
	struct am_discrete_stats_by_index stats;

	ilt = (struct am_timeline_discrete_layer_type*)il->super.super.type;

	if(am_discrete_stats_by_index_init(&stats, il->statistics.max_index))
		return 1;

	am_discrete_stats_by_index_reset(&stats);

	ilt->stats_subtree(&il->super, &stats, hn, i);
	*index_valid = am_discrete_stats_by_index_max(&stats, index);

	am_discrete_stats_by_index_destroy(&stats);

	return 0;
}
