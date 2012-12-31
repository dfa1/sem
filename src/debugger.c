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

#include "sem.h"

#include <ctype.h>		/* for isspace() */

/* Debugger's commands. */
struct Cmd
{
    char *name;
    char *doc;
    int (*cb) (struct vm *);
};

PRIVATE struct Cmd *Cmds;

/* Command's return values. */
enum
{
    CONTINUE = 0,
    QUIT
};

/* This function expands special characters. */
char *
repr(register const char *s)
{
    register char *p;
    register int size, i, j = 0;

    /* (size * 2) + 3 is ridiculously large enough :-) */
    size = strlen(s);
    p = (char *) xmalloc((size * 2) + 3);

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

/* Return an array of strings containing all the line of a file. */
PRIVATE char **
file2lines(FILE *fp, int n)
{
    char buf[1024];
    register char *o, *p, *q, **r;
    register int i = 0;

    r = (char **) xmalloc(sizeof(char *) * n);

    for (rewind(fp), p = fgets(buf, 1024, fp); !feof(fp) && p != NULL;
	 p = fgets(buf, 1024, fp)) {

	/* Cut comment, if needed. */
      q = xstrdup(trim(p));
	o = strchr(p, '#');

	if (o != NULL)
	    q[strlen(p) - strlen(o)] = '\0';

	/* Store it! */
	*(r + i++) = q;
    }

    *(r + i) = NULL;
    return r;
}

/* Re-initialize a VM struct. */
PRIVATE void
re(struct vm *v)
{
    register int i;

    /* Re-initialize the instruction pointer. */
    VIP(v) = CHD(VCD(v));
    VLN(v) = 1;

    /* Re-initialize the memory. */
    for (i = 0; i < VMS(v); i++)
	*(VMM(v) + i) = 0;

    /* Re-initialize the stack. */
    VTP(v) = VST(v);

    for (i = 0; i < VSS(v); i++)
	*(VST(v) + i) = 0;

    /* Clean flags. */
    VF(v) = 0;
}


int
ask(const char *question)
{
    const char p[20];
    int answer;

    /* 2004: This piece of code is goto powered! :-) */
    /* 2011: This piece of code is cursed! ;-(       */
  again:
    printf("%s", question);
    fgets(p, sizeof 20, stdin);

    if (p[0] == 'Y' || p[0] == 'y') {
	if (strlen(p) == 1) {
	    answer = YES;
	    goto exit;
	}
	else if (strcasecmp(p, "yes") == 0) {
	    answer = YES;
	    goto exit;
	}
	else
	    goto again;

    }
    else if (p[0] == 'N' || p[0] == 'n') {
	if (strlen(p) == 1) {
	    answer = NO;
	    goto exit;
	}
	else if (strcasecmp(p, "no") == 0) {
	    answer = NO;
	    goto exit;
	}
	else
	    goto again;

    }
    else
	goto again;

  exit:
    return answer;
}

/* 
 * Commands 
 * --------
 */

/* dump */
PRIVATE char dumpDoc[] = "Dump internal code representation.";

/* *INDENT-OFF* */
PRIVATE const char *opstr[] = {	/* TODO: gperf */
	"START",	"SET", 		"JUMP", 	"JUMPT",
	"SETLINENO", 	"INT", 		"READ", 	"WRITE_INT",
	"WRITE_STR", 	"WRITELN_INT",	"WRITELN_STR",	"MEM", 		
	"ADD", 		"SUB", 		"MUL", 		"DIV", 		
	"MOD", 		"EQ",		"NE", 		"GT", 		
	"LT", 		"GE",		"LE", 		"IP",
	"HALT"
};
/* *INDENT-ON* */

PRIVATE int
dumpCmd(struct vm *v)
{
    register struct instr *o;
    register struct code *c;

    for (c = VCD(v), o = CHD(c); o != NULL; o = ONX(o)) {
	fprintf(stdout, "%-20s\t", *(opstr + OOP(o)));

	if (OIV(o) != -1) {
	    fprintf(stdout, "%ld", OIV(o));
	    goto outnl;
	}
	else if (OSV(o) != NULL) {
	    register char *s;

	    s = repr(OSV(o));
	    fprintf(stdout, "%.50s", s);
	    free(s);
	    goto outnl;
	}
	else {
	  outnl:
	    fprintf(stdout, "\n");
	    continue;
	}
    }

    return CONTINUE;
}

/* help */
PRIVATE char helpDoc[] = "Print this help.";

PRIVATE int
helpCmd(struct vm *v)
{
    register struct Cmd *cmd;
    UNUSED(v);

    for (cmd = Cmds; cmd->name != NULL; cmd++)
	fprintf(stdout, "%-20s %-10c %-30s\n", cmd->name,
		cmd->name[0], cmd->doc);

    return 0;
}

/* ip */
PRIVATE char ipDoc[] = "Print the instruction pointer.";

PRIVATE int
ipCmd(struct vm *v)
{
    if (with(VF(v), STEP))
	fprintf(stdout, "ip is %ld.\n", VLN(v));
    else
	fprintf(stdout, "Nothing to show.\n");

    return CONTINUE;
}

/* list */
PRIVATE char listDoc[] = "List the SIMPLESEM source.";

PRIVATE int
listCmd(struct vm *v)
{
    register int i;

    for (i = 0; i < CSZ(VCD(v)) - 1; i++)
	fprintf(stdout, "%-3d %s\n", i + 1, PLI(Parsing)[i]);

    return CONTINUE;
}

/* mem */
PRIVATE char memDoc[] = "Dump the memory.";

PRIVATE int
memCmd(struct vm *v)
{
    register int i, j, k;

    for (i = 0; i < VMS(v); i++) {
	/* Print at most 10 item. */
	for (j = 0; i < VMS(v) && j < 10; j++)
	    fprintf(stdout, "%4ld ", *(VMM(v) + i++));

	/* Fill the line. */
	for (k = 10 - j; k >= 0; k--)
	    fprintf(stdout, "     ");

	/* Append the range at the end of the line. */
	fprintf(stdout, "  %4d - %4d\n", i - j, i);
    }

    return CONTINUE;
}

/* next */
PRIVATE char nextDoc[] = "Execute next instruction.";

PRIVATE int
nextCmd(struct vm *v)
{
    int sts;

    /* Going to STEP mode. */
    if (with(VF(v), STEP))
	/* void */ ;
    else {
	rewind(PFP(Parsing));
	re(v);
	set(VF(v), STEP);
	fprintf(stdout, "Step mode enabled\n");
    }

    /* Print to stdout this line. */
    fprintf(stdout, "%-3ld %s\n", VLN(v), PLI(Parsing)[VLN(v) - 1]);

    /* Execute it. */
    sts = evalCode(v);

    if (with(VF(v), HALTED)) {
	VF(v) = 0;
	fprintf(stdout, "Debug finished.\n");
    }
    else {
	if (sts == 0)
	    /* void */ ;
	else {
	    VF(v) = 0;
	    fprintf(stderr, "Debug aborted.\n");
	}
    }

    return CONTINUE;
}

/* quit */
PRIVATE char quitDoc[] = "Quit the debugger.";

PRIVATE int
quitCmd(struct vm *v)
{
    /* Cannot exit if step mode is on. */
    if (with(VF(v), STEP)) {
	int answer;

	answer = ask("The script is running. Exit anyway? (y or n) ");

	if (answer == YES)
	    goto cleanup_and_exit;
	else
	    return CONTINUE;
    }
    else
	goto cleanup_and_exit;

 cleanup_and_exit: // TODO: really useful?
    do {
	register char **p;

	/* Clean. */
	for (p = PLI(Parsing); *p != NULL; p++)
	    free(*p);

        free(PLI(Parsing));
    } while (0);

    /* Quit!!! */
    return QUIT;
}

/* run */
PRIVATE char runDoc[] = "Run the program.";

PRIVATE int
runCmd(struct vm *v)
{
    if (with(VF(v), STEP)) {
	int answer;

	fprintf(stdout, "The debug has been already started.\n");
	answer = ask("Start it from the beginning? (y or n) ");

	if (answer == YES)
	    goto run;
	else
	    goto exit;
    }

  run:
    re(v);
    (void) evalCode(v);
  exit:
    return CONTINUE;
}

/* warranty */
PRIVATE char warrantyDoc[] = "Show the warranty.";

PRIVATE int
warrantyCmd(struct vm *v)
{
    const char *warranty = "\
    This program is free software; you can redistribute it and/or modify\n\
    it under the terms of the GNU General Public License as published by\n\
    the Free Software Foundation; either version 2 of the License, or\n\
    (at your option) any later version.\n\
\n\
    This program is distributed in the hope that it will be useful,\n\
    but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
    GNU General Public License for more details.\n";

    UNUSED(v);
    fprintf(stdout, "%s", warranty);
    return CONTINUE;
}

/* not implemented */
PRIVATE char notImplDoc[] = "NOT IMPLEMENTED.";

PRIVATE int
notImplCmd(struct vm *v)
{
    UNUSED(v);
    fprintf(stderr, "Not implemented.\n");
    return CONTINUE;
}

/* End of commands. */

/* Compare two strings. */
PRIVATE int
nameCmp(const char *p, const char *q)
{
    return *p == *q && strcmp(p, q) == 0;
}

/* Compare two characters (passed as strings). */
PRIVATE int
aliasCmp(const char *p, const char *q)
{
    return *p == *q;
}

PRIVATE int
runCommand(const char *p, struct vm *v)
{
    register struct Cmd *cmd;
    register int sts = 0;
    register int (*cmp) (const char *, const char *);

    /* Is the input an alias? */
    if (strlen(p) > 1)
	cmp = nameCmp;	/* No, it isn't. */
    else
	cmp = aliasCmp;	/* Yes Sir, it's an alias! */

    /* Search the command. */
    for (cmd = Cmds; cmd->name != NULL; cmd++) {
	if ((cmp) (p, cmd->name)) {
	    /* Run it's callback. */
	    if (cmd->cb != NULL) {
		sts = (cmd->cb) (v);
		goto exit;
	    }
	    else
		break;
	}
    }

    /* Ooops. */
    fprintf(stderr, "Undefined command `%s'; Try `help'.\n", p);

  exit:
    return sts;
}

struct codeInfo {
  FILE *fp;
  char **lines;
};

int
debugCode(struct vm *vm, FILE *fp)
{
    /* TODO: gperf */
    static struct Cmd cmds[] = {	
	/* *INDENT-OFF* */
	{ "next",       nextDoc,	nextCmd		},	
	{ "run",        runDoc, 	runCmd		},
	{ "memory",     memDoc,		memCmd		},
	{ "break",      notImplDoc,	notImplCmd	},
	{ "ip",		ipDoc,		ipCmd		},
	{ "list",	listDoc,	listCmd		},
	{ "quit",       quitDoc, 	quitCmd		},
	{ "warranty",   warrantyDoc,    warrantyCmd     },
	{ "dump",       dumpDoc,        dumpCmd         },
	{ "trace",      notImplDoc,	notImplCmd	},
	{ "help",       helpDoc,	helpCmd		},
	
	/* Sentinel */
	{ NULL,         NULL,           NULL            }
	/* *INDENT-ON* */
    };
    
    Cmds = cmds;
    char**lines = file2lines(fp, CSZ(VCD(vm)));
    char s[20];
    for (fprintf(stdout, "Type `help' to get help.\n");;) {

	printf("sem> ");
	fgets(s, sizeof s, stdin);
	
	if (strcmp(s, "") == 0)
	    continue;

	if (runCommand(s, v) == 1)
	    break;
    }

    return 0;
}
