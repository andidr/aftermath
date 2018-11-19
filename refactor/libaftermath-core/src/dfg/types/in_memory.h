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

#ifndef AM_DFG_TYPE_IN_MEMORY_H
#define AM_DFG_TYPE_IN_MEMORY_H

#include <aftermath/core/dfg_type.h>
#include <aftermath/core/in_memory.h>

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_trace,
	"const am::core::trace",
	sizeof(struct am_trace*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_event_mapping,
	"const am::core::event_mapping",
	sizeof(struct am_event_mapping*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_hierarchy,
	"const am::core::hierarchy",
	sizeof(struct am_event_hierarchy*),
	NULL, NULL, NULL, NULL)

AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_const_hierarchy_node,
	"const am::core::hierarchy_node",
	sizeof(struct am_event_hierarchy_node*),
	NULL, NULL, NULL, NULL)


AM_DFG_DECL_BUILTIN_TYPE(
	am_dfg_type_state_event,
	"am::core::state_event",
	sizeof(struct am_state_event),
	NULL, NULL, NULL, NULL)

AM_DFG_ADD_BUILTIN_TYPES(
	&am_dfg_type_const_trace,
	&am_dfg_type_const_event_mapping,
	&am_dfg_type_const_hierarchy,
	&am_dfg_type_const_hierarchy_node,
	&am_dfg_type_state_event
)

#endif
