/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.3.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 9 "parse.y" /* yacc.c:337  */

#include <config.h>
#include <alloca.h>
#include <string.h>
#include <unistd.h>


#include <stdarg.h>

#include <stdio.h>
#include <wbuild.h>

#include <malloc.h>

#define new(ptr) ptr = calloc(1, sizeof(*(ptr))) /* set to zeros */
#ifdef YYBISON
#define yerr(m) if (! YYRECOVERING()) yyerror(m)
#else
#define yerr(m) yyerror(m)
#endif

/* extern int lineno; */
extern char *yytext;
extern int yyleng;
char *filename = "<undefined>";
time_t filetime;

/* #define alloca malloc */
/* void *alloca (); */

extern int yylex (void);
extern void err (int fatal, char *format,...);
static void yyerror (const char *s);
static void add_section (Section *psect, Section s);
static Section new_section (STRING, Decl, Section);
static char *no_quotes(char *s);


#line 109 "parse.tab.c" /* yacc.c:337  */
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parse.tab.h".  */
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
#line 48 "parse.y" /* yacc.c:352  */

  STRING string;
  Doctype doctype;
  Class class;
  Section section;
  Decl decl;
  taglist tag;
  int i;
  struct { char *name; char *open; char *close; } onetag;
  struct { int i; char *s; time_t t; } finfo;

#line 221 "parse.tab.c" /* yacc.c:352  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSE_TAB_H_INCLUDED  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  14
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   461

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  78
/* YYNRULES -- Number of rules.  */
#define YYNRULES  233
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  421

#define YYUNDEFTOK  2
#define YYMAXUTOK   311

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   193,   193,   194,   195,   196,   200,   203,   209,   212,
     216,   222,   232,   244,   245,   247,   250,   251,   255,   256,
     259,   260,   261,   262,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   283,   286,   287,   290,
     291,   292,   295,   300,   304,   307,   308,   311,   312,   317,
     321,   323,   326,   328,   329,   332,   337,   341,   346,   350,
     352,   355,   357,   359,   362,   363,   367,   372,   375,   376,
     379,   381,   382,   385,   387,   388,   392,   397,   400,   401,
     404,   405,   406,   409,   416,   421,   422,   423,   424,   427,
     429,   432,   436,   438,   441,   443,   446,   448,   450,   452,
     455,   456,   459,   461,   464,   466,   470,   473,   476,   480,
     484,   487,   488,   491,   493,   494,   497,   500,   501,   503,
     507,   510,   521,   524,   529,   534,   536,   539,   540,   543,
     544,   545,   549,   552,   553,   556,   558,   559,   562,   565,
     567,   569,   571,   572,   575,   578,   579,   583,   586,   587,
     588,   589,   592,   594,   597,   598,   599,   600,   601,   602,
     603,   604,   605,   606,   607,   608,   609,   610,   611,   612,
     613,   614,   615,   616,   617,   620,   621,   622,   625,   626,
     627,   630,   632,   633,   634,   639,   642,   643,   646,   648,
     649,   652,   655,   657,   658,   665,   668,   669,   672,   673,
     674,   677,   679,   681,   682,   685,   688,   689,   693,   698,
     701,   703,   706,   708,   709,   712,   715,   716,   720,   721,
     727,   730,   731,   734,   736,   737,   740,   741,   745,   746,
     751,   752,   753,   754
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ACTIONS", "CLASS", "CLASSVARS",
  "CONSTRAINTS", "DOCTYPE", "PRIVCONSTRAINTS", "DEF", "EXPORTS", "IMPORTS",
  "INCL", "METHODS", "PRIVATE", "PROC", "PUBLIC", "TAG", "TRANS",
  "TRANSLATIONS", "TYPE", "UTILITIES", "VAR", "BODY", "BRACKETED",
  "COMMENT", "CSTRING", "CTOK", "IDENT", "NUMBER", "TEXT", "DOTDOTDOT",
  "COLON", "COMMA", "DIRECTORY", "DOLLAR", "EQUALS", "EXCLAM", "EXTENSION",
  "FILE_OPT", "GUARD", "GUARDP", "ILL_BODY", "ILL_CHAR", "LBRACK", "LPAR",
  "NOCODE", "NODOC", "PLUS", "RBRACK", "RPAR", "SEMI", "SHORTDOC", "SLASH",
  "STAR", "TILDE", "UNKNOWN", "$accept", "program", "doctype", "docopts",
  "docopt", "class", "superclass", "supername", "options", "option",
  "sections", "publicvars", "pubvarsections", "pubvartextsections",
  "pubvardef", "opt_semi", "symbol", "constraints", "constraintsections",
  "constrainttextsections", "constraintdef", "privconstraints",
  "privconstraintsections", "privconstrainttextsections",
  "privconstraintdef", "actions", "actionsections", "actiontextsections",
  "actiondef", "translations", "transsections", "transtextsections",
  "transline", "mod_syms", "modifiers", "modifier", "event", "more_events",
  "count", "detail", "actioncalls", "actioncall", "arguments",
  "more_arguments", "methods", "methodsections", "methodtextsections",
  "methoddef", "type_and_name", "stars", "class_id", "exports",
  "exportsections", "exporttextsections", "exportdef", "macro_stuff",
  "C_stuff", "C_thing", "params", "paramlist", "more_params", "imports",
  "importsections", "importtextsections", "importdecl", "utilities",
  "utilsections", "utiltextsections", "utildef", "classvars",
  "classvarsections", "classvartextsections", "classvardecl",
  "privatevars", "privatesections", "privatetextsections", "privatedecl",
  "myerror", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311
};
# endif

#define YYPACT_NINF -331

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-331)))

