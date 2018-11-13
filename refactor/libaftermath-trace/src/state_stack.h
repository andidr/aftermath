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

#ifndef AM_STATE_STACK_H
#define AM_STATE_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aftermath/trace/base_types.h>
#include <aftermath/trace/buffered_event_collection.h>

/* Single entry on a state stack */
struct am_state_stack_entry {
	/* Id of the associated state */
	am_state_t state;

	/* Timestamp of the moment when the state was entered */
	am_timestamp_t start;
};

struct am_state_stack {
	/* Next position */
	size_t top;

	/* Maximal depth of the stack */
	size_t size;

	/* Actual stack entries */
	struct am_state_stack_entry* entries;
};

int am_state_stack_init(struct am_state_stack* s, size_t max_entries);
void am_state_stack_destroy(struct am_state_stack* s);
int am_state_stack_is_empty(struct am_state_stack* s);
int am_state_stack_push(struct am_state_stack* s,
			am_state_t state,
			am_timestamp_t tsc);
int am_state_stack_push_trace(struct am_state_stack* s,
			      struct am_buffered_event_collection* bec,
			      am_state_t state,
			      am_timestamp_t start_ts,
			      uint32_t state_event_type_id);
int am_state_stack_pop(struct am_state_stack* s, am_timestamp_t tsc);
int am_state_stack_try_pop_trace(struct am_state_stack* s,
				 struct am_buffered_event_collection* bec,
				 am_timestamp_t end_ts,
				 int* err,
				 uint32_t state_event_type_id);
int am_state_stack_pop_trace(struct am_state_stack* s,
			     struct am_buffered_event_collection* bec,
			     am_timestamp_t end_ts,
			     uint32_t state_event_type_id);

#ifdef __cplusplus
}
#endif

#endif
