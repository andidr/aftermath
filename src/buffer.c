/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "buffer.h"

int check_buffer_grow_n(void** buffer, size_t elem_size, size_t used, size_t* free, size_t n, size_t prealloc_elems)
{
	void* ptr;
	size_t alloc;

	if(*free < n) {
		alloc = (prealloc_elems < n-(*free)) ? n-(*free) : prealloc_elems;

		if(!(ptr = realloc(*buffer, (used+alloc)*elem_size))) {
			return 1;
		} else {
			*free = alloc;
			*buffer = ptr;
		}
	}

	return 0;
}

int check_buffer_grow(void** buffer, size_t elem_size, size_t used, size_t* free, size_t prealloc_elems)
{
	return check_buffer_grow_n(buffer, elem_size, used, free, 1, prealloc_elems);
}

int add_buffer_grow(void** buffer, void* elem, size_t elem_size, size_t* used, size_t* free, size_t prealloc_elems)
{
	if(check_buffer_grow(buffer, elem_size, *used, free, prealloc_elems))
		return 1;

	void* addr = ((char*)(*buffer))+(elem_size*(*used));

	memcpy(addr, elem, elem_size);
	(*free)--;
	(*used)++;

	return 0;
}
