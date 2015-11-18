#include "unit_tests.h"
#include <stdlib.h>
#include <ctype.h>

int __unit_test_nr__ = 0;
int __unit_tests_ok__ = 0;
int __unit_assert_cnt_glob__ = 0;
int __unit_assert_cnt__ = 0;

unit_test_func* test_funcs[MAX_TESTS];

/*unsigned int __get_list_length__(struct list_entry* root)
{
	unsigned int list_length = 0;
	struct list_entry* iter;

	list_foreach(root, iter)
		list_length++;

	return list_length;
	}*/

void __fail__(void)
{
	exit(EXIT_FAILURE);
}

void __add_test__(unit_test_func* func)
{
	if(__unit_test_nr__ == MAX_TESTS) {
		fprintf(stderr, "Could not add test: table for test functions is full...\n");
		exit(EXIT_FAILURE);
	}

	test_funcs[__unit_test_nr__++] = func;
}

void __run_tests__(void)
{
	unsigned int i;

	for(i = 0; i < __unit_test_nr__; ++i) {
		__unit_assert_cnt__ = 0;
		test_funcs[i]();
		__unit_assert_cnt_glob__ += __unit_assert_cnt__;
	}
	printf("\t%d test%s with %d assertion%s passed.\n\n",
	       __unit_test_nr__,
	       (__unit_test_nr__ > 1) ? "s" : "",
	       __unit_assert_cnt_glob__,
	       (__unit_assert_cnt_glob__ > 1) ? "s" : "");
}

void str_skipws(char* str)
{
	int i = 0;

	while(str[i] && isblank(str[i]))
		++i;

	memmove(str, &(str[i]), strlen(str)-i);
}

int fgetline(const char* filename, char* buffer, int buf_len, int lineno)
{
	FILE* fp = fopen(filename, "r");

	if(!fp)
		return -1;

	for(int i = 0; i < lineno; ++i)
		if(!fgets(buffer, buf_len, fp))
			return -1;

	fclose(fp);
	str_skipws(buffer);

	return 0;
}
