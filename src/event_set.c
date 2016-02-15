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

#include "event_set.h"
#include "filter.h"
#include <stdlib.h>
#include <inttypes.h>

static int comm_event_compare_time(const void* p1, const void* p2)
{
	const struct comm_event* s1 = p1;
	const struct comm_event* s2 = p2;

	if(s1->time > s2->time)
		return 1;
	else if(s1->time < s2->time)
		return -1;

	return 0;
}

void event_set_sort_comm(struct event_set* es)
{
	qsort(es->comm_events, es->num_comm_events, sizeof(struct comm_event), comm_event_compare_time);
}

static int annotation_compare_time(const void* p1, const void* p2)
{
	const struct annotation* s1 = p1;
	const struct annotation* s2 = p2;

	if(s1->time > s2->time)
		return 1;
	else if(s1->time < s2->time)
		return -1;

	return 0;
}

void event_set_sort_annotations(struct event_set* es)
{
	qsort(es->annotations, es->num_annotations, sizeof(struct annotation), annotation_compare_time);
}

int event_set_compare_cpus(const void* p1, const void* p2)
{
	const struct event_set* s1 = p1;
	const struct event_set* s2 = p2;

	if(s1->cpu > s2->cpu)
		return 1;
	else if(s1->cpu < s2->cpu)
		return -1;

	return 0;
}

 /*
 * Returns the index of the first state event in es->state_events that
 * lies entirely within [interval_start; interval_end]. If no state
 * event exists that covers the specified interval, the function
 * returns -1.
 */
int event_set_get_first_state_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_state_events-1;
	int center_idx = 0;

	if(es->num_state_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->state_events[center_idx].start > interval_end)
			end_idx = center_idx-1;
		else if(es->state_events[center_idx].end < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx > 0 && es->state_events[center_idx-1].start < interval_end && es->state_events[center_idx-1].end > interval_start)
		center_idx--;

	if(es->state_events[center_idx].start > interval_end || es->state_events[center_idx].end < interval_start)
		return -1;

	return center_idx;
}

/*
 * Checks whether the interval defined by [start; end] contains at
 * least one state event that overlaps with the interval and that has
 * not been filtered out by f. If no such state is found, the function
 * return 0, otherwise 1.
 */
int event_set_has_state_in_interval(struct event_set* es, struct filter* f, uint64_t start, uint64_t end)
{
	int idx = event_set_get_first_state_in_interval(es, start, end);

	if(idx == -1)
		return 0;

	for(; idx < es->num_state_events && es->state_events[idx].start <= end;
	    idx++)
	{
		if(filter_has_state_event(f, &es->state_events[idx]))
			return 1;
	}

	return 0;
}

/*
 * Returns the index of the first state event in es->state_events that
 * starts within [interval_start; interval_end]. If no such state
 * event exists (e.g., if there are no state events in the specified
 * interval or if none of them starts in the interval), the function
 * returns -1.
 */
int event_set_get_first_state_starting_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int idx = event_set_get_first_state_in_interval(es, interval_start, interval_end);

	if(idx == -1)
		return -1;

	if(es->state_events[idx].start < interval_start) {
		while(idx+1 < es->num_state_events &&
		      es->state_events[idx].start < interval_start)
		{
			idx++;
		}
	}

	if(es->state_events[idx].start < interval_start ||
	   es->state_events[idx].start > interval_end)
		return -1;

	return idx;
}

/*
 * Returns the index of the first state event in es->state_events that
 * starts within [interval_start; interval_end] and that is of the
 * specified type. If no such state event exists (e.g., if there are
 * no state events in the specified interval, if there is no state
 * event of the specified type in the interval or if none of the state
 * events of the correct type starts in the interval), the function
 * returns -1.
 */
int event_set_get_first_state_starting_in_interval_type(struct event_set* es, uint64_t start, uint64_t end, int type)
{
	int idx = event_set_get_first_state_starting_in_interval(es, start, end);

	if(idx == -1)
		return -1;

	if(es->state_events[idx].state_id != type) {
		if((idx = event_set_get_next_state_event(es, idx, type)) != -1)
			if(es->state_events[idx].start > end)
				return -1;
	}

	return idx;
}

/* Returns the index of the counter event set associated to the
 * counter index counter_idx. If no such counter event set exists, the
 * function returns -1. */
int event_set_get_counter_event_set(struct event_set* es, int counter_idx)
{
	for(int i = 0; i < es->num_counter_event_sets; i++)
		if(es->counter_event_sets[i].desc->index == counter_idx)
			return i;

	return -1;
}

/* Returns the index of the next state event following the event with
 * the index curr_idx whose type is equal to the specified type. If no
 * such event exists (e.g., if there are no following events or if
 * their types do not match the specified type), the function returns
 * -1.
 */
