/*
 * eval.c -- The interpreter (this piece of code was cursed)
 *
 * Copyright (C) 2004-2010 Davide Angelocola <davide.angelocola@gmail.com> 
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

#include <float.h>		/* for DBL_EPSILON */
#include <errno.h>		/* for errno */
#include <stdarg.h>		/* for va_start(), va_end() and va_list */

/* 
 * Proposed Additions to the PDP-11 Instruction Set:
 *
 * BBW     Branch Both Ways      D
 * BEW     Branch Either Way
 * BBBF    Branch on Bit Bucket Full
 * BH      Branch and Hang       D
 * BMR     Branch Multiple RegistDrs
 * BOB     Branch On Bug
 * BPO     Branch on Power Off
 * BST     Backspace and Stretch Tape
 * CDS     Condense and Destroy System
 * CLBR    Clobber Register
 * CLBRI   Clobber Register Immediately
 * CM      Circulate Memory
 * CMFRM   Come From -- essential for truly structured programming
 * CPPR    Crumple Printer Paper and Rip
 * CRN     Convert to Roman Numerals
 *  
 * 	-- Anonymous
 */

/* Hide GCC attributes from compilers that don't support them. */
#if !defined(__GNUC__) || __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
PRIVATE void printErr(const char *, ...)
	__attribute__ (format(printf, 1, 2));
#endif

PRIVATE void
printErr(const char *format, ...)
{
    va_list va;
    char buf[1024];

    va_start(va, format);
    (void) vsnprintf(buf, 1024, format, va);
    va_end(va);

    fprintf(stderr, "sem: %s\n", buf);
    (void) fflush(stderr);
}

PUBLIC struct VM *
initVM(struct Code *c, int ms, int ss)
{
    register struct VM *v;
    register int i;

    /* Create a new VM structure... */
    v = (struct VM *) xmalloc(sizeof(struct VM));

    VCD(v) = c;
    VIP(v) = CHD(c);

    VMS(v) = ms;
    VMM(v) = (long *) xmalloc(sizeof(long) * VMS(v));

    /* Initialize the memory. */
    for (i = 0; i < VMS(v); i++)
	*(VMM(v) + i) = 0;

    VSS(v) = ss;
    VST(v) = (long *) xmalloc(sizeof(long) * VSS(v));

    /* Initialize the stack. */
    for (i = 0; i < VSS(v); i++)
	*(VST(v) + i) = 0;

    VTP(v) = VST(v);
    VLN(v) = 1;
    VF(v) = 0;
    return v;
}

PUBLIC void
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

