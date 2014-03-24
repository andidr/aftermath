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

#ifndef UNCOMPRESS_H
#define UNCOMPRESS_H

#include <stdio.h>
#include <sys/types.h>

enum compression_type {
	COMPRESSION_TYPE_UNCOMPRESSED = 0,
	COMPRESSION_TYPE_GZIP,
	COMPRESSION_TYPE_BZIP2,
	COMPRESSION_TYPE_XZ,
	COMPRESSION_TYPE_UNKNOWN
};

struct uncompress_pipe {
	pid_t pid;
	int stdout_fd;
	int stderr_fd;

	FILE* stdout;
	FILE* stderr;
};

int uncompress_detect_type(const char* filename, enum compression_type* type);
int uncompress_pipe_open_cmd(struct uncompress_pipe* p, char* cmd, const char* const argv[]);
int uncompress_pipe_open(struct uncompress_pipe* p, enum compression_type type, const char* filename);
int uncompress_pipe_close(struct uncompress_pipe* p, int* status);

#endif
