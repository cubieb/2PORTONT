/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     A = 258,
     B = 259,
     C = 260,
     E = 261,
     F = 262,
     I = 263,
     L = 264,
     N = 265,
     P = 266,
     R = 267,
     S = 268,
     T = 269,
     ALL = 270,
     SP = 271,
     CRLF = 272,
     COMMA = 273,
     USER = 274,
     PASS = 275,
     ACCT = 276,
     REIN = 277,
     QUIT = 278,
     PORT = 279,
     PASV = 280,
     TYPE = 281,
     STRU = 282,
     MODE = 283,
     RETR = 284,
     STOR = 285,
     APPE = 286,
     MLFL = 287,
     MAIL = 288,
     MSND = 289,
     MSOM = 290,
     MSAM = 291,
     MRSQ = 292,
     MRCP = 293,
     ALLO = 294,
     REST = 295,
     RNFR = 296,
     RNTO = 297,
     ABOR = 298,
     DELE = 299,
     CWD = 300,
     LIST = 301,
     NLST = 302,
     SITE = 303,
     STAT = 304,
     HELP = 305,
     NOOP = 306,
     MKD = 307,
     RMD = 308,
     PWD = 309,
     CDUP = 310,
     STOU = 311,
     SMNT = 312,
     SYST = 313,
     SIZE = 314,
     MDTM = 315,
     LPRT = 316,
     LPSV = 317,
     EPRT = 318,
     EPSV = 319,
     FEAT = 320,
     UMASK = 321,
     IDLE = 322,
     CHMOD = 323,
     LEXERR = 324,
     STRING = 325,
     NUMBER = 326
   };
#endif
/* Tokens.  */
#define A 258
#define B 259
#define C 260
#define E 261
#define F 262
#define I 263
#define L 264
#define N 265
#define P 266
#define R 267
#define S 268
#define T 269
#define ALL 270
#define SP 271
#define CRLF 272
#define COMMA 273
#define USER 274
#define PASS 275
#define ACCT 276
#define REIN 277
#define QUIT 278
#define PORT 279
#define PASV 280
#define TYPE 281
#define STRU 282
#define MODE 283
#define RETR 284
#define STOR 285
#define APPE 286
#define MLFL 287
#define MAIL 288
#define MSND 289
#define MSOM 290
#define MSAM 291
#define MRSQ 292
#define MRCP 293
#define ALLO 294
#define REST 295
#define RNFR 296
#define RNTO 297
#define ABOR 298
#define DELE 299
#define CWD 300
#define LIST 301
#define NLST 302
#define SITE 303
#define STAT 304
#define HELP 305
#define NOOP 306
#define MKD 307
#define RMD 308
#define PWD 309
#define CDUP 310
#define STOU 311
#define SMNT 312
#define SYST 313
#define SIZE 314
#define MDTM 315
#define LPRT 316
#define LPSV 317
#define EPRT 318
#define EPSV 319
#define FEAT 320
#define UMASK 321
#define IDLE 322
#define CHMOD 323
#define LEXERR 324
#define STRING 325
#define NUMBER 326




/* Copy the first part of user declarations.  */
#line 37 "ftpcmd.y"


#ifndef lint
static char sccsid[] = "@(#)ftpcmd.y	8.3 (Berkeley) 4/6/94";
#endif /* not lint */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/ftp.h>

#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <unistd.h>
#include <limits.h>
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
/* Include glob.h last, because it may define "const" which breaks
   system headers on some platforms. */
#include <glob.h>
#include <netdb.h>

#include "extern.h"

#if ! defined (NBBY) && defined (CHAR_BIT)
#define NBBY CHAR_BIT
#endif

off_t restart_point;

#ifdef FULL_FTPD
static char cbuf[512];           /* Command Buffer.  */
#else
static char cbuf[128];           /* Command Buffer.  */
#endif
static char *fromname;
static int cmd_type;
static int cmd_form;
static int cmd_bytesz;

