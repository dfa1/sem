/*
 * sem.h 
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

// TODO: remove 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "version.h"

// memory.c
extern void *xmalloc(size_t);
extern char* xstrdup(const char *);

// io.c
extern void ask(const char *question, char *answer, int answer_size);
extern int ask_yes_no(const char *question);
    
/* Macros for setting/clearing/getting bits in flags. */
#define SET(var,flag)          (var) |= (flag)
#define UNSET(var,flag)        (var) |= ~(flag)
#define IS_SET(var,flag)         (((var) & (flag)) != 0)

/* Opcodes. */
typedef enum
{
    START = 0,
    SET,
    JUMP,
    JUMPT,
    SETLINENO,
    INT,
    READ,
    WRITE_INT,
    WRITE_STR,
    WRITELN_INT,
    WRITELN_STR,
    MEM,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    EQ,
    NE,
    GT,
    LT,
    GE,
    LE,
    IP,
    HALT
} op_t;

struct Op // TODO: rename to instr
{
  op_t opcode;		/* opcode */
  int intv;		/* integer argument */
  char *strv;		/* string argument */
  struct Op *next;	/* next opcode */
};

/* struct Op access macros. */
#define OOP(o)	((o)->opcode)
#define OIV(o)	((o)->intv)
#define OSV(o)	((o)->strv)
#define ONX(o)	((o)->next)

struct Code
{
    struct Op *head;	/* the head */
    struct Op *code;	/* the code (as linked list) */
    int size;		/* the code size (number of lines) */
    struct Op **jumps;	/* jumps (as linked list) */
};

/* struct Code access macros. */
#define CHD(c)	((c)->head)
#define CCD(c)	((c)->code)
#define CSZ(c)  ((c)->size)
#define CJM(c)  ((c)->jumps)

struct Parsing {
  char *filename;
  FILE *fp;	
  char *token;	
  int lineno;	
  int offset;	
  char str[1024];
  char *pstr;
};

/* The interpreter. */
struct VM
{
    struct Code *code; // TODO: remove

    /* The Instruction Pointer. */
    struct Op *ip;
    int lineno;

    /*
     * The data memory
     * ===============
     *
     * This is the data memory (D). Data memory's addresses start
     * at 0 and it can be used /only/ to store/retrieve integers.
     * The memory size can changed via -m option.
     */
    int *mem;
    int memsize;

    /*
     * The evaluation stack
     * ====================
     *
     * The stack is a fixed size, which means there's a limit on
     * the nesting allowed in expressions. A more sophisticated
     * system could let it grow dynamically but at this point is
     * useless. The stack size can be changed via -s option.
     */
    int *stack;
    int stacksize;
    int *stacktop;

    /* Flags */
#define	STEP	1 << 0		/* Eval step by step.           */
#define TRACE 	1 << 1		/* Dump opcode execution.       */
#define HALTED  1 << 2		/* Terminated.                  */
    int flags;
};

// TODO: drop
/* struct VM access macros. */
#define VCD(v)	((v)->code)
#define VIP(v)	((v)->ip)
#define VMM(v)	((v)->mem)
#define VMS(v)	((v)->memsize)
#define VST(v)	((v)->stack)
#define VSS(v)	((v)->stacksize)
#define VTP(v)	((v)->stacktop)
#define VLN(v)	((v)->lineno)
#define VF(v)	((v)->flags)

extern struct Code *compile_code(const char* filename);
extern void code_destroy(struct Code *);
extern struct VM *vm_init(struct Code *, int, int);
extern void vm_destroy(struct VM *);
extern int eval_code(struct VM *);
extern int debug_code(struct VM *);

