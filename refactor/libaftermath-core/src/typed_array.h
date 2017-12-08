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

#ifndef AM_TYPED_ARRAY_H
#define AM_TYPED_ARRAY_H

#include <aftermath/core/buffer.h>
#include <stdint.h>

/* Default number of elements that is preallocated when space is insufficient */
#define AM_DEFAULT_TYPED_ARRAY_PREALLOC 32

/* Generic structure representing the metadata of a typed array. Each typed
 * array can be cast into a struct am_typed_array_generic. Useful for generic
 * code that processes a typed array without knowing the exact element type.
 */
struct am_typed_array_generic {
	size_t num_elements;
	size_t num_free;
	size_t num_prealloc;
	void* elements;
};

/* Casts an arbitrary typed array into a generic typed array */
#define AM_TYPED_ARRAY_GENERIC(parr) ((struct am_typed_array_generic*)parr)

/* Declare a new type of arrays. The macro parameter 'prefix' defines the prefix
 * for the type of the structure as well as for all operations. */
#define AM_DECL_TYPED_ARRAY_EXTRA_FIELDS(prefix, T, extra_fields)	\
	struct prefix {						\
		size_t num_elements;					\
		size_t num_free;					\
		size_t num_prealloc;					\
		T* elements;						\
		extra_fields						\
	};								\
									\
	typedef T prefix##_element_type;				\
									\
	static inline int prefix##_reserve_pos(struct prefix* a,	\
					       size_t pos);		\
	static inline int prefix##_reserve_end(struct prefix* a);	\
	static inline int prefix##_appendp(struct prefix* a,		\
					   T* e);			\
	static inline int prefix##_append(struct prefix* a,		\
					  T e);			\
	static inline int prefix##_insertp(struct prefix* a,		\
					   size_t pos, T* e);		\
	static inline int prefix##_insert(struct prefix* a,		\
					  size_t pos, T e);		\
	static inline int prefix##_prealloc_n(struct prefix* a,	\
					      size_t n);		\
	static inline int prefix##_prealloc(struct prefix* a);		\
	static inline void prefix##_init(struct prefix* a);		\
	static inline void prefix##_destroy(struct prefix* a);		\
	static inline size_t prefix##_index(const struct prefix* a,	\
					    const T* e);		\
									\
	static inline int prefix##_is_element_ptr(const struct prefix* a, \
						  T* e);		\
									\
	/* Checks if e is a pointer to an element of the array. */	\
	static inline int prefix##_is_element_ptr(const struct prefix* a, \
						  T* e)		\
	{								\
		return ((uintptr_t)e) >= ((uintptr_t)a->elements) &&	\
			((uintptr_t)e) < ((uintptr_t)&a->elements[a->num_elements]); \
	}								\
									\
	static inline size_t prefix##_index(const struct prefix* a,	\
					    const T* e)		\
	{								\
		return (((uintptr_t)e) - ((uintptr_t)a->elements)) /	\
			sizeof(a->elements[0]);			\
	}								\
									\
	/* Reserves space for at least one element at position p. */	\
	/* Returns 0 on success, otherwise 1. */			\
	static inline int prefix##_reserve_pos(struct prefix* a,	\
					       size_t pos)		\
	{								\
		if(pos > a->num_elements)				\
			return 1;					\
									\
		if(prefix##_prealloc(a))				\
			return 1;					\
									\
		if(pos < a->num_elements) {				\
			memmove(&a->elements[pos+1],			\
				&a->elements[pos],			\
				(a->num_elements-pos)*sizeof(T));	\
		}							\
									\
		a->num_free--;						\
		a->num_elements++;					\
									\
		return 0;						\
	}								\
									\
	/* Reserves space for at least one element at the end. */	\
	/* Returns 0 on success, otherwise 1. */			\
	static inline int prefix##_reserve_end(struct prefix* a)	\
	{								\
		return prefix##_reserve_pos(a, a->num_elements);	\
	}								\
									\
	/* Add an element by reference to the array. Returns 0 on */	\
	/* success, otherwise 1. */					\
	static inline int prefix##_appendp(struct prefix* a, T* e)	\
	{								\
		return am_add_buffer_grow((void**)&a->elements,	\
					  e,				\
					  sizeof(T),			\
					  &a->num_elements,		\
					  &a->num_free,		\
					  a->num_prealloc);		\
	}								\
									\
	/* Add an element by value to the array. Returns 0 on */	\
	/* success, otherwise 1. */					\
	static inline int prefix##_append(struct prefix* a, T e)	\
	{								\
		if(am_check_buffer_grow((void**)&a->elements,		\
					sizeof(T),			\
					a->num_elements,		\
					&a->num_free,			\
					a->num_prealloc))		\
		{							\
			return 1;					\
		}							\
									\
		a->elements[a->num_elements] = e;			\
		a->num_elements++;					\
		a->num_free--;						\
									\
		return 0;						\
	}								\
									\
	/* Insert an element by address into the array at position p. */\
	/* Returns 0 on success, otherwise 1. */			\
	static inline int prefix##_insertp(struct prefix* a,		\
					   size_t pos, T* e)		\
	{								\
		if(prefix##_reserve_pos(a, pos))			\
			return 1;					\
									\
		a->elements[pos] = *e;					\
									\
		return 0;						\
	}								\
									\
	/* Remove the element at position p from the array. */		\
	static inline void prefix##_remove(struct prefix* a,		\
					   size_t pos)			\
	{								\
		if(pos >= a->num_elements)				\
			return;					\
									\
		if(pos+1 < a->num_elements) {				\
			memmove(&a->elements[pos],			\
				&a->elements[pos+1],			\
				(a->num_elements - pos - 1) * sizeof(T)); \
		}							\
									\
		a->num_free++;						\
		a->num_elements--;					\
	}								\
									\
	/* Remove the element pointed to by e from the array. */	\
	static inline void prefix##_removep(struct prefix* a, T* e)	\
	{								\
		size_t pos;						\
									\
		if(!prefix##_is_element_ptr(a, e))			\
			return;					\
									\
		pos = (((uintptr_t)e) - ((uintptr_t)a->elements)) / sizeof(T);\
									\
		prefix##_remove(a, pos);				\
	}								\
									\
	/* Insert an element by value into the array at position p. */	\
	/* Returns 0 on success, otherwise 1. */			\
	static inline int prefix##_insert(struct prefix* a,		\
					  size_t pos, T e)		\
	{								\
		return prefix##_insertp(a, pos, &e);			\
	}								\
									\
	/* Pre-allocates space for at least n new elements. Returns 0 */\
	/* on success, otherwise 1. */					\
	static inline int prefix##_prealloc_n(struct prefix* a,	\
					      size_t n)		\
	{								\
		return am_check_buffer_grow_n((void**)&a->elements,	\
					      sizeof(T),		\
					      a->num_elements,		\
					      &a->num_free,		\
					      n,			\
					      a->num_prealloc);	\
	}								\
									\
	/* Pre-allocates space for a number of new elements defined */	\
	/* a->num_prealloc. Returns 0 on success, otherwise 1. */	\
	static inline int prefix##_prealloc(struct prefix* a)		\
	{								\
		return prefix##_prealloc_n(a, a->num_prealloc);	\
	}								\
									\
	/* Initialize an array. The array will be empty without any */	\
	/* space pre-allocated. */					\
	static inline void prefix##_init(struct prefix* a)		\
	{								\
		a->num_elements = 0;					\
		a->num_free = 0;					\
		a->num_prealloc = AM_DEFAULT_TYPED_ARRAY_PREALLOC;	\
		a->elements = NULL;					\
	}								\
									\
	/* Destroys the array. Note that this function does not call */ \
	/* any destructor on the elements. */				\
	static inline void prefix##_destroy(struct prefix* a) \
	{								\
		free(a->elements);					\
	}

