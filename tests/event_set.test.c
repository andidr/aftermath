/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
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
#include "../src/event_set.h"
#include <inttypes.h>

int trace_update_task_execution_bounds(struct event_set* es);

/*
 * Add single events for num_tasks tasks to es.
 */
void __create_tasks(struct event_set* es, int num_tasks, uint64_t* durations, uint64_t gap, uint64_t curr_time)
{
	struct single_event sge;

	/* Create set of single events for the beginning and end of
	 * the execution of the tasks. */
	for(int i = 0; i < 2*num_tasks; i++) {
		sge.event_set = NULL;
		sge.active_task = NULL;
		sge.active_frame = NULL;
		sge.what = NULL;
		sge.prev_texec_start = NULL;
		sge.prev_texec_end = NULL;
		sge.next_texec_start = NULL;
		sge.next_texec_end = NULL;
		sge.time = curr_time;

		if(i % 2 == 0) {
			/* Chaining for single events for the
			 * beginning of the execution of a task. */
			sge.type = SINGLE_TYPE_TEXEC_START;
			curr_time += durations[i/2];
		} else {
			/* Chaining for single events for the end of
			 * the execution of a task. */
			sge.type = SINGLE_TYPE_TEXEC_END;
			curr_time += gap;
		}

		ASSERT_EQUALS(event_set_add_single_event(es, &sge), 0);
	}

	ASSERT_EQUALS(trace_update_task_execution_bounds(es), 0);
}

/*
 * Initializes an empty event set and destroys it. No assertions are
 * used, the test is only useful when run with valgrind.
 */
UNIT_TEST(init_destroy_empty_test)
{
	struct event_set es;

	event_set_init(&es, 0);
	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds a single state event to an empty event set.
 */
UNIT_TEST(add_single_state_event_test)
{
	struct event_set es;
	struct state_event se;

	event_set_init(&es, 0);

	se.event_set = NULL;
	se.start = 100;
	se.end = 1000;
	se.active_task = NULL;
	se.active_frame = NULL;
	se.state = 1;
	se.texec_start = NULL;
	se.texec_end = NULL;

	ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds multiple state events to an empty event set.
 */
UNIT_TEST(add_multi_state_event_test)
{
	struct event_set es;
	struct state_event se;

	event_set_init(&es, 0);

	se.event_set = NULL;
	se.active_task = NULL;
	se.active_frame = NULL;
	se.texec_start = NULL;
	se.texec_end = NULL;
	se.state = 1;

	for(int i = 0; i < 1000; i++) {
		se.start = i*1000;
		se.end = i*1000+999;

		ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds multiple state events to an empty event set and checks if
 * get_first_state_in_interval returns the correct index for different
 * intervals.
 */
UNIT_TEST(get_first_state_in_interval_test)
{
	struct event_set es;
	struct state_event se;

	event_set_init(&es, 0);

	for(int i = 0; i < 10; i++) {
		se.event_set = NULL;
		se.start = (i+1)*1000;
		se.end = (i+1)*1000+999;
		se.active_task = NULL;
		se.active_frame = NULL;
		se.state = i;
		se.texec_start = NULL;
		se.texec_end = NULL;

		ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);
	}

	/* Match outside, before first state */
	ASSERT_EQUALS(event_set_get_first_state_in_interval(&es, 0, 999), -1);

	/* Match outside, after last state */
	ASSERT_EQUALS(event_set_get_first_state_in_interval(&es, 11000, UINT64_MAX), -1);

	/* Exact matches */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_in_interval(&es, (i+1)*1000, (i+1)*1000+999), i);

	/* Exact match on beginning */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_in_interval(&es, (i+1)*1000, UINT64_MAX), i);

	/* Matches in the middle of each state */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_in_interval(&es, (i+1)*1000+500, UINT64_MAX), i);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds multiple state events to an empty event set and checks if
 * get_first_state_starting_in_interval returns the correct index for
 * different intervals.
 */
UNIT_TEST(get_first_state_starting_in_interval_test)
{
	struct event_set es;
	struct state_event se;

	event_set_init(&es, 0);

	for(int i = 0; i < 10; i++) {
		se.event_set = NULL;
		se.start = (i+1)*1000;
		se.end = (i+1)*1000+999;
		se.active_task = NULL;
		se.active_frame = NULL;
		se.state = i;
		se.texec_start = NULL;
		se.texec_end = NULL;

		ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);
	}

	/* Match outside, before first state */
	ASSERT_EQUALS(event_set_get_first_state_starting_in_interval(&es, 0, 999), -1);

	/* Match outside, after last state */
	ASSERT_EQUALS(event_set_get_first_state_starting_in_interval(&es, 11000, UINT64_MAX), -1);

	/* Exact matches */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval(&es, (i+1)*1000, (i+1)*1000+999), i);

	/* Exact match on beginning */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval(&es, (i+1)*1000, UINT64_MAX), i);

	/* Matches in the middle of each state, should return next
	 * state */
	for(int i = 0; i < 9; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval(&es, (i+1)*1000+500, UINT64_MAX), i+1);

	/* Query on intervals in which no state event starts */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval(&es, (i+1)*1000+1, (i+1)*1000+999), -1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds multiple state events to an empty event set and checks if
 * get_first_state_starting_in_interval_type returns the correct index
 * for different intervals.
 */
UNIT_TEST(get_first_state_starting_in_interval_type_test)
{
	struct event_set es;
	struct state_event se;

	event_set_init(&es, 0);

	for(int i = 0; i < 10; i++) {
		se.event_set = NULL;
		se.start = (i+1)*1000;
		se.end = (i+1)*1000+999;
		se.active_task = NULL;
		se.active_frame = NULL;
		se.state = i;
		se.texec_start = NULL;
		se.texec_end = NULL;

		ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);
	}

	/* Match outside, before first state */
	for(int type = 0; type < 10; type++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval_type(&es, 0, 999, type), -1);

	/* Match outside, after last state */
	for(int type = 0; type < 10; type++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval_type(&es, 11000, UINT64_MAX, type), -1);

	/* Exact matches */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval_type(&es, (i+1)*1000, (i+1)*1000+999, i), i);

	/* Exact matches with wrong type */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval_type(&es, (i+1)*1000, (i+1)*1000+999, i+1), -1);

	/* Exact match on beginning */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval_type(&es, (i+1)*1000, UINT64_MAX, i), i);

	/* Matches in the middle of each state, should return -1 */
	for(int i = 0; i < 9; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval_type(&es, (i+1)*1000+500, UINT64_MAX, i), -1);

	/* Matching on entire interval */
	for(int type = 0; type < 10; type++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval_type(&es, 0, UINT64_MAX, type), type);

	/* Query on intervals in which no state event starts */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_first_state_starting_in_interval_type(&es, (i+1)*1000+1, (i+1)*1000+999, i), -1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds multiple state events to an empty event set and checks if
 * get_next_state_event returns the correct index for different
 * intervals.
 */
UNIT_TEST(get_next_state_event_test)
{
	struct event_set es;
	struct state_event se;

	event_set_init(&es, 0);

	se.event_set = NULL;
	se.active_task = NULL;
	se.active_frame = NULL;
	se.texec_start = NULL;
	se.texec_end = NULL;

	for(int i = 0; i < 10; i++) {
		se.start = (i+1)*1000;
		se.end = (i+1)*1000+999;
		se.state = i;

		ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);
	}

	for(int idx = -1; idx < 9; idx++)
		ASSERT_EQUALS(event_set_get_next_state_event(&es, idx, idx+1), idx+1);

	/* Wrong type */
	for(int idx = 0; idx < 10; idx++)
		ASSERT_EQUALS(event_set_get_next_state_event(&es, idx, idx), -1);

	/* After the last index */
	for(int type = 0; type < 10; type++)
		ASSERT_EQUALS(event_set_get_next_state_event(&es, 10, type), -1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Tests event_set_get_major_state on a set without state events
 */
UNIT_TEST(get_major_state_empty_test)
{
	struct event_set es;
	int major_state;

	event_set_init(&es, 0);

	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 0, 1000, &major_state), 0);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Tests event_set_get_major_state on a set with a single state event.
 */
UNIT_TEST(get_major_state_one_event_test)
{
	struct event_set es;
	struct state_event se;
	int major_state;

	event_set_init(&es, 0);

	se.event_set = NULL;
	se.active_task = NULL;
	se.active_frame = NULL;
	se.texec_start = NULL;
	se.texec_end = NULL;
	se.start = 1000;
	se.end = 1999;
	se.state = 0;
	ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);

	/* Exact match */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 1000, 1999, &major_state), 1);
	ASSERT_EQUALS(major_state, 0);

	/* Exact match at the start, end somewhere in the middle */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 1000, 1500, &major_state), 1);
	ASSERT_EQUALS(major_state, 0);

	/* Start and end somewhere in the middle */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 1250, 1500, &major_state), 1);
	ASSERT_EQUALS(major_state, 0);

	/* Start before event, end after the event */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 0, 3000, &major_state), 1);
	ASSERT_EQUALS(major_state, 0);

	/* Entirely before the event */
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 0, 999, &major_state), 0);

	/* Entirely after the event */
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 3000, 30000, &major_state), 0);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Tests event_set_get_major_state on a set with two state events.
 */
