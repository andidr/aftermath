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

#ifndef AM_TIMELINEWIDGETCREATOR_H
#define AM_TIMELINEWIDGETCREATOR_H

#include "GUIFactory.h"

/* Widget creator creating Timelines. The expected node format is
 *
 *   amgui_timeline {
 *     layers: [ "layertype1", "layertype2", ... ]
 *   }
 *
 * The layers are optional.
 */
class TimelineWidgetCreator : public NonContainerWidgetCreator {
	public:
		class Exception : public WidgetCreator::Exception {
			public:
				Exception(const std::string& msg) :
					WidgetCreator::Exception(msg) { };
		};

		class LayerListNotAStringListException : public Exception {
			public:
				LayerListNotAStringListException() :
					Exception("Member 'layers' is not a "
						  "list of strings.")
				{ };
		};

		class UnknownRenderLayerException : public Exception {
			public:
				UnknownRenderLayerException(const char* rl) :
					Exception(std::string("Unknown render "
							      "layer '")+rl+"'.")
				{ };
		};

		TimelineWidgetCreator(
			struct am_timeline_render_layer_type_registry* rltr);

		virtual QWidget*
		instantiate(const struct am_object_notation_node_group* n);

	protected:
		struct am_timeline_render_layer_type_registry* rltr;
};

#endif
