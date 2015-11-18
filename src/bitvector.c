/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
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

#include "bitvector.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int bitvector_num_bits_set(struct bitvector* bv)
{
	int count = 0;

	for(int i = bv->min_set_bit; i <= bv->max_set_bit; ++i)
		if(bitvector_test_bit(bv, i))
			count++;

	return count;
}

int bitvector_get_nth_set_bit(struct bitvector* bv, int n)
{
	int ret = bitvector_get_next_bit(bv, -1);

	for(int i = 0; i < n; ++i) {
		ret = bitvector_get_next_bit(bv, ret);
	}

	return ret;
}

int bitvector_init(struct bitvector* bv, int max_bits)
{
	int num_chunks = max_bits / BITS_PER_CHUNK;

	if(max_bits % BITS_PER_CHUNK)
		num_chunks++;

	bv->max_bits = BITS_PER_CHUNK*num_chunks;
	bv->min_set_bit = max_bits;
	bv->max_set_bit = 0;
	bv->bits = malloc(BYTES_PER_CHUNK*num_chunks);
	bv->num_chunks = num_chunks;
	bv->min_chunk = num_chunks;
	bv->max_chunk = 0;

	if(!bv->bits)
		return 1;

	bitvector_clear(bv);

	return 0;
}

void bitvector_clear(struct bitvector* bv)
{
	memset(bv->overview_chunks, 0, NUM_OVERVIEW_CHUNKS*BYTES_PER_CHUNK);
	memset(bv->bits, 0, BYTES_PER_CHUNK*bv->num_chunks);
}

void bitvector_destroy(struct bitvector* bv)
{
	free(bv->bits);
}

void bitvector_set_bit(struct bitvector* bv, int bit)
{
	int chunk = bit / BITS_PER_CHUNK;
	int chunk_bit = bit % BITS_PER_CHUNK;

	int ov_chunk_bit = (bit*NUM_OVERVIEW_CHUNKS*BITS_PER_CHUNK) / bv->max_bits;
	int ov_chunk = ov_chunk_bit / BITS_PER_CHUNK;

	bv->overview_chunks[ov_chunk] |= (bitvector_chunk_t)1 << ov_chunk_bit % BITS_PER_CHUNK;

	if(bit > bv->max_set_bit) {
		bv->max_set_bit = bit;
		bv->max_chunk = chunk;
	}

	if(bit < bv->min_set_bit) {
		bv->min_set_bit = bit;
		bv->min_chunk = chunk;
	}

	bv->bits[chunk] |= (bitvector_chunk_t)1 << chunk_bit;
}

void bitvector_update_overview_for_bit(struct bitvector* bv, int bit)
{
	int ov_bit = (bit*NUM_OVERVIEW_CHUNKS*BITS_PER_CHUNK) / bv->max_bits;
	int ov_chunk_bit = ov_bit % BITS_PER_CHUNK;
	int ov_chunk = ov_bit / BITS_PER_CHUNK;
	int ibit;

	bv->overview_chunks[ov_chunk] &= ~((bitvector_chunk_t)1 << ov_chunk_bit);

	for(int dec = 0; dec < 2; dec++) {
		ibit = bit;

		while((ibit*NUM_OVERVIEW_CHUNKS*BITS_PER_CHUNK) / bv->max_bits == ov_bit) {
			if(bitvector_test_bit(bv, ibit)) {
				bv->overview_chunks[ov_chunk] |= ((bitvector_chunk_t)1 << ov_chunk_bit);
				return;
			}

			ibit += dec ? -1 : 1;
		}
	}
}

void bitvector_update_min(struct bitvector* bv)
{
	bv->min_set_bit = BITS_PER_CHUNK*bv->num_chunks;

	for(int chunk = 0; chunk < bv->num_chunks; chunk++) {
		if(bv->bits[chunk]) {
			for(int chunk_bit = 0; chunk_bit < BITS_PER_CHUNK; chunk_bit++) {
				if(bv->bits[chunk] & ((bitvector_chunk_t)1 << chunk_bit)) {
					bv->min_set_bit = chunk_bit + chunk*BITS_PER_CHUNK;
					bv->min_chunk = chunk;
					return;
				}
			}
		}
	}
}

