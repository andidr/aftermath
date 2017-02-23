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
#include "../src/interval_array.h"

struct augmented_interval {
	struct am_interval interval;
	int value;
};

DECL_TYPED_ARRAY(augint_array, struct augmented_interval)
DECL_INTERVAL_EVENT_ARRAY_BSEARCH_FIRST_OVERLAPPING(augint_array, struct augmented_interval, interval)

UNIT_TEST(first_overlap_test)
{
	struct augmented_interval intervals[] = {
		{{   0,   99},  0},
		{{ 100,  199},  1},
		{{ 200,  299},  3},
		{{ 300,  399},  4},
		{{ 400,  499},  5},
		{{ 500,  599},  6},
		{{ 600,  699},  7},
		{{ 700,  799},  8},
		{{ 800,  899},  9},
		{{ 900,  999}, 10},
		{{1000, 1099}, 11}
	};

	size_t n = ARRAY_SIZE(intervals);
	off_t stride = sizeof(struct augmented_interval);

	struct am_interval query;
	struct am_interval* base = &intervals[0].interval;

	/* Exact overlap
	 *
	 * ref:   [     i     ]
	 * query: [           ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100;
		query.end = i*100+99;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i].interval);
	}

	/* Partial overlap: self
	 *
	 * ref:   [     i     ]
	 * query:   [       ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100+1;
		query.end = i*100+99-1;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i].interval);
	}

	/* Partial overlap: left
	 *
	 * ref:   [    i-1    ][     i     ]
	 * query:           [         ]
	 */
	for(size_t i = 1; i < n; i++) {
		query.start = i*100-1;
		query.end = i*100+99-1;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i-1].interval);
	}

	/* Partial overlap: right
	 *
	 * ref:   [     i     ][    i+1    ]
	 * query:      [         ]
	 */
	for(size_t i = 0; i < n-1; i++) {
		query.start = i*100;
		query.end = i*100+99+1;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i].interval);
	}

	/* Full overlap: left + right
	 *
	 * ref:     [     i     ][    i+1    ]
	 * query: [                            ]
	 */
	for(size_t i = 1; i < n-1; i++) {
		query.start = i*100-1;
		query.end = i*100+99+1;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i-1].interval);
	}

	/* Exact overlap: [0..i]
	 *
	 * ref:   [     0     ][     1     ]...[     i     ]
	 * query: [                                        ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = 0;
		query.end = i*100+99;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[0].interval);
	}

	/* Exact overlap: [0..i]
	 *
	 * ref:   [     0     ]...[     i     ][    i+1    ]...[     n     ]
	 * query:                 [                                        ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100;
		query.end = (n-1)*100+99;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i].interval);
	}
}
END_TEST()

UNIT_TEST(typed_first_overlap_test)
{
	struct augint_array a;
	struct am_interval query;
	struct augmented_interval intervals[] = {
		{{   0,   99},  0},
		{{ 100,  199},  1},
		{{ 200,  299},  3},
		{{ 300,  399},  4},
		{{ 400,  499},  5},
		{{ 500,  599},  6},
		{{ 600,  699},  7},
		{{ 700,  799},  8},
		{{ 800,  899},  9},
		{{ 900,  999}, 10},
		{{1000, 1099}, 11}
	};

	size_t n = ARRAY_SIZE(intervals);

	augint_array_init(&a);

	for(size_t i = 0; i < n; i++)
		ASSERT_EQUALS(augint_array_appendp(&a, &intervals[i]), 0);


	/* Exact overlap
	 *
	 * ref:   [     i     ]
	 * query: [           ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100;
		query.end = i*100+99;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i]);
	}

	/* Partial overlap: self
	 *
	 * ref:   [     i     ]
	 * query:   [       ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100+1;
		query.end = i*100+99-1;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i]);
	}

	/* Partial overlap: left
	 *
	 * ref:   [    i-1    ][     i     ]
	 * query:           [         ]
	 */
	for(size_t i = 1; i < n; i++) {
		query.start = i*100-1;
		query.end = i*100+99-1;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i-1]);
	}

	/* Partial overlap: right
	 *
	 * ref:   [     i     ][    i+1    ]
	 * query:      [         ]
	 */
	for(size_t i = 0; i < n-1; i++) {
		query.start = i*100;
		query.end = i*100+99+1;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i]);
	}

	/* Full overlap: left + right
	 *
	 * ref:     [     i     ][    i+1    ]
	 * query: [                            ]
	 */
	for(size_t i = 1; i < n-1; i++) {
		query.start = i*100-1;
		query.end = i*100+99+1;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i-1]);
	}

	/* Exact overlap: [0..i]
	 *
	 * ref:   [     0     ][     1     ]...[     i     ]
	 * query: [                                        ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = 0;
		query.end = i*100+99;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[0]);
	}

	/* Exact overlap: [0..i]
	 *
	 * ref:   [     0     ]...[     i     ][    i+1    ]...[     n     ]
	 * query:                 [                                        ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100;
		query.end = (n-1)*100+99;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i]);
	}

	augint_array_destroy(&a);
}
END_TEST()

UNIT_TEST(first_overlap_gap_test)
{
	struct augmented_interval intervals[] = {
		{{  10,   90},  0},
		{{ 110,  190},  1},
		{{ 210,  290},  3},
		{{ 310,  390},  4},
		{{ 410,  490},  5},
		{{ 510,  590},  6},
		{{ 610,  690},  7},
		{{ 710,  790},  8},
		{{ 810,  890},  9},
		{{ 910,  990}, 10},
		{{1010, 1090}, 11}
	};

	size_t n = ARRAY_SIZE(intervals);
	off_t stride = sizeof(struct augmented_interval);

	struct am_interval query;
	struct am_interval* base = &intervals[0].interval;

	/* Gaps
	 *
	 * ref:     [     i     ]     [    i+1    ]
	 * query:                  [ ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100;
		query.end = i*100+9;
		ASSERT_NULL(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query));
	}

	/* Gap + partial overlap right
	 *
	 * ref:     [     i     ]     [    i+1    ]
	 * query:                  [      ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100;
		query.end = i*100+50;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i].interval);
	}

	/* Gap + partial overlap left
	 *
	 * ref:     [     i     ]     [    i+1    ]
	 * query:            [      ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = (i+1)*100-50;
		query.end = (i+1)*100;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i].interval);
	}

	/* Gap + partial overlap both
	 *
	 * ref:     [     i     ]     [    i+1    ]
	 * query:         [                 ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = (i+1)*100-50;
		query.end = (i+1)*100+50;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i].interval);
	}

	/* Exact overlap: [0..i]
	 *
	 * ref:     [     0     ][     1     ]...[     i     ]
	 * query: [                                          ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = 0;
		query.end = i*100+90;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[0].interval);
	}

	/* Exact overlap: [0..i]
	 *
	 * ref:   [     0     ]...[     i     ][    i+1    ]...[     n     ]
	 * query:                 [                                        ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100+10;
		query.end = (n-1)*100+90;
		ASSERT_EQUALS_PTR(interval_array_bsearch_first_strided_overlapping(base, n, stride, &query), &intervals[i].interval);
	}
}
END_TEST()

UNIT_TEST(typed_first_overlap_gap_test)
{
	struct augint_array a;
	struct am_interval query;
	struct augmented_interval intervals[] = {
		{{  10,   90},  0},
		{{ 110,  190},  1},
		{{ 210,  290},  3},
		{{ 310,  390},  4},
		{{ 410,  490},  5},
		{{ 510,  590},  6},
		{{ 610,  690},  7},
		{{ 710,  790},  8},
		{{ 810,  890},  9},
		{{ 910,  990}, 10},
		{{1010, 1090}, 11}
	};

	size_t n = ARRAY_SIZE(intervals);

	augint_array_init(&a);

	for(size_t i = 0; i < n; i++)
		ASSERT_EQUALS(augint_array_appendp(&a, &intervals[i]), 0);

	/* Gaps
	 *
	 * ref:     [     i     ]     [    i+1    ]
	 * query:                  [ ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100;
		query.end = i*100+9;
		ASSERT_NULL(augint_array_bsearch_first_overlapping(&a, &query));
	}

	/* Gap + partial overlap right
	 *
	 * ref:     [     i     ]     [    i+1    ]
	 * query:                  [      ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100;
		query.end = i*100+50;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i]);
	}

	/* Gap + partial overlap left
	 *
	 * ref:     [     i     ]     [    i+1    ]
	 * query:            [      ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = (i+1)*100-50;
		query.end = (i+1)*100;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i]);
	}

	/* Gap + partial overlap both
	 *
	 * ref:     [     i     ]     [    i+1    ]
	 * query:         [                 ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = (i+1)*100-50;
		query.end = (i+1)*100+50;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i]);
	}

	/* Exact overlap: [0..i]
	 *
	 * ref:     [     0     ][     1     ]...[     i     ]
	 * query: [                                          ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = 0;
		query.end = i*100+90;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[0]);
	}

	/* Exact overlap: [0..i]
	 *
	 * ref:   [     0     ]...[     i     ][    i+1    ]...[     n     ]
	 * query:                 [                                        ]
	 */
	for(size_t i = 0; i < n; i++) {
		query.start = i*100+10;
		query.end = (n-1)*100+90;
		ASSERT_EQUALS_PTR(augint_array_bsearch_first_overlapping(&a, &query), &a.elements[i]);
	}

	augint_array_destroy(&a);
}
END_TEST()

UNIT_TEST_SUITE(interval_test)
	ADD_TEST(first_overlap_test);
	ADD_TEST(typed_first_overlap_test);
	ADD_TEST(first_overlap_gap_test);
	ADD_TEST(typed_first_overlap_gap_test);
END_TEST_SUITE()
