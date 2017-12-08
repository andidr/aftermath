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

#ifndef AM_STATE_DESCRIPTION_ARRAY_H
#define AM_STATE_DESCRIPTION_ARRAY_H

#include <aftermath/core/typed_array.h>
#include <aftermath/core/in_memory.h>

#define ACC_STATE_ID(x) (x).state_id

AM_DECL_TYPED_ARRAY(am_state_description_array,
		    struct am_state_description)
AM_DECL_TYPED_ARRAY_BSEARCH(am_state_description_array,
			 struct am_state_description,
			 am_state_t,
			 ACC_STATE_ID)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_state_description_array,
			   struct am_state_description,
			   am_state_t,
			   ACC_STATE_ID)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_state_description_array,
				struct am_state_description,
				am_state_t)

/* Destroy the elements of the array, but frees neither the memory associated to
 * the elements, nor the array itself. */
static inline void
am_state_description_array_destroy_elements(struct am_state_description_array* a)
{
	for(size_t i = 0; i < a->num_elements; i++)
		am_state_description_destroy(&a->elements[i]);
}

#endif
