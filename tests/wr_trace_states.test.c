/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
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

#include <unit_tests.h>
#include <time.h>
#include "common.h"
#include "../src/events.h"
#include "../src/trace_file.h"
#include "../src/convert.h"
#include "../src/multi_event_set.h"

/* Fills a header structure with default data */
static inline void fill_header(struct trace_header* dsk_header)
{
	/* Determine date and time */
	time_t t = time(NULL);
	struct tm* now = localtime(&t);

	/* Fill header */
	dsk_header->magic = TRACE_MAGIC;
	dsk_header->version = TRACE_VERSION;
	dsk_header->day = now->tm_mday;
	dsk_header->month = now->tm_mon+1;
	dsk_header->year = now->tm_year+1900;
	dsk_header->hour = now->tm_hour;
	dsk_header->minute = now->tm_min;
}

static inline int write_default_header(FILE* fp)
{
	struct trace_header dsk_header;

	/* Initialize header */
	fill_header(&dsk_header);

	/* Convert header to on-disk endianness and write to disk */
	return write_struct_convert(fp, &dsk_header, sizeof(dsk_header),
				    trace_header_conversion_table, 0);
}

static inline int write_state_event(FILE* fp, uint32_t cpu,
				    uint64_t start, uint64_t end,
				    uint32_t state)
{
	struct trace_state_event dsk_se;

	dsk_se.header.type = EVENT_TYPE_STATE;
	dsk_se.header.cpu = cpu;
	dsk_se.header.worker = cpu;
	dsk_se.header.active_task = 0;
	dsk_se.header.active_frame = 0;
	dsk_se.header.time = start;
	dsk_se.end_time = end;
	dsk_se.state = state;

	/* Convert state event to on-disk endianness and write to disk */
	return write_struct_convert(fp, &dsk_se, sizeof(dsk_se),
				    trace_state_event_conversion_table, 0);
}

static inline int write_state_description(FILE* fp, uint32_t state_id,
					  const char* name)
{
	struct trace_state_description dsk_sd;

	dsk_sd.type = EVENT_TYPE_STATE_DESCRIPTION;
	dsk_sd.state_id = state_id;
	dsk_sd.name_len = strlen(name);

	/* Convert state event to on-disk endianness and write to disk */
	if(write_struct_convert(fp, &dsk_sd, sizeof(dsk_sd),
				trace_state_description_conversion_table, 0))
	{
		return 1;
	}

	if(fwrite(name, strlen(name), 1, fp) != 1)
		return 1;

	return 0;
}