UNIT_TEST(get_major_state_two_events_test)
{
	struct event_set es;
	struct state_event se;
	int major_state;

	event_set_init(&es, 0);

	se.event_set = NULL;
	se.active_task = NULL;
	se.active_frame = NULL;
	se.texec_start = NULL;
	se.texec_end = NULL;

	se.start = 1000;
	se.end = 1999;
	se.state = 0;
	ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);

	se.start = 2000;
	se.end = 2999;
	se.state = 1;
	ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);

	/* Leave out just one cycle of the first event */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 1001, 2999, &major_state), 1);
	ASSERT_EQUALS(major_state, 1);

	/* Leave out just one cycle of the second event */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 1000, 2998, &major_state), 1);
	ASSERT_EQUALS(major_state, 0);

	/* Half of both events with one cycle more for the first
	 * event */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 1499, 2500, &major_state), 1);
	ASSERT_EQUALS(major_state, 0);

	/* Half of both events with one cycle more for the second
	 * event */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 1500, 2500, &major_state), 1);
	ASSERT_EQUALS(major_state, 1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Tests event_set_get_major_state on a set with lots of short state
 * events
 */
UNIT_TEST(get_major_state_short_events_test)
{
	struct event_set es;
	struct state_event se;
	int major_state;

	event_set_init(&es, 0);

	se.event_set = NULL;
	se.active_task = NULL;
	se.active_frame = NULL;
	se.texec_start = NULL;
	se.texec_end = NULL;

	/* Add lots of short events of the same duration, alternating
	 * between type 0 and 1. Type 0 has one more event than type
	 * 1. */
	for(int i = 0; i < 1001; i++) {
		se.start = (i+1)*1000;
		se.end = (i+1)*1000 + 999;
		se.state = (i % 2);
		ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);
	}

	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 1000, 1001999, &major_state), 1);
	ASSERT_EQUALS(major_state, 0);

	/* Leave out first and last event, such that type 1 becomes dominant */
	major_state = -1;
	ASSERT_EQUALS(event_set_get_major_state(&es, NULL, 2000, 1000999, &major_state), 1);
	ASSERT_EQUALS(major_state, 1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_enclosing_state using a single state event.
 */
UNIT_TEST(get_enclosing_state_test)
{
	struct event_set es;
	struct state_event se;

	event_set_init(&es, 0);

	/* Before adding the state event */
	ASSERT_EQUALS(event_set_get_enclosing_state(&es, 0), -1);
	ASSERT_EQUALS(event_set_get_enclosing_state(&es, 1000), -1);

	/* Add single state event */
	se.event_set = NULL;
	se.active_task = NULL;
	se.active_frame = NULL;
	se.texec_start = NULL;
	se.texec_end = NULL;
	se.start = 1000;
	se.end = 1999;
	se.state = 0;
	ASSERT_EQUALS(event_set_add_state_event(&es, &se), 0);

	/* Before the state event */
	ASSERT_EQUALS(event_set_get_enclosing_state(&es, 999), -1);

	/* during the state event */
	ASSERT_EQUALS(event_set_get_enclosing_state(&es, 1000), 0);
	ASSERT_EQUALS(event_set_get_enclosing_state(&es, 1500), 0);
	ASSERT_EQUALS(event_set_get_enclosing_state(&es, 1999), 0);

	/* After the state event */
	ASSERT_EQUALS(event_set_get_enclosing_state(&es, 2000), -1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds a single communication event to an empty event set.
 */
UNIT_TEST(add_single_comm_event_test)
{
	struct event_set es;
	struct comm_event ce;

	event_set_init(&es, 0);

	ce.event_set = NULL;
	ce.time = 100;
	ce.dst_cpu = 1;
	ce.src_cpu = 0;
	ce.size = 1234;
	ce.what = NULL;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = NULL;
	ce.texec_end = NULL;
	ce.prod_ts = 0;
	ce.type = COMM_TYPE_STEAL;

	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds multiple communication events to an empty event set.
 */
UNIT_TEST(add_multi_comm_event_test)
{
	struct event_set es;
	struct comm_event ce;

	event_set_init(&es, 0);
	ce.event_set = NULL;
	ce.dst_cpu = 1;
	ce.src_cpu = 0;
	ce.size = 1234;
	ce.what = NULL;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = NULL;
	ce.texec_end = NULL;
	ce.prod_ts = 0;
	ce.type = COMM_TYPE_STEAL;

	for(int i = 0; i < 1000; i++) {
		ce.time = 1000*i;
		ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds multiple communication events to an empty event set and checks
 * whether event_set_get_next_comm_event returns the correct indices.
 */
UNIT_TEST(get_next_comm_event_test)
{
	struct event_set es;
	struct comm_event ce;

	event_set_init(&es, 0);
	ce.event_set = NULL;
	ce.dst_cpu = 1;
	ce.src_cpu = 0;
	ce.size = 1234;
	ce.what = NULL;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = NULL;
	ce.texec_end = NULL;
	ce.prod_ts = 0;

	ce.time = 1000;
	ce.type = COMM_TYPE_STEAL;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 2000;
	ce.type = COMM_TYPE_PUSH;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 3000;
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 4000;
	ce.type = COMM_TYPE_DATA_WRITE;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Beginning of the array */
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, -1, COMM_TYPE_STEAL), 0);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, -1, COMM_TYPE_PUSH), 1);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, -1, COMM_TYPE_DATA_READ), 2);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, -1, COMM_TYPE_DATA_WRITE), 3);

	/* Start search at the indexes with the appropriate types */
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, 0, COMM_TYPE_STEAL), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, 1, COMM_TYPE_PUSH), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, 2, COMM_TYPE_DATA_READ), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, 3, COMM_TYPE_DATA_WRITE), -1);

	/* Start search at the indexes after those with the
	 * appropriate types */
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, 1, COMM_TYPE_STEAL), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, 2, COMM_TYPE_PUSH), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, 3, COMM_TYPE_DATA_READ), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event(&es, 4, COMM_TYPE_DATA_WRITE), -1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds multiple communication events to an empty event set and checks
 * whether event_set_get_next_comm_event_arr returns the correct
 * indices.
 */
UNIT_TEST(get_next_comm_event_arr_test)
{
	struct event_set es;
	struct comm_event ce;

	event_set_init(&es, 0);
	ce.event_set = NULL;
	ce.dst_cpu = 1;
	ce.src_cpu = 0;
	ce.size = 1234;
	ce.what = NULL;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = NULL;
	ce.texec_end = NULL;
	ce.prod_ts = 0;

	ce.time = 1000;
	ce.type = COMM_TYPE_STEAL;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 2000;
	ce.type = COMM_TYPE_PUSH;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 3000;
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 4000;
	ce.type = COMM_TYPE_DATA_WRITE;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	enum comm_event_type tsteal = COMM_TYPE_STEAL;
	enum comm_event_type tpush = COMM_TYPE_PUSH;
	enum comm_event_type tread = COMM_TYPE_DATA_READ;
	enum comm_event_type twrite = COMM_TYPE_DATA_WRITE;

	/* Single type, beginning of the array */
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, -1, 1, &tsteal), 0);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, -1, 1, &tpush), 1);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, -1, 1, &tread), 2);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, -1, 1, &twrite), 3);

	/* Single type, start search at the indexes with the
	 * appropriate types */
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 0, 1, &tsteal), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 1, 1, &tpush), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 2, 1, &tread), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 3, 1, &twrite), -1);

	/* Single type, start search at the indexes after those with the
	 * appropriate types */
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 1, 1, &tsteal), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 2, 1, &tpush), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 3, 1, &tread), -1);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 4, 1, &twrite), -1);

	/* Two types */
	enum comm_event_type tsteal_push[] = { COMM_TYPE_STEAL, COMM_TYPE_PUSH };
	enum comm_event_type tpush_steal[] = { COMM_TYPE_PUSH, COMM_TYPE_STEAL };
	enum comm_event_type tread_write[] = { COMM_TYPE_DATA_READ, COMM_TYPE_DATA_WRITE };
	enum comm_event_type twrite_read[] = { COMM_TYPE_DATA_WRITE, COMM_TYPE_DATA_READ };

	/* Beginning of the array */
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, -1, 2, tsteal_push), 0);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, -1, 2, tpush_steal), 0);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, -1, 2, tread_write), 2);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, -1, 2, twrite_read), 2);

	/* Somwhere in the middle of the array */
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 1, 2, tread_write), 2);
	ASSERT_EQUALS(event_set_get_next_comm_event_arr(&es, 1, 2, twrite_read), 2);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_first_comm_in_interval using a set of
 * communication events.
 */
UNIT_TEST(get_first_comm_in_interval_test)
{
	struct event_set es;
	struct comm_event ce;

	event_set_init(&es, 0);

	ce.event_set = NULL;
	ce.dst_cpu = 1;
	ce.src_cpu = 0;
	ce.size = 1234;
	ce.what = NULL;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = NULL;
	ce.texec_end = NULL;
	ce.prod_ts = 0;

	ce.time = 1000;
	ce.type = COMM_TYPE_STEAL;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 2000;
	ce.type = COMM_TYPE_PUSH;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 3000;
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 4000;
	ce.type = COMM_TYPE_DATA_WRITE;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Interval without communication */
	ASSERT_EQUALS(event_set_get_first_comm_in_interval(&es, 0, 100), -1);

	/* First event only */
	ASSERT_EQUALS(event_set_get_first_comm_in_interval(&es, 1000, 1500), 0);

	/* First event only, exact match */
	ASSERT_EQUALS(event_set_get_first_comm_in_interval(&es, 1000, 1000), 0);

	/* First event only, exact match at the start of the interval */
	ASSERT_EQUALS(event_set_get_first_comm_in_interval(&es, 1000, 1100), 0);

	/* First event only, exact match at the end of the interval */
	ASSERT_EQUALS(event_set_get_first_comm_in_interval(&es, 900, 1000), 0);

	/* First and second event */
	ASSERT_EQUALS(event_set_get_first_comm_in_interval(&es, 500, 2500), 0);

	/* First and second and third event */
	ASSERT_EQUALS(event_set_get_first_comm_in_interval(&es, 500, 3500), 0);

	/* Second and third event */
	ASSERT_EQUALS(event_set_get_first_comm_in_interval(&es, 1500, 3500), 1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_last_comm_event_in_interval using a set of
 * communication events.
 */
UNIT_TEST(get_last_comm_in_interval_test)
{
	struct event_set es;
	struct comm_event ce;

	event_set_init(&es, 0);

	ce.event_set = NULL;
	ce.dst_cpu = 1;
	ce.src_cpu = 0;
	ce.size = 1234;
	ce.what = NULL;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = NULL;
	ce.texec_end = NULL;
	ce.prod_ts = 0;

	ce.time = 1000;
	ce.type = COMM_TYPE_STEAL;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 2000;
	ce.type = COMM_TYPE_PUSH;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 3000;
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 4000;
	ce.type = COMM_TYPE_DATA_WRITE;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Interval without communication */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval(&es, 0, 100), -1);

	/* First event only */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval(&es, 1000, 1500), 0);

	/* First event only, exact match */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval(&es, 1000, 1000), 0);

	/* First event only, exact match at the start of the interval */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval(&es, 1000, 1100), 0);

	/* First event only, exact match at the end of the interval */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval(&es, 900, 1000), 0);

	/* First and second event */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval(&es, 500, 2500), 1);

	/* First and second and third event */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval(&es, 500, 3500), 2);

	/* Second and third event */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval(&es, 1500, 3500), 2);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_last_comm_event_in_interval_type using a set
 * of communication events.
 */
