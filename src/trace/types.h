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

#ifndef AFTERMATH_TYPES_H
#define AFTERMATH_TYPES_H

#include <stdint.h>
#include <inttypes.h>

typedef uint32_t am_cpu_t;
#define AM_CPU_T_FMT PRIu32

typedef uint32_t am_state_t;
#define AM_STATE_T_FMT PRIu32

typedef uint32_t am_counter_t;
#define AM_COUNTER_T_FMT PRIu32

typedef uint64_t am_counter_value_t;
#define AM_COUNTER_VALUE_T_FMT PRIu64

typedef uint64_t am_timestamp_t;
#define AM_TIMESTAMP_T_FMT PRIu64

typedef uint64_t am_nanoseconds_t;
#define AM_NANOSECONDS_T_FMT PRIu64

typedef int64_t am_timestamp_diff_t;
#define AM_TIMESTAMP_DIFF_T_FMT PRId64

typedef int64_t am_nanoseconds_diff_t;
#define AM_NANOSECONDS_DIFF_T_FMT PRId64

typedef uint32_t am_hierarchy_id_t;
#define AM_HIERARCHY_ID_T_FMT PRIu32

typedef uint32_t am_hierarchy_node_id_t;
#define AM_HIERARCHY_NODE_ID_T_FMT PRIu32

typedef uint32_t am_event_collection_id_t;
#define AM_EVENT_COLLECTION_ID_T_FMT PRIu32

struct am_interval {
	am_timestamp_t start;
	am_timestamp_t end;
};

typedef uint32_t am_omp_worker_t;
#define AM_OMP_WORKER_T_FMT PRIu32

typedef uint64_t am_omp_for_instance_id_t;
#define AM_OMP_FOR_INSTANCE_ID_T_FMT PRIu64

typedef uint64_t am_omp_for_address_t;
#define AM_OMP_FOR_ADDRESS_T_FMT PRIu64

typedef uint32_t am_omp_for_flags_t;
#define AM_OMP_FOR_FLAGS_T_FMT PRIu32

typedef uint64_t am_omp_for_increment_t;
#define AM_OMP_FOR_INCREMENT_T_FMT PRIu64

typedef uint64_t am_omp_for_iterator_t;
#define AM_OMP_FOR_ITERATOR_T_FMT PRIu64

typedef uint64_t am_omp_for_chunk_set_id_t;
#define AM_OMP_FOR_CHUNK_SET_ID_T_FMT PRIu64

typedef uint64_t am_omp_task_instance_id_t;
#define AM_OMP_TASK_INSTANCE_ID_T_FMT PRIu64

typedef uint64_t am_omp_task_address_t;
#define AM_OMP_TASK_ADDRESS_T_FMT PRIu64

enum am_omp_for_flag {
	OMP_FOR_SCHEDULE_STATIC = (1 << 0),
	OMP_FOR_SCHEDULE_DYNAMIC = (1 << 1),
	OMP_FOR_SCHEDULE_GUIDED = (1 << 2),
	OMP_FOR_SCHEDULE_AUTO = (1 << 3),
	OMP_FOR_SCHEDULE_RUNTIME = (1 << 4),
	OMP_FOR_SIGNED_INCREMENT = (1 << 5),
	OMP_FOR_SIGNED_ITERATION_SPACE = (1 << 6),
	OMP_FOR_CHUNKED = (1 << 7)
};

#endif
