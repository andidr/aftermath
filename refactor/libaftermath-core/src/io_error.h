/**
 * Author: Andi Drebes <andi@drebesium.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef IO_ERROR_H
#define IO_ERROR_H

#include <stdio.h>
#include <stdlib.h>

/* The I/O error data structures and functions implement basic error handling
 * when loading an Aftermath trace file. Individual errors are represented by
 * am_io_error structures, indicating a machine-readable error category with its
 * error id and a human-redable cause with its error message. Errors can be
 * organized from the root error (i.e., the failing operation) to the error of
 * the topmost depending operation in an I/O error stack.
 */

enum am_io_error_id {
	/* Memory allocation failed */
	AM_IOERR_ALLOC,

	/* Data format Conversion failed */
	AM_IOERR_CONVERT,

	/* An item referred to some other item that cannot be found (e.g., a
	 * hierarchy node referring to an unknown hierarchy)
	 */
	AM_IOERR_FIND_RELATED,

	/* Definition of an already defined item was found (e.g., a hierarchy
	 * node with the same ID) */
	AM_IOERR_DUPLICATE,

	/* An item could not be added to some collection */
	AM_IOERR_ADD,

	/* Initialization of an item failed */
	AM_IOERR_INIT,

	/* Some assertion failed (e.g., a frame field has an invalid value) */
	AM_IOERR_ASSERT,

	/* File not found */
	AM_IOERR_NOSUCHFILE,

	/* Insufficient permissions to access a file */
	AM_IOERR_PERMISSION,

	/* Attempt to open a file which is not an Aftermath trace file */
	AM_IOERR_MAGIC,

	/* Fiel with incompatible version of the file format */
	AM_IOERR_VERSION,

	/* Read operation on a file failed */
	AM_IOERR_READ,

	/* Write operation on a file failed */
	AM_IOERR_WRITE,

	/* Error in the main loop processing all frames of a trace file */
	AM_IOERR_READ_FRAMES,

	/* Error in during post-processing of a loaded trace */
	AM_IOERR_POSTPROCESS,

	/* Error in the function reading and processing a frame */
	AM_IOERR_LOAD_FRAME,

	/* Error during read operation of a frame */
	AM_IOERR_READ_FRAME,

	/* Error during write operation of a frame */
	AM_IOERR_WRITE_FRAME,

	/* Error during read operation of a frame's field */
	AM_IOERR_READ_FIELD,

	/* Error during write operation of a frame's field */
	AM_IOERR_WRITE_FIELD,

	/* Unknown frame type encountered */
	AM_IOERR_UNKNOWN_FRAME,

	/* Unknown error occurred */
	AM_IOERR_UNKNOWN,

	/* Overflow happened where it shouldn't */
	AM_IOERR_OVERFLOW,
};

struct am_io_error {
	enum am_io_error_id error_id;
	size_t max_msglen;
	char* msgbuf;
};

int am_io_error_init(struct am_io_error* e, size_t max_msglen);
void am_io_error_destroy(struct am_io_error* e);
void am_io_error_set(struct am_io_error* e,
		     enum am_io_error_id error,
		     const char* fmt,
		     ...);

struct am_io_error_stack {
	struct am_io_error* errors;
	size_t pos;
	size_t max_nesting;
	size_t max_msglen;
};

int am_io_error_stack_move(struct am_io_error_stack* dst,
			   struct am_io_error_stack* src);

int am_io_error_stack_init(struct am_io_error_stack* s,
			   size_t max_nesting,
			   size_t max_msglen);
int am_io_error_stack_definit(struct am_io_error_stack* s);
void am_io_error_stack_destroy(struct am_io_error_stack* s);

int am_io_error_stack_push(struct am_io_error_stack* s,
			   enum am_io_error_id error_id,
			   const char* fmt,
			   ...);
void am_io_error_stack_dump_file(struct am_io_error_stack* s, FILE* fp);
void am_io_error_stack_dump(struct am_io_error_stack* s);
void am_io_error_stack_dump_stderr(struct am_io_error_stack* s);

/* Returns 1 if the error stack s is empty, i.e., does not contain any
 * errors. */
static inline int am_io_error_stack_empty(struct am_io_error_stack* s)
{
	return s->pos == 0;
}

#endif