#define AM_DECL_TYPED_ARRAY(prefix, T) \
	AM_DECL_TYPED_ARRAY_EXTRA_FIELDS(prefix, T, )

/* Declare a binary seach function for a typed array. prefix and T must be the
 * same values as those used in DECL_TYPED_ARRAY. suffix is simply appended to
 * the name of the declared function, without a leading underscore. NEEDLE_T is
 * the type of the search value passed to the search function. ACC_EXPR must be
 * a macro that takes one argument of type T and that returns the value that is
 * compared to the search value. */
#define AM_DECL_TYPED_ARRAY_BSEARCH_SUFFIX(prefix, suffix, T, NEEDLE_T, ACC_EXPR)\
	/* Finds the address of the element whose value is needle or NULL if */ \
	/* no such element exists. */						\
	static inline T* prefix##_bsearch##suffix(const struct prefix* a,	\
						  const NEEDLE_T needle)	\
	{									\
		size_t l = 0;							\
		size_t r;							\
		size_t m;							\
		NEEDLE_T curr;							\
										\
		if(a->num_elements == 0)					\
			return NULL;						\
										\
		r = a->num_elements-1;						\
										\
		while(l <= r) {						\
			m = (l + r) / 2;					\
			curr = (ACC_EXPR(a->elements[m]));			\
										\
			if(curr < needle) {					\
				if(m == a->num_elements-1)			\
					return NULL;				\
				else						\
					l = m + 1;				\
			} else if(curr > needle) {				\
				if(m == 0)					\
					return NULL;				\
				else						\
					r = m - 1;				\
			} else {						\
				return &a->elements[m];			\
			}							\
		}								\
										\
		return NULL;							\
	}									\

