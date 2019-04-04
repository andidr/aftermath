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

#include <aftermath/core/dfg/nodes/double_constant.h>

int am_dfg_double_constant_node_init(struct am_dfg_node* n)
{
	struct am_dfg_double_constant_node* in =
		(struct am_dfg_double_constant_node*)n;

	in->value = 0;
	in->num_samples = 1;
	in->num_samples64 = 1;

	return 0;
}

int am_dfg_double_constant_node_process(struct am_dfg_node* n)
{
	struct am_dfg_double_constant_node* in =
		(struct am_dfg_double_constant_node*)n;
	double* out;

	if(am_dfg_port_activated(&n->ports[0])) {
		if(!(out = am_dfg_buffer_reserve(
			     n->ports[0].buffer, in->num_samples)))
		{
			return 1;
		}

		for(size_t i = 0; i < in->num_samples; i++)
			out[i] = in->value;
	}

	return 0;
}

int am_dfg_double_constant_node_set_property(
	struct am_dfg_node* n,
	const struct am_dfg_property* property,
	const void* value)
{
	struct am_dfg_double_constant_node* in =
		(struct am_dfg_double_constant_node*)n;

	if(strcmp(property->name, "value") == 0) {
		in->value = *((double*)value);
		return 0;
	} else if(strcmp(property->name, "num_samples") == 0) {
		if(am_safe_size_from_u64(&in->num_samples, *((uint64_t*)value)))
			return 1;

		in->num_samples64 = *((uint64_t*)value);

		return 0;
	}

	return 1;
}

int am_dfg_double_constant_node_get_property(
	const struct am_dfg_node* n,
	const struct am_dfg_property* property,
	void** value)
{
	struct am_dfg_double_constant_node* in =
		(struct am_dfg_double_constant_node*)n;

	if(strcmp(property->name, "value") == 0) {
		*value = &in->value;
		return 0;
	} else if(strcmp(property->name, "num_samples") == 0) {
		*value = &in->num_samples64;
		return 0;
	}

	return 1;
}

int am_dfg_double_constant_node_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_double_constant_node* in =
		(struct am_dfg_double_constant_node*)n;
	uint64_t num_samples64;

	if(am_object_notation_eval_retrieve_double(
		   &g->node, "value", &in->value))
	{
			return 1;
	}

	if(am_object_notation_eval_retrieve_uint64(
		   &g->node, "num_samples", &num_samples64) == 0)
	{
		if(am_safe_size_from_u64(&in->num_samples, num_samples64))
			return 1;

		in->num_samples64 = num_samples64;
	}

	return 0;
}

int am_dfg_double_constant_node_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_double_constant_node* in =
		(struct am_dfg_double_constant_node*)n;

	return am_object_notation_node_group_build_add_members(
		g,
		AM_OBJECT_NOTATION_BUILD_MEMBER, "value",
		AM_OBJECT_NOTATION_BUILD_DOUBLE, in->value,
		AM_OBJECT_NOTATION_BUILD_MEMBER, "num_samples",
		AM_OBJECT_NOTATION_BUILD_UINT64, in->num_samples64);
}
