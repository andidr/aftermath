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

#include "io_error.h"
#include <stdarg.h>
#include <stdio.h>

#define AM_IO_ERROR_MAX_MSGLEN 256
#define AM_IO_ERROR_MAX_NESTING 16

/* Intialize an I/O error with a maximum error message length of
 * max_msglen. Returns 0 on success, otherwise 1.
 */
int am_io_error_init(struct am_io_error* e,
		     size_t max_msglen)
{
	if(!(e->msgbuf = malloc(max_msglen)))
		return 1;

	e->msgbuf[0] = '\0';
	e->max_msglen = max_msglen;

	return 0;
}

/* Destroys an I/O error */
void am_io_error_destroy(struct am_io_error* e)
{
	free(e->msgbuf);
}

/* Sets the error ID and error message of an I/O error using an argument
 * list. Characters of the message that do not fit into the message buffer are
 * silently discarded.
 */
void am_io_error_setv(struct am_io_error* e,
		      enum am_io_error_id error_id,
		      const char* fmt,
		      va_list ap)
{
	va_list aq;

	e->error_id = error_id;

	va_copy(aq, ap);
	vsnprintf(e->msgbuf, e->max_msglen, fmt, aq);
	va_end(aq);
}

/* Sets the error ID and error message of an I/O error in printf-style using a
 * format string and a variable number of arguments. Characters of the message
 * that do not fit into the message buffer are silently discarded.
 */
void am_io_error_set(struct am_io_error* e,
		     enum am_io_error_id error_id,
		     const char* fmt,
		     ...)
{
	va_list ap;

	va_start(ap, fmt);
	am_io_error_setv(e, error_id, fmt, ap);
	va_end(ap);
}

/* Initialize an I/O error stack with a maximum number of errors (max_nesting),
 * each with a maximum message length of max_msglen. Returns 0 on success,
 * otherwise 1.
 */
int am_io_error_stack_init(struct am_io_error_stack* s,
			   size_t max_nesting,
			   size_t max_msglen)
{
	size_t i;
	size_t j;

	s->pos = 0;
	s->max_nesting = max_nesting;
	s->max_msglen = max_msglen;

	if(!(s->errors = malloc(max_nesting * sizeof(s->errors[0]))))
		return 1;

	for(i = 0; i < max_nesting; i++)
		if(am_io_error_init(&s->errors[i], max_msglen))
			goto out_err;

	return 0;

out_err:
	for(j = 0; j < i; j++)
		am_io_error_destroy(&s->errors[j]);

	free(s->errors);

	return 1;
}

/* Initialize an I/O error stack with the default maximum nesting and the
 * default maximum length for error messages. Returns 0 on success, otherwise 1.
 */
int am_io_error_stack_definit(struct am_io_error_stack* s)
{
	return am_io_error_stack_init(s,
				      AM_IO_ERROR_MAX_NESTING,
				      AM_IO_ERROR_MAX_MSGLEN);
}

/* Destroys an I/O error stack */
void am_io_error_stack_destroy(struct am_io_error_stack* s)
{
	for(size_t i = 0; i < s->max_nesting; i++)
		am_io_error_destroy(&s->errors[i]);

	free(s->errors);
}

/* Pushes a new error on an I/O error stack using a printf-style format string
 * and a variable number of arguments. If the maximum nesting for errors is
 * reached, the function fails and returns 1. On success, 0 is returned.
 */
int am_io_error_stack_push(struct am_io_error_stack* s,
			   enum am_io_error_id error_id,
			   const char* fmt,
			   ...)
{
	va_list ap;

	if(s->pos < s->max_nesting) {
		va_start(ap, fmt);
		am_io_error_setv(&s->errors[s->pos], error_id, fmt, ap);
		va_end(ap);

		s->pos++;

		return 0;
	}

	return 1;
}

/* Dumps an I/O error stack to stdout. Errors are displayed from the first error
 * at the bottom of the stack to the last error at the top of the stack.
 */
void am_io_error_stack_dump(struct am_io_error_stack* s)
{
	for(size_t i = 0; i < s->pos; i++)
		printf("#%zu: %s\n", i, s->errors[i].msgbuf);
}
