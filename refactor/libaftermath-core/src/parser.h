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

#ifndef AM_PARSER_H
#define AM_PARSER_H

#include <stddef.h>
#include <string.h>
#include <ctype.h>

/* A very simple parser; merely a collection of convenience functions
 * to parse strings, such as reading identifiers, strings, skipping
 * whitespace, etc. */
struct am_parser {
	const char* curr;
	const char* start;
	const char* end;
};

/* A token represents a portion of the string that is being processed
 * by the parser. As this is not a fully-developed parser, not even
 * designed for a specific language there is no notion of token types.
 * The field str is a pointer to the first character in the input
 * sequence and is not necessarily terminated by a zero character.
 */
struct am_parser_token {
	const char* str;
	size_t len;
};

/* Checks whether the token is equal to a zero-terminated string
 * str. */
static inline int
am_parser_token_equals_str(const struct am_parser_token* t, const char* str)
{
	if(strncmp(t->str, str, t->len) == 0)
		return 1;

	return 0;
}

/* Checks whether the token is equal to any of the a zero-terminated
 * strings in sstr. The last entry of sstr must be NULL. */
static inline int
am_parser_token_equals_str_oneof(const struct am_parser_token* t,
				 const char** sstr)
{
	const char* str = *sstr;

	while(str) {
		if(strncmp(t->str, str, t->len) == 0)
			return 1;

		sstr++;
		str = *sstr;
	}

	return 0;
}

/* Checks whether the token represents a single character and whether
 * the character is equal to the parameter c. */
static inline int
am_parser_token_equals_char(const struct am_parser_token* t, char c)
{
	return (t->len == 1 && t->str[0] == c);
}

/* Checks whether the token represents a single character and whether
 * the character is equal to one of the characters of the
 * zero-terminated string "chars" (the zero character is not included
 * in the comparison. */
static inline int
am_parser_token_equals_char_oneof(const struct am_parser_token* t,
				  const char* chars)
{
	size_t len = strlen(chars);

	if(t->len != 1)
		return 0;

	for(size_t i = 0; i < len; i++)
		if(t->str[0] == chars[i])
			return 1;

	return 0;
}

/* Initializes a parser with the sequence of characters starting at
 * the address indicated by start. As the input does not necessarily
 * have to be zero-terminated its size must be indicated. */
static inline void
am_parser_init(struct am_parser* p, const char* start, size_t size)
{
	p->start = start;
	p->curr = start;
	p->end = start+size;
}

/* Checks whether the parser has reached the end of the input
 * sequence. */
static inline int am_parser_reached_end(const struct am_parser* p)
{
	return p->curr == p->end;
}

/* Checks if the next characters are identical to the sequence of characters
 * specified in str (except the terminating '\0'). If so, the function advances
 * the parser by strlen(str) characters and returns 0. Otherwise, 1 is
 * returned. */
static inline int am_parser_expect_chars(struct am_parser* p, const char* str)
{
	while(*str) {
		if(am_parser_reached_end(p))
			return 1;

		if(*p->curr != *str)
			return 1;

		str++;
		p->curr++;
	}

	return 0;
}

/* Advances the current position to the next character that is not
 * considered as whitespace (i.e., the newline character and all
 * characters for which isblank is true). */
static inline void am_parser_skip_ws(struct am_parser* p)
{
	while(!am_parser_reached_end(p) &&
	      (isblank(*p->curr) || *p->curr == '\n'))
	{
		p->curr++;
	}
}

/* Checks whether c is a character that is allowed as the first
 * character of an identifier (i.e., [A-Za-z_]). */
static inline int isident_start_char(char c)
{
	return isalpha(c) || c == '_';
}

/* Checks whether c is a character that is allowed in identifiers,
 * starting from the second charatcter (i.e., alphanumeric characters
 * and the underscore). */
static inline int isident_next_char(char c)
{
	return isalnum(c) || c == '_';
}

/* Checks whether c is a character that is allowed as the first
 * character of a group name. */
static inline int isgroup_name_start_char(char c)
{
	return isident_start_char(c);
}

/* Checks whether c is a character that is allowed in group names, starting from
 * the second character (i.e., alphanumeric characters, an underscore, colon,
 * asterisk, smaller and greater sign, or a comma). */
static inline int isgroup_name_next_char(char c)
{
	return isalnum(c) ||
		c == '_' || c == ':' || c == '<' || c == '>' || c == '*' ||
		c == ',';
}

/* Consumes all characters starting from the current position that
 * make up an identifier. If no such character exists the function
 * returns 1. Otherwise, it sets the token to the character sequence
 * representing the identifier and returns 0.
 */
