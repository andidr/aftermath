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

#include <aftermath/core/dfg/nodes/conditional_forward_all.h>

int am_dfg_conditional_forward_all_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pa = &n->ports[0];
	struct am_dfg_port* pb = &n->ports[1];
	struct am_dfg_port* pctrl = &n->ports[2];
	struct am_dfg_port* pout = &n->ports[3];
	struct am_dfg_port* pfwd;
	int* ctrl;
	void* dst;
	size_t nold_out;
	size_t nin;
	const struct am_dfg_type* type;

	/* All input ports must be connected  */
	if((!(am_dfg_port_is_connected(pa) &&
	      am_dfg_port_is_connected(pb) &&
	      am_dfg_port_is_connected(pctrl))))
	{
		return 1;
	}

	if(am_dfg_port_activated(pout) && am_dfg_port_has_data(pctrl)) {
		if(pctrl->buffer->num_samples != 1)
			return 1;

		nold_out = pout->buffer->num_samples;
		ctrl = pctrl->buffer->data;
		type = pa->buffer->sample_type;

		if(ctrl[0])
			pfwd = pa;
		else
			pfwd = pb;

		nin = pfwd->buffer->num_samples;

		if(!(dst = am_dfg_buffer_reserve(pout->buffer, nin)))
			return 1;

		if(type->copy_samples(type, nin, pfwd->buffer->data, dst)) {
			pout->buffer->num_samples -= nin;
			am_dfg_buffer_resize(pout->buffer, nold_out);
			return 1;
		}
	}

	return 0;
}