int event_set_get_next_state_event(struct event_set* es, int curr_idx, int state)
{
	int idx = curr_idx+1;

	while(idx < es->num_state_events) {
		if(es->state_events[idx].state_id == state)
			return idx;

		idx++;
	}

	return -1;
}

/* Returns the index of the next communication event following the
 * event with the index curr_idx whose type is equal to the specified
 * type. If no such event exists (e.g., if there are no following
 * events or if their types do not match the specified type), the
 * function returns -1.
 */
int event_set_get_next_comm_event(struct event_set* es, int curr_idx, enum comm_event_type type)
{
	int idx = curr_idx+1;

	while(idx < es->num_comm_events) {
		if(es->comm_events[idx].type == type)
			return idx;

		idx++;
	}

	return -1;
}

/* Returns the index of the first communication event following the
 * event with the index curr_idx whose type is equal to one of the
 * types specified in the array types. If no such event exists (e.g.,
 * if there are no following events or if their types do not match any
 * of the specified types), the function returns -1.
 */
int event_set_get_next_comm_event_arr(struct event_set* es, int curr_idx, int num_types, enum comm_event_type* types)
{
	int idx = curr_idx+1;

	while(idx < es->num_comm_events) {
		for(int i = 0; i < num_types; i++)
			if(es->comm_events[idx].type == types[i])
				return idx;

		idx++;
	}

	return -1;
}

/* Determines for each type of state how much time was spent in that
 * state suring the interval defined by [start; end]. The filter f is
 * evaluated on each of the state events that overlap into the
 * interval. If no state event overlaps with the interval, the
 * function returns 0, otherwise 1. If break_half is set to 1
 * calculations stop as soon as one state occupies more than half of
 * the duration of the interval. If init is set to 1, the state
 * durations array is initialized with zeros.
 */
int event_set_get_state_durations(struct event_set* es, struct filter* f,
				  uint64_t start, uint64_t end,
				  int num_states, uint64_t* state_durations,
				  int init, int break_half)
{
	int idx_start = event_set_get_first_state_in_interval(es, start, end);
	uint64_t half = (end - start) / 2;

	if(init)
		memset(state_durations, 0, num_states * sizeof(uint64_t));

	if(idx_start == -1)
		return 0;

	for(int i = idx_start; i < es->num_state_events && es->state_events[i].start < end; i++) {
		if(filter_has_state_event(f, &es->state_events[i])) {
			state_durations[es->state_events[i].state_id_seq] +=
				state_event_length_in_interval(&es->state_events[i], start, end);

			if(break_half && state_durations[es->state_events[i].state_id_seq] > half)
				return 1;
		}
	}

	return 1;
}

/* Finds out which type of state dominates the interval defined by
 * [start; end]. If for two or more states the time is identical, the
 * type with the lowest id takes precedence. The filter f is evaluated
 * on each of the state events that overlap into the interval. State
 * events that are not filtered out are taken into account for the
 * determination of the dominant state. If there is a dominant state,
 * the function returns 1 and the sequential id of state is stored at
 * the address specified by major_state_seq. If no state event overlaps
 * with the interval, the function returns 0.
 */
int event_set_get_major_state_seq(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, int num_states, int* major_state_seq)
{
	uint64_t state_durations[num_states];
	uint64_t max = 0;

	if(!event_set_get_state_durations(es, f, start, end, num_states,
					  state_durations, 1, 1))
	{
		return 0;
	}

	for(int id_seq = 0; id_seq < num_states; id_seq++) {
		if(state_durations[id_seq] > max) {
			max = state_durations[id_seq];
			*major_state_seq = id_seq;
		}
	}

	return max > 0;
}

/* Returns the index of the state event that starts before or at the
 * indicated time and that ends after or at the indicated time. If no
 * such event exists, the function return -1. */
int event_set_get_enclosing_state(struct event_set* es, uint64_t time)
{
	int idx = event_set_get_first_state_in_interval(es, time, time);

	if(idx == -1)
		return -1;

	if(es->state_events[idx].start > time || es->state_events[idx].end < time)
		return -1;

	return idx;
}

/* Returns the index of the first communication event within the
 * interval [interval_start; interval_end]. If no such event exists,
 * the function return -1. */
int event_set_get_first_comm_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_comm_events-1;
	int center_idx = 0;

	if(es->num_comm_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->comm_events[center_idx].time > interval_end)
			end_idx = center_idx-1;
		else if(es->comm_events[center_idx].time < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx > 0 && es->comm_events[center_idx-1].time < interval_end && es->comm_events[center_idx-1].time > interval_start)
		center_idx--;

	if(center_idx >= 0 && center_idx <= es->num_comm_events-1) {
		if(es->comm_events[center_idx].time < interval_start ||
		   es->comm_events[center_idx].time > interval_end)
		{
			return -1;
		}
	}

	return center_idx;
}

