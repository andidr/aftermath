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

#ifndef AM_DFG_NODE_INTERVAL_DURATION_H
#define AM_DFG_NODE_INTERVAL_DURATION_H

#include <aftermath/core/dfg_node.h>

int am_dfg_interval_duration_node_process(struct am_dfg_node* n);

/**
 * Node that calculates the durations of intervals.
 */
AM_DFG_DECL_BUILTIN_NODE_TYPE(
	am_dfg_interval_duration_node_type,
	"interval_duration",
	"Interval Duration",
	AM_DFG_NODE_DEFAULT_SIZE,
	AM_DFG_NODE_FUNCTIONS({
		.process = am_dfg_interval_duration_node_process
	}),
	AM_DFG_NODE_PORTS(
		{ "interval", "interval", AM_DFG_PORT_IN | AM_DFG_PORT_MANDATORY },
		{ "duration", "duration", AM_DFG_PORT_OUT }))

AM_DFG_ADD_BUILTIN_NODE_TYPES(&am_dfg_interval_duration_node_type)

#endif