UNIT_TEST(get_last_comm_in_interval_type_test)
{
	struct event_set es;
	struct comm_event ce;

	event_set_init(&es, 0);

	ce.event_set = NULL;
	ce.dst_cpu = 1;
	ce.src_cpu = 0;
	ce.size = 1234;
	ce.what = NULL;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = NULL;
	ce.texec_end = NULL;
	ce.prod_ts = 0;

	ce.time = 1000;
	ce.type = COMM_TYPE_STEAL;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 2000;
	ce.type = COMM_TYPE_PUSH;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 3000;
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 4000;
	ce.type = COMM_TYPE_DATA_WRITE;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 5000;
	ce.type = COMM_TYPE_STEAL;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 6000;
	ce.type = COMM_TYPE_PUSH;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 7000;
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	ce.time = 8000;
	ce.type = COMM_TYPE_DATA_WRITE;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Interval without communication */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 0, 100, COMM_TYPE_STEAL), -1);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 0, 100, COMM_TYPE_PUSH), -1);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 0, 100, COMM_TYPE_DATA_READ), -1);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 0, 100, COMM_TYPE_DATA_WRITE), -1);

	/* First event only */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 1000, 1500, COMM_TYPE_STEAL), 0);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 1000, 1500, COMM_TYPE_PUSH), -1);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 1000, 1500, COMM_TYPE_DATA_READ), -1);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 1000, 1500, COMM_TYPE_DATA_WRITE), -1);

	/* First event only, exact match */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 1000, 1000, COMM_TYPE_STEAL), 0);

	/* First event only, exact match at the start of the interval */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 1000, 1100, COMM_TYPE_STEAL), 0);

	/* First event only, exact match at the end of the interval */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 900, 1000, COMM_TYPE_STEAL), 0);

	/* First and second event */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 2500, COMM_TYPE_STEAL), 0);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 2500, COMM_TYPE_PUSH), 1);

	/* First and second and third event */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 3500, COMM_TYPE_STEAL), 0);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 3500, COMM_TYPE_PUSH), 1);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 3500, COMM_TYPE_DATA_READ), 2);

	/* All events */
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 8500, COMM_TYPE_STEAL), 4);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 8500, COMM_TYPE_PUSH), 5);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 8500, COMM_TYPE_DATA_READ), 6);
	ASSERT_EQUALS(event_set_get_last_comm_event_in_interval_type(&es, 500, 8500, COMM_TYPE_DATA_WRITE), 7);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds a single single event to an empty event set.
 */
UNIT_TEST(add_single_single_event_test)
{
	struct event_set es;
	struct single_event sge;

	event_set_init(&es, 0);

	sge.event_set = NULL;
	sge.time = 100;
	sge.what = NULL;
	sge.active_task = NULL;
	sge.active_frame = NULL;
	sge.next_texec_start = NULL;
	sge.next_texec_end = NULL;
	sge.prev_texec_start = NULL;
	sge.prev_texec_end = NULL;

	ASSERT_EQUALS(event_set_add_single_event(&es, &sge), 0);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds a multiple single events to an empty event set.
 */
UNIT_TEST(add_multi_single_event_test)
{
	struct event_set es;
	struct single_event sge;

	event_set_init(&es, 0);

	sge.event_set = NULL;
	sge.what = NULL;
	sge.active_task = NULL;
	sge.active_frame = NULL;
	sge.next_texec_start = NULL;
	sge.next_texec_end = NULL;
	sge.prev_texec_start = NULL;
	sge.prev_texec_end = NULL;

	for(int i = 0; i < 1000; i++) {
		sge.time = 1000*i;
		ASSERT_EQUALS(event_set_add_single_event(&es, &sge), 0);
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_next_single_event on an event set with four
 * different types of single events.
 */
UNIT_TEST(get_next_single_event_test)
{
	struct event_set es;
	struct single_event sge;

	event_set_init(&es, 0);

	sge.event_set = NULL;
	sge.what = NULL;
	sge.active_task = NULL;
	sge.active_frame = NULL;
	sge.next_texec_start = NULL;
	sge.next_texec_end = NULL;
	sge.prev_texec_start = NULL;
	sge.prev_texec_end = NULL;

	/* Add a thousand events with types 0...3 */
	for(int i = 0; i < 1000; i++) {
		sge.time = 1000*i;
		sge.type = i % 4;
		ASSERT_EQUALS(event_set_add_single_event(&es, &sge), 0);
	}

	/* Find next event of the same type for types 0...3 */
	for(int i = 0; i < 1000-4; i += 4) {
		ASSERT_EQUALS(event_set_get_next_single_event(&es, i, 0), i+4);
		ASSERT_EQUALS(event_set_get_next_single_event(&es, i, 1), i+1);
		ASSERT_EQUALS(event_set_get_next_single_event(&es, i, 2), i+2);
		ASSERT_EQUALS(event_set_get_next_single_event(&es, i, 3), i+3);
	}

	/* End of the array, no following event */
	ASSERT_EQUALS(event_set_get_next_single_event(&es, 996, 0), -1);
	ASSERT_EQUALS(event_set_get_next_single_event(&es, 997, 1), -1);
	ASSERT_EQUALS(event_set_get_next_single_event(&es, 998, 2), -1);
	ASSERT_EQUALS(event_set_get_next_single_event(&es, 999, 3), -1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_first_single_event_in_interval on an event set
 * with 1000 single events of the same type.
 */
UNIT_TEST(get_first_single_event_in_interval_test)
{
	struct event_set es;
	struct single_event sge;

	event_set_init(&es, 0);

	sge.type = SINGLE_TYPE_TCREATE;
	sge.event_set = NULL;
	sge.what = NULL;
	sge.active_task = NULL;
	sge.active_frame = NULL;
	sge.next_texec_start = NULL;
	sge.next_texec_end = NULL;
	sge.prev_texec_start = NULL;
	sge.prev_texec_end = NULL;

	/* Add a thousand events */
	for(int i = 0; i < 1000; i++) {
		sge.time = 1000*(i+1);
		ASSERT_EQUALS(event_set_add_single_event(&es, &sge), 0);
	}

	/* First event, exact match at the beginning of the interval */
	ASSERT_EQUALS(event_set_get_first_single_event_in_interval(&es, 1000, 1001), 0);

	/* First event, exact match at the end of the interval */
	ASSERT_EQUALS(event_set_get_first_single_event_in_interval(&es, 999, 1000), 0);

	/* Second event, exact match at the beginning of the interval */
	ASSERT_EQUALS(event_set_get_first_single_event_in_interval(&es, 2000, 2001), 1);

	/* Second event, exact match at the end of the interval */
	ASSERT_EQUALS(event_set_get_first_single_event_in_interval(&es, 1999, 2000), 1);

	for(int i = 0; i < 1000; i++) {
		/* Exactly one event in the interval */
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval(&es, (i+1)*1000-1, (i+1)*1000+1), i);

		/* Exactly two events in the interval */
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval(&es, (i+1)*1000-1, (i+2)*1000+1), i);

		/* Exactly ten events in the interval */
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval(&es, (i+1)*1000-1, (i+10)*1000+1), i);

	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_last_single_event_in_interval on an event set
 * with 1000 single events of the same type.
 */
UNIT_TEST(get_last_single_event_in_interval_test)
{
	struct event_set es;
	struct single_event sge;

	event_set_init(&es, 0);

	sge.type = SINGLE_TYPE_TCREATE;
	sge.event_set = NULL;
	sge.what = NULL;
	sge.active_task = NULL;
	sge.active_frame = NULL;
	sge.next_texec_start = NULL;
	sge.next_texec_end = NULL;
	sge.prev_texec_start = NULL;
	sge.prev_texec_end = NULL;

	for(int i = 0; i < 1000; i++) {
		sge.time = 1000*(i+1);
		ASSERT_EQUALS(event_set_add_single_event(&es, &sge), 0);
	}

	/* First event, exact match at the beginning of the interval */
	ASSERT_EQUALS(event_set_get_last_single_event_in_interval(&es, 1000, 1001), 0);

	/* First event, exact match at the end of the interval */
	ASSERT_EQUALS(event_set_get_last_single_event_in_interval(&es, 999, 1000), 0);

	/* Second event, exact match at the beginning of the interval */
	ASSERT_EQUALS(event_set_get_last_single_event_in_interval(&es, 2000, 2001), 1);

	/* Second event, exact match at the end of the interval */
	ASSERT_EQUALS(event_set_get_first_single_event_in_interval(&es, 1999, 2000), 1);

	for(int i = 0; i < 1000; i++) {
		/* Exactly one event in the interval */
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval(&es, (i+1)*1000-1, (i+1)*1000+1), i);
	}

	for(int i = 0; i < 999; i++) {
		/* Exactly two events in the interval */
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval(&es, (i+1)*1000-1, (i+2)*1000+1), i+1);
	}

	for(int i = 0; i < 989; i++) {
		/* Exactly ten events in the interval */
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval(&es, (i+1)*1000-1, (i+10)*1000+1), i+9);
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_first_single_event_in_interval_type on an
 * event set with four different types of single events.
 */
UNIT_TEST(get_first_single_event_in_interval_type_test)
{
	struct event_set es;
	struct single_event sge;

	event_set_init(&es, 0);

	sge.event_set = NULL;
	sge.what = NULL;
	sge.active_task = NULL;
	sge.active_frame = NULL;
	sge.next_texec_start = NULL;
	sge.next_texec_end = NULL;
	sge.prev_texec_start = NULL;
	sge.prev_texec_end = NULL;

	/* Add a thousand events with types 0...3 */
	for(int i = 0; i < 1000; i++) {
		sge.time = 1000*(i+1);
		sge.type = i % 4;
		ASSERT_EQUALS(event_set_add_single_event(&es, &sge), 0);
	}

	for(int i = 0; i < 1000; i++) {
		/* Exactly one event in the interval */
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval_type(&es, (i+1)*1000-1, (i+1)*1000+1, i % 4), i);
	}

	for(int i = 0; i < 999; i++) {
		/* Exactly two events in the interval */
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval_type(&es, (i+1)*1000-1, (i+2)*1000+1, i % 4), i);
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval_type(&es, (i+1)*1000-1, (i+2)*1000+1, (i + 1) % 4), i+1);
	}

	for(int i = 0; i < 998; i++) {
		/* Exactly three events in the interval */
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval_type(&es, (i+1)*1000-1, (i+2)*1000+1, i % 4), i);
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval_type(&es, (i+1)*1000-1, (i+2)*1000+1, (i + 1) % 4), i+1);
		ASSERT_EQUALS(event_set_get_first_single_event_in_interval_type(&es, (i+1)*1000-1, (i+2)*1000+1, (i + 2) % 4), i+2);
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks event_set_get_last_single_event_in_interval_type on an event
 * set with four different types of single events.
 */
