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

#ifndef AM_ARITHMETIC_H
#define AM_ARITHMETIC_H

#include <stdint.h>
#include <inttypes.h>
#include <float.h>
#include <stdlib.h>

/* Indicates the status of an arithmetic operation */
enum am_arithmetic_status {
	AM_ARITHMETIC_STATUS_EXACT = 0,
	AM_ARITHMETIC_STATUS_UNDERFLOW,
	AM_ARITHMETIC_STATUS_OVERFLOW
};

static inline enum am_arithmetic_status
am_sub_safe_u64(uint64_t a, uint64_t b, uint64_t* out);

static inline enum am_arithmetic_status
am_sub_sat_u64(uint64_t a, uint64_t b, uint64_t* out);

static inline enum am_arithmetic_status
am_sub_safe_i64(int64_t a, int64_t b, int64_t* out);

static inline enum am_arithmetic_status
am_sub_sat_i64(int64_t a, int64_t b, int64_t* out);


/* Saturate a uint64_t according to the overflow status of an arithmetic
 * operation. */
static inline void am_sat_u64(uint64_t* i, enum am_arithmetic_status s)
{
	if(s == AM_ARITHMETIC_STATUS_OVERFLOW)
		*i = UINT64_MAX;
	else if(s == AM_ARITHMETIC_STATUS_UNDERFLOW)
		*i = 0;
}

/* Saturate an int64_t according to the overflow status of an arithmetic
 * operation. */
static inline void am_sat_i64(int64_t* i, enum am_arithmetic_status s)
{
	if(s == AM_ARITHMETIC_STATUS_OVERFLOW)
		*i = INT64_MAX;
	else if(s == AM_ARITHMETIC_STATUS_UNDERFLOW)
		*i = INT64_MIN;
}

/* Saturate a size_t according to the overflow status of an arithmetic
 * operation. */
static inline void am_sat_size(size_t* i, enum am_arithmetic_status s)
{
	if(s == AM_ARITHMETIC_STATUS_OVERFLOW)
		*i = SIZE_MAX;
	else if(s == AM_ARITHMETIC_STATUS_UNDERFLOW)
		*i = 0;
}

/* Checks if the 128 bit signed integer *i can safely be converted into an
 * int64_t without loss. */
static inline enum am_arithmetic_status am_i128_check_i64(const __int128* i)
{
	if((*i) >= 0) {
		/* Positive integer must not have any of the upper bits set */
		if(((*i) >> 64) & 0xFFFFFFFFFFFFFFFF)
			return AM_ARITHMETIC_STATUS_OVERFLOW;
	} else {
		/* Sign bit set, but not extended until 64th bit -> underflow */
		if((((*i) >> 64) & 0xFFFFFFFFFFFFFFFF) != 0xFFFFFFFFFFFFFFFF)
			return AM_ARITHMETIC_STATUS_UNDERFLOW;

		/* Sign bit extended in upper 64 bit, but no sign bit in 64-bit
		 * value -> underflow */
		if(!((*i) & 0x8000000000000000))
			return AM_ARITHMETIC_STATUS_UNDERFLOW;
	}

	return AM_ARITHMETIC_STATUS_EXACT;
}

/* Checks if the 128 bit signed integer *i can safely be converted into a
 * uint64_t without loss. */
static inline enum am_arithmetic_status am_i128_check_u64(const __int128* i)
{
	if((*i) >= 0) {
		/* Positive integer must not have any of the upper bits set */
		if(((*i) >> 64) & 0xFFFFFFFFFFFFFFFF)
			return AM_ARITHMETIC_STATUS_OVERFLOW;
	} else {
		/* All negative values cannot be converted into an unsigned
		 * value -> underflow */
		return AM_ARITHMETIC_STATUS_UNDERFLOW;
	}

	return AM_ARITHMETIC_STATUS_EXACT;
}


/* Checks if the 128 bit unsigned integer *i can safely be converted into a
 * uint64_t without loss. */
static inline enum am_arithmetic_status am_u128_check_u64(const unsigned __int128* i)
{
	/* Any of the upper 64 bits set -> overflow */
	if((*i) >> 64)
		return AM_ARITHMETIC_STATUS_OVERFLOW;
	else
		return AM_ARITHMETIC_STATUS_EXACT;
}

/* Calculates *out = a * b / c for 64-bit unsigned integers without overflows if
 * the final value fits into a 64-bit unsigned integer. The return status
 * indicates whether the result is exact or if an overflow took place. */
static inline enum am_arithmetic_status
am_muldiv_safe_u64(uint64_t a, uint64_t b, uint64_t c, uint64_t* out)
{
	unsigned __int128 ax = a;
	unsigned __int128 bx = b;
	unsigned __int128 cx = c;
	unsigned __int128 res;

	res = (ax * bx) / cx;
	*out = (uint64_t)res;

	return am_u128_check_u64(&res);
}

#define AM_DECL_MULDIV_U_FUN(BITS, T, TBIG)					\
	/* Calculates *out = a * b / c for BITS-bit unsigned integers without	\
	 * overflows if the final value fits into a 64-bit unsigned integer.	\
	 * The function only guarantees not to overflow during the		\
	 * multiplication and division, but does not check if the final result	\
	 * can be represented using BITS bits. */				\
	static inline void am_muldiv_u##BITS(T a, T b, T c, T* out)	\
	{								\
		TBIG ax = a;						\
		TBIG bx = b;						\
		TBIG cx = c;						\
		TBIG res;						\
									\
		res = (ax * bx) / cx;					\
		*out = (T)res;						\
	}

AM_DECL_MULDIV_U_FUN(8, uint8_t, uint_fast16_t)
AM_DECL_MULDIV_U_FUN(16, uint16_t, uint_fast32_t)
AM_DECL_MULDIV_U_FUN(32, uint32_t, uint_fast64_t)
AM_DECL_MULDIV_U_FUN(64, uint64_t, unsigned __int128)

/* Saturated computation of *out = a * b / c for unsigned 64-bit integers. The
 * return value indicates whether saturation took place. */
static inline enum am_arithmetic_status
am_muldiv_sat_u64(uint64_t a, uint64_t b, uint64_t c, uint64_t* out)
{
	enum am_arithmetic_status status = am_muldiv_safe_u64(a, b, c, out);
	am_sat_u64(out, status);
	return status;
}

/* Calculates *out = a * b / c for 64-bit signed integers without overflows if
 * the final value fits into a 64-bit signed integer. The return status
 * indicates whether the result is exact or if an overflow took place. */
static inline enum am_arithmetic_status
am_muldiv_safe_i64(int64_t a, int64_t b, int64_t c, int64_t* out)
{
	__int128 ax = a;
	__int128 bx = b;
	__int128 cx = c;
	__int128 res;

	res = (ax * bx) / cx;
	*out = (int64_t)res;

	return am_i128_check_i64(&res);
}

/* Saturated computation of *out = a * b / c for signed 64-bit integers. The
 * return value indicates whether saturation took place. */
static inline enum am_arithmetic_status
am_muldiv_sat_i64(int64_t a, int64_t b, int64_t c, int64_t* out)
{
	enum am_arithmetic_status status = am_muldiv_safe_i64(a, b, c, out);
	am_sat_i64(out, status);
	return status;
}

/* Calculates *out = a * b / c for 64-bit unsigned integers a and *out and
 * signed 64-bit integers b and c without overflows if the final value fits into
 * a 64-bit unsigned integer. The return status indicates whether the result is
 * exact or if an overflow took place. */
static inline enum am_arithmetic_status
am_muldiv_safe_uii64(uint64_t a, int64_t b, int64_t c, uint64_t* out)
{
	__int128 ax = a;
	__int128 bx = b;
	__int128 cx = c;
	__int128 res;

	res = (ax * bx) / cx;
	*out = (uint64_t)res;

	return am_i128_check_u64(&res);
}