extern struct tab sitetab[];
static char *copy         __P ((char *));
static void help          __P ((struct tab *, char *));
static struct tab *lookup __P ((struct tab *, const char *));
static void sizecmd       __P ((char *));
static int yylex          __P ((void));
static void yyerror       __P ((const char *s));
static int port_check(const char *);
static int port_check_v6(const char *);
static void v4map_data_dest();



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 115 "ftpcmd.y"
{
	off_t	o;
	char   *s;
}
/* Line 193 of yacc.c.  */
#line 321 "y.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 334 "y.tab.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   301

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  84
/* YYNRULES -- Number of states.  */
#define YYNSTATES  274

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    15,    20,    26,    32,
      38,    42,    46,    52,    58,    62,    65,    70,    75,    80,
      85,    94,   100,   106,   112,   116,   122,   126,   132,   138,
     141,   147,   153,   156,   160,   166,   169,   174,   177,   183,
     189,   193,   197,   202,   209,   215,   223,   233,   238,   246,
     252,   255,   261,   267,   270,   273,   279,   284,   286,   287,
     289,   291,   303,   345,   363,   365,   367,   369,   371,   375,
     377,   381,   383,   385,   389,   392,   394,   396,   398,   400,
     402,   404,   406,   408,   410
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      73,     0,    -1,    -1,    73,    74,    -1,    73,    75,    -1,
      19,    16,    76,    17,    -1,    20,    16,    77,    17,    -1,
      24,    88,    16,    79,    17,    -1,    61,    88,    16,    80,
      17,    -1,    63,    88,    16,    70,    17,    -1,    25,    88,
      17,    -1,    62,    88,    17,    -1,    64,    88,    16,    71,
      17,    -1,    64,    88,    16,    15,    17,    -1,    64,    88,
      17,    -1,    65,    17,    -1,    26,    16,    82,    17,    -1,
      27,    16,    83,    17,    -1,    28,    16,    84,    17,    -1,
      39,    16,    71,    17,    -1,    39,    16,    71,    16,    12,
      16,    71,    17,    -1,    29,    88,    16,    85,    17,    -1,
      30,    88,    16,    85,    17,    -1,    31,    88,    16,    85,
      17,    -1,    47,    88,    17,    -1,    47,    88,    16,    70,
      17,    -1,    46,    88,    17,    -1,    46,    88,    16,    85,
      17,    -1,    49,    88,    16,    85,    17,    -1,    49,    17,
      -1,    44,    88,    16,    85,    17,    -1,    42,    88,    16,
      85,    17,    -1,    43,    17,    -1,    45,    88,    17,    -1,
      45,    88,    16,    85,    17,    -1,    50,    17,    -1,    50,
      16,    70,    17,    -1,    51,    17,    -1,    52,    88,    16,
      85,    17,    -1,    53,    88,    16,    85,    17,    -1,    54,
      88,    17,    -1,    55,    88,    17,    -1,    48,    16,    50,
      17,    -1,    48,    16,    50,    16,    70,    17,    -1,    48,
      16,    66,    88,    17,    -1,    48,    16,    66,    88,    16,
      87,    17,    -1,    48,    16,    68,    88,    16,    87,    16,
      85,    17,    -1,    48,    16,    67,    17,    -1,    48,    16,
      88,    67,    16,    71,    17,    -1,    56,    88,    16,    85,
      17,    -1,    58,    17,    -1,    59,    88,    16,    85,    17,
      -1,    60,    88,    16,    85,    17,    -1,    23,    17,    -1,
       1,    17,    -1,    41,    88,    16,    85,    17,    -1,    40,
      16,    71,    17,    -1,    70,    -1,    -1,    70,    -1,    71,
      -1,    71,    18,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    -1,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    18,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    18,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    18,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    18,    71,    -1,    71,    18,    71,    18,    71,
      18,    71,    18,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    -1,    10,    -1,    14,    -1,     5,    -1,     3,
      -1,     3,    16,    81,    -1,     6,    -1,     6,    16,    81,
      -1,     8,    -1,     9,    -1,     9,    16,    78,    -1,     9,
      78,    -1,     7,    -1,    12,    -1,    11,    -1,    13,    -1,
       4,    -1,     5,    -1,    86,    -1,    70,    -1,    71,    -1,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   154,   154,   156,   163,   167,   172,   177,   200,   222,
     322,   329,   336,   356,   364,   369,   373,   408,   420,   432,
     436,   440,   447,   454,   461,   466,   473,   478,   485,   492,
     496,   503,   516,   520,   525,   532,   536,   553,   557,   564,
     571,   576,   581,   585,   591,   601,   616,   630,   636,   652,
     659,   705,   722,   744,   749,   755,   766,   781,   786,   789,
     793,   800,   814,   840,   858,   862,   866,   873,   878,   883,
     888,   893,   897,   902,   908,   916,   920,   924,   931,   935,
     939,   946,   983,   987,  1015
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "A", "B", "C", "E", "F", "I", "L", "N",
  "P", "R", "S", "T", "ALL", "SP", "CRLF", "COMMA", "USER", "PASS", "ACCT",
  "REIN", "QUIT", "PORT", "PASV", "TYPE", "STRU", "MODE", "RETR", "STOR",
  "APPE", "MLFL", "MAIL", "MSND", "MSOM", "MSAM", "MRSQ", "MRCP", "ALLO",
  "REST", "RNFR", "RNTO", "ABOR", "DELE", "CWD", "LIST", "NLST", "SITE",
  "STAT", "HELP", "NOOP", "MKD", "RMD", "PWD", "CDUP", "STOU", "SMNT",
  "SYST", "SIZE", "MDTM", "LPRT", "LPSV", "EPRT", "EPSV", "FEAT", "UMASK",
  "IDLE", "CHMOD", "LEXERR", "STRING", "NUMBER", "$accept", "cmd_list",
  "cmd", "rcmd", "username", "password", "byte_size", "host_port",
  "host_long_port", "form_code", "type_code", "struct_code", "mode_code",
  "pathname", "pathstring", "octal_number", "check_login", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    72,    73,    73,    73,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    75,    75,    76,    77,    77,
      78,    79,    80,    80,    81,    81,    81,    82,    82,    82,
      82,    82,    82,    82,    82,    83,    83,    83,    84,    84,
      84,    85,    86,    87,    88
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     4,     4,     5,     5,     5,
       3,     3,     5,     5,     3,     2,     4,     4,     4,     4,
       8,     5,     5,     5,     3,     5,     3,     5,     5,     2,
       5,     5,     2,     3,     5,     2,     4,     2,     5,     5,
       3,     3,     4,     6,     5,     7,     9,     4,     7,     5,
       2,     5,     5,     2,     2,     5,     4,     1,     0,     1,
       1,    11,    41,    17,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,    84,    84,     0,
       0,     0,    84,    84,    84,     0,     0,    84,    84,     0,
      84,    84,    84,    84,     0,    84,     0,     0,    84,    84,
      84,    84,    84,     0,    84,    84,    84,    84,    84,    84,
       0,     3,     4,    54,     0,    58,    53,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    32,
       0,     0,     0,     0,     0,    29,     0,     0,    35,    37,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,    15,    57,     0,    59,     0,     0,    10,    67,
      69,    71,    72,     0,    75,    77,    76,     0,    79,    80,
      78,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    33,     0,    26,     0,    24,     0,    84,     0,    84,
       0,     0,     0,     0,     0,    40,    41,     0,     0,     0,
       0,    11,     0,     0,    14,     5,     6,     0,     0,     0,
       0,     0,    60,    74,    16,    17,    18,    82,     0,    81,
       0,     0,     0,    19,    56,     0,     0,     0,     0,     0,
       0,     0,    42,     0,    47,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       7,    66,    64,    65,    68,    70,    73,    21,    22,    23,
       0,    55,    31,    30,    34,    27,    25,     0,     0,    44,
       0,     0,    28,    38,    39,    49,    51,    52,     0,     8,
       9,    13,    12,     0,     0,    43,    83,     0,     0,     0,
       0,     0,     0,    45,     0,    48,     0,     0,    20,     0,
       0,     0,    46,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    61,     0,     0,     0,     0,     0,     0,    63,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    62
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    41,    42,    84,    86,   143,   138,   175,   184,
      93,    97,   101,   148,   149,   217,    47
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -104
static const yytype_int16 yypact[] =
{
    -104,    41,  -104,   -14,    -1,    18,    31,  -104,  -104,    43,
      46,    82,  -104,  -104,  -104,    99,   106,  -104,  -104,   107,
    -104,  -104,  -104,  -104,   109,   110,    -3,   111,  -104,  -104,
    -104,  -104,  -104,   112,  -104,  -104,  -104,  -104,  -104,  -104,
     113,  -104,  -104,  -104,   -48,    53,  -104,   115,   116,   105,
       5,     6,   119,   120,   121,    55,    67,   123,   124,  -104,
     125,    14,    57,    59,   -11,  -104,   126,    73,  -104,  -104,
     128,   129,   130,   131,   133,  -104,   134,   135,   136,   137,
     139,    61,  -104,  -104,   140,  -104,   141,    75,  -104,   143,
     144,  -104,    -8,   145,  -104,  -104,  -104,   146,  -104,  -104,
    -104,   147,    83,    83,    83,    93,   148,    83,    83,    83,
      83,  -104,    83,  -104,    86,  -104,   101,  -104,   149,  -104,
      94,    83,   150,    83,    83,  -104,  -104,    83,    83,    83,
      97,  -104,   100,   -13,  -104,  -104,  -104,   151,   154,   102,
     102,   104,  -104,  -104,  -104,  -104,  -104,  -104,   155,  -104,
     156,   157,   164,  -104,  -104,   160,   161,   162,   163,   165,
     166,   114,  -104,   103,  -104,   169,   170,   171,  -104,   172,
     173,   174,   175,   176,   177,   179,   180,   181,   182,   132,
    -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,
     178,  -104,  -104,  -104,  -104,  -104,  -104,   183,   138,  -104,
     138,   142,  -104,  -104,  -104,  -104,  -104,  -104,   152,  -104,
    -104,  -104,  -104,   184,   153,  -104,  -104,   187,   185,   188,
     189,   158,   191,  -104,    83,  -104,   159,   192,  -104,   194,
     196,   167,  -104,   168,   197,   198,   186,   190,   199,   200,
     193,   195,  -104,   201,   202,   203,   204,   207,   205,   208,
     206,   209,   210,   213,   211,   214,   212,   215,   216,   217,
     218,   219,   220,   222,   221,   223,   224,   225,   226,   227,
     228,   229,   230,  -104
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -104,  -104,  -104,  -104,  -104,  -104,    40,  -104,  -104,    47,
    -104,  -104,  -104,  -103,  -104,    12,    15
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     150,   151,   177,    43,   155,   156,   157,   158,   141,   159,
      98,    99,    94,    67,    68,    44,    95,    96,   167,   100,
     169,   170,    83,    48,   171,   172,   173,    52,    53,    54,
     110,   111,    57,    58,    45,    60,    61,    62,    63,   116,
      66,     2,     3,    70,    71,    72,    73,    74,    46,    76,
      77,    78,    79,    80,    81,   117,   118,   119,   178,    49,
       4,     5,    50,   142,     6,     7,     8,     9,    10,    11,
      12,    13,    14,   112,   113,   114,   115,   133,   134,   120,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    51,    33,
      34,    35,    36,    37,    38,    39,    40,   181,    89,   152,
     153,    90,   182,    91,    92,    55,   183,   161,   162,   198,
     199,   229,    56,    85,    59,    64,   105,    65,    69,    75,
      82,    87,   163,    88,   165,   102,   103,   104,   106,   107,
     108,   109,   121,   122,   123,   124,   137,   125,   126,   127,
     128,   129,   130,   147,   131,   132,   160,   135,   136,   139,
     140,   166,   144,   145,   146,   154,   164,   168,   174,   179,
     176,   180,   187,   188,   189,   142,   190,   191,   192,   193,
     194,   186,   195,   196,   197,   200,   201,   185,   202,   203,
     204,   205,   206,   207,   214,   208,   209,   210,   211,   212,
     215,   224,   221,   213,   223,   225,     0,   226,   228,   216,
     231,   232,   218,   219,   233,   236,   237,   240,   241,   244,
       0,   246,     0,   220,   222,   248,   250,   252,     0,   227,
     230,   254,   256,   258,     0,   260,     0,   262,   234,   235,
     264,   266,     0,   268,     0,   270,     0,   272,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   238,     0,     0,
       0,   239,     0,     0,   242,     0,   243,     0,     0,     0,
       0,     0,     0,   245,     0,   247,   249,   251,     0,     0,
       0,   253,   255,   257,     0,     0,     0,   259,     0,   261,
       0,   263,   265,     0,     0,   267,     0,   269,     0,   271,
       0,   273
};

