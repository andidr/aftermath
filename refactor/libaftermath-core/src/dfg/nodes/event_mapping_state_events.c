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

#include "event_mapping_state_events.h"
#include <aftermath/core/interval.h>
#include <aftermath/core/event_mapping.h>
#include <aftermath/core/event_collection.h>
#include <aftermath/core/state_event_array.h>

int am_dfg_event_mapping_state_events_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pmappings = &n->ports[0];
	struct am_dfg_port* pintervals = &n->ports[1];
	struct am_dfg_port* pstates = &n->ports[2];
	struct am_state_event_array* arr;
	struct am_event_mapping** mappings;
	struct am_event_mapping* mapping;
	struct am_event_collection* ecoll;
	static const struct am_interval iall = { .start = 0, .end = UINT64_MAX };
	const struct am_interval* filter_intervals;
	struct am_interval* sorted_intervals = NULL;
	const struct am_interval* filter_interval;
	struct am_state_event* estart;
	struct am_state_event* eend;
	size_t nfilter_intervals = 0;
	int ret = 1;
	size_t nevents;

	if(!am_dfg_port_is_connected(pmappings) ||
	   !am_dfg_port_is_connected(pstates))
	{
		return 0;
	}

	mappings = pmappings->buffer->data;

	/* If there are filter intervals, merge them in order to avoid counting
	 * events twice when iterating over the intervals. */
	if(am_dfg_port_is_connected(pintervals)) {
		filter_intervals = pintervals->buffer->data;
		nfilter_intervals = pintervals->buffer->num_samples;

		/* No intervals -> treat as empty interval */
		if(pintervals->buffer->num_samples == 0)
			return 0;

		if(am_intervals_merge_overlapping(
			   pintervals->buffer->data,
			   pintervals->buffer->num_samples,
			   &sorted_intervals,
			   &nfilter_intervals))
		{
			goto out;
		}

		filter_intervals = sorted_intervals;
	} else {
		filter_intervals = &iall;
		nfilter_intervals = 1;
	}

	for(size_t i = 0; i < pmappings->buffer->num_samples; i++) {
		mapping = mappings[i];

		for(size_t j = 0; j < nfilter_intervals; j++) {
			filter_interval = &filter_intervals[j];

			am_event_mapping_for_each_collection_overlapping(
				mapping, filter_interval, ecoll)
			{
				if(!(arr = am_event_collection_find_event_array(
					     ecoll, "am::generic::state")))
				{
					continue;
				}

				if(!sorted_intervals) {
					estart = arr->elements;
					eend = &arr->elements[arr->num_elements-1];
				} else {
					if(!(estart = am_state_event_array_bsearch_first_overlapping(
						     arr, filter_interval)))
					{
						continue;
					}

					eend = am_state_event_array_bsearch_last_overlapping(
						arr, filter_interval);
				}

				nevents = AM_ARRAY_INDEX_DISTANCE(estart, eend);

				if(am_size_inc_safe(&nevents, 1))
					goto out_free;

				if(am_dfg_buffer_write(pstates->buffer,
						       nevents,
						       estart))
				{
					goto out_free;
				}
			}
		}
	}

	ret = 0;

out_free:
	free(sorted_intervals);
out:
	return ret;
}
