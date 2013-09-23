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

#include "annotation.h"
#include "ansi_extras.h"
#include <string.h>

int annotation_init(struct annotation* a, int cpu, uint64_t time, const char* text)
{
	if(!(a->text = strdup(text)))
		return 1;

	a->time = time;
	a->cpu = cpu;
	a->event_set = NULL;

	return 0;
}

int annotation_copy(struct annotation* dst, struct annotation* src)
{
	return annotation_init(dst, src->cpu, src->time, src->text);
}

int annotation_set_text(struct annotation* a, const char* text)
{
	char* newtext = strdup(text);

	if(!newtext)
		return 1;

	free(a->text);
	a->text = newtext;

	return 0;
}

void annotation_destroy(struct annotation* a)
{
	free(a->text);
}
