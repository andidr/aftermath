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

#ifndef AM_DFG_TYPE_HISTOGRAM_H
#define AM_DFG_TYPE_HISTOGRAM_H

#include <aftermath/core/dfg_type.h>
#include <aftermath/core/dfg/types/generic.h>

struct am_histogram1d_uint8;
struct am_histogram1d_uint16;
struct am_histogram1d_uint32;
struct am_histogram1d_uint64;

struct am_histogram1d_int8;
struct am_histogram1d_int16;
struct am_histogram1d_int32;
struct am_histogram1d_int64;

struct am_histogram1d_double;

#define AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(SUFFIX)		\
	void am_dfg_type_histogram1d_##SUFFIX##_free_samples(		\
		const struct am_dfg_type* t,				\
		size_t num_samples,					\
		void* ptr);

AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(uint8)
AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(uint16)
AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(uint32)
AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(uint64)

AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(int8)
AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(int16)
AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(int32)
AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(int64)

AM_DFG_TYPE_HISTOGRAM1D_DECL_FREE_SAMPLES(double)

#define AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(SUFFIX, SUFFIX_UPCASE)	\
	AM_DFG_DECL_BUILTIN_TYPE(					\
		am_dfg_type_histogram1d_##SUFFIX,			\
		"am::core::histogram1d<" #SUFFIX ">",			\
		sizeof(struct am_histogram1d_##SUFFIX*),		\
		am_dfg_type_histogram1d_##SUFFIX##_free_samples,	\
		NULL,							\
		NULL,							\
		NULL)

AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint8, UINT8)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint16, UINT16)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint32, UINT32)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(uint64, UINT64)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int8, INT8)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int16, INT16)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int32, INT32)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(int64, INT64)
AM_DFG_HISTOGRAM_1D_STATIC_TYPE_DECL(double, DOUBLE)

AM_DFG_ADD_BUILTIN_TYPES(
	&am_dfg_type_histogram1d_uint8,
	&am_dfg_type_histogram1d_uint16,
	&am_dfg_type_histogram1d_uint32,
	&am_dfg_type_histogram1d_uint64,
	&am_dfg_type_histogram1d_int8,
	&am_dfg_type_histogram1d_int16,
	&am_dfg_type_histogram1d_int32,
	&am_dfg_type_histogram1d_int64,
	&am_dfg_type_histogram1d_double
)

#endif
