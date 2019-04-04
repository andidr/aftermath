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

#ifndef AFTERMATH_BUFFERED_TRACE_H
#define AFTERMATH_BUFFERED_TRACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aftermath/trace/buffered_event_collection.h>
#include <aftermath/trace/simple_hierarchy.h>
#include <stdlib.h>
#include <stdio.h>

/* Root data structure for a trace during sampling. */
struct am_buffered_trace {
	/* Number of buffered event collections */
	size_t num_collections;

	/* Array with all buffered event collections */
	struct am_buffered_event_collection** collections;

	/* For num_collections > 0: highest ID among all event
	 * collections belonging to this trace */
	am_event_collection_id_t highest_collection_id;

	/* Number of hierarchies */
	size_t num_hierarchies;

	/* Array with all hierarchies */
	struct am_simple_hierarchy** hierarchies;

	/* For num_hierarchies > 0: highest ID among all hierarchies belonging
	 * to this trace */
	am_hierarchy_id_t highest_hierarchy_id;

	/* Trace global data, not associated to any specific worker
	 * (e.g., event descriptions) */
	struct am_write_buffer data;
};

int am_buffered_trace_init(struct am_buffered_trace* bt, size_t data_size);
void am_buffered_trace_destroy(struct am_buffered_trace* bt);

int am_buffered_trace_dump(struct am_buffered_trace* bt, const char* filename);
int am_buffered_trace_dump_fp(struct am_buffered_trace* bt, FILE* fp);

struct am_buffered_event_collection*
am_buffered_trace_new_collection(struct am_buffered_trace* bt,
				 size_t buffer_size);

int am_buffered_trace_add_collection(struct am_buffered_trace* bt,
				     struct am_buffered_event_collection* bec);

struct am_simple_hierarchy*
am_buffered_trace_new_hierarchy(struct am_buffered_trace* bt,
				const char* name,
				const char* spec);

struct am_simple_hierarchy*
am_buffered_trace_get_hierarchy(struct am_buffered_trace* bt,
				const char* name);

#ifdef __cplusplus
}
#endif

#endif
