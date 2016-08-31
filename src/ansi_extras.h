/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
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

#ifndef ANSI_EXTRAS_H
#define ANSI_EXTRAS_H

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <regex.h>
#include <alloca.h>
#include <stdarg.h>

/* Very simple string replacement function.
 * haystack must contain enough space for all
 * replacements*/
static inline void strreplace(char* haystack, const char* needle, const char* replacement)
{
	char* pos;
	int needle_len = strlen(needle);
	int replacement_len = strlen(replacement);

	while((pos = strstr(haystack, needle))) {
		memmove(pos+replacement_len, pos+needle_len, strlen(pos+needle_len)+1);
		memcpy(pos, replacement, replacement_len);
		haystack = pos+replacement_len;
	}
}

static inline off_t file_size(const char* filename)
{
	struct stat stat_buf;

	if(stat(filename, &stat_buf) == -1)
		return -1;

	return stat_buf.st_size;
}

static inline char* strdupn(const char* s, size_t len)
{
	char* ret = malloc(len+1);

	if(ret)
		strncpy(ret, s, len);

	ret[len] = '\0';

	return ret;
}

static inline void print_short(char* buf, size_t max_len, const char* src)
{
	size_t rem_len = max_len - 3 - 1;
	size_t suff_len = rem_len/2;
	size_t pref_len = rem_len-suff_len;

	if(max_len < 4 || strlen(src) < max_len) {
		strncpy(buf, src, max_len);
		return;
	}

	if(max_len < 6) {
		snprintf(buf, max_len, "%.*s...", (int)(max_len-3), src);
		return;
	}

	snprintf(buf, max_len, "%.*s...%.*s", (int)pref_len, src, (int)suff_len, &src[strlen(src)-suff_len]);
}

static inline int isodigit(char c)
{
	return (c >= '0' && c <= '7');
}

char* escape_string(const char* str);
char* unescape_string(const char* str);
char* unescape_stringn(const char* str, size_t len);
int unescape_string_in_place(char** sstr, int shrink);

static inline int xdigit_val(char c)
{
	if(isdigit(c))
		return c-'0';

	if(c >= 'a' && c <= 'f')
		return c-'a'+10;

	if(c >= 'A' && c <= 'F')
		return c-'A'+10;

	return -1;
}

/* Checks whether the expression given in sregex is a valid POSIX
 * extended regular expression */
static inline int is_valid_regex(const char* sregex)
{
	regex_t regex;

	if(regcomp(&regex, sregex, REG_EXTENDED | REG_NOSUB))
		return 0;

	regfree(&regex);

	return 1;
}

/* Converts the substring of the first len characters of str to an
 * integer. Returns 0 if the expression is valid, otherwise 1.
 */
static inline int atou64n(const char* str, size_t len, uint64_t* out)
{
	int digit_found = 0;
	int ws_after_digit = 0;

	uint64_t val = 0;

	for(size_t i = 0; i < len; i++) {
		val *= 10;

		if(isdigit(str[i])) {
			/* Only leading and / or trailing whitespace is allowed,
			 * but no whitespace in between digits */
			if(ws_after_digit)
				return 1;

			val += str[i]-'0';
			digit_found = 1;
		} else if(isspace(str[i])) {
			if(digit_found)
				ws_after_digit = 1;
		}
	}

	if(!digit_found)
		return 1;

	*out = val;

	return 0;
}

/* Returns the multiplier for a unit prefix, e.g., 1000 for K, 1000000
 * for M and so on. */
static inline int uint_multiplier(char unit, uint64_t* val)
{
	switch(unit) {
		case 'K': *val = (uint64_t)1000; break;
		case 'M': *val = (uint64_t)1000000; break;
		case 'G': *val = (uint64_t)1000000000; break;
		case 'T': *val = (uint64_t)1000000000000; break;
		case 'P': *val = (uint64_t)1000000000000000; break;
		default:
			return 1;
	}

	return 0;
}

