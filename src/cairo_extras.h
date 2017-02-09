/**
 * Copyright (C) 2014 Andi Drebes <andi.drebes@lip6.fr>
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

#ifndef CAIRO_EXTRAS_H
#define CAIRO_EXTRAS_H

#include <cairo.h>

void cairo_extra_striped_rectangle(cairo_t* cr, double rect_left, double rect_top, double rect_width, double rect_height, double* dash, int num_dash);

struct cairo_extra_rgba {
	double r;
	double g;
	double b;
	double a;
};

#define CAIRO_EXTRA_RGBA_ARGS(rgba) (rgba).r, (rgba).g, (rgba).b, (rgba).a
#define CAIRO_EXTRA_PRGBA_ARGS(rgba) (rgba)->r, (rgba)->g, (rgba)->b, (rgba)->a

#endif
