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

#ifndef AM_BSEARCH_H
#define AM_BSEARCH_H

#include <sys/types.h>
#include <stdlib.h>
#include <aftermath/core/ptr.h>

/* Compares two expressions of primitive values a and b. Evaluates to -1 if a <
 * b, to 1 if a > b, otherwise to 0.
 */
#define AM_VALCMP_EXPR(a, b)				\
	({						\
		typeof(a) _a = (a);			\
		typeof(b) _b = (b);			\
							\
		_a > _b ? 1 : (_a < _b ? -1 : 0);	\
	})

/* Compares two pointers as AM_VALCMP_EXPR, but after casting them to
 * uintptr_t */
#define AM_VALCMP_PTR(a, b) \
	AM_VALCMP_EXPR((uintptr_t)(a), (uintptr_t)(b))

/* Calculates (a + b) / 2 without overflows */
static inline size_t am_bsearch_safe_center_idx(size_t a, size_t b)
{
	return (a / 2) + (b / 2) + ((a % 2) + (b % 2)) / 2;
}

/* Declare a binary search function named <prefix>bsearch_strided<suffix> for an
 * array whose elements are of the type T. The search key of the function is of
 * type NEEDLE_T and the value used for comparison is extracted from an array
 * element using ACC_EXPR. The generated function returns a pointer to the found
 * array element or NULL if no such element exists. CMP_EXPR is an expression
 * that returns 0 if the two arguments are equal, 1 if the first argument is
 * greater than the second and -1 if the first argument is smaller than the
 * second argument.*/
#define AM_DECL_VSTRIDED_BSEARCH_SUFFIX(prefix, suffix, T, NEEDLE_T, ACC_EXPR,	\
					CMP_EXPR)				\
	static inline T* prefix##bsearch_strided##suffix(T* a,			\
							size_t num_elements,	\
							off_t stride,		\
							const NEEDLE_T needle)	\
	{									\
		size_t l = 0;							\
		size_t r;							\
		size_t m;							\
		T* pcurr;							\
		int cmpres;							\
										\
		if(num_elements == 0)						\
			return NULL;						\
										\
		r = num_elements-1;						\
										\
		while(l <= r) {						\
			m = am_bsearch_safe_center_idx(l, r);			\
			pcurr = ((void*)a)+m*stride;				\
			const NEEDLE_T curr = (ACC_EXPR((*pcurr)));		\
			cmpres = CMP_EXPR(curr, needle);			\
										\
			if(cmpres < 0) {					\
				if(m == num_elements-1)			\
					return NULL;				\
				else						\
					l = m + 1;				\
			} else if(cmpres > 0) {				\
				if(m == 0)					\
					return NULL;				\
				else						\
					r = m - 1;				\
			} else {						\
				return pcurr;					\
			}							\
		}								\
										\
		return NULL;							\
	}

/* Declare a binary search function named <prefix>bsearch_strided. Besides the
 * name, the generated function is identical with a function generated using
 * DECL_VSTRIDED_BSEARCH_SUFFIX. */
#define AM_DECL_VSTRIDED_BSEARCH(prefix, T, NEEDLE_T, ACC_EXPR, CMP_EXPR) \
	AM_DECL_VSTRIDED_BSEARCH_SUFFIX(prefix, , T, NEEDLE_T, ACC_EXPR, CMP_EXPR)

/* Same as DECL_VSTRIDED_BSEARCH_SUFFIX, but the generated function allows for
 * searches on arrays with duplicate elements and has the name
 * <prefix>bsearch_first_strided<suffix>. Upon a successful search, the
 * generated function returns a pointer to the first of the duplicates. The
 * macro GREATER_EXPR is used for comparison of array elements. CMP_EXPR is an
 * expression that returns 0 if the two arguments are equal, 1 if the first
 * argument is greater than the second and -1 if the first argument is smaller
 * than the second argument. */
