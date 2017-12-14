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

#ifndef AM_DFG_NODE_TYPE_REGISTRY_H
#define AM_DFG_NODE_TYPE_REGISTRY_H

#include <aftermath/core/dfg_node.h>
#include <aftermath/core/object_notation.h>

enum am_dfg_node_type_registry_flags {
	AM_DFG_NODE_TYPE_REGISTRY_DESTROY_TYPES = (1 << 0)
};

/* Set of types */
struct am_dfg_node_type_registry {
	/* Chaining of all types in the registry */
	struct list_head types;

	long flags;
};

#define am_dfg_node_type_registry_for_each_type(r, t) \
	am_typed_list_for_each(r, types, t, list)

#define am_dfg_node_type_registry_for_each_type_prev(r, t) \
	am_typed_list_for_each_prev(r, types, t, list)

#define am_dfg_node_type_registry_for_each_type_safe(r, n, i) \
	am_typed_list_for_each_safe(r, types, t, i, list)

#define am_dfg_node_type_registry_for_each_type_prev_safe(r, n, i) \
	am_typed_list_for_each_prev_safe(r, types, t, i, list)

void am_dfg_node_type_registry_init(struct am_dfg_node_type_registry* reg,
				    long flags);
void am_dfg_node_type_registry_destroy(struct am_dfg_node_type_registry* reg);

void am_dfg_node_type_registry_add(struct am_dfg_node_type_registry* reg,
				   struct am_dfg_node_type* t);

struct am_dfg_node_type*
am_dfg_node_type_registry_lookup(struct am_dfg_node_type_registry* reg,
				 const char* name);

struct am_dfg_node*
am_dfg_node_type_registry_node_from_object_notation(
	struct am_dfg_node_type_registry* reg,
	struct am_object_notation_node* n_node);

#endif
