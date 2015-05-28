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

#include "uncompress.h"
#include "ansi_extras.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

int uncompress_detect_type(const char* filename, enum compression_type* type)
{
	char magic[6];

	FILE* fp = fopen(filename, "r");

	if(!fp)
		return 1;

	if(fread(magic, sizeof(magic), 1, fp) != 1) {
		fclose(fp);
		return 1;
	}

	if(strncmp(magic, "OSTV", 4) == 0)
		*type = COMPRESSION_TYPE_UNCOMPRESSED;
	else if(strncmp(magic, "\x1F\x8B\x08", 3) == 0)
		*type = COMPRESSION_TYPE_GZIP;
	else if(strncmp(magic, "\x42\x5A\x68", 3) == 0)
		*type = COMPRESSION_TYPE_BZIP2;
	else if(strncmp(magic, "\xFD\x37\x7A\x58\x5A\x00", 6) == 0)
		*type = COMPRESSION_TYPE_XZ;
	else
		*type = COMPRESSION_TYPE_UNKNOWN;

	fclose(fp);
	return 0;
}

int uncompress_pipe_open_cmd(struct uncompress_pipe* p, char* cmd, const char* const argv[])
{
	int pipe_stdout_fd[2];
	int pipe_stderr_fd[2];

	if(pipe(pipe_stdout_fd) == -1)
		goto out_err;
	if(pipe(pipe_stderr_fd) == -1)
		goto out_err_stdout;

	p->pid = fork();

	if(p->pid == 0) {
		/* Child process */
		dup2(pipe_stdout_fd[1], 1);
		dup2(pipe_stderr_fd[1], 2);
		close(pipe_stdout_fd[0]);
		close(pipe_stderr_fd[0]);

		/* Exec command */
		if(execvp(cmd, (char* const*)argv))
			exit(1);
	} else if(p->pid == -1) {
		goto out_err_fork;
	}

	close(pipe_stdout_fd[1]);
	close(pipe_stderr_fd[1]);

	p->stdout_fd = pipe_stdout_fd[0];
	p->stderr_fd = pipe_stderr_fd[0];

	if(!(p->stdout = fdopen(p->stdout_fd, "r")))
		goto out_err_fp_stdout;

	if(!(p->stderr = fdopen(p->stderr_fd, "r")))
		goto out_err_fp_stderr;

	return 0;

out_err_fp_stderr:
	fclose(p->stdout);
out_err_fp_stdout:
out_err_fork:
	close(pipe_stderr_fd[0]);
	close(pipe_stderr_fd[1]);
out_err_stdout:
	close(pipe_stdout_fd[0]);
	close(pipe_stdout_fd[1]);
out_err:
	return 1;
}

int uncompress_pipe_open(struct uncompress_pipe* p, enum compression_type type, const char* filename)
{
	switch(type) {
		case COMPRESSION_TYPE_UNCOMPRESSED:
		case COMPRESSION_TYPE_UNKNOWN:
			return 1;
		case COMPRESSION_TYPE_GZIP:
		{
			const char* const args[] = {"gunzip", "--stdout", filename, NULL};
			return uncompress_pipe_open_cmd(p, "gunzip", args);
		}
		case COMPRESSION_TYPE_BZIP2:
		{
			const char* const args[] = {"bunzip2", "-k", "--stdout", filename, NULL};
			return  uncompress_pipe_open_cmd(p, "bunzip2", args);
		}
		case COMPRESSION_TYPE_XZ:
		{
			const char* const args[] = {"unxz", "-k", "--stdout", filename, NULL};
			return  uncompress_pipe_open_cmd(p, "unxz", args);
		}
	}

	return 1;
}

int gzip_get_uncompressed_size(const char* filename, off_t* size)
{
	struct uncompress_pipe p;
	char* line_ptr;
	size_t n;
	int status;
	int ret = 1;
	size_t compressed_size;
	size_t uncompressed_size;

	const char* const args[] = {"gzip", "-l", filename, NULL};

	if(uncompress_pipe_open_cmd(&p, "gzip", args))
		goto out_err;

	/* Discard first line */
	if(getline(&line_ptr, &n, p.stdout) == -1)
		goto out_pipe;

	if(getline(&line_ptr, &n, p.stdout) == -1)
		goto out_free;

	if(sscanf(line_ptr, "%zu %zu", &compressed_size, &uncompressed_size) != 2)
		goto out_free;

	*size = uncompressed_size;

	ret = 0;
out_free:
	free(line_ptr);
out_pipe:
	uncompress_pipe_close(&p, &status);
out_err:
	return ret;
}

int xz_get_uncompressed_size(const char* filename, off_t* size)
{
	struct uncompress_pipe p;
	char* line_ptr = NULL;
	size_t n;
	int status;
	int ret = 1;
	size_t compressed_size;
	size_t uncompressed_size;
	int found = 0;
	int garbage;

	const char* const args[] = {"xz", "-l", "--robot", filename, NULL};

	if(uncompress_pipe_open_cmd(&p, "xz", args))
		goto out_err;

	/* Discard all but the last line */
	while(!found) {
		if(getline(&line_ptr, &n, p.stdout) == -1)
			goto out_free;

		if(strncmp(line_ptr, "totals", 6) == 0)
			found = 1;
	}

	/* Skip "totals " */
	if(sscanf(line_ptr+7, "%d %d %zu %zu", &garbage, &garbage, &compressed_size, &uncompressed_size) != 4)
		goto out_free;

	*size = uncompressed_size;

	ret = 0;

out_free:
	free(line_ptr);
	uncompress_pipe_close(&p, &status);
out_err:
	return ret;
}

int uncompress_get_size(const char* filename, off_t* size)
{
	enum compression_type type;

	if(uncompress_detect_type(filename, &type))
		return 1;

	switch(type) {
		case COMPRESSION_TYPE_UNKNOWN:
		case COMPRESSION_TYPE_BZIP2:
			return 1;

		case COMPRESSION_TYPE_UNCOMPRESSED:
			*size = file_size(filename);
			return 0;

		case COMPRESSION_TYPE_GZIP:
			return gzip_get_uncompressed_size(filename, size);

		case COMPRESSION_TYPE_XZ:
			return xz_get_uncompressed_size(filename, size);
	}

	return 1;
}

int uncompress_pipe_close(struct uncompress_pipe* p, int* status)
{
	int s;

	if(waitpid(p->pid, &s, 0) == -1)
		return 1;

	fclose(p->stdout);
	fclose(p->stderr);
	close(p->stdout_fd);
	close(p->stderr_fd);

	if(WIFEXITED(s))
		*status = WEXITSTATUS(s);
	else
		*status = 1;

	return 0;
}
