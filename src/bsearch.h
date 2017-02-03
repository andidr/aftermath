/**
 * Copyright (C) 2017 Andi Drebes <andi.drebes@lip6.fr>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef BSEARCH_H
#define BSEARCH_H

#include <sys/types.h>
#include <stdlib.h>

#define DECL_VSTRIDED_BSEARCH_SUFFIX(prefix, suffix, T, NEEDLE_T, ACC_EXPR)	\
	static inline T* prefix##bsearch_strided##suffix(T* a,			\
							size_t num_elements,	\
							off_t stride,		\
							const NEEDLE_T needle)	\
	{									\
		size_t l = 0;							\
		size_t r = num_elements-1;					\
		size_t m;							\
		NEEDLE_T curr;							\
		T* pcurr;							\
										\
		while(l <= r) {						\
			m = (l + r) / 2;					\
			pcurr = ((void*)a)+m*stride;				\
			curr = (ACC_EXPR((*pcurr)));				\
										\
			if(curr < needle) {					\
				if(m == num_elements-1)			\
					return NULL;				\
				else						\
					l = m + 1;				\
			} else if(curr > needle) {				\
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

#define DECL_VSTRIDED_BSEARCH(prefix, T, NEEDLE_T, ACC_EXPR) \
	DECL_VSTRIDED_BSEARCH_SUFFIX(prefix, , T, NEEDLE_T, ACC_EXPR)

#define DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(prefix, suffix, T, NEEDLE_T, ACC_EXPR, SMALLER_EXPR, GREATER_EXPR) \
	static inline T* prefix##bsearch_first_strided##suffix(T* a,		\
							       size_t num_elements, \
							       off_t stride,	\
							       const NEEDLE_T needle) \
	{									\
		size_t l = 0;							\
		size_t r = num_elements-1;					\
		size_t m;							\
		T* ret = NULL;							\
		NEEDLE_T curr;							\
		T* pcurr;							\
										\
		while(l <= r) {						\
			m = (l + r) / 2;					\
			pcurr = ((void*)a)+m*stride;				\
			curr = (ACC_EXPR((*pcurr)));				\
										\
			if(SMALLER_EXPR(curr, needle)) {			\
				if(m == num_elements-1)			\
					return ret;				\
				else						\
					l = m + 1;				\
			} else if(GREATER_EXPR(curr, needle)) {		\
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

#define DECL_VSTRIDED_BSEARCH_FIRST(prefix, suffix, T, NEEDLE_T, ACC_EXPR, SMALLER_EXPR, GREATER_EXPR) \
	DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(prefix, , T, NEEDLE_T, ACC_EXPR, SMALLER_EXPR, GREATER_EXPR) \

#endif
