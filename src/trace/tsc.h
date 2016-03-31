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

#ifndef AFTERMATH_TSC_H
#define AFTERMATH_TSC_H

#include "types.h"

am_nanoseconds_t am_current_nanoseconds(void);
am_timestamp_t am_timestamps_per_second(void);

/* Data structure used to calculate the difference between timestamp
 * counters with a constant offset. */
struct am_timestamp_reference {
	/* Reference CPU */
	am_cpu_t ref_cpu;

	/* Reference nanoseconds since epoch */
	am_nanoseconds_t ref_ns;

	/* Reference timestamp */
	am_timestamp_t ref_ts;

	/* Timestamps per second */
	am_timestamp_t tsps;

	/* Indicates whether the reference has actually been set */
	int set;
};

void am_timestamp_reference_init(struct am_timestamp_reference* tsr);
int am_timestamp_reference_set(struct am_timestamp_reference* tsr,
			       am_cpu_t this_cpu);

#endif
