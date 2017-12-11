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

#ifndef AM_ANSI_EXTRAS_H
#define AM_ANSI_EXTRAS_H

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <regex.h>
#include <stdarg.h>
#include <limits.h>

#define AM_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Retrieves the offset of an array field from a pointer to an instance of the
 * containing struct */
#define AM_OFFSETOF_PTR(ptr, field) offsetof(typeof(*(ptr)), field)

/* Returns the size of a in bits */
#define AM_SIZEOF_BITS(a) ((sizeof(a) * CHAR_BIT))

/* Protects macro arguments from evaluation */
#define AM_MACRO_ARG_PROTECT(...) __VA_ARGS__

/* Very simple string replacement function. Haystack must contain enough space
 * for all replacements*/
static inline void
am_strreplace(char* haystack, const char* needle, const char* replacement)
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

/* Returns the size of a file. If the file cannot be found the function returns
 * -1, otherwise the size of the file. */
static inline off_t am_file_size(const char* filename)
{
	struct stat stat_buf;

	if(stat(filename, &stat_buf) == -1)
		return -1;

	return stat_buf.st_size;
}

/* Duplicates the first len bytes of a string s into a new zero-terminates
 * string. Upon failure, NULL is returned. */
static inline char* am_strdupn(const char* s, size_t len)
{
	char* ret = malloc(len+1);

	if(ret) {
		strncpy(ret, s, len);
		ret[len] = '\0';
	}

	return ret;
}

/* Prints the contents of a buffer src into a target buffer buf of size max_len
 * using an ellipsis if the target buffer is not large enough to hold all
 * characters of the source. The ellipsis is placed in the middle of the
 * string. */
static inline void am_print_short(char* buf, size_t max_len, const char* src)
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

/* Returns true if the character c is an octal digit. */
static inline int isodigit(char c)
{
	return (c >= '0' && c <= '7');
}

char* am_escape_string(const char* str);
char* am_unescape_string(const char* str);
char* am_unescape_stringn(const char* str, size_t len);
int am_unescape_string_in_place(char** sstr, int shrink);

/* Returns the value of a hexadecimal digit c (can be upper-case or lower-case).
 * If c is not a hexadecimal digit the function returns -1. */
static inline int am_xdigit_val(char c)
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
static inline int am_is_valid_regex(const char* sregex)
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
static inline int am_atou64n(const char* str, size_t len, uint64_t* out)
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
static inline int am_uint_multiplier(char unit, uint64_t* val)
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
static inline int am_atou64n_unit(const char* str, size_t len, uint64_t* val)
{
	uint64_t mult;
	size_t i;

	if(len == 0)
		return 1;

	/* If unit is used, there must be at least one digit */
	if(!isdigit(str[len-1]) && len < 2)
		return 1;

	if(!isdigit(str[len-1])) {
		if(am_uint_multiplier(str[len-1], &mult))
			return 1;

		i = len-2;

		/* Skip whitespace between unit and number */
		while(isspace(str[i]) && i > 0)
			i--;

		if(am_atou64n(str, i+1, val))
			return 1;

		*val *= mult;
	} else {
		return am_atou64n(str, len, val);
	}

	return 0;
}

/* Parses the first len characters of str for an double
 * expression. The value is returned in val. If the characters do not
 * form a valid expression the function returns 1, otherwise 0. */
static inline int am_atodbln(const char* str, size_t len, double* val)
{
	char* buffer;

	if(!(buffer = malloc(len+1)))
		return 1;

	memcpy(buffer, str, len);
	buffer[len] = '\0';

	*val = strtod(buffer, NULL);
	free(buffer);

	return 0;
}

/* Parses the first len characters of str for an double expression
 * with an optional unit prefix (K, M, G, T, P). The value is returned
 * in val. If the characters do not form a valid expression the
 * function returns 1, otherwise 0. */
static inline int am_atodbln_unit(const char* str, size_t len, double* val)
{
	uint64_t mult;
	size_t i;

	if(len == 0)
		return 1;

	/* If unit is used, there must be at least one digit */
	if(!isdigit(str[len-1]) && len < 2)
		return 1;

	if(!isdigit(str[len-1])) {
		if(am_uint_multiplier(str[len-1], &mult))
			return 1;

		i = len-2;

		/* Skip whitespace between unit and number */
		while(isspace(str[i]) && i > 0)
			i--;

		if(i == 0 && !isdigit(str[i]))
			return 1;

		if(am_atodbln(str, i+1, val))
			return 1;

		*val *= (double)mult;
	} else {
		if(am_atodbln(str, len, val))
			return 1;
	}

	return 0;
}

/* Compares the first len_a characters of a to the first len_b
 * characters of b. Returns 1 if both sequences have the same length
 * and if the characters are identical, otherwise 0. */
static inline int
am_strnneq(const char* a, size_t len_a, const char* b, size_t len_b)
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
static inline int am_strn1eq(const char* a, size_t len_a, const char* b)
{
	return am_strnneq(a, len_a, b, strlen(b));
}

/* Writes the prefix nprefix times to fp before writing the string str
 * to fp. */
static inline int
am_fputs_prefix(const char* str, const char* prefix, int nprefix, FILE* fp)
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
static inline int am_fprintf_prefix(FILE* fp, const char* prefix, int nprefix,
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

int am_siformat_u64(uint64_t value, size_t max_sigd, char* buf, size_t max_len);

/* Returns the maximum of a and b. */
static inline double am_max_double(double a, double b)
{
	return a > b ? a : b;
}

/* Returns the minimum of a and b. */
static inline double am_min_double(double a, double b)
{
	return a < b ? a : b;
}

/* Assigns the value of an unsigned integer of src_bits bits to an unsigned
 * integer of dst_bits bits using their respective pointers. The number of bits
 * must either be 64, 32, 16 or 8. */
static inline void
am_assign_uint(void* dst, unsigned int dst_bits,
	       const void* src, unsigned int src_bits)
{
	if(dst_bits == 64) {
		if(src_bits == 64)      *((uint64_t*)dst) = *((uint64_t*)src);
		else if(src_bits == 32) *((uint64_t*)dst) = *((uint32_t*)src);
		else if(src_bits == 16) *((uint64_t*)dst) = *((uint16_t*)src);
		else if(src_bits == 8)  *((uint64_t*)dst) = *((uint8_t*)src);
	} else if(dst_bits == 32) {
		if(src_bits == 64)      *((uint32_t*)dst) = *((uint64_t*)src);
		else if(src_bits == 32) *((uint32_t*)dst) = *((uint32_t*)src);
		else if(src_bits == 16) *((uint32_t*)dst) = *((uint16_t*)src);
		else if(src_bits == 8)  *((uint32_t*)dst) = *((uint8_t*)src);
	} else if(dst_bits == 16) {
		if(src_bits == 64)      *((uint16_t*)dst) = *((uint64_t*)src);
		else if(src_bits == 32) *((uint16_t*)dst) = *((uint32_t*)src);
		else if(src_bits == 16) *((uint16_t*)dst) = *((uint16_t*)src);
		else if(src_bits == 8)  *((uint16_t*)dst) = *((uint8_t*)src);
	} else if(dst_bits == 8) {
		if(src_bits == 64)      *((uint8_t*)dst) = *((uint64_t*)src);
		else if(src_bits == 32) *((uint8_t*)dst) = *((uint32_t*)src);
		else if(src_bits == 16) *((uint8_t*)dst) = *((uint16_t*)src);
		else if(src_bits == 8)  *((uint8_t*)dst) = *((uint8_t*)src);
	}
}

#endif
