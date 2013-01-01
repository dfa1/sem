#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sem.h"

void assert_str_eq(char *expected, char *got)
{
	if (strcmp(expected, got) != 0) {
		fprintf(stderr, "expected '%s', got '%s'\n", expected, got);
		exit(EXIT_FAILURE);
	}
}

void test_repr()
{
	char buf[4];
	assert_str_eq("\"\\n\"", repr("\n", buf, sizeof(buf)));
	assert_str_eq("\"\\t\"", repr("\t", buf, sizeof(buf)));
	assert_str_eq("\"a\"", repr("a", buf, sizeof(buf)));
	assert_str_eq("\"\"", repr("", buf, sizeof(buf)));
	assert_str_eq("\"ab\"", repr("abcd", buf, sizeof(buf)));
}

void test_drop_first_last_inplace()
{
	char buf[] = "123456";
	drop_first_last_inplace(buf, strlen(buf));
	assert_str_eq("2345", buf);
}

void test_xstrdup()
{
	assert_str_eq("hello", xstrdup("hello"));
}

int main()
{
	test_repr();
	test_drop_first_last_inplace();
	test_xstrdup();
	return 0;
}
