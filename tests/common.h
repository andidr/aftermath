/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
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

#include <stdio.h>
#include <unistd.h>
#include "../src/ansi_extras.h"

/* Creates a temporary file based on a template, opens it and returns
 * a pointer to the FILE structure.
 */
static inline FILE* tmpfile_template(char* template, const char* mode)
{
	FILE* fp;
	int fd;

	if((fd = mkstemp(template)) == -1)
		return NULL;

	if(!(fp = fdopen(fd, mode))) {
		close(fd);
		return NULL;
	}

	return fp;
}
