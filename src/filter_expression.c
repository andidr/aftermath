/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
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

#include "filter_expression.h"
#include "parser.h"
#include "ansi_extras.h"
#include <ctype.h>

/* Set fo valid filter functions */
static const char* valid_filter_funs[] = {
	"cpu",
	"task",
	"event",
	"counter",
	"tasklen",
	NULL
};

/* Constants for mode characters */
enum filter_expr_mode {
	FILTER_EXPR_MODE_SET = 0,
	FILTER_EXPR_MODE_ADD,
	FILTER_EXPR_MODE_REMOVE,
	FILTER_EXPR_MODE_NONE
};

/* Returns the constant associated to the string representation of a
 * single event type in *type. If the string is not a valid identifier
 * for a single event type the function returns 1, otherwise 0. */
int filter_single_event_type_from_stringn(const char* str,
					  size_t len,
					  enum single_event_type* type)
{
	if(strn1eq(str, len, "tcreate")) {
		*type = SINGLE_TYPE_TCREATE;
		return 0;
	} else if(strn1eq(str, len, "texec_start")) {
		*type = SINGLE_TYPE_TEXEC_START;
		return 0;
	} else if(strn1eq(str, len, "texec_end")) {
		*type = SINGLE_TYPE_TEXEC_END;
		return 0;
	} else if(strn1eq(str, len, "tdestroy")) {
		*type = SINGLE_TYPE_TDESTROY;
		return 0;
	}

	return 1;
}

int filter_single_event_type_from_string(const char* str, enum single_event_type* type)
{
	return filter_single_event_type_from_stringn(str, strlen(str), type);
}

/* Evaluates the argument of a event filter function and modifies f
 * accordingly. If f and mes are NULL the function just checks the
 * sub-expression for validity. The parameter expr is the escaped, not
 * necessarily zero-terminated string representing the argument,
 * without parantheses. Returns 0 on success, otherwise 1.
 *
 * The argument to an event function is either the identifier tcreate,
 * tdestroy, texec_start or texec_end.
 */
int filter_eval_event_arg(struct filter* f,
			  struct multi_event_set* mes,
			  enum filter_expr_mode mode,
			  const char* expr,
			  size_t expr_len)
{
	struct parser p;
	struct parser_token t;
	enum single_event_type type;

	parser_init(&p, expr, expr_len);

	/* Single event name */
	if(parser_read_next_identifier(&p, &t))
		return 1;

	if(filter_single_event_type_from_stringn(t.str, t.len, &type))
		return 1;

	parser_skip_ws(&p);

	if(!parser_reached_end(&p))
		return 1;

	if(f) {
		if(mode == FILTER_EXPR_MODE_SET)
			filter_clear_single_event_types(f);

		if(mode == FILTER_EXPR_MODE_SET ||
		   mode == FILTER_EXPR_MODE_ADD)
		{
			filter_set_single_event_type_filtering(f, 1);
			filter_add_single_event_type(f, type);
		}

		if(mode == FILTER_EXPR_MODE_REMOVE)
			filter_remove_single_event_type(f, type);
	}

	return 0;
}

/* Checks if the not necessarily zero-terminated string expr is a vlid
 * string or extended POSIX regex literal. If the expression is valid,
 * the function returns 0, otherwise 1. If the expression is a regular
 * expression, *is_regex is set to 1, otherwise to 0. The unescaped
 * string or regular expression is returned as a zero-terminated
 * string in *out. Ownership for memory for this string is transferred
 * to the caller.*/
int filter_is_valid_regex_or_string_arg(const char* expr,
					size_t expr_len,
					int* is_regex,
					char** out)
{
	struct parser p;
	struct parser_token t;
	char* regex;

	parser_init(&p, expr, expr_len);
	parser_skip_ws(&p);

	if(parser_peek_any_char(&p, &t))
		return 1;

	if(!parser_token_equals_char_oneof(&t, "\"r"))
		return 1;

	if(t.str[0] == 'r') {
		parser_read_next_char(&p, &t, 'r');

		if(parser_read_string(&p, &t))
			return 1;

		if(!(regex = unescape_stringn(t.str+1, t.len-2)))
			return 1;

		if(!is_valid_regex(regex)) {
			free(regex);
			return 1;
		}

		if(out)
			*out = regex;
		else
			free(regex);

		*is_regex = 1;
	} else {
		if(parser_read_string(&p, &t))
			return 1;

		if(out)
			*out = strdupn(t.str+1, t.len-2);

		*is_regex = 0;
	}

	return 0;
}

