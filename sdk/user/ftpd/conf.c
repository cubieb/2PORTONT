#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "extern.h"

#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>

#include <rtk/options.h>
#include <rtk/mib.h>


#ifndef LINE_MAX
# define LINE_MAX 2048
#endif

#ifdef CONFIG_MULTI_FTPD_ACCOUNT
#define FTP_ACCOUNT_FILE	"/var/passwd.ftp"
static char line[100];
static struct passwd passwd;
#endif

int
display_file (const char *name, int code)
{
  char *cp, line[LINE_MAX];
  FILE *fp = fopen (name, "r");
  if (fp != NULL)
    {
      while (fgets (line, sizeof(line), fp) != NULL)
	{
	  cp = strchr (line, '\n');
	  if (cp != NULL)
	    *cp = '\0';
	  lreply (code, "%s", line);
	}
      (void) fflush (stdout);
      (void) fclose (fp);
      return 0;
    }
  return errno;
}

/* Check if a user is in the file PATH_FTPUSERS
   return 1 if yes 0 otherwise.  */
int
checkuser (const char *filename, const char *name)
{
  FILE *fp;
  int found = 0;
#ifndef __uClinux__
  char *p, line[BUFSIZ];

  fp = fopen (filename, "r");
  if (fp != NULL)
    {
      while (fgets (line, sizeof(line), fp) != NULL)
	{
	  if (line[0] == '#')
	    continue;
	  p = strchr (line, '\n');
	  if (p != NULL)
	    {
	      *p = '\0';
	      if (strcmp (line, name) == 0)
		{
		  found = 1;
		  break;
		}
	    }
	}
      (void) fclose (fp);
    }
#endif
  return (found);
}

#ifdef CONFIG_MULTI_FTPD_ACCOUNT
static char *pwskip(char *p)
{
	while( *p && *p != ':' )
		++p;
	if( *p ) *p++ = 0;
	return(p);
}

/*
 * rewrite getpwnam
 */
struct passwd * getpwnam(const char *name)
{
	FILE *fp;
	char *p;
	int found = 0;
	
	fp = fopen(FTP_ACCOUNT_FILE, "r");
	if (!fp)
		return NULL;

	while (1)
	{
		memset(line, 0, sizeof(line));
		if ( (p = fgets(line, sizeof(line), fp)) == NULL )
			break;
		
		passwd.pw_name = p;
		p = pwskip(p);

		if (strcmp(passwd.pw_name, name))
			continue;
		
		passwd.pw_passwd = p;
		p = pwskip(p);
		passwd.pw_uid = atoi(p);
		p = pwskip(p);
		passwd.pw_gid = atoi(p);
		p = pwskip(p);
		passwd.pw_gecos = p;
		p = pwskip(p);
		passwd.pw_dir = p;
		p = pwskip(p);
		passwd.pw_shell = p;
		
		while(*p && *p != '\n') p++;
		*p = '\0';

		found = 1;
		break;
	}
	fclose(fp);
	
	if (found)
		return (&passwd);
	else
		return NULL;
}
#endif

