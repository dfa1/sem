/*
 * debugger.c -- The debugger
 *
 * Copyright (C) 2003-2013 Davide Angelocola <davide.angelocola@gmail.com>
 *
 * Sem is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Sem is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#include <stdio.h>
#include <string.h>
#include "sem.h"
#include "config.h"

typedef enum {
	RUNNING = 1,
	HALTED
} state_t;

struct debug_state {
	state_t state;
	struct vm *vm;
	struct code *code;
	struct cmd *cmds;
	char *filename;
};

struct cmd {
	char *name;
	char *doc;
	int (*func) (struct debug_state *);
};

/* Command's return values. */
enum {
	CONTINUE = 0,
	QUIT
};

/*
 * Commands
 * --------
 */

/* dump */
static char dump_doc[] = "Dump internal code representation.";

/* *INDENT-OFF* */
static const char *opstr[] = {	/* TODO: autogenerate these from sem.h */
	"SETLINENO", 	"SET", 		"JUMP", 	"JUMPT",
	"INT", 		"READ", 	"WRITE_INT",	"WRITE_STR",
 	"WRITELN_INT",	"WRITELN_STR",	"MEM", 		"ADD",
	"SUB", 		"MUL", 		"DIV", 		"MOD",
	"EQ",		"NE", 		"GT", 		"LT",
	"GE",		"LE", 		"IP",		"HALT"
};
/* *INDENT-ON* */

static int dump_func(struct debug_state *ds)
{
	for (struct instr *i = ds->code->head; i != NULL; i = i->next) {
		fprintf(stdout, "%-20s\t", opstr[i->opcode]);

		if (i->intv != -1) {
			fprintf(stdout, "%d\n", i->intv);
		} else if (i->strv != NULL) {
			const size_t ridiculously_large_enough = strlen(i->strv) * 2 + 4;
			char tmp[ridiculously_large_enough];
			fprintf(stdout, "%s\n",
				unquote(i->strv, tmp,
					ridiculously_large_enough));
		} else {
			fprintf(stdout, "\n");
		}
	}

	return CONTINUE;
}

/* help */
static char help_doc[] = "Print this help.";

static int help_func(struct debug_state *ds)
{
	for (struct cmd *cmd = ds->cmds; cmd->name != NULL; cmd++) {
		fprintf(stdout, "%-20s %-10c %-30s\n", cmd->name, cmd->name[0],
			cmd->doc);
	}
	return CONTINUE;
}

/* ip */
static char ip_doc[] = "Print the instruction pointer.";

static int ip_func(struct debug_state *ds)
{
	if (ds->state == RUNNING) {
		char line[1000];
		const int lineno = ds->vm->lineno;
		const char *filename = ds->code->filename;
		if (fetch_line_from_file(filename, lineno, line, sizeof(line)) < 0) {
			printf("cannot fetch line %d from file %s\n", lineno,
			       filename);
		} else {
			opcode_t opcode = ds->vm->ip->opcode;
			printf("op = %d %s.\n", opcode, opstr[opcode]);
			printf("%d %s", lineno, line);
		}
	} else {
		printf("Debugger not started.\n");
	}
	return CONTINUE;
}

/* list */
static char list_doc[] = "List program source.";

static int list_func(struct debug_state *ds)
{
	FILE *fp = fopen(ds->code->filename, "r");

	if (fp == nullptr) {
		printf("Failed to open file.\n");
		return CONTINUE;
	}

	int lineno = 1;
	char line[1000];
	while (fgets(line, sizeof(line), fp) != NULL) {
		printf("%d %s", lineno++, line);
	}
	fclose(fp);
	return CONTINUE;
}

/* mem */
static char mem_doc[] = "Dump the memory.";

static int mem_func(struct debug_state *ds)
{
	for (int i = 0; i < ds->vm->memsize; i++) {	// TODO: screaming for refactoring
		int j;
		/* Print at most 10 item. */
		for (j = 0; i < ds->vm->memsize && j < 10; j++) {
			fprintf(stdout, "%4d ", ds->vm->mem[i++]);
		}

		/* Fill the line. */
		for (int k = 10 - j; k >= 0; k--) {
			fprintf(stdout, "     ");
		}

		/* Append the memory range at the end of the line. */
		fprintf(stdout, "  %4d - %4d\n", i - j, i);
	}

	return CONTINUE;
}

