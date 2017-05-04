/*
 ============================================================================
 Name        : si_html_conv.h
 Author      : Bougas Paul
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SIHTMLCONV
#define SIHTMLCONV

#define MAX_DOCSIZE 5000
#define MAX_LEVELS 50
#define WHITECHAR(x) ((x) == 9 || (x) == 10 || (x) == 12 || (x) == 13 || (x) == ' ')
#define MAX_TAGS 56
#define MAX_HTML_LEN 5000

struct doc
	  {
		char *doc;
		char * rdpt;
		char * wrpt;
		char * end;
	  };

struct level
	  {
		char levels [MAX_LEVELS];
		unsigned int levelpt;
	  };

struct tags
	  {
		char *tag;
		char (*opnfunc) (struct doc *, struct doc *, struct level*);       //-1 to exit, 0 nothing added, 1 something added
		char (*clfunc) (struct doc *, struct doc *, struct level*);       //-1 to exit, 0 nothing added, 1 something added
	  };


char* text2html ( char *text, char * title);
void ini_parser ();
char * html2text (char *html);

#endif /* SIHTMLCONV */
