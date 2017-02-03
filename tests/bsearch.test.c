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
#include "../src/bsearch.h"

#define IDENT(x) (x)
#define SMALLER_P(a, b) ((a) < (b))
#define GREATER_P(a, b) ((a) > (b))

DECL_VSTRIDED_BSEARCH_SUFFIX(, _float, float, float, IDENT)
DECL_VSTRIDED_BSEARCH_SUFFIX(, _int, int, int, IDENT)
DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(, _float, float, float, IDENT, SMALLER_P, GREATER_P)
DECL_VSTRIDED_BSEARCH_FIRST_SUFFIX(, _int, int, int, IDENT, SMALLER_P, GREATER_P)

struct composed {
	float a;
	int b;
	struct {
		int i;
		int j;
	} c;
};

UNIT_TEST(diffvals_test)
{
	struct composed e[100];

	for(size_t i = 0; i < 100; i++) {
		e[i].a = 1*i;
		e[i].b = 2*i;
		e[i].c.i = 3*i;
		e[i].c.j = 4*i;
	}

	for(size_t i = 0; i < 100; i++) {
		ASSERT_EQUALS(bsearch_strided_float(&e[0].a, 100, sizeof(struct composed), i), &e[i].a);
		ASSERT_EQUALS(bsearch_strided_int(&e[0].b, 100, sizeof(struct composed), 2*i), &e[i].b);
	}
}
END_TEST()

UNIT_TEST(dupvals_test)
{
	struct composed e[] = {
		{0, 0, {0, 0}},    /* 0 */
		{0, 0, {0, 1}},    /* 1 */
		{1, 2, {3, 1}},    /* 2 */
		{1, 2, {3, 1}},    /* 3 */
		{1, 2, {3, 29}},    /* 4 */
		{2, 4, {6, 29}},    /* 5 */
		{6, 12, {18, 2678}}, /* 6 */
		{6, 12, {18, 2678}}, /* 7 */
		{6, 12, {18, 24000}}, /* 8 */
		{6, 12, {18, 24000}}, /* 9 */
		{9, 18, {27, 36000}}, /* 10 */
		{14, 28, {42, 56000}}, /* 11 */
	};

	ASSERT_EQUALS(bsearch_first_strided_float(&e[0].a, 12, sizeof(struct composed), 0), &e[0].a);
	ASSERT_EQUALS(bsearch_first_strided_float(&e[0].a, 12, sizeof(struct composed), 1), &e[2].a);
	ASSERT_EQUALS(bsearch_first_strided_float(&e[0].a, 12, sizeof(struct composed), 2), &e[5].a);
	ASSERT_EQUALS(bsearch_first_strided_float(&e[0].a, 12, sizeof(struct composed), 6), &e[6].a);
	ASSERT_EQUALS(bsearch_first_strided_float(&e[0].a, 12, sizeof(struct composed), 9), &e[10].a);
	ASSERT_EQUALS(bsearch_first_strided_float(&e[0].a, 12, sizeof(struct composed), 14), &e[11].a);

	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].b, 12, sizeof(struct composed), 0), &e[0].b);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].b, 12, sizeof(struct composed), 2), &e[2].b);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].b, 12, sizeof(struct composed), 4), &e[5].b);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].b, 12, sizeof(struct composed), 12), &e[6].b);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].b, 12, sizeof(struct composed), 18), &e[10].b);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].b, 12, sizeof(struct composed), 28), &e[11].b);

	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.i, 12, sizeof(struct composed), 0), &e[0].c.i);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.i, 12, sizeof(struct composed), 3), &e[2].c.i);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.i, 12, sizeof(struct composed), 6), &e[5].c.i);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.i, 12, sizeof(struct composed), 18), &e[6].c.i);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.i, 12, sizeof(struct composed), 27), &e[10].c.i);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.i, 12, sizeof(struct composed), 42), &e[11].c.i);

	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.j, 12, sizeof(struct composed), 0), &e[0].c.j);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.j, 12, sizeof(struct composed), 1), &e[1].c.j);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.j, 12, sizeof(struct composed), 29), &e[4].c.j);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.j, 12, sizeof(struct composed), 2678), &e[6].c.j);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.j, 12, sizeof(struct composed), 24000), &e[8].c.j);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.j, 12, sizeof(struct composed), 36000), &e[10].c.j);
	ASSERT_EQUALS(bsearch_first_strided_int(&e[0].c.j, 12, sizeof(struct composed), 56000), &e[11].c.j);
}
END_TEST()

UNIT_TEST_SUITE(typed_array_test)
{
	ADD_TEST(diffvals_test);
	ADD_TEST(dupvals_test);
}
END_TEST_SUITE()

