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

#include "trace.h"

int am_dfg_trace_node_init(struct am_dfg_node* n)
{
	struct am_dfg_node_trace* t = (typeof(t))n;

	t->trace = NULL;

	return 0;
}

int am_dfg_trace_node_process(struct am_dfg_node* n)
{
	struct am_dfg_node_trace* t = (typeof(t))n;
	struct am_dfg_port* ptrace = &n->ports[0];

	return am_dfg_buffer_write(ptrace->buffer, 1, &t->trace);
}