/* Parses the first len characters of str for an unsigned integer with
 * an optional unit prefix (K, M, G, T, P). The value is returned in
 * val. If the characters do not form a valid expression the function
 * returns 1, otherwise 0. */
static inline int atou64n_unit(const char* str, size_t len, uint64_t* val)
{
	uint64_t mult;
	size_t i;

	if(len == 0)
		return 1;

	/* If unit is used, there must be at least one digit */
	if(!isdigit(str[len-1]) && len < 2)
		return 1;

	if(!isdigit(str[len-1])) {
		if(uint_multiplier(str[len-1], &mult))
			return 1;

		i = len-2;

		/* Skip whitespace between unit and number */
		while(isspace(str[i]) && i > 0)
			i--;

		if(atou64n(str, i+1, val))
			return 1;

		*val *= mult;
	} else {
		return atou64n(str, len, val);
	}

	return 0;
}

/* Parses the first len characters of str for an double
 * expression. The value is returned in val. If the characters do not
 * form a valid expression the function returns 1, otherwise 0. */
static inline int atodbln(const char* str, size_t len, double* val)
{
	char* buffer;

	if(!(buffer = alloca(len+1)))
		return 1;

	memcpy(buffer, str, len);
	buffer[len] = '\0';

	*val = strtod(buffer, NULL);

	return 0;
}

/* Parses the first len characters of str for an double expression
 * with an optional unit prefix (K, M, G, T, P). The value is returned
 * in val. If the characters do not form a valid expression the
 * function returns 1, otherwise 0. */
static inline int atodbln_unit(const char* str, size_t len, double* val)
{
	uint64_t mult;
	size_t i;

	if(len == 0)
		return 1;

	/* If unit is used, there must be at least one digit */
	if(!isdigit(str[len-1]) && len < 2)
		return 1;

	if(!isdigit(str[len-1])) {
		if(uint_multiplier(str[len-1], &mult))
			return 1;

		i = len-2;

		/* Skip whitespace between unit and number */
		while(isspace(str[i]) && i > 0)
			i--;

		if(i == 0)
			return 1;

		if(atodbln(str, i+1, val))
			return 1;

		*val *= (double)mult;
	} else {
		if(atodbln(str, len, val))
			return 1;
	}

	return 0;
}

/* Compares the first len_a characters of a to the first len_b
 * characters of b. Returns 1 if both sequences have the same length
 * and if the characters are identical, otherwise 0. */
static inline int strnneq(const char* a, size_t len_a, const char* b, size_t len_b)
{
	if(len_a != len_b)
		return 0;

	if(strncmp(a, b, len_a) == 0)
		return 1;

	return 0;
}

/* Compares the first len_a characters of a to zero-terminated string
 * b. Returns 1 if both sequences have the same length and if the
 * characters are identical, otherwise 0. */
static inline int strn1eq(const char* a, size_t len_a, const char* b)
{
	return strnneq(a, len_a, b, strlen(b));
}

/* Writes the prefix nprefix times to fp before writing the string str
 * to fp. */
static inline int fputs_prefix(const char* str, const char* prefix, int nprefix,
			       FILE* fp)
{
	for(int i = 0; i < nprefix; i++)
		if(fputs(prefix, fp) < 0)
			return 1;

	if(fputs(str, fp) < 0)
		return 1;

	return 0;
}

/* Writes the prefix nprefix times to fp before writing the string str to fp
 * generated by the format and arguments in frprintf-style. Returns the number
 * of characters printed or a negative value if an error occurred.
 */
static inline int fprintf_prefix(FILE* fp, const char* prefix, int nprefix,
				 const char *format, ...)
{
	int nchar = 0;
	int ret;
	va_list ap;

	for(int i = 0; i < nprefix; i++) {
		if((ret = fputs(prefix, fp)) < 0)
			return ret;

		nchar += ret;
	}

	va_start(ap, format);
	ret = vfprintf(fp, format, ap);
	va_end(ap);

	if(ret < 0)
		return ret;
	else
		nchar += ret;

	return nchar;
}

#endif
