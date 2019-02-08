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

#include <aftermath/core/dfg/nodes/telamon_filter.h>
#include <aftermath/core/base_types.h>
#include <aftermath/core/telamon.h>

#define AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(FILTER_TYPE, TYPE_LIST)	\
	int am_dfg_telamon_candidate_type_filter_##FILTER_TYPE##_node_process(	\
		struct am_dfg_node* n)						\
	{									\
		struct am_dfg_port* pin = &n->ports[0];			\
		struct am_dfg_port* ptime = &n->ports[1];			\
		struct am_dfg_port* pout = &n->ports[2];			\
		struct am_telamon_candidate** in;				\
		am_timestamp_t time = AM_TIMESTAMP_T_MAX;			\
		static const enum am_telamon_candidate_type include_types[] =	\
			TYPE_LIST;						\
		enum am_telamon_candidate_type type;				\
										\
		if(am_dfg_port_activated_and_has_data(ptime)) {		\
			if(am_dfg_buffer_read_last(ptime->buffer, &time))	\
				return 1;					\
		}								\
										\
		if(am_dfg_port_activated_and_has_data(pin) &&			\
		   am_dfg_port_activated(pout))				\
		{								\
			in = pin->buffer->data;				\
										\
			for(size_t i = 0; i < pin->buffer->num_samples; i++) {	\
				type = am_telamon_candidate_get_type(in[i], time); \
										\
				for(size_t j = 0;				\
				    j < AM_ARRAY_SIZE(include_types);		\
				    j++)					\
				{						\
					if(type == include_types[j]) {		\
						if(am_dfg_buffer_write(	\
							   pout->buffer, 1, &in[i])) \
						{				\
							return 1;		\
						}				\
					}					\
				}						\
			}							\
		}								\
										\
		return 0;							\
	}

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(unknown, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_UNKNOWN }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(any_internal, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_INTERNAL_NODE, AM_TELAMON_CANDIDATE_INTERNAL_DEADEND }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(any_rollout, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_ROLLOUT_NODE, AM_TELAMON_CANDIDATE_ROLLOUT_DEADEND, AM_TELAMON_CANDIDATE_IMPLEMENTATION_NODE, AM_TELAMON_CANDIDATE_IMPLEMENTATION_DEADEND }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(any_rollout_not_implementation, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_ROLLOUT_NODE, AM_TELAMON_CANDIDATE_ROLLOUT_DEADEND }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(any_implementation, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_IMPLEMENTATION_NODE, AM_TELAMON_CANDIDATE_IMPLEMENTATION_DEADEND }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(implementation_not_deadend, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_IMPLEMENTATION_NODE }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(internal_not_deadend, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_INTERNAL_NODE }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(rollout_not_deadend, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_ROLLOUT_NODE, AM_TELAMON_CANDIDATE_IMPLEMENTATION_NODE }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(rollout_not_deadend_not_implementation, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_ROLLOUT_NODE }))

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(any_deadend, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_INTERNAL_DEADEND, AM_TELAMON_CANDIDATE_ROLLOUT_DEADEND, AM_TELAMON_CANDIDATE_IMPLEMENTATION_DEADEND }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(internal_deadend, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_INTERNAL_DEADEND }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(rollout_deadend, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_ROLLOUT_DEADEND }))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(implementation_deadend, AM_MACRO_ARG_PROTECT({ AM_TELAMON_CANDIDATE_IMPLEMENTATION_DEADEND }))
