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

#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <stdint.h>

struct event_set;

struct annotation {
	struct event_set* event_set;
	char* text;
	uint64_t time;
	int cpu;

	double color_r;
	double color_g;
	double color_b;
};

int annotation_init(struct annotation* a, int cpu, uint64_t time, const char* text);
int annotation_copy(struct annotation* dst, struct annotation* src);
int annotation_set_text(struct annotation* a, const char* text);
void annotation_destroy(struct annotation* a);

#endif
