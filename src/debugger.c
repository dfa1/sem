/*
 * debug.c -- The debugger
 *
 * Copyright (C) 2003-2011 Davide Angelocola <davide.angelocola@gmail.com>
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sem.h"
#include "version.h"

enum {
	RUNNING,
	HALTED,
};

struct debug_state {
	int state;	
	struct vm *vm;
	struct code *code;
	struct cmd *cmds;
	char **lines;
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
	"START",	"SET", 		"JUMP", 	"JUMPT",
	"SETLINENO", 	"INT", 		"READ", 	"WRITE_INT",
	"WRITE_STR", 	"WRITELN_INT",	"WRITELN_STR",	"MEM", 		
	"ADD", 		"SUB", 		"MUL", 		"DIV", 		
	"MOD", 		"EQ",		"NE", 		"GT", 		
	"LT", 		"GE",		"LE", 		"IP",
	"HALT"
};
/* *INDENT-ON* */

static int dump_func(struct debug_state *ds)
{
	struct instr *i;

	for (i = ds->code->head; i != NULL; i = i->next) {
		fprintf(stdout, "%-20s\t", opstr[i->opcode]);

		if (i->intv != -1) { // TODO: -1 is a valid integer
			fprintf(stdout, "%d\n", i->intv);
		} else if (i->strv != NULL) {
			char *t = repr(i->strv);
			fprintf(stdout, "%.50s\n", t);
			free(t);
		} else {
			fprintf(stdout, "\n");
			continue;
		}
	}

	return CONTINUE;
}

/* help */
static char help_doc[] = "Print this help.";

static int help_func(struct debug_state *ds)
{
	struct cmd *cmd;
	for (cmd = ds->cmds; cmd->name != NULL; cmd++) {
		fprintf(stdout, "%-20s %-10c %-30s\n", cmd->name, cmd->name[0], cmd->doc);
	}
	return CONTINUE;
}

/* ip */
static char ip_doc[] = "Print the instruction pointer.";

static int ip_func(struct debug_state *ds)
{
/*	if ((VF(v), STEP))
		fprintf(stdout, "ip is %ld.\n", VLN(v));
	else
		fprintf(stdout, "Nothing to show.\n");
*/
	return CONTINUE;
}

/* mem */
static char mem_doc[] = "Dump the memory.";

static int mem_func(struct debug_state *ds)
{
	int i;
	int j;
	for (i = 0; i < ds->vm->memsize; i++) {
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

/* next */
static char next_doc[] = "Execute next instruction.";

static int next_func(struct debug_state *ds)
{
	return CONTINUE;
}

	/* Print to stdout this line. */
	//fprintf(stdout, "%-3ld %s\n", VLN(v), PLI(Parsing)[VLN(v) - 1]);

	/* Execute it. */
	//sts = evalCode(v);
/*
	if (with(VF(v), HALTED)) {
		VF(v) = 0;
		fprintf(stdout, "Debug finished.\n");
	} else {
		if (sts != 0)
			VF(v) = 0;
			fprintf(stderr, "Debug aborted.\n");
		}
	}
*/

/* quit */
static char quit_doc[] = "Quit the debugger.";

static int quit_func(struct debug_state *ds)
{
	/* Cannot exit if step mode is on. */
	//answer = ask("The script is running. Exit anyway? (y or n) ");
	return QUIT;
}

/* run */
static char run_doc[] = "Run the program.";

static int run_func(struct debug_state *ds)
{
	//fprintf(stdout, "The debug has been already started.\n");
	//answer = ask("Start it from the beginning? (y or n) ");
	return CONTINUE;
}

/* not implemented */
static char notimpl_doc[] = "NOT IMPLEMENTED.";

static int notimpl_func(struct debug_state *ds)
{
	fprintf(stderr, "Not implemented.\n");
	return CONTINUE;
}

static struct cmd cmds[] = {
	{"dump", dump_doc, dump_func},
	{"next", next_doc, next_func},
	{"run", run_doc, run_func},
	{"memory", mem_doc, mem_func},
	{"break", notimpl_doc, notimpl_func},
	{"ip", ip_doc, ip_func},
	{"source", notimpl_doc, notimpl_func},
	{"quit", quit_doc, quit_func},
	{"trace", notimpl_doc, notimpl_func},
	{"break", notimpl_doc, notimpl_func},
	{"help", help_doc, help_func},
	{NULL, NULL, NULL}
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
	struct cmd *cmd;
	int (*cmp) (const char *, const char *);

	/* Is the input an alias? */
	cmp = (strlen(cmd_name) > 1) ? cmp_by_name : cmp_by_alias;

	/* Search the command. */
	for (cmd = ds->cmds; cmd->name != NULL; cmd++) {
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
	pds->vm = vm;
	pds->code = code;
	pds->cmds = cmds;
	fprintf(stdout, "sem %s -- Debugger \n", VERSION);
	fprintf(stdout, "Type 'help' to list available commands.\n");
	for (;;) {
		int res = ask("sem> ", cmd_name, sizeof(cmd_name));
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