void bitvector_update_max(struct bitvector* bv)
{
	bv->max_set_bit = 0;

	for(int chunk = bv->num_chunks-1; chunk >= 0 ; chunk--) {
		if(bv->bits[chunk]) {
			for(int chunk_bit = BITS_PER_CHUNK-1; chunk_bit >= 0; chunk_bit--) {
				if(bv->bits[chunk] & ((bitvector_chunk_t)1 << chunk_bit)) {
					bv->max_set_bit = chunk_bit + chunk*BITS_PER_CHUNK;
					bv->max_chunk = chunk;
					return;
				}
			}
		}
	}
}

void bitvector_clear_bit(struct bitvector* bv, int bit)
{
	int chunk = bit / BITS_PER_CHUNK;
	int chunk_bit = bit % BITS_PER_CHUNK;

	bv->bits[chunk] &= ~((bitvector_chunk_t)1 << chunk_bit);

	if(bit == bv->max_set_bit)
		bitvector_update_max(bv);

	if(bit == bv->min_set_bit)
		bitvector_update_min(bv);

	bitvector_update_overview_for_bit(bv, bit);
}

int bitvector_test_bit(struct bitvector* bv, int bit)
{
	int chunk = bit / BITS_PER_CHUNK;
	int chunk_bit = bit % BITS_PER_CHUNK;

	if(bv->bits[chunk] & ((bitvector_chunk_t)1 << chunk_bit))
		return 1;

	return 0;
}

int bitvector_equals(struct bitvector* bv1, struct bitvector* bv2)
{
	if(bv1->max_bits != bv2->max_bits)
		return 0;

	return memcmp(bv1->bits, bv2->bits, bv1->num_chunks * BYTES_PER_CHUNK) == 0;
}

int bitvector_is_subset(struct bitvector* superset, struct bitvector* subset)
{
	if(subset->max_set_bit > superset->max_set_bit ||
		subset->min_set_bit < superset->min_set_bit)
	{
		return 0;
	}

	for(int i = 0; i < NUM_OVERVIEW_CHUNKS; ++i) {
		if((superset->overview_chunks[i] & subset->overview_chunks[i]) != subset->overview_chunks[i])
			return 0;
	}

	for(int i = subset->min_chunk; i <= subset->max_chunk; ++i) {
		if((subset->bits[i] & superset->bits[i]) != subset->bits[i])
			return 0;
	}

	return 1;
}

void bitvector_merge(struct bitvector* dst, struct bitvector* src)
{
	int min_chunk = src->min_set_bit / BITS_PER_CHUNK;
	int max_chunk = src->max_set_bit / BITS_PER_CHUNK;

	for(int i = 0; i < NUM_OVERVIEW_CHUNKS; ++i) {
		dst->overview_chunks[i] |= src->overview_chunks[i];
	}

	for(int i = min_chunk; i <= max_chunk; ++i)
		dst->bits[i] |= src->bits[i];

	if(dst->max_set_bit < src->max_set_bit) {
		dst->max_set_bit = src->max_set_bit;
		dst->max_chunk = src->max_chunk;
// 		printf("update max_set_bit to %d\n", src->max_set_bit);
	}

	if(dst->min_set_bit > src->min_set_bit) {

// 		if(dst->min_set_bit > 1024)
// 			printf("update min_set_bit to %d (%d > %d)\n", src->min_set_bit, dst->min_set_bit, src->min_set_bit);
		dst->min_set_bit = src->min_set_bit;
		dst->min_chunk = src->min_chunk;

	}
}

int bitvector_get_next_bit(struct bitvector* bv, int start_bit)
{
	start_bit++;

	for(int i = start_bit; i <= bv->max_set_bit; ++i) {
		if(bitvector_test_bit(bv, i))
			return i;
	}

	return -1;
}

struct bitvector* bitvector_copy(struct bitvector* bv)
{
	struct bitvector* ret = malloc(sizeof(struct bitvector));
	bitvector_init(ret, bv->max_bits);
	bitvector_merge(ret, bv);

	return ret;
}

void bitvector_dump(struct bitvector* bv)
{
	printf("O=%d, MISB = %d, MASB = %d, C = ", (int)(bv->overview_chunks[0]), bv->min_set_bit, bv->max_set_bit);
	for(int i = bv->min_set_bit; i <= bv->max_set_bit; ++i) {
		if(bitvector_test_bit(bv, i))
			printf("%d ", i);
	}
	puts("");
}
