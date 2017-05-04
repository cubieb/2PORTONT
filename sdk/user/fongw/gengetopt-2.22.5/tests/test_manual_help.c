/* test_manual_help.c test */

/* test manual definition of help and version */

#include <stdlib.h>
#include <stdio.h>

#include "test_manual_help_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{
  int result = 0;
  /* if --help is specified with other options, do not print
   the complete output of help, but only the description of that option */
  short help_printed = 0;

  if (test_manual_help_cmd_parser (argc, argv, &args_info) != 0) {
    result = 1;
    goto end;
  }

  if (args_info.detailed_help_given)
    test_manual_help_cmd_parser_print_detailed_help();

  if (args_info.version_given)
    test_manual_help_cmd_parser_print_version();

  if (args_info.height_given) {
    printf ("specified --height %d\n", args_info.height_arg);
    if (args_info.help_given) {
      printf ("%s\n", args_info.height_help);
      help_printed = 1;
    }
  }

  if (args_info.vavoom_given) {
    printf ("specified --vavoom %s\n", args_info.vavoom_arg);
    if (args_info.help_given) {
      printf ("%s\n", args_info.vavoom_help);
      help_printed = 1;
    }
  }

  if (args_info.help_given && !help_printed)
    test_manual_help_cmd_parser_print_help();

 end:
  test_manual_help_cmd_parser_free (&args_info);
  return result;
}
