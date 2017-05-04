/* test_simple_multiple.cc test */

/* test options that can be given more than once */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "test_simple_multiple_cmd.h"

static struct gengetopt_args_info args_info;

using namespace std;

int
main (int argc, char **argv)
{ 
  unsigned int i = 0;
  int result = 0;
 
  if (test_simple_multiple_cmd_parser (argc, argv, &args_info) != 0) {
    result = 1;
    goto end;
  }

  if (args_info.string_given) {
    for (i = 0; i < args_info.string_given; ++i)
      printf ("passed string: %s\n", args_info.string_arg[i]);
  } else {
    printf ("default value: %s\n", args_info.string_arg[0]);
  }

 end:
  test_simple_multiple_cmd_parser_free (&args_info);

  return result;
}
