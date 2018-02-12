/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "TimelineWidgetCreator.h"
#include "../widgets/TimelineWidget.h"

TimelineWidgetCreator::TimelineWidgetCreator(
	struct am_timeline_render_layer_type_registry* rltr) :
	NonContainerWidgetCreator("amgui_timeline"), rltr(rltr)
{
}

QWidget* TimelineWidgetCreator::instantiate(
	const struct am_object_notation_node_group* n)
{
	struct am_object_notation_node* nlayers;
	struct am_object_notation_node_list* layers;
	struct am_object_notation_node_string* layer;
	struct am_timeline_render_layer* rl;

	TimelineWidget* t = new TimelineWidget();

	t->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	nlayers = am_object_notation_node_group_get_member_def(n, "layers");

	if(nlayers) {
		if(nlayers->type != AM_OBJECT_NOTATION_NODE_TYPE_LIST)
			throw LayerListNotAStringListException();

		layers = (struct am_object_notation_node_list*)nlayers;

		if(!layers)
			return t;

		if(!am_object_notation_is_string_list(layers))
			throw LayerListNotAStringListException();

		am_object_notation_for_each_list_item_string(layers, layer) {
			rl = am_timeline_render_layer_type_registry_instantiate(
				this->rltr, layer->value);

			if(!rl) {
				delete t;
				throw UnknownRenderLayerException(layer->value);
			}

			t->addLayer(rl);
		}
	}

	return t;
}
