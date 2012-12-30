/* *INDENT-OFF* */
%{
/* *INDENT-ON* */

/*
 * compiler.y -- The compiler 
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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sem.h"

#if defined(WITH_PARSER_DEBUG)
# define YYDEBUG 	1
#endif

#if defined(WITH_VERBOSE_ERROR)
# define YYERROR_VERBOSE
#endif

// TODO: runtime
#ifdef WITH_COMPILER_DEBUG 
# define DPRINTF(...) printf(__VA_ARGS__) 
#else 
# define DPRINTF(...)
#endif

#define YYSTYPE 	struct op *
extern int yylex();
static void yyerror(struct Code* code, struct Parsing *parsing, const char *); 
#define error(msg) yyerror(code, parsing, (msg))

/* Emit an opcode to the list (see Code struct). */
#define emit_op(op)            emit(code, (op), -1, NULL)
#define emit_op_int(op, iv)    emit(code, (op), (iv), NULL);
#define emit_op_string(op, sv) emit(code, (op), -1, (sv))
static void emit(struct Code *, int, int, char *);

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
: { emit_op_int(SETLINENO, parsing->lineno); } tNEWLINE
| { emit_op_int(SETLINENO, parsing->lineno); } simple_stmt tNEWLINE
| error { YYABORT; }
;

simple_stmt
: halt_stmt
| set_stmt
| jump_stmt
;

halt_stmt
: kHALT {
    emit_op(HALT);
}
;

jump_stmt
: kJUMP expr {
    emit_op(JUMP);
}
| kJUMPT expr ',' test {
    emit_op(JUMPT);
}
;	

set_stmt
: kSET expr ',' expr {
    emit_op(SET);
}
| kSET rWRITE ',' expr {
    emit_op(WRITE_INT);
}
| kSET rWRITE ',' tSTRING {
    emit_op_string(WRITE_STR, parsing->str);
}
| kSET rWRITELN ',' expr {	/* this is an extension */
    emit_op(WRITELN_INT);
}
| kSET rWRITELN ',' tSTRING {	/* this is an extension */
    emit_op_string(WRITELN_STR, parsing->str);
}
| kSET expr ',' rREAD {
    emit_op(READ);
}
;

test
: relational_test
;

relational_test
: expr tEQ expr {
    emit_op(EQ);
}
| expr tNE expr {
    emit_op(NE);
}
| expr tGT expr {
    emit_op(GT);
}
| expr tLT expr {
    emit_op(LT);
}
| expr tGE expr {
    emit_op(GE);
}
| expr tLE expr {
    emit_op(LE);
}
;

expr
: additive_expr
;

additive_expr
: multiplicative_expr
| additive_expr '+' multiplicative_expr {
    emit_op(ADD);
}
| additive_expr '-' multiplicative_expr {
    emit_op(SUB);
}
;

multiplicative_expr
: atom 
| multiplicative_expr '*' atom {
    emit_op(MUL);
}
| multiplicative_expr '/' atom {		
    emit_op(DIV);
}
| multiplicative_expr '%' atom { /* this is an extension */
    emit_op(MOD);
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
    value = strtol(parsing->token, &ep, 10);

    if (errno == ERANGE) {
	error("integer literal too large");
	YYABORT;
    }

    if (*ep != '\0') {
	error("invalid integer literal");
	YYABORT;
    }

    emit_op_int(INT, value);
}
;

amem
: rD '[' expr ']' {
    emit_op(MEM);
}
;

aip
: rIP {
    emit_op(IP);
}
;
%%
  /* *INDENT-ON* */

static void
yyerror(struct Code* code, struct Parsing * parsing, const char *msg)
{
    fprintf(stderr, "%s: %s at line %d", parsing->filename, msg, parsing->lineno);

    if (parsing->token != NULL && *parsing->token != '\0') {
	fprintf(stderr, ", near token '%s'", parsing->token);
    }

    fprintf(stderr, "\n");
}


static struct Op *
op_init(int opcode, int iv, char *sv) {
  struct Op *op = xmalloc(sizeof(struct Op));
  op->opcode = opcode;
  op->intv = iv;
  op->strv = (sv != NULL) ? xstrdup(sv) : NULL;
  op->next = NULL;
  return op;
}

static void
emit(struct Code *code, int opcode, int iv, char *sv)
{
  struct Op *op = op_init(opcode, iv, sv);
  code->code->next = op;
  code->code = op;
  
  if (opcode == SETLINENO) {
    code->size += 1;
  }
}

extern FILE* yyin;

struct Code *
compile_code(const char *filename) // TODO: use FILE*
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
  struct Op *start = op_init(START, 0, NULL);
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
   */
  int j;
  struct Op *i;

  for (j = 0, i = code->head; i != NULL; i = i->next) {
    DPRINTF("COMPILE: %p (op=%d,intv=%d,strv=%s)\n", i, i->opcode, i->intv, i->strv);
    if (i->opcode == SETLINENO) {
      DPRINTF("COMPILE:  line %j jumps to %p\n", j, i);
      code->jumps[j++] = i;
    }
  }
  
  /* Add, if needed, a trailing HALT opcode. */
  struct Op *last_instr = code->code;
  switch (last_instr->opcode) {
	case HALT:
        case JUMP:
        case JUMPT:
          /* do nothing */
          break;
   
      default:
        emit_op(HALT);
	DPRINTF("COMPILE: inserting missing HALT instruction at end");
  }
  
  return code;
}

void
code_destroy(struct Code *c)
{
  struct Op *o = c->head;
  struct Op *t;

  while (o != NULL) {
	 t = o->next;

	    /* Free the string, if needed. */
	    if (o->strv != NULL) {
		free(o->strv);
            }

	    /* Free this node. */
	    free(o);
	    o = t;
    }

    free(c->jumps);
    free(c);
}
