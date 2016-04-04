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

#include "trace.h"
#include "trace_file.h"
#include "convert.h"
#include <time.h>
#include <string.h>

/**
 * Initialize data buffer
 * @param size Maximum size of the buffer
 * @return 0 on success, otherwise 1
 */
int am_buffer_init(struct am_buffer* buf, size_t size)
{
	if(!(buf->data = malloc(size)))
		return 1;

	buf->size = size;
	buf->used = 0;

	return 0;
}

/**
 * Free all resources of a data buffer
 */
void am_buffer_destroy(struct am_buffer* buf)
{
	free(buf->data);
}

/**
 * Reserve n bytes in the data buffer of an event set. If less than n
 * bytes are available the function returns NULL. Otherwise it returns
 * the pointer to the beginning of the reserved memory region within
 * the buffer.
 */
void* am_buffer_reserve_bytes(struct am_buffer* buf, size_t n)
{
	void* ret;

	if(buf->size - buf->used < n)
		return NULL;

	ret = buf->data + buf->used;
	buf->used += n;

	return ret;
}

/**
 * Dump all events of a data buffer set to the file fp.
 * @return 0 on success, 1 on failure
 */
int am_buffer_dump_fp(struct am_buffer* buf, FILE* fp)
{
	if(buf->used == 0)
		return 0;

	if(fwrite(buf->data, buf->used, 1, fp) != 1)
		return 1;

	buf->used = 0;

	return 0;
}

/**
 * Initialize an in-memory trace for the writer.
 * @return 0 on success, otherwise 1
 */
int am_trace_init(struct am_trace* trace, size_t data_size)
{
	if(am_buffer_init(&trace->data, data_size))
		return 1;

	trace->num_event_sets = 0;
	trace->num_event_sets_free = 0;
	trace->event_sets = NULL;

	am_timestamp_reference_init(&trace->ts_ref);

	return 0;
}

/**
 * Free all resources of a trace
 */
void am_trace_destroy(struct am_trace* trace)
{
	for(size_t i = 0; i < trace->num_event_sets; i++)
		am_event_set_destroy(&trace->event_sets[i]);

	free(trace->event_sets);
	am_buffer_destroy(&trace->data);
}

/**
 * Set the timestamp reference
 *
 * @param this_cpu CPU identifier of the calling CPU
 * @return 0 on success, 1 otherwise (e.g., if the reference has
 * already been initialized)
 */
int am_trace_set_reference_timestamp(struct am_trace* trace, am_cpu_t this_cpu)
{
	return am_timestamp_reference_set(&trace->ts_ref, this_cpu);
}

/**
 * Initialize an event set
 * @param cpu Physical ID of the CPU to be added
 * @param data_size Initial size in bytes of the data buffer
 * @return 0 on success, 1 on failure
 */
int am_event_set_init(struct am_event_set* es, am_cpu_t cpu, size_t data_size)
{
	if(am_buffer_init(&es->data, data_size))
		return 1;

	es->cpu = cpu;

	return 0;
}

/**
 * Free all resources of an event set
 */
void am_event_set_destroy(struct am_event_set* es)
{
	am_buffer_destroy(&es->data);
}

/**
 * Trace a state event
 * @param state The ID of the state to be traced
 * @param start Start of the state event
 * @param end End of the state event
 * @return 0 on success, 1 otherwise
 */
int am_event_set_trace_state(struct am_event_set* es, am_state_t state,
			     am_timestamp_t start, am_timestamp_t end)
{
	struct trace_state_event* dsk_se;

	if(!(dsk_se = am_buffer_reserve_bytes(&es->data, sizeof(*dsk_se))))
		return 1;

	dsk_se->header.type = EVENT_TYPE_STATE;
	dsk_se->header.time = start;
	dsk_se->header.worker = 0;
	dsk_se->header.cpu = es->cpu;
	dsk_se->header.active_task = 0;
	dsk_se->header.active_frame = 0;
	dsk_se->end_time = end;
	dsk_se->state = state;

	convert_struct(dsk_se,
		       trace_state_event_conversion_table,
		       0,
		       CONVERT_HOST_TO_DSK);

	return 0;
}

/**
 * Trace a counter event
 * @param counter_id The ID of the counter to be traced
 * @param time Timestamp of the counter sample
 * @param value Value of the counter sample
 * @return 0 on success, 1 otherwise
 */
