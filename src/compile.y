/* *INDENT-OFF* */
%{
/* *INDENT-ON* */

/*
 * compile.y -- The compiler 
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
 *
 */

#include "sem.h"

#include <errno.h>		/* for errno */

/*
 * In a five year period we can get one superb programming language. Only
 * we can't control when the five year period will begin.
 *
 * 	-- Anonymous
 */

#if defined(WITH_PARSER_DEBUG)
# define YYDEBUG 	1
#endif

#if defined(WITH_VERBOSE_ERROR)
# define YYERROR_VERBOSE
#endif

#define YYSTYPE 	struct op *
extern int yylex();

/* 
 * The GNU bison parser detects a "syntax error" or "parse error" whenever
 * it reads a token which cannot satisfy any syntax rule. This function
 * is called whenever a syntax error occurs. 
 */
 static void yyerror(struct Code* code, struct Parsing *parsing, const char *); 

#define error(msg) yyerror(code, parsing, (msg))

/* Handy macros. */
#define addOp(op)            addOp4(code, (op), -1, NULL)
#define addOpIV(op, iv)      addOp4(code, (op), (iv), NULL);
#define addOpSV(op, sv)      addOp4(code, (op), -1, (sv))

/* Append an opcode to the list (see Code). */
static void addOp4(struct Code *, int, int, char *);

/* *INDENT-OFF* */
%}
%define api.pure
%parse-param {struct Code *code}
%parse-param {struct Parsing *parsing}

/* Keywords. */
%token kSET
%token kHALT
%token kJUMP
%token kJUMPT

/* Special and/or unique and/or reserved names. */
%token rD
%token rIP
%token rREAD
%token rWRITE
%token rWRITELN

/* Literals. */
%token tINT
%token tSTRING

/* 
 * This is used to allows lexical scanner to complete names
 * such as 'rite', 'rea' when the expected keyword is 'write' or
 * 'read'.   
 */
%token tNAME	

/* Relational operators. */
%token tEQ
%token tNE
%token tGT 
%token tLT
%token tGE
%token tLE

/* This is the end of a statement. */
%token tNEWLINE
%%

input
: none {
    fprintf(stderr, "%s: empty source\n", PFL(parsing));
    YYABORT;
 }
| stmts
;

none
: /* none */
;

stmts
: stmt
| stmt stmts
;

stmt    
: { addOpIV(SETLINENO, PLN(parsing)); } tNEWLINE
| { addOpIV(SETLINENO, PLN(parsing)); } simple_stmt tNEWLINE
| error { YYABORT; }
;

simple_stmt
: halt_stmt
| set_stmt
| jump_stmt
;

halt_stmt
: kHALT {
    addOp(HALT);
}
;

jump_stmt
: kJUMP expr {
    addOp(JUMP);
}
| kJUMPT expr ',' test {
    addOp(JUMPT);
}
;	

set_stmt
: kSET expr ',' expr {
    addOp(SET);
}
| kSET rWRITE ',' expr {
    addOp(WRITE_INT);
}
| kSET rWRITE ',' tSTRING {
    addOpSV(WRITE_STR, PST(parsing));
}
| kSET rWRITELN ',' expr {	/* this is an extension */
    addOp(WRITELN_INT);
}
| kSET rWRITELN ',' tSTRING {	/* this is an extension */
    addOpSV(WRITELN_STR, PST(parsing));
}
| kSET expr ',' rREAD {
    addOp(READ);
}
;

test
: relational_test
;

relational_test
: expr tEQ expr {
    addOp(EQ);
}
| expr tNE expr {
    addOp(NE);
}
| expr tGT expr {
    addOp(GT);
}
| expr tLT expr {
    addOp(LT);
}
| expr tGE expr {
    addOp(GE);
}
| expr tLE expr {
    addOp(LE);
}
;

expr
: additive_expr
;

additive_expr
: multiplicative_expr
| additive_expr '+' multiplicative_expr {
    addOp(ADD);
}
| additive_expr '-' multiplicative_expr {
    addOp(SUB);
}
;

