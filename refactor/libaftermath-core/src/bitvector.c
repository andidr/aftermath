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

#include <aftermath/core/bitvector.h>
#include <aftermath/core/aux.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Returns the number of bits set to one in the bitvector. */
int am_bitvector_num_bits_set(struct am_bitvector* bv)
{
	int count = 0;

	for(int i = bv->min_set_bit; i <= bv->max_set_bit; ++i)
		if(am_bitvector_test_bit(bv, i))
			count++;

	return count;
}

/* Returns the zero-indexed position of the n-th bit that is 1. If no such bit
 * exists, the function returns -1. */
int am_bitvector_get_nth_set_bit(struct am_bitvector* bv, int n)
{
	int ret = am_bitvector_get_next_bit(bv, -1);

	for(int i = 0; i < n; ++i)
		ret = am_bitvector_get_next_bit(bv, ret);

	return ret;
}

/* Initializes a bitvector. Max_bits specifies how many bits the vector can
 * hold, where max_bits-1 is the highest bit index. Returns 0 on success,
 * otherwise 1. */
int am_bitvector_init(struct am_bitvector* bv, int max_bits)
{
	bv->bits = NULL;
	bv->num_chunks = 0;
	bv->max_bits = 0;
	bv->min_set_bit = max_bits;
	bv->max_set_bit = 0;
	bv->min_chunk = 0;
	bv->max_chunk = 0;

	if(am_bitvector_resize(bv, max_bits, 0))
		return 1;

	return 0;
}

/* Sets all bits to zero. */
void am_bitvector_clear(struct am_bitvector* bv)
{
	memset(bv->bits, 0, AM_BV_BYTES_PER_CHUNK*bv->num_chunks);
	bv->min_set_bit = bv->max_bits;
	bv->max_set_bit = 0;
	bv->min_chunk = bv->num_chunks;
	bv->max_chunk = 0;
}

/* Frees all the resources of a bitvector, but not the bitvector itself. */
void am_bitvector_destroy(struct am_bitvector* bv)
{
	free(bv->bits);
}

/* Sets the bit at the zero-indexed position specified by the parameter
 * "bit". No bounds check is performed by the function. */
void am_bitvector_set_bit(struct am_bitvector* bv, int bit)
{
	int chunk = bit / AM_BV_BITS_PER_CHUNK;
	int chunk_bit = bit % AM_BV_BITS_PER_CHUNK;

	if(bit > bv->max_set_bit) {
		bv->max_set_bit = bit;
		bv->max_chunk = chunk;
	}

	if(bit < bv->min_set_bit) {
		bv->min_set_bit = bit;
		bv->min_chunk = chunk;
	}

	bv->bits[chunk] |= (am_bitvector_chunk_t)1 << chunk_bit;
}

/* Sets or clears the bit at the zero-indexed position specified by the
 * parameter "bit" depending on its current state. No bounds check is performed
 * by the function. */
void am_bitvector_toggle_bit(struct am_bitvector* bv, int bit)
{
	if(am_bitvector_test_bit(bv, bit))
		am_bitvector_clear_bit(bv, bit);
	else
		am_bitvector_set_bit(bv, bit);
}

/* Updates the position of the first bit that is one. */
void am_bitvector_update_min(struct am_bitvector* bv)
{
	bv->min_set_bit = AM_BV_BITS_PER_CHUNK*bv->num_chunks;

	for(int chunk = 0; chunk < bv->num_chunks; chunk++) {
		if(bv->bits[chunk]) {
			for(int chunk_bit = 0;
			    chunk_bit < AM_BV_BITS_PER_CHUNK;
			    chunk_bit++)
			{
				if(bv->bits[chunk] &
				   ((am_bitvector_chunk_t)1 << chunk_bit))
				{
					bv->min_set_bit = chunk_bit +
						chunk*AM_BV_BITS_PER_CHUNK;
					bv->min_chunk = chunk;
					return;
				}
			}
		}
	}
}

