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
#include "dialogs.h"
#include <math.h>
#include "globals.h"

void state_statistics_init(struct state_statistics* s)
{
	s->state_cycles = malloc(sizeof(uint64_t) * g_mes.num_states);
	memset(s->state_cycles, 0, sizeof(uint64_t) * g_mes.num_states);
}

void state_statistics_gather_cycles(struct multi_event_set* mes, struct filter* f, struct state_statistics* s, int64_t start, int64_t end)
{
	int state_idx;
	struct state_event* se;
	struct event_set* es;

	for_each_event_set(mes, es) {
		if((state_idx = event_set_get_first_state_in_interval(es, start, end)) == -1)
			continue;

		se = &es->state_events[state_idx];

		while(state_idx < es->num_state_events &&
		      se->start < end)
		{
			if(filter_has_state_event(f, se))
				s->state_cycles[se->state_id_seq] += state_event_length_in_interval(se, start, end);

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
	struct event_set* es;

	for_each_event_set(mes, es) {
		if((evt_idx = event_set_get_first_single_event_in_interval(es, start, end)) == -1)
			continue;

		sge = &es->single_events[evt_idx];

		while(evt_idx < es->num_single_events && sge->time < end) {
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

/* Initializes a multi_task_statistics structure with num_task_stats
 * entries each with num_hist_bins bins. */
int multi_task_statistics_init(struct multi_task_statistics* ms, int num_tasks_stats, unsigned int num_hist_bins)
{
	ms->num_tasks_stats = num_tasks_stats;
	ms->min_all = UINT64_MAX;
	ms->max_all = 0;
	int idx;

	if(!(ms->stats = calloc(num_tasks_stats, sizeof(struct task_statistics*))))
		goto out_err;

	for(idx = 0; idx < num_tasks_stats; idx++) {
		if(!(ms->stats[idx] = malloc(sizeof(struct task_statistics)))) {
			show_error_message("Cannot allocate task statistics structure in multi task statistics\n");
			goto out_err_destroy;
		}

		if(task_statistics_init(ms->stats[idx], num_hist_bins))
			goto out_err_free;
	}

	if(!(ms->task_ids = calloc(num_tasks_stats, sizeof(int)))) {
		show_error_message("Cannot allocate task identifiers array in multi task statistics\n");
		goto out_err_destroy;
	}

	return 0;

out_err_free:
	free(ms->stats[idx]);
out_err_destroy:
	for(int i = 0; i < idx; i++) {
		task_statistics_destroy(ms->stats[i]);
		free(ms->stats[i]);
	}
out_err:
	return 1;
}

/* Recursively destroys a multi_task_statistics structure */
void multi_task_statistics_destroy(struct multi_task_statistics* ms)
{
	for(int idx = 0; idx < ms->num_tasks_stats; idx++)
		task_statistics_destroy(ms->stats[idx]);

	free(ms->task_ids);
	free(ms->stats);
}

void task_statistics_gather(struct multi_event_set* mes, struct filter* f, struct task_statistics* s, int64_t start, int64_t end)
{
	int start_idx;
	uint64_t curr_length;
	int hist_bin;
	struct event_set* es;

	for(int histogram = 0; histogram < 2; histogram++) {
		for_each_event_set(mes, es) {
			if((start_idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START)) == -1)
				continue;

			if(!filter_has_cpu(f, es->cpu))
				continue;


			struct single_event* se = &es->single_events[start_idx];

			while(se && se->next_texec_end && se->next_texec_end->time <= end) {
				if(!filter_has_frame(f, se->active_frame))
					goto next;

				curr_length = se->next_texec_end->time - se->time;

				if(!filter_has_task_duration(f, curr_length))
					goto next;

				if(!filter_has_task(f, se->active_task))
					goto next;

				if(f->filter_writes_to_numa_nodes &&
				   !event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize))
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

void task_statistics_gather_with_min_max_cycles(struct multi_event_set* mes, struct filter* f, struct task_statistics* s, int64_t start, int64_t end, uint64_t min_cycles, uint64_t max_cycles)
{
	int start_idx;
	uint64_t curr_length;
	int hist_bin;
	struct event_set* es;

	for_each_event_set(mes, es) {
		if((start_idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START)) == -1)
			continue;

		if(!filter_has_cpu(f, es->cpu))
			continue;


		struct single_event* se = &es->single_events[start_idx];

		while(se && se->next_texec_end && se->next_texec_end->time <= end) {
			if(!filter_has_frame(f, se->active_frame))
				goto next;

			curr_length = se->next_texec_end->time - se->time;

			if(!filter_has_task_duration(f, curr_length))
				goto next;

			if(!filter_has_task(f, se->active_task))
				goto next;

			if(f->filter_writes_to_numa_nodes &&
			   !event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize))
				goto next;

			s->cycles += curr_length;
			s->num_tasks++;

			if(max_cycles - min_cycles > 0)
				hist_bin = ((uint64_t)(s->num_hist_bins-1) * (curr_length - min_cycles)) / (max_cycles - min_cycles);
			else
				hist_bin = 0;

			s->cycles_hist[hist_bin]++;
next:
			se = se->next_texec_start;

		}
	}

	for(int i = 0; i < s->num_hist_bins; i++) {
		if(s->cycles_hist[i] > s->max_hist)
			s->max_hist = s->cycles_hist[i];
	}
}

int array_has_task(struct task** tasks, int length, struct task* t)
{
	for(int idx = 0; idx < length; idx++)
		if (tasks[idx]->id == t->id)
			return 1;

	return 0;
}

int multi_task_statistics_shrink(struct multi_task_statistics* ms, int num_tasks_stats)
{
	for(int task_idx = num_tasks_stats; task_idx < ms->num_tasks_stats; task_idx++)
		task_statistics_destroy(ms->stats[task_idx]);

	if(num_tasks_stats == 0) {
		free(ms->stats);
		free(ms->task_ids);

		ms->stats = NULL;
		ms->task_ids = NULL;
	} else {
		if(!(ms->stats = realloc(ms->stats, num_tasks_stats*sizeof(struct task_statistics*))))
			return 1;

		if(!(ms->task_ids = realloc(ms->task_ids, num_tasks_stats*sizeof(int*))))
			return 1;
	}

	ms->num_tasks_stats = num_tasks_stats;

	return 0;
}

int multi_task_statistics_gather(struct multi_event_set* mes, struct filter* f, struct multi_task_statistics* ms, int64_t start, int64_t end)
{
	struct task* tasks_in_filter[mes->num_tasks];
	int nb_tasks_in_filter = 0;
	uint64_t min_all, max_all;
	int reset_filter = 0;
	int ms_index = 0;
	struct task* t;

	if(f) {
		if(!(f->filter_tasks)) {
			reset_filter = 1;
		} else {
			nb_tasks_in_filter = f->num_tasks;
			memcpy(tasks_in_filter, f->tasks, f->num_tasks*sizeof(struct task*));
		}

		multi_event_set_get_min_max_task_duration_in_interval(mes, f, start, end, &min_all, &max_all);

		ms->min_all = min_all;
		ms->max_all = max_all;

		for_each_task(mes, t) {
			filter_clear_tasks(f);

			if(!(reset_filter || array_has_task(tasks_in_filter, nb_tasks_in_filter, t)))
				continue;

			filter_add_task(f, t);
			task_statistics_gather_with_min_max_cycles(mes, f, ms->stats[ms_index], start, end, min_all, max_all);

			if(ms->stats[ms_index]->num_tasks == 0)
				continue;

			ms->task_ids[ms_index] = t->id;
			ms_index++;
		}

		if(ms_index < ms->num_tasks_stats)
			if (multi_task_statistics_shrink(ms, ms_index))
				return 1;

		if(reset_filter) {
			filter_clear_tasks(f);
			filter_set_task_filtering(f, 0);
		} else {
			for(int task_idx = 0; task_idx < nb_tasks_in_filter; task_idx++)
				filter_add_task(f, tasks_in_filter[task_idx]);
		}
	}

	return 0;
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

long double sum_bin_values(struct multi_task_statistics* ms, unsigned int nb_bins, long double* values)
{
	long double sum;
	long double max_sum = 0;

	for(int bin_idx = 0; bin_idx < nb_bins; bin_idx++) {
		sum = 0;

		for(int i = 0; i < ms->num_tasks_stats; i++)
			sum += (long double) ms->stats[i]->cycles_hist[bin_idx];

		values[bin_idx] = sum;

		if(sum > max_sum)
			max_sum = sum;
	}

	return max_sum;
}

int multi_task_statistics_to_task_length_multi_histogram(struct multi_task_statistics* ms, struct multi_histogram* mh)
{
      	long double values[mh->num_hist_bins];
	long double max_value = sum_bin_values(ms, mh->num_hist_bins, values);

	for(int bin_idx = 0; bin_idx < mh->num_hist_bins; bin_idx++)
		mh->max_values[bin_idx] = values[bin_idx] / max_value;

	for(int task_idx = 0; task_idx < ms->num_tasks_stats; task_idx++) {
		mh->histograms[task_idx]->left = 0;
		mh->histograms[task_idx]->right = ms->stats[task_idx]->max_cycles;
		mh->histograms[task_idx]->max_hist = ms->stats[task_idx]->max_hist;
		mh->histograms[task_idx]->num_hist = ms->stats[task_idx]->num_tasks;

		mh->task_ids[task_idx] = ms->task_ids[task_idx];

		for(int i = 0; i < mh->histograms[task_idx]->num_bins; i++) {
			if(ms->stats[task_idx]->max_hist > 0) {
				mh->histograms[task_idx]->values[i] =
					((long double)ms->stats[task_idx]->cycles_hist[i] /
					 (long double)ms->stats[task_idx]->max_hist) *
					((long double) ms->stats[task_idx]->max_hist /
					 max_value);
			} else {
				mh->histograms[task_idx]->values[i] = 0;
			}
		}
	}

	return 0;
}

void counter_statistics_gather(struct multi_event_set* mes, struct filter* f, struct task_statistics* cs, struct counter_description* cd, int64_t start, int64_t end)
{
	struct counter_event_set* ces;
	struct event_set* es;
	char buffer[128];
	int64_t value_start, value_end;
	int start_idx;
	uint64_t value;
	uint64_t curr_length;
	int hist_bin;

	for(int histogram = 0; histogram < 2; histogram++) {
		for_each_event_set(mes, es) {
			ces = event_set_find_counter_event_set(es, cd);

			if(!ces)
				continue;

			if((start_idx = event_set_get_first_single_event_in_interval_type(es, start, end, SINGLE_TYPE_TEXEC_START)) == -1)
				continue;

			if(!counter_event_set_is_monotonously_increasing(ces)) {
				snprintf(buffer, sizeof(buffer), "Counter \"%s\" is not monotonously increasing on CPU %d\n", ces->desc->name, es->cpu);
				gtk_label_set_text(GTK_LABEL(g_label_info_counter), buffer);
			} else
				gtk_label_set_text(GTK_LABEL(g_label_info_counter), "");

			if(!filter_has_cpu(f, es->cpu))
				continue;

			struct single_event* se = &es->single_events[start_idx];

			while(se && se->next_texec_end && se->next_texec_end->time <= end) {

				if(!filter_has_frame(f, se->active_frame))
					goto next;

				curr_length = se->next_texec_end->time - se->time;

				if(!filter_has_task_duration(f, curr_length))
					goto next;

				if(!filter_has_task(f, se->active_task))
					goto next;

				if(f->filter_writes_to_numa_nodes &&
				   !event_set_has_write_to_numa_nodes_in_interval(es, &f->writes_to_numa_nodes, se->time, se->next_texec_end->time, f->writes_to_numa_nodes_minsize))
					goto next;

				if(counter_event_set_interpolate_value(ces, se->time, &value_start))
					goto next;

				if(counter_event_set_interpolate_value(ces, se->next_texec_end->time, &value_end))
					goto next;

				value = value_end - value_start;

				if(histogram) {
					if(cs->max_cycles - cs->min_cycles > 0)
						hist_bin = ((uint64_t)(cs->num_hist_bins-1) * (value - cs->min_cycles))
							/ (cs->max_cycles - cs->min_cycles);
					else
						hist_bin = 0;

					cs->cycles_hist[hist_bin]++;
				} else {
					cs->cycles += value;
					cs->num_tasks++;

					if(value > cs->max_cycles)
						cs->max_cycles = value;
					if(value < cs->min_cycles)
						cs->min_cycles = value;
				}

			next:
				se = se->next_texec_start;
			}
		}
	}

	for(int i = 0; i < cs->num_hist_bins; i++) {
		if(cs->cycles_hist[i] > cs->max_hist)
			cs->max_hist = cs->cycles_hist[i];
	}
}

int counter_statistics_to_counter_histogram(struct task_statistics* cs, struct histogram* h)
{
	h->left = 0;
	h->right = cs->max_cycles;
	h->max_hist = cs->max_hist;
	h->num_hist = cs->num_tasks;

	for(int i = 0; i < h->num_bins; i++) {
		if(cs->max_hist > 0)
			h->values[i] = ((long double)cs->cycles_hist[i] / (long double)cs->max_hist);
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

/* Initializes a multi_histogram structure with num_hists histograms
 * each having num_bins bins. Left and right are the values associated
 * to the first and last bin, respectively. */
int multi_histogram_init(struct multi_histogram* mh, int num_hists, unsigned int num_bins, long double left, long double right)
{
	int idx;

	mh->num_hists = num_hists;
	mh->num_hist_bins = num_bins;

	if(!(mh->histograms = calloc(num_hists, sizeof(struct histogram *))))
		goto out_err;

	for(idx = 0; idx < num_hists; idx++) {
		if(!(mh->histograms[idx] = malloc(sizeof(struct histogram)))) {
			show_error_message("Cannot allocate histogram structure in multi histogram");
			goto out_err_destroy;
		}

		if(histogram_init(mh->histograms[idx], num_bins, left, right))
			goto out_err_free;
	}

	if(!(mh->task_ids = calloc(num_hists, sizeof(int)))) {
		show_error_message("Cannot allocate task identifiers array in multi histogram");
		goto out_err_destroy;
	}

	if(!(mh->max_values = calloc(num_bins, sizeof(long double)))) {
		show_error_message("Cannot allocate maximum values array in multi histogram");
		free(mh->task_ids);
		goto out_err_destroy;
	}

	return 0;

out_err_free:
	free(mh->histograms[idx]);
out_err_destroy:
	for(int i = 0; i < idx; i++) {
		histogram_destroy(mh->histograms[i]);
		free(mh->histograms[i]);
	}
out_err:
	return 1;
}

/* Recursively destroys a multi_histogram structure */
void multi_histogram_destroy(struct multi_histogram* mh)
{
	for(int idx = 0; idx < mh->num_hists; idx++)
		histogram_destroy(mh->histograms[idx]);

	free(mh->task_ids);
	free(mh->histograms);
}

int numa_node_exchange_matrix_gather(struct multi_event_set* mes, struct filter* f, struct intensity_matrix* m, int64_t start, int64_t end, int comm_type_mask, int exclude_reflexive, int ignore_direction, int num_only)
{
	struct event_set* es;

	if(intensity_matrix_init(m, mes->max_numa_node_id+1, mes->max_numa_node_id+1))
		return 1;

	for_each_event_set(mes, es) {
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

int cpu_exchange_matrix_gather(struct multi_event_set* mes, struct filter* f, struct intensity_matrix* m, int64_t start, int64_t end, int comm_type_mask, int exclude_reflexive, int ignore_direction, int num_only)
{
	struct event_set* es;
	int src, dst;

	if(intensity_matrix_init(m, mes->max_cpu+1, mes->max_cpu+1))
		return 1;

	for_each_event_set(mes, es) {
		int first_comm_index = event_set_get_first_comm_in_interval(es, start, end);

		if(first_comm_index == -1)
			continue;

		for(struct comm_event* ce = &es->comm_events[first_comm_index];
		    ce < &es->comm_events[es->num_comm_events] && ce->time <= end;
		    ce++)
		{
			int in_mask = (1 << ce->type) & comm_type_mask;

			if(!in_mask || !filter_has_comm_event(f, mes, ce) ||
			   (exclude_reflexive &&
			    ((ce->type == COMM_TYPE_DATA_READ && es->cpu == ce->src_cpu) ||
			     (ce->type == COMM_TYPE_DATA_WRITE && es->cpu == ce->dst_cpu) ||
			     (ce->type == COMM_TYPE_PUSH && es->cpu == ce->dst_cpu) ||
			     (ce->type == COMM_TYPE_STEAL && es->cpu == ce->src_cpu))))
				continue;

			src = ce->src_cpu;
			dst = ce->dst_cpu;

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
