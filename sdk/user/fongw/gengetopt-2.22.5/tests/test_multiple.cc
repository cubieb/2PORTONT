/* test_multiple.c test */

/* test options that can be given more than once */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "test_multiple_cmd.h"

static struct gengetopt_args_info args_info;

using namespace std;

int
main (int argc, char **argv)
{ 
  unsigned int i = 0;
  int result = 0;
 
  if (test_multiple_cmd_parser (argc, argv, &args_info) != 0) {
    result = 1;
    goto end;
  }

  for (i = 0; i < args_info.string_given; ++i)
    printf ("passed string: %s\n", args_info.string_arg[i]);

  for (i = 0; i < args_info.int_given; ++i)
    printf ("passed int: %d\n", args_info.int_arg[i]);

  if (args_info.float_given)
    {
      for (i = 0; i < args_info.float_given; ++i)
        printf ("passed float: %f\n", args_info.float_arg[i]);
    }
  else
    if (args_info.float_arg)
      printf ("default float: %f\n", args_info.float_arg[0]);

  for (i = 0; i < args_info.no_short_opt_given; ++i)
    printf ("passed no-short-opt: %s\n", args_info.no_short_opt_arg[i]);

  printf ("noarg given %d times\n", args_info.noarg_given);
  printf ("noarg_noshort given %d times\n", args_info.noarg_noshort_given);

  printf ("optarg_given %d times\n", args_info.optarg_given);
  for (i = 0; i < args_info.optarg_given; ++i)
    printf ("optarg argument: %s\n", 
            (args_info.optarg_arg[i] ? args_info.optarg_arg[i] : "no arg given"));

  printf ("longlong_given %d times\n", args_info.longlong_given);
  for (i = 0; i < args_info.longlong_given; ++i)
    cout << "longlong argument: " << 
      args_info.longlong_arg[i] << endl;

  printf ("optarg_noshort_given %d times\n", args_info.optarg_noshort_given);
  for (i = 0; i < args_info.optarg_noshort_given; ++i)
    printf ("optarg_noshort argument: %s\n", 
            (args_info.optarg_noshort_arg[i] ? args_info.optarg_noshort_arg[i] : "no arg given"));

  if (args_info.file_save_given && test_multiple_cmd_parser_file_save (args_info.file_save_arg, &args_info) != EXIT_SUCCESS) {
    result = 1;
    goto end;
  }

 end:
  test_multiple_cmd_parser_free (&args_info);

  return result;
}
