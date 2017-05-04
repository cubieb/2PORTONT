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
     PRIVSEP = 258,
     USER = 259,
     GROUP = 260,
     CHROOT = 261,
     PATH = 262,
     PATHTYPE = 263,
     INCLUDE = 264,
     IDENTIFIER = 265,
     VENDORID = 266,
     LOGGING = 267,
     LOGLEV = 268,
     PADDING = 269,
     PAD_RANDOMIZE = 270,
     PAD_RANDOMIZELEN = 271,
     PAD_MAXLEN = 272,
     PAD_STRICT = 273,
     PAD_EXCLTAIL = 274,
     LISTEN = 275,
     X_ISAKMP = 276,
     X_ISAKMP_NATT = 277,
     X_ADMIN = 278,
     STRICT_ADDRESS = 279,
     ADMINSOCK = 280,
     DISABLED = 281,
     LDAPCFG = 282,
     LDAP_HOST = 283,
     LDAP_PORT = 284,
     LDAP_PVER = 285,
     LDAP_BASE = 286,
     LDAP_BIND_DN = 287,
     LDAP_BIND_PW = 288,
     LDAP_SUBTREE = 289,
     LDAP_ATTR_USER = 290,
     LDAP_ATTR_ADDR = 291,
     LDAP_ATTR_MASK = 292,
     LDAP_ATTR_GROUP = 293,
     LDAP_ATTR_MEMBER = 294,
     MODECFG = 295,
     CFG_NET4 = 296,
     CFG_MASK4 = 297,
     CFG_DNS4 = 298,
     CFG_NBNS4 = 299,
     CFG_DEFAULT_DOMAIN = 300,
     CFG_AUTH_SOURCE = 301,
     CFG_AUTH_GROUPS = 302,
     CFG_SYSTEM = 303,
     CFG_RADIUS = 304,
     CFG_PAM = 305,
     CFG_LDAP = 306,
     CFG_LOCAL = 307,
     CFG_NONE = 308,
     CFG_GROUP_SOURCE = 309,
     CFG_ACCOUNTING = 310,
     CFG_CONF_SOURCE = 311,
     CFG_MOTD = 312,
     CFG_POOL_SIZE = 313,
     CFG_AUTH_THROTTLE = 314,
     CFG_SPLIT_NETWORK = 315,
     CFG_SPLIT_LOCAL = 316,
     CFG_SPLIT_INCLUDE = 317,
     CFG_SPLIT_DNS = 318,
     CFG_PFS_GROUP = 319,
     CFG_SAVE_PASSWD = 320,
     RETRY = 321,
     RETRY_COUNTER = 322,
     RETRY_INTERVAL = 323,
     RETRY_PERSEND = 324,
     RETRY_PHASE1 = 325,
     RETRY_PHASE2 = 326,
     NATT_KA = 327,
     ALGORITHM_CLASS = 328,
     ALGORITHMTYPE = 329,
     STRENGTHTYPE = 330,
     SAINFO = 331,
     FROM = 332,
     REMOTE = 333,
     ANONYMOUS = 334,
     INHERIT = 335,
     EXCHANGE_MODE = 336,
     EXCHANGETYPE = 337,
     DOI = 338,
     DOITYPE = 339,
     SITUATION = 340,
     SITUATIONTYPE = 341,
     CERTIFICATE_TYPE = 342,
     CERTTYPE = 343,
     PEERS_CERTFILE = 344,
     CA_TYPE = 345,
     VERIFY_CERT = 346,
     SEND_CERT = 347,
     SEND_CR = 348,
     IDENTIFIERTYPE = 349,
     IDENTIFIERQUAL = 350,
     MY_IDENTIFIER = 351,
     PEERS_IDENTIFIER = 352,
     VERIFY_IDENTIFIER = 353,
     DNSSEC = 354,
     CERT_X509 = 355,
     CERT_PLAINRSA = 356,
     NONCE_SIZE = 357,
     DH_GROUP = 358,
     KEEPALIVE = 359,
     PASSIVE = 360,
     INITIAL_CONTACT = 361,
     NAT_TRAVERSAL = 362,
     REMOTE_FORCE_LEVEL = 363,
     PROPOSAL_CHECK = 364,
     PROPOSAL_CHECK_LEVEL = 365,
     GENERATE_POLICY = 366,
     GENERATE_LEVEL = 367,
     SUPPORT_PROXY = 368,
     PROPOSAL = 369,
     EXEC_PATH = 370,
     EXEC_COMMAND = 371,
     EXEC_SUCCESS = 372,
     EXEC_FAILURE = 373,
     GSS_ID = 374,
     GSS_ID_ENC = 375,
     GSS_ID_ENCTYPE = 376,
     COMPLEX_BUNDLE = 377,
     DPD = 378,
     DPD_DELAY = 379,
     DPD_RETRY = 380,
     DPD_MAXFAIL = 381,
     PH1ID = 382,
     XAUTH_LOGIN = 383,
     WEAK_PHASE1_CHECK = 384,
     PREFIX = 385,
     PORT = 386,
     PORTANY = 387,
     UL_PROTO = 388,
     ANY = 389,
     IKE_FRAG = 390,
     ESP_FRAG = 391,
     MODE_CFG = 392,
     PFS_GROUP = 393,
     LIFETIME = 394,
     LIFETYPE_TIME = 395,
     LIFETYPE_BYTE = 396,
     STRENGTH = 397,
     REMOTEID = 398,
     SCRIPT = 399,
     PHASE1_UP = 400,
     PHASE1_DOWN = 401,
     NUMBER = 402,
     SWITCH = 403,
     BOOLEAN = 404,
     HEXSTRING = 405,
     QUOTEDSTRING = 406,
     ADDRSTRING = 407,
     ADDRRANGE = 408,
     UNITTYPE_BYTE = 409,
     UNITTYPE_KBYTES = 410,
     UNITTYPE_MBYTES = 411,
     UNITTYPE_TBYTES = 412,
     UNITTYPE_SEC = 413,
     UNITTYPE_MIN = 414,
     UNITTYPE_HOUR = 415,
     EOS = 416,
     BOC = 417,
     EOC = 418,
     COMMA = 419
   };
#endif
/* Tokens.  */
#define PRIVSEP 258
#define USER 259
#define GROUP 260
#define CHROOT 261
#define PATH 262
#define PATHTYPE 263
#define INCLUDE 264
#define IDENTIFIER 265
#define VENDORID 266
#define LOGGING 267
#define LOGLEV 268
#define PADDING 269
#define PAD_RANDOMIZE 270
#define PAD_RANDOMIZELEN 271
#define PAD_MAXLEN 272
#define PAD_STRICT 273
#define PAD_EXCLTAIL 274
#define LISTEN 275
#define X_ISAKMP 276
#define X_ISAKMP_NATT 277
#define X_ADMIN 278
#define STRICT_ADDRESS 279
#define ADMINSOCK 280
#define DISABLED 281
#define LDAPCFG 282
#define LDAP_HOST 283
#define LDAP_PORT 284
#define LDAP_PVER 285
#define LDAP_BASE 286
#define LDAP_BIND_DN 287
#define LDAP_BIND_PW 288
#define LDAP_SUBTREE 289
#define LDAP_ATTR_USER 290
#define LDAP_ATTR_ADDR 291
#define LDAP_ATTR_MASK 292
#define LDAP_ATTR_GROUP 293
#define LDAP_ATTR_MEMBER 294
#define MODECFG 295
#define CFG_NET4 296
#define CFG_MASK4 297
#define CFG_DNS4 298
#define CFG_NBNS4 299
#define CFG_DEFAULT_DOMAIN 300
#define CFG_AUTH_SOURCE 301
#define CFG_AUTH_GROUPS 302
#define CFG_SYSTEM 303
#define CFG_RADIUS 304
#define CFG_PAM 305
#define CFG_LDAP 306
#define CFG_LOCAL 307
#define CFG_NONE 308
#define CFG_GROUP_SOURCE 309
#define CFG_ACCOUNTING 310
#define CFG_CONF_SOURCE 311
#define CFG_MOTD 312
#define CFG_POOL_SIZE 313
#define CFG_AUTH_THROTTLE 314
#define CFG_SPLIT_NETWORK 315
#define CFG_SPLIT_LOCAL 316
#define CFG_SPLIT_INCLUDE 317
#define CFG_SPLIT_DNS 318
#define CFG_PFS_GROUP 319
#define CFG_SAVE_PASSWD 320
#define RETRY 321
#define RETRY_COUNTER 322
#define RETRY_INTERVAL 323
#define RETRY_PERSEND 324
#define RETRY_PHASE1 325
#define RETRY_PHASE2 326
#define NATT_KA 327
#define ALGORITHM_CLASS 328
#define ALGORITHMTYPE 329
#define STRENGTHTYPE 330
#define SAINFO 331
#define FROM 332
#define REMOTE 333
#define ANONYMOUS 334
#define INHERIT 335
#define EXCHANGE_MODE 336
#define EXCHANGETYPE 337
#define DOI 338
#define DOITYPE 339
#define SITUATION 340
#define SITUATIONTYPE 341
#define CERTIFICATE_TYPE 342
#define CERTTYPE 343
#define PEERS_CERTFILE 344
#define CA_TYPE 345
#define VERIFY_CERT 346
#define SEND_CERT 347
#define SEND_CR 348
#define IDENTIFIERTYPE 349
#define IDENTIFIERQUAL 350
#define MY_IDENTIFIER 351
#define PEERS_IDENTIFIER 352
#define VERIFY_IDENTIFIER 353
#define DNSSEC 354
#define CERT_X509 355
#define CERT_PLAINRSA 356
#define NONCE_SIZE 357
#define DH_GROUP 358
#define KEEPALIVE 359
#define PASSIVE 360
#define INITIAL_CONTACT 361
#define NAT_TRAVERSAL 362
#define REMOTE_FORCE_LEVEL 363
#define PROPOSAL_CHECK 364
#define PROPOSAL_CHECK_LEVEL 365
#define GENERATE_POLICY 366
#define GENERATE_LEVEL 367
#define SUPPORT_PROXY 368
#define PROPOSAL 369
#define EXEC_PATH 370
#define EXEC_COMMAND 371
#define EXEC_SUCCESS 372
#define EXEC_FAILURE 373
#define GSS_ID 374
#define GSS_ID_ENC 375
#define GSS_ID_ENCTYPE 376
#define COMPLEX_BUNDLE 377
#define DPD 378
#define DPD_DELAY 379
#define DPD_RETRY 380
#define DPD_MAXFAIL 381
#define PH1ID 382
#define XAUTH_LOGIN 383
#define WEAK_PHASE1_CHECK 384
#define PREFIX 385
#define PORT 386
#define PORTANY 387
#define UL_PROTO 388
#define ANY 389
#define IKE_FRAG 390
#define ESP_FRAG 391
#define MODE_CFG 392
#define PFS_GROUP 393
#define LIFETIME 394
#define LIFETYPE_TIME 395
#define LIFETYPE_BYTE 396
#define STRENGTH 397
#define REMOTEID 398
#define SCRIPT 399
#define PHASE1_UP 400
#define PHASE1_DOWN 401
#define NUMBER 402
#define SWITCH 403
#define BOOLEAN 404
#define HEXSTRING 405
#define QUOTEDSTRING 406
#define ADDRSTRING 407
#define ADDRRANGE 408
#define UNITTYPE_BYTE 409
#define UNITTYPE_KBYTES 410
#define UNITTYPE_MBYTES 411
#define UNITTYPE_TBYTES 412
#define UNITTYPE_SEC 413
#define UNITTYPE_MIN 414
#define UNITTYPE_HOUR 415
#define EOS 416
#define BOC 417
#define EOC 418
#define COMMA 419




/* Copy the first part of user declarations.  */
#line 5 "./cfparse.y"

/*
 * Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002 and 2003 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include PATH_IPSEC_H

#ifdef ENABLE_HYBRID
#include <arpa/inet.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include <grp.h>

#include "var.h"
#include "misc.h"
#include "vmbuf.h"
#include "plog.h"
#include "sockmisc.h"
#include "str2val.h"
#include "genlist.h"
#include "debug.h"

#include "admin.h"
#include "privsep.h"
#include "cfparse_proto.h"
#include "cftoken_proto.h"
#include "algorithm.h"
#include "localconf.h"
#include "policy.h"
#include "sainfo.h"
#include "oakley.h"
#include "pfkey.h"
#include "remoteconf.h"
#include "grabmyaddr.h"
#include "isakmp_var.h"
#include "handler.h"
#include "isakmp.h"
#include "nattraversal.h"
#include "isakmp_frag.h"
#ifdef ENABLE_HYBRID
#include "resolv.h"
#include "isakmp_unity.h"
#include "isakmp_xauth.h"
#include "isakmp_cfg.h"
#endif
#include "ipsec_doi.h"
#include "strnames.h"
#include "gcmalloc.h"
#ifdef HAVE_GSSAPI
#include "gssapi.h"
#endif
#include "vendorid.h"
#include "rsalist.h"

struct proposalspec {
	time_t lifetime;		/* for isakmp/ipsec */
	int lifebyte;			/* for isakmp/ipsec */
	struct secprotospec *spspec;	/* the head is always current spec. */
	struct proposalspec *next;	/* the tail is the most prefered. */
	struct proposalspec *prev;
};

struct secprotospec {
	int prop_no;
	int trns_no;
	int strength;		/* for isakmp/ipsec */
	int encklen;		/* for isakmp/ipsec */
	time_t lifetime;	/* for isakmp */
	int lifebyte;		/* for isakmp */
	int proto_id;		/* for ipsec (isakmp?) */
	int ipsec_level;	/* for ipsec */
	int encmode;		/* for ipsec */
	int vendorid;		/* for isakmp */
	char *gssid;
	struct sockaddr *remote;
	int algclass[MAXALGCLASS];

	struct secprotospec *next;	/* the tail is the most prefiered. */
	struct secprotospec *prev;
	struct proposalspec *back;
};

static int num2dhgroup[] = {
	0,
	OAKLEY_ATTR_GRP_DESC_MODP768,
	OAKLEY_ATTR_GRP_DESC_MODP1024,
	OAKLEY_ATTR_GRP_DESC_EC2N155,
	OAKLEY_ATTR_GRP_DESC_EC2N185,
	OAKLEY_ATTR_GRP_DESC_MODP1536,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	OAKLEY_ATTR_GRP_DESC_MODP2048,
	OAKLEY_ATTR_GRP_DESC_MODP3072,
	OAKLEY_ATTR_GRP_DESC_MODP4096,
	OAKLEY_ATTR_GRP_DESC_MODP6144,
	OAKLEY_ATTR_GRP_DESC_MODP8192
};

static struct remoteconf *cur_rmconf;
static int tmpalgtype[MAXALGCLASS];
static struct sainfo *cur_sainfo;
static int cur_algclass;
static int oldloglevel = LLV_BASE;

static struct proposalspec *newprspec __P((void));
static void insprspec __P((struct proposalspec *, struct proposalspec **));
static struct secprotospec *newspspec __P((void));
static void insspspec __P((struct secprotospec *, struct proposalspec **));
static void adminsock_conf __P((vchar_t *, vchar_t *, vchar_t *, int));

static int set_isakmp_proposal
	__P((struct remoteconf *, struct proposalspec *));
static void clean_tmpalgtype __P((void));
static int expand_isakmpspec __P((int, int, int *,
	int, int, time_t, int, int, int, char *, struct remoteconf *));
static int listen_addr __P((struct sockaddr *addr, int udp_encap));

void freeetypes (struct etypes **etypes);