/* Updates the position of the last bit that is one. */
void am_bitvector_update_max(struct am_bitvector* bv)
{
	bv->max_set_bit = 0;

	for(int chunk = bv->num_chunks-1; chunk >= 0 ; chunk--) {
		if(bv->bits[chunk]) {
			for(int chunk_bit = AM_BV_BITS_PER_CHUNK-1;
			    chunk_bit >= 0;
			    chunk_bit--)
			{
				if(bv->bits[chunk] &
				   ((am_bitvector_chunk_t)1 << chunk_bit))
				{
					bv->max_set_bit = chunk_bit +
						chunk*AM_BV_BITS_PER_CHUNK;
					bv->max_chunk = chunk;
					return;
				}
			}
		}
	}
}

/* Clears the bit at the zero-indexed position specified by the parameter
 * "bit". */
void am_bitvector_clear_bit(struct am_bitvector* bv, int bit)
{
	int chunk = bit / AM_BV_BITS_PER_CHUNK;
	int chunk_bit = bit % AM_BV_BITS_PER_CHUNK;

	bv->bits[chunk] &= ~((am_bitvector_chunk_t)1 << chunk_bit);

	if(bit == bv->max_set_bit)
		am_bitvector_update_max(bv);

	if(bit == bv->min_set_bit)
		am_bitvector_update_min(bv);
}

/* Returns 1 if the bit at the zero-indexed position specified by the parameter
 * "bit" is set. Otherwise the function returns 0. */
int am_bitvector_test_bit(struct am_bitvector* bv, int bit)
{
	int chunk = bit / AM_BV_BITS_PER_CHUNK;
	int chunk_bit = bit % AM_BV_BITS_PER_CHUNK;

	if(bv->bits[chunk] & ((am_bitvector_chunk_t)1 << chunk_bit))
		return 1;

	return 0;
}

/* Returns 1 if the two bitvectors represent identical bit sequences, otherwise
 * 0. */
int am_bitvector_equals(struct am_bitvector* bv1, struct am_bitvector* bv2)
{
	if(bv1->max_bits != bv2->max_bits)
		return 0;

	return memcmp(bv1->bits,
		      bv2->bits,
		      bv1->num_chunks * AM_BV_BYTES_PER_CHUNK) == 0;
}

/* Tests if all the bits of the bitvector "subset" are also set in the bitvector
 * "superset". If this is the case, the function returns 1, otherwise 0. */
int am_bitvector_is_subset(struct am_bitvector* superset, struct am_bitvector* subset)
{
	if(subset->max_set_bit > superset->max_set_bit ||
		subset->min_set_bit < superset->min_set_bit)
	{
		return 0;
	}

	for(int i = subset->min_chunk; i <= subset->max_chunk; ++i) {
		if((subset->bits[i] & superset->bits[i]) != subset->bits[i])
			return 0;
	}

	return 1;
}

/* Sets all the bits in dst, which are set in src. No bounds check is performed
 * by the function. */
void am_bitvector_merge(struct am_bitvector* dst, struct am_bitvector* src)
{
	int min_chunk = src->min_set_bit / AM_BV_BITS_PER_CHUNK;
	int max_chunk = src->max_set_bit / AM_BV_BITS_PER_CHUNK;

	for(int i = min_chunk; i <= max_chunk; ++i)
		dst->bits[i] |= src->bits[i];

	if(dst->max_set_bit < src->max_set_bit) {
		dst->max_set_bit = src->max_set_bit;
		dst->max_chunk = src->max_chunk;
	}

	if(dst->min_set_bit > src->min_set_bit) {
		dst->min_set_bit = src->min_set_bit;
		dst->min_chunk = src->min_chunk;
	}
}

/* Returns the zero-indexed position of the bit that is equal to one and that
 * has a higher index than start_bit. If no such bit exists, the function
 * returns -1. */
int am_bitvector_get_next_bit(struct am_bitvector* bv, int start_bit)
{
	start_bit++;

	for(int i = start_bit; i <= bv->max_set_bit; ++i) {
		if(am_bitvector_test_bit(bv, i))
			return i;
	}

	return -1;
}

/* Allocates a bitvector with the same size and the same bits set as bv. Returns
 * a pointer to the copy or NULL on failure. */
