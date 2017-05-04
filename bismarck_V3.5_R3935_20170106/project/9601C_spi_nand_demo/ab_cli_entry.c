#include <soc.h>
#include <util.h>
#include <cli/cli_util.h>
#include <cli/cli_parser.h>
#include <init_define.h>

#ifndef SECTION_CLI_UTIL
    #define SECTION_CLI_UTIL
#endif


SECTION_CLI_UTIL void
cli_entry(void) {
    // check if enter cli
    if (tstc()==0) return;
    if (getc()!='.') return;

    // cli start
    puts("Preloader cli start\n]");    
    //util_init();
    
    // cli main-loop
    cli_main_loop();

}
REG_INIT_FUNC(cli_entry, 29);

// for cli command: call
    //ex: cli_add_tail(init_ddr, call, (void*)init_ddr);
    //    cli_add_help(call_init_ddr, "call init_ddr");
