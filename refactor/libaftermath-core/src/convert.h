/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * ************************************************************************
 * * THIS FILE IS PART OF THE CODE RELEASED UNDER THE LGPL, VERSION 2.1   *
 * * UNLIKE THE MAJORITY OF THE CODE OF LIBAFTERMATH-CORE, RELEASED UNDER *
 * * THE GPL, VERSION 2.                                                  *
 * ************************************************************************
 *
 * This file can be redistributed it and/or modified under the terms of
 * the GNU Lesser General Public License version 2.1 as published by the
 * Free Software Foundation.
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

#ifndef AM_CONVERT_H
#define AM_CONVERT_H

#include <endian.h>
#include <stdint.h>
#include <math.h>

/* Check for IEC 60559 floating point arithmetic to guarantee 4 bytes for float
 * and 8 bytes for double */
#ifndef __STDC_IEC_559__
	#error "IEC 60559 floating point arithmetic not supported by your compiler"
#endif

/* Converts from one endianness into the other. Val is the expression to be
 * converted, ret receives the converted value and type is the type of the input
 * value. */
#define AM_SWAP_BITS(val, ret, type)					\
	do {								\
		ret = 0;						\
		for(unsigned int i = 0; i < 8*sizeof(type); i += 8)	\
			ret |= ((val >> i) & 0xFF) << ((sizeof(type)*8-8) - i); \
	} while(0)

/* Converts a float with guaranteed size of 4 bytes from one endianness to the
 * other */
static inline float am_float32_swap(float val)
{
	union {
		float fval;
		uint32_t uival;
	} u;
	uint32_t retu;

	u.fval = val;

	AM_SWAP_BITS(u.uival, retu, uint32_t);
	u.uival = retu;

	return u.fval;
}

/* Converts a double with guaranteed size of 8 bytes from one endianness to the
 * other */
static inline double am_double64_swap(double val)
{
	union {
		double dval;
		uint64_t uival;
	} u;
	uint64_t retu;

	u.dval = val;

	AM_SWAP_BITS(u.uival, retu, uint64_t);
	u.uival = retu;

	return u.dval;
}

/* Convert int64_t from one endianness to the other */
static inline int64_t am_int64_swap(int64_t val)
{
	int64_t ret;
	AM_SWAP_BITS(val, ret, int64_t);

	return ret;
}

/* Convert int32_t from one endianness to the other */
static inline int32_t am_int32_swap(int32_t val)
{
	int32_t ret;
	AM_SWAP_BITS(val, ret, int32_t);

	return ret;
}

/* Convert int16_t from one endianness to the other */
static inline int16_t am_int16_swap(int16_t val)
{
	int16_t ret;
	AM_SWAP_BITS(val, ret, int16_t);

	return ret;
}

/* Define host to little endian and little endian to host endian macros. If the
 * host endianness is little endian, this expands to the original value,
 * otherwise to a call to the conversion function. */
#if __BYTE_ORDER == __LITTLE_ENDIAN
	#define am_float32_htole(val) val
	#define am_double64_htole(val) val
	#define am_int16_htole(val) val
	#define am_int32_htole(val) val
	#define am_int64_htole(val) val
	#define am_float32_letoh(val) val
	#define am_double64_letoh(val) val
	#define am_int16_letoh(val) val
	#define am_int32_letoh(val) val
	#define am_int64_letoh(val) val
#elif __BYTE_ORDER == __BIG_ENDIAN
	#define am_float32_htole(val) am_float32_swap(val)
	#define am_double64_htole(val) am_double64_swap(val)
	#define am_int16_htole(val) am_int16_swap(val)
	#define am_int32_htole(val) am_int32_swap(val)
	#define am_int64_htole(val) am_int64_swap(val)
	#define am_float32_letoh(val) am_float32_swap(val)
	#define am_double64_letoh(val) am_double64_swap(val)
	#define am_int16_letoh(val) am_int16_swap(val)
	#define am_int32_letoh(val) am_int32_swap(val)
	#define am_int64_letoh(val) am_int64_swap(val)
#else
	#error "Could not determine your system's endianness"
#endif

#endif
