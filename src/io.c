#include "sem.h"

char *
ask(const char *question, char *answer, int answer_size) {
  fputs(question, stdout);
  return fgets(answer, sizeof(answer_size), stdin);
}

int
ask_yes_no(const char *question)
{
  char answer[20];
  const char *p;
  do {
    p = ask(question, answer, sizeof(answer));
    if (strcmp(p, "yes") == 0 || strcmp(p, "y") == 0) {
      return 1;
    } else if (strcmp(p, "no") == 0 || strcmp(p, "n") == 0) {
      return 0;
    }
  } while(1);
}
