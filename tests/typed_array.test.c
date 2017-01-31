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
#include "../src/typed_array.h"
#include "common.h"

#define IDENT(x) (x)
#define SMALLER_P(a, b) ((a) < (b))
#define GREATER_P(a, b) ((a) > (b))

DECL_TYPED_ARRAY(int_array, int)
DECL_TYPED_ARRAY_BSEARCH(int_array, int, int, IDENT)
DECL_TYPED_ARRAY_BSEARCH_FIRST(int_array, int, int, IDENT, SMALLER_P, GREATER_P)
DECL_TYPED_ARRAY_INSERTPOS(int_array, int, int, IDENT)
DECL_TYPED_ARRAY_RESERVE_SORTED(int_array, int, int, IDENT)

struct composed {
	float a;
	int b;
	struct {
		int i;
		int j;
	} c;
};

#define ACC_A(x) ((x).a)
#define ACC_B(x) ((x).b)
#define ACC_CI(x) ((x).c.i)
#define ACC_CJ(x) ((x).c.j)
#define ACC_APLUSB(x) ((x).a+(x).b)

DECL_TYPED_ARRAY(composed_array, struct composed)
DECL_TYPED_ARRAY_BSEARCH_SUFFIX(composed_array, _a, struct composed, float, ACC_A)
DECL_TYPED_ARRAY_BSEARCH_SUFFIX(composed_array, _b, struct composed, int, ACC_B)
DECL_TYPED_ARRAY_BSEARCH_SUFFIX(composed_array, _ci, struct composed, int, ACC_CI)
DECL_TYPED_ARRAY_BSEARCH_SUFFIX(composed_array, _cj, struct composed, int, ACC_CJ)
DECL_TYPED_ARRAY_BSEARCH_SUFFIX(composed_array, _aplusb, struct composed, int, ACC_APLUSB)