struct am_bitvector* am_bitvector_copy(struct am_bitvector* bv)
{
	struct am_bitvector* ret = malloc(sizeof(struct am_bitvector));
	am_bitvector_init(ret, bv->max_bits);
	am_bitvector_merge(ret, bv);

	return ret;
}

/* Returns 1 if no bit is set, otherwise 0. */
int am_bitvector_is_zero(struct am_bitvector* bv)
{
	for(int i = 0; i < bv->num_chunks; i++)
		if(bv->bits[i])
			return 0;

	return 1;
}

/* Dumps a bitvector to stdout. */
void am_bitvector_dump(struct am_bitvector* bv)
{
	printf("MISB = %d, MASB = %d, C = ", bv->min_set_bit, bv->max_set_bit);
	for(int i = bv->min_set_bit; i <= bv->max_set_bit; ++i) {
		if(am_bitvector_test_bit(bv, i))
			printf("%d ", i);
	}
	puts("");
}

/* Clears all the bits in a zero-indexed range, including the bits at the
 * bounds. */
void am_bitvector_clear_range(struct am_bitvector* bv, int start_bit, int end_bit)
{
	size_t start_chunk = start_bit / AM_BV_BITS_PER_CHUNK;
	size_t end_chunk = end_bit / AM_BV_BITS_PER_CHUNK;
	am_bitvector_chunk_t mask = 0;
	size_t startplus = 0;

	if(start_bit % AM_BV_BITS_PER_CHUNK) {
		for(size_t b = 0; b < start_bit % AM_BV_BITS_PER_CHUNK; b++)
			mask |= ((am_bitvector_chunk_t)1 << b);

		if(start_chunk == end_chunk) {
			for(size_t b = end_bit; b < AM_BV_BITS_PER_CHUNK; b++)
				mask |= ((am_bitvector_chunk_t)1 << b);
		}

		bv->bits[start_chunk] &= mask;
		startplus = 1;
	}

	for(size_t c = start_chunk + startplus; c < end_chunk; c++)
		bv->bits[c] = 0;

	if(start_chunk != end_chunk && end_bit % AM_BV_BITS_PER_CHUNK) {
		mask = 0;

		for(size_t b = end_bit; b < AM_BV_BITS_PER_CHUNK; b++)
			mask |= ((am_bitvector_chunk_t)1 << b);

		bv->bits[end_chunk] &= mask;
	}

	if(start_bit < bv->min_set_bit)
		am_bitvector_update_min(bv);

	if(end_bit > bv->max_set_bit)
		am_bitvector_update_max(bv);
}

/* Resizes a bitvector to a new maximum number of bits specified by
 * new_max_bits. If this value is lower than the current maximum and shrink is
 * specified, memory not longer required is freed. Returns 0 on success,
 * otherwise 1. */
int am_bitvector_resize(struct am_bitvector* bv, int new_max_bits, int shrink)
{
	size_t new_num_chunks = AM_DIV_ROUND_UP(new_max_bits, AM_BV_BITS_PER_CHUNK);
	int rounded_new_max_bits = new_num_chunks * AM_BV_BITS_PER_CHUNK;
	int old_max_bits = bv->max_bits;
	int old_num_chunks = bv->num_chunks;
	void* tmp;

	if(old_max_bits == new_max_bits)
		return 0;

	if((new_num_chunks > old_num_chunks) ||
	   (new_num_chunks < old_num_chunks && shrink))
	{
		if(!(tmp = realloc(bv->bits, AM_BV_BYTES_PER_CHUNK*new_num_chunks)))
			return 1;

		bv->bits = tmp;
	}

	bv->num_chunks = new_num_chunks;
	bv->max_bits = rounded_new_max_bits;

	if(new_max_bits < old_max_bits)
		am_bitvector_clear_range(bv, new_max_bits, rounded_new_max_bits);
	else
		am_bitvector_clear_range(bv, old_max_bits, rounded_new_max_bits);

	if(new_max_bits < bv->min_set_bit)
		am_bitvector_update_min(bv);

	if(new_max_bits < bv->max_set_bit)
		am_bitvector_update_max(bv);

	return 0;
}
