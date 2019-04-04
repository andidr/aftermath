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

#ifndef AM_CIRCULAR_BUFFER_H
#define AM_CIRCULAR_BUFFER_H

#include <stdlib.h>

/* Declares a circular buffer of type T */
#define AM_DECL_CIRCULAR_BUFFER(T, SUFFIX)					\
	struct am_circular_buffer##SUFFIX {					\
		size_t size;							\
		T* entries;							\
		size_t start_idx;						\
		size_t end_idx;						\
	};									\
										\
	static inline void am_circular_buffer##SUFFIX##_static_init(		\
		struct am_circular_buffer##SUFFIX* c,				\
		size_t size,							\
		size_t* entries)						\
	{									\
		c->entries = entries;						\
		c->size = size;						\
		c->start_idx = 0;						\
		c->end_idx = 0;						\
	}									\
										\
	static inline void am_circular_buffer##SUFFIX##_set(			\
		struct am_circular_buffer##SUFFIX* c, size_t idx)		\
	{									\
		c->entries[c->end_idx] = idx;					\
	}									\
										\
	static inline void am_circular_buffer##SUFFIX##_push(			\
		struct am_circular_buffer##SUFFIX* c, size_t idx)		\
	{									\
		c->end_idx = (c->end_idx + 1) % c->size;			\
										\
		/* When crossing start index, discard first element */		\
		if(c->end_idx == ((c->start_idx + 1) % c->size))		\
			c->start_idx = (c->start_idx + 1) % c->size;		\
										\
		am_circular_buffer##SUFFIX##_set(c, idx);			\
	}									\
										\
	static inline void am_circular_buffer##SUFFIX##_top(			\
		struct am_circular_buffer##SUFFIX* c, size_t* idx)		\
	{									\
		size_t this_idx = (c->end_idx + c->size - 1) % c->size; 	\
		*idx = c->entries[this_idx];					\
	}									\
										\
	static inline int am_circular_buffer##SUFFIX##_pop(			\
		struct am_circular_buffer##SUFFIX* c, size_t* idx)		\
	{									\
		if(c->end_idx != c->start_idx) {				\
			am_circular_buffer##SUFFIX##_top(c, idx);		\
			c->end_idx = (c->end_idx + c->size - 1) % c->size;	\
			*idx = c->entries[c->end_idx];				\
			return 0;						\
		} else {							\
			return 1;						\
		}								\
	}

#endif
