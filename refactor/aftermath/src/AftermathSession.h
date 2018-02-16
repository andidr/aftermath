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

#ifndef AM_GUI_AFTERMATHSESSION_H
#define AM_GUI_AFTERMATHSESSION_H

#include <map>
#include <cstdint>
#include "Exception.h"
#include "gui/AftermathGUI.h"

extern "C" {
	#include <aftermath/core/trace.h>
	#include <aftermath/core/dfg_node_type_registry.h>
	#include <aftermath/core/dfg_type_registry.h>
	#include <aftermath/core/dfg_graph.h>
	#include <aftermath/render/dfg/dfg_coordinate_mapping.h>
	#include <aftermath/render/timeline/layer.h>
}

/* The AftermathSession class contains all the run-time data of an Aftermath
 * instance.
 */
class AftermathSession {
	public:
		class Exception : public AftermathException {
			public:
				Exception(const std::string& msg) :
					AftermathException(msg) { };
		};

		class RegisterDFGTypesException : public Exception {
			public:
				RegisterDFGTypesException() :
					Exception("Could not register builtin "
						  "DFG types.")
				{ }
		};

		class RegisterDFGNodeTypesException : public Exception {
			public:
				RegisterDFGNodeTypesException() :
					Exception("Could not register builtin "
						  "DFG node types.")
				{ }
		};

		AftermathSession();
		~AftermathSession();

		struct am_dfg_type_registry* getDFGTypeRegistry() noexcept;
		struct am_dfg_node_type_registry* getDFGNodeTypeRegistry() noexcept;
		struct am_trace* getTrace() noexcept;
		struct am_dfg_graph* getDFG() noexcept;
		struct am_dfg_coordinate_mapping* getDFGCoordinateMapping() noexcept;

		void setTrace(struct am_trace* t) noexcept;
		void setDFG(struct am_dfg_graph* g) noexcept;
		void setDFGCoordinateMapping(struct am_dfg_coordinate_mapping* m) noexcept;

		struct am_timeline_render_layer_type_registry*
		getRenderLayerTypeRegistry() noexcept;

		AftermathGUI& getGUI();

	protected:
		void cleanup();

		struct {
			struct am_dfg_graph* graph;
			struct am_dfg_type_registry type_registry;
			struct am_dfg_node_type_registry node_type_registry;
			struct am_dfg_coordinate_mapping* coordinate_mapping;
		} dfg;

		struct am_trace* trace;
		struct am_timeline_render_layer_type_registry rltr;

		AftermathGUI gui;
};

#endif
