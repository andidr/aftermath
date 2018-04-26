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

#include "duration_to_uint64.h"
#include <aftermath/core/interval.h>

int am_dfg_duration_to_uint64_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	struct am_time_offset* durations;
	uint64_t* us;

	if(!am_dfg_port_is_connected(pin) || !am_dfg_port_is_connected(pout))
		return 0;

	if(am_dfg_buffer_resize(pout->buffer, pin->buffer->num_samples))
		return 1;

	durations = pin->buffer->data;
	us = pout->buffer->data;

	for(size_t i = 0; i < pin->buffer->num_samples; i++) {
		/* We're converting to *unsigned* int, return if input is
		 * negative */
		if(durations[i].sign)
			return 1;

		us[i] = durations[i].abs;
	}

	pout->buffer->num_samples = pin->buffer->num_samples;

	return 0;
}