/* Checks if the not necessarily zero-terminated string expr
 * represents an interval of integers in brackets, separated with a
 * dash. Unit prefixes might be used for each individual integer. If
 * the expression is valid the start and end values are returned in s
 * and e and the function returns 0. Otherwise the function returns
 * 1.*/
int filter_is_valid_uint64_interval_arg(const char* expr,
					size_t expr_len,
					uint64_t* s,
					uint64_t* e)
{
	struct parser p;
	struct parser_token t;

	parser_init(&p, expr, expr_len);
	parser_skip_ws(&p);

	if(parser_read_next_char(&p, &t, '['))
		return 1;

	if(parser_read_next_int_with_unit(&p, &t))
		return 1;

	if(s)
		if(atou64n_unit(t.str, t.len, s))
			return 1;

	if(parser_read_next_char(&p, &t, '-'))
		return 1;

	if(parser_read_next_int_with_unit(&p, &t))
		return 1;

	if(e)
		if(atou64n_unit(t.str, t.len, e))
			return 1;

	if(parser_read_next_char(&p, &t, ']'))
		return 1;

	parser_skip_ws(&p);

	if(!parser_reached_end(&p))
		return 1;

	return 0;
}

/* Evaluates the interval passed to a cpu filter function and modifies
 * f accordingly. Returns 0 on success, otherwise 1.
 */
void filter_mode_cpu_range(struct filter* f,
			   struct multi_event_set* mes,
			   enum filter_expr_mode mode,
			   int cpu_start,
			   int cpu_end)
{
	if(mode == FILTER_EXPR_MODE_SET)
		filter_clear_cpus(f);

	/* Not filtering CPUs is as if all CPUs would already be in
	 * the filter */
	if(mode == FILTER_EXPR_MODE_ADD && !f->filter_cpus)
		return;

	if(mode == FILTER_EXPR_MODE_REMOVE && !f->filter_cpus)
		filter_add_cpu_range(f, mes->min_cpu, mes->max_cpu);

	filter_set_cpu_filtering(f, 1);

	/* Set empty? */
	if(cpu_start > mes->max_cpu || cpu_end < mes->min_cpu) {
		if(mode == FILTER_EXPR_MODE_SET)
			filter_clear_cpus(f);
		else
			return;
	}

	if(cpu_start < mes->min_cpu)
		cpu_start = mes->min_cpu;

	if(cpu_end > mes->max_cpu)
		cpu_end = mes->max_cpu;

	if(mode == FILTER_EXPR_MODE_SET || mode == FILTER_EXPR_MODE_ADD)
		filter_add_cpu_range(f, cpu_start, cpu_end);

	if(mode == FILTER_EXPR_MODE_REMOVE)
		filter_remove_cpu_range(f, cpu_start, cpu_end);
}

/* Evaluates the argument of a cpu filter function and modifies f
 * accordingly. If f and mes are NULL the function just checks the
 * sub-expression for validity. The parameter expr is the escaped, not
 * necessarily zero-terminated string representing the argument,
 * without parantheses. Returns 0 on success, otherwise 1.
 *
 * The argument to a cpu function is an interval of integers in
 * brackets, separated by a dash. Unit prefixes might be used for each
 * individual integer (e.g., [2K-5M]). */
int filter_eval_cpu_arg(struct filter* f,
			struct multi_event_set* mes,
			enum filter_expr_mode mode,
			const char* expr,
			size_t expr_len)
{
	struct parser p;
	struct parser_token t;
	uint64_t cpu_start;
	uint64_t cpu_end;
	uint64_t cpu;

	parser_init(&p, expr, expr_len);
	parser_skip_ws(&p);

	if(parser_peek_any_char(&p, &t))
		return 1;

	if(isdigit(t.str[0])) {
		/* Single integer */
		if(parser_read_int_with_unit(&p, &t))
			return 1;

		parser_skip_ws(&p);

		if(f) {
			if(atou64n(t.str, t.len, &cpu))
				return 1;

			filter_mode_cpu_range(f, mes, mode, cpu, cpu);
		}

		if(!parser_reached_end(&p))
			return 1;
	} else if(t.str[0] == '[') {
		/* Range of integers */
		if(filter_is_valid_uint64_interval_arg(expr,
						       expr_len,
						       &cpu_start,
						       &cpu_end))
		{
			return 1;
		}

		if(f)
			filter_mode_cpu_range(f, mes, mode, cpu_start, cpu_end);
	} else {
		return 1;
	}

	return 0;
}

