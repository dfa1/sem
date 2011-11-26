#include "sem.h"

char *ask(char *question, char *answer, int answer_size) {
  fputs(question, stdout);
  fgets(answer, sizeof(answer_size), stdin);
  answer[strlen(answer) - 1] = 0; 
  return answer;
}

int
ask_yes_no(char *question)
{
  char answer[20];
  const char *p;
  do {
    p = ask(question, answer, sizeof(answer));
    if (strcasecmp(p, "yes") == 0 || strcasecmp(p, "y") == 0) {
      return 1;
    } else if (strcasecmp(p, "no") == 0 || strcasecmp(p, "n") == 0) {
      return 0;
    }
  } while(1);
}
