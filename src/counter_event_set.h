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
#include <stdint.h>
#include <malloc.h>

struct counter_event_set {
	struct counter_event* events;
	int num_events;
	int num_events_free;
	uint64_t counter_id;
	int counter_index;
};

int counter_event_set_get_event_outside_interval(struct counter_event_set* es, uint64_t interval_start, uint64_t interval_end);

static inline void counter_event_set_destroy(struct counter_event_set* ces)
{
	free(ces->events);
}

static inline void counter_event_set_init(struct counter_event_set* ces, uint64_t counter_id, int counter_index)
{
	ces->events = NULL;
	ces->num_events = 0;
	ces->num_events_free = 0;
	ces->counter_id = counter_id;
	ces->counter_index = counter_index;
}

#endif
