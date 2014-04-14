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

#include "statistics.h"
#include <inttypes.h>

void state_statistics_init(struct state_statistics* s)
{
	memset(s->state_cycles, 0, sizeof(s->state_cycles));
}

void state_statistics_gather_cycles(struct multi_event_set* mes, struct filter* f, struct state_statistics* s, int64_t start, int64_t end)
{
	int state_idx;
	struct state_event* se;

	for(int cpu_idx = 0; cpu_idx < mes->num_sets; cpu_idx++) {
		if((state_idx = event_set_get_first_state_in_interval(&mes->sets[cpu_idx], start, end)) == -1)
			continue;

		se = &mes->sets[cpu_idx].state_events[state_idx];

		while(state_idx < mes->sets[cpu_idx].num_state_events &&
		      se->start < end)
		{
			if(filter_has_state_event(f, se))
				s->state_cycles[se->state] += state_event_length_in_interval(se, start, end);

			state_idx++;
			se++;
		}
	}
}

void single_event_statistics_init(struct single_event_statistics* s)
{
	s->num_tcreate_events = 0;
}

void single_event_statistics_gather(struct multi_event_set* mes, struct filter* f, struct single_event_statistics* s, int64_t start, int64_t end)
{
	int evt_idx;
	struct single_event* sge;

	for(int cpu_idx = 0; cpu_idx < mes->num_sets; cpu_idx++) {
		if((evt_idx = event_set_get_first_single_event_in_interval(&mes->sets[cpu_idx], start, end)) == -1)
			continue;

		sge = &mes->sets[cpu_idx].single_events[evt_idx];

		while(evt_idx < mes->sets[cpu_idx].num_single_events &&
		      sge->time < end)
		{
			if(sge->type == SINGLE_TYPE_TCREATE && filter_has_single_event(f, sge))
				s->num_tcreate_events++;

			evt_idx++;
			sge++;
		}
	}
}

void task_statistics_reset(struct task_statistics* s)
{
	s->num_tasks = 0;
	s->cycles = 0;
	s->max_hist = 0;
	s->max_cycles = 0;
	s->min_cycles = UINT64_MAX;

	memset(s->cycles_hist, 0, sizeof(s->cycles_hist[0]) * s->num_hist_bins);
}

int task_statistics_init(struct task_statistics* s, unsigned int num_hist_bins)
{
	s->num_hist_bins = num_hist_bins;

	if(!(s->cycles_hist = malloc(sizeof(s->cycles_hist[0]) * num_hist_bins)))
		return 1;

	task_statistics_reset(s);

	return 0;
}

void task_statistics_destroy(struct task_statistics* s)
{
	free(s->cycles_hist);
}

