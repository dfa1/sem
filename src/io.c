#include <stdio.h>
#include <string.h>
#include "sem.h"

// Display a prompt, read from stdin, dropping \n before returning the user input. Return -1 on EOF.  
int ask(const char *question, char *answer, int answer_size) {
	fputs(question, stdout);
	const char *res = fgets(answer, answer_size, stdin);
	if (res == nullptr) {
		return -1;
	}
	answer[strlen(answer) - 1] = 0;
	return 0;
}

int ask_yes_no(const char *question) {
	char answer[20];
	const char *p = answer;
	do {
		ask(question, answer, sizeof(answer));
		if (strcmp(p, "yes") == 0 || strcmp(p, "y") == 0) {
			return 1;
		}
		if (strcmp(p, "no") == 0 || strcmp(p, "n") == 0) {
			return 0;
		}
	} while (1);
}

int fetch_line_from_file(const char *filename, const int lineno, char *dest,
                         const int dest_size) {
	FILE *fp = fopen(filename, "r");
	if (fp == nullptr) {
		return -1;
	}
	int i = 0;
	while (i++ < lineno) {
		const char *res = fgets(dest, dest_size, fp);
		if (res == nullptr) {
			fclose(fp);
			return -1;
		}
	}
	fclose(fp);
	return 0;
}
