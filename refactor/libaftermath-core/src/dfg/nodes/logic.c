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

#include <aftermath/core/dfg/nodes/logic.h>

int am_dfg_folding_and_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	int* in;
	int res = 1;

	if(am_dfg_port_activated_and_has_data(pin) &&
	   am_dfg_port_activated(pout))
	{
		in = pin->buffer->data;

		for(size_t i = 0; i < pin->buffer->num_samples; i++) {
			if(!in[i]) {
				res = 0;
				break;
			}
		}

		if(am_dfg_buffer_write(pout->buffer, 1, &res))
			return 1;
	}

	return 0;
}

int am_dfg_folding_or_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	int* in;
	int res = 0;

	if(am_dfg_port_activated_and_has_data(pin) &&
	   am_dfg_port_activated(pout))
	{
		in = pin->buffer->data;

		for(size_t i = 0; i < pin->buffer->num_samples; i++) {
			if(in[i]) {
				res = 1;
				break;
			}
		}

		if(am_dfg_buffer_write(pout->buffer, 1, &res))
			return 1;
	}

	return 0;
}

int am_dfg_pairwise_and_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pa = &n->ports[0];
	struct am_dfg_port* pb = &n->ports[1];
	struct am_dfg_port* pout = &n->ports[2];
	int* a;
	int* b;
	int val;
	size_t nsamples;

	/* Both input ports must be connected and have the same number of
	 * samples */
	if((!(am_dfg_port_is_connected(pa) && am_dfg_port_is_connected(pb))) ||
	   pa->buffer->num_samples != pb->buffer->num_samples)
	{
		return 1;
	}

	if(am_dfg_port_activated(pout)) {
		a = pa->buffer->data;
		b = pb->buffer->data;
		nsamples = pa->buffer->num_samples;

		for(size_t i = 0; i < nsamples; i++) {
			val = a[i] && b[i];

			if(am_dfg_buffer_write(pout->buffer, 1, &val))
				return 1;
		}
	}

	return 0;
}

int am_dfg_pairwise_or_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pa = &n->ports[0];
	struct am_dfg_port* pb = &n->ports[1];
	struct am_dfg_port* pout = &n->ports[2];
	int* a;
	int* b;
	int val;
	size_t nsamples;

	/* Both input ports must be connected and have the same number of
	 * samples */
	if((!(am_dfg_port_is_connected(pa) && am_dfg_port_is_connected(pb))) ||
	   pa->buffer->num_samples != pb->buffer->num_samples)
	{
		return 1;
	}

	if(am_dfg_port_activated(pout)) {
		a = pa->buffer->data;
		b = pb->buffer->data;
		nsamples = pa->buffer->num_samples;

		for(size_t i = 0; i < nsamples; i++) {
			val = a[i] || b[i];

			if(am_dfg_buffer_write(pout->buffer, 1, &val))
				return 1;
		}
	}

	return 0;
}
