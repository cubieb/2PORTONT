/*
 ============================================================================
 Name        : si_html_conv.c
 Author      : Bougas Paul
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <si_print_api.h>

#include "si_html_conv.h"

static void opnenablelv (struct level* levels);
static void opndisablelv (struct level* levels);
static void opndisablelv (struct level* levels);
static void clsrestore (struct level* levels);
static char parsecomment (struct doc *indoc, struct doc *outdoc, struct level* levels);
static char parsetag (struct doc *indoc);
static char opparagraphcall (struct doc *indoc, struct doc *outdoc, struct level* levels);
static char paragraphmark (struct doc *indoc, struct doc *outdoc, struct level* levels);
static char nothingToAdd(struct doc *indoc, struct doc *outdoc, struct level* levels);
static char clcall (struct doc *indoc, struct doc *outdoc, struct level* levels);
static char nothingToAdd(struct doc *indoc, struct doc *outdoc, struct level* levels);
static char clcall (struct doc *indoc, struct doc *outdoc, struct level* levels);
static char opdiscall (struct doc *indoc, struct doc *outdoc, struct level* levels);
static char opencall (struct doc *indoc, struct doc *outdoc, struct level* levels);

struct tags htmlTags[MAX_TAGS]=
{
//  {"!", nothingToAdd},
//  {"!DOCTYPE", nothingToAdd},
  {"!--", parsecomment, nothingToAdd},                    //works for DOCTYPE as well
  {"address", nothingToAdd, nothingToAdd},
  {"acronym", nothingToAdd, nothingToAdd},
  {"abbr", nothingToAdd, nothingToAdd},
  {"a", nothingToAdd, nothingToAdd},
  //{"applet", nothingToAdd, nothingToAdd},
  //{"area", nothingToAdd, nothingToAdd},
  {"b", nothingToAdd, nothingToAdd},
  //{"base", nothingToAdd, nothingToAdd},
  //{"basefont", nothingToAdd, nothingToAdd},
  //{"bdo", nothingToAdd, nothingToAdd},
  {"big", nothingToAdd, nothingToAdd},
  {"blockquote", nothingToAdd, nothingToAdd},
  {"body", opparagraphcall,clcall},
  {"br", paragraphmark, nothingToAdd},
  //{"button", nothingToAdd, nothingToAdd},
  {"caption", nothingToAdd, nothingToAdd},
  {"center", nothingToAdd, nothingToAdd},
  {"cite", nothingToAdd, nothingToAdd},
  {"code", nothingToAdd, nothingToAdd},
  //{"col", nothingToAdd, nothingToAdd},
  //{"colgroup", nothingToAdd, nothingToAdd},
  //{"dd", nothingToAdd, nothingToAdd},
  //{"del", nothingToAdd, nothingToAdd},
  {"dfn", nothingToAdd, nothingToAdd},
  //{"dir", nothingToAdd, nothingToAdd},
  {"div", paragraphmark, nothingToAdd},
  //{"dl", nothingToAdd, nothingToAdd},
  //{"dt", nothingToAdd, nothingToAdd},
  {"em", nothingToAdd, nothingToAdd},
  //{"fieldset", nothingToAdd, nothingToAdd},
  //{"font", nothingToAdd, nothingToAdd},
  //{"form", nothingToAdd, nothingToAdd},
  //{"frame", nothingToAdd, nothingToAdd},
  //{"frameset", nothingToAdd, nothingToAdd},
  //{"head", nothingToAdd, nothingToAdd},
  {"h1", paragraphmark, nothingToAdd},
  {"h2", paragraphmark, nothingToAdd},
  {"h3", paragraphmark, nothingToAdd},
  {"h4", paragraphmark, nothingToAdd},
  {"h5", paragraphmark, nothingToAdd},
  {"h6", paragraphmark, nothingToAdd},
  {"hr", nothingToAdd, nothingToAdd},
  {"html", nothingToAdd, nothingToAdd},
  {"i", nothingToAdd, nothingToAdd},
  //{"iframe", nothingToAdd, nothingToAdd},
  //{"img", nothingToAdd, nothingToAdd},
  //{"input", nothingToAdd, nothingToAdd},
  {"ins", nothingToAdd, nothingToAdd},
  {"kbd", nothingToAdd, nothingToAdd},
  //{"label", nothingToAdd, nothingToAdd},
  {"legend", nothingToAdd, nothingToAdd},
  {"li", paragraphmark, nothingToAdd},
  //{"link", nothingToAdd, nothingToAdd},
  //{"map", nothingToAdd, nothingToAdd},
  {"menu", nothingToAdd, nothingToAdd},
  //{"meta", nothingToAdd, nothingToAdd},
  {"noframes", nothingToAdd, nothingToAdd},
  {"noscript", nothingToAdd, nothingToAdd},
  //{"object", nothingToAdd, nothingToAdd},
  {"ol", nothingToAdd, nothingToAdd},
  //{"optgroup", nothingToAdd, nothingToAdd},
  {"option", opdiscall, clcall},
  {"p", opparagraphcall,clcall},
  //{"param", nothingToAdd, nothingToAdd},
  {"pre", nothingToAdd, nothingToAdd},
  {"q", nothingToAdd, nothingToAdd},
  {"s", nothingToAdd, nothingToAdd},
  {"samp", nothingToAdd, nothingToAdd},
  {"script", opdiscall, clcall},
  //{"select", nothingToAdd, nothingToAdd},
  {"small", nothingToAdd, nothingToAdd},
  //{"span", nothingToAdd, nothingToAdd},
  {"strike", nothingToAdd, nothingToAdd},
  {"strong", nothingToAdd, nothingToAdd},
  {"style", opdiscall, clcall},
  {"sub", nothingToAdd, nothingToAdd},
  {"span", nothingToAdd, nothingToAdd},
  {"sup", nothingToAdd, nothingToAdd},
  {"table", nothingToAdd, nothingToAdd},
  //{"tbody", nothingToAdd, nothingToAdd},
  {"td", nothingToAdd, nothingToAdd},
  //{"textarea", nothingToAdd, nothingToAdd},
  //{"tfoot", nothingToAdd, nothingToAdd},
  {"th", nothingToAdd, nothingToAdd},
  {"thead", nothingToAdd, nothingToAdd},
  {"title", opparagraphcall,clcall},
  {"u", nothingToAdd, nothingToAdd},
  {"ul", paragraphmark, nothingToAdd},
  {"var", nothingToAdd, nothingToAdd}
};


static void opnenablelv (struct level* levels)
{
  if (levels->levelpt<MAX_LEVELS)
    {
      levels->levelpt++;
      levels->levels[levels->levelpt]=1;
    }
}
static void opndisablelv (struct level* levels)
{
  if (levels->levelpt<MAX_LEVELS)
    {
      levels->levelpt++;
      levels->levels[levels->levelpt]=0;
    }
}

static void clsrestore (struct level* levels)
{
  if (levels->levelpt>0)
    {
      levels->levels[levels->levelpt]=0;
      levels->levelpt--;
    }
}


static char parsecomment (struct doc *indoc, struct doc *outdoc, struct level* levels)
{
  char found=0;

  while ((!((*indoc->rdpt=='>')&&(found>=2)))&&(indoc->rdpt<indoc->end))
    {
      if ((*(indoc->rdpt))=='-')
        found++;
      else
        found=0;
      indoc->rdpt++;
    };
  if (indoc->rdpt<indoc->end) indoc->rdpt++;
  return (found);
}

static char parsetag (struct doc *indoc)
{
  char found=0;
  while ((*(indoc->rdpt)!='>')&&(indoc->rdpt<indoc->end))
    {
      if ((*(indoc->rdpt))=='/')
        found=1;
      else
        found=0;
      indoc->rdpt++;
    };
  if (indoc->rdpt<indoc->end) indoc->rdpt++;
  return (found);
}

static char opparagraphcall (struct doc *indoc, struct doc *outdoc, struct level* levels)
{
  if (parsetag (indoc)==0)
    {
        opnenablelv (levels);
        //*outdoc->wrpt='\r';
        //outdoc->wrpt++;
        *outdoc->wrpt='\n';
        outdoc->wrpt++;
    }
  return 0;

}

static char paragraphmark (struct doc *indoc, struct doc *outdoc, struct level* levels)
{
  parsetag (indoc);
  //*outdoc->wrpt='\r';
  //outdoc->wrpt++;
  outdoc->wrpt--;
  if (*outdoc->wrpt!='\n')
    {
      outdoc->wrpt++;
      *outdoc->wrpt='\n';
      outdoc->wrpt++;
    }
  else
    outdoc->wrpt++;

  return (0);
}

static char opencall (struct doc *indoc, struct doc *outdoc, struct level* levels)
{
  if (parsetag (indoc)==0)
     {
         opnenablelv (levels);
     }
  return (0);
}

static char opdiscall (struct doc *indoc, struct doc *outdoc, struct level* levels)
{
  if (parsetag (indoc)==0)
     {
       opndisablelv (levels);
     }
  return 0;
}

static char clcall (struct doc *indoc, struct doc *outdoc, struct level* levels)
{
  parsetag (indoc);
  clsrestore (levels);
  return 0;

}

static char nothingToAdd(struct doc *indoc, struct doc *outdoc, struct level* levels)
{
  parsetag (indoc);
  return 0;
};


const char *head1= " <!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"> \
                    <HTML> \
                    <HEAD>";

const char *head2= " </TITLE>"
                  " </HEAD>"
                  "<BODY>"
                  "<p>";
const char *head3=  "</p>"
                    "</BODY>"
                    "</HTML>\r\n";

char* text2html ( char *text, char * title)
{
  char *retext, *bodydata, *textdata;

  retext=malloc (MAX_HTML_LEN);
  unsigned int pt=0;
  unsigned int addlen=0;

  addlen=strlen ("<TITLE>");
  memcpy (&retext[pt], "<TITLE>", addlen);
  pt+=addlen;

 if (title !=NULL)
 {
   addlen=strlen (title);
   memcpy (&retext[pt], title, addlen);
   pt+=addlen;
}
 else
   {
     addlen=strlen ("Instant Message");
     memcpy (&retext[pt], "Instant Message", addlen);
     pt+=addlen;
   }

 addlen=strlen (head2);
 memcpy (&retext[pt], head2, addlen);
 pt+=addlen;


 bodydata=&retext[pt];
 textdata=text;

while (*textdata!=0)
  {
                  switch (*textdata)
                  {
                    case '\n':  *bodydata=*textdata;
                                addlen=strlen ("</p>\r\n<p>");
                                memcpy (bodydata, "</p>\r\n<p>", addlen);
                                bodydata+=addlen;
                                pt+=addlen;
                                break;
                    case '\r': break;
                    default:
                                *bodydata=*textdata;
                                bodydata++;
                                pt++;
                                break;
                  }
    textdata++;
  }
 addlen=strlen (head3);
 memcpy (&retext[pt], head3, addlen);
 pt+=addlen;
 retext[pt]=0;
return (retext);

}


void ini_parser ()
{


};
static int findTaglen (struct doc *indoc)
{
  char *stpt=indoc->rdpt;

  //check for comment
  if (strncasecmp (indoc->rdpt,"!--", 3)==0)
      return (3);

  stpt=indoc->rdpt;

  while ((!(WHITECHAR(*stpt)))&& *stpt!='/' && *stpt!='>' && stpt<indoc->end)
  {
    stpt++;
  }

  return (stpt-indoc->rdpt);

}
static void processtag( struct doc *indoc, struct doc* outdoc, struct level *levels)
{
  unsigned int tagpt=0;
  unsigned char closing_tag=0, found=0;
  int taglen;


  indoc->rdpt++;       //ignore open tag

  if (*(indoc->rdpt)=='/')
    {
      closing_tag=1;
      indoc->rdpt++;
    }
  taglen=findTaglen (indoc);

  while ((tagpt<MAX_TAGS)&&(found==0))
  {

    if (taglen==strlen (htmlTags[tagpt].tag))
      {
        if (strncasecmp (indoc->rdpt,htmlTags[tagpt].tag, taglen)==0)
          {
            if (closing_tag)
              htmlTags[tagpt].clfunc(indoc, outdoc, levels);
            else
              htmlTags[tagpt].opnfunc(indoc, outdoc, levels);
            found=1;
          }
        else
          tagpt++;
      }
     else
       tagpt++;
  };

  if (found==0)
    nothingToAdd(indoc, outdoc, levels);

};

static int readspecialchar( struct doc *indoc, struct doc* outdoc, struct level *levels)
{
//TODO do the special char match
  while (!(WHITECHAR(*(indoc->rdpt)))&& (indoc->rdpt<indoc->end) && (*(indoc->rdpt)!=';'))
    indoc->rdpt++;

  if (indoc->rdpt<indoc->end) indoc->rdpt++;

  return ('*');

};

static void parse_html (struct doc *indoc, struct doc* outdoc)
{
  struct level levels;
  char trim=0;

  indoc->rdpt=indoc->doc;
  outdoc->wrpt=outdoc->wrpt;

  unsigned int i;
  levels.levelpt=0;

  for (i=0;i<MAX_LEVELS;i++)
    levels.levels[i]=0;

  levels.levels[0]=1;
  while ( indoc->rdpt<indoc->end)
  {
     switch (*indoc->rdpt)
     {
       case '<':processtag(indoc, outdoc, &levels);
                break;
       case 9:
       case 10:
       case 12:
       case 13:
       case ' ':
                 indoc->rdpt++;
                 //if ((trim==0) && (levels.levels[levels.levelpt]!=0))
                 if ((levels.levels[levels.levelpt]!=0))
                   {
                     *outdoc->wrpt=' ';
                     outdoc->wrpt++;
                     trim=1;
                   }


                 break;
       case '&': readspecialchar(indoc, outdoc, &levels);
                 break;
       default:
               if (levels.levels[levels.levelpt]!=0)
                 {
                   *outdoc->wrpt=*indoc->rdpt;
                   outdoc->wrpt++;
                   trim=0;
                 }
               indoc->rdpt++;
               break;

     };
    }
  outdoc->wrpt=0;
}

char * html2text (char *html)
{
  struct doc indoc, outdoc;
  char *mesbuf;
		
  mesbuf=malloc(MAX_DOCSIZE);
  if (mesbuf)
	memset(mesbuf, 0, MAX_DOCSIZE);
  else {si_print(PRINT_LEVEL_ERR, "UNABLE TO ALLOC MEM\n"); return NULL ;}

  indoc.doc=html;
  indoc.rdpt=html;
  indoc.end=html+strlen(html);

  outdoc.doc=mesbuf;
  outdoc.end=&mesbuf[MAX_DOCSIZE-1];
  outdoc.wrpt=mesbuf;
  outdoc.rdpt=mesbuf;

  parse_html (&indoc, &outdoc);

  return (mesbuf);
}

/*------------------------------------------------------
 * Usage Example

int main(void)
{
  char *html;

  html=malloc (MAX_DOCSIZE);

  FILE *f, *fout;
  ini_parser ();

  f=fopen ("document3.html", "r");
  unsigned int i=0;
  char ch;

  while ((ch=fgetc (f))!=EOF)
    {
      html[i]=ch;
      i++;
    }
  fclose (f);
  //--

  fout=fopen ("out3.txt","w");
  char *pt, *stpt;
  pt= html2text(html);
  stpt=pt;
  while (pt!=0)
    {
        fputc(*pt,fout);
        putchar (*pt);
        pt++;
    }
  fclose (fout);

  fout=fopen ("out3.html","w");
  fputs (text2html ( stpt, NULL),fout);
  fclose (fout);
  free (stpt);
  free(pt);

  return EXIT_SUCCESS;
}
*/
