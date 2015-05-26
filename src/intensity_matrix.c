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

#include "intensity_matrix.h"
#include <stdlib.h>
#include <string.h>

int intensity_matrix_init(struct intensity_matrix* m, int width, int height)
{
	m->width = width;
	m->height = height;

	if(!(m->intensity = malloc(width*height*sizeof(double))))
		return 1;

	if(!(m->absolute = malloc(width*height*sizeof(int64_t)))) {
		free(m->intensity);
		return 1;
	}

	memset(m->intensity, 0, width*height*sizeof(double));
	memset(m->absolute, 0, width*height*sizeof(int64_t));

	return 0;
}

void intensity_matrix_destroy(struct intensity_matrix* m)
{
	free(m->intensity);
	free(m->absolute);
}

void intensity_matrix_update_intensity(struct intensity_matrix* m)
{
	int64_t max = INT64_MIN;

	for(int x = 0; x < m->width; x++)
		for(int y = 0; y < m->height; y++)
			if(max < intensity_matrix_absolute_value_at(m, x, y))
				max = intensity_matrix_absolute_value_at(m, x, y);

	for(int x = 0; x < m->width; x++) {
		for(int y = 0; y < m->height; y++) {
			int64_t val = intensity_matrix_absolute_value_at(m, x, y);
			long double intensity;

			if(max == 0 && val == 0)
				intensity = 0;
			else
				intensity = ((long double)val) / ((long double)max);

			intensity_matrix_set_intensity_at(m, x, y, intensity);
		}
	}
}