UNIT_TEST(get_last_single_event_in_interval_type_test)
{
	struct event_set es;
	struct single_event sge;

	event_set_init(&es, 0);

	sge.event_set = NULL;
	sge.what = NULL;
	sge.active_task = NULL;
	sge.active_frame = NULL;
	sge.next_texec_start = NULL;
	sge.next_texec_end = NULL;
	sge.prev_texec_start = NULL;
	sge.prev_texec_end = NULL;

	for(int i = 0; i < 1000; i++) {
		sge.time = 1000*(i+1);
		sge.type = i % 4;
		ASSERT_EQUALS(event_set_add_single_event(&es, &sge), 0);
	}

	for(int i = 0; i < 1000; i++) {
		/* Exactly one event in the interval */
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval_type(&es, (i+1)*1000-1, (i+1)*1000+1, i % 4), i);
	}

	for(int i = 0; i < 999; i++) {
		/* Exactly two events in the interval */
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval_type(&es, (i+1)*1000-1, (i+2)*1000+1, i % 4), i);
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval_type(&es, (i+1)*1000-1, (i+2)*1000+1, (i + 1) % 4), i+1);
	}

	/* First N events */
	for(int i = 999; i >= 0; i -= 4) {
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval_type(&es, 0, (i+1)*1000+1, 0), i-3);
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval_type(&es, 0, (i+1)*1000+1, 1), i-2);
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval_type(&es, 0, (i+1)*1000+1, 2), i-1);
		ASSERT_EQUALS(event_set_get_last_single_event_in_interval_type(&es, 0, (i+1)*1000+1, 3), i);
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds 10 counters to an event set and checks whether
 * event_set_get_counter_event_set finds them.
 */
UNIT_TEST(get_counter_event_set_test)
{
	struct event_set es;
	struct counter_description cd[10];

	event_set_init(&es, 0);

	for(int i = 0; i < 10; i++) {
		cd[i].counter_id = i;
		cd[i].min = 100;
		cd[i].max = 1000;
		cd[i].min_slope = 0;
		cd[i].max_slope = 5;
		cd[i].index = i;
		cd[i].name = "foo";
		cd[i].color_r = 0;
		cd[i].color_g = 0;
		cd[i].color_b = 0;

		ASSERT_EQUALS(event_set_alloc_counter_event_set(&es, &cd[i]), 0);
	}

	/* Check indexes that exist */
	for(int i = 0; i < 10; i++)
		ASSERT_EQUALS(event_set_get_counter_event_set(&es, i), i);

	/* Check a few non-existing indexes */
	ASSERT_EQUALS(event_set_get_counter_event_set(&es, 10), -1);
	ASSERT_EQUALS(event_set_get_counter_event_set(&es, 11), -1);
	ASSERT_EQUALS(event_set_get_counter_event_set(&es, 104), -1);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds 10 counters to an event set and checks whether
 * event_set_has_counter returns the correct values.
 */
UNIT_TEST(has_counter_test)
{
	struct event_set es;
	struct counter_description cd[10];

	event_set_init(&es, 0);

	for(int i = 0; i < 10; i++) {
		cd[i].counter_id = i;
		cd[i].min = 100;
		cd[i].max = 1000;
		cd[i].min_slope = 0;
		cd[i].max_slope = 5;
		cd[i].index = i;
		cd[i].name = "foo";
		cd[i].color_r = 0;
		cd[i].color_g = 0;
		cd[i].color_b = 0;

		ASSERT_EQUALS(event_set_alloc_counter_event_set(&es, &cd[i]), 0);
	}

	for(int i = 0; i < 10; i++)
		ASSERT_TRUE(event_set_has_counter(&es, &cd[i]));

	event_set_destroy(&es);
}
END_TEST()

/*
 * Checks whether 
 */
UNIT_TEST(counters_monotonously_increasing_test)
{
	struct event_set es_mon_lin;
	struct event_set es_mon_const;
	struct event_set es_nomon;
	struct event_set es_nomon_comb;

	struct counter_description cd_mon_lin;
	struct counter_description cd_mon_const;
	struct counter_description cd_nomon;

	struct counter_description* cd_err;
	struct counter_event ce;

	event_set_init(&es_mon_lin, 0);
	event_set_init(&es_mon_const, 0);
	event_set_init(&es_nomon, 0);
	event_set_init(&es_nomon_comb, 0);

	/* Add strictly monotonously increasing counter */
	cd_mon_lin.counter_id = 0;
	cd_mon_lin.index = 0;
	ASSERT_EQUALS(event_set_alloc_counter_event_set(&es_mon_lin, &cd_mon_lin), 0);

	/* Add constant counter */
	cd_mon_const.counter_id = 0;
	cd_mon_const.index = 0;
	ASSERT_EQUALS(event_set_alloc_counter_event_set(&es_mon_const, &cd_mon_const), 0);

	/* Add increasing and decreasing counter */
	cd_nomon.counter_id = 1;
	cd_nomon.index = 1;
	ASSERT_EQUALS(event_set_alloc_counter_event_set(&es_nomon, &cd_nomon), 0);

	ASSERT_EQUALS(event_set_alloc_counter_event_set(&es_nomon_comb, &cd_mon_lin), 0);
	ASSERT_EQUALS(event_set_alloc_counter_event_set(&es_nomon_comb, &cd_mon_const), 0);
	ASSERT_EQUALS(event_set_alloc_counter_event_set(&es_nomon_comb, &cd_nomon), 0);

	for(int i = 0; i < 100; i++) {
		/* Linear increase */
		ce.time = i*1000000;
		ce.value = 1000*i;
		ASSERT_EQUALS(event_set_add_counter_event(&es_mon_lin, &ce, &cd_mon_lin, 0), 0);
		ASSERT_EQUALS(event_set_add_counter_event(&es_nomon_comb, &ce, &cd_mon_lin, 0), 0);

		/* Constant values */
		ce.time = i*1000000;
		ce.value = 1000;
		ASSERT_EQUALS(event_set_add_counter_event(&es_mon_const, &ce, &cd_mon_const, 0), 0);
		ASSERT_EQUALS(event_set_add_counter_event(&es_nomon_comb, &ce, &cd_mon_const, 0), 0);

		/* Linear increase, just one decrease */
		ce.time = i*1000000;
		ce.value = (i == 50) ? 0 : 1000*i;
		ASSERT_EQUALS(event_set_add_counter_event(&es_nomon, &ce, &cd_nomon, 0), 0);
		ASSERT_EQUALS(event_set_add_counter_event(&es_nomon_comb, &ce, &cd_nomon, 0), 0);
	}

	ASSERT_EQUALS(event_set_counters_monotonously_increasing(&es_mon_lin, NULL, &cd_err), 1);
	ASSERT_EQUALS(event_set_counters_monotonously_increasing(&es_mon_const, NULL, &cd_err), 1);

	cd_err = NULL;
	ASSERT_EQUALS(event_set_counters_monotonously_increasing(&es_nomon, NULL, &cd_err), 0);
	ASSERT_EQUALS_PTR(cd_err, &cd_nomon);

	ASSERT_EQUALS(event_set_counters_monotonously_increasing(&es_nomon_comb, NULL, &cd_err), 0);

	event_set_destroy(&es_mon_lin);
	event_set_destroy(&es_mon_const);
	event_set_destroy(&es_nomon);
	event_set_destroy(&es_nomon_comb);
}
END_TEST()

/*
 * Initializes an event set with a linear counter and a hundres tasks
 * and exports per-task counter values to a temporary file. The file
 * is reloaded and values are checked.
 */
UNIT_TEST(dump_per_task_counter_values_test)
{
	struct event_set es;
	struct counter_description cd;
	struct counter_event ce;
	static const int ntasks = 100;
	static const uint64_t gap = 1000;
	static const uint64_t start = 41562;
	static const int cpu = 1234;
	uint64_t total_duration = 0;

	struct task task;

	uint64_t durations[ntasks];
	FILE* fp;
	int nb_errors_out;

	char symbol_name[40];
	char ctr_name[40];
	int task_cpu;
	uint64_t nb_events;
	uint64_t task_duration;

	task.addr = 0xc0ffee;
	task.source_filename = NULL;
	task.symbol_name = "task-1";
	task.source_line = 0;
	task.id = 0;

	for(int i = 0; i < ntasks; i++) {
		durations[i] = 1000000 + (i % 17) * 1270002;
		total_duration += durations[i];
	}

	event_set_init(&es, cpu);

	cd.counter_id = 0;
	cd.index = 0;
	cd.name = "ctr1";
	ASSERT_EQUALS(event_set_alloc_counter_event_set(&es, &cd), 0);

	__create_tasks(&es, ntasks, durations, gap, start);

	/* Set task types */
	for(int i = 0; i < ntasks; i++) {
		es.single_events[2*i].active_task = &task;
		es.single_events[2*i+1].active_task = &task;
	}

	/* Linear increase, one unit per cycle */
	ce.time = 0;
	ce.value = 0;
	ASSERT_EQUALS(event_set_add_counter_event(&es, &ce, &cd, 0), 0);

	ce.time = ntasks*gap+total_duration+start+1;
	ce.value = ce.time;
	ASSERT_EQUALS(event_set_add_counter_event(&es, &ce, &cd, 0), 0);

	fp = tmpfile();
	ASSERT_NONNULL(fp);

	event_set_dump_per_task_counter_values(&es, NULL, fp, &nb_errors_out);
	ASSERT_EQUALS(nb_errors_out, 0);

	rewind(fp);

	/* Scan file */
	for(int i = 0; i < ntasks; i++) {
		ASSERT_EQUALS(fscanf(fp, "%s %s %d %"PRIu64" %"PRId64"\n",
				     symbol_name,
				     ctr_name,
				     &task_cpu,
				     &task_duration,
				     &nb_events), 5);

		ASSERT_EQUALS_STRING(symbol_name, task.symbol_name);
		ASSERT_EQUALS_STRING(ctr_name, cd.name);
		ASSERT_EQUALS(task_cpu, cpu);
		ASSERT_EQUALS(task_duration, durations[i]);
		ASSERT_EQUALS(nb_events, durations[i]);
	}


	ASSERT_EQUALS(fclose(fp), 0);
	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds a single annotation to an event set.
 */
UNIT_TEST(add_single_annotation_test)
{
	struct event_set es;
	struct annotation a;

	event_set_init(&es, 0);

	ASSERT_EQUALS(annotation_init(&a, 0, 1000, "foo"), 0);
	ASSERT_EQUALS(event_set_add_annotation(&es, &a), 0);
	annotation_destroy(&a);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds 100 annotations to an event set.
 */
UNIT_TEST(add_multi_annotation_test)
{
	struct event_set es;
	struct annotation a;
	char buf[20];

	event_set_init(&es, 0);

	for(int i = 0; i < 10; i++) {
		snprintf(buf, sizeof(buf), "annotation-%d", i);
		ASSERT_EQUALS(annotation_init(&a, 0, i*1000, buf), 0);
		ASSERT_EQUALS(event_set_add_annotation(&es, &a), 0);
		annotation_destroy(&a);
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds 10 annotations to an event set and checks
 * event_set_get_first_annotation_in_interval on different intervals.
 */
UNIT_TEST(find_get_annotation_in_interval_test)
{
	struct event_set es;
	struct annotation a;
	char buf[20];

	event_set_init(&es, 0);

	for(int i = 0; i < 10; i++) {
		snprintf(buf, sizeof(buf), "annotation-%d", i);
		ASSERT_EQUALS(annotation_init(&a, 0, (i+1)*1000, buf), 0);
		ASSERT_EQUALS(event_set_add_annotation(&es, &a), 0);
		annotation_destroy(&a);
	}

	/* No annotation */
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 100, 200), -1);
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 0, 999), -1);
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 1001, 1999), -1);

	/* First annotation */
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 1000, 1001), 0);
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 999, 1000), 0);
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 999, 1001), 0);

	/* First and second annotation */
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 999, 2000), 0);
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 1000, 2000), 0);
	ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, 1000, 2001), 0);


	/* i-th annotation */
	for(int i = 0; i < 10; i++) {
		ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, (i+1)*1000, (i+1)*1000+1), i);
		ASSERT_EQUALS(event_set_get_first_annotation_in_interval(&es, (i+1)*1000-500, (i+1)*1000+500), i);
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds 11 tasks using 3 frames to an event set and checks whether
 * event_set_find_next_texec_start_for_frame on it.
 */
