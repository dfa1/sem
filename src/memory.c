#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sem.h"

void *xmalloc(const size_t size)
{
	void *mem = malloc(size);

	if (mem == nullptr) {
		abort();
	}

	return mem;
}

char *xstrdup(const char *str)
{
	assert(str != nullptr);
	char *dup = xmalloc(strlen(str) + 1);
	return strcpy(dup, str);
}

/* 
 * example:
 *   '"\\n"' -> '\n'
 */
char *quote(const char *str, char *dest, size_t dest_size)
{
	assert(str != nullptr);
	assert(dest != nullptr);
	assert(dest_size >= 1);
	const size_t src_size = strlen(str);
	char *p = dest;
	for (int i = 1; i < dest_size && i < src_size - 1; i++) {
		if (str[i] == '\\') {
			const char lookahead = str[i + 1];
			i++;	// consuming two chars
			switch (lookahead) {
			case 'n':
				*p++ = '\n';
				break;
			case 't':
				*p++ = '\t';
				break;
			case 'r':
				*p++ = '\r';
				break;
			case 'v':
				*p++ = '\v';
				break;
			case 'f':
				*p++ = '\f';
				break;
			case 'b':
				*p++ = '\b';
				break;
			case 'a':
				*p++ = '\a';
				break;
			default:
				*p++ = '\\';
				*p++ = lookahead;
			}
		} else {
			*p++ = str[i];
		}
	}
	*p = 0;
	return dest;
}

/* 
 * example:
 *   '\n' -> '"\\n"'
 */
char *unquote(const char *str, char *dest, const size_t dest_size)
{
	assert(str != nullptr);
	assert(dest != nullptr);
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