/* Write and read an empty file. */
UNIT_TEST(empty_file_test)
{
	struct multi_event_set mes;
	char tmpname[] = "test-tmp/traceXXXXXX";
	FILE* fp;
	off_t bytes_read;

	fp = tmpfile_template(tmpname, "wb+");
	ASSERT_NONNULL(fp);

	ASSERT_EQUALS(fflush(fp), 0);
	rewind(fp);

	multi_event_set_init(&mes);
	ASSERT_DIFFERENT(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);

	ASSERT_EQUALS(fclose(fp), 0);
	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

/* Write and read a file that only contains a header. */
UNIT_TEST(header_only_test)
{
	struct multi_event_set mes;
	char tmpname[] = "test-tmp/traceXXXXXX";
	FILE* fp;
	off_t bytes_read;

	fp = tmpfile_template(tmpname, "wb+");
	ASSERT_NONNULL(fp);

	ASSERT_EQUALS(write_default_header(fp), 0);

	ASSERT_EQUALS(fflush(fp), 0);
	rewind(fp);

	multi_event_set_init(&mes);
	ASSERT_EQUALS(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);
	ASSERT_EQUALS(mes.num_sets, 0);
	ASSERT_EQUALS(mes.num_tasks, 0);
	ASSERT_EQUALS(mes.num_frames, 0);
	ASSERT_EQUALS(mes.num_counters, 0);
	ASSERT_EQUALS(mes.num_states, OPENSTREAM_WORKER_STATE_MAX);
	ASSERT_EQUALS(mes.max_numa_node_id, 0);
	ASSERT_EQUALS(mes.max_write_size, 0);
	ASSERT_EQUALS(mes.max_read_size, 0);
	ASSERT_EQUALS(mes.min_cpu, -1);
	ASSERT_EQUALS(mes.max_cpu, -1);
	ASSERT_EQUALS(mes.num_global_single_events, 0);

	ASSERT_EQUALS(fclose(fp), 0);
	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

/* Write and read a file with a single state event without
 * description. */
UNIT_TEST(state_nodesc_test)
{
	struct multi_event_set mes;
	struct event_set* es;
	char tmpname[] = "test-tmp/traceXXXXXX";
	FILE* fp;
	off_t bytes_read;

	fp = tmpfile_template(tmpname, "wb+");
	ASSERT_NONNULL(fp);

	ASSERT_EQUALS(write_default_header(fp), 0);
	ASSERT_EQUALS(write_state_event(fp, 0, 0, 1000, 0), 0);

	ASSERT_EQUALS(fflush(fp), 0);
	rewind(fp);

	multi_event_set_init(&mes);
	ASSERT_EQUALS(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);
	ASSERT_EQUALS(mes.num_sets, 1);
	ASSERT_EQUALS(mes.num_tasks, 1);
	ASSERT_EQUALS(mes.num_frames, 1);
	ASSERT_EQUALS(mes.num_counters, 0);
	ASSERT_EQUALS(mes.num_states, OPENSTREAM_WORKER_STATE_MAX);
	ASSERT_EQUALS(mes.max_numa_node_id, 0);
	ASSERT_EQUALS(mes.max_write_size, 0);
	ASSERT_EQUALS(mes.max_read_size, 0);
	ASSERT_EQUALS(mes.min_cpu, 0);
	ASSERT_EQUALS(mes.max_cpu, 0);
	ASSERT_EQUALS(mes.num_global_single_events, 0);

	es = &mes.sets[0];
	ASSERT_EQUALS(es->num_state_events, 1);
	ASSERT_EQUALS(es->num_comm_events, 0);
	ASSERT_EQUALS(es->num_single_events, 0);
	ASSERT_EQUALS(es->num_counter_event_sets, 0);
	ASSERT_EQUALS(es->num_annotations, 0);
	ASSERT_EQUALS(es->cpu, 0);
	ASSERT_EQUALS(es->first_start, 0);
	ASSERT_EQUALS(es->last_end, 1000);

	ASSERT_EQUALS(fclose(fp), 0);
	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

/* Write and read a file with a single state event with a
 * description. */
UNIT_TEST(state_desc_test)
{
	struct multi_event_set mes;
	struct event_set* es;
	char tmpname[] = "test-tmp/traceXXXXXX";
	FILE* fp;
	off_t bytes_read;

	fp = tmpfile_template(tmpname, "wb+");
	ASSERT_NONNULL(fp);

	ASSERT_EQUALS(write_default_header(fp), 0);
	ASSERT_EQUALS(write_state_event(fp, 0, 0, 1000, 0), 0);
	ASSERT_EQUALS(write_state_description(fp, 0, "a name"), 0);

	ASSERT_EQUALS(fflush(fp), 0);
	rewind(fp);

	multi_event_set_init(&mes);
	ASSERT_EQUALS(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);
	ASSERT_EQUALS(mes.num_sets, 1);
	ASSERT_EQUALS(mes.num_tasks, 1);
	ASSERT_EQUALS(mes.num_frames, 1);
	ASSERT_EQUALS(mes.num_counters, 0);
	ASSERT_EQUALS(mes.num_states, 1);
	ASSERT_EQUALS(mes.max_numa_node_id, 0);
	ASSERT_EQUALS(mes.max_write_size, 0);
	ASSERT_EQUALS(mes.max_read_size, 0);
	ASSERT_EQUALS(mes.min_cpu, 0);
	ASSERT_EQUALS(mes.max_cpu, 0);
	ASSERT_EQUALS(mes.num_global_single_events, 0);

	ASSERT_EQUALS_STRING(mes.states[0].name, "a name");

	es = &mes.sets[0];
	ASSERT_EQUALS(es->num_state_events, 1);
	ASSERT_EQUALS(es->num_comm_events, 0);
	ASSERT_EQUALS(es->num_single_events, 0);
	ASSERT_EQUALS(es->num_counter_event_sets, 0);
	ASSERT_EQUALS(es->num_annotations, 0);
	ASSERT_EQUALS(es->cpu, 0);
	ASSERT_EQUALS(es->first_start, 0);
	ASSERT_EQUALS(es->last_end, 1000);

	ASSERT_EQUALS(fclose(fp), 0);
	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

/* Write and read a file with a single state event with a description
 * + an additional, unreferenced state description. */
UNIT_TEST(one_state_two_descs_test)
{
	struct multi_event_set mes;
	struct event_set* es;
	char tmpname[] = "test-tmp/traceXXXXXX";
	FILE* fp;
	off_t bytes_read;

	fp = tmpfile_template(tmpname, "wb+");
	ASSERT_NONNULL(fp);

	ASSERT_EQUALS(write_default_header(fp), 0);
	ASSERT_EQUALS(write_state_event(fp, 0, 0, 1000, 0), 0);
	ASSERT_EQUALS(write_state_description(fp, 0, "a name"), 0);
	ASSERT_EQUALS(write_state_description(fp, 1, "another description"), 0);

	ASSERT_EQUALS(fflush(fp), 0);
	rewind(fp);

	multi_event_set_init(&mes);
	ASSERT_EQUALS(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);
	ASSERT_EQUALS(mes.num_sets, 1);
	ASSERT_EQUALS(mes.num_tasks, 1);
	ASSERT_EQUALS(mes.num_frames, 1);
	ASSERT_EQUALS(mes.num_counters, 0);
	ASSERT_EQUALS(mes.num_states, 2);
	ASSERT_EQUALS(mes.max_numa_node_id, 0);
	ASSERT_EQUALS(mes.max_write_size, 0);
	ASSERT_EQUALS(mes.max_read_size, 0);
	ASSERT_EQUALS(mes.min_cpu, 0);
	ASSERT_EQUALS(mes.max_cpu, 0);
	ASSERT_EQUALS(mes.num_global_single_events, 0);

	ASSERT_EQUALS_STRING(mes.states[0].name, "a name");
	ASSERT_EQUALS_STRING(mes.states[1].name, "another description");

	es = &mes.sets[0];
	ASSERT_EQUALS(es->num_state_events, 1);
	ASSERT_EQUALS(es->num_comm_events, 0);
	ASSERT_EQUALS(es->num_single_events, 0);
	ASSERT_EQUALS(es->num_counter_event_sets, 0);
	ASSERT_EQUALS(es->num_annotations, 0);
	ASSERT_EQUALS(es->cpu, 0);
	ASSERT_EQUALS(es->first_start, 0);
	ASSERT_EQUALS(es->last_end, 1000);

	ASSERT_EQUALS(fclose(fp), 0);
	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

/* Write and read a file with n state events and n state
 * descriptions. */
UNIT_TEST(nstates_ndescs_test)
{
	struct multi_event_set mes;
	struct event_set* es;
	char tmpname[] = "test-tmp/traceXXXXXX";
	FILE* fp;
	off_t bytes_read;
	int num_states = 100;
	char buf[20];

	fp = tmpfile_template(tmpname, "wb+");
	ASSERT_NONNULL(fp);

	ASSERT_EQUALS(write_default_header(fp), 0);

	for(int i = 0; i < num_states; i++) {
		snprintf(buf, sizeof(buf), "state %d", i);
		ASSERT_EQUALS(write_state_description(fp, i, buf), 0);
	}

	for(int i = 0; i < num_states; i++)
		ASSERT_EQUALS(write_state_event(fp, 0, i*1000, (i+1)*1000-1, i), 0);

	ASSERT_EQUALS(fflush(fp), 0);
	rewind(fp);

	multi_event_set_init(&mes);
	ASSERT_EQUALS(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);
	ASSERT_EQUALS(mes.num_sets, 1);
	ASSERT_EQUALS(mes.num_tasks, 1);
	ASSERT_EQUALS(mes.num_frames, 1);
	ASSERT_EQUALS(mes.num_counters, 0);
	ASSERT_EQUALS(mes.num_states, num_states);
	ASSERT_EQUALS(mes.max_numa_node_id, 0);
	ASSERT_EQUALS(mes.max_write_size, 0);
	ASSERT_EQUALS(mes.max_read_size, 0);
	ASSERT_EQUALS(mes.min_cpu, 0);
	ASSERT_EQUALS(mes.max_cpu, 0);
	ASSERT_EQUALS(mes.num_global_single_events, 0);

	for(int i = 0; i < num_states; i++) {
		snprintf(buf, sizeof(buf), "state %d", i);
		ASSERT_EQUALS_STRING(mes.states[i].name, buf);
	}

	es = &mes.sets[0];
	ASSERT_EQUALS(es->num_state_events, num_states);
	ASSERT_EQUALS(es->num_comm_events, 0);
	ASSERT_EQUALS(es->num_single_events, 0);
	ASSERT_EQUALS(es->num_counter_event_sets, 0);
	ASSERT_EQUALS(es->num_annotations, 0);
	ASSERT_EQUALS(es->cpu, 0);
	ASSERT_EQUALS(es->first_start, 0);
	ASSERT_EQUALS(es->last_end, num_states*1000-1);

	ASSERT_EQUALS(fclose(fp), 0);
	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

/* Write and read a file with two states with non-sequential IDs */
UNIT_TEST(noncontiguous_state_ids)
{
	struct multi_event_set mes;
	struct event_set* es;
	char tmpname[] = "test-tmp/traceXXXXXX";
	FILE* fp;
	off_t bytes_read;

	fp = tmpfile_template(tmpname, "wb+");
	ASSERT_NONNULL(fp);

	ASSERT_EQUALS(write_default_header(fp), 0);
	ASSERT_EQUALS(write_state_event(fp, 0, 0, 1000, 0), 0);
	ASSERT_EQUALS(write_state_event(fp, 0, 2000, 3000, UINT32_MAX), 0);

	ASSERT_EQUALS(fflush(fp), 0);
	rewind(fp);

	multi_event_set_init(&mes);
	ASSERT_EQUALS(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);
	ASSERT_EQUALS(mes.num_sets, 1);
	ASSERT_EQUALS(mes.num_tasks, 1);
	ASSERT_EQUALS(mes.num_frames, 1);
	ASSERT_EQUALS(mes.num_counters, 0);
	ASSERT_EQUALS(mes.num_states, 2);
	ASSERT_EQUALS(mes.max_numa_node_id, 0);
	ASSERT_EQUALS(mes.max_write_size, 0);
	ASSERT_EQUALS(mes.max_read_size, 0);
	ASSERT_EQUALS(mes.min_cpu, 0);
	ASSERT_EQUALS(mes.max_cpu, 0);
	ASSERT_EQUALS(mes.num_global_single_events, 0);

	es = &mes.sets[0];
	ASSERT_EQUALS(es->num_state_events, 2);
	ASSERT_EQUALS(es->num_comm_events, 0);
	ASSERT_EQUALS(es->num_single_events, 0);
	ASSERT_EQUALS(es->num_counter_event_sets, 0);
	ASSERT_EQUALS(es->num_annotations, 0);
	ASSERT_EQUALS(es->cpu, 0);
	ASSERT_EQUALS(es->first_start, 0);
	ASSERT_EQUALS(es->last_end, 3000);

	ASSERT_EQUALS(fclose(fp), 0);
	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

/* Write and read a file with n state events on m CPUs and n state
 * descriptions. */
UNIT_TEST(nstates_ndescs_mcpus_test)
{
	struct multi_event_set mes;
	struct event_set* es;
	char tmpname[] = "test-tmp/traceXXXXXX";
	FILE* fp;
	off_t bytes_read;
	int num_states = 100;
	int num_cpus = 32;
	char buf[20];

	fp = tmpfile_template(tmpname, "wb+");
	ASSERT_NONNULL(fp);

	ASSERT_EQUALS(write_default_header(fp), 0);

	for(int i = 0; i < num_states; i++) {
		snprintf(buf, sizeof(buf), "state %d", i);
		ASSERT_EQUALS(write_state_description(fp, i, buf), 0);
	}

	for(int i = 0; i < num_states; i++) {
		for(int cpu = 0; cpu < num_cpus; cpu++)
			ASSERT_EQUALS(write_state_event(fp, cpu, cpu+i*1000, cpu+(i+1)*1000-1, i), 0);
	}

	ASSERT_EQUALS(fflush(fp), 0);
	rewind(fp);

	multi_event_set_init(&mes);
	ASSERT_EQUALS(read_trace_sample_file(&mes, tmpname, &bytes_read), 0);
	ASSERT_EQUALS(mes.num_sets, num_cpus);
	ASSERT_EQUALS(mes.num_tasks, 1);
	ASSERT_EQUALS(mes.num_frames, 1);
	ASSERT_EQUALS(mes.num_counters, 0);
	ASSERT_EQUALS(mes.num_states, num_states);
	ASSERT_EQUALS(mes.max_numa_node_id, 0);
	ASSERT_EQUALS(mes.max_write_size, 0);
	ASSERT_EQUALS(mes.max_read_size, 0);
	ASSERT_EQUALS(mes.min_cpu, 0);
	ASSERT_EQUALS(mes.max_cpu, num_cpus-1);
	ASSERT_EQUALS(mes.num_global_single_events, 0);

	for(int i = 0; i < num_states; i++) {
		snprintf(buf, sizeof(buf), "state %d", i);
		ASSERT_EQUALS_STRING(mes.states[i].name, buf);
	}

	for_each_event_set(&mes, es) {
		ASSERT_EQUALS(es->num_state_events, num_states);
		ASSERT_EQUALS(es->num_comm_events, 0);
		ASSERT_EQUALS(es->num_single_events, 0);
		ASSERT_EQUALS(es->num_counter_event_sets, 0);
		ASSERT_EQUALS(es->num_annotations, 0);
		ASSERT_EQUALS(es->first_start, es->cpu);
		ASSERT_EQUALS(es->last_end, es->cpu+num_states*1000-1);
	}

	ASSERT_EQUALS(fclose(fp), 0);
	multi_event_set_destroy(&mes);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

UNIT_TEST_SUITE(wr_trace_states_test)
{
	ADD_TEST(empty_file_test);
	ADD_TEST(header_only_test);
	ADD_TEST(state_nodesc_test);
	ADD_TEST(state_desc_test);
	ADD_TEST(one_state_two_descs_test);
	ADD_TEST(nstates_ndescs_test);
	ADD_TEST(noncontiguous_state_ids);
	ADD_TEST(nstates_ndescs_mcpus_test);
}
END_TEST_SUITE()