#define YYTABLE_NINF -185

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     371,   324,    12,    62,   117,   371,   371,   371,  -331,  -331,
    -331,    38,  -331,   112,  -331,  -331,  -331,  -331,    40,  -331,
     112,   107,  -331,  -331,   -14,   147,   121,   136,  -331,   137,
    -331,  -331,   165,  -331,  -331,  -331,   344,   188,  -331,   344,
     143,   247,    56,   252,   298,   323,   347,   291,   223,   314,
      17,   163,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,  -331,    47,    52,   247,  -331,
    -331,   247,    53,   214,    29,    56,  -331,  -331,    56,    81,
     194,   252,  -331,  -331,   252,    83,    29,   298,  -331,  -331,
     298,   111,   257,    29,   114,    29,   323,  -331,  -331,   323,
     260,    29,    29,   347,  -331,  -331,   347,   115,    29,   291,
    -331,  -331,   291,   118,   227,    29,   223,  -331,  -331,   223,
     120,   194,   314,  -331,  -331,   314,   193,    17,  -331,  -331,
      17,   127,    29,   133,    29,   163,  -331,  -331,   163,   213,
     243,   244,   272,  -331,   247,  -331,   213,   243,   278,   270,
     274,   318,   305,  -331,    56,  -331,   213,   243,  -331,    29,
    -331,   252,  -331,   213,   243,   138,   328,  -331,   298,  -331,
     213,   243,  -331,  -331,   190,    14,   283,   312,   151,  -331,
     323,  -331,  -331,  -331,   219,   138,  -331,   347,  -331,   213,
     243,   167,   328,  -331,   291,  -331,   213,   243,   315,   138,
    -331,   223,  -331,   213,   243,    29,  -331,   314,  -331,  -331,
    -331,  -331,   333,   316,     6,   332,  -331,    17,  -331,   213,
     243,   190,   229,   337,   338,   187,  -331,   163,  -331,    23,
     340,  -331,  -331,  -331,  -331,   335,  -331,  -331,  -331,  -331,
    -331,  -331,   243,   243,  -331,  -331,   341,  -331,  -331,  -331,
    -331,  -331,   276,   339,  -331,  -331,  -331,   350,  -331,   276,
     360,   276,   288,    71,  -331,  -331,   276,  -331,   354,  -331,
     180,  -331,   355,  -331,  -331,  -331,  -331,   356,  -331,   370,
    -331,   276,   276,   276,  -331,  -331,   343,  -331,  -331,   359,
    -331,   122,  -331,   361,  -331,   276,  -331,  -331,   362,  -331,
     180,  -331,  -331,   368,  -331,   366,  -331,   372,  -331,   365,
    -331,   379,  -331,   276,   276,   276,  -331,  -331,   289,   200,
     353,    99,   243,  -331,  -331,  -331,  -331,   339,  -331,   243,
     276,   357,   276,   363,   364,  -331,  -331,   377,   276,   243,
     373,   243,   243,  -331,   276,   276,   276,  -331,   243,  -331,
    -331,   243,   276,   243,   374,  -331,  -331,   316,   369,   375,
     343,   372,   243,  -331,   276,   276,   276,    28,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,   194,
    -331,  -331,  -331,  -331,  -331,   194,   366,    31,   383,    20,
    -331,  -331,  -331,  -331,   289,   200,   276,   276,  -331,   320,
     358,   372,   378,   367,   380,  -331,  -331,   276,   276,   381,
    -331,  -331,   343,   390,  -331,  -331,  -331,  -331,  -331,   378,
    -331
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   233,     0,     9,     0,     0,     0,     0,   232,   230,
     231,    15,     9,     7,     1,     3,     2,     4,     0,    19,
       6,     0,     8,    16,     0,     0,    35,     0,    13,     0,
      14,    35,     0,    20,    21,    18,    12,     0,    17,    11,
       0,    72,   214,    54,    63,   137,   190,   115,   225,    41,
      82,   200,    25,    27,    28,    30,    31,    29,    33,    32,
      34,    24,    26,    10,    23,    22,     0,     0,    72,    67,
      69,    72,     0,     0,     0,   214,   209,   211,   214,     0,
      48,    54,    49,    51,    54,     0,     0,    63,    58,    60,
      63,     0,     0,     0,     0,     0,   137,   132,   134,   137,
       0,     0,     0,   190,   185,   187,   190,     0,     0,   115,
     110,   112,   115,     0,     0,     0,   225,   220,   222,   225,
       0,    48,    41,    36,    38,    41,     0,    82,    77,    79,
      82,     0,     0,     0,     0,   200,   195,   197,   200,   177,
     151,     0,     0,    71,    72,    68,   177,   151,     0,   130,
     128,     0,     0,   213,   214,   210,   177,   151,    47,     0,
      53,    54,    50,   177,   151,    46,     0,    62,    63,    59,
     177,   151,   138,   139,   128,     0,     0,     0,    46,   136,
     137,   133,   191,   192,   177,    46,   189,   190,   186,   177,
     151,   128,   117,   114,   115,   111,   177,   151,     0,    46,
     224,   225,   221,   177,   151,     0,    40,    41,    37,    85,
      86,    87,     0,     0,    90,     0,    81,    82,    78,   177,
     151,   128,     0,     0,     0,    46,   199,   200,   196,     0,
       0,   163,   173,   168,   172,   156,   167,   160,   166,   174,
     169,   159,   151,   151,   170,   162,   164,   161,   171,   157,
     158,    76,   150,   153,    73,    74,    70,     0,   218,     0,
       0,     0,     0,   131,   125,   126,     0,   212,     0,    57,
     128,    52,     0,    66,    45,    64,    61,     0,   147,     0,
     145,     0,     0,     0,   142,   135,    46,   193,   188,     0,
     120,     0,   113,     0,   228,     0,   226,   223,     0,    44,
     128,    39,    91,   101,    93,    95,    89,     0,    80,     0,
     208,     0,   206,     0,     0,     0,   203,   198,   130,   128,
       0,     0,   151,   155,   148,   149,   165,   152,   154,   151,
     219,     0,   215,     0,     0,   122,   127,     0,   216,   151,
       0,   151,   151,   144,   140,   141,   143,   194,   151,   116,
     118,   151,   229,   151,     0,   100,    92,     0,    99,     0,
      46,   103,   151,   205,   201,   202,   204,     0,   179,   178,
     175,   176,    75,   217,   129,   123,   124,   121,    56,    48,
      65,   146,   119,   227,    43,    48,    95,     0,     0,     0,
      84,   102,   207,   183,   130,   128,     0,     0,    94,     0,
       0,     0,   109,     0,     0,   182,   181,    55,    42,     0,
      96,    98,    46,     0,   106,   104,   105,    97,    83,   109,
     108
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -331,   376,  -331,   392,  -331,  -331,  -331,  -331,  -331,  -331,
     387,  -331,  -117,   284,   297,  -176,  -120,  -331,   -74,   342,
     345,  -331,   -63,   334,   346,  -331,   -55,   382,   384,  -331,
    -118,   295,   300,  -331,   210,  -331,    68,    42,  -331,  -331,
    -330,  -331,  -331,    10,  -331,   -98,   325,   326,   -82,  -331,
     169,  -331,   -77,   348,   349,  -102,  -107,  -208,     7,  -331,
    -294,  -331,   -89,   336,   351,  -331,  -135,   301,   302,  -331,
     -50,   385,   386,  -331,  -104,   322,   327,     0
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,    13,    22,     6,    19,    24,    26,    35,
      36,    52,   123,   124,   125,   275,   159,    53,    82,    83,
      84,    54,    88,    89,    90,    55,    69,    70,    71,    56,
     128,   129,   130,   212,   213,   214,   305,   358,   388,   356,
     360,   361,   403,   414,    57,   110,   111,   112,   150,   263,
     151,    58,    97,    98,    99,   251,   252,   253,   230,   320,
     368,    59,   104,   105,   106,    60,   136,   137,   138,    61,
      76,    77,    78,    62,   117,   118,   119,   166
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       7,   205,   284,   228,   165,     7,     7,     7,   208,   287,
     162,   174,   218,   178,   195,   202,   145,   188,    25,   184,
     185,     1,   181,   296,     1,   369,   191,   169,   155,     1,
       1,   391,     1,   199,   -88,   126,    28,   280,   209,    29,
      11,     1,   265,   210,   327,   258,   402,   127,     1,   316,
     221,  -131,   225,     1,     1,   269,  -131,  -131,   318,   393,
     399,   211,   273,   394,   149,    72,   140,   142,    23,   278,
    -107,   412,   147,  -180,   152,   139,    73,   270,    74,   157,
     141,   146,     1,    18,     1,   164,    75,   271,   290,   256,
     301,   171,   317,   175,   177,   294,   292,   297,   288,   308,
     405,   406,   299,   285,   267,   276,   149,   190,   192,   156,
     347,   163,     1,   197,    12,     1,     1,    14,   310,     1,
     204,     1,   327,   300,   327,   336,   215,   265,     1,    21,
     327,   220,   222,   224,     1,    27,   327,   327,   327,   170,
     324,   325,   176,   189,   327,   349,   196,   319,   203,   371,
     340,    31,   330,   257,   332,   219,   327,   327,   327,   338,
      32,   223,    37,   268,   350,    38,  -128,    33,    34,    64,
     272,    65,   131,  -128,   344,   345,   346,   277,   132,  -128,
     354,   279,   262,   133,   390,   134,  -128,   283,   352,   274,
    -177,   286,  -128,   135,     1,   262,   289,    30,   291,   327,
     327,    40,   274,   293,   158,  -128,   364,   365,   366,  -177,
     298,   262,   229,  -177,    63,  -128,   -48,   -90,   158,   -90,
     372,   -88,  -128,   315,   262,   209,   309,   373,   311,   321,
     210,   262,   113,   367,   262,   229,   418,   378,   274,   380,
     381,  -128,   148,   114,   262,   115,   382,  -128,   211,   383,
    -184,   384,   312,   116,  -128,   198,    66,   265,   229,   396,
     392,    79,    67,   262,   229,   397,   231,   254,   232,   233,
     234,   235,   236,  -128,    80,   237,   238,    68,   239,   240,
     241,   172,    81,   173,   182,   395,   183,   242,   243,   407,
     408,   244,   245,   246,   247,   255,   248,   249,   250,   231,
     107,   232,   233,   234,   235,   236,   108,    85,   237,   238,
     261,   239,   240,   241,   259,   260,   333,   334,   262,   281,
      86,   109,   367,   120,   244,   245,   246,   247,    87,   248,
     249,   250,    91,   265,   260,    92,   121,   335,    93,  -184,
     303,   266,   304,    94,   122,    95,   264,    41,   282,    42,
      43,   295,    44,    96,    45,    46,   265,    47,    48,   100,
      49,   302,   101,    50,   307,    51,     8,     9,   409,   102,
     410,    -5,     1,   313,   314,     2,   322,   103,     3,   323,
      10,    15,    16,    17,   328,   326,   329,   400,   331,   404,
     339,   341,   342,   343,   274,   348,   355,   351,   353,   357,
     359,   362,   363,   370,    20,   377,   206,   374,   411,   379,
     385,   413,   375,   376,   387,   401,   419,   415,    39,   207,
     389,   167,   216,   160,   306,   386,   161,   217,   398,   420,
     416,   417,   337,   168,   193,   194,   226,   227,   200,   186,
       0,     0,     0,   201,   179,   180,     0,     0,     0,     0,
     143,     0,   144,     0,   187,     0,     0,     0,     0,     0,
     153,   154
};

static const yytype_int16 yycheck[] =
{
       0,   121,   178,   138,    86,     5,     6,     7,   125,   185,
      84,    93,   130,    95,   112,   119,    71,   106,    18,   101,
     102,     1,    99,   199,     1,   319,   108,    90,    78,     1,
       1,   361,     1,   115,    28,    18,    50,    23,    32,    53,
      28,     1,    28,    37,   252,   147,    26,    30,     1,   225,
     132,    28,   134,     1,     1,   157,    28,    28,    35,    31,
      29,    55,   164,    35,    35,     9,    66,    67,    28,   171,
      50,   401,    72,    50,    74,    28,    20,   159,    22,    79,
      28,    28,     1,    45,     1,    85,    30,   161,   190,   144,
     207,    91,   227,    93,    94,   197,   194,   201,   187,   217,
     394,   395,   204,   180,   154,   168,    35,   107,   108,    28,
     286,    28,     1,   113,    52,     1,     1,     0,   220,     1,
     120,     1,   330,   205,   332,    54,   126,    28,     1,    17,
     338,   131,   132,   133,     1,    28,   344,   345,   346,    28,
     242,   243,    28,    28,   352,    23,    28,   229,    28,    50,
     270,    30,   259,   146,   261,    28,   364,   365,   366,   266,
      39,    28,    26,   156,    42,    28,    28,    46,    47,    26,
     163,    28,     9,    35,   281,   282,   283,   170,    15,    28,
     300,   174,    44,    20,   360,    22,    35,    36,   295,    51,
      23,   184,    54,    30,     1,    44,   189,    50,   191,   407,
     408,    36,    51,   196,    24,    54,   313,   314,   315,    42,
     203,    44,    45,    23,    26,    28,    36,    24,    24,    26,
     322,    28,    35,    36,    44,    32,   219,   329,   221,   229,
      37,    44,     9,    33,    44,    45,   412,   339,    51,   341,
     342,    54,    28,    20,    44,    22,   348,    28,    55,   351,
      50,   353,    23,    30,    35,    28,     9,    28,    45,   379,
     362,     9,    15,    44,    45,   385,    23,    23,    25,    26,
      27,    28,    29,    54,    22,    32,    33,    30,    35,    36,
      37,    24,    30,    26,    24,   367,    26,    44,    45,   396,
     397,    48,    49,    50,    51,    23,    53,    54,    55,    23,
       9,    25,    26,    27,    28,    29,    15,     9,    32,    33,
      36,    35,    36,    37,    36,    45,    28,    29,    44,    36,
      22,    30,    33,     9,    48,    49,    50,    51,    30,    53,
      54,    55,     9,    28,    45,    12,    22,    49,    15,    50,
      24,    36,    26,    20,    30,    22,    28,     3,    36,     5,
       6,    36,     8,    30,    10,    11,    28,    13,    14,    12,
      16,    28,    15,    19,    32,    21,    42,    43,    48,    22,
      50,     0,     1,    36,    36,     4,    36,    30,     7,    44,
      56,     5,     6,     7,    45,    44,    36,   387,    28,   389,
      36,    36,    36,    23,    51,    36,    28,    36,    36,    33,
      28,    36,    23,    50,    12,    28,   122,    50,    50,    36,
      36,    33,    49,    49,    45,    32,    26,    50,    31,   122,
      45,    87,   127,    81,   214,   357,    81,   127,   386,   419,
      50,    50,   263,    87,   109,   109,   135,   135,   116,   103,
      -1,    -1,    -1,   116,    96,    96,    -1,    -1,    -1,    -1,
      68,    -1,    68,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      75,    75
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     7,    58,    59,    62,   134,    42,    43,
      56,    28,    52,    60,     0,    58,    58,    58,    45,    63,
      60,    17,    61,    28,    64,   134,    65,    28,    50,    53,
      50,    30,    39,    46,    47,    66,    67,    26,    28,    67,
      36,     3,     5,     6,     8,    10,    11,    13,    14,    16,
      19,    21,    68,    74,    78,    82,    86,   101,   108,   118,
     122,   126,   130,    26,    26,    28,     9,    15,    30,    83,
      84,    85,     9,    20,    22,    30,   127,   128,   129,     9,
      22,    30,    75,    76,    77,     9,    22,    30,    79,    80,
      81,     9,    12,    15,    20,    22,    30,   109,   110,   111,
      12,    15,    22,    30,   119,   120,   121,     9,    15,    30,
     102,   103,   104,     9,    20,    22,    30,   131,   132,   133,
       9,    22,    30,    69,    70,    71,    18,    30,    87,    88,
      89,     9,    15,    20,    22,    30,   123,   124,   125,    28,
     134,    28,   134,    84,    85,    83,    28,   134,    28,    35,
     105,   107,   134,   128,   129,   127,    28,   134,    24,    73,
      76,    77,    75,    28,   134,   105,   134,    80,    81,    79,
      28,   134,    24,    26,   105,   134,    28,   134,   105,   110,
     111,   109,    24,    26,   105,   105,   120,   121,   119,    28,
     134,   105,   134,   103,   104,   102,    28,   134,    28,   105,
     132,   133,   131,    28,   134,    73,    70,    71,    69,    32,
      37,    55,    90,    91,    92,   134,    88,    89,    87,    28,
     134,   105,   134,    28,   134,   105,   124,   125,   123,    45,
     115,    23,    25,    26,    27,    28,    29,    32,    33,    35,
      36,    37,    44,    45,    48,    49,    50,    51,    53,    54,
      55,   112,   113,   114,    23,    23,    83,   115,   112,    36,
      45,    36,    44,   106,    28,    28,    36,   127,   115,   112,
     105,    75,   115,   112,    51,    72,    79,   115,   112,   115,
      23,    36,    36,    36,    72,   109,   115,    72,   119,   115,
     112,   115,   102,   115,   112,    36,    72,   131,   115,   112,
     105,    69,    28,    24,    26,    93,    91,    32,    87,   115,
     112,   115,    23,    36,    36,    36,    72,   123,    35,   105,
     116,   134,    36,    44,   112,   112,    44,   114,    45,    36,
     113,    28,   113,    28,    29,    49,    54,   107,   113,    36,
      73,    36,    36,    23,   113,   113,   113,    72,    36,    23,
      42,    36,   113,    36,    73,    28,    96,    33,    94,    28,
      97,    98,    36,    23,   113,   113,   113,    33,   117,   117,
      50,    50,   112,   112,    50,    49,    49,    28,   112,    36,
     112,   112,   112,   112,   112,    36,    93,    45,    95,    45,
      72,    97,   112,    31,    35,   105,    73,    73,    94,    29,
     134,    32,    26,    99,   134,   117,   117,   113,   113,    48,
      50,    50,    97,    33,   100,    50,    50,    50,    72,    26,
     100
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    57,    58,    58,    58,    58,    59,    59,    60,    60,
      61,    62,    62,    63,    63,    63,    64,    64,    65,    65,
      66,    66,    66,    66,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    68,    69,    69,    70,
      70,    70,    71,    71,    71,    72,    72,    73,    73,    74,
      75,    75,    76,    76,    76,    77,    77,    77,    78,    79,
      79,    80,    80,    80,    81,    81,    81,    82,    83,    83,
      84,    84,    84,    85,    85,    85,    85,    86,    87,    87,
      88,    88,    88,    89,    89,    90,    90,    90,    90,    91,
      91,    92,    93,    93,    94,    94,    95,    95,    95,    95,
      96,    96,    97,    97,    98,    98,    99,    99,   100,   100,
     101,   102,   102,   103,   103,   103,   104,   104,   104,   104,
     104,   105,   105,   105,   105,   105,   105,   106,   106,   107,
     107,   107,   108,   109,   109,   110,   110,   110,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   112,   112,
     112,   112,   113,   113,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   115,   115,   115,   116,   116,
     116,   117,   117,   117,   117,   118,   119,   119,   120,   120,
     120,   121,   121,   121,   121,   122,   123,   123,   124,   124,
     124,   125,   125,   125,   125,   125,   125,   125,   125,   126,
     127,   127,   128,   128,   128,   129,   129,   129,   129,   129,
     130,   131,   131,   132,   132,   132,   133,   133,   133,   133,
     134,   134,   134,   134
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     2,     0,     3,     2,     2,     0,
       4,     6,     5,     3,     3,     0,     1,     3,     2,     0,
       1,     1,     3,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     0,     2,     2,     1,     3,
       2,     0,     7,     5,     3,     1,     0,     1,     0,     2,
       2,     1,     3,     2,     0,     7,     5,     3,     2,     2,
       1,     3,     2,     0,     3,     5,     3,     2,     2,     1,
       3,     2,     0,     3,     3,     5,     3,     2,     2,     1,
       3,     2,     0,     8,     5,     1,     1,     1,     0,     2,
       0,     2,     2,     1,     3,     0,     3,     4,     3,     0,
       1,     0,     2,     1,     4,     4,     2,     0,     3,     0,
       2,     2,     1,     3,     2,     0,     4,     2,     4,     5,
       3,     4,     3,     4,     4,     2,     2,     2,     0,     4,
       1,     0,     2,     2,     1,     3,     2,     0,     2,     2,
       4,     4,     3,     4,     4,     3,     5,     3,     2,     2,
       1,     0,     2,     1,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     0,     2,     2,
       0,     3,     3,     2,     0,     2,     2,     1,     3,     2,
       0,     2,     2,     3,     4,     2,     2,     1,     3,     2,
       0,     4,     4,     3,     4,     4,     3,     5,     3,     2,
       2,     1,     3,     2,     0,     4,     4,     5,     3,     4,
       2,     2,     1,     3,     2,     0,     3,     5,     3,     4,
       2,     2,     2,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 193 "parse.y" /* yacc.c:1652  */
    {add_class((yyvsp[-1].class));}
#line 1638 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 3:
#line 194 "parse.y" /* yacc.c:1652  */
    {add_doctype((yyvsp[-1].doctype));}
#line 1644 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 4:
#line 195 "parse.y" /* yacc.c:1652  */
    {yerr("error in class");}
#line 1650 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 6:
#line 200 "parse.y" /* yacc.c:1652  */
    {new((yyval.doctype)); (yyval.doctype)->shortdoc = 1;
					copy_taglist(&((yyval.doctype)->tag), &((yyvsp[0].tag)));
					(yyval.doctype)->next = 0;}
#line 1658 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 7:
#line 203 "parse.y" /* yacc.c:1652  */
    {new((yyval.doctype)); (yyval.doctype)->shortdoc = 0;
					copy_taglist(&((yyval.doctype)->tag), &((yyvsp[0].tag)));
					(yyval.doctype)->next = 0;}
#line 1666 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 8:
#line 209 "parse.y" /* yacc.c:1652  */
    {copy_taglist(&((yyval.tag)), &((yyvsp[-1].tag)));
					set_doctag(((yyval.tag)), (yyvsp[0].onetag).name, (yyvsp[0].onetag).open,
					(yyvsp[0].onetag).close);}
#line 1674 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 9:
#line 212 "parse.y" /* yacc.c:1652  */
    {zero_taglist(&((yyval.tag)));}
#line 1680 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 10:
#line 216 "parse.y" /* yacc.c:1652  */
    {(yyval.onetag).name = get((yyvsp[-2].string));
					(yyval.onetag).open = get((yyvsp[-1].string));
					(yyval.onetag).close = get((yyvsp[0].string)); }
#line 1688 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 11:
#line 223 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[0].class); (yyval.class)->name = (yyvsp[-4].string); (yyval.class)->superclass = (yyvsp[-3].string);
				(yyval.class)->lineno = (yyvsp[-5].finfo).i; (yyval.class)->text = (yyvsp[-1].string);
				(yyval.class)->nocode = classnocode;
				(yyval.class)->nodoc = classnodoc;
				(yyval.class)->filename = hash((yyvsp[-5].finfo).s);
				(yyval.class)->filenamepart = classfilename;
				(yyval.class)->filetime = (yyvsp[-5].finfo).t;
				classnocode = classnodoc = 0;
				classfilename = 0;}
