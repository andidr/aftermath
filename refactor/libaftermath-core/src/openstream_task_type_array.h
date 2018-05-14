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

#ifndef AM_OPENSTREAM_TASK_TYPE_ARRAY_H
#define AM_OPENSTREAM_TASK_TYPE_ARRAY_H

#include <aftermath/core/typed_array.h>
#include <aftermath/core/in_memory.h>

#define ACC_TYPE_ID(x) (x).type_id

AM_DECL_TYPED_ARRAY(am_openstream_task_type_array, struct am_openstream_task_type)
AM_DECL_TYPED_ARRAY_BSEARCH(am_openstream_task_type_array,
			    struct am_openstream_task_type,
			    am_counter_t,
			    ACC_TYPE_ID,
			    AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_INSERTPOS(am_openstream_task_type_array,
			      struct am_openstream_task_type,
			      am_counter_t,
			      ACC_TYPE_ID,
			      AM_VALCMP_EXPR)
AM_DECL_TYPED_ARRAY_RESERVE_SORTED(am_openstream_task_type_array,
				struct am_openstream_task_type,
				am_counter_t)

/* Destroys all of the elements of the array, but neither frees the array
 * structure itself, nor the space occupied by the array elements. */
static inline void
am_openstream_task_type_array_destroy_elements(struct am_openstream_task_type_array* a)
{
	for(size_t i = 0; i < a->num_elements; i++)
		am_openstream_task_type_destroy(&a->elements[i]);
}

#endif
