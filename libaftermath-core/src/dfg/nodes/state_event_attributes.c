/**
 * Author: Andi Drebes <andi@drebesium.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include "state_event_attributes.h"
#include <aftermath/core/hierarchy.h>

int am_dfg_state_event_attributes_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pinterval = &n->ports[1];
	struct am_dfg_port* pindex = &n->ports[2];
	struct am_state_event** events;
	struct am_interval* intervals;
	uint64_t* indexes;
	size_t nin;

	if(!am_dfg_port_is_connected(pin))
		return 0;

	if((nin = pin->buffer->num_samples) == 0)
		return 0;

	events = pin->buffer->data;

	if(am_dfg_port_is_connected(pinterval)) {
		if(!(intervals = am_dfg_buffer_reserve(pinterval->buffer, nin)))
			return 1;

		for(size_t i = 0; i < nin; i++)
			intervals[i] = events[i]->interval;
	}

	if(am_dfg_port_is_connected(pindex)) {
		if(!(indexes = am_dfg_buffer_reserve(pindex->buffer, nin)))
			return 1;

		for(size_t i = 0; i < nin; i++)
			indexes[i] = events[i]->state_idx;
	}

	return 0;
}