#line 1702 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 12:
#line 233 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[0].class); (yyval.class)->name = (yyvsp[-3].string); (yyval.class)->superclass = (yyvsp[-2].string);
				(yyval.class)->lineno = (yyvsp[-4].finfo).i;
				(yyval.class)->nocode = classnocode;
				(yyval.class)->nodoc = classnodoc;
				(yyval.class)->filename = hash((yyvsp[-4].finfo).s);
				(yyval.class)->filenamepart = classfilename;
				(yyval.class)->filetime = (yyvsp[-4].finfo).t;
				classnocode = classnodoc = 0;
				classfilename = 0;}
#line 1716 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 13:
#line 244 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[-1].string);}
#line 1722 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 14:
#line 245 "parse.y" /* yacc.c:1652  */
    {yerr("name of superclass expected");
				(yyval.string) = NULL;}
#line 1729 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 15:
#line 247 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = NULL;}
#line 1735 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 16:
#line 250 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 1741 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 17:
#line 251 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(3, get((yyvsp[-2].string)), "/", get((yyvsp[0].string)));
				delete((yyvsp[-2].string)); delete((yyvsp[0].string));}
#line 1748 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 18:
#line 255 "parse.y" /* yacc.c:1652  */
    {;}
#line 1754 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 19:
#line 256 "parse.y" /* yacc.c:1652  */
    {;}
