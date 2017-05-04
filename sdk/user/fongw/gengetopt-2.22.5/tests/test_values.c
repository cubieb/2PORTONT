/* test_values.c test */

/* test values options */

#include <stdlib.h>
#include <stdio.h>

#include "test_values_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{
  unsigned int i;
  int result = 0;

  if (test_values_cmd_parser (argc, argv, &args_info) != 0) {
    result = 1;
    goto end;
  }

  if (args_info.string_values_given)
    {
      printf ("specified --string-values with argument %s\n", args_info.string_values_arg);
      printf ("specified --string-values with (original) argument %s\n", args_info.string_values_orig);
    }

  if (!args_info.string_values_def_given)
    printf ("(un)");
  printf ("specified --string-values-def with argument %s\n", args_info.string_values_def_arg);

  if (!args_info.string_values_def_argopt_given)
    printf ("(un)");
  printf ("specified --string-values-def-argopt with argument %s\n",
          args_info.string_values_def_argopt_arg);

  if (args_info.string_values_no_short_given)
    {
      printf ("specified --string-values-no-short with argument %s\n",
              args_info.string_values_no_short_arg);
    }

  for (i = 0; i < args_info.multistring_values_given; ++i)
    {
      printf ("specified --multistring-values with argument %s\n",
              args_info.multistring_values_arg[i]);
      printf ("specified --multistring-values with (original) argument %s\n",
              args_info.multistring_values_orig[i]);
    }

  for (i = 0; i < args_info.multistring_values_def_given; ++i)
    {
      printf ("specified --multistring-values-def with argument %s\n",
              args_info.multistring_values_def_arg[i]);
      printf
        ("specified --multistring-values-def with (original) argument %s\n",
         (args_info.multistring_values_def_orig[i] ?
          args_info.multistring_values_def_orig[i] : "(null)"));
    }

  for (i = 0; i < args_info.multistring_values_no_short_given; ++i)
    printf ("specified --multistring-values-no-short with argument %s\n",
            args_info.multistring_values_no_short_arg[i]);

  if (args_info.int_values_given)
    {
      printf ("specified --int-values with argument %d\n",
              args_info.int_values_arg);
      printf ("specified --int-values with (original) argument %s\n",
              args_info.int_values_orig);
    }

  for (i = 0; i < args_info.int_values_m_given; ++i) {
    printf("specified --int-values-m with argument %d\n",
           args_info.int_values_m_arg[i]);
    printf ("specified --int-values-m with (original) argument %s\n",
              args_info.int_values_m_orig[i]);
  }

  if (!args_info.int_values_def_given)
    printf ("(un)");
  printf ("specified --int-values-def with argument %d\n",
          args_info.int_values_def_arg);

  if (args_info.values_first_enums_given)
    printf("specified --values-first-enums with argument %d (%s)\n",
           args_info.values_first_enums_arg,
           test_values_cmd_parser_values_first_enums_values
           [args_info.values_first_enums_arg]);

  if (args_info.values_first_enums_plus_given)
      printf("specified --values-first-enums-plus with argument %d (%s)\n",
             args_info.values_first_enums_plus_arg,
             test_values_cmd_parser_values_first_enums_plus_values
             [args_info.values_first_enums_plus_arg]);

  if (!args_info.values_second_enums_given)
    printf ("(un)");
  printf("specified --values-second-enums with argument %d (%s)\n",
         args_info.values_second_enums_arg,
         test_values_cmd_parser_values_second_enums_values
         [args_info.values_second_enums_arg]);

  for (i = 0; i < args_info.values_multiple_enums_given; ++i)
    {
      printf ("specified --values-multiple-enums with argument %d\n",
              args_info.values_multiple_enums_arg[i]);
      printf
        ("specified --values-multiple-enums with (original) argument %s\n",
         (args_info.values_multiple_enums_orig[i] ?
          args_info.values_multiple_enums_orig[i] : "(null)"));
    }

  if (args_info.values_unspecified_enums_arg == values_unspecified_enums__NULL)
    printf("--values-unspecified-enums: %d\n", args_info.values_unspecified_enums_arg);

  if (args_info.file_save_given) {
    if (test_values_cmd_parser_file_save (args_info.file_save_arg, &args_info) == EXIT_FAILURE)
      result = 1;
    else
      printf ("saved configuration file %s\n", args_info.file_save_arg);
  }

 end:
  test_values_cmd_parser_free (&args_info);

  return result;
}
