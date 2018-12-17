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
#include <aftermath/core/dfg/types/generic.h>
#include <stdint.h>

#define AM_DFG_INT_TYPE_STATIC_DECL(TPREFIX)		\
	AM_DFG_DECL_BUILTIN_TYPE(			\
		am_dfg_type_##TPREFIX,			\
		"am::core::" #TPREFIX,			\
		sizeof(TPREFIX##_t),			\
		NULL,					\
		am_dfg_type_generic_plain_copy_samples, \
		am_dfg_type_##TPREFIX##_to_string,	\
		am_dfg_type_##TPREFIX##_from_string,	\
		am_dfg_type_##TPREFIX##_check_string)

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
						 const char* str);		\
										\
	AM_DFG_INT_TYPE_STATIC_DECL(TPREFIX)

AM_DFG_TYPE_INT_DEFINE(int8)
AM_DFG_TYPE_INT_DEFINE(int16)
AM_DFG_TYPE_INT_DEFINE(int32)
AM_DFG_TYPE_INT_DEFINE(int64)

AM_DFG_TYPE_INT_DEFINE(uint8)
AM_DFG_TYPE_INT_DEFINE(uint16)
AM_DFG_TYPE_INT_DEFINE(uint32)
AM_DFG_TYPE_INT_DEFINE(uint64)

AM_DFG_ADD_BUILTIN_TYPES(
	&am_dfg_type_int8,
	&am_dfg_type_int16,
	&am_dfg_type_int32,
	&am_dfg_type_int64,

	&am_dfg_type_uint8,
	&am_dfg_type_uint16,
	&am_dfg_type_uint32,
	&am_dfg_type_uint64
)

#endif
