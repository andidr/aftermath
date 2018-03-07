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

#include "interval.h"
#include <aftermath/core/ansi_extras.h>
#include <stdio.h>

#if AM_TIMESTAMP_T_BITS != 64 || AM_TIMESTAMP_T_SIGNED != 0
#error "Assuming am_timestamp_t to be an unsigned 64 bit value, but it isn't"
#endif

int am_dfg_type_interval_to_string(const struct am_dfg_type* t,
				   void* ptr,
				   char** out,
				   int* cst)
{
	char* ret;
	struct am_interval* i = ptr;

	if(!(ret = malloc(2*AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 5)))
		return 1;

	snprintf(ret, 2 * AM_TIMESTAMP_T_MAX_DECIMAL_DIGITS + 4,
		 "[%" AM_TIMESTAMP_T_FMT ", %" AM_TIMESTAMP_T_FMT "]",
		 i->start, i->end);

	*out = ret;
	*cst = 0;

	return 0;
}

/* Parses a string of the form [<start: timestamp>, <end: timestamp>] and
 * returns the address of the first character of the start string, the first
 * character of the end string and the lengths in characters of the start and
 * end string in *pstart, *pend, *pstart_len, and *pend_len, respectively. The
 * return value indicates whether the string is well-formed. A return value of 0
 * indicates a success, while a return value of 1 indicates an error. */
static inline int find_pos(const char* str,
			   const char** pstart,
			   size_t* pstart_len,
			   const char** pend,
			   size_t* pend_len)
{
	const char* tmp;
	const char* start;
	const char* end;
	size_t start_len = 0;
	size_t end_len = 0;

	tmp = str;

	while(isspace(*tmp))
		tmp++;

	if(*tmp != '[')
		return 1;

	start = tmp;
	tmp++;

	while(*tmp && *tmp != ',') {
		tmp++;
		start_len++;
	}

	if(*tmp != ',')
		return 1;

	while(isspace(*tmp))
		tmp++;

	end = tmp;

	while(*tmp && *tmp != ']') {
		tmp++;
		end_len++;
	}

	if(*tmp != ']')
		return 1;

	while(isspace(*tmp))
		tmp++;

	if(*tmp != '\0')
		return 1;

	*pstart = start;
	*pend = end;
	*pstart_len = start_len;
	*pend_len = end_len;

	return 0;
}

/* Tries to parse a string of the form [<start: timestamp>, <end: timestamp>]
 * and assings *start and *end the start and end timestamps. Returns 0 on
 * success, otherwise 1. */
static inline int from_string_tmp(const char* str,
				  am_timestamp_t* start,
				  am_timestamp_t* end)
{
	const char* start_str;
	const char* end_str;
	size_t start_len;
	size_t end_len;
	am_timestamp_t start_tmp;
	am_timestamp_t end_tmp;

	if(find_pos(str, &start_str, &start_len, &end_str, &end_len))
		return 1;

	if(am_atou64n_unit(start_str, start_len, &start_tmp))
		return 1;

	if(am_atou64n_unit(end_str, end_len, &end_tmp))
		return 1;

	*start = start_tmp;
	*end = end_tmp;

	return 0;
}

int am_dfg_type_interval_from_string(const struct am_dfg_type* t,
				     const char* str,
				     void* out)
{
	struct am_interval* i = out;

	return from_string_tmp(str, &i->start, &i->end);
}

int am_dfg_type_interval_check_string(const struct am_dfg_type* t,
				      const char* str)
{
	am_timestamp_t start;
	am_timestamp_t end;

	return from_string_tmp(str, &start, &end);
}