static const yytype_int16 yycheck[] =
{
     103,   104,    15,    17,   107,   108,   109,   110,    16,   112,
       4,     5,     7,    16,    17,    16,    11,    12,   121,    13,
     123,   124,    70,     8,   127,   128,   129,    12,    13,    14,
      16,    17,    17,    18,    16,    20,    21,    22,    23,    50,
      25,     0,     1,    28,    29,    30,    31,    32,    17,    34,
      35,    36,    37,    38,    39,    66,    67,    68,    71,    16,
      19,    20,    16,    71,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    16,    17,    16,    17,    16,    17,    64,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    16,    58,
      59,    60,    61,    62,    63,    64,    65,     5,     3,    16,
      17,     6,    10,     8,     9,    16,    14,    16,    17,    16,
      17,   224,    16,    70,    17,    16,    71,    17,    17,    17,
      17,    16,   117,    17,   119,    16,    16,    16,    71,    16,
      16,    16,    16,    70,    16,    16,    71,    17,    17,    16,
      16,    16,    16,    70,    17,    16,    70,    17,    17,    16,
      16,    67,    17,    17,    17,    17,    17,    17,    71,    18,
      70,    17,    17,    17,    17,    71,    12,    17,    17,    17,
      17,   141,    17,    17,    70,    16,    16,   140,    17,    17,
      17,    17,    17,    17,    16,    18,    17,    17,    17,    17,
      17,    16,    18,    71,    17,    17,    -1,    18,    17,    71,
      18,    17,   200,    71,    18,    18,    18,    18,    18,    18,
      -1,    18,    -1,    71,    71,    18,    18,    18,    -1,    71,
      71,    18,    18,    18,    -1,    18,    -1,    18,    71,    71,
      18,    18,    -1,    18,    -1,    18,    -1,    18,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      -1,    71,    -1,    -1,    71,    -1,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    71,    71,    71,    -1,    -1,
      -1,    71,    71,    71,    -1,    -1,    -1,    71,    -1,    71,
      -1,    71,    71,    -1,    -1,    71,    -1,    71,    -1,    71,
      -1,    71
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    73,     0,     1,    19,    20,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    58,    59,    60,    61,    62,    63,    64,
      65,    74,    75,    17,    16,    16,    17,    88,    88,    16,
      16,    16,    88,    88,    88,    16,    16,    88,    88,    17,
      88,    88,    88,    88,    16,    17,    88,    16,    17,    17,
      88,    88,    88,    88,    88,    17,    88,    88,    88,    88,
      88,    88,    17,    70,    76,    70,    77,    16,    17,     3,
       6,     8,     9,    82,     7,    11,    12,    83,     4,     5,
      13,    84,    16,    16,    16,    71,    71,    16,    16,    16,
      16,    17,    16,    17,    16,    17,    50,    66,    67,    68,
      88,    16,    70,    16,    16,    17,    17,    16,    16,    16,
      16,    17,    16,    16,    17,    17,    17,    71,    79,    16,
      16,    16,    71,    78,    17,    17,    17,    70,    85,    86,
      85,    85,    16,    17,    17,    85,    85,    85,    85,    85,
      70,    16,    17,    88,    17,    88,    67,    85,    17,    85,
      85,    85,    85,    85,    71,    80,    70,    15,    71,    18,
      17,     5,    10,    14,    81,    81,    78,    17,    17,    17,
      12,    17,    17,    17,    17,    17,    17,    70,    16,    17,
      16,    16,    17,    17,    17,    17,    17,    17,    18,    17,
      17,    17,    17,    71,    16,    17,    71,    87,    87,    71,
      71,    18,    71,    17,    16,    17,    18,    71,    17,    85,
      71,    18,    17,    18,    71,    71,    18,    18,    71,    71,
      18,    18,    71,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    18,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    18,    71,    18,    71,    18,    71,    18,    71,
      18,    71,    18,    71
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
	    /* Fall through.  */
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

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
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
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
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
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 157 "ftpcmd.y"
    {
			if (fromname != NULL)
				free (fromname);
			fromname = (char *) 0;
			restart_point = (off_t) 0;
		}
    break;

  case 5:
#line 168 "ftpcmd.y"
    {
			user((yyvsp[(3) - (4)].s));
			free((yyvsp[(3) - (4)].s));
		}
    break;

  case 6:
#line 173 "ftpcmd.y"
    {
			pass((yyvsp[(3) - (4)].s));
			free((yyvsp[(3) - (4)].s));
		}
    break;

  case 7:
#line 178 "ftpcmd.y"
    {
			if (epsvall) {
				reply(501, "no PORT allowed after EPSV ALL");
				goto port_done;
			}
			if (!(yyvsp[(2) - (5)].o))
				goto port_done;
			if (port_check("PORT") == 1)
				goto port_done;
#ifdef IPV6
			if ((his_addr.su_family != AF_INET6 ||
			     !IN6_IS_ADDR_V4MAPPED(&his_addr.su_sin6.sin6_addr))) {
				/* shoud never happen */
				usedefault = 1;
				reply(500, "Invalid address rejected.");
				goto port_done;
			}
			port_check_v6("pcmd");
#endif
		port_done:
			;
		}
    break;

  case 8:
#line 201 "ftpcmd.y"
    {
			if (epsvall) {
				reply(501, "no LPRT allowed after EPSV ALL");
				goto lprt_done;
			}
			if (!(yyvsp[(2) - (5)].o))
				goto lprt_done;
			if (port_check("LPRT") == 1)
				goto lprt_done;
#ifdef IPV6
			if (his_addr.su_family != AF_INET6) {
				usedefault = 1;
				reply(500, "Invalid address rejected.");
				goto lprt_done;
			}
			if (port_check_v6("LPRT") == 1)
				goto lprt_done;
#endif
		lprt_done:
			;
		}
    break;

  case 9:
#line 223 "ftpcmd.y"
    {
			char delim;
			char *tmp = NULL;
			char *p, *q;
			char *result[3];
			struct addrinfo hints;
			struct addrinfo *res;
			int i;

			if (epsvall) {
				reply(501, "no EPRT allowed after EPSV ALL");
				goto eprt_done;
			}
			if (!(yyvsp[(2) - (5)].o))
				goto eprt_done;

			memset(&data_dest, 0, sizeof(data_dest));
			tmp = strdup((yyvsp[(4) - (5)].s));
			if (debug)
				syslog(LOG_DEBUG, "%s", tmp);
			if (!tmp) {
				fatal("not enough core");
				/*NOTREACHED*/
			}
			p = tmp;
			delim = p[0];
			p++;
			memset(result, 0, sizeof(result));
			for (i = 0; i < 3; i++) {
				q = strchr(p, delim);
				if (!q || *q != delim) {
		parsefail:
					reply(500,
						"Invalid argument, rejected.");
					if (tmp)
						free(tmp);
					usedefault = 1;
					goto eprt_done;
				}
				*q++ = '\0';
				result[i] = p;
				if (debug)
					syslog(LOG_DEBUG, "%d: %s", i, p);
				p = q;
			}

			/* some more sanity check */
			p = result[0];
			while (*p) {
				if (!isdigit(*p))
					goto parsefail;
				p++;
			}
			p = result[2];
			while (*p) {
				if (!isdigit(*p))
					goto parsefail;
				p++;
			}

			/* grab address */
			memset(&hints, 0, sizeof(hints));
			if (atoi(result[0]) == 1)
				hints.ai_family = AF_INET;
#ifdef IPV6
			else if (atoi(result[0]) == 2)
				hints.ai_family = AF_INET6;
#endif
			else
				hints.ai_family = AF_UNSPEC;	/*XXX*/
			hints.ai_socktype = SOCK_STREAM;
			i = getaddrinfo(result[1], result[2], &hints, &res);
			if (i)
				goto parsefail;
			memcpy(&data_dest, res->ai_addr, res->ai_addrlen);
#ifdef IPV6
			if (his_addr.su_family == AF_INET6
			    && data_dest.su_family == AF_INET6) {
				/* XXX more sanity checks! */
				data_dest.su_sin6.sin6_scope_id =
					his_addr.su_sin6.sin6_scope_id;
			}
#endif
			free(tmp);
			tmp = NULL;

			if (port_check("EPRT") == 1)
				goto eprt_done;
#ifdef IPV6
			if (his_addr.su_family != AF_INET6) {
				usedefault = 1;
				reply(500, "Invalid address rejected.");
				goto eprt_done;
			}
			if (port_check_v6("EPRT") == 1)
				goto eprt_done;
#endif
		eprt_done:;
		}
    break;

  case 10:
#line 323 "ftpcmd.y"
    {
			if (epsvall)
				reply(501, "no PASV allowed after EPSV ALL");
			else if ((yyvsp[(2) - (3)].o))
				passive();
		}
    break;

  case 11:
#line 330 "ftpcmd.y"
    {
			if (epsvall)
				reply(501, "no LPSV allowed after EPSV ALL");
			else if ((yyvsp[(2) - (3)].o))
				long_passive("LPSV", AF_UNSPEC);
		}
    break;

  case 12:
#line 337 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o)) {
				int af;
				switch ((yyvsp[(4) - (5)].o)) {
				case 1:
					af = AF_INET;
					break;
#ifdef IPV6
				case 2:
					af = AF_INET6;
					break;
#endif
				default:
					af = -1;	/*junk value*/
					break;
				}
				long_passive("EPSV", af);
			}
		}
    break;

  case 13:
#line 357 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o)) {
				reply(200,
				      "EPSV ALL command successful.");
				epsvall++;
			}
		}
    break;

  case 14:
#line 365 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (3)].o))
				long_passive("EPSV", AF_UNSPEC);
		}
    break;

  case 15:
#line 370 "ftpcmd.y"
    {
			feat();
		}
    break;

  case 16:
#line 374 "ftpcmd.y"
    {
			switch (cmd_type) {

			case TYPE_A:
				if (cmd_form == FORM_N) {
					reply(200, "Type set to A.");
					type = cmd_type;
					form = cmd_form;
				} else
					reply(504, "Form must be N.");
				break;

			case TYPE_E:
				reply(504, "Type E not implemented.");
				break;

			case TYPE_I:
				reply(200, "Type set to I.");
				type = cmd_type;
				break;

			case TYPE_L:
#if defined (NBBY) && NBBY == 8
				if (cmd_bytesz == 8) {
					reply(200,
					    "Type set to L (byte size 8).");
					type = cmd_type;
				} else
					reply(504, "Byte size must be 8.");
#else /* NBBY == 8 */
				UNIMPLEMENTED for NBBY != 8
#endif /* NBBY == 8 */
			}
		}
    break;

  case 17:
#line 409 "ftpcmd.y"
    {
			switch ((yyvsp[(3) - (4)].o)) {

			case STRU_F:
				reply(200, "STRU F ok.");
				break;

			default:
				reply(504, "Unimplemented STRU type.");
			}
		}
    break;

  case 18:
#line 421 "ftpcmd.y"
    {
			switch ((yyvsp[(3) - (4)].o)) {

			case MODE_S:
				reply(200, "MODE S ok.");
				break;

			default:
				reply(502, "Unimplemented MODE type.");
			}
		}
    break;

  case 19:
#line 433 "ftpcmd.y"
    {
			reply(202, "ALLO command ignored.");
		}
    break;

  case 20:
#line 437 "ftpcmd.y"
    {
			reply(202, "ALLO command ignored.");
		}
    break;

  case 21:
#line 441 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				retrieve((char *) 0, (yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 22:
#line 448 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				store((yyvsp[(4) - (5)].s), "w", 0);
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 23:
#line 455 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				store((yyvsp[(4) - (5)].s), "a", 0);
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 24:
#line 462 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (3)].o))
				send_file_list(".");
		}
    break;

  case 25:
#line 467 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				send_file_list((yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 26:
#line 474 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (3)].o))
				retrieve("/bin/ls -lgA", "");
		}
    break;

  case 27:
#line 479 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				retrieve("/bin/ls -lgA %s", (yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 28:
#line 486 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				statfilecmd((yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 29:
#line 493 "ftpcmd.y"
    {
			statcmd();
		}
    break;

  case 30:
#line 497 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				delete((yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 31:
#line 504 "ftpcmd.y"
    {
		    if ((yyvsp[(2) - (5)].o)) {
			if (fromname) {
				renamecmd(fromname, (yyvsp[(4) - (5)].s));
				free(fromname);
				fromname = (char *) 0;
			} else {
				reply(503, "Bad sequence of commands.");
			}
		    }
		    free ((yyvsp[(4) - (5)].s));
		}
    break;

  case 32:
#line 517 "ftpcmd.y"
    {
			reply(225, "ABOR command successful.");
		}
    break;

  case 33:
#line 521 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (3)].o))
				cwd(cred.homedir);
		}
    break;

  case 34:
#line 526 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				cwd((yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 35:
#line 533 "ftpcmd.y"
    {
			help(cmdtab, (char *) 0);
		}
    break;

  case 36:
#line 537 "ftpcmd.y"
    {
			char *cp = (yyvsp[(3) - (4)].s);

			if (strncasecmp(cp, "SITE", 4) == 0) {
				cp = (yyvsp[(3) - (4)].s) + 4;
				if (*cp == ' ')
					cp++;
				if (*cp)
					help(sitetab, cp);
				else
					help(sitetab, (char *) 0);
			} else
				help(cmdtab, (yyvsp[(3) - (4)].s));
			if ((yyvsp[(3) - (4)].s) != NULL)
			    free ((yyvsp[(3) - (4)].s));
		}
    break;

  case 37:
#line 554 "ftpcmd.y"
    {
			reply(200, "NOOP command successful.");
		}
    break;

  case 38:
#line 558 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				makedir((yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 39:
#line 565 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				removedir((yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 40:
#line 572 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (3)].o))
				pwd();
		}
    break;

  case 41:
#line 577 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (3)].o))
				cwd("..");
		}
    break;

  case 42:
#line 582 "ftpcmd.y"
    {
			help(sitetab, (char *) 0);
		}
    break;

  case 43:
#line 586 "ftpcmd.y"
    {
			help(sitetab, (yyvsp[(5) - (6)].s));
			if ((yyvsp[(5) - (6)].s) != NULL)
			    free ((yyvsp[(5) - (6)].s));
		}
    break;

  case 44:
