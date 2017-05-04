/* no_unamed.c test */

#include <stdlib.h>
#include "no_unamed_cmd.h"

int
main (int argc, char **argv)
{
  struct gengetopt_args_info args_info;

  /* let's call our cmdline parser */
  if (no_unamed_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  return 0;
}
