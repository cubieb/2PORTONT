/* test options that can be given more than once */

#include <stdlib.h>
#include <stdio.h>

#include "test_multiple_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{ 
  int i = 0;
 
  if (test_multiple_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  for (i = 0; i < args_info.string_given; ++i)
    printf ("passed string: %s\n", args_info.string_arg[i]);

  for (i = 0; i < args_info.int_given; ++i)
    printf ("passed int: %d\n", args_info.int_arg[i]);

  return 0;
}
