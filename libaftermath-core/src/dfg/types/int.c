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

#include "timestamp.h"
#include <aftermath/core/ansi_extras.h>
#include <stdio.h>
#include <inttypes.h>

#define AM_UINT8_T_MAX_DECIMAL_DIGITS 3
#define AM_UINT16_T_MAX_DECIMAL_DIGITS 5
#define AM_UINT32_T_MAX_DECIMAL_DIGITS 11
#define AM_UINT64_T_MAX_DECIMAL_DIGITS 21

#define AM_INT8_T_MAX_DECIMAL_DIGITS (AM_UINT8_T_MAX_DECIMAL_DIGITS+1)
#define AM_INT16_T_MAX_DECIMAL_DIGITS (AM_UINT16_T_MAX_DECIMAL_DIGITS+1)
#define AM_INT32_T_MAX_DECIMAL_DIGITS (AM_UINT32_T_MAX_DECIMAL_DIGITS+1)
#define AM_INT64_T_MAX_DECIMAL_DIGITS (AM_UINT64_T_MAX_DECIMAL_DIGITS+1)

#define AM_DFG_TYPE_UINT_IMPL(T, TPREFIX, TPREFIXUP, AMPREFIX, FMT)		\
	int am_dfg_type_##TPREFIX##_to_string(const struct am_dfg_type* t,	\
					      void* ptr,			\
					      char** out,			\
					      int* cst)			\
	{									\
		char* ret;							\
		T* pintval = ptr;						\
										\
		if(!(ret = malloc(AM_##TPREFIXUP##_T_MAX_DECIMAL_DIGITS + 1)))	\
			return 1;						\
										\
		snprintf(ret, AM_##TPREFIXUP##_T_MAX_DECIMAL_DIGITS + 1,	\
			 "%" FMT, *pintval);					\
										\
		*out = ret;							\
		*cst = 0;							\
										\
		return 0;							\
	}									\
										\
	int am_dfg_type_##TPREFIX##_from_string(const struct am_dfg_type* t,	\
						const char* str,		\
						void* out)			\
	{									\
		if(am_ato##AMPREFIX##_safe(str, out) != AM_ATO_SAFE_STATUS_VALID)\
			return 1;						\
										\
		return 0;							\
	}									\
										\
	int am_dfg_type_##TPREFIX##_check_string(const struct am_dfg_type* t,	\
						 const char* str)		\
	{									\
		T i;								\
										\
		if(am_ato##AMPREFIX##_safe(str, &i) != AM_ATO_SAFE_STATUS_VALID)\
			return 0;						\
										\
		return 1;							\
	}

AM_DFG_TYPE_UINT_IMPL( int8_t,  int8,  INT8,  i8, PRId8)
AM_DFG_TYPE_UINT_IMPL(int16_t, int16, INT16, i16, PRId16)
AM_DFG_TYPE_UINT_IMPL(int32_t, int32, INT32, i32, PRId32)
AM_DFG_TYPE_UINT_IMPL(int64_t, int64, INT64, i64, PRId64)

AM_DFG_TYPE_UINT_IMPL( uint8_t,  uint8,  UINT8,  u8, PRIu8)
AM_DFG_TYPE_UINT_IMPL(uint16_t, uint16, UINT16, u16, PRIu16)
AM_DFG_TYPE_UINT_IMPL(uint32_t, uint32, UINT32, u32, PRIu32)
AM_DFG_TYPE_UINT_IMPL(uint64_t, uint64, UINT64, u64, PRIu64)
