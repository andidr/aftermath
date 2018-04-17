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

/**
 * Contains a few bit twiddling hacks inspired by
 * http://graphics.stanford.edu/~seander/bithacks.html
 */

#ifndef AM_BITS_H
#define AM_BITS_H

#include <stdint.h>

/* Generates a uint64_t with the lower n bits set */
#define AM_NBITS_SET_U64(n) \
	((n == 64) ? 0xFFFFFFFFFFFFFFFF : ((UINT64_C(1) << n) - 1))

/* Returns a mask containing only the lowest set bit of v. */
static inline uint64_t am_extract_lsb_u64(uint64_t v)
{
	/* Avoid overflow of (~v) + 1 */
	if(v == 0)
		return 0;

	return v & ((~v) + 1);
}

/* Returns the number of zero bits with a bit position lower than the first bit
 * that is 1. If mask is 0, the function returns 64. */
static inline size_t am_zero_bits_right_u64(uint64_t mask)
{
	size_t ret = 64;
	uint64_t lsb = am_extract_lsb_u64(mask);

	if(lsb != 0) ret--;
	if(lsb & 0x00000000FFFFFFFF) ret -= 32;
	if(lsb & 0x0000FFFF0000FFFF) ret -= 16;
	if(lsb & 0x00FF00FF00FF00FF) ret -= 8;
	if(lsb & 0x0F0F0F0F0F0F0F0F) ret -= 4;
	if(lsb & 0x3333333333333333) ret -= 2;
	if(lsb & 0x5555555555555555) ret -= 1;

	return ret;
}

/* Returns the zero-based index of the first bit that is 1. If mask is 0, the
 * function returns 64. */
static inline size_t am_first_set_bit_idx_u64(uint64_t mask)
{
	return am_zero_bits_right_u64(mask);
}

/* Iterates over the set bits in a uint64_t. At each iteration, the zero-based
 * index of the bit is assigned to idx. Tmpmask must be a uint64_t that is used
 * internally by the macro. */
#define am_for_each_bit_idx_u64(mask, tmpmask, idx)			\
	for(tmpmask = mask, idx = am_first_set_bit_idx_u64(mask);	\
	    idx != 64;							\
	    tmpmask = tmpmask & (~(UINT64_C(1) << idx)),		\
		    idx = am_first_set_bit_idx_u64(tmpmask))

/* Returns the number of bits set to 1 in a uint64_t. */
static inline size_t am_num_bits_set_u64(uint64_t mask)
{
	static const size_t nbits_nibble[16] = {
		/* 0b0000 */ 0, /* 0b0001 */ 1, /* 0b0010 */ 1, /* 0b0011 */ 2,
		/* 0b0100 */ 1, /* 0b0101 */ 2, /* 0b0110 */ 2, /* 0b0111 */ 3,
		/* 0b1000 */ 1, /* 0b1001 */ 2, /* 0b1010 */ 2, /* 0b1011 */ 3,
		/* 0b1100 */ 2, /* 0b1101 */ 3, /* 0b1110 */ 3, /* 0b1111 */ 4
	};

	return nbits_nibble[(mask >>   0) & 0xF] +
		nbits_nibble[(mask >>  4) & 0xF] +
		nbits_nibble[(mask >>  8) & 0xF] +
		nbits_nibble[(mask >> 12) & 0xF] +
		nbits_nibble[(mask >> 16) & 0xF] +
		nbits_nibble[(mask >> 20) & 0xF] +
		nbits_nibble[(mask >> 24) & 0xF] +
		nbits_nibble[(mask >> 28) & 0xF] +
		nbits_nibble[(mask >> 32) & 0xF] +
		nbits_nibble[(mask >> 36) & 0xF] +
		nbits_nibble[(mask >> 40) & 0xF] +
		nbits_nibble[(mask >> 44) & 0xF] +
		nbits_nibble[(mask >> 48) & 0xF] +
		nbits_nibble[(mask >> 52) & 0xF] +
		nbits_nibble[(mask >> 56) & 0xF] +
		nbits_nibble[(mask >> 60) & 0xF];
}

#endif
