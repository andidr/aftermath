/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
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

#ifndef TRACE_FILE_H
#define TRACE_FILE_H

#include <stdio.h>
#include <stdint.h>

/**
 * OpenStream trace format:
 *
 * +----------------------------+
 * | trace_header               |
 * +----------------------------+
 * | Arbitrary number of:       |
 * | - trace_state_description  |
 * | - trace_state_events       |
 * | - trace_comm_events        |
 * | - trace_single_events      |
 * | - trace_counter_description|
 * | - trace_counter_event      |
 * +----------------------------+
 *
 * Each of these structs contains a header that specifies its type and
 * that contains common fields used in every trace sample. An exception to this
 * rule are counter descriptions, which only contain a field for the type in the
 * header.
 *
 * The on-disk byte order for every integer field is little endian
 */

/* OSTV in ASCII */
#define TRACE_MAGIC 0x5654534f
#define TRACE_VERSION 16

static inline int trace_version_compatible(int version)
{
	return (version == 13 || version == 14 || version == 15 || version == 16);
}

enum event_type {
	EVENT_TYPE_STATE = 0,
	EVENT_TYPE_COMM = 1,
	EVENT_TYPE_SINGLE = 2,
	EVENT_TYPE_COUNTER = 3,
	EVENT_TYPE_COUNTER_DESCRIPTION = 4,
	EVENT_TYPE_FRAME_INFO = 5,
	EVENT_TYPE_CPU_INFO = 6,
	EVENT_TYPE_GLOBAL_SINGLE_EVENT = 7,
	EVENT_TYPE_STATE_DESCRIPTION = 8
};

enum comm_event_type {
	COMM_TYPE_UNKNOWN = 0,
	COMM_TYPE_STEAL = 1,
	COMM_TYPE_PUSH = 2,
	COMM_TYPE_DATA_READ = 3,
	COMM_TYPE_DATA_WRITE = 4
};

enum single_event_type {
	SINGLE_TYPE_TCREATE = 0,
	SINGLE_TYPE_TEXEC_START = 1,
	SINGLE_TYPE_TEXEC_END = 2,
	SINGLE_TYPE_TDESTROY = 3
};

enum global_single_event_type {
	GLOBAL_SINGLE_TYPE_MEASURE_START = 0,
	GLOBAL_SINGLE_TYPE_MEASURE_END = 1
};

/* File header */
struct trace_header {
	/* Magic number */
	uint32_t magic;

	/* Version of the file format used on disk */
	uint32_t version;

	/* Day of month in [1;31] */
	uint8_t day;

	/* Month in [1;12] */
	uint8_t month;

	/* Year as plain integer e.g. 2013 */
	uint16_t year;

	/* Hour in [0;23] */
	uint8_t hour;

	/* Minute in [0;59] */
	uint8_t minute;
} __attribute__((packed));

extern int trace_header_conversion_table[];

/* A trace event header is included in every data structure
 * used in the trace file in order to identify the type of
 * the sample.
 */
struct trace_event_header {
	/* Struct type */
	uint32_t type;

	/* Timestamp of the event in [0;UINT64_MAX] */
	uint64_t time;

	/* CPU in [0;UINT32_MAX] */
	uint32_t cpu;

	/* Worker in [0;UINT32_MAX] */
	uint32_t worker;

	/* Task that was being executed when the event occured
	 * (address of the work function) */
	uint64_t active_task;

	/* Task frame of the active task */
	uint64_t active_frame;
} __attribute__((packed));;

extern int trace_event_header_conversion_table[];

/* Describes a state in which the worker was during a period of time.
 * Per-worker states might not overlap.
 */
struct trace_state_event {
	/* Start time of the state is stored in header.time */
	struct trace_event_header header;

	/* Indicates when the worker left the state */
	uint64_t end_time;

	/* State identifier */
	uint32_t state;
} __attribute__((packed));

extern int trace_state_event_conversion_table[];

/* Communication events can be any events that involves point-to-point
 * communication between workers.
 */
struct trace_comm_event {
	/* header.cpu / header.worker represents the source
	 * of the communication */
	struct trace_event_header header;

	/* Communication type, e.g. steal or push */
	uint32_t type;

	/* Source or destination CPU depending on the communication
	 * type. For data reads and steals this is the source CPU,
	 * for pushes this is the destination CPU. For data writes
	 * the value is ignored.
	 */
	uint32_t src_or_dst_cpu;

	/* Indicates how much data in bytes was transferred */
	uint64_t size;

	/* When was the data of the communication produced? */
	uint64_t prod_ts;

	/* What was transferred, e.g. value of the pointer to the data */
	uint64_t what;
} __attribute__((packed));

extern int trace_comm_event_conversion_table[];

/* Describes a counter, e.g. a hardware performance counter */
struct trace_counter_description {
	/* Short header field */
	uint32_t type;

	/* Id used in trace_counter_events */
	uint64_t counter_id;

	/* Length of the counter name not including the terminating zero byte */
	uint32_t name_len;

	/* The last field is followed by the name as a
	 * zero-terminated ASCII string
	 */
} __attribute__((packed));

extern int trace_counter_description_conversion_table[];

/* Counter events are events are dumps of performance
 * counters, e.g. number of cache misses over time
 */
struct trace_counter_event {
	struct trace_event_header header;

	/* The unique identifier of the counter whose
	 * value is dumped */
	uint64_t counter_id;

	/* The counter value */
	int64_t value;
} __attribute__((packed));


extern int trace_counter_event_conversion_table[];

/* Single events are events that only involve one worker
 * and whose duration is not important, e.g. task creation.
 */
struct trace_single_event {
	struct trace_event_header header;

	/* Single event type */
	uint32_t type;

	/* Depending on the event the what field might be
	 * used for different purposes:
	 * - tcreate: the frame pointer of the newly created task
	 * - texec start: the frame pointer of the task that starts execution
	 * - texec finish: the frame pointer of the task that finishes execution
	*/
	uint64_t what;
} __attribute__((packed));

extern int trace_single_event_conversion_table[];

struct trace_frame_info {
	struct trace_event_header header;

	/* Frame's address */
	uint64_t addr;

	/* Owning NUMA node */
	int32_t numa_node;

	/* Size of the frame */
	uint32_t size;
} __attribute__((packed));

extern int trace_frame_info_conversion_table[];

struct trace_cpu_info {
	struct trace_event_header header;

	/* NUMA node of the CPU's local allocations */
	int32_t numa_node;
} __attribute__((packed));

extern int trace_cpu_info_conversion_table[];

struct trace_global_single_event {
	/* Short header field */
	uint32_t type;

	/* Timestamp of the event */
	uint64_t time;

	/* Sub-type */
	int32_t single_type;
} __attribute__((packed));

extern int trace_global_single_event_conversion_table[];

/* Struct describing a worker's state present in the trace. */
struct trace_state_description {
        /* Short header field */
        uint32_t type;

        /* State id */
        uint32_t state_id;

	/* Length of the state name not including the terminating zero byte */
        uint32_t name_len;
} __attribute__((packed));

extern int trace_state_description_conversion_table[];

/* Performs an integrity check on a header in host format */
int trace_verify_header(struct trace_header* header);

#endif