/* Returns the index of the next single event following the event with
 * the index start_idx whose type is equal to the specified type. If
 * no such event exists (e.g., if there are no following events or if
 * their types do not match the specified type), the function returns
 * -1.
 */
int event_set_get_next_single_event(struct event_set* es, int start_idx, enum single_event_type type)
{
	int idx = start_idx+1;

	while(idx < es->num_single_events) {
		if(es->single_events[idx].type == type)
			return idx;

		idx++;
	}

	return -1;
}

/*
 * Returns the index of the first single event that lies within
 * [interval_start; interval_end]. If no single event exists in the
 * specified interval, the function returns -1.
 */
int event_set_get_first_single_event_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_single_events-1;
	int center_idx = 0;

	if(es->num_single_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->single_events[center_idx].time > interval_end)
			end_idx = center_idx-1;
		else if(es->single_events[center_idx].time < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx > 0 && es->single_events[center_idx-1].time < interval_end && es->single_events[center_idx-1].time >= interval_start)
		center_idx--;

	if(es->single_events[center_idx].time > interval_end || es->single_events[center_idx].time < interval_start)
		return -1;

	return center_idx;
}

/*
 * Returns the index of the last single event that lies within
 * [interval_start; interval_end]. If no single event exists in the
 * specified interval, the function returns -1.
 */
int event_set_get_last_single_event_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_single_events-1;
	int center_idx = 0;

	if(es->num_single_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->single_events[center_idx].time > interval_end)
			end_idx = center_idx-1;
		else if(es->single_events[center_idx].time < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx < es->num_single_events-1 && es->single_events[center_idx+1].time < interval_end && es->single_events[center_idx+1].time > interval_start)
		center_idx++;

	if(es->single_events[center_idx].time > interval_end || es->single_events[center_idx].time < interval_start)
		return -1;

	return center_idx;
}

/*
 * Returns the index of the first annotation that lies within
 * [interval_start; interval_end]. If no annotation exists in the
 * specified interval the function returns -1.
 */
int event_set_get_first_annotation_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_annotations-1;
	int center_idx = 0;

	if(es->num_annotations == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->annotations[center_idx].time > interval_end)
			end_idx = center_idx-1;
		else if(es->annotations[center_idx].time < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx > 0 && es->annotations[center_idx-1].time < interval_end && es->annotations[center_idx-1].time >= interval_start)
		center_idx--;

	if(es->annotations[center_idx].time > interval_end || es->annotations[center_idx].time < interval_start)
		return -1;

	return center_idx;
}

/*
 * Returns the index of the first single event of a certain type that
 * lies within [interval_start; interval_end]. If no single event
 * exists in the specified interval or if none of the single events in
 * the interval is of the requested type the function returns -1.
 */
int event_set_get_first_single_event_in_interval_type(struct event_set* es, uint64_t start, uint64_t end, enum single_event_type type)
{
	int idx = event_set_get_first_single_event_in_interval(es, start, end);

	if(idx == -1)
		return -1;

	while(es->single_events[idx].type != type && idx+1 < es->num_single_events)
		idx++;

	if(es->single_events[idx].type != type)
		return -1;

	return idx;
}

/*
 * Returns the index of the last single event of a certain type that
 * lies within [interval_start; interval_end]. If no single event
 * exists in the specified interval or if none of the single events in
 * the interval is of the requested type the function returns -1.
 */
int event_set_get_last_single_event_in_interval_type(struct event_set* es, uint64_t start, uint64_t end, enum single_event_type type)
{
	int idx = event_set_get_last_single_event_in_interval(es, start, end);

	if(idx == -1)
		return -1;

	while(es->single_events[idx].type != type && idx-1 >= 0)
		idx--;

	if(es->single_events[idx].type != type)
		return -1;

	return idx;
}

/*
 * Returns the index of the last communication event within the
 * interval [interval_start; interval_end]. If no such event exists,
 * the function return -1.
 */
int event_set_get_last_comm_event_in_interval(struct event_set* es, uint64_t interval_start, uint64_t interval_end)
{
	int start_idx = 0;
	int end_idx = es->num_comm_events-1;
	int center_idx = 0;

	if(es->num_comm_events == 0)
		return -1;

	while(end_idx - start_idx >= 0) {
		center_idx = (start_idx + end_idx) / 2;

		if(es->comm_events[center_idx].time > interval_end)
			end_idx = center_idx-1;
		else if(es->comm_events[center_idx].time < interval_start)
			start_idx = center_idx+1;
		else
			break;
	}

	while(center_idx < es->num_comm_events-1 && es->comm_events[center_idx+1].time < interval_end && es->comm_events[center_idx+1].time > interval_start)
		center_idx++;

	if(es->comm_events[center_idx].time > interval_end || es->comm_events[center_idx].time < interval_start)
		return -1;

	return center_idx;
}

