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

#ifndef AM_QSELECT_H
#define AM_QSELECT_H

#include "ansi_extras.h"

/* Declare a function <prefix>qselect_nth_greatest<suffix> returning a pointer
 * to the n-th greatest element (in ascending order, i.e., there are n smaller
 * values) in an array whose element type is T. The comparison expression must
 * be an expression that returns 0 if the two arguments are equal, 1 if the
 * first argument is greater than the second and -1 if the first argument is
 * smaller than the second argument. Arguments to the expression are passed as
 * pointers (i.e., they are of type T* and not T). The last argument of the
 * generated function is a pointer named data of type P* that is passed verbatim
 * to all recursive invocations and that can thus be used by CMP_EXPR.
 *
 * Te return value of the function is a pointer to the first element that has
 * the same value as the n-th greatest.
 */
#define AM_DECL_QSELECT_NTH_GREATEST_SUFFIX_DATA_ARG(				\
	prefix, suffix, T, CMP_EXPR, P)					\
	static inline T* prefix##qselect_nth_greatest##suffix(			\
		T* a,								\
		size_t num_elements,						\
		size_t n,							\
		P* data)							\
	{									\
		static const size_t elem_size = sizeof(T);			\
		size_t initial_pivot_idx = num_elements / 2;			\
		T* ppivot_val;							\
		size_t lt;							\
		size_t gt;							\
		size_t i;							\
		int res;							\
										\
		if(n >= num_elements)						\
			return NULL;						\
										\
		if(num_elements < 2)						\
			return &a[0];						\
										\
		/* Make sure pivot element is at index 0 */			\
		if(initial_pivot_idx != 0)					\
			am_memswp(&a[0], &a[initial_pivot_idx], elem_size);	\
										\
		lt = 0;							\
		i = 0;								\
		gt = num_elements-1;						\
		ppivot_val = &a[0];						\
										\
		while(i <= gt) {						\
			res = CMP_EXPR(&a[i], ppivot_val);			\
										\
			if(res < 0) {						\
				am_memswp(&a[lt], &a[i], elem_size);		\
										\
				if(&a[lt] == ppivot_val)			\
					ppivot_val = &a[lt+1];			\
										\
				lt++;						\
				i++;						\
			} else if(res > 0) {					\
				am_memswp(&a[gt], &a[i], elem_size);		\
										\
				if(&a[gt] == ppivot_val)			\
					ppivot_val = &a[gt-1];			\
										\
				gt--;						\
										\
			} else {						\
				i++;						\
			}							\
		}								\
										\
		if(n < lt) {							\
			return prefix##qselect_nth_greatest##suffix(		\
				a, lt, n, data);				\
		} else if(n > gt) {						\
			return prefix##qselect_nth_greatest##suffix(		\
				a + gt + 1,					\
				num_elements - gt - 1,				\
				n - gt - 1,					\
				data);						\
		} else {							\
			/* Median is in the partition with equal values ->	\
			 * return directly, since these indexes will never	\
			 * change */						\
			return &a[lt];						\
		}								\
	}

/* Declare a function <prefix>qselect_nth_greatest<suffix> that returns a
 * pointer to the n-th greatest value (in ascending order, i.e., there are n
 * smaller values) of an array whose type is T. The comparison expression must
 * be an expression that returns 0 if the two arguments are equal, 1 if the
 * first argument is greater than the second and -1 if the first argument is
 * smaller than the second argument. Arguments to the expression are passed as
 * pointers (i.e., they are of type T* and not T). */
#define AM_DECL_QSELECT_NTH_GREATEST_SUFFIX(prefix, suffix, T, CMP_EXPR)	\
	AM_DECL_QSELECT_NTH_GREATEST_SUFFIX_DATA_ARG(				\
		prefix, suffix##_arg, T, CMP_EXPR, void)			\
										\
	static inline void prefix##qselect_nth_greatest##suffix(		\
		T* a, size_t num_elements, size_t n)				\
	{									\
		prefix##qselect_nth_greatest##suffix##_arg(			\
			a, num_elements, n, NULL);				\
	}

#endif