multiplicative_expr
: atom 
| multiplicative_expr '*' atom {
    addOp(MUL);
}
| multiplicative_expr '/' atom {		
    addOp(DIV);
}
| multiplicative_expr '%' atom { /* this is an extension */
    addOp(MOD);
}
;

atom
: literal
| amem 
| aip
;

literal
: tINT {
    long value;
    char *ep;

    errno = 0;
    value = strtol(PTK(parsing), &ep, 10);

    if (errno == ERANGE) {
	error("integer literal too large");
	YYABORT;
    }

    if (*ep != '\0') {
	error("invalid integer literal");
	YYABORT;
    }

    addOpIV(INT, value);
}
;

amem
: rD '[' expr ']' {
    addOp(MEM);
}
;

aip
: rIP {
    addOp(IP);
}
;
%%
  /* *INDENT-ON* */

void
yyerror(struct Code* code, struct Parsing * parsing, const char *msg)
{
    fprintf(stderr, "%s: %s at line %d", PFL(parsing), msg, PLN(parsing));

    if (PTK(parsing) != NULL && *PTK(parsing) != '\0') {
	fprintf(stderr, ", near token '%s'", PTK(parsing));
    }

    fprintf(stderr, "\n");
}

PRIVATE void
addOp4(struct Code *c, int op, int iv, char *sv)
{
    register struct Op *o;
	 
    /* Allocating a new Op structure... */
    o = xmalloc(sizeof(struct Op));

    /* ... and filling it... */
    OOP(o) = op;
    OIV(o) = iv;
    OSV(o) = sv != NULL ? xstrdup(sv) : NULL;
    ONX(o) = NULL;

    /* ...link it to the list... */
    ONX(CCD(c)) = o;
    CCD(c) = o;
	 
    /* and update the Code structure, if needed. */
    if (op == SETLINENO)
	CSZ(c) += 1;
}

/* This opcode is *always* the first. */
struct Op o = {
    START,		/* opcode       */
    -1,			/* intv         */
    NULL,		/* strv         */
    NULL		/* next         */
};

struct Code *
compileSource(void)
{
    struct Code *code = xmalloc(sizeof(struct Code));
    struct Parsing parsing = {
      NULL,		/* filename     */
      NULL,		/* fp           */
      NULL,		/* lines        */
      NULL,		/* tok          */
      1,			/* lineno       */
      0,			/* offset       */
      "",			/* strtok       */
      NULL		/* strtok_p     */
    };

#if defined(WITH_PARSER_DEBUG)
    yydebug = 1;
#endif
    code->size = 1;
    code->jumps = NULL;
    code->head = &o; // TODO: really necessary?
    code->code = &o; 
    
    if (yyparse(code, &parsing) == 0) {
	int j;
	struct Op *i;

	code->jumps = (struct Op **) xmalloc(code->size - 1 * sizeof(void *));  

	/*
	 * Jump-table generation. It maps the source's lines with
	 * the internal code representation. For example the
	 * internal code representation of:
	 *
	 *   set D[1] + 1, D[0]
	 *
	 * will be:
	 *
	 *   SETLINENO             19
	 *   INT                   1
	 *   MEM
	 *   INT                   1
	 *   ADD
	 *   INT                   0
	 *   MEM
	 *   SET
	 *
	 * In such case the nth SETLINENO opcode (i) will be mapped in
	 * c->jumps[18], i.e. *(CJM(c) + 18) = i.
	 */
	for (j = 0, i = code->head; i != NULL; i = ONX(i))
	    if (OOP(i) == SETLINENO)
		*(CJM(code) + j++) = i;

	/* Add, if needed, a trailing HALT opcode. */
	if (OOP(CCD(code)) != HALT)
	    if (OOP(CCD(code)) != JUMP)
		if (OOP(CCD(code)) != JUMPT)
		    addOp(HALT);

	return code;
    }
    else
	return NULL;
}
