#include <stdio.h>
#include <stdlib.h>

#include "test_main_cmdline_cmd.h"
#include "test_first_cmdline_cmd.h"
#include "test_second_cmdline_cmd.h"

int
main(int argc, char **argv)
{
  struct gengetopt_args_info main_args_info;
  struct test_first_cmdline_cmd_struct first_args_info;
  struct test_second_cmdline_cmd_struct second_args_info;
  int exit_code = 0;
  unsigned int i, j;

  if (test_main_cmdline_cmd_parser (argc, argv, &main_args_info) != 0) {
    exit_code = 1;
    return exit_code;
  }

  for (j = 0; j < main_args_info.second_cmd_given; ++j) {
    printf("second cmdline: %s\n", main_args_info.second_cmd_arg[j]);
    if (test_second_cmdline_cmd_parser_string 
	(main_args_info.second_cmd_arg[j], &second_args_info, argv[0]) == 0) {
      if (second_args_info.option_a_given)
	printf("  --option-a: %s\n", second_args_info.option_a_arg);
      if (second_args_info.option_b_given)
	printf("  --option-b: %s\n", second_args_info.option_b_arg);
      for (i = 0; i < second_args_info.my_multi_given; ++i)
	printf("  --my-multi: %s\n", second_args_info.my_multi_arg[i]);

      test_second_cmdline_cmd_parser_free (&second_args_info);
    }
  }

  for (j = 0; j < main_args_info.first_cmd_given; ++j) {
    printf("first cmdline: %s\n", main_args_info.first_cmd_arg[j]);
    if (test_first_cmdline_cmd_parser_string 
	(main_args_info.first_cmd_arg[j], &first_args_info, argv[0]) == 0) {
      if (first_args_info.option_a_given)
	printf("  --option-a: %d\n", first_args_info.option_a_arg);
      for (i = 0; i < first_args_info.multi_given; ++i)
	printf("  --multi: %s\n", first_args_info.multi_arg[i]);

      test_first_cmdline_cmd_parser_free (&first_args_info);
    }
  }

  test_main_cmdline_cmd_parser_free (&main_args_info);

  return exit_code;
}
