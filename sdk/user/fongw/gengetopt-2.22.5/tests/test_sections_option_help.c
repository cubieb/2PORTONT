/* test_sections_option_help.c test */

/* test option help fields in the presence of sections */

#include <stdlib.h>
#include <stdio.h>

#include "test_sections_cmd.h"

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

  test_sections_cmd_parser_init (&args_info);

  PRINT_OPTION (help_help);
  PRINT_OPTION (version_help);
  PRINT_OPTION (string_help);
  PRINT_OPTION (int_help);
  PRINT_OPTION (short_help);
  PRINT_OPTION (long_help);
  PRINT_OPTION (float_help);
  PRINT_OPTION (double_help);
  PRINT_OPTION (longdouble_help);
  PRINT_OPTION (longlong_help);
  PRINT_OPTION (flag_help);
  PRINT_OPTION (function_help);
  PRINT_OPTION (no_short_help);
  PRINT_OPTION (required_help);

  return 0;
}
