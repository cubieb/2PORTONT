/* test_redef_help.c test */

/* test redefinition of help and version */

#include <stdlib.h>
#include <stdio.h>

#include "test_redef_help_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{
  int result = 0;

  if (test_redef_help_cmd_parser (argc, argv, &args_info) != 0) {
    result = 1;
    goto end;
  }

  if (args_info.height_given)
    {
      printf ("specified --height\n");
    }

  if (args_info.vavoom_given)
    {
      printf ("specified -V\n");
    }

  test_redef_help_cmd_parser_free (&args_info);

 end:
  return result;
}
