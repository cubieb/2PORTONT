/* more_than_once.c test */

/* it will call the command line parser more than once */

#include <stdlib.h>
#include <stdio.h>

#include "more_than_once_cmd.h"

void
print_args (void);

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{  
  /* let's call our cmdline parser a first time */
  if (more_than_once_cmd_parser (argc, argv, &args_info) != 0)
    exit (1);

  print_args ();

  more_than_once_cmd_parser_free (&args_info);

  /* OK, let's call it again! */
  if (more_than_once_cmd_parser (argc, argv, &args_info) != 0)
    exit (1);

  print_args ();

  more_than_once_cmd_parser_free (&args_info);

  return 0;
}

void
print_args (void)
{
  unsigned int i;

  if (args_info.foo_given)
    printf ("foo option: %d\n", args_info.foo_arg);

  if (args_info.bar_given)
    printf ("bar option: %s\n", args_info.bar_arg);

  if (args_info.inputs_num)
    {
      for (i = 0; i < args_info.inputs_num; ++i)
        printf ("arg: %s\n", args_info.inputs [i]);
    }
}
