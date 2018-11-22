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

#include "hierarchy_attributes.h"
#include <aftermath/core/hierarchy.h>

struct collect_nodes_cb_data {
	size_t idx;
	struct am_hierarchy_node** base;
};

/* Callback function for collecting all nodes from all hierarchies */
static enum am_hierarchy_node_callback_status
collect_nodes_cb(const struct am_hierarchy* h,
		 struct am_hierarchy_node* n,
		 void* data)
{
	struct collect_nodes_cb_data* d = data;

	d->base[d->idx] = n;
	d->idx++;

	return AM_HIERARCHY_NODE_CALLBACK_STATUS_CONTINUE;
}

int am_dfg_hierarchy_attributes_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pnodes = &n->ports[1];
	struct am_dfg_port* pname = &n->ports[2];
	size_t nin;
	struct am_hierarchy** hierarchies;
	struct am_hierarchy* h;
	char* name;
	struct collect_nodes_cb_data cb_data;
	size_t total_nodes = 0;

	if(!am_dfg_port_activated(pin) ||
	   (!am_dfg_port_activated(pnodes) &&
	    !am_dfg_port_activated(pname)))
	{
		return 0;
	}

	if((nin = pin->buffer->num_samples) == 0)
		return 0;

	hierarchies = pin->buffer->data;

	/* Count total nodes from all hierarchies */
	for(size_t i = 0; i < pin->buffer->num_samples; i++) {
		h = hierarchies[i];

		if(h->root) {
			if(am_size_inc_safe(&total_nodes,
					    h->root->num_descendants) ||
			   am_size_inc_safe(&total_nodes, 1))
			{
				return 1;
			}
		}
	}

	if(am_dfg_port_activated(pname)) {
		for(size_t i = 0; i < pin->buffer->num_samples; i++) {
			if(!(name = strdup(hierarchies[i]->name)))
				return 1;

			if(am_dfg_buffer_write(pname->buffer, 1, &name)) {
				free(name);
				return 1;
			}
		}
	}

	if(am_dfg_port_activated(pnodes)) {
		if(am_dfg_buffer_resize(pnodes->buffer, total_nodes))
			return 1;

		cb_data.base = pnodes->buffer->data;
		cb_data.idx = 0;

		for(size_t i = 0; i < nin; i++) {
			h = hierarchies[i];
			am_hierarchy_for_each_node(h, collect_nodes_cb, &cb_data);
		}

		pnodes->buffer->num_samples = total_nodes;
	}

	return 0;
}
