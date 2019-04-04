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

#ifndef AM_BUFFERED_EVENT_COLLECTION_H
#define AM_BUFFERED_EVENT_COLLECTION_H

#include <aftermath/trace/base_types.h>
#include <aftermath/trace/write_buffer.h>
#include <stdio.h>

/* The set of events recorded for an event source */
struct am_buffered_event_collection {
	/* Buffer with the events occured on the associated CPU */
	struct am_write_buffer data;

	/* If of the event collection this */
	am_event_collection_id_t id;
};

int am_buffered_event_collection_init(
	struct am_buffered_event_collection* bec,
	am_event_collection_id_t id,
	size_t buffer_size);

void
am_buffered_event_collection_destroy(struct am_buffered_event_collection* bec);

int
am_buffered_event_collection_dump_fp(struct am_buffered_event_collection* bec,
				     FILE* fp);

#endif