#define AM_DECL_TYPED_ARRAY_BSEARCH(prefix, T, NEEDLE_T, ACC_EXPR) \
	AM_DECL_TYPED_ARRAY_BSEARCH_SUFFIX(prefix, , T, NEEDLE_T, ACC_EXPR)

/* Declare a binary seach function for a typed array with duplicate
 * values. prefix and T must be the same values as those used in
 * DECL_TYPED_ARRAY. suffix is simply appended to the name of the declared
 * function, without a leading underscore. NEEDLE_T is the type of the search
 * value passed to the search function. ACC_EXPR must be a macro that takes one
 * argument of type T and that returns the value that is compared to the search
 * value. SMALLER_EXPR and GREATER_EXPR must be macros with two parameters that
 * return true if the first parameter is smaller / greater than the second
 * parameter. */
#define AM_DECL_TYPED_ARRAY_BSEARCH_FIRST_SUFFIX(prefix, suffix, T, NEEDLE_T,	\
					      ACC_EXPR, SMALLER_EXPR, GREATER_EXPR) \
	/* Finds the address of the first element whose value is needle or NULL */ \
	/* if no such element exists. */					\
	static inline T* prefix##_bsearch_first##suffix(const struct prefix* a, \
							const NEEDLE_T needle)	\
	{									\
		size_t l = 0;							\
		size_t r;							\
		size_t m;							\
		T* ret = NULL;							\
		NEEDLE_T curr;							\
										\
		if(a->num_elements == 0)					\
			return NULL;						\
										\
		r = a->num_elements-1;						\
										\
		while(l <= r) {						\
			m = (l + r) / 2;					\
			curr = (ACC_EXPR(a->elements[m]));			\
										\
			if(SMALLER_EXPR(curr, needle)) {			\
				if(m == a->num_elements-1)			\
					return ret;				\
				else						\
					l = m + 1;				\
			} else if(GREATER_EXPR(curr, needle)) {		\
				if(m == 0)					\
					return ret;				\
				else						\
					r = m - 1;				\
			} else {						\
				ret = &a->elements[m];				\
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

#define AM_DECL_TYPED_ARRAY_BSEARCH_FIRST(prefix, T, NEEDLE_T, ACC_EXPR,	\
				       SMALLER_EXPR, GREATER_EXPR)		\
	AM_DECL_TYPED_ARRAY_BSEARCH_FIRST_SUFFIX(prefix, , T, NEEDLE_T, ACC_EXPR,\
					      SMALLER_EXPR, GREATER_EXPR)

/* Declare a binary remove function for a typed array that removes the element
 * that matches a key. Requires a binary search function declared via
 * DECL_TYPED_ARRAY_BSEARCH_SUFFIX. prefix and T must be the same values as
 * those used in DECL_TYPED_ARRAY. suffix is simply appended to the name of the
 * declared function, without a leading underscore. NEEDLE_T is the type of the
 * search value passed to the search function. */
#define AM_DECL_TYPED_ARRAY_REMOVE_SUFFIX(prefix, suffix, T, NEEDLE_T) \
	/* Removes the the element whose value is needle. If no such element	\
	 * exists, nothing happens. */						\
	static inline void prefix##_remove_sorted##suffix(struct prefix* a,	\
							  const NEEDLE_T needle)\
	{									\
		T* e;								\
										\
		if((e = prefix##_bsearch##suffix(a, needle)))			\
			prefix##_removep##suffix(a, e);			\
	}

#define AM_DECL_TYPED_ARRAY_REMOVE(prefix, T, NEEDLE_T) \
	AM_DECL_TYPED_ARRAY_REMOVE_SUFFIX(prefix, , T, NEEDLE_T)

/* Declare a function for a typed array that returns the position at which a new
 * value specified by needle would have to be inserted. prefix and T must be the
 * same values as those used in DECL_TYPED_ARRAY. suffix is simply appended to
 * the name of the declared function, without a leading underscore. NEEDLE_T is
 * the type of the search value passed to the search function. ACC_EXPR must be
 * a macro that takes one argument of type T and that returns the value that is
 * compared to the needle. */
#define AM_DECL_TYPED_ARRAY_INSERTPOS_SUFFIX(prefix, suffix, T, NEEDLE_T, ACC_EXPR)\
	/* Finds the insertion position of the element whose value is needle. */\
	static inline size_t prefix##_insertpos##suffix(struct prefix* a,	\
							const NEEDLE_T needle)	\
	{									\
		size_t l = 0;							\
		size_t r = a->num_elements;					\
		size_t m;							\
		NEEDLE_T curr;							\
										\
		while(l < r) {							\
			m = (l + r) / 2;					\
			curr = (ACC_EXPR(a->elements[m]));			\
										\
			if(curr > needle)					\
				r = m;						\
			else							\
				l = m + 1;					\
		}								\
										\
		return l;							\
	}

#define AM_DECL_TYPED_ARRAY_INSERTPOS(prefix, T, NEEDLE_T, ACC_EXPR) \
	AM_DECL_TYPED_ARRAY_INSERTPOS_SUFFIX(prefix, , T, NEEDLE_T, ACC_EXPR)


/* Declare a function that inserts a new element into the array at the correct
 * position assuming that the array is already sorted. Requires a binary search
 * function to determine the position for insertion. prefix and T must be the
 * same values as those used in DECL_TYPED_ARRAY. suffix is simply appended to
 * the name of the declared function, without a leading underscore. NEEDLE_T is
 * the type of the search value passed to the search function. NEEDLE_T must be
 * identical the parameters passed to the macro used for the declaration of the
 * function finding the position for insertion.
 */
#define AM_DECL_TYPED_ARRAY_RESERVE_SORTED_SUFFIX(prefix, suffix, T, NEEDLE_T)	\
	/* Reserves space for a new element at the position for needle */	\
	/* assuming that the array is sorted. Returns a pointer to the newly */ \
	/* allocated element or NULL if space couldn't be reserved. */		\
	static inline T*							\
	prefix##_reserve_sorted##suffix(struct prefix* a,			\
					const NEEDLE_T needle)			\
	{									\
		size_t pos;							\
										\
		pos = prefix##_insertpos##suffix(a, needle);			\
										\
		if(prefix##_reserve_pos(a, pos))				\
			return NULL;						\
										\
		return &a->elements[pos];					\
	}

#define AM_DECL_TYPED_ARRAY_RESERVE_SORTED(prefix, T, NEEDLE_T) \
	AM_DECL_TYPED_ARRAY_RESERVE_SORTED_SUFFIX(prefix, , T, NEEDLE_T)

#endif
