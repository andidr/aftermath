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
#include "../src/state_index.h"
#include "../src/event_set.h"

void __add_state_event(struct event_set* es, int id, uint64_t start, uint64_t end)
{
	struct state_event se;

	se.event_set = es;
	se.active_task = NULL;
	se.active_frame = NULL;
	se.texec_start = NULL;
	se.texec_end = NULL;

	se.start = start;
	se.end = end;
	se.state_id = id;
	se.state_id_seq = id;

	ASSERT_EQUALS(event_set_add_state_event(es, &se), 0);
}

void __build_test_set(struct event_set* es, size_t num_states, size_t num_events)
{
	for(size_t i = 0; i < num_events; i++)
		__add_state_event(es, i % num_states, (i+1)*1000000, (i+1)*1000000+999999);
}

UNIT_TEST(init_test)
{
	struct event_set es;
	struct state_index idx;
	size_t num_states = 10;
	size_t num_events = 1000;

	event_set_init(&es, 0);
	__build_test_set(&es, num_states, num_events);

	ASSERT_EQUALS(state_index_init(&idx, num_states, &es, 10), 0);

	state_index_destroy(&idx);
	event_set_destroy(&es);
}
END_TEST()

UNIT_TEST(update_test)
{
	struct event_set es;
	struct state_index idx;
	size_t num_states = 10;
	size_t num_events = 1000;

	event_set_init(&es, 0);
	__build_test_set(&es, num_states, num_events);

	ASSERT_EQUALS(state_index_init(&idx, num_states, &es, 10), 0);
	state_index_update(&idx, &es, NULL);

	state_index_destroy(&idx);
	event_set_destroy(&es);
}
END_TEST()

UNIT_TEST(empty_update_test)
{
	struct event_set es;
	struct state_index idx;

	event_set_init(&es, 0);

	ASSERT_EQUALS(state_index_init(&idx, 0, &es, 10), 0);
	state_index_update(&idx, &es, NULL);

	state_index_destroy(&idx);
	event_set_destroy(&es);
}
END_TEST()

UNIT_TEST(duration_test)
{
	struct event_set es;
	struct state_index idx;
	size_t num_states = 10;
	size_t num_events = 1000;
	uint64_t durations[num_states];

	event_set_init(&es, 0);
	__build_test_set(&es, num_states, num_events);

	ASSERT_EQUALS(state_index_init(&idx, num_states, &es, 10), 0);
	state_index_update(&idx, &es, NULL);

	/* Before first state event */
	ASSERT_EQUALS(state_index_get_state_durations(&idx, &es, NULL,
						      0,
						      999999,
						      durations, 1, 0), 0);

	/* After last state event */
	ASSERT_EQUALS(state_index_get_state_durations(&idx, &es, NULL,
						      (num_events+1)*1000000,
						      (num_events+1)*1000000 + 999999,
						      durations, 1, 0), 0);

	/* Part of the first event */
	ASSERT_EQUALS(state_index_get_state_durations(&idx, &es, NULL,
						      1000000,
						      1500000,
						      durations, 1, 0), 1);
	ASSERT_EQUALS(durations[0], 500000);

	for(size_t i = 1; i < num_states; i++)
		ASSERT_EQUALS(durations[i], 0);

	/* Exactly the first event */
	ASSERT_EQUALS(state_index_get_state_durations(&idx, &es, NULL,
						      1000000,
						      1999999,
						      durations, 1, 0), 1);
	ASSERT_EQUALS(durations[0], 999999);

	for(size_t i = 1; i < num_states; i++)
		ASSERT_EQUALS(durations[i], 0);

	/* Half of the first and the second event */
	ASSERT_EQUALS(state_index_get_state_durations(&idx, &es, NULL,
						      1500000,
						      2500000,
						      durations, 1, 0), 1);
	ASSERT_EQUALS(durations[0], 499999);
	ASSERT_EQUALS(durations[1], 500000);

	for(size_t i = 2; i < num_states; i++)
		ASSERT_EQUALS(durations[i], 0);

	/* Everything */
	ASSERT_EQUALS(state_index_get_state_durations(&idx, &es, NULL,
						      0,
						      (num_events+1)*1000000,
						      durations, 1, 0), 1);

	for(size_t i = 0; i < num_states; i++)
		ASSERT_EQUALS(durations[i], (num_events / num_states)*999999);


	/* First i state events */
	for(size_t i = 0; i < num_events; i++) {
		ASSERT_EQUALS(state_index_get_state_durations(&idx, &es, NULL,
							      0,
							      (i+1)*1000000+999999,
							      durations, 1, 0), 1);

		for(size_t j = 0; j < num_states; j++)
			ASSERT_EQUALS(durations[j], (i / num_states + ((i % num_states >= j) ? 1 : 0))*999999);
	}

	state_index_destroy(&idx);
	event_set_destroy(&es);
}
END_TEST()

UNIT_TEST_SUITE(monotonic_index_test)
{
	ADD_TEST(init_test);
	ADD_TEST(update_test);
	ADD_TEST(empty_update_test);
	ADD_TEST(duration_test);
}
END_TEST_SUITE()