/* Evaluates the argument of a tasklen filter function and modifies f
 * accordingly. If f and mes are NULL the function just checks the
 * sub-expression for validity. The parameter expr is the escaped, not
 * necessarily zero-terminated string representing the argument,
 * without parantheses. Returns 0 on success, otherwise 1.
 *
 * The argument to a tasklen function is an interval of integers in
 * brackets, separated by a dash. Unit prefixes might be used for each
 * individual integer (e.g., [2K-5M]). */
int filter_eval_tasklen_arg(struct filter* f, const char* expr, size_t expr_len)
{
	uint64_t min;
	uint64_t max;
	uint64_t tmp;

	if(filter_is_valid_uint64_interval_arg(expr, expr_len, &min, &max))
		return 1;

	if(min > max) {
		tmp = max;
		max = min;
		min = tmp;
	}

	if(f) {
		filter_set_task_length_filtering_range(f, min, max);
		filter_set_task_length_filtering(f, 1);
	}

	return 0;
}

/* Evaluates the string or regex passed to a task filter function and
 * modifies f accordingly. If f and mes are NULL the function just
 * checks the sub-expression for validity. The parameter str is the
 * unescaped, zero-terminated string representing the
 * argument. Returns 0 on success, otherwise 1.
 */
int filter_mode_task(struct filter* f,
		     struct multi_event_set* mes,
		     enum filter_expr_mode mode,
		     const char* str,
		     int is_regex)
{
	regex_t regex;
	struct task* t;

	if(mode == FILTER_EXPR_MODE_SET)
		filter_clear_tasks(f);

	/* Not filtering tasks is as if all tasks would
	 * already be in the filter */
	if(mode == FILTER_EXPR_MODE_ADD && !f->filter_tasks)
		return 0;

	if(mode == FILTER_EXPR_MODE_REMOVE && !f->filter_tasks)
		for_each_task(mes, t)
			if(filter_add_task(f, t))
				return 1;

	filter_set_task_filtering(f, 1);

	if(is_regex) {
		if(regcomp(&regex, str, REG_EXTENDED | REG_NOSUB))
			return 1;

		for_each_task(mes, t) {
			if(t->symbol_name &&
			   regexec(&regex, t->symbol_name, 0, NULL, 0) !=
			     REG_NOMATCH)
			{
				if(mode == FILTER_EXPR_MODE_SET ||
				   mode == FILTER_EXPR_MODE_ADD)
				{
					if(!filter_has_task(f, t)) {
						if(filter_add_task(f, t)) {
							regfree(&regex);
							return 1;
						}
					}
				} else {
					filter_remove_task(f, t);
				}
			}
		}

		regfree(&regex);
	} else {
		for_each_task(mes, t) {
			if(t->symbol_name && strcmp(str, t->symbol_name) == 0) {
				if(mode == FILTER_EXPR_MODE_SET ||
				   mode == FILTER_EXPR_MODE_ADD)
				{
					if(!filter_has_task(f, t))
						if(filter_add_task(f, t))
							return 1;
				} else {
					filter_remove_task(f, t);
				}
			}
		}
	}

	return 0;
}

/* Evaluates the argument of a task filter function and modifies f
 * accordingly. If f and mes are NULL the function just checks the
 * sub-expression for validity. The parameter expr is the escaped, not
 * necessarily zero-terminated string representing the unescaped
 * argument, without parantheses. Returns 0 on success, otherwise 1.
 *
 * The argument to a task function is either a string or a POSIX
 * extended regular expression. */
int filter_eval_task_arg(struct filter* f,
			 struct multi_event_set* mes,
			 enum filter_expr_mode mode,
			 const char* expr,
			 size_t expr_len)
{
	int is_regex;
	char* str = NULL;

	if(filter_is_valid_regex_or_string_arg(expr,
					       expr_len,
					       &is_regex,
					       f ? &str : NULL))
	{
		return 1;
	}

	if(f) {
		if(filter_mode_task(f, mes, mode, str, is_regex)) {
			free(str);
			return 1;
		}

		free(str);
	}

	return 0;
}

