/*
 * eval.c -- The interpreter (this piece of code was cursed)
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

struct VM *
initVM(struct Code *c, int ms, int ss)
{
    struct VM *v;
    int i;

    /* Create a new VM structure... */
    v = (struct VM *) xmalloc(sizeof(struct VM));

    VCD(v) = c;
    VIP(v) = CHD(c);

    VMS(v) = ms;
    VMM(v) = xmalloc(sizeof(int) * VMS(v));

    /* Initialize the memory. */
    for (i = 0; i < VMS(v); i++)
	*(VMM(v) + i) = 0;

    VSS(v) = ss;
    VST(v) = xmalloc(sizeof(int) * VSS(v));

    /* Initialize the stack. */
    for (i = 0; i < VSS(v); i++)
	*(VST(v) + i) = 0;

    VTP(v) = VST(v);
    VLN(v) = 1;
    VF(v) = 0;
    return v;
}

void
finiVM(struct VM *v)
{
    free(VMM(v));
    free(VST(v));
    free(v);
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
 *  | START | LOAD_REGISTERS
 *  +-------+
 *      |     
 *      | <-----------------+
 *      |                   |                    DIE
 *      |                   |           +-------------------> sts is 1
 *      |                   |           |
 *  +-------+           +-------+       |      +------+
 *  | Fetch | --------> |  Run  | ------+      | HALT | ----> sts is 0 
 *  +-------+           +-------+              +------+
 *                          |                     ^
 *                          | HALT                | SAVE_REGISTERS
 *                          |                     | 
 *                          +---------------------+
 */
int evalCode(struct VM *v)
{
    int p;	/* first operand                */
    int q;	/* second operand               */
    int sts;	/* status                       */
    char answer[20]; /* used by ask */
    struct Op *ip;
    struct Code *c;
    int lineno;

    /* Handy macros. */
#define LOAD_REGISTERS				\
    do {					\
	c = VCD(v);				\
	ip = VIP(v);				\
	lineno = VLN(v);			\
    } while(0)

#define SAVE_REGISTERS				\
    do {					\
	VIP(v) = ip;				\
	VLN(v) = lineno;			\
    } while(0)

#define DIE(...)				\
    do {					\
      fprintf(stderr, "sem: " __VA_ARGS__);	\
      fprintf(stderr, "\n");			\
      fprintf(stderr, "line: %d\n", lineno);	\
      fprintf(stderr, "stack: \n");		\
      while (!EMPTY()) {			\
      	fprintf(stderr, " [%d] %d\n", (int) LEVEL(), POP());	\
      }						\
      sts = 1;					\
      goto halt;				\
    } while(0)

    /* Stack manipulation macros. */
#define TOP()           (*VTP(v))
#define LEVEL()         (VTP(v) - VST(v))
#define EMPTY()         (LEVEL() == 0)
#define POP()           (*--VTP(v))
#define PUSH(x)					\
    do {					\
	if (LEVEL() < VSS(v))			\
	    (*VTP((v))++ = (x));		\
	else					\
	    DIE("stack overflow");		\
    } while(0)

    /* Initialization. */
    LOAD_REGISTERS;
    sts = 0;
    
    /* Main loop. */
    for (;;) {
	/* Instruction fetch. */
	ip = ONX(ip);

	/*
	 * Instruction execution. These cases are ordered in a
	 * MFUF (Most Frequently Used First) fashion, in
	 * which, the most frequently used opcode, INT, goes
	 * in the first case and so on.
	 */
	switch (OOP(ip)) {
	case INT:
	    PUSH(OIV(ip));
	    break;

	case SET:
	    q = POP();
	    p = POP();

	    if (p < 0 || p >= VMS(v))
		DIE("invalid memory address %d for target at line %d", p, lineno);

	    *(VMM(v) + p) = q;
	    break;

	case MEM:
	    p = POP();

	    if (p < 0 || p >= VMS(v))
		DIE("invalid memory address %d at "
		     "line %d", p, lineno);

	    PUSH(*(VMM(v) + p));
	    break;

	case SETLINENO:
	    lineno = OIV(ip);

	    /* Is step mode requested ? */
	    if (IS_SET(VF(v), STEP) && lineno > 1) {
		sts = 0;
		goto halt;
	    }
	    else
		break;

	case JUMP:
	    q = POP();

	    if (q < 1 || q >= CSZ(c)) {
		DIE("cannot jump from line %d to line %d", lineno, q);
	    }

	    ip = *(CJM(c) + q - 1);	/* jump */

	    if (IS_SET(VF(v), STEP)) {
		lineno = q;
		sts = 0;
		goto halt;
	    }
	    break;

	case JUMPT:
	    p = POP();
	    q = POP();

	    if (q < 1 || q >= CSZ(c))
		DIE("cannot jump from line %d to line %d", lineno, q);

	    if (p == 0)
		/* void */ ;
	    else {
		ip = *(CJM(c) + q - 1);	/* jump */

		if (IS_SET(VF(v), STEP)) {
		    lineno = q;
		    sts = 0;
		    goto halt;
		}
	    }
	    break;

	case HALT:
	    if (IS_SET(VF(v), STEP))
		SET(VF(v), HALTED);

	    sts = 0;
	    goto halt;

	case IP:
	    PUSH(lineno + 1);
	    break;

	case WRITE_INT:
	    p = POP();
	    printf("%d", p);
	    if (IS_SET(VF(v), STEP)) {
	      printf("\n");
	    }
	    break;

	case WRITE_STR:
	    printf("%s", OSV(ip));
	    if (IS_SET(VF(v), STEP)) {
	      printf("\n");
	    }
	    break;

	case WRITELN_INT:
	    p = POP();
	    printf("%d\n", p);
	    break;

	case WRITELN_STR:
	    printf("%s\n", OSV(ip));
	    break;

	case READ:
	    p = POP();

	    if (p < 0 || p >= VMS(v))
		DIE("invalid memory address %d for "
		     "read at line %d", p, lineno);
	    
	    ask("", answer, sizeof(answer));

	    do {
		char *ep;

		errno = 0;
		q = strtol(answer, &ep, 10);

		if (errno == ERANGE) {
		  DIE("invalid integer literal '%s'", answer);
		}

		if (*ep != 0) {
		  DIE("invalid '%c' in integer literal '%s' ", *ep, answer);
		}
	    } while (0);

	    *(VMM(v) + p) = q;
	    break;

	case ADD:	/* p + q */
	  q = POP();			
	  p = POP();			
	  PUSH(p + q);
	  break;

	case SUB:	/* p - q */
	  q = POP();			
	  p = POP();			
	  PUSH(p - q);
	  break;

	case MUL:	/* p * q */
	  q = POP();			
	  p = POP();			
	  PUSH(p * q);
	  break;

	case DIV:	/* p / q */
	  q = POP();			
	  p = POP();			
	  if (q == 0) {				
	    DIE("division by zero");		
	  }
	  PUSH(p / q);
	  break;

	case MOD:	/* p % q */
	  q = POP();			
	  p = POP();			
	  if (q == 0) {				
	    DIE("division by zero");		
	  }
	  PUSH(p % q);
	  break;

	case EQ:	/* p = q */
	  q = POP();			
	  p = POP();			
	  PUSH(p == q);				
	  break;

	case NE:	/* p != q */
	  q = POP();			
	  p = POP();			
	  PUSH(p != q);				
	  break;

	case GT:	/* p > q */
	  q = POP();			
	  p = POP();			
	  PUSH(p > q);				
	  break;

	case LT:	/* p < q */
	  q = POP();			
	  p = POP();			
	  PUSH(p < q);				
	  break;

	case GE:	/* p >= q */
	  q = POP();			
	  p = POP();			
	  PUSH(p >= q);				
	  break;

	case LE:	/* p <= q */
	  q = POP();			
	  p = POP();			
	  PUSH(p <= q);				
	  break;

	default:
	    DIE("unknown opcode (%d); top is %d", OOP(ip), TOP());
	}
    }
    /* End main loop. */
    
  halt:
    if (sts != 0)
	/* void */ ;
    else
	SAVE_REGISTERS;

    return sts;
}
