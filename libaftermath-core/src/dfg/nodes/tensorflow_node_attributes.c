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

#include "tensorflow_node_attributes.h"
#include <aftermath/core/hierarchy.h>

int am_dfg_tensorflow_node_attributes_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pnames = &n->ports[1];
	struct am_tensorflow_node** nodes;
	size_t old_num_samples;
	char* name_dup;
	size_t nin;

	if(!am_dfg_port_is_connected(pin) || !am_dfg_port_is_connected(pnames))
		return 0;

	if((nin = pin->buffer->num_samples) == 0)
		return 0;

	nodes = pin->buffer->data;

	if(am_dfg_port_activated(pnames)) {
		old_num_samples = pnames->buffer->num_samples;

		for(size_t i = 0; i < nin; i++) {
			if(!(name_dup = strdup(nodes[i]->name)))
				goto out_err;

			if(am_dfg_buffer_write(pnames->buffer, 1, &name_dup))
				goto out_err_free;
		}
	}

	return 0;

out_err_free:
	free(name_dup);
out_err:
	am_dfg_buffer_resize(pnames->buffer, old_num_samples);
	return 1;
}