#line 592 "ftpcmd.y"
    {
			int oldmask;

			if ((yyvsp[(4) - (5)].o)) {
				oldmask = umask(0);
				(void) umask(oldmask);
				reply(200, "Current UMASK is %03o", oldmask);
			}
		}
    break;

  case 45:
#line 602 "ftpcmd.y"
    {
			int oldmask;

			if ((yyvsp[(4) - (7)].o)) {
				if (((yyvsp[(6) - (7)].o) == -1) || ((yyvsp[(6) - (7)].o) > 0777)) {
					reply(501, "Bad UMASK value");
				} else {
					oldmask = umask((yyvsp[(6) - (7)].o));
					reply(200,
					    "UMASK set to %03o (was %03o)",
					    (yyvsp[(6) - (7)].o), oldmask);
				}
			}
		}
    break;

  case 46:
#line 617 "ftpcmd.y"
    {
			if ((yyvsp[(4) - (9)].o) && ((yyvsp[(8) - (9)].s) != NULL)) {
				if ((yyvsp[(6) - (9)].o) > 0777)
					reply(501,
				"CHMOD: Mode value must be between 0 and 0777");
				else if (chmod((yyvsp[(8) - (9)].s), (yyvsp[(6) - (9)].o)) < 0)
					perror_reply(550, (yyvsp[(8) - (9)].s));
				else
					reply(200, "CHMOD command successful.");
			}
			if ((yyvsp[(8) - (9)].s) != NULL)
				free((yyvsp[(8) - (9)].s));
		}
    break;

  case 47:
#line 631 "ftpcmd.y"
    {
			reply(200,
			    "Current IDLE time limit is %d seconds; max %d",
				timeout, maxtimeout);
		}
    break;

  case 48:
#line 637 "ftpcmd.y"
    {
		    	if ((yyvsp[(3) - (7)].o)) {
			    if ((yyvsp[(6) - (7)].o) < 30 || (yyvsp[(6) - (7)].o) > maxtimeout) {
				reply (501,
			"Maximum IDLE time must be between 30 and %d seconds",
					maxtimeout);
			    } else {
				timeout = (yyvsp[(6) - (7)].o);
				(void) alarm((unsigned) timeout);
				reply(200,
					"Maximum IDLE time set to %d seconds",
					timeout);
			    }
			}
		}
    break;

  case 49:
#line 653 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				store((yyvsp[(4) - (5)].s), "w", 1);
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 50:
#line 660 "ftpcmd.y"
    {
		        const char *sys_type; /* Official rfc-defined os type.  */
			char *version = 0; /* A more specific type. */

#ifdef HAVE_UNAME
			struct utsname u;
			if (uname (&u) == 0) {
				version =
				  malloc (strlen (u.sysname)
					  + 1 + strlen (u.release) + 1);
				if (version)
					sprintf (version, "%s %s",
						 u.sysname, u.release);
		        }
#else
#ifdef BSD
			version = "BSD";
#endif
#endif

#ifdef unix
			sys_type = "UNIX";
#else
			sys_type = "UNKNOWN";
#endif

			if (version)
				reply(215, "%s Type: L%d Version: %s",
				      sys_type, NBBY, version);
			else
				reply(215, "%s Type: L%d", sys_type, NBBY);

#ifdef HAVE_UNAME
			if (version)
				free (version);
#endif
		}
    break;

  case 51:
#line 706 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL)
				sizecmd((yyvsp[(4) - (5)].s));
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 52:
#line 723 "ftpcmd.y"
    {
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s) != NULL) {
				struct stat stbuf;
				if (stat((yyvsp[(4) - (5)].s), &stbuf) < 0)
					reply(550, "%s: %s",
					    (yyvsp[(4) - (5)].s), strerror(errno));
				else if (!S_ISREG(stbuf.st_mode)) {
					reply(550, "%s: not a plain file.", (yyvsp[(4) - (5)].s));
				} else {
					struct tm *t;
					t = gmtime(&stbuf.st_mtime);
					reply(213,
					    "%04d%02d%02d%02d%02d%02d",
					    1900 + t->tm_year, t->tm_mon+1,
					    t->tm_mday, t->tm_hour, t->tm_min,
					    t->tm_sec);
				}
			}
			if ((yyvsp[(4) - (5)].s) != NULL)
				free((yyvsp[(4) - (5)].s));
		}
    break;

  case 53:
#line 745 "ftpcmd.y"
    {
			reply(221, "Goodbye.");
			dologout(0);
		}
    break;

  case 54:
#line 750 "ftpcmd.y"
    {
			yyerrok;
		}
    break;

  case 55:
#line 756 "ftpcmd.y"
    {
			restart_point = (off_t) 0;
			if ((yyvsp[(2) - (5)].o) && (yyvsp[(4) - (5)].s)) {
			    if (fromname != NULL)
				free (fromname);
			    fromname = renamefrom((yyvsp[(4) - (5)].s));
			}
			if (fromname == (char *) 0 && (yyvsp[(4) - (5)].s))
			    free((yyvsp[(4) - (5)].s));
		}
    break;

  case 56:
#line 767 "ftpcmd.y"
    {
		    	if (fromname != NULL)
				free (fromname);
			fromname = (char *) 0;
			restart_point = (yyvsp[(3) - (4)].o);
			reply(350,
			      (sizeof(restart_point) > sizeof(long)
			       ? "Restarting at %qd. %s"
			       : "Restarting at %ld. %s"), restart_point,
			    "Send STORE or RETRIEVE to initiate transfer.");
		}
    break;

  case 58:
#line 786 "ftpcmd.y"
    {
			(yyval.s) = (char *)calloc(1, sizeof(char));
		}
    break;

  case 60:
#line 794 "ftpcmd.y"
    {
			(yyval.o) = (yyvsp[(1) - (1)].o);
		}
    break;

  case 61:
#line 802 "ftpcmd.y"
    {
			char *a, *p;

			data_dest.su_family = AF_INET;
			p = (char *)&data_dest.su_sin.sin_port;
			p[0] = (yyvsp[(9) - (11)].o); p[1] = (yyvsp[(11) - (11)].o);
			a = (char *)&data_dest.su_sin.sin_addr;
			a[0] = (yyvsp[(1) - (11)].o); a[1] = (yyvsp[(3) - (11)].o); a[2] = (yyvsp[(5) - (11)].o); a[3] = (yyvsp[(7) - (11)].o);
		}
    break;

  case 62:
#line 820 "ftpcmd.y"
    {
			char *a, *p;

			memset(&data_dest, 0, sizeof(data_dest));
			data_dest.su_family = AF_INET6;
			p = (char *)&data_dest.su_port;
			p[0] = (yyvsp[(39) - (41)].o); p[1] = (yyvsp[(41) - (41)].o);
			a = (char *)&data_dest.su_sin6.sin6_addr;
			 a[0] =  (yyvsp[(5) - (41)].o);  a[1] =  (yyvsp[(7) - (41)].o);  a[2] =  (yyvsp[(9) - (41)].o);  a[3] = (yyvsp[(11) - (41)].o);
			 a[4] = (yyvsp[(13) - (41)].o);  a[5] = (yyvsp[(15) - (41)].o);  a[6] = (yyvsp[(17) - (41)].o);  a[7] = (yyvsp[(19) - (41)].o);
			 a[8] = (yyvsp[(21) - (41)].o);  a[9] = (yyvsp[(23) - (41)].o); a[10] = (yyvsp[(25) - (41)].o); a[11] = (yyvsp[(27) - (41)].o);
			a[12] = (yyvsp[(29) - (41)].o); a[13] = (yyvsp[(31) - (41)].o); a[14] = (yyvsp[(33) - (41)].o); a[15] = (yyvsp[(35) - (41)].o);
			if (his_addr.su_family == AF_INET6) {
				/* XXX more sanity checks! */
				data_dest.su_sin6.sin6_scope_id =
					his_addr.su_sin6.sin6_scope_id;
			}
			if ((yyvsp[(1) - (41)].o) != 6 || (yyvsp[(3) - (41)].o) != 16 || (yyvsp[(37) - (41)].o) != 2)
				memset(&data_dest, 0, sizeof(data_dest));
		}
    break;

  case 63:
