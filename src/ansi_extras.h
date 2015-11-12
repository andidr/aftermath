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

#if !HAVE_STRDUP
static inline char* strdup(const char* s)
{
	char* ret = malloc(strlen(s)+1);

	if(ret)
		strcpy(ret, s);

	return ret;
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
int unescape_string_in_place(char** sstr, int shrink);

#endif
