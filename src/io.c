#include "sem.h"

char *
ask(char *question, int size) {
  fputs(question, stdout);
  fgets(question, sizeof(size), stdin);
  question[strlen(question) - 1] = 0;
  return question;
}

int
ask_yes_no()
{
  char question[20];
  const char *p;
  do {
    p = ask(question, sizeof(question));
    if (strcasecmp(p, "yes") == 0 || strcasecmp(p, "y") == 0) {
      return 1;
    } else if (strcasecmp(p, "no") == 0 || strcasecmp(p, "n") == 0) {
      return 0;
    }
  } while(1);
}
