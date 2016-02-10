/**
 * Copyright (C) 2016 Andi Drebes <andi.drebes@lip6.fr>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "state_index.h"
#include "multi_event_set.h"
#include "event_set.h"
#include "filter.h"

/*
 * Allocate a state index with es->num_state_events / factor samples.
 */
int state_index_init(struct state_index* idx,
		     size_t num_states,
		     struct event_set* es,
		     int factor)
{
	return monotonic_index_init(idx,
				    num_states,
				    es->num_state_events / factor);
}

/*
 * Build the actual state index. The index must have been allocated previously
 * using state_index_alloc. The index will only take into acount state events
 * that have not been filtered out by the filter.
 */
void state_index_update(struct state_index* idx,
			struct event_set* es,
			struct filter* f)
{
	size_t s = 0;
	size_t factor;
	struct state_event* se;
	size_t num_valid = 0;
	int curr_valid = 0;
	size_t i;

	if(idx->num_samples == 0)
		return;

	factor = es->num_state_events / idx->num_samples;

	/* Initialize first sample with 0 */
	memset(idx->samples, 0, idx->num_dimensions * sizeof(uint64_t));

	for_each_state_event_i(es, se, i) {
		if(s >= idx->num_samples)
			break;

		if(filter_has_state_event(f, se)) {
			idx->samples[s*idx->num_dimensions + se->state_id_seq] +=
				se->end - se->start;

			idx->timestamps[s] = se->end;

			if(!curr_valid) {
				idx->valid[s] = num_valid;
				curr_valid = 1;
				num_valid++;
			}
		}

		if(i > 0 && i % factor == 0) {
			if(s+1 < idx->num_samples) {
				/* Initialize next sample with the current
				 * sample's values */
				memcpy(&idx->samples[(s+1)*idx->num_dimensions],
				       &idx->samples[s*idx->num_dimensions],
				       idx->num_dimensions * sizeof(uint64_t));

				idx->timestamps[s+1] = idx->timestamps[s];
				idx->valid[s+1] = num_valid;
			}

			s++;
			curr_valid = 0;
		}
	}
}

/*
 * Calculate the durations for each state id using a state index for the
 * interval [start; end]. The result is passed in durations[0..num_states-1]. If
 * init is 1, the output array for the durations is initialized with zero
 * values. Otherwise the durations will be added to existing durations. If
 * break_half is set to 1 calculations stop as soon as one state occupies more
 * than half of the duration of the interval.
 */
int state_index_get_state_durations(struct state_index* idx,
				    struct event_set* es,
				    struct filter* f,
				    uint64_t start,
				    uint64_t end,
				    uint64_t* durations,
				    int init, int break_half)
{
	size_t lidx_start, lidx_end;
	uint64_t durations_start[idx->num_dimensions];
	uint64_t durations_end[idx->num_dimensions];
	int has_state = 0;
	size_t valid_start, valid_end;
	uint64_t ts_start, ts_end;

	if(init)
		memset(durations, 0, idx->num_dimensions * sizeof(uint64_t));

	/* Try to determine index for durations for [0; start] */
	if(monotonic_index_get_lindex(idx, start, &lidx_start)) {
		/* Try to determine index for [0; end] */
		if(monotonic_index_get_lindex(idx, end, &lidx_end)) {
			if(lidx_start == lidx_end) {
				/* In between two samples; index doesn't help */
				has_state = event_set_get_state_durations(es, f,
									  start,
									  end,
									  idx->num_dimensions,
									  durations,
									  0, break_half);
			} else {
				/* Durations [0; start] */
				monotonic_index_get(idx, lidx_start, &ts_start, durations_start, &valid_start);

				/* Add durations [ts_start; start] */
				event_set_get_state_durations(es, f,
							      ts_start,
							      start,
							      idx->num_dimensions,
							      durations_start, 0, 0);

				/* Durations [0; end] */
				monotonic_index_get(idx, lidx_end, &ts_end, durations_end, &valid_end);

				/* Distance higher than 1 sample and valid in
				 * between or valid between start and timestamp
				 * of the next sample
				 */
				if(idx->valid[lidx_end] - idx->valid[lidx_start+1] > 0 ||
				   event_set_has_state_in_interval(es, f, start, idx->timestamps[lidx_start+1]))
				{
					has_state = 1;
				}

				/* Add durations [idx_ts_end; end] */
				has_state |= event_set_get_state_durations(es, f,
									   ts_end,
									   end,
									   idx->num_dimensions,
									   durations_end, 0, 0);

				for(size_t d = 0; d < idx->num_dimensions; d++) {
					durations[d] += durations_end[d] -
						durations_start[d];
				}
			}
		}
	} else { /* No start index */
		/* Try to determine index for [0; end] */
		if(monotonic_index_get_lindex(idx, end, &lidx_end)) {
			/* Durations [0; start] */
			event_set_get_state_durations(es, f,
						      0,
						      start,
						      idx->num_dimensions,
						      durations_start, 1, 0);

			/* Durations [0; end] */
			monotonic_index_get(idx, lidx_end, &ts_end, durations_end, &valid_end);

			/* Distance higher than 1 sample and valid in
			 * between or valid between start and timestamp
			 * of the next sample
			 */
			if(idx->valid[lidx_end] - idx->valid[0] ||
			   event_set_has_state_in_interval(es, f, start, idx->timestamps[0]))
			{
				has_state = 1;
			}

			/* Add durations [ts_end; end] */
			has_state |= event_set_get_state_durations(es, f,
								   ts_end,
								   end,
								   idx->num_dimensions,
								   durations_end, 0, 0);

			for(size_t d = 0; d < idx->num_dimensions; d++) {
				durations[d] += durations_end[d] -
					durations_start[d];
			}
		} else {
			/* Neither start nor end index; index won't help */
			has_state = event_set_get_state_durations(es, f,
								  start,
								  end,
								  idx->num_dimensions,
								  durations, 0, 0);
		}
	}

	return has_state;
}
