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
#include "common.h"
#include "../src/color_scheme.h"

#define test_color_scheme_set_from_string(css, str)	\
	color_scheme_set_from_string(css, str, strlen(str))

/* Initialize a color scheme rule and destroy it */
UNIT_TEST(color_scheme_rule_init_test)
{
	struct color_scheme_rule csr;

	ASSERT_EQUALS(color_scheme_rule_init(&csr, COLOR_SCHEME_RULE_TYPE_TASKNAME, "foo"), 0);
	color_scheme_rule_destroy(&csr);
}
END_TEST()

/* Initialize a color scheme rule, change regex and destroy it */
UNIT_TEST(color_scheme_rule_set_regex_test)
{
	struct color_scheme_rule csr;

	ASSERT_EQUALS(color_scheme_rule_init(&csr, COLOR_SCHEME_RULE_TYPE_TASKNAME, "foo"), 0);
	ASSERT_EQUALS(color_scheme_rule_set_regex(&csr, "abc"), 0);
	ASSERT_EQUALS_STRING(csr.regex, "abc");
	ASSERT_EQUALS(color_scheme_rule_set_regex(&csr, "some longer string"), 0);
	ASSERT_EQUALS_STRING(csr.regex, "some longer string");
	color_scheme_rule_destroy(&csr);
}
END_TEST()

/* Initialize a color scheme and destroy it */
UNIT_TEST(color_scheme_init_test)
{
	struct color_scheme cs;

	ASSERT_EQUALS(color_scheme_init(&cs, "foo"), 0);
	color_scheme_destroy(&cs);
}
END_TEST()

/* Initialize a color scheme, change name and destroy it */
UNIT_TEST(color_scheme_set_name_test)
{
	struct color_scheme cs;

	ASSERT_EQUALS(color_scheme_init(&cs, "foo"), 0);
	ASSERT_EQUALS(color_scheme_set_name(&cs, "bar"), 0);
	ASSERT_EQUALS_STRING(cs.name, "bar");
	ASSERT_EQUALS(color_scheme_set_name(&cs, "some longer string"), 0);
	ASSERT_EQUALS_STRING(cs.name, "some longer string");
	color_scheme_destroy(&cs);
}
END_TEST()

/* Initialize a color scheme and add a color scheme rule */
UNIT_TEST(color_scheme_add_rule_test)
{
	struct color_scheme cs;
	struct color_scheme_rule* csr = malloc(sizeof(struct color_scheme_rule));

	ASSERT_NONNULL(csr);

	ASSERT_EQUALS(color_scheme_init(&cs, "foo"), 0);
	ASSERT_EQUALS(color_scheme_rule_init(csr, COLOR_SCHEME_RULE_TYPE_TASKNAME, "foo"), 0);
	ASSERT_EQUALS(color_scheme_add_color_scheme_rule(&cs, csr), 0);
	color_scheme_destroy(&cs);
}
END_TEST()

/* Initialize a color scheme, add a color scheme rule and remove it */
UNIT_TEST(color_scheme_add_remove_rule_test)
{
	struct color_scheme cs;
	struct color_scheme_rule* csr = malloc(sizeof(struct color_scheme_rule));

	ASSERT_NONNULL(csr);

	ASSERT_EQUALS(color_scheme_init(&cs, "foo"), 0);
	ASSERT_EQUALS(color_scheme_rule_init(csr, COLOR_SCHEME_RULE_TYPE_TASKNAME, "foo"), 0);
	ASSERT_EQUALS(color_scheme_add_color_scheme_rule(&cs, csr), 0);
	ASSERT_EQUALS(color_scheme_remove_color_scheme_rule(&cs, csr), 0);
	ASSERT_EQUALS(cs.num_rules, 0);

	color_scheme_rule_destroy(csr);
	free(csr);
	color_scheme_destroy(&cs);
}
END_TEST()

/* Initialize a color scheme set and destroy it */
UNIT_TEST(color_scheme_set_init_test)
{
	struct color_scheme_set css;

	/* No assertions; for valgrind only */
	color_scheme_set_init(&css);
	color_scheme_set_destroy(&css);
}
END_TEST()

