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

#define AM_DFG_TYPE_UINT_DEFINE(NBITS)						\
	int am_dfg_type_uint##NBITS##_to_string(const struct am_dfg_type* t,	\
						void* ptr,			\
						char** out,			\
						int* cst);			\
										\
	int am_dfg_type_uint##NBITS##_from_string(const struct am_dfg_type* t,	\
						  const char* str,		\
						  void* out);			\
										\
	int am_dfg_type_uint##NBITS##_check_string(const struct am_dfg_type* t, \
						   const char* str);

#define AM_DFG_TYPE_UINT8_SAMPLE_SIZE sizeof(uint8_t)
#define AM_DFG_TYPE_UINT16_SAMPLE_SIZE sizeof(uint16_t)
#define AM_DFG_TYPE_UINT32_SAMPLE_SIZE sizeof(uint32_t)
#define AM_DFG_TYPE_UINT64_SAMPLE_SIZE sizeof(uint64_t)

AM_DFG_TYPE_UINT_DEFINE(8)
AM_DFG_TYPE_UINT_DEFINE(16)
AM_DFG_TYPE_UINT_DEFINE(32)
AM_DFG_TYPE_UINT_DEFINE(64)

#endif
