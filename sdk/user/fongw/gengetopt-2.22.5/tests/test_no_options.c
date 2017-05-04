/* test_no_options.c test */

/* test when no option is specified but the default ones */

#include <stdlib.h>
#include <stdio.h>

#include "test_no_options_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{  
  if (test_no_options_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  test_no_options_cmd_parser_free (&args_info);

  return 0;
}