#line 1760 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 20:
#line 259 "parse.y" /* yacc.c:1652  */
    {classnocode = 1; }
#line 1766 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 21:
#line 260 "parse.y" /* yacc.c:1652  */
    {classnodoc = 1; }
#line 1772 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 22:
#line 261 "parse.y" /* yacc.c:1652  */
    {classfilename = hdup((yyvsp[0].string)); }
#line 1778 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 23:
#line 262 "parse.y" /* yacc.c:1652  */
    {char *s; s = no_quotes(get((yyvsp[0].string)));
				classfilename = hash(s); free(s); delete((yyvsp[0].string));}
#line 1785 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 24:
#line 268 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->classvars, (yyvsp[0].section));}
#line 1791 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 25:
#line 269 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->publicvars, (yyvsp[0].section));}
#line 1797 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 26:
#line 270 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->privatevars, (yyvsp[0].section));}
#line 1803 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 27:
#line 271 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->constraints, (yyvsp[0].section));}
#line 1809 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 28:
#line 272 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->privconstr, (yyvsp[0].section));}
#line 1815 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 29:
#line 273 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->methods, (yyvsp[0].section));}
#line 1821 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 30:
#line 274 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->actions, (yyvsp[0].section));}
#line 1827 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 31:
#line 275 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->translations, (yyvsp[0].section));}
#line 1833 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 32:
#line 276 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->imports, (yyvsp[0].section));}
#line 1839 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 33:
#line 277 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->exports, (yyvsp[0].section));}
#line 1845 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 34:
#line 278 "parse.y" /* yacc.c:1652  */
    {(yyval.class) = (yyvsp[-1].class); add_section(&(yyval.class)->utilities, (yyvsp[0].section));}
#line 1851 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 35:
#line 279 "parse.y" /* yacc.c:1652  */
    {new((yyval.class));}
#line 1857 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 36:
#line 283 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 1863 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 37:
#line 286 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 1869 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 39:
#line 290 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 1875 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 40:
#line 291 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 1881 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 41:
#line 292 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 1887 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 42:
#line 297 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-4].decl); (yyval.decl)->typesym = (yyvsp[-5].string); (yyval.decl)->valuesym = (yyvsp[-1].string);
				(yyval.decl)->value = (yyvsp[0].string); (yyval.decl)->namesym = (yyvsp[-3].string); (yyval.decl)->lineno =
				(yyvsp[-6].finfo).i; (yyval.decl)->tp = Var;}