#if 0
static int fix_lifebyte __P((u_long));
#endif


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
#line 174 "./cfparse.y"
{
	unsigned long num;
	vchar_t *val;
	struct remoteconf *rmconf;
	struct sockaddr *saddr;
	struct sainfoalg *alg;
}
/* Line 187 of yacc.c.  */
#line 601 "y.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 614 "y.tab.c"

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
#define YYLAST   489

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  165
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  193
/* YYNRULES -- Number of rules.  */
#define YYNRULES  359
/* YYNRULES -- Number of states.  */
#define YYNSTATES  654

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   419

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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    29,    31,    33,    35,    40,
      41,    44,    45,    50,    51,    56,    57,    62,    63,    68,
      69,    74,    75,    81,    82,    87,    91,    95,    98,    99,
     104,   105,   110,   114,   116,   118,   123,   124,   127,   128,
     133,   134,   139,   140,   145,   146,   151,   152,   157,   162,
     163,   166,   167,   172,   173,   178,   179,   184,   185,   193,
     194,   199,   200,   205,   206,   210,   213,   214,   216,   217,
     223,   224,   227,   228,   233,   234,   239,   240,   245,   246,
     251,   252,   257,   258,   263,   264,   269,   270,   275,   276,
     281,   282,   287,   288,   293,   294,   299,   304,   305,   308,
     309,   314,   315,   320,   324,   328,   329,   335,   336,   342,
     343,   348,   349,   354,   355,   360,   361,   366,   367,   372,
     373,   378,   379,   384,   385,   390,   391,   396,   397,   402,
     403,   408,   409,   414,   415,   420,   421,   426,   427,   432,
     433,   438,   439,   444,   445,   450,   451,   456,   457,   462,
     463,   468,   470,   474,   476,   478,   482,   484,   486,   490,
     493,   495,   499,   501,   503,   507,   509,   514,   515,   518,
     519,   524,   525,   531,   532,   537,   538,   544,   545,   551,
     552,   558,   559,   560,   569,   571,   574,   577,   580,   586,
     593,   596,   597,   601,   604,   605,   608,   609,   614,   615,
     620,   621,   628,   629,   636,   637,   642,   643,   648,   649,
     655,   657,   658,   663,   666,   667,   669,   670,   672,   674,
     676,   678,   680,   681,   683,   684,   691,   692,   697,   701,
     704,   706,   707,   710,   711,   716,   717,   722,   723,   728,
     731,   732,   737,   738,   744,   745,   751,   752,   758,   759,
     764,   765,   770,   771,   776,   777,   782,   783,   789,   790,
     797,   798,   803,   804,   810,   811,   818,   819,   824,   825,
     830,   831,   836,   837,   842,   843,   848,   849,   854,   855,
     860,   861,   867,   868,   874,   875,   880,   881,   886,   887,
     892,   893,   898,   899,   904,   905,   910,   911,   916,   917,
     922,   923,   928,   929,   934,   935,   940,   941,   946,   947,
     952,   953,   960,   961,   966,   967,   974,   975,   981,   982,
     985,   986,   992,   993,   998,  1000,  1002,  1003,  1005,  1007,
    1008,  1011,  1012,  1017,  1018,  1025,  1026,  1033,  1034,  1039,
    1040,  1045,  1046,  1052,  1054,  1056,  1058,  1060,  1062,  1064
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     166,     0,    -1,    -1,   166,   167,    -1,   168,    -1,   176,
      -1,   180,    -1,   181,    -1,   182,    -1,   186,    -1,   188,
      -1,   196,    -1,   208,    -1,   224,    -1,   262,    -1,   271,
      -1,   293,    -1,   178,    -1,     3,   162,   169,   163,    -1,
      -1,   169,   170,    -1,    -1,     4,   151,   171,   161,    -1,
      -1,     4,   147,   172,   161,    -1,    -1,     5,   151,   173,
     161,    -1,    -1,     5,   147,   174,   161,    -1,    -1,     6,
     151,   175,   161,    -1,    -1,     7,     8,   151,   177,   161,
      -1,    -1,   122,   148,   179,   161,    -1,     9,   151,   161,
      -1,   120,   121,   161,    -1,    10,   183,    -1,    -1,    11,
     184,   151,   161,    -1,    -1,    94,   151,   185,   161,    -1,
      12,   187,   161,    -1,   150,    -1,    13,    -1,    14,   162,
     189,   163,    -1,    -1,   189,   190,    -1,    -1,    15,   148,
     191,   161,    -1,    -1,    16,   148,   192,   161,    -1,    -1,
      17,   147,   193,   161,    -1,    -1,    18,   148,   194,   161,
      -1,    -1,    19,   148,   195,   161,    -1,    20,   162,   197,
     163,    -1,    -1,   197,   198,    -1,    -1,    21,   206,   199,
     161,    -1,    -1,    22,   206,   200,   161,    -1,    -1,    23,
     201,   131,   161,    -1,    -1,    25,   151,   151,   151,   147,
     202,   161,    -1,    -1,    25,   151,   203,   161,    -1,    -1,
      25,    26,   204,   161,    -1,    -1,    24,   205,   161,    -1,
     152,   207,    -1,    -1,   131,    -1,    -1,    27,   209,   162,
     210,   163,    -1,    -1,   210,   211,    -1,    -1,    30,   147,
     212,   161,    -1,    -1,    28,   151,   213,   161,    -1,    -1,
      29,   147,   214,   161,    -1,    -1,    31,   151,   215,   161,
      -1,    -1,    34,   148,   216,   161,    -1,    -1,    32,   151,
     217,   161,    -1,    -1,    33,   151,   218,   161,    -1,    -1,
      35,   151,   219,   161,    -1,    -1,    36,   151,   220,   161,
      -1,    -1,    37,   151,   221,   161,    -1,    -1,    38,   151,
     222,   161,    -1,    -1,    39,   151,   223,   161,    -1,    40,
     162,   225,   163,    -1,    -1,   225,   226,    -1,    -1,    41,
     152,   227,   161,    -1,    -1,    42,   152,   228,   161,    -1,
      43,   252,   161,    -1,    44,   254,   161,    -1,    -1,    60,
      61,   256,   229,   161,    -1,    -1,    60,    62,   256,   230,
     161,    -1,    -1,    63,   260,   231,   161,    -1,    -1,    45,
     151,   232,   161,    -1,    -1,    46,    48,   233,   161,    -1,
      -1,    46,    49,   234,   161,    -1,    -1,    46,    50,   235,
     161,    -1,    -1,    46,    51,   236,   161,    -1,    -1,    47,
     258,   237,   161,    -1,    -1,    54,    48,   238,   161,    -1,
      -1,    54,    51,   239,   161,    -1,    -1,    55,    53,   240,
     161,    -1,    -1,    55,    48,   241,   161,    -1,    -1,    55,
      49,   242,   161,    -1,    -1,    55,    50,   243,   161,    -1,
      -1,    58,   147,   244,   161,    -1,    -1,    64,   147,   245,
     161,    -1,    -1,    65,   148,   246,   161,    -1,    -1,    59,
     147,   247,   161,    -1,    -1,    56,    52,   248,   161,    -1,
      -1,    56,    49,   249,   161,    -1,    -1,    56,    51,   250,
     161,    -1,    -1,    57,   151,   251,   161,    -1,   253,    -1,
     253,   164,   252,    -1,   152,    -1,   255,    -1,   255,   164,
     254,    -1,   152,    -1,   257,    -1,   256,   164,   257,    -1,
     152,   130,    -1,   259,    -1,   259,   164,   258,    -1,   151,
      -1,   261,    -1,   261,   164,   260,    -1,   151,    -1,    66,
     162,   263,   163,    -1,    -1,   263,   264,    -1,    -1,    67,
     147,   265,   161,    -1,    -1,    68,   147,   356,   266,   161,
      -1,    -1,    69,   147,   267,   161,    -1,    -1,    70,   147,
     356,   268,   161,    -1,    -1,    71,   147,   356,   269,   161,
      -1,    -1,    72,   147,   356,   270,   161,    -1,    -1,    -1,
      76,   272,   274,   276,   162,   277,   273,   163,    -1,    79,
      -1,    79,   275,    -1,   275,    79,    -1,   275,   275,    -1,
      94,   152,   289,   290,   291,    -1,    94,   152,   153,   289,
     290,   291,    -1,    94,   151,    -1,    -1,    77,    94,   347,
      -1,     5,   151,    -1,    -1,   277,   278,    -1,    -1,   138,
     346,   279,   161,    -1,    -1,   143,   147,   280,   161,    -1,
      -1,   139,   140,   147,   356,   281,   161,    -1,    -1,   139,
     141,   147,   357,   282,   161,    -1,    -1,    73,   283,   286,
     161,    -1,    -1,    10,    94,   284,   161,    -1,    -1,    96,
      94,   151,   285,   161,    -1,   288,    -1,    -1,   288,   287,
     164,   286,    -1,    74,   292,    -1,    -1,   130,    -1,    -1,
     131,    -1,   132,    -1,   147,    -1,   133,    -1,   134,    -1,
      -1,   147,    -1,    -1,    78,   297,    80,   297,   294,   296,
      -1,    -1,    78,   297,   295,   296,    -1,   162,   298,   163,
      -1,    79,   207,    -1,   206,    -1,    -1,   298,   299,    -1,
      -1,    81,   300,   342,   161,    -1,    -1,    83,    84,   301,
     161,    -1,    -1,    85,    86,   302,   161,    -1,    87,   343,
      -1,    -1,    89,   151,   303,   161,    -1,    -1,    90,   100,
     151,   304,   161,    -1,    -1,    89,   100,   151,   305,   161,
      -1,    -1,    89,   101,   151,   306,   161,    -1,    -1,    89,
      99,   307,   161,    -1,    -1,    91,   148,   308,   161,    -1,
      -1,    92,   148,   309,   161,    -1,    -1,    93,   148,   310,
     161,    -1,    -1,    96,    94,   347,   311,   161,    -1,    -1,
      96,    94,    95,   347,   312,   161,    -1,    -1,   128,   347,
     313,   161,    -1,    -1,    97,    94,   347,   314,   161,    -1,
      -1,    97,    94,    95,   347,   315,   161,    -1,    -1,    98,
     148,   316,   161,    -1,    -1,   102,   147,   317,   161,    -1,
      -1,   103,   318,   346,   161,    -1,    -1,   105,   148,   319,
     161,    -1,    -1,   135,   148,   320,   161,    -1,    -1,   135,
     108,   321,   161,    -1,    -1,   136,   147,   322,   161,    -1,
      -1,   144,   151,   145,   323,   161,    -1,    -1,   144,   151,
     146,   324,   161,    -1,    -1,   137,   148,   325,   161,    -1,
      -1,   129,   148,   326,   161,    -1,    -1,   111,   148,   327,
     161,    -1,    -1,   111,   112,   328,   161,    -1,    -1,   113,
     148,   329,   161,    -1,    -1,   106,   148,   330,   161,    -1,
      -1,   107,   148,   331,   161,    -1,    -1,   107,   108,   332,
     161,    -1,    -1,   123,   148,   333,   161,    -1,    -1,   124,
     147,   334,   161,    -1,    -1,   125,   147,   335,   161,    -1,
      -1,   126,   147,   336,   161,    -1,    -1,   127,   147,   337,
     161,    -1,    -1,   139,   140,   147,   356,   338,   161,    -1,
      -1,   109,   110,   339,   161,    -1,    -1,   139,   141,   147,
     357,   340,   161,    -1,    -1,   114,   341,   162,   348,   163,
      -1,    -1,   342,    82,    -1,    -1,   100,   151,   151,   344,
     161,    -1,    -1,   101,   151,   345,   161,    -1,    74,    -1,
     147,    -1,    -1,   152,    -1,   151,    -1,    -1,   348,   349,
      -1,    -1,   142,   350,    75,   161,    -1,    -1,   139,   140,
     147,   356,   351,   161,    -1,    -1,   139,   141,   147,   357,
     352,   161,    -1,    -1,   103,   346,   353,   161,    -1,    -1,
     119,   151,   354,   161,    -1,    -1,    73,    74,   292,   355,
     161,    -1,   158,    -1,   159,    -1,   160,    -1,   154,    -1,
     155,    -1,   156,    -1,   157,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   260,   260,   262,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   283,   285,
     287,   291,   290,   301,   301,   303,   302,   313,   313,   314,
     314,   320,   319,   340,   340,   345,   359,   371,   375,   374,
     380,   379,   395,   398,   407,   421,   423,   425,   428,   428,
     429,   429,   430,   430,   431,   431,   432,   432,   437,   439,
     441,   445,   444,   450,   449,   459,   458,   464,   463,   473,
     472,   482,   481,   490,   490,   493,   505,   506,   511,   511,
     522,   524,   528,   527,   539,   538,   550,   549,   559,   558,
     570,   569,   579,   578,   590,   589,   601,   600,   612,   611,
     623,   622,   634,   633,   645,   644,   659,   661,   663,   667,
     666,   678,   677,   688,   690,   693,   692,   702,   701,   711,
     710,   718,   717,   730,   729,   739,   738,   752,   751,   765,
     764,   778,   777,   785,   784,   794,   793,   807,   806,   816,
     815,   825,   824,   838,   837,   851,   850,   861,   860,   870,
     869,   879,   878,   888,   887,   897,   896,   910,   909,   923,
     922,   936,   937,   940,   957,   958,   961,   978,   979,   982,
    1004,  1005,  1008,  1038,  1039,  1042,  1075,  1077,  1079,  1083,
    1082,  1088,  1087,  1093,  1092,  1098,  1097,  1103,  1102,  1108,
    1107,  1124,  1132,  1123,  1167,  1172,  1177,  1182,  1189,  1238,
    1303,  1332,  1335,  1360,  1373,  1375,  1379,  1378,  1384,  1383,
    1389,  1388,  1394,  1393,  1405,  1405,  1410,  1409,  1415,  1414,
    1422,  1427,  1426,  1433,  1489,  1490,  1493,  1494,  1495,  1498,
    1499,  1500,  1503,  1504,  1510,  1509,  1536,  1535,  1559,  1630,
    1636,  1645,  1647,  1651,  1650,  1655,  1655,  1656,  1656,  1657,
    1659,  1658,  1672,  1671,  1683,  1682,  1693,  1692,  1716,  1715,
    1725,  1725,  1726,  1726,  1727,  1727,  1729,  1728,  1738,  1737,
    1747,  1746,  1764,  1763,  1781,  1780,  1797,  1797,  1798,  1798,
    1800,  1799,  1805,  1805,  1806,  1806,  1807,  1807,  1808,  1808,
    1818,  1818,  1825,  1825,  1832,  1832,  1833,  1833,  1836,  1836,
    1837,  1837,  1838,  1838,  1839,  1839,  1841,  1840,  1852,  1851,
    1863,  1862,  1871,  1870,  1880,  1879,  1889,  1888,  1898,  1897,
    1903,  1902,  1907,  1907,  1909,  1908,  1923,  1922,  1933,  1935,
    1959,  1958,  1974,  1973,  1995,  2003,  2015,  2016,  2017,  2019,
    2021,  2025,  2024,  2029,  2028,  2034,  2033,  2046,  2045,  2051,
    2050,  2064,  2063,  2161,  2162,  2163,  2166,  2167,  2168,  2169
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "PRIVSEP", "USER", "GROUP", "CHROOT",
  "PATH", "PATHTYPE", "INCLUDE", "IDENTIFIER", "VENDORID", "LOGGING",
  "LOGLEV", "PADDING", "PAD_RANDOMIZE", "PAD_RANDOMIZELEN", "PAD_MAXLEN",
  "PAD_STRICT", "PAD_EXCLTAIL", "LISTEN", "X_ISAKMP", "X_ISAKMP_NATT",
  "X_ADMIN", "STRICT_ADDRESS", "ADMINSOCK", "DISABLED", "LDAPCFG",
  "LDAP_HOST", "LDAP_PORT", "LDAP_PVER", "LDAP_BASE", "LDAP_BIND_DN",
  "LDAP_BIND_PW", "LDAP_SUBTREE", "LDAP_ATTR_USER", "LDAP_ATTR_ADDR",
  "LDAP_ATTR_MASK", "LDAP_ATTR_GROUP", "LDAP_ATTR_MEMBER", "MODECFG",
  "CFG_NET4", "CFG_MASK4", "CFG_DNS4", "CFG_NBNS4", "CFG_DEFAULT_DOMAIN",
  "CFG_AUTH_SOURCE", "CFG_AUTH_GROUPS", "CFG_SYSTEM", "CFG_RADIUS",
  "CFG_PAM", "CFG_LDAP", "CFG_LOCAL", "CFG_NONE", "CFG_GROUP_SOURCE",
  "CFG_ACCOUNTING", "CFG_CONF_SOURCE", "CFG_MOTD", "CFG_POOL_SIZE",
  "CFG_AUTH_THROTTLE", "CFG_SPLIT_NETWORK", "CFG_SPLIT_LOCAL",
  "CFG_SPLIT_INCLUDE", "CFG_SPLIT_DNS", "CFG_PFS_GROUP", "CFG_SAVE_PASSWD",
  "RETRY", "RETRY_COUNTER", "RETRY_INTERVAL", "RETRY_PERSEND",
  "RETRY_PHASE1", "RETRY_PHASE2", "NATT_KA", "ALGORITHM_CLASS",
  "ALGORITHMTYPE", "STRENGTHTYPE", "SAINFO", "FROM", "REMOTE", "ANONYMOUS",
  "INHERIT", "EXCHANGE_MODE", "EXCHANGETYPE", "DOI", "DOITYPE",
  "SITUATION", "SITUATIONTYPE", "CERTIFICATE_TYPE", "CERTTYPE",
  "PEERS_CERTFILE", "CA_TYPE", "VERIFY_CERT", "SEND_CERT", "SEND_CR",
  "IDENTIFIERTYPE", "IDENTIFIERQUAL", "MY_IDENTIFIER", "PEERS_IDENTIFIER",
  "VERIFY_IDENTIFIER", "DNSSEC", "CERT_X509", "CERT_PLAINRSA",
  "NONCE_SIZE", "DH_GROUP", "KEEPALIVE", "PASSIVE", "INITIAL_CONTACT",
  "NAT_TRAVERSAL", "REMOTE_FORCE_LEVEL", "PROPOSAL_CHECK",
  "PROPOSAL_CHECK_LEVEL", "GENERATE_POLICY", "GENERATE_LEVEL",
  "SUPPORT_PROXY", "PROPOSAL", "EXEC_PATH", "EXEC_COMMAND", "EXEC_SUCCESS",
  "EXEC_FAILURE", "GSS_ID", "GSS_ID_ENC", "GSS_ID_ENCTYPE",
  "COMPLEX_BUNDLE", "DPD", "DPD_DELAY", "DPD_RETRY", "DPD_MAXFAIL",
  "PH1ID", "XAUTH_LOGIN", "WEAK_PHASE1_CHECK", "PREFIX", "PORT", "PORTANY",
  "UL_PROTO", "ANY", "IKE_FRAG", "ESP_FRAG", "MODE_CFG", "PFS_GROUP",
  "LIFETIME", "LIFETYPE_TIME", "LIFETYPE_BYTE", "STRENGTH", "REMOTEID",
  "SCRIPT", "PHASE1_UP", "PHASE1_DOWN", "NUMBER", "SWITCH", "BOOLEAN",
  "HEXSTRING", "QUOTEDSTRING", "ADDRSTRING", "ADDRRANGE", "UNITTYPE_BYTE",
  "UNITTYPE_KBYTES", "UNITTYPE_MBYTES", "UNITTYPE_TBYTES", "UNITTYPE_SEC",
  "UNITTYPE_MIN", "UNITTYPE_HOUR", "EOS", "BOC", "EOC", "COMMA", "$accept",
  "statements", "statement", "privsep_statement", "privsep_stmts",
  "privsep_stmt", "@1", "@2", "@3", "@4", "@5", "path_statement", "@6",
  "special_statement", "@7", "include_statement", "gssenc_statement",
  "identifier_statement", "identifier_stmt", "@8", "@9",
  "logging_statement", "log_level", "padding_statement", "padding_stmts",
  "padding_stmt", "@10", "@11", "@12", "@13", "@14", "listen_statement",
  "listen_stmts", "listen_stmt", "@15", "@16", "@17", "@18", "@19", "@20",
  "@21", "ike_addrinfo_port", "ike_port", "ldapcfg_statement", "@22",
  "ldapcfg_stmts", "ldapcfg_stmt", "@23", "@24", "@25", "@26", "@27",
  "@28", "@29", "@30", "@31", "@32", "@33", "@34", "modecfg_statement",
  "modecfg_stmts", "modecfg_stmt", "@35", "@36", "@37", "@38", "@39",
  "@40", "@41", "@42", "@43", "@44", "@45", "@46", "@47", "@48", "@49",
  "@50", "@51", "@52", "@53", "@54", "@55", "@56", "@57", "@58", "@59",
  "addrdnslist", "addrdns", "addrwinslist", "addrwins", "splitnetlist",
  "splitnet", "authgrouplist", "authgroup", "splitdnslist", "splitdns",
  "timer_statement", "timer_stmts", "timer_stmt", "@60", "@61", "@62",
  "@63", "@64", "@65", "sainfo_statement", "@66", "@67", "sainfo_name",
  "sainfo_id", "sainfo_param", "sainfo_specs", "sainfo_spec", "@68", "@69",
  "@70", "@71", "@72", "@73", "@74", "algorithms", "@75", "algorithm",
  "prefix", "port", "ul_proto", "keylength", "remote_statement", "@76",
  "@77", "remote_specs_block", "remote_index", "remote_specs",
  "remote_spec", "@78", "@79", "@80", "@81", "@82", "@83", "@84", "@85",
  "@86", "@87", "@88", "@89", "@90", "@91", "@92", "@93", "@94", "@95",
  "@96", "@97", "@98", "@99", "@100", "@101", "@102", "@103", "@104",
  "@105", "@106", "@107", "@108", "@109", "@110", "@111", "@112", "@113",
  "@114", "@115", "@116", "@117", "@118", "@119", "exchange_types",
  "cert_spec", "@120", "@121", "dh_group_num", "identifierstring",
  "isakmpproposal_specs", "isakmpproposal_spec", "@122", "@123", "@124",
  "@125", "@126", "@127", "unittype_time", "unittype_byte", 0
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   165,   166,   166,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   168,   169,
     169,   171,   170,   172,   170,   173,   170,   174,   170,   175,
     170,   177,   176,   179,   178,   180,   181,   182,   184,   183,
     185,   183,   186,   187,   187,   188,   189,   189,   191,   190,
     192,   190,   193,   190,   194,   190,   195,   190,   196,   197,
     197,   199,   198,   200,   198,   201,   198,   202,   198,   203,
     198,   204,   198,   205,   198,   206,   207,   207,   209,   208,
     210,   210,   212,   211,   213,   211,   214,   211,   215,   211,
     216,   211,   217,   211,   218,   211,   219,   211,   220,   211,
     221,   211,   222,   211,   223,   211,   224,   225,   225,   227,
     226,   228,   226,   226,   226,   229,   226,   230,   226,   231,
     226,   232,   226,   233,   226,   234,   226,   235,   226,   236,
     226,   237,   226,   238,   226,   239,   226,   240,   226,   241,
     226,   242,   226,   243,   226,   244,   226,   245,   226,   246,
     226,   247,   226,   248,   226,   249,   226,   250,   226,   251,
     226,   252,   252,   253,   254,   254,   255,   256,   256,   257,
     258,   258,   259,   260,   260,   261,   262,   263,   263,   265,
     264,   266,   264,   267,   264,   268,   264,   269,   264,   270,
     264,   272,   273,   271,   274,   274,   274,   274,   275,   275,
     275,   276,   276,   276,   277,   277,   279,   278,   280,   278,
     281,   278,   282,   278,   283,   278,   284,   278,   285,   278,
     286,   287,   286,   288,   289,   289,   290,   290,   290,   291,
     291,   291,   292,   292,   294,   293,   295,   293,   296,   297,
     297,   298,   298,   300,   299,   301,   299,   302,   299,   299,
     303,   299,   304,   299,   305,   299,   306,   299,   307,   299,
     308,   299,   309,   299,   310,   299,   311,   299,   312,   299,
     313,   299,   314,   299,   315,   299,   316,   299,   317,   299,
     318,   299,   319,   299,   320,   299,   321,   299,   322,   299,
     323,   299,   324,   299,   325,   299,   326,   299,   327,   299,
     328,   299,   329,   299,   330,   299,   331,   299,   332,   299,
     333,   299,   334,   299,   335,   299,   336,   299,   337,   299,
     338,   299,   339,   299,   340,   299,   341,   299,   342,   342,
     344,   343,   345,   343,   346,   346,   347,   347,   347,   348,
     348,   350,   349,   351,   349,   352,   349,   353,   349,   354,
     349,   355,   349,   356,   356,   356,   357,   357,   357,   357
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     0,
       2,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     5,     0,     4,     3,     3,     2,     0,     4,
       0,     4,     3,     1,     1,     4,     0,     2,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     4,     0,
       2,     0,     4,     0,     4,     0,     4,     0,     7,     0,
       4,     0,     4,     0,     3,     2,     0,     1,     0,     5,
       0,     2,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     4,     0,     2,     0,
       4,     0,     4,     3,     3,     0,     5,     0,     5,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     1,     3,     1,     1,     3,     1,     1,     3,     2,
       1,     3,     1,     1,     3,     1,     4,     0,     2,     0,
       4,     0,     5,     0,     4,     0,     5,     0,     5,     0,
       5,     0,     0,     8,     1,     2,     2,     2,     5,     6,
       2,     0,     3,     2,     0,     2,     0,     4,     0,     4,
       0,     6,     0,     6,     0,     4,     0,     4,     0,     5,
       1,     0,     4,     2,     0,     1,     0,     1,     1,     1,
       1,     1,     0,     1,     0,     6,     0,     4,     3,     2,
       1,     0,     2,     0,     4,     0,     4,     0,     4,     2,
       0,     4,     0,     5,     0,     5,     0,     5,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     5,     0,     6,
       0,     4,     0,     5,     0,     6,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     5,     0,     5,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     6,     0,     4,     0,     6,     0,     5,     0,     2,
       0,     5,     0,     4,     1,     1,     0,     1,     1,     0,
       2,     0,     4,     0,     6,     0,     6,     0,     4,     0,
       4,     0,     5,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,     0,
      78,     0,     0,   191,     0,     0,     0,     3,     4,     5,
      17,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    19,     0,     0,    38,     0,    37,    44,    43,
       0,    46,    59,     0,   107,   177,     0,    76,    76,   240,
     236,     0,    33,     0,    31,    35,     0,    40,    42,     0,
       0,    80,     0,     0,   194,     0,   201,     0,    77,   239,
      75,     0,     0,    36,     0,     0,     0,     0,    18,    20,
       0,     0,     0,     0,     0,     0,     0,     0,    45,    47,
       0,     0,    65,    73,     0,    58,    60,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   106,   108,     0,     0,     0,
       0,     0,     0,   176,   178,   195,   200,   224,     0,     0,
       0,   196,   197,   234,   241,   237,    34,    23,    21,    27,
      25,    29,    32,    39,    41,    48,    50,    52,    54,    56,
      61,    63,     0,     0,    71,    69,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    79,    81,
     109,   111,   163,     0,   161,   166,     0,   164,   121,   123,
     125,   127,   129,   172,   131,   170,   133,   135,   139,   141,
     143,   137,   155,   157,   153,   159,   145,   151,     0,     0,
     175,   119,   173,   147,   149,   179,     0,   183,     0,     0,
       0,   225,   224,   226,   203,   336,   204,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,    84,    86,    82,    88,
      92,    94,    90,    96,    98,   100,   102,   104,     0,     0,
     113,     0,   114,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   115,   167,   117,     0,     0,     0,
       0,     0,   353,   354,   355,   181,     0,   185,   187,   189,
     226,   227,   228,     0,   338,   337,   202,   192,   235,   243,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,     0,     0,     0,     0,     0,     0,   326,
       0,     0,     0,     0,     0,   336,     0,     0,     0,     0,
       0,     0,   238,   242,    24,    22,    28,    26,    30,    49,
      51,    53,    55,    57,    62,    64,    66,    72,     0,    70,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   110,   112,   162,   165,   122,   124,   126,   128,
     130,   132,   171,   134,   136,   140,   142,   144,   138,   156,
     158,   154,   160,   146,   152,   169,     0,     0,     0,   120,
     174,   148,   150,   180,     0,   184,     0,     0,     0,     0,
     230,   231,   229,   198,     0,   214,     0,     0,     0,     0,
       0,   205,   328,   245,   247,     0,     0,   249,   258,     0,
       0,   250,     0,   260,   262,   264,   336,   336,   276,   278,
       0,   282,   304,   308,   306,   322,   300,   298,   302,     0,
     310,   312,   314,   316,   318,   270,   296,   286,   284,   288,
     294,     0,     0,     0,    67,    85,    87,    83,    89,    93,
      95,    91,    97,    99,   101,   103,   105,   168,   116,   118,
     182,   186,   188,   190,   199,   216,     0,     0,   334,   335,
     206,     0,     0,   208,   193,     0,     0,     0,     0,   332,
       0,   254,   256,     0,   252,     0,     0,     0,   336,   266,
     336,   272,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   339,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   290,   292,     0,
       0,   232,     0,   220,   218,     0,     0,     0,     0,   329,
     244,   246,   248,   330,     0,   259,     0,     0,   251,     0,
     261,   263,   265,   268,     0,   274,     0,   277,   279,   281,
     283,   305,   309,   307,   323,   301,   299,   303,     0,   311,
     313,   315,   317,   319,   271,   297,   287,   285,   289,   295,
     320,   356,   357,   358,   359,   324,     0,     0,    68,   217,
     233,   223,   215,     0,     0,   207,   210,   212,   209,     0,
     333,   255,   257,   253,     0,   267,     0,   273,     0,     0,
       0,     0,   341,   327,   340,     0,     0,   291,   293,     0,
     219,     0,     0,   331,   269,   275,   232,   347,   349,     0,
       0,     0,   321,   325,   222,   211,   213,   351,     0,     0,
       0,     0,     0,     0,   348,   350,   343,   345,   342,   352,
       0,     0,   344,   346
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    17,    18,    53,    79,   220,   219,   222,   221,
     223,    19,    80,    20,    74,    21,    22,    23,    37,    56,
      82,    24,    40,    25,    59,    89,   224,   225,   226,   227,
     228,    26,    60,    96,   229,   230,   152,   529,   235,   233,
     153,    49,    69,    27,    43,    97,   169,   352,   350,   351,
     353,   356,   354,   355,   357,   358,   359,   360,   361,    28,
      62,   116,   248,   249,   387,   388,   277,   254,   255,   256,
     257,   258,   259,   261,   262,   266,   263,   264,   265,   271,
     279,   280,   272,   269,   267,   268,   270,   173,   174,   176,
     177,   274,   275,   184,   185,   201,   202,    29,    63,   124,
     281,   394,   286,   396,   397,   398,    30,    46,   410,    66,
      67,   130,   297,   411,   535,   538,   621,   622,   476,   530,
     594,   532,   593,   533,   213,   293,   403,   591,    31,   217,
      72,   135,    50,   218,   333,   412,   486,   487,   493,   549,
     546,   547,   490,   495,   496,   497,   554,   604,   519,   556,
     606,   502,   503,   430,   505,   522,   521,   523,   586,   587,
     524,   520,   511,   510,   512,   506,   508,   507,   514,   515,
     516,   517,   518,   615,   509,   616,   439,   485,   417,   599,
     544,   480,   296,   568,   614,   631,   650,   651,   638,   639,
     643,   285,   585
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -519
static const yytype_int16 yypact[] =
{
    -519,    63,  -519,   -91,    74,   -67,    25,     7,   -63,   -51,
    -519,   -49,   -44,  -519,   -57,    17,     6,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,    27,   -18,  -519,    38,  -519,  -519,  -519,
      26,  -519,  -519,    29,  -519,  -519,    -1,    73,    73,  -519,
     127,    50,  -519,     1,  -519,  -519,    62,  -519,  -519,    -7,
      -8,  -519,    -4,   -23,   121,    -3,    59,     0,  -519,  -519,
    -519,   -57,    56,  -519,    58,   -71,   -35,    70,  -519,  -519,
      61,    64,    72,    75,    87,    77,    88,    89,  -519,  -519,
      68,    68,  -519,  -519,     9,  -519,  -519,    -5,    82,    91,
      92,    94,    90,    76,    97,    69,    48,    93,    98,   105,
     106,   104,    99,   107,   108,  -519,  -519,   110,   111,   113,
     114,   115,   116,  -519,  -519,  -519,  -519,   -62,   100,   151,
     102,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,   124,   109,  -519,   117,   118,   120,   125,   122,
     123,   126,   128,   129,   130,   131,   132,   133,  -519,  -519,
    -519,  -519,  -519,   134,    95,  -519,   135,   101,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,   136,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,   119,   119,
    -519,  -519,   137,  -519,  -519,  -519,     3,  -519,     3,     3,
       3,  -519,   145,    36,  -519,    18,  -519,    56,   103,   138,
     141,   142,   143,   144,   146,   147,   148,   149,   150,   152,
     153,   154,  -519,   155,   139,   157,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,   158,   159,
    -519,    92,  -519,    94,   160,   161,   162,   163,   164,   165,
      97,   166,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   156,   179,  -519,   179,   180,    99,   181,
     183,   184,  -519,  -519,  -519,  -519,   185,  -519,  -519,  -519,
      36,  -519,  -519,   -45,  -519,  -519,  -519,     8,  -519,  -519,
     194,   193,    71,   -14,   187,   140,   192,   199,   191,   195,
     200,   202,  -519,   203,   204,   -41,   182,   -38,   205,  -519,
     206,   208,   209,   210,   211,    18,   212,   -39,   214,   215,
      33,   213,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,   218,  -519,
     189,   198,   201,   207,   216,   217,   219,   220,   221,   222,
     223,   224,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,   119,   225,   226,  -519,
    -519,  -519,  -519,  -519,   227,  -519,   228,   229,   230,   -45,
    -519,  -519,  -519,  -519,   197,  -519,   272,   -17,    35,   232,
     231,  -519,  -519,  -519,  -519,   241,   242,  -519,  -519,   244,
     245,  -519,   246,  -519,  -519,  -519,   -37,   -30,  -519,  -519,
     -17,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,   236,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,   252,   253,    52,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,   238,   250,  -519,  -519,
    -519,   255,   256,  -519,  -519,   -61,   243,   247,   254,  -519,
     248,  -519,  -519,   249,  -519,   251,   257,   258,    18,  -519,
      18,  -519,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,  -519,   270,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,     3,   -22,  -519,  -519,   283,
     284,   286,   285,   287,  -519,   288,     3,   -22,   289,  -519,
    -519,  -519,  -519,  -519,   291,  -519,   292,   293,  -519,   294,
    -519,  -519,  -519,  -519,   295,  -519,   296,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,   -11,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,   297,   298,  -519,  -519,
    -519,  -519,  -519,   299,   300,  -519,  -519,  -519,  -519,   301,
    -519,  -519,  -519,  -519,   303,  -519,   304,  -519,   302,   -17,
     309,    40,  -519,  -519,  -519,   305,   306,  -519,  -519,   238,
    -519,   307,   308,  -519,  -519,  -519,   286,  -519,  -519,   323,
     324,   331,  -519,  -519,  -519,  -519,  -519,  -519,   311,   312,
       3,   -22,   313,   314,  -519,  -519,  -519,  -519,  -519,  -519,
     315,   316,  -519,  -519
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,   112,   319,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,    42,  -519,    41,
    -519,   290,   -89,    46,  -519,    20,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
      86,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -250,  -519,  -519,   235,    80,   -28,  -254,  -519,  -519,
    -519,   190,   340,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -427,  -321,  -519,  -519,  -519,  -519,  -519,  -519,  -519,
    -519,  -208,  -518
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -222
static const yytype_int16 yytable[] =
{
     287,   288,   289,   504,   445,    75,    76,    77,    83,    84,
      85,    86,    87,    90,    91,    92,    93,    94,   404,   597,
      38,   539,    47,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   154,    35,    98,    99,   100,
     101,   102,   103,   104,   117,   118,   119,   120,   121,   122,
     105,   106,   107,   108,   109,   110,   111,   478,   498,   112,
     113,   114,   608,     2,   128,   500,     3,   433,   211,   447,
       4,    32,     5,     6,   436,     7,   137,     8,    64,   131,
     138,   405,    33,     9,    34,   418,   419,   420,   400,   401,
      10,   212,   609,    65,    65,    48,   188,   189,   190,    41,
     540,   191,   402,    11,   406,   499,   501,   434,   610,   448,
     437,    42,   139,    44,   294,   295,   140,   186,    45,    36,
     187,   294,   295,   647,   179,   180,   181,   182,   611,    12,
     479,   612,   581,   582,   583,   584,   129,   421,    51,    13,
     123,    14,   192,    55,   193,   194,   407,   408,   126,   127,
     125,   409,   613,   132,    52,    95,    88,    39,   168,   115,
     155,   282,   283,   284,    78,   198,   199,   291,   292,   294,
     295,   415,   416,   451,   452,   481,   482,   553,    54,   555,
     629,   630,   627,    15,   299,    16,   300,    58,   301,    57,
     302,    61,   303,   304,   305,   306,   307,   527,   528,   308,
     309,   310,   150,   151,    68,   311,   312,    71,   313,   314,
     315,    73,   316,    81,   317,    65,   318,   319,   134,   136,
      48,   141,   142,   145,   147,   143,   320,   321,   322,   323,
     324,   325,   326,   144,   170,   146,   148,   149,   327,   328,
     329,   178,   330,   171,   172,   215,   175,   331,   183,   195,
     200,   214,   196,   197,   203,   231,   204,   205,   206,   251,
     207,   208,   209,   210,   216,   253,   332,   237,   234,   236,
     232,   273,   238,   239,   240,   211,   242,   241,   413,   414,
     243,   244,   245,   246,   247,   426,   385,   422,   423,   427,
     348,   475,   435,   364,   365,   250,   252,   467,   390,   334,
     260,   278,   335,   336,   337,   338,   372,   339,   340,   341,
     342,   343,   531,   344,   345,   346,   347,   580,   349,   362,
     363,   366,   367,   368,   369,   370,   371,   373,   596,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     424,   389,   391,   386,   392,   393,   395,   425,   428,   429,
     455,   431,   432,   438,   440,   441,   442,   443,   444,   456,
     446,   449,   457,   450,   453,   454,   477,    70,   458,   634,
     399,   474,   637,     0,     0,     0,   626,   459,   460,   483,
     461,   462,   463,   464,   465,   466,   468,   469,   470,   471,
     472,   473,   488,   489,   484,   491,   492,   494,   513,   525,
     526,   534,   536,   537,   541,   543,   642,   298,   542,   545,
     548,   133,   550,     0,     0,     0,     0,     0,   551,   552,
     557,   558,   559,   560,   561,   562,   563,   564,   565,   566,
     567,   569,   646,   590,   570,   571,   572,   573,   574,   575,
     576,   577,   578,   579,   588,   589,   592,   290,     0,   595,
     598,  -221,   600,   601,   602,   603,   605,   607,   617,   618,
     628,   620,   623,   619,   624,   625,   632,   633,   635,   636,
     640,   641,   644,   645,   648,   649,   652,   653,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   276
};

static const yytype_int16 yycheck[] =
{
     208,   209,   210,   430,   325,     4,     5,     6,    15,    16,
      17,    18,    19,    21,    22,    23,    24,    25,    10,   537,
      13,    82,    79,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    26,    11,    41,    42,    43,
      44,    45,    46,    47,    67,    68,    69,    70,    71,    72,
      54,    55,    56,    57,    58,    59,    60,    74,    95,    63,
      64,    65,    73,     0,     5,    95,     3,   108,   130,   108,
       7,   162,     9,    10,   112,    12,   147,    14,    79,    79,
     151,    73,     8,    20,   151,    99,   100,   101,   133,   134,
      27,   153,   103,    94,    94,   152,    48,    49,    50,   162,
     161,    53,   147,    40,    96,   426,   427,   148,   119,   148,
     148,   162,   147,   162,   151,   152,   151,    48,   162,    94,
      51,   151,   152,   641,    48,    49,    50,    51,   139,    66,
     147,   142,   154,   155,   156,   157,    77,   151,   121,    76,
     163,    78,    49,   161,    51,    52,   138,   139,   151,   152,
      64,   143,   163,    67,   148,   163,   163,   150,   163,   163,
     151,   158,   159,   160,   163,    61,    62,   131,   132,   151,
     152,   100,   101,   140,   141,   140,   141,   498,   151,   500,
     140,   141,   609,   120,    81,   122,    83,   161,    85,   151,
      87,   162,    89,    90,    91,    92,    93,   145,   146,    96,
      97,    98,    90,    91,   131,   102,   103,    80,   105,   106,
     107,   161,   109,   151,   111,    94,   113,   114,   162,   161,
     152,   151,   161,   148,   147,   161,   123,   124,   125,   126,
     127,   128,   129,   161,   152,   148,   148,   148,   135,   136,
     137,   151,   139,   152,   152,    94,   152,   144,   151,   151,
     151,   151,   147,   147,   147,   131,   148,   147,   147,   164,
     147,   147,   147,   147,   162,   164,   163,   147,   151,   151,
     161,   152,   147,   151,   151,   130,   148,   151,    84,    86,
     151,   151,   151,   151,   151,    94,   130,   100,   148,    94,
     151,    94,   110,   251,   253,   161,   161,   386,   278,   161,
     164,   164,   161,   161,   161,   161,   260,   161,   161,   161,
     161,   161,    74,   161,   161,   161,   161,   525,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   536,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     148,   161,   161,   164,   161,   161,   161,   148,   148,   147,
     161,   148,   148,   148,   148,   147,   147,   147,   147,   161,
     148,   147,   161,   148,   151,   147,    94,    48,   161,   619,
     290,   399,   626,    -1,    -1,    -1,    74,   161,   161,   147,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   151,   151,   163,   151,   151,   151,   162,   147,
     147,   151,   147,   147,   161,   151,    75,   217,   161,   161,
     161,    71,   161,    -1,    -1,    -1,    -1,    -1,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   640,   147,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   212,    -1,   161,
     161,   164,   161,   161,   161,   161,   161,   161,   161,   161,
     151,   161,   161,   164,   161,   161,   161,   161,   161,   161,
     147,   147,   161,   161,   161,   161,   161,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   199
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   166,     0,     3,     7,     9,    10,    12,    14,    20,
      27,    40,    66,    76,    78,   120,   122,   167,   168,   176,
     178,   180,   181,   182,   186,   188,   196,   208,   224,   262,
     271,   293,   162,     8,   151,    11,    94,   183,    13,   150,
     187,   162,   162,   209,   162,   162,   272,    79,   152,   206,
     297,   121,   148,   169,   151,   161,   184,   151,   161,   189,
     197,   162,   225,   263,    79,    94,   274,   275,   131,   207,
     207,    80,   295,   161,   179,     4,     5,     6,   163,   170,
     177,   151,   185,    15,    16,    17,    18,    19,   163,   190,
      21,    22,    23,    24,    25,   163,   198,   210,    41,    42,
      43,    44,    45,    46,    47,    54,    55,    56,    57,    58,
      59,    60,    63,    64,    65,   163,   226,    67,    68,    69,
      70,    71,    72,   163,   264,   275,   151,   152,     5,    77,
     276,    79,   275,   297,   162,   296,   161,   147,   151,   147,
     151,   151,   161,   161,   161,   148,   148,   147,   148,   148,
     206,   206,   201,   205,    26,   151,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,   163,   211,
     152,   152,   152,   252,   253,   152,   254,   255,   151,    48,
      49,    50,    51,   151,   258,   259,    48,    51,    48,    49,
      50,    53,    49,    51,    52,   151,   147,   147,    61,    62,
     151,   260,   261,   147,   148,   147,   147,   147,   147,   147,
     147,   130,   153,   289,   151,    94,   162,   294,   298,   172,
     171,   174,   173,   175,   191,   192,   193,   194,   195,   199,
     200,   131,   161,   204,   151,   203,   151,   147,   147,   151,
     151,   151,   148,   151,   151,   151,   151,   151,   227,   228,
     161,   164,   161,   164,   232,   233,   234,   235,   236,   237,
     164,   238,   239,   241,   242,   243,   240,   249,   250,   248,
     251,   244,   247,   152,   256,   257,   256,   231,   164,   245,
     246,   265,   158,   159,   160,   356,   267,   356,   356,   356,
     289,   131,   132,   290,   151,   152,   347,   277,   296,    81,
      83,    85,    87,    89,    90,    91,    92,    93,    96,    97,
      98,   102,   103,   105,   106,   107,   109,   111,   113,   114,
     123,   124,   125,   126,   127,   128,   129,   135,   136,   137,
     139,   144,   163,   299,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   151,   161,
     213,   214,   212,   215,   217,   218,   216,   219,   220,   221,
     222,   223,   161,   161,   252,   254,   161,   161,   161,   161,
     161,   161,   258,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   130,   164,   229,   230,   161,
     260,   161,   161,   161,   266,   161,   268,   269,   270,   290,
     133,   134,   147,   291,    10,    73,    96,   138,   139,   143,
     273,   278,   300,    84,    86,   100,   101,   343,    99,   100,
     101,   151,   100,   148,   148,   148,    94,    94,   148,   147,
     318,   148,   148,   108,   148,   110,   112,   148,   148,   341,
     148,   147,   147,   147,   147,   347,   148,   108,   148,   147,
     148,   140,   141,   151,   147,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   257,   161,   161,
     161,   161,   161,   161,   291,    94,   283,    94,    74,   147,
     346,   140,   141,   147,   163,   342,   301,   302,   151,   151,
     307,   151,   151,   303,   151,   308,   309,   310,    95,   347,
      95,   347,   316,   317,   346,   319,   330,   332,   331,   339,
     328,   327,   329,   162,   333,   334,   335,   336,   337,   313,
     326,   321,   320,   322,   325,   147,   147,   145,   146,   202,
     284,    74,   286,   288,   151,   279,   147,   147,   280,    82,
     161,   161,   161,   151,   345,   161,   305,   306,   161,   304,
     161,   161,   161,   347,   311,   347,   314,   161,   161,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   348,   161,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     356,   154,   155,   156,   157,   357,   323,   324,   161,   161,
     147,   292,   161,   287,   285,   161,   356,   357,   161,   344,
     161,   161,   161,   161,   312,   161,   315,   161,    73,   103,
     119,   139,   142,   163,   349,   338,   340,   161,   161,   164,
     161,   281,   282,   161,   161,   161,    74,   346,   151,   140,
     141,   350,   161,   161,   286,   161,   161,   292,   353,   354,
     147,   147,    75,   355,   161,   161,   356,   357,   161,   161,
     351,   352,   161,   161
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
        case 21:
#line 291 "./cfparse.y"
    {
			struct passwd *pw;

			if ((pw = getpwnam((yyvsp[(2) - (2)].val)->v)) == NULL) {
				yyerror("unknown user \"%s\"", (yyvsp[(2) - (2)].val)->v);
				return -1;
			}
			lcconf->uid = pw->pw_uid;
		}
    break;

  case 23:
#line 301 "./cfparse.y"
    { lcconf->uid = (yyvsp[(2) - (2)].num); }
    break;

  case 25:
#line 303 "./cfparse.y"
    {
			struct group *gr;

			if ((gr = getgrnam((yyvsp[(2) - (2)].val)->v)) == NULL) {
				yyerror("unknown group \"%s\"", (yyvsp[(2) - (2)].val)->v);
				return -1;
			}
			lcconf->gid = gr->gr_gid;
		}
    break;

  case 27:
#line 313 "./cfparse.y"
    { lcconf->gid = (yyvsp[(2) - (2)].num); }
    break;

  case 29:
#line 314 "./cfparse.y"
    { lcconf->chroot = (yyvsp[(2) - (2)].val)->v; }
    break;

  case 31:
#line 320 "./cfparse.y"
    {
			if ((yyvsp[(2) - (3)].num) >= LC_PATHTYPE_MAX) {
				yyerror("invalid path type %d", (yyvsp[(2) - (3)].num));
				return -1;
			}

			/* free old pathinfo */
			if (lcconf->pathinfo[(yyvsp[(2) - (3)].num)])
				racoon_free(lcconf->pathinfo[(yyvsp[(2) - (3)].num)]);

			/* set new pathinfo */
			lcconf->pathinfo[(yyvsp[(2) - (3)].num)] = racoon_strdup((yyvsp[(3) - (3)].val)->v);
			STRDUP_FATAL(lcconf->pathinfo[(yyvsp[(2) - (3)].num)]);
			vfree((yyvsp[(3) - (3)].val));
		}
    break;

  case 33:
#line 340 "./cfparse.y"
    { lcconf->complex_bundle = (yyvsp[(2) - (2)].num); }
    break;

  case 35:
#line 346 "./cfparse.y"
    {
			char path[MAXPATHLEN];

			getpathname(path, sizeof(path),
				LC_PATHTYPE_INCLUDE, (yyvsp[(2) - (3)].val)->v);
			vfree((yyvsp[(2) - (3)].val));
			if (yycf_switch_buffer(path) != 0)
				return -1;
		}
    break;

  case 36:
#line 360 "./cfparse.y"
    {
			if ((yyvsp[(2) - (3)].num) >= LC_GSSENC_MAX) {
				yyerror("invalid GSS ID encoding %d", (yyvsp[(2) - (3)].num));
				return -1;
			}
			lcconf->gss_id_enc = (yyvsp[(2) - (3)].num);
		}
    break;

  case 38:
#line 375 "./cfparse.y"
    {
			/*XXX to be deleted */
		}
    break;

  case 40:
#line 380 "./cfparse.y"
    {
			/*XXX to be deleted */
			(yyvsp[(2) - (2)].val)->l--;	/* nuke '\0' */
			lcconf->ident[(yyvsp[(1) - (2)].num)] = (yyvsp[(2) - (2)].val);
			if (lcconf->ident[(yyvsp[(1) - (2)].num)] == NULL) {
				yyerror("failed to set my ident: %s",
					strerror(errno));
				return -1;
			}
		}
    break;

  case 43:
#line 399 "./cfparse.y"
    {
			/*
			 * XXX ignore it because this specification
			 * will be obsoleted.
			 */
			yywarn("see racoon.conf(5), such a log specification will be obsoleted.");
			vfree((yyvsp[(1) - (1)].val));
		}
    break;

  case 44:
#line 408 "./cfparse.y"
    {
			/*
			 * set the loglevel to the value specified
			 * in the configuration file plus the number
			 * of -d options specified on the command line
			 */
			loglevel += (yyvsp[(1) - (1)].num) - oldloglevel;
			oldloglevel = (yyvsp[(1) - (1)].num);
		}
    break;

  case 48:
#line 428 "./cfparse.y"
    { lcconf->pad_random = (yyvsp[(2) - (2)].num); }
    break;

  case 50:
#line 429 "./cfparse.y"
    { lcconf->pad_randomlen = (yyvsp[(2) - (2)].num); }
    break;

  case 52:
#line 430 "./cfparse.y"
    { lcconf->pad_maxsize = (yyvsp[(2) - (2)].num); }
    break;

  case 54:
#line 431 "./cfparse.y"
    { lcconf->pad_strict = (yyvsp[(2) - (2)].num); }
    break;

  case 56:
#line 432 "./cfparse.y"
    { lcconf->pad_excltail = (yyvsp[(2) - (2)].num); }
    break;

  case 61:
#line 445 "./cfparse.y"
    {
			listen_addr ((yyvsp[(2) - (2)].saddr), 0);
		}
    break;

  case 63:
#line 450 "./cfparse.y"
    {
#ifdef ENABLE_NATT
			listen_addr ((yyvsp[(2) - (2)].saddr), 1);
#else
			yyerror("NAT-T support not compiled in.");
#endif
		}
    break;

  case 65:
#line 459 "./cfparse.y"
    {
			yyerror("admin directive is obsoleted.");
		}
    break;

  case 67:
#line 464 "./cfparse.y"
    {
#ifdef ENABLE_ADMINPORT
			adminsock_conf((yyvsp[(2) - (5)].val), (yyvsp[(3) - (5)].val), (yyvsp[(4) - (5)].val), (yyvsp[(5) - (5)].num));
#else
			yywarn("admin port support not compiled in");
#endif
		}
    break;

  case 69:
#line 473 "./cfparse.y"
    {
#ifdef ENABLE_ADMINPORT
			adminsock_conf((yyvsp[(2) - (2)].val), NULL, NULL, -1);
#else
			yywarn("admin port support not compiled in");
#endif
		}
    break;

  case 71:
#line 482 "./cfparse.y"
    {
#ifdef ENABLE_ADMINPORT
			adminsock_path = NULL;
#else
			yywarn("admin port support not compiled in");
#endif
		}
    break;

  case 73:
#line 490 "./cfparse.y"
    { lcconf->strict_address = TRUE; }
    break;

  case 75:
#line 494 "./cfparse.y"
    {
			char portbuf[10];

			snprintf(portbuf, sizeof(portbuf), "%ld", (yyvsp[(2) - (2)].num));
			(yyval.saddr) = str2saddr((yyvsp[(1) - (2)].val)->v, portbuf);
			vfree((yyvsp[(1) - (2)].val));
			if (!(yyval.saddr))
				return -1;
		}
    break;

  case 76:
#line 505 "./cfparse.y"
    { (yyval.num) = PORT_ISAKMP; }
    break;

  case 77:
#line 506 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 78:
#line 511 "./cfparse.y"
    {
#ifndef ENABLE_HYBRID
			yyerror("racoon not configured with --enable-hybrid");
			return -1;
#endif
#ifndef HAVE_LIBLDAP
			yyerror("racoon not configured with --with-libldap");
			return -1;
#endif
		}
    break;

  case 82:
#line 528 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (((yyvsp[(2) - (2)].num)<2)||((yyvsp[(2) - (2)].num)>3))
				yyerror("invalid ldap protocol version (2|3)");
			xauth_ldap_config.pver = (yyvsp[(2) - (2)].num);
#endif
#endif
		}
    break;

  case 84:
#line 539 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.host != NULL)
				vfree(xauth_ldap_config.host);
			xauth_ldap_config.host = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 86:
#line 550 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			xauth_ldap_config.port = (yyvsp[(2) - (2)].num);
#endif
#endif
		}
    break;

  case 88:
