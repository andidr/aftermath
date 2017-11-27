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

#include <aftermath/core/statistics/interval.h>

int am_interval_stats_by_index_init(struct am_interval_stats_by_index* is,
				    size_t max_index)
{
	if(max_index == SIZE_MAX-1)
		return 0;

	is->max_index = max_index;

	if(!(is->times = calloc(is->max_index + 1, sizeof(is->times[0]))))
		return 1;

	return 0;
}

/* Resets the number of cycles to 0 for each interval index */
void am_interval_stats_by_index_reset(struct am_interval_stats_by_index* is)
{
	memset(is->times, 0, (is->max_index + 1) * sizeof(is->times[0]));
}

/* Returns the index with the maximum timestamp in *out. If there is more than
 * one index with a maximum timestamp, the lowest index is provided. If all
 * values are 0, the function returns 0, otherwise 1. */
int am_interval_stats_by_index_max(struct am_interval_stats_by_index* is,
				   size_t* out)
{
	am_timestamp_t max = 0;
	size_t ret = 0;

	for(size_t i = 0; i <= is->max_index; i++) {
		if(is->times[i] > max) {
			max = is->times[i];
			ret = i;
		}
	}

	if(max > 0) {
		*out = ret;
		return 1;
	} else {
		return 0;
	}
}

void am_interval_stats_by_index_destroy(struct am_interval_stats_by_index* is)
{
	free(is->times);
}

/* Accumulate the duration of all intervals overlapping with *query for the
 * respective indexes for all elements of an array of structures
 * arr. Element_size is the size in bytes of each array element,
 * interval_field_offset the offset in bytes of the embedded interval of a
 * structure, idx_field_offset is the offset of the index that the interval
 * should account for and idx_bits is the width in bits of the index field.
 */
void am_interval_stats_by_index_collect(struct am_interval_stats_by_index* is,
					const struct am_interval* query,
					struct am_typed_array_generic* arr,
					size_t element_size,
					off_t interval_field_offset,
					off_t idx_field_offset,
					unsigned int idx_bits)
{
	struct am_time_offset offs;
	struct am_interval* i;
	uint64_t idx;

	/* Prevent compiler from complaining about uninitialized variable */
	idx = 0;

	am_interval_array_for_each_overlapping_uint_offs(arr,
							 i,
							 &idx,
							 sizeof(idx)*8,
							 element_size,
							 interval_field_offset,
							 idx_field_offset,
							 idx_bits,
							 query)
	{
		am_interval_intersection_duration(i, query, &offs);
		am_timestamp_add_sat_offset(&is->times[idx], &offs);
	}
}
