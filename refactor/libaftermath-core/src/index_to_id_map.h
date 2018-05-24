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

#ifndef AM_INDEX_TO_ID_MAP_H
#define AM_INDEX_TO_ID_MAP_H

#include "typed_array.h"
#include "bsearch.h"
#include "qsort.h"
#include <stdint.h>

#define AM_ACC_ID(x) ((x).id)

/* An index-to-ID-map is nothing but a dynamic array of pairs values,
 * associateing array indexes to integer IDs. The purpose of these structures is
 * to provide a generic way to associate IDs of on-disk structures with
 * instances of the corresponding in-memory structures without storing the IDs
 * permanently within the in-memory structure when IDs are only used to match
 * related data structures.
 */

#define AM_DECL_IO_INDEX_TO_ID_MAP(BITS)					\
	struct am_index_to_id_map_u##BITS##_entry {				\
		size_t index;							\
		uint##BITS##_t id;						\
	};									\
										\
	AM_DECL_TYPED_ARRAY(am_index_to_id_map_u##BITS,			\
			    struct am_index_to_id_map_u##BITS##_entry)		\
	AM_DECL_TYPED_ARRAY_BSEARCH(am_index_to_id_map_u##BITS,		\
				    struct am_index_to_id_map_u##BITS##_entry,	\
				    uint##BITS##_t,				\
				    AM_ACC_ID,					\
				    AM_VALCMP_EXPR)				\
									\
	static inline int am_index_to_id_map_u##BITS##_cmp_ids(\
		const struct am_index_to_id_map_u##BITS##_entry* a,		\
		const struct am_index_to_id_map_u##BITS##_entry* b)		\
	{								\
		return AM_VALCMP_EXPR(a->id, b->id);			\
	}								\
									\
	AM_DECL_QSORT_SUFFIX(am_index_to_id_map_u##BITS##_, ,		\
			     struct am_index_to_id_map_u##BITS##_entry,	\
			     am_index_to_id_map_u##BITS##_cmp_ids)		\
									\
	static inline void am_index_to_id_map_u##BITS##_sort_by_id(	\
		struct am_index_to_id_map_u##BITS* m)			\
	{								\
		am_index_to_id_map_u##BITS##_qsort(m->elements, m->num_elements); \
	}

AM_DECL_IO_INDEX_TO_ID_MAP(8)
AM_DECL_IO_INDEX_TO_ID_MAP(16)
AM_DECL_IO_INDEX_TO_ID_MAP(32)
AM_DECL_IO_INDEX_TO_ID_MAP(64)

#endif