#line 559 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.base != NULL)
				vfree(xauth_ldap_config.base);
			xauth_ldap_config.base = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 90:
#line 570 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			xauth_ldap_config.subtree = (yyvsp[(2) - (2)].num);
#endif
#endif
		}
    break;

  case 92:
#line 579 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.bind_dn != NULL)
				vfree(xauth_ldap_config.bind_dn);
			xauth_ldap_config.bind_dn = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 94:
#line 590 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.bind_pw != NULL)
				vfree(xauth_ldap_config.bind_pw);
			xauth_ldap_config.bind_pw = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 96:
#line 601 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.attr_user != NULL)
				vfree(xauth_ldap_config.attr_user);
			xauth_ldap_config.attr_user = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 98:
#line 612 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.attr_addr != NULL)
				vfree(xauth_ldap_config.attr_addr);
			xauth_ldap_config.attr_addr = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 100:
#line 623 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.attr_mask != NULL)
				vfree(xauth_ldap_config.attr_mask);
			xauth_ldap_config.attr_mask = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 102:
#line 634 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.attr_group != NULL)
				vfree(xauth_ldap_config.attr_group);
			xauth_ldap_config.attr_group = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 104:
#line 645 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			if (xauth_ldap_config.attr_member != NULL)
				vfree(xauth_ldap_config.attr_member);
			xauth_ldap_config.attr_member = vdup((yyvsp[(2) - (2)].val));
