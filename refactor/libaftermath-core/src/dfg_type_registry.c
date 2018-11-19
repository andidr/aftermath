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

#include <aftermath/core/dfg_type_registry.h>
#include <string.h>

/*
 * Add a type to the registry
 */
void am_dfg_type_registry_add(struct am_dfg_type_registry* reg,
			      struct am_dfg_type* t)
{
	list_add(&t->list, &reg->types);
}

/*
 * Find a type by name. Returns NULL if the type cannot be found.
 */
struct am_dfg_type* am_dfg_type_registry_lookup(struct am_dfg_type_registry* reg,
						const char* name)
{
	struct am_dfg_type* t;

	am_dfg_type_registry_for_each_type(reg, t)
		if(strcmp(t->name, name) == 0)
			return t;

	return NULL;
}

/* Initialize a registry */
void am_dfg_type_registry_init(struct am_dfg_type_registry* reg, long flags)
{
	INIT_LIST_HEAD(&reg->types);
	reg->flags = flags;
}

/* Destroy a registry */
void am_dfg_type_registry_destroy(struct am_dfg_type_registry* reg)
{
	struct am_dfg_type* t;
	struct am_dfg_type* tmp;

	if(reg->flags & AM_DFG_TYPE_REGISTRY_DESTROY_TYPES) {
		am_dfg_type_registry_for_each_type_safe(reg, t, tmp) {
			am_dfg_type_destroy(t);
			free(t);
		}
	}
}

/* Returns 1 if the types tsrc and tdst are compatible, i.e., if an instance of
 * tsrc can be converted into tdst. Otherwise, 0 is returned.
 */
int am_dfg_type_registry_types_compatible(const struct am_dfg_type_registry* reg,
					  const struct am_dfg_type* tsrc,
					  const struct am_dfg_type* tdst)
{
	/* Currently, types are not converted, so types must be identical */
	return (tsrc == tdst);
}

/* Destroys a list of types */
static void am_dfg_builtin_type_list_destroy(struct list_head* list)
{
	struct am_dfg_type* t;
	struct am_dfg_type* next;

	am_typed_list_for_each_safe_genentry(list, t, next, list) {
		am_dfg_type_destroy(t);
		free(t);
	}
}

/* Registers static DFG type definitions atomically at the type registry tr. The
 * definitions are given as a set of pointers to sets of static type
 * definitions, terminated by a NULL entry. This allows for grouping as in the
 * example below, where ALLdefs contains the definitions of two groups of types:
 *
 *   struct am_dfg_static_type_def tA1 = { ... };
 *   struct am_dfg_static_type_def tA2 = { ... };
 *   struct am_dfg_static_type_def* Adefs[] = { &tA1, &tA2, NULL };
 *
 *   struct am_dfg_static_type_def tB1 = { ... };
 *   struct am_dfg_static_type_def tB2 = { ... };
 *   struct am_dfg_static_type_def* Bdefs[] = { &tB1, &tB2, NULL };
 *
 *   struct am_dfg_static_node_type_def** ALLdefs[] = { Adefs, Bdefs, NULL };
 *
 * The function returns 0 on success, otherwise 1.
 */
int am_dfg_type_registry_add_static(struct am_dfg_type_registry* tr,
				    struct am_dfg_static_type_def*** defsets)
{
	struct list_head list;
	struct am_dfg_static_type_def*** pset;
	struct am_dfg_static_type_def** pstd;
	struct am_dfg_static_type_def* std;
	struct am_dfg_type* t;
	struct am_dfg_type* next;

	INIT_LIST_HEAD(&list);

	/* First reserve memory for each type and initialize */
	for(pset = defsets; *pset; pset++) {
		for(pstd = *pset; *pstd; pstd++) {
			std = *pstd;

			if(!(t = malloc(sizeof(*t))))
				goto out_err_t;

			if(am_dfg_type_init(t, std->name, std->sample_size))
				goto out_err;

			t->destroy_samples = std->destroy_samples;
			t->to_string = std->to_string;
			t->from_string = std->from_string;
			t->check_string = std->check_string;

			list_add(&t->list, &list);
		}
	}

	/* Register entire list of initialized types. Use safe version for
	 * iteration, since the embedded list of the type will be used to
	 * enqueue the type at the type registry.  */
	am_typed_list_for_each_safe_genentry(&list, t, next, list)
		am_dfg_type_registry_add(tr, t);

	return 0;

out_err_t:
	free(t);
out_err:
	am_dfg_builtin_type_list_destroy(&list);
	return 1;
}
