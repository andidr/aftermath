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
#include <aftermath/core/tensorflow_node_array.h>
#include <aftermath/render/dfg/nodes/timeline/layers/tensorflow_node_execution.h>
#include <aftermath/render/timeline/layers/interval.h>
#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/timeline/renderer.h>

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	tensorflow_node_execution,
	"tensorflow::node_execution",
	struct am_timeline_interval_layer)

int am_render_dfg_timeline_tensorflow_node_execution_layer_configuration_node_process(
	struct am_dfg_node* n)
{
	size_t num_layers;
	struct am_timeline_render_layer** layers;
	int changed = 0;
	int rendering_enabled;

	/* No input layers -> Exit */
	if(!am_dfg_port_activated(&n->ports[0]) ||
	   n->ports[0].buffer->num_samples == 0)
	{
		return 0;
	}

	num_layers = n->ports[0].buffer->num_samples;
	layers = n->ports[0].buffer->data;

	/* Enable */
	if(am_dfg_port_activated(&n->ports[1])) {
		if(am_dfg_buffer_read_last(n->ports[1].buffer, &rendering_enabled))
			return 1;

		changed = 1;

		for(size_t i = 0; i < num_layers; i++)
			layers[i]->enabled = rendering_enabled;
	}

	/* Notify renderers if necessary */
	if(changed) {
		for(size_t i = 0; i < num_layers; i++) {
			am_timeline_renderer_indicate_layer_appearance_change(
				layers[i]->renderer,
				layers[i]);
		}
	}

	return 0;
}

int am_render_dfg_timeline_tensorflow_node_execution_layer_dominant_node_at_pos_node_type_process(
	struct am_dfg_node* n)
{
	struct am_dfg_port* player = &n->ports[0];
	struct am_dfg_port* ppos = &n->ports[1];
	struct am_dfg_port* pdom_nodes = &n->ports[2];

	struct am_dfg_type_pair_timestamp_const_hierarchy_node* mouse_pos;
	struct am_timeline_render_layer** layers;
	struct am_timeline_render_layer* layer;
	struct am_timeline_renderer* renderer;
	struct am_tensorflow_node_array* node_array;
	struct am_interval px_interval;
	struct am_tensorflow_node* dom_node;
	int dom_index_valid;
	size_t dom_index;
	size_t num_layers;
	size_t num_pos;
	size_t old_num_out;
	double px;

	if(!am_dfg_port_activated(player) || player->buffer->num_samples == 0 ||
	   !am_dfg_port_activated(ppos) || ppos->buffer->num_samples == 0 ||
	   !am_dfg_port_activated(pdom_nodes))
	{
		return 0;
	}

	num_layers = player->buffer->num_samples;
	layers = player->buffer->data;

	num_pos = ppos->buffer->num_samples;
	mouse_pos = ppos->buffer->data;

	old_num_out = pdom_nodes->buffer->num_samples;

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
				if(!(node_array = am_trace_find_trace_array(
					     renderer->trace,
					     "am::tensorflow::node")))
				{
					goto out_err_resize;
				}

				if(dom_index >= node_array->num_elements)
					goto out_err_resize;

				dom_node = &node_array->elements[dom_index];

				if(am_dfg_buffer_write(
					   pdom_nodes->buffer, 1, &dom_node))
				{
					goto out_err_resize;
				}
			}
		}
	}

	return 0;

out_err_resize:
	am_dfg_buffer_resize(pdom_nodes->buffer, old_num_out);
	return 1;
}
