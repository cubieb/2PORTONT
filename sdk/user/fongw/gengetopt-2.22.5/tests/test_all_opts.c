/* test_all_opts.c test */

/* test all kinds of options */

#include <stdlib.h>
#include <stdio.h>

#include "test_all_opts_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{  
  if (test_all_opts_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  if (args_info.opt_arg_given)
    {
      printf ("specified --opt-arg");
      if (args_info.opt_arg_arg)
        printf (" with argument %s\n", args_info.opt_arg_arg);
      else
        printf ("\n");
    }

  if (args_info.opt_arg_short_given)
    {
      printf ("specified --opt-arg-short");
      if (args_info.opt_arg_short_arg)
        printf (" with argument %s\n", args_info.opt_arg_short_arg);
      else
        printf ("\n");
    }

  if (args_info.hidden_given)
    printf ("specified --hidden: %d\n", args_info.hidden_arg);

  if (args_info.file_save_given && test_all_opts_cmd_parser_file_save (args_info.file_save_arg, &args_info) != EXIT_SUCCESS)
    exit(1) ;

  test_all_opts_cmd_parser_free (&args_info);

  return 0;
}
