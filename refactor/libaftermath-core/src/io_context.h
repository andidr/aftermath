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

#ifndef AM_IO_CONTEXT_H
#define AM_IO_CONTEXT_H

#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <aftermath/core/io_error.h>
#include <aftermath/core/io_hierarchy_context.h>
#include <aftermath/core/trace.h>
#include <aftermath/core/frame_type_registry.h>
#include <aftermath/core/array_collection.h>

/* An IO context serves as a compound structure for temporary data needed when
 * loading / writing a trace from / to disk. When an IO operation fails, the
 * error stack captures the errors at the different levels from the outermost
 * function call down to the low-level IO / memory management / processing
 * function.
 */
struct am_io_context {
	struct am_io_error_stack error_stack;
	struct am_trace* trace;
	int bounds_valid;
	char* filename;
	FILE* fp;

	struct am_io_hierarchy_context hierarchy_context;
	struct am_frame_type_registry* frame_types;
};

enum am_io_mode {
	AM_IO_READ,
	AM_IO_WRITE
};

int am_io_context_init(struct am_io_context* ctx,
		       struct am_frame_type_registry* frame_types);
void am_io_context_destroy(struct am_io_context* ctx);
void am_io_context_reset(struct am_io_context* ctx);
int am_io_context_set_filename(struct am_io_context* ctx, const char* filename);
int am_io_context_open(struct am_io_context* ctx,
		       const char* filename,
		       enum am_io_mode m);
void am_io_context_close(struct am_io_context* ctx);
void am_io_fail(void);

/* Convenience macro that pushes a new error onto the I/O error stack of an I/O
 * context using a printf-style format string and a variable argument list,
 * invokes am_io_fail (for debugging) and returns 1. */
#define AM_IOERR_RET1(pctx, error_id, fmt, ...)		\
	do {							\
		am_io_error_stack_push(&(pctx)->error_stack,	\
					  error_id,		\
					  fmt,			\
					  __VA_ARGS__);	\
								\
		am_io_fail();					\
								\
		return 1;					\
	} while(0)

/* Convenience macro that pushes a new error described by a single string onto
 * the I/O error stack of an I/O context, invokes am_io_fail (for debugging) and
 * returns 1. */
#define AM_IOERR_RET1_NA(pctx, error_id, str)			\
	do {							\
		am_io_error_stack_push(&(pctx)->error_stack, \
					  error_id,		\
					  str);		\
								\
		am_io_fail();					\
								\
		return 1;					\
	} while(0)

/* Convenience macro that pushes a new error onto the I/O error stack of an I/O
 * context using a printf-style format string and a variable argument list,
 * invokes am_io_fail (for debugging) and jumps to the provided label. */
#define AM_IOERR_GOTO(pctx, label, error_id, fmt, ...)		\
	do {							\
		am_io_error_stack_push(&(pctx)->error_stack, \
					  error_id,		\
					  fmt,			\
					  __VA_ARGS__);	\
								\
		am_io_fail();					\
								\
		goto label;					\
	} while(0)

/* Convenience macro that pushes a new error described by a single string onto
 * the I/O error stack of an I/O context, invokes am_io_fail (for debugging) and
 * jumps to the provided label. */
#define AM_IOERR_GOTO_NA(pctx, label, error_id, str)		\
	do {							\
		am_io_error_stack_push(&(pctx)->error_stack,	\
					  error_id,		\
					  str);		\
								\
		am_io_fail();					\
								\
		goto label;					\
	} while(0)

#endif
