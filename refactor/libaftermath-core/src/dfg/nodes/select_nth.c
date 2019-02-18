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

#include <aftermath/core/dfg/nodes/select_nth.h>

int am_dfg_select_nth_node_init(struct am_dfg_node* n)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;

	snn->num_connections = 0;

	/* Copy of pointer to special type "am::core::any" */
	snn->any_type = n->ports[0].type->type;
	snn->current_type = snn->any_type;

	snn->N = 0;
	snn->fail_if_no_input = 1;

	return 0;
}

int am_dfg_select_nth_node_process(struct am_dfg_node* n)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;
	struct am_dfg_port* pin = &n->ports[0];
	struct am_dfg_port* pout = &n->ports[1];
	const struct am_dfg_type* type;
	size_t nold_out;
	size_t nin;
	void* dst;
	void* src;
	int64_t inin;
	size_t copyidx;

	if(!am_dfg_port_activated(pout))
		return 0;

	nold_out = pout->buffer->num_samples;

	if(am_dfg_port_activated(pin)) {
		if(!am_dfg_port_has_data(pin)) {
			if(snn->fail_if_no_input)
				return 1;
			else
				return 0;
		}

		type = pin->buffer->sample_type;
		nin = pin->buffer->num_samples;

		if(am_safe_i64_from_size(&inin, nin))
			return 1;

		if(snn->N >= 0) {
			/* Positive value: take index as is */
			if(snn->N >= inin)
				return 1;

			copyidx = inin;
		} else {
			/* Negative value: index in reverse order */
			if(-inin > snn->N)
				return 1;

			copyidx = nin - (size_t)(-snn->N);
		}

		if(am_dfg_buffer_ptr(pin->buffer, copyidx, 1, &src))
			return 1;

		if(!(dst = am_dfg_buffer_reserve(pout->buffer, 1)))
			goto out_err;

		if(type->copy_samples(type, 1, src, dst)) {
			pout->buffer->num_samples--;
			goto out_err;
		}
	}

	return 0;

out_err:
	am_dfg_buffer_resize(pout->buffer, nold_out);
	return 1;
}

int am_dfg_select_nth_node_pre_connect(
	const struct am_dfg_node* n,
	const struct am_dfg_port* pi,
	const struct am_dfg_port* pother,
	size_t max_error_msg,
	char* error_msg)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;

	if(snn->current_type == snn->any_type ||
	   snn->current_type == pother->type->type)
	{
		return 0;
	} else {
		snprintf(error_msg, max_error_msg,
			 "Current type of this node is '%s', "
			 "cannot connect to port with type '%s'.",
			 snn->current_type->name,
			 pother->type->type->name);
		return 1;
	}

	return 0;
}

void am_dfg_select_nth_node_connect(struct am_dfg_node* n, struct am_dfg_port* pi)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;

	snn->num_connections++;

	/* First connection sets the type */
	if(snn->num_connections == 1)
		snn->current_type = pi->connections[0]->type->type;

	/* If this is an input port and this is the first connection, we need to
	 * check if its buffer has still the type "am::core::any" assigned upon
	 * creation.
	 */
	if(am_dfg_port_is_output_port(pi) && pi->num_connections == 1)
		if(pi->buffer->sample_type != snn->current_type)
			am_dfg_buffer_change_type(pi->buffer, snn->current_type);
}

void am_dfg_select_nth_node_disconnect(struct am_dfg_node* n, struct am_dfg_port* pi)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;

	snn->num_connections--;

	/* Last connection resets the type */
	if(snn->num_connections == 0)
		snn->current_type = snn->any_type;
}

int am_dfg_select_nth_node_from_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;
	uint64_t uval;

	am_object_notation_eval_retrieve_int64(&g->node, "N", &snn->N);

	if(am_object_notation_eval_retrieve_uint64(
		   &g->node, "fail_if_no_input", &uval) == 0)
	{
		snn->fail_if_no_input = (uval) ? 1 : 0;
	}

	return 0;
}

int am_dfg_select_nth_node_to_object_notation(
	struct am_dfg_node* n,
	struct am_object_notation_node_group* g)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;

	return am_object_notation_node_group_build_add_members(
		g,
		AM_OBJECT_NOTATION_BUILD_MEMBER, "N",
		AM_OBJECT_NOTATION_BUILD_INT64, snn->N,
		AM_OBJECT_NOTATION_BUILD_MEMBER, "fail_if_no_input",
		AM_OBJECT_NOTATION_BUILD_UINT64,
		(uint64_t)snn->fail_if_no_input);
}

int am_dfg_select_nth_node_set_property(
	struct am_dfg_node* n,
	const struct am_dfg_property* property,
	const void* value)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;

	if(strcmp(property->name, "N") == 0) {
		snn->N = *((int64_t*)value);
		return 0;
	} else if(strcmp(property->name, "fail_if_no_input") == 0) {
		snn->fail_if_no_input = *((int*)value);
		return 0;
	}

	return 1;
}

int am_dfg_select_nth_node_get_property(
	const struct am_dfg_node* n,
	const struct am_dfg_property* property,
	void** value)
{
	struct am_dfg_select_nth_node* snn = (struct am_dfg_select_nth_node*)n;

	if(strcmp(property->name, "N") == 0) {
		*value = &snn->N;
		return 0;
	} else if(strcmp(property->name, "fail_if_no_input") == 0) {
		*value = &snn->fail_if_no_input;
		return 0;
	}

	return 1;
}
