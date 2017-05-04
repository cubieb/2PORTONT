/* no_prog_name.c test */

/* 
   Checks that the program name is not in the returned command
   line arguments.

   Simulates the program name in the arguments by copying a pointer
   to argv[0]
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test_all_opts_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{
  unsigned int i = 0;
  char *my_argv[6];
  int result = 0;

  my_argv[0] = "prog name";
  my_argv[1] = my_argv[0];
  my_argv[2] = "-r";
  my_argv[3] = "foo";
  my_argv[4] = "unamed";
  my_argv[5] = 0;

  if (test_all_opts_cmd_parser (5, my_argv, &args_info) != 0)
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

  if (args_info.required_given)
    printf ("specified --required: %s\n", args_info.required_arg);

  for (i = 0; i < args_info.inputs_num; i++) {
    printf ("additional: %s\n", args_info.inputs[i]);
    if (strcmp(args_info.inputs[i], "prog name") == 0)
      result = 1;
    /* the test fails if the "prog name" is in the remaining
       arguments */
  }

  if (args_info.file_save_given && test_all_opts_cmd_parser_file_save (args_info.file_save_arg, &args_info) != EXIT_SUCCESS)
    exit(1) ;

  test_all_opts_cmd_parser_free (&args_info);

  return result;
}
