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

#include "interval_attributes.h"
#include <aftermath/core/base_types.h>
#include <aftermath/core/interval.h>

int am_dfg_interval_attributes_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pinterval = &n->ports[0];
	struct am_dfg_port* pstart = &n->ports[1];
	struct am_dfg_port* pend = &n->ports[2];

	struct am_interval* interval;
	am_timestamp_t* start;
	am_timestamp_t* end;

	size_t num_samples;

	if(!am_dfg_port_is_connected(pinterval))
		return 0;

	if((num_samples = pinterval->buffer->num_samples) == 0)
		return 0;

	/* Reserve space for all samples in pstart and pend */
	if(am_dfg_port_is_connected(pstart))
		if(am_dfg_buffer_resize(pstart->buffer, num_samples))
			return 1;

	if(am_dfg_port_is_connected(pend))
		if(am_dfg_buffer_resize(pend->buffer, num_samples))
			return 1;

	/* Write start samples */
	if(am_dfg_port_is_connected(pstart)) {
		interval = pinterval->buffer->data;
		start = pstart->buffer->data;

		for(size_t i = 0; i < num_samples; i++)
			start[i] = interval[i].start;

		pstart->buffer->num_samples = num_samples;
	}

	/* Write end samples */
	if(am_dfg_port_is_connected(pend)) {
		interval = pinterval->buffer->data;
		end = pend->buffer->data;

		for(size_t i = 0; i < num_samples; i++)
			end[i] = interval[i].end;

		pend->buffer->num_samples = num_samples;
	}

	return 0;
}
