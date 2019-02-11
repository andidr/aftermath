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

#include <aftermath/core/dfg/nodes/conditional_forward_pairwise.h>

int am_dfg_conditional_forward_pairwise_node_init(struct am_dfg_node* n)
{
	struct am_dfg_conditional_forward_node* mn =
		(struct am_dfg_conditional_forward_node*)n;

	mn->num_connections = 0;

	/* Copy of pointer to special type "am::core::any" */
	mn->any_type = n->ports[0].type->type;
	mn->current_type = mn->any_type;

	return 0;
}

int am_dfg_conditional_forward_pairwise_node_pre_connect(
	const struct am_dfg_node* n,
	const struct am_dfg_port* pi,
	const struct am_dfg_port* pother,
	size_t max_error_msg,
	char* error_msg)
{
	struct am_dfg_conditional_forward_node* mn =
		(struct am_dfg_conditional_forward_node*)n;

	if(strcmp(pi->type->name, "control") == 0)
		return 0;

	if(mn->current_type == mn->any_type ||
	   mn->current_type == pother->type->type)
	{
		return 0;
	} else {
		snprintf(error_msg, max_error_msg,
			 "Current type of this node is '%s', "
			 "cannot connect to port with type '%s'.",
			 mn->current_type->name,
			 pother->type->type->name);
		return 1;
	}

	return 0;
}

void am_dfg_conditional_forward_pairwise_node_connect(
	struct am_dfg_node* n, struct am_dfg_port* pi)
{
	struct am_dfg_conditional_forward_node* mn =
		(struct am_dfg_conditional_forward_node*)n;

	if(strcmp(pi->type->name, "control") == 0)
		return;

	mn->num_connections++;

	/* First connection sets the type */
	if(mn->num_connections == 1)
		mn->current_type = pi->connections[0]->type->type;

	/* If this is an input port and this is the first connection, we need to
	 * check if its buffer has still the type "am::core::any" assigned upon
	 * creation.
	 */
	if(am_dfg_port_is_output_port(pi) && pi->num_connections == 1)
		if(pi->buffer->sample_type != mn->current_type)
			am_dfg_buffer_change_type(pi->buffer, mn->current_type);
}

void am_dfg_conditional_forward_pairwise_node_disconnect(
	struct am_dfg_node* n, struct am_dfg_port* pi)
{
	struct am_dfg_conditional_forward_node* mn =
		(struct am_dfg_conditional_forward_node*)n;

	mn->num_connections--;

	/* Last connection resets the type */
	if(mn->num_connections == 0)
		mn->current_type = mn->any_type;
}

int am_dfg_conditional_forward_pairwise_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pa = &n->ports[0];
	struct am_dfg_port* pb = &n->ports[1];
	struct am_dfg_port* pctrl = &n->ports[2];
	struct am_dfg_port* pout = &n->ports[3];
	int* ctrl;
	size_t na_idx = 0;
	size_t nb_idx = 0;
	void* dst;
	size_t nold_out;
	const struct am_dfg_type* type;
	void* inptr;

	/* All input ports must be connected  */
	if((!(am_dfg_port_is_connected(pa) &&
	      am_dfg_port_is_connected(pb) &&
	      am_dfg_port_is_connected(pctrl))))
	{
		return 1;
	}

	if(am_dfg_port_activated(pout)) {
		nold_out = pout->buffer->num_samples;
		ctrl = pctrl->buffer->data;
		type = pa->buffer->sample_type;

		for(size_t i = 0; i < pctrl->buffer->num_samples; i++) {
			if(ctrl[i]) {
				if(am_dfg_buffer_ptr(pa->buffer, na_idx, 1, &inptr))
					goto out_err;

				na_idx++;
			} else {
				if(am_dfg_buffer_ptr(pb->buffer, nb_idx, 1, &inptr))
					goto out_err;

				nb_idx++;
			}

			if(!(dst = am_dfg_buffer_reserve(pout->buffer, 1)))
				goto out_err;

			if(type->copy_samples(type, 1, inptr, dst)) {
				pout->buffer->num_samples--;
				goto out_err;
			}
		}
	}

	return 0;

out_err:
	am_dfg_buffer_resize(pout->buffer, nold_out);
	return 1;
}