/*
 * Returns the index of the last communication event within the
 * interval [interval_start; interval_end] that is of the specified
 * type. If no such event exists, the function return -1.
 */
int event_set_get_last_comm_event_in_interval_type(struct event_set* es, uint64_t start, uint64_t end, enum comm_event_type type)
{
	int idx = event_set_get_last_comm_event_in_interval(es, start, end);

	if(idx == -1)
		return -1;

	while(es->comm_events[idx].type != type && idx-1 >= 0)
		idx--;

	if(es->comm_events[idx].type != type)
		return -1;

	return idx;
}

/*
 * Finds the next task execution start event for a task whose
 * associated frame has the same address as the frame indicated by f
 * and whose timestamp is greater or equal than start. If no such
 * event is found the function returns NULL, otherwise a pointer to
 * the single event.
 */
struct single_event* event_set_find_next_texec_start_for_frame(struct event_set* es, uint64_t start, struct frame* f)
{
	int se_idx = event_set_get_first_single_event_in_interval_type(es, start, es->last_end, SINGLE_TYPE_TEXEC_START);
	struct single_event* se = &es->single_events[se_idx];

	if(se_idx == -1)
		return NULL;

	while(se && se->what->addr != f->addr)
		se = se->next_texec_start;

	return se;
}

/*
 * Calculates the average task duration in the interval [start;
 * end]. The output parameter num_tasks indicates how many tasks have
 * been considered. Non-integer values are possible when tasks overlap
 * only partially with the specified interval. If no task overlaps
 * with the interval or if all of the tasks in the interval are
 * filtered out the number of tasks is zero and the return value
 * should be ignored.
 */
uint64_t event_set_get_average_task_length_in_interval(struct event_set* es, struct filter* f, long double* num_tasks, uint64_t start, uint64_t end)
{
	long double lnum_tasks = 0.0;
	uint64_t length_in_interval;
	uint64_t length = 0;
	struct single_event* texec_start;
	int texec_start_idx;

	if(!filter_has_cpu(f, es->cpu))
		goto out;

	if((texec_start_idx = event_set_get_last_single_event_in_interval_type(es, 0, start, SINGLE_TYPE_TEXEC_START)) == -1)
		if((texec_start_idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START)) == -1)
			goto out;

	texec_start = &es->single_events[texec_start_idx];

	while(texec_start && texec_start->time < end) {
		uint64_t task_length = texec_start->next_texec_end->time - texec_start->time;

		if(!f || (filter_has_task(f, texec_start->active_task) &&
			  filter_has_frame(f, texec_start->active_frame) &&
			  filter_has_task_duration(f, task_length) &&
			  (!f-> filter_writes_to_numa_nodes || event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, texec_start->time, texec_start->next_texec_end->time, f->writes_to_numa_nodes_minsize))))
		{
			if(texec_start->time < start &&
			   texec_start->next_texec_end->time > start &&
			   texec_start->next_texec_end->time <= end)
			{
				/* Overlap of the end of the task
				 * execution with the interval:
				 *
				 *    start  end
				 *    [--------]
				 *  [------]
				 *  TS    TE
				 */
				length_in_interval = texec_start->next_texec_end->time - start;
				length += length_in_interval;
				lnum_tasks += ((long double)length_in_interval) /
					((long double)task_length);
			} else if(texec_start->time < start &&
				  texec_start->next_texec_end->time > end)
			{
				/* Full overlap, start and end
				 * outside of the interval:
				 *
				 *    start  end
				 *    [--------]
				 *  [------------]
				 *  TS          TE
				 */
				length_in_interval = end - start;
				length += length_in_interval;
				lnum_tasks += ((long double)length_in_interval) /
					((long double)task_length);
			} else if(texec_start->time >= start &&
				  texec_start->next_texec_end->time <= end)
			{
				/* Task execution entirely within
				 * the interval:
				 *
				 *    start  end
				 *    [--------]
				 *      [----]
				 *      TS  TE
				 */
				length_in_interval = texec_start->next_texec_end->time - texec_start->time;
				length += length_in_interval;
				lnum_tasks += ((long double)length_in_interval) /
					((long double)task_length);
			} else if(texec_start->time >= start &&
				  texec_start->next_texec_end->time > end)
			{
				/* Overlap of the start of the task
				 * execution with the interval:
				 *
				 *    start  end
				 *    [--------]
				 *         [------]
				 *         TS    TE
				 */
				length_in_interval = end - texec_start->time;
				length += length_in_interval;
				lnum_tasks += ((long double)length_in_interval) /
					((long double)task_length);
			}
		}

		texec_start = texec_start->next_texec_start;
	}

out:
	*num_tasks = lnum_tasks;
	return (uint64_t)(((long double)(length)) / lnum_tasks);
}

/*
 * Determines the node that was accessed in majority in the interval
 * [start; end] with respect to the communication types specified in
 * types[0..num_types-1]. The major node is returned in
 * *major_node. If no node such node exists (e.g., if no communication
 * takes place in the specified interval or if all communication in
 * the interval is filtered out by f) the return value of the function
 * is 0. If there is a major node, the return value is different from
 * 0. If there are two nodes with the same amount of data the node
 * with the lowest id is returned in *major_node.
 *
 * The function assumes that communication happens continuously
 * throughout the execution of each task that overlaps with the
 * specified interval. The amount of data that is communicated in the
 * interval is thus assumed to be equivalent to the fraction of the
 * task duration within the interval multiplied with the total amount
 * of data communicated by the task.
 *
 * For example, if a task reads 1000 bytes, 2000 bytes and 3000 bytes
 * from nodes 1, 2 and 1, respectively and if each of these accesses
 * matches a type in types[0..num_types-1] and if the events are not
 * filtered out by f, the function assumes that the task accesses 6000
 * bytes in total during its execution, with 4000 bytes read from node
 * 1 and 2000 bytes read from node 2. If only 50% of the execution
 * time of the task overlaps with the specified interval, the function
 * assumes that 2000 bytes are read from node 1 and 1000 bytes are
 * read from node 2. As the accesses are assumed to take place
 * continuously, it does not matter whether the actual communication
 * events take place within the specified interval or not.
 */
int __event_set_get_major_accessed_node_in_interval(struct event_set* es, enum comm_event_type* types, unsigned int num_types, struct filter* f, uint64_t start, uint64_t end, int max_numa_node_id, int* major_node)
{
	uint64_t node_data[max_numa_node_id+1];
	uint64_t max = 0;
	uint64_t task_time;
	int texec_start_idx;
	long double time_fraction;
	long double data_fraction;
	struct single_event* texec_start;
	struct single_event* texec_end;

	/* Find first task execution starting in interval or the last
	 * one starting right before the interval. */
	if((texec_start_idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START)) == -1)
		if((texec_start_idx = event_set_get_last_single_event_in_interval_type(es, 0, start, SINGLE_TYPE_TEXEC_START)) == -1)
			return 0;

	texec_start = &es->single_events[texec_start_idx];

	/* There might be a previous task starting before the
	 * interval, but ending in the interval */
	if(texec_start->prev_texec_end && texec_start->prev_texec_end->time >= start)
		texec_start = texec_start->prev_texec_start;

	memset(node_data, 0, (max_numa_node_id+1)*sizeof(node_data[0]));

	for(; texec_start && texec_start->time < end; texec_start = texec_start->next_texec_start) {
		if(filter_has_single_event(f, texec_start)) {
			texec_end = texec_start->next_texec_end;

			task_time = task_length_in_interval(texec_start, texec_end, start, end);
			time_fraction = ((long double)task_time) / ((long double)(texec_end->time - texec_start->time));

			struct comm_event* ce;
			for_each_comm_event_in_interval(es,
							texec_start->time,
							texec_start->next_texec_end->time,
							ce)
			{
				for(unsigned int i = 0; i < num_types; i++) {
					if(ce->type == types[i] && ce->what->numa_node != -1) {
						data_fraction = ((long double)ce->size * time_fraction);
						node_data[ce->what->numa_node] += data_fraction;
						break;
					}
				}
			}
		}
	}

	for(int node = 0; node < (max_numa_node_id+1); node++) {
		if(node_data[node] > max) {
			max = node_data[node];
			*major_node = node;
		}
	}

	return max > 0;
}

/*
 * Determines the duration of the shortest task that starts and
 * terminates within the interval [start; end] and that is not
 * filtered out by f. If no task starts and finishes in the interval
 * or if all tasks starting in the interval are filtered out the
 * function returns 0. If a task is found, its duration is returned in
 * *duration and the return value is different from 0.
 */
int event_set_get_min_task_duration_in_interval(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, uint64_t* duration)
{
	int idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START);

	if (idx == -1)
		return 0;

	uint64_t min = UINT64_MAX;
	uint64_t curr;

	for(struct single_event* se = &es->single_events[idx];
	    se && se->next_texec_end->time <= end;
	    se = se->next_texec_start)
	{
		curr = se->next_texec_end->time - se->time;

		if(curr < min && (!f || (filter_has_task(f, se->active_task) &&
		   filter_has_frame(f, se->active_frame) &&
		   filter_has_task_duration(f, curr) &&
		   (!f-> filter_writes_to_numa_nodes ||
		    event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize)))))
			min = curr;
	}

	*duration = min;

	return (min < UINT64_MAX);
}

