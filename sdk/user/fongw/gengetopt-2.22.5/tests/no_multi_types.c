/* 
   no_multi_types.c test 
   contains multiple options none of which has type
*/

#include <stdlib.h>
#include "no_multi_types_cmd.h"

int
main (int argc, char **argv)
{
  struct gengetopt_args_info args_info;

  /* let's call our cmdline parser */
  if (no_multi_types_cmd_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  return 0;
}
