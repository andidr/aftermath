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

#include <unit_tests.h>
#include "../src/parser.h"

struct token_test {
	const char* str;
	const char* exp;
	const int res;
};

UNIT_TEST(read_string_test)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};

	static const struct token_test tests[] = {
		{"\"\"", "\"\"", 0},
		{"\"\"   ", "\"\"", 0},
		{"\"A\"", "\"A\"", 0},
		{"\"ABC\"", "\"ABC\"", 0},
		{"\"ABC\\\"\"", "\"ABC\\\"\"", 0},
		{"\"ABC\"\"", "\"ABC\"", 0},
		{"", NULL, 1},
		{"\"", NULL, 1},
		{"ABC", NULL, 1},
		{"A\"BC\"", NULL, 1},
		{NULL}
	};

	for(const struct token_test* t = tests; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		if(t->res == 0) {
			ASSERT_EQUALS(parser_read_string(&p, &token), 0);
			ASSERT_EQUALS(token.len, strlen(t->exp));
			ASSERT_STRING_PREFIX(token.str, t->exp);
		} else {
			ASSERT_DIFFERENT(parser_read_string(&p, &token), 0);
		}
	}
}
END_TEST()

UNIT_TEST(read_string_test_ws)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};

	static const struct token_test tests_ws[] = {
		{" \"\"", "\"\"", 0},
		{"   \"\"   ", "\"\"", 0},
		{"\t   \"\"   ", "\"\"", 0},
		{"\n\t \n  \"\"   ", "\"\"", 0},
		{" \"A\"", "\"A\"", 0},
		{"\n\t\"ABC\"", "\"ABC\"", 0},
		{"\n\t\"ABC\\\"\"", "\"ABC\\\"\"", 0},
		{"\n\t\"ABC\"\"", "\"ABC\"", 0},
		{"\n\t", NULL, 1},
		{"  \"", NULL, 1},
		{"\n\tABC", NULL, 1},
		{" A\"BC\"", NULL, 1},
		{NULL}
	};

	for(const struct token_test* t = tests_ws; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		if(t->res == 0) {
			ASSERT_EQUALS(parser_read_next_string(&p, &token), 0);
			ASSERT_EQUALS(token.len, strlen(t->exp));
			ASSERT_STRING_PREFIX(token.str, t->exp);
		} else {
			ASSERT_DIFFERENT(parser_read_string(&p, &token), 0);
		}
	}
}
END_TEST()

UNIT_TEST(read_int_test)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};

	static const struct token_test tests[] = {
		{"1", "1", 0},
		{"123", "123", 0},
		{"0123456789", "0123456789", 0},
		{"a1", NULL, 1},
		{"", NULL, 1},
		{NULL}
	};

	for(const struct token_test* t = tests; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		if(t->res == 0) {
			ASSERT_EQUALS(parser_read_int(&p, &token), 0);
			ASSERT_EQUALS(token.len, strlen(t->exp));
			ASSERT_STRING_PREFIX(token.str, t->exp);
		} else {
			ASSERT_DIFFERENT(parser_read_int(&p, &token), 0);
		}
	}
}
END_TEST()

UNIT_TEST(read_identifier_test)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};

	static const struct token_test tests[] = {
		{"A", "A", 0},
		{"ABC", "ABC", 0},
		{"ABC123", "ABC123", 0},
		{"_", "_", 0},
		{"_a", "_a", 0},
		{"1st", NULL, 1},
		{"\"", NULL, 1},
		{NULL}
	};

	for(const struct token_test* t = tests; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		if(t->res == 0) {
			ASSERT_EQUALS(parser_read_identifier(&p, &token), 0);
			ASSERT_EQUALS(token.len, strlen(t->exp));
			ASSERT_STRING_PREFIX(token.str, t->exp);
		} else {
			ASSERT_DIFFERENT(parser_read_identifier(&p, &token), 0);
		}
	}
}
END_TEST()

