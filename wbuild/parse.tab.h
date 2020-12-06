/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_PARSE_TAB_H_INCLUDED
# define YY_YY_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ACTIONS = 258,
    CLASS = 259,
    CLASSVARS = 260,
    CONSTRAINTS = 261,
    DOCTYPE = 262,
    PRIVCONSTRAINTS = 263,
    DEF = 264,
    EXPORTS = 265,
    IMPORTS = 266,
    INCL = 267,
    METHODS = 268,
    PRIVATE = 269,
    PROC = 270,
    PUBLIC = 271,
    TAG = 272,
    TRANS = 273,
    TRANSLATIONS = 274,
    TYPE = 275,
    UTILITIES = 276,
    VAR = 277,
    BODY = 278,
    BRACKETED = 279,
    COMMENT = 280,
    CSTRING = 281,
    CTOK = 282,
    IDENT = 283,
    NUMBER = 284,
    TEXT = 285,
    DOTDOTDOT = 286,
    COLON = 287,
    COMMA = 288,
    DIRECTORY = 289,
    DOLLAR = 290,
    EQUALS = 291,
    EXCLAM = 292,
    EXTENSION = 293,
    FILE_OPT = 294,
    GUARD = 295,
    GUARDP = 296,
    ILL_BODY = 297,
    ILL_CHAR = 298,
    LBRACK = 299,
    LPAR = 300,
    NOCODE = 301,
    NODOC = 302,
    PLUS = 303,
    RBRACK = 304,
    RPAR = 305,
    SEMI = 306,
    SHORTDOC = 307,
    SLASH = 308,
    STAR = 309,
    TILDE = 310,
    UNKNOWN = 311
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 48 "parse.y" /* yacc.c:1921  */

  STRING string;
  Doctype doctype;
  Class class;
  Section section;
  Decl decl;
  taglist tag;
  int i;
  struct { char *name; char *open; char *close; } onetag;
  struct { int i; char *s; time_t t; } finfo;

#line 127 "parse.tab.h" /* yacc.c:1921  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSE_TAB_H_INCLUDED  */