/*
 * Determines the duration of the shortest and the longest task that
 * starts and terminates within the interval [start; end] and that is
 * not filtered out by f. If no task starts and finishes in the
 * interval or if all tasks starting in the interval are filtered out
 * the function returns 0. If at least one task is found, the minimal
 * and maximal durations are returned in *min and *max and the return
 * value is different from 0.
 */
int event_set_get_min_max_task_duration_in_interval(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, uint64_t* min, uint64_t* max)
{
	int idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START);

	if (idx == -1)
		return 0;

	uint64_t lmin = UINT64_MAX;
	uint64_t lmax = 0;
	uint64_t curr;

	for(struct single_event* se = &es->single_events[idx];
	    se && se->next_texec_end->time <= end;
	    se = se->next_texec_start)
	{
		curr = se->next_texec_end->time - se->time;

		if(!f || (filter_has_task(f, se->active_task) &&
		   filter_has_frame(f, se->active_frame) &&
		   filter_has_task_duration(f, curr) &&
		   (!f-> filter_writes_to_numa_nodes ||
		    event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize))))
		{
			if(curr < lmin)
				lmin = curr;
			if(curr > lmax)
				lmax = curr;
		}
	}

	*min = lmin;
	*max = lmax;

	return (lmin < UINT64_MAX);
}

/*
 * Determines the duration of the longest task that starts and
 * terminates within the interval [start; end] and that is not
 * filtered out by f. If no task starts and finishes in the interval
 * or if all tasks starting in the interval are filtered out the
 * function returns 0. If a task is found, its duration is returned in
 * *duration and the return value is different from 0.
 */
int event_set_get_max_task_duration_in_interval(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, uint64_t* duration)
{
	int idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START);

	if (idx == -1)
		return 0;

	uint64_t max = 0;
	uint64_t curr;

	for(struct single_event* se = &es->single_events[idx];
	    se && se->next_texec_end->time <= end;
	    se = se->next_texec_start)
	{
		curr = se->next_texec_end->time - se->time;

		if(curr > max && (!f || (filter_has_task(f, se->active_task) &&
		   filter_has_frame(f, se->active_frame) &&
		   filter_has_task_duration(f, curr) &&
		   (!f-> filter_writes_to_numa_nodes ||
		    event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize)))))
			max = curr;
	}

	*duration = max;

	return (max > 0);
}

/*
 * Calculates the duration of each task type within the interval
 * [start; end]. The cumulative durarion for each task type is
 * returned in durations[0..maxid], where maxid is the highest task
 * type identifier. The function only takes into account task
 * instances that are not filtered out by f. Task instances that
 * partially overlap with the specified interval are taken into
 * account.
 *
 * If at least one task instance that is not filtered out and that
 * overlaps with the interval the function returns 1, otherwise the
 * return value is 0.
 */
int event_set_get_task_duration_in_interval(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, uint64_t* durations)
{
	int idx;
	uint64_t curr;
	int valid = 0;

	if((idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START)) == -1) {
		if((idx = event_set_get_last_single_event_in_interval_type(es, 0, start, SINGLE_TYPE_TEXEC_START)) == -1)
			return 0;

		if(es->single_events[idx].next_texec_end->time < start)
			return 0;
	}

	for(struct single_event* se = &es->single_events[idx];
	    se && se->time <= end;
	    se = se->next_texec_start)
	{
		curr = se->next_texec_end->time - se->time;

		if(!f ||
		   (filter_has_task(f, se->active_task) &&
		    filter_has_frame(f, se->active_frame) &&
		    filter_has_task_duration(f, curr) &&
		    (!f->filter_writes_to_numa_nodes ||
		     event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize))))
		{
			if(se->time < start && se->next_texec_end->time > start && se->next_texec_end->time < end) {
				curr -= (start - se->time);
			} else if(se->time < start && se->next_texec_end->time > end) {
				curr -= (start - se->time);
				curr -= (se->next_texec_end->time - end);
			} else if(se->time > start && se->next_texec_end->time > end) {
				curr -= (se->next_texec_end->time - end);
			}

			durations[se->active_task->id] += curr;
			valid = 1;
		}
	}

	return valid;
}

