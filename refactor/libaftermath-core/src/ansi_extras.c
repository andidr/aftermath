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

#include <aftermath/core/ansi_extras.h>

/* Resize a string to a size of new_len bytes. The new address of the
 * reallocated string is returned in *str. Returns 0 on success, otherwise 1. */
static inline int am_realloc_str(char** str, size_t new_len)
{
	char* tmp;

	if(!(tmp = realloc(*str, new_len)))
		return 1;

	*str = tmp;

	return 0;
}

/* Duplicates the first len bytes of a string s into a new zero-terminates
 * string. Upon failure, NULL is returned. */
char* am_strdupn(const char* s, size_t len)
{
	char* ret = malloc(len+1);

	if(ret) {
		strncpy(ret, s, len);
		ret[len] = '\0';
	}

	return ret;
}


/* Appends a string str of length str_slen to a string *dst with a length of
 * *dst_slen characters and a maximum size of *dst_size characters. If the
 * destination needs to be resized, prealloc additional bytes are allocated. The
 * new size and length are returned in *dst_size and *dst_slen, respectively.
 *
 * Returns 0 on success, otherwise 1. */
static inline int am_append_string(char** dst, const char* str,
				   size_t* dst_slen, size_t* dst_size,
				   size_t prealloc, size_t str_slen)
{
	/* Final size, including final \0 */
	size_t final_size = *dst_slen + str_slen + 1;

	if(*dst_size < final_size) {
		if(*dst_size + prealloc < final_size)
			return 1;

		if(am_realloc_str(dst, *dst_size + prealloc))
			return 1;

		*dst_size += prealloc;
	}

	memcpy(&((*dst)[*dst_slen]), str, str_slen);
	*dst_slen += str_slen;
	(*dst)[*dst_slen] = '\0';

	return 0;
}

struct am_replacement {
	char c;
	const char* r;
};

static const struct am_replacement escape_repl[] = {
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

/* Escapes a string by replacing control characters (alerts, backspaces, form
 * feeds, line feeds, carriage returns, tabs, vertical tabs, backslashes and
 * double quotes) with their respective escape sequences (\a, \b, \f, \n, \r,
 * \t, \v, \\, and \"). The function returns a newly allocated, escaped string
 * or NULL if allocation fails. */
char* am_escape_string(const char* str)
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

		for(const struct am_replacement* r = &escape_repl[0]; r->r; r++) {
			if(str[i] == r->c) {
				if(am_append_string(&curr, r->r, &outpos, &outlen, prealloc, 2))
					goto out_err;

				replaced = 1;
				break;
			}
		}

		if(!replaced) {
			if(isprint(str[i]) || str[i] == '\0') {
				if(am_append_string(&curr, &str[i], &outpos, &outlen, prealloc, 1))
					goto out_err;
			} else {
				snprintf(hexrepl, 5, "\\x%02X", (int)str[i]);

				if(am_append_string(&curr, hexrepl, &outpos, &outlen, prealloc, 4))
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

/* Unescapes a string in place (reverse operation of escape_string), also
 * honoring hexadecimal escape sequences with up to two digits (e.g., \xEF or
 * \xE) and octal escape sequences with up to three digits (e.g., \100, \10 or
 * \1). If shrink is true, the string is reallocated if the number of characters
 * of the unescaped string is smaller than the number of characters of the
 * original string.
 *
 * Returns 0 on success, otherwise 1.
 */
int am_unescape_string_in_place(char** sstr, int shrink)
{
	char* str = *sstr;
	int in_escape = 0;
	const struct am_replacement* r;
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

/* Unescapes the first len characters of a string str and returns the result as
 * a newly allocated, zero-terminated string. Upon failure, NULL is returned. */
char* am_unescape_stringn(const char* str, size_t len)
{
	char* tmp = malloc(len+1);

	if(!tmp)
		return NULL;

	strncpy(tmp, str, len);
	tmp[len] = '\0';

	if(am_unescape_string_in_place(&tmp, 1)) {
		free(tmp);
		return NULL;
	}

	return tmp;
}

/* Unescapes a string str and returns the result as a newly allocated,
 * zero-terminated string. Upon failure, NULL is returned. */
char* am_unescape_string(const char* str)
{
	return am_unescape_stringn(str, strlen(str));
}

static const char si_prefixes[] =
	{ '\0', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y' };

static const uint64_t si_values_u64[] = {
	1,
	1000,
	1000000,
	1000000000,
	1000000000000,
	1000000000000000,
	100000000000000000
};

/* Formats a uint64_t using K, M, G, ... suffixes with max_sigd significant
 * digits and writes the result to buf of size max_len characters. Returns 0 if
 * the entire formatted string fitted into buf and 1 if characters were omitted.
 */
int am_siformat_u64(uint64_t value, size_t max_sigd, char* buf, size_t max_len)
{
	size_t si_idx;
	size_t i = 0;
	uint64_t pre_unit;
	uint64_t tmp;
	uint64_t f;
	uint64_t digit;
	int ret = 1;
	int unit_digits;

	/* Early exit if we don't even have the space for the terminating zero
	 * byte */
	if(max_len == 0)
		goto out;

	/* If just space for terminating 0, just write zero */
	if(max_len == 1)
		goto out_zero;

	if(value > 0) {
		/* Determine prefix */
		for(si_idx = AM_ARRAY_SIZE(si_values_u64); si_idx != 0; si_idx--)
			if(value / si_values_u64[si_idx-1] != 0)
				break;

		/* Adjust si_idx from loop condition */
		si_idx--;
	} else {
		si_idx = 0;
	}

	pre_unit = value / si_values_u64[si_idx];

	if(pre_unit >= 100) {
		f = 100 * si_values_u64[si_idx];
		unit_digits = 3;
	} else if(pre_unit >= 10) {
		f = 10 * si_values_u64[si_idx];
		unit_digits = 2;
	} else {
		f = si_values_u64[si_idx];
		unit_digits = 1;
	}

	tmp = value;

	while(max_sigd--) {
		digit = tmp / f;

		if(unit_digits)
			unit_digits--;

		/* Write current digit */
		if(i < max_len - 1)
			buf[i++] = '0' + digit;
		else
			goto out_zero;

		tmp = tmp % f;

		/* All significant zeros written and further digits zero? */
		if(tmp == 0 && !unit_digits)
			break;

		/* Write decimal dot (only if digits follow) */
		if(f == si_values_u64[si_idx] && max_sigd > 0) {
			if(i < max_len - 1)
				buf[i++] = '.';
			else
				goto out_zero;
		}

		f /= 10;
	}

	/* Report omitted characters if we missed at least one digit before the
	 * dot */
	if(unit_digits == 0)
		ret = 0;

	if(i < max_len - 1 && si_idx != 0)
		buf[i++] = si_prefixes[si_idx];
out_zero:
	buf[i] = '\0';
out:
	return ret;
}
