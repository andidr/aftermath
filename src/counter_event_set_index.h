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

#ifndef COUNTER_EVENT_SET_INDEX_H
#define COUNTER_EVENT_SET_INDEX_H

#include <stdint.h>

#define COUNTER_EVENT_SET_INDEX_DEFAULT_FAN_OUT 10

enum counter_event_set_index_node_type {
	COUNTER_EVENT_SET_INDEX_NODE_TYPE_INTERNAL = 0,
	COUNTER_EVENT_SET_INDEX_NODE_TYPE_LEAF
};

struct counter_event_set_index_node {
	enum counter_event_set_index_node_type type;

	/* Minimum slope of all leaves of this sub-tree */
	long double slope_min;

	/* Maximum slope of all leaves of this sub-tree */
	long double slope_max;

	/* Minimum value of all leaves of this sub-tree */
	int64_t value_min;

	/* Maximum value of all leaves of this sub-tree */
	int64_t value_max;

	/* First event of first leaf of this sub-tree */
	struct counter_event* left_event;

	/* Last event of last leaf of this sub-tree*/
	struct counter_event* right_event;

	/* First child */
	struct counter_event_set_index_node* left_child;

	/* Last child; unused in leave nodes */
	struct counter_event_set_index_node* right_child;
};

struct counter_event_set_index {
	/* Root of the index tree */
	struct counter_event_set_index_node* nodes;

	/* Maximum number of children per node */
	int fan_out;
};


struct counter_event_set;
int counter_event_set_index_create(struct counter_event_set_index* idx,
				   struct counter_event_set* ces,
				   int fan_out);

void counter_event_set_index_destroy(struct counter_event_set_index* idx);
int counter_event_set_index_min_max_value(struct counter_event_set_index* idx,
					  int64_t start, int64_t end,
					  int64_t* min, int64_t* max);

int counter_event_set_index_min_max_slope(struct counter_event_set_index* idx,
					  int64_t start, int64_t end,
					  long double* min, long double* max);

#endif
