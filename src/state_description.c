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

#include "state_description.h"
#include "multi_event_set.h"

static const char* openstream_state_names[] = {
	"seeking",
	"taskexec",
	"tcreate",
	"resdep",
	"tdec",
	"bcast",
	"init",
	"estimate_costs",
	"reorder"
};

static double openstream_state_colors[][3] = {{COL_NORM(117.0), COL_NORM(195.0), COL_NORM(255.0)},
					      {COL_NORM(  0.0), COL_NORM(  0.0), COL_NORM(255.0)},
					      {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(255.0)},
					      {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(  0.0)},
					      {COL_NORM(255.0), COL_NORM(  0.0), COL_NORM(174.0)},
					      {COL_NORM(179.0), COL_NORM(  0.0), COL_NORM(  0.0)},
					      {COL_NORM(  0.0), COL_NORM(255.0), COL_NORM(  0.0)},
					      {COL_NORM(255.0), COL_NORM(255.0), COL_NORM(  0.0)},
					      {COL_NORM(235.0), COL_NORM(  0.0), COL_NORM(  0.0)}};

int add_openstream_state_description(struct multi_event_set* mes, int state_id)
{
	struct state_description* sd;

	if(!(sd = multi_event_set_state_description_alloc_ptr(mes, state_id, strlen(openstream_state_names[state_id]))))
		return 1;

	sd->artificial = 1;
	strcpy(sd->name, openstream_state_names[state_id]);

	sd->color_r = openstream_state_colors[sd->state_id][0];
	sd->color_g = openstream_state_colors[sd->state_id][1];
	sd->color_b = openstream_state_colors[sd->state_id][2];

	return 0;
}

/* Set default openstream worker's states for retro compatibility with
 * trace version [13, 14, 15] */
int set_openstream_state_descriptions(struct multi_event_set* mes)
{
	struct state_description* sd;
	const char* name;
	int states_found = 0;

	for(int i = 0; i < mes->num_states; i++) {
		sd = &mes->states[i];

		if(sd->artificial && sd->state_id < OPENSTREAM_WORKER_STATE_MAX) {
			name = openstream_state_names[sd->state_id];

			if(state_description_set_name(sd, strlen(name), name))
				return 1;

			sd->color_r = openstream_state_colors[sd->state_id][0];
			sd->color_g = openstream_state_colors[sd->state_id][1];
			sd->color_b = openstream_state_colors[sd->state_id][2];

			states_found |= (1 << sd->state_id);
		}
	}

	if(states_found != (1 << OPENSTREAM_WORKER_STATE_MAX)-1) {
		for(int i = 0; i < OPENSTREAM_WORKER_STATE_MAX; i++)
			if(!(states_found & (1 << i)))
				if(add_openstream_state_description(mes, i))
					return 1;
	}

	return 0;
}
