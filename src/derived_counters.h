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

#ifndef DERIVED_COUNTERS_H
#define DERIVED_COUNTERS_H

#include "multi_event_set.h"
#include "filter.h"
#include "bitvector.h"

enum access_type {
	ACCESS_TYPE_READS_ONLY,
	ACCESS_TYPE_WRITES_ONLY,
	ACCESS_TYPE_READS_AND_WRITES
};

enum source_type {
	SOURCE_TYPE_LOCAL,
	SOURCE_TYPE_REMOTE,
	SOURCE_TYPE_LOCAL_AND_REMOTE
};

enum access_model {
	ACCESS_MODEL_SPIKES,
	ACCESS_MODEL_LINEAR
};

enum ratio_type {
	RATIO_TYPE_PLAIN_DIV,
	RATIO_TYPE_DIV_SUM
};

int derive_aggregate_counter(struct multi_event_set* mes, struct counter_description** cd_out, const char* counter_name, unsigned int counter_idx, int num_samples, int cpu);
int derive_parallelism_counter(struct multi_event_set* mes, struct counter_description** cd_out, const char* counter_name, enum worker_state state, int num_samples, int cpu);
int derive_numa_contention_counter(struct multi_event_set* mes, struct counter_description** cd_out, const char* counter_name, unsigned int numa_node, enum source_type source, enum access_type contention_type, enum access_model model, int num_samples, int cpu);
int derive_ratio_counter(struct multi_event_set* g_mes, struct counter_description** cd_out, const char* counter_name, enum ratio_type ratio_type, int counter_idx, int divcounter_idx, int num_samples, int cpu);
int derive_task_length_counter(struct multi_event_set* mes, struct counter_description** cd_out, const char* counter_name, struct bitvector* cpus, struct filter* task_filter, int num_samples, int cpu);

#endif
