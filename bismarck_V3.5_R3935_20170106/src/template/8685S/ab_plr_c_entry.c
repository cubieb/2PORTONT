#include <init_define.h>
#include <soc.h>
#include "init_level.h"

void traversal_init_table(void) {
    // run init functions by merge lplr's and plr's init functions
    run_init_level(VZERO, VZERO, _soc_header.init_func_list, _soc_header.end_of_init_func_list);
    while(1) ; // should never return
}

