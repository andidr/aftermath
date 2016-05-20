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

#include "util.h"
#include "ansi_extras.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

unsigned int pretty_print_get_powern(long double v, long double base, int step, unsigned int min_pow, unsigned int max_pow)
{
	long double multiplier = 1;

	for(int i = max_pow; i > min_pow ; i--) {
		multiplier = powl(base, step*i);

		if(v > multiplier)
			return i;
	}

	return min_pow;
}

unsigned int pretty_print_get_power10(long double v, unsigned int min_pow, unsigned int max_pow)
{
	return pretty_print_get_powern(v, 10.0, 3, min_pow, max_pow);
}

unsigned int pretty_print_get_power2(long double v, unsigned int min_pow, unsigned int max_pow)
{
	return pretty_print_get_powern(v, 2.0, 10, min_pow, max_pow);
}

void pretty_print_number(char* buffer, int buffer_size, uint64_t bytes, const char* add)
{
	const char* units[] = { "", "K", "M", "G", "T", "P" };

	unsigned int unit_idx = pretty_print_get_power10(bytes, 0, (sizeof(units) / sizeof(char*))-1);
	long double multiplier = powl(10, 3*unit_idx);

	snprintf(buffer, buffer_size, "%.2Lf %s%s", (long double)bytes / multiplier, units[unit_idx], add);
}

void pretty_print_bytes(char* buffer, int buffer_size, uint64_t bytes, const char* add)
{
	const char* units[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB" };

	unsigned int unit_idx = pretty_print_get_power2(bytes, 0, (sizeof(units) / sizeof(char*))-1);
	long double multiplier = powl(2, 10*unit_idx);

	snprintf(buffer, buffer_size, "%.2Lf %s%s", (long double)bytes / multiplier, units[unit_idx], add);
}

void pretty_print_cycles(char* buffer, int buffer_size, uint64_t cycles)
{
	const char* units[] = { "", " K", " M", " G", " T", " P" };

	unsigned int unit_idx = pretty_print_get_power10(cycles, 0, (sizeof(units) / sizeof(char*))-1);
	long double multiplier = powl(10.0, 3*unit_idx);

	snprintf(buffer, buffer_size, "%.2Lf%s", (long double)cycles / multiplier, units[unit_idx]);
}

int pretty_read_cycles(const char* buffer, uint64_t* cycles)
{
	double d;

	if(atodbln_unit(buffer, strlen(buffer), &d))
		return 1;

	*cycles = (uint64_t)d;

	return 0;
}
