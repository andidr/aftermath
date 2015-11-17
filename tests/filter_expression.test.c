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
#include "../src/filter_expression.h"

#define INT_EXPR_FILTER_TEST(prefix, fun)				 \
	UNIT_TEST(prefix##_filter_expression_test)			 \
	{								 \
		ASSERT_FALSE(filter_is_valid_expression(fun "(5)"));     \
		ASSERT_FALSE(filter_is_valid_expression(fun "(5K)"));    \
		ASSERT_FALSE(filter_is_valid_expression(fun "(5M)"));    \
		ASSERT_FALSE(filter_is_valid_expression(fun "(5G)"));    \
		ASSERT_FALSE(filter_is_valid_expression(fun "(5T)"));    \
		ASSERT_FALSE(filter_is_valid_expression(fun "(5P)"));    \
		ASSERT_TRUE(filter_is_valid_expression(" !" fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("! " fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun " (5)")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "( 5)")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(5 )")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(5) ")); \
		ASSERT_TRUE(filter_is_valid_expression(" !" fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("! " fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun " (5)")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "( 5)")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(5 )")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(5) ")); \
									 \
		ASSERT_TRUE(filter_is_valid_expression(" +" fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("+ " fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun " (5)")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "( 5)")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "(5 )")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "(5) ")); \
		ASSERT_TRUE(filter_is_valid_expression(" +" fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("+ " fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun " (5)")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "( 5)")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "(5 )")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "(5) ")); \
									 \
		ASSERT_TRUE(filter_is_valid_expression(" -" fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("- " fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun " (5)")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "( 5)")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "(5 )")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "(5) ")); \
		ASSERT_TRUE(filter_is_valid_expression(" -" fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("- " fun "(5)")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun " (5)")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "( 5)")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "(5 )")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "(5) ")); \
	}								 \
	END_TEST()

#define INT_INTERVAL_EXPR_FILTER_TEST(prefix, fun)			     \
	UNIT_TEST(prefix##_filter_expression_test)			     \
	{								     \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0-5])"));  \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0K-5K])"));\
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0M-5M])"));\
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0G-5G])"));\
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0T-5T])"));\
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0P-5P])"));\
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0-5K])")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0K-5M])"));\
		ASSERT_TRUE(filter_is_valid_expression(" !" fun "([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("! " fun "([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun " ([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "( [0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([ 0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0 -5])")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0- 5])")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0-5 ])")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0-5] )")); \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "([0-5]) ")); \
									     \
		ASSERT_TRUE(filter_is_valid_expression(" +" fun "([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("+ " fun "([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun " ([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "( [0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "([ 0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "([0 -5])")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "([0- 5])")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "([0-5 ])")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "([0-5] )")); \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "([0-5]) ")); \
									     \
		ASSERT_TRUE(filter_is_valid_expression(" -" fun "([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("- " fun "([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun " ([0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "( [0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "([ 0-5])")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "([0 -5])")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "([0- 5])")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "([0-5 ])")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "([0-5] )")); \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "([0-5]) ")); \
	}								     \
	END_TEST()

#define INT_INTERVAL_EXPR_NOMODE_FILTER_TEST(prefix, fun)		     \
	UNIT_TEST(prefix##_filter_expression_test)			     \
	{								     \
		ASSERT_TRUE(filter_is_valid_expression(fun "([0-5])"));      \
		ASSERT_TRUE(filter_is_valid_expression(fun " ([0-5])"));     \
		ASSERT_TRUE(filter_is_valid_expression(fun "( [0-5])"));     \
		ASSERT_TRUE(filter_is_valid_expression(fun "([ 0-5])"));     \
		ASSERT_TRUE(filter_is_valid_expression(fun "([0 -5])"));     \
		ASSERT_TRUE(filter_is_valid_expression(fun "([0- 5])"));     \
		ASSERT_TRUE(filter_is_valid_expression(fun "([0-5 ])"));     \
		ASSERT_TRUE(filter_is_valid_expression(fun "([0-5] )"));     \
		ASSERT_TRUE(filter_is_valid_expression(fun "([0-5]) "));     \
									     \
		ASSERT_FALSE(filter_is_valid_expression("!" fun "([0-5])")); \
		ASSERT_FALSE(filter_is_valid_expression("+" fun "([0-5])")); \
		ASSERT_FALSE(filter_is_valid_expression("-" fun "([0-5])")); \
	}								     \
	END_TEST()

#define STR_REGEX_EXPR_FILTER_TEST(prefix, fun)                                \
	UNIT_TEST(prefix##_filter_expression_test)                             \
	{					                               \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(\"\")"));     \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(\"A\")"));    \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "(\"A\")"));    \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "(\"A\")"));    \
									       \
		ASSERT_TRUE(filter_is_valid_expression(" !" fun "(\"A\")"));   \
		ASSERT_TRUE(filter_is_valid_expression("! " fun "(\"A\")"));   \
		ASSERT_TRUE(filter_is_valid_expression("!" fun " (\"A\")"));   \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "( \"A\")"));   \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(\"A\" )"));   \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(\"A\") "));   \
									       \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(r\"\")"));    \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(r\"A\")"));   \
		ASSERT_TRUE(filter_is_valid_expression("+" fun "(r\"A\")"));   \
		ASSERT_TRUE(filter_is_valid_expression("-" fun "(r\"A\")"));   \
									       \
		ASSERT_TRUE(filter_is_valid_expression(" !" fun "(r\"A\")"));  \
		ASSERT_TRUE(filter_is_valid_expression("! " fun "(r\"A\")"));  \
		ASSERT_TRUE(filter_is_valid_expression("!" fun " (r\"A\")"));  \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "( r\"A\")"));  \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(r\"A\" )"));  \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(r\"A\") "));  \
									       \
		ASSERT_TRUE(filter_is_valid_expression("!" fun "(\"A[\")"));   \
		ASSERT_FALSE(filter_is_valid_expression("!" fun "(r\"A[\")")); \
	}								       \
	END_TEST()

