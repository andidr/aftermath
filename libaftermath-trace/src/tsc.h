/**
 * Author: Andi Drebes <andi@drebesium.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_TRACE_TSC_H
#define AM_TRACE_TSC_H

#include <aftermath/trace/base_types.h>
#include <aftermath/trace/arch.h>

/* Indicates by how many timestamps the current timestamp must be decreased in
 * order to obtain a uniform timestamp wrt to timestamp 0 for an event
 * source. */
struct am_timestamp_reference {
	am_timestamp_t ref;
};

/* Initialize the timestamp reference tsr with ref. All timestamps obtained
 * through am_timestamp_reference_now() will be normalized wrt. ref. */
static inline void
am_timestamp_reference_init(struct am_timestamp_reference* tsr,
			    am_timestamp_t ref)
{
	tsr->ref = ref;
}

/* Returns the current timestamp corrected wrt. the timestamp reference tsr.
 *
 * Returns 0 on success, otherwise 1 (i.e., if the corrected timestamp would be
 * negative)
 */
static inline int
am_timestamp_reference_now(const struct am_timestamp_reference* tsr,
			   am_timestamp_t* t)
{
	am_timestamp_t local_now = am_tsc();

	if(tsr->ref > local_now)
		return 1;

	*t = local_now - tsr->ref;

	return 0;
}

#endif