/* Evaluates the string or regex passed to a counter filter function
 * and modifies f accordingly. If f and mes are NULL the function just
 * checks the sub-expression for validity. The parameter str is the
 * unescaped, zero-terminated string representing the
 * argument. Returns 0 on success, otherwise 1.
*/
int filter_mode_counter(struct filter* f,
			struct multi_event_set* mes,
			enum filter_expr_mode mode,
			const char* str,
			int is_regex)
{
	regex_t regex;
	struct counter_description* cd;

	if(mode == FILTER_EXPR_MODE_SET)
		filter_clear_counters(f);

	/* Not filtering counters is as if all counters would
	 * already be in the filter */
	if(mode == FILTER_EXPR_MODE_ADD && !f->filter_counters)
		return 0;

	if(mode == FILTER_EXPR_MODE_REMOVE && !f->filter_counters)
		for_each_counterdesc(mes, cd)
			filter_add_counter(f, cd);

	filter_set_counter_filtering(f, 1);

	if(is_regex) {
		if(regcomp(&regex, str, REG_EXTENDED | REG_NOSUB))
			return 1;

		for_each_counterdesc(mes, cd) {
			if(cd->name &&
			   regexec(&regex, cd->name, 0, NULL, 0) != REG_NOMATCH)
			{
				if(mode == FILTER_EXPR_MODE_SET ||
				   mode == FILTER_EXPR_MODE_ADD)
				{
					if(!filter_has_counter(f, cd))
						filter_add_counter(f, cd);
				} else {
					filter_remove_counter(f, cd);
				}
			}
		}

		regfree(&regex);
	} else {
		for_each_counterdesc(mes, cd) {
			if(cd->name && strcmp(str, cd->name) == 0) {
				if(mode == FILTER_EXPR_MODE_SET ||
				   mode == FILTER_EXPR_MODE_ADD)
				{
					if(!filter_has_counter(f, cd))
						filter_add_counter(f, cd);
				} else {
					filter_remove_counter(f, cd);
				}
			}
		}
	}

	return 0;
}

/* Evaluates the argument of a counter filter function and modifies f
 * accordingly. If f and mes are NULL the function just checks the
 * sub-expression for validity. The parameter expr is the escaped, not
 * necessarily zero-terminated string representing the unescaped
 * argument, without parantheses. Returns 0 on success, otherwise 1.
 *
 * The argument to a counter function is either a string or a POSIX
 * extended regular expression. */
int filter_eval_counter_arg(struct filter* f,
			    struct multi_event_set* mes,
			    enum filter_expr_mode mode,
			    const char* expr,
			    size_t expr_len)
{
	int is_regex;
	char* str = NULL;

	/* Check if valid string / regex and unescape */
	if(filter_is_valid_regex_or_string_arg(expr,
					       expr_len,
					       &is_regex,
					       f ? &str : NULL))
	{
		return 1;
	}

	if(f) {
		if(filter_mode_counter(f, mes, mode, str, is_regex)) {
			free(str);
			return 1;
		}

		free(str);
	}

	return 0;
}

/* Evaluates a single filter function and modifies f accordingly. If f
 * and mes are NULL the function just checks the sub-expression for
 * validity. Returns 0 on success, otherwise 1.*/
int filter_eval_filter_fun(struct filter* f,
			   struct multi_event_set* mes,
			   enum filter_expr_mode mode,
			   const char* fun, size_t fun_len,
			   const char* expr, size_t expr_len)
{
	/* Check if mode is required */
	if(strn1eq(fun, fun_len, "tasklen")) {
		if(mode != FILTER_EXPR_MODE_NONE)
			return 1;
	} else {
		if(mode == FILTER_EXPR_MODE_NONE)
			return 1;
	}

	/* Call function-specific evaluation functions */
	if(strn1eq(fun, fun_len, "cpu")) {
		return filter_eval_cpu_arg(f, mes, mode, expr, expr_len);
	} else if(strn1eq(fun, fun_len, "task")) {
		return filter_eval_task_arg(f, mes, mode, expr, expr_len);
	} else if(strn1eq(fun, fun_len, "counter")) {
		return filter_eval_counter_arg(f, mes, mode, expr, expr_len);
	} else if(strn1eq(fun, fun_len, "tasklen")) {
		return filter_eval_tasklen_arg(f, expr, expr_len);
	} else if(strn1eq(fun, fun_len, "event")) {
		return filter_eval_event_arg(f, mes, mode, expr, expr_len);
	}

	return 0;
}

