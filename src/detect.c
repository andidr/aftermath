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

#include "detect.h"
#include "trace_file.h"
#include <string.h>

int detect_trace_format(const char* file, enum trace_format* out)
{
	int ret = 1;
	FILE* fp = fopen(file, "r");
	char magic[8];

	if(!fp)
		goto out;

	if(fread(magic, sizeof(magic), 1, fp) != 1)
		goto out_fp;

	if(strncmp(magic, "#Paraver", sizeof(magic)) == 0)
		*out = TRACE_FORMAT_PARAVER;
	else if(strncmp(magic, "OSTV", 4) == 0)
		*out = TRACE_FORMAT_OSTV;
	else
		*out = TRACE_FORMAT_UNKNOWN;

	ret = 0;

out_fp:
	fclose(fp);
out:
	return ret;
}