/* Saturated computation of *out = a * b / c for 64-bit unsigned integers a and
 * *out and signed 64-bit integers b and c. The return value indicates whether
 * saturation took place. */
static inline enum am_arithmetic_status
am_muldiv_sat_uii64(uint64_t a, int64_t b, int64_t c, uint64_t* out)
{
	enum am_arithmetic_status status = am_muldiv_safe_uii64(a, b, c, out);
	am_sat_u64(out, status);
	return status;
}

/* Calculates *out = a * b / c for 64-bit signed integers a and *out and
 * unsigned 64-bit integers b and c without overflows if the final value fits
 * into a 64-bit signed integer. The return status indicates whether the result
 * is exact or if an overflow took place. */
static inline enum am_arithmetic_status
am_muldiv_safe_iuu64(int64_t a, uint64_t b, uint64_t c, int64_t* out)
{
	__int128 ax = a;
	__int128 bx = b;
	__int128 cx = c;
	__int128 res;

	res = (ax * bx) / cx;
	*out = (int64_t)res;

	return am_i128_check_i64(&res);
}

/* Saturated computation of *out = a * b / c for 64-bit signed integers a and
 * *out and unsigned 64-bit integers b and c. The return value indicates whether
 * saturation took place. */
static inline enum am_arithmetic_status
am_muldiv_sat_iuu64(int64_t a, uint64_t b, uint64_t c, int64_t* out)
{
	enum am_arithmetic_status status = am_muldiv_safe_iuu64(a, b, c, out);
	am_sat_i64(out, status);
	return status;
}

/* Calculates *out = a * b for 64-bit signed integers without overflows if the
 * final value fits into a 64-bit signed integer. The return status indicates
 * whether the result is exact or if an overflow took place. */
static inline enum am_arithmetic_status
am_mul_safe_i64(int64_t a, int64_t b, int64_t* out)
{
	__int128 ax = a;
	__int128 bx = b;
	__int128 res;

	res = ax * bx;
	*out = (int64_t)res;

	return am_i128_check_i64(&res);
}

/* Saturated computation of *out = a * b for 64-bit signed integers. The return
 * value indicates whether saturation took place. */
static inline enum am_arithmetic_status
am_mul_sat_i64(int64_t a, int64_t b, int64_t* out)
{
	enum am_arithmetic_status status = am_mul_safe_i64(a, b, out);
	am_sat_i64(out, status);
	return status;
}

/* Calculates *out = a * b for 64-bit unsigned integers without overflows if the
 * final value fits into a 64-bit unsigned integer. The return status indicates
 * whether the result is exact or if an overflow took place. */
static inline enum am_arithmetic_status
am_mul_safe_u64(uint64_t a, uint64_t b, uint64_t* out)
{
	__int128 ax = a;
	__int128 bx = b;
	__int128 res;

	res = ax * bx;
	*out = (uint64_t)res;

	return am_i128_check_u64(&res);
}

/* Saturated computation of *out = a * b for 64-bit unsigned integers. The
 * return value indicates whether saturation took place. */
static inline enum am_arithmetic_status
am_mul_sat_u64(uint64_t a, uint64_t b, uint64_t* out)
{
	enum am_arithmetic_status status = am_mul_safe_u64(a, b, out);
	am_sat_u64(out, status);
	return status;
}

/* Calculates *out = a * b for 64-bit signed integers a and *out and an unsigned
 * 64-bit integer b without overflows if the final value fits into a 64-bit
 * signed integer. The return status indicates whether the result is exact or if
 * an overflow took place. */
static inline enum am_arithmetic_status
am_mul_safe_iu64(int64_t a, uint64_t b, int64_t* out)
{
	__int128 ax = a;
	__int128 bx = b;
	__int128 res;

	res = ax * bx;
	*out = (int64_t)res;

	return am_i128_check_i64(&res);
}

