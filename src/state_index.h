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

#ifndef STATE_INDEX_H
#define STATE_INDEX_H

#include "monotonic_index.h"

#define state_index monotonic_index
#define DEFAULT_STATE_INDEX_FACTOR 30

struct event_set;
struct filter;

int state_index_init(struct state_index* idx,
		     size_t num_states,
		     struct event_set* es,
		     int factor);

void state_index_update(struct state_index* idx,
			struct event_set* es,
			struct filter* f);

int state_index_get_state_durations(struct state_index* idx,
				    struct event_set* es,
				    struct filter* f,
				    uint64_t start,
				    uint64_t end,
				    uint64_t* durations,
				    int init,
				    int break_half);

#define state_index_destroy monotonic_index_destroy

#endif
