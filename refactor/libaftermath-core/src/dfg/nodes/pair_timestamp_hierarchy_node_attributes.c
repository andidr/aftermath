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

#include <aftermath/core/dfg/nodes/pair_timestamp_hierarchy_node_attributes.h>
#include <aftermath/core/dfg/types/pair_timestamp_hierarchy_node.h>

int am_dfg_pair_timestamp_hierarchy_attributes_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* ppair = &n->ports[0];
	struct am_dfg_port* ptimestamp = &n->ports[1];
	struct am_dfg_port* phierarchy_node = &n->ports[2];

	struct am_dfg_type_pair_timestamp_const_hierarchy_node* pair;
	am_timestamp_t* timestamp;
	struct am_hierarchy_node** hierarchy_node;

	size_t num_samples;

	if(!am_dfg_port_is_connected(ppair))
		return 0;

	if((num_samples = ppair->buffer->num_samples) == 0)
		return 0;

	/* Reserve space for all samples in ptimestamp and phierarchy_node */
	if(am_dfg_port_is_connected(ptimestamp))
		if(am_dfg_buffer_resize(ptimestamp->buffer, num_samples))
			return 1;

	if(am_dfg_port_is_connected(phierarchy_node))
		if(am_dfg_buffer_resize(phierarchy_node->buffer, num_samples))
			return 1;

	/* Write timestamp samples */
	if(am_dfg_port_is_connected(ptimestamp)) {
		pair = ppair->buffer->data;
		timestamp = ptimestamp->buffer->data;

		for(size_t i = 0; i < num_samples; i++)
			timestamp[i] = pair[i].timestamp;

		ptimestamp->buffer->num_samples = num_samples;
	}

	/* Write hierarchy_node samples */
	if(am_dfg_port_is_connected(phierarchy_node)) {
		pair = ppair->buffer->data;
		hierarchy_node = phierarchy_node->buffer->data;

		for(size_t i = 0; i < num_samples; i++)
			hierarchy_node[i] = pair[i].node;

		phierarchy_node->buffer->num_samples = num_samples;
	}

	return 0;
}
