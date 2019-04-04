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

#ifndef AM_BUFFER_H
#define AM_BUFFER_H

#include <stdlib.h>
#include <string.h>

int am_check_buffer_grow_n(void** buffer, size_t elem_size, size_t used, size_t* free, size_t n, size_t prealloc_elems);
int am_check_buffer_grow(void** buffer, size_t elem_size, size_t used, size_t* free, size_t prealloc_elems);
int am_add_buffer_grow(void** buffer, void* elem, size_t elem_size, size_t* used, size_t* free, size_t prealloc_elems);

#endif
