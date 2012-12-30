#include "sem.h"

// drop \n 
void
ask(const char *question, char *answer, int answer_size) {
  fputs(question, stdout);
  fgets(answer, sizeof(answer_size), stdin);
  answer[strlen(answer) - 1] = 0;
}

int
ask_yes_no(const char *question)
{
  char answer[20];
  const char *p = answer;
  do {
    ask(question, answer, sizeof(answer));
    if (strcmp(p, "yes") == 0 || strcmp(p, "y") == 0) {
      return 1;
    } else if (strcmp(p, "no") == 0 || strcmp(p, "n") == 0) {
      return 0;
    }
  } while(1);
}