UNIT_TEST(find_next_texec_start_for_frame_test)
{
	struct event_set es;
	struct single_event* sge;
	static const int ntasks = 11;

	int nframes = 3;
	struct frame frames[nframes];
	struct frame nonexframe;

	uint64_t durations[ntasks];

	for(int i = 0; i < ntasks; i++)
		durations[i] = 1000;

	event_set_init(&es, 0);

	__create_tasks(&es, ntasks, durations, 1000, 0);

	/* Initialize frames */
	for(int i = 0; i < nframes; i++) {
		frames[i].addr = i;
		frames[i].num_pushes = 0;
		frames[i].num_steals = 0;
		frames[i].numa_node = 0;
		frames[i].size = 1234;
		frames[i].first_tcreate = NULL;
		frames[i].first_texec_start = &es.single_events[2*i];
		frames[i].first_write = NULL;
		frames[i].first_max_write = NULL;
	}

	/* Create set of single events for the beginning and end of
	 * the execution of the tasks. */
	for(int i = 0; i < 2*ntasks; i++) {
		es.single_events[i].active_frame = &frames[(i/2) % nframes];
		es.single_events[i].what = &frames[(i/2) % nframes];
	}

	/* Frame that is never used */
	nonexframe.addr = 0xc0ffee;
	ASSERT_NULL(event_set_find_next_texec_start_for_frame(&es, 0, &nonexframe));

	/* Interval start after last task execution */
	ASSERT_NULL(event_set_find_next_texec_start_for_frame(&es, 2*ntasks*1000, &frames[0]));

	for(int i = 0; i < ntasks; i++) {
		int this_frame = i % nframes;

		for(int j = 0; j < nframes; j++) {
			if(i+j < ntasks) {
				int next_frame = (this_frame + j) % nframes;

				/* Timestamp is identical with a task
				 * execution start event */
				sge = event_set_find_next_texec_start_for_frame(&es, 2*i*1000, &frames[next_frame]);
				ASSERT_NONNULL(sge);
				ASSERT_EQUALS(sge->time, 2*(i+j)*1000);

				if(i > 0) {
					/* Timestamp right before a
					 * task execution start
					 * event */
					sge = event_set_find_next_texec_start_for_frame(&es, 2*i*1000-1, &frames[next_frame]);
					ASSERT_NONNULL(sge);
					ASSERT_EQUALS(sge->time, 2*(i+j)*1000);
				}
			}
		}

		if(i + nframes < ntasks) {
			/* Timestamp right after the current task
			 * execution start event */
			sge = event_set_find_next_texec_start_for_frame(&es, 2*i*1000+1, &frames[this_frame]);
			ASSERT_NONNULL(sge);
			ASSERT_EQUALS(sge->time, 2*(i+nframes)*1000);
		}
	}

	event_set_destroy(&es);
}
END_TEST()

/*
 * Creates 5 tasks with different durations and checks the return
 * values of event_set_get_average_task_length_in_interval for
 * different intervals.
 */
UNIT_TEST(get_average_task_length_in_interval_test)
{
	static const int ntasks = 5;
	struct event_set es;
	long double num_tasks;
	uint64_t durations[ntasks];

	durations[0] = 1000;
	durations[1] = 1000;
	durations[2] = 100;
	durations[3] = 200;
	durations[4] = 300;

	event_set_init(&es, 0);

	__create_tasks(&es, ntasks, durations, 1000, 1000);

	event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 0, 999);
	ASSERT_EQUALS_LD(num_tasks, 0);

	/* First task, exact match */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 1000, 2000), 1000);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 1.0L, 0.00001L);

	/* First task, exact match of the start */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 1000, 1900), 1000);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 0.9L, 0.00001L);

	/* First task, exact match of the end */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 1100, 2000), 1000);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 0.9L, 0.00001L);

	/* First task, fully included */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 999, 2001), 1000);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 1.0L, 0.00001L);

	/* First and second task, exact match */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 1000, 4000), 1000);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 2.0L, 0.00001L);

	/* First and second task, exact match of the start */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 1000, 3900), 1000);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 1.9L, 0.00001L);

	/* First and second task, exact match of the end */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 1100, 4000), 1000);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 1.9L, 0.00001L);

	/* First and second task, fully included */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 999, 4001), 1000);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 2.0L, 0.00001L);

	/* Second and third task, fully included */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 3000, 5100), (durations[1] + durations[2]) / 2);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 2.0L, 0.00001L);

	/* Second and third task, second task partially included */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 3500, 5100), ((durations[1]/2 + durations[2])*2)/3);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 1.5L, 0.00001L);

	/* All tasks */
	ASSERT_EQUALS(event_set_get_average_task_length_in_interval(&es, NULL, &num_tasks, 1000, 7600), 520);
	ASSERT_EQUALS_LD_DELTA(num_tasks, 5.0L, 0.00001L);

	event_set_destroy(&es);
}
END_TEST()

UNIT_TEST(get_min_task_duration_in_interval_test)
{
	static const int num_tasks = 5;
	struct event_set es;
	uint64_t durations[num_tasks];
	uint64_t duration;

	durations[0] = 1000;
	durations[1] = 999;
	durations[2] = 100;
	durations[3] = 200;
	durations[4] = 300;

	event_set_init(&es, 0);

	__create_tasks(&es, num_tasks, durations, 1000, 1000);

	/* Interval without tasks */
	ASSERT_EQUALS(event_set_get_min_task_duration_in_interval(&es, NULL, 0, 100, &duration), 0);

	/* First task overlaps with interval, but does not start
	 * within the interval*/
	ASSERT_EQUALS(event_set_get_min_task_duration_in_interval(&es, NULL, 1001, 1999, &duration), 0);

	/* First task starts in the interval, but does not terminate
	 * within the interval*/
	ASSERT_EQUALS(event_set_get_min_task_duration_in_interval(&es, NULL, 1000, 1999, &duration), 0);
	ASSERT_EQUALS(event_set_get_min_task_duration_in_interval(&es, NULL, 999, 1999, &duration), 0);

	/* First task starts and ends in the interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_min_task_duration_in_interval(&es, NULL, 1000, 2000, &duration), 0);
	ASSERT_EQUALS(duration, 1000);

	/* First task starts and ends in the interval, second task
	 * only starts in the interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_min_task_duration_in_interval(&es, NULL, 1000, 3100, &duration), 0);
	ASSERT_EQUALS(duration, 1000);

	/* First and second task start and end in the interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_min_task_duration_in_interval(&es, NULL, 1000, 3999, &duration), 0);
	ASSERT_EQUALS(duration, 999);

	/* First, second and third task start and end in the
	 * interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_min_task_duration_in_interval(&es, NULL, 1000, 5099, &duration), 0);
	ASSERT_EQUALS(duration, 100);

	/* All tasks */
	duration = 0;
	ASSERT_GREATER(event_set_get_min_task_duration_in_interval(&es, NULL, 1000, 15099, &duration), 0);
	ASSERT_EQUALS(duration, 100);

	event_set_destroy(&es);
}
END_TEST()

