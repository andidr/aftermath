/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
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

#include "ansi_extras.h"

static inline int realloc_str(char** str, size_t new_len)
{
	char* tmp;

	if(!(tmp = realloc(*str, new_len)))
		return 1;

	*str = tmp;

	return 0;
}

static inline int append_string(char** dst, const char* str, size_t* dst_slen, size_t* dst_size, size_t prealloc, size_t str_slen)
{
	if(*dst_size < *dst_slen+str_slen) {
		if(realloc_str(dst, *dst_slen+prealloc))
			return 1;

		*dst_size += prealloc;
	}

	strncpy(&((*dst)[*dst_slen]), str, str_slen);
	*dst_slen += str_slen;

	return 0;
}

char* escape_string(const char* str)
{
	struct replacement {
		char c;
		const char* r;
	};

	static const struct replacement repl[] = {
		{ .c = '\a', .r = "\\a" },
		{ .c = '\b', .r = "\\b" },
		{ .c = '\f', .r = "\\f" },
		{ .c = '\n', .r = "\\n" },
		{ .c = '\r', .r = "\\r" },
		{ .c = '\t', .r = "\\t" },
		{ .c = '\v', .r = "\\v" },
		{ .c = '\\', .r = "\\\\" },
		{ .c = '\"', .r = "\\\"" },
		{ 0x00, NULL }
	};

	static const size_t prealloc = 32;

	size_t len = strlen(str);
	char* curr = NULL;
	char* tmp;
	size_t outlen = len+1;
	size_t outpos = 0;
	int replaced;
	char hexrepl[5];

	if(!(curr = malloc(len+1)))
		return NULL;

	for(size_t i = 0; i < len+1; i++) {
		replaced = 0;

		for(const struct replacement* r = &repl[0]; r->r; r++) {
			if(str[i] == r->c) {
				if(append_string(&curr, r->r, &outpos, &outlen, prealloc, 2))
					goto out_err;

				replaced = 1;
				break;
			}
		}

		if(!replaced) {
			if(isprint(str[i]) || str[i] == '\0') {
				if(append_string(&curr, &str[i], &outpos, &outlen, prealloc, 1))
					goto out_err;
			} else {
				snprintf(hexrepl, 5, "\\x%02X", (int)str[i]);

				if(append_string(&curr, hexrepl, &outpos, &outlen, prealloc, 4))
					goto out_err;

			}
		}

		if(str[i] == '\0')
			break;
	}

	if(outlen > outpos) {
		if(!(tmp = realloc(curr, outlen)))
			goto out_err;

		curr = tmp;
	}

	return curr;
out_err:
	free(curr);
	return NULL;
}
