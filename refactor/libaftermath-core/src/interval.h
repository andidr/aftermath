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

#ifndef AM_INTERVAL_H
#define AM_INTERVAL_H

#include "in_memory.h"
#include "timestamp.h"

/* Checks if two intervals overlap. Returns true if this is the case, otherwise
 * false. */
static inline int am_interval_intersect_p(const struct am_interval* a,
					  const struct am_interval* b)
{
	return !(a->end < b->start || a->start > b->end);
}

/* Calculates the overlap between two intervals. If the intervals do not
 * overlap, the function returns 1. If they do overlap, out is initialized with
 * the intersection and the function returns 0. */
static inline int am_interval_intersection(const struct am_interval* a,
					   const struct am_interval* b,
					   struct am_interval* out)
{
	if(!am_interval_intersect_p(a, b))
		return 1;

	out->start = (a->start <= b->start) ? b->start : a->start;
	out->end = (a->end <= b->end) ? a->end : b->end;

	return 0;
}

/* Extends the interval a, such that the interval b is included in a. */
static inline void
am_interval_extend(struct am_interval* a, const struct am_interval* b)
{
	if(b->start < a->start)
		a->start = b->start;

	if(b->end > a->end)
		a->end = b->end;
}

/* Extends the interval a, such that the timestamp t is included in a. */
static inline void
am_interval_extend_timestamp(struct am_interval* a, am_timestamp_t t)
{
	if(t < a->start)
		a->start = t;

	if(t > a->end)
		a->end = t;
}

/* Calculates the duration of an interval. The return value indicates whether
 * the result is saturated. */
static inline enum am_arithmetic_status
am_interval_duration(const struct am_interval* i, am_timestamp_diff_t* out)
{
	am_timestamp_t udiff = i->end - i->start;

	if(udiff <= AM_TIMESTAMP_DIFF_T_MAX)
		*out = udiff;
	else
		*out = AM_TIMESTAMP_DIFF_T_MAX;

	return am_timestamp_diff_add_sat_u64(out, 1);
}

/* Calculates the duration of an interval as an unsigned value. The return value
 * indicates whether the result is saturated. */
static inline enum am_arithmetic_status
am_interval_duration_u(const struct am_interval* i, am_timestamp_t* out)
{
	*out = i->end - i->start;
	return am_timestamp_add_sat(out, 1);
}

/* Decreases the start of the interval by d and increases the end by d. If the
 * result would cause the start to move beyond the end, the interval is reduced
 * to an interval of size 1 at its middle.
 *
 * Returns 1 if the interval hasn't been extended entirely by d at both ends,
 * otherwise 0.
 */
static inline int am_interval_widen(struct am_interval* i, am_timestamp_diff_t d)
{
	am_timestamp_diff_t dur;
	int ret = 0;

	am_interval_duration(i, &dur);

	if(d < 0 && d < -(dur / 2)) {
		am_timestamp_add_sat_diff(&i->start, dur / 2);
		i->end = i->start;

		ret = 1;
	} else {
		if(am_timestamp_sub_sat_diff(&i->start, d) !=
		   AM_ARITHMETIC_STATUS_EXACT)
		{
			ret = 1;
		}

		if(am_timestamp_add_sat_diff(&i->end, d) !=
		   AM_ARITHMETIC_STATUS_EXACT)
		{
			ret = 1;
		}
	}

	return ret;
}

/* Widens the interval i by an unsigned value d.
 *
 * Returns 1 if the interval hasn't been extended entirely by d at both ends,
 * otherwise 0. */
static inline int am_interval_widen_u(struct am_interval* i, am_timestamp_t d)
{
	int ret;

	if(am_timestamp_sub_sat(&i->start, d) != AM_ARITHMETIC_STATUS_EXACT)
		ret = 1;

	if(am_timestamp_add_sat(&i->end, d) != AM_ARITHMETIC_STATUS_EXACT)
		ret = 1;

	return ret;
}

/* Shrinks the interval i by an unsigned value d. If the result would cause the
 * start to move beyond the end, the interval is reduced to an interval of size
 * 1 at its middle.
 *
 * Returns 1 if the interval hasn't been extended entirely by d at both ends,
 * otherwise 0. */
static inline int am_interval_shrink_u(struct am_interval* i, am_timestamp_t d)
{
	int ret = 0;
	am_timestamp_t dur;

	am_interval_duration_u(i, &dur);

	if(d > dur / 2) {
		i->end = i->start + dur / 2;
		i->start = i->end;
		ret = 1;
	} else {
		/* Addition cannot overflow */
		i->start += d;

		if(am_timestamp_sub_sat(&i->end, d) !=
		   AM_ARITHMETIC_STATUS_EXACT)
		{
			ret = 1;
		}
	}

	return ret;
}

/* Shifts the interval by d. If either the start or the end of the interval
 * reaches the minimum or maximum value, the interval is only shifted by the an
 * amount that does not cause an overflow / underflow.
 *
 * Returns 1 if the interval couldn't be shifted by the entire value of d,
 * otherwise 0.
 */
static inline int am_interval_shift(struct am_interval* a, am_timestamp_diff_t d)
{
	am_timestamp_t tmp;
	int ret;

	if(d >= 0) {
		tmp = a->end;

		ret = am_timestamp_add_sat_diff(&a->end, d);
		am_timestamp_add_sat(&a->start, a->end - tmp);
	} else {
		tmp = a->start;

		ret = am_timestamp_add_sat_diff(&a->start, d);
		am_timestamp_add_sat(&a->end, tmp - a->start);
	}

	return ret;
}

/* Shifts the interval by an unsigned value d to the right. If the end of the
 * interval reaches the maximum value, the interval is only shifted by the an
 * amount that does not cause an overflow.
 *
 * Returns 1 if the interval couldn't be shifted by the entire value of d,
 * otherwise 0.
 */
static inline int am_interval_shift_right_u(struct am_interval* a, am_timestamp_t d)
{
	am_timestamp_t tmp;
	int ret;

	tmp = a->end;

	ret = am_timestamp_add_sat(&a->end, d);
	am_timestamp_add_sat(&a->start, a->end - tmp);

	return ret;
}

/* Shifts the interval by an unsigned value d to the left. If the start of the
 * interval reaches 0, the interval is only shifted by the an amount that does
 * not cause an underflow.
 *
 * Returns 1 if the interval couldn't be shifted by the entire value of d,
 * otherwise 0.
 */
static inline int am_interval_shift_left_u(struct am_interval* a, am_timestamp_t d)
{
	am_timestamp_t tmp;
	int ret;

	tmp = a->start;

	ret = am_timestamp_sub_sat(&a->start, d);
	am_timestamp_sub_sat(&a->end, tmp - a->start);

	return ret;
}

#endif
