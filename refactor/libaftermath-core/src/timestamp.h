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

#ifndef AM_TIMESTAMP_H
#define AM_TIMESTAMP_H

#include "base_types.h"
#include "arithmetic.h"

/* Saturated computation of *d = (*d) * mul / div for a timestamp and two other
 * timestamps. The return value indicates if the result is exact or
 * saturated. */
static inline enum am_arithmetic_status
am_timestamp_muldiv_sat(am_timestamp_t* t,
			am_timestamp_t mul,
			am_timestamp_t div)
{
	return am_muldiv_sat_u64(*t, mul, div, t);
}

/* Saturated addition of a timestamp *t and a timestamp o. */
static inline enum am_arithmetic_status
am_timestamp_add_sat(am_timestamp_t* t, am_timestamp_t o)
{
	return am_add_sat_u64(*t, o, t);
}

/* Saturated addition of a timestamp *t and a timestamp offset o. */
static inline enum am_arithmetic_status
am_timestamp_add_sat_offset(am_timestamp_t* t, const struct am_time_offset* o)
{
	if(o->sign)
		return am_sub_sat_u64(*t, o->abs, t);
	else
		return am_add_sat_u64(*t, o->abs, t);
}

/* Saturated subtraction of a timestamp *t and a timestamp o. */
static inline enum am_arithmetic_status
am_timestamp_sub_sat(am_timestamp_t* t, am_timestamp_t o)
{
	return am_sub_sat_u64(*t, o, t);
}

/* Saturated subtraction of a timestamp *t and a timestamp offset o. */
static inline enum am_arithmetic_status
am_timestamp_sub_sat_offset(am_timestamp_t* t, const struct am_time_offset* o)
{
	if(o->sign)
		return am_add_sat_u64(*t, o->abs, t);
	else
		return am_sub_sat_u64(*t, o->abs, t);
}

/* Saturated multiplication of a timestamp *t with another timestamp o. */
static inline enum am_arithmetic_status
am_timestamp_mul_sat(am_timestamp_t* t, am_timestamp_t o)
{
	return am_mul_sat_u64(*t, o, t);
}

#endif
