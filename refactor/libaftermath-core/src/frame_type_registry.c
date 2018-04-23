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

#include "frame_type_registry.h"
#include "frame_type_registry.priv.h"

static inline void am_frame_type_destroy(struct am_frame_type* ft)
{
	free(ft->name);
}

int am_frame_type_registry_init(struct am_frame_type_registry* r,
				size_t max_types)
{
	r->max_types = max_types;
	am_frame_type_tree_init(&r->name_tree);

	if(!(r->types = calloc(sizeof(*r->types), max_types)))
		return 1;

	return 0;
}

void am_frame_type_registry_destroy(struct am_frame_type_registry* r)
{
	struct am_frame_type* iter;
	struct am_frame_type* tmp;

	am_frame_type_tree_for_each_safe(&r->name_tree, iter, tmp) {
		am_frame_type_destroy(iter);
		free(iter);
	}

	free(r->types);
}

/* Retrieves a registered frame type by its name. If the registry does not
 * contain any type with the specified name, NULL is returned. */
struct am_frame_type*
am_frame_type_registry_find(struct am_frame_type_registry* r,
			    const char* name)
{
	return am_frame_type_tree_find(&r->name_tree, name);
}

/* Adds a new frame type to a registry r. The name is duplicated and can safely
 * be freed after the call. The parameter "load" must be a pointer to a function
 * that is able to read and process the frame type using an I/O context or NULL.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_frame_type_registry_add(struct am_frame_type_registry* r,
			       const char* name,
			       int (*load)(struct am_io_context* ctx))
{
	struct am_frame_type* ft;

	if(!(ft = malloc(sizeof(*ft))))
		goto out_err;

	if(!(ft->name = strdup(name)))
		goto out_err_free_ft;

	ft->seq_id = 0;
	ft->load = load;

	if(am_frame_type_tree_insert(&r->name_tree, ft))
		goto out_err_free_name;

	return 0;

out_err_free_name:
	free(ft->name);
out_err_free_ft:
	free(ft);
out_err:
	return 1;
}

/* Associates the id seq_id with the frame type ft. Returns 0 on success,
 * otherwise 1. */
int am_frame_type_registry_set_id(struct am_frame_type_registry* r,
				  struct am_frame_type* ft,
				  size_t seq_id)
{
	/* Invalid ID or ID already registered? */
	if(seq_id >= r->max_types || r->types[seq_id])
		return 1;

	ft->seq_id = seq_id;
	r->types[seq_id] = ft;

	return 0;
}
