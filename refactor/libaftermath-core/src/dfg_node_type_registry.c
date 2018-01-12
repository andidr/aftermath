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

#include <aftermath/core/dfg_node_type_registry.h>
#include <string.h>

/*
 * Add a node type to the registry
 */
void am_dfg_node_type_registry_add(struct am_dfg_node_type_registry* reg,
				   struct am_dfg_node_type* t)
{
	list_add(&t->list, &reg->types);
}

/*
 * Find a node type by name. Returns NULL if the node type cannot be found.
 */
struct am_dfg_node_type*
am_dfg_node_type_registry_lookup(struct am_dfg_node_type_registry* reg,
				 const char* name)
{
	struct am_dfg_node_type* t;

	am_dfg_node_type_registry_for_each_type(reg, t)
		if(strcmp(t->name, name) == 0)
			return t;

	return NULL;
}

/* Initialize a registry */
void am_dfg_node_type_registry_init(struct am_dfg_node_type_registry* reg,
				    long flags)
{
	INIT_LIST_HEAD(&reg->types);
	reg->flags = flags;
}

/* Destroy a registry */
void am_dfg_node_type_registry_destroy(struct am_dfg_node_type_registry* reg)
{
	struct am_dfg_node_type* t;
	struct am_dfg_node_type* tmp;

	if(reg->flags & AM_DFG_NODE_TYPE_REGISTRY_DESTROY_TYPES) {
		am_dfg_node_type_registry_for_each_type_safe(reg, t, tmp) {
			am_dfg_node_type_destroy(t);
			free(t);
		}
	}
}

/* Allocate and instantiate a new node that corresponds to a description given
 * in object notation. Returns the new node or NULL on failure.
 */
struct am_dfg_node*
am_dfg_node_type_registry_node_from_object_notation(
	struct am_dfg_node_type_registry* reg,
	struct am_object_notation_node* n_node)
{
	struct am_object_notation_node_group* n_gnode;
	struct am_object_notation_node* n_type;
	struct am_object_notation_node_string* n_stype;
	struct am_object_notation_node* n_id;
	struct am_object_notation_node_int* n_iid;

	struct am_dfg_node* ret = NULL;
	struct am_dfg_node_type* nt;

	if(n_node->type !=  AM_OBJECT_NOTATION_NODE_TYPE_GROUP)
		return NULL;

	n_gnode = (struct am_object_notation_node_group*)n_node;

	if(strcmp(n_gnode->name, "am_dfg_node") != 0)
		return NULL;

	if(!am_object_notation_node_group_has_exactly_members(n_gnode,
							      "type", "id",
							      NULL))
	{
		return NULL;
	}

	n_type = am_object_notation_node_group_get_member_def(n_gnode, "type");
	n_id = am_object_notation_node_group_get_member_def(n_gnode, "id");

	if(n_type->type != AM_OBJECT_NOTATION_NODE_TYPE_STRING ||
	   n_id->type != AM_OBJECT_NOTATION_NODE_TYPE_INT)
	{
		return NULL;
	}

	n_stype = (struct am_object_notation_node_string*)n_type;
	n_iid = (struct am_object_notation_node_int*)n_id;

	if(!(nt = am_dfg_node_type_registry_lookup(reg, n_stype->value)))
		return NULL;

	if(!(ret = am_dfg_node_alloc(nt)))
		return NULL;

	if(am_dfg_node_instantiate(ret, nt, n_iid->value)) {
		free(ret);
		return NULL;
	}

	return ret;
}
