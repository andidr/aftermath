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

struct replacement {
	char c;
	const char* r;
};

static const struct replacement escape_repl[] = {
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

char* escape_string(const char* str)
{
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

		for(const struct replacement* r = &escape_repl[0]; r->r; r++) {
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

int unescape_string_in_place(char** sstr, int shrink)
{
	char* str = *sstr;
	int in_escape = 0;
	const struct replacement* r;
	size_t len = strlen(str);
	size_t nshrink = 0;
	char* tmp;
	unsigned int c;
	int replaced;
	int num_digits;

	for(size_t i = 0; i < len; i++) {
		if(in_escape) {
			if(str[i] == 'x') {
				/* Hex escape sequence with 2 digits
				 * or 1 digit */
				if(len-i < 2 || !isxdigit(str[i+1]))
					return 1;

				if(len-i > 2 && isxdigit(str[i+2])) {
					num_digits = 2;

					if(sscanf(&str[i+1], "%02X", &c) != 1)
						return 1;
				} else {
					num_digits = 1;

					if(sscanf(&str[i+1], "%1X", &c) != 1)
						return 1;
				}

				str[i-1] = (char)c;
				memmove(&str[i], &str[i+1+num_digits], len-i-1-num_digits);
				in_escape = 0;
				len -= 1+num_digits;
				nshrink += 1+num_digits;
				i--;
			} else if(isodigit(str[i])) {
				/* Octal escape sequence with 3, 2 or
				 * 1 digit */
				if(len-i > 2 &&
				   isodigit(str[i+1]) &&
				   isodigit(str[i+2]))
				{
					num_digits = 3;

					if(sscanf(&str[i], "%03o", &c) != 1)
						return 1;
				} else if(len-i > 1 && isodigit(str[i+1])) {
					num_digits = 2;

					if(sscanf(&str[i], "%02o", &c) != 1)
						return 1;
				} else {
					num_digits = 1;

					if(sscanf(&str[i], "%1o", &c) != 1)
						return 1;
				}

				str[i-1] = (char)c;
				memmove(&str[i], &str[i+num_digits], len-i-num_digits+1);
				in_escape = 0;
				len -= num_digits;
				nshrink += num_digits;
				i--;
			} else {
				replaced = 0;

				/* One of the escape sequences? */
				for(r = &escape_repl[0]; r->r; r++) {
					if(str[i] == r->r[1]) {
						str[i-1] = r->c;
						memmove(&str[i], &str[i+1], len-i-1);
						in_escape = 0;
						len--;
						nshrink++;
						i--;
						replaced = 1;
						break;
					}
				}

				if(!replaced)
					return 1;
			}
		} else {
			if(str[i] == '\\')
				in_escape = 1;
		}
	}

	if(nshrink) {
		str[len] = '\0';

		if(shrink) {
			if(!(tmp = realloc(str, len+1)))
				return 1;

			*sstr = tmp;
		}
	}

	return 0;
}

char* unescape_string(const char* str)
{
	char* tmp = strdup(str);

	if(!tmp)
		return NULL;

	if(unescape_string_in_place(&tmp, 1)) {
		free(tmp);
		return NULL;
	}

	return tmp;
}