#endif
#endif
		}
    break;

  case 109:
#line 667 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			if (inet_pton(AF_INET, (yyvsp[(2) - (2)].val)->v,
			     &isakmp_cfg_config.network4) != 1)
				yyerror("bad IPv4 network address.");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 111:
#line 678 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			if (inet_pton(AF_INET, (yyvsp[(2) - (2)].val)->v,
			    &isakmp_cfg_config.netmask4) != 1)
				yyerror("bad IPv4 netmask address.");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 115:
#line 693 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.splitnet_type = UNITY_LOCAL_LAN;
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 117:
#line 702 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.splitnet_type = UNITY_SPLIT_INCLUDE;
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 119:
#line 711 "./cfparse.y"
    {
#ifndef ENABLE_HYBRID
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 121:
#line 718 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			strncpy(&isakmp_cfg_config.default_domain[0], 
			    (yyvsp[(2) - (2)].val)->v, MAXPATHLEN);
			isakmp_cfg_config.default_domain[MAXPATHLEN] = '\0';
			vfree((yyvsp[(2) - (2)].val));
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 123:
#line 730 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.authsource = ISAKMP_CFG_AUTH_SYSTEM;
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 125:
#line 739 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBRADIUS
			isakmp_cfg_config.authsource = ISAKMP_CFG_AUTH_RADIUS;