/* Saturated computation of *out = a * b for 64-bit signed integers a and *out
 * and an unsigned 64-bit integer b. The return value indicates whether
 * saturation took place. */
static inline enum am_arithmetic_status
am_mul_sat_iu64(int64_t a, uint64_t b, int64_t* out)
{
	enum am_arithmetic_status status = am_mul_safe_iu64(a, b, out);
	am_sat_i64(out, status);
	return status;
}

#define AM_DECL_ADD_SAFE_UINT_FUN(T, SUFFIX, VMAX)				\
	/* Calculates *out = a + b without overflows if the final value fits	\
	 * into a T. The return value indicates whether an overflow took	\
	 * place. */								\
	static inline enum am_arithmetic_status				\
	am_add_safe_##SUFFIX(T a, T b, T* out)					\
	{									\
		*out = a + b;							\
										\
		if((a > b && VMAX - a < b) || (b > a && VMAX - b < a))		\
			return AM_ARITHMETIC_STATUS_OVERFLOW;			\
		else								\
			return AM_ARITHMETIC_STATUS_EXACT;			\
	}									\

AM_DECL_ADD_SAFE_UINT_FUN( uint8_t,  u8,  UINT8_MAX)
AM_DECL_ADD_SAFE_UINT_FUN(uint16_t, u16, UINT16_MAX)
AM_DECL_ADD_SAFE_UINT_FUN(uint32_t, u32, UINT32_MAX)
AM_DECL_ADD_SAFE_UINT_FUN(uint64_t, u64, UINT64_MAX)
AM_DECL_ADD_SAFE_UINT_FUN(size_t,  size, SIZE_MAX)

/* Saturated computation of *out = a + b for 64-bit unsigned integers. The
 * return value indicates whether saturation took place. */
static inline enum am_arithmetic_status
am_add_sat_u64(uint64_t a, uint64_t b, uint64_t* out)
{
	enum am_arithmetic_status status = am_add_safe_u64(a, b, out);
	am_sat_u64(out, status);
	return status;
}

/* Saturated computation of *out = a + b for size_t. The return value indicates
 * whether saturation took place. */
static inline enum am_arithmetic_status
am_add_sat_size(size_t a, size_t b, size_t* out)
{
	enum am_arithmetic_status status = am_add_safe_size(a, b, out);
	am_sat_size(out, status);
	return status;
}

#define AM_DECL_ADD_SAFE_SIGNED_FUN(T, SUFFIX, VMIN, VMAX)			\
	/* Calculates *out = a + b without overflows / underflows if the final	\
	 * value fits into a T. The return value indicates whether an overflow /\
	 * underflow took place or if the result is exact. */			\
	static inline enum am_arithmetic_status				\
	am_add_safe_##SUFFIX(T a, T b, T* out)					\
	{									\
		*out = a + b;							\
										\
		if(a > 0 && b > 0) {						\
			if((a > b && VMAX - a < b) ||				\
			   (b > a && VMAX - b < a))				\
			{							\
				return AM_ARITHMETIC_STATUS_OVERFLOW;		\
			}							\
		} else if(a < 0 && b < 0) {					\
			if((a < b && -(VMIN - a) > b) ||			\
			   (b < a && -(VMIN - b) > a))				\
			{							\
				return AM_ARITHMETIC_STATUS_UNDERFLOW;		\
			}							\
		}								\
										\
		return AM_ARITHMETIC_STATUS_EXACT;				\
	}

AM_DECL_ADD_SAFE_SIGNED_FUN( int8_t,  i8,  INT8_MIN,  INT8_MAX)
AM_DECL_ADD_SAFE_SIGNED_FUN(int16_t, i16, INT16_MIN, INT16_MAX)
AM_DECL_ADD_SAFE_SIGNED_FUN(int32_t, i32, INT32_MIN, INT32_MAX)
AM_DECL_ADD_SAFE_SIGNED_FUN(int64_t, i64, INT64_MIN, INT64_MAX)
AM_DECL_ADD_SAFE_SIGNED_FUN(double, double, DBL_MIN, DBL_MAX)

