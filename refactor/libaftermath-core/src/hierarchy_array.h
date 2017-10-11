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

#ifndef AM_HIERARCHY_ARRAY_H
#define AM_HIERARCHY_ARRAY_H

#include "typed_array.h"
#include "hierarchy.h"

#define HIERARCHYP_ACC_ID(x) (x)->id

AM_DECL_TYPED_ARRAY(am_hierarchyp_array, struct am_hierarchy*)

#define am_hierarchyp_array_for_each(a, e)		\
	for((e) = &(a)->elements[0];			\
	    (e) != &(a)->elements[(a)->num_elements];	\
	    (e)++)

AM_DECL_TYPED_ARRAY_BSEARCH(am_hierarchyp_array,
			 struct am_hierarchy*,
			 am_hierarchy_id_t,
			 HIERARCHYP_ACC_ID)

/* Finds a hierarchy by ID. Returns the address of the pointer to the hierarchy
 * or NULL if no such hierarchy could be found. */
static inline struct am_hierarchy**
am_hierarchyp_array_find(struct am_hierarchyp_array* hpa, am_hierarchy_id_t id)
{
	return am_hierarchyp_array_bsearch(hpa, id);
}

/* Destroys all hierarchies that the array contains pointers to. The memory
 * associated to the hierarchies is freed, but not the space occupied by the
 * pointers in the array, nor the array itself. */
static inline void
am_hierarchyp_array_destroy_elements(struct am_hierarchyp_array* a)
{
	for(size_t i = 0; i < a->num_elements; i++) {
		am_hierarchy_destroy(a->elements[i]);
		free(a->elements[i]);
	}
}

AM_DECL_TYPED_ARRAY_INSERTPOS(am_hierarchyp_array,
			      struct am_hierarchy*,
			      am_hierarchy_id_t,
			      HIERARCHYP_ACC_ID)

AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_hierarchyp_array,
				   struct am_hierarchy*,
				   am_hierarchy_id_t)

AM_DECL_TYPED_ARRAY_REMOVE(am_hierarchyp_array,
			   struct am_hierarchy*,
			   am_hierarchy_id_t)

/* Add a hierarchy to the array. Returns 0 on success, otherwise 1. */
static inline int
am_hierarchyp_array_add(struct am_hierarchyp_array* hpa,
			struct am_hierarchy* h)
{
	struct am_hierarchy** hp;

	if(!(hp = am_hierarchyp_array_reserve_sorted(hpa, h->id)))
		return 1;

	*hp = h;

	return 0;
}

#endif