/* The interpreter. */
PUBLIC int
evalCode(struct VM *v)
{
    register long p;	/* first operand                */
    register long q;	/* second operand               */
    register int sts;	/* status                       */
    register char *s;	/* for xreadline()              */
    register double x;	/* for overflow checking        */
    register double y;	/* for overflow checking        */

    /* These are cloned here to .  */
    register struct Op *ip;
    register struct Code *c;
    register long lineno;

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

#define DIE(a)					\
    do {					\
	printErr a;				\
	sts = 1;				\
	goto halt;				\
    } while(0)

    /* Stack manipulation macros. It's all magic :-) */
#define TOP()           (*VTP(v))
#define LEVEL()         (VTP(v) - VST(v))
#define EMPTY()         (LEVEL() == 0)
#define POP()           (*--VTP(v))
#define PUSH(x)					\
    do {					\
	if (LEVEL() < VSS(v))			\
	    (*VTP((v))++ = (x));		\
	else					\
	    DIE(("stack overflow"));		\
    } while(0)

    /* Initialization. */
    LOAD_REGISTERS;
    sts = 0;
    s = NULL;
    
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
		DIE(("invalid memory address %d for "
		     "target at line %d", p, lineno));

	    *(VMM(v) + p) = q;
	    break;

	case MEM:
	    p = POP();

	    if (p < 0 || p >= VMS(v))
		DIE(("invalid memory address %d at "
		     "line %d", p, lineno));

	    PUSH(*(VMM(v) + p));
	    break;

	case SETLINENO:
	    lineno = OIV(ip);

	    /* Is step mode requested ? */
	    if (with(VF(v), STEP) && lineno > 1) {
		sts = 0;
		goto halt;
	    }
	    else
		break;

	case JUMP:
	    q = POP();

	    if (q < 1 || q >= CSZ(c))
		DIE(("cannot jump from line %d to "
		     "line %d", lineno, q));

	    ip = *(CJM(c) + q - 1);	/* jump */

	    if (with(VF(v), STEP)) {
		lineno = q;
		sts = 0;
		goto halt;
	    }
	    break;

	case JUMPT:
	    p = POP();
	    q = POP();

	    if (q < 1 || q >= CSZ(c))
		DIE(("cannot jump from line %d to "
		     "line %d", lineno, q));

	    if (p == 0)
		/* void */ ;
	    else {
		ip = *(CJM(c) + q - 1);	/* jump */

		if (with(VF(v), STEP)) {
		    lineno = q;
		    sts = 0;
		    goto halt;
		}
	    }
	    break;

	case HALT:
	    if (with(VF(v), STEP))
		set(VF(v), HALTED);

	    sts = 0;
	    goto halt;

	case IP:
	    PUSH(lineno + 1);
	    break;

	case WRITE_INT:
	    p = POP();
	    s = with(VF(v), STEP) ? "\n" : "";
	    fprintf(stdout, "%ld%s", p, s);
	    goto flush;

	case WRITE_STR:
	    s = with(VF(v), STEP) ? "\n" : "";
	    fprintf(stdout, "%s%s", OSV(ip), s);
	    goto flush;

	case WRITELN_INT:
	    p = POP();
	    fprintf(stdout, "%ld\n", p);
	    goto flush;

	case WRITELN_STR:
	    fprintf(stdout, "%s\n", OSV(ip));
	    goto flush;

	flush:
	    (void) fflush(stdout);
	    break;

	case READ:
	    p = POP();

	    if (p < 0 || p >= VMS(v))
		DIE(("invalid memory address %d for "
		     "read at line %d", p, lineno));

	    free(s);
	    s = readline("? "); /* FIXME: leak is possible here. */

	    if (s == NULL) {
		sts = 1;
		goto halt;
	    }
	    else
		s[strlen(s) - 1] = '\0';

	    do {
		char *ep;

		errno = 0;
		q = strtol(s, &ep, 10);

		if (errno == ERANGE)
		    DIE(("invalid integer literal '%s'", s));

		if (*ep != '\0')
		    DIE(("invalid '%c' in integer " "literal '%s' ", *ep, s));
	    } while (0);

	    *(VMM(v) + p) = q;
	    break;

#define LOAD_OP					\
	    do {				\
		q = POP();			\
		p = POP();			\
	    } while(0)

#define OVR_OP(i, d)				\
	    do {				\
		x = (double) (i);		\
		y = (d);			\
		if ((x - y) > DBL_EPSILON)	\
		    DIE(("integer overflow"));	\
		else				\
		    PUSH((i));			\
	    } while(0);				\
	    break

	case ADD:	/* p + q */
	    LOAD_OP;
	    OVR_OP(p + q, (double) p + (double) q);

	case SUB:	/* p - q */
	    LOAD_OP;
	    OVR_OP(p - q, (double) p - (double) q);

	case MUL:	/* p * q */
	    LOAD_OP;
	    OVR_OP(p * q, (double) p * (double) q);

#define DIV_OP(op)				\
	    do {				\
                if (q == 0)			\
		    DIE(("division by zero"));	\
		PUSH((op));			\
	    } while(0);				\
	    break

	case DIV:	/* p / q */
	    LOAD_OP;
	    DIV_OP(p / q);

	case MOD:	/* p % q */
	    LOAD_OP;
	    DIV_OP(p % q);

#define TEST(op)				\
	    do {				\
		LOAD_OP;			\
        	PUSH((op));			\
	    } while(0);				\
	    break

	case EQ:	/* p = q */
	    TEST(p == q);

	case NE:	/* p != q */
	    TEST(p != q);

	case GT:	/* p > q */
	    TEST(p > q);

	case LT:	/* p < q */
	    TEST(p < q);

	case GE:	/* p >= q */
	    TEST(p >= q);

	case LE:	/* p <= q */
	    TEST(p <= q);

	default:
	    DIE(("unknown opcode (%d); top is %d",
		 OOP(ip), TOP()));
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
