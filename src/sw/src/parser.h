/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
     TIDENT = 258,
     TNUM = 259,
     TLANGLE = 260,
     TRANGLE = 261,
     TLPAR = 262,
     TRPAR = 263,
     TSLASH = 264,
     TBODY = 265,
     TR = 266,
     TD = 267,
     TCLASS = 268,
     TSPAN = 269,
     TLBRA = 270,
     TRBRA = 271,
     TCOMMA = 272,
     TPLUS = 273,
     TNL = 274
   };
#endif
/* Tokens.  */
#define TIDENT 258
#define TNUM 259
#define TLANGLE 260
#define TRANGLE 261
#define TLPAR 262
#define TRPAR 263
#define TSLASH 264
#define TBODY 265
#define TR 266
#define TD 267
#define TCLASS 268
#define TSPAN 269
#define TLBRA 270
#define TRBRA 271
#define TCOMMA 272
#define TPLUS 273
#define TNL 274




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 12 "parser.y"
{
  int value;
  char *name;
  struct ast *ast;
}
/* Line 1529 of yacc.c.  */
#line 93 "parser.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

