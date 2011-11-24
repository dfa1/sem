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

#if defined(WITH_PARSER_DEBUG)
# define YYDEBUG 	1
#endif

#if defined(WITH_VERBOSE_ERROR)
# define YYERROR_VERBOSE
#endif

#define YYSTYPE 	struct op *
extern int yylex();
static void yyerror(struct Code* code, struct Parsing *parsing, const char *); 
#define error(msg) yyerror(code, parsing, (msg))

/* Append an opcode to the list (see Code struct). */
#define addOp(op)            addOp4(code, (op), -1, NULL)
#define addOpIV(op, iv)      addOp4(code, (op), (iv), NULL);
#define addOpSV(op, sv)      addOp4(code, (op), -1, (sv))
static void addOp4(struct Code *, int, int, char *);

/* *INDENT-OFF* */
%}
%parse-param {struct Code *code}
%parse-param {struct Parsing *parsing}
%lex-param {struct Parsing *parsing}

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
%token tNEWLINE
%%

input
: none {
    fprintf(stderr, "%s: empty source\n", parsing->filename);
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
    addOpSV(WRITE_STR, parsing->str);
}
| kSET rWRITELN ',' expr {	/* this is an extension */
    addOp(WRITELN_INT);
}
| kSET rWRITELN ',' tSTRING {	/* this is an extension */
    addOpSV(WRITELN_STR, parsing->str);
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

    if (parsing->token != NULL && *parsing->token != '\0') {
	fprintf(stderr, ", near token '%s'", parsing->token);
    }

    fprintf(stderr, "\n");
}


struct Op *
createOp(int opcode, int iv, char *sv) {
  struct Op *op = xmalloc(sizeof(struct Op));
  op->opcode = opcode;
  op->intv = iv;
  op->strv = (sv != NULL) ? xstrdup(sv) : NULL;
  op->next = NULL;
  return op;
}

static void
addOp4(struct Code *code, int opcode, int iv, char *sv)
{
  struct Op *op = createOp(opcode, iv, sv);
  ONX(CCD(code)) = op;
  CCD(code) = op;
  if (opcode == SETLINENO) {
    CSZ(code) += 1;
  }
}

extern FILE* yyin;

#ifdef WITH_COMPILER_DEBUG
# define DPRINTF(...) printf(__VA_ARGS__) 
#else 
# define DPRINTF(...)
#endif

struct Code *
compileSource(const char *filename)
{
#if defined(WITH_PARSER_DEBUG)
  yydebug = 1;
#endif
  
  if ((yyin = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "sem: cannot open '%s'\n", filename);
    return NULL;
  }
  
  struct Parsing *parsing = xmalloc(sizeof(struct Parsing)); 
  parsing->token = NULL;		
  parsing->lineno = 1;		
  parsing->offset = 0;		
  parsing->filename = xstrdup(filename); 
  parsing->fp = yyin; 
  struct Op *start = createOp(START, 0, NULL);
  struct Code *code = xmalloc(sizeof(struct Code));
  code->size = 1;
  code->jumps = NULL;
  code->head = start; 
  code->code = start; 
  
  if (yyparse(code, parsing) != 0) {
    return NULL;
  }
  
  DPRINTF("code size = %d\n", code->size);
  code->jumps = (struct Op **) xmalloc(code->size * sizeof(void *));  
   
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
  struct Op *i;
  int j;
  for (j = 0, i = code->head; i != NULL; i = ONX(i)) {
    DPRINTF("COMPILE: %p (op=%d,intv=%d,strv=%s)\n", 
	    i, i->opcode, i->intv, i->strv);
    if (i->opcode == SETLINENO) {
      DPRINTF("COMPILE:  line %j jumps to %p\n", j, i);
      code->jumps[j++] = i;
    }
  }
  
  /* Add, if needed, a trailing HALT opcode. */
  if (OOP(CCD(code)) != HALT) {
    if (OOP(CCD(code)) != JUMP) {
      if (OOP(CCD(code)) != JUMPT) {
	addOp(HALT);
	DPRINTF("COMPILE: inserting missing HALT instruction at end");
      }
    }
  }
  
  return code;
}

void
finiCompiler(struct Code *c)
{
  struct Op *o = CHD(c);
  struct Op *t;

  for (;;) {
    if (o != NULL) {
	    t = ONX(o);

	    /* Free the string, if needed. */
	    if (OSV(o) != NULL)
		free(OSV(o));

	    /* Free this node. */
	    free(o);
	    o = t;
	}
	else
	    break;
    }

    free(CJM(c));
    free(c);
    fclose(yyin);
}
