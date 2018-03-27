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

/* Forces expansion of X and Y and returns a token corresponding to the
 * concatenation of X and Y */
#define AM_MACRO_ARG_EXPAND_JOIN2(X, Y) X ## Y
#define AM_MACRO_ARG_EXPAND_JOIN(X, Y) AM_MACRO_ARG_EXPAND_JOIN2(X, Y)

/* The tests below are very, very ugly. Some code needs to know the number of
 * bits in a size_t. During compilation, the number of bits could be determined
 * by evaluating AM_SIZEOF_BITS(size_t). However, since this involves the sizeof
 * operator, the expression cannot be evaluated during preprocessing. The
 * purpose of the code below is to set AM_SIZE_BITS to a simple preprocessor
 * constant that can be used within #if directives.
 *
 * The code only supports bit widths, which are powers of 2. On some "weird"
 * platform with CHAR_BIT != 8 or if sizeof(size_t) is not a power of two, this
 * might break.
 */
#ifndef AM_SIZE_BITS
  #if SIZE_MAX > UINT64_MAX
    #error "size_t seems to be very big on your system (> 64 bits). "	\
    	   "Cannot determine the size automatically. "			\
    	   "Please set AM_SIZE_BITS accordingly."
  #elif SIZE_MAX == UINT64_MAX
    #define AM_SIZE_BITS 64
  #elif SIZE_MAX == UINT32_MAX
    #define AM_SIZE_BITS 32
  #elif SIZE_MAX == UINT16_MAX
    #define AM_SIZE_BITS 16
  #elif SIZE_MAX == UINT8_MAX
    #define AM_SIZE_BITS 8
  #else
    #warning "SIZE_MAX is not an integer value of the form 2^N - 1, where N "	\
             "is 64, 32, 16 or 8. Setting AM_SIZE_BITS to the smallest "	\
             "number of bits N, such that 2^(N-1)-1 >= SIZE_MAX. "		\
             "This behavior can be overridden by setting AM_SIZE_BITS manually."

    #if SIZE_MAX > UINT32_MAX
      #define AM_SIZE_BITS 64
    #elif SIZE_MAX > UINT16_MAX
      #define AM_SIZE_BITS 32
    #elif SIZE_MAX > UINT8_MAX
      #define AM_SIZE_BITS 16
    #else
      #define AM_SIZE_BITS 8
    #endif
  #endif
#endif

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

char* am_strdupn(const char* s, size_t len);

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

/* Abbreviates a string str with an allipsis (...) at the end if its length is
 * greater than max_len. The modification is done in-place. If the maximum
 * length is not exceeded, the string remains untouched.
 */