/* Saturated computation of *out = a + b for 64-bit signed integers. The
 * return value indicates whether saturation took place. */
static inline enum am_arithmetic_status
am_add_sat_i64(int64_t a, int64_t b, int64_t* out)
{
	enum am_arithmetic_status status = am_add_safe_i64(a, b, out);
	am_sat_i64(out, status);
	return status;
}

/* Calculates *out = a + b without overflows / underflows for unsigned integers
 * a and *out and a signed integer b if the final value fits into a 64 bit
 * unsigned integer. The return value indicates whether an overflow / underflow
 * took place or if the result is exact. */
static inline enum am_arithmetic_status
am_add_safe_ui64(uint64_t a, int64_t b, uint64_t* out)
{
	if(b < 0)
		return am_sub_safe_u64(a, -b, out);
	else
		return am_add_safe_u64(a, b, out);
}

/* Saturated computation of *out = a + b for 64-bit unsigned integers a and *out
 * and a signed 64-bit integer b. The return value indicates whether saturation
 * took place. */
static inline enum am_arithmetic_status
am_add_sat_ui64(uint64_t a, int64_t b, uint64_t* out)
{
	if(b < 0)
		return am_sub_sat_u64(a, -b, out);
	else
		return am_add_sat_u64(a, b, out);
}

#define AM_DECL_SAFE_NEGATE_FUN(BITS)						\
	/* Safely changes the sign of a negative signed 64-bit integer and	\
	 * returns the corresponding unsigned 64-bit integer without		\
	 * overflowing. The argument must be negative.				\
	 */									\
	static inline uint##BITS##_t am_safe_negate_i##BITS(int##BITS##_t v)	\
	{									\
		uint##BITS##_t* up = (uint##BITS##_t*)&v;			\
		uint##BITS##_t u = *up;					\
										\
		return ~u + 1;							\
	}

AM_DECL_SAFE_NEGATE_FUN(8)
AM_DECL_SAFE_NEGATE_FUN(16)
AM_DECL_SAFE_NEGATE_FUN(32)
AM_DECL_SAFE_NEGATE_FUN(64)

