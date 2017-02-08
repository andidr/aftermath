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

UNIT_TEST(set_test)
{
	int random_bits[] = {1, 4, 6, 7, 9, 34, 67, 19, 88, 14, 52 };
	int bitval;
	struct bitvector bv;

	ASSERT_EQUALS(bitvector_init(&bv, 100), 0);

	for(int i = 0; i < 100; i++)
		ASSERT_EQUALS(bitvector_test_bit(&bv, i), 0);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		bitvector_set_bit(&bv, random_bits[i]);

	for(int i = 0; i < ARRAY_SIZE(random_bits); i++)
		ASSERT_TRUE(bitvector_test_bit(&bv, random_bits[i]));

	for(int i = 0; i < 100; i++) {
		bitval = 0;

		for(int j = 0; j < ARRAY_SIZE(random_bits); j++) {
			if(i == random_bits[j]) {
				bitval = 1;
				break;
			}
		}

		if(bitval) {
			ASSERT_TRUE(bitvector_test_bit(&bv, i));
		} else {
			ASSERT_FALSE(bitvector_test_bit(&bv, i));
		}
	}

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

UNIT_TEST_SUITE(bitvector_test)
	ADD_TEST(set_test);
	ADD_TEST(count_test);
	ADD_TEST(subset_test);
	ADD_TEST(next_bit_test);
	ADD_TEST(nth_bit_test);
	ADD_TEST(clear_bit_test);
	ADD_TEST(clear_bit_minmax_test);
END_TEST_SUITE()
