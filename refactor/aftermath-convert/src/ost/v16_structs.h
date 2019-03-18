/**
 * Author: Andi Drebes <andi@drebesium.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include <stdint.h>

enum v16_event_type {
	V16_EVENT_TYPE_STATE = 0,
	V16_EVENT_TYPE_COMM = 1,
	V16_EVENT_TYPE_SINGLE = 2,
	V16_EVENT_TYPE_COUNTER = 3,
	V16_EVENT_TYPE_COUNTER_DESCRIPTION = 4,
	V16_EVENT_TYPE_FRAME_INFO = 5,
	V16_EVENT_TYPE_CPU_INFO = 6,
	V16_EVENT_TYPE_GLOBAL_SINGLE_EVENT = 7,
	V16_EVENT_TYPE_STATE_DESCRIPTION = 8,
	V16_EVENT_TYPE_OMP_FOR = 9,
	V16_EVENT_TYPE_OMP_FOR_CHUNK_SET = 10,
	V16_EVENT_TYPE_OMP_FOR_CHUNK_SET_PART = 11,
	V16_EVENT_TYPE_OMP_TASK_INSTANCE = 12,
	V16_EVENT_TYPE_OMP_TASK_INSTANCE_PART = 13
};

enum v16_comm_event_type {
	V16_COMM_TYPE_UNKNOWN = 0,
	V16_COMM_TYPE_STEAL = 1,
	V16_COMM_TYPE_PUSH = 2,
	V16_COMM_TYPE_DATA_READ = 3,
	V16_COMM_TYPE_DATA_WRITE = 4
};

enum v16_single_event_type {
	V16_SINGLE_TYPE_TCREATE = 0,
	V16_SINGLE_TYPE_TEXEC_START = 1,
	V16_SINGLE_TYPE_TEXEC_END = 2,
	V16_SINGLE_TYPE_TDESTROY = 3
};

enum v16_global_single_event_type {
	V16_GLOBAL_SINGLE_TYPE_MEASURE_START = 0,
	V16_GLOBAL_SINGLE_TYPE_MEASURE_END = 1
};

/* File header */
struct v16_trace_header {
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

/* A trace event header is included in every data structure
 * used in the trace file in order to identify the type of
 * the sample.
 */
struct v16_trace_event_header {
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
} __attribute__((packed));

struct v16_trace_state_event {
	/* Start time of the state is stored in header.time */
	struct v16_trace_event_header header;

	/* Indicates when the worker left the state */
	uint64_t end_time;

	/* State identifier */
	uint32_t state;
} __attribute__((packed));

/* Communication events can be any events that involves point-to-point
 * communication between workers.
 */
struct v16_trace_comm_event {
	/* header.cpu / header.worker represents the source
	 * of the communication */
	struct v16_trace_event_header header;

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

/* Describes a counter, e.g. a hardware performance counter */
struct v16_trace_counter_description {
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

/* Counter events are events are dumps of performance
 * counters, e.g. number of cache misses over time
 */
struct v16_trace_counter_event {
	struct v16_trace_event_header header;

	/* The unique identifier of the counter whose
	 * value is dumped */
	uint64_t counter_id;

	/* The counter value */
	int64_t value;
} __attribute__((packed));

/* Single events are events that only involve one worker
 * and whose duration is not important, e.g. task creation.
 */
struct v16_trace_single_event {
	struct v16_trace_event_header header;

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

struct v16_trace_frame_info {
	struct v16_trace_event_header header;

	/* Frame's address */
	uint64_t addr;

	/* Owning NUMA node */
	int32_t numa_node;

	/* Size of the frame */
	uint32_t size;
} __attribute__((packed));

struct v16_trace_cpu_info {
	struct v16_trace_event_header header;

	/* NUMA node of the CPU's local allocations */
	int32_t numa_node;
} __attribute__((packed));

struct v16_trace_global_single_event {
	/* Short header field */
	uint32_t type;

	/* Timestamp of the event */
	uint64_t time;

	/* Sub-type */
	int32_t single_type;
} __attribute__((packed));

/* Struct describing a worker's state present in the trace. */
struct v16_trace_state_description {
	/* Short header field */
	uint32_t type;

	/* State id */
	uint32_t state_id;

	/* Length of the state name not including the terminating zero byte */
	uint32_t name_len;
} __attribute__((packed));

/* Struct describing an OpenMP parallel for instance */
struct v16_trace_omp_for_instance {
	/* Short header field */
	uint32_t type;

	/* Set of values from enum omp_for_flag */
	uint32_t flags;

	/* Unique address of the loop (e.g., instruction that calls
	 * the runtime function determining the bounds for each
	 * chunk_set) */
	uint64_t addr;

	/* Unique id */
	uint64_t id;

	/* Loop increment; raw 64-bit representation, including signed
	 * extension for signed increments */
	uint64_t increment;

	/* Lower bound of the iteration space; raw 64-bit
	 * representation, including signed extension for signed
	 * loop bounds */
	uint64_t lower_bound;

	/* Upper bound of the iteration space; raw 64-bit
	 * representation, including signed extension for signed loop
	 * bounds */
	uint64_t upper_bound;

	/* Number of workers executing the loop */
	uint32_t num_workers;
} __attribute__((packed));

/* Struct describing an OpenMP parallel for chunk_set */
struct v16_trace_omp_for_chunk_set {
	/* Short header field */
	uint32_t type;

	/* Identifier of the associated for loop instance */
	uint64_t for_id;

	/* Unique chunk_set id */
	uint64_t id;

	/* Lower bound of the chunk_set's iteration space; raw 64-bit
	 * representation, including signed extension for signed loop
	 * bounds */
	uint64_t first_lower;

	/* Upper bound of the chunk_set's iteration space; raw 64-bit
	 * representation, including signed extension for signed loop
	 * bounds */
	uint64_t last_upper;
} __attribute__((packed));

/* Struct describing a part of an OpenMP parallel for chunk_set */
struct v16_trace_omp_for_chunk_set_part {
	/* Short header field */
	uint32_t type;

	/* Cpu */
	uint32_t cpu;

	/* Identifier of the associated chunk_set */
	uint64_t chunk_set_id;

	/* Start timestamp */
	uint64_t start;

	/* End timestamp */
	uint64_t end;
} __attribute__((packed));

/* Struct describing an OpenMP parallel for instance */
struct v16_trace_omp_task_instance {
	/* Short header field */
	uint32_t type;

	/* Unique address of the task (e.g., address of its outlined
	 * work function) */
	uint64_t addr;

	/* Unique id */
	uint64_t id;
} __attribute__((packed));

/* Struct describing a part of an OpenMP parallel for chunk_set */
struct v16_trace_omp_task_instance_part {
	/* Short header field */
	uint32_t type;

	/* CPU identifier */
	uint32_t cpu;

	/* Identifier of the associated task instance */
	uint64_t task_instance_id;

	/* Start timestamp */
	uint64_t start;

	/* End timestamp */
	uint64_t end;
} __attribute__((packed));
