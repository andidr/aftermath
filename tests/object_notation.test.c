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
#include "../src/object_notation.h"

#define test_object_notation_parse(str) \
	object_notation_parse(str, strlen(str))

/* Parse empty group */
UNIT_TEST(empty_group_test)
{
	struct object_notation_node* root;
	struct object_notation_node_group* root_group;

	root = test_object_notation_parse("foo {}");
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, OBJECT_NOTATION_NODE_TYPE_GROUP);

	root_group = (struct object_notation_node_group*)root;
	ASSERT_EQUALS_STRING(root_group->name, "foo");
	ASSERT_TRUE(list_empty(&root_group->members));
	ASSERT_EQUALS(object_notation_node_group_num_members(root_group), 0);

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse empty list */
UNIT_TEST(empty_list_test)
{
	struct object_notation_node* root;
	struct object_notation_node_list* root_list;

	root = test_object_notation_parse("[]");
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, OBJECT_NOTATION_NODE_TYPE_LIST);

	root_list = (struct object_notation_node_list*)root;
	ASSERT_TRUE(list_empty(&root_list->items));
	ASSERT_EQUALS(object_notation_node_list_num_items(root_list), 0);

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse empty string */
UNIT_TEST(empty_string_test)
{
	struct object_notation_node* root;
	struct object_notation_node_string* root_string;

	root = test_object_notation_parse("\"AAA\"");
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, OBJECT_NOTATION_NODE_TYPE_STRING);

	root_string = (struct object_notation_node_string*)root;
	ASSERT_EQUALS_STRING(root_string->value, "AAA");
	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse list of strings */
