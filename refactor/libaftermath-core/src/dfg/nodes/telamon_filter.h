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

#ifndef AM_DFG_NODE_TELAMON_FILTER_H
#define AM_DFG_NODE_TELAMON_FILTER_H

#include <aftermath/core/dfg_node.h>

#define AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(FILTER_TYPE, FILTER_TYPE_CAP) \
	int am_dfg_telamon_candidate_type_filter_##FILTER_TYPE##_node_process(	\
		struct am_dfg_node* n);					\
										\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_telamon_candidate_type_filter_##FILTER_TYPE##_node_type, \
		"am::telamon::candidate::filter::" #FILTER_TYPE,		\
		"Telamon " FILTER_TYPE_CAP,					\
		AM_DFG_NODE_DEFAULT_SIZE,					\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,			\
		AM_DFG_NODE_FUNCTIONS({					\
			.process = am_dfg_telamon_candidate_type_filter_##FILTER_TYPE##_node_process, \
		}),								\
		AM_DFG_NODE_PORTS(						\
			{ "in", "const am::telamon::candidate*", AM_DFG_PORT_IN }, \
			{ "time", "am::core::timestamp", AM_DFG_PORT_IN },	\
			{ "out", "const am::telamon::candidate*", AM_DFG_PORT_OUT }, \
		),								\
		AM_DFG_PORT_DEPS(						\
			AM_DFG_PORT_DEP_UPDATE_IN_PORT("in"),			\
			AM_DFG_PORT_DEP_UPDATE_IN_PORT("time"),		\
			AM_DFG_PORT_DEP_INDEPENDENT_OUT_PORT("out"),		\
			AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_NEW, "time",	\
					AM_DFG_PORT_DEP_PUSH_NEW, "out"),	\
			AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_NEW, "time",	\
					AM_DFG_PORT_DEP_PULL_OLD, "in"),	\
			AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_NEW, "in",		\
					AM_DFG_PORT_DEP_PUSH_NEW, "out"),	\
			AM_DFG_PORT_DEP(AM_DFG_PORT_DEP_ON_NEW, "in",		\
					AM_DFG_PORT_DEP_PULL_OLD, "time")),	\
		AM_DFG_NODE_PROPERTIES())

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(unknown, "Yet Unknown Nodes")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(any_internal, "Any Internal Node")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(any_rollout, "Any Rollout Node")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(any_rollout_not_implementation, "Any Rollout Node (No Implementations)")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(any_implementation, "Any Implementation Node")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(implementation_not_deadend, "Implementation Nodes (No Deadends)")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(internal_not_deadend, "Any Internal Node (No Deadends)")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(rollout_not_deadend, "Any Rollout Node (No Deadends)")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(rollout_not_deadend_not_implementation, "Any Rollout Node (No Deadends, No Implementations)")

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(any_deadend, "Any Deadend")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(internal_deadend, "Internal Deadends")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(rollout_deadend, "Rollout Deadends")
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_DECL(implementation_deadend, "Implementation Deadends")

AM_DFG_ADD_BUILTIN_NODE_TYPES(
	&am_dfg_telamon_candidate_type_filter_unknown_node_type,
	&am_dfg_telamon_candidate_type_filter_any_internal_node_type,
	&am_dfg_telamon_candidate_type_filter_any_rollout_node_type,
	&am_dfg_telamon_candidate_type_filter_any_rollout_not_implementation_node_type,
	&am_dfg_telamon_candidate_type_filter_any_implementation_node_type,
	&am_dfg_telamon_candidate_type_filter_implementation_not_deadend_node_type,
	&am_dfg_telamon_candidate_type_filter_internal_not_deadend_node_type,
	&am_dfg_telamon_candidate_type_filter_rollout_not_deadend_node_type,
	&am_dfg_telamon_candidate_type_filter_rollout_not_deadend_not_implementation_node_type,
	&am_dfg_telamon_candidate_type_filter_any_deadend_node_type,
	&am_dfg_telamon_candidate_type_filter_internal_deadend_node_type,
	&am_dfg_telamon_candidate_type_filter_rollout_deadend_node_type,
	&am_dfg_telamon_candidate_type_filter_implementation_deadend_node_type)

#endif
