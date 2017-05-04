/* test_conf_parser.c test */

/* 
   test all kinds of options and the conf file parser.
   Moreover it tries to use a previously saved configuration file.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test_conf_parser_cmd.h"

static struct my_args_info args_info;

char conf_file[256];

int
main (int argc, char **argv)
{  
  unsigned int i;
  int result = 0;

  /* initialized but no check required */
  if (test_conf_parser_cmd_parser2 (argc, argv, &args_info, 0, 1, 0) != 0) {
    result = 1;
    goto stop;
  }

  conf_file[0] = 0;
  strcpy(conf_file, args_info.conf_file_arg);

  /* override cmd options, but do not initialize args_info, check for
     required options */
  if (test_conf_parser_cmd_parser_configfile
      (args_info.conf_file_arg, &args_info, 1, 0, 1) != 0) 
    {
      result = 1;
      goto stop;
    }
  else
    {
      printf ("read configuration file: %s\n", conf_file);
    }

  printf ("value of required: %s\n", args_info.required_arg);
  printf ("value of string: %s\n", args_info.string_arg);
  printf ("value of no-short_given: %d\n", args_info.no_short_given);
  printf ("value of int: %d\n", args_info.int_arg);
  printf ("value of float: %f\n", args_info.float_arg);

  printf ("value of multi-string_given: %d\n", args_info.multi_string_given);
  for (i = 0; i < args_info.multi_string_given; ++i)
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

  if (args_info.inputs_num) {
    printf ("options without names:\n");
    for (i = 0; i < args_info.inputs_num; ++i)
      printf ("  %s\n", args_info.inputs[i]);
  }

  if (args_info.file_save_given) {
    if (test_conf_parser_cmd_parser_file_save (args_info.file_save_arg, &args_info) == EXIT_FAILURE)
      result = 1;
  }

 stop:
  test_conf_parser_cmd_parser_free (&args_info);

  return result;
}
