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

#include <aftermath/core/dfg_builtin_node_types.h>
#include <aftermath/core/ansi_extras.h>

/* Undef the default no-ops */
#undef AM_DFG_DECL_BUILTIN_NODE_TYPE_SWITCH
#undef AM_DFG_ADD_BUILTIN_NODE_TYPES_SWITCH

/* Generates the static port definitions and the node definitions from an
 * invocation of AM_DFG_DECL_BUILTIN_NODE_TYPE. See
 * AM_DFG_DECL_BUILTIN_NODE_TYPE for documentation. */
#define AM_DFG_DECL_BUILTIN_NODE_TYPE_SWITCH(ID, NODE_TYPE_NAME, INSTANCE_SIZE, \
					     FUNCTIONS, ...)			\
	static struct am_dfg_static_port_type_def ID##_ports[] = {		\
		__VA_ARGS__							\
	};									\
										\
	static struct am_dfg_static_node_type_def ID = {			\
		.name = NODE_TYPE_NAME,					\
		.instance_size = INSTANCE_SIZE,				\
		.functions = FUNCTIONS,					\
		.num_ports = AM_ARRAY_SIZE(ID##_ports),			\
		.ports = ID##_ports						\
	};

/* Generates a NULL-terminated list of static node type definitions for a header
 * file from its invocation of AM_DFG_ADD_BUILTIN_NODE_TYPES. The macro
 * DEFS_NAME() must be defined prior to the invocation, e.g., before inclusion
 * of the header. */
#define AM_DFG_ADD_BUILTIN_NODE_TYPES_SWITCH(...)			\
	static struct am_dfg_static_node_type_def* DEFS_NAME()[] = {	\
		__VA_ARGS__, NULL					\
	};

/* Use the definitions above as replacements for the default no-ops of
 * AM_DFG_DECL_BUILTIN_NODE_TYPE and AM_DFG_ADD_BUILTIN_NODE_TYPES. */
#define AM_DFG_GEN_BUILTIN_NODES

/* Final list of all lists of node types from all headers included above */
static struct am_dfg_static_node_type_def** defsets[] = {
};

/* Destroys a list of node types */
static void am_dfg_builtin_type_list_destroy(struct list_head* list)
{
	struct am_dfg_node_type* nt;
	struct am_dfg_node_type* next;

	am_typed_list_for_each_safe_genentry(list, nt, next, list) {
		am_dfg_node_type_destroy(nt);
		free(nt);
	}
}

/* Register the builtin node types at the node type registry ntr using the type
 * registry tr. Returns 0 on success, otherwise 1. */
int am_dfg_builtin_node_types_register(struct am_dfg_node_type_registry* ntr,
				       struct am_dfg_type_registry* tr)
{
	struct am_dfg_static_node_type_def** curr_defset;
	struct am_dfg_static_node_type_def** pcurr_def;
	struct am_dfg_static_node_type_def* curr_def;
	struct am_dfg_node_type* nt;
	struct am_dfg_node_type* next;
	struct list_head types;

	INIT_LIST_HEAD(&types);

	/* First reserve memory for each node type and initialize */
	for(size_t i = 0; i < AM_ARRAY_SIZE(defsets); i++) {
		curr_defset = defsets[i];

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
	am_dfg_builtin_type_list_destroy(&types);
	return 1;
}