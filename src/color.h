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

#include "ansi_extras.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define COL_NORM(x) (((double)x) / 255.0)
#define COL_EXP(x) (((double)x) * 255.0)

/* pastel18 color scheme from Graphviz */
#define NUM_NODE_COLORS 8
extern double node_colors_dbl[NUM_NODE_COLORS][3];

#define NUM_TASK_TYPE_COLORS 10
extern double task_type_colors[NUM_TASK_TYPE_COLORS][3];

#define NUM_STATE_COLORS 9
extern double state_colors[NUM_STATE_COLORS][3];

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


static inline void get_node_color_uchar(unsigned int node, unsigned int max_node, unsigned char* r, unsigned char* g, unsigned char* b)
{
	double dr, dg, db;
	get_node_color_dbl(node, max_node, &dr, &dg, &db);
	*r = COL_EXP(dr);
	*g = COL_EXP(dg);
	*b = COL_EXP(db);
}

static inline void get_node_color_htmlrgb(unsigned int node, unsigned int max_node, char* buff)
{
	unsigned char r, g, b;
	get_node_color_uchar(node, max_node, &r, &g, &b);
	snprintf(buff, 8, "#%02X%02X%02X", (int)r, (int)g, (int)b);
}

static inline void color_to_htmlrgb(double r, double g, double b, char* buff)
{
	snprintf(buff, 8, "#%02X%02X%02X", (int)COL_EXP(r), (int)COL_EXP(g), (int)COL_EXP(b));
}

static inline int htmlrgb_to_uchar(const char* str, unsigned char* r, unsigned char* g, unsigned char* b)
{
	size_t len = strlen(str);

	if(len != 4 && len != 7)
		return 1;

	if(str[0] != '#')
		return 1;

	for(size_t i = 1; i < len; i++)
		if(!isxdigit(str[i]))
			return 1;

	if(len == 4) {
		*r = xdigit_val(str[1]);
		*g = xdigit_val(str[2]);
		*b = xdigit_val(str[3]);

		*r |= (*r) << 4;
		*g |= (*g) << 4;
		*b |= (*b) << 4;
	} else {
		*r = (xdigit_val(str[1]) << 4) | xdigit_val(str[2]);
		*g = (xdigit_val(str[3]) << 4) | xdigit_val(str[4]);
		*b = (xdigit_val(str[5]) << 4) | xdigit_val(str[6]);
	}

	return 0;
}

static inline int htmlrgb_to_double(const char* str, double* r, double* g, double* b)
{
	unsigned char ur;
	unsigned char ug;
	unsigned char ub;

	if(htmlrgb_to_uchar(str, &ur, &ug, &ub))
		return 1;

	*r = COL_NORM(ur);
	*g = COL_NORM(ug);
	*b = COL_NORM(ub);

	return 0;
}

#endif