UNIT_TEST(get_max_task_duration_in_interval_test)
{
	static const int num_tasks = 5;
	struct event_set es;
	uint64_t durations[num_tasks];
	uint64_t duration;

	durations[0] = 1000;
	durations[1] = 999;
	durations[2] = 100;
	durations[3] = 200;
	durations[4] = 300;

	event_set_init(&es, 0);

	__create_tasks(&es, num_tasks, durations, 1000, 1000);

	/* Interval without tasks */
	ASSERT_EQUALS(event_set_get_max_task_duration_in_interval(&es, NULL, 0, 100, &duration), 0);

	/* First task overlaps with interval, but does not start
	 * within the interval*/
	ASSERT_EQUALS(event_set_get_max_task_duration_in_interval(&es, NULL, 1001, 1999, &duration), 0);

	/* First task starts in the interval, but does not terminate
	 * within the interval*/
	ASSERT_EQUALS(event_set_get_max_task_duration_in_interval(&es, NULL, 1000, 1999, &duration), 0);
	ASSERT_EQUALS(event_set_get_max_task_duration_in_interval(&es, NULL, 999, 1999, &duration), 0);

	/* First task starts and ends in the interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_max_task_duration_in_interval(&es, NULL, 1000, 2000, &duration), 0);
	ASSERT_EQUALS(duration, 1000);

	/* First task starts and ends in the interval, second task
	 * only starts in the interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_max_task_duration_in_interval(&es, NULL, 1000, 3100, &duration), 0);
	ASSERT_EQUALS(duration, 1000);

	/* First and second task start and end in the interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_max_task_duration_in_interval(&es, NULL, 1000, 3999, &duration), 0);
	ASSERT_EQUALS(duration, 1000);

	/* Second task starts and ends in the interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_max_task_duration_in_interval(&es, NULL, 2500, 3999, &duration), 0);
	ASSERT_EQUALS(duration, 999);

	/* First, second and third task start and end in the
	 * interval */
	duration = 0;
	ASSERT_GREATER(event_set_get_max_task_duration_in_interval(&es, NULL, 1000, 5099, &duration), 0);
	ASSERT_EQUALS(duration, 1000);

	/* All tasks */
	duration = 0;
	ASSERT_GREATER(event_set_get_max_task_duration_in_interval(&es, NULL, 1000, 15099, &duration), 0);
	ASSERT_EQUALS(duration, 1000);

	event_set_destroy(&es);
}
END_TEST()

UNIT_TEST(get_min_max_task_duration_in_interval_test)
{
	static const int num_tasks = 5;
	struct event_set es;
	uint64_t durations[num_tasks];
	uint64_t min;
	uint64_t max;

	durations[0] = 1000;
	durations[1] = 999;
	durations[2] = 100;
	durations[3] = 200;
	durations[4] = 300;

	event_set_init(&es, 0);

	__create_tasks(&es, num_tasks, durations, 1000, 1000);

	/* Interval without tasks */
	ASSERT_EQUALS(event_set_get_min_max_task_duration_in_interval(&es, NULL, 0, 100, &min, &max), 0);

	/* First task overlaps with interval, but does not start
	 * within the interval*/
	ASSERT_EQUALS(event_set_get_min_max_task_duration_in_interval(&es, NULL, 1001, 1999, &min, &max), 0);

	/* First task starts in the interval, but does not terminate
	 * within the interval*/
	ASSERT_EQUALS(event_set_get_min_max_task_duration_in_interval(&es, NULL, 1000, 1999, &min, &max), 0);
	ASSERT_EQUALS(event_set_get_min_max_task_duration_in_interval(&es, NULL, 999, 1999, &min, &max), 0);

	/* First task starts and ends in the interval */
	min = 0;
	max = 0;
	ASSERT_GREATER(event_set_get_min_max_task_duration_in_interval(&es, NULL, 1000, 2000, &min, &max), 0);
	ASSERT_EQUALS(min, 1000);
	ASSERT_EQUALS(max, 1000);

	/* First task starts and ends in the interval, second task
	 * only starts in the interval */
	min = 0;
	max = 0;
	ASSERT_GREATER(event_set_get_min_max_task_duration_in_interval(&es, NULL, 1000, 3100, &min, &max), 0);
	ASSERT_EQUALS(min, 1000);
	ASSERT_EQUALS(max, 1000);

	/* First and second task start and end in the interval */
	min = 0;
	max = 0;
	ASSERT_GREATER(event_set_get_min_max_task_duration_in_interval(&es, NULL, 1000, 3999, &min, &max), 0);
	ASSERT_EQUALS(min, 999);
	ASSERT_EQUALS(max, 1000);

	/* First, second and third task start and end in the
	 * interval */
	min = 0;
	max = 0;
	ASSERT_GREATER(event_set_get_min_max_task_duration_in_interval(&es, NULL, 1000, 5099, &min, &max), 0);
	ASSERT_EQUALS(min, 100);
	ASSERT_EQUALS(max, 1000);

	/* All tasks */
	min = 0;
	max = 0;
	ASSERT_GREATER(event_set_get_min_max_task_duration_in_interval(&es, NULL, 1000, 15099, &min, &max), 0);
	ASSERT_EQUALS(min, 100);
	ASSERT_EQUALS(max, 1000);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds 100 tasks with 17 types to an event set and checks the return
 * value of event_set_get_task_duration_in_interval for different
 * intervals.
 */
UNIT_TEST(get_task_duration_in_interval_test)
{
	static const int num_tasks = 100;
	static const int num_types = 17;
	struct event_set es;
	uint64_t gap = 1000;
	struct task task_types[num_types];
	uint64_t durations[num_tasks];
	uint64_t type_durations[num_types];

	/* All tasks have the same duration */
	for(int i = 0; i < num_tasks; i++)
		durations[i] = 1000;

	/* Initialize task types */
	for(int i = 0; i < num_types; i++) {
		task_types[i].addr = i;
		task_types[i].source_filename = NULL;
		task_types[i].symbol_name = NULL;
		task_types[i].source_line = 0;
		task_types[i].id = i;
	}

	event_set_init(&es, 0);

	__create_tasks(&es, num_tasks, durations, gap, 1000);

	/* Set task types */
	for(int i = 0; i < num_tasks; i++) {
		es.single_events[2*i].active_task = &task_types[i % num_types];
		es.single_events[2*i+1].active_task = &task_types[i % num_types];
	}

	/* Interval without tasks */
	memset(type_durations, 0, num_types*sizeof(type_durations[0]));
	ASSERT_EQUALS(event_set_get_task_duration_in_interval(&es, NULL, 0, 999, type_durations), 0);

	/* First task only, partial overlap */
	memset(type_durations, 0, num_types*sizeof(type_durations[0]));
	ASSERT_EQUALS(event_set_get_task_duration_in_interval(&es, NULL, 0, 1500, type_durations), 1);
	ASSERT_EQUALS(type_durations[0], 500);

	for(int i = 1; i < num_types; i++)
		ASSERT_EQUALS(type_durations[i], 0);

	/* First task only, exact overlap */
	memset(type_durations, 0, num_types*sizeof(type_durations[0]));
	ASSERT_EQUALS(event_set_get_task_duration_in_interval(&es, NULL, 1000, 2000, type_durations), 1);
	ASSERT_EQUALS(type_durations[0], 1000);

	for(int i = 1; i < num_types; i++)
		ASSERT_EQUALS(type_durations[i], 0);

	/* First task only, full overlap */
	memset(type_durations, 0, num_types*sizeof(type_durations[0]));
	ASSERT_EQUALS(event_set_get_task_duration_in_interval(&es, NULL, 500, 2500, type_durations), 1);
	ASSERT_EQUALS(type_durations[0], 1000);

	for(int i = 1; i < num_types; i++)
		ASSERT_EQUALS(type_durations[i], 0);

	/* First and second task, full overlap for the first task,
	 * partial overlap for the second task */
	memset(type_durations, 0, num_types*sizeof(type_durations[0]));
	ASSERT_EQUALS(event_set_get_task_duration_in_interval(&es, NULL, 500, 3500, type_durations), 1);
	ASSERT_EQUALS(type_durations[0], 1000);
	ASSERT_EQUALS(type_durations[1], 500);

	for(int i = 2; i < num_types; i++)
		ASSERT_EQUALS(type_durations[i], 0);

	/* First and second task, full overlap for both tasks */
	memset(type_durations, 0, num_types*sizeof(type_durations[0]));
	ASSERT_EQUALS(event_set_get_task_duration_in_interval(&es, NULL, 500, 4000, type_durations), 1);
	ASSERT_EQUALS(type_durations[0], 1000);
	ASSERT_EQUALS(type_durations[1], 1000);

	for(int i = 2; i < num_types; i++)
		ASSERT_EQUALS(type_durations[i], 0);

	for(int i = 0; i < num_tasks; i++) {
		/* Full overlap, n-th task */
		memset(type_durations, 0, num_types*sizeof(type_durations[0]));
		ASSERT_EQUALS(event_set_get_task_duration_in_interval(&es, NULL, 1000*(2*i+1), 1000*(2*i+2), type_durations), 1);

		ASSERT_EQUALS(type_durations[i % num_types], 1000);

		for(int j = 0; j < num_types; j++)
			if(j != i % num_types)
				ASSERT_EQUALS(type_durations[j], 0);

		/* Full overlap, first n tasks */
		memset(type_durations, 0, num_types*sizeof(type_durations[0]));
		ASSERT_EQUALS(event_set_get_task_duration_in_interval(&es, NULL, 1000, 1000*(2*i+2), type_durations), 1);

		for(int j = 0; j < num_types; j++) {
			int add = (i % num_types >= j) ? 1 : 0;
			ASSERT_EQUALS(type_durations[j], ((i/num_types)+add)*1000);
		}
	}

	event_set_destroy(&es);
}
END_TEST()

/* Internal function of event_set.c */
int __event_set_get_major_accessed_node_in_interval(struct event_set* es, enum comm_event_type* types, unsigned int num_types, struct filter* f, uint64_t start, uint64_t end, int max_numa_node_id, int* major_node);

/*
 * Adds two tasks with different memory accesses to multiple nodes to
 * an event set and checks the major accessed node as returned by
 * __event_set_get_major_accessed_node_in_interval for different
 * intervals.
 */
UNIT_TEST(__event_set_get_major_accessed_node_in_interval_test)
{
	static const int num_tasks = 2;
	struct event_set es;
	struct comm_event ce;
	uint64_t durations[num_tasks];
	int max_node_id = 1;
	int major_node;

	durations[0] = 1000;
	durations[1] = 1000;

	event_set_init(&es, 0);

	__create_tasks(&es, num_tasks, durations, 1000, 1000);

	struct frame frame_n0_1k;
	frame_n0_1k.addr = 0xc0ffee;
	frame_n0_1k.num_pushes = 0;
	frame_n0_1k.num_steals = 0;
	frame_n0_1k.numa_node = 0;
	frame_n0_1k.size = 1000;
	frame_n0_1k.first_tcreate = NULL;
	frame_n0_1k.first_texec_start = NULL;
	frame_n0_1k.first_write = NULL;
	frame_n0_1k.first_max_write = NULL;

	struct frame frame_n0_2k;
	frame_n0_2k.addr = 0xc0ffee;
	frame_n0_2k.num_pushes = 0;
	frame_n0_2k.num_steals = 0;
	frame_n0_2k.numa_node = 0;
	frame_n0_2k.size = 2000;
	frame_n0_2k.first_tcreate = NULL;
	frame_n0_2k.first_texec_start = NULL;
	frame_n0_2k.first_write = NULL;
	frame_n0_2k.first_max_write = NULL;

	struct frame frame_n1_2k;
	frame_n1_2k.addr = 0xc0ffee;
	frame_n1_2k.num_pushes = 0;
	frame_n1_2k.num_steals = 0;
	frame_n1_2k.numa_node = 1;
	frame_n1_2k.size = 2000;
	frame_n1_2k.first_tcreate = NULL;
	frame_n1_2k.first_texec_start = NULL;
	frame_n1_2k.first_write = NULL;
	frame_n1_2k.first_max_write = NULL;

	struct frame frame_n1_10k;
	frame_n1_10k.addr = 0xc0ffee;
	frame_n1_10k.num_pushes = 0;
	frame_n1_10k.num_steals = 0;
	frame_n1_10k.numa_node = 1;
	frame_n1_10k.size = 10000;
	frame_n1_10k.first_tcreate = NULL;
	frame_n1_10k.first_texec_start = NULL;
	frame_n1_10k.first_write = NULL;
	frame_n1_10k.first_max_write = NULL;

	struct frame frame_n0_50k;
	frame_n0_50k.addr = 0xc0ffee;
	frame_n0_50k.num_pushes = 0;
	frame_n0_50k.num_steals = 0;
	frame_n0_50k.numa_node = 0;
	frame_n0_50k.size = 50000;
	frame_n0_50k.first_tcreate = NULL;
	frame_n0_50k.first_texec_start = NULL;
	frame_n0_50k.first_write = NULL;
	frame_n0_50k.first_max_write = NULL;

	ce.event_set = NULL;
	ce.dst_cpu = 1;
	ce.src_cpu = 0;
	ce.prod_ts = 0;
	ce.active_task = NULL;
	ce.active_frame = NULL;

	/* Task T1: read 1000 bytes from node 0 */
	ce.time = 1100;
	ce.size = 1000;
	ce.what = &frame_n0_1k;
	ce.texec_start = &es.single_events[0];
	ce.texec_end = &es.single_events[1];
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Task T1: read 2000 bytes from node 0 */
	ce.time = 1200;
	ce.size = 2000;
	ce.what = &frame_n0_2k;
	ce.texec_start = &es.single_events[0];
	ce.texec_end = &es.single_events[1];
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Task T1: write 10000 bytes to node 1 */
	ce.time = 1300;
	ce.size = 10000;
	ce.what = &frame_n1_10k;
	ce.texec_start = &es.single_events[0];
	ce.texec_end = &es.single_events[1];
	ce.type = COMM_TYPE_DATA_WRITE;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Task T2: read 1000 bytes from node 0 */
	ce.time = 3100;
	ce.size = 1000;
	ce.what = &frame_n0_1k;
	ce.texec_start = &es.single_events[2];
	ce.texec_end = &es.single_events[3];
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Task T2: read 2000 bytes from node 1 */
	ce.time = 3200;
	ce.size = 2000;
	ce.what = &frame_n1_2k;
	ce.texec_start = &es.single_events[2];
	ce.texec_end = &es.single_events[3];
	ce.type = COMM_TYPE_DATA_READ;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	/* Task T2: write 50000 bytes to node 0 */
	ce.time = 3300;
	ce.size = 50000;
	ce.what = &frame_n0_50k;
	ce.texec_start = &es.single_events[2];
	ce.texec_end = &es.single_events[3];
	ce.type = COMM_TYPE_DATA_WRITE;
	ASSERT_EQUALS(event_set_add_comm_event(&es, &ce), 0);

	enum comm_event_type types[2] = {
		COMM_TYPE_DATA_READ,
		COMM_TYPE_DATA_WRITE
	};

	/* No task in interval */
	major_node = -1;
	ASSERT_EQUALS(__event_set_get_major_accessed_node_in_interval(&es, types, 2, NULL, 0, 100, max_node_id, &major_node), 0);

	/* Reads of T1 only, whole task */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, types, 1, NULL, 1000, 2000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 0);

	/* Writes of T1 only, whole task */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, &types[1], 1, NULL, 1000, 2000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 1);

	/* Reads and writes of T1 only, whole task */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, types, 2, NULL, 1000, 2000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 1);

	/* Empty set of types */
	major_node = -1;
	ASSERT_EQUALS(__event_set_get_major_accessed_node_in_interval(&es, types, 0, NULL, 1000, 2000, max_node_id, &major_node), 0);

	/* Reads of T2 only, whole task */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, types, 1, NULL, 3000, 4000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 1);

	/* Writes of T2 only, whole task */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, &types[1], 1, NULL, 3000, 4000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 0);

	/* Reads and writes of T2 only, whole task */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, types, 2, NULL, 3000, 4000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 0);


	/* Reads of T1 and T2, whole tasks */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, types, 1, NULL, 1000, 4000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 0);

	/* Writes of T1 and T2, whole tasks */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, &types[1], 1, NULL, 1000, 4000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 0);

	/* Reads and writes of T1 and T2, whole tasks */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, types, 2, NULL, 1000, 4000, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 0);


	/* Writes of T1 and T2, 99% of T1, 1% of T2 */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, &types[1], 1, NULL, 1000, 3010, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 1);

	/* Writes of T1 and T2, 90% of T1, 10% of T2 */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, &types[1], 1, NULL, 1000, 3100, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 1);

	/* Writes of T1 and T2, 79% of T1, 21% of T2 */
	major_node = -1;
	ASSERT_GREATER(__event_set_get_major_accessed_node_in_interval(&es, &types[1], 1, NULL, 1000, 3210, max_node_id, &major_node), 0);
	ASSERT_EQUALS(major_node, 0);

	event_set_destroy(&es);
}
END_TEST()


