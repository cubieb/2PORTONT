/* test_conf_parser.c test */

/* test all kinds of options and the conf file parser */

#include <stdlib.h>
#include <stdio.h>

#include "test_conf_parser_cmd.h"

static struct my_args_info args_info;

int
main (int argc, char **argv)
{  
  unsigned int i;
  int result = 0;

  struct test_conf_parser_cmd_parser_params *params;
  
  /* initialize the parameters structure */
  params = test_conf_parser_cmd_parser_params_create();

  /* call the command line parser */
  if (test_conf_parser_cmd_parser (argc, argv, &args_info) != 0) {
    result = 1;
    goto stop;
  }

  /* 
     override command line options,
     but do not initialize args_info, check for required options.
     NOTICE: we must NOT skip the 0 assignment to initialize,
     since its default value is 1 and override defaults to 0
     while check_required is already set to its default value, 1
  */
  params->initialize = 0;
  params->override = 1;

  /* call the config file parser */
  if (test_conf_parser_cmd_parser_config_file
      (args_info.conf_file_arg, &args_info, params) != 0) 
    {
      result = 1;
      goto stop;
    }

  printf ("value of required: %s\n", args_info.required_arg);
  printf ("value of string: %s\n", args_info.string_arg);
  printf ("value of no-short_given: %d\n", args_info.no_short_given);
  printf ("value of int: %d\n", args_info.int_arg);
  printf ("value of float: %f\n", args_info.float_arg);

  printf ("value of multi-string_given: %d\n", args_info.multi_string_given);
  for (i = 0; i < args_info.multi_string_given; i++)
    printf ("  value of multi-string: %s\n", args_info.multi_string_arg [i]);

  printf ("value of multi-string-def_given: %d\n",
          args_info.multi_string_def_given);
  for (i = 0; i < args_info.multi_string_def_given; ++i)
    printf ("  value of multi-string-def: %s\n",
            args_info.multi_string_def_arg [i]);
  if (!args_info.multi_string_def_given && args_info.multi_string_def_arg [0])
    printf ("default value of multi-string-def: %s\n",
            args_info.multi_string_def_arg [0]);

  printf ("value of opta: %s\n", args_info.opta_arg);

  printf ("noarg given %d times\n", args_info.noarg_given);
  printf ("noarg_noshort given %d times\n", args_info.noarg_noshort_given);

  printf ("opt-arg given: %d\n", args_info.opt_arg_given);
  printf ("opt-arg value: %s\n", (args_info.opt_arg_arg ? args_info.opt_arg_arg : "not given"));

  if (args_info.file_save_given) {
    if (test_conf_parser_cmd_parser_file_save (args_info.file_save_arg, &args_info) == EXIT_FAILURE)
      result = 1;
    else
      printf ("saved configuration file %s\n", args_info.file_save_arg);
  }

 stop:
  /* deallocate structures */
  test_conf_parser_cmd_parser_free (&args_info);
  free (params);

  return result;
}
