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

#include <aftermath/core/dfg/types/pair_timestamp_hierarchy_node.h>
#include <aftermath/core/state_description_array.h>
#include <aftermath/render/dfg/nodes/timeline/layers/state.h>
#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/renderer.h>

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	state,
	"state",
	struct am_timeline_state_layer)

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_ENABLE_CONFIGURATION_NODE_TYPE(
	state,
	"state")

int am_render_dfg_timeline_state_layer_dominant_state_at_pos_node_type_process(
	struct am_dfg_node* n)
{
	struct am_dfg_port* player = &n->ports[0];
	struct am_dfg_port* ppos = &n->ports[1];
	struct am_dfg_port* pdom_states = &n->ports[2];

	struct am_dfg_type_pair_timestamp_const_hierarchy_node* mouse_pos;
	struct am_timeline_render_layer** layers;
	struct am_timeline_render_layer* layer;
	struct am_timeline_renderer* renderer;
	struct am_state_description_array* state_description_array;
	struct am_interval px_interval;
	struct am_state_description* dom_state_description;
	int dom_index_valid;
	size_t dom_index;
	size_t num_layers;
	size_t num_pos;
	size_t old_num_out;
	double px;

	if(!am_dfg_port_activated(player) || player->buffer->num_samples == 0 ||
	   !am_dfg_port_activated(ppos) || ppos->buffer->num_samples == 0 ||
	   !am_dfg_port_activated(pdom_states))
	{
		return 0;
	}

	num_layers = player->buffer->num_samples;
	layers = player->buffer->data;

	num_pos = ppos->buffer->num_samples;
	mouse_pos = ppos->buffer->data;

	old_num_out = pdom_states->buffer->num_samples;

	for(size_t i = 0; i < num_layers; i++) {
		layer = layers[i];

		for(size_t j = 0; j < num_pos; j++) {
			if(!layer->renderer)
				goto out_err_resize;

			renderer = layer->renderer;

			/* If there is no trace, there cannot be any dominant TF
			 * node */
			if(!layer->renderer->trace)
				continue;

			/* Ignore invisible positions */
			if(!am_interval_contains_p(&renderer->visible_interval,
						   mouse_pos[j].timestamp))
			{
				continue;
			}

			/* Pixel at the input timestamp */
			px = am_timeline_renderer_timestamp_to_x(
				renderer, mouse_pos[j].timestamp);

			/* Calculate interval that corresponds to the pixel */
			if(am_timeline_renderer_x_to_timestamp(
				   renderer, px, &px_interval.start) ||
			   am_timeline_renderer_x_to_timestamp(
				   renderer, px+1, &px_interval.end))
			{
				goto out_err_resize;
			}

			/* px interval is to be interpreted half-open (end
			 * excluded); Calculate equivalent closed interval. */
			if(px_interval.start != px_interval.end)
				px_interval.end--;

			/* Determine dominant index */
			if(am_timeline_interval_layer_get_dominant_index(
				   (struct am_timeline_interval_layer*)layer,
				   mouse_pos[j].node,
				   &px_interval,
				   &dom_index,
				   &dom_index_valid))
			{
				goto out_err_resize;
			}

			if(dom_index_valid) {
				/* Find array the index is for */
				if(!(state_description_array = am_trace_find_trace_array(
					     renderer->trace,
					     "am::core::state_description")))
				{
					goto out_err_resize;
				}

				if(dom_index >= state_description_array->num_elements)
					goto out_err_resize;

				dom_state_description = &state_description_array->elements[dom_index];

				if(am_dfg_buffer_write(
					   pdom_states->buffer, 1, &dom_state_description))
				{
					goto out_err_resize;
				}
			}
		}
	}

	return 0;

out_err_resize:
	am_dfg_buffer_resize(pdom_states->buffer, old_num_out);
	return 1;
}