#else /* HAVE_LIBRADIUS */
			yyerror("racoon not configured with --with-libradius");
#endif /* HAVE_LIBRADIUS */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 127:
#line 752 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBPAM
			isakmp_cfg_config.authsource = ISAKMP_CFG_AUTH_PAM;
#else /* HAVE_LIBPAM */
			yyerror("racoon not configured with --with-libpam");
#endif /* HAVE_LIBPAM */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 129:
#line 765 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			isakmp_cfg_config.authsource = ISAKMP_CFG_AUTH_LDAP;
#else /* HAVE_LIBLDAP */
			yyerror("racoon not configured with --with-libldap");
#endif /* HAVE_LIBLDAP */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 131:
#line 778 "./cfparse.y"
    {
#ifndef ENABLE_HYBRID
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 133:
#line 785 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.groupsource = ISAKMP_CFG_GROUP_SYSTEM;
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 135:
#line 794 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			isakmp_cfg_config.groupsource = ISAKMP_CFG_GROUP_LDAP;
#else /* HAVE_LIBLDAP */
			yyerror("racoon not configured with --with-libldap");
#endif /* HAVE_LIBLDAP */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 137:
#line 807 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.accounting = ISAKMP_CFG_ACCT_NONE;
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 139:
#line 816 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.accounting = ISAKMP_CFG_ACCT_SYSTEM;
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 141:
#line 825 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBRADIUS
			isakmp_cfg_config.accounting = ISAKMP_CFG_ACCT_RADIUS;
#else /* HAVE_LIBRADIUS */
			yyerror("racoon not configured with --with-libradius");
#endif /* HAVE_LIBRADIUS */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 143:
#line 838 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBPAM
			isakmp_cfg_config.accounting = ISAKMP_CFG_ACCT_PAM;
#else /* HAVE_LIBPAM */
			yyerror("racoon not configured with --with-libpam");
#endif /* HAVE_LIBPAM */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 145:
#line 851 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			if (isakmp_cfg_resize_pool((yyvsp[(2) - (2)].num)) != 0)
				yyerror("cannot allocate memory for pool");
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 147:
#line 861 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.pfs_group = (yyvsp[(2) - (2)].num);
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 149:
#line 870 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.save_passwd = (yyvsp[(2) - (2)].num);
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 151:
#line 879 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.auth_throttle = (yyvsp[(2) - (2)].num);
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 153:
#line 888 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.confsource = ISAKMP_CFG_CONF_LOCAL;
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 155:
#line 897 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBRADIUS
			isakmp_cfg_config.confsource = ISAKMP_CFG_CONF_RADIUS;
#else /* HAVE_LIBRADIUS */
			yyerror("racoon not configured with --with-libradius");
#endif /* HAVE_LIBRADIUS */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 157:
#line 910 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBLDAP
			isakmp_cfg_config.confsource = ISAKMP_CFG_CONF_LDAP;
#else /* HAVE_LIBLDAP */
			yyerror("racoon not configured with --with-libldap");
#endif /* HAVE_LIBLDAP */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 159:
#line 923 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			strncpy(&isakmp_cfg_config.motd[0], (yyvsp[(2) - (2)].val)->v, MAXPATHLEN);
			isakmp_cfg_config.motd[MAXPATHLEN] = '\0';
			vfree((yyvsp[(2) - (2)].val));
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 163:
#line 941 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			struct isakmp_cfg_config *icc = &isakmp_cfg_config;

			if (icc->dns4_index > MAXNS)
				yyerror("No more than %d DNS", MAXNS);
			if (inet_pton(AF_INET, (yyvsp[(1) - (1)].val)->v,
			    &icc->dns4[icc->dns4_index++]) != 1)
				yyerror("bad IPv4 DNS address.");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 166:
#line 962 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			struct isakmp_cfg_config *icc = &isakmp_cfg_config;

			if (icc->nbns4_index > MAXWINS)
				yyerror("No more than %d WINS", MAXWINS);
			if (inet_pton(AF_INET, (yyvsp[(1) - (1)].val)->v,
			    &icc->nbns4[icc->nbns4_index++]) != 1)
				yyerror("bad IPv4 WINS address.");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 169:
#line 983 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			struct isakmp_cfg_config *icc = &isakmp_cfg_config;
			struct unity_network network;

			if (inet_pton(AF_INET, (yyvsp[(1) - (2)].val)->v, &network.addr4) != 1)
				yyerror("bad IPv4 SPLIT address.");

			/* Turn $2 (the prefix) into a subnet mask */
			network.mask4.s_addr = ((yyvsp[(2) - (2)].num)) ? htonl(~((1 << (32 - (yyvsp[(2) - (2)].num))) - 1)) : 0;

			/* add the network to our list */ 
			if (splitnet_list_add(&icc->splitnet_list, &network,&icc->splitnet_count))
				yyerror("Unable to allocate split network");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 172:
#line 1009 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			char * groupname = NULL;
			char ** grouplist = NULL;
			struct isakmp_cfg_config *icc = &isakmp_cfg_config;

			grouplist = racoon_realloc(icc->grouplist,
					sizeof(char**)*(icc->groupcount+1));
			if (grouplist == NULL)
				yyerror("unable to allocate auth group list");

			groupname = racoon_malloc((yyvsp[(1) - (1)].val)->l+1);
			if (groupname == NULL)
				yyerror("unable to allocate auth group name");

			memcpy(groupname,(yyvsp[(1) - (1)].val)->v,(yyvsp[(1) - (1)].val)->l);
			groupname[(yyvsp[(1) - (1)].val)->l]=0;
			grouplist[icc->groupcount]=groupname;
			icc->grouplist = grouplist;
			icc->groupcount++;

			vfree((yyvsp[(1) - (1)].val));
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 175:
#line 1043 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			struct isakmp_cfg_config *icc = &isakmp_cfg_config;

			if (!icc->splitdns_len)
			{
				icc->splitdns_list = racoon_malloc((yyvsp[(1) - (1)].val)->l);
				if(icc->splitdns_list == NULL)
					yyerror("error allocating splitdns list buffer");
				memcpy(icc->splitdns_list,(yyvsp[(1) - (1)].val)->v,(yyvsp[(1) - (1)].val)->l);
				icc->splitdns_len = (yyvsp[(1) - (1)].val)->l;
			}
			else
			{
				int len = icc->splitdns_len + (yyvsp[(1) - (1)].val)->l + 1;
				icc->splitdns_list = racoon_realloc(icc->splitdns_list,len);
				if(icc->splitdns_list == NULL)
					yyerror("error allocating splitdns list buffer");
				icc->splitdns_list[icc->splitdns_len] = ',';
				memcpy(icc->splitdns_list + icc->splitdns_len + 1, (yyvsp[(1) - (1)].val)->v, (yyvsp[(1) - (1)].val)->l);
				icc->splitdns_len = len;
			}
			vfree((yyvsp[(1) - (1)].val));
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 179:
#line 1083 "./cfparse.y"
    {
			lcconf->retry_counter = (yyvsp[(2) - (2)].num);
		}
    break;

  case 181:
#line 1088 "./cfparse.y"
    {
			lcconf->retry_interval = (yyvsp[(2) - (3)].num) * (yyvsp[(3) - (3)].num);
		}
    break;

  case 183:
#line 1093 "./cfparse.y"
    {
			lcconf->count_persend = (yyvsp[(2) - (2)].num);
		}
    break;

  case 185:
#line 1098 "./cfparse.y"
    {
			lcconf->retry_checkph1 = (yyvsp[(2) - (3)].num) * (yyvsp[(3) - (3)].num);
		}
    break;

  case 187:
#line 1103 "./cfparse.y"
    {
			lcconf->wait_ph2complete = (yyvsp[(2) - (3)].num) * (yyvsp[(3) - (3)].num);
		}
    break;

  case 189:
#line 1108 "./cfparse.y"
    {
#ifdef ENABLE_NATT
        		if (libipsec_opt & LIBIPSEC_OPT_NATT)
				lcconf->natt_ka_interval = (yyvsp[(2) - (3)].num) * (yyvsp[(3) - (3)].num);
			else
                		yyerror("libipsec lacks NAT-T support");
#else
			yyerror("NAT-T support not compiled in.");
#endif
		}
    break;

  case 191:
#line 1124 "./cfparse.y"
    {
			cur_sainfo = newsainfo();
			if (cur_sainfo == NULL) {
				yyerror("failed to allocate sainfo");
				return -1;
			}
		}
    break;

  case 192:
#line 1132 "./cfparse.y"
    {
			struct sainfo *check;

			/* default */
			if (cur_sainfo->algs[algclass_ipsec_enc] == 0) {
				yyerror("no encryption algorithm at %s",
					sainfo2str(cur_sainfo));
				return -1;
			}
			if (cur_sainfo->algs[algclass_ipsec_auth] == 0) {
				yyerror("no authentication algorithm at %s",
					sainfo2str(cur_sainfo));
				return -1;
			}
			if (cur_sainfo->algs[algclass_ipsec_comp] == 0) {
				yyerror("no compression algorithm at %s",
					sainfo2str(cur_sainfo));
				return -1;
			}

			/* duplicate check */
			check = getsainfo(cur_sainfo->idsrc,
					  cur_sainfo->iddst,
					  cur_sainfo->id_i,
					  cur_sainfo->remoteid);
			if (check && (!check->idsrc && !cur_sainfo->idsrc)) {
				yyerror("duplicated sainfo: %s",
					sainfo2str(cur_sainfo));
				return -1;
			}
			inssainfo(cur_sainfo);
		}
    break;

  case 194:
#line 1168 "./cfparse.y"
    {
			cur_sainfo->idsrc = NULL;
			cur_sainfo->iddst = NULL;
		}
    break;

  case 195:
#line 1173 "./cfparse.y"
    {
			cur_sainfo->idsrc = NULL;
			cur_sainfo->iddst = (yyvsp[(2) - (2)].val);
		}
    break;

  case 196:
#line 1178 "./cfparse.y"
    {
			cur_sainfo->idsrc = (yyvsp[(1) - (2)].val);
			cur_sainfo->iddst = NULL;
		}
    break;

  case 197:
#line 1183 "./cfparse.y"
    {
			cur_sainfo->idsrc = (yyvsp[(1) - (2)].val);
			cur_sainfo->iddst = (yyvsp[(2) - (2)].val);
		}
    break;

  case 198:
#line 1190 "./cfparse.y"
    {
			char portbuf[10];
			struct sockaddr *saddr;

			if (((yyvsp[(5) - (5)].num) == IPPROTO_ICMP || (yyvsp[(5) - (5)].num) == IPPROTO_ICMPV6)
			 && ((yyvsp[(4) - (5)].num) != IPSEC_PORT_ANY || (yyvsp[(4) - (5)].num) != IPSEC_PORT_ANY)) {
				yyerror("port number must be \"any\".");
				return -1;
			}

			snprintf(portbuf, sizeof(portbuf), "%lu", (yyvsp[(4) - (5)].num));
			saddr = str2saddr((yyvsp[(2) - (5)].val)->v, portbuf);
			vfree((yyvsp[(2) - (5)].val));
			if (saddr == NULL)
				return -1;

			switch (saddr->sa_family) {
			case AF_INET:
				if ((yyvsp[(5) - (5)].num) == IPPROTO_ICMPV6) {
					yyerror("upper layer protocol mismatched.\n");
					racoon_free(saddr);
					return -1;
				}
				(yyval.val) = ipsecdoi_sockaddr2id(saddr,
										  (yyvsp[(3) - (5)].num) == ~0 ? (sizeof(struct in_addr) << 3): (yyvsp[(3) - (5)].num),
										  (yyvsp[(5) - (5)].num));
				break;
#ifdef INET6
			case AF_INET6:
				if ((yyvsp[(5) - (5)].num) == IPPROTO_ICMP) {
					yyerror("upper layer protocol mismatched.\n");
					racoon_free(saddr);
					return -1;
				}
				(yyval.val) = ipsecdoi_sockaddr2id(saddr, 
										  (yyvsp[(3) - (5)].num) == ~0 ? (sizeof(struct in6_addr) << 3): (yyvsp[(3) - (5)].num),
										  (yyvsp[(5) - (5)].num));
				break;
#endif
			default:
				yyerror("invalid family: %d", saddr->sa_family);
				(yyval.val) = NULL;
				break;
			}
			racoon_free(saddr);
			if ((yyval.val) == NULL)
				return -1;
		}
    break;

  case 199:
#line 1239 "./cfparse.y"
    {
			char portbuf[10];
			struct sockaddr *laddr = NULL, *haddr = NULL;
			char *cur = NULL;

			if (((yyvsp[(6) - (6)].num) == IPPROTO_ICMP || (yyvsp[(6) - (6)].num) == IPPROTO_ICMPV6)
			 && ((yyvsp[(5) - (6)].num) != IPSEC_PORT_ANY || (yyvsp[(5) - (6)].num) != IPSEC_PORT_ANY)) {
				yyerror("port number must be \"any\".");
				return -1;
			}

			snprintf(portbuf, sizeof(portbuf), "%lu", (yyvsp[(5) - (6)].num));
			
			laddr = str2saddr((yyvsp[(2) - (6)].val)->v, portbuf);
			if (laddr == NULL) {
			    return -1;
			}
			vfree((yyvsp[(2) - (6)].val));
			haddr = str2saddr((yyvsp[(3) - (6)].val)->v, portbuf);
			if (haddr == NULL) {
			    racoon_free(laddr);
			    return -1;
			}
			vfree((yyvsp[(3) - (6)].val));

			switch (laddr->sa_family) {
			case AF_INET:
				if ((yyvsp[(6) - (6)].num) == IPPROTO_ICMPV6) {
				    yyerror("upper layer protocol mismatched.\n");
				    if (laddr)
					racoon_free(laddr);
				    if (haddr)
					racoon_free(haddr);
				    return -1;
				}
                                (yyval.val) = ipsecdoi_sockrange2id(laddr, haddr, 
							   (yyvsp[(6) - (6)].num));
				break;
#ifdef INET6
			case AF_INET6:
				if ((yyvsp[(6) - (6)].num) == IPPROTO_ICMP) {
					yyerror("upper layer protocol mismatched.\n");
					if (laddr)
					    racoon_free(laddr);
					if (haddr)
					    racoon_free(haddr);
					return -1;
				}
				(yyval.val) = ipsecdoi_sockrange2id(laddr, haddr, 
							       (yyvsp[(6) - (6)].num));
				break;
#endif
			default:
				yyerror("invalid family: %d", laddr->sa_family);
				(yyval.val) = NULL;
				break;
			}
			if (laddr)
			    racoon_free(laddr);
			if (haddr)
			    racoon_free(haddr);
			if ((yyval.val) == NULL)
				return -1;
		}
    break;

  case 200:
#line 1304 "./cfparse.y"
    {
			struct ipsecdoi_id_b *id_b;

			if ((yyvsp[(1) - (2)].num) == IDTYPE_ASN1DN) {
				yyerror("id type forbidden: %d", (yyvsp[(1) - (2)].num));
				(yyval.val) = NULL;
				return -1;
			}

			(yyvsp[(2) - (2)].val)->l--;

			(yyval.val) = vmalloc(sizeof(*id_b) + (yyvsp[(2) - (2)].val)->l);
			if ((yyval.val) == NULL) {
				yyerror("failed to allocate identifier");
				return -1;
			}

			id_b = (struct ipsecdoi_id_b *)(yyval.val)->v;
			id_b->type = idtype2doi((yyvsp[(1) - (2)].num));

			id_b->proto_id = 0;
			id_b->port = 0;

			memcpy((yyval.val)->v + sizeof(*id_b), (yyvsp[(2) - (2)].val)->v, (yyvsp[(2) - (2)].val)->l);
		}
    break;

  case 201:
#line 1332 "./cfparse.y"
    {
			cur_sainfo->id_i = NULL;
		}
    break;

  case 202:
#line 1336 "./cfparse.y"
    {
			struct ipsecdoi_id_b *id_b;
			vchar_t *idv;

			if (set_identifier(&idv, (yyvsp[(2) - (3)].num), (yyvsp[(3) - (3)].val)) != 0) {
				yyerror("failed to set identifer.\n");
				return -1;
			}
			cur_sainfo->id_i = vmalloc(sizeof(*id_b) + idv->l);
			if (cur_sainfo->id_i == NULL) {
				yyerror("failed to allocate identifier");
				return -1;
			}

			id_b = (struct ipsecdoi_id_b *)cur_sainfo->id_i->v;
			id_b->type = idtype2doi((yyvsp[(2) - (3)].num));

			id_b->proto_id = 0;
			id_b->port = 0;

			memcpy(cur_sainfo->id_i->v + sizeof(*id_b),
			       idv->v, idv->l);
			vfree(idv);
		}
    break;

  case 203:
#line 1361 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			if ((cur_sainfo->group = vdup((yyvsp[(2) - (2)].val))) == NULL) {
				yyerror("failed to set sainfo xauth group.\n");
				return -1;
			}