/* stack */
static char stack_doc[] = "Dump the stack.";

static int stack_func(struct debug_state *ds)
{
	for (int i = 0; i < ds->vm->stacksize; i++) {
		const size_t is_top = ds->vm->stack + i == ds->vm->stacktop;
		printf("%02d value=%d %s\n", i, ds->vm->stack[i],
		       is_top ? "<< TOP" : "");
	}
	return CONTINUE;
}

/* next */
static char next_doc[] = "Execute next instruction.";

static int next_func(struct debug_state *ds)
{
	if (ds->state == HALTED) {
		printf("Not in debug.\n");
	} else {
		const struct instr *ip = ds->vm->ip;
		const opcode_t opcode = ip->opcode;
		printf("%d %s (int=%d,str=%s)\n", opcode, opstr[opcode],
		       ip->intv, ip->strv);
		const int sts = eval_code_one_step(ds->vm, ds->code);
		if (sts < 0) {
			printf("Program aborted.\n");
			ds->state = HALTED;
		}
		if (sts == 1) {
			printf("Program ended.\n");
			ds->state = HALTED;
		}
	}
	return CONTINUE;
}

/* quit */
static char quit_doc[] = "Quit the debugger.";

static int quit_func(struct debug_state *ds)
{
	if (ds->state == RUNNING) {
		const int answer =ask_yes_no("Debugger is running a program. Exit anyway? (y or n)");
		if (!answer) {
			return CONTINUE;
		}
	}
	return QUIT;
}

/* run */
static char run_doc[] = "Run the program.";

static int run_func(struct debug_state *ds)
{
	if (ds->state == RUNNING) {
		const int answer = ask_yes_no("Already in debugging. Restart it from the beginning? (y or n)");
		if (answer) {
			ds->vm->ip = ds->code->head;
		}
	}
	ds->state = RUNNING;
	ds->vm->ip = ds->code->head;
	printf("Started.\n");
	return CONTINUE;
}

/* not implemented */
static char notimpl_doc[] = "NOT IMPLEMENTED.";

static int notimpl_func(struct debug_state *ds)
{
	fprintf(stdout, "Not implemented.\n");
	return CONTINUE;
}

static struct cmd cmds[] = {
	{"dump", dump_doc, dump_func},
	{"next", next_doc, next_func},
	{"run", run_doc, run_func},
	{"memory", mem_doc, mem_func},
	{"stack", stack_doc, stack_func},
	{"ip", ip_doc, ip_func},
	{"list", list_doc, list_func},
	{"quit", quit_doc, quit_func},
	{"help", help_doc, help_func},
	{"break", notimpl_doc, notimpl_func},
	{nullptr, nullptr, nullptr}
};

/* End of commands. */

/* Compare two strings. */
static int cmp_by_name(const char *p, const char *q)
{
	return *p == *q && strcmp(p, q) == 0;
}

/* Compare two characters (passed as strings). */
static int cmp_by_alias(const char *p, const char *q)
{
	return *p == *q;
}

static int run_command(struct debug_state *ds, const char *cmd_name)
{
	int (*cmp) (const char *, const char *);

	/* Is the input an alias? */
	cmp = (strlen(cmd_name) > 1) ? cmp_by_name : cmp_by_alias;

	/* Search the command. */
	for (struct cmd *cmd = ds->cmds; cmd->name != NULL; cmd++) {
		if ((cmp) (cmd_name, cmd->name)) {
			return (cmd->func) (ds);
		}
	}

	/* Ooops. */
	fprintf(stderr, "Undefined command '%s'; Try 'help'.\n", cmd_name);
	return CONTINUE;
}

int debug_code(struct vm *vm, struct code *code)
{
	char cmd_name[20];
	struct debug_state ds;
	struct debug_state *pds = &ds;
	pds->state = HALTED;
	pds->vm = vm;
	pds->code = code;
	pds->cmds = cmds;
	fprintf(stdout, "sem %s -- Debugger \n", PACKAGE_VERSION);
	fprintf(stdout, "Type 'help' to list available commands.\n");
	for (;;) {
		const int res = ask("sem> ", cmd_name, sizeof(cmd_name));
		if (res < 0) {
			break;
		}
		if (strcmp(cmd_name, "") == 0) {
			continue;
		}
		if (run_command(pds, cmd_name) == QUIT) {
			break;
		}
	}
	return 0;
}
