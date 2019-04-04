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

#include "hierarchy_selector.h"
#include <aftermath/core/hierarchy_array.h>
#include <aftermath/core/trace.h>

static int
am_dfg_hierarchy_selector_set_name(struct am_dfg_hierarchy_selector_node* hs,
				   const char* name)
{
	char* tmp;

	if(!(tmp = strdup(name)))
		return 1;

	free(hs->name);
	hs->name = tmp;

	return 0;
}

int am_dfg_hierarchy_selector_node_init(struct am_dfg_node* n)
{
	struct am_dfg_hierarchy_selector_node* hs = (typeof(hs))n;

	hs->name = NULL;

	return am_dfg_hierarchy_selector_set_name(hs, "");
}

void am_dfg_hierarchy_selector_node_destroy(struct am_dfg_node* n)
{
	struct am_dfg_hierarchy_selector_node* hs = (typeof(hs))n;

	free(hs->name);
}

int am_dfg_hierarchy_selector_node_process(struct am_dfg_node* n)
{
	struct am_dfg_hierarchy_selector_node* hs = (typeof(hs))n;
	struct am_dfg_port* ptrace = &n->ports[0];
	struct am_dfg_port* phierarchies = &n->ports[1];
	struct am_trace** traces;
	struct am_hierarchy* h;
	struct am_dfg_buffer* hbuffer;

	if(!am_dfg_port_is_connected(ptrace) ||
	   !am_dfg_port_is_connected(phierarchies) ||
	   !hs->name)
	{
		return 0;
	}

	traces = ptrace->buffer->data;
	hbuffer = phierarchies->buffer;

	for(size_t i = 0; i < ptrace->buffer->num_samples; i++) {
		for(size_t j = 0; j < traces[i]->hierarchies.num_elements; j++) {
			h = traces[i]->hierarchies.elements[j];

			if(strcmp(h->name, hs->name) == 0) {
				if(am_dfg_buffer_write(hbuffer, 1, &h))
					return 1;
			}
		}
	}

	return 0;
}


int am_dfg_hierarchy_selector_node_set_property(
	struct am_dfg_node* n,
	const struct am_dfg_property* property,
	const void* value)
{
	struct am_dfg_hierarchy_selector_node* hs = (typeof(hs))n;
	char* const* pstr = value;

	if(strcmp(property->name, "name") != 0)
		return 1;

	return am_dfg_hierarchy_selector_set_name(hs, *pstr);
}

int am_dfg_hierarchy_selector_node_get_property(
	const struct am_dfg_node* n,
	const struct am_dfg_property* property,
	void** value)
{
	struct am_dfg_hierarchy_selector_node* hs = (typeof(hs))n;

	*value = hs->name;

	return 0;
}

int am_dfg_hierarchy_selector_node_from_object_notation(
	struct am_dfg_node* n, struct am_object_notation_node_group* g)
{
	struct am_dfg_hierarchy_selector_node* hs = (typeof(hs))n;
	const char* name;

	if(am_object_notation_eval_retrieve_string(
		   &g->node, "name", &name) == 0)
	{
		if(am_dfg_hierarchy_selector_set_name(hs, name))
			return 1;
	}

	return 0;
}

int am_dfg_hierarchy_selector_node_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_hierarchy_selector_node* hs = (typeof(hs))n;

	return am_object_notation_node_group_build_add_members(
			g,
			AM_OBJECT_NOTATION_BUILD_MEMBER, "name",
			AM_OBJECT_NOTATION_BUILD_STRING, hs->name);
}