#line 1895 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 43:
#line 301 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 1903 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 44:
#line 304 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 1909 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 47:
#line 311 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 1915 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 48:
#line 312 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = NULL;}
#line 1921 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 49:
#line 318 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 1927 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 50:
#line 322 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 1933 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 52:
#line 327 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 1939 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 53:
#line 328 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 1945 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 54:
#line 329 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 1951 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 55:
#line 334 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-4].decl); (yyval.decl)->typesym = (yyvsp[-5].string); (yyval.decl)->valuesym = (yyvsp[-1].string);
				(yyval.decl)->value = (yyvsp[0].string); (yyval.decl)->namesym = (yyvsp[-3].string); (yyval.decl)->lineno =
				(yyvsp[-6].finfo).i; (yyval.decl)->tp = Var;}
#line 1959 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 56:
#line 338 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 1967 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 57:
#line 341 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 1973 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 58:
#line 347 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 1979 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 59:
#line 351 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 1985 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 61:
#line 356 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 1991 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 62:
#line 358 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 1997 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 63:
#line 359 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2003 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 64:
#line 362 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-1].decl); (yyval.decl)->lineno = (yyvsp[-2].finfo).i; (yyval.decl)->tp = Var;}
#line 2009 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 65:
#line 364 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 2017 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 66:
#line 367 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 2023 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 67:
#line 372 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 2029 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 68:
#line 375 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2035 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 70:
#line 380 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2041 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 71:
#line 381 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 2047 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 72:
#line 382 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2053 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 73:
#line 385 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-1].string); (yyval.decl)->body = (yyvsp[0].string);
				(yyval.decl)->lineno = (yyvsp[-2].finfo).i; (yyval.decl)->tp = Proc;}
#line 2060 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 74:
#line 387 "parse.y" /* yacc.c:1652  */
    {yerr("error in action name"); (yyval.decl) = NULL;}
#line 2066 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 75:
#line 389 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 2074 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 76:
#line 392 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 2080 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 77:
#line 397 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 2086 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 78:
#line 400 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2092 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 80:
#line 404 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2098 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 81:
#line 405 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 2104 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 82:
#line 406 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2110 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 83:
#line 411 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->tp = Trans; (yyval.decl)->type =
				catstr(4, get((yyvsp[-6].string)), get((yyvsp[-5].string)), get((yyvsp[-4].string)), get((yyvsp[-3].string)));
				(yyval.decl)->value = (yyvsp[-1].string); (yyval.decl)->lineno = (yyvsp[-7].finfo).i;
				delete((yyvsp[-6].string)); delete((yyvsp[-5].string));
				delete((yyvsp[-4].string)); delete((yyvsp[-3].string));}
#line 2120 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 84:
#line 417 "parse.y" /* yacc.c:1652  */
    {yerr("error before ':' in translation"); (yyval.decl) =
				NULL;}
#line 2127 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 85:
#line 421 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash(":");}
#line 2133 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 86:
#line 422 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("!");}
#line 2139 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 87:
#line 423 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("~");}
#line 2145 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 88:
#line 424 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = 0;}
#line 2151 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 89:
#line 427 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[-1].string)), get((yyvsp[0].string))); delete((yyvsp[-1].string));
				delete((yyvsp[0].string));}
#line 2158 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 90:
#line 429 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = 0;}
#line 2164 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 91:
#line 432 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[-1].string)), get((yyvsp[0].string))); delete((yyvsp[-1].string));
				delete((yyvsp[0].string));}
#line 2171 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 92:
#line 436 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(4, "<", get((yyvsp[-1].string)), ">", get((yyvsp[0].string)));
				delete((yyvsp[-1].string)); delete((yyvsp[0].string));}
#line 2178 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 93:
#line 438 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2184 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 94:
#line 441 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(3, ",", get((yyvsp[-1].string)), get((yyvsp[0].string)));
				delete((yyvsp[-1].string)); delete((yyvsp[0].string));}
#line 2191 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 95:
#line 443 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = 0;}
#line 2197 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 96:
#line 446 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(3, "(", get((yyvsp[-1].string)), ")");
				delete((yyvsp[-1].string));}
#line 2204 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 97:
#line 448 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(3, "(", get((yyvsp[-2].string)), "+)");
				delete((yyvsp[-2].string));}
#line 2211 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 98:
#line 450 "parse.y" /* yacc.c:1652  */
    {yerr("count expected after '('");
				(yyval.string) = 0;}
#line 2218 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 99:
#line 452 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = 0;}
#line 2224 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 100:
#line 455 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2230 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 101:
#line 456 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = 0;}
#line 2236 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 102:
#line 459 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[-1].string)), get((yyvsp[0].string))); delete((yyvsp[-1].string));
				delete((yyvsp[0].string));}
#line 2243 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 103:
#line 461 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2249 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 104:
#line 464 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(4, get((yyvsp[-3].string)), "(", get((yyvsp[-1].string)), ") ");
				delete((yyvsp[-3].string)); delete((yyvsp[-1].string));}
#line 2256 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 105:
#line 466 "parse.y" /* yacc.c:1652  */
    {yerr("error in argument");
				(yyval.string) = NULL;}
#line 2263 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 106:
#line 470 "parse.y" /* yacc.c:1652  */
    {char *s; s = no_quotes(get((yyvsp[-1].string))); delete((yyvsp[-1].string));
				(yyval.string) = catstr(2, s, get((yyvsp[0].string))); free(s);
				delete((yyvsp[0].string));}
#line 2271 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 107:
#line 473 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = NULL;}
#line 2277 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 108:
#line 476 "parse.y" /* yacc.c:1652  */
    {char *s; s = no_quotes(get((yyvsp[-1].string))); delete((yyvsp[-1].string));
				(yyval.string) = catstr(3, ",", s, get((yyvsp[0].string))); free(s);
				delete((yyvsp[0].string));}
#line 2285 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 109:
#line 480 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = NULL;}
#line 2291 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 110:
#line 484 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 2297 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 111:
#line 487 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2303 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 113:
#line 492 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2309 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 114:
#line 493 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 2315 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 115:
#line 494 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2321 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 116:
#line 498 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-2].decl); (yyval.decl)->params = (yyvsp[-1].decl); (yyval.decl)->body = (yyvsp[0].string);
				(yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->tp = Proc;}
#line 2328 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 117:
#line 500 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL; yerr("error in method");}
#line 2334 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 118:
#line 502 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL; yerr("missing closing brace");}
#line 2340 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 119:
#line 504 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 2348 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 120:
#line 507 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 2354 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 121:
#line 511 "parse.y" /* yacc.c:1652  */
    {STRING h; (yyval.decl) = (yyvsp[-3].decl);
				if ((yyval.decl)->type) {
				  h = catstr(4, get((yyval.decl)->type), " ",
				  get((yyval.decl)->name), get((yyvsp[-2].string))); delete((yyval.decl)->type);
				  (yyval.decl)->type = h; delete((yyval.decl)->name);
				} else {
				  (yyval.decl)->type = catstr(2, get((yyval.decl)->name), get((yyvsp[-2].string)));
				  delete((yyval.decl)->name);
				}
				(yyval.decl)->class_id = (yyvsp[-1].string); (yyval.decl)->name = (yyvsp[0].string);}