/* Evaluates a filter expression. The filter f is modified according
 * to the expression. Returns 0 on success, otherwise 1. If f and mes
 * are NULL, the function only checks for validity of the
 * expression.
 *
 * A filter expression may be composed of several,
 * whitespace-separated sub-expression, where each sub-expression has
 * the following form:
 *
 *   - A mode character (+, - or !). A plus sign indicates that the
 *     sub-expression adds something to the filter, the minus sign
 *     indicates removal from the filter while the exclamation mark
 *     sets the filter to a certain value. For certain filter
 *     functions the mode character is not required (e.g., for
 *     tasklen; see below).
 *
 *   - A filter function (e.g., cpu, task, tasklen, event) that
 *     indicates which part of the filter the sub-expression applies to.
 *
 *   - A function-specific argument in parantheses (e.g., the range of
 *     CPUs to be added to the filter, the interval of task durations
 *     for the tasklen function, etc.)
 *
 * Depending on the function an argument can either be a single
 * integer with an optionaml unit prefix (e.g., 5, 123, 19K or 189M),
 * a range of integers in brackets with optional unit prefixes (e.g.,
 * [1-10], [500-55K], [100M-1G]), a string in quotes (e.g., "foo", "a
 * string with \"quotes\"") or a POSIX extended regular expression in
 * quotes prefixed by the character r (e.g., r"^init", r"[a-zA-Z]*").
 *
 * Examples:
 *   - "+cpu(5)" adds cpu 5 to f.
 *
 *   - "-cpu(5)" removes cpu 5 from f.
 *
 *   - "!cpu(5)" clears the set of CPUs of f before adding cpu 5.
 *
 *   - "tasklen([5K-100M])" configures the filter to include only task
 *     instances whose duration is between 5000 and 100000000 cycles.
 *
 *   - "+task(\"init\")" adds all tasks to f whose associated symbol
 *     is equal to "init".
 *
 *   - "!task(\"^init.*\")" configures f to include only tasks whose
 *     associated symbol starts with the string "init".
 *
 *   - "!event(tdestroy)" configures f to include only task
 *     destruction events.
 *
 * A filter function does not affect the filter for any other aspect
 * of the filter. For example, limiting the filter to CPU 5 using
 * "!cpu(5)" neither affects the task types included in the filter,
 * nor does it affect the set of single events of the filter.
 */
int filter_eval_expression(struct filter* f,
			   struct multi_event_set* mes,
			   const char* expr)
{
	struct parser p;
	struct parser_token t;
	const char* expr_start;
	const char* expr_end;
	const char* fun_start;
	const char* fun_end;

	enum filter_expr_mode mode;

	parser_init(&p, expr, strlen(expr));

	do {
		parser_skip_ws(&p);

		if(parser_peek_any_char(&p, &t))
			return 1;

		/* Parse mode character */
		if(t.str[0] == '!')
			mode = FILTER_EXPR_MODE_SET;
		else if(t.str[0] == '+')
			mode = FILTER_EXPR_MODE_ADD;
		else if(t.str[0] == '-')
			mode = FILTER_EXPR_MODE_REMOVE;
		else
			mode = FILTER_EXPR_MODE_NONE;

		/* Consumer mode character */
		if(mode != FILTER_EXPR_MODE_NONE)
			if(parser_read_any_char(&p, &t))
				return 1;

		/* Parse filter function */
		if(parser_read_next_identifier(&p, &t))
			return 1;

		fun_start = t.str;
		fun_end = t.str + t.len;

		/* Check that the function name is valid */
		if(!parser_token_equals_str_oneof(&t, valid_filter_funs))
			return 1;

		/* Match parantheses */
		if(parser_read_next_char(&p, &t, '('))
			return 1;

		expr_start = p.curr;

		if(!(expr_end = parser_find_char(&p, ')')-1))
			return 1;

		if(parser_read_next_char(&p, &t, ')'))
			return 1;

		/* Evaluate current function */
		if(filter_eval_filter_fun(f,
					  mes,
					  mode,
					  fun_start,
					  fun_end - fun_start,
					  expr_start,
					  expr_end - expr_start + 1))
		{
			return 1;
		}

		parser_skip_ws(&p);
	} while (!parser_reached_end(&p));

	return 0;
}

/* Checks if a string is a valid filter expression. Returns 1 if this
 * is the case, otherwise 0. */
int filter_is_valid_expression(const char* expr)
{
	return !filter_eval_expression(NULL, NULL, expr);
}