#define AM_DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(prefix, suffix, T, NEEDLE_T,	\
					      ACC_EXPR,			\
					      CMP_EXPR)			\
	static inline T* prefix##bsearch_first_strided##suffix(T* a,		\
							       size_t num_elements, \
							       off_t stride,	\
							       const NEEDLE_T needle) \
	{									\
		size_t l = 0;							\
		size_t r;							\
		size_t m;							\
		T* ret = NULL;							\
		T* pcurr;							\
		int cmpres;							\
										\
		if(num_elements == 0)						\
			return NULL;						\
										\
		r = num_elements-1;						\
										\
		while(l <= r) {						\
			m = am_bsearch_safe_center_idx(l, r);			\
			pcurr = AM_PTR_ADD(a, m*stride);			\
			const NEEDLE_T curr = (ACC_EXPR((*pcurr)));		\
			cmpres = CMP_EXPR(curr, needle);			\
										\
			if(cmpres < 0) {					\
				if(m == num_elements-1)			\
					return ret;				\
				else						\
					l = m + 1;				\
			} else if(cmpres > 0) {				\
				if(m == 0)					\
					return ret;				\
				else						\
					r = m - 1;				\
			} else {						\
				ret = pcurr;					\
										\
				if(m == 0)					\
					return ret;				\
				else						\
					r = m - 1;				\
			}							\
		}								\
										\
		return ret;							\
	}

/* Declare a binary search function named <prefix>bsearch_first_strided. Besides
 * the name, the generated function is identical with a function generated using
 * DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX. */
#define AM_DECL_VSTRIDED_BSEARCH_FIRST(prefix, T, NEEDLE_T, ACC_EXPR,CMP_EXPR)	\
	AM_DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(prefix, , T, NEEDLE_T, ACC_EXPR,	\
					      CMP_EXPR)

/* Same as DECL_VSTRIDED_BSEARCH_SUFFIX, but the generated function allows for
 * searches on arrays with duplicate elements and has the name
 * <prefix>bsearch_last_strided<suffix>. Upon a successful search, the generated
 * function returns a pointer to the last of the duplicates. The macro
 * GREATER_EXPR is used for comparison of array elements. CMP_EXPR is an
 * expression that returns 0 if the two arguments are equal, 1 if the first
 * argument is greater than the second and -1 if the first argument is smaller
 * than the second argument. */
#define AM_DECL_VSTRIDED_BSEARCH_LAST_SUFFIX(prefix, suffix, T, NEEDLE_T,	\
					     ACC_EXPR,				\
					     CMP_EXPR)				\
	static inline T* prefix##bsearch_last_strided##suffix(T* a,		\
							      size_t num_elements, \
							      off_t stride, \
							      const NEEDLE_T needle) \
	{									\
		size_t l = 0;							\
		size_t r;							\
		size_t m;							\
		T* ret = NULL;							\
		T* pcurr;							\
		int cmpres;							\
										\
		if(num_elements == 0)						\
			return NULL;						\
										\
		r = num_elements-1;						\
										\
		while(l <= r) {						\
			m = am_bsearch_safe_center_idx(l, r);			\
			pcurr = AM_PTR_ADD(a, m*stride);			\
			const NEEDLE_T curr = (ACC_EXPR((*pcurr)));		\
			cmpres = CMP_EXPR(curr, needle);			\
										\
			if(cmpres < 0) {					\
				if(m == num_elements-1)			\
					return ret;				\
				else						\
					l = m + 1;				\
			} else if(cmpres > 0) {				\
				if(m == 0)					\
					return ret;				\
				else						\
					r = m - 1;				\
			} else {						\
				ret = pcurr;					\
										\
				if(m == num_elements-1)			\
					return ret;				\
				else						\
					l = m + 1;				\
			}							\
		}								\
										\
		return ret;							\
	}

/* Declare a binary search function named <prefix>bsearch_last_strided. Besides
 * the name, the generated function is identical with a function generated using
 * DECL_VSTRIDED_BSEARCH_LAST_SUFFIX. */
#define AM_DECL_VSTRIDED_BSEARCH_LAST(prefix, T, NEEDLE_T, ACC_EXPR,		\
				      SMALLER_EXPR, GREATER_EXPR)		\
	AM_DECL_VSTRIDED_BSEARCH_LAST_SUFFIX(prefix, , T, NEEDLE_T, ACC_EXPR,	\
					     CMP_EXPR)

#endif
