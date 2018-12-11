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
#include <aftermath/render/dfg/nodes/timeline/layers/state.h>
#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/timeline/renderer.h>

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	state,
	"state",
	struct am_timeline_state_layer)

int am_render_dfg_timeline_state_layer_configuration_node_process(
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