#line 2369 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 122:
#line 521 "parse.y" /* yacc.c:1652  */
    {STRING h; (yyval.decl) = (yyvsp[-2].decl); h = catstr(2,
				get((yyval.decl)->suffix), "[]"); delete((yyval.decl)->suffix);
				(yyval.decl)->suffix = h;}
#line 2377 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 123:
#line 525 "parse.y" /* yacc.c:1652  */
    {STRING h; (yyval.decl) = (yyvsp[-3].decl); h = catstr(4,
				get((yyval.decl)->suffix), "[", get((yyvsp[-1].string)), "]");
				delete((yyvsp[-1].string)); delete((yyval.decl)->suffix);
				(yyval.decl)->suffix = h;}
#line 2386 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 124:
#line 530 "parse.y" /* yacc.c:1652  */
    {STRING h; (yyval.decl) = (yyvsp[-3].decl); h = catstr(4,
				get((yyval.decl)->suffix), "[", get((yyvsp[-1].string)), "]");
				delete((yyvsp[-1].string)); delete((yyval.decl)->suffix);
				(yyval.decl)->suffix = h;}
#line 2395 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 125:
#line 534 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[0].string); (yyval.decl)->class_id = (yyvsp[-1].string);
				(yyval.decl)->suffix = 0;}
#line 2402 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 126:
#line 536 "parse.y" /* yacc.c:1652  */
    {yerr("error in type expression"); new((yyval.decl));}
#line 2408 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 127:
#line 539 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[-1].string)), "*");}
#line 2414 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 128:
#line 540 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash(" ");}
#line 2420 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 129:
#line 543 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[-1].string);}
#line 2426 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 130:
#line 544 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("$");}
#line 2432 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 131:
#line 545 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = 0;}
#line 2438 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 132:
#line 549 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 2444 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 133:
#line 552 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2450 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 135:
#line 557 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2456 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 136:
#line 558 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 2462 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 137:
#line 559 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2468 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 138:
#line 562 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->lineno = (yyvsp[-1].finfo).i; (yyval.decl)->name =
				catstr(3, "<", get((yyvsp[0].string)), ">"); (yyval.decl)->tp = Incl;
				delete((yyvsp[0].string));}
#line 2476 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 139:
#line 565 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->lineno = (yyvsp[-1].finfo).i; (yyval.decl)->name = (yyvsp[0].string);
				(yyval.decl)->tp = Incl;}
#line 2483 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 140:
#line 567 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->type = (yyvsp[0].string);
				(yyval.decl)->name = (yyvsp[-2].string); (yyval.decl)->tp = Type;}
#line 2490 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 141:
#line 569 "parse.y" /* yacc.c:1652  */
    {(yyvsp[0].string) = NULL;
				yerr("should be one identifier before '='");}
#line 2497 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 142:
#line 571 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-1].decl); (yyval.decl)->lineno = (yyvsp[-2].finfo).i; (yyval.decl)->tp = Var;}
#line 2503 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 143:
#line 573 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-2].decl); (yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->tp = Var;
				(yyval.decl)->value = (yyvsp[0].string);}
#line 2510 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 144:
#line 576 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-2].decl); (yyval.decl)->params = (yyvsp[-1].decl); (yyval.decl)->body = (yyvsp[0].string);
				(yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->tp = Proc;}
#line 2517 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 145:
#line 578 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL; yerr("error in function heading");}
#line 2523 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 146:
#line 580 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 2531 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 147:
#line 583 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 2537 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 148:
#line 586 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, "[", get((yyvsp[0].string))); delete((yyvsp[0].string));}
#line 2543 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 149:
#line 587 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, "(", get((yyvsp[0].string))); delete((yyvsp[0].string));}
#line 2549 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 150:
#line 588 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2555 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 151:
#line 589 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = 0;}
#line 2561 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 152:
#line 592 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[-1].string)), get((yyvsp[0].string))); delete((yyvsp[-1].string));
				delete((yyvsp[0].string));}
#line 2568 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 153:
#line 594 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2574 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 154:
#line 597 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[-1].string)), "("); delete((yyvsp[-1].string));}
#line 2580 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 155:
#line 598 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[-1].string)), "["); delete((yyvsp[-1].string));}
#line 2586 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 156:
#line 599 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[0].string)), " "); delete((yyvsp[0].string));}
#line 2592 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 157:
#line 600 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("*");}
#line 2598 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 158:
#line 601 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("~");}
#line 2604 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 159:
#line 602 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("!");}
#line 2610 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 160:
#line 603 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash(":");}
#line 2616 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 161:
#line 604 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash(";");}
#line 2622 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 162:
#line 605 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("]");}
#line 2628 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 163:
#line 606 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2634 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 164:
#line 607 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash(")");}
#line 2640 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 165:
#line 608 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash(")[");}
#line 2646 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 166:
#line 609 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash(",");}
#line 2652 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 167:
#line 610 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = catstr(2, get((yyvsp[0].string)), " "); delete((yyvsp[0].string));}
#line 2658 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 168:
#line 611 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2664 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 169:
#line 612 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("=");}
#line 2670 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 170:
#line 613 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("+");}
#line 2676 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 171:
#line 614 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("/");}
#line 2682 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 172:
#line 615 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2688 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 173:
#line 616 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = (yyvsp[0].string);}
#line 2694 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 174:
#line 617 "parse.y" /* yacc.c:1652  */
    {(yyval.string) = hash("$");}
#line 2700 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 175:
#line 620 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-1].decl);}
#line 2706 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 176:
#line 621 "parse.y" /* yacc.c:1652  */
    {yerr("error in parameter list");(yyval.decl) = NULL;}
#line 2712 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 177:
#line 622 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;}
#line 2718 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 178:
#line 625 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-1].decl); (yyval.decl)->next = (yyvsp[0].decl);}
#line 2724 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 179:
#line 626 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = hash("$"); (yyval.decl)->next = (yyvsp[0].decl);}
#line 2730 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 180:
#line 627 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;}
#line 2736 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 181:
#line 631 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-1].decl); (yyval.decl)->next = (yyvsp[0].decl);}
#line 2742 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 182:
#line 632 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = hash("$"); (yyval.decl)->next = (yyvsp[0].decl);}
#line 2748 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 183:
#line 633 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = hash("...");}
#line 2754 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 184:
#line 634 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;}
#line 2760 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 185:
#line 639 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 2766 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 186:
#line 642 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2772 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 188:
#line 647 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2778 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 189:
#line 648 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 2784 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 190:
#line 649 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2790 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 191:
#line 652 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->lineno = (yyvsp[-1].finfo).i; (yyval.decl)->name =
				catstr(3, "<", get((yyvsp[0].string)), ">"); (yyval.decl)->tp = Incl;
				delete((yyvsp[0].string));}
