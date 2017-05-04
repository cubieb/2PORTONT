/**
 * Copyright (C) 1999-2007  Free Software Foundation, Inc.
 *
 * This file is part of GNU gengetopt
 *
 * GNU gengetopt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU gengetopt is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with gengetopt; see the file COPYING. If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


%{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <sstream>

#include "argsdef.h"
#include "parser.h"
extern int gengetopt_count_line;

static void update_count_line (char *str);
static void updateTokenInfo (int pos);

// the buffer for strings (possibly spanning more lines)
static std::ostringstream buff;

static void buffer(const char *s);
static char *flush_buffer();

#define LINEBUF_LEN 1024

#define PUSH(s) yy_push_state(s);
#define POP() yy_pop_state();

int tokenpos = 0;
char linebuf[LINEBUF_LEN] ; /* current line */

//#define DEBUG_SCANNER
#ifdef DEBUG_SCANNER
#include <iostream> // for debug
#define DEB(s) std::cerr << s << std::endl;
#define DEB2(s,s2) std::cerr << s << ": " << s2 << std::endl;
#else
#define DEB(s)
#define DEB2(s,s2)
#endif

%}

%option stack nomain yylineno noyywrap nounput

ws [ ]+
tabs [\t]+

%s SIZE_STATE STRING_STATE

%%


<INITIAL>string		updateTokenInfo (-1); yylval.argtype = ARG_STRING; return TOK_ARGTYPE;
<INITIAL>int		updateTokenInfo (-1); yylval.argtype = ARG_INT; return TOK_ARGTYPE;
<INITIAL>short		updateTokenInfo (-1); yylval.argtype = ARG_SHORT; return TOK_ARGTYPE;
<INITIAL>long		updateTokenInfo (-1); yylval.argtype = ARG_LONG; return TOK_ARGTYPE;
<INITIAL>float		updateTokenInfo (-1); yylval.argtype = ARG_FLOAT; return TOK_ARGTYPE;
<INITIAL>double		updateTokenInfo (-1); yylval.argtype = ARG_DOUBLE; return TOK_ARGTYPE;
<INITIAL>longdouble	updateTokenInfo (-1); yylval.argtype = ARG_LONGDOUBLE; return TOK_ARGTYPE;
<INITIAL>longlong	updateTokenInfo (-1); yylval.argtype = ARG_LONGLONG; return TOK_ARGTYPE;
<INITIAL>enum		updateTokenInfo (-1); yylval.argtype = ARG_ENUM; return TOK_ARGTYPE;

<INITIAL>package        	updateTokenInfo (-1); return TOK_PACKAGE;
<INITIAL>version        	updateTokenInfo (-1); return TOK_VERSION;
<INITIAL>groupoption     updateTokenInfo (-1); yylloc.first_line = gengetopt_count_line; return TOK_GROUPOPTION;
<INITIAL>modeoption     updateTokenInfo (-1); yylloc.first_line = gengetopt_count_line; return TOK_MODEOPTION;
<INITIAL>yes|required    updateTokenInfo (-1); return TOK_YES;
<INITIAL>no|optional	updateTokenInfo (-1); return TOK_NO;
<INITIAL>option		updateTokenInfo (-1); yylloc.first_line = gengetopt_count_line; return TOK_OPTION;
<INITIAL>flag		updateTokenInfo (-1); return TOK_FLAG;
<INITIAL>on	        updateTokenInfo (-1); return TOK_ON;
<INITIAL>off	        updateTokenInfo (-1); return TOK_OFF;
<INITIAL>purpose	updateTokenInfo (-1); return TOK_PURPOSE;
<INITIAL>description	updateTokenInfo (-1); return TOK_DESCRIPTION;
<INITIAL>usage		updateTokenInfo (-1); return TOK_USAGE;
<INITIAL>default	updateTokenInfo (-1); return TOK_DEFAULT;
<INITIAL>typestr	updateTokenInfo (-1); return TOK_TYPESTR;
<INITIAL>group		updateTokenInfo (-1); return TOK_GROUP;
<INITIAL>groupdesc	updateTokenInfo (-1); return TOK_GROUPDESC;
<INITIAL>defgroup	updateTokenInfo (-1); return TOK_DEFGROUP;
<INITIAL>mode		updateTokenInfo (-1); return TOK_MODE;
<INITIAL>modedesc	updateTokenInfo (-1); return TOK_MODEDESC;
<INITIAL>defmode	updateTokenInfo (-1); return TOK_DEFMODE;
<INITIAL>multiple	updateTokenInfo (-1); return TOK_MULTIPLE;
<INITIAL>argoptional	updateTokenInfo (-1); return TOK_ARGOPTIONAL;
<INITIAL>sectiondesc     updateTokenInfo (-1); return TOK_SECTIONDESC;
<INITIAL>section         updateTokenInfo (-1); return TOK_SECTION;
<INITIAL>values          updateTokenInfo (-1); return TOK_VALUES;
<INITIAL>hidden		updateTokenInfo (-1); return TOK_HIDDEN;
<INITIAL>dependon	updateTokenInfo (-1); return TOK_DEPENDON;
<INITIAL>details	updateTokenInfo (-1); return TOK_DETAILS;
<INITIAL>text    	updateTokenInfo (-1); return TOK_TEXT;
<INITIAL>args    	updateTokenInfo (-1); return TOK_ARGS;

