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

#ifndef AM_STATISTICS_DISCRETE_H
#define AM_STATISTICS_DISCRETE_H


#include <aftermath/core/base_types.h>
#include <aftermath/core/in_memory.h>
#include <aftermath/core/timestamp_array.h>

/* A statistics object that accumulates the total number of occurences for
 * indexes ranging from 0 to max_index */
struct am_discrete_stats_by_index {
	size_t* counts;
	size_t max_index;
};

int am_discrete_stats_by_index_init(struct am_discrete_stats_by_index* is,
				    size_t max_index);
void am_discrete_stats_by_index_reset(struct am_discrete_stats_by_index* is);
int am_discrete_stats_by_index_max(struct am_discrete_stats_by_index* is, size_t* idx);
void am_discrete_stats_by_index_destroy(struct am_discrete_stats_by_index* is);

void am_discrete_stats_by_index_collect(struct am_discrete_stats_by_index* is,
					const struct am_interval* query,
					struct am_typed_array_generic* arr,
					size_t element_size,
					off_t timestamp_field_offset,
					off_t idx_field_offset,
					unsigned int idx_bits);

void am_discrete_stats_by_index_fun_collect(
	struct am_discrete_stats_by_index* is,
	const struct am_interval* query,
	struct am_typed_array_generic* arr,
	size_t element_size,
	off_t timestamp_field_offset,
	size_t (*calculate_index)(void*, void*),
	void* data);

#endif