int am_event_set_trace_counter(struct am_event_set* es, am_counter_t counter_id,
			       am_timestamp_t time, am_counter_value_t value)
{
	struct trace_counter_event* dsk_ce;

	if(!(dsk_ce = am_buffer_reserve_bytes(&es->data, sizeof(*dsk_ce))))
		return 1;

	dsk_ce->header.type = EVENT_TYPE_COUNTER;
	dsk_ce->header.time = time;
	dsk_ce->header.worker = 0;
	dsk_ce->header.cpu = es->cpu;
	dsk_ce->header.active_task = 0;
	dsk_ce->header.active_frame = 0;
	dsk_ce->counter_id = counter_id;
	dsk_ce->value = value;

	convert_struct(dsk_ce,
		       trace_counter_event_conversion_table,
		       0,
		       CONVERT_HOST_TO_DSK);

	return 0;
}

/**
 * Trace an OpenMP for instance
 * @param id The ID of the for loop instance
 * @param addr The address of the for loop (e.g., the address of the
 * instruction calling the run-time to determine the bounds of a
 * chunk)
 * @param flags A set of flags from enum omp_for_flag
 * @param increment Loop increment; raw 64-bit representation,
 * including signed extension for signed increments
 * @param upper_bound Upper bound of the iteration space; raw 64-bit
 * representation, including signed extension for signed loop bounds
 * @param num_workers Number of workers executing the loop
 * @return 0 on success, 1 otherwise
 */
int am_event_set_trace_omp_for_instance(struct am_event_set* es,
					am_omp_for_instance_id_t id,
					am_omp_for_address_t addr,
					am_omp_for_flags_t flags,
					am_omp_for_increment_t increment,
					am_omp_for_iterator_t lower_bound,
					am_omp_for_iterator_t upper_bound,
					am_omp_worker_t num_workers)
{
	struct trace_omp_for_instance* dsk_ofi;

	if(!(dsk_ofi = am_buffer_reserve_bytes(&es->data, sizeof(*dsk_ofi))))
		return 1;

	dsk_ofi->type = EVENT_TYPE_OMP_FOR;
	dsk_ofi->id = id;
	dsk_ofi->addr = addr;
	dsk_ofi->flags = flags;
	dsk_ofi->increment = increment;
	dsk_ofi->lower_bound = lower_bound;
	dsk_ofi->upper_bound = upper_bound;
	dsk_ofi->num_workers = num_workers;

	convert_struct(dsk_ofi,
		       trace_omp_for_conversion_table,
		       0,
		       CONVERT_HOST_TO_DSK);

	return 0;
}

/**
 * Trace an OpenMP for chunk set
 * @param id The ID of the for chunk set
 * @param for_id ID of the associated for loop instance
 * @param first_lower Lower bound of the chunk_set's iteration space;
 * raw 64-bit representation, including signed extension for signed
 * loop bounds
 * @param first_upper Upper bound of the chunk_set's iteration space;
 * raw 64-bit representation, including signed extension for signed
 * loop bounds
 * @return 0 on success, 1 otherwise
 */
int am_event_set_trace_omp_for_chunk_set(struct am_event_set* es,
					 am_omp_for_instance_id_t for_id,
					 am_omp_for_chunk_set_id_t id,
					 am_omp_for_iterator_t first_lower,
					 am_omp_for_iterator_t first_upper)
{
	struct trace_omp_for_chunk_set* dsk_ofcs;

	if(!(dsk_ofcs = am_buffer_reserve_bytes(&es->data, sizeof(*dsk_ofcs))))
		return 1;

	dsk_ofcs->type = EVENT_TYPE_OMP_FOR_CHUNK_SET;
	dsk_ofcs->id = id;
	dsk_ofcs->for_id = for_id;
	dsk_ofcs->first_lower = first_lower;
	dsk_ofcs->first_upper = first_upper;

	convert_struct(dsk_ofcs,
		       trace_omp_for_chunk_set_conversion_table,
		       0,
		       CONVERT_HOST_TO_DSK);

	return 0;
}

/**
 * Trace an OpenMP for chunk set part
 * @param chunk_set_id The ID of the for chunk set
 * @param start Start of the execution of the chunk set part
 * @param end End of the execution of the chunk set part
 * @return 0 on success, 1 otherwise
 */
