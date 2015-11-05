/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
 * Copyright (C) 2015 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
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

#ifndef STATE_DESCRIPTION_H
#define STATE_DESCRIPTION_H

#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define STATE_PREALLOC 16

#define STATE_UNKNOWN_NAME "(no description found)"

struct state_description {
	uint32_t state_id;
	double per;
	double par;

	double color_r;
	double color_g;
	double color_b;

	char* name;
	int artificial;
};

enum openstream_worker_state {
	OPENSTREAM_WORKER_STATE_SEEKING = 0,
	OPENSTREAM_WORKER_STATE_TASKEXEC = 1,
	OPENSTREAM_WORKER_STATE_RT_TCREATE = 2,
	OPENSTREAM_WORKER_STATE_RT_RESDEP = 3,
	OPENSTREAM_WORKER_STATE_RT_TDEC = 4,
	OPENSTREAM_WORKER_STATE_RT_BCAST = 5,
	OPENSTREAM_WORKER_STATE_RT_INIT = 6,
	OPENSTREAM_WORKER_STATE_RT_ESTIMATE_COSTS = 7,
	OPENSTREAM_WORKER_STATE_RT_REORDER = 8,
	OPENSTREAM_WORKER_STATE_MAX = 9
};

struct multi_event_set;
int set_openstream_state_descriptions(struct multi_event_set* mes);

static inline int state_description_init(struct state_description* sd, uint32_t state_id, size_t name_len)
{
	sd->state_id = state_id;
	sd->name = malloc(name_len+1);

	if(sd->name == NULL)
		return 1;

	return 0;
}

static inline int state_description_realloc_name(struct state_description* sd, size_t name_len)
{
	char* newname = realloc(sd->name, name_len+1);

	if(!newname)
		return 1;

	sd->name = newname;

	return 0;
}

static inline int state_description_set_name(struct state_description* sd, size_t name_len, const char* name)
{
	if(state_description_realloc_name(sd, name_len))
		return 1;

	strncpy(sd->name, name, name_len);
	sd->name[name_len] = '\0';

	return 0;
}

static inline void state_description_destroy(struct state_description* sd)
{
	free(sd->name);
}

#endif
