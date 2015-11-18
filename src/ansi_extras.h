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

#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <regex.h>

#define SWAP_BITS(val, ret, type)					\
	do {								\
		ret = 0;						\
		for(unsigned int i = 0; i < 8*sizeof(type); i += 8)	\
			ret |= ((val >> i) & 0xFF) << ((sizeof(type)*8-8) - i); \
	} while(0)

static inline int64_t int64_swap(int64_t val)
{
	int64_t ret;
	SWAP_BITS(val, ret, int64_t);

	return ret;
}

static inline int32_t int32_swap(int32_t val)
{
	int32_t ret;
	SWAP_BITS(val, ret, int32_t);

	return ret;
}

static inline int16_t int16_swap(int16_t val)
{
	int16_t ret;
	SWAP_BITS(val, ret, int16_t);

	return ret;
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
	#define int16_htole(val) val
	#define int32_htole(val) val
	#define int64_htole(val) val
	#define int16_letoh(val) val
	#define int32_letoh(val) val
	#define int64_letoh(val) val
#elif __BYTE_ORDER == __BIG_ENDIAN
	#define int16_htole(val) int16_swap(val)
	#define int32_htole(val) int32_swap(val)
	#define int64_htole(val) int64_swap(val)
	#define int16_letoh(val) int16_swap(val)
	#define int32_letoh(val) int32_swap(val)
	#define int64_letoh(val) int64_swap(val)
#else
	#error "Could not determine your system's endianness"
#endif

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

#if !HAVE_STRDUP
static inline char* strdup(const char* s)
{
	return strdupn(s, strlen(s));
}
#endif

static inline void print_short(char* buf, int max_len, const char* src)
{
	int rem_len = max_len - 3 - 1;
	int suff_len = rem_len/2;
	int pref_len = rem_len-suff_len;

	if(max_len < 4 || strlen(src) < max_len) {
		strncpy(buf, src, max_len);
		return;
	}

	if(max_len < 6) {
		snprintf(buf, max_len, "%.*s...", max_len-3, src);
		return;
	}

	snprintf(buf, max_len, "%.*s...%.*s", pref_len, src, suff_len, &src[strlen(src)-suff_len]);
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
 * integer */
static inline uint64_t atou64n(const char* str, size_t len)
{
	uint64_t val = 0;

	for(size_t i = 0; i < len; i++) {
		val *= 10;
		val += str[i]-'0';
	}

	return val;
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

	if(!isdigit(str[len-1])) {
		if(uint_multiplier(str[len-1], &mult))
			return 1;

		i = len-2;

		/* Skip whitespace between unit and number */
		while(isspace(str[i]) && i >= 0)
			i--;

		*val = atou64n(str, i+1);
		*val *= mult;
	} else {
		*val = atou64n(str, len);
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

#endif
