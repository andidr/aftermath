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

#include "monotonic_index.h"
#include <string.h>

int monotonic_index_init(struct monotonic_index* idx,
			 size_t num_dimensions,
			 size_t num_samples)
{
	idx->num_samples = num_samples;
	idx->num_dimensions = num_dimensions;
	idx->min_timestamp = 0;
	idx->max_timestamp = 0;

	if(!(idx->samples = malloc(num_samples * num_dimensions *
				   sizeof(uint64_t))))
	{
		goto out_err;
	}

	if(!(idx->timestamps = malloc(num_samples * sizeof(uint64_t))))
		goto out_err_ts;

	if(!(idx->valid = malloc(num_samples * sizeof(size_t))))
		goto out_err_v;

	return 0;

out_err_v:
	free(idx->timestamps);
out_err_ts:
	free(idx->samples);
out_err:
	return 1;
}

void monotonic_index_destroy(struct monotonic_index* idx)
{
	free(idx->samples);
	free(idx->timestamps);
	free(idx->valid);
}

/* Set the values of the i-th sample of the index. */
void monotonic_index_set(struct monotonic_index* idx,
			 size_t i,
			 uint64_t time,
			 uint64_t* sample,
			 size_t valid)
{
	idx->timestamps[i] = time;
	idx->valid[i] = valid;

	memcpy(&idx->samples[i*idx->num_dimensions],
	       sample,
	       idx->num_dimensions * sizeof(uint64_t));

	if(time < idx->min_timestamp)
		idx->min_timestamp = time;

	if(time > idx->max_timestamp)
		idx->max_timestamp = time;

	idx->valid[i] = valid;
}

/* Get the values of the i-th sample of the index. */
void monotonic_index_get(struct monotonic_index* idx,
			 size_t i,
			 uint64_t* time,
			 uint64_t* sample,
			 size_t* valid)
{
	*time = idx->timestamps[i];

	memcpy(sample,
	       &idx->samples[i*idx->num_dimensions],
	       idx->num_dimensions * sizeof(uint64_t));

	*valid = idx->valid[i];
}

/* Retrieve the index of the sample with the highest timestamp lower
 * than time. If no such sample is found the function returns 0,
 * otherwise 1. The index of the sample is returned in *i. */
int monotonic_index_get_lindex(struct monotonic_index* idx,
			       uint64_t time,
			       size_t* i)
{
	size_t start_idx = 0;
	size_t end_idx = idx->num_samples-1;
	size_t center_idx;

	if(idx->num_samples == 0 || time < idx->min_timestamp)
		return 0;

	/* Binary search */
	while(end_idx >= start_idx) {
		center_idx = (start_idx + end_idx) / 2;

		if(idx->timestamps[center_idx] > time) {
			if(center_idx == 0)
				break;

			end_idx = center_idx - 1;
		} else if(idx->timestamps[center_idx] < time) {
			if(center_idx == SIZE_MAX)
				break;

			start_idx = center_idx + 1;
		} else {
			break;
		}
	}

	if(idx->timestamps[center_idx] == time) {
		*i = center_idx;
	} else if(center_idx > 0 && idx->timestamps[center_idx] > time) {
		*i = center_idx-1;
	} else if(idx->timestamps[center_idx] < time &&
		  (center_idx == idx->num_samples-1 || idx->timestamps[center_idx+1] > time)) {
		*i = center_idx;
	} else {
		return 0;
	}

	return 1;
}
