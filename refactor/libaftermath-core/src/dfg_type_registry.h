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

#ifndef AM_DFG_TYPE_REGISTRY_H
#define AM_DFG_TYPE_REGISTRY_H

#include <aftermath/core/dfg_type.h>
#include <aftermath/core/typed_list.h>

/* TODO: Make lookup faster using binary search */

enum am_dfg_type_registry_flags {
	AM_DFG_TYPE_REGISTRY_DESTROY_TYPES = (1 << 0)
};

/* Set of types */
struct am_dfg_type_registry {
	/* Chaining of all types in the registry */
	struct list_head types;

	long flags;
};

#define am_dfg_type_registry_for_each_type(r, t) \
	am_typed_list_for_each(r, types, t, list)

#define am_dfg_type_registry_for_each_type_prev(r, t) \
	am_typed_list_for_each_prev(r, types, t, list)

#define am_dfg_type_registry_for_each_type_safe(r, n, i) \
	am_typed_list_for_each_safe(r, types, t, i, list)

#define am_dfg_type_registry_for_each_type_prev_safe(r, t, i) \
	am_typed_list_for_each_prev_safe(r, types, t, i, list)

void am_dfg_type_registry_init(struct am_dfg_type_registry* reg, long flags);
void am_dfg_type_registry_destroy(struct am_dfg_type_registry* reg);

void am_dfg_type_registry_add(struct am_dfg_type_registry* reg,
			      struct am_dfg_type* t);

struct am_dfg_type* am_dfg_type_registry_lookup(struct am_dfg_type_registry* reg,
						const char* name);

int am_dfg_type_registry_types_compatible(const struct am_dfg_type_registry* reg,
					  const struct am_dfg_type* tsrc,
					  const struct am_dfg_type* tdst);

#endif
