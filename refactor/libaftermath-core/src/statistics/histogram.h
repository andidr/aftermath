/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AM_HISTOGRAM_H
#define AM_HISTOGRAM_H

#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include "../ansi_extras.h"
#include "../arithmetic.h"

struct am_histogram1d_data {
	size_t num_bins;
	uint64_t* bins;
};

int am_histogram1d_data_init(struct am_histogram1d_data* hd, size_t num_bins);
struct am_histogram1d_data*
am_histogram1d_data_clone(struct am_histogram1d_data* hd);
void am_histogram1d_data_destroy(struct am_histogram1d_data* hd);

enum am_histogram_bin_mode {
	/* Ingore samples which are not in the [min; max] range of the
	 * histogram */
	AM_HISTOGRAM_BIN_MODE_IGNORE,

	/* Saturated mode: if a value is lower than the minimum value it is
	 * added to the first bin, if it is greater than the maximum value, it
	 * is added to the last bin */
	AM_HISTOGRAM_BIN_MODE_SAT
};

/* Creates a histogram for integer values. Assumes that the maximum value of an
 * N-bit signed integer is exactly one less than minus its minimum value.
 *
 *  T: The data type for samples (e.g., int64_t)
 *  SUFFIX: Suffix for naming
 *  SIGNED: Indicates whether T is a signed type (1) or not (0)
 *  UT: Unsigned integer type with the same number of bits as T
 *
 */
#define AM_DECL_INT_HISTOGRAM_1D(T, BITS, SUFFIX, SIGNED, UT)		\
	struct am_histogram1d_##SUFFIX {					\
		struct am_histogram1d_data data;				\
		enum am_histogram_bin_mode mode;				\
		T left;							\
		T right;							\
		UT range;							\
	};									\
										\
	/* Sets the left, right and range value of h. Returns 0 on success, 1	\
	 * otherwise (e.g., if the range cannot be represented by the unsigned	\
	 * type).								\
	 */									\
	static inline int							\
	am_histogram1d_##SUFFIX##_set_range(struct am_histogram1d_##SUFFIX* h,	\
					    T left,				\
					    T right)				\
	{									\
		if(left > right)						\
			return 1;						\
										\
		if(SIGNED) {							\
			if(am_safe_closed_interval_size_i##BITS(&h->range,	\
								left,		\
								right))	\
			{							\
				return 1;					\
			}							\
		} else {							\
			if(am_safe_closed_interval_size_u##BITS(&h->range,	\
								left,		\
								right))	\
			{							\
				return 1;					\
			}							\
		}								\
										\
		h->left = left;						\
		h->right = right;						\
										\
		return 0;							\
	}									\
										\
	static inline int							\
	am_histogram1d_##SUFFIX##_init(struct am_histogram1d_##SUFFIX* h,	\
					  size_t num_bins,			\
					  T left,				\
					  T right,				\
					  enum am_histogram_bin_mode mode)	\
	{									\
		if(am_histogram1d_##SUFFIX##_set_range(h, left, right))	\
			return 1;						\
										\
		if(am_histogram1d_data_init(&h->data, num_bins))		\
			return 1;						\
										\
		h->mode = mode;						\
										\
		return 0;							\
	}									\
										\
	static inline void							\
	am_histogram1d_##SUFFIX##_destroy(					\
		struct am_histogram1d_##SUFFIX* h)				\
	{									\
		am_histogram1d_data_destroy(&h->data);				\
	}									\
										\
	/* Adds a sample s to the histogram, i.e., increments the count for the \
	 * corresponding bin by 1. Returns 0 on success, otherwise 1. */	\
	static inline int							\
	am_histogram1d_##SUFFIX##_add_sample(struct am_histogram1d_##SUFFIX* h, \
					     T s)				\
	{									\
		/* Value of s - h->left */					\
		UT s_shifted_u = 0;						\
										\
		/* Final bin number as a UT */					\
		UT utbin;							\
										\
		/* Final bin number */						\
		size_t bin;							\
										\
		/* Treat cases where the sample is outside of the histogram's	\
		 * range */							\
		if(s < h->left || s > h->right) {				\
			if(h->mode == AM_HISTOGRAM_BIN_MODE_IGNORE) {		\
				return 0;					\
			} else if(h->mode == AM_HISTOGRAM_BIN_MODE_SAT) {	\
				bin = (s < h->left) ? 0 : h->data.num_bins - 1; \
				goto assign;					\
			}							\
		}								\
										\
		/* Cannot fail, since left < right && [left; s) representable	\
		 * (since h->range = right - left is representable) */		\
		if(SIGNED) {							\
			am_safe_rightopen_interval_size_i##BITS(&s_shifted_u,	\
								h->left,	\
								s);		\
		} else {							\
			am_safe_rightopen_interval_size_u##BITS(&s_shifted_u,	\
								h->left,	\
								s);		\
		}								\
										\
		if(sizeof(size_t) <= sizeof(UT)) {				\
			/* The result is guaranteed to be in			\
			 * [0; h->data.num_bins). Since h->num_bins is a size_t,\
			 * this is guaranteed to be representable by a size_t.	\
			 * Since sizeof(size_t) <= sizeof(UT), the muldiv	\
			 * operation cannot produce a result that cannot be	\
			 * represented by a UT. */				\
			am_muldiv_u##BITS(s_shifted_u, (UT)h->data.num_bins,	\
					  h->range, &utbin);			\
										\
			bin = (size_t)utbin;					\
		} else {							\
			/* Only using size_t with sizeof(size_t) > sizeof(UT),	\
			 * so there is nothing that cannot be represented by a	\
			 * size_t. */						\
			AM_MACRO_ARG_EXPAND_JOIN(am_muldiv_u, AM_SIZE_BITS)(	\
				(size_t)s_shifted_u,				\
				h->data.num_bins,				\
				(size_t)h->range, &bin);			\
		}								\
										\
	assign:								\
		if(am_add_sat_u64(h->data.bins[bin], 1, &h->data.bins[bin]) !=	\
		   AM_ARITHMETIC_STATUS_EXACT)					\
		{								\
			return 1;						\
		}								\
										\
		return 0;							\
	}

AM_DECL_INT_HISTOGRAM_1D( int8_t,  8,  int8, 1,  uint8_t)
AM_DECL_INT_HISTOGRAM_1D(int16_t, 16, int16, 1, uint16_t)
AM_DECL_INT_HISTOGRAM_1D(int32_t, 32, int32, 1, uint32_t)
AM_DECL_INT_HISTOGRAM_1D(int64_t, 64, int64, 1, uint64_t)

AM_DECL_INT_HISTOGRAM_1D( uint8_t,  8,  uint8, 1,  uint8_t)
AM_DECL_INT_HISTOGRAM_1D(uint16_t, 16, uint16, 1, uint16_t)
AM_DECL_INT_HISTOGRAM_1D(uint32_t, 32, uint32, 1, uint32_t)
AM_DECL_INT_HISTOGRAM_1D(uint64_t, 64, uint64, 1, uint64_t)

struct am_histogram1d_double {
	struct am_histogram1d_data data;
	enum am_histogram_bin_mode mode;
	double left;
	double right;
	double range;
};

/* Sets the left, right and range value of h. Returns 0 on success, 1 otherwise
 * (e.g., if the range cannot be represented).
 */
static inline int
am_histogram1d_double_set_range(struct am_histogram1d_double* h,
				double left,
				double right)
{
	if(left > right)
		return 1;

	/* Final + 1.0 would overflow */
	if(right - left == DBL_MAX)
		return 1;

	h->left = left;
	h->right = right;
	h->range = right - left + 1.0;

	return 0;
}

static inline int
am_histogram1d_double_init(struct am_histogram1d_double* h,
			   size_t num_bins,
			   double left,
			   double right,
			   enum am_histogram_bin_mode mode)
{
	if(am_histogram1d_double_set_range(h, left, right))
		return 1;

	if(am_histogram1d_data_init(&h->data, num_bins))
		return 1;

	h->mode = mode;

	return 0;
}

static inline void am_histogram1d_double_destroy(struct am_histogram1d_double* h)
{
	am_histogram1d_data_destroy(&h->data);
}

/* Adds a sample s to a 1D double histogram, i.e., increments the count for the
 * corresponding bin by 1. Returns 0 on success, otherwise 1.
 */
static inline int
am_histogram1d_double_add_sample(struct am_histogram1d_double* h, double s)
{
	/* Final bin number as a double */
	double dblbin;

	/* Final bin number */
	size_t bin;

	/* Treat cases where the sample is outside of the histogram's
	 * range */
	if(s < h->left || s > h->right) {
		if(h->mode == AM_HISTOGRAM_BIN_MODE_IGNORE) {
			return 0;
		} else if(h->mode == AM_HISTOGRAM_BIN_MODE_SAT) {
			bin = (s < h->left) ? 0 : h->data.num_bins - 1;
			goto assign;
		}
	}

	dblbin = ((s - h->left) / h->range) * ((double)h->data.num_bins);
	bin = (size_t)dblbin;

	/* Catch rounding errors */
	if(bin >= h->data.num_bins)
		bin = h->data.num_bins - 1;

assign:
	if(am_add_sat_u64(h->data.bins[bin], 1, &h->data.bins[bin]) !=
	   AM_ARITHMETIC_STATUS_EXACT)
	{
		return 1;
	}

	return 0;
}


#endif
