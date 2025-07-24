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

void test_quote()
{
	char buf[4];
	assert_str_eq("\n", quote("\"\\n\"", buf, sizeof(buf)));
	assert_str_eq("a", quote("\"a\"", buf, sizeof(buf)));
	assert_str_eq("", quote("\"\"", buf, sizeof(buf)));
}

void test_unquote()
{
	char buf[4];
	assert_str_eq("\"\\n\"", unquote("\n", buf, sizeof(buf)));
	assert_str_eq("\"\\t\"", unquote("\t", buf, sizeof(buf)));
	assert_str_eq("\"a\"", unquote("a", buf, sizeof(buf)));
	assert_str_eq("\"\"", unquote("", buf, sizeof(buf)));
	assert_str_eq("\"ab\"", unquote("abcd", buf, sizeof(buf)));
}

void test_xstrdup()
{
	assert_str_eq("hello", xstrdup("hello"));
}

int main()
{
	test_quote();
	test_unquote();
	test_xstrdup();
	return 0;
}
