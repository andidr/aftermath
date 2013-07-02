/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
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

#include "counter_event_set.h"

int counter_event_set_get_event_outside_interval(struct counter_event_set* es, uint64_t counter_id, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_events-1;
	int center_idx = 0;

	if(es->num_events == 0)
		return -1;

	while(end_idx - start_idx > 1) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->events[center_idx].time > interval_end)
			end_idx = center_idx;
		else if(es->events[center_idx].time < interval_start)
			start_idx = center_idx;
		else
			break;
	}

	while(center_idx > 0 && es->events[center_idx].time >= interval_start)
		center_idx--;

	return center_idx;
}
