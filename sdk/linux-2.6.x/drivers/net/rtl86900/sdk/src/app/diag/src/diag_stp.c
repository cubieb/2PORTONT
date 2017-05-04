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
 * Purpose : Definition those command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) switch commands.    
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
#include <rtk/stp.h>
/* 
 * Symbol Definition 
 */

/* 
 * Data Declaration 
 */

/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */
 
 
/*
 * stp create <UINT:instance>
 */
cparser_result_t
cparser_cmd_stp_create_instance(
    cparser_context_t *context,
    uint32_t  *instance_ptr)
{
    rtk_switch_devInfo_t devInfo;
    int32       ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
#if 0
    if((ret = diag_om_get_deviceInfo(&devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }


    RT_PARAM_CHK((NULL == instance_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= devInfo.capacityInfo.max_num_of_msti), CPARSER_ERR_INVALID_PARAMS);     

    DIAG_UTIL_ERR_CHK(rtk_stp_mstpInstance_create(*instance_ptr), ret);
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_stp_create_instance */

/*
 * stp destroy <UINT:instance>
 */
cparser_result_t
cparser_cmd_stp_destroy_instance(
    cparser_context_t *context,
    uint32_t  *instance_ptr)
{
    int32       ret = RT_ERR_FAILED;
    rtk_switch_devInfo_t devInfo;

#if 0
    DIAG_UTIL_PARAM_CHK();
    if((ret = diag_om_get_deviceInfo(&devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == instance_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= devInfo.capacityInfo.max_num_of_msti), CPARSER_ERR_INVALID_PARAMS);   

    DIAG_UTIL_ERR_CHK(rtk_stp_mstpInstance_destroy(*instance_ptr), ret);
#endif    
    return CPARSER_OK;
}    /* end of cparser_cmd_stp_destroy_instance */

/*
 * stp dump <UINT:instance> ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_stp_dump_instance_port_all(
    cparser_context_t *context,
    uint32_t  *instance_ptr,
    char * *port_ptr)
{
    int32           ret = RT_ERR_FAILED;

    rtk_port_t         port = 0;
    rtk_stp_state_t stp_state = 0;
    rtk_switch_devInfo_t devInfo;
    diag_portlist_t               portlist;

    DIAG_UTIL_PARAM_CHK();

    if((ret = diag_om_get_deviceInfo(&devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    RT_PARAM_CHK((NULL == instance_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= devInfo.capacityInfo.max_num_of_msti), CPARSER_ERR_INVALID_PARAMS);    

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("MSTI %d Status:\n", *instance_ptr);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {                  
        DIAG_UTIL_ERR_CHK(rtk_stp_mstpState_get(*instance_ptr, port, &stp_state), ret);
        diag_util_mprintf("\tPort %2d: ", port);

        if (STP_STATE_DISABLED == stp_state)
        {
            diag_util_mprintf("DISABLED\n");
        }
        else if (STP_STATE_BLOCKING == stp_state)
        {
            diag_util_mprintf("BLOCKING\n");
        }
        else if (STP_STATE_LEARNING == stp_state)
        {
            diag_util_mprintf("LEARNING\n");
        }
        else if (STP_STATE_FORWARDING == stp_state)
        {
            diag_util_mprintf("FORWARDING\n");
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    diag_util_mprintf("\n");

    
    return CPARSER_OK;
}    /* end of cparser_cmd_stp_dump_instance_port_all */

/*
 * stp set <UINT:instance> ( <PORT_LIST:port> | all )  ( blocking | disable | forwarding | learning )
 */
cparser_result_t
cparser_cmd_stp_set_instance_port_all_blocking_disable_forwarding_learning(
    cparser_context_t *context,
    uint32_t  *instance_ptr,
    char * *port_ptr)
{
    int32           ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    rtk_stp_state_t stp_state = 0;
    rtk_switch_devInfo_t devInfo;
    diag_portlist_t               portlist;

    DIAG_UTIL_PARAM_CHK();

    if((ret = diag_om_get_deviceInfo(&devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    RT_PARAM_CHK((NULL == instance_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*instance_ptr >= devInfo.capacityInfo.max_num_of_msti), CPARSER_ERR_INVALID_PARAMS);    

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(4, 0))
    {
        stp_state = STP_STATE_DISABLED;
    }
    else if ('b' == TOKEN_CHAR(4, 0))
    {
        stp_state = STP_STATE_BLOCKING;
    }
    else if ('l' == TOKEN_CHAR(4, 0))
    {
        stp_state = STP_STATE_LEARNING;
    }
    else if ('f' == TOKEN_CHAR(4, 0))
    {
        stp_state = STP_STATE_FORWARDING;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_stp_mstpState_set(*instance_ptr, port, stp_state), ret);
    }    
    return CPARSER_OK;
}    /* end of cparser_cmd_stp_set_instance_port_all_blocking_disable_forwarding_learning */

