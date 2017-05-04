/* no_optgiven.c test */

#include <stdlib.h>
#include <stdio.h>
#include "no_optgiven_cmd.h"

int
main (int argc, char **argv)
{
  struct gengetopt_args_info args_info;

  /* let's call our cmdline parser */
  /* since it is called with no option we expect an error since the
     option --foo is required, however we check for this explicitly
     by calling the _required function */
  if (no_optgiven_cmd_parser2 (argc, argv, &args_info, 0, 1, 0) == 0)
    {
      fprintf(stderr, "the no_optgiven_cmd_parser2 returned successfully\n");
    }

  if (no_optgiven_cmd_parser_required (&args_info, argv[0]) == 0)
    exit(0);

  return 1;
}
