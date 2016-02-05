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

#ifndef MONOTONIC_INDEX_H
#define MONOTONIC_INDEX_H

#include <stdint.h>
#include <stdlib.h>

struct monotonic_index {
	size_t num_samples;
	size_t num_dimensions;
	uint64_t* samples;
	uint64_t* timestamps;
	size_t* valid;

	uint64_t min_timestamp;
	uint64_t max_timestamp;
};

int monotonic_index_init(struct monotonic_index* idx,
			 size_t num_dimensions,
			 size_t num_samples);

void monotonic_index_set(struct monotonic_index* idx,
			 size_t i,
			 uint64_t time,
			 uint64_t* sample,
			 size_t valid);

void monotonic_index_get(struct monotonic_index* idx,
			 size_t i,
			 uint64_t* time,
			 uint64_t* sample,
			 size_t* valid);

void monotonic_index_destroy(struct monotonic_index* idx);

int monotonic_index_get_lindex(struct monotonic_index* idx,
			       uint64_t time,
			       size_t* i);

#endif
