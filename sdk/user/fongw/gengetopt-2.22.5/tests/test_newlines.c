/* test_newlines.c test */

/* test when \n are specified (escaped) in options */

#include <stdlib.h>
#include <stdio.h>

#include "test_newlines_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{  
  if (test_newlines_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  test_newlines_cmd_parser_free (&args_info);

  return 0;
}
