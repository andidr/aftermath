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

#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <stdint.h>

typedef uint64_t bitvector_chunk_t;
#define BYTES_PER_CHUNK sizeof(bitvector_chunk_t)
#define BITS_PER_CHUNK (BYTES_PER_CHUNK*8)
#define NUM_OVERVIEW_CHUNKS 5

struct bitvector {
	int max_bits;
	int num_chunks;
	int min_set_bit;
	int max_set_bit;
	int min_chunk;
	int max_chunk;
	bitvector_chunk_t* bits;
	bitvector_chunk_t overview_chunks[NUM_OVERVIEW_CHUNKS];
};

int bitvector_init(struct bitvector* bv, int max_bits);
void bitvector_destroy(struct bitvector* bv);
void bitvector_clear(struct bitvector* bv);
void bitvector_set_bit(struct bitvector* bv, int bit);
void bitvector_clear_bit(struct bitvector* bv, int bit);
int bitvector_test_bit(struct bitvector* bv, int bit);
int bitvector_equals(struct bitvector* bv1, struct bitvector* bv2);
int bitvector_is_subset(struct bitvector* superset, struct bitvector* subset);
void bitvector_merge(struct bitvector* dst, struct bitvector* src);
int bitvector_get_next_bit(struct bitvector* bv, int start_bit);
int bitvector_num_bits_set(struct bitvector* bv);
int bitvector_get_nth_set_bit(struct bitvector* bv, int n);

void bitvector_dump(struct bitvector* bv);
struct bitvector* bitvector_copy(struct bitvector* bv);

int next_pow2(int val);

#endif
