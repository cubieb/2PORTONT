#include <soc.h>
#include <soc_exam_util/bus_traffic_generator.h>
#include <cli/cli_access.h>

#ifndef SECTION_CLI_UTIL
    #define SECTION_CLI_UTIL
#endif

#define CLI_NO_DATA_BASE       (0xFFFFFFFF)
#define DFT_CLI_BURST_LEN      (1)
#define DFT_CLI_PACKAGE_LEN    (0x20)
#define DFT_CLI_ADDR_BASE      (0xA1000000)
#define DFT_CLI_ADDR_MASK      (0x00800000-1)

btg_para_v3_t btg_cli_data[] = {
    {
    	.gbase = CLI_NO_DATA_BASE,
    	.lxid  = CLI_NO_DATA_BASE,
    	.feat  = BTG_WRITE,
        .rec_latest = 1,
        .burstlen = DFT_CLI_BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = DFT_CLI_PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 800,  /* in cycles. */
        .perid_time = 800, /* in cycles. */
        .addr_base = DFT_CLI_ADDR_BASE,
        .addr_mask = DFT_CLI_ADDR_MASK,
        .addr_gap = DFT_CLI_PACKAGE_LEN,
    },
    {
        .gbase = CLI_NO_DATA_BASE,
        .lxid  = CLI_NO_DATA_BASE,
        .feat  = BTG_READ,
        .rec_latest = 1,
        .burstlen = DFT_CLI_BURST_LEN, /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
        .pktlen = DFT_CLI_PACKAGE_LEN,  /* from 32B ~ 8160B in a unit of 32B. */
        .resp_time = 800,  /* in cycles. */
        .perid_time = 800, /* in cycles. */
        .addr_base = DFT_CLI_ADDR_BASE,
        .addr_mask = DFT_CLI_ADDR_MASK,
        .addr_gap = DFT_CLI_PACKAGE_LEN,
    },
};


SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_lx(const void *user, u32_t argc, const char *argv[])
{
    if(argc<3){
        return CCR_INCOMPLETE_CMD;
    }
    u32_t  lx_id = atoi(argv[2]);
    inline_bzero(&btg_cli_data[0], sizeof(btg_cli_data));
    btg_cli_data[0].gbase = CLI_NO_DATA_BASE;
    btg_cli_data[1].gbase = CLI_NO_DATA_BASE;

    switch(lx_id){
        case 0:
        case 1:            
        case 2:
            memcpy((char *)&btg_cli_data[0], (char *)&btg_parameter[(lx_id*2)], sizeof(btg_para_v3_t));
            memcpy((char *)&btg_cli_data[1], (char *)&btg_parameter[(lx_id*2)+1], sizeof(btg_para_v3_t));
            break;

        case 3:
        case 5:
            memcpy((char *)&btg_cli_data[0], (char *)&btg_parameter[(lx_id+3)], sizeof(btg_para_v3_t));
            break;

        case 4:
        case 6:
            memcpy((char *)&btg_cli_data[1], (char *)&btg_parameter[(lx_id+3)], sizeof(btg_para_v3_t));
    		break;
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t 
_check_btg_existing(u8_t *wExist, u8_t *rExist)
{
    *wExist = (CLI_NO_DATA_BASE != btg_cli_data[0].gbase)?1:0;
    *rExist = (CLI_NO_DATA_BASE != btg_cli_data[1].gbase)?1:0;
    if(!(*wExist|*rExist)){
        printf("EE: LX ID not yet assigned\n");
        return CCR_FAIL;
    }
    return CCR_OK;
}


SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_burstlen(const void *user, u32_t argc, const char *argv[])
{
    if(argc<4){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    _check_btg_existing(&w_exist, &r_exist);

    if(w_exist){
        btg_cli_data[0].burstlen = atoi(argv[2]);
    }
    if(r_exist){
        btg_cli_data[1].burstlen = atoi(argv[3]);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_pktlen(const void *user, u32_t argc, const char *argv[])
{
    if(argc<4){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    _check_btg_existing(&w_exist, &r_exist);

    if(w_exist){
        btg_cli_data[0].pktlen= atoi(argv[2]);
    }
    if(r_exist){
        btg_cli_data[1].pktlen = atoi(argv[3]);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_addrgap(const void *user, u32_t argc, const char *argv[])
{
    if(argc<4){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    _check_btg_existing(&w_exist, &r_exist);

    if(w_exist){
        btg_cli_data[0].addr_gap= atoi(argv[2]);
    }
    if(r_exist){
        btg_cli_data[1].addr_gap = atoi(argv[3]);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_addrbase(const void *user, u32_t argc, const char *argv[])
{
    if(argc<4){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    _check_btg_existing(&w_exist, &r_exist);

    if(w_exist){
        btg_cli_data[0].addr_base= atoi(argv[2]);
    }
    if(r_exist){
        btg_cli_data[1].addr_base = atoi(argv[3]);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_addrmask(const void *user, u32_t argc, const char *argv[])
{
    if(argc<4){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    _check_btg_existing(&w_exist, &r_exist);

    if(w_exist){
        btg_cli_data[0].addr_mask= atoi(argv[2]);
    }
    if(r_exist){
        btg_cli_data[1].addr_mask= atoi(argv[3]);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_respTime(const void *user, u32_t argc, const char *argv[])
{
    if(argc<4){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    _check_btg_existing(&w_exist, &r_exist);

    if(w_exist){
        btg_cli_data[0].resp_time= atoi(argv[2]);
    }
    if(r_exist){
        btg_cli_data[1].resp_time= atoi(argv[3]);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_periodTime(const void *user, u32_t argc, const char *argv[])
{
    if(argc<4){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    _check_btg_existing(&w_exist, &r_exist);

    if(w_exist){
        btg_cli_data[0].perid_time= atoi(argv[2]);
    }
    if(r_exist){
        btg_cli_data[1].perid_time= atoi(argv[3]);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t 
cli_btg_set_recLatest(const void *user, u32_t argc, const char *argv[])
{
    if(argc<4){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    _check_btg_existing(&w_exist, &r_exist);

    if(w_exist){
        btg_cli_data[0].rec_latest= atoi(argv[2]);
    }
    if(r_exist){
        btg_cli_data[1].rec_latest= atoi(argv[3]);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_start_lx(const void *user, u32_t argc, const char *argv[])
{
    btg_para_v3_t *btg_entry0 = &btg_cli_data[0];
    btg_para_v3_t *btg_entry1 = &btg_cli_data[1];

    u8_t w_exist, r_exist;
    if(CCR_OK != _check_btg_existing(&w_exist, &r_exist)) return CCR_FAIL;


    if(w_exist){
        _gdma_reset(btg_entry0);
    }else if(r_exist){
        _gdma_reset(btg_entry1);
    }
    
    if(w_exist){
        _btg_setup(btg_entry0);
        _btg_start(btg_entry0);
    }
    udelay(5);
    if(r_exist){
        _btg_setup(btg_entry1);
        _btg_start(btg_entry1);
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_stop_lx(const void *user, u32_t argc, const char *argv[])
{
    u8_t w_exist, r_exist;
    if(CCR_OK != _check_btg_existing(&w_exist, &r_exist)) return CCR_FAIL;

    if(w_exist){
        _btg_stop((btg_para_v3_t *)(&btg_cli_data[0]));
    }
    if(r_exist){
        _btg_stop((btg_para_v3_t *)(&btg_cli_data[1]));
    }

    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_show_status_lx(const void *user, u32_t argc, const char *argv[])
{
    if(argc<=1){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    if(CCR_OK != _check_btg_existing(&w_exist, &r_exist)) return CCR_FAIL;

    if(w_exist){
        _btg_para_decode((btg_para_v3_t *)(&btg_cli_data[0]));
    }
    if(r_exist){
        _btg_para_decode((btg_para_v3_t *)(&btg_cli_data[1]));
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_show_reg_lx(const void *user, u32_t argc, const char *argv[])
{
    if(argc<=1){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    if(CCR_OK != _check_btg_existing(&w_exist, &r_exist)) return CCR_FAIL;

    if(w_exist){
        _gdma_show_reg((btg_para_v3_t *)(&btg_cli_data[0]));
    }else if(r_exist){
        _gdma_show_reg((btg_para_v3_t *)(&btg_cli_data[1]));
    }
    
    if(w_exist){
        _btg_show_reg((btg_para_v3_t *)(&btg_cli_data[0]));
    }
    if(r_exist){
        _btg_show_reg((btg_para_v3_t *)(&btg_cli_data[1]));
    }
    return CCR_OK;
}


SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_verify_lx(const void *user, u32_t argc, const char *argv[])
{
    if(argc<=1){
        return CCR_INCOMPLETE_CMD;
    }

    u8_t w_exist, r_exist;
    if(CCR_OK != _check_btg_existing(&w_exist, &r_exist)) return CCR_FAIL;

    if(w_exist){
        _btg_print_error((btg_para_v3_t *)(&btg_cli_data[0]));
    }
    if(r_exist){
        _btg_print_error((btg_para_v3_t *)(&btg_cli_data[1]));
    }
    return CCR_OK;
}


SECTION_CLI_UTIL cli_cmd_ret_t
clibtg_start_all(const void *user, u32_t argc, const char *argv[])
{
    btg_para_v3_t *btg_entry = &btg_parameter[0];
    BTG_STOP_ALL(btg_entry);
    btg_check_exist(btg_entry);    
    GDMA_RESET_ALL(btg_entry);
    BTG_SETUP_ALL(btg_entry);
    BTG_START_ALL(btg_entry);
    BTG_PARA_DECODE_ALL(btg_entry);
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_stop_all(const void *user, u32_t argc, const char *argv[])
{
    BTG_STOP_ALL(&btg_parameter[0]);
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_reset_all(const void *user, u32_t argc, const char *argv[])
{
    GDMA_RESET_ALL(&btg_parameter[0]);
    return CCR_OK;
}


SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_verify_all(const void *user, u32_t argc, const char *argv[])
{
    BTG_SHOW_REG_ALL(&btg_parameter[0]);
    BTG_PRI_ERR_ALL(&btg_parameter[0]);
    BTG_PARA_DECODE_ALL(&btg_parameter[0]);
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_btg_concur_all(const void *user, u32_t argc, const char *argv[])
{
    u32_t mt_loops=1;
    if(argc<3){
        return CCR_INCOMPLETE_CMD;
    }
    
    if(atoi(argv[2])){
#ifdef PROJECT_WITH_PBO_LX_BUS
        btg_pbo_like_data_init();
#endif
    }        

    if(argc>3){
        mt_loops = atoi(argv[3]);
    }

    btg_concur_mt_test(mt_loops);
    return CCR_OK;
}

extern cli_cmd_ret_t cli_std_call(const void *user, u32_t argc, const char *argv[]);

cli_top_node(btg, cli_std_call);
    cli_add_node(concur, btg, cli_btg_concur_all);   
    cli_add_help(concur, "btg concur <PBO_Like:1|0> [mt_loops]");
    
    cli_add_node(stopAll, btg, cli_btg_stop_all);
    cli_add_help(stopAll, "btg stopAll");

    cli_add_node(reset_all, btg, cli_btg_reset_all);
    cli_add_help(resetAll, "btg resetAll");

    cli_add_node(chkAll, btg, cli_btg_verify_all);
    cli_add_help(chkAll, "btg chkAll");

    cli_add_node(set_lx, btg, cli_btg_set_lx);   
    cli_add_help(setLx, "btg setLX <lx_number 0|1|2|3|4|5|6 (0|1|2:W/R)(3:PBO_UW)(4:PBO_UR)(5:PBO_DW)(6:PBO_DR)>");

    cli_add_node(burstlen, btg, cli_btg_set_burstlen);   
    cli_add_help(burstlen, "    btg burstlen <W_BurstLen:0|1|2|3 (16B|32B|64B|128B)> <R_BurstLen:0|1|2|3>");

    cli_add_node(pktlen, btg, cli_btg_set_pktlen);   
    cli_add_help(pktlen, "    btg pktlen <W_PackageLen> <R_PackageLen>");

    cli_add_node(addrgap, btg, cli_btg_set_addrgap);   
    cli_add_help(addrgap, "    btg addrgap <W_AddrGap> <R_AddrGap>");

    cli_add_node(addrbase, btg, cli_btg_set_addrbase);   
    cli_add_help(addrbase, "    btg addrbase <W_AddrBase> <R_AddrBase>");

    cli_add_node(addrmask, btg, cli_btg_set_addrmask);   
    cli_add_help(addrmask, "    btg addrmask <W_AddrMask> <R_AddrMask>");

    cli_add_node(respTime, btg, cli_btg_set_respTime);   
    cli_add_help(respTime, "    btg respTime <W_respTime> <R_respTime>");

    cli_add_node(periodTime, btg, cli_btg_set_periodTime);   
    cli_add_help(periodTime, "    btg periodTime <W_periodTime> <R_periodTime>");

    cli_add_node(recLatest, btg, cli_btg_set_recLatest);   
    cli_add_help(recLatest, "    btg recLatest <W_recLatest> <R_recLatest>");
    
    cli_add_node(startLX, btg, cli_btg_start_lx);   
    cli_add_help(startLX, "    btg startLX");

    cli_add_node(startAll, btg, clibtg_start_all);   
    cli_add_help(startAll, "    btg startAll");
    
    cli_add_node(showStatusLX, btg, cli_btg_show_status_lx);
    cli_add_help(showStatusLX, "    btg showStatusLX");

    cli_add_node(showRegLX, btg, cli_btg_show_reg_lx);
    cli_add_help(showRegLX, "    btg showRegLX");

    cli_add_node(chkLX, btg, cli_btg_verify_lx);
    cli_add_help(chkLX, "    btg chkLX");

    cli_add_node(stopLX, btg, cli_btg_stop_lx);
    cli_add_help(stopLX, "    btg stopLX");

