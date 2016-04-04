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

#ifndef AFTERMATH_TRACE_H
#define AFTERMATH_TRACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include "arch.h"
#include "types.h"
#include "tsc.h"

struct am_buffer {
	/* Buffer storing raw event data that will be dumped to the
	 * trace file */
	void* data;

	/* Total size of the data buffer in bytes */
	size_t size;

	/* Number of bytes already used in the data buffer */
	size_t used;
};

int am_buffer_init(struct am_buffer* buf, size_t size);
void am_buffer_destroy(struct am_buffer* buf);
void* am_buffer_reserve_bytes(struct am_buffer* buf, size_t n);
int am_buffer_dump_fp(struct am_buffer* buf, FILE* fp);

struct am_trace;

/* The set of events recorded for a CPU */
struct am_event_set {
	/* Buffer with the events occured on the associated CPU */
	struct am_buffer data;

	/* Physical ID of the associated CPU */
	am_cpu_t cpu;

	/* Reference timestamp deduces from local timestamps */
	am_timestamp_diff_t ts_offs;
};

int am_event_set_init(struct am_event_set* es, uint32_t cpu, size_t data_size);
void am_event_set_destroy(struct am_event_set* es);
int am_event_set_trace_state(struct am_event_set* es, am_state_t state,
			     am_timestamp_t start, am_timestamp_t end);
int am_event_set_trace_counter(struct am_event_set* es, am_counter_t counter_id,
			       am_timestamp_t time, am_counter_value_t value);
int am_event_set_trace_omp_for_instance(struct am_event_set* es,
					am_omp_for_instance_id_t id,
					am_omp_for_address_t addr,
					am_omp_for_flags_t flags,
					am_omp_for_increment_t increment,
					am_omp_for_iterator_t lower_bound,
					am_omp_for_iterator_t upper_bound,
					am_omp_worker_t num_workers);
int am_event_set_dump_fp(struct am_event_set* es, FILE* fp);
int am_event_set_sync_timestamp_offset(struct am_event_set* es,
				       struct am_trace* trace);

/* Root data structure for an in-memory trace of the writer */
struct am_trace {
	/* Number of event sets */
	size_t num_event_sets;

	/* Number of event sets */
	size_t num_event_sets_free;

	/* Array of event tables with one entry for each CPU. */
	struct am_event_set* event_sets;

	/* Trace global data, not associated to any specific worker
	 * (e.g., event descriptions) */
	struct am_buffer data;

	/* Timestamp reference */
	struct am_timestamp_reference ts_ref;
};

int am_trace_init(struct am_trace* trace, size_t data_size);
int am_trace_set_reference_timestamp(struct am_trace* trace, am_cpu_t this_cpu);
void am_trace_destroy(struct am_trace* trace);
int am_trace_register_cpu(struct am_trace* trace, am_cpu_t cpu,
			  size_t data_size);
int am_trace_register_state(struct am_trace* trace, am_state_t state_id,
			    const char* name);
int am_trace_register_counter(struct am_trace* trace, am_counter_t counter_id,
			      const char* name);
int am_trace_start_measurement_interval(struct am_trace* trace,
					am_timestamp_t time);
int am_trace_end_measurement_interval(struct am_trace* trace,
					am_timestamp_t time);
int am_trace_dump(struct am_trace* trace, const char* filename);
int am_trace_dump_fp(struct am_trace* trace, FILE* fp);
struct am_event_set* am_trace_get_event_set(struct am_trace* trace,
					    am_cpu_t cpu);

static inline am_timestamp_t am_timestamp_now(struct am_event_set* es)
{
	return am_tsc() - es->ts_offs;
}

#ifdef __cplusplus
}
#endif

#endif