#else
			yyerror("racoon not configured with --enable-hybrid");
			return -1;
#endif
 		}
    break;

  case 206:
#line 1379 "./cfparse.y"
    {
			cur_sainfo->pfs_group = (yyvsp[(2) - (2)].num);
		}
    break;

  case 208:
#line 1384 "./cfparse.y"
    {
			cur_sainfo->remoteid = (yyvsp[(2) - (2)].num);
		}
    break;

  case 210:
#line 1389 "./cfparse.y"
    {
			cur_sainfo->lifetime = (yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num);
		}
    break;

  case 212:
#line 1394 "./cfparse.y"
    {
#if 1
			yyerror("byte lifetime support is deprecated");
			return -1;
#else
			cur_sainfo->lifebyte = fix_lifebyte((yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num));
			if (cur_sainfo->lifebyte == 0)
				return -1;
#endif
		}
    break;

  case 214:
#line 1405 "./cfparse.y"
    {
			cur_algclass = (yyvsp[(1) - (1)].num);
		}
    break;

  case 216:
#line 1410 "./cfparse.y"
    {
			yyerror("it's deprecated to specify a identifier in phase 2");
		}
    break;

  case 218:
#line 1415 "./cfparse.y"
    {
			yyerror("it's deprecated to specify a identifier in phase 2");
		}
    break;

  case 220:
#line 1423 "./cfparse.y"
    {
			inssainfoalg(&cur_sainfo->algs[cur_algclass], (yyvsp[(1) - (1)].alg));
		}
    break;

  case 221:
#line 1427 "./cfparse.y"
    {
			inssainfoalg(&cur_sainfo->algs[cur_algclass], (yyvsp[(1) - (1)].alg));
		}
    break;

  case 223:
#line 1434 "./cfparse.y"
    {
			int defklen;

			(yyval.alg) = newsainfoalg();
			if ((yyval.alg) == NULL) {
				yyerror("failed to get algorithm allocation");
				return -1;
			}

			(yyval.alg)->alg = algtype2doi(cur_algclass, (yyvsp[(1) - (2)].num));
			if ((yyval.alg)->alg == -1) {
				yyerror("algorithm mismatched");
				racoon_free((yyval.alg));
				(yyval.alg) = NULL;
				return -1;
			}

			defklen = default_keylen(cur_algclass, (yyvsp[(1) - (2)].num));
			if (defklen == 0) {
				if ((yyvsp[(2) - (2)].num)) {
					yyerror("keylen not allowed");
					racoon_free((yyval.alg));
					(yyval.alg) = NULL;
					return -1;
				}
			} else {
				if ((yyvsp[(2) - (2)].num) && check_keylen(cur_algclass, (yyvsp[(1) - (2)].num), (yyvsp[(2) - (2)].num)) < 0) {
					yyerror("invalid keylen %d", (yyvsp[(2) - (2)].num));
					racoon_free((yyval.alg));
					(yyval.alg) = NULL;
					return -1;
				}
			}

			if ((yyvsp[(2) - (2)].num))
				(yyval.alg)->encklen = (yyvsp[(2) - (2)].num);
			else
				(yyval.alg)->encklen = defklen;

			/* check if it's supported algorithm by kernel */
			if (!(cur_algclass == algclass_ipsec_auth && (yyvsp[(1) - (2)].num) == algtype_non_auth)
			 && pk_checkalg(cur_algclass, (yyvsp[(1) - (2)].num), (yyval.alg)->encklen)) {
				int a = algclass2doi(cur_algclass);
				int b = algtype2doi(cur_algclass, (yyvsp[(1) - (2)].num));
				if (a == IPSECDOI_ATTR_AUTH)
					a = IPSECDOI_PROTO_IPSEC_AH;
				yyerror("algorithm %s not supported by the kernel (missing module?)",
					s_ipsecdoi_trns(a, b));
				racoon_free((yyval.alg));
				(yyval.alg) = NULL;
				return -1;
			}
		}
    break;

  case 224:
#line 1489 "./cfparse.y"
    { (yyval.num) = ~0; }
    break;

  case 225:
#line 1490 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 226:
#line 1493 "./cfparse.y"
    { (yyval.num) = IPSEC_PORT_ANY; }
    break;

  case 227:
#line 1494 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 228:
#line 1495 "./cfparse.y"
    { (yyval.num) = IPSEC_PORT_ANY; }
    break;

  case 229:
#line 1498 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 230:
#line 1499 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 231:
#line 1500 "./cfparse.y"
    { (yyval.num) = IPSEC_ULPROTO_ANY; }
    break;

  case 232:
#line 1503 "./cfparse.y"
    { (yyval.num) = 0; }
    break;

  case 233:
#line 1504 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 234:
#line 1510 "./cfparse.y"
    {
			struct remoteconf *new;
			struct proposalspec *prspec;

			new = copyrmconf((yyvsp[(4) - (4)].saddr));
			if (new == NULL) {
				yyerror("failed to get remoteconf for %s.", saddr2str ((yyvsp[(4) - (4)].saddr)));
				return -1;
			}

			new->remote = (yyvsp[(2) - (4)].saddr);
			new->inherited_from = getrmconf_strict((yyvsp[(4) - (4)].saddr), 1);
			new->proposal = NULL;
			new->prhead = NULL;
			cur_rmconf = new;

			prspec = newprspec();
			if (prspec == NULL || !cur_rmconf->inherited_from 
				|| !cur_rmconf->inherited_from->proposal)
				return -1;
			prspec->lifetime = cur_rmconf->inherited_from->proposal->lifetime;
			prspec->lifebyte = cur_rmconf->inherited_from->proposal->lifebyte;
			insprspec(prspec, &cur_rmconf->prhead);
		}
    break;

  case 236:
#line 1536 "./cfparse.y"
    {
			struct remoteconf *new;
			struct proposalspec *prspec;

			new = newrmconf();
			if (new == NULL) {
				yyerror("failed to get new remoteconf.");
				return -1;
			}

			new->remote = (yyvsp[(2) - (2)].saddr);
			cur_rmconf = new;

			prspec = newprspec();
			if (prspec == NULL)
				return -1;
			prspec->lifetime = oakley_get_defaultlifetime();
			insprspec(prspec, &cur_rmconf->prhead);
		}
    break;

  case 238:
#line 1560 "./cfparse.y"
    {
			/* check a exchange mode */
			if (cur_rmconf->etypes == NULL) {
				yyerror("no exchange mode specified.\n");
				return -1;
			}

			if (cur_rmconf->idvtype == IDTYPE_UNDEFINED)
				cur_rmconf->idvtype = IDTYPE_ADDRESS;


			if (cur_rmconf->idvtype == IDTYPE_ASN1DN) {
				if (cur_rmconf->mycertfile) {
					if (cur_rmconf->idv)
						yywarn("Both CERT and ASN1 ID "
						       "are set. Hope this is OK.\n");
					/* TODO: Preparse the DN here */
				} else if (cur_rmconf->idv) {
					/* OK, using asn1dn without X.509. */
				} else {
					yyerror("ASN1 ID not specified "
						"and no CERT defined!\n");
					return -1;
				}
			}
			
			if (cur_rmconf->prhead->spspec == NULL
				&& cur_rmconf->inherited_from
				&& cur_rmconf->inherited_from->prhead) {
				cur_rmconf->prhead->spspec = cur_rmconf->inherited_from->prhead->spspec;
			}
			if (set_isakmp_proposal(cur_rmconf, cur_rmconf->prhead) != 0)
				return -1;

			/* DH group settting if aggressive mode is there. */
			if (check_etypeok(cur_rmconf, ISAKMP_ETYPE_AGG) != NULL) {
				struct isakmpsa *p;
				int b = 0;

				/* DH group */
				for (p = cur_rmconf->proposal; p; p = p->next) {
					if (b == 0 || (b && b == p->dh_group)) {
						b = p->dh_group;
						continue;
					}
					yyerror("DH group must be equal "
						"in all proposals "
						"when aggressive mode is "
						"used.\n");
					return -1;
				}
				cur_rmconf->dh_group = b;

				if (cur_rmconf->dh_group == 0) {
					yyerror("DH group must be set in the proposal.\n");
					return -1;
				}

				/* DH group settting if PFS is required. */
				if (oakley_setdhgroup(cur_rmconf->dh_group,
						&cur_rmconf->dhgrp) < 0) {
					yyerror("failed to set DH value.\n");
					return -1;
				}
			}

			insrmconf(cur_rmconf);
		}
    break;

  case 239:
#line 1631 "./cfparse.y"
    {
			(yyval.saddr) = newsaddr(sizeof(struct sockaddr));
			(yyval.saddr)->sa_family = AF_UNSPEC;
			((struct sockaddr_in *)(yyval.saddr))->sin_port = htons((yyvsp[(2) - (2)].num));
		}
    break;

  case 240:
#line 1637 "./cfparse.y"
    {
			(yyval.saddr) = (yyvsp[(1) - (1)].saddr);
			if ((yyval.saddr) == NULL) {
				yyerror("failed to allocate sockaddr");
				return -1;
			}
		}
    break;

  case 243:
#line 1651 "./cfparse.y"
    {
			cur_rmconf->etypes = NULL;
		}
    break;

  case 245:
#line 1655 "./cfparse.y"
    { cur_rmconf->doitype = (yyvsp[(2) - (2)].num); }
    break;

  case 247:
#line 1656 "./cfparse.y"
    { cur_rmconf->sittype = (yyvsp[(2) - (2)].num); }
    break;

  case 250:
#line 1659 "./cfparse.y"
    {
			yywarn("This directive without certtype will be removed!\n");
			yywarn("Please use 'peers_certfile x509 \"%s\";' instead\n", (yyvsp[(2) - (2)].val)->v);
			cur_rmconf->getcert_method = ISAKMP_GETCERT_LOCALFILE;

			if (cur_rmconf->peerscertfile != NULL)
				racoon_free(cur_rmconf->peerscertfile);
			cur_rmconf->peerscertfile = racoon_strdup((yyvsp[(2) - (2)].val)->v);
			STRDUP_FATAL(cur_rmconf->peerscertfile);
			vfree((yyvsp[(2) - (2)].val));
		}
    break;

  case 252:
#line 1672 "./cfparse.y"
    {
			cur_rmconf->cacerttype = (yyvsp[(2) - (3)].num);
			cur_rmconf->getcacert_method = ISAKMP_GETCERT_LOCALFILE;
			if (cur_rmconf->cacertfile != NULL)
				racoon_free(cur_rmconf->cacertfile);
			cur_rmconf->cacertfile = racoon_strdup((yyvsp[(3) - (3)].val)->v);
			STRDUP_FATAL(cur_rmconf->cacertfile);
			vfree((yyvsp[(3) - (3)].val));
		}
    break;

  case 254:
#line 1683 "./cfparse.y"
    {
			cur_rmconf->getcert_method = ISAKMP_GETCERT_LOCALFILE;
			if (cur_rmconf->peerscertfile != NULL)
				racoon_free(cur_rmconf->peerscertfile);
			cur_rmconf->peerscertfile = racoon_strdup((yyvsp[(3) - (3)].val)->v);
			STRDUP_FATAL(cur_rmconf->peerscertfile);
			vfree((yyvsp[(3) - (3)].val));
		}
    break;

  case 256:
#line 1693 "./cfparse.y"
    {
			char path[MAXPATHLEN];
			int ret = 0;

			getpathname(path, sizeof(path),
				LC_PATHTYPE_CERT, (yyvsp[(3) - (3)].val)->v);
			vfree((yyvsp[(3) - (3)].val));

			if (cur_rmconf->getcert_method == ISAKMP_GETCERT_DNS) {
				yyerror("Different peers_certfile method "
					"already defined: %d!\n",
					cur_rmconf->getcert_method);
				return -1;
			}
			cur_rmconf->getcert_method = ISAKMP_GETCERT_LOCALFILE;
			if (rsa_parse_file(cur_rmconf->rsa_public, path, RSA_TYPE_PUBLIC)) {
				yyerror("Couldn't parse keyfile.\n", path);
				return -1;
			}
			plog(LLV_DEBUG, LOCATION, NULL, "Public PlainRSA keyfile parsed: %s\n", path);
		}
    break;

  case 258:
#line 1716 "./cfparse.y"
    {
			if (cur_rmconf->getcert_method) {
				yyerror("Different peers_certfile method already defined!\n");
				return -1;
			}
			cur_rmconf->getcert_method = ISAKMP_GETCERT_DNS;
			cur_rmconf->peerscertfile = NULL;
		}
    break;

  case 260:
#line 1725 "./cfparse.y"
    { cur_rmconf->verify_cert = (yyvsp[(2) - (2)].num); }
    break;

  case 262:
#line 1726 "./cfparse.y"
    { cur_rmconf->send_cert = (yyvsp[(2) - (2)].num); }
    break;

  case 264:
#line 1727 "./cfparse.y"
    { cur_rmconf->send_cr = (yyvsp[(2) - (2)].num); }
    break;

  case 266:
#line 1729 "./cfparse.y"
    {
			if (set_identifier(&cur_rmconf->idv, (yyvsp[(2) - (3)].num), (yyvsp[(3) - (3)].val)) != 0) {
				yyerror("failed to set identifer.\n");
				return -1;
			}
			cur_rmconf->idvtype = (yyvsp[(2) - (3)].num);
		}
    break;

  case 268:
#line 1738 "./cfparse.y"
    {
			if (set_identifier_qual(&cur_rmconf->idv, (yyvsp[(2) - (4)].num), (yyvsp[(4) - (4)].val), (yyvsp[(3) - (4)].num)) != 0) {
				yyerror("failed to set identifer.\n");
				return -1;
			}
			cur_rmconf->idvtype = (yyvsp[(2) - (4)].num);
		}
    break;

  case 270:
#line 1747 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			/* formerly identifier type login */
			if (xauth_rmconf_used(&cur_rmconf->xauth) == -1) {
				yyerror("failed to allocate xauth state\n");
				return -1;
			}
			if ((cur_rmconf->xauth->login = vdup((yyvsp[(2) - (2)].val))) == NULL) {
				yyerror("failed to set identifer.\n");
				return -1;
			}
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 272:
#line 1764 "./cfparse.y"
    {
			struct idspec  *id;
			id = newidspec();
			if (id == NULL) {
				yyerror("failed to allocate idspec");
				return -1;
			}
			if (set_identifier(&id->id, (yyvsp[(2) - (3)].num), (yyvsp[(3) - (3)].val)) != 0) {
				yyerror("failed to set identifer.\n");
				racoon_free(id);
				return -1;
			}
			id->idtype = (yyvsp[(2) - (3)].num);
			genlist_append (cur_rmconf->idvl_p, id);
		}
    break;

  case 274:
#line 1781 "./cfparse.y"
    {
			struct idspec  *id;
			id = newidspec();
			if (id == NULL) {
				yyerror("failed to allocate idspec");
				return -1;
			}
			if (set_identifier_qual(&id->id, (yyvsp[(2) - (4)].num), (yyvsp[(4) - (4)].val), (yyvsp[(3) - (4)].num)) != 0) {
				yyerror("failed to set identifer.\n");
				racoon_free(id);
				return -1;
			}
			id->idtype = (yyvsp[(2) - (4)].num);
			genlist_append (cur_rmconf->idvl_p, id);
		}
    break;

  case 276:
#line 1797 "./cfparse.y"
    { cur_rmconf->verify_identifier = (yyvsp[(2) - (2)].num); }
    break;

  case 278:
#line 1798 "./cfparse.y"
    { cur_rmconf->nonce_size = (yyvsp[(2) - (2)].num); }
    break;

  case 280:
#line 1800 "./cfparse.y"
    {
			yyerror("dh_group cannot be defined here.");
			return -1;
		}
    break;

  case 282:
#line 1805 "./cfparse.y"
    { cur_rmconf->passive = (yyvsp[(2) - (2)].num); }
    break;

  case 284:
#line 1806 "./cfparse.y"
    { cur_rmconf->ike_frag = (yyvsp[(2) - (2)].num); }
    break;

  case 286:
#line 1807 "./cfparse.y"
    { cur_rmconf->ike_frag = ISAKMP_FRAG_FORCE; }
    break;

  case 288:
#line 1808 "./cfparse.y"
    { 
#ifdef SADB_X_EXT_NAT_T_FRAG
        		if (libipsec_opt & LIBIPSEC_OPT_FRAG)
				cur_rmconf->esp_frag = (yyvsp[(2) - (2)].num); 
			else
                		yywarn("libipsec lacks IKE frag support");
#else
			yywarn("Your kernel does not support esp_frag");
#endif
		}
    break;

  case 290:
#line 1818 "./cfparse.y"
    { 
			if (cur_rmconf->script[SCRIPT_PHASE1_UP] != NULL)
				vfree(cur_rmconf->script[SCRIPT_PHASE1_UP]);

			cur_rmconf->script[SCRIPT_PHASE1_UP] = 
			    script_path_add(vdup((yyvsp[(2) - (3)].val)));
		}
    break;

  case 292:
#line 1825 "./cfparse.y"
    { 
			if (cur_rmconf->script[SCRIPT_PHASE1_DOWN] != NULL)
				vfree(cur_rmconf->script[SCRIPT_PHASE1_DOWN]);

			cur_rmconf->script[SCRIPT_PHASE1_DOWN] = 
			    script_path_add(vdup((yyvsp[(2) - (3)].val)));
		}
    break;

  case 294:
#line 1832 "./cfparse.y"
    { cur_rmconf->mode_cfg = (yyvsp[(2) - (2)].num); }
    break;

  case 296:
#line 1833 "./cfparse.y"
    {
			cur_rmconf->weak_phase1_check = (yyvsp[(2) - (2)].num);
		}
    break;

  case 298:
#line 1836 "./cfparse.y"
    { cur_rmconf->gen_policy = (yyvsp[(2) - (2)].num); }
    break;

  case 300:
#line 1837 "./cfparse.y"
    { cur_rmconf->gen_policy = (yyvsp[(2) - (2)].num); }
    break;

  case 302:
#line 1838 "./cfparse.y"
    { cur_rmconf->support_proxy = (yyvsp[(2) - (2)].num); }
    break;

  case 304:
#line 1839 "./cfparse.y"
    { cur_rmconf->ini_contact = (yyvsp[(2) - (2)].num); }
    break;

  case 306:
#line 1841 "./cfparse.y"
    {
#ifdef ENABLE_NATT
        		if (libipsec_opt & LIBIPSEC_OPT_NATT)
				cur_rmconf->nat_traversal = (yyvsp[(2) - (2)].num);
			else
                		yyerror("libipsec lacks NAT-T support");
#else
			yyerror("NAT-T support not compiled in.");
#endif
		}
    break;

  case 308:
#line 1852 "./cfparse.y"
    {
#ifdef ENABLE_NATT
			if (libipsec_opt & LIBIPSEC_OPT_NATT)
				cur_rmconf->nat_traversal = NATT_FORCE;
			else
                		yyerror("libipsec lacks NAT-T support");
#else
			yyerror("NAT-T support not compiled in.");
#endif
		}
    break;

  case 310:
#line 1863 "./cfparse.y"
    {
#ifdef ENABLE_DPD
			cur_rmconf->dpd = (yyvsp[(2) - (2)].num);
#else
			yyerror("DPD support not compiled in.");
#endif
		}
    break;

  case 312:
#line 1871 "./cfparse.y"
    {
#ifdef ENABLE_DPD
			cur_rmconf->dpd_interval = (yyvsp[(2) - (2)].num);
#else
			yyerror("DPD support not compiled in.");
#endif
		}
    break;

  case 314:
#line 1880 "./cfparse.y"
    {
#ifdef ENABLE_DPD
			cur_rmconf->dpd_retry = (yyvsp[(2) - (2)].num);
#else
			yyerror("DPD support not compiled in.");
#endif
		}
    break;

  case 316:
#line 1889 "./cfparse.y"
    {
#ifdef ENABLE_DPD
			cur_rmconf->dpd_maxfails = (yyvsp[(2) - (2)].num);
#else
			yyerror("DPD support not compiled in.");
#endif
		}
    break;

  case 318:
#line 1898 "./cfparse.y"
    {
			cur_rmconf->ph1id = (yyvsp[(2) - (2)].num);
		}
    break;

  case 320:
#line 1903 "./cfparse.y"
    {
			cur_rmconf->prhead->lifetime = (yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num);
		}
    break;

  case 322:
#line 1907 "./cfparse.y"
    { cur_rmconf->pcheck_level = (yyvsp[(2) - (2)].num); }
    break;

  case 324:
#line 1909 "./cfparse.y"
    {
#if 1
			yyerror("byte lifetime support is deprecated in Phase1");
			return -1;
#else
			yywarn("the lifetime of bytes in phase 1 "
				"will be ignored at the moment.");
			cur_rmconf->prhead->lifebyte = fix_lifebyte((yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num));
			if (cur_rmconf->prhead->lifebyte == 0)
				return -1;
#endif
		}
    break;

  case 326:
#line 1923 "./cfparse.y"
    {
			struct secprotospec *spspec;

			spspec = newspspec();
			if (spspec == NULL)
				return -1;
			insspspec(spspec, &cur_rmconf->prhead);
		}
    break;

  case 329:
#line 1936 "./cfparse.y"
    {
			struct etypes *new;
			new = racoon_malloc(sizeof(struct etypes));
			if (new == NULL) {
				yyerror("failed to allocate etypes");
				return -1;
			}
			new->type = (yyvsp[(2) - (2)].num);
			new->next = NULL;
			if (cur_rmconf->etypes == NULL)
				cur_rmconf->etypes = new;
			else {
				struct etypes *p;
				for (p = cur_rmconf->etypes;
				     p->next != NULL;
				     p = p->next)
					;
				p->next = new;
			}
		}
    break;

  case 330:
#line 1959 "./cfparse.y"
    {
			cur_rmconf->certtype = (yyvsp[(1) - (3)].num);
			if (cur_rmconf->mycertfile != NULL)
				racoon_free(cur_rmconf->mycertfile);
			cur_rmconf->mycertfile = racoon_strdup((yyvsp[(2) - (3)].val)->v);
			STRDUP_FATAL(cur_rmconf->mycertfile);
			vfree((yyvsp[(2) - (3)].val));
			if (cur_rmconf->myprivfile != NULL)
				racoon_free(cur_rmconf->myprivfile);
			cur_rmconf->myprivfile = racoon_strdup((yyvsp[(3) - (3)].val)->v);
			STRDUP_FATAL(cur_rmconf->myprivfile);
			vfree((yyvsp[(3) - (3)].val));
		}
    break;

  case 332:
#line 1974 "./cfparse.y"
    {
			char path[MAXPATHLEN];
			int ret = 0;

			getpathname(path, sizeof(path),
				LC_PATHTYPE_CERT, (yyvsp[(2) - (2)].val)->v);
			vfree((yyvsp[(2) - (2)].val));

			cur_rmconf->certtype = (yyvsp[(1) - (2)].num);
			cur_rmconf->send_cr = FALSE;
			cur_rmconf->send_cert = FALSE;
			cur_rmconf->verify_cert = FALSE;
			if (rsa_parse_file(cur_rmconf->rsa_private, path, RSA_TYPE_PRIVATE)) {
				yyerror("Couldn't parse keyfile.\n", path);
				return -1;
			}
			plog(LLV_DEBUG, LOCATION, NULL, "Private PlainRSA keyfile parsed: %s\n", path);
		}
    break;

  case 334:
#line 1996 "./cfparse.y"
    {
			(yyval.num) = algtype2doi(algclass_isakmp_dh, (yyvsp[(1) - (1)].num));
			if ((yyval.num) == -1) {
				yyerror("must be DH group");
				return -1;
			}
		}
    break;

  case 335:
#line 2004 "./cfparse.y"
    {
			if (ARRAYLEN(num2dhgroup) > (yyvsp[(1) - (1)].num) && num2dhgroup[(yyvsp[(1) - (1)].num)] != 0) {
				(yyval.num) = num2dhgroup[(yyvsp[(1) - (1)].num)];
			} else {
				yyerror("must be DH group");
				(yyval.num) = 0;
				return -1;
			}
		}
    break;

  case 336:
#line 2015 "./cfparse.y"
    { (yyval.val) = NULL; }
    break;

  case 337:
#line 2016 "./cfparse.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val); }
    break;

  case 338:
#line 2017 "./cfparse.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val); }
    break;

  case 341:
#line 2025 "./cfparse.y"
    {
			yyerror("strength directive is obsoleted.");
		}
    break;

  case 343:
#line 2029 "./cfparse.y"
    {
			cur_rmconf->prhead->spspec->lifetime = (yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num);
		}
    break;

  case 345:
#line 2034 "./cfparse.y"
    {
#if 1
			yyerror("byte lifetime support is deprecated");
			return -1;
#else
			cur_rmconf->prhead->spspec->lifebyte = fix_lifebyte((yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num));
			if (cur_rmconf->prhead->spspec->lifebyte == 0)
				return -1;
#endif
		}
    break;

  case 347:
#line 2046 "./cfparse.y"
    {
			cur_rmconf->prhead->spspec->algclass[algclass_isakmp_dh] = (yyvsp[(2) - (2)].num);
		}
    break;

  case 349:
#line 2051 "./cfparse.y"
    {
			if (cur_rmconf->prhead->spspec->vendorid != VENDORID_GSSAPI) {
				yyerror("wrong Vendor ID for gssapi_id");
				return -1;
			}
			if (cur_rmconf->prhead->spspec->gssid != NULL)
				racoon_free(cur_rmconf->prhead->spspec->gssid);
			cur_rmconf->prhead->spspec->gssid = 
			    racoon_strdup((yyvsp[(2) - (2)].val)->v);
			STRDUP_FATAL(cur_rmconf->prhead->spspec->gssid);
		}
    break;

  case 351:
#line 2064 "./cfparse.y"
    {
			int doi;
			int defklen;

			doi = algtype2doi((yyvsp[(1) - (3)].num), (yyvsp[(2) - (3)].num));
			if (doi == -1) {
				yyerror("algorithm mismatched 1");
				return -1;
			}

			switch ((yyvsp[(1) - (3)].num)) {
			case algclass_isakmp_enc:
			/* reject suppressed algorithms */
#ifndef HAVE_OPENSSL_RC5_H
				if ((yyvsp[(2) - (3)].num) == algtype_rc5) {
					yyerror("algorithm %s not supported",
					    s_attr_isakmp_enc(doi));
					return -1;
				}
#endif
#ifndef HAVE_OPENSSL_IDEA_H
				if ((yyvsp[(2) - (3)].num) == algtype_idea) {
					yyerror("algorithm %s not supported",
					    s_attr_isakmp_enc(doi));
					return -1;
				}
#endif

				cur_rmconf->prhead->spspec->algclass[algclass_isakmp_enc] = doi;
				defklen = default_keylen((yyvsp[(1) - (3)].num), (yyvsp[(2) - (3)].num));
				if (defklen == 0) {
					if ((yyvsp[(3) - (3)].num)) {
						yyerror("keylen not allowed");
						return -1;
					}
				} else {
					if ((yyvsp[(3) - (3)].num) && check_keylen((yyvsp[(1) - (3)].num), (yyvsp[(2) - (3)].num), (yyvsp[(3) - (3)].num)) < 0) {
						yyerror("invalid keylen %d", (yyvsp[(3) - (3)].num));
						return -1;
					}
				}
				if ((yyvsp[(3) - (3)].num))
					cur_rmconf->prhead->spspec->encklen = (yyvsp[(3) - (3)].num);
				else
					cur_rmconf->prhead->spspec->encklen = defklen;
				break;
			case algclass_isakmp_hash:
				cur_rmconf->prhead->spspec->algclass[algclass_isakmp_hash] = doi;
				break;
			case algclass_isakmp_ameth:
				cur_rmconf->prhead->spspec->algclass[algclass_isakmp_ameth] = doi;
				/*
				 * We may have to set the Vendor ID for the
				 * authentication method we're using.
				 */
				switch ((yyvsp[(2) - (3)].num)) {
				case algtype_gssapikrb:
					if (cur_rmconf->prhead->spspec->vendorid !=
					    VENDORID_UNKNOWN) {
						yyerror("Vendor ID mismatch "
						    "for auth method");
						return -1;
					}
					/*
					 * For interoperability with Win2k,
					 * we set the Vendor ID to "GSSAPI".
					 */
					cur_rmconf->prhead->spspec->vendorid =
					    VENDORID_GSSAPI;
					break;
				case algtype_rsasig:
					if (cur_rmconf->certtype == ISAKMP_CERT_PLAINRSA) {
						if (rsa_list_count(cur_rmconf->rsa_private) == 0) {
							yyerror ("Private PlainRSA key not set. "
								"Use directive 'certificate_type plainrsa ...'\n");
							return -1;
						}
						if (rsa_list_count(cur_rmconf->rsa_public) == 0) {
							yyerror ("Public PlainRSA keys not set. "
								"Use directive 'peers_certfile plainrsa ...'\n");
							return -1;
						}
					}
					break;
				default:
					break;
				}
				break;
			default:
				yyerror("algorithm mismatched 2");
				return -1;
			}
		}
    break;

  case 353:
#line 2161 "./cfparse.y"
    { (yyval.num) = 1; }
    break;

  case 354:
#line 2162 "./cfparse.y"
    { (yyval.num) = 60; }
    break;

  case 355:
#line 2163 "./cfparse.y"
    { (yyval.num) = (60 * 60); }
    break;

  case 356:
#line 2166 "./cfparse.y"
    { (yyval.num) = 1; }
    break;

  case 357:
#line 2167 "./cfparse.y"
    { (yyval.num) = 1024; }
    break;

  case 358:
#line 2168 "./cfparse.y"
    { (yyval.num) = (1024 * 1024); }
    break;

  case 359:
#line 2169 "./cfparse.y"
    { (yyval.num) = (1024 * 1024 * 1024); }
    break;


/* Line 1267 of yacc.c.  */
#line 4643 "y.tab.c"
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


#line 2171 "./cfparse.y"


static struct proposalspec *
newprspec()
{
	struct proposalspec *new;

	new = racoon_calloc(1, sizeof(*new));
	if (new == NULL)
		yyerror("failed to allocate proposal");

	return new;
}

/*
 * insert into head of list.
 */
static void
insprspec(prspec, head)
	struct proposalspec *prspec;
	struct proposalspec **head;
{
	if (*head != NULL)
		(*head)->prev = prspec;
	prspec->next = *head;
	*head = prspec;
}

static struct secprotospec *
newspspec()
{
	struct secprotospec *new;

	new = racoon_calloc(1, sizeof(*new));
	if (new == NULL) {
		yyerror("failed to allocate spproto");
		return NULL;
	}

	new->encklen = 0;	/*XXX*/

	/*
	 * Default to "uknown" vendor -- we will override this
	 * as necessary.  When we send a Vendor ID payload, an
	 * "unknown" will be translated to a KAME/racoon ID.
	 */
	new->vendorid = VENDORID_UNKNOWN;

	return new;
}

/*
 * insert into head of list.
 */
static void
insspspec(spspec, head)
	struct secprotospec *spspec;
	struct proposalspec **head;
{
	spspec->back = *head;

	if ((*head)->spspec != NULL)
		(*head)->spspec->prev = spspec;
	spspec->next = (*head)->spspec;
	(*head)->spspec = spspec;
}

/* set final acceptable proposal */
static int
set_isakmp_proposal(rmconf, prspec)
	struct remoteconf *rmconf;
	struct proposalspec *prspec;
{
	struct proposalspec *p;
	struct secprotospec *s;
	int prop_no = 1; 
	int trns_no = 1;
	int32_t types[MAXALGCLASS];

	p = prspec;
	if (p->next != 0) {
		plog(LLV_ERROR, LOCATION, NULL,
			"multiple proposal definition.\n");
		return -1;
	}

	/* mandatory check */
	if (p->spspec == NULL) {
		yyerror("no remote specification found: %s.\n",
			saddr2str(rmconf->remote));
		return -1;
	}
	for (s = p->spspec; s != NULL; s = s->next) {
		/* XXX need more to check */
		if (s->algclass[algclass_isakmp_enc] == 0) {
			yyerror("encryption algorithm required.");
			return -1;
		}
		if (s->algclass[algclass_isakmp_hash] == 0) {
			yyerror("hash algorithm required.");
			return -1;
		}
		if (s->algclass[algclass_isakmp_dh] == 0) {
			yyerror("DH group required.");
			return -1;
		}
		if (s->algclass[algclass_isakmp_ameth] == 0) {
			yyerror("authentication method required.");
			return -1;
		}
	}

	/* skip to last part */
	for (s = p->spspec; s->next != NULL; s = s->next)
		;

	while (s != NULL) {
		plog(LLV_DEBUG2, LOCATION, NULL,
			"lifetime = %ld\n", (long)
			(s->lifetime ? s->lifetime : p->lifetime));
		plog(LLV_DEBUG2, LOCATION, NULL,
			"lifebyte = %d\n",
			s->lifebyte ? s->lifebyte : p->lifebyte);
		plog(LLV_DEBUG2, LOCATION, NULL,
			"encklen=%d\n", s->encklen);

		memset(types, 0, ARRAYLEN(types));
		types[algclass_isakmp_enc] = s->algclass[algclass_isakmp_enc];
		types[algclass_isakmp_hash] = s->algclass[algclass_isakmp_hash];
		types[algclass_isakmp_dh] = s->algclass[algclass_isakmp_dh];
		types[algclass_isakmp_ameth] =
		    s->algclass[algclass_isakmp_ameth];

		/* expanding spspec */
		clean_tmpalgtype();
		trns_no = expand_isakmpspec(prop_no, trns_no, types,
				algclass_isakmp_enc, algclass_isakmp_ameth + 1,
				s->lifetime ? s->lifetime : p->lifetime,
				s->lifebyte ? s->lifebyte : p->lifebyte,
				s->encklen, s->vendorid, s->gssid,
				rmconf);
		if (trns_no == -1) {
			plog(LLV_ERROR, LOCATION, NULL,
				"failed to expand isakmp proposal.\n");
			return -1;
		}

		s = s->prev;
	}

	if (rmconf->proposal == NULL) {
		plog(LLV_ERROR, LOCATION, NULL,
			"no proposal found.\n");
		return -1;
	}

	return 0;
}

static void
clean_tmpalgtype()
{
	int i;
	for (i = 0; i < MAXALGCLASS; i++)
		tmpalgtype[i] = 0;	/* means algorithm undefined. */
}

static int
expand_isakmpspec(prop_no, trns_no, types,
		class, last, lifetime, lifebyte, encklen, vendorid, gssid,
		rmconf)
	int prop_no, trns_no;
	int *types, class, last;
	time_t lifetime;
	int lifebyte;
	int encklen;
	int vendorid;
	char *gssid;
	struct remoteconf *rmconf;
{
	struct isakmpsa *new;

	/* debugging */
    {
	int j;
	char tb[10];
	plog(LLV_DEBUG2, LOCATION, NULL,
		"p:%d t:%d\n", prop_no, trns_no);
	for (j = class; j < MAXALGCLASS; j++) {
		snprintf(tb, sizeof(tb), "%d", types[j]);
		plog(LLV_DEBUG2, LOCATION, NULL,
			"%s%s%s%s\n",
			s_algtype(j, types[j]),
			types[j] ? "(" : "",
			tb[0] == '0' ? "" : tb,
			types[j] ? ")" : "");
	}
	plog(LLV_DEBUG2, LOCATION, NULL, "\n");
    }

#define TMPALGTYPE2STR(n) \
	s_algtype(algclass_isakmp_##n, types[algclass_isakmp_##n])
		/* check mandatory values */
		if (types[algclass_isakmp_enc] == 0
		 || types[algclass_isakmp_ameth] == 0
		 || types[algclass_isakmp_hash] == 0
		 || types[algclass_isakmp_dh] == 0) {
			yyerror("few definition of algorithm "
				"enc=%s ameth=%s hash=%s dhgroup=%s.\n",
				TMPALGTYPE2STR(enc),
				TMPALGTYPE2STR(ameth),
				TMPALGTYPE2STR(hash),
				TMPALGTYPE2STR(dh));
			return -1;
		}
#undef TMPALGTYPE2STR

	/* set new sa */
	new = newisakmpsa();
	if (new == NULL) {
		yyerror("failed to allocate isakmp sa");
		return -1;
	}
	new->prop_no = prop_no;
	new->trns_no = trns_no++;
	new->lifetime = lifetime;
	new->lifebyte = lifebyte;
	new->enctype = types[algclass_isakmp_enc];
	new->encklen = encklen;
	new->authmethod = types[algclass_isakmp_ameth];
	new->hashtype = types[algclass_isakmp_hash];
	new->dh_group = types[algclass_isakmp_dh];
	new->vendorid = vendorid;
#ifdef HAVE_GSSAPI
	if (new->authmethod == OAKLEY_ATTR_AUTH_METHOD_GSSAPI_KRB) {
		if (gssid != NULL) {
			if ((new->gssid = vmalloc(strlen(gssid))) == NULL) {
				racoon_free(new);
				yyerror("failed to allocate gssid");
				return -1;
			}
			memcpy(new->gssid->v, gssid, new->gssid->l);
			racoon_free(gssid);
		} else {
			/*
			 * Allocate the default ID so that it gets put
			 * into a GSS ID attribute during the Phase 1
			 * exchange.
			 */
			new->gssid = gssapi_get_default_gss_id();
		}
	}
#endif
	insisakmpsa(new, rmconf);

	return trns_no;
}

static int
listen_addr (struct sockaddr *addr, int udp_encap)
{
	struct myaddrs *p;

	p = newmyaddr();
	if (p == NULL) {
		yyerror("failed to allocate myaddrs");
		return -1;
	}
	p->addr = addr;
	if (p->addr == NULL) {
		yyerror("failed to copy sockaddr ");
		delmyaddr(p);
		return -1;
	}
	p->udp_encap = udp_encap;

	insmyaddr(p, &lcconf->myaddrs);

	lcconf->autograbaddr = 0;
	return 0;
}

#if 0
/*
 * fix lifebyte.
 * Must be more than 1024B because its unit is kilobytes.
 * That is defined RFC2407.
 */
static int
fix_lifebyte(t)
	unsigned long t;
{
	if (t < 1024) {
		yyerror("byte size should be more than 1024B.");
		return 0;
	}

	return(t / 1024);
}
#endif

int
cfparse()
{
	int error;

	yycf_init_buffer();
	if (yycf_switch_buffer(lcconf->racoon_conf) != 0) {
		plog(LLV_ERROR, LOCATION, NULL, 
		    "could not read configuration file \"%s\"\n", 
		    lcconf->racoon_conf);
		return -1;
	}

	error = yyparse();
	if (error != 0) {
		if (yyerrorcount) {
			plog(LLV_ERROR, LOCATION, NULL,
				"fatal parse failure (%d errors)\n",
				yyerrorcount);
		} else {
			plog(LLV_ERROR, LOCATION, NULL,
				"fatal parse failure.\n");
		}
		return -1;
	}

	if (error == 0 && yyerrorcount) {
		plog(LLV_ERROR, LOCATION, NULL,
			"parse error is nothing, but yyerrorcount is %d.\n",
				yyerrorcount);
		exit(1);
	}

	yycf_clean_buffer();

	plog(LLV_DEBUG2, LOCATION, NULL, "parse successed.\n");

	return 0;
}

int
cfreparse()
{
	flushph2();
	flushph1();
	flushrmconf();
	flushsainfo();
	clean_tmpalgtype();
	yycf_init_buffer();

	if (yycf_switch_buffer(lcconf->racoon_conf) != 0)
		return -1;

	return(cfparse());
}

#ifdef ENABLE_ADMINPORT
static void
adminsock_conf(path, owner, group, mode_dec)
	vchar_t *path;
	vchar_t *owner;
	vchar_t *group;
	int mode_dec;
{
	struct passwd *pw = NULL;
	struct group *gr = NULL;
	mode_t mode = 0;
	uid_t uid;
	gid_t gid;
	int isnum;

	adminsock_path = path->v;

	if (owner == NULL)
		return;

	errno = 0;
	uid = atoi(owner->v);
	isnum = !errno;
	if (((pw = getpwnam(owner->v)) == NULL) && !isnum)
		yyerror("User \"%s\" does not exist", owner->v);

	if (pw)
		adminsock_owner = pw->pw_uid;
	else
		adminsock_owner = uid;

	if (group == NULL)
		return;

	errno = 0;
	gid = atoi(group->v);
	isnum = !errno;
	if (((gr = getgrnam(group->v)) == NULL) && !isnum)
		yyerror("Group \"%s\" does not exist", group->v);

	if (gr)
		adminsock_group = gr->gr_gid;
	else
		adminsock_group = gid;

	if (mode_dec == -1)
		return;

	if (mode_dec > 777)
		yyerror("Mode 0%03o is invalid", mode_dec);
	if (mode_dec >= 400) { mode += 0400; mode_dec -= 400; }
	if (mode_dec >= 200) { mode += 0200; mode_dec -= 200; }
	if (mode_dec >= 100) { mode += 0200; mode_dec -= 100; }

	if (mode_dec > 77)
		yyerror("Mode 0%03o is invalid", mode_dec);
	if (mode_dec >= 40) { mode += 040; mode_dec -= 40; }
	if (mode_dec >= 20) { mode += 020; mode_dec -= 20; }
	if (mode_dec >= 10) { mode += 020; mode_dec -= 10; }

	if (mode_dec > 7)
		yyerror("Mode 0%03o is invalid", mode_dec);
	if (mode_dec >= 4) { mode += 04; mode_dec -= 4; }
	if (mode_dec >= 2) { mode += 02; mode_dec -= 2; }
	if (mode_dec >= 1) { mode += 02; mode_dec -= 1; }
	
	adminsock_mode = mode;

	return;
}
#endif

