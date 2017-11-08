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

/* Saturated computation of *d = (*d) * mul / div for a timestamp difference and
 * two signed 64-bit integers. The return value indicates if the result is exact
 * or saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_muldiv_sat_i64(am_timestamp_diff_t* d,
				 int64_t mul,
				 int64_t div)
{
	return am_muldiv_sat_i64(*d, mul, div, d);
}

/* Saturated computation of *d = (*d) * mul / div for a timestamp and two
 * timestamp differences. The return value indicates if the result is exact or
 * saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_muldiv_sat_diff(am_timestamp_diff_t* d,
				  am_timestamp_diff_t mul,
				  am_timestamp_diff_t div)
{
	return am_muldiv_sat_i64(*d, mul, div, d);
}

/* Saturated computation of *d = (*d) * mul / div for a timestamp and two 64-bit
 * unsigned integers. The return value indicates if the result is exact or
 * saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_muldiv_sat_u64(am_timestamp_diff_t* d,
				 uint64_t mul,
				 uint64_t div)
{
	return am_muldiv_sat_iuu64(*d, mul, div, d);
}

/* Saturated multiplication of a timestamp difference with a signed 64-bit
 * integer. The return value indicates if the result is exact or saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_mul_sat_i64(am_timestamp_diff_t* d, int64_t mul)
{
	return am_mul_sat_i64(*d, mul, d);
}

/* Saturated multiplication of a timestamp difference with an unsigned 64-bit
 * integer. The return value indicates if the result is exact or saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_mul_sat_u64(am_timestamp_diff_t* d, uint64_t mul)
{
	return am_mul_sat_iu64(*d, mul, d);
}

/* Saturated multiplication of a timestamp difference with a timestamp
 * difference. The return value indicates if the result is exact or
 * saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_mul_sat_diff(am_timestamp_diff_t* d, am_timestamp_diff_t mul)
{
	return am_mul_sat_i64(*d, mul, d);
}

/* Saturated addition of a timestamp difference and a signed 64-bit
 * integer. The return value indicates if the result is exact or saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_add_sat_i64(am_timestamp_diff_t* d, int64_t add)
{
	return am_add_sat_i64(*d, add, d);
}

/* Saturated addition of a timestamp difference and an unsigned 64-bit
 * integer. The return value indicates if the result is exact or saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_add_sat_u64(am_timestamp_diff_t* d, uint64_t add)
{
	return am_add_sat_iu64(*d, add, d);
}

/* Saturated addition of a timestamp difference and a timestamp difference. The
 * return value indicates if the result is exact or saturated. */
static inline enum am_arithmetic_status
am_timestamp_diff_add_sat_diff(am_timestamp_diff_t* d, am_timestamp_diff_t add)
{
	return am_add_sat_i64(*d, add, d);
}

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

/* Saturated computation of *d = (*d) * mul / div for a timestamp and two
 * timestamp differences. The return value indicates if the result is exact or
 * saturated. */
static inline enum am_arithmetic_status
am_timestamp_muldiv_sat_diff(am_timestamp_t* t,
			     am_timestamp_diff_t mul,
			     am_timestamp_diff_t div)
{
	return am_muldiv_sat_uii64(*t, mul, div, t);
}

/* Saturated addition of a timestamp *t and a timestamp o. */
static inline enum am_arithmetic_status
am_timestamp_add_sat(am_timestamp_t* t, am_timestamp_t o)
{
	return am_add_sat_u64(*t, o, t);
}

/* Saturated subtraction of a timestamp *t and a timestamp o. */
static inline enum am_arithmetic_status
am_timestamp_sub_sat(am_timestamp_t* t, am_timestamp_t o)
{
	return am_sub_sat_u64(*t, o, t);
}

/* Saturated multiplication of a timestamp *t with another timestamp o. */
static inline enum am_arithmetic_status
am_timestamp_mul_sat(am_timestamp_t* t, am_timestamp_t o)
{
	return am_mul_sat_u64(*t, o, t);
}

/* Saturated addition of a timestamp *t and a timestamp difference o. */
static inline enum am_arithmetic_status
am_timestamp_add_sat_diff(am_timestamp_t* t, am_timestamp_diff_t o)
{
	return am_add_sat_ui64(*t, o, t);
}

/* Saturated subtraction of a timestamp *t and a timestamp difference o. */
static inline enum am_arithmetic_status
am_timestamp_sub_sat_diff(am_timestamp_t* t, am_timestamp_diff_t o)
{
	return am_sub_sat_ui64(*t, o, t);
}

/* Saturated subtraction of a timestamp difference *d and a timestamp difference
 * o. */
static inline enum am_arithmetic_status
am_timestamp_diff_sub_sat(am_timestamp_t* d, am_timestamp_diff_t o)
{
	return am_sub_sat_ui64(*d, o, d);
}

#endif
