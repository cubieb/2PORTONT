#include <soc.h>
#include <cli/cli_access.h>
#include <soc_exam_util/ecc_bch_test.h>

#ifndef SECTION_CLI_UTIL
    #define SECTION_CLI_UTIL
#endif


SECTION_CLI_UTIL cli_cmd_ret_t
cli_std_ecc_controller_test(const void *user, u32_t argc, const char *argv[]) {
    if (argc<1) return CCR_INCOMPLETE_CMD;
    ecc_controller_test();
    return CCR_OK;
}

cli_top_node(ecc, cli_std_ecc_controller_test);
cli_add_help(ecc, "ecc");