static inline void am_abbreviate_string_inplace(char* str, size_t max_len)
{
	size_t len = strlen(str);
	char* pos;
	int i = 0;

	if(len > max_len) {
		if(max_len > 0) {
			pos = &str[max_len];

			while(i++ < 3 && pos-- != str)
				*pos = '.';
		}

		str[max_len] = '\0';
	}
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

/* Skips leading whitespace of a non-zero-terminated string. */
static inline void am_skip_leading_whitespacen(const char** str, size_t* len)
{
	while(isspace(**str) && (*len) > 0) {
		(*str)++;
		(*len)--;
	}
}

/* Skips trailing whitespace of a non-zero-terminated string. */
static inline void am_skip_trailing_whitespacen(const char** str, size_t* len)
{
	while((*len) > 0 && isspace((*str)[(*len)-1]))
		(*len)--;
}

/* Skips both leading and trailing whitespace of a non-zero-terminated
 * string. */
static inline void am_skip_whitespacen(const char** str, size_t* len)
{
	am_skip_leading_whitespacen(str, len);
	am_skip_trailing_whitespacen(str, len);
}

/* Converts the substring of the first len characters of str to a signed 64-bit
 * integer. Returns 0 if the expression is valid, otherwise 1.
 */
static inline int am_atoi64n(const char* str, size_t len, int64_t* out)
{
	int64_t val = 0;
	int sign = 0;

	am_skip_whitespacen(&str, &len);

	if(len == 0)
		return 1;

	if(str[0] == '-') {
		sign = 1;
		str++;
		len--;
	}

	for(size_t i = 0; i < len; i++) {
		if(!isdigit(str[i]))
			return 1;

		val *= 10;
		val += str[i]-'0';
	}

	*out = (sign) ? -val : val;

	return 0;
}

/* Converts the substring of the first len characters of str to an
 * integer. Returns 0 if the expression is valid, otherwise 1.
 */
static inline int am_atou64n(const char* str, size_t len, uint64_t* out)
{
	uint64_t val = 0;

	am_skip_whitespacen(&str, &len);

	if(len == 0)
		return 1;

	for(size_t i = 0; i < len; i++) {
		if(!isdigit(str[i]))
			return 1;

		val *= 10;
		val += str[i]-'0';
	}

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

	am_skip_whitespacen(&str, &len);

	if(len == 0)
		return 1;

	/* If unit is used, there must be at least one digit */
	if(!isdigit(str[len-1]) && len < 2)
		return 1;

	if(!isdigit(str[len-1])) {
		if(am_uint_multiplier(str[len-1], &mult))
			return 1;

		if(am_atou64n(str, len-1, val))
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

	am_skip_whitespacen(&str, &len);

	if(len == 0)
		return 1;

	if(!(buffer = (char*)malloc(len+1)))
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

	am_skip_whitespacen(&str, &len);

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

enum am_ato_safe_status {
	AM_ATO_SAFE_STATUS_VALID = 0,
	AM_ATO_SAFE_STATUS_INVALID = 1,
	AM_ATO_SAFE_STATUS_OVERFLOW = 2,
	AM_ATO_SAFE_STATUS_UNDERFLOW = 3
};

#define AM_DEFINE_ATOU_SAFE_FUN(NBITS)						\
	/* Safely converts a string str into a uint<NBITS>_t. That is, the	\
	 * function detects any malformed input, including an empty string and	\
	 * indicates an overflow if the string contains a value that is higher	\
	 * than the maximum value for a uint<NBITS>_t. If the string is	\
	 * well-formed and does not lead to an overflow, its integer value is	\
	 * returned in *out. */						\
	static inline enum am_ato_safe_status					\
	am_atou##NBITS##_safe(const char* str, uint##NBITS##_t* out)		\
	{									\
		static const uint##NBITS##_t val_last_shift_ok =		\
			UINT##NBITS##_MAX / 10;				\
		const char* curr = str;					\
		uint##NBITS##_t val = 0;					\
		uint##NBITS##_t digit_val;					\
		char digit;							\
										\
		if(str[0] == '\0')						\
			return AM_ATO_SAFE_STATUS_INVALID;			\
										\
		while(*curr) {							\
			digit = *curr;						\
										\
			if(!isdigit(digit))					\
				return AM_ATO_SAFE_STATUS_INVALID;		\
										\
			digit_val = digit - '0';				\
										\
			if(val > val_last_shift_ok)				\
				return AM_ATO_SAFE_STATUS_OVERFLOW;		\
										\
			val *= 10;						\
										\
			if(UINT##NBITS##_MAX - val < digit_val)		\
				return AM_ATO_SAFE_STATUS_OVERFLOW;		\
										\
			val += digit_val;					\
										\
			curr++;						\
		}								\
										\
		*out = val;							\
										\
		return AM_ATO_SAFE_STATUS_VALID;				\
	}

AM_DEFINE_ATOU_SAFE_FUN(8)
AM_DEFINE_ATOU_SAFE_FUN(16)
AM_DEFINE_ATOU_SAFE_FUN(32)
AM_DEFINE_ATOU_SAFE_FUN(64)

#define AM_DEFINE_ATOI_SAFE_FUN(NBITS)		\
	/* Safely converts a string str into an int<NBITS>_t. That is, the	\
	 * function detects any malformed input, including an empty string and	\
	 * indicates an overflow if the string contains a value that is higher	\
	 * than the maximum value for a int<NBITS>_t. If the string is		\
	 * well-formed and does not lead to an overflow, its integer value is	\
	 * returned in *out. */						\
	static inline enum am_ato_safe_status					\
	am_atoi##NBITS##_safe(const char* str, int##NBITS##_t* out)		\
	{									\
		uint##NBITS##_t uval;						\
		int sign = 0;							\
		enum am_ato_safe_status ret;					\
										\
		if(str[0] == '\0')						\
			return AM_ATO_SAFE_STATUS_INVALID;			\
										\
		if(str[0] == '-') {						\
			str++;							\
			sign = 1;						\
		}								\
										\
		if((ret = am_atou##NBITS##_safe(str, &uval)) !=		\
		   AM_ATO_SAFE_STATUS_VALID)					\
		{								\
			return ret;						\
		}								\
										\
		if(!sign) {							\
			if(uval >= INT##NBITS##_MAX)				\
				return AM_ATO_SAFE_STATUS_OVERFLOW;		\
										\
			*out = (int##NBITS##_t)uval;				\
		} else {							\
			if(uval > ((uint##NBITS##_t)INT##NBITS##_MAX) + 1)	\
				return AM_ATO_SAFE_STATUS_UNDERFLOW;		\
										\
			if(uval == ((uint##NBITS##_t)INT##NBITS##_MAX) + 1) {	\
				*out = -INT##NBITS##_MAX;			\
				*out -= 1;					\
			} else {						\
				*out = uval;					\
				*out *= -1;					\
			}							\
		}								\
										\
		return AM_ATO_SAFE_STATUS_VALID;				\
	}

AM_DEFINE_ATOI_SAFE_FUN(8)
AM_DEFINE_ATOI_SAFE_FUN(16)
AM_DEFINE_ATOI_SAFE_FUN(32)
AM_DEFINE_ATOI_SAFE_FUN(64)

#define AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN(T, SUFFIX, MAX)		\
	/* Safely converts a size_t to a T, i.e., performs a range check prior	\
	 * to the conversion. Returns 0 on success, otherwise 1. */		\
	static inline int							\
	am_safe_##SUFFIX##_from_size(T* out, size_t v)				\
	{									\
		if(sizeof(T) >= sizeof(size_t)) {				\
			if((T)v > MAX)						\
				return 1;					\
		}								\
										\
		*out = (T)v;							\
		return 0;							\
	}

AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN (uint8_t,  u8,  UINT8_MAX)
AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN(uint16_t, u16, UINT16_MAX)
AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN(uint32_t, u32, UINT32_MAX)
AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN(uint64_t, u64, UINT64_MAX)

AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN( int8_t,  i8,  INT8_MAX)
AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN(int16_t, i16, INT16_MAX)
AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN(int32_t, i32, INT32_MAX)
AM_DEFINE_CONVERT_SAFE_SIZE_TO_INT_FUN(int64_t, i64, INT64_MAX)

#define AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(T, SUFFIX, SIGNED)		\
	/* Safely converts a T to a size_t, i.e., performs a range check prior	\
	 * to the conversion. Returns 0 on success, otherwise 1. */		\
	static inline int							\
	am_safe_size_from_##SUFFIX(size_t* out, T v)				\
	{									\
		if(sizeof(size_t) >= sizeof(T)) {				\
			if(SIGNED) {						\
				if(v < 0)					\
					return 1;				\
			}							\
		} else {							\
			if(v > (T)SIZE_MAX)					\
				return 1;					\
		}								\
										\
		*out = (size_t)v;						\
		return 0;							\
	}

AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(uint8_t, u8, 0)
AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(uint16_t, u16, 0)
AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(uint32_t, u32, 0)
AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(uint64_t, u64, 0)

AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(int8_t, i8, 1)
AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(int16_t, i16, 1)
AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(int32_t, i32, 1)
AM_DEFINE_CONVERT_SAFE_INT_TO_SIZE_FUN(int64_t, i64, 1)

#define AM_DEFINE_CONVERT_SAFE_UINT64_TO_U(BITS)				\
	/* Safely converts a uint64_t to a uint##BITS##_t, i.e., performs a	\
	 * range check prior to the conversion. Returns 0 on success,		\
	 * otherwise 1. */							\
	static inline int							\
	am_safe_u##BITS##_from_u64(uint##BITS##_t* out, uint64_t v)		\
	{									\
		if(v > (uint64_t)UINT##BITS##_MAX)				\
			return 1;						\
										\
		*out = (uint##BITS##_t)v;					\
										\
		return 0;							\
	}

AM_DEFINE_CONVERT_SAFE_UINT64_TO_U(8)
AM_DEFINE_CONVERT_SAFE_UINT64_TO_U(16)
AM_DEFINE_CONVERT_SAFE_UINT64_TO_U(32)
AM_DEFINE_CONVERT_SAFE_UINT64_TO_U(64)

#endif
