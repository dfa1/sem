#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
void drop_first_last_inplace(char *str, int str_size)
{
	for (int i = 0; i < str_size - 1; i++) {
		str[i] = str[i + 1];
	}
	str[str_size - 2] = 0;
}

/* example:
 *   '\nì -> '"\\n"''
 * NB: the caller is responsible for providing a suitable 'dest' array. 
 */
char *repr(const char *str, char *dest, int dest_size)
{
	assert(str != NULL);
	assert(dest != NULL);
	assert(dest_size >= 3); 
	char *p = dest;
	*p++ = '\"';
	for (int i = 0; i < dest_size - 2 && str[i] != 0; i++) {
		switch (str[i]) {
		case '\n':
			*p++ = '\\';
			*p++ = 'n';
			break;
		case '\t':
			*p++ = '\\';
			*p++ = 't';
			break;
		case '\r':
			*p++ = '\\';
			*p++ = 'r';
			break;
		case '\v':
			*p++ = '\\';
			*p++ = 'v';
			break;
		case '\f':
			*p++ = '\\';
			*p++ = 'f';
			break;
		case '\b':
			*p++ = '\\';
			*p++ = 'b';
			break;
		case '\a':
			*p++ = '\\';
			*p++ = 'a';
			break;
		default:
			*p++ = str[i];
		}
	}

	*p++ = '\"';
	*p = 0;
	return dest;
}
