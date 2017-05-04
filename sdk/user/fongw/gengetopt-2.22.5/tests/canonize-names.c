/* no_strings.c test */

#include <stdlib.h>
#include "canonize-names-cmd.h"

int
main (int argc, char **argv)
{
  struct gengetopt_args_info args_info;

  /* let's call our cmdline parser */
  /* since it is called with no option we expect an error since the
     option --foo is required */
  if (canonize_names_cmd_parser (argc, argv, &args_info) == 0)
    exit(0) ;

  return 1;
}