UNIT_TEST(read_identifier_test_ws)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};

	static const struct token_test tests[] = {
		{"  A", "A", 0},
		{" \t\nA", "A", 0},
		{" \t\nABC", "ABC", 0},
		{" \t\nABC123", "ABC123", 0},
		{" \t\n_", "_", 0},
		{" \t\n_a", "_a", 0},
		{" \t\n1st", NULL, 1},
		{" \t\n\"", NULL, 1},
		{NULL}
	};

	for(const struct token_test* t = tests; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		if(t->res == 0) {
			ASSERT_EQUALS(parser_read_next_identifier(&p, &token), 0);
			ASSERT_EQUALS(token.len, strlen(t->exp));
			ASSERT_STRING_PREFIX(token.str, t->exp);
		} else {
			ASSERT_DIFFERENT(parser_read_identifier(&p, &token), 0);
		}
	}
}
END_TEST()

UNIT_TEST(read_char_test)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};
	char c = 'A';

	static const struct token_test tests[] = {
		{"A", "A", 0},
		{"B", NULL, 1},
		{" ", NULL, 1},
		{NULL}
	};

	for(const struct token_test* t = tests; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		if(t->res == 0) {
			ASSERT_EQUALS(parser_read_char(&p, &token, c), 0);
			ASSERT_EQUALS(token.len, strlen(t->exp));
			ASSERT_STRING_PREFIX(token.str, t->exp);
		} else {
			ASSERT_DIFFERENT(parser_read_char(&p, &token, c), 0);
		}
	}
}
END_TEST()

UNIT_TEST(peek_char_test)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};
	char c = 'A';

	static const struct token_test tests[] = {
		{"A", "A", 0},
		{"B", NULL, 1},
		{" ", NULL, 1},
		{NULL}
	};

	for(const struct token_test* t = tests; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		for(int i = 0; i < 2; i++) {
			if(t->res == 0) {
				ASSERT_EQUALS(parser_peek_char(&p, &token, c), 0);
				ASSERT_EQUALS(token.len, strlen(t->exp));
				ASSERT_STRING_PREFIX(token.str, t->exp);
			} else {
				ASSERT_DIFFERENT(parser_peek_char(&p, &token, c), 0);
			}
		}
	}
}
END_TEST()

UNIT_TEST(read_any_char_test)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};

	static const struct token_test tests[] = {
		{"A", "A", 0},
		{"B", "B", 0},
		{"C", "C", 0},
		{"/", "/", 0},
		{"AB", "A", 0},
		{"BA", "B", 0},
		{"C/", "C", 0},
		{"D", "D", 0},
		{"", NULL, 1},
		{" ", " ", 0},
		{NULL}
	};

	for(const struct token_test* t = tests; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		if(t->res == 0) {
			ASSERT_EQUALS(parser_read_any_char(&p, &token), 0);
			ASSERT_EQUALS(token.len, strlen(t->exp));
			ASSERT_STRING_PREFIX(token.str, t->exp);
		} else {
			ASSERT_DIFFERENT(parser_read_any_char(&p, &token), 0);
		}
	}
}
END_TEST()

UNIT_TEST(read_next_any_char_test)
{
	struct parser p;
	struct parser_token token = {.str = NULL, .len = 0};

	static const struct token_test tests[] = {
		{"    A", "A", 0},
		{"      B", "B", 0},
		{"      C", "C", 0},
		{"  \t /", "/", 0},
		{"  \t   \t AB", "A", 0},
		{"   BA", "B", 0},
		{"\n   \t   C/", "C", 0},
		{"D", "D", 0},
		{"", NULL, 1},
		{" ", NULL, 1},
		{NULL}
	};

	for(const struct token_test* t = tests; t->str; t++) {
		parser_init(&p, t->str, strlen(t->str));

		if(t->res == 0) {
			ASSERT_EQUALS(parser_read_next_any_char(&p, &token), 0);
			ASSERT_EQUALS(token.len, strlen(t->exp));
			ASSERT_STRING_PREFIX(token.str, t->exp);
		} else {
			ASSERT_DIFFERENT(parser_read_next_any_char(&p, &token), 0);
		}
	}
}
END_TEST()

UNIT_TEST_SUITE(parser_test)
{
	ADD_TEST(read_string_test);
	ADD_TEST(read_string_test_ws);
	ADD_TEST(read_int_test);
	ADD_TEST(read_identifier_test);
	ADD_TEST(read_identifier_test_ws);
	ADD_TEST(read_char_test);
	ADD_TEST(peek_char_test);
	ADD_TEST(read_any_char_test);
	ADD_TEST(read_next_any_char_test);
}
END_TEST_SUITE()
