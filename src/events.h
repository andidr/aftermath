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

#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>
#include "trace_file.h"

#define EVENT_PREALLOC (5*1024)

struct state_event {
	uint64_t start;
	uint64_t end;
	uint64_t active_task;
	int state;
};

struct comm_event {
	uint64_t time;
	int dst_cpu;
	int dst_worker;
	int size;
	enum comm_event_type type;
	uint64_t active_task;
	uint64_t what;
};

struct counter_event {
	uint64_t time;
	uint64_t active_task;
	uint64_t counter_id;
	int64_t value;
	long double slope;
	int counter_index;
};

struct single_event {
	uint64_t time;
	enum single_event_type type;
	uint64_t active_task;
};

#endif
