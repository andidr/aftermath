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

#ifndef AM_DFG_TYPE_UINT_H
#define AM_DFG_TYPE_UINT_H

#include <aftermath/core/dfg_type.h>
#include <stdint.h>

#define AM_DFG_TYPE_INT_DEFINE(TPREFIX)					\
	int am_dfg_type_##TPREFIX##_to_string(const struct am_dfg_type* t,	\
					      void* ptr,			\
					      char** out,			\
					      int* cst);			\
										\
	int am_dfg_type_##TPREFIX##_from_string(const struct am_dfg_type* t,	\
						const char* str,		\
						void* out);			\
										\
	int am_dfg_type_##TPREFIX##_check_string(const struct am_dfg_type* t,	\
						 const char* str);

#define AM_DFG_TYPE_INT8_SAMPLE_SIZE sizeof(int8_t)
#define AM_DFG_TYPE_INT16_SAMPLE_SIZE sizeof(int16_t)
#define AM_DFG_TYPE_INT32_SAMPLE_SIZE sizeof(int32_t)
#define AM_DFG_TYPE_INT64_SAMPLE_SIZE sizeof(int64_t)

#define AM_DFG_TYPE_UINT8_SAMPLE_SIZE sizeof(uint8_t)
#define AM_DFG_TYPE_UINT16_SAMPLE_SIZE sizeof(uint16_t)
#define AM_DFG_TYPE_UINT32_SAMPLE_SIZE sizeof(uint32_t)
#define AM_DFG_TYPE_UINT64_SAMPLE_SIZE sizeof(uint64_t)

AM_DFG_TYPE_INT_DEFINE(int8)
AM_DFG_TYPE_INT_DEFINE(int16)
AM_DFG_TYPE_INT_DEFINE(int32)
AM_DFG_TYPE_INT_DEFINE(int64)

AM_DFG_TYPE_INT_DEFINE(uint8)
AM_DFG_TYPE_INT_DEFINE(uint16)
AM_DFG_TYPE_INT_DEFINE(uint32)
AM_DFG_TYPE_INT_DEFINE(uint64)

#endif
