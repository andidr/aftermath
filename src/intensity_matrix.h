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

#ifndef INTENSITY_MATRIX_H
#define INTENSITY_MATRIX_H

#include <stdint.h>

struct intensity_matrix {
	int width;
	int height;
	double* intensity;
	int64_t* absolute;
};

int intensity_matrix_init(struct intensity_matrix* m, int width, int height);
void intensity_matrix_destroy(struct intensity_matrix* m);
void intensity_matrix_update_intensity(struct intensity_matrix* m);

static inline double intensity_matrix_intensity_at(struct intensity_matrix* m, int x, int y)
{
	return m->intensity[y*m->width + x];
}

static inline double intensity_matrix_set_intensity_at(struct intensity_matrix* m, int x, int y, double v)
{
	return m->intensity[y*m->width + x] = v;
}

static inline double intensity_matrix_add_intensity_at(struct intensity_matrix* m, int x, int y, double v)
{
	return m->intensity[y*m->width + x] += v;
}

static inline double intensity_matrix_absolute_value_at(struct intensity_matrix* m, int x, int y)
{
	return m->absolute[y*m->width + x];
}

static inline double intensity_matrix_set_absolute_value_at(struct intensity_matrix* m, int x, int y, int64_t v)
{
	return m->absolute[y*m->width + x] = v;
}

static inline double intensity_matrix_add_absolute_value_at(struct intensity_matrix* m, int x, int y, int64_t v)
{
	return m->absolute[y*m->width + x] += v;
}

#endif
