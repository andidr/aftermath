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

#include <aftermath/render/dfg/nodes/timeline/layers/background.h>
#include <aftermath/render/timeline/layers/background.h>
#include <aftermath/render/timeline/layer.h>
#include <aftermath/render/timeline/renderer.h>
#include <aftermath/render/dfg/timeline_layer_common.h>

AM_RENDER_DFG_IMPL_TIMELINE_LAYER_FILTER_NODE_TYPE(
	background,
	"background",
	struct am_timeline_background_layer)

int am_render_dfg_timeline_background_layer_configuration_node_process(
	struct am_dfg_node* n)
{
	size_t num_layers;
	struct am_timeline_background_layer** layers;
	int changed = 0;

	/* No input layers -> Exit */
	if(!am_dfg_port_activated(&n->ports[0]) ||
	   n->ports[0].buffer->num_samples == 0)
	{
		return 0;
	}

	num_layers = n->ports[0].buffer->num_samples;
	layers = n->ports[0].buffer->data;

	/* Update parameters */
	if(am_render_dfg_valupd_rgba(&n->ports[0], &n->ports[1], offsetof(struct am_timeline_background_layer, params.colors.even), &changed) ||
	   am_render_dfg_valupd_rgba(&n->ports[0], &n->ports[2], offsetof(struct am_timeline_background_layer, params.colors.odd), &changed))
	{
		return 1;
	}

	/* Notify renderers if necessary */
	if(changed) {
		for(size_t i = 0; i < num_layers; i++) {
			am_timeline_renderer_indicate_layer_appearance_change(
				((struct am_timeline_render_layer*)layers[i])->renderer,
				((struct am_timeline_render_layer*)layers[i]));
		}
	}

	return 0;
}
