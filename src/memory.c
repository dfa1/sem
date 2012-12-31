#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sem.h"

void *xmalloc(const size_t len)
{
	void *mem = malloc(len);

	if (mem == NULL) {
		abort();
	}

	return mem;
}

char *xstrdup(const char *s)
{
	char *dup = strdup(s);

	if (dup == NULL) {
		abort();
	}

	return dup;
}

/* example:
 *   drop_first_last_inplace("abcd", 4) -> "bc"
 */
void drop_first_last_inplace(char *str, int len)
{
	for (int i = 0; i < len - 1; i++) {
		str[i] = str[i + 1];
	}
	str[len - 2] = 0;
}

/* example:
 *   '\nì -> '"\\n"''
 */
char *repr(const char *s)
{
	char *p;
	int size, i, j = 0;

	/* (size * 2) + 3 is ridiculously large enough :-) */
	size = strlen(s);
	p = (char *)xmalloc((size * 2) + 3);

	/* Begin quoting. */
	*(p + j++) = '\"';

	for (i = 0; i < size; i++) {
		register char c = *(s + i);

		if (c == '\0')
			break;

		/* Expands special character while don't touchs the others. */
		if (c == '\n')
			*(p + j++) = '\\', *(p + j++) = 'n';
		else if (c == '\t')
			*(p + j++) = '\\', *(p + j++) = 't';
		else if (c == '\r')
			*(p + j++) = '\\', *(p + j++) = 'r';
		else if (c == '\v')
			*(p + j++) = '\\', *(p + j++) = 'v';
		else if (c == '\f')
			*(p + j++) = '\\', *(p + j++) = 'f';
		else if (c == '\b')
			*(p + j++) = '\\', *(p + j++) = 'b';
		else if (c == '\a')
			*(p + j++) = '\\', *(p + j++) = 'a';
		else
			*(p + j++) = c;
	}

	/* End quoting. */
	*(p + j++) = '\"';
	*(p + j++) = '\0';
	return p;
}
