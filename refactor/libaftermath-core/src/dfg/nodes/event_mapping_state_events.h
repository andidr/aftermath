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
#include <aftermath/core/dfg/nodes/event_mapping_common.h>
#include <aftermath/core/in_memory.h>

/* A node extracting all state events from an event mapping overlapping with at
 * least one of the intervals provided at the "intervals" input port. If the
 * port is disconnected, all events of the mapping will be output. */
AM_DFG_DECL_EVENT_MAPPING_EXTRACT_OVERLAPPING_INTERVAL_NODE(
	state_events, "state events", "State Events", "am::core::state_event")

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_event_mapping_state_events_node_type)

#endif