<INITIAL>"=" { updateTokenInfo (-1); return '='; }
<INITIAL>"," { updateTokenInfo (-1); return ','; }

<INITIAL>"(" { PUSH(SIZE_STATE); updateTokenInfo (-1); return '('; }
<SIZE_STATE>"-" { updateTokenInfo (-1); return '-'; }
<SIZE_STATE>[[:digit:]]+ { updateTokenInfo (-1); yylval.str = strdup(yytext); return TOK_SIZE; }
<SIZE_STATE>")" { POP(); updateTokenInfo (-1); return ')'; }

<INITIAL>[[:alnum:]-]|\?	 updateTokenInfo (-1); yylval.chr = yytext[0]; return TOK_CHAR;

<INITIAL>\" { updateTokenInfo (-1); DEB("start string"); PUSH(STRING_STATE) ; }
<STRING_STATE>\\\\n { updateTokenInfo (2); buffer("\\\\n"); }
<STRING_STATE>\\n { updateTokenInfo (1); buffer("\n"); }
<STRING_STATE>\n+ {  update_count_line (yytext); buffer( yytext ) ; }
<STRING_STATE>\\{ws}*\n { update_count_line (yytext); /* a line break */ }
<STRING_STATE>\\\" { updateTokenInfo (-1); buffer(yytext); }
<STRING_STATE>\" { updateTokenInfo (-1); DEB("end string"); POP() ; yylval.str = flush_buffer(); return TOK_STRING; }
<STRING_STATE>[^\n] {  updateTokenInfo (-1); buffer( yytext ) ; }

<INITIAL>{ws}           updateTokenInfo (-1);
<INITIAL>{tabs}         updateTokenInfo (8*yyleng);

<INITIAL,SIZE_STATE>#[^\n]*	/* comments begin with # in any place (but strings) */
<INITIAL>\n.*		update_count_line (0); yyless(1) ; /* give back all but the \n to rescan */

\r              {}

.		{
  return 1000; /* little hack to produce a parse error too. */
}

%%

/*
  Otherwise '\n' within a TOK_MLSTRING would not be counted
*/
void
update_count_line (char *str)
{
  if (str)
    {
      char *p;
      for (p = str; *p; ++p)
        if (*p == '\n') {
          ++gengetopt_count_line;
          tokenpos = 0 ; /* reset token position */
          strncpy (linebuf, ( (p+1) ? p+1 : ""), LINEBUF_LEN - 1);
        }
    }
  else
    {
      ++gengetopt_count_line;
      tokenpos = 0 ; /* reset token position */
      strncpy (linebuf, yytext+1, LINEBUF_LEN - 1); /* save the next line */
    }
}

void
updateTokenInfo( int pos )
{
  if ( pos >= 0 )
    tokenpos += pos ;
  else
    tokenpos += yyleng ;
}

void buffer(const char *s)
{
  buff << s;
}

char *flush_buffer()
{
  char *ret = strdup(buff.str().c_str());
  buff.str("");
  return ret;
}