/* Initialize a color scheme set and add a color scheme */
UNIT_TEST(color_scheme_set_add_scheme_test)
{
	struct color_scheme_set css;
	struct color_scheme* cs = malloc(sizeof(struct color_scheme));

	ASSERT_NONNULL(cs);
	ASSERT_EQUALS(color_scheme_init(cs, "foo"), 0);

	color_scheme_set_init(&css);
	ASSERT_EQUALS(color_scheme_set_add_color_scheme(&css, cs), 0);
	color_scheme_set_destroy(&css);
}
END_TEST()

/* Initialize a color scheme set, add a color scheme and remove it */
UNIT_TEST(color_scheme_set_add_remove_scheme_test)
{
	struct color_scheme_set css;
	struct color_scheme* cs = malloc(sizeof(struct color_scheme));

	ASSERT_NONNULL(cs);
	ASSERT_EQUALS(color_scheme_init(cs, "foo"), 0);

	color_scheme_set_init(&css);
	ASSERT_EQUALS(color_scheme_set_add_color_scheme(&css, cs), 0);
	ASSERT_EQUALS(color_scheme_set_remove_color_scheme(&css, cs), 0);
	color_scheme_set_destroy(&css);

	color_scheme_destroy(cs);
	free(cs);
}
END_TEST()