int am_event_set_trace_omp_for_chunk_set_part(struct am_event_set* es,
					      am_omp_for_chunk_set_id_t chunk_set_id,
					      am_timestamp_t start,
					      am_timestamp_t end)
{
	struct trace_omp_for_chunk_set_part* dsk_ofcsp;

	if(!(dsk_ofcsp = am_buffer_reserve_bytes(&es->data, sizeof(*dsk_ofcsp))))
		return 1;

	dsk_ofcsp->type = EVENT_TYPE_OMP_FOR_CHUNK_SET_PART;
	dsk_ofcsp->cpu = es->cpu;
	dsk_ofcsp->chunk_set_id = chunk_set_id;
	dsk_ofcsp->start = start;
	dsk_ofcsp->end = end;

	convert_struct(dsk_ofcsp,
		       trace_omp_for_chunk_set_part_conversion_table,
		       0,
		       CONVERT_HOST_TO_DSK);

	return 0;
}

/**
 * Synchronize the timestamp offset of an event set with the reference
 * of a trace. The function must be called by the CPU associated to
 * the event set.
 *
 * @param trace The trace to synchronize with
 * @return 0 on success, 1 otherwise (e.g., if the reference timestamp
 * of trace has not been initialized before)
 */
int am_event_set_sync_timestamp_offset(struct am_event_set* es,
				       struct am_trace* trace)
{
	am_nanoseconds_t ns = am_current_nanoseconds();
	am_timestamp_t ts = am_tsc();
	am_nanoseconds_diff_t ns_diff;
	am_timestamp_t ns_ts;

	if(!trace->ts_ref.set)
		return 1;

	/* Difference in nanoseconds to reference */
	ns_diff = (am_nanoseconds_diff_t)ns -
		(am_nanoseconds_diff_t)trace->ts_ref.ref_ns;

	/* Difference between this moment and reference nanoseconds in
	 * timestamps */
	ns_ts = (ns_diff * trace->ts_ref.tsps) / 1000000000;

	/* What would the timestamp of this CPU have been at the
	 * reference nanoseconds? */
	am_timestamp_t norm_ts = ts - ns_ts;

	/* Timestamp offset  */
	es->ts_offs = (am_timestamp_diff_t)norm_ts;

	return 0;
}

static int trace_global_single_event(struct am_trace* trace,
				     enum global_single_event_type type,
				     am_timestamp_t time)
{
	struct trace_global_single_event* dsk_gse;

	if(!(dsk_gse = am_buffer_reserve_bytes(&trace->data, sizeof(*dsk_gse))))
		return 1;

	dsk_gse->type = EVENT_TYPE_GLOBAL_SINGLE_EVENT;
	dsk_gse->time = time;
	dsk_gse->single_type = type;

	convert_struct(dsk_gse,
		       trace_global_single_event_conversion_table,
		       0,
		       CONVERT_HOST_TO_DSK);

	return 0;
}

/**
 * Trace a measurement interval start event
 * @param time The timestamp of the event
 * @return 0 on success, 1 otherwise
 */
int am_trace_start_measurement_interval(struct am_trace* trace, am_timestamp_t time)
{
	return trace_global_single_event(trace,
					 GLOBAL_SINGLE_TYPE_MEASURE_START,
					 time);
}

/**
 * Trace a measurement interval end event
 * @param time The timestamp of the event
 * @return 0 on success, 1 otherwise
 */
int am_trace_end_measurement_interval(struct am_trace* trace, am_timestamp_t time)
{
	return trace_global_single_event(trace,
					 GLOBAL_SINGLE_TYPE_MEASURE_END,
					 time);
}

/**
 * Dump all events of an event set to the file fp.
 * @return 0 on success, 1 on failure
 */
int am_event_set_dump_fp(struct am_event_set* es, FILE* fp)
{
	return am_buffer_dump_fp(&es->data, fp);
}


/**
 * Add a CPU to a trace
 * @param cpu Physical ID of the CPU to be added
 * @param data_size Initial size in bytes of the data buffer of the event set
 * @return 0 on success, 1 on failure
 */
int am_trace_register_cpu(struct am_trace* trace,
			  am_cpu_t cpu, size_t data_size)
{
	struct am_event_set* tmp;

	if(!trace->num_event_sets_free) {
		if(!(tmp = realloc(trace->event_sets,
				   sizeof(trace->event_sets[0]) *
				   (trace->num_event_sets+1))))
		{
			return 1;
		}

		trace->event_sets = tmp;
		trace->num_event_sets_free++;
	}

	if(am_event_set_init(&trace->event_sets[trace->num_event_sets],
			     cpu, data_size))
	{
		return 1;
	}

	trace->num_event_sets_free--;
	trace->num_event_sets++;

	return 0;
}