INT_EXPR_FILTER_TEST(cpu_int, "cpu")
INT_INTERVAL_EXPR_FILTER_TEST(cpu_int_interval, "cpu")
INT_INTERVAL_EXPR_NOMODE_FILTER_TEST(tasklen, "tasklen")
STR_REGEX_EXPR_FILTER_TEST(task, "task")
STR_REGEX_EXPR_FILTER_TEST(counter, "counter")

UNIT_TEST(event_filter_expression_test)
{
	ASSERT_TRUE(filter_is_valid_expression("!event(tcreate)"));
	ASSERT_TRUE(filter_is_valid_expression(" !event(tcreate)"));
	ASSERT_TRUE(filter_is_valid_expression("! event(tcreate)"));
	ASSERT_TRUE(filter_is_valid_expression("!event (tcreate)"));
	ASSERT_TRUE(filter_is_valid_expression("!event( tcreate)"));
	ASSERT_TRUE(filter_is_valid_expression("!event(tcreate )"));
	ASSERT_TRUE(filter_is_valid_expression("!event(tcreate) "));

	ASSERT_TRUE(filter_is_valid_expression("!event(texec_start)"));
	ASSERT_TRUE(filter_is_valid_expression(" !event(texec_start)"));
	ASSERT_TRUE(filter_is_valid_expression("! event(texec_start)"));
	ASSERT_TRUE(filter_is_valid_expression("!event (texec_start)"));
	ASSERT_TRUE(filter_is_valid_expression("!event( texec_start)"));
	ASSERT_TRUE(filter_is_valid_expression("!event(texec_start )"));
	ASSERT_TRUE(filter_is_valid_expression("!event(texec_start) "));

	ASSERT_TRUE(filter_is_valid_expression("!event(texec_end)"));
	ASSERT_TRUE(filter_is_valid_expression(" !event(texec_end)"));
	ASSERT_TRUE(filter_is_valid_expression("! event(texec_end)"));
	ASSERT_TRUE(filter_is_valid_expression("!event (texec_end)"));
	ASSERT_TRUE(filter_is_valid_expression("!event( texec_end)"));
	ASSERT_TRUE(filter_is_valid_expression("!event(texec_end )"));
	ASSERT_TRUE(filter_is_valid_expression("!event(texec_end) "));

	ASSERT_TRUE(filter_is_valid_expression("!event(tdestroy)"));
	ASSERT_TRUE(filter_is_valid_expression(" !event(tdestroy)"));
	ASSERT_TRUE(filter_is_valid_expression("! event(tdestroy)"));
	ASSERT_TRUE(filter_is_valid_expression("!event (tdestroy)"));
	ASSERT_TRUE(filter_is_valid_expression("!event( tdestroy)"));
	ASSERT_TRUE(filter_is_valid_expression("!event(tdestroy )"));
	ASSERT_TRUE(filter_is_valid_expression("!event(tdestroy) "));
}
END_TEST()

UNIT_TEST(combined_test)
{
	ASSERT_TRUE(filter_is_valid_expression("!event(tdestroy) !cpu(5)"));
	ASSERT_TRUE(filter_is_valid_expression("!event(tdestroy) !cpu(5) +cpu(6)"));
	ASSERT_TRUE(filter_is_valid_expression("!event(tdestroy) !cpu(5) +cpu([6-10])"));
}
END_TEST()

UNIT_TEST_SUITE(filter_expression_test)
{
	ADD_TEST(cpu_int_filter_expression_test);
	ADD_TEST(cpu_int_interval_filter_expression_test);
	ADD_TEST(tasklen_filter_expression_test);
	ADD_TEST(task_filter_expression_test);
	ADD_TEST(counter_filter_expression_test);
	ADD_TEST(event_filter_expression_test);
	ADD_TEST(combined_test);
}
END_TEST_SUITE()
