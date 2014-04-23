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

#ifndef COLOR_H
#define COLOR_H

#define COL_NORM(x) (((double)x) / 255.0)
#define COL_EXP(x) (((double)x) * 255.0)

/* pastel18 color scheme from Graphviz */
#define NUM_NODE_COLORS 8

extern double node_colors_dbl[NUM_NODE_COLORS][3];

static inline void get_node_color_dbl(unsigned int node, unsigned int max_node, double* r, double* g, double* b)
{
	int max_node_wraps = max_node / NUM_NODE_COLORS;
	int colidx = node % NUM_NODE_COLORS;
	int node_wraps = node / NUM_NODE_COLORS;
	double wrap_intensity = (max_node_wraps > 0) ? 0.5+(0.5*(((double)node_wraps) / ((double)max_node_wraps))) : 1.0;


	*r = node_colors_dbl[colidx][0] * wrap_intensity;
	*g = node_colors_dbl[colidx][1] * wrap_intensity;
	*b = node_colors_dbl[colidx][2] * wrap_intensity;
}

#define NUM_TASK_TYPE_COLORS 10
extern double task_type_colors[NUM_TASK_TYPE_COLORS][3];

#endif
