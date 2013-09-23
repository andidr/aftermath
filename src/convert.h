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

#ifndef CONVERT_H
#define CONVERT_H

#include <stdio.h>
#include <stdint.h>

#define CONVERSION_TABLE_END -1
#define FIELD_SIZE(st, fld) sizeof(((st*)0)->fld)

enum conversion_direction {
	CONVERT_DSK_TO_HOST = 0,
	CONVERT_HOST_TO_DSK,
};

/* Converts a structure either from or to on-disk format */
void convert_struct(void* ptr, int* conversion_table, int offset, enum conversion_direction dir);

/* Read a data structure from disk and convert it to host format */
int read_struct_convert(FILE* fp, void* out, int size, int* conversion_table, int offset);

/* Write a data structure to disk and convert it to on-disk format */
int write_struct_convert(FILE* fp, void* out, int size, int* conversion_table, int offset);

/* Read a unsigned 32-bit integer from disk and convert it to host format */
int read_uint32_convert(FILE* fp, uint32_t* out);

#endif
