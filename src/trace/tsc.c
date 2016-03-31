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

#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include "arch.h"
#include "tsc.h"

/**
 * Get the epoch in nanoseconds
 */
am_nanoseconds_t am_current_nanoseconds(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	return 1000000000 * (am_nanoseconds_t)ts.tv_sec + ts.tv_nsec;
}

/**
 * Calculate the number of ticks per second.
 */
am_timestamp_t am_timestamps_per_second(void)
{
	am_nanoseconds_t ns_start;
	am_nanoseconds_t ns_end;
	am_nanoseconds_t ns;

	am_timestamp_t ts_start;
	am_timestamp_t ts_end;

	ns_start = am_current_nanoseconds();
	ts_start = am_tsc();

	/* Idle for at least 10ms */
	do {
		ns = am_current_nanoseconds();
	} while(ns - ns_start < 10000000);

	ns_end = am_current_nanoseconds();
	ts_end = am_tsc();

	return (ts_end - ts_start)*1000000000 / (ns_end - ns_start);
}

/**
 * Initialize a timestamp reference
 */
void am_timestamp_reference_init(struct am_timestamp_reference* tsr)
{
	tsr->set = 0;
}

/**
 * Set a timestamp reference
 *
 * @param this_cpu CPU identifier of the calling CPU
 * @return 0 on success, 1 otherwise (e.g., if the reference has
 * already been initialized).
 */
int am_timestamp_reference_set(struct am_timestamp_reference* tsr,
			       am_cpu_t this_cpu)
{
	am_nanoseconds_t ref_ns = am_current_nanoseconds();
	am_timestamp_t ref_ts = am_tsc();
	am_timestamp_t tsps = am_timestamps_per_second();

	if(tsr->set)
		return 1;

	tsr->ref_cpu = this_cpu;
	tsr->ref_ns = ref_ns;
	tsr->ref_ts = ref_ts;
	tsr->tsps = tsps;
	tsr->set = 1;

	return 0;
}