/*
 * Write per-task values to a file at the current position of 'file'
 * for all counters and tasks not filtered out by f. A per-task value
 * is calculated by subtracting the number of events at the beginning
 * of the task execution from the number of events at the end of the
 * execution. The number of taks not filtered out, but for which the
 * number of events could not be determined (e.g., due to missing
 * samples) is returned in *nb_errors_out.
 *
 * For tasks whose timestamps for the start and end do not correspond
 * to timestamps of counter samples, the values are interpolated
 * linearly using the neighboring samples.
 */
void event_set_dump_per_task_counter_values(struct event_set* es, struct filter* f, FILE* file, int* nb_errors_out)
{
	struct counter_event_set* ces;
	struct single_event* first_texec_start;
	int64_t value_start, value_end;
	int64_t nb_events;
	int nb_errors = 0;

	int idx = event_set_get_first_single_event_in_interval_type(es, es->first_start, es->last_end, SINGLE_TYPE_TEXEC_START);

	if(idx == -1)
		return;

	first_texec_start = &es->single_events[idx];

	for(int ctr_ev_idx = 0; ctr_ev_idx < es->num_counter_event_sets; ctr_ev_idx++) {
		ces = &es->counter_event_sets[ctr_ev_idx];

		if(!filter_has_counter(f, ces->desc))
			continue;

		for(struct single_event* se = first_texec_start;
		    se;
		    se = se->next_texec_start)
		{
			if(!f ||
			   (filter_has_task(f, se->active_task) &&
			    filter_has_frame(f, se->active_frame) &&
			    filter_has_cpu(f, es->cpu) &&
			    filter_has_task_duration(f, se->next_texec_end->time - se->time) &&
			    (!f->filter_writes_to_numa_nodes ||
			     event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize))))
			{
				if(counter_event_set_interpolate_value(ces, se->time, &value_start)) {
					nb_errors++;
					continue;
				}

				if(counter_event_set_interpolate_value(ces, se->next_texec_end->time, &value_end)) {
					nb_errors++;
					continue;
				}

				nb_events = value_end - value_start;

				if (se->active_task->symbol_name != NULL)
					fprintf(file, "%s %s %d %"PRIu64" %"PRId64"\n", se->active_task->symbol_name, ces->desc->name, es->cpu, (se->next_texec_end->time - se->time), nb_events);
				else
					fprintf(file, "(no-symbol-found) %s %d %"PRIu64" %"PRId64"\n", ces->desc->name, es->cpu, (se->next_texec_end->time - se->time), nb_events);
			}
		}
	}

	*nb_errors_out = nb_errors;
}

/*
 * Checks whether the samples of the all counters of the event set
 * that are not filtered out by f are monotonously increasing. If
 * there is at least one counter for which the samples do not increase
 * monotonously, the function sets *cd to the according counter
 * description and returns 0. If all counters have monotonously
 * increasing samples, the function returns 1.
 */
int event_set_counters_monotonously_increasing(struct event_set* es, struct filter* f, struct counter_description** cd)
{
	for(int idx = 0; idx < es->num_counter_event_sets; idx++) {
		if(!f ||
		   (filter_has_counter(f, es->counter_event_sets[idx].desc) &&
		    filter_has_cpu(f, es->cpu)))
		{
			if(!counter_event_set_is_monotonously_increasing(&es->counter_event_sets[idx])) {
				*cd = es->counter_event_sets[idx].desc;
				return 0;
			}
		}
	}

	return 1;
}

/* Checks whether the event set has a counter event set referring to
 * the counter descirbed by the counter description in cd. Returns 1
 * on success and 0 if no such counter event set was found. */
int event_set_has_counter(struct event_set* es, struct counter_description* cd)
{
	for (int ctr_idx = 0; ctr_idx < es->num_counter_event_sets; ctr_idx++)
		if (es->counter_event_sets[ctr_idx].desc->counter_id == cd->counter_id)
			return 1;

	return 0;
}

/* Counts the number of bytes accessed locally and remotely for the
 * event set es in the interval defined by [start; end]. The function
 * only considers tasks that are not filtered out by the filter
 * f. Tasks not entirely included in the interval contribute as if all
 * memory accesses of the task, including those outside the interval
 * [start; end] happened grandually. The identifier of the local node
 * of the cpu associated to the event set must be specified in
 * local_node. The result is stored in local_bytes and remote_bytes.
 *
 * The function returns 1 if at least one task was not filtered out. A
 * return value of 0 indicates that no task in the interval passed the
 * filter.
 */