static inline int
am_parser_read_identifier(struct am_parser* p, struct am_parser_token* t)
{
	if(am_parser_reached_end(p))
		return 1;

	if(!isident_start_char(*p->curr))
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	while(p->curr != p->end && isident_next_char(*p->curr)) {
		t->len++;
		p->curr++;
	}

	return 0;
}

/* Same as am_parser_read_identifier, but skips all preceding
 * whitespace */
static inline int
am_parser_read_next_identifier(struct am_parser* p, struct am_parser_token* t)
{
	am_parser_skip_ws(p);
	return am_parser_read_identifier(p, t);
}

/* Consumes all characters starting from the current position that make up a
 * group name. If no such character exists the function returns 1. Otherwise, it
 * sets the token to the character sequence representing the group name and
 * returns 0.
 */
static inline int
am_parser_read_group_name(struct am_parser* p, struct am_parser_token* t)
{
	if(am_parser_reached_end(p))
		return 1;

	if(!isgroup_name_start_char(*p->curr))
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	while(p->curr != p->end && isgroup_name_next_char(*p->curr)) {
		t->len++;
		p->curr++;
	}

	return 0;
}

/* Same as am_parser_read_group_name, but skips all preceding
 * whitespace */
static inline int
am_parser_read_next_group_name(struct am_parser* p, struct am_parser_token* t)
{
	am_parser_skip_ws(p);
	return am_parser_read_group_name(p, t);
}

/* Consumes the next character, initializes the token with it and
 * returns 0 if equal to c. Otherwise, the function returns 1. */
static inline int
am_parser_read_char(struct am_parser* p, struct am_parser_token* t, char c)
{
	if(am_parser_reached_end(p) || *p->curr != c)
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	return 0;
}

/* Same as am_parser_read_char, but does not consume the character. */
static inline int
am_parser_peek_char(struct am_parser* p, struct am_parser_token* t, char c)
{
	if(am_parser_reached_end(p) || *p->curr != c)
		return 1;

	t->str = p->curr;
	t->len = 1;

	return 0;
}

/* Same as am_parser_read_next_nonws_char, but skips all preceding
 * whitespace. */
static inline int
am_parser_read_next_char(struct am_parser* p, struct am_parser_token* t, char c)
{
	am_parser_skip_ws(p);
	return am_parser_read_char(p, t, c);
}

/* Consumes the next character, initializes the token with it and
 * returns 0 if the next character is not whitespace. Otherwise the
 * function returns 1. */
static inline int
am_parser_read_any_char(struct am_parser* p, struct am_parser_token* t)
{
	if(am_parser_reached_end(p))
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	return 0;
}

/* Same as am_parser_read_any_char, but skips preceding
 * whitespace */
static inline int
am_parser_read_next_any_char(struct am_parser* p, struct am_parser_token* t)
{
	am_parser_skip_ws(p);
	return am_parser_read_any_char(p, t);
}

/* Same as am_parser_read_any_char, but does not consume the
 * character. */
static inline int
am_parser_peek_any_char(struct am_parser* p, struct am_parser_token* t)
{
	if(am_parser_reached_end(p))
		return 1;

	t->str = p->curr;
	t->len = 1;

	return 0;
}

/* Reads a string, intializes the token with it and returns. Note that
 * the quotes around the string are included in t->str. If string
 * parsing fails the function returns 1. */
static inline int
am_parser_read_string(struct am_parser* p, struct am_parser_token* t)
{
	int last_bs = 0;

	if(am_parser_reached_end(p))
		return 1;

	if(*p->curr != '"')
		return 1;

	t->str = p->curr;
	p->curr++;

	while(1) {
		if(am_parser_reached_end(p))
			return 1;

		if(*p->curr == '\\') {
			last_bs = !last_bs;
		} else {
			if(!last_bs && *p->curr == '"') {
				t->len = p->curr - t->str + 1;
				p->curr++;
				return 0;
			}

			last_bs = 0;
		}

		p->curr++;
	}
}

/* Same as am_parser_read_string, but skips all preceding whitespace. */
static inline int
am_parser_read_next_string(struct am_parser* p, struct am_parser_token* t)
{
	am_parser_skip_ws(p);
	return am_parser_read_string(p, t);
}

/* Consumes all numeric characters from the current position. If no
 * such character exists the function returns 1. Otherwise, it sets
 * the token to the sequence of numeric characters and returns 0.
 */
static inline int
am_parser_read_int(struct am_parser* p, struct am_parser_token* t)
{
	if(am_parser_reached_end(p))
		return 1;

	if(!isdigit(*p->curr))
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	while(p->curr != p->end && isdigit(*p->curr)) {
		t->len++;
		p->curr++;
	}

	return 0;
}

