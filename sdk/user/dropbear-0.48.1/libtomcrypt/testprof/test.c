#include <tomcrypt_test.h>

void run_cmd(int res, int line, char *file, char *cmd)
{
   if (res != CRYPT_OK) {
      fprintf(stderr, "%s (%d)\n%s:%d:%s\n", error_to_string(res), res, file, line, cmd);
      exit(EXIT_FAILURE);
   }
}

/* $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/testprof/test.c,v $ */
/* $Revision: 1.1 $ */
/* $Date: 2006/06/08 13:51:48 $ */
