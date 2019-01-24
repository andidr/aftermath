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

#include "telamon_candidate_tree_roots.h"
#include <aftermath/core/trace.h>
#include <aftermath/core/telamon_candidate_array.h>

int am_dfg_telamon_candidate_tree_roots_node_process(struct am_dfg_node* n)
{
	struct am_dfg_port* ptrace_in = &n->ports[0];
	struct am_dfg_port* proots_out = &n->ports[1];
	struct am_trace** trace_in;
	struct am_trace* trace;
	struct am_telamon_candidatep_array* arr;

	if(am_dfg_port_activated_and_has_data(ptrace_in) &&
	   am_dfg_port_activated(proots_out))
	{
		trace_in = ptrace_in->buffer->data;

		for(size_t i = 0; i < ptrace_in->buffer->num_samples; i++) {
			trace = trace_in[i];

			arr = (struct am_telamon_candidatep_array*)
				am_trace_find_trace_array(
					trace,
					"am::telamon::candidate_root");

			if(arr) {
				if(am_dfg_buffer_write(proots_out->buffer,
						       arr->num_elements,
						       arr->elements))
				{
					return 1;
				}
			}
		}
	}

	return 0;
}
