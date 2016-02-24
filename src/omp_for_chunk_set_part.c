/**
 * Copyright (C) 2016 Jean-Baptiste Bréjon <jean-baptiste.brejon@lip6.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more deta *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "omp_for_chunk_set_part.h"
#include <string.h>

int compare_ofcpsp(const void *po1, const void *po2)
{
	const struct omp_for_chunk_set_part* o1 = *((struct omp_for_chunk_set_part**)po1);
	const struct omp_for_chunk_set_part* o2 = *((struct omp_for_chunk_set_part**)po2);

	if(o1->start < o2->start)
		return -1;
	else if(o1->start > o2->start)
		return 1;
	else {
		if(o1->cpu < o2->cpu)
			return -1;
		else if(o1->cpu > o2->cpu)
			return 1;
	}

	return 0;
}
