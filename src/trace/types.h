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

typedef uint32_t am_cpu_t;
typedef uint32_t am_state_t;
typedef uint32_t am_counter_t;
typedef uint64_t am_counter_value_t;
typedef uint64_t am_timestamp_t;
typedef uint64_t am_nanoseconds_t;
typedef int64_t am_timestamp_diff_t;
typedef int64_t am_nanoseconds_diff_t;

typedef uint32_t am_omp_worker_t;
typedef uint64_t am_omp_for_instance_id_t;
typedef uint64_t am_omp_for_address_t;
typedef uint32_t am_omp_for_flags_t;
typedef uint64_t am_omp_for_increment_t;
typedef uint64_t am_omp_for_iterator_t;
typedef uint64_t am_omp_for_chunk_set_id_t;
typedef uint64_t am_omp_task_instance_id_t;
typedef uint64_t am_omp_task_address_t;

enum am_omp_for_flag {
	OMP_FOR_SCHEDULE_STATIC = (1 << 0),
	OMP_FOR_SCHEDULE_DYNAMIC = (1 << 1),
	OMP_FOR_SCHEDULE_GUIDED = (1 << 2),
	OMP_FOR_SCHEDULE_AUTO = (1 << 3),
	OMP_FOR_SCHEDULE_RUNTIME = (1 << 4),
	OMP_FOR_SIGNED_INCREMENT = (1 << 5),
	OMP_FOR_SIGNED_ITERATION_SPACE = (1 << 6),
	OMP_FOR_MULTI_CHUNK_SETS = (1 << 7)
};

#endif