UNIT_TEST(scalar_empty_test)
{
	struct int_array a;

	int_array_init(&a);

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_test)
{
	struct int_array a;

	int_array_init(&a);

	for(size_t i = 0; i < 100; i++) {
		ASSERT_EQUALS(int_array_append(&a, i), 0);
		ASSERT_EQUALS(a.num_elements, i+1);
	}

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(a.elements[i], i);

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_insert_test)
{
	struct int_array a;

	int_array_init(&a);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_append(&a, 2*i+1), 0);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_insert(&a, 2*i, 2*i), 0);

	ASSERT_EQUALS(a.num_elements, 200);

	for(size_t i = 0; i < 200; i++)
		ASSERT_EQUALS(a.elements[i], i);

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_insert_sorted_test)
{
	int values[] = { 1, 9, 15, 23, 101, 290, 34, 2, 0, -999, 19 };
	int values_sorted[] = { -999, 0, 1, 2, 9, 15, 19, 23, 34, 101, 290 };
	struct int_array a;
	int* pv;

	int_array_init(&a);

	for(size_t i = 0; i < ARRAY_SIZE(values); i++) {
		pv = int_array_reserve_sorted(&a, values[i]);
		ASSERT_NONNULL(pv);
		*pv = values[i];
		ASSERT_EQUALS(a.num_elements, i+1);
	}

	for(size_t i = 0; i < ARRAY_SIZE(values); i++)
		ASSERT_EQUALS(a.elements[i], values_sorted[i]);

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_insertpos_test)
{
	struct int_array a;

	int_array_init(&a);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_append(&a, 2*i+1), 0);

	ASSERT_EQUALS(int_array_insertpos(&a, -1), 0);
	ASSERT_EQUALS(int_array_insertpos(&a, -100), 0);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_insertpos(&a, 2*i), i);

	ASSERT_EQUALS(int_array_insertpos(&a, 200), 100);
	ASSERT_EQUALS(int_array_insertpos(&a, 201), 100);
	ASSERT_EQUALS(int_array_insertpos(&a, 1000), 100);

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_remove_last_test)
{
	struct int_array a;

	int_array_init(&a);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_append(&a, i), 0);

	ASSERT_EQUALS(a.num_elements, 100);

	for(size_t i = 0; i < 100; i++) {
		int_array_remove(&a, 100-i-1);
		ASSERT_EQUALS(a.num_elements, 100-i-1);
	}

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_remove_first_test)
{
	struct int_array a;

	int_array_init(&a);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_append(&a, i), 0);

	ASSERT_EQUALS(a.num_elements, 100);

	for(size_t i = 0; i < 100; i++) {
		int_array_remove(&a, 0);
		ASSERT_EQUALS(a.num_elements, 100-i-1);
	}

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_remove_mid_test)
{
	struct int_array a;

	int_array_init(&a);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_append(&a, i), 0);

	ASSERT_EQUALS(a.num_elements, 100);

	for(size_t i = 0; i < 98; i++) {
		int_array_remove(&a, 1);
		ASSERT_EQUALS(a.num_elements, 100-i-1);
	}

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(struct_test)
{
	struct composed_array a;
	struct composed e;

	composed_array_init(&a);

	for(size_t i = 0; i < 100; i++) {
		e.a = 1*i;
		e.b = 2*i;
		e.c.i = 3*i;
		e.c.j = 4*i;

		ASSERT_EQUALS(composed_array_append(&a, e), 0);
	}

	for(size_t i = 0; i < 100; i++) {
		e.a = 1000*i;
		e.b = 2000*i;
		e.c.i = 3000*i;
		e.c.j = 4000*i;

		ASSERT_EQUALS(composed_array_appendp(&a, &e), 0);
	}

	for(size_t i = 0; i < 100; i++) {
		ASSERT_EQUALS(a.elements[i].a, 1*i);
		ASSERT_EQUALS(a.elements[i].b, 2*i);
		ASSERT_EQUALS(a.elements[i].c.i, 3*i);
		ASSERT_EQUALS(a.elements[i].c.j, 4*i);

		ASSERT_EQUALS(a.elements[100+i].a, 1000*i);
		ASSERT_EQUALS(a.elements[100+i].b, 2000*i);
		ASSERT_EQUALS(a.elements[100+i].c.i, 3000*i);
		ASSERT_EQUALS(a.elements[100+i].c.j, 4000*i);
	}

	composed_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_bsearch_empty_test)
{
	struct int_array a;

	int_array_init(&a);

	for(size_t i = 0; i < 100; i++)
		ASSERT_NULL(int_array_bsearch(&a, i));

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_bsearch_test)
{
	struct int_array a;

	int_array_init(&a);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_append(&a, i), 0);

	for(size_t i = 0; i < 100; i++)
		ASSERT_EQUALS(int_array_bsearch(&a, i), &a.elements[i]);

	ASSERT_NULL(int_array_bsearch(&a, 100));
	ASSERT_NULL(int_array_bsearch(&a, -1));

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(scalar_bsearch_first_test)
{
	struct int_array a;

	int_array_init(&a);

	ASSERT_EQUALS(int_array_append(&a, 0), 0); /* 0 */
	ASSERT_EQUALS(int_array_append(&a, 0), 0); /* 1 */
	ASSERT_EQUALS(int_array_append(&a, 1), 0); /* 2 */
	ASSERT_EQUALS(int_array_append(&a, 1), 0); /* 3 */
	ASSERT_EQUALS(int_array_append(&a, 1), 0); /* 4 */
	ASSERT_EQUALS(int_array_append(&a, 2), 0); /* 5 */
	ASSERT_EQUALS(int_array_append(&a, 2), 0); /* 6 */
	ASSERT_EQUALS(int_array_append(&a, 2), 0); /* 7 */
	ASSERT_EQUALS(int_array_append(&a, 3), 0); /* 8 */
	ASSERT_EQUALS(int_array_append(&a, 9), 0); /* 9 */
	ASSERT_EQUALS(int_array_append(&a, 9), 0); /* 10 */
	ASSERT_EQUALS(int_array_append(&a, 9), 0); /* 11 */
	ASSERT_EQUALS(int_array_append(&a, 9), 0); /* 12 */
	ASSERT_EQUALS(int_array_append(&a, 9), 0); /* 13 */
	ASSERT_EQUALS(int_array_append(&a, 11), 0);/* 14 */

	ASSERT_NULL(int_array_bsearch_first(&a, -1));
	ASSERT_EQUALS(int_array_bsearch_first(&a, 0), &a.elements[0]);
	ASSERT_EQUALS(int_array_bsearch_first(&a, 1), &a.elements[2]);
	ASSERT_EQUALS(int_array_bsearch_first(&a, 2), &a.elements[5]);
	ASSERT_EQUALS(int_array_bsearch_first(&a, 3), &a.elements[8]);
	ASSERT_NULL(int_array_bsearch_first(&a, 4));
	ASSERT_NULL(int_array_bsearch_first(&a, 5));
	ASSERT_NULL(int_array_bsearch_first(&a, 6));
	ASSERT_NULL(int_array_bsearch_first(&a, 7));
	ASSERT_NULL(int_array_bsearch_first(&a, 8));
	ASSERT_EQUALS(int_array_bsearch_first(&a, 9), &a.elements[9]);
	ASSERT_NULL(int_array_bsearch_first(&a, 10));
	ASSERT_EQUALS(int_array_bsearch_first(&a, 11), &a.elements[14]);
	ASSERT_NULL(int_array_bsearch_first(&a, 12));
	ASSERT_NULL(int_array_bsearch_first(&a, 13));

	int_array_destroy(&a);
}
END_TEST()

UNIT_TEST(composed_bsearch_test)
{
	struct composed_array a;
	struct composed e;

	composed_array_init(&a);

	for(size_t i = 0; i < 100; i++) {
		e.a = 2*i;
		e.b = 263*i;
		e.c.i = 917*i;
		e.c.j = 4289*i;

		ASSERT_EQUALS(composed_array_append(&a, e), 0);
	}

	for(size_t i = 0; i < 100; i++) {
		ASSERT_EQUALS(composed_array_bsearch_a(&a, 2*i), &a.elements[i]);
		ASSERT_EQUALS(composed_array_bsearch_b(&a, 263*i), &a.elements[i]);
		ASSERT_EQUALS(composed_array_bsearch_ci(&a, 917*i), &a.elements[i]);
		ASSERT_EQUALS(composed_array_bsearch_cj(&a, 4289*i), &a.elements[i]);
		ASSERT_EQUALS(composed_array_bsearch_aplusb(&a, 265*i), &a.elements[i]);
	}

	composed_array_destroy(&a);
}
END_TEST()

UNIT_TEST_SUITE(typed_array_test)
{
	ADD_TEST(scalar_empty_test);
	ADD_TEST(scalar_test);
	ADD_TEST(scalar_insertpos_test);
	ADD_TEST(scalar_insert_test);
	ADD_TEST(scalar_insert_sorted_test);
	ADD_TEST(scalar_remove_last_test);
	ADD_TEST(scalar_remove_first_test);
	ADD_TEST(scalar_remove_mid_test);
	ADD_TEST(struct_test);
	ADD_TEST(scalar_bsearch_empty_test);
	ADD_TEST(scalar_bsearch_test);
	ADD_TEST(scalar_bsearch_first_test);
	ADD_TEST(composed_bsearch_test);
}
END_TEST_SUITE()
