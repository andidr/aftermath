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

#include <unit_tests.h>
#include "../src/bitvector.h"
#include "common.h"

static inline size_t size_t_min(size_t a, size_t b)
{
	return (a < b) ? a : b;
}

void check_set_bits(struct bitvector* bv, size_t max_check_bits, int* bits, size_t num_bits)
{
	int bitval;

	for(int i = 0; i < bv->max_bits; i++) {
		bitval = 0;

		if(i < max_check_bits) {
			for(int j = 0; j < num_bits; j++) {
				if(i == bits[j]) {
					bitval = 1;
					break;
				}
			}
		}

		if(bitval)
			ASSERT_TRUE(bitvector_test_bit(bv, i));
		else
			ASSERT_FALSE(bitvector_test_bit(bv, i));
	}
}

void verify_range(struct bitvector* bv, size_t start, size_t end, int val)
{
	for(size_t b = start; b < end; b++)
		ASSERT_EQUALS(bitvector_test_bit(bv, b), val);
}

void verify_set_bitrange_safe(struct bitvector* bv, size_t min_set_bit, size_t max_set_bit)
{
	verify_range(bv, 0, size_t_min(min_set_bit, bv->max_bits), 0);
	verify_range(bv, min_set_bit, size_t_min(max_set_bit, bv->max_bits), 1);

	if(bv->max_bits > max_set_bit)
		verify_range(bv, max_set_bit+1, bv->max_bits, 0);

	if(bv->max_bits > min_set_bit) {
		if(max_set_bit > min_set_bit) {
			ASSERT_EQUALS(bv->min_set_bit, min_set_bit);
			ASSERT_EQUALS(bv->max_set_bit, max_set_bit-1);

			if(bv->max_bits > max_set_bit) {
				ASSERT_EQUALS(bitvector_num_bits_set(bv), max_set_bit - min_set_bit);
				ASSERT_EQUALS(bv->min_set_bit, min_set_bit);

				if(max_set_bit > min_set_bit)
					ASSERT_EQUALS(bv->max_set_bit, max_set_bit-1);
				else
					ASSERT_EQUALS(bv->max_set_bit, max_set_bit);
			} else {
				ASSERT_EQUALS(bitvector_num_bits_set(bv), bv->max_bits - min_set_bit);
			}
		} else {
			ASSERT_EQUALS(bitvector_num_bits_set(bv), 0);
		}
	}
}

UNIT_TEST(set_test)
{
	int random_bits[] = {1, 4, 6, 7, 9, 34, 67, 19, 88, 14, 52 };
	struct bitvector bv;

	ASSERT_EQUALS(bitvector_init(&bv, 100), 0);

	for(int i = 0; i < 100; i++)
		ASSERT_EQUALS(bitvector_test_bit(&bv, i), 0);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		bitvector_set_bit(&bv, random_bits[i]);

	check_set_bits(&bv, 100, random_bits, ARRAY_SIZE(random_bits));

	bitvector_destroy(&bv);
}
END_TEST()

UNIT_TEST(count_test)
{
	int random_bits[] = {1, 4, 6, 7, 9, 34, 67, 19, 88, 14, 52 };
	struct bitvector bv;

	ASSERT_EQUALS(bitvector_init(&bv, 100), 0);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		bitvector_set_bit(&bv, random_bits[i]);

	ASSERT_EQUALS(bitvector_num_bits_set(&bv), ARRAY_SIZE(random_bits));

	bitvector_destroy(&bv);
}
END_TEST()

UNIT_TEST(subset_test)
{
	struct bitvector superset;
	struct bitvector subset;
	int random_bits[] = {1, 4, 6, 7, 9, 34, 67, 19, 88, 14, 52, 92, 65, 21, 73 };

	ASSERT_EQUALS(bitvector_init(&superset, 100), 0);
	ASSERT_EQUALS(bitvector_init(&subset, 100), 0);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		bitvector_set_bit(&superset, random_bits[i]);

	ASSERT_TRUE(bitvector_is_subset(&superset, &subset));
	ASSERT_FALSE(bitvector_equals(&superset, &subset));

	for(int i = 0; i < ARRAY_SIZE(random_bits); i += 2)
		bitvector_set_bit(&subset, random_bits[i]);

	ASSERT_TRUE(bitvector_is_subset(&superset, &subset));
	ASSERT_FALSE(bitvector_equals(&superset, &subset));

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		bitvector_set_bit(&subset, random_bits[i]);

	ASSERT_TRUE(bitvector_equals(&superset, &subset));

	bitvector_destroy(&superset);
	bitvector_destroy(&subset);
}
END_TEST()