/* Helper function that adds task executions and memory accesses to an
 * event set */
void __init_event_set_with_memaccesses(struct event_set* es, struct frame* local_frame, struct frame* remote_frame)
{
	static const int num_tasks = 3;

	struct comm_event ce;
	uint64_t durations[num_tasks];

	durations[0] = 1000;
	durations[1] = 2000;
	durations[2] = 100;

	event_set_init(es, 0);

	__create_tasks(es, num_tasks, durations, 1000, 1000);

	/* First task:
	 * 2 local writes, 512 KiB + 1 MiB
	 * 1 remote write, 1 MiB
	 * 1 local read, 256 KiB
	 */
	ce.time = 1001;
	ce.src_cpu = 0;
	ce.dst_cpu = 1;
	ce.size = 512*1024;
	ce.type = COMM_TYPE_DATA_WRITE;
	ce.what = local_frame;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = &es->single_events[0];
	ce.texec_end = &es->single_events[1];
	ASSERT_EQUALS(event_set_add_comm_event(es, &ce), 0);

	ce.time = 1101;
	ce.src_cpu = 0;
	ce.dst_cpu = 2;
	ce.size = 1024*1024;
	ce.type = COMM_TYPE_DATA_WRITE;
	ce.what = local_frame;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = &es->single_events[0];
	ce.texec_end = &es->single_events[1];
	ASSERT_EQUALS(event_set_add_comm_event(es, &ce), 0);

	ce.time = 1201;
	ce.src_cpu = 0;
	ce.dst_cpu = 20;
	ce.size = 1024*1024;
	ce.type = COMM_TYPE_DATA_WRITE;
	ce.what = remote_frame;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = &es->single_events[0];
	ce.texec_end = &es->single_events[1];
	ASSERT_EQUALS(event_set_add_comm_event(es, &ce), 0);

	ce.time = 1301;
	ce.src_cpu = 0;
	ce.dst_cpu = 3;
	ce.size = 256*1024;
	ce.type = COMM_TYPE_DATA_READ;
	ce.what = local_frame;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = &es->single_events[0];
	ce.texec_end = &es->single_events[1];
	ASSERT_EQUALS(event_set_add_comm_event(es, &ce), 0);

	/* Second task:
	 * 1 local write, 512 KiB
	 * 1 remote read, 1 MiB
	 */
	ce.time = 3001;
	ce.src_cpu = 0;
	ce.dst_cpu = 1;
	ce.size = 512*1024;
	ce.type = COMM_TYPE_DATA_WRITE;
	ce.what = local_frame;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = &es->single_events[2];
	ce.texec_end = &es->single_events[3];
	ASSERT_EQUALS(event_set_add_comm_event(es, &ce), 0);

	ce.time = 4001;
	ce.src_cpu = 0;
	ce.dst_cpu = 20;
	ce.size = 1024*1024;
	ce.type = COMM_TYPE_DATA_READ;
	ce.what = remote_frame;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = &es->single_events[2];
	ce.texec_end = &es->single_events[3];
	ASSERT_EQUALS(event_set_add_comm_event(es, &ce), 0);

	/* Third task:
	 * 1 local write, 1 MiB
	 */
	ce.time = 6001;
	ce.src_cpu = 0;
	ce.dst_cpu = 1;
	ce.size = 1024*1024;
	ce.type = COMM_TYPE_DATA_WRITE;
	ce.what = local_frame;
	ce.active_task = NULL;
	ce.active_frame = NULL;
	ce.texec_start = &es->single_events[4];
	ce.texec_end = &es->single_events[5];
	ASSERT_EQUALS(event_set_add_comm_event(es, &ce), 0);
}

