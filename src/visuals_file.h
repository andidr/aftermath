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

#ifndef VISUALS_H
#define VISUALS_H

#include "multi_event_set.h"

/* OSTA in ASCII */
#define VISUALS_MAGIC 0x4154534f
#define VISUALS_VERSION 1

enum visual_type {
	VISUAL_TYPE_ANNOTATION = 0
};

/* File header */
struct visuals_header {
	/* Magic number */
	uint32_t magic;

	/* Version of the file format used on disk */
	uint32_t version;
} __attribute__((packed));

extern int visuals_header_conversion_table[];

/* A visuals element header is included in every data structure
 * used in the visuals file in order to identify the type of
 * the element.
 */
struct visuals_element_header {
	/* Struct type */
	uint32_t type;
} __attribute__((packed));

extern int visuals_element_header_conversion_table[];

/* An annotation with user-defiend text.
 * The text ist stored directly behind this structure.
 */
struct visuals_annotation {
	struct visuals_element_header header;

	/* CPU of the annotation */
	uint32_t cpu;

	/* Time of the annotation */
	uint64_t time;

	/* Annotation color, 16 bits per channel */
	uint16_t color_r;
	uint16_t color_g;
	uint16_t color_b;

	/* Length of the text not including the
	 * terminating zero byte */
	uint32_t length;
} __attribute__((packed));

extern int visuals_annotation_conversion_table[];

int load_visuals(const char* file, struct multi_event_set* mes);
int store_visuals(const char* file, struct multi_event_set* mes);

#endif
