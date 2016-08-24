/**
 * Copyright (C) 2016 Andi Drebes <andi.drebes@lip6.fr>
 *
 * Libaftermath-trace is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "state_stack.h"

/**
 * Initialize a state stack
 * @param max_entries Maximum depth of the stack
 * @return 0 on success, 1 otherwise
 */
int am_state_stack_init(struct am_state_stack* s, size_t max_entries)
{
	s->size = max_entries;
	s->top = 0;

	if(!(s->entries = malloc(s->size * sizeof(*s->entries))))
		return 1;

	return 0;
}

/**
 * Free all resources associated to a state stack
 */
void am_state_stack_destroy(struct am_state_stack* s)
{
	free(s->entries);
}

/**
 * Check if the stack is empty.
 * @return true if the stack is empty, otherwise false.
 */
int am_state_stack_is_empty(struct am_state_stack* s)
{
	return s->top == 0;
}

/**
 * Push a new state onto the stack. The timestamp of the entry is the
 * current timestamp.
 * @param state The ID of the newly entered state
 * @return 0 on success, 1 otherwise
 */
int am_state_stack_push(struct am_state_stack* s, am_state_t state, am_timestamp_t tsc)
{
	if(s->top >= s->size)
		return 1;

	s->entries[s->top].state = state;
	s->entries[s->top].start = tsc;

	s->top++;

	return 0;
}

/**
 * Push a new state onto the stack and create a new state event in an
 * event set. The timestamp for the start of the event is the
 * timestamp of the moment when the state on the top of the stack was
 * entered. The timestamp for the end is the current timestamp.
 *
 * If the stack is empty, no event is traced.
 *
 * @param state The ID of the newly entered state
 * @return 0 on success, 1 otherwise
 */
int am_state_stack_push_trace(struct am_state_stack* s,
			      struct am_event_set* es,
			      am_state_t state)
{
	am_timestamp_t tsc = am_timestamp_now(es);

	if(am_state_stack_push(s, state, tsc))
		return 1;

	if(s->top > 1) {
		if(am_event_set_trace_state(es, s->entries[s->top-2].state,
					    s->entries[s->top-2].start,
					    tsc))
		{
			return 1;
		}
	}

	return 0;
}

/**
 * Pop a state from the stack.
 * @return 0 on success, 1 otherwise (e.g., if the stack is already empty)
 */
int am_state_stack_pop(struct am_state_stack* s, am_timestamp_t tsc)
{
	if(s->top == 0)
		return 1;

	s->top--;

	if(s->top > 0)
		s->entries[s->top-1].start = tsc;

	return 0;
}

/**
 * Pop a state from the stack and create a new state event in an event
 * set. The timestamp for the start of the event is the timestamp of
 * the moment when the state on the top of the stack was entered. The
 * timestamp for the end is the current timestamp.
 *
 * @param es The event set in which the event should be traced
 * @return 0 on success, 1 otherwise
 */
int am_state_stack_pop_trace(struct am_state_stack* s, struct am_event_set* es)
{
	am_timestamp_t tsc = am_timestamp_now(es);

	if(s->top == 0)
		return 1;

	if(am_event_set_trace_state(es, s->entries[s->top-1].state,
				    s->entries[s->top-1].start,
				    tsc))
	{
		return 1;
	}

	return am_state_stack_pop(s, tsc);
}