/*
 * Adds several tasks with remote and local memory accesses to an
 * event set and checks the number of locally and remotely accessed
 * bytes returned by event_set_get_remote_local_numa_bytes_in_interval
 * for different intervals.
 */
UNIT_TEST(get_remote_local_numa_bytes_in_interval_test)
{
	struct event_set es;

	static const int32_t local_node = 0;
	static const int32_t remote_node = 1;

	struct frame local_frame = {
		.numa_node = local_node
	};

	struct frame remote_frame = {
		.numa_node = remote_node
	};

	uint64_t local_bytes;
	uint64_t remote_bytes;

	__init_event_set_with_memaccesses(&es, &local_frame, &remote_frame);

	/* Intervals without tasks */
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 0, 1000, local_node, &local_bytes, &remote_bytes), 0);
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 2000, 3000, local_node, &local_bytes, &remote_bytes), 0);
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 5000, 6000, local_node, &local_bytes, &remote_bytes), 0);
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 6100, 7000, local_node, &local_bytes, &remote_bytes), 0);
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 9000, 11000, local_node, &local_bytes, &remote_bytes), 0);

	/* First task, entirely included */
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 1000, 2000, local_node, &local_bytes, &remote_bytes), 1);
	ASSERT_EQUALS(local_bytes, (1024+512+256)*1024);
	ASSERT_EQUALS(remote_bytes, 1024*1024);

	/* Second task, entirely included */
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 3000, 5000, local_node, &local_bytes, &remote_bytes), 1);
	ASSERT_EQUALS(local_bytes, 512*1024);
	ASSERT_EQUALS(remote_bytes, 1024*1024);

	/* Third task, entirely included */
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 6000, 6100, local_node, &local_bytes, &remote_bytes), 1);
	ASSERT_EQUALS(local_bytes, 1024*1024);
	ASSERT_EQUALS(remote_bytes, 0);

	/* First half of the first task */
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 1000, 1500, local_node, &local_bytes, &remote_bytes), 1);
	ASSERT_EQUALS(local_bytes, ((1024+512+256)*1024)/2);
	ASSERT_EQUALS(remote_bytes, (1024*1024)/2);

	/* Second half of the first task */
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 1500, 2000, local_node, &local_bytes, &remote_bytes), 1);
	ASSERT_EQUALS(local_bytes, ((1024+512+256)*1024)/2);
	ASSERT_EQUALS(remote_bytes, (1024*1024)/2);

	/* First and second task */
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 1000, 5000, local_node, &local_bytes, &remote_bytes), 1);
	ASSERT_EQUALS(local_bytes, (1024+512+256+512)*1024);
	ASSERT_EQUALS(remote_bytes, 2*1024*1024);

	/* All tasks */
	ASSERT_EQUALS(event_set_get_remote_local_numa_bytes_in_interval(&es, NULL, 1000, 6100, local_node, &local_bytes, &remote_bytes), 1);
	ASSERT_EQUALS(local_bytes, (1024+512+256+512+1024)*1024);
	ASSERT_EQUALS(remote_bytes, 2*1024*1024);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds several tasks with remote and local memory accesses to an
 * event set and checks the id of the node receiving the majority of
 * the data of write accesses returned by
 * event_set_get_major_written_node_in_interval for different
 * intervals.
 */
UNIT_TEST(get_major_written_node_in_interval_test)
{
	struct event_set es;

	int major_node;

	static const int32_t local_node = 0;
	static const int32_t remote_node = 1;
	static const int32_t max_numa_node_id = 1;

	struct frame local_frame = {
		.numa_node = local_node
	};

	struct frame remote_frame = {
		.numa_node = remote_node
	};

	__init_event_set_with_memaccesses(&es, &local_frame, &remote_frame);

	/* Intervals without tasks */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 0, 1000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 2000, 3000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 5000, 6000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 6100, 7000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 9000, 11000, max_numa_node_id, &major_node), 0);

	/* First task, entirely included */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 1000, 2000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Second task, entirely included */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 3000, 5000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Third task, entirely included */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 6000, 6100, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* First half of the first task */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 1000, 1500, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Tiny fraction of the first task */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 1000, 1001, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Second half of the first task */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 1500, 2000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* First and second task */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 1000, 5000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* All tasks */
	ASSERT_EQUALS(event_set_get_major_written_node_in_interval(&es, NULL, 1000, 6100, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds several tasks with remote and local memory accesses to an
 * event set and checks the id of the node providing the majority of
 * the data of read accesses returned by
 * event_set_get_major_read_node_in_interval for different
 * intervals.
 */
UNIT_TEST(get_major_read_node_in_interval_test)
{
	struct event_set es;

	int major_node;

	static const int32_t local_node = 0;
	static const int32_t remote_node = 1;
	static const int32_t max_numa_node_id = 1;

	struct frame local_frame = {
		.numa_node = local_node
	};

	struct frame remote_frame = {
		.numa_node = remote_node
	};

	__init_event_set_with_memaccesses(&es, &local_frame, &remote_frame);

	/* Intervals without tasks */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 0, 1000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 2000, 3000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 5000, 6000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 6100, 7000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 9000, 11000, max_numa_node_id, &major_node), 0);

	/* First task, entirely included */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 1000, 2000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Second task, entirely included */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 3000, 5000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, remote_node);

	/* Third task, entirely included */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 6000, 6100, max_numa_node_id, &major_node), 0);

	/* First half of the first task */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 1000, 1500, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Tiny fraction of the first task */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 1000, 1001, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Second half of the first task */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 1500, 2000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* First and second task */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 1000, 5000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, remote_node);

	/* All tasks */
	ASSERT_EQUALS(event_set_get_major_read_node_in_interval(&es, NULL, 1000, 6100, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, remote_node);

	event_set_destroy(&es);
}
END_TEST()

/*
 * Adds several tasks with remote and local memory accesses to an
 * event set and checks the id of the node targeted by the highest
 * amount of data including both read and accesses returned by
 * event_set_get_major_accessed_node_in_interval for different
 * intervals.
 */
UNIT_TEST(get_major_accessed_node_in_interval_test)
{
	struct event_set es;

	int major_node;

	static const int32_t local_node = 0;
	static const int32_t remote_node = 1;
	static const int32_t max_numa_node_id = 1;

	struct frame local_frame = {
		.numa_node = local_node
	};

	struct frame remote_frame = {
		.numa_node = remote_node
	};

	__init_event_set_with_memaccesses(&es, &local_frame, &remote_frame);

	/* Intervals without tasks */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 0, 1000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 2000, 3000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 5000, 6000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 6100, 7000, max_numa_node_id, &major_node), 0);
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 9000, 11000, max_numa_node_id, &major_node), 0);

	/* First task, entirely included */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 1000, 2000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Second task, entirely included */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 3000, 5000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, remote_node);

	/* Third task, entirely included */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 6000, 6100, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* First half of the first task */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 1000, 1500, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Tiny fraction of the first task */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 1000, 1001, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* Second half of the first task */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 1500, 2000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* First and second task */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 1000, 5000, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	/* All tasks */
	ASSERT_EQUALS(event_set_get_major_accessed_node_in_interval(&es, NULL, 1000, 6100, max_numa_node_id, &major_node), 1);
	ASSERT_EQUALS(major_node, local_node);

	event_set_destroy(&es);
}
END_TEST()

UNIT_TEST_SUITE(event_set_test)
	ADD_TEST(init_destroy_empty_test);

	ADD_TEST(add_single_state_event_test);
	ADD_TEST(add_multi_state_event_test);
	ADD_TEST(get_first_state_in_interval_test);
	ADD_TEST(get_first_state_starting_in_interval_test);
	ADD_TEST(get_first_state_starting_in_interval_type_test);
	ADD_TEST(get_next_state_event_test);
	ADD_TEST(get_major_state_empty_test);
	ADD_TEST(get_major_state_one_event_test);
	ADD_TEST(get_major_state_two_events_test);
	ADD_TEST(get_major_state_short_events_test);
	ADD_TEST(get_enclosing_state_test);
	ADD_TEST(add_single_comm_event_test);

	ADD_TEST(add_multi_comm_event_test);
	ADD_TEST(get_next_comm_event_test);
	ADD_TEST(get_next_comm_event_arr_test);
	ADD_TEST(get_first_comm_in_interval_test);
	ADD_TEST(get_last_comm_in_interval_test);
	ADD_TEST(get_last_comm_in_interval_type_test);

	ADD_TEST(add_single_single_event_test);
	ADD_TEST(add_multi_single_event_test);
	ADD_TEST(get_next_single_event_test);
	ADD_TEST(get_first_single_event_in_interval_test);
	ADD_TEST(get_last_single_event_in_interval_test);
	ADD_TEST(get_first_single_event_in_interval_type_test);
	ADD_TEST(get_last_single_event_in_interval_type_test);

	ADD_TEST(add_single_annotation_test);
	ADD_TEST(add_multi_annotation_test);
	ADD_TEST(find_get_annotation_in_interval_test);

	ADD_TEST(get_counter_event_set_test);
	ADD_TEST(has_counter_test);
	ADD_TEST(counters_monotonously_increasing_test);
	ADD_TEST(dump_per_task_counter_values_test);

	ADD_TEST(find_next_texec_start_for_frame_test);
	ADD_TEST(get_average_task_length_in_interval_test);
	ADD_TEST(__event_set_get_major_accessed_node_in_interval_test);
	ADD_TEST(get_min_task_duration_in_interval_test);
	ADD_TEST(get_max_task_duration_in_interval_test);
	ADD_TEST(get_min_max_task_duration_in_interval_test);

	ADD_TEST(get_task_duration_in_interval_test);
	ADD_TEST(get_remote_local_numa_bytes_in_interval_test)
	ADD_TEST(get_major_written_node_in_interval_test);
	ADD_TEST(get_major_read_node_in_interval_test);
	ADD_TEST(get_major_accessed_node_in_interval_test);
END_TEST_SUITE()
