/*
 * sem.h -- Just an handy header
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

/* 
 *       THE LESSER-KNOWN PROGRAMMING LANGUAGES #10: SIMPLE
 *
 * SIMPLE is an acronym for Sheer Idiot's Monopurpose Programming Language
 * Environment.  This language, developed at the Hanover College for
 * Technological Misfits, was designed to make it impossible to write code
 * with errors in it.  The statements are, therefore, confined to BEGIN,
 * END and STOP.  No matter how you arrange the statements, you can't make
 * a syntax error.  Programs written in SIMPLE do nothing useful.  Thus
 * they achieve the results of programs written in other languages without
 * the tedious, frustrating process of testing and debugging.
 */

#define VERSION "3.0"

// HACKS
#define xmalloc malloc
#define xstrdup strdup
#define trim(s) (s)

/* Features */

/* 
 * If the Bison grammar (compile.y) compiles properly but doesn't do
 * what you want when it runs, the `yydebug' parser-trace feature can
 * help you figure out why. Define the macro `WITH_PARSER_DEBUG'
 * before you compile sem. This is compliant with POSIX Yacc.
 */
/* #define WITH_PARSER_DEBUG */

/* This enables a more precise parsing error messages. */
/* #define WITH_VERBOSE_ERROR */

/* Symbols visibility. */
#define PUBLIC
#define PRIVATE	static
#define IMPORT	extern

/* Hide GCC attributes if aren't available. */
#if !defined(__GNUC__) || __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
# define ATTRIBUTE(x)
#else
# define ATTRIBUTE(x) __attribute__(x)
#endif

/* Macros for setting/clearing/getting bits in flags. */
#define set(var,flag)          (var) |= (flag)
#define clear(var,flag)        (var) |= ~(flag)
#define with(var,flag)         (((var) & (flag)) != 0)

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

/* Structures */
struct Op
{
op_t opcode:8;		/* opcode (as a bit field) */
    long intv;		/* integer argument */
    char *strv;		/* string argument */
    struct Op *next;	/* next opcode */
} ATTRIBUTE((packed));

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

struct Parsing
{
    char *filename;		/* the input */
    FILE *fp;		/* the stream */
    char **lines;		/* the file */
    char *token;		/* the current token */
    int lineno;		/* line number */
    int offset;		/* offset of the current line */
    char string[1024];	/* latest string token */
    char *string_p;		/* pointer to string */
};

/* struct Parsing access macros. */
#define PFL(p)	((p)->filename)
#define PFP(p)	((p)->fp)
#define PTK(p)	((p)->token)
#define PLN(p)	((p)->lineno)
#define POF(p)	((p)->offset)
#define PST(p)	((p)->string)
#define PSP(p)	((p)->string_p)

/* Init/Fini the compiler. */
IMPORT int initCompiler(const char *);
IMPORT void finiCompiler(struct Code *);

/* Compile a SIMPLESEM source file. */
IMPORT struct Code *compileSource(void);

/* The interpreter. */
struct VM
{
    struct Code *code;

    /* The Instruction Pointer. */
    struct Op *ip;
    long lineno;

    /*
     * The data memory
     * ===============
     *
     * This is the data memory (D). Data memory's addresses start
     * at 0 and it can be used /only/ to store/retrieve integers.
     * The memory size can changed via -m option.
     */
    long *mem;
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
    long *stack;
    int stacksize;
    long *stacktop;

    /* Flags */
#define	STEP	1 << 0		/* Eval step by step.           */
#define TRACE 	1 << 1		/* Dump opcode execution.       */
#define HALTED  1 << 2		/* Terminated.                  */
    int flags;
};

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

IMPORT struct VM *initVM(struct Code *, int, int);
IMPORT void finiVM(struct VM *);

/* The interpreter. */
IMPORT int evalCode(struct VM *);

/* The debugger. */
IMPORT int debugCode(struct VM *);

