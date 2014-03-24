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
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

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
