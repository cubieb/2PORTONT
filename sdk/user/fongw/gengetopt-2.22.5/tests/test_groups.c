/* test_groups.c test */

/* test all kinds of options */

#include <stdlib.h>
#include <stdio.h>

#include "test_groups_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{  
  unsigned int i;
  if (test_groups_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  if (args_info.optA_given)
    {
      printf("optA given\n");
      if (args_info.optA_arg)
        printf("optA arg: %s\n", args_info.optA_arg);
    }

  if (args_info.optAmul_given)
    {
      printf("optAmul given %d time(s)\n", args_info.optAmul_given);
      for (i = 0; i < args_info.optAmul_given; ++i)
        printf("optAmul arg: %s\n", 
               (args_info.optAmul_arg[i] ? args_info.optAmul_arg[i] : "(null)"));
    }

  test_groups_cmd_parser_free (&args_info);

  return 0;
}
