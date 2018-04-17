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

#ifndef AM_DFG_NODE_EVENT_MAPPING_STATES_H
#define AM_DFG_NODE_EVENT_MAPPING_STATES_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/core/in_memory.h>

int am_dfg_event_mapping_state_events_node_process(struct am_dfg_node* n);

/* A node extracting all state events from an event mapping overlapping with at
 * least one of the intervals provided at the "intervals" input port. If the
 * port is disconnected, all events of the mapping will be output. */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_event_mapping_state_events_node_type,
	"am::core::event_mapping_state_events",
	"Event Mapping: State Events",
	sizeof(struct am_dfg_node),
	AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,
	AM_DFG_NODE_FUNCTIONS({
		.process = am_dfg_event_mapping_state_events_node_process
	}),
	AM_DFG_NODE_PORTS(
		{ "event mapping", "const am::core::event_mapping",
				AM_DFG_PORT_IN },
		{ "intervals", "am::core::interval",
				AM_DFG_PORT_IN },
		{ "state events", "am::core::state_event",
				AM_DFG_PORT_OUT }),
	AM_DFG_PORT_DEPS(),
	AM_DFG_NODE_PROPERTIES())

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_event_mapping_state_events_node_type)

#endif
