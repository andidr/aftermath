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

#include "merge.h"

int am_dfg_merge_node_init(struct am_dfg_node* n)
{
	struct am_dfg_merge_node* mn = (struct am_dfg_merge_node*)n;

	mn->num_connections = 0;

	/* Copy of pointer to special type "am::core::any" */
	mn->any_type = n->ports[0].type->type;
	mn->current_type = mn->any_type;

	return 0;
}

int am_dfg_merge_node_process(struct am_dfg_node* n)
{
	size_t num_in_ports = n->type->num_ports - 1;
	struct am_dfg_port* in_ports = &n->ports[0];
	struct am_dfg_port* out_port = &n->ports[num_in_ports];
	const struct am_dfg_type* type;
	size_t nold_out;
	size_t nin;
	void* dst;

	if(!am_dfg_port_activated(out_port))
		return 0;

	nold_out = out_port->buffer->num_samples;

	for(size_t i = 0; i < num_in_ports; i++) {
		if(!am_dfg_port_activated(&in_ports[i]))
			continue;

		type = in_ports[i].buffer->sample_type;
		nin = in_ports[i].buffer->num_samples;

		if(nin == 0)
			continue;

		if(!(dst = am_dfg_buffer_reserve(out_port->buffer, nin)))
			goto out_err;

		if(type->copy_samples(type, nin, in_ports[i].buffer->data, dst)) {
			out_port->buffer->num_samples -= nin;
			goto out_err;
		}
	}

	return 0;

out_err:
	am_dfg_buffer_resize(out_port->buffer, nold_out);
	return 1;
}

int am_dfg_merge_node_pre_connect(
	const struct am_dfg_node* n,
	const struct am_dfg_port* pi,
	const struct am_dfg_port* pother,
	size_t max_error_msg,
	char* error_msg)
{
	struct am_dfg_merge_node* mn = (struct am_dfg_merge_node*)n;

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

void am_dfg_merge_node_connect(struct am_dfg_node* n, struct am_dfg_port* pi)
{
	struct am_dfg_merge_node* mn = (struct am_dfg_merge_node*)n;

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

void am_dfg_merge_node_disconnect(struct am_dfg_node* n, struct am_dfg_port* pi)
{
	struct am_dfg_merge_node* mn = (struct am_dfg_merge_node*)n;

	mn->num_connections--;

	/* Last connection resets the type */
	if(mn->num_connections == 0)
		mn->current_type = mn->any_type;
}
