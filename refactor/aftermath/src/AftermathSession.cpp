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

#include "AftermathSession.h"
#include "dfg/nodes/builtin_nodes.h"

extern "C" {
	#include <aftermath/render/timeline/common_layers.h>
	#include <aftermath/core/dfg_builtin_types.h>
	#include <aftermath/core/dfg_builtin_node_types.h>
	#include <aftermath/core/frame_type_registry.h>
	#include <aftermath/core/io_context.h>
	#include <aftermath/core/io_error.h>
	#include <aftermath/core/on_disk.h>
}

AftermathSession::AftermathSession() :
	trace(NULL)
{
	this->dfg.graph = NULL;
	this->dfg.coordinate_mapping = NULL;

	am_dfg_type_registry_init(&this->dfg.type_registry,
				  AM_DFG_TYPE_REGISTRY_DESTROY_TYPES);
	am_dfg_node_type_registry_init(&this->dfg.node_type_registry,
				       AM_DFG_NODE_TYPE_REGISTRY_DESTROY_TYPES);

	am_timeline_render_layer_type_registry_init(&this->rltr);
	am_register_common_timeline_layer_types(&this->rltr);

	if(am_dfg_builtin_types_register(&this->dfg.type_registry)) {
		this->cleanup();
		throw RegisterDFGTypesException();
	}

	if(am_dfg_builtin_node_types_register(&this->dfg.node_type_registry,
					      &this->dfg.type_registry))
	{
		this->cleanup();
		throw RegisterDFGNodeTypesException();
	}

	if(amgui_dfg_builtin_node_types_register(&this->dfg.node_type_registry,
						 &this->dfg.type_registry))
	{
		this->cleanup();
		throw RegisterDFGNodeTypesException();
	}
}

void AftermathSession::cleanup()
{
	am_timeline_render_layer_type_registry_destroy(&this->rltr);

	if(this->trace) {
		am_trace_destroy(this->trace);
		free(this->trace);
	}

	if(this->dfg.graph) {
		am_dfg_graph_destroy(this->dfg.graph);
		free(this->dfg.graph);
	}

	if(this->dfg.coordinate_mapping) {
		am_dfg_coordinate_mapping_destroy(this->dfg.coordinate_mapping);
		free(this->dfg.coordinate_mapping);
	}

	am_dfg_node_type_registry_destroy(&this->dfg.node_type_registry);
	am_dfg_type_registry_destroy(&this->dfg.type_registry);
}

AftermathSession::~AftermathSession()
{
	this->cleanup();
}

struct am_dfg_type_registry* AftermathSession::getDFGTypeRegistry() noexcept
{
	return &this->dfg.type_registry;
}

struct am_dfg_node_type_registry* AftermathSession::getDFGNodeTypeRegistry()
	noexcept
{
	return &this->dfg.node_type_registry;
}

struct am_trace* AftermathSession::getTrace() noexcept
{
	return this->trace;
}

struct am_timeline_render_layer_type_registry*
AftermathSession::getRenderLayerTypeRegistry() noexcept
{
	return &this->rltr;
}

void AftermathSession::setTrace(struct am_trace* t) noexcept
{
	this->trace = t;
}

struct am_dfg_graph* AftermathSession::getDFG() noexcept
{
	return this->dfg.graph;
}

void AftermathSession::setDFG(struct am_dfg_graph* g) noexcept
{
	if(this->dfg.graph) {
		am_dfg_graph_destroy(this->dfg.graph);
		free(this->dfg.graph);
	}

	this->dfg.graph = g;
	this->dfgProcessor.setDFG(g);
}

struct am_dfg_coordinate_mapping* AftermathSession::getDFGCoordinateMapping()
	noexcept
{
	return this->dfg.coordinate_mapping;
}

void AftermathSession::setDFGCoordinateMapping(
	struct am_dfg_coordinate_mapping* m) noexcept
{
	this->dfg.coordinate_mapping = m;
}

AftermathGUI& AftermathSession::getGUI()
{
	return this->gui;
}

DFGQTProcessor& AftermathSession::getDFGProcessor()
{
	return this->dfgProcessor;
}

DFGQTProcessor* AftermathSession::getDFGProcessorp()
{
	return &this->dfgProcessor;
}

/* Schedules the entire DFG graph of the session. Throws an exception if
 * scheduling fails. */
void AftermathSession::scheduleDFG()
{
	int ret;

	if(!this->dfg.graph)
		throw NoDFGException();

	this->dfgProcessor.disable();

	ret = am_dfg_schedule_graph(this->dfg.graph);

	this->dfgProcessor.enable();

	if(ret)
		throw DFGSchedulingException();
}

/* For each error message in the error stack s, a line with the message is
 * appended to msg */
static void errorStackToString(struct am_io_error_stack* s, std::string& msg)
{
	for(size_t i = 0; i < s->pos; i++) {
		msg += s->errors[i].msgbuf;
		msg += "\n";
	}
}

/* Reads the trace file whose filename including its path is given from disk and
 * sets it as the trace for this Aftermath session.
 *
 * Throws an exception on error.
 */
void AftermathSession::loadTrace(const char* filename)
{
	struct am_trace* trace;
	struct am_io_context ioctx;
	struct am_frame_type_registry frame_types;

	if(am_frame_type_registry_init(&frame_types, AM_MAX_FRAME_TYPES)) {
		throw AftermathException("Could not initialize frame type "
					 "registry");
	}

	if(am_dsk_register_frame_types(&frame_types)) {
		am_frame_type_registry_destroy(&frame_types);
		throw AftermathException("Could not register builtin frame types");
	}

	if(am_io_context_init(&ioctx, &frame_types)) {
		am_frame_type_registry_destroy(&frame_types);
		throw AftermathException("Could not initialize I/O context");
	}

	try {
		if(am_io_context_open(&ioctx, filename, AM_IO_READ)) {
			throw AftermathException("Could not open trace file "
						 "for reading");
		}

		if(am_dsk_load_trace(&ioctx, &trace)) {
			std::string msg;

			errorStackToString(&ioctx.error_stack, msg);
			throw AftermathException(msg);
		}
	} catch(...) {
		am_io_context_destroy(&ioctx);
		am_frame_type_registry_destroy(&frame_types);
		throw;
	}

	am_io_context_destroy(&ioctx);
	am_frame_type_registry_destroy(&frame_types);
	this->setTrace(trace);
}
