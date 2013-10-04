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

int histogram_init(struct histogram* h, unsigned int num_bins, long double left, long double right);
void histogram_destroy(struct histogram* h);

struct state_statistics {
	uint64_t state_cycles[WORKER_STATE_MAX];
};

void state_statistics_init(struct state_statistics* s);
void state_statistics_gather_cycles(struct multi_event_set* mes, struct filter* f, struct state_statistics* s, int64_t start, int64_t end);

struct task_statistics {
	unsigned int num_tasks;
	uint64_t cycles;
	uint64_t max_cycles;
	uint64_t min_cycles;
	unsigned int num_hist_bins;
	uint64_t* cycles_hist;
	unsigned int max_hist;
};

int task_statistics_init(struct task_statistics* s, unsigned int num_hist_bins);
void task_statistics_reset(struct task_statistics* s);
void task_statistics_destroy(struct task_statistics* s);
void task_statistics_gather(struct multi_event_set* mes, struct filter* f, struct task_statistics* s, int64_t start, int64_t end);
int task_statistics_to_task_length_histogram(struct task_statistics* s, struct histogram* h);

int numa_node_exchange_matrix_gather(struct multi_event_set* mes, struct filter* f, struct intensity_matrix* m, int comm_type_mask, int exclude_reflexive, int ignore_direction);

#endif
