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

#define TYPE_ONLY 0x1
#define LIVENESS_ONLY 0x2
#define BOTH (TYPE_ONLY | LIVENESS_ONLY)

#define AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(FILTER_TYPE, PREDICATE) \
	int am_dfg_telamon_candidate_type_filter_##FILTER_TYPE##_node_process(	\
		struct am_dfg_node* n)						\
	{									\
		struct am_dfg_port* pin = &n->ports[0];			\
		struct am_dfg_port* ptime = &n->ports[1];			\
		struct am_dfg_port* pout = &n->ports[2];			\
		struct am_telamon_candidate** in;				\
		am_timestamp_t time = AM_TIMESTAMP_T_MAX;			\
		struct am_telamon_candidate_classification cls;		\
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
				am_telamon_candidate_classify(in[i], time, &cls);\
										\
				if(PREDICATE(&cls)) {				\
					if(am_dfg_buffer_write(pout->buffer,	\
							       1,		\
							       &in[i]))	\
					{					\
						return 1;			\
					}					\
				}						\
			}							\
		}								\
										\
		return 0;							\
	}

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(
	unknown, am_telamon_candidate_is_unknown_node)

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(
	any_internal, am_telamon_candidate_is_internal_node)

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(
	any_rollout, am_telamon_candidate_is_rollout_node)

#define P(pcls) (am_telamon_candidate_is_rollout_node(pcls) && \
		 !am_telamon_candidate_is_implementation(pcls))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(any_rollout_not_implementation, P)

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(
	any_implementation, am_telamon_candidate_is_implementation)

#undef P
#define P(pcls) (am_telamon_candidate_is_implementation(pcls) && \
		 am_telamon_candidate_is_alive(pcls))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(implementation_not_deadend, P)

#undef P
#define P(pcls) (am_telamon_candidate_is_internal_node(pcls) && \
		 am_telamon_candidate_is_alive(pcls))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(internal_not_deadend, P)

#undef P
#define P(pcls) (am_telamon_candidate_is_rollout_node(pcls) && \
		 am_telamon_candidate_is_alive(pcls))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(rollout_not_deadend, P)

#undef P
#define P(pcls) (am_telamon_candidate_is_rollout_node(pcls) &&	       \
		 am_telamon_candidate_is_alive(pcls) &&	       \
		 !am_telamon_candidate_is_implementation(pcls))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(
	rollout_not_deadend_not_implementation, P)

AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(
	any_deadend, am_telamon_candidate_is_deadend)

#undef P
#define P(pcls) (am_telamon_candidate_is_internal_node(pcls) && \
		 am_telamon_candidate_is_deadend(pcls))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(internal_deadend, P)

#undef P
#define P(pcls) (am_telamon_candidate_is_rollout_node(pcls) &&	\
		 am_telamon_candidate_is_deadend(pcls))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(rollout_deadend, P)

#undef P
#define P(pcls) (am_telamon_candidate_is_implementation(pcls) && \
		 am_telamon_candidate_is_deadend(pcls))
AM_DFG_TELAMON_CANDIDATE_TYPE_FILTER_NODE_IMPL(implementation_deadend, P)

int am_dfg_telamon_candidate_type_filter_perfmodel_lowest_n_node_process(
	struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* ptime = &n->ports[1];
	struct am_dfg_port* pN = &n->ports[1];
	struct am_dfg_port* pout = &n->ports[3];
	struct am_telamon_candidate** in;
	struct am_telamon_candidate** out;
	am_timestamp_t time = AM_TIMESTAMP_T_MAX;
	uint64_t N = 1;
	size_t nknown = 0;
	size_t nin;
	size_t old_nout;
	struct am_telamon_candidate_classification cls;

	if(am_dfg_port_activated_and_has_data(ptime)) {
		if(am_dfg_buffer_read_last(ptime->buffer, &time))
			return 1;
	}

	if(am_dfg_port_activated_and_has_data(pN)) {
		if(am_dfg_buffer_read_last(pN->buffer, &N))
			return 1;
	}

	if(am_dfg_port_activated_and_has_data(pin) &&
	   am_dfg_port_activated(pout))
	{
		in = pin->buffer->data;
		nin = pin->buffer->num_samples;
		old_nout = pout->buffer->num_samples;

		if(!(out = am_dfg_buffer_reserve(pout->buffer, nin)))
			return 1;

		/* Filter out nodes that are unknown at this point in time */
		for(size_t i = 0; i < nin; i++) {
			am_telamon_candidate_classify(in[i], time, &cls);

			if(!am_telamon_candidate_is_unknown_node(&cls) &&
			   am_telamon_candidate_perfmodel_bound_valid(in[i]))
			{
				out[nknown++] = in[i];
			}
		}

		pout->buffer->num_samples -= nin - nknown;

		am_telamon_candidate_qsort_perfmodel_bound_ascending(
			out, nknown);

		if(nknown > N)
			pout->buffer->num_samples = old_nout + N;
	}

	return 0;
}
