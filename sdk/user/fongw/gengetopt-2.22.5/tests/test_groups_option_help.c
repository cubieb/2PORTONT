/* test_groups_option_help.c test */

/* test option help fields in the presence of groups */

#include <stdlib.h>
#include <stdio.h>

#include "test_groups_cmd.h"

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
  test_groups_cmd_parser_init (&args_info);

  PRINT_OPTION(help_help);
  PRINT_OPTION(version_help);
  PRINT_OPTION(opta_help);
  PRINT_OPTION(optA_help);
  PRINT_OPTION(optAmul_help);
  PRINT_OPTION(optb_help);
  PRINT_OPTION(optc_help);
  PRINT_OPTION(optd_help);

  test_groups_cmd_parser_free (&args_info);

  return 0;
}
