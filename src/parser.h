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

#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <string.h>
#include <ctype.h>

/* A very simple parser; merely a collection of convenience functions
 * to parse strings, such as reading identifiers, strings, skipping
 * whitespace, etc. */
struct parser {
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
struct parser_token {
	const char* str;
	size_t len;
};

/* Checks whether the token is equal to a zero-terminated string
 * str. */
static inline int parser_token_equals_str(const struct parser_token* t, const char* str)
{
	if(strncmp(t->str, str, t->len) == 0)
		return 1;

	return 0;
}

/* Checks whether the token is equal to any of the a zero-terminated
 * strings in sstr. The last entry of sstr must be NULL. */
static inline int parser_token_equals_str_oneof(const struct parser_token* t, const char** sstr)
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
static inline int parser_token_equals_char(const struct parser_token* t, char c)
{
	return (t->len == 1 && t->str[0] == c);
}

/* Checks whether the token represents a single character and whether
 * the character is equal to one of the characters of the
 * zero-terminated string "chars" (the zero character is not included
 * in the comparison. */
static inline int parser_token_equals_char_oneof(const struct parser_token* t, const char* chars)
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
static inline void parser_init(struct parser* p, const char* start, size_t size)
{
	p->start = start;
	p->curr = start;
	p->end = start+size;
}

/* Checks whether the parser has reached the end of the input
 * sequence. */
static inline int parser_reached_end(const struct parser* p)
{
	return p->curr == p->end;
}

/* Advances the current position to the next character that is not
 * considered as whitespace (i.e., the newline character and all
 * characters for which isblank is true). */
static inline void parser_skip_ws(struct parser* p)
{
	while(!parser_reached_end(p) &&
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

/* Consumes all characters starting from the current position that
 * make up an identifier. If no such character exists the function
 * returns 1. Otherwise, it sets the token to the character sequence
 * representing the identifier and returns 0.
 */
static inline int parser_read_identifier(struct parser* p, struct parser_token* t)
{
	if(parser_reached_end(p))
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

/* Same as parser_read_identifier, but skips all preceding
 * whitespace */
static inline int parser_read_next_identifier(struct parser* p, struct parser_token* t)
{
	parser_skip_ws(p);
	return parser_read_identifier(p, t);
}

/* Consumes the next character, initializes the token with it and
 * returns 0 if equal to c. Otherwise, the function returns 1. */
static inline int parser_read_char(struct parser* p, struct parser_token* t, char c)
{
	if(parser_reached_end(p) || *p->curr != c)
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	return 0;
}

/* Same as parser_read_char, but does not consume the character. */
static inline int parser_peek_char(struct parser* p, struct parser_token* t, char c)
{
	if(parser_reached_end(p) || *p->curr != c)
		return 1;

	t->str = p->curr;
	t->len = 1;

	return 0;
}

/* Same as parser_read_next_nonws_char, but skips all preceding
 * whitespace. */
static inline int parser_read_next_char(struct parser* p, struct parser_token* t, char c)
{
	parser_skip_ws(p);
	return parser_read_char(p, t, c);
}

/* Consumes the next character, initializes the token with it and
 * returns 0 if the next character is not whitespace. Otherwise the
 * function returns 1. */
static inline int parser_read_any_char(struct parser* p, struct parser_token* t)
{
	if(parser_reached_end(p))
		return 1;

	t->str = p->curr;
	t->len = 1;
	p->curr++;

	return 0;
}

/* Same as parser_read_any_char, but skips preceding
 * whitespace */
static inline int parser_read_next_any_char(struct parser* p, struct parser_token* t)
{
	parser_skip_ws(p);
	return parser_read_any_char(p, t);
}

/* Same as parser_read_any_char, but does not consume the
 * character. */
static inline int parser_peek_any_char(struct parser* p, struct parser_token* t)
{
	if(parser_reached_end(p))
		return 1;

	t->str = p->curr;
	t->len = 1;

	return 0;
}

/* Reads a string, intializes the token with it and returns. Note that
 * the quotes around the string are included in t->str. If string
 * parsing fails the function returns 1. */
static inline int parser_read_string(struct parser* p, struct parser_token* t)
{
	int last_bs = 0;

	if(parser_reached_end(p))
		return 1;

	if(*p->curr != '"')
		return 1;

	t->str = p->curr;
	p->curr++;

	while(1) {
		if(parser_reached_end(p))
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

/* Same as parser_read_string, but skips all preceding whitespace. */
static inline int parser_read_next_string(struct parser* p, struct parser_token* t)
{
	parser_skip_ws(p);
	return parser_read_string(p, t);
}

/* Consumes all numeric characters from the current position. If no
 * such character exists the function returns 1. Otherwise, it sets
 * the token to the sequence of numeric characters and returns 0.
 */
static inline int parser_read_int(struct parser* p, struct parser_token* t)
{
	if(parser_reached_end(p))
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

/* Same as parser_read_int, but skips all preceding whitespace */
static inline int parser_read_next_int(struct parser* p, struct parser_token* t)
{
	parser_skip_ws(p);
	return parser_read_int(p, t);
}

#endif
