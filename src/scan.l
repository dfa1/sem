/* *INDENT-OFF* */
%{
/* *INDENT-ON* */

/*
 * scan.l -- yylex() 
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "sem.h"
#include "compile.h"

/*
 *       THE LESSER-KNOWN PROGRAMMING LANGUAGES #12: LITHP
 *
 * This otherwise unremarkable language is distinguished by the absence of
 * an "S" in its character set; users must substitute "TH".  LITHP is said
 * to be useful in protheththing lithtth.
 */

#define YY_USER_INIT		/* void */
#define YY_USER_ACTION  upd();	/* update the Parsing structure */

/* Keep me public! */
PUBLIC struct Parsing *Parsing;

/* Private stuff. */
PRIVATE struct Parsing p = {
    NULL,		/* filename     */
    NULL,		/* fp           */
    NULL,		/* lines        */
    NULL,		/* tok          */
    1,			/* lineno       */
    0,			/* offset       */
    "",			/* strtok       */
    NULL		/* strtok_p     */
};

/* Update the Parsing struct (via Parsing pointer). */
PRIVATE void upd(void);

/* *INDENT-OFF* */
%}
	/* GNU Flex options. */
%option never-interactive 
%option noyywrap 

	/* Character classes. */
L               [A-Z]|[a-z]
D               [0-9]
	
	/* Start conditions. */
%x STRING
%%
	/* Keywords. */
"halt"  		return kHALT;
"set"   		return kSET;
"jump"  		return kJUMP;
"jumpt" 		return kJUMPT;

	/* Special and/or unique and/or reserved names. */
"D"			return rD;
"ip"			return rIP;
"read"  		return rREAD;
"write" 		return rWRITE;
"writeln" 		return rWRITELN;
	
	/* Other names. Used to report errors. */
{L}*			return tNAME;

	/* Integers. */ 
{D}*  			return tINT; 

	/* Strings. */ 
"\"" {
    BEGIN STRING;
    PSP(Parsing) = PST(Parsing);
}

<STRING>"\"" {
    BEGIN INITIAL;
    *PSP(Parsing) = '\0';
    return tSTRING;
 }

<STRING>\\n   		*PSP(Parsing)++ = '\n';
<STRING>\\r   		*PSP(Parsing)++ = '\r';
<STRING>\\t   		*PSP(Parsing)++ = '\t';
<STRING>\\v   		*PSP(Parsing)++ = '\v';
<STRING>\\f   		*PSP(Parsing)++ = '\f';
<STRING>\\b   		*PSP(Parsing)++ = '\b';
<STRING>\\a   		*PSP(Parsing)++ = '\a';
<STRING>.     		*PSP(Parsing)++ = *yytext;

	/* Operators. */
"+"			return '+';
"-"			return '-';
"*"			return '*';
"/"			return '/';
"%" 			return '%';

"="			return tEQ;
"!="			return tNE;
">"			return tGT;
"<"			return tLT;
">="			return tGE;
"<="			return tLE;

	/* Other tokens. */
"\n"			return tNEWLINE;
","			return ',';
"["			return '[';
"]"			return ']';

	/* Ignore blanks. */
[ \t]+

	/* Comment. */
#[^*\n]*


<<EOF>>			yyterminate();
. 			return 0; 
%%

/* *INDENT-ON* */
PRIVATE void
upd(void)
{
    register int i;

    if (yytext != NULL && yytext[0] != '\n')
	PTK(Parsing) = yytext;
    else
	PTK(Parsing) = NULL;

    for (i = 0; yytext[i] != '\0'; i++) {
	switch (yytext[i]) {
	case '\n':
	    PLN(Parsing)++;
	    break;
	    
	case '\t':
	    POF(Parsing) += 8 - (POF(Parsing) % 8);
	    break;

	default:
	    POF(Parsing)++;
	}
    }
}

/* Initialize the compiler. */
PUBLIC int
initCompiler(const char *filename)
{
    /* Initialize the (global) Parsing structure. */
    Parsing = &p;

    /* Initialize yyin. */
    if ((yyin = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "sem: cannot open `%s'\n", filename);
	return 1;
    }
    else {
	PFL(Parsing) = xstrdup(filename);
	PFP(Parsing) = yyin;
	return 0;
    }
}

/* Finalize the compiler. */
PUBLIC void
finiCompiler(struct Code *c)
{
    register struct Op *o = CHD(c);
    register struct Op *t;

    /*
     * Anything free is worth what you pay for it.
     *      -- Anonymous
     */

    o = ONX(o);		/* XXX: skip START */

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

    /* Free other stuff. */
    free(PFL(Parsing));
    fclose(yyin);

    /* This fix compile warning "yyunput unused" (gcc). */
    unput(0);

    /* Free the current scanner's buffer and close the input stream. */
    yy_delete_buffer(YY_CURRENT_BUFFER);
}