#define AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_I_FUN(BITS)			\
	/* Calculates the size of the interval [left; right) of two signed	\
	 * integers without overflows. The size is returned in an unsigned	\
	 * integer *out with the same number of bits.				\
	 *									\
	 * Returns 0 if the result can be represented, otherwise 1. */		\
	static inline int							\
	am_safe_rightopen_interval_size_i##BITS(uint##BITS##_t* out,		\
						int##BITS##_t left,		\
						int##BITS##_t right)		\
	{									\
		/* Swap if necessary */					\
		if(right < left)						\
			return 1;						\
										\
		if(left < 0 && right < 0) {					\
			*out = right - left;					\
		} else if(left < 0 && right >= 0) {				\
			*out = am_safe_negate_i##BITS(left) +			\
				((uint##BITS##_t)right);			\
		} else	/* both positive */ {					\
			*out = right - left;					\
		}								\
										\
		return 0;							\
	}

AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_I_FUN(8)
AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_I_FUN(16)
AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_I_FUN(32)
AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_I_FUN(64)

#define AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_U_FUN(BITS)			\
	/* Calculates the size of the interval [left; right) of two unsigned	\
	 * integers. The size is returned in an unsigned integer *out with the	\
	 * same number of bits.						\
	 *									\
	 * Returns 0 if the result can be represented, otherwise 1. */		\
	static inline int							\
	am_safe_rightopen_interval_size_u##BITS(uint##BITS##_t* out,		\
						int##BITS##_t left,		\
						int##BITS##_t right)		\
	{									\
		if(right < left)						\
			return 1;						\
										\
		*out = right - left;						\
										\
		return 0;							\
	}

AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_U_FUN(8)
AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_U_FUN(16)
AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_U_FUN(32)
AM_DECL_SAFE_RIGHTOPEN_INTERVAL_SIZE_U_FUN(64)

#define AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_I_FUN(BITS)				\
	/* Calculates the size of the interval [left; right] of two signed	\
	 * integers without overflows. The size is returned in an unsigned	\
	 * integer *out with the same number of bits.				\
	 *									\
	 * Returns 0 if the result can be represented, otherwise 1. */		\
	static inline int							\
	am_safe_closed_interval_size_i##BITS(uint##BITS##_t* out,		\
					     int##BITS##_t left,		\
					     int##BITS##_t right)		\
	{									\
		/* Final + 1 would overflow */					\
		if(left == INT##BITS##_MIN && right == INT##BITS##_MAX)	\
			return 1;						\
										\
		if(am_safe_rightopen_interval_size_i##BITS(out, left, right))	\
			return 1;						\
										\
		*out += 1;							\
										\
		return 0;							\
	}

AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_I_FUN(8)
AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_I_FUN(16)
AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_I_FUN(32)
AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_I_FUN(64)

#define AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_U_FUN(BITS)				\
	/* Calculates the size of the interval [left; right] of two unsigned	\
	 * integers without overflows. The size is returned in an unsigned	\
	 * integer *out with the same number of bits.				\
	 *									\
	 * Returns 0 if the result can be represented, otherwise 1. */		\
	static inline int							\
	am_safe_closed_interval_size_u##BITS(uint##BITS##_t* out,		\
					     uint##BITS##_t left,		\
					     uint##BITS##_t right)		\
	{									\
		if(right < left)						\
			return 1;						\
										\
		/* Final + 1 would overflow */					\
		if(left == 0 && right == UINT##BITS##_MAX)			\
			return 1;						\
										\
		*out = right - left + 1;					\
										\
		return 0;							\
	}

AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_U_FUN(8)
AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_U_FUN(16)
AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_U_FUN(32)
AM_DECL_SAFE_CLOSED_INTERVAL_SIZE_U_FUN(64)

/* Calculates *out = a + b without overflows / underflows for signed integers a
 * and *out and an unsigned integer b if the final value fits into a 64 bit
 * signed integer. The return value indicates whether an overflow / underflow
 * took place or if the result is exact. */
static inline enum am_arithmetic_status
am_add_safe_iu64(int64_t a, uint64_t b, int64_t* out)
{
	uint64_t n;

	*out = a + b;

	if(a >= 0 && ((uint64_t)(INT64_MAX - a)) < b)
		return AM_ARITHMETIC_STATUS_OVERFLOW;
	else if(a < 0) {
		n = am_safe_negate_i64(a);

		if(n < b && b - n > ((uint64_t)INT64_MAX))
			return AM_ARITHMETIC_STATUS_OVERFLOW;
	}

	return AM_ARITHMETIC_STATUS_EXACT;
}

/* Saturated computation of *out = a + b for a 64-bit signed integer and a
 * 64-bit unsigned integer. The return value indicates whether saturation took
 * place. */
static inline enum am_arithmetic_status
am_add_sat_iu64(int64_t a, uint64_t b, int64_t* out)
{
	enum am_arithmetic_status status = am_add_safe_iu64(a, b, out);
	am_sat_i64(out, status);
	return status;
}

#define AM_DECL_SUB_SAFE_UINT_FUN(T, SUFFIX)					\
	/* Calculates *out = a - b without overflows if the final value fits	\
	 * into a T. The return value indicates whether an overflow / underflow \
	 * took place. */							\
	static inline enum am_arithmetic_status				\
	am_sub_safe_##SUFFIX(T a, T b, T* out)					\
	{									\
		*out = a - b;							\
										\
		if(a < b)							\
			return AM_ARITHMETIC_STATUS_UNDERFLOW;			\
		else								\
			return AM_ARITHMETIC_STATUS_EXACT;			\
	}

AM_DECL_SUB_SAFE_UINT_FUN( uint8_t,  u8)
AM_DECL_SUB_SAFE_UINT_FUN(uint16_t, u16)
AM_DECL_SUB_SAFE_UINT_FUN(uint32_t, u32)
AM_DECL_SUB_SAFE_UINT_FUN(uint64_t, u64)

/* Saturated computation of *out = a - b for 64-bit unsigned integers. The
 * return value indicates whether saturation took place. */
static inline enum am_arithmetic_status
am_sub_sat_u64(uint64_t a, uint64_t b, uint64_t* out)
{
	enum am_arithmetic_status status = am_sub_safe_u64(a, b, out);
	am_sat_u64(out, status);
	return status;
}

#define AM_DECL_SUB_SAFE_SIGNED_FUN(T, SUFFIX, VMIN, VMAX)			\
	/* Calculates *out = a - b without overflows / underflows if the final	\
	 * value fits into a T. The return value indicates whether an overflow /\
	 * underflow took place. */						\
	static inline enum am_arithmetic_status				\
	am_sub_safe_##SUFFIX(T a, T b, T* out)					\
	{									\
		*out = a - b;							\
										\
		if(a < 0 && b > 0 && -(VMIN - a) < b)				\
			return AM_ARITHMETIC_STATUS_UNDERFLOW;			\
		else if(a > 0 && b < 0 && (b == VMIN || VMAX - a < -b))	\
			return AM_ARITHMETIC_STATUS_OVERFLOW;			\
		else								\
			return AM_ARITHMETIC_STATUS_EXACT;			\
	}

AM_DECL_SUB_SAFE_SIGNED_FUN( int8_t,  i8,  INT8_MIN,  INT8_MAX)
AM_DECL_SUB_SAFE_SIGNED_FUN(int16_t, i16, INT16_MIN, INT16_MAX)
AM_DECL_SUB_SAFE_SIGNED_FUN(int32_t, i32, INT32_MIN, INT32_MAX)
AM_DECL_SUB_SAFE_SIGNED_FUN(int64_t, i64, INT64_MIN, INT64_MAX)
AM_DECL_SUB_SAFE_SIGNED_FUN(double, double, DBL_MIN, DBL_MAX)

/* Saturated computation of *out = a - b for 64-bit signed integers. The
 * return value indicates whether saturation took place. */
static inline enum am_arithmetic_status
am_sub_sat_i64(int64_t a, int64_t b, int64_t* out)
{
	enum am_arithmetic_status status = am_sub_safe_i64(a, b, out);
	am_sat_i64(out, status);
	return status;
}

/* Calculates *out = a - b for 64-bit unsigned integers a and *out and a signed
 * 64-bit integer b without overflows / underflows. The return value indicates
 * whether an overflow / underflow took place. */
static inline enum am_arithmetic_status
am_sub_safe_ui64(uint64_t a, int64_t b, uint64_t* out)
{
	*out = a - b;

	if(b > 0 && a < (uint64_t)b)
		return AM_ARITHMETIC_STATUS_UNDERFLOW;
	else if(b < 0 && (UINT64_MAX - a) < am_safe_negate_i64(b))
		return AM_ARITHMETIC_STATUS_OVERFLOW;
	else
		return AM_ARITHMETIC_STATUS_EXACT;
}

/* Saturated computation of *out = a - b for 64-bit unsigned integers a and *out
 * and a signed 64-bit integer b. The return value indicates whether saturation
 * took place. */
static inline enum am_arithmetic_status
am_sub_sat_ui64(uint64_t a, int64_t b, uint64_t* out)
{
	enum am_arithmetic_status status = am_sub_safe_ui64(a, b, out);
	am_sat_u64(out, status);
	return status;
}

#endif
