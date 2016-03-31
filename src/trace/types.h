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

#endif
