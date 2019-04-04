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

#include "interval_duration.h"
#include <aftermath/core/base_types.h>
#include <aftermath/core/interval.h>

int am_dfg_interval_duration_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pinterval = &n->ports[0];
	struct am_dfg_port* pduration = &n->ports[1];

	struct am_interval* interval;
	struct am_time_offset* duration;

	size_t num_samples;

	if(!am_dfg_port_is_connected(pinterval))
		return 0;

	num_samples = pinterval->buffer->num_samples;

	/* Reserve space for all samples in pstart and pend */
	if(!am_dfg_port_is_connected(pduration))
		return 0;

	if(am_dfg_buffer_resize(pduration->buffer, num_samples))
		return 1;

	interval = pinterval->buffer->data;
	duration = pduration->buffer->data;

	for(size_t i = 0; i < num_samples; i++)
		am_interval_duration(&interval[i], &duration[i]);

	pduration->buffer->num_samples = num_samples;

	return 0;
}
