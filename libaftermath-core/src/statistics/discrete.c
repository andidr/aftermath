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

#include <aftermath/core/statistics/discrete.h>

int am_discrete_stats_by_index_init(struct am_discrete_stats_by_index* is,
				    size_t max_index)
{
	if(max_index == SIZE_MAX-1)
		return 0;

	is->max_index = max_index;

	if(!(is->counts = calloc(is->max_index + 1, sizeof(is->counts[0]))))
		return 1;

	return 0;
}

/* Resets the number of cycles to 0 for each discrete index */
void am_discrete_stats_by_index_reset(struct am_discrete_stats_by_index* is)
{
	memset(is->counts, 0, (is->max_index + 1) * sizeof(is->counts[0]));
}

/* Returns the index with the maximum count in *out. If there is more than one
 * index with a maximum count, the lowest index is provided. If all values are
 * 0, the function returns 0, otherwise 1. */
int am_discrete_stats_by_index_max(struct am_discrete_stats_by_index* is,
				   size_t* out)
{
	size_t max = 0;
	size_t ret = 0;

	for(size_t i = 0; i <= is->max_index; i++) {
		if(is->counts[i] > max) {
			max = is->counts[i];
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

void am_discrete_stats_by_index_destroy(struct am_discrete_stats_by_index* is)
{
	free(is->counts);
}

/* Accumulate the count of all discrete events with a timestamp in *query for
 * the respective indexes for all elements of an array of structures
 * arr. Element_size is the size in bytes of each array element,
 * timestamp_field_offset the offset in bytes of the embedded timestamp of a
 * structure, idx_field_offset is the offset of the index that the timestamp
 * should account for and idx_bits is the width in bits of the index field.
 */
void am_discrete_stats_by_index_collect(struct am_discrete_stats_by_index* is,
					const struct am_interval* query,
					struct am_typed_array_generic* arr,
					size_t element_size,
					off_t timestamp_field_offset,
					off_t idx_field_offset,
					unsigned int idx_bits)
{
	am_timestamp_t* t;
	uint64_t idx;

	/* Prevent compiler from complaining about uninitialized variable */
	idx = 0;

	am_timestamp_array_for_each_within_uint_offs(arr,
						     t,
						     &idx,
						     sizeof(idx)*8,
						     element_size,
						     timestamp_field_offset,
						     idx_field_offset,
						     idx_bits,
						     query)
	{
		am_add_sat_size(is->counts[idx], 1, &is->counts[idx]);
	}
}

/* Accumulate the counts of all events whose timestamp is within *query for the
 * respective indexes for all elements of an array of structures
 * arr. Element_size is the size in bytes of each array element,
 * timestamp_field_offset the offset in bytes of the embedded timestamp of a
 * structure.
 *
 * Calculate_index is a function that is called for each element that return the
 * index for the element. The pointer data is passed verbatim to the function.
 */
void am_discrete_stats_by_index_fun_collect(
	struct am_discrete_stats_by_index* is,
	const struct am_interval* query,
	struct am_typed_array_generic* arr,
	size_t element_size,
	off_t timestamp_field_offset,
	size_t (*calculate_index)(void*, void*),
	void* data)
{
	am_timestamp_t* t;
	size_t idx;

	/* Prevent compiler from complaining about uninitialized variable */
	idx = 0;

	am_timestamp_array_for_each_within_offs(
		arr, t, element_size, timestamp_field_offset, query)
	{
		idx = calculate_index(data, AM_PTR_SUB(t, timestamp_field_offset));
		am_add_sat_size(is->counts[idx], 1, &is->counts[idx]);
	}
}
