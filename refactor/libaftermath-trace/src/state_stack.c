/**
 * Author: Andi Drebes <andi@drebesium.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include <aftermath/trace/state_stack.h>
#include <aftermath/trace/on_disk_structs.h>
#include <aftermath/trace/on_disk_write_to_buffer.h>
#include <aftermath/trace/safe_alloc.h>
#include <aftermath/trace/timestamp.h>

/**
 * Initialize a state stack
 * @param max_entries Maximum depth of the stack
 * @return 0 on success, 1 otherwise
 */
int am_state_stack_init(struct am_state_stack* s, size_t max_entries)
{
	s->size = max_entries;
	s->top = 0;

	if(!(s->entries = am_alloc_array_safe(s->size, sizeof(*s->entries))))
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
int am_state_stack_push(struct am_state_stack* s,
			am_state_t state,
			am_timestamp_t tsc)
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
 * event collection. The timestamp for the start of the event is the
 * timestamp of the moment when the state on the top of the stack was
 * entered. The timestamp for the start of the new state start_ts.
 *
 * If the stack is empty, no event is traced.
 *
 * @param bec The event collection for which the event should be traced
 * @param state The ID of the newly entered state
 * @param end_ts Timestamp for the start of the new state
 * @param state_event_type_id The numerical on-disk ID for the type
 * am_dsk_state_event
 * @return 0 on success, 1 otherwise
 */
int am_state_stack_push_trace(struct am_state_stack* s,
			      struct am_buffered_event_collection* bec,
			      am_state_t state,
			      am_timestamp_t start_ts,
			      uint32_t state_event_type_id)
{
	struct am_dsk_state_event se;

	if(am_state_stack_push(s, state, start_ts))
		return 1;

	if(s->top > 1) {
		se.collection_id = bec->id;
		se.state = s->entries[s->top-2].state;
		se.interval.start = s->entries[s->top-2].start;
		se.interval.end = start_ts;

		if(am_dsk_state_event_write_to_buffer(&bec->data,
						      &se,
						      state_event_type_id))
		{
			return 1;
		}
	}

	return 0;
}

/**
 * Try to pop a state from the stack.
 * @return 1 if a state was popped, otherwise 0.
 */
int am_state_stack_try_pop(struct am_state_stack* s, am_timestamp_t tsc)
{
	if(s->top == 0)
		return 0;

	s->top--;

	if(s->top > 0)
		s->entries[s->top-1].start = tsc;

	return 1;
}

/**
 * Pop a state from the stack.
 * @return 0 on success, 1 otherwise (e.g., if the stack is already empty)
 */
int am_state_stack_pop(struct am_state_stack* s, am_timestamp_t tsc)
{
	return !am_state_stack_try_pop(s, tsc);
}

/**
 * Pop a state from the stack and create a new state event in an event
 * collection. The timestamp for the start of the event is the timestamp of
 * the moment when the state on the top of the stack was entered. The
 * timestamp for the end is given by end_ts.
 *
 * @param bec The event collection for which the event should be traced
 * @param end_ts The timestamp for the end of the popped state event
 * @param state_event_type_id The numerical on-disk ID for the type
 * am_dsk_state_event
 * @return 0 on success, 1 otherwise
 */
int am_state_stack_pop_trace(struct am_state_stack* s,
			     struct am_buffered_event_collection* bec,
			     am_timestamp_t end_ts,
			     uint32_t state_event_type_id)
{
	struct am_dsk_state_event se;

	if(s->top == 0)
		return 1;

	se.collection_id = bec->id;
	se.state = s->entries[s->top-1].state;
	se.interval.start = s->entries[s->top-1].start;
	se.interval.end = end_ts;

	if(am_dsk_state_event_write_to_buffer(&bec->data,
					      &se,
					      state_event_type_id))
	{
		return 1;
	}

	return am_state_stack_pop(s, end_ts);
}

/**
 * Try to pop a state from the stack and create a new state event in an event
 * collection. The timestamp for the start of the event is the timestamp of the
 * moment when the state on the top of the stack was entered. The timestamp for
 * the end is given in end_ts.
 *
 * @param bec The event collection for which the event should be traced
 * @param end_ts The timestamp for the end of the popped state event
 * @param err Set to 1 if a state was present on the stack, but could not be traced
 * or set to 0 if a state was popped and traced successfully. If no state is
 * popped, the value remains unchanged. Can be NULL if no information on errors
 * is needed by the caller
 * @param state_event_type_id The numerical on-disk ID for the type
 * am_dsk_state_event
 * @return 1 if a state was present, otherwise 0
 */
int am_state_stack_try_pop_trace(struct am_state_stack* s,
				 struct am_buffered_event_collection* bec,
				 am_timestamp_t end_ts,
				 int* err,
				 uint32_t state_event_type_id)
{
	struct am_dsk_state_event se;

	if(s->top == 0)
		return 0;

	se.collection_id = bec->id;
	se.state = s->entries[s->top-1].state;
	se.interval.start = s->entries[s->top-1].start;
	se.interval.end = end_ts;

	if(am_dsk_state_event_write_to_buffer(&bec->data,
					      &se,
					      state_event_type_id) ||
	   am_state_stack_pop(s, end_ts))
	{
		if(err)
			*err = 1;

		return 1;
	} else {
		if(err)
			*err = 0;
	}

	return 1;
}
