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

int main()
{
	assert_str_eq("\"\\n\"", repr("\n"));
	assert_str_eq("\"\\t\"", repr("\t"));
	assert_str_eq("\"a\"", repr("a"));

	char s[] = "1234";
	drop_first_last_inplace(s, strlen(s));
	assert_str_eq("23", s);
	return 0;
}
