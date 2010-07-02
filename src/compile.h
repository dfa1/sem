
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     kSET = 258,
     kHALT = 259,
     kJUMP = 260,
     kJUMPT = 261,
     rD = 262,
     rIP = 263,
     rREAD = 264,
     rWRITE = 265,
     rWRITELN = 266,
     tINT = 267,
     tSTRING = 268,
     tNAME = 269,
     tEQ = 270,
     tNE = 271,
     tGT = 272,
     tLT = 273,
     tGE = 274,
     tLE = 275,
     tNEWLINE = 276
   };
#endif
/* Tokens.  */
#define kSET 258
#define kHALT 259
#define kJUMP 260
#define kJUMPT 261
#define rD 262
#define rIP 263
#define rREAD 264
#define rWRITE 265
#define rWRITELN 266
#define tINT 267
#define tSTRING 268
#define tNAME 269
#define tEQ 270
#define tNE 271
#define tGT 272
#define tLT 273
#define tGE 274
#define tLE 275
#define tNEWLINE 276




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