#line 843 "ftpcmd.y"
    {
			char *a, *p;

			memset(&data_dest, 0, sizeof(data_dest));
			data_dest.su_family = AF_INET;
			p = (char *)&data_dest.su_port;
			p[0] = (yyvsp[(15) - (17)].o); p[1] = (yyvsp[(17) - (17)].o);
			a = (char *)&data_dest.su_sin.sin_addr;
			a[0] =  (yyvsp[(5) - (17)].o);  a[1] =  (yyvsp[(7) - (17)].o);  a[2] =  (yyvsp[(9) - (17)].o);  a[3] = (yyvsp[(11) - (17)].o);
			if ((yyvsp[(1) - (17)].o) != 4 || (yyvsp[(3) - (17)].o) != 4 || (yyvsp[(13) - (17)].o) != 2)
				memset(&data_dest, 0, sizeof(data_dest));
		}
    break;

  case 64:
#line 859 "ftpcmd.y"
    {
			(yyval.o) = FORM_N;
		}
    break;

  case 65:
#line 863 "ftpcmd.y"
    {
			(yyval.o) = FORM_T;
		}
    break;

  case 66:
#line 867 "ftpcmd.y"
    {
			(yyval.o) = FORM_C;
		}
    break;

  case 67:
#line 874 "ftpcmd.y"
    {
			cmd_type = TYPE_A;
			cmd_form = FORM_N;
		}
    break;

  case 68:
#line 879 "ftpcmd.y"
    {
			cmd_type = TYPE_A;
			cmd_form = (yyvsp[(3) - (3)].o);
		}
    break;

  case 69:
#line 884 "ftpcmd.y"
    {
			cmd_type = TYPE_E;
			cmd_form = FORM_N;
		}
    break;

  case 70:
#line 889 "ftpcmd.y"
    {
			cmd_type = TYPE_E;
			cmd_form = (yyvsp[(3) - (3)].o);
		}
    break;

  case 71:
#line 894 "ftpcmd.y"
    {
			cmd_type = TYPE_I;
		}
    break;

  case 72:
#line 898 "ftpcmd.y"
    {
			cmd_type = TYPE_L;
			cmd_bytesz = NBBY;
		}
    break;

  case 73:
#line 903 "ftpcmd.y"
    {
			cmd_type = TYPE_L;
			cmd_bytesz = (yyvsp[(3) - (3)].o);
		}
    break;

  case 74:
#line 909 "ftpcmd.y"
    {
			cmd_type = TYPE_L;
			cmd_bytesz = (yyvsp[(2) - (2)].o);
		}
    break;

  case 75:
#line 917 "ftpcmd.y"
    {
			(yyval.o) = STRU_F;
		}
    break;

  case 76:
#line 921 "ftpcmd.y"
    {
			(yyval.o) = STRU_R;
		}
    break;

  case 77:
#line 925 "ftpcmd.y"
    {
			(yyval.o) = STRU_P;
		}
    break;

  case 78:
#line 932 "ftpcmd.y"
    {
			(yyval.o) = MODE_S;
		}
    break;

  case 79:
#line 936 "ftpcmd.y"
    {
			(yyval.o) = MODE_B;
		}
    break;

  case 80:
#line 940 "ftpcmd.y"
    {
			(yyval.o) = MODE_C;
		}
    break;

  case 81:
#line 947 "ftpcmd.y"
    {
			/*
			 * Problem: this production is used for all pathname
			 * processing, but only gives a 550 error reply.
			 * This is a valid reply in some cases but not in others.
			 */
			if (cred.logged_in && (yyvsp[(1) - (1)].s) && *(yyvsp[(1) - (1)].s) == '~') {
				glob_t gl;
				int flags = GLOB_NOCHECK;

#ifdef GLOB_BRACE
				flags |= GLOB_BRACE;
#endif
#ifdef GLOB_QUOTE
				flags |= GLOB_QUOTE;
#endif
#ifdef GLOB_TILDE
				flags |= GLOB_TILDE;
#endif

				memset(&gl, 0, sizeof(gl));
				if (glob((yyvsp[(1) - (1)].s), flags, NULL, &gl) ||
				    gl.gl_pathc == 0) {
					reply(550, "not found");
					(yyval.s) = NULL;
				} else {
					(yyval.s) = strdup(gl.gl_pathv[0]);
				}
				globfree(&gl);
				free((yyvsp[(1) - (1)].s));
			} else
				(yyval.s) = (yyvsp[(1) - (1)].s);
		}
    break;

  case 83:
#line 988 "ftpcmd.y"
    {
			int ret, dec, multby, digit;

			/*
			 * Convert a number that was read as decimal number
			 * to what it would be if it had been read as octal.
			 */
			dec = (yyvsp[(1) - (1)].o);
			multby = 1;
			ret = 0;
			while (dec) {
				digit = dec%10;
				if (digit > 7) {
					ret = -1;
					break;
				}
				ret += digit * multby;
				multby *= 8;
				dec /= 10;
			}
			(yyval.o) = ret;
		}
    break;

  case 84:
#line 1015 "ftpcmd.y"
    {
			if (cred.logged_in)
				(yyval.o) = 1;
			else {
				reply(530, "Please login with USER and PASS.");
				(yyval.o) = 0;
			}
		}
    break;


/* Line 1267 of yacc.c.  */
#line 2777 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
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

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 1025 "ftpcmd.y"


#define	CMD	0	/* beginning of command */
#define	ARGS	1	/* expect miscellaneous arguments */
#define	STR1	2	/* expect SP followed by STRING */
#define	STR2	3	/* expect STRING */
#define	OSTR	4	/* optional SP then STRING */
#define	ZSTR1	5	/* SP then optional STRING */
#define	ZSTR2	6	/* optional STRING after SP */
#define	SITECMD	7	/* SITE command */
#define	NSTR	8	/* Number followed by a string */

