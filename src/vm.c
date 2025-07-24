/*
 * vm.c -- The interpreter 
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "sem.h"

struct vm *vm_init(int memsize, int stacksize) {
	assert(memsize > 0);
	assert(stacksize > 0);
	struct vm *vm = (struct vm *) xmalloc(sizeof(struct vm));
	// memory
	vm->memsize = memsize;
	vm->mem = xmalloc(sizeof(int) * memsize);
	memset(vm->mem, 0, sizeof(int) * memsize);
	// stack
	vm->stacksize = stacksize;
	vm->stack = xmalloc(sizeof(int) * stacksize);
	memset(vm->stack, 0, sizeof(int) * stacksize);
	vm->stacktop = vm->stack;
	// ip
	vm->ip = NULL;
	vm->lineno = 1;
	return vm;
}

void vm_destroy(struct vm *vm) {
	assert(vm != NULL);
	free(vm->mem);
	free(vm->stack);
	free(vm);
}

/*
 * This interpreter (or virtual machine) uses an operand stack to
 * supply parameters operations, and to receive results back from
 * them. All the the instructions (opcodes) take operands from the the
 * stack, operate on them, and return results to the top of the stack.
 *
 * The operand stack follows the last-in first-out (LIFO) methodology
 * and the operands on the stack to be in the *reverse* order.
 *
 *  +-------+
 *  | START |
 *  +-------+
 *      |
 *      | <-----------------+
 *      |                   |                    ERROR
 *      |                   |           +-------------------> sts is 1
 *      |                   |           |
 *  +-------+           +-------+       |      +------+
 *  | Fetch | --------> |  Run  | ------+      | HALT | ----> sts is 0
 *  +-------+           +-------+              +------+
 *                          |                     ^
 *                          | HALT                |
 *                          |                     |
 *                          +---------------------+
 */
int eval_code(struct vm *vm, struct code *code) {
	vm->ip = code->head;

	for (;;) {
		int sts = eval_code_one_step(vm, code);
		if (sts < 0) {
			return sts;
		}

		if (sts == 1) {
			return 0;
		}
	}
}

// returns 1 on halt
// returns 0 on success
// returns < 0 on error
int eval_code_one_step(struct vm *vm, struct code *code) {
	int p; /* first operand                */
	int q; /* second operand               */
	int sts; /* status                       */

#define ERROR(...)				\
    do {					\
      fprintf(stderr, "sem: " __VA_ARGS__);	\
      fprintf(stderr, "\n");			\
      fprintf(stderr, "line: %d\n", vm->lineno);	\
      fprintf(stderr, "stack: \n");		\
      while (!EMPTY()) {			\
      	fprintf(stderr, " [%d] %d\n", (int) LEVEL(), POP());	\
      }						\
      sts = -1;					\
      goto halt;				\
    } while(0)

	/* Stack manipulation macros. */
#define TOP()           (*vm->stacktop)
#define LEVEL()         (vm->stacktop - vm->stack)
#define EMPTY()         (LEVEL() == 0)
#define POP()           (*--vm->stacktop)
#define PUSH(x)					\
    do {					\
	if (LEVEL() < vm->stacksize)		\
	    *vm->stacktop++ = (x);		\
	else					\
	    ERROR("stack overflow");		\
    } while(0)

	/* Initialization. */
	sts = 0;

	/* Instruction execution. */
	switch (vm->ip->opcode) {
		case INT:
			PUSH(vm->ip->intv);
			break;

		case SET:
			q = POP();
			p = POP();

			if (p < 0 || p >= vm->memsize) {
				ERROR("invalid memory address %d for target", p);
			}
			vm->mem[p] = q;
			break;

		case MEM:
			p = POP();

			if (p < 0 || p >= vm->memsize)
				ERROR("invalid memory address %d", p);

			PUSH(vm->mem[p]);
			break;

		case SETLINENO:
			vm->lineno = vm->ip->intv;
			break;

		case JUMP:
			q = POP();

			if (q < 1 || q >= code->size) {
				ERROR("cannot jump to line %d", q);
			}

			vm->ip = code->jumps[q - 1];
			break;

		case JUMPT:
			p = POP();
			q = POP();

			if (q < 1 || q >= code->size) {
				ERROR("cannot jump to line %d", q);
			}

			if (p != 0) {
				vm->ip = code->jumps[q - 1];
			}
			break;

		case HALT:
			sts = 1;
			goto halt;

		case IP:
			PUSH(vm->lineno + 1);
			break;

		case WRITE_INT:
			p = POP();
			printf("%d", p);
			break;

		case WRITE_STR:
			printf("%s", vm->ip->strv);
			break;

		case WRITELN_INT:
			p = POP();
			printf("%d\n", p);
			break;

		case WRITELN_STR:
			printf("%s\n", vm->ip->strv);
			break;

		case READ: {
			p = POP();

			if (p < 0 || p >= vm->memsize) {
				ERROR("invalid memory address %d for read", p);
			}

			char answer[1024];
			char *ep;
			if (ask("", answer, sizeof(answer)) < 0) {
				ERROR("EOF during read");
			}
			errno = 0;
			q = strtol(answer, &ep, 10);

			if (errno == ERANGE) {
				ERROR("invalid integer literal '%s'", answer);
			}

			if (*ep != 0) {
				ERROR("invalid '%c' in integer literal '%s' ",
				      *ep, answer);
			}
			vm->mem[p] = q;
			break;
		}

		case ADD: /* p + q */
			q = POP();
			p = POP();
			PUSH(p + q);
			break;

		case SUB: /* p - q */
			q = POP();
			p = POP();
			PUSH(p - q);
			break;

		case MUL: /* p * q */
			q = POP();
			p = POP();
			PUSH(p * q);
			break;

		case DIV: /* p / q */
			q = POP();
			p = POP();
			if (q == 0) {
				ERROR("division by zero");
			}
			PUSH(p / q);
			break;

		case MOD: /* p % q */
			q = POP();
			p = POP();
			if (q == 0) {
				ERROR("division by zero");
			}
			PUSH(p % q);
			break;

		case EQ: /* p = q */
			q = POP();
			p = POP();
			PUSH(p == q);
			break;

		case NE: /* p != q */
			q = POP();
			p = POP();
			PUSH(p != q);
			break;

		case GT: /* p > q */
			q = POP();
			p = POP();
			PUSH(p > q);
			break;

		case LT: /* p < q */
			q = POP();
			p = POP();
			PUSH(p < q);
			break;

		case GE: /* p >= q */
			q = POP();
			p = POP();
			PUSH(p >= q);
			break;

		case LE: /* p <= q */
			q = POP();
			p = POP();
			PUSH(p <= q);
			break;

		default:
			ERROR("unknown opcode (%d); top is %d", vm->ip->opcode, TOP());
	}

halt:
	/* Next instruction fetch. */
	vm->ip = vm->ip->next;

	return sts;
}
