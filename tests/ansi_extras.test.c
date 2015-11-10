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
#include "../src/ansi_extras.h"

UNIT_TEST(int64_swap_test)
{
	ASSERT_EQUALS(int64_swap(0x0100000000000000), (int64_t)0x0000000000000001);
	ASSERT_EQUALS(int64_swap(0x0001000000000000), (int64_t)0x0000000000000100);
	ASSERT_EQUALS(int64_swap(0x0000010000000000), (int64_t)0x0000000000010000);
	ASSERT_EQUALS(int64_swap(0x0000000100000000), (int64_t)0x0000000001000000);
	ASSERT_EQUALS(int64_swap(0x0000000001000000), (int64_t)0x0000000100000000);
	ASSERT_EQUALS(int64_swap(0x0000000000010000), (int64_t)0x0000010000000000);
	ASSERT_EQUALS(int64_swap(0x0000000000000100), (int64_t)0x0001000000000000);
	ASSERT_EQUALS(int64_swap(0x0000000000000001), (int64_t)0x0100000000000000);

	ASSERT_EQUALS(int64_swap(0x1000000000000000), (int64_t)0x0000000000000010);
	ASSERT_EQUALS(int64_swap(0x0010000000000000), (int64_t)0x0000000000001000);
	ASSERT_EQUALS(int64_swap(0x0000100000000000), (int64_t)0x0000000000100000);
	ASSERT_EQUALS(int64_swap(0x0000001000000000), (int64_t)0x0000000010000000);
	ASSERT_EQUALS(int64_swap(0x0000000010000000), (int64_t)0x0000001000000000);
	ASSERT_EQUALS(int64_swap(0x0000000000100000), (int64_t)0x0000100000000000);
	ASSERT_EQUALS(int64_swap(0x0000000000001000), (int64_t)0x0010000000000000);
	ASSERT_EQUALS(int64_swap(0x0000000000000010), (int64_t)0x1000000000000000);

	ASSERT_EQUALS(int64_swap(0x0102030405060708), (int64_t)0x0807060504030201);
	ASSERT_EQUALS(int64_swap(0x91A2B3C4D5E6F708), (int64_t)0x08F7E6D5C4B3A291);
}
END_TEST()

UNIT_TEST(int32_swap_test)
{
	ASSERT_EQUALS(int32_swap(0x01000000), (int32_t)0x00000001);
	ASSERT_EQUALS(int32_swap(0x00010000), (int32_t)0x00000100);
	ASSERT_EQUALS(int32_swap(0x00000100), (int32_t)0x00010000);
	ASSERT_EQUALS(int32_swap(0x00000001), (int32_t)0x01000000);

	ASSERT_EQUALS(int32_swap(0x10000000), (int32_t)0x00000010);
	ASSERT_EQUALS(int32_swap(0x00100000), (int32_t)0x00001000);
	ASSERT_EQUALS(int32_swap(0x00001000), (int32_t)0x00100000);
	ASSERT_EQUALS(int32_swap(0x00000010), (int32_t)0x10000000);

	ASSERT_EQUALS(int32_swap(0x01020304), (int32_t)0x04030201);
	ASSERT_EQUALS(int32_swap(0x91A2B3C4), (int32_t)0xC4B3A291);
}
END_TEST()

UNIT_TEST(int16_swap_test)
{
	ASSERT_EQUALS(int16_swap(0x0100), (int16_t)0x0001);
	ASSERT_EQUALS(int16_swap(0x0001), (int16_t)0x0100);

	ASSERT_EQUALS(int16_swap(0x1000), (int16_t)0x0010);
	ASSERT_EQUALS(int16_swap(0x0010), (int16_t)0x1000);

	ASSERT_EQUALS(int16_swap(0x0102), (int16_t)0x0201);
	ASSERT_EQUALS(int16_swap(0x91A2), (int16_t)0xA291);
}
END_TEST()

UNIT_TEST(strreplace_test)
{
	struct fourstrs {
		const char* init;
		const char* needle;
		const char* repl;
		const char* exp;
	};

	size_t max_size = 64;

	static const struct fourstrs t[] = {
		{"AAA", "A", "B", "BBB"},
		{"AAB", "AA", "BB", "BBB"},
		{"AAA", "AAA", "", ""},
		{"ABABA", "ABA", "A", "ABA"},
		{"A", "A", "AAAAAAA", "AAAAAAA"},
		{NULL}
	};

	char *tmp = malloc(max_size);
	ASSERT_NONNULL(tmp);

	for(const struct fourstrs* i = &t[0]; i->init; i++) {
		strcpy(tmp, i->init);
		strreplace(tmp, i->needle, i->repl);
		ASSERT_EQUALS_STRING(tmp, i->exp);
	}

	free(tmp);
}
END_TEST()

UNIT_TEST(escape_string_test)
{
	struct twostrs {
		const char* r;
		const char* e;
	};

	static const struct twostrs exp[] = {
		{"AAA", "AAA"},
		{"A\"", "A\\\""},
		{"A\t\tA", "A\\t\\tA"},
		{"A\a\b\f\n\r\t\v\\\"A", "A\\a\\b\\f\\n\\r\\t\\v\\\\\\\"A"},
		{"String with funky\x01\x02\x01 chars", "String with funky\\x01\\x02\\x01 chars"},
		{NULL, NULL}
	};

	char *tmp;

	for(const struct twostrs* i = &exp[0]; i->r; i++) {
		tmp = escape_string(i->r);
		ASSERT_NONNULL(tmp);
		ASSERT_EQUALS_STRING(tmp, i->e);
		free(tmp);
	}
}
END_TEST()

UNIT_TEST_SUITE(ansi_extras_test)
{
	ADD_TEST(strreplace_test);
	ADD_TEST(int64_swap_test);
	ADD_TEST(int32_swap_test);
	ADD_TEST(int16_swap_test);
	ADD_TEST(escape_string_test);
}
END_TEST_SUITE()
