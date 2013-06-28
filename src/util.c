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
#include <math.h>
#include <stdio.h>

void pretty_print_bytes(char* buffer, int buffer_size, double bytes, const char* add)
{
	const char* units[] = { "B", "KB", "MB", "GB", "TB", "PB" };
	const char* unit = units[0];
	double multiplier = 1;

	for(int i = (sizeof(units) / sizeof(char*))-1; i > 0 ; i--) {
		multiplier = pow(10.0, 3*i);

		if(bytes > multiplier) {
			unit = units[i];
			break;
		}
	}

	snprintf(buffer, buffer_size, "%.2f %s%s", (double)bytes / multiplier, unit, add);
}