UNIT_TEST(next_bit_test)
{
	int random_bits[] = {1, 4, 6, 7, 9, 14, 19, 34, 52, 67, 88, 89, 90};
	int arrsize = ARRAY_SIZE(random_bits);
	struct bitvector bv;

	ASSERT_EQUALS(bitvector_init(&bv, 100), 0);

	for(int i = 0; i < arrsize; i++)
		bitvector_set_bit(&bv, random_bits[i]);

	for(int i = 0; i < arrsize - 1; i++)
		ASSERT_EQUALS(bitvector_get_next_bit(&bv, random_bits[i]), random_bits[i+1]);

	ASSERT_EQUALS(bitvector_get_next_bit(&bv, random_bits[arrsize-1]), -1);

	bitvector_destroy(&bv);
}
END_TEST()

UNIT_TEST(nth_bit_test)
{
	int random_bits[] = {1, 4, 6, 7, 9, 14, 19, 34, 52, 67, 88, 89, 90};
	int arrsize = ARRAY_SIZE(random_bits);
	struct bitvector bv;

	ASSERT_EQUALS(bitvector_init(&bv, 100), 0);

	for(int i = 0; i < arrsize; i++)
		bitvector_set_bit(&bv, random_bits[i]);

	for(int i = 0; i < arrsize - 1; i++)
		ASSERT_EQUALS(bitvector_get_nth_set_bit(&bv, i), random_bits[i]);

	bitvector_destroy(&bv);
}
END_TEST()

UNIT_TEST(clear_bit_test)
{
	int random_bits[] = {1, 4, 6, 7, 9, 34, 67, 19, 88, 14, 52 };
	struct bitvector bv;

	ASSERT_EQUALS(bitvector_init(&bv, 100), 0);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		bitvector_set_bit(&bv, random_bits[i]);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++) {
		ASSERT_TRUE(bitvector_test_bit(&bv, random_bits[i]));
		bitvector_clear_bit(&bv, random_bits[i]);
		ASSERT_FALSE(bitvector_test_bit(&bv, random_bits[i]));
	}

	bitvector_destroy(&bv);
}
END_TEST()

UNIT_TEST(clear_bit_minmax_test)
{
	struct bitvector bv;
	ASSERT_EQUALS(bitvector_init(&bv, 100), 0);

	bitvector_set_bit(&bv, 0);
	bitvector_set_bit(&bv, 10);
	bitvector_set_bit(&bv, 20);
	bitvector_set_bit(&bv, 90);
	bitvector_set_bit(&bv, 98);
	bitvector_set_bit(&bv, 99);

	ASSERT_EQUALS(bv.max_set_bit, 99);
	ASSERT_EQUALS(bv.min_set_bit, 0);
	ASSERT_EQUALS(bitvector_num_bits_set(&bv), 6);

	bitvector_clear_bit(&bv, 99);
	ASSERT_EQUALS(bv.max_set_bit, 98);
	ASSERT_EQUALS(bv.min_set_bit, 0);
	ASSERT_EQUALS(bitvector_num_bits_set(&bv), 5);

	bitvector_clear_bit(&bv, 98);
	ASSERT_EQUALS(bv.max_set_bit, 90);
	ASSERT_EQUALS(bv.min_set_bit, 0);
	ASSERT_EQUALS(bitvector_num_bits_set(&bv), 4);

	bitvector_clear_bit(&bv, 0);
	ASSERT_EQUALS(bv.max_set_bit, 90);
	ASSERT_EQUALS(bv.min_set_bit, 10);
	ASSERT_EQUALS(bitvector_num_bits_set(&bv), 3);

	bitvector_clear_bit(&bv, 20);
	ASSERT_EQUALS(bv.max_set_bit, 90);
	ASSERT_EQUALS(bv.min_set_bit, 10);
	ASSERT_EQUALS(bitvector_num_bits_set(&bv), 2);

	bitvector_destroy(&bv);
}
END_TEST()

