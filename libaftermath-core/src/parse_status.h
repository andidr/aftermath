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

#ifndef AM_PARSE_STATUS_H
#define AM_PARSE_STATUS_H

#include <stdarg.h>

enum am_parse_result {
	AM_PARSE_RESULT_SUCCESS = 0,
	AM_PARSE_RESULT_ERROR = 1
};

#define AM_PARSE_STATUS_MAX_ERROR_LEN 128
#define AM_PARSE_STATUS_MAX_PATH_LEN 128

/* Parser status providing context when parsing fails */
struct am_parse_status {
	/* Parsing result; errmsg, line and character are only valid if
	 * result == AM_PARSE_RESULT_ERROR */
	enum am_parse_result result;

	/* Human-redable error message */
	char errmsg[AM_PARSE_STATUS_MAX_ERROR_LEN];

	/* Filename or other identifier for the source with the object notation
	 * string being parsed */
	char source_name[AM_PARSE_STATUS_MAX_PATH_LEN];

	/* 1-based line index at which the error occured */
	unsigned int line;

	/* O-based character index of the erroneous line */
	unsigned int character;

	/* Address of first character to be parsed */
	const char* base;
};

void am_parse_status_init(struct am_parse_status* status,
			  const char* source_name,
			  const char* base);

void am_parse_status_update_location(struct am_parse_status* s,
				     const char* loc);

void am_parse_status_set_error(struct am_parse_status* s,
			       const char* loc,
			       const char* fmt,
			       ...);

void am_parse_status_set_errorv(struct am_parse_status* s,
				const char* loc,
				const char* fmt,
				va_list vl);

/* Same as am_parse_status_set_error, but becomes a no-op if s is NULL. */
static inline void am_parse_status_set_error_nn(struct am_parse_status* s,
						const char* loc,
						const char* fmt,
						...)
{
	va_list vl;

	if(s) {
		va_start(vl, fmt);
		am_parse_status_set_errorv(s, loc, fmt, vl);
		va_end(vl);
	}
}

#endif
