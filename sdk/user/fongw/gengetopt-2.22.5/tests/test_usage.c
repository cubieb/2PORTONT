/* test_usage.c test */

/* test correct formatting of usage specification */

#include <stdlib.h>
#include <stdio.h>

#include "test_usage_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{  
  if (test_usage_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  test_usage_cmd_parser_free (&args_info);

  return 0;
}
