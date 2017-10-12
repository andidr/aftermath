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

#ifndef AM_BITVECTOR_H
#define AM_BITVECTOR_H

/* A naive implementation of a bitvector. */

#include <stdint.h>

typedef uint64_t am_bitvector_chunk_t;
#define AM_BV_BYTES_PER_CHUNK sizeof(am_bitvector_chunk_t)
#define AM_BV_BITS_PER_CHUNK (AM_BV_BYTES_PER_CHUNK*8)

struct am_bitvector {
	int max_bits;
	int num_chunks;
	int min_set_bit;
	int max_set_bit;
	int min_chunk;
	int max_chunk;
	am_bitvector_chunk_t* bits;
};

int am_bitvector_init(struct am_bitvector* bv, int max_bits);
int am_bitvector_resize(struct am_bitvector* bv, int max_bits, int shrink);
void am_bitvector_destroy(struct am_bitvector* bv);
void am_bitvector_clear(struct am_bitvector* bv);
void am_bitvector_set_bit(struct am_bitvector* bv, int bit);
void am_bitvector_clear_bit(struct am_bitvector* bv, int bit);
void am_bitvector_clear_range(struct am_bitvector* bv, int start_bit, int end_bit);
int am_bitvector_test_bit(struct am_bitvector* bv, int bit);
int am_bitvector_equals(struct am_bitvector* bv1, struct am_bitvector* bv2);
int am_bitvector_is_subset(struct am_bitvector* superset, struct am_bitvector* subset);
void am_bitvector_merge(struct am_bitvector* dst, struct am_bitvector* src);
int am_bitvector_get_next_bit(struct am_bitvector* bv, int start_bit);
int am_bitvector_num_bits_set(struct am_bitvector* bv);
int am_bitvector_get_nth_set_bit(struct am_bitvector* bv, int n);
int am_bitvector_is_zero(struct am_bitvector* bv);

void am_bitvector_dump(struct am_bitvector* bv);
struct am_bitvector* am_bitvector_copy(struct am_bitvector* bv);

#endif
