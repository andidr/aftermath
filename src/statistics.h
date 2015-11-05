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

#ifndef STATISTICS_H
#define STATISTICS_H

#include "trace_file.h"
#include "multi_event_set.h"
#include "intensity_matrix.h"

#include "filter.h"

#define HISTOGRAM_DEFAULT_NUM_BINS 100

struct histogram {
	unsigned int num_bins;
	long double* values;
	long double left;
	long double right;

	unsigned int max_hist;
	unsigned int num_hist;
};

struct multi_histogram {
	int num_hists;
	int* task_ids;  // array of num_hists int
	struct histogram** histograms;

	unsigned int num_hist_bins;
	long double* max_values;  // array of num_hist_bins long double
};

int histogram_init(struct histogram* h, unsigned int num_bins, long double left, long double right);
void histogram_destroy(struct histogram* h);

int multi_histogram_init(struct multi_histogram* mh, int num_hists, unsigned int num_bins, long double left, long double right);
void multi_histogram_destroy(struct multi_histogram* mh);

struct state_statistics {
	uint64_t* state_cycles;
};

void state_statistics_init(struct state_statistics* s);
void state_statistics_gather_cycles(struct multi_event_set* mes, struct filter* f, struct state_statistics* s, int64_t start, int64_t end);

struct single_event_statistics {
	uint64_t num_tcreate_events;
};

void single_event_statistics_init(struct single_event_statistics* s);
void single_event_statistics_gather(struct multi_event_set* mes, struct filter* f, struct single_event_statistics* s, int64_t start, int64_t end);

struct task_statistics {
	unsigned int num_tasks;
	uint64_t cycles;
	uint64_t max_cycles;
	uint64_t min_cycles;
	unsigned int num_hist_bins;
	uint64_t* cycles_hist;
	unsigned int max_hist;
};

struct multi_task_statistics {
	int num_tasks_stats;
	uint64_t min_all;
	uint64_t max_all;
	int* task_ids;
	struct task_statistics** stats;
};

int task_statistics_init(struct task_statistics* s, unsigned int num_hist_bins);
void task_statistics_reset(struct task_statistics* s);
void task_statistics_destroy(struct task_statistics* s);
void task_statistics_gather(struct multi_event_set* mes, struct filter* f, struct task_statistics* s, int64_t start, int64_t end);
int task_statistics_to_task_length_histogram(struct task_statistics* s, struct histogram* h);

int multi_task_statistics_init(struct multi_task_statistics* ms, int num_tasks_stats, unsigned int num_hist_bins);
void multi_task_statistics_destroy(struct multi_task_statistics* ms);
int multi_task_statistics_shrink(struct multi_task_statistics* ms, int num_tasks_stats);
void task_statistics_gather_with_min_max_cycles(struct multi_event_set* mes, struct filter* f, struct task_statistics* s, int64_t start, int64_t end, uint64_t min_cycles, uint64_t max_cycles);
int multi_task_statistics_gather(struct multi_event_set* mes, struct filter* f, struct multi_task_statistics* ms, int64_t start, int64_t end);
int multi_task_statistics_to_task_length_multi_histogram(struct multi_task_statistics* ms, struct multi_histogram* mh);

void counter_statistics_gather(struct multi_event_set* mes, struct filter* f, struct task_statistics* cs, struct counter_description* cd, int64_t start, int64_t end);
int counter_statistics_to_counter_histogram(struct task_statistics* cs, struct histogram* h);

int numa_node_exchange_matrix_gather(struct multi_event_set* mes, struct filter* f, struct intensity_matrix* m, int64_t start, int64_t end, int comm_type_mask, int exclude_reflexive, int ignore_direction, int num_only);
int cpu_exchange_matrix_gather(struct multi_event_set* mes, struct filter* f, struct intensity_matrix* m, int64_t start, int64_t end, int comm_type_mask, int exclude_reflexive, int ignore_direction, int num_only);

#endif
