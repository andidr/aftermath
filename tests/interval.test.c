/**
 * Copyright (C) 2017 Andi Drebes <andi.drebes@lip6.fr>
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
#include "common.h"
#include "../src/interval.h"

UNIT_TEST(intersect_p_test)
{
	struct {
		struct am_interval a;
		struct am_interval b;
		int res;
	} t[] = {
		{{0, 1000}, {0, 1000}, 1},
		{{0, 1000}, {1000, 2000}, 1},
		{{0, 1000}, {500, 2000}, 1},
		{{1100, 1200}, {1000, 2000}, 1},
		{{0, 999}, {1000, 2000}, 0},
	};

	for(size_t i = 0; i < ARRAY_SIZE(t); i++) {
		ASSERT_EQUALS(interval_intersect_p(&t[i].a, &t[i].b), t[i].res);

		ASSERT_EQUALS(interval_intersect_p(&t[i].b, &t[i].a), t[i].res);
	}
}
END_TEST()

UNIT_TEST(intersect_test)
{
	struct am_interval tmp;
	struct {
		struct am_interval a;
		struct am_interval b;
		struct am_interval i;
		int res;
	} t[] = {
		{{0, 1000}, {0, 1000}, {0, 1000}, 0},
		{{0, 1001}, {0, 1000}, {0, 1000}, 0},
		{{1000, 2000}, {1500, 2500}, {1500, 2000}, 0},
		{{1000, 2000}, {1500, 1700}, {1500, 1700}, 0},
		{{0, 999}, {1000, 2000}, {0}, 1},
	};

	for(size_t i = 0; i < ARRAY_SIZE(t); i++) {
		ASSERT_EQUALS(interval_intersection(&t[i].a, &t[i].b, &tmp), t[i].res);

		if(t[i].res == 0) {
			ASSERT_EQUALS(t[i].i.start, tmp.start);
			ASSERT_EQUALS(t[i].i.end, tmp.end);
		}

		ASSERT_EQUALS(interval_intersection(&t[i].b, &t[i].a, &tmp), t[i].res);

		if(t[i].res == 0) {
			ASSERT_EQUALS(t[i].i.start, tmp.start);
			ASSERT_EQUALS(t[i].i.end, tmp.end);
		}
	}
}
END_TEST()

UNIT_TEST(duration_test)
{
	struct {
		struct am_interval i;
		am_timestamp_diff_t d;
	} t[] = {
		{{0, 1000}, 1000},
		{{100, 1000}, 900},
		{{1000, 1000}, 0}
	};

	for(size_t i = 0; i < ARRAY_SIZE(t); i++)
		ASSERT_EQUALS(interval_duration(&t[i].i), t[i].d);
}
END_TEST()

UNIT_TEST_SUITE(interval_test)
	ADD_TEST(intersect_p_test);
	ADD_TEST(intersect_test);
	ADD_TEST(duration_test);
END_TEST_SUITE()
