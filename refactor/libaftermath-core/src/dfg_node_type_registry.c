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
	reg->instantiate_callback.fun = NULL;
	reg->instantiate_callback.data = NULL;
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
	struct am_object_notation_node* n)
{
	struct am_object_notation_node_group* g;
	struct am_dfg_node* ret = NULL;
	struct am_dfg_node_type* nt;
	const char* type;

	void* callback_data = NULL;

	if(n->type != AM_OBJECT_NOTATION_NODE_TYPE_GROUP)
		goto out_err;

	g = (struct am_object_notation_node_group*)n;

	if(strcmp(g->name, "am_dfg_node") != 0)
		goto out_err;

	if(am_object_notation_eval_retrieve_string(n, "type", &type))
		goto out_err;

	if(!(nt = am_dfg_node_type_registry_lookup(reg, type)))
		goto out_err;

	if(!(ret = am_dfg_node_from_object_notation(nt, g)))
		goto out_err;

	if(reg->instantiate_callback.fun) {
		callback_data = reg->instantiate_callback.data;

		if(reg->instantiate_callback.fun(reg, ret, callback_data))
			goto out_err_dest;
	}

	return ret;

out_err_dest:
	am_dfg_node_destroy(ret);
	free(ret);
out_err:
	return NULL;
}

/* Destroys a list of node types */
static void am_dfg_node_type_list_destroy(struct list_head* list)
{
	struct am_dfg_node_type* nt;
	struct am_dfg_node_type* next;

	am_typed_list_for_each_safe_genentry(list, nt, next, list) {
		am_dfg_node_type_destroy(nt);
		free(nt);
	}
}

/* Registers static DFG node type definitions atomically at a node type registry
 * nt using the type registry tr. The definitions are given as a set of pointers
 * to sets of static node type definitions, terminated by a NULL entry. This
 * allows for grouping as in the example below, where ALLdefs contains the
 * definitions of two groups of ndoe types:
 *
 *   struct am_dfg_static_node_type_def tA1 = { ... };
 *   struct am_dfg_static_node_type_def tA2 = { ... };
 *   struct am_dfg_static_node_type_def* Adefs[] = { &tA1, &tA2, NULL };
 *
 *   struct am_dfg_static_node_type_def tB1 = { ... };
 *   struct am_dfg_static_node_type_def tB2 = { ... };
 *   struct am_dfg_static_node_type_def* Bdefs[] = { &tB1, &tB2, NULL };
 *
 *   struct am_dfg_static_node_type_def** ALLdefs[] = { Adefs, Bdefs, NULL };
 *
 * The function returns 0 on success, otherwise 1.
 */
int am_dfg_node_type_registry_add_static(
	struct am_dfg_node_type_registry* ntr,
	struct am_dfg_type_registry* tr,
	struct am_dfg_static_node_type_def*** defsets)
{
	struct am_dfg_static_node_type_def*** pcurr_defset;
	struct am_dfg_static_node_type_def** curr_defset;
	struct am_dfg_static_node_type_def** pcurr_def;
	struct am_dfg_static_node_type_def* curr_def;
	struct am_dfg_node_type* nt;
	struct am_dfg_node_type* next;
	struct list_head types;

	INIT_LIST_HEAD(&types);

	/* First reserve memory for each node type and initialize */
	for(pcurr_defset = defsets; *pcurr_defset; pcurr_defset++) {
		curr_defset = *pcurr_defset;

		for(pcurr_def = curr_defset; *pcurr_def; pcurr_def++) {
			curr_def = *pcurr_def;

			if(!(nt = malloc(sizeof(*nt))))
				goto out_err;

			if(am_dfg_node_type_builds(nt, tr, curr_def)) {
				free(nt);
				goto out_err;
			}

			list_add(&nt->list, &types);
		}
	}

	/* Register entire list of initialized node types. Use safe version for
	 * iteration, since the embedded list of the node type will be used to
	 * enqueue the type at the node type registry. */
	am_typed_list_for_each_safe_genentry(&types, nt, next, list)
		am_dfg_node_type_registry_add(ntr, nt);

	return 0;

out_err:
	am_dfg_node_type_list_destroy(&types);
	return 1;
}

/* Sets a function that is called back every time a node type is instantiated
 * from the registry reg. The pointer data is passed verbatim to the callback
 * function. If the callback function's return value is different from zero,
 * node instantiation fails.
 */
void am_dfg_node_type_registry_set_instantiate_callback_fun(
	struct am_dfg_node_type_registry* reg,
	am_dfg_node_type_instantiate_callback_fun_t f,
	void* data)
{
	reg->instantiate_callback.fun = f;
	reg->instantiate_callback.data = data;
}