/* Same as am_parser_read_int, but skips all preceding whitespace */
static inline int
am_parser_read_next_int(struct am_parser* p, struct am_parser_token* t)
{
	am_parser_skip_ws(p);
	return am_parser_read_int(p, t);
}

/* Consumes all numeric characters from the current position. An
 * optional unit prefix at the end is allowed (i.e., K, M, G, T,
 * P). If no such sequence exists the function returns 1. Otherwise,
 * it sets the token to the sequence and returns 0.
 */
static inline int
am_parser_read_int_with_unit(struct am_parser* p, struct am_parser_token* t)
{
	if(am_parser_reached_end(p))
		return 1;

	if(!isdigit(*p->curr))
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	while(p->curr != p->end && isdigit(*p->curr)) {
		t->len++;
		p->curr++;
	}

	if(!am_parser_reached_end(p)) {
		if(*p->curr == 'K' ||
		   *p->curr == 'M' ||
		   *p->curr == 'G' ||
		   *p->curr == 'T' ||
		   *p->curr == 'P')
		{
			t->len++;
			p->curr++;
		}
	}

	return 0;
}

/* Same as am_parser_read_int_with_unit, but skips all preceding
 * whitespace */
static inline int
am_parser_read_next_int_with_unit(struct am_parser* p, struct am_parser_token* t)
{
	am_parser_skip_ws(p);
	return am_parser_read_int_with_unit(p, t);
}

/* Consumes all characters of a double starting from the current position. If no
 * such character exists the function returns 1. Otherwise, it sets the token to
 * the sequence of numeric characters and returns 0.
 */
static inline int
am_parser_read_double(struct am_parser* p, struct am_parser_token* t)
{
	int dot_read = 0;
	int is_signed = 0;

	if(am_parser_reached_end(p))
		return 1;

	if(!isdigit(*p->curr) && *p->curr != '.' && *p->curr != '-')
		return 1;

	if(*p->curr == '-')
		is_signed = 1;

	if(*p->curr == '.')
		dot_read = 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	while(p->curr != p->end &&
	      (isdigit(*p->curr) || (!dot_read && *p->curr == '.')))
	{
		if(*p->curr == '.')
			dot_read = 1;

		t->len++;
		p->curr++;
	}

	if(!dot_read || (t->len == 1) || (is_signed && t->len == 2))
		return 1;

	return 0;
}

/* Same as am_parser_read_double, but skips all preceding whitespace */
static inline int
am_parser_read_next_double(struct am_parser* p, struct am_parser_token* t)
{
	am_parser_skip_ws(p);
	return am_parser_read_double(p, t);
}

/* Same as am_parser_read_double, but does not consume the characters. */
static inline int
am_parser_peek_double(struct am_parser* p, struct am_parser_token* t)
{
	int ret;
	const char* curr = p->curr;

	ret = am_parser_read_double(p, t);
	p->curr = curr;

	return ret;
}

/* Consumes all characters of a double starting at the current position. An
 * optional unit prefix at the end is allowed (i.e., K, M, G, T, P). If no such
 * sequence exists the function returns 1. Otherwise, it sets the token to the
 * sequence and returns 0.
 */
static inline int
am_parser_read_double_with_unit(struct am_parser* p, struct am_parser_token* t)
{
	if(am_parser_reached_end(p))
		return 1;

	if(!isdigit(*p->curr))
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	while(p->curr != p->end && isdigit(*p->curr)) {
		t->len++;
		p->curr++;
	}

	if(!am_parser_reached_end(p)) {
		if(*p->curr == 'K' ||
		   *p->curr == 'M' ||
		   *p->curr == 'G' ||
		   *p->curr == 'T' ||
		   *p->curr == 'P')
		{
			t->len++;
			p->curr++;
		}
	}

	return 0;
}

/* Same as am_parser_read_double_with_unit, but skips all preceding
 * whitespace */
static inline int am_parser_read_next_double_with_unit(struct am_parser* p,
						       struct am_parser_token* t)
{
	am_parser_skip_ws(p);
	return am_parser_read_double_with_unit(p, t);
}


/* Returns the address of the next occurrence of c. The position of
 * the parser is advanced to the position of the character. If the
 * character is not found NULL is returned. */
static inline const char* am_parser_find_char(struct am_parser* p, char c)
{
	while(!am_parser_reached_end(p) && *p->curr != c)
		p->curr++;

	if(*p->curr != c)
		return NULL;

	return p->curr;
}

#endif
