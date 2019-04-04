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

#include "hierarchy_node_attributes.h"
#include <aftermath/core/hierarchy.h>

int am_dfg_hierarchy_node_attributes_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pmapping = &n->ports[1];
	struct am_dfg_port* pname = &n->ports[2];
	size_t nin;
	struct am_event_mapping** mapping;
	struct am_hierarchy_node** nodes;
	char* name;

	if(!am_dfg_port_activated(pin) ||
	   (!am_dfg_port_activated(pmapping) &&
	    !am_dfg_port_activated(pname)))
	{
		return 0;
	}

	if((nin = pin->buffer->num_samples) == 0)
		return 0;

	nodes = pin->buffer->data;

	if(am_dfg_port_activated(pname)) {
		for(size_t i = 0; i < nin; i++) {
			if(!(name = strdup(nodes[i]->name)))
				return 1;

			if(am_dfg_buffer_write(pname->buffer, 1, &name)) {
				free(name);
				return 1;
			}
		}
	}

	if(am_dfg_port_activated(pmapping)) {
		if(!(mapping = am_dfg_buffer_reserve(pmapping->buffer, nin)))
			return 1;

		for(size_t i = 0; i < nin; i++)
			mapping[i] = &nodes[i]->event_mapping;
	}

	return 0;
}
