#ifndef UNIT_TESTS_H
#define UNIT_TESTS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

typedef void (unit_test_func)(void);
void __add_test__(unit_test_func func);
void __run_tests__(void);
void __fail__(void);
int fgetline(const char* filename, char* buffer, int buf_len, int lineno);

#define MAX_TESTS 1024

extern int __unit_tests_ok__;
extern int __unit_test_nr__;
extern int __unit_assert_cnt__;

#define UNIT_TEST(__test_name) \
	void __test_name(void) { \
	printf("\t[TEST %d/%d] " #__test_name "... ", (__unit_tests_ok__+1), __unit_test_nr__); \
	fflush(stdout);


#define END_TEST() \
	printf("OK (%d assertions)\n", __unit_assert_cnt__); \
	fflush(stdout); \
	__unit_tests_ok__++;\
	}

#define UNIT_TEST_SUITE(__suite_name) \
	int main(int argc, char** argv) {\
		printf("[TESTSUITE] " #__suite_name "\n"); \
		fflush(stdout);

#define END_TEST_SUITE() \
		__run_tests__(); \
		return 0; \
	}

#define ADD_TEST(__test_name) \
	__add_test__(__test_name);

#define ASSERT_INTRO() \
	do {

#define ASSERT_OUTRO(exp_type, exp, res) \
		{ \
			fprintf(stderr, "\n\tASSERTION FAILED:\n" __FILE__ ":%d: in function %s(): expected " exp_type " but was " exp_type " \n", __LINE__, __func__, exp, res); \
			char line[1024]; \
			fgetline(__FILE__, line, sizeof(line), __LINE__); \
			fprintf(stderr, "Failed test: %s", line); \
			fflush(stderr); \
			__fail__(); \
		} \
		__unit_assert_cnt__++; \
	} while(0);

#define ASSERT_EQUALS(a, b) \
	ASSERT_INTRO() \
		if((a) != (b)) \
			ASSERT_OUTRO("%ld", (unsigned long)(b), (unsigned long)(a))

#define ASSERT_EQUALS_LD(a, b) \
	ASSERT_INTRO() \
		if((a) != (b)) \
			ASSERT_OUTRO("%Lf", (long double)(b), (long double)(a))

#define ASSERT_EQUALS_LD_DELTA(a, b, delta)	\
	do {					\
		if(fabsl((a) - (b)) > (delta)) {			\
			fprintf(stderr, "\n\tASSERTION FAILED:\n" __FILE__ ":%d: in function %s(): expected %Lf with a delta of %Lf, but was %Lf \n", __LINE__, __func__, (b), (delta), (a)); \
			fflush(stderr);				\
			__fail__();					\
		}							\
		__unit_assert_cnt__++;					\
	} while(0)

#define ASSERT_EQUALS_PTR(a, b) \
	ASSERT_INTRO() \
		if((a) != (b)) \
			ASSERT_OUTRO("%p", (void*)(b), (void*)(a))

#define ASSERT_EQUALS_STRING(a, b) \
	ASSERT_INTRO() \
		if(strcmp((a), (b)) != 0) \
			ASSERT_OUTRO("\"%s\"", (const char*)(b), (const char*)(a))

#define ASSERT_EQUALS_STRING_N(a, b, n) \
	do { \
		if(strncmp((a), (b), (n)) != 0) { \
			fprintf(stderr, "\n\tASSERTION FAILED:\n" __FILE__ ":%d: in function %s(): expected prefix \"%.*s\", but was \"%.*s\"\n", __LINE__, __func__, (n), (b), (n), (a)); \
			fflush(stderr); \
			__fail__(); \
		} \
	} while(0)

#define ASSERT_STRING_PREFIX(a, b) \
	ASSERT_INTRO() \
		if(strncmp((a), (b), strlen(b)) != 0) \
			ASSERT_OUTRO("%s", (const char*)(b), (const char*)(a))

#define ASSERT_DIFFERENT(a, b) \
	ASSERT_INTRO() \
		if((a) == (b)) \
			ASSERT_OUTRO("%s", "something else", "a certain value")

#define ASSERT_GREATER(a, b) \
	ASSERT_INTRO() \
		if((a) <= (b)) \
			ASSERT_OUTRO("%s", "something greater", "smaller or equal")

#define ASSERT_LESS_EQ(a, b) \
	ASSERT_INTRO() \
		if((a) > (b)) \
			ASSERT_OUTRO("%s", "something smaller", "greater")

#define ASSERT_GREATER_EQ(a, b) \
	ASSERT_INTRO() \
		if((a) < (b)) \
			ASSERT_OUTRO("%s", "something greater", "smaller")

#define ASSERT_LESS(a, b) \
	ASSERT_INTRO() \
		if((a) >= (b)) \
			ASSERT_OUTRO("%s", "something smaller", "greater or equal")

#define ASSERT_TRUE(a) \
	ASSERT_INTRO() \
		if(!(a)) \
	ASSERT_OUTRO("%s", "true", "false")

#define ASSERT_FALSE(a) \
	ASSERT_INTRO() \
		if((a)) \
	ASSERT_OUTRO("%s", "false", "true")

#define ASSERT_NULL(a) \
	ASSERT_INTRO() \
		if((a) != NULL) \
			ASSERT_OUTRO("%p", NULL, (a))

#define ASSERT_NONNULL(a) \
	ASSERT_INTRO() \
		if((a) == NULL) \
	ASSERT_OUTRO("%s", "value different from NULL", "NULL")

#define ASSERT_LIST_LENGTH(list, explength) \
	do { \
		unsigned int list_length = __get_list_length__(list); \
		ASSERT_EQUALS(explength, list_length); \
	} while(0);

#define ASSERT_INTERVAL(val, exp, frac) \
	do { \
		double val_d = (val); \
		double exp_d = (exp); \
		double frac_d = (frac); \
		double interval_low_d = exp_d-(exp_d*frac_d); \
		double interval_high_d = exp_d+(exp_d*frac_d); \
		if(val_d < interval_low_d || val_d > interval_high_d) { \
			fprintf(stderr, "\n\tASSERTION FAILED: " __FILE__ " line %d: expected a value within [%f;%f], but was %f \n", __LINE__, interval_low_d, interval_high_d, val_d); \
			fflush(stderr); \
			__fail__(); \
		} \
		__unit_assert_cnt__++; \
	} while(0);

#endif //UNIT_TESTS_H