#line 2798 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 192:
#line 655 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->lineno = (yyvsp[-1].finfo).i; (yyval.decl)->name = (yyvsp[0].string);
				(yyval.decl)->tp = Incl;}
#line 2805 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 193:
#line 657 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-1].decl); (yyval.decl)->lineno = (yyvsp[-2].finfo).i; (yyval.decl)->tp = Var;}
#line 2811 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 194:
#line 659 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-2].decl); (yyval.decl)->params = (yyvsp[-1].decl); (yyval.decl)->lineno = (yyvsp[-3].finfo).i;
				(yyval.decl)->tp = Proc;}
#line 2818 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 195:
#line 665 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 2824 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 196:
#line 668 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2830 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 198:
#line 672 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2836 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 199:
#line 673 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 2842 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 200:
#line 674 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2848 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 201:
#line 677 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->type = (yyvsp[0].string);
				(yyval.decl)->tp = Type; (yyval.decl)->name = (yyvsp[-2].string);}
#line 2855 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 202:
#line 679 "parse.y" /* yacc.c:1652  */
    {(yyvsp[0].string) = NULL;
				yerr("should be one identifier before '='");}
#line 2862 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 203:
#line 681 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-1].decl); (yyval.decl)->lineno = (yyvsp[-2].finfo).i; (yyval.decl)->tp = Var;}
#line 2868 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 204:
#line 683 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-2].decl); (yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->tp = Var;
				(yyval.decl)->value = (yyvsp[0].string);}
#line 2875 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 205:
#line 686 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-2].decl); (yyval.decl)->params = (yyvsp[-1].decl); (yyval.decl)->body = (yyvsp[0].string);
				(yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->tp = Proc;}
#line 2882 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 206:
#line 688 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL; yerr("error in function heading");}
#line 2888 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 207:
#line 690 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 2896 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 208:
#line 693 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 2902 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 209:
#line 698 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 2908 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 210:
#line 702 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2914 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 212:
#line 707 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2920 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 213:
#line 708 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 2926 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 214:
#line 709 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2932 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 215:
#line 713 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-2].decl); (yyval.decl)->value = (yyvsp[0].string); (yyval.decl)->lineno = (yyvsp[-3].finfo).i;
				(yyval.decl)->tp = Var;}
#line 2939 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 216:
#line 715 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL; yerr("error in class variable");}
#line 2945 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 217:
#line 717 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 2953 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 218:
#line 720 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 2959 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 219:
#line 721 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->type = (yyvsp[0].string);
				(yyval.decl)->tp = Type; (yyval.decl)->name = (yyvsp[-2].string);}
#line 2966 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 220:
#line 727 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = (yyvsp[0].section);}
#line 2972 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 221:
#line 730 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section(0, (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2978 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 223:
#line 735 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-2].string), (yyvsp[-1].decl), (yyvsp[0].section));}
#line 2984 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 224:
#line 736 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = new_section((yyvsp[-1].string), (Decl) NULL, (yyvsp[0].section));}
#line 2990 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 225:
#line 737 "parse.y" /* yacc.c:1652  */
    {(yyval.section) = NULL;}
#line 2996 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 226:
#line 740 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = (yyvsp[-1].decl); (yyval.decl)->lineno = (yyvsp[-2].finfo).i; (yyval.decl)->tp = Var;}
#line 3002 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 227:
#line 742 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->name = (yyvsp[-3].string); (yyval.decl)->tp = Def;
				(yyval.decl)->body = (yyvsp[0].string); (yyval.decl)->params = (yyvsp[-2].decl); (yyval.decl)->lineno =
				(yyvsp[-4].finfo).i;}
#line 3010 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 228:
#line 745 "parse.y" /* yacc.c:1652  */
    {(yyval.decl) = NULL;yerr("incorrect macro definition");}
#line 3016 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 229:
#line 746 "parse.y" /* yacc.c:1652  */
    {new((yyval.decl)); (yyval.decl)->lineno = (yyvsp[-3].finfo).i; (yyval.decl)->type = (yyvsp[0].string);
				(yyval.decl)->tp = Type; (yyval.decl)->name = (yyvsp[-2].string);}
#line 3023 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 230:
#line 751 "parse.y" /* yacc.c:1652  */
    {yerr("illegal character");}
#line 3029 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 231:
#line 752 "parse.y" /* yacc.c:1652  */
    {yerr("unknown keyword");}
#line 3035 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 232:
#line 753 "parse.y" /* yacc.c:1652  */
    {yerr("missing closing brace");}
#line 3041 "parse.tab.c" /* yacc.c:1652  */
    break;

  case 233:
#line 754 "parse.y" /* yacc.c:1652  */
    {yerr("syntax error");}
#line 3047 "parse.tab.c" /* yacc.c:1652  */
    break;


#line 3051 "parse.tab.c" /* yacc.c:1652  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 757 "parse.y" /* yacc.c:1918  */


/*
 * new_section -- allocate a new section and initialize it
 */
static Section new_section(STRING text, Decl decl, Section next)
{
	Section h;

	new(h);
	h->text = text;
	h->decl = decl;
	h->next = next;
	return h;
}


/*
 * yyerror -- write error message to screen
 * for this to work, the parser must have been generated with option -t
 */
#ifdef __STDC__
static void yyerror(const char *s)
#else
static void yyerror(s)
  char *s;
#endif
{
# if defined(YYBISON) && defined(YYDEBUG)
  char temp[20];
  (void) strncpy(temp, yytext, 15);
  if (yyleng > 15) (void) strcpy(temp + 15, "...");
  err(0, "%s at `%s' (token = %s)\n",
    s, temp, yytname[YYTRANSLATE(yychar)]);
# else /* YYBISON && YYDEBUG*/
  err(0, "%s\n", s);
# endif /* YYBISON && YYDEBUG */
}

/*
 * add_section -- add a section to a class, append if already set
 */
static void add_section(psect, s)
  Section *psect;
  Section s;
{
  Section p;

  if (*psect == NULL) {
    *psect = s;
  } else {
    for (p = *psect; p->next; p = p->next) ; /* skip */
    p->next = s;
  }
}

/*
 * no_quotes -- make a copy of a string, but remove the quotes
 */
static char * no_quotes(s)
    char *s;
{
    char *t;

    t = strdup(s + 1);
    t[strlen(t)-1] = '\0';
    return t;
}
