#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sem.h"

void *
xmalloc(const size_t len) 
{
  void *mem = malloc(len);
  
  if (mem == NULL) {
    abort();
  }

  return mem;
}

char * 
xstrdup(const char *s) {
  char *dup = strdup(s);

  if (dup == NULL) {
    abort();
  }

  return dup;
}

/* example:
 *   drop_first_last_inplace("abcd", 4) -> "bc"
 */
void
drop_first_last_inplace(char *str, int len) {
        for (int i = 0; i < len - 1; i++) {
           str[i] = str[i + 1];
        }
        str[len - 2] = 0;
}
