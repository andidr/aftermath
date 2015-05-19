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

#ifndef COUNTER_EVENT_SET
#define COUNTER_EVENT_SET

#include "events.h"
#include "buffer.h"
#include <stdint.h>
#include <malloc.h>

struct counter_event_set {
	struct counter_event* events;
	struct counter_description* desc;
	int num_events;
	int num_events_free;
};

int counter_event_set_get_last_event_in_interval(struct counter_event_set* es, uint64_t interval_start, uint64_t interval_end);
uint64_t counter_event_set_get_value(struct counter_event_set* ces, uint64_t time);

int counter_event_set_interpolate_value(struct counter_event_set* ces, uint64_t time, int64_t* val_out);

int counter_event_set_is_monotonously_increasing(struct counter_event_set* ces, struct counter_description** cd, int* cpu);

static inline void counter_event_set_destroy(struct counter_event_set* ces)
{
	free(ces->events);
}

static inline void counter_event_set_init(struct counter_event_set* ces, struct counter_description* cd)
{
	ces->events = NULL;
	ces->num_events = 0;
	ces->num_events_free = 0;
	ces->desc = cd;
}

static inline void counter_event_set_add_offset(struct counter_event_set* ces, int64_t offset)
{
	for(int i = 0; i < ces->num_events; i++)
		ces->events[i].time += offset;
}

static inline int counter_event_set_add_event(struct counter_event_set* ces, struct counter_event* ce, int calc_slope)
{
	if(add_buffer_grow((void**)&ces->events, ce, sizeof(*ce),
			&ces->num_events, &ces->num_events_free,
			   EVENT_PREALLOC) != 0)
	{
		return 1;
	}


	if(calc_slope) {
		if(ces->num_events >= 2) {
			ce->slope = (long double)(ces->events[ces->num_events-1].value - ces->events[ces->num_events-2].value) /
				(long double)(ces->events[ces->num_events-1].time - ces->events[ces->num_events-2].time);
		} else {
			ce->slope = 0;
		}
	}

	ces->events[ces->num_events-1].slope = ce->slope;

	return 0;
}

#endif