/* Save a color scheme to a file */
UNIT_TEST(color_scheme_set_save_test)
{
	struct color_scheme_set css;
	struct color_scheme_rule* csr1 = malloc(sizeof(struct color_scheme_rule));
	struct color_scheme_rule* csr2 = malloc(sizeof(struct color_scheme_rule));
	struct color_scheme_rule* csr3 = malloc(sizeof(struct color_scheme_rule));
	struct color_scheme* cs1 = malloc(sizeof(struct color_scheme));
	struct color_scheme* cs2 = malloc(sizeof(struct color_scheme));

	char tmpname[] = "test-tmp/color_schemeXXXXXX";
	FILE* fp = tmpfile_template(tmpname, "w+");

	ASSERT_NONNULL(fp);
	ASSERT_NONNULL(csr1);
	ASSERT_NONNULL(csr2);
	ASSERT_NONNULL(csr3);
	ASSERT_NONNULL(cs1);
	ASSERT_NONNULL(cs2);

	ASSERT_EQUALS(color_scheme_rule_init(csr1, COLOR_SCHEME_RULE_TYPE_TASKNAME, "[A-Za-z0-9]{2,10}"), 0);
	color_scheme_rule_set_color(csr1, 1.0, 0.0, 0.0);

	ASSERT_EQUALS(color_scheme_rule_init(csr2, COLOR_SCHEME_RULE_TYPE_STATENAME, "[^0-9]+"), 0);
	color_scheme_rule_set_color(csr2, 0.0, 1.0, 0.0);

	ASSERT_EQUALS(color_scheme_rule_init(csr3, COLOR_SCHEME_RULE_TYPE_COUNTERNAME, "^my_counter\"$"), 0);
	color_scheme_rule_set_color(csr3, 0.0, 0.0, 1.0);

	ASSERT_EQUALS(color_scheme_init(cs1, "CS1"), 0);
	ASSERT_EQUALS(color_scheme_add_color_scheme_rule(cs1, csr1), 0);
	ASSERT_EQUALS(color_scheme_add_color_scheme_rule(cs1, csr2), 0);

	ASSERT_EQUALS(color_scheme_init(cs2, "CS2"), 0);
	ASSERT_EQUALS(color_scheme_add_color_scheme_rule(cs2, csr3), 0);

	color_scheme_set_init(&css);
	ASSERT_EQUALS(color_scheme_set_add_color_scheme(&css, cs1), 0);
	ASSERT_EQUALS(color_scheme_set_add_color_scheme(&css, cs2), 0);

	ASSERT_EQUALS(color_scheme_set_save_fp(&css, fp), 0);

	color_scheme_set_destroy(&css);

	ASSERT_EQUALS(fclose(fp), 0);
	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

/* Build a color scheme set from an empty set */
UNIT_TEST(color_scheme_set_from_string_test_empty_set)
{
	struct color_scheme_set css;

	color_scheme_set_init(&css);
	ASSERT_EQUALS(test_color_scheme_set_from_string(&css, "color_scheme_set { schemes: []}"), 0);
	color_scheme_set_destroy(&css);
}
END_TEST()

/* Build a color scheme set from a string with empty color schemes */
UNIT_TEST(color_scheme_set_from_string_test_empty_schemes)
{
	struct color_scheme_set css;

	color_scheme_set_init(&css);
	ASSERT_EQUALS(test_color_scheme_set_from_string(&css, "color_scheme_set {\n"
							"  schemes: [\n"
							"    color_scheme {\n"
							"       name: \"CS1\",\n"
							"       rules: []\n"
							"    },\n"
							"    color_scheme {\n"
							"       name: \"CS2\",\n"
							"       rules: []\n"
							"    }]\n"
							"}"), 0);
	color_scheme_set_destroy(&css);
}
END_TEST()

/* Build a color scheme set from a string */
UNIT_TEST(color_scheme_set_from_string_test)
{
	struct color_scheme_set css;

	color_scheme_set_init(&css);
	ASSERT_EQUALS(test_color_scheme_set_from_string(&css, "color_scheme_set {\n"
							"  schemes: [\n"
							"    color_scheme {\n"
							"       name: \"CS1\",\n"
							"       rules: [\n"
							"         color_scheme_rule {\n"
							"           type: \"task_symbol\",\n"
							"           regex: \"init.*\",\n"
							"           color: \"#123456\"\n"
							"         }\n"
							"       ]\n"
							"    },\n"
							"    color_scheme {\n"
							"       name: \"CS2\",\n"
							"       rules: [\n"
							"         color_scheme_rule {\n"
							"           type: \"task_symbol\",\n"
							"           regex: \"init.*\",\n"
							"           color: \"#123456\"\n"
							"         },\n"
							"         color_scheme_rule {\n"
							"           type: \"state_name\",\n"
							"           regex: \"init.*\",\n"
							"           color: \"#123456\"\n"
							"         },\n"
							"         color_scheme_rule {\n"
							"           type: \"counter_name\",\n"
							"           regex: \"init.*\",\n"
							"           color: \"#fff\"\n"
							"         }\n"
							"       ]\n"
							"    }]\n"
							"}"), 0);
	color_scheme_set_destroy(&css);
}
END_TEST()

/* Save a color scheme to a file and reload it afterwards */
UNIT_TEST(color_scheme_set_save_load_test)
{
	struct color_scheme_set css;
	struct color_scheme_set css2;
	struct color_scheme_rule* csr1 = malloc(sizeof(struct color_scheme_rule));
	struct color_scheme_rule* csr2 = malloc(sizeof(struct color_scheme_rule));
	struct color_scheme_rule* csr3 = malloc(sizeof(struct color_scheme_rule));
	struct color_scheme* cs1 = malloc(sizeof(struct color_scheme));
	struct color_scheme* cs2 = malloc(sizeof(struct color_scheme));

	char tmpname[] = "test-tmp/color_schemeXXXXXX";
	FILE* fp = tmpfile_template(tmpname, "w+");

	ASSERT_NONNULL(fp);
	ASSERT_NONNULL(csr1);
	ASSERT_NONNULL(csr2);
	ASSERT_NONNULL(csr3);
	ASSERT_NONNULL(cs1);
	ASSERT_NONNULL(cs2);

	ASSERT_EQUALS(color_scheme_rule_init(csr1, COLOR_SCHEME_RULE_TYPE_TASKNAME, "[A-Za-z0-9]{2,10}"), 0);
	color_scheme_rule_set_color(csr1, 1.0, 0.0, 0.0);

	ASSERT_EQUALS(color_scheme_rule_init(csr2, COLOR_SCHEME_RULE_TYPE_STATENAME, "[^0-9]+"), 0);
	color_scheme_rule_set_color(csr2, 0.0, 1.0, 0.0);

	ASSERT_EQUALS(color_scheme_rule_init(csr3, COLOR_SCHEME_RULE_TYPE_COUNTERNAME, "^my_counter\"$"), 0);
	color_scheme_rule_set_color(csr3, 0.0, 0.0, 1.0);

	ASSERT_EQUALS(color_scheme_init(cs1, "CS1"), 0);
	ASSERT_EQUALS(color_scheme_add_color_scheme_rule(cs1, csr1), 0);
	ASSERT_EQUALS(color_scheme_add_color_scheme_rule(cs1, csr2), 0);

	ASSERT_EQUALS(color_scheme_init(cs2, "CS2"), 0);
	ASSERT_EQUALS(color_scheme_add_color_scheme_rule(cs2, csr3), 0);

	color_scheme_set_init(&css);
	ASSERT_EQUALS(color_scheme_set_add_color_scheme(&css, cs1), 0);
	ASSERT_EQUALS(color_scheme_set_add_color_scheme(&css, cs2), 0);

	ASSERT_EQUALS(color_scheme_set_save_fp(&css, fp), 0);

	color_scheme_set_destroy(&css);

	ASSERT_EQUALS(fclose(fp), 0);

	color_scheme_set_init(&css2);
	ASSERT_EQUALS(color_scheme_set_load(&css2, tmpname), 0);

	ASSERT_EQUALS(css2.num_schemes, 2);
	ASSERT_EQUALS_STRING(css2.schemes[0]->name, "CS1");
	ASSERT_EQUALS_STRING(css2.schemes[1]->name, "CS2");

	ASSERT_EQUALS(css2.schemes[0]->num_rules, 2);
	ASSERT_EQUALS(css2.schemes[1]->num_rules, 1);

	ASSERT_EQUALS_STRING(css2.schemes[0]->rules[0]->regex, "[A-Za-z0-9]{2,10}");
	ASSERT_EQUALS(css2.schemes[0]->rules[0]->type, COLOR_SCHEME_RULE_TYPE_TASKNAME);
	ASSERT_EQUALS(css2.schemes[0]->rules[0]->color_r, 1.0);
	ASSERT_EQUALS(css2.schemes[0]->rules[0]->color_g, 0.0);
	ASSERT_EQUALS(css2.schemes[0]->rules[0]->color_b, 0.0);

	ASSERT_EQUALS_STRING(css2.schemes[0]->rules[1]->regex, "[^0-9]+");
	ASSERT_EQUALS(css2.schemes[0]->rules[1]->type, COLOR_SCHEME_RULE_TYPE_STATENAME);
	ASSERT_EQUALS(css2.schemes[0]->rules[1]->color_r, 0.0);
	ASSERT_EQUALS(css2.schemes[0]->rules[1]->color_g, 1.0);
	ASSERT_EQUALS(css2.schemes[0]->rules[1]->color_b, 0.0);

	ASSERT_EQUALS_STRING(css2.schemes[1]->rules[0]->regex, "^my_counter\"$");
	ASSERT_EQUALS(css2.schemes[1]->rules[0]->type, COLOR_SCHEME_RULE_TYPE_COUNTERNAME);
	ASSERT_EQUALS(css2.schemes[1]->rules[0]->color_r, 0.0);
	ASSERT_EQUALS(css2.schemes[1]->rules[0]->color_g, 0.0);
	ASSERT_EQUALS(css2.schemes[1]->rules[0]->color_b, 1.0);

	color_scheme_set_destroy(&css2);

	ASSERT_EQUALS(unlink(tmpname), 0);
}
END_TEST()

UNIT_TEST_SUITE(color_scheme_test)
{
	ADD_TEST(color_scheme_rule_init_test);
	ADD_TEST(color_scheme_rule_set_regex_test);

	ADD_TEST(color_scheme_init_test);
	ADD_TEST(color_scheme_set_name_test);
	ADD_TEST(color_scheme_add_rule_test);
	ADD_TEST(color_scheme_add_remove_rule_test);

	ADD_TEST(color_scheme_set_init_test);
	ADD_TEST(color_scheme_set_add_scheme_test);
	ADD_TEST(color_scheme_set_add_remove_scheme_test);

	ADD_TEST(color_scheme_set_save_test);
	ADD_TEST(color_scheme_set_from_string_test_empty_set);
	ADD_TEST(color_scheme_set_from_string_test_empty_schemes);
	ADD_TEST(color_scheme_set_from_string_test);

	ADD_TEST(color_scheme_set_save_load_test);
}
END_TEST_SUITE()
