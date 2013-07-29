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

void state_statistics_init(struct state_statistics* s)
{
	memset(s->state_cycles, 0, sizeof(s->state_cycles));
}

void state_statistics_gather(struct multi_event_set* mes, struct state_statistics* s, int64_t start, int64_t end)
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
			s->state_cycles[se->state] += state_event_length_in_interval(se, start, end);
			state_idx++;
			se++;
		}
	}
}