int event_set_get_remote_local_numa_bytes_in_interval(struct event_set* es,
						      struct filter* f,
						      uint64_t start,
						      uint64_t end,
						      uint32_t local_node,
						      uint64_t* local_bytes,
						      uint64_t* remote_bytes)
{
	uint64_t length_in_interval;
	uint64_t length = 0;
	struct single_event* texec_start;
	int texec_start_idx;

	*local_bytes = 0;
	*remote_bytes = 0;

	if(!filter_has_cpu(f, es->cpu))
		goto out;

	/* Find first task execution starting in interval or the last
	 * one starting right before the interval. */
	if((texec_start_idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START)) == -1)
		if((texec_start_idx = event_set_get_last_single_event_in_interval_type(es, 0, start, SINGLE_TYPE_TEXEC_START)) == -1)
			goto out;

	texec_start = &es->single_events[texec_start_idx];

	/* There might be a previous task starting before the
	 * interval, but ending in the interval */
	if(texec_start->prev_texec_end && texec_start->prev_texec_end->time >= start)
		texec_start = texec_start->prev_texec_start;

	while(texec_start && texec_start->time < end) {
		uint64_t task_length = texec_start->next_texec_end->time - texec_start->time;

		/* Check if task is included in the filter */
		if(!f || (filter_has_task(f, texec_start->active_task) &&
			  filter_has_frame(f, texec_start->active_frame) &&
			  filter_has_task_duration(f, task_length) &&
			  (!f-> filter_writes_to_numa_nodes || event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, texec_start->time, texec_start->next_texec_end->time, f->writes_to_numa_nodes_minsize))))
		{
			uint64_t task_length = texec_start->next_texec_end->time - texec_start->time;
			length_in_interval = task_length_in_interval(texec_start, texec_start->next_texec_end, start, end);
			length += length_in_interval;

			struct comm_event* ce;

			/* Add up all communication of the task and
			 * add the contribution to
			 * {local,remote}_bytes according to the
			 * overlap of the task with the interval */
			for_each_comm_event_in_interval(es,
							texec_start->time,
							texec_start->next_texec_end->time,
							ce)
			{
				if(ce->what->numa_node != -1) {
					if(ce->what->numa_node == local_node)
						*local_bytes += (length_in_interval * ce->size) / task_length;
					else
						*remote_bytes += (length_in_interval * ce->size) / task_length;
				}
			}
		}

		texec_start = texec_start->next_texec_start;
	}

out:
	return (length > 0);
}

/* Determines to which node the majority of the data in the interval
 * defined by [start; end] was written. The function only considers
 * tasks that are not filtered out by the filter f. Tasks not entirely
 * included in the interval contribute as if all writes of the task,
 * including those outside the interval [start; end] happened
 * grandually. The result is provided in major_node. The parameter
 * max_numa_node_id must be set to the highest possible identifier of
 * a NUMA node within the trace.
 *
 * The function returns 1 if at least one write access was not
 * filtered out. A return value of 0 indicates that no write access in
 * the interval passed the filter.
 */
int event_set_get_major_written_node_in_interval(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, int max_numa_node_id, int* major_node)
{
	enum comm_event_type t = COMM_TYPE_DATA_WRITE;
	return __event_set_get_major_accessed_node_in_interval(es, &t, 1, f, start, end, max_numa_node_id, major_node);
}

/* Determines from which node the majority of the data in the interval
 * defined by [start; end] was read. The function only considers tasks
 * that are not filtered out by the filter f. Tasks not entirely
 * included in the interval contribute as if all reads of the task,
 * including those outside the interval [start; end] happened
 * grandually. The result is provided in major_node. The parameter
 * max_numa_node_id must be set to the highest possible identifier of
 * a NUMA node within the trace.
 *
 * The function returns 1 if at least one read access was not filtered
 * out. A return value of 0 indicates that no read access in the
 * interval passed the filter.
 */
int event_set_get_major_read_node_in_interval(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, int max_numa_node_id, int* major_node)
{
	enum comm_event_type t = COMM_TYPE_DATA_READ;
	return __event_set_get_major_accessed_node_in_interval(es, &t, 1, f, start, end, max_numa_node_id, major_node);
}

/* Determines the node from which / to which the majority of the data
 * in the interval defined by [start; end] was read / written. The
 * function only considers tasks that are not filtered out by the
 * filter f. Tasks not entirely included in the interval contribute as
 * if all memory of the task, including those outside the interval
 * [start; end] happened grandually. The result is provided in
 * major_node. The parameter max_numa_node_id must be set to the
 * highest possible identifier of a NUMA node within the trace.
 *
 * The function returns 1 if at least one memory access was not
 * filtered out. A return value of 0 indicates that no access in the
 * interval passed the filter.
 */
int event_set_get_major_accessed_node_in_interval(struct event_set* es, struct filter* f, uint64_t start, uint64_t end, int max_numa_node_id, int* major_node)
{
	enum comm_event_type t[2] = { COMM_TYPE_DATA_READ, COMM_TYPE_DATA_WRITE };
	return __event_set_get_major_accessed_node_in_interval(es, t, 2, f, start, end, max_numa_node_id, major_node);
}
