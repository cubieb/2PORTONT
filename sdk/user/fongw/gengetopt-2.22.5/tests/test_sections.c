/* test_sections.c test */

/* test sections printed by --help */

#include <stdlib.h>
#include <stdio.h>

#include "test_sections_cmd.h"

static struct gengetopt_args_info args_info;

int
main (int argc, char **argv)
{ 
  if (test_sections_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  test_sections_cmd_parser_free (&args_info);

  return 0;
}