UNIT_TEST(resize_test)
{
	int random_bits[] = {1, 4, 6, 7, 9, 34, 67, 19, 88, 14, 52, 278, 299,
			     104, 267, 401, 589, 672, 298, 891, 182, 999 };
	struct bitvector bv;

	ASSERT_EQUALS(bitvector_init(&bv, 1000), 0);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		bitvector_set_bit(&bv, random_bits[i]);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		ASSERT_TRUE(bitvector_test_bit(&bv, random_bits[i]));

	check_set_bits(&bv, 1000, random_bits, ARRAY_SIZE(random_bits));

	for(size_t i = 0; i < 200; i++) {
		ASSERT_EQUALS(bitvector_resize(&bv, 1000+9*i, 0), 0);
		check_set_bits(&bv, 1000+9*i, random_bits, ARRAY_SIZE(random_bits));
	}

	for(size_t i = 200; i > 0; i--) {
		ASSERT_EQUALS(bitvector_resize(&bv, 1000+9*(i-1), 1), 0);
		check_set_bits(&bv, 1000+9*i, random_bits, ARRAY_SIZE(random_bits));
	}

	for(size_t i = 0; i < 1000; i++) {
		ASSERT_EQUALS(bitvector_resize(&bv, 1000-i, 1), 0);
		check_set_bits(&bv, 1000-i, random_bits, ARRAY_SIZE(random_bits));
	}

	bitvector_destroy(&bv);
}
END_TEST()

UNIT_TEST(resize_minmax_test)
{
	size_t max_bits = 65;
	size_t max_grow = 65;
	struct bitvector bv;

	for(size_t min_set_bit = 0; min_set_bit < max_bits; min_set_bit++) {
		for(size_t max_set_bit = min_set_bit; max_set_bit < max_bits; max_set_bit++) {
			for(int shrink = 0; shrink <= 1; shrink++) {
				for(size_t shrink_bits = 0; shrink_bits < max_set_bit; shrink_bits++) {
					ASSERT_EQUALS(bitvector_init(&bv, max_set_bit), 0);

					for(size_t b = min_set_bit; b < max_set_bit; b++)
						bitvector_set_bit(&bv, b);

					verify_set_bitrange_safe(&bv, min_set_bit, max_set_bit);

					ASSERT_EQUALS(bitvector_resize(&bv, max_set_bit - shrink_bits, shrink), 0);

					verify_set_bitrange_safe(&bv, min_set_bit, max_set_bit - shrink_bits);

					bitvector_destroy(&bv);
				}
			}

			for(size_t grow_bits = 0; grow_bits < max_grow; grow_bits++) {
				ASSERT_EQUALS(bitvector_init(&bv, max_set_bit), 0);

				for(size_t b = min_set_bit; b < max_set_bit; b++)
					bitvector_set_bit(&bv, b);

				verify_set_bitrange_safe(&bv, min_set_bit, max_set_bit);

				ASSERT_EQUALS(bitvector_resize(&bv, max_set_bit + grow_bits, 0), 0);

				verify_set_bitrange_safe(&bv, min_set_bit, max_set_bit);

				bitvector_destroy(&bv);
			}
		}
	}
}
END_TEST()

UNIT_TEST_SUITE(bitvector_test)
	ADD_TEST(set_test);
	ADD_TEST(count_test);
	ADD_TEST(subset_test);
	ADD_TEST(next_bit_test);
	ADD_TEST(nth_bit_test);
	ADD_TEST(clear_bit_test);
	ADD_TEST(clear_bit_minmax_test);
	ADD_TEST(resize_test);
	ADD_TEST(resize_minmax_test);
END_TEST_SUITE()
