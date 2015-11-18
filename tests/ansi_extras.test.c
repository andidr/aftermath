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

struct twostrs_v {
	const char* r;
	const char* e;
	const int v;
};

static const struct twostrs_v exp_unescape[] = {
	{"AAA", "AAA", 0},
	{"A\\\"", "A\"", 0},
	{"A\\t\\tA", "A\t\tA", 0},
	{"A\\a\\b\\f\\n\\r\\t\\v\\\\\\\"A", "A\a\b\f\n\r\t\v\\\"A", 0},
	{"String with funky\\x01\\x02\\x01 chars", "String with funky\x01\x02\x01 chars", 0},
	{"String with funky\\001\\002\\001 chars", "String with funky\x01\x02\x01 chars", 0},
	{"Some \\\" quotes with \\\"   spaces    \\\"   ", "Some \" quotes with \"   spaces    \"   ", 0},
	{"\\x", "", 1},
	{"\\xA", "\x0A", 0},
	{"\\xAG", "\xAG", 0},
	{"\\xAB", "\xAB", 0},
	{"\\xab", "\xAB", 0},
	{"\\xAb", "\xAB", 0},
	{"\\xaB", "\xAB", 0},
	{"\\1", "\1", 0},
	{"\\01", "\01", 0},
	{"\\008", "", 0},
	{"\\009", "", 0},
	{"\\c", "", 1},
	{"\\d", "", 1},
	{"\\e", "", 1},
	{"\\g", "", 1},
	{"\\h", "", 1},
	{"\\i", "", 1},
	{"\\j", "", 1},
	{"\\k", "", 1},
	{"\\l", "", 1},
	{"\\m", "", 1},
	{"\\o", "", 1},
	{"\\p", "", 1},
	{"\\q", "", 1},
	{"\\s", "", 1},
	{"\\u", "", 1},
	{"\\w", "", 1},
	{"\\x", "", 1},
	{"\\y", "", 1},
	{"\\z", "", 1},
	{"\\C", "", 1},
	{"\\D", "", 1},
	{"\\E", "", 1},
	{"\\G", "", 1},
	{"\\H", "", 1},
	{"\\I", "", 1},
	{"\\J", "", 1},
	{"\\K", "", 1},
	{"\\L", "", 1},
	{"\\M", "", 1},
	{"\\O", "", 1},
	{"\\P", "", 1},
	{"\\Q", "", 1},
	{"\\S", "", 1},
	{"\\U", "", 1},
	{"\\W", "", 1},
	{"\\X", "", 1},
	{"\\Y", "", 1},
	{"\\Z", "", 1},
	{"\\0", "", 0},
	{"\\1", "\1", 0},
	{"\\2", "\2", 0},
	{"\\3", "\3", 0},
	{"\\4", "\4", 0},
	{"\\5", "\5", 0},
	{"\\6", "\6", 0},
	{"\\7", "\7", 0},
	{"\\8", "", 1},
	{"\\9", "", 1},
	{NULL, NULL, 0}
};

UNIT_TEST(unescape_string_in_place_test)
{
	char *tmp;

	for(const struct twostrs_v* i = &exp_unescape[0]; i->r; i++) {
		tmp = strdup(i->r);
		ASSERT_NONNULL(tmp);
		ASSERT_EQUALS(unescape_string_in_place(&tmp, 1), i->v);

		if(i->v == 0)
			ASSERT_EQUALS_STRING(tmp, i->e);

		free(tmp);
	}
}
END_TEST()

UNIT_TEST(unescape_string_test)
{
	char *tmp;

	for(const struct twostrs_v* i = &exp_unescape[0]; i->r; i++) {
		tmp = unescape_string(i->r);

		if(i->v == 0) {
			ASSERT_NONNULL(tmp);
			ASSERT_EQUALS_STRING(tmp, i->e);
			free(tmp);
		} else {
			ASSERT_NULL(tmp);
		}
	}
}
END_TEST()