UNIT_TEST(string_list_test)
{
	struct object_notation_node* root;
	struct object_notation_node_list* root_list;
	struct object_notation_node* iter;
	struct object_notation_node_string* node_string;
	const char* strings[] = { "a", "b", "abc", "a   \" B", "c"};
	const char** str = strings;

	root = test_object_notation_parse("[\"a\", \"b\", \"abc\", \"a   \\\" B\", \"c\"]");
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, OBJECT_NOTATION_NODE_TYPE_LIST);

	root_list = (struct object_notation_node_list*)root;
	ASSERT_FALSE(list_empty(&root_list->items));
	ASSERT_EQUALS(object_notation_node_list_num_items(root_list), 5);

	object_notation_for_each_list_item(root_list, iter) {
		ASSERT_EQUALS(iter->type, OBJECT_NOTATION_NODE_TYPE_STRING);
		node_string = (struct object_notation_node_string*)iter;
		ASSERT_EQUALS_STRING(node_string->value, *str);
		str++;
	}

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse list of integers */
UNIT_TEST(int_list_test)
{
	struct object_notation_node* root;
	struct object_notation_node_list* root_list;
	struct object_notation_node* iter;
	struct object_notation_node_int* node_int;
	const uint64_t ints[] = { 1, 2, 3, 12345, 98102};
	const uint64_t* vals = ints;

	root = test_object_notation_parse("[1, 2, 3, 12345, 098102]");
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, OBJECT_NOTATION_NODE_TYPE_LIST);

	root_list = (struct object_notation_node_list*)root;
	ASSERT_FALSE(list_empty(&root_list->items));
	ASSERT_EQUALS(object_notation_node_list_num_items(root_list), 5);

	object_notation_for_each_list_item(root_list, iter) {
		ASSERT_EQUALS(iter->type, OBJECT_NOTATION_NODE_TYPE_INT);
		node_int = (struct object_notation_node_int*)iter;
		ASSERT_EQUALS(node_int->value, *vals);
		vals++;
	}

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse string with a single group with a single member */
UNIT_TEST(group_onemember_test)
{
	struct object_notation_node* root;
	struct object_notation_node_group* root_group;
	struct object_notation_node* member;
	struct object_notation_node_member* mmember;
	struct object_notation_node_string* member_str;

	root = test_object_notation_parse("foo {a: \"A\"}");
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, OBJECT_NOTATION_NODE_TYPE_GROUP);

	root_group = (struct object_notation_node_group*)root;
	ASSERT_EQUALS_STRING(root_group->name, "foo");
	ASSERT_FALSE(list_empty(&root_group->members));
	ASSERT_EQUALS(object_notation_node_group_num_members(root_group), 1);

	member = object_notation_node_group_first_member(root_group);
	ASSERT_NONNULL(member);
	ASSERT_EQUALS(member->type, OBJECT_NOTATION_NODE_TYPE_MEMBER);

	mmember = (struct object_notation_node_member*)member;
	ASSERT_EQUALS_STRING(mmember->name, "a");
	ASSERT_NONNULL(mmember->def);

	member_str = (struct object_notation_node_string*)mmember->def;
	ASSERT_EQUALS(member_str->node.type, OBJECT_NOTATION_NODE_TYPE_STRING);
	ASSERT_EQUALS_STRING(member_str->value, "A");

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse string with a single group with multiple members */
UNIT_TEST(group_nmembers_test)
{
	struct object_notation_node* root;
	struct object_notation_node_group* root_group;
	struct object_notation_node_member* mmember;
	struct object_notation_node_string* member_str;

	root = test_object_notation_parse("foo {a: \"A\","
					  "b: \"B\","
					  "c: \"C\","
					  "d: \"D\","
					  "e: \"E\","
					  "f: \"F\","
					  "g: \"G\","
					  "h: \"H\","
					  "i: \"I\","
					  "j: \"J\" }");
	const char* member_names[] = {
		"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"
	};

	const char* member_vals[] = {
		"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"
	};

	int i = 0;

	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, OBJECT_NOTATION_NODE_TYPE_GROUP);

	root_group = (struct object_notation_node_group*)root;
	ASSERT_EQUALS_STRING(root_group->name, "foo");
	ASSERT_FALSE(list_empty(&root_group->members));
	ASSERT_EQUALS(object_notation_node_group_num_members(root_group), 10);

	object_notation_for_each_group_member(root_group, mmember) {
		ASSERT_EQUALS(mmember->node.type, OBJECT_NOTATION_NODE_TYPE_MEMBER);
		ASSERT_EQUALS_STRING(mmember->name, member_names[i]);
		ASSERT_NONNULL(mmember->def);

		member_str = (struct object_notation_node_string*)mmember->def;
		ASSERT_EQUALS(member_str->node.type, OBJECT_NOTATION_NODE_TYPE_STRING);
		ASSERT_EQUALS_STRING(member_str->value, member_vals[i]);

		i++;
	}

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse object notation with multiple types and nested nodes */
UNIT_TEST(mix_parse_test)
{
	const char* str = "group_a {\n"
		"	int_list: [1, 2, 3, 4],\n"
		"	mixed_list: [1, 2, \"ABC\", message { str: \"foo\", type: \"text\" }],\n"
		"\n"
		"	ig: inner_group {\n"
		"		group_member: another_group {\n"
		"			value: 5\n"
		"		}\n"
		"	}\n"
		"}";

	struct object_notation_node* root;
	struct object_notation_node_group* root_group;

	root = test_object_notation_parse(str);
	ASSERT_NONNULL(root);
	ASSERT_EQUALS(root->type, OBJECT_NOTATION_NODE_TYPE_GROUP);

	root_group = (struct object_notation_node_group*)root;
	ASSERT_EQUALS(object_notation_node_group_num_members(root_group), 3);

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse 100 nested groups */
UNIT_TEST(deep_nesting_test)
{
	size_t ngroups = 100;
	struct object_notation_node* root;
	struct object_notation_node_group* node_group;
	struct object_notation_node_int* node_int;
	struct object_notation_node* member;
	struct object_notation_node_member* mmember;
	size_t len = (strlen("groupXXX{nest:")+strlen("}"))*ngroups+2;
	char* str = malloc(len);
	char* curr = str;

	ASSERT_NONNULL(str);

	/* Build string containing
	 * group001{nest:group002{nest:...groupn-1{nest:1 */
	for(size_t i = 0; i < ngroups; i++) {
		if(i == ngroups-1) {
			sprintf(curr, "group%03zu{nest:1", i);
			curr += 15;
		} else {
			sprintf(curr, "group%03zu{nest:", i);
			curr += 14;
		}
	}

	/* Add closing braces */
	for(size_t i = 0; i < ngroups; i++) {
		*curr = '}';
		curr++;
	}

	*curr = '\0';

	root = test_object_notation_parse(str);
	ASSERT_NONNULL(root);

	node_group = (struct object_notation_node_group*)root;

	/* Check nested groups */
	for(size_t i = 0; i < ngroups-1; i++) {
		member = object_notation_node_group_first_member(node_group);
		ASSERT_EQUALS(object_notation_node_group_num_members(node_group), 1);
		ASSERT_NONNULL(member);

		ASSERT_EQUALS(member->type, OBJECT_NOTATION_NODE_TYPE_MEMBER);
		mmember = (struct object_notation_node_member*)member;
		ASSERT_EQUALS_STRING(mmember->name, "nest");

		ASSERT_NONNULL(mmember->def);
		ASSERT_EQUALS(mmember->def->type, OBJECT_NOTATION_NODE_TYPE_GROUP);
		node_group = (struct object_notation_node_group*)mmember->def;
	}

	/* Check innermost node */
	member = object_notation_node_group_first_member(node_group);
	ASSERT_NONNULL(member);

	ASSERT_EQUALS(member->type, OBJECT_NOTATION_NODE_TYPE_MEMBER);
	mmember = (struct object_notation_node_member*)member;
	ASSERT_EQUALS_STRING(mmember->name, "nest");
	ASSERT_EQUALS(mmember->def->type, OBJECT_NOTATION_NODE_TYPE_INT);
	node_int = (struct object_notation_node_int*)mmember->def;
	ASSERT_EQUALS(node_int->value, 1);

	free(str);
	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse string with a single group with multiple members and call
 * object_notation_node_group_check_members */
UNIT_TEST(has_members_test)
{
	struct object_notation_node* root;
	struct object_notation_node_group* root_group;

	root = test_object_notation_parse("foo {a: \"A\","
					  "b: \"B\","
					  "c: \"C\","
					  "d: \"D\","
					  "e: \"E\","
					  "f: \"F\","
					  "g: \"G\","
					  "h: \"H\","
					  "i: \"I\","
					  "j: \"J\" }");

	ASSERT_NONNULL(root);
	root_group = (struct object_notation_node_group*)root;
	ASSERT_TRUE(object_notation_node_group_has_members(root_group, 0, "a", NULL));
	ASSERT_FALSE(object_notation_node_group_has_members(root_group, 1, "a", NULL));

	ASSERT_TRUE(object_notation_node_group_has_members(root_group, 0,
	       "a", "b", "c", "d", NULL));
	ASSERT_FALSE(object_notation_node_group_has_members(root_group, 1,
	       "a", "b", "c", "d", NULL));

	ASSERT_TRUE(object_notation_node_group_has_members(root_group, 0,
	       "a", "d", NULL));
	ASSERT_FALSE(object_notation_node_group_has_members(root_group, 1,
	       "a", "d", NULL));

	ASSERT_TRUE(object_notation_node_group_has_members(root_group, 0,
	       "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", NULL));
	ASSERT_TRUE(object_notation_node_group_has_members(root_group, 1,
	       "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", NULL));

	ASSERT_FALSE(object_notation_node_group_has_members(root_group, 0,
	       "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", NULL));
	ASSERT_FALSE(object_notation_node_group_has_members(root_group, 1,
	       "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", NULL));

	ASSERT_FALSE(object_notation_node_group_has_members(root_group, 0,
	       "k", NULL));
	ASSERT_FALSE(object_notation_node_group_has_members(root_group, 1,
	       "k", NULL));

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

/* Parse string with a single group with exactly 64 member (the
 * maximum number of names allowed by
 * object_notation_node_group_check_members) and call
 * object_notation_node_group_check_members */
UNIT_TEST(has_members_test64)
{
	struct object_notation_node* root;
	struct object_notation_node_group* root_group;

	root = test_object_notation_parse("foo {"
					  "m0: 0, m1: 1, m2: 2, m3: 3,"
					  "m4: 4, m5: 5, m6: 6, m7: 7,"
					  "m8: 8, m9: 9, m10: 10, m11: 11,"
					  "m12: 12, m13: 13, m14: 14, m15: 15,"
					  "m16: 16, m17: 17, m18: 18, m19: 19,"
					  "m20: 20, m21: 21, m22: 22, m23: 23,"
					  "m24: 24, m25: 25, m26: 26, m27: 27,"
					  "m28: 28, m29: 29, m30: 30, m31: 31,"
					  "m32: 32, m33: 33, m34: 34, m35: 35,"
					  "m36: 36, m37: 37, m38: 38, m39: 39,"
					  "m40: 40, m41: 41, m42: 42, m43: 43,"
					  "m44: 44, m45: 45, m46: 46, m47: 47,"
					  "m48: 48, m49: 49, m50: 50, m51: 51,"
					  "m52: 52, m53: 53, m54: 54, m55: 55,"
					  "m56: 56, m57: 57, m58: 58, m59: 59,"
					  "m60: 60, m61: 61, m62: 62, m63: 63"
					  "}");

	ASSERT_NONNULL(root);
	root_group = (struct object_notation_node_group*)root;

	ASSERT_TRUE(object_notation_node_group_has_members(root_group, 0,
		"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7",
		"m8", "m9", "m10", "m11", "m12", "m13", "m14", "m15",
		"m16","m17", "m18", "m19", "m20", "m21", "m22", "m23",
		"m24", "m25", "m26", "m27", "m28", "m29", "m30", "m31",
		"m32", "m33", "m34", "m35", "m36", "m37", "m38", "m39",
		"m40", "m41", "m42", "m43", "m44", "m45", "m46", "m47",
		"m48", "m49", "m50", "m51", "m52", "m53", "m54", "m55",
		"m56", "m57", "m58", "m59", "m60", "m61", "m62", "m63",
		NULL));
	ASSERT_TRUE(object_notation_node_group_has_members(root_group, 1,
		"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7",
		"m8", "m9", "m10", "m11", "m12", "m13", "m14", "m15",
		"m16","m17", "m18", "m19", "m20", "m21", "m22", "m23",
		"m24", "m25", "m26", "m27", "m28", "m29", "m30", "m31",
		"m32", "m33", "m34", "m35", "m36", "m37", "m38", "m39",
		"m40", "m41", "m42", "m43", "m44", "m45", "m46", "m47",
		"m48", "m49", "m50", "m51", "m52", "m53", "m54", "m55",
		"m56", "m57", "m58", "m59", "m60", "m61", "m62", "m63",
		NULL));

	object_notation_node_destroy(root);
	free(root);

	root = test_object_notation_parse("foo {"
					  "m0: 0, m1: 1, m2: 2, m3: 3,"
					  "m4: 4, m5: 5, m6: 6, m7: 7,"
					  "m8: 8, m9: 9, m10: 10, m11: 11,"
					  "m12: 12, m13: 13, m14: 14, m15: 15,"
					  "m16: 16, m17: 17, m18: 18, m19: 19,"
					  "m20: 20, m21: 21, m22: 22, m23: 23,"
					  "m24: 24, m25: 25, m26: 26, m27: 27,"
					  "m28: 28, m29: 29, m30: 30, m31: 31,"
					  "m32: 32, m33: 33, m34: 34, m35: 35,"
					  "m36: 36, m37: 37, m38: 38, m39: 39,"
					  "m40: 40, m41: 41, m42: 42, m43: 43,"
					  "m44: 44, m45: 45, m46: 46, m47: 47,"
					  "m48: 48, m49: 49, m50: 50, m51: 51,"
					  "m52: 52, m53: 53, m54: 54, m55: 55,"
					  "m56: 56, m57: 57, m58: 58, m59: 59,"
					  "m60: 60, m61: 61, m62: 62, m63: 63,"
					  "m65: 65"
					  "}");

	ASSERT_NONNULL(root);
	root_group = (struct object_notation_node_group*)root;

	ASSERT_FALSE(object_notation_node_group_has_members(root_group, 0,
		"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7",
		"m8", "m9", "m10", "m11", "m12", "m13", "m14", "m15",
		"m16","m17", "m18", "m19", "m20", "m21", "m22", "m23",
		"m24", "m25", "m26", "m27", "m28", "m29", "m30", "m31",
		"m32", "m33", "m34", "m35", "m36", "m37", "m38", "m39",
		"m40", "m41", "m42", "m43", "m44", "m45", "m46", "m47",
		"m48", "m49", "m50", "m51", "m52", "m53", "m54", "m55",
		"m56", "m57", "m58", "m59", "m60", "m61", "m62", "m63",
		"m65", NULL));

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

UNIT_TEST(get_member_def_test)
{
	struct object_notation_node* root;
	struct object_notation_node_group* root_group;
	struct object_notation_node* member;
	struct object_notation_node_string* member_str;

	root = test_object_notation_parse("foo {a: \"A\","
					  "b: \"B\","
					  "c: \"C\""
					  " }");

	ASSERT_NONNULL(root);
	root_group = (struct object_notation_node_group*)root;

	member = object_notation_node_group_get_member_def(root_group, "a");
	ASSERT_NONNULL(member);
	ASSERT_EQUALS(member->type, OBJECT_NOTATION_NODE_TYPE_STRING);
	member_str = (struct object_notation_node_string*)member;
	ASSERT_EQUALS_STRING(member_str->value, "A");

	member = object_notation_node_group_get_member_def(root_group, "b");
	ASSERT_NONNULL(member);
	ASSERT_EQUALS(member->type, OBJECT_NOTATION_NODE_TYPE_STRING);
	member_str = (struct object_notation_node_string*)member;
	ASSERT_EQUALS_STRING(member_str->value, "B");

	member = object_notation_node_group_get_member_def(root_group, "c");
	ASSERT_NONNULL(member);
	ASSERT_EQUALS(member->type, OBJECT_NOTATION_NODE_TYPE_STRING);
	member_str = (struct object_notation_node_string*)member;
	ASSERT_EQUALS_STRING(member_str->value, "C");

	object_notation_node_destroy(root);
	free(root);
}
END_TEST()

UNIT_TEST_SUITE(object_notation_test)
{
	ADD_TEST(empty_group_test);
	ADD_TEST(empty_list_test);
	ADD_TEST(empty_string_test);
	ADD_TEST(string_list_test);
	ADD_TEST(int_list_test);
	ADD_TEST(group_onemember_test);
	ADD_TEST(group_nmembers_test);
	ADD_TEST(mix_parse_test);
	ADD_TEST(deep_nesting_test);
	ADD_TEST(has_members_test);
	ADD_TEST(has_members_test64);
	ADD_TEST(get_member_def_test);
}
END_TEST_SUITE()
