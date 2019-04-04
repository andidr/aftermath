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

#include "histogram.h"
#include <aftermath/core/statistics/histogram.h>

#define AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(T, SUFFIX)		\
	void am_dfg_type_histogram1d_##SUFFIX##_free_samples(		\
		const struct am_dfg_type* t,				\
		size_t num_samples,					\
		void* ptr)						\
	{								\
		struct am_histogram1d_##SUFFIX** hs = ptr;		\
									\
		for(size_t i = 0; i < num_samples; i++) {		\
			am_histogram1d_##SUFFIX##_destroy(hs[i]);	\
			free(hs[i]);					\
		}							\
	}

AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(uint8_t, uint8)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(uint16_t, uint16)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(uint32_t, uint32)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(uint64_t, uint64)

AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(int8_t, int8)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(int16_t, int16)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(int32_t, int32)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(int64_t, int64)

AM_DFG_TYPE_HISTOGRAM1D_IMPL_FREE_SAMPLES(double, double)

#define AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(T, SUFFIX)			\
	int am_dfg_type_histogram1d_##SUFFIX##_copy_samples(			\
		const struct am_dfg_type* t,					\
		size_t num_samples,						\
		void* ptr_in,							\
		void* ptr_out)							\
	{									\
		struct am_histogram1d_##SUFFIX** hs_in = ptr_in;		\
		struct am_histogram1d_##SUFFIX** hs_out = ptr_out;		\
										\
		for(size_t i = 0; i < num_samples; i++) {			\
			if(!(hs_out[i] =					\
			     am_histogram1d_##SUFFIX##_clone(hs_in[i]))) {	\
				for(size_t j = 0; j < i; j++) {		\
					am_histogram1d_##SUFFIX##_destroy(hs_out[i]);\
					free(hs_out[i]);			\
				}						\
										\
				return 1;					\
			}							\
		}								\
										\
		return 0;							\
	}

AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(uint8_t, uint8)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(uint16_t, uint16)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(uint32_t, uint32)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(uint64_t, uint64)

AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(int8_t, int8)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(int16_t, int16)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(int32_t, int32)
AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(int64_t, int64)

AM_DFG_TYPE_HISTOGRAM1D_IMPL_COPY_SAMPLES(double, double)
