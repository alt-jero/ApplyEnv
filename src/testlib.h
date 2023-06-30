#include <stdbool.h>

void assert_not_implemented();

void assert_str_equal(char* actual, char* expected);

void assert_size_t_equal(size_t actual, size_t expected);

void assert_int_equal(int actual, int expected);

void assert_bool_equal(bool actual, bool expected);

void assert_is_null(char* actual);

void testlib_init();

int testlib_finalize();

void describeFile(char* description);

void describeSection(char* description);

void describe(char* description);

char* createString(int n);