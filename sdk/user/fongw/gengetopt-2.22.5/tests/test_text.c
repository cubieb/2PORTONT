/* test_text.c test */

/* test when text is specified in options */

#include <stdlib.h>
#include <stdio.h>

#include "test_text_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{  
  if (test_text_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  test_text_cmd_parser_free (&args_info);

  return 0;
}