UNIT_TEST(xdigitval_test)
{
	ASSERT_EQUALS(xdigit_val('0'), 0);
	ASSERT_EQUALS(xdigit_val('1'), 1);
	ASSERT_EQUALS(xdigit_val('2'), 2);
	ASSERT_EQUALS(xdigit_val('3'), 3);
	ASSERT_EQUALS(xdigit_val('4'), 4);
	ASSERT_EQUALS(xdigit_val('5'), 5);
	ASSERT_EQUALS(xdigit_val('6'), 6);
	ASSERT_EQUALS(xdigit_val('7'), 7);
	ASSERT_EQUALS(xdigit_val('8'), 8);
	ASSERT_EQUALS(xdigit_val('9'), 9);

	ASSERT_EQUALS(xdigit_val('a'), 10);
	ASSERT_EQUALS(xdigit_val('b'), 11);
	ASSERT_EQUALS(xdigit_val('c'), 12);
	ASSERT_EQUALS(xdigit_val('d'), 13);
	ASSERT_EQUALS(xdigit_val('e'), 14);
	ASSERT_EQUALS(xdigit_val('f'), 15);

	ASSERT_EQUALS(xdigit_val('A'), 10);
	ASSERT_EQUALS(xdigit_val('B'), 11);
	ASSERT_EQUALS(xdigit_val('C'), 12);
	ASSERT_EQUALS(xdigit_val('D'), 13);
	ASSERT_EQUALS(xdigit_val('E'), 14);
	ASSERT_EQUALS(xdigit_val('F'), 15);

	ASSERT_EQUALS(xdigit_val('g'), -1);
	ASSERT_EQUALS(xdigit_val('G'), -1);
}
END_TEST()

UNIT_TEST(atou64n_test)
{
	const char* str = "1234567890";

	ASSERT_EQUALS(atou64n(str, 10), 1234567890);
	ASSERT_EQUALS(atou64n(str, 9), 123456789);
	ASSERT_EQUALS(atou64n(str, 8), 12345678);
	ASSERT_EQUALS(atou64n(str, 7), 1234567);
	ASSERT_EQUALS(atou64n(str, 6), 123456);
	ASSERT_EQUALS(atou64n(str, 5), 12345);
	ASSERT_EQUALS(atou64n(str, 4), 1234);
	ASSERT_EQUALS(atou64n(str, 3), 123);
	ASSERT_EQUALS(atou64n(str, 2), 12);
	ASSERT_EQUALS(atou64n(str, 1), 1);

	ASSERT_EQUALS(atou64n(str+1, 9), 234567890);
	ASSERT_EQUALS(atou64n(str+2, 8), 34567890);
	ASSERT_EQUALS(atou64n(str+3, 7), 4567890);
	ASSERT_EQUALS(atou64n(str+4, 6), 567890);
	ASSERT_EQUALS(atou64n(str+5, 5), 67890);
	ASSERT_EQUALS(atou64n(str+6, 4), 7890);
	ASSERT_EQUALS(atou64n(str+7, 3), 890);
	ASSERT_EQUALS(atou64n(str+8, 2), 90);
	ASSERT_EQUALS(atou64n(str+9, 1), 0);
}
END_TEST()

UNIT_TEST(atou64n_unit_test)
{
	uint64_t val = 0;

	ASSERT_EQUALS(atou64n_unit("1234", 4, &val), 0);
	ASSERT_EQUALS(val, 1234);

	ASSERT_EQUALS(atou64n_unit("1234", 3, &val), 0);
	ASSERT_EQUALS(val, 123);

	ASSERT_EQUALS(atou64n_unit("1234", 2, &val), 0);
	ASSERT_EQUALS(val, 12);

	ASSERT_EQUALS(atou64n_unit("1234", 1, &val), 0);
	ASSERT_EQUALS(val, 1);

	ASSERT_EQUALS(atou64n_unit("1234K", 5, &val), 0);
	ASSERT_EQUALS(val, 1234000);

	ASSERT_EQUALS(atou64n_unit("1234 K", 6, &val), 0);
	ASSERT_EQUALS(val, 1234000);

	ASSERT_EQUALS(atou64n_unit("1234    K", 9, &val), 0);
	ASSERT_EQUALS(val, 1234000);

	ASSERT_EQUALS(atou64n_unit("1234M", 5, &val), 0);
	ASSERT_EQUALS(val, 1234000000);

	ASSERT_EQUALS(atou64n_unit("1234G", 5, &val), 0);
	ASSERT_EQUALS(val, 1234000000000);

	ASSERT_EQUALS(atou64n_unit("1234T", 5, &val), 0);
	ASSERT_EQUALS(val, 1234000000000000);

	ASSERT_EQUALS(atou64n_unit("1234P", 5, &val), 0);
	ASSERT_EQUALS(val, 1234000000000000000);
}
END_TEST()

UNIT_TEST_SUITE(ansi_extras_test)
{
	ADD_TEST(strreplace_test);
	ADD_TEST(int64_swap_test);
	ADD_TEST(int32_swap_test);
	ADD_TEST(int16_swap_test);
	ADD_TEST(escape_string_test);
	ADD_TEST(unescape_string_in_place_test);
	ADD_TEST(unescape_string_test);
	ADD_TEST(xdigitval_test);
	ADD_TEST(atou64n_test);
	ADD_TEST(atou64n_unit_test);
}
END_TEST_SUITE()