struct tab cmdtab[] = {		/* In order defined in RFC 765 */
	{ "USER", USER, STR1, 1,	"<sp> username" },
	{ "PASS", PASS, ZSTR1, 1,	"<sp> password" },
#ifdef FULL_FTPD
	{ "ACCT", ACCT, STR1, 0,	"(specify account)" },
	{ "SMNT", SMNT, ARGS, 0,	"(structure mount)" },
	{ "REIN", REIN, ARGS, 0,	"(reinitialize server state)" },
#endif
	{ "QUIT", QUIT, ARGS, 1,	"(terminate service)", },
	{ "PORT", PORT, ARGS, 1,	"<sp> b0, b1, b2, b3, b4" },
	{ "LPRT", LPRT, ARGS, 1,	"<sp> af, hal, h1, h2, h3,..., pal, p1, p2..." },
	{ "EPRT", EPRT, STR1, 1,	"<sp> |af|addr|port|" },
	{ "PASV", PASV, ARGS, 1,	"(set server in passive mode)" },
	{ "LPSV", LPSV, ARGS, 1,	"(set server in passive mode)" },
	{ "EPSV", EPSV, ARGS, 1,	"[<sp> af|ALL]" },
	{ "FEAT", FEAT, ARGS, 1,	"(display extended features)" },
	{ "TYPE", TYPE, ARGS, 1,	"<sp> [ A | E | I | L ]" },
#ifdef FULL_FTPD
	{ "STRU", STRU, ARGS, 1,	"(specify file structure)" },
	{ "MODE", MODE, ARGS, 1,	"(specify transfer mode)" },
#endif
	{ "RETR", RETR, STR1, 1,	"<sp> file-name" },
	{ "STOR", STOR, STR1, 1,	"<sp> file-name" },
#ifdef FULL_FTPD
	{ "APPE", APPE, STR1, 1,	"<sp> file-name" },
	{ "MLFL", MLFL, OSTR, 0,	"(mail file)" },
	{ "MAIL", MAIL, OSTR, 0,	"(mail to user)" },
	{ "MSND", MSND, OSTR, 0,	"(mail send to terminal)" },
	{ "MSOM", MSOM, OSTR, 0,	"(mail send to terminal or mailbox)" },
	{ "MSAM", MSAM, OSTR, 0,	"(mail send to terminal and mailbox)" },
	{ "MRSQ", MRSQ, OSTR, 0,	"(mail recipient scheme question)" },
	{ "MRCP", MRCP, STR1, 0,	"(mail recipient)" },
	{ "ALLO", ALLO, ARGS, 1,	"allocate storage (vacuously)" },
#endif
	{ "REST", REST, ARGS, 1,	"<sp> offset (restart command)" },
#ifdef FULL_FTPD
	{ "RNFR", RNFR, STR1, 1,	"<sp> file-name" },
	{ "RNTO", RNTO, STR1, 1,	"<sp> file-name" },
#endif
	{ "ABOR", ABOR, ARGS, 1,	"(abort operation)" },
	{ "DELE", DELE, STR1, 1,	"<sp> file-name" },
#ifdef FULL_FTPD
	{ "CWD",  CWD,  OSTR, 1,	"[ <sp> directory-name ]" },
	{ "XCWD", CWD,	OSTR, 1,	"[ <sp> directory-name ]" },
#endif
	{ "LIST", LIST, OSTR, 1,	"[ <sp> path-name ]" },
#ifdef FULL_FTPD
	{ "NLST", NLST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "SITE", SITE, SITECMD, 1,	"site-cmd [ <sp> arguments ]" },
	{ "SYST", SYST, ARGS, 1,	"(get type of operating system)" },
	{ "STAT", STAT, OSTR, 1,	"[ <sp> path-name ]" },
#endif
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ "NOOP", NOOP, ARGS, 1,	"" },
#ifdef FULL_FTPD
	{ "MKD",  MKD,  STR1, 1,	"<sp> path-name" },
	{ "XMKD", MKD,  STR1, 1,	"<sp> path-name" },
	{ "RMD",  RMD,  STR1, 1,	"<sp> path-name" },
	{ "XRMD", RMD,  STR1, 1,	"<sp> path-name" },
#endif
	{ "PWD",  PWD,  ARGS, 1,	"(return current directory)" },
	{ "XPWD", PWD,  ARGS, 1,	"(return current directory)" },
#ifdef FULL_FTPD
	{ "CDUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "XCUP", CDUP, ARGS, 1,	"(change to parent directory)" },
	{ "STOU", STOU, STR1, 1,	"<sp> file-name" },
	{ "SIZE", SIZE, OSTR, 1,	"<sp> path-name" },
	{ "MDTM", MDTM, OSTR, 1,	"<sp> path-name" },
#endif
	{ NULL,   0,    0,    0,	0 }
};

struct tab sitetab[] = {
	{ "UMASK", UMASK, ARGS, 1,	"[ <sp> umask ]" },
	{ "IDLE", IDLE, ARGS, 1,	"[ <sp> maximum-idle-time ]" },
	{ "CHMOD", CHMOD, NSTR, 1,	"<sp> mode <sp> file-name" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ NULL,   0,    0,    0,	0 }
};

static struct tab *
lookup(struct tab *p, const char *cmd)
{

	for (; p->name != NULL; p++)
		if (strcmp(cmd, p->name) == 0)
			return (p);
	return (0);
}

#include <arpa/telnet.h>

/*
 * getline - a hacked up version of fgets to ignore TELNET escape codes.
 */
char *
telnet_fgets(char *s, int n, FILE *iop)
{
	int c;
	register char *cs;

	cs = s;
/* tmpline may contain saved command from urgent mode interruption */
	for (c = 0; tmpline[c] != '\0' && --n > 0; ++c) {
		*cs++ = tmpline[c];
		if (tmpline[c] == '\n') {
			*cs++ = '\0';
			if (debug)
				syslog(LOG_DEBUG, "command: %s", s);
			tmpline[0] = '\0';
			return(s);
		}
		if (c == 0)
			tmpline[0] = '\0';
	}
	while ((c = getc(iop)) != EOF) {
		c &= 0377;
		if (c == IAC) {
		    if ((c = getc(iop)) != EOF) {
			c &= 0377;
			switch (c) {
			case WILL:
			case WONT:
				c = getc(iop);
				printf("%c%c%c", IAC, DONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case DO:
			case DONT:
				c = getc(iop);
				printf("%c%c%c", IAC, WONT, 0377&c);
				(void) fflush(stdout);
				continue;
			case IAC:
				break;
			default:
				continue;	/* ignore command */
			}
		    }
		}
		*cs++ = c;
		if (--n <= 0 || c == '\n')
			break;
	}
	if (c == EOF && cs == s)
	    return (NULL);
	*cs++ = '\0';
	if (debug) {
		if (!cred.guest && strncasecmp("pass ", s, 5) == 0) {
			/* Don't syslog passwords */
			syslog(LOG_DEBUG, "command: %.5s ???", s);
		} else {
			register char *cp;
			register int len;

			/* Don't syslog trailing CR-LF */
			len = strlen(s);
			cp = s + len - 1;
			while (cp >= s && (*cp == '\n' || *cp == '\r')) {
				--cp;
				--len;
			}
			syslog(LOG_DEBUG, "command: %.*s", len, s);
		}
	}
	return (s);
}

void
toolong(int signo)
{
  (void)signo;
	reply(421,
	    "Timeout (%d seconds): closing control connection.", timeout);
	if (logging)
		syslog(LOG_INFO, "User %s timed out after %d seconds",
		    (cred.name ? cred.name : "unknown"), timeout);
	dologout(1);
}

static int
yylex()
{
	static int cpos, state;
	char *cp, *cp2;
	struct tab *p;
	int n;
	char c;

	for (;;) {
		switch (state) {

		case CMD:
			(void) signal(SIGALRM, toolong);
			(void) alarm((unsigned) timeout);
			if (telnet_fgets(cbuf, sizeof(cbuf)-1, stdin) == NULL) {
				reply(221, "You could at least say goodbye.");
				dologout(0);
			}
			(void) alarm(0);
#ifdef HAVE_SETPROCTITLE
			if (strncasecmp(cbuf, "PASS", 4) != NULL)
				setproctitle("%s: %s", proctitle, cbuf);
#endif /* HAVE_SETPROCTITLE */
			if ((cp = strchr(cbuf, '\r'))) {
				*cp++ = '\n';
				*cp = '\0';
			}
			if ((cp = strpbrk(cbuf, " \n")))
				cpos = cp - cbuf;
			if (cpos == 0)
				cpos = 4;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cbuf);
			p = lookup(cmdtab, cbuf);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				yylval.s = (char *)p->name;
				return (p->token);
			}
			break;

		case SITECMD:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			cp = &cbuf[cpos];
			if ((cp2 = strpbrk(cp, " \n")))
				cpos = cp2 - cbuf;
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cp);
			p = lookup(sitetab, cp);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					state = CMD;
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				yylval.s = (char *)p->name;
				return (p->token);
			}
			state = CMD;
			break;

		case OSTR:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR1:
		case ZSTR1:
		dostr1:
			if (cbuf[cpos] == ' ') {
				cpos++;
				state = state == OSTR ? STR2 : state + 1;
				return (SP);
			}
			break;

		case ZSTR2:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALLTHROUGH */

		case STR2:
			cp = &cbuf[cpos];
			n = strlen(cp);
			cpos += n - 1;
			/*
			 * Make sure the string is nonempty and \n terminated.
			 */
			if (n > 1 && cbuf[cpos] == '\n') {
				cbuf[cpos] = '\0';
				yylval.s = copy(cp);
				cbuf[cpos] = '\n';
				state = ARGS;
				return (STRING);
			}
			break;

		case NSTR:
			if (cbuf[cpos] == ' ') {
				cpos++;
				return (SP);
			}
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval.o = atoi(cp);
				cbuf[cpos] = c;
				state = STR1;
				return (NUMBER);
			}
			state = STR1;
			goto dostr1;

		case ARGS:
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval.o = strtoull(cp, NULL, 10);
				cbuf[cpos] = c;
				return (NUMBER);
			}
			switch (cbuf[cpos++]) {

			case '\n':
				state = CMD;
				return (CRLF);

			case ' ':
				return (SP);

			case ',':
				return (COMMA);

			case 'A':
			case 'a':
				return (A);

			case 'B':
			case 'b':
				return (B);

			case 'C':
			case 'c':
				return (C);

			case 'E':
			case 'e':
				return (E);

			case 'F':
			case 'f':
				return (F);

			case 'I':
			case 'i':
				return (I);

			case 'L':
			case 'l':
				return (L);

			case 'N':
			case 'n':
				return (N);

			case 'P':
			case 'p':
				return (P);

			case 'R':
			case 'r':
				return (R);

			case 'S':
			case 's':
				return (S);

			case 'T':
			case 't':
				return (T);

			}
			break;

		default:
			fatal("Unknown state in scanner.");
		}
		yyerror((char *) 0);
		state = CMD;
		longjmp(errcatch,0);
	}
}

