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

#ifndef AM_DFG_NODE_EVENT_MAPPING_COMMON_H
#define AM_DFG_NODE_EVENT_MAPPING_COMMON_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/core/event_collection.h>
#include <aftermath/core/event_mapping.h>
#include <aftermath/core/interval.h>
#include <aftermath/core/interval_array.h>

/* Declares a node "am::core::event_mapping_<NAMES>" that takes a vector of
 * event mappings and a vector of intervals as an input and that returns all
 * events of DFG type IDENT of that mapping whose interval overlaps with at
 * least one of the input intervals. */
#define AM_DFG_DECL_EVENT_MAPPING_EXTRACT_OVERLAPPING_INTERVAL_NODE(		\
	NAMES, PORT_NAME, HRNAMES, IDENT)					\
	int am_dfg_event_mapping_##NAMES##_node_process(struct am_dfg_node* n); \
										\
	AM_DFG_DECL_BUILTIN_NODE_TYPE(						\
		am_dfg_event_mapping_##NAMES##_node_type,			\
		"am::core::event_mapping_" #NAMES,				\
		"Event Mapping: " HRNAMES,					\
		sizeof(struct am_dfg_node),					\
		AM_DFG_DEFAULT_PORT_DEPS_PURE_FUNCTIONAL,			\
		AM_DFG_NODE_FUNCTIONS({					\
			.process = am_dfg_event_mapping_##NAMES##_node_process	\
		}),								\
		AM_DFG_NODE_PORTS(						\
			{ "event mapping", "const am::core::event_mapping*",	\
					AM_DFG_PORT_IN },			\
			{ "intervals", "am::core::interval", AM_DFG_PORT_IN },	\
			{ PORT_NAME, "const " IDENT "*", AM_DFG_PORT_OUT }),	\
		AM_DFG_PORT_DEPS(),						\
		AM_DFG_NODE_PROPERTIES())

/* Implements the processing function for a node declared with
 * AM_DFG_DECL_EVENT_MAPPING_EXTRACT_OVERLAPPING_INTERVAL_NODE. */
#define AM_DFG_IMPL_EVENT_MAPPING_EXTRACT_OVERLAPPING_INTERVAL_NODE(		\
	NAMES, TEVENT, TEVENT_ARRAY, IDENT)					\
	int am_dfg_event_mapping_##NAMES##_node_process(struct am_dfg_node* n)	\
	{									\
		struct am_dfg_port* pmappings = &n->ports[0];			\
		struct am_dfg_port* pintervals = &n->ports[1];			\
		struct am_dfg_port* pevents = &n->ports[2];			\
		struct TEVENT_ARRAY* arr;					\
		struct am_event_mapping** mappings;				\
		struct am_event_mapping* mapping;				\
		struct am_event_collection* ecoll;				\
		static const struct am_interval iall = {			\
			.start = 0,						\
			.end = UINT64_MAX					\
		};								\
		const struct am_interval* filter_intervals;			\
		struct am_interval* sorted_intervals = NULL;			\
		const struct am_interval* filter_interval;			\
		struct TEVENT* estart;						\
		struct TEVENT* eend;						\
		struct TEVENT** events_out;					\
		size_t nfilter_intervals = 0;					\
		int ret = 1;							\
		size_t nevents;						\
										\
		if(!am_dfg_port_is_connected(pmappings) ||			\
		   !am_dfg_port_is_connected(pevents))				\
		{								\
			return 0;						\
		}								\
										\
		mappings = pmappings->buffer->data;				\
										\
		/* If there are filter intervals, merge them in order to avoid	\
		 * counting events twice when iterating over the intervals. */	\
		if(am_dfg_port_is_connected(pintervals)) {			\
			filter_intervals = pintervals->buffer->data;		\
			nfilter_intervals = pintervals->buffer->num_samples;	\
										\
			/* No intervals -> treat as empty interval */		\
			if(pintervals->buffer->num_samples == 0)		\
				return 0;					\
										\
			if(am_intervals_merge_overlapping(			\
				   pintervals->buffer->data,			\
				   pintervals->buffer->num_samples,		\
				   &sorted_intervals,				\
				   &nfilter_intervals))			\
			{							\
				goto out;					\
			}							\
										\
			filter_intervals = sorted_intervals;			\
		} else {							\
			filter_intervals = &iall;				\
			nfilter_intervals = 1;					\
		}								\
										\
		for(size_t i = 0; i < pmappings->buffer->num_samples; i++) {	\
			mapping = mappings[i];					\
										\
			for(size_t j = 0; j < nfilter_intervals; j++) {	\
				filter_interval = &filter_intervals[j];	\
										\
				am_event_mapping_for_each_collection_overlapping(\
					mapping, filter_interval, ecoll)	\
				{						\
					if(!(arr = am_event_collection_find_event_array( \
						     ecoll, IDENT)))		\
					{					\
						continue;			\
					}					\
										\
					if(!sorted_intervals) {		\
						estart = arr->elements;	\
						eend = &arr->elements[arr->num_elements-1]; \
					} else {				\
						if(!(estart = TEVENT_ARRAY##_bsearch_first_overlapping( \
							     arr, filter_interval))) \
						{				\
							continue;		\
						}				\
										\
						eend = TEVENT_ARRAY##_bsearch_last_overlapping( \
							arr, filter_interval);	\
					}					\
										\
					nevents = AM_ARRAY_INDEX_DISTANCE(estart, eend); \
										\
					if(am_size_inc_safe(&nevents, 1))	\
						goto out_free;			\
										\
					if(!(events_out = am_dfg_buffer_reserve( \
						   pevents->buffer,		\
						   nevents)))			\
					{					\
						goto out_free;			\
					}					\
										\
					for(size_t i = 0; i < nevents; i++)	\
						events_out[i] = estart+i;	\
				}						\
			}							\
		}								\
										\
		ret = 0;							\
										\
	out_free:								\
		free(sorted_intervals);					\
	out:									\
		return ret;							\
	}									\


#endif
