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

#include <aftermath/core/buffer.h>
#include <aftermath/core/safe_alloc.h>

/* Checks if a buffer *buffer with used elements and free remaining elements of
 * size elem_size needs to be resized in order to store n additional
 * elements. If the buffer needs to be resized, at least prealloc_elements are
 * added to the size of the buffer. If prealloc_elements is smaller than the
 * number of required elements, then a larger amount of memory is added to the
 * buffer.  The reallocated buffer is returned in *buffer.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_check_buffer_grow_n(void** buffer, size_t elem_size, size_t used,
			   size_t* free, size_t n, size_t prealloc_elems)
{
	void* ptr;
	size_t alloc;
	size_t total_elements;
	size_t new_free;

	/* Already enough space left? */
	if(*free >= n)
		return 0;

	if(am_size_add_safe(&total_elements, used, *free))
		return 1;

	if(prealloc_elems < n-(*free))
		alloc = n - (*free);
	else
		alloc = prealloc_elems;

	if(am_size_add_safe(&total_elements, total_elements, alloc))
		return 1;

	if(am_size_add_safe(&new_free, *free, alloc))
		return 1;

	if(!(ptr = am_realloc_array_safe(*buffer, total_elements, elem_size)))
		return 1;

	*free = new_free;
	*buffer = ptr;

	return 0;
}

/* Same as check_buffer_grow_n, but for a single element */
int am_check_buffer_grow(void** buffer, size_t elem_size, size_t used,
			 size_t* free, size_t prealloc_elems)
{
	return am_check_buffer_grow_n(buffer, elem_size, used, free,
				      1, prealloc_elems);
}

/* Copies the element *elem to the last unused position of a buffer *buffer. If
 * necessary, the buffer is resized to receive the new element. Elem size is the
 * size of the element in bytes, *used indicates how many elements are already
 * used in the buffer, *free indicates how many free positions there are in the
 * buffer and prealloc_elemens indicated by how many additional elements the
 * buffer is resized if it cannot hold the additional element.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_add_buffer_grow(void** buffer, void* elem, size_t elem_size, size_t* used,
		       size_t* free, size_t prealloc_elems)
{
	void* addr;
	void* addr_end;

	if(am_check_buffer_grow(buffer, elem_size, *used, free, prealloc_elems))
		return 1;

	if(!(addr = am_array_element_ptr_safe(*buffer, *used, elem_size)))
		return 1;

	/* Safe guard for subtraction below */
	if(elem_size == 0)
		return 1;

	/* Would a write of one element wrap around in the address space? */
	if(!(addr_end = am_array_element_ptr_safe(addr, 1, elem_size-1)))
		return 1;

	memcpy(addr, elem, elem_size);
	(*free)--;
	(*used)++;

	return 0;
}
