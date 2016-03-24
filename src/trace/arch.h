/**
 * Copyright (C) 2016 Andi Drebes <andi.drebes@lip6.fr>
 *
 * Libaftermath-trace is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef AFTERMATH_ARCH_H
#define AFTERMATH_ARCH_H

#ifdef __i386
	static inline uint64_t am_tsc(void)
	{
		uint64_t x;
		__asm__ volatile ("rdtsc" : "=A" (x));
		return x;
	}

#elif defined __amd64
	static inline uint64_t am_tsc(void)
	{
		uint64_t a, d;
		__asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
		return (d<<32) | a;
	}
#else
	#error "No timestamp counter function defined for your architecture"
#endif

#endif