void task_statistics_gather(struct multi_event_set* mes, struct filter* f, struct task_statistics* s, int64_t start, int64_t end)
{
	int start_idx;
	uint64_t curr_length;
	int hist_bin;
	int first_texec_idx;

	for(int histogram = 0; histogram < 2; histogram++) {
		for(int cpu_idx = 0; cpu_idx < mes->num_sets; cpu_idx++) {
			if((start_idx = event_set_get_first_single_event_in_interval_type(&mes->sets[cpu_idx], start, end, SINGLE_TYPE_TEXEC_START)) == -1)
				continue;

			if(!filter_has_cpu(f, mes->sets[cpu_idx].cpu))
				continue;


			struct single_event* se = &mes->sets[cpu_idx].single_events[start_idx];

			while(se && se->next_texec_end && se->next_texec_end->time <= end) {
				if(!filter_has_frame(f, se->active_frame))
					goto next;

				curr_length = se->next_texec_end->time - se->time;

				if(!filter_has_task_duration(f, curr_length))
					goto next;

				first_texec_idx = event_set_get_first_state_starting_in_interval_type(&mes->sets[cpu_idx],
												      se->time,
												      se->next_texec_end->time,
												      WORKER_STATE_TASKEXEC);

				if(first_texec_idx == -1) {
					fprintf(stderr, "Oops: no texec state found between texec start "
						"and texec end (CPU %d, [%"PRIu64" ; %"PRIu64"])!\n",
						cpu_idx, se->time, se->next_texec_end->time);
					goto next;
				}

				if(!filter_has_task(f, mes->sets[cpu_idx].state_events[first_texec_idx].active_task))
					goto next;

				if(f->filter_writes_to_numa_nodes &&
				   !event_set_has_write_to_numa_nodes_in_interval(&mes->sets[cpu_idx], &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize))
					goto next;

				if(histogram) {
					if(s->max_cycles - s->min_cycles > 0)
						hist_bin = ((uint64_t)(s->num_hist_bins-1) * (curr_length - s->min_cycles)) / (s->max_cycles - s->min_cycles);
					else
						hist_bin = 0;

					s->cycles_hist[hist_bin]++;
				} else {
					s->cycles += curr_length;
					s->num_tasks++;

					if(curr_length > s->max_cycles)
						s->max_cycles = curr_length;
					if(curr_length < s->min_cycles)
						s->min_cycles = curr_length;
				}

next:
				se = se->next_texec_start;
			}
		}
	}

	for(int i = 0; i < s->num_hist_bins; i++) {
		if(s->cycles_hist[i] > s->max_hist)
			s->max_hist = s->cycles_hist[i];
	}
}

int task_statistics_to_task_length_histogram(struct task_statistics* s, struct histogram* h)
{
	if(s->num_hist_bins != h->num_bins)
		return 1;

	h->left = 0;
	h->right = s->max_cycles;
	h->max_hist = s->max_hist;
	h->num_hist = s->num_tasks;

	for(int i = 0; i < h->num_bins; i++) {
		if(s->max_hist > 0)
			h->values[i] = (long double)s->cycles_hist[i] / (long double)s->max_hist;
		else
			h->values[i] = 0;
	}

	return 0;
}

int histogram_init(struct histogram* h, unsigned int num_bins, long double left, long double right)
{
	h->num_bins = num_bins;
	h->left = left;
	h->right = right;
	h->max_hist = 0;
	h->num_hist = 0;

	if(!(h->values = calloc(sizeof(h->values[0]), num_bins)))
		return 1;

	return 0;
}

void histogram_destroy(struct histogram* h)
{
	free(h->values);
}

int numa_node_exchange_matrix_gather(struct multi_event_set* mes, struct filter* f, struct intensity_matrix* m, int64_t start, int64_t end, int comm_type_mask, int exclude_reflexive, int ignore_direction, int num_only)
{
	if(intensity_matrix_init(m, mes->max_numa_node_id+1, mes->max_numa_node_id+1))
		return 1;

	for(struct event_set* es = mes->sets; es < &mes->sets[mes->num_sets]; es++) {
		int first_comm_index = event_set_get_first_comm_in_interval(es, start, end);

		if(first_comm_index == -1)
			continue;

		for(struct comm_event* ce = &es->comm_events[first_comm_index];
		    ce < &es->comm_events[es->num_comm_events] && ce->time <= end;
		    ce++)
		{
			int in_mask = (1 << ce->type) & comm_type_mask;

			if(!in_mask || !filter_has_comm_event(f, mes, ce) ||
			   (exclude_reflexive && es->numa_node == ce->what->numa_node))
				continue;

			int src = es->numa_node;
			int dst = ce->what->numa_node;
			int size = (num_only) ? 1 : ce->size;

			if(src == -1 || dst == -1)
				continue;

			if(ignore_direction) {
				if(src > dst) {
					int tmp = src;
					src = dst;
					dst = tmp;
				}
			}

			intensity_matrix_add_absolute_value_at(m, src, dst, size);
		}
	}

	intensity_matrix_update_intensity(m);

	return 0;
}
