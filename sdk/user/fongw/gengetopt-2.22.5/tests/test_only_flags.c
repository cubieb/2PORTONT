/* test_only_flags.c test */

#include <stdlib.h>
#include "test_only_flags_cmd.h"

int
main (int argc, char **argv)
{
  struct gengetopt_args_info args_info;

  /* let's call our cmdline parser */
  if (test_only_flags_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  return 0;
}
