/* *INDENT-OFF* */
%{
/* *INDENT-ON* */

/*
 * scanner.l -- the lexical scanner 
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
#include "tokens.h"

/* *INDENT-OFF* */
%}
%option reentrant
%option never-interactive 
%option noyywrap 
%option yylineno 

	/* Character classes. */
L               [A-Z]|[a-z]
D               [0-9]
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
\"([^\\\"]|\\.)*\"      {
        drop_first_last_inplace(yytext, yyleng);
	return tSTRING;
}
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
