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
#include <string.h>
#include <ctype.h>
#include "sem.h"

struct debug_state {
	int state;		// TODO: RUNNING, HALTED, STEP, TRACE..
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

/* This function expands special characters. */
char *repr(const char *s)
{ // TODO: this function is shit
	register char *p;
	register int size, i, j = 0;

	/* (size * 2) + 3 is ridiculously large enough :-) */
	size = strlen(s);
	p = (char *)xmalloc((size * 2) + 3);

	/* Begin quoting. */
	*(p + j++) = '\"';

	for (i = 0; i < size; i++) {
		register char c = *(s + i);

		if (c == '\0')
			break;

		/* Expands special character while don't touchs the others. */
		if (c == '\n')
			*(p + j++) = '\\', *(p + j++) = 'n';
		else if (c == '\t')
			*(p + j++) = '\\', *(p + j++) = 't';
		else if (c == '\r')
			*(p + j++) = '\\', *(p + j++) = 'r';
		else if (c == '\v')
			*(p + j++) = '\\', *(p + j++) = 'v';
		else if (c == '\f')
			*(p + j++) = '\\', *(p + j++) = 'f';
		else if (c == '\b')
			*(p + j++) = '\\', *(p + j++) = 'b';
		else if (c == '\a')
			*(p + j++) = '\\', *(p + j++) = 'a';
		else
			*(p + j++) = c;
	}

	/* End quoting. */
	*(p + j++) = '\"';
	*(p + j++) = '\0';
	return p;
}

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
/*	register struct instr *o;
	register struct code *c;

	for (c = VCD(v), o = CHD(c); o != NULL; o = ONX(o)) {
		fprintf(stdout, "%-20s\t", *(opstr + OOP(o)));

		if (OIV(o) != -1) {
			fprintf(stdout, "%ld", OIV(o));
			goto outnl;
		} else if (OSV(o) != NULL) {
			register char *s;

			s = repr(OSV(o));
			fprintf(stdout, "%.50s", s);
			free(s);
			goto outnl;
		} else {
 outnl:
			fprintf(stdout, "\n");
			continue;
		}
	}
*/
	return CONTINUE;
}

/* help */
static char help_doc[] = "Print this help.";

static int help_func(struct debug_state *ds)
{
/*	register struct Cmd *cmd;

	for (cmd = Cmds; cmd->name != NULL; cmd++)
		fprintf(stdout, "%-20s %-10c %-30s\n", cmd->name,
			cmd->name[0], cmd->doc);
*/
	return CONTINUE;
}

/* ip */
static char ip_doc[] = "Print the instruction pointer.";

static int ip_func(struct debug_state *ds)
{
/*	if (with(VF(v), STEP))
		fprintf(stdout, "ip is %ld.\n", VLN(v));
	else
		fprintf(stdout, "Nothing to show.\n");
*/
	return CONTINUE;
}

/* list */
static char list_doc[] = "List the SIMPLESEM source.";

static int list_func(struct debug_state *ds)
{
/*	for (int i = 0; i < CSZ(VCD(v)) - 1; i++) {
		fprintf(stdout, "%-3d %s\n", i + 1, "TODO"); // TODO: source
	}
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
		for (j = 0; i < ds->vm->memsize && j < 10; j++)
			fprintf(stdout, "%4d ", ds->vm->mem[i++]);

		/* Fill the line. */
		for (int k = 10 - j; k >= 0; k--)
			fprintf(stdout, "     ");

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
	{"next", next_doc, next_func},
	{"run", run_doc, run_func},
	{"memory", mem_doc, mem_func},
	{"break", notimpl_doc, notimpl_func},
	{"ip", ip_doc, ip_func},
	{"source", notimpl_doc, notimpl_func},
	{"quit", quit_doc, quit_func},
	{"trace", notimpl_doc, notimpl_func},
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
	int sts = 0;
	int (*cmp) (const char *, const char *);

	/* Is the input an alias? */
	cmp = (strlen(cmd_name) > 1) ? cmp_by_name : cmp_by_alias;

	/* Search the command. */
	for (cmd = cmds; cmd->name != NULL; cmd++) {
		if ((cmp) (cmd_name, cmd->name)) {
			if (cmd->func != NULL) {
				sts = (cmd->func) (ds);
				goto exit;
			} else {
				break;
			}
		}
	}

	/* Ooops. */
	fprintf(stderr, "Undefined command `%s'; Try `help'.\n", cmd_name);

 exit:
	return sts;
}

int debug_code(struct vm *vm, struct code *code)
{
	char input[20];
	struct debug_state ds;
	struct debug_state *pds = &ds;
	pds->vm = vm;
	pds->code = code;
	pds->cmds = cmds;
	fprintf(stdout, "Type `help' to get help.\n");
	for (;;) {
		ask("sem> ", input, sizeof(input));

		if (strcmp(input, "") == 0) {	// TODO: simplify
			continue;
		}

		if (run_command(pds, input) == QUIT) {
			break;
		}
	}

	return 0;
}