/**
 * Add a state to a trace
 * @param state_id ID of the new state
 * @param name Name of the state that is to be associated with the ID
 * @return 0 on success, 1 on failure
 */
int am_trace_register_state(struct am_trace* trace, am_state_t state_id,
			    const char* name)
{
	struct trace_state_description* dsk_sd;
	size_t name_len = strlen(name);
	size_t size = sizeof(*dsk_sd)+name_len;

	if(!(dsk_sd = am_buffer_reserve_bytes(&trace->data, size)))
		return 1;

	dsk_sd->type = EVENT_TYPE_STATE_DESCRIPTION;
	dsk_sd->state_id = state_id;
	dsk_sd->name_len = name_len;

	convert_struct(dsk_sd,
		       trace_state_description_conversion_table,
		       0,
		       CONVERT_HOST_TO_DSK);

	memcpy(dsk_sd+1, name, name_len);

	return 0;
}

/**
 * Add a counter to a trace
 * @param counter_id ID of the new counter
 * @param name Name of the counter that is to be associated with the ID
 * @return 0 on success, 1 on failure
 */
int am_trace_register_counter(struct am_trace* trace, am_counter_t counter_id,
			      const char* name)
{
	struct trace_counter_description* dsk_cd;
	size_t name_len = strlen(name);
	size_t size = sizeof(*dsk_cd)+name_len;

	if(!(dsk_cd = am_buffer_reserve_bytes(&trace->data, size)))
		return 1;

	dsk_cd->type = EVENT_TYPE_COUNTER_DESCRIPTION;
	dsk_cd->counter_id = counter_id;
	dsk_cd->name_len = name_len;

	convert_struct(dsk_cd,
		       trace_counter_description_conversion_table,
		       0,
		       CONVERT_HOST_TO_DSK);

	memcpy(dsk_cd+1, name, name_len);

	return 0;
}

/**
 * Write the header of a trace file*
 * @return 0 on success, otherwise 1
 */
static int am_trace_write_header_fp(struct am_trace* trace, FILE* fp)
{
	time_t t = time(NULL);
	struct tm* now = localtime(&t);
	struct trace_header dsk_header;

	/* Write header */
	dsk_header.magic = TRACE_MAGIC;
	dsk_header.version = TRACE_VERSION;
	dsk_header.day = now->tm_mday;
	dsk_header.month = now->tm_mon+1;
	dsk_header.year = now->tm_year+1900;
	dsk_header.hour = now->tm_hour;
	dsk_header.minute = now->tm_min;

	return write_struct_convert(fp, &dsk_header, sizeof(dsk_header),
				    trace_event_header_conversion_table, 0);
}

/**
 * Write the contents of a trace to a file already opened
 * @return 0 on sucess, 1 on failure
 */
int am_trace_dump_fp(struct am_trace* trace, FILE* fp)
{
	if(am_trace_write_header_fp(trace, fp))
		return 1;

	if(am_buffer_dump_fp(&trace->data, fp))
		return 1;

	for(size_t i = 0; i < trace->num_event_sets; i++)
		if(am_event_set_dump_fp(&trace->event_sets[i], fp))
			return 1;

	return 0;
}

/**
 * Write the contents of a trace to a file
 * @return 0 on sucess, 1 on failure
 */
int am_trace_dump(struct am_trace* trace, const char* filename)
{
	FILE* fp;
	int ret = 1;

	if(!(fp = fopen(filename, "wb+")))
		goto out;

	if(am_trace_dump_fp(trace, fp))
		goto out_fp;

	ret = 0;
out_fp:
	fclose(fp);
out:
	return ret;
}

/**
 * Retrieve the event set associated to a cpu. This function can only
 * be called once all CPUs have been registered, as pointers to event
 * sets previously returned by the function become invalid when a new
 * CPU is registered.
 *
 * @return NULL on failure (e.g., if the CPU has not been registered),
 * otherwise a pointer to the event set associated to the CPU
 */
struct am_event_set* am_trace_get_event_set(struct am_trace* trace,
					    am_cpu_t cpu)
{
	for(size_t i = 0; i < trace->num_event_sets; i++)
		if(trace->event_sets[i].cpu == cpu)
			return &trace->event_sets[i];

	return NULL;
}
