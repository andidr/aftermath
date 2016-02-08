/**
 * Copyright (C) 2016 Andi Drebes <andi.drebes@lip6.fr>
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
#include "../src/monotonic_index.h"


UNIT_TEST(init_test)
{
	struct monotonic_index idx;
	size_t num_samples = 100;
	size_t num_dimensions = 3;

	ASSERT_EQUALS(monotonic_index_init(&idx, num_dimensions, num_samples), 0);
	ASSERT_EQUALS(idx.num_samples, num_samples);
	ASSERT_EQUALS(idx.num_dimensions, num_dimensions);
	monotonic_index_destroy(&idx);
}
END_TEST()

UNIT_TEST(set_test)
{
	struct monotonic_index idx;
	size_t num_samples = 100;
	size_t num_dimensions = 3;
	uint64_t data[num_dimensions];

	ASSERT_EQUALS(monotonic_index_init(&idx, num_dimensions, num_samples), 0);

	for(size_t s = 0; s < num_samples; s++) {
		for(size_t d = 0; d < num_dimensions; d++)
			data[d] = s*d*1000;

		monotonic_index_set(&idx, s, s * 1000000, data, 1);
	}

	ASSERT_EQUALS(idx.min_timestamp, 0);
	ASSERT_EQUALS(idx.max_timestamp, (num_samples-1) * 1000000);

	monotonic_index_destroy(&idx);
}
END_TEST()

UNIT_TEST(get_lindex_empty_test)
{
	struct monotonic_index idx;
	size_t num_samples = 0;
	size_t num_dimensions = 3;
	uint64_t i;

	ASSERT_EQUALS(monotonic_index_init(&idx, num_dimensions, num_samples), 0);
	ASSERT_FALSE(monotonic_index_get_lindex(&idx, 1000000, &i));

	monotonic_index_destroy(&idx);
}
END_TEST()

UNIT_TEST(get_lindex_test)
{
	struct monotonic_index idx;
	size_t num_samples = 100;
	size_t num_dimensions = 3;
	size_t i;
	uint64_t data[num_dimensions];

	ASSERT_EQUALS(monotonic_index_init(&idx, num_dimensions, num_samples), 0);

	for(size_t s = 0; s < num_samples; s++) {
		for(size_t d = 0; d < num_dimensions; d++)
			data[d] = s*d*1000;

		monotonic_index_set(&idx, s, (s+1) * 1000000, data, 1);
	}

	ASSERT_EQUALS(idx.min_timestamp, 0);
	ASSERT_EQUALS(idx.max_timestamp, num_samples * 1000000);

	/* Exact matches */
	for(size_t s = 0; s < num_samples; s++) {
		ASSERT_TRUE(monotonic_index_get_lindex(&idx, (s+1)*1000000, &i));
		ASSERT_EQUALS(i, s);
	}

	/* Matches between samples */
	for(size_t s = 0; s < num_samples-1; s++) {
		ASSERT_TRUE(monotonic_index_get_lindex(&idx, (s+1)*1000000+500000, &i));
		ASSERT_EQUALS(i, s);
	}

	/* After last sample */
	ASSERT_TRUE(monotonic_index_get_lindex(&idx, num_samples*1000000+500000, &i));
	ASSERT_EQUALS(i, num_samples-1);

	/* Before first sample */
	ASSERT_FALSE(monotonic_index_get_lindex(&idx, 500000, &i));

	monotonic_index_destroy(&idx);
}
END_TEST()

UNIT_TEST(get_test)
{
	struct monotonic_index idx;
	size_t num_samples = 100;
	size_t num_dimensions = 3;
	size_t num_valid;
	uint64_t time;
	uint64_t data[num_dimensions];

	ASSERT_EQUALS(monotonic_index_init(&idx, num_dimensions, num_samples), 0);

	for(size_t s = 0; s < num_samples; s++) {
		for(size_t d = 0; d < num_dimensions; d++)
			data[d] = s*d*1000;

		monotonic_index_set(&idx, s, (s+1) * 1000000, data, s);
	}

	for(size_t s = 0; s < num_samples; s++) {
		monotonic_index_get(&idx, s, &time, data, &num_valid);

		for(size_t d = 0; d < num_dimensions; d++)
			ASSERT_EQUALS(data[d], s*d*1000);

		ASSERT_EQUALS(num_valid, s);
		ASSERT_EQUALS(time, (s+1) * 1000000);
	}

	monotonic_index_destroy(&idx);
}
END_TEST()

UNIT_TEST_SUITE(monotonic_index_test)
{
	ADD_TEST(init_test);
	ADD_TEST(set_test);
	ADD_TEST(get_lindex_empty_test);
	ADD_TEST(get_lindex_test);
	ADD_TEST(get_test);
}
END_TEST_SUITE()
