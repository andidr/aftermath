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

#ifndef AM_STATISTICS_INTERVAL_H
#define AM_STATISTICS_INTERVAL_H

#include <aftermath/core/base_types.h>
#include <aftermath/core/state_event_array.h>

/* A statistics object that accumulates the total time for indexes ranging from
 * 0 to num_times-1 */
struct am_interval_stats_by_index {
	am_timestamp_t* times;
	size_t max_index;
};

int am_interval_stats_by_index_init(struct am_interval_stats_by_index* is,
				    size_t max_index);
void am_interval_stats_by_index_reset(struct am_interval_stats_by_index* is);
int am_interval_stats_by_index_max(struct am_interval_stats_by_index* is, size_t* idx);
void am_interval_stats_by_index_destroy(struct am_interval_stats_by_index* is);

void am_interval_stats_by_index_collect(struct am_interval_stats_by_index* is,
					const struct am_interval* query,
					struct am_typed_array_generic* arr,
					size_t element_size,
					off_t interval_field_offset,
					off_t idx_field_offset,
					unsigned int idx_bits);

#endif
