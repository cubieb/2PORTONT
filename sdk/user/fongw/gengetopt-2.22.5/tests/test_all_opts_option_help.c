/* test_all_opts_option_help.c test */

/* test option help fields in the presence of text before and after */

#include <stdlib.h>
#include <stdio.h>

#include "test_all_opts_cmd.h"

#define PRINT_OPTION(s) printf("%s: %s\n", # s, args_info.s)

#ifndef FIX_UNUSED
#define FIX_UNUSED(X) (void) (X)
#endif // FIX_UNUSED

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{  
  FIX_UNUSED (argc);
  FIX_UNUSED (argv);
  test_all_opts_cmd_parser_init (&args_info);

  PRINT_OPTION(help_help);
  PRINT_OPTION(full_help_help);
  PRINT_OPTION(version_help);
  PRINT_OPTION(string_help);
  PRINT_OPTION(int_help);
  PRINT_OPTION(short_help);
  PRINT_OPTION(long_help);
  PRINT_OPTION(float_help);
  PRINT_OPTION(double_help);
  PRINT_OPTION(longdouble_help);
  PRINT_OPTION(longlong_help);
  PRINT_OPTION(flag_help);
  PRINT_OPTION(function_help);
  PRINT_OPTION(no_short_help);
  PRINT_OPTION(opt_arg_help);
  PRINT_OPTION(opt_arg_short_help);
  PRINT_OPTION(required_help);
  PRINT_OPTION(hidden_help);
  PRINT_OPTION(dependant_help);
  PRINT_OPTION(very_very_long_option_help);
  PRINT_OPTION(assume_multimeth_virtual_help);
  PRINT_OPTION(file_save_help);


  test_all_opts_cmd_parser_free (&args_info);

  return 0;
}
