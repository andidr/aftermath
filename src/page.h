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

#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

static inline uint64_t get_base_address(uint64_t addr, uint64_t alignment)
{
	return addr & ~(alignment-1);
}

static inline void get_page_bounds(uint64_t addr, uint64_t* start, uint64_t* end, uint64_t page_size)
{
	*start = get_base_address(addr, page_size);
	*end = (*start) + (page_size-1);
}

#endif
