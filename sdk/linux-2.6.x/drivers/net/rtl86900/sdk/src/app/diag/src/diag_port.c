/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition those Port command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <parser/cparser_priv.h>
#include <dal/apollo/raw/apollo_raw_port.h>
#include <diag_str.h>

/*
 * port dump isolation
 */
cparser_result_t
cparser_cmd_port_dump_isolation(
    cparser_context_t *context)
{
    uint32 port;
    uint32 vidx;
    int32 ret;
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_PARAM_CHK();
    diag_util_mprintf("\n");
    diag_util_mprintf(" port   Vid(port)   Vid(port_l34)\n");
    diag_util_mprintf("----------------------------------\n");
    for (port = 0; port < HAL_GET_PORTNUM(); port++)
    {
        diag_util_printf(" %-5u  ",port);
        DIAG_UTIL_ERR_CHK(apollo_raw_port_isolation_get(port, &vidx), ret);
        diag_util_printf("%-9u   ",vidx);
        DIAG_UTIL_ERR_CHK(apollo_raw_port_isolationL34_get(port, &vidx), ret);
        diag_util_printf("%u   ",vidx);
        diag_util_mprintf("\n");
    }
    diag_util_mprintf("\n");
    diag_util_mprintf("\n");
    diag_util_mprintf(" port   Vid(extport)   Vid(extport_l34)\n");
    diag_util_mprintf("---------------------------------------\n");
    for (port = 0; port < APOLLO_EXTPORTNO; port++)
    {
        diag_util_printf(" %-5u  ",port);
        DIAG_UTIL_ERR_CHK(apollo_raw_extport_isolation_get(port, &vidx), ret);
        diag_util_printf("%-12u  ",vidx);
        DIAG_UTIL_ERR_CHK(apollo_raw_extport_isolationL34_get(port, &vidx), ret);
        diag_util_printf("%u   ",vidx);
        diag_util_mprintf("\n");
    }
    diag_util_mprintf("\n");
    diag_util_mprintf("\n");
    diag_util_mprintf("\n");
    diag_util_mprintf(" port   Vid(dsl)\n");
    diag_util_mprintf("----------------\n");
    for (port = 0; port < APOLLO_DSLVCNO; port++)
    {
        diag_util_printf(" %-5u  ",port);
        DIAG_UTIL_ERR_CHK(apollo_raw_dslvc_isolation_get(port, &vidx), ret);
        diag_util_mprintf("%u   \n",vidx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_port_dump_isolation */

/*
 * port get auto-nego port ( <PORT_LIST:port> | all ) ability
 */
cparser_result_t
cparser_cmd_port_get_auto_nego_port_port_all_ability(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_port_ability_t portAbilityR;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Speed Duplex Status TxFC RxFc Nway Lpi100 LpiGiga Master NwayStatus\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_ability_get(port, &portAbilityR), ret);


        diag_util_mprintf("%-4d %-5s %-6s %-6s %-4s %-4s %-4s %-6s %-7s %-6s %s\n",
                            port,
                            diagStr_portSpeed[portAbilityR.speed],
                            diagStr_portDuplex[portAbilityR.duplex],
                            diagStr_portLinkStatus[portAbilityR.linkStatus],
                            diagStr_enDisplay[portAbilityR.txFc],
                            diagStr_enDisplay[portAbilityR.rxFc],
                            diagStr_enDisplay[portAbilityR.nwayAbility],
                            diagStr_enDisplay[portAbilityR.lpi_100m],
                            diagStr_enDisplay[portAbilityR.lpi_giga],
                            diagStr_enDisplay[portAbilityR.masterMod],
                            diagStr_portNwayFault[portAbilityR.nwayFault]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_auto_nego_port_port_all_ability */

#if defined(CMD_PORT_GET_AUTO_NEGO_PORT_PORT_ALL_STATE)
/*
 * port get auto-nego port ( <PORT_LIST:port> | all ) state
 */
cparser_result_t
cparser_cmd_port_get_auto_nego_port_port_all_state(
    cparser_context_t *context,
    char * *port_ptr)
{
    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_auto_nego_port_port_all_state */
#endif
/*
 * port get enhanced-fid port ( <PORT_LIST:src_port> | all )
 */
cparser_result_t
cparser_cmd_port_get_enhanced_fid_port_src_port_all(
    cparser_context_t *context,
    char * *src_port_ptr)
{

    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    rtk_fid_t           efid;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *src_port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_efid_get(port, &efid), ret);
        diag_util_mprintf("\tPort %2u : efid = %u\n", port, efid);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_enhanced_fid_port_src_port_all */

/*
 * port get force port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_port_get_force_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_port_ability_t portAbilityR;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Speed Duplex Status TxFC RxFc Nway Lpi100 LpiGiga Master\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_ForceAbility_get(port, &portAbilityR), ret);
        diag_util_mprintf("%-4d %-5s %-6s %-6s %-4s %-4s %-4s %-6s %-7s %-s\n",
                            port,
                            diagStr_portSpeed[portAbilityR.speed],
                            diagStr_portDuplex[portAbilityR.duplex],
                            diagStr_portLinkStatus[portAbilityR.linkStatus],
                            diagStr_enDisplay[portAbilityR.txFc],
                            diagStr_enDisplay[portAbilityR.rxFc],
                            diagStr_enDisplay[portAbilityR.nwayAbility],
                            diagStr_enDisplay[portAbilityR.lpi_100m],
                            diagStr_enDisplay[portAbilityR.lpi_giga],
                            diagStr_enDisplay[portAbilityR.masterMod]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_force_port_port_all */
/*
 * port get force-dmp port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_port_get_force_dmp_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    rtk_portmask_t portmask;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_forceDmpMask_get(port, &portmask), ret);
        diag_util_mprintf("\tPort %2u : port mask = %x\n", port, portmask.bits[0]);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_port_get_force_dmp_port_port_all */

/*
 * port get force-dmp
 */
cparser_result_t
cparser_cmd_port_get_force_dmp(
    cparser_context_t *context)
{
    rtk_enable_t state;
    int32 ret = CPARSER_NOT_OK;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(apollo_raw_port_forceDmp_get(&state), ret);
    diag_util_mprintf("The fore dmp mode : %s\n", state?"enable":"disable");
    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_force_dmp */

/*
 * port get isolation dsl ( <PORT_LIST:dsl> | dsl_all )
 */
cparser_result_t
cparser_cmd_port_get_isolation_dsl_dsl_dsl_all(
    cparser_context_t *context,
    char * *dsl_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    rtk_fid_t           vidx;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *dsl_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_dslvc_isolation_get(port, &vidx), ret);
        diag_util_mprintf("\tPort %2u : vidx = %u\n", port, vidx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_isolation_dsl_dsl_dsl_all */

/*
 * port get isolation ext <PORT_LIST:ext>
 */
cparser_result_t
cparser_cmd_port_get_isolation_ext_ext(
    cparser_context_t *context,
    char * *ext_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    rtk_fid_t           vidx;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *ext_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        RT_PARAM_CHK(port == 0, CPARSER_ERR_INVALID_PARAMS);
        DIAG_UTIL_ERR_CHK(apollo_raw_extport_isolation_get(port-1, &vidx), ret);
        diag_util_mprintf("\tPort %2u : vidx = %u\n", port, vidx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_isolation_ext_ext_ext_all */

/*
 * port get isolation ext_l34 <PORT_LIST:ext>
 */
cparser_result_t
cparser_cmd_port_get_isolation_ext_l34_ext(
    cparser_context_t *context,
    char * *ext_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    rtk_fid_t           vidx;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *ext_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        RT_PARAM_CHK(port == 0, CPARSER_ERR_INVALID_PARAMS);
        DIAG_UTIL_ERR_CHK(apollo_raw_extport_isolationL34_get(port-1, &vidx), ret);
        diag_util_mprintf("\tPort %2u : vidx = %u\n", port, vidx);
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_isolation_ext_l34_ext_ext_all */


/*
 * port get isolation port ( <PORT_LIST:src_port> | all )
 */
cparser_result_t
cparser_cmd_port_get_isolation_port_src_port_all(
    cparser_context_t *context,
    char * *src_port_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    rtk_fid_t           vidx;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *src_port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_isolation_get(port, &vidx), ret);
        diag_util_mprintf("\tPort %2u : vidx = %u\n", port, vidx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_isolation_port_src_port_all */

/*
 * port get isolation port_l34 ( <PORT_LIST:src_port> | all )
 */
cparser_result_t
cparser_cmd_port_get_isolation_port_l34_src_port_all(
    cparser_context_t *context,
    char * *src_port_ptr)
{
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;
    rtk_fid_t           vidx;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *src_port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_isolationL34_get(port, &vidx), ret);
        diag_util_mprintf("\tPort %2u : vidx = %u\n", port, vidx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_isolation_port_l34_src_port_all */

/*
 * port get local-packet port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_port_get_local_packet_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{

    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Action\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_localPacket_get(port, &action), ret);

        diag_util_mprintf("%-5d%s\n", port, diagStr_actionStr[action]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_local_packet_port_port_all */

#if defined(CMD_PORT_GET_MASTER_SLAVE_PORT_PORT_ALL)
/*
 * port get master-slave port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_port_get_master_slave_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_master_slave_port_port_all */
#endif
/*
 * port get phy-reg phy ( <PORT_LIST:phy> | all ) page <UINT:page> register <UINT:register>
 */
cparser_result_t
cparser_cmd_port_get_phy_reg_phy_phy_all_page_page_register_register(
    cparser_context_t *context,
    char * *phy_ptr,
    uint32_t  *page_ptr,
    uint32_t  *register_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint16 outputData;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("PHY Data\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
#if defined (FPGA_DEFINED)
        uint16 inputData;
        /*change page*/
        inputData = 0xC000 + (port<<5) + (31);

        io_mii_phy_reg_write(8,1,*page_ptr);
        io_mii_phy_reg_write(8,0,inputData);

        inputData = 0x8000 + (port<<5) + (*register_ptr);
        io_mii_phy_reg_write(8,0,inputData);
        io_mii_phy_reg_read(8,2,&outputData);
#else
        uint32_t phyId;
        uint32_t page;
        uint32_t reg;

        switch(port)
        {
            case 0:
            case 1:
                phyId = port;
                break;
            case 4:
                phyId = 2;
                break;
            case 5:
                phyId = 3;
                break;
            case 3:
                phyId = 4;
                break;
            default:
                diag_util_printf("no internal phy for port:%d\n",port);
                continue;
        }

        DIAG_UTIL_ERR_CHK(apollo_interPhy_read(phyId, *page_ptr,*register_ptr,&outputData), ret);
#endif
        diag_util_mprintf("%-3d 0x%4.4X\n", port, outputData);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_phy_reg_phy_phy_all_page_page_register_register */

#if defined(CMD_PORT_SET_AUTO_NEGO_PORT_PORT_ALL_ABILITY_10H_10F_100H_100F_1000F_FLOW_CONTROL_ASY_FLOW_CONTROL)
/*
 * port set auto-nego port ( <PORT_LIST:port> | all ) ability { 10h } { 10f } { 100h } { 100f } { 1000f } { flow-control } { asy-flow-control }
 */
cparser_result_t
cparser_cmd_port_set_auto_nego_port_port_all_ability_10h_10f_100h_100f_1000f_flow_control_asy_flow_control(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_auto_nego_port_port_all_ability_10h_10f_100h_100f_1000f_flow_control_asy_flow_control */
#endif

#if defined(CMD_PORT_SET_AUTO_NEGO_PORT_PORT_ALL_STATE_DISABLE_ENABLE)
/*
 * port set auto-nego port ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_port_set_auto_nego_port_port_all_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_auto_nego_port_port_all_state_disable_enable */
#endif
/*
 * port set enhanced-fid port ( <PORT_LIST:src_port> | all ) efid <UINT:efid>
 */
cparser_result_t
cparser_cmd_port_set_enhanced_fid_port_src_port_all_efid_efid(
    cparser_context_t *context,
    char * *src_port_ptr,
    uint32_t  *efid_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *src_port_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_efid_set(port, *efid_ptr), ret);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_port_set_enhanced_fid_port_src_port_all_efid_efid */

/*
 * port set force port ( <PORT_LIST:port> | all ) ability ( 10h | 10f | 100h | 100f | 1000f ) flow-control ( disable | enable )
 */
cparser_result_t
cparser_cmd_port_set_force_port_port_all_ability_10h_10f_100h_100f_1000f_flow_control_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_port_ability_t portAbilityR;
    apollo_raw_port_ability_t portAbilityW;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if(!osal_strcmp(TOKEN_STR(6),"10h"))
    {
        portAbilityW.duplex = PORT_HALF_DUPLEX;
        portAbilityW.speed = PORT_SPEED_10M;
    }
    else if(!osal_strcmp(TOKEN_STR(6),"10f"))
    {
        portAbilityW.duplex = PORT_FULL_DUPLEX;
        portAbilityW.speed = PORT_SPEED_10M;
    }
    else if(!osal_strcmp(TOKEN_STR(6),"100h"))
    {
        portAbilityW.duplex = PORT_HALF_DUPLEX;
        portAbilityW.speed = PORT_SPEED_100M;
    }
    else if(!osal_strcmp(TOKEN_STR(6),"100f"))
    {
        portAbilityW.duplex = PORT_FULL_DUPLEX;
        portAbilityW.speed = PORT_SPEED_100M;
    }
    else if(!osal_strcmp(TOKEN_STR(6),"1000f"))
    {
        portAbilityW.duplex = PORT_FULL_DUPLEX;
        portAbilityW.speed = PORT_SPEED_1000M;
    }

    if ('e' == TOKEN_CHAR(8,0))
    {
        portAbilityW.txFc = ENABLED;
        portAbilityW.rxFc = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(8,0))
    {
        portAbilityW.txFc = DISABLED;
        portAbilityW.rxFc = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_ForceAbility_get(port, &portAbilityR), ret);

        portAbilityR.txFc = portAbilityW.txFc;
        portAbilityR.rxFc = portAbilityW.rxFc;
        portAbilityR.speed = portAbilityW.speed;
        portAbilityR.duplex = portAbilityW.duplex;

        DIAG_UTIL_ERR_CHK(apollo_raw_port_ForceAbility_set(port, &portAbilityR), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_force_port_port_all_ability_10h_10f_100h_100f_1000f_flow_control_disable_enable */
/*
 * port set force-dmp port ( <PORT_LIST:port> | all ) port_mask ( <PORT_LIST:port_mask> | all )
 */
cparser_result_t
cparser_cmd_port_set_force_dmp_port_port_all_port_mask_port_mask_all(
    cparser_context_t *context,
    char * *port_ptr,
    char * *port_mask_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    diag_portlist_t    portMask;
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portMask, 6), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_forceDmpMask_set(port, portMask.portmask), ret);
    }
     return CPARSER_OK;
}    /* end of cparser_cmd_port_set_force_dmp_port_port_all_port_mask_port_mask_all */

/*
 * port set force-dmp state ( disable | enable )
 */
cparser_result_t
cparser_cmd_port_set_force_dmp_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret = CPARSER_NOT_OK;
    rtk_enable_t enable;
    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;
    DIAG_UTIL_ERR_CHK(apollo_raw_port_forceDmp_set(enable), ret);
    return CPARSER_OK;

}    /* end of cparser_cmd_port_set_force_dmp_state_disable_enable */

/*
 * port set isolation dsl ( <PORT_LIST:dsl> | dsl_all ) vidx <UINT:vidx>
 */
cparser_result_t
cparser_cmd_port_set_isolation_dsl_dsl_dsl_all_vidx_vidx(
    cparser_context_t *context,
    char * *dsl_ptr,
    uint32_t  *vidx_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *dsl_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_dslvc_isolation_set(port, *vidx_ptr), ret);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_port_set_isolation_dsl_dsl_dsl_all_vidx_vidx */

/*
 * port set isolation ext <PORT_LIST:ext> vidx <UINT:vidx>
 */
cparser_result_t
cparser_cmd_port_set_isolation_ext_ext_vidx_vidx(
    cparser_context_t *context,
    char * *ext_ptr,
    uint32_t  *vidx_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *ext_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        RT_PARAM_CHK(port == 0, CPARSER_ERR_INVALID_PARAMS);
        DIAG_UTIL_ERR_CHK(apollo_raw_extport_isolation_set(port-1, *vidx_ptr), ret);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_port_set_isolation_ext_ext_ext_all_vidx_vidx */

/*
 * port set isolation ext_l34 <PORT_LIST:ext> vidx <UINT:vidx>
 */
cparser_result_t
cparser_cmd_port_set_isolation_ext_l34_ext_vidx_vidx(
    cparser_context_t *context,
    char * *ext_ptr,
    uint32_t  *vidx_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == *ext_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        RT_PARAM_CHK(port == 0, CPARSER_ERR_INVALID_PARAMS);
        DIAG_UTIL_ERR_CHK(apollo_raw_extport_isolationL34_set(port-1, *vidx_ptr), ret);
    }
    return CPARSER_OK;

}    /* end of cparser_cmd_port_set_isolation_ext_l34_ext_ext_all_vidx_vidx */

/*
 * port set isolation port ( <PORT_LIST:src_port> | all ) vidx <UINT:vidx>
 */
cparser_result_t
cparser_cmd_port_set_isolation_port_src_port_all_vidx_vidx(
    cparser_context_t *context,
    char * *src_port_ptr,
    uint32_t  *vidx_ptr)
{


    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((*src_port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_isolation_set(port, *vidx_ptr), ret);
    }



    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_isolation_port_src_port_all_vidx_vidx */

/*
 * port set isolation port_l34 ( <PORT_LIST:src_port> | all ) vidx <UINT:vidx>
 */
cparser_result_t
cparser_cmd_port_set_isolation_port_l34_src_port_all_vidx_vidx(
    cparser_context_t *context,
    char * *src_port_ptr,
    uint32_t  *vidx_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_isolationL34_set(port, *vidx_ptr), ret);
    }
     return CPARSER_OK;

}    /* end of cparser_cmd_port_set_isolation_port_l34_src_port_all_vidx_vidx */

/*
 * port set local-packet port ( <PORT_LIST:port> | all ) action ( forward | drop )
 */
cparser_result_t
cparser_cmd_port_set_local_packet_port_port_all_action_forward_drop(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(6,0))
    {
        action = ACTION_DROP;
    }
    else if ('f' == TOKEN_CHAR(6,0))
    {
        action = ACTION_FORWARD;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_localPacket_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_local_packet_port_port_all_action_forward_drop */

#if defined(CMD_PORT_SET_MASTER_SLAVE_PORT_PORT_ALL_MASTER_SLAVE)
/*
 * port set master-slave port ( <PORT_LIST:port> | all ) ( master | slave )
 */
cparser_result_t
cparser_cmd_port_set_master_slave_port_port_all_master_slave(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_master_slave_port_port_all_master_slave */
#endif
/*
 * port set phy-reg phy ( <PORT_LIST:phy> | all ) page <UINT:page> register <UINT:register> data <UINT:data>
 */
cparser_result_t
cparser_cmd_port_set_phy_reg_phy_phy_all_page_page_register_register_data_data(
    cparser_context_t *context,
    char * *phy_ptr,
    uint32_t  *page_ptr,
    uint32_t  *register_ptr,
    uint32_t  *data_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
#if defined (FPGA_DEFINED)
        uint16 inputData;

        /*change page*/
        inputData = 0xC000 + (port<<5) + (31);

        io_mii_phy_reg_write(8,1,*page_ptr);
        io_mii_phy_reg_write(8,0,inputData);

        inputData = 0xC000 + (port<<5) + (*register_ptr);
        io_mii_phy_reg_write(8,1,*data_ptr);
        io_mii_phy_reg_write(8,0,inputData);
#else
        uint32_t phyId;
        uint32_t page;
        uint32_t reg;

        switch(port)
        {
            case 0:
            case 1:
                phyId = port;
                break;
            case 4:
                phyId = 2;
                break;
            case 5:
                phyId = 3;
                break;
            case 3:
                phyId = 4;
                break;
            default:
                diag_util_printf("no internal phy for port:%d\n",port);
                continue;
        }
        DIAG_UTIL_ERR_CHK(apollo_interPhy_write(phyId, *page_ptr,*register_ptr,*data_ptr), ret);
#endif
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_phy_reg_phy_phy_all_page_page_register_register_data_data */

/*
 * port set rtct ( <PORT_LIST:port> | all ) start
 */
cparser_result_t
cparser_cmd_port_set_rtct_port_all_start(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    diag_util_mprintf("not implement");
    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_rtct_port_all_start */

/*
 * port get rtct ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_port_get_rtct_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_rtct_port_all */


/*
 * port get phy-reg phy ( <PORT_LIST:phy> | all ) register <UINT:register>
 */
cparser_result_t
cparser_cmd_port_get_phy_reg_phy_phy_all_register_register(
    cparser_context_t *context,
    char * *phy_ptr,
    uint32_t  *register_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint16 outputData;

#if defined (FPGA_DEFINED)
    uint16 inputData;
#endif

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("PHY Data\n");
#if defined (FPGA_DEFINED)
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        inputData = 0x8000 + (port<<5) + (*register_ptr);
        io_mii_phy_reg_write(8,0,inputData);
        io_mii_phy_reg_read(8,2,&outputData);
        diag_util_mprintf("%-3d 0x%4.4X\n", port, outputData);
    }

#else
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        uint32_t phyId;
        uint32_t page;
        uint32_t reg;

        RT_PARAM_CHK((*register_ptr > 15), CPARSER_ERR_INVALID_PARAMS);
        switch(port)
        {
            case 0:
            case 1:
                phyId = port;
                break;
            case 4:
                phyId = 2;
                break;
            case 5:
                phyId = 3;
                break;
            case 3:
                phyId = 4;
                break;
            default:
                diag_util_printf("no internal phy for port:%d\n",port);
                continue;
        }

        if(*register_ptr>=8)
            page = 0xa41;
        else
            page = 0xa40;

        DIAG_UTIL_ERR_CHK(apollo_interPhy_read(phyId, page,*register_ptr,&outputData), ret);

        diag_util_mprintf("%-3d 0x%4.4X\n", port, outputData);

    }
#endif


    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_phy_reg_phy_phy_all_register_register */

/*
 * port set phy-reg phy ( <PORT_LIST:phy> | all ) register <UINT:register> data <UINT:data>
 */
cparser_result_t
cparser_cmd_port_set_phy_reg_phy_phy_all_register_register_data_data(
    cparser_context_t *context,
    char * *phy_ptr,
    uint32_t  *register_ptr,
    uint32_t  *data_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;


    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
#if defined (FPGA_DEFINED)
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        uint16 inputData;
        inputData = 0xC000 + (port<<5) + (*register_ptr);
        io_mii_phy_reg_write(8,1,*data_ptr);
        io_mii_phy_reg_write(8,0,inputData);
    }
#else
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        uint32_t phyId;
        uint32_t page;
        uint32_t reg;

        RT_PARAM_CHK((*register_ptr > 15), CPARSER_ERR_INVALID_PARAMS);
        switch(port)
        {
            case 0:
            case 1:
                phyId = port;
                break;
            case 4:
                phyId = 2;
                break;
            case 5:
                phyId = 3;
                break;
            case 3:
                phyId = 4;
                break;
            default:
                diag_util_printf("no internal phy for port:%d\n",port);
                continue;
        }

        if(*register_ptr>=8)
            page = 0xa41;
        else
            page = 0xa40;

        DIAG_UTIL_ERR_CHK(apollo_interPhy_write(phyId, page,*register_ptr,*data_ptr), ret);
    }
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_phy_reg_phy_phy_all_register_register_data_data */

/*
 * port set force port ( <PORT_LIST:port> | all ) ( lpi-100M | lpi-giga ) ( disable | enable )
 */
cparser_result_t
cparser_cmd_port_set_force_port_port_all_lpi_100M_lpi_giga_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_port_ability_t portAbility;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(6,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(6,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_ForceAbility_get(port, &portAbility), ret);

        if ('1' == TOKEN_CHAR(5,4))
        {
            portAbility.lpi_100m = enable;
        }
        else if ('g' == TOKEN_CHAR(5,4))
        {
            portAbility.lpi_giga = enable;
        }

        DIAG_UTIL_ERR_CHK(apollo_raw_port_ForceAbility_set(port, &portAbility), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_force_port_port_all_lpi_100m_lpi_giga_disable_enable */

/*
 * port set force port ( <PORT_LIST:port> | all ) link-state ( link-down | link-up )
 */
cparser_result_t
cparser_cmd_port_set_force_port_port_all_link_state_link_down_link_up(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_port_ability_t portAbilityR;
	rtk_port_linkStatus_t 	linkStatus;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(6,5))
    {
        linkStatus = PORT_LINKDOWN;
    }
    else if ('u' == TOKEN_CHAR(6,5))
    {
        linkStatus = PORT_LINKUP;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_ForceAbility_get(port, &portAbilityR), ret);

        portAbilityR.linkStatus = linkStatus;

        DIAG_UTIL_ERR_CHK(apollo_raw_port_ForceAbility_set(port, &portAbilityR), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_force_port_port_all_link_state_link_down_link_up */

/*
 * port set isolation leaky ip-mcast port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_port_set_isolation_leaky_ip_mcast_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);

    if ('d' == TOKEN_CHAR(8,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(8,0))
    {
        enable = ENABLED;
    }


    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_isoIpmcastLeaky_set(port, enable), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_isolation_leaky_ip_mcast_port_ports_all_state_enable_disable */

/*
 * port get isolation leaky ip-mcast port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_port_get_isolation_leaky_ip_mcast_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);

    diag_util_mprintf("Port State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_port_isoIpmcastLeaky_get(port, &enable), ret);

        diag_util_mprintf("%-5d%s\n", port, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_isolation_leaky_ip_mcast_port_ports_all_state */




