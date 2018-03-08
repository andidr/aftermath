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

#define AM_DFG_TYPE_UINT_IMPL(NBITS)						\
	int am_dfg_type_uint##NBITS##_to_string(const struct am_dfg_type* t,	\
						void* ptr,			\
						char** out,			\
						int* cst)			\
	{									\
		char* ret;							\
		uint##NBITS##_t* pintval = ptr;				\
										\
		if(!(ret = malloc(AM_UINT##NBITS##_T_MAX_DECIMAL_DIGITS + 1)))	\
			return 1;						\
										\
		snprintf(ret, AM_UINT##NBITS##_T_MAX_DECIMAL_DIGITS + 1,	\
			 "%" PRIu##NBITS, *pintval);				\
										\
		*out = ret;							\
		*cst = 0;							\
										\
		return 0;							\
	}									\
										\
	int am_dfg_type_uint##NBITS##_from_string(const struct am_dfg_type* t,	\
						  const char* str,		\
						  void* out)			\
	{									\
		if(am_atou##NBITS##_safe(str, out) != AM_ATOU_SAFE_STATUS_VALID)\
			return 1;						\
										\
		return 0;							\
	}									\
										\
	int am_dfg_type_uint##NBITS##_check_string(const struct am_dfg_type* t, \
						   const char* str)		\
	{									\
		uint##NBITS##_t i;						\
										\
		if(am_atou##NBITS##_safe(str, &i) != AM_ATOU_SAFE_STATUS_VALID) \
			return 0;						\
										\
		return 1;							\
	}

AM_DFG_TYPE_UINT_IMPL(8)
AM_DFG_TYPE_UINT_IMPL(16)
AM_DFG_TYPE_UINT_IMPL(32)
AM_DFG_TYPE_UINT_IMPL(64)
