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
