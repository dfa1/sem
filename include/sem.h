/*
 * sem.h 
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

#pragma once

// memory.c
extern void *xmalloc(size_t);

extern char *xstrdup(const char *);

extern char *unquote(const char *str, char *dest, size_t dest_size);

extern char *quote(const char *str, char *dest, size_t dest_size);

// io.c
extern int ask(const char *question, char *answer, int answer_size);

extern int ask_yes_no(const char *question);

extern int fetch_line_from_file(const char *filename, int lineno, char *dest, int dest_size);

/* Opcodes. */
typedef enum {
	SETLINENO,
	SET,
	JUMP,
	JUMPT,
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
} opcode_t;

struct instr {
	opcode_t opcode; /* opcode */
	int intv; /* integer argument */
	char *strv; /* string argument */
	struct instr *next; /* next opcode */
};

struct code {
	struct instr *head; /* the head */
	struct instr *code; /* the code as linked list; it grows as compiler emits opcodes */
	size_t size; /* the code size as number of lines */
	struct instr **jumps; /* jump table */
	char *filename;
};

/* The interpreter. */
struct vm {
	/* The Instruction Pointer. */
	struct instr *ip;
	int lineno;

	/*
	 * The data memory
	 * ===============
	 *
	 * This is the data memory (D). Data memory's addresses start
	 * at 0 and it can be used /only/ to store/retrieve integers.
	 */
	int *mem;
	size_t memsize;

	/*
	 * The evaluation stack
	 * ====================
	 *
	 * The stack is a fixed size, which means there's a limit on
	 * the nesting allowed in expressions.
	 */
	int *stack;
	size_t stacksize;
	int *stacktop;
};

extern struct code *compile_code(const char *filename);

extern void code_destroy(struct code *code);

extern struct vm *vm_init(size_t mem_size, size_t stack_size);

extern void vm_destroy(struct vm *vm);

extern int eval_code(struct vm *vm, struct code *code);

extern int eval_code_one_step(struct vm *vm, struct code *code);

extern int debug_code(struct vm *vm, struct code *code);
