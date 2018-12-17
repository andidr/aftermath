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

#include "count.h"
#include <aftermath/core/ansi_extras.h>

int am_dfg_count_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	uint64_t num_samples;

	if(!am_dfg_port_activated(pin) || !am_dfg_port_activated(pout))
		return 0;

	if(am_safe_u64_from_size(&num_samples, pin->buffer->num_samples))
		return 1;

	if(am_dfg_buffer_write(pout->buffer, 1, &num_samples))
		return 1;

	return 0;
}
