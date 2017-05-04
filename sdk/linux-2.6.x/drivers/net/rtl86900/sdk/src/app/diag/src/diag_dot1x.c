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
 *
 * Purpose : Definition those register command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) register
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <common/debug/mem.h>
#include <ioal/ioal_init.h>
#include <ioal/mem32.h>
#include <hal/chipdef/allreg.h>
#include <hal/mac/reg.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>
#include <dal/apollo/raw/apollo_raw_dot1x.h>
#include <diag_str.h>

/*
 * dot1x ( add | del ) auth-mac port <UINT:port> svl fid <UINT:fid> mac <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_dot1x_add_del_auth_mac_port_port_svl_fid_fid_mac_mac(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    
    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_add_del_auth_mac_port_port_svl_fid_fid_mac_mac */

/*
 * dot1x ( add | del ) auth-mac port <UINT:port> svl fid <UINT:fid> mac <MACADDR:mac> extension-filter-id <UINT:efid>
 */
cparser_result_t
cparser_cmd_dot1x_add_del_auth_mac_port_port_svl_fid_fid_mac_mac_extension_filter_id_efid(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *efid_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_add_del_auth_mac_port_port_svl_fid_fid_mac_mac_extension_filter_id_efid */

/*
 * dot1x ( add | del ) auth-mac port <UINT:port> vid <UINT:vid> mac <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_dot1x_add_del_auth_mac_port_port_vid_vid_mac_mac(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_add_del_auth_mac_port_port_vid_vid_mac_mac */

/*
 * dot1x ( add | del ) auth-mac port <UINT:port> vid <UINT:vid> mac <MACADDR:mac> extension-filter-id <UINT:efid>
 */
cparser_result_t
cparser_cmd_dot1x_add_del_auth_mac_port_port_vid_vid_mac_mac_extension_filter_id_efid(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *efid_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_add_del_auth_mac_port_port_vid_vid_mac_mac_extension_filter_id_efid */

/*
 * dot1x get auth-mac  
 */
cparser_result_t
cparser_cmd_dot1x_get_auth_mac(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_auth_mac */

/*
 * dot1x get auth-mac mac <MACADDR:mac> svl fid <UINT:fid> 
 */
cparser_result_t
cparser_cmd_dot1x_get_auth_mac_mac_mac_svl_fid_fid(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *fid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_auth_mac_mac_mac_svl_fid_fid */

/*
 * dot1x get auth-mac mac <MACADDR:mac> svl fid <UINT:fid> extension-filter-id  <UINT:efid> 
 */
cparser_result_t
cparser_cmd_dot1x_get_auth_mac_mac_mac_svl_fid_fid_extension_filter_id_efid(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *fid_ptr,
    uint32_t  *efid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_auth_mac_mac_mac_svl_fid_fid_extension_filter_id_efid */

/*
 * dot1x get auth-mac mac <MACADDR:mac> vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_dot1x_get_auth_mac_mac_mac_vid_vid(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_auth_mac_mac_mac_vid_vid */

/*
 * dot1x get auth-mac mac <MACADDR:mac> vid <UINT:vid> extension-filter-id <UINT:efid>
 */
cparser_result_t
cparser_cmd_dot1x_get_auth_mac_mac_mac_vid_vid_extension_filter_id_efid(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *vid_ptr,
    uint32_t  *efid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Not implement\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_auth_mac_mac_mac_vid_vid_extension_filter_id_efid */

/*
 * dot1x get guest-vlan
 */
cparser_result_t
cparser_cmd_dot1x_get_guest_vlan(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 vid;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = apollo_raw_dot1x_guestVidx_get(&vid)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    diag_util_mprintf("Guest vlan index:%d\n",vid);

    if((ret = apollo_raw_dot1x_guestVlanOpdir_get(&enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  
    diag_util_mprintf("Allow guest vlan talk to auth. DA:%s\n",diagStr_enable[enable]);


    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_guest_vlan */

/*
 * dot1x get ( mac-based | port-based )
 */
cparser_result_t
cparser_cmd_dot1x_get_mac_based_port_based(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_dot1x_opDir_t opDir;
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('m' == TOKEN_CHAR(2,0))
    {    
        if((ret = apollo_raw_dot1x_macBaseVlanOpdir_get(&opDir)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_printf("mac-based operational direction:");
        diag_util_mprintf("%s\n",diagStr_1xOpDir[opDir]);

        diag_util_mprintf("\nMAC-based function enable/disable setting:\n");        
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    
        diag_util_mprintf("Port State\n");      
          
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            if((ret = apollo_raw_dot1x_portBasedEnable_get(port,&enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }        
            diag_util_mprintf("%-4d %s\n",port,diagStr_enable[enable]);      
         
        } 

    }
    else
    {
        diag_util_mprintf("port-based function enable/disable setting:\n");        
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

        diag_util_mprintf("Port State\n");      
          
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            if((ret = apollo_raw_dot1x_portBasedEnable_get(port,&enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }        
            diag_util_mprintf("%-4d %s\n",port,diagStr_enable[enable]);      
         
        } 
    }   
    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_mac_based_port_based */

/*
 * dot1x get ( mac-based | port-based ) port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_dot1x_get_mac_based_port_based_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_dot1x_opDir_t opDir;
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
    apollo_raw_dot1x_authState_t auth;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('m' == TOKEN_CHAR(2,0))
    {    
        if((ret = apollo_raw_dot1x_macBaseVlanOpdir_get(&opDir)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_printf("mac-based operational direction:");
        if(RAW_DOT1X_DIRECT_BOTH == opDir)
            diag_util_mprintf("BOTH\n");
        else
            diag_util_mprintf("IN\n");

        diag_util_mprintf("\nMAC-based function enable/disable setting:\n");        
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    
        diag_util_mprintf("Port State\n");      
          
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            if((ret = apollo_raw_dot1x_macBasedEnable_get(port,&enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }        
            diag_util_mprintf("%-4d %s\n",port,diagStr_enable[enable]);      
         
        } 

    }
    else
    {
        diag_util_mprintf("port-based function setting:\n");        
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    
        diag_util_mprintf("%4S %12s %15s %5s\n","Port","Enable_State","Authorize_State","OpDir");      
          
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            if((ret = apollo_raw_dot1x_portBasedEnable_get(port,&enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            if((ret = apollo_raw_dot1x_portAuthState_get(port,&auth)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }            
           
            if((ret = apollo_raw_dot1x_portBaseVlanOpdir_get(port,&opDir)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            
 
                    
            diag_util_mprintf("%-4d %-12s %-15s %-5s\n",port,diagStr_enable[enable],diagStr_authstate[auth],diagStr_1xOpDir[opDir]);      
         
        } 
    }   
    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_mac_based_port_based_port_port_all */

/*
 * dot1x get trap
 */
cparser_result_t
cparser_cmd_dot1x_get_trap(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 pri;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = apollo_raw_dot1x_trapPri_get(&pri)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    diag_util_mprintf("Trap priority:%d\n",pri);

    return CPARSER_OK;    
}    /* end of cparser_cmd_dot1x_get_trap */

/*
 * dot1x get unauth-packet port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_dot1x_get_unauth_packet_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_dot1x_unAuthAct_t act;
    diag_portlist_t portlist;
    rtk_port_t port;
   
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_OUTPUT_INIT();
    diag_util_mprintf("Port Unauth Action\n");      
      
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_dot1x_unauthAct_get(port,&act)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }        
        diag_util_mprintf("%-4d %s\n",port,diagStr_unAuthAct[act]);      
     
    } 

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_get_unauth_packet_port_port_all */

/*
 * dot1x set guest-vlan to-auth-da ( allow | disallow )
 */
cparser_result_t
cparser_cmd_dot1x_set_guest_vlan_to_auth_da_allow_disallow(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('a' == TOKEN_CHAR(4,0))
        enable = ENABLED;
    else
        enable = DISABLED;
        
    if((ret = apollo_raw_dot1x_guestVlanOpdir_set(enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_guest_vlan_to_auth_da_allow_disallow */

/*
 * dot1x set guest-vlan vidx <UINT:vidx>
 */
cparser_result_t
cparser_cmd_dot1x_set_guest_vlan_vidx_vidx(
    cparser_context_t *context,
    uint32_t  *vidx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = apollo_raw_dot1x_guestVidx_set(*vidx_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_guest_vlan_vidx_vidx */

/*
 * dot1x set mac-based direction ( both | in )
 */
cparser_result_t
cparser_cmd_dot1x_set_mac_based_direction_both_in(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_dot1x_opDir_t opDir;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('b' == TOKEN_CHAR(4,0))
        opDir = RAW_DOT1X_DIRECT_BOTH;
    else
        opDir = RAW_DOT1X_DIRECT_IN;
    
    if((ret = apollo_raw_dot1x_macBaseVlanOpdir_set(opDir)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    
    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_mac_based_direction_both_in */

/*
 * dot1x set mac-based port ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_dot1x_set_mac_based_port_port_all_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e' == TOKEN_CHAR(6,0))
        enable = ENABLED;
    else
        enable = DISABLED;
        
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_dot1x_macBasedEnable_set(port,enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }        
     
    } 

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_mac_based_port_port_all_state_disable_enable */

/*
 * dot1x set port-based port ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_dot1x_set_port_based_port_port_all_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    rtk_enable_t enable;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e' == TOKEN_CHAR(6,0))
        enable = ENABLED;
    else
        enable = DISABLED;
        
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_dot1x_portBasedEnable_set(port,enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }        
     
    } 

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_port_based_port_port_all_state_disable_enable */

/*
 * dot1x set port-based port ( <PORT_LIST:port> | all ) ( auth | unauth )
 */
cparser_result_t
cparser_cmd_dot1x_set_port_based_port_port_all_auth_unauth(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_dot1x_authState_t auth;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('a' == TOKEN_CHAR(5,0))
        auth = RAW_DOT1X_AUTHORIZED;
    else
        auth = RAW_DOT1X_UNAUTHORIZED;
        
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_dot1x_portAuthState_set(port,auth)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }        
     
    } 

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_port_based_port_port_all_auth_unauth */

/*
 * dot1x set port-based port ( <PORT_LIST:port> | all ) direction ( both | in )
 */
cparser_result_t
cparser_cmd_dot1x_set_port_based_port_port_all_direction_both_in(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_dot1x_opDir_t opDir;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('b' == TOKEN_CHAR(6,0))
        opDir = RAW_DOT1X_DIRECT_BOTH;
    else
        opDir = RAW_DOT1X_DIRECT_IN;
        
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_dot1x_portBaseVlanOpdir_set(port,opDir)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }        
     
    } 

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_port_based_port_port_all_direction_both_in */


/*
 * dot1x set trap-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_dot1x_set_trap_priority_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = apollo_raw_dot1x_trapPri_set(*priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_trap_priority_priority */

/*
 * dot1x set unauth-packet port ( <PORT_LIST:port> | all ) ( drop | guest-vlan | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_dot1x_set_unauth_packet_port_port_all_drop_guest_vlan_trap_to_cpu(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_dot1x_unAuthAct_t act;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('d' == TOKEN_CHAR(5,0))
        act = RAW_DOT1X_UNAUTH_DROP;
    else if('g' == TOKEN_CHAR(5,0))
        act = RAW_DOT1X_UNAUTH_GVLAN;
    else        
        act = RAW_DOT1X_UNAUTH_TRAP;
        
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_dot1x_unauthAct_set(port,act)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }        
     
    } 

    return CPARSER_OK;
}    /* end of cparser_cmd_dot1x_set_unauth_packet_port_port_all_drop_guest_vlan_trap_to_cpu */

