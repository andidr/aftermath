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

#include <aftermath/core/parse_status.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void am_parse_status_init(struct am_parse_status* s,
			  const char* source_name,
			  const char* base)
{
	size_t max_len;

	s->result = AM_PARSE_RESULT_SUCCESS;
	s->errmsg[0] = '\0';
	s->line = 0;
	s->character = 0;
	s->base = base;

	max_len = sizeof(s->source_name) - 1;
	strncpy(s->source_name, source_name, max_len);
	s->source_name[max_len] = '\0';
}

/* Sets the fields "line" and "character" of a status for the location "loc"
 * based on the previously set base.
 */
void am_parse_status_update_location(struct am_parse_status* s,
				     const char* loc)
{
	s->line = 1;
	s->character = 0;

	for(const char* c = s->base; c != loc; c++) {
		if(*c == '\n') {
			s->line++;
			s->character = 0;
		} else {
			s->character++;
		}
	}
}

/* Sets the error message of a parse status. The parameter loc is the location
 * at which the error occured. The line and character of the status are updated
 * for loc, based on the previously set base of the status. Fmt is a printf
 * formatting string and vl is a list of arguments passed through to the string
 * formatting routine .
 */
void am_parse_status_set_errorv(struct am_parse_status* s,
				const char* loc,
				const char* fmt,
				va_list vl)
{
	va_list vlc;

	va_copy(vlc, vl);

	s->result = AM_PARSE_RESULT_ERROR;
	am_parse_status_update_location(s, loc);
	vsnprintf(s->errmsg, AM_PARSE_STATUS_MAX_PATH_LEN-1, fmt, vlc);
	s->errmsg[AM_PARSE_STATUS_MAX_PATH_LEN-1] = '\0';

	va_end(vlc);
}

/* Same as am_parse_status_set_errorv, but takes a variable number of parameters
 * instead of a parameter list. */
void am_parse_status_set_error(struct am_parse_status* s,
			       const char* loc,
			       const char* fmt,
			       ...)
{
	va_list vl;

	va_start(vl, fmt);
	am_parse_status_set_errorv(s, loc, fmt, vl);
	va_end(vl);
}
