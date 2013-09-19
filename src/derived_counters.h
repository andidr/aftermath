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

enum access_type {
	ACCESS_TYPE_READS_ONLY,
	ACCESS_TYPE_WRITES_ONLY,
	ACCESS_TYPE_READS_AND_WRITES
};

int derive_aggregate_counter(struct multi_event_set* mes, struct counter_description** cd_out, const char* counter_name, unsigned int counter_idx, int num_samples, int cpu);
int derive_parallelism_counter(struct multi_event_set* mes, struct counter_description** cd_out, const char* counter_name, enum worker_state state, int num_samples, int cpu);
int derive_numa_contention_counter(struct multi_event_set* mes, struct counter_description** cd_out, const char* counter_name, unsigned int numa_node, enum access_type contention_type, int num_samples, int cpu);

#endif
