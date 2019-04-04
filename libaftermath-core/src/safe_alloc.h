/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * ************************************************************************
 * * THIS FILE IS PART OF THE CODE RELEASED UNDER THE LGPL, VERSION 2.1   *
 * * UNLIKE THE MAJORITY OF THE CODE OF LIBAFTERMATH-CORE, RELEASED UNDER *
 * * THE GPL, VERSION 2.                                                  *
 * ************************************************************************
 *
 * This file can be redistributed it and/or modified under the terms of
 * the GNU Lesser General Public License version 2.1 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef AM_SAFE_ALLOC_H
#define AM_SAFE_ALLOC_H

#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

/* Returns 1 if a + b would overflow, otherwise 0. */
static inline int am_uintptr_add_overflows(uintptr_t a, uintptr_t b)
{
	return (UINTPTR_MAX - a < b);
}

/* Assigns *out = a + b iff the addition of a and b does not overflow. If an
 * overflow would occur, the function returns 1 and leaves *out
 * untouched. Otherwise, the function returns 0.
 */
static inline int am_uintptr_add_safe(uintptr_t* out, uintptr_t a, uintptr_t b)
{
	if(am_uintptr_add_overflows(a, b))
		return 1;

	*out = a + b;

	return 0;
}

/* Returns 1 if a + b would overflow, otherwise 0. */
static inline int am_size_add_overflows(size_t a, size_t b)
{
	return (SIZE_MAX - a < b);
}

/* Returns 1 if a * b would overflow, otherwise 0. */
static inline int am_size_mul_overflows(size_t a, size_t b)
{
	return a != 0 && (SIZE_MAX / a < b);
}

/* Assigns *out = a + b iff the addition of a and b does not overflow. If an
 * overflow would occur, the function returns 1 and leaves *out
 * untouched. Otherwise, the function returns 0.
 */
static inline int am_size_add_safe(size_t* out, size_t a, size_t b)
{
	if(am_size_add_overflows(a, b))
		return 1;

	*out = a + b;

	return 0;
}

/* Assigns *out = *out + b iff the addition of *out and a does not overflow. If
 * an overflow would occur, the function returns 1 and leaves *out
 * untouched. Otherwise, the function returns 0.
 */
static inline int am_size_inc_safe(size_t* out, size_t a)
{
	return am_size_add_safe(out, *out, a);
}

/* Assigns *out = a * b iff the multiplication of a and b does not overflow. If
 * an overflow would occur, the function returns 1 and leaves *out
 * untouched. Otherwise, the function returns 0.
 */
static inline int am_size_mul_safe(size_t* out, size_t a, size_t b)
{
	if(am_size_mul_overflows(a, b))
		return 1;

	*out = a * b;

	return 0;
}

/* Returns a pointer to the n-th element of size s of an array starting at the
 * address a without overflowing. If an overflow would occur, the function
 * returns NULL.  */
static inline void* am_array_element_ptr_safe(void* a, size_t n, size_t s)
{
	size_t offset;
	void* ret;
	uintptr_t retu;

	/* Overflow on total number of bytes? */
	if(am_size_mul_safe(&offset, n, s))
		return NULL;

	if(offset > UINTPTR_MAX)
		return NULL;

	if(am_uintptr_add_safe(&retu, (uintptr_t)a, offset))
		return NULL;

	/* Conversion back to a pointer safe? */
	if(sizeof(void*) < sizeof(uintptr_t) && retu > UINTPTR_MAX)
		return NULL;

	ret = (void*)retu;

	return ret;
}

/* Reallocates a previously allocated array a, such that it can hold n of s
 * bytes. Upon success, the address of the resized array is returned, otherwise
 * NULL. */
static inline void* am_realloc_array_safe(void* a, size_t n, size_t s)
{
	size_t total_size;

	/* Overflow on total number of bytes? */
	if(am_size_mul_safe(&total_size, n, s))
		return NULL;

	return realloc(a, total_size);
}

/* Allocates a new array, such that it can hold n elements of s bytes. Upon
 * success, the address of the array is returned, otherwise NULL. */
static inline void* am_alloc_array_safe(size_t n, size_t s)
{
	size_t total_size;

	/* Overflow on total number of bytes? */
	if(am_size_mul_safe(&total_size, n, s))
		return NULL;

	return malloc(total_size);
}

/* Reallocates a previously allocated array with n samples, such that it can
 * hold add more samples of s bytes. Upon success, the address of the resized
 * array is returned, otherwise NULL. */
static inline void*
am_grow_array_safe(void* a, size_t n, size_t add, size_t s)
{
	size_t total_samples;

	/* Overflow on total number of samples? */
	if(am_size_add_safe(&total_samples, n, add))
		return NULL;

	return am_realloc_array_safe(a, total_samples, s);
}

#endif