void
upper(char *s)
{
	while (*s != '\0') {
		if (islower(*s))
			*s = toupper(*s);
		s++;
	}
}

static char *
copy(char *s)
{
	char *p;

	p = malloc((unsigned) strlen(s) + 1);
	if (p == NULL)
		fatal("Ran out of memory.");
	(void) strcpy(p, s);
	return (p);
}

static void
help(struct tab *ctab, char *s)
{
	struct tab *c;
	int width, NCMDS;
	const char *help_type;

	if (ctab == sitetab)
		help_type = "SITE ";
	else
		help_type = "";
	width = 0, NCMDS = 0;
	for (c = ctab; c->name != NULL; c++) {
		int len = strlen(c->name);

		if (len > width)
			width = len;
		NCMDS++;
	}
	width = (width + 8) &~ 7;
	if (s == 0) {
		int i, j, w;
		int columns, lines;

		lreply(214, "The following %scommands are recognized %s.",
		    help_type, "(* =>'s unimplemented)");
		columns = 76 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			printf("   ");
			for (j = 0; j < columns; j++) {
				c = ctab + j * lines + i;
				printf("%s%c", c->name,
					c->implemented ? ' ' : '*');
				if (c + lines >= &ctab[NCMDS])
					break;
				w = strlen(c->name) + 1;
				while (w < width) {
					putchar(' ');
					w++;
				}
			}
			printf("\r\n");
		}
		(void) fflush(stdout);
		reply(214, "Direct comments to ftp-bugs@%s.", hostname);
		return;
	}
	upper(s);
	c = lookup(ctab, s);
	if (c == (struct tab *)0) {
		reply(502, "Unknown command %s.", s);
		return;
	}
	if (c->implemented)
		reply(214, "Syntax: %s%s %s", help_type, c->name, c->help);
	else
		reply(214, "%s%-*s\t%s; unimplemented.", help_type, width,
		    c->name, c->help);
}

static void
sizecmd(char *filename)
{
	switch (type) {
	case TYPE_L:
	case TYPE_I: {
		struct stat stbuf;
		if (stat(filename, &stbuf) < 0 || !S_ISREG(stbuf.st_mode))
			reply(550, "%s: not a plain file.", filename);
		else
			reply(213,
			      (sizeof (stbuf.st_size) > sizeof(long)
			       ? "%qu" : "%lu"), stbuf.st_size);
		break; }
	case TYPE_A: {
		FILE *fin;
		int c;
		off_t count;
		struct stat stbuf;
		fin = fopen(filename, "r");
		if (fin == NULL) {
			perror_reply(550, filename);
			return;
		}
		if (fstat(fileno(fin), &stbuf) < 0 || !S_ISREG(stbuf.st_mode)) {
			reply(550, "%s: not a plain file.", filename);
			(void) fclose(fin);
			return;
		}

		count = 0;
		while((c=getc(fin)) != EOF) {
			if (c == '\n')	/* will get expanded to \r\n */
				count++;
			count++;
		}
		(void) fclose(fin);

		reply(213, sizeof(count) > sizeof(long) ? "%qd" : "%ld",
		      count);
		break; }
	default:
		reply(504, "SIZE not implemented for Type %c.", "?AEIL"[type]);
	}
}

/* Return 1, if port check is done. Return 0, if not yet. */
static int
port_check(pcmd)
	const char *pcmd;
{
	if (his_addr.su_family == AF_INET) {
		if (data_dest.su_family != AF_INET) {
			usedefault = 1;
			reply(500, "Invalid address rejected.");
			return 1;
		}
		if (paranoid &&
		    ((ntohs(data_dest.su_port) < IPPORT_RESERVED) ||
		     memcmp(&data_dest.su_sin.sin_addr,
			    &his_addr.su_sin.sin_addr,
			    sizeof(data_dest.su_sin.sin_addr)))) {
			usedefault = 1;
			reply(500, "Illegal PORT range rejected.");
		} else {
			usedefault = 0;
			if (pdata >= 0) {
				(void) close(pdata);
				pdata = -1;
			}
			reply(200, "%s command successful.", pcmd);
		}
		return 1;
	}
	return 0;
}

#ifdef IPV6
/* Return 1, if port check is done. Return 0, if not yet. */
static int
port_check_v6(pcmd)
	const char *pcmd;
{
	if (his_addr.su_family == AF_INET6) {
		if (IN6_IS_ADDR_V4MAPPED(&his_addr.su_sin6.sin6_addr))
			/* Convert data_dest into v4 mapped sockaddr.*/
			v4map_data_dest();
		if (data_dest.su_family != AF_INET6) {
			usedefault = 1;
			reply(500, "Invalid address rejected.");
			return 1;
		}
		if (paranoid &&
		    ((ntohs(data_dest.su_port) < IPPORT_RESERVED) ||
		     memcmp(&data_dest.su_sin6.sin6_addr,
			    &his_addr.su_sin6.sin6_addr,
			    sizeof(data_dest.su_sin6.sin6_addr)))) {
			usedefault = 1;
			reply(500, "Illegal PORT range rejected.");
		} else {
			usedefault = 0;
			if (pdata >= 0) {
				(void) close(pdata);
				pdata = -1;
			}
			reply(200, "%s command successful.", pcmd);
		}
		return 1;
	}
	return 0;
}

static void
v4map_data_dest()
{
	struct in_addr savedaddr;
	in_port_t savedport;

	if (data_dest.su_family != AF_INET) {
		usedefault = 1;
		reply(500, "Invalid address rejected.");
		return;
	}

	savedaddr = data_dest.su_sin.sin_addr;
	savedport = data_dest.su_port;

	memset(&data_dest, 0, sizeof(data_dest));
	data_dest.su_sin6.sin6_family = AF_INET6;
	data_dest.su_sin6.sin6_port = savedport;
	memset(&data_dest.su_sin6.sin6_addr.s6_addr[10], 0xff, 2);
	memcpy(&data_dest.su_sin6.sin6_addr.s6_addr[12],
	       &savedaddr, sizeof(savedaddr));
}
#endif

static void
yyerror(const char *s)
{
  char *cp;

  (void)s;
  cp = strchr(cbuf,'\n');
  if (cp != NULL)
    *cp = '\0';
  reply(500, "'%s': command not understood.", cbuf);
}

