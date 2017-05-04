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
#include <diag_str.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#include <rtk/l34.h>
#include <dal/apollo/raw/apollo_raw_l34.h>



const char *ext_type_Str[L34_EXTIP_TYPE_END] = {
DIAG_STR_NAPT,
DIAG_STR_NAT,
DIAG_STR_LOCAL_PUBLIC
};

/*
 * l34 reset table ( l3 | pppoe | nexthop | interface | external_ip | arp | naptr | napt )
 */
cparser_result_t
cparser_cmd_l34_reset_table_l3_pppoe_nexthop_interface_external_ip_arp_naptr_napt(
    cparser_context_t *context)
{
    rtk_l34_table_type_t   l34_table;
    int32   ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    switch(TOKEN_STR(3)[0])
    {
        case 'l':
            l34_table = L34_ROUTING_TABLE;
            break;    
        case 'p':
            l34_table = L34_PPPOE_TABLE;
            break;    
        case 'i':
            l34_table = L34_NETIF_TABLE;
            break;    
        case 'e':
            l34_table = L34_INTIP_TABLE;
            break;    
        case 'a':
            l34_table = L34_ARP_TABLE;
            break;    
        case 'n':
            if(TOKEN_STR(3)[1]=='e')
                l34_table = L34_NEXTHOP_TABLE;                
            else 
            {
                if(strlen(TOKEN_STR(3)) == 5)
                    l34_table = L34_NAPTR_TABLE;
                else
                    l34_table = L34_NAPT_TABLE;
            }
            break;    
        default:
            return CPARSER_NOT_OK;
    }

    if((ret = rtk_l34_table_reset(l34_table)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_reset_table_l3_pppoe_nexthop_interface_external_ip_arp_naptr_napt */



void _cparser_cmd_l34_arp_header_display(void)
{
    diag_util_printf("%-5s ","Index" );
    diag_util_printf("%-7s ","Valid");
    diag_util_mprintf("%-6s\n","NH_IDX");    
}



void _cparser_cmd_l34_arp_entry_display(uint32 index,rtk_l34_arp_entry_t *entry)
{
    diag_util_printf("%-5d ",index);

    if(entry->valid == 1)
    {
        /*valid*/
        diag_util_printf("%-7s ",DIAG_STR_VALID);      
        diag_util_mprintf("%-6d\n",entry->nhIdx);    
    }
    else
    {
        /*invalid*/
        diag_util_mprintf("%-7s \n",DIAG_STR_INVALID);      
    }
}

/*
 * l34 get arp
 */
cparser_result_t
cparser_cmd_l34_get_arp(
    cparser_context_t *context)
{
    uint32 index;
    rtk_l34_arp_entry_t entry;
    int32   ret = RT_ERR_FAILED;
      
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    _cparser_cmd_l34_arp_header_display();
    for(index = 0; index <L34_ARP_TABLE_MAX_IDX; index++)
    {
        if((ret = rtk_l34_arpTable_get(index,&entry)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        _cparser_cmd_l34_arp_entry_display(index,&entry);        
            
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_arp */

/*
 * l34 get arp idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_arp_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    rtk_l34_arp_entry_t entry;
    int32   ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = rtk_l34_arpTable_get(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    _cparser_cmd_l34_arp_header_display();
    _cparser_cmd_l34_arp_entry_display(*table_idx_ptr,&entry);

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_arp_idx_table_idx */

/*
 * l34 set arp idx <UINT:arp_idx> nh_idx <UINT:nh_idx>
 */
cparser_result_t
cparser_cmd_l34_set_arp_idx_arp_idx_nh_idx_nh_idx(
    cparser_context_t *context,
    uint32_t  *arp_idx_ptr,
    uint32_t  *nh_idx_ptr)
{
    rtk_l34_arp_entry_t entry;
    int32   ret = RT_ERR_FAILED;
  
    DIAG_UTIL_PARAM_CHK();
    
    if(*arp_idx_ptr >= L34_ARP_TABLE_MAX_IDX) 
        DIAG_ERR_PRINT(RT_ERR_ENTRY_INDEX);
    
    entry.valid = 1;
    entry.nhIdx = *nh_idx_ptr;
    if((ret = rtk_l34_arpTable_set(*arp_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }        
        
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_arp_idx_arp_idx_nh_idx_nh_idx */

/*
 * l34 del arp idx <UINT:arp_idx>
 */
cparser_result_t
cparser_cmd_l34_del_arp_idx_arp_idx(
    cparser_context_t *context,
    uint32_t  *arp_idx_ptr)
{
    int32   ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    if(*arp_idx_ptr >= L34_ARP_TABLE_MAX_IDX) 
    {
        DIAG_ERR_PRINT(RT_ERR_ENTRY_INDEX);
        return CPARSER_NOT_OK;
    }

    if((ret = rtk_l34_arpTable_del(*arp_idx_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_del_arp_idx_arp_idx */




void _cparser_cmd_l34_external_ip_table_header_display(void)
{
    diag_util_mprintf("%-5s %-7s %-15s %-15s %-6s %-3s %-8s\n",
                                                             "Index",
                                                             "Valid",
                                                             "Ext_IP",
                                                             "Int_IP",
                                                             "TYPE",
                                                             "NHidx",
                                                             "priority");    
}    

void _cparser_cmd_l34_external_ip_table_display(uint32 index,rtk_l34_ext_intip_entry_t *entry)
{
    uint32  ip_tmp_buf[UTIL_IP_TMP_BUFFER_LENGTH];
    
    diag_util_printf("%-5d ",index);    
    
    
    if(entry->valid == 1)
    {
        /*valid*/
        diag_util_printf("%-7s ",DIAG_STR_VALID);      

        /*external IP address*/

        diag_util_ip2str((uint8*)ip_tmp_buf, (uint32)entry->extIpAddr);
        diag_util_printf("%-15s ",ip_tmp_buf);   

        diag_util_ip2str((uint8*)ip_tmp_buf, (uint32)entry->intIpAddr);
        diag_util_printf("%-15s ",ip_tmp_buf);   


        /*Table Type*/
        diag_util_printf("%-6s ",ext_type_Str[entry->type]);   
        
        /*next hop index*/
        diag_util_printf("%-8d ",entry->nhIdx);   
        
        /*priority*/
        if(0x1 == entry->prival)
            diag_util_mprintf("%-8d \n",entry->pri);   
        else
            diag_util_mprintf("%-8s \n",DIAG_STR_NO);   
          
    }
    else
    {
        diag_util_mprintf("%-7s \n",DIAG_STR_INVALID);      
    }
    
 
}



/*
 * l34 get external_ip 
 */
cparser_result_t
cparser_cmd_l34_get_external_ip(
    cparser_context_t *context)
{
    uint32_t index; /*tble index*/
    int32   ret = RT_ERR_FAILED;
    rtk_l34_ext_intip_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    _cparser_cmd_l34_external_ip_table_header_display();
    for(index = 0; index < L34_EXTIP_TABLE_MAX_IDX ; index++)
    {

        if((ret = rtk_l34_extIntIPTable_get(index,&entry)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        _cparser_cmd_l34_external_ip_table_display(index,&entry);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_external_ip */

/*
 * l34 get external_ip idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_external_ip_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    rtk_l34_ext_intip_entry_t entry;
    int32   ret = RT_ERR_FAILED;    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(*table_idx_ptr >= L34_EXTIP_TABLE_MAX_IDX) 
    {
        DIAG_ERR_PRINT(RT_ERR_ENTRY_INDEX);
        return CPARSER_NOT_OK;
    }
    if((ret = rtk_l34_extIntIPTable_get(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    _cparser_cmd_l34_external_ip_table_header_display();
    _cparser_cmd_l34_external_ip_table_display(*table_idx_ptr,&entry);

  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_external_ip_idx_table_idx */




cparser_result_t
_cparser_cmd_l34_set_external_ip_set(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *external_ip_ptr,
    uint32_t  *internal_ip_ptr,
    uint32_t  *nh_idx_ptr,
    int32_t  pri)
{
    rtk_l34_ext_intip_entry_t entry;
    int32   ret = RT_ERR_FAILED; 
    int32   index;
    DIAG_UTIL_PARAM_CHK();
    
    /*get index*/
    index = *table_idx_ptr;
    
    /*get type*/
    if('N' == TOKEN_CHAR(6,0))
    {
        if('T' == TOKEN_CHAR(6,2))    
            entry.type = L34_EXTIP_TYPE_NAT;            
        else
            entry.type = L34_EXTIP_TYPE_NAPT;            
        
    }
    else
    {
        entry.type = L34_EXTIP_TYPE_LP;            
    }

    /*get external IP address*/
    entry.extIpAddr = *external_ip_ptr;

    
    /*get internal IP address*/
    entry.intIpAddr = *internal_ip_ptr;

    /*get nexthop index*/
    entry.nhIdx = *nh_idx_ptr;
    
    /*get priority*/
    
    
    if(-1 == pri)
    {
        entry.prival = 0;
    }
    else
    {
        entry.prival = 1;
        entry.pri = pri;
    
    }     
    
    /*set vlid bit*/
    entry.valid = 1;
    
    
    if((ret = rtk_l34_extIntIPTable_set(index,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    return CPARSER_OK;
}    /* end of _cparser_cmd_l34_set_external_ip_set */


/*
 * l34 set external_ip idx <UINT:table_idx> type ( NAT | NAPT | LP ) external_ip <IPV4ADDR:external_ip> int_ip <IPV4ADDR:internal_ip> nh_idx <UINT:nh_idx> nat_prioirty enable priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_l34_set_external_ip_idx_table_idx_type_NAT_NAPT_LP_external_ip_external_ip_int_ip_internal_ip_nh_idx_nh_idx_nat_prioirty_enable_priority_priority(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *external_ip_ptr,
    uint32_t  *internal_ip_ptr,
    uint32_t  *nh_idx_ptr,
    uint32_t  *priority_ptr)
{
    uint32_t  pri;
    pri = *priority_ptr;
    
  
    return _cparser_cmd_l34_set_external_ip_set(context,
                                                table_idx_ptr,
                                                external_ip_ptr,
                                                internal_ip_ptr,
                                                nh_idx_ptr,
                                                (int32_t)pri);
}    /* end of cparser_cmd_l34_set_external_ip_idx_table_idx_type_nat_napt_lp_external_ip_external_ip_int_ip_internal_ip_nh_idx_nh_idx_nat_prioirty_enable_priority_priority */

/*
 * l34 set external_ip idx <UINT:table_idx> type ( NAT | NAPT | LP ) external_ip <IPV4ADDR:external_ip> int_ip <IPV4ADDR:internal_ip> nh_idx <UINT:nh_idx> nat_prioirty disable
 */
cparser_result_t
cparser_cmd_l34_set_external_ip_idx_table_idx_type_NAT_NAPT_LP_external_ip_external_ip_int_ip_internal_ip_nh_idx_nh_idx_nat_prioirty_disable(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *external_ip_ptr,
    uint32_t  *internal_ip_ptr,
    uint32_t  *nh_idx_ptr)
{
    return _cparser_cmd_l34_set_external_ip_set(context,
                                                table_idx_ptr,
                                                external_ip_ptr,
                                                internal_ip_ptr,
                                                nh_idx_ptr,
                                                -1);
}    /* end of cparser_cmd_l34_set_external_ip_idx_table_idx_type_nat_napt_lp_external_ip_external_ip_int_ip_internal_ip_nh_idx_nh_idx_nat_prioirty_disable */

/*
 * l34 del external_ip idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_del_external_ip_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    int32   ret = RT_ERR_FAILED; 
    DIAG_UTIL_PARAM_CHK();

    if((ret = rtk_l34_extIntIPTable_del((uint32_t)*table_idx_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_del_external_ip_idx_table_idx */




void _cparser_cmd_l34_routing_table_header_display(void)
{
    diag_util_mprintf("%-5s %-7s %-15s %-4s %-8s %-14s %-10s\n",
                                                             "Index",
                                                             "Valid",
                                                             "IP_Addr",
                                                             "MASK",
                                                             "Internal",
                                                             "ROUTE_TYPE",
                                                             "MISC_INFO");    
}    

void _cparser_cmd_l34_routing_table_display(uint32 index, rtk_l34_routing_entry_t *entry)
{
  
    diag_util_printf("%-5d ",index);    
    if(entry->valid == 1)
    {
        /*valid*/
        diag_util_printf("%-7s ",DIAG_STR_VALID);      

        /*ip address*/
        diag_util_printf("%-15s ",diag_util_inet_ntoa(entry->ipAddr));   


        /*mask*/
        diag_util_printf("%-4d ",entry->ipMask);   
        
        /*is internal*/
        if(entry->internal)
            diag_util_printf("%-8s ",DIAG_STR_YES);  
        else
            diag_util_printf("%-8s ",DIAG_STR_NO); 
        
        /*ROUTE_TYPE*/
        switch(entry->process)
        {
            case L34_PROCESS_CPU:
                diag_util_mprintf("%-14s \n",DIAG_STR_TRAP2CPU); 
                break;
            case L34_PROCESS_DROP:
                diag_util_mprintf("%-14s \n",DIAG_STR_DROP); 
                break;

            case L34_PROCESS_ARP:
                diag_util_printf("%-14s ",DIAG_STR_ROUTE_TYPE_LOCAL); 
                /*arp info*/
                diag_util_mprintf("netif_idx(%2d) arp_start(%3d) arp_end(%3d)\n",entry->netifIdx,entry->arpStart,entry->arpEnd); 
                break;
                
            case L34_PROCESS_NH:
                diag_util_printf("%-14s ",DIAG_STR_ROUTE_TYPE_GLOBAL); 
                /*next hop info*/
                diag_util_mprintf("nh_idx(%4d) nh_num(%2d) nh_algo(%1d) ip_doman_num(%d)\n",entry->nhStart,entry->nhNum,entry->nhAlgo,entry->ipDomain); 
               
                break;
            default:
                diag_util_mprintf("%-14s \n",DIAG_STR_INVALID); 
                break;

        }

    }
    else
    {
        diag_util_mprintf("%-7s \n",DIAG_STR_INVALID);      
    }
 
  
}



/*
 * l34 get routing 
 */
cparser_result_t
cparser_cmd_l34_get_routing(
    cparser_context_t *context)
{
    rtk_l34_routing_entry_t entry;
    int32 ret = RT_ERR_FAILED; 
    uint32 index;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    _cparser_cmd_l34_routing_table_header_display();

    for(index =0;index < L34_ROUTING_TABLE_MAX_IDX;index++)
    {
        memset(&entry,0x0,sizeof(rtk_l34_routing_entry_t));
        if((ret = rtk_l34_routingTable_get(index,&entry)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }     
        
        _cparser_cmd_l34_routing_table_display(index,&entry);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_routing */

/*
 * l34 get routing idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_routing_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    rtk_l34_routing_entry_t entry;
    int32 ret = RT_ERR_FAILED; 

        
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((*table_idx_ptr >= L34_ROUTING_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);   

    DIAG_UTIL_OUTPUT_INIT();

    memset(&entry,0x0,sizeof(rtk_l34_routing_entry_t));
    if((ret = rtk_l34_routingTable_get(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    _cparser_cmd_l34_routing_table_header_display();
    _cparser_cmd_l34_routing_table_display(*table_idx_ptr,&entry);
    

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_routing_idx_table_idx */

/*
 * l34 set routing idx <UINT:table_idx> ip <IPV4ADDR:routing_ip_addr> mask <UINT:ip_mask> ( internal | external ) type ( drop | trap )
 */
cparser_result_t
cparser_cmd_l34_set_routing_idx_table_idx_ip_routing_ip_addr_mask_ip_mask_internal_external_type_drop_trap(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *routing_ip_addr_ptr,
    uint32_t  *ip_mask_ptr)
{
    rtk_l34_routing_entry_t entry;
    int32   ret = RT_ERR_FAILED; 
    uint32   index;

    DIAG_UTIL_PARAM_CHK();
    
    RT_PARAM_CHK((*table_idx_ptr >= L34_ROUTING_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);   
    RT_PARAM_CHK((*ip_mask_ptr > L34_ROUTING_IP_MAX_MASK), CPARSER_ERR_INVALID_PARAMS);   
    
    memset(&entry,0x0,sizeof(rtk_l34_routing_entry_t));
    
    /*set to valid*/
    entry.valid = 1;
    
    /*get index*/
    index = *table_idx_ptr;

   
    /*IP address*/
    if((ret = diag_util_str2ip((uint32 *)&entry.ipAddr,(uint8 *)TOKEN_STR(6))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }


    /*IP Mask*/
    entry.ipMask = *ip_mask_ptr; 
    
    /*internal/external*/    
    if('i' == TOKEN_CHAR(9,0))
        entry.internal = 1; 
    else
        entry.internal = 0;

    /*PROCESS Type drop/trap*/    
    if('d' == TOKEN_CHAR(11,0))     
        entry.process = L34_PROCESS_DROP;
    else
        entry.process = L34_PROCESS_CPU;
    
    /*call API to set entry*/            
    if((ret = rtk_l34_routingTable_set(index,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_routing_idx_table_idx_ip_routing_ip_addr_mask_ip_mask_internal_external_type_drop_trap */

/*
 * l34 set routing idx <UINT:table_idx> ip <IPV4ADDR:routing_ip_addr> mask <UINT:ip_mask> ( internal | external ) type local_route dest_netif_idx <UINT:netif_idx> arp_start <UINT:arp_start_addr> arp_end <UINT:arp_end_addr>
 */
cparser_result_t
cparser_cmd_l34_set_routing_idx_table_idx_ip_routing_ip_addr_mask_ip_mask_internal_external_type_local_route_dest_netif_idx_netif_idx_arp_start_arp_start_addr_arp_end_arp_end_addr(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *routing_ip_addr_ptr,
    uint32_t  *ip_mask_ptr,
    uint32_t  *netif_idx_ptr,
    uint32_t  *arp_start_addr_ptr,
    uint32_t  *arp_end_addr_ptr)
{
    rtk_l34_routing_entry_t entry;
    int32   ret = RT_ERR_FAILED; 
    uint32   index;
    
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((*table_idx_ptr >= L34_ROUTING_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);   
    RT_PARAM_CHK((*ip_mask_ptr > L34_ROUTING_IP_MAX_MASK), CPARSER_ERR_INVALID_PARAMS);   

    memset(&entry,0x0,sizeof(rtk_l34_routing_entry_t));

    /*set to valid*/
    entry.valid = 1;
    
    /*get index*/
    index = *table_idx_ptr;
    
    /*IP address*/
    if((ret = diag_util_str2ip((uint32 *)&entry.ipAddr,(uint8 *)TOKEN_STR(6))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    /*IP Mask*/
    entry.ipMask = *ip_mask_ptr; 
    
    /*internal/external*/    
    if('i' == TOKEN_CHAR(9,0))
        entry.internal = 1; 
    else
        entry.internal = 0;

    /*PROCESS*/    
    entry.process = L34_PROCESS_ARP;

    /*NETIF index*/
    entry.netifIdx = *netif_idx_ptr;
    
    /*ARP start end*/
    entry.arpStart = *arp_start_addr_ptr;
    entry.arpEnd = *arp_end_addr_ptr;
    
    /*call APi to set entry*/            
    if((ret = rtk_l34_routingTable_set(index,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }     

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_routing_idx_table_idx_ip_routing_ip_addr_mask_ip_mask_internal_external_type_local_route_dest_netif_idx_netif_idx_arp_start_arp_start_addr_arp_end_arp_end_addr */

/*
 * l34 set routing idx <UINT:table_idx> ip <IPV4ADDR:routing_ip_addr> mask <UINT:ip_mask> ( internal | external ) type global_route nh_idx <UINT:nh_idx>
 */
cparser_result_t
cparser_cmd_l34_set_routing_idx_table_idx_ip_routing_ip_addr_mask_ip_mask_internal_external_type_global_route_nh_idx_nh_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *routing_ip_addr_ptr,
    uint32_t  *ip_mask_ptr,
    uint32_t  *nh_idx_ptr)
{
    rtk_l34_routing_entry_t entry;
    int32   ret = RT_ERR_FAILED; 
    uint32   index;
    
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((*table_idx_ptr >= L34_ROUTING_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);   
    RT_PARAM_CHK((*ip_mask_ptr > L34_ROUTING_IP_MAX_MASK), CPARSER_ERR_INVALID_PARAMS);   

    memset(&entry,0x0,sizeof(rtk_l34_routing_entry_t));

    /*set to valid*/
    entry.valid = 1;
    
    /*get index*/
    index = *table_idx_ptr;
    
    /*IP address*/
    if((ret = diag_util_str2ip((uint32 *)&entry.ipAddr,(uint8 *)TOKEN_STR(6))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    /*IP Mask*/
    entry.ipMask = *ip_mask_ptr; 
    
    /*internal/external*/    
    if('i' == TOKEN_CHAR(9,0))
        entry.internal = 1; 
    else
        entry.internal = 0;

    /*PROCESS*/    
    entry.process = L34_PROCESS_NH;

    /*next hop index*/
    entry.nhStart = *nh_idx_ptr;
    
    /*do not support this setting we set to default*/
    entry.ipDomain = 6;
    entry.nhNum = 1;
    entry.nhAlgo = 0;
    
    /*call APi to set entry*/            
    if((ret = rtk_l34_routingTable_set(index,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_routing_idx_table_idx_ip_routing_ip_addr_mask_ip_mask_internal_external_type_global_route_nh_idx_nh_idx */

/*
 * l34 set routing idx <UINT:table_idx> ip <IPV4ADDR:routing_ip_addr> mask <UINT:ip_mask> ( internal | external ) type global_route nh_start_idx <UINT:nh_idx> nh_number <UINT:nh_number> nh_algo ( per_packet | per_session | per_source_ip ) ip_domain_range <UINT:domain_range>
 */
cparser_result_t
cparser_cmd_l34_set_routing_idx_table_idx_ip_routing_ip_addr_mask_ip_mask_internal_external_type_global_route_nh_start_idx_nh_idx_nh_number_nh_number_nh_algo_per_packet_per_session_per_source_ip_ip_domain_range_domain_range(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *routing_ip_addr_ptr,
    uint32_t  *ip_mask_ptr,
    uint32_t  *nh_idx_ptr,
    uint32_t  *nh_number_ptr,
    uint32_t  *domain_range_ptr)
{
    rtk_l34_routing_entry_t entry;
    int32   ret = RT_ERR_FAILED; 
    uint32   index;
    
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((*table_idx_ptr >= L34_ROUTING_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);   
    RT_PARAM_CHK((*ip_mask_ptr > L34_ROUTING_IP_MAX_MASK), CPARSER_ERR_INVALID_PARAMS);   

    memset(&entry,0x0,sizeof(rtk_l34_routing_entry_t));

    /*set to valid*/
    entry.valid = 1;
    
    /*get index*/
    index = *table_idx_ptr;
    
    /*IP address*/
    if((ret = diag_util_str2ip((uint32 *)&entry.ipAddr,(uint8 *)TOKEN_STR(6))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*IP Mask*/
    entry.ipMask = *ip_mask_ptr; 
    
    /*internal/external*/    
    if('i' == TOKEN_CHAR(9,0))
        entry.internal = 1; 
    else
        entry.internal = 0;

    /*PROCESS*/    
    entry.process = L34_PROCESS_NH;

    /*next hop index*/
    entry.nhStart = *nh_idx_ptr;
    
    entry.ipDomain = *domain_range_ptr;
    entry.nhNum = *nh_number_ptr;
    
    
    switch(TOKEN_CHAR(17,5))
    {
        case 'a':
            entry.nhAlgo = 0;
            break;
        case 'e':
            entry.nhAlgo = 1;
            break;
        case 'o':
            entry.nhAlgo = 2;
            break;
        default:
            return CPARSER_NOT_OK;
    }
    /*call APi to set entry*/            
    if((ret = rtk_l34_routingTable_set(index,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }  
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_routing_idx_table_idx_ip_routing_ip_addr_mask_ip_mask_internal_external_type_global_route_nh_start_idx_nh_idx_nh_number_nh_number_nh_algo_per_packet_per_session_per_source_ip_ip_domain_range_domain_range */

/*
 * l34 del routing idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_del_routing_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    int32   ret = RT_ERR_FAILED; 
    DIAG_UTIL_PARAM_CHK();

    if((ret = rtk_l34_routingTable_del((uint32_t)*table_idx_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_del_routing_idx_table_idx */








void _cparser_cmd_l34_netif_table_header_display(void)
{
    diag_util_mprintf("%-5s %-7s %-7s %-4s %-8s %-17s %-4s\n",
                                                             "Index",
                                                             "Valid",
                                                             "Vlan ID",
                                                             "MTU",
                                                             "EN_ROUTE",
                                                             "GATEWAY_MAC",
                                                             "MASK");    
}    

void _cparser_cmd_l34_netif_table_display(uint32 index,rtk_l34_netif_entry_t *entry)
{
  
    diag_util_printf("%-5d ",index);    
    if(entry->valid == 1)
    {
        /*valid*/
        diag_util_printf("%-7s ",DIAG_STR_VALID);      

        /*vlan id*/
        diag_util_printf("%-7d ",entry->vlan_id);   


        /*MTU*/
        diag_util_printf("%-4d ",entry->mtu);   
        
        /*EN_ROUTE*/
        if(entry->enable_rounting)
            diag_util_printf("%-8s ",DIAG_STR_YES);  
        else
            diag_util_printf("%-8s ",DIAG_STR_NO); 
        
        /*GATEWAY_MAC*/
        diag_util_printf("%-17s ",diag_util_inet_mactoa((uint8 *)&entry->gateway_mac));   
        
        /*MASK*/
        diag_util_mprintf("%-4d \n",entry->mac_mask); 
    }
    else
    {
        diag_util_mprintf("%-7s \n",DIAG_STR_INVALID);      
    }
 
  
}

/*
 * l34 get netif 
 */
cparser_result_t
cparser_cmd_l34_get_netif(
    cparser_context_t *context)
{
    uint32 index;
    int32   ret = RT_ERR_FAILED;
    rtk_l34_netif_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    _cparser_cmd_l34_netif_table_header_display();
    
    for(index = 0; index < L34_NETIF_TABLE_MAX_IDX ; index++)
    {

        if((ret = rtk_l34_netifTable_get(index,&entry)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        _cparser_cmd_l34_netif_table_display(index,&entry);
    }    
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_netif */






/*
 * l34 get netif idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_netif_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{

    int32   ret = RT_ERR_FAILED;
    rtk_l34_netif_entry_t entry;
  

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if((ret = rtk_l34_netifTable_get(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    _cparser_cmd_l34_netif_table_header_display();
    _cparser_cmd_l34_netif_table_display(*table_idx_ptr,&entry);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_netif_idx_table_idx */

/*
 * l34 set netif idx <UINT:table_idx> gateway_mac <MACADDR:mac> mac_mask ( no_mask | 1bit_mask | 2bit_mask | 3bit_mask ) vlan_id <UINT:vid> mtu <UINT:mtu> ( enable_routing | disable_routing )
 */
cparser_result_t
cparser_cmd_l34_set_netif_idx_table_idx_gateway_mac_mac_mac_mask_no_mask_1bit_mask_2bit_mask_3bit_mask_vlan_id_vid_mtu_mtu_enable_routing_disable_routing(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *vid_ptr,
    uint32_t  *mtu_ptr)
{
    int32   ret = RT_ERR_FAILED;
    rtk_l34_netif_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_netif_entry_t));
    
    /*valid bit*/
    entry.valid = 1;
  
    /*gateway mac*/
    osal_memcpy((uint8 *)&entry.gateway_mac,(uint8 *)mac_ptr,sizeof(cparser_macaddr_t));
    
    /*vid*/
    entry.vlan_id = *vid_ptr;
    
    /*mtu*/
    entry.mtu = *mtu_ptr;
    
    
    /*mask*/
    switch(TOKEN_CHAR(8,0))
    {
        case 'n':
               entry.mac_mask = 0x7; 
            break;    
        case '1':
               entry.mac_mask = 0x6; 
            break;    
        case '2':
               entry.mac_mask = 0x4; 
            break;    
        case '3':
               entry.mac_mask = 0x0; 
            break;    
        default:
               DIAG_ERR_PRINT(RT_ERR_INPUT); 
               return CPARSER_NOT_OK;
    }

    /*en_routing*/
    if('e' == TOKEN_CHAR(13,0))
        entry.enable_rounting = 1;
    else
        entry.enable_rounting = 0; 

    if((ret = rtk_l34_netifTable_set(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
        

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_netif_idx_table_idx_gateway_mac_mac_mac_mask_no_mask_1bit_mask_2bit_mask_3bit_mask_vlan_id_vid_mtu_mtu_enable_routing_disable_routing */

/*
 * l34 del netif idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_del_netif_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    int32   ret = RT_ERR_FAILED;
    rtk_l34_netif_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_netif_entry_t));
    
    entry.valid = 0;
    
    if((ret = rtk_l34_netifTable_set(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_del_netif_idx_table_idx */






void _cparser_cmd_l34_nexthop_table_header_display(void)
{
    diag_util_mprintf("%-5s %-8s %-9s %-9s %-9s\n",
                      "Index",
                      "TYPE",
                      "NH_L2_IDX",
                      "IF_IDX",
                      "PPPoE_IDX");    
}    

void _cparser_cmd_l34_nexthop_table_display(uint32 index,rtk_l34_nexthop_entry_t *entry)
{
    /*index*/  
    diag_util_printf("%-5d ",index);    

    /*type*/
    diag_util_printf("%-8s ",diagStr_l34NexthopTypeStr[entry->type]);   

    /*NH_L2_IDX*/
    diag_util_printf("%-9d ",entry->nhIdx);   
        
    /*IF_IDX*/
    diag_util_printf("%-9d ",entry->ifIdx);   

    /*PPPoE_IDX*/
    diag_util_mprintf("%-9d \n",entry->pppoeIdx);   
   
  
}




/*
 * l34 get nexthop 
 */
cparser_result_t
cparser_cmd_l34_get_nexthop(
    cparser_context_t *context)
{
    uint32 index;
    int32   ret = RT_ERR_FAILED;
    rtk_l34_nexthop_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    _cparser_cmd_l34_nexthop_table_header_display();
    
    for(index = 0; index < L34_NH_TABLE_MAX_IDX ; index++)
    {

        if((ret = rtk_l34_nexthopTable_get(index,&entry)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        _cparser_cmd_l34_nexthop_table_display(index,&entry);
    }    
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_nexthop */

/*
 * l34 get nexthop idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_nexthop_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    uint32 index;
    int32   ret = RT_ERR_FAILED;
    rtk_l34_nexthop_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    

    if((ret = rtk_l34_nexthopTable_get(index, &entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    _cparser_cmd_l34_nexthop_table_header_display();
        
    _cparser_cmd_l34_nexthop_table_display(index,&entry);
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_nexthop_idx_table_idx */

/*
 * l34 set nexthop idx <UINT:table_idx> netif <UINT:netif_idx> l2_idx <UINT:l2_idx> type pppoe pppoe_idx <UINT:pppoe_idx>
 */
cparser_result_t
cparser_cmd_l34_set_nexthop_idx_table_idx_netif_netif_idx_l2_idx_l2_idx_type_pppoe_pppoe_idx_pppoe_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *netif_idx_ptr,
    uint32_t  *l2_idx_ptr,
    uint32_t  *pppoe_idx_ptr)
{
    int32   ret = RT_ERR_FAILED;
    rtk_l34_nexthop_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_nexthop_entry_t));
    
    /*if index*/
    entry.ifIdx = *netif_idx_ptr;
  
    entry.type = L34_NH_PPPOE;

    entry.nhIdx = *l2_idx_ptr;
    
    entry.pppoeIdx = *pppoe_idx_ptr;
    
    if((ret = rtk_l34_nexthopTable_set(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_nexthop_idx_table_idx_netif_netif_idx_l2_idx_l2_idx_type_pppoe_pppoe_idx_pppoe_idx */

/*
 * l34 set nexthop idx <UINT:table_idx> netif <UINT:netif_idx> l2_idx <UINT:l2_idx> type ethernet
 */
cparser_result_t
cparser_cmd_l34_set_nexthop_idx_table_idx_netif_netif_idx_l2_idx_l2_idx_type_ethernet(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *netif_idx_ptr,
    uint32_t  *l2_idx_ptr)
{
    int32   ret = RT_ERR_FAILED;
    rtk_l34_nexthop_entry_t entry;

    DIAG_UTIL_PARAM_CHK();
   
    osal_memset(&entry,0x0,sizeof(rtk_l34_nexthop_entry_t));
    
    /*if index*/
    entry.ifIdx = *netif_idx_ptr;
  
    entry.type = L34_NH_ETHER;

    entry.nhIdx = *l2_idx_ptr;
    
    /*set to default*/
    entry.pppoeIdx = 0;
    
    if((ret = rtk_l34_nexthopTable_set(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}  /* end of cparser_cmd_l34_set_nexthop_idx_table_idx_netif_netif_idx_l2_idx_l2_idx_type_ethernet */




void _cparser_cmd_l34_pppoe_table_header_display(void)
{
    diag_util_mprintf("%-5s %-12s\n",
                      "Index",
                      "Session_ID");    
}    

void _cparser_cmd_l34_pppoe_table_display(uint32 index,rtk_l34_pppoe_entry_t *entry)
{
    /*index*/  
    diag_util_printf("%-5d ",index);    

    /*PPPoE_IDX*/
    diag_util_mprintf("%-12d \n",entry->sessionID);   
}

/*
 * l34 get pppoe 
 */
cparser_result_t
cparser_cmd_l34_get_pppoe(
    cparser_context_t *context)
{
    uint32 index;
    int32   ret = RT_ERR_FAILED;
    rtk_l34_pppoe_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    _cparser_cmd_l34_pppoe_table_header_display();
    
    for(index = 0; index < L34_PPPOE_TABLE_MAX_IDX ; index++)
    {

        if((ret = rtk_l34_pppoeTable_get(index,&entry)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        _cparser_cmd_l34_pppoe_table_display(index,&entry);
    }    
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_pppoe */



/*
 * l34 get pppoe idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_pppoe_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    uint32 index;
    int32 ret = RT_ERR_FAILED;
    rtk_l34_pppoe_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    if((ret = rtk_l34_pppoeTable_get(index,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    _cparser_cmd_l34_pppoe_table_header_display();
    _cparser_cmd_l34_pppoe_table_display(index,&entry);
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_pppoe_idx_table_idx */

/*
 * l34 set pppoe idx <UINT:table_idx> session_id <UINT:session_id>
 */
cparser_result_t
cparser_cmd_l34_set_pppoe_idx_table_idx_session_id_session_id(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *session_id_ptr)
{
    int32   ret = RT_ERR_FAILED;
    rtk_l34_pppoe_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_pppoe_entry_t));
    
    /*session*/
    entry.sessionID = *session_id_ptr;
  
    if((ret = rtk_l34_pppoeTable_set(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_pppoe_idx_table_idx_session_id_session_id */





void _cparser_cmd_l34_napt_table_header_display(void)
{
    diag_util_mprintf("%-5s %-7s %-8s %-9s %-3s\n",
                      "Index",
                      "Valid",
                      "Hash_IDX",
                      "Pri_valid",
                      "Pri");    
}    

void _cparser_cmd_l34_napt_table_display(uint32 index,rtk_l34_naptOutbound_entry_t *entry)
{
    diag_util_printf("%-5d ",index); 
    
    if(entry->valid == 1)
    {
        /*valid*/
        diag_util_printf("%-7s ",DIAG_STR_VALID);      

        /*hashIdx*/
        diag_util_printf("%-8d ",entry->hashIdx);   

        /*pri valid*/
        if(entry->priValid == 1)
        {
            diag_util_printf("%-9s ",DIAG_STR_VALID);      
            /*pri*/
            diag_util_mprintf("%-8d \n",entry->priValue);   
        }
        else
        {
            diag_util_mprintf("%-9s \n",DIAG_STR_INVALID);      
        }
    }
    else
    {
        diag_util_mprintf("%-7s \n",DIAG_STR_INVALID);      
    } 
}


/*
 * l34 get napt 
 */
cparser_result_t
cparser_cmd_l34_get_napt(
    cparser_context_t *context)
{
    uint32 index;
    int32   ret = RT_ERR_FAILED;
    rtk_l34_naptOutbound_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    _cparser_cmd_l34_napt_table_header_display();
    
    for(index = 0; index < L34_NAPT_TABLE_MAX_IDX ; index++)
    {

        if((ret = rtk_l34_naptOutboundTable_get(index,&entry)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        _cparser_cmd_l34_napt_table_display(index,&entry);
    }    
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_napt */

/*
 * l34 get napt idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_napt_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    int32   ret = RT_ERR_FAILED;
    rtk_l34_naptOutbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptOutbound_entry_t));
    
  
    if((ret = rtk_l34_naptOutboundTable_get(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    _cparser_cmd_l34_napt_table_header_display();
    _cparser_cmd_l34_napt_table_display(*table_idx_ptr,&entry);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_napt_idx_table_idx */

/*
 * l34 set napt idx <UINT:table_idx> hash_idx <UINT:idx> napt_priority disable
 */
cparser_result_t
cparser_cmd_l34_set_napt_idx_table_idx_hash_idx_idx_napt_priority_disable(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *idx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int8 forced = 1;
    rtk_l34_naptOutbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptOutbound_entry_t));
    
    /*valid*/
    entry.valid = 1;

    /*hashIdx*/
    entry.hashIdx = *idx_ptr;


    /*priValid*/
    entry.priValid = 0;

  
    if((ret = rtk_l34_naptOutboundTable_set(forced,*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_napt_idx_table_idx_hash_idx_idx_napt_priority_disable */

/*
 * l34 set napt idx <UINT:table_idx> hash_idx <UINT:idx> napt_priority enable priority <UINT:priority> 
 */
cparser_result_t
cparser_cmd_l34_set_napt_idx_table_idx_hash_idx_idx_napt_priority_enable_priority_priority(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *idx_ptr,
    uint32_t  *priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int8 forced = 1;
    rtk_l34_naptOutbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptOutbound_entry_t));
    
    /*valid*/
    entry.valid = 1;

    /*hashIdx*/
    entry.hashIdx = *idx_ptr;


    /*priValid*/
    entry.priValid = 1;

    /*priValid*/
    entry.priValue = *priority_ptr;

  
    if((ret = rtk_l34_naptOutboundTable_set(forced,*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_napt_idx_table_idx_hash_idx_idx_napt_priority_enable_priority_priority */

/*
 * l34 del napt idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_del_napt_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int8 forced = 1;
    rtk_l34_naptOutbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptOutbound_entry_t));
    
    /*valid*/
    entry.valid = 0;


   if((ret = rtk_l34_naptOutboundTable_set(forced,*table_idx_ptr,&entry)) != RT_ERR_OK)
   {
       DIAG_ERR_PRINT(ret);
       return CPARSER_NOT_OK;
   }
   return CPARSER_OK;
}    /* end of cparser_cmd_l34_del_napt_idx_table_idx */





void _cparser_cmd_l34_naptr_table_header_display(void)
{
    diag_util_mprintf("%-5s %-7s %-15s %-13s %-6s %-10s %-4s %-5s %-9s %-3s\n",
                      "Index",
                      "Valid",
                      "Internal_IP",
                      "Internal_Port",
                      "R_HASH",
                      "EXT_IP_IDX",
                      "Plsb",
                      "Proto",
                      "Pri_valid",
                      "Pri");    
}    



void _cparser_cmd_l34_naptr_table_display(uint32 index,rtk_l34_naptInbound_entry_t *entry)
{
    uint32  ip_tmp_buf[UTIL_IP_TMP_BUFFER_LENGTH];

    diag_util_printf("%-5d ",index); 
    
    if(entry->valid != L34_NAPTR_ENTRY_INVALID)
    {
        /*valid*/
        diag_util_printf("%-7d ",entry->valid);      

        /*internal IP*/
        diag_util_ip2str((uint8*)ip_tmp_buf, (uint32)entry->intIp);
        diag_util_printf("%-15s ",ip_tmp_buf);   

        /*internal port*/
        diag_util_printf("%-13d ",entry->intPort);   

        /*R_HASH*/
        if(L34_NAPTR_ENTRY_NO_HASH == entry->valid)    
            diag_util_printf("%-6s ",DIAG_STR_NONE);   
        else
            diag_util_printf("0x%-4.4x ",entry->remHash);   

        /*EXT_IP_IDX*/
        diag_util_printf("%-10d ",entry->extIpIdx);   

        /*EXT_Port_LSB*/
        diag_util_printf("0x%2.2x ",entry->extPortLSB);   

        /*isTcp*/
        if(1 == entry->isTcp)
            diag_util_printf("%-5s ",DIAG_STR_TCP);   
        else
            diag_util_printf("%-5s ",DIAG_STR_UDP);   
        

        /*pri valid*/
        if(1 == entry->priValid)
        {
            diag_util_printf("%-9s ",DIAG_STR_VALID);      
            /*pri*/
            diag_util_mprintf("%-3d \n",entry->priId);   
        }
        else
        {
            diag_util_mprintf("%-9s \n",DIAG_STR_INVALID);      
        }
    }
    else
    {
        diag_util_mprintf("%-7s \n",DIAG_STR_INVALID);      
    } 
}





/*
 * l34 get naptr 
 */
cparser_result_t
cparser_cmd_l34_get_naptr(
    cparser_context_t *context)
{
    uint32 index;
    int32   ret = RT_ERR_FAILED;
    rtk_l34_naptInbound_entry_t entry;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    _cparser_cmd_l34_naptr_table_header_display();
    
    for(index = 0; index < L34_NAPTR_TABLE_MAX_IDX ; index++)
    {

        if((ret = rtk_l34_naptInboundTable_get(index,&entry)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        _cparser_cmd_l34_naptr_table_display(index,&entry);
    }    
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_naptr */

/*
 * l34 get naptr idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_naptr_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    int32   ret = RT_ERR_FAILED;
    rtk_l34_naptInbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptInbound_entry_t));
    
  
    if((ret = rtk_l34_naptInboundTable_get(*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    _cparser_cmd_l34_naptr_table_header_display();
    _cparser_cmd_l34_naptr_table_display(*table_idx_ptr,&entry);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_naptr_idx_table_idx */



/*
 * l34 set naptr idx <UINT:table_idx> int_ip <IPV4ADDR:ip_addr> int_port <UINT:int_port> protocol ( tcp | udp ) ext_ip_idx <UINT:ext_ip_idx> export_lsb <UINT:export_lsb_idx> remote_hash_type none naptr_priority enable priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_l34_set_naptr_idx_table_idx_int_ip_ip_addr_int_port_int_port_protocol_tcp_udp_ext_ip_idx_ext_ip_idx_export_lsb_export_lsb_idx_remote_hash_type_none_naptr_priority_enable_priority_priority(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *int_port_ptr,
    uint32_t  *ext_ip_idx_ptr,
    uint32_t  *export_lsb_idx_ptr,
    uint32_t  *priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int8 forced = 1;
    rtk_l34_naptInbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptInbound_entry_t));
    
    /*error check for valid type*/
    
    /*valid*/
    entry.valid = L34_NAPTR_ENTRY_NO_HASH;

    /*IP address*/
    if((ret = diag_util_str2ip((uint32 *)&entry.intIp,(uint8 *)TOKEN_STR(6))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    /*internal port*/
    entry.intPort = *int_port_ptr;

    /*EXT_IP_IDX*/
    entry.extIpIdx = *ext_ip_idx_ptr;

    /*isTcp*/
    if('t' ==TOKEN_CHAR(10,0))
        entry.isTcp = 1;   
    else
        entry.isTcp = 0;    

    /*pri valid*/
    entry.priValid = 1;
    entry.priId = *priority_ptr;   

     /*R_HASH*/
    entry.remHash = 0x0;

    /*EXT_Port_LSB*/
    entry.extPortLSB = *export_lsb_idx_ptr;
    
  
    if((ret = rtk_l34_naptInboundTable_set(forced,*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_naptr_idx_table_idx_int_ip_ip_addr_int_port_int_port_protocol_tcp_udp_ext_ip_idx_ext_ip_idx_export_lsb_export_lsb_idx_remote_hash_type_none_naptr_priority_enable_priority_priority */

/*
 * l34 set naptr idx <UINT:table_idx> int_ip <IPV4ADDR:ip_addr> int_port <UINT:int_port> protocol ( tcp | udp ) ext_ip_idx <UINT:ext_ip_idx> export_lsb <UINT:export_lsb_idx> remote_hash_type none naptr_priority disable 
 */
cparser_result_t
cparser_cmd_l34_set_naptr_idx_table_idx_int_ip_ip_addr_int_port_int_port_protocol_tcp_udp_ext_ip_idx_ext_ip_idx_export_lsb_export_lsb_idx_remote_hash_type_none_naptr_priority_disable(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *int_port_ptr,
    uint32_t  *ext_ip_idx_ptr,
    uint32_t  *export_lsb_idx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int8 forced = 1;
    rtk_l34_naptInbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptInbound_entry_t));
    
    /*error check for valid type*/
    
    /*valid*/
    entry.valid = L34_NAPTR_ENTRY_NO_HASH;

    /*IP address*/
    if((ret = diag_util_str2ip((uint32 *)&entry.intIp,(uint8 *)TOKEN_STR(6))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    /*internal port*/
    entry.intPort = *int_port_ptr;

    /*EXT_IP_IDX*/
    entry.extIpIdx = *ext_ip_idx_ptr;

    /*isTcp*/
    if('t' ==TOKEN_CHAR(10,0))
        entry.isTcp = 1;   
    else
        entry.isTcp = 0;    

    /*pri valid*/
    entry.priValid = 0;
    entry.priId = 0;   

     /*R_HASH*/
    entry.remHash = 0x0;

    /*EXT_Port_LSB*/
    entry.extPortLSB = *export_lsb_idx_ptr;
    
  
    if((ret = rtk_l34_naptInboundTable_set(forced,*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_naptr_idx_table_idx_int_ip_ip_addr_int_port_int_port_protocol_tcp_udp_ext_ip_idx_ext_ip_idx_export_lsb_export_lsb_idx_remote_hash_type_none_naptr_priority_disable */

/*
 * l34 set naptr idx <UINT:table_idx> int_ip <IPV4ADDR:ip_addr> int_port <UINT:int_port> protocol ( tcp | udp ) ext_ip_idx <UINT:ext_ip_idx> export_lsb <UINT:export_lsb_idx> remote_hash_type ( remote_ip | remote_ip_remote_port ) hash_value <UINT:remote_hash> naptr_priority enable priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_l34_set_naptr_idx_table_idx_int_ip_ip_addr_int_port_int_port_protocol_tcp_udp_ext_ip_idx_ext_ip_idx_export_lsb_export_lsb_idx_remote_hash_type_remote_ip_remote_ip_remote_port_hash_value_remote_hash_naptr_priority_enable_priority_priority(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *int_port_ptr,
    uint32_t  *ext_ip_idx_ptr,
    uint32_t  *export_lsb_idx_ptr,
    uint32_t  *remote_hash_ptr,
    uint32_t  *priority_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int8 forced = 1;
    rtk_l34_naptInbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptInbound_entry_t));
    
    /*error check for valid type*/
    
    /*valid*/

    
    if(strcmp(TOKEN_STR(16),"remote_ip_remote_port")==0)
        entry.valid = L34_NAPTR_ENTRY_IP_PORT_HASH; 
    else
        entry.valid = L34_NAPTR_ENTRY_IP_HASH; 
 
    /*IP address*/
    if((ret = diag_util_str2ip((uint32 *)&entry.intIp,(uint8 *)TOKEN_STR(6))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    /*internal port*/
    entry.intPort = *int_port_ptr;

    /*EXT_IP_IDX*/
    entry.extIpIdx = *ext_ip_idx_ptr;

    /*isTcp*/
    if('t' ==TOKEN_CHAR(10,0))
        entry.isTcp = 1;   
    else
        entry.isTcp = 0;    

    /*pri valid*/
    entry.priValid = 1;
    entry.priId = *priority_ptr;   

     /*R_HASH*/
    entry.remHash = *remote_hash_ptr;

    /*EXT_Port_LSB*/
    entry.extPortLSB = *export_lsb_idx_ptr;
    
  
    if((ret = rtk_l34_naptInboundTable_set(forced,*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_naptr_idx_table_idx_int_ip_ip_addr_int_port_int_port_protocol_tcp_udp_ext_ip_idx_ext_ip_idx_export_lsb_export_lsb_idx_remote_hash_type_remote_ip_remote_ip_remote_port_hash_value_remote_hash_naptr_priority_enable_priority_priority */

/*
 * l34 set naptr idx <UINT:table_idx> int_ip <IPV4ADDR:ip_addr> int_port <UINT:int_port> protocol ( tcp | udp ) ext_ip_idx <UINT:ext_ip_idx> export_lsb <UINT:export_lsb_idx> remote_hash_type ( remote_ip | remote_ip_remote_port ) hash_value <UINT:remote_hash> naptr_priority disable
 */
cparser_result_t
cparser_cmd_l34_set_naptr_idx_table_idx_int_ip_ip_addr_int_port_int_port_protocol_tcp_udp_ext_ip_idx_ext_ip_idx_export_lsb_export_lsb_idx_remote_hash_type_remote_ip_remote_ip_remote_port_hash_value_remote_hash_naptr_priority_disable(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *int_port_ptr,
    uint32_t  *ext_ip_idx_ptr,
    uint32_t  *export_lsb_idx_ptr,
    uint32_t  *remote_hash_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int8 forced = 1;
    rtk_l34_naptInbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptInbound_entry_t));
    
    /*error check for valid type*/
    
    /*valid*/
    if(strcmp(TOKEN_STR(16),"remote_ip_remote_port")==0)
        entry.valid = L34_NAPTR_ENTRY_IP_PORT_HASH; 
    else
        entry.valid = L34_NAPTR_ENTRY_IP_HASH; 
 
    /*IP address*/
    if((ret = diag_util_str2ip((uint32 *)&entry.intIp,(uint8 *)TOKEN_STR(6))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    /*internal port*/
    entry.intPort = *int_port_ptr;

    /*EXT_IP_IDX*/
    entry.extIpIdx = *ext_ip_idx_ptr;

    /*isTcp*/
    if('t' ==TOKEN_CHAR(10,0))
        entry.isTcp = 1;   
    else
        entry.isTcp = 0;    

    /*pri valid*/
    entry.priValid = 0;
    entry.priId = 0;   

     /*R_HASH*/
    entry.remHash = *remote_hash_ptr;

    /*EXT_Port_LSB*/
    entry.extPortLSB = *export_lsb_idx_ptr;
    
  
    if((ret = rtk_l34_naptInboundTable_set(forced,*table_idx_ptr,&entry)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_naptr_idx_table_idx_int_ip_ip_addr_int_port_int_port_protocol_tcp_udp_ext_ip_idx_ext_ip_idx_export_lsb_export_lsb_idx_remote_hash_type_remote_ip_remote_ip_remote_port_hash_value_remote_hash_naptr_priority_disable */

/*
 * l34 del naptr idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_del_naptr_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int8 forced = 1;
    rtk_l34_naptInbound_entry_t entry;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry,0x0,sizeof(rtk_l34_naptInbound_entry_t));
    
    /*valid*/
    entry.valid = 0;


   if((ret = rtk_l34_naptInboundTable_set(forced,*table_idx_ptr,&entry)) != RT_ERR_OK)
   {
       DIAG_ERR_PRINT(ret);
       return CPARSER_NOT_OK;
   }
   return CPARSER_OK;
}    /* end of cparser_cmd_l34_del_naptr_idx_table_idx */

/*
 * l34 set port ( <PORT_LIST:port> | all ) netif_idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_set_port_port_all_netif_idx_table_idx(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *table_idx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK((*table_idx_ptr > L34_NETIF_TABLE_MAX_IDX), ret);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l34_portIntfIdx_set(port, (uint8)*table_idx_ptr), ret);
    }    


    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_port_port_all_netif_idx_table_idx */

/*
 * l34 get port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l34_get_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint8 if_idx;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("port ifidx\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l34_portIntfIdx_get(port, &if_idx), ret);
        diag_util_mprintf("%-4d %-5d\n",port,if_idx);
    }    
    

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_port_port_all */

/*
 * l34 set ext_port ( <PORT_LIST:port> | ext_all ) netif_idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_set_ext_port_port_ext_all_netif_idx_table_idx(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *table_idx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK((*table_idx_ptr > L34_NETIF_TABLE_MAX_IDX), ret);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l34_extPortIntfIdx_set(port, (uint8)*table_idx_ptr), ret);
    }    


    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_ext_port_port_ext_all_netif_idx_table_idx */

/*
 * l34 get ext_port ( <PORT_LIST:port> | ext_all )
 */
cparser_result_t
cparser_cmd_l34_get_ext_port_port_ext_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint8 if_idx;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("port ifidx\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l34_extPortIntfIdx_get(port, &if_idx), ret);
        diag_util_mprintf("%-4d %-5d\n",port,if_idx);
    }    

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_ext_port_port_ext_all */

/*
 * l34 set vc_port ( <PORT_LIST:port> | dsl_all ) netif_idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_set_vc_port_port_dsl_all_netif_idx_table_idx(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *table_idx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK((*table_idx_ptr > L34_NETIF_TABLE_MAX_IDX), ret);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l34_vcPortIntfIdx_set(port, (uint8)*table_idx_ptr), ret);
    }    

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_vc_port_port_dsl_all_netif_idx_table_idx */

/*
 * l34 get vc_port ( <PORT_LIST:port> | dsl_all )
 */
cparser_result_t
cparser_cmd_l34_get_vc_port_port_dsl_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint8 if_idx;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("port ifidx\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l34_vcPortIntfIdx_get(port, &if_idx), ret);
        diag_util_mprintf("%-4d %-5d\n",port,if_idx);
    }    

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_vc_port_port_dsl_all */

/*
 * l34 set l4_fragment ( trap_to_cpu | forward )
 */
cparser_result_t
cparser_cmd_l34_set_l4_fragment_trap_to_cpu_forward(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    if('t' == TOKEN_CHAR(3,0))
        mode = ENABLED;
    else
        mode = DISABLED;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_l34_FragPkt2Cpu_set(mode), ret);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_l4_fragment_trap_to_cpu_forward */

/*
 * l34 get l4_fragment
 */
cparser_result_t
cparser_cmd_l34_get_l4_fragment(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_FragPkt2Cpu_get(&mode), ret);

    diag_util_mprintf("L4 Fragment to CPU:%s\n",diagStr_enable[mode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_l4_fragment */

/*
 * l34 set l3_checksum_error ( forward | drop )
 */
cparser_result_t
cparser_cmd_l34_set_l3_checksum_error_forward_drop(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    if('f' == TOKEN_CHAR(3,0))
        mode = ENABLED;
    else
        mode = DISABLED;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_l34_L3chksumErrAllow_set(mode), ret);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_l3_checksum_error_forward_drop */

/*
 * l34 get l3_checksum_error
 */
cparser_result_t
cparser_cmd_l34_get_l3_checksum_error(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_L3chksumErrAllow_get(&mode), ret);

    diag_util_mprintf("L3 checksum error forward:%s\n",diagStr_enable[mode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_l3_checksum_error */

/*
 * l34 set l4_checksum_error ( forward | drop )
 */
cparser_result_t
cparser_cmd_l34_set_l4_checksum_error_forward_drop(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    if('f' == TOKEN_CHAR(3,0))
        mode = ENABLED;
    else
        mode = DISABLED;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_l34_L4chksumErrAllow_set(mode), ret);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_l4_checksum_error_forward_drop */

/*
 * l34 get l4_checksum_error
 */
cparser_result_t
cparser_cmd_l34_get_l4_checksum_error(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_L4chksumErrAllow_get(&mode), ret);

    diag_util_mprintf("L4 checksum error forward:%s\n",diagStr_enable[mode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_l4_checksum_error */

/*
 * l34 set ttl_1 ( enable | disable )
 */
cparser_result_t
cparser_cmd_l34_set_ttl_1_enable_disable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    if('e' == TOKEN_CHAR(3,0))
        mode = ENABLED;
    else
        mode = DISABLED;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_l34_TtlOperationMode_set(mode), ret);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_ttl_1_enable_disable */

/*
 * l34 get ttl_1
 */
cparser_result_t
cparser_cmd_l34_get_ttl_1(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_TtlOperationMode_get(&mode), ret);

    diag_util_mprintf("TTL-1 operation:%s\n",diagStr_enable[mode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_ttl_1 */

/*
 * l34 set interface_decision_mode ( vlan_based | port_based | mac_based )
 */
cparser_result_t
cparser_cmd_l34_set_interface_decision_mode_vlan_based_port_based_mac_based(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_l34_limbc_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    switch(TOKEN_CHAR(3,0))
    {
        case 'v':
            mode = RAW_L34_LIMBC_VLAN_BASED;
            break;
        case 'p':
            mode = RAW_L34_LIMBC_PORT_BASED;
            break;
        case 'm':
            mode = RAW_L34_LIMBC_MAC_BASED;
            break;
        default:
            return CPARSER_NOT_OK;
            
    }
 
    DIAG_UTIL_ERR_CHK(apollo_raw_l34_LanIntfMDBC_set(mode), ret);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_interface_decision_mode_vlan_based_port_based_mac_based */

/*
 * l34 get interface_decision_mode
 */
cparser_result_t
cparser_cmd_l34_get_interface_decision_mode(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_l34_limbc_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_LanIntfMDBC_get(&mode), ret);

    diag_util_printf("L34 interface decision mode:");
    
    switch(mode)
    {
        case RAW_L34_LIMBC_VLAN_BASED:
            diag_util_printf(" %s based\n",DIAG_STR_VLAN);
            break;
        case RAW_L34_LIMBC_PORT_BASED:
            diag_util_printf(" %s based\n",DIAG_STR_PORT);
            break;
        case RAW_L34_LIMBC_MAC_BASED:
            diag_util_printf(" %s based\n",DIAG_STR_MAC);
            break;
        default:
            diag_util_printf(" %s based\n",DIAG_STR_INVALID);
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_interface_decision_mode */

/*
 * l34 set nat_attack ( drop | trap_to_cpu )
 */
cparser_result_t
cparser_cmd_l34_set_nat_attack_drop_trap_to_cpu(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    if('t' == TOKEN_CHAR(3,0))
        mode = ENABLED;
    else
        mode = DISABLED;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_l34_natAttack2CPU_set(mode), ret);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_nat_attack_drop_trap_to_cpu */

/*
 * l34 get nat_attack
 */
cparser_result_t
cparser_cmd_l34_get_nat_attack(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_natAttack2CPU_get(&mode), ret);

    diag_util_printf("Attack packet operation:");

    if(ENABLED == mode)
        diag_util_printf(" %s\n",DIAG_STR_TRAP2CPU);
    else
        diag_util_printf(" %s\n",DIAG_STR_DROP);


    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_nat_attack */

/*
 * l34 set wan_route ( drop | trap_to_cpu | forward )
 */
cparser_result_t
cparser_cmd_l34_set_wan_route_drop_trap_to_cpu_forward(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_l34_wanroute_act_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    switch(TOKEN_CHAR(3,0))
    {
        case 'd':
            mode = L34_WAN_ROUTE_DROP;
            break;
        case 't':
            mode = L34_WAN_ROUTE_FWD_TO_CPU;
            break;
        case 'f':
            mode = L34_WAN_ROUTE_FWD;
            break;
        default:
            return CPARSER_NOT_OK;
            
    }
 
    DIAG_UTIL_ERR_CHK(apollo_raw_l34_wanRouteAct_set(mode), ret);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_wan_route_drop_trap_to_cpu_forward */

/*
 * l34 get wan_route
 */
cparser_result_t
cparser_cmd_l34_get_wan_route(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_l34_wanroute_act_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_wanRouteAct_get(&mode), ret);

    diag_util_printf("L34 WAN Route packet action:");
    
    switch(mode)
    {
        case L34_WAN_ROUTE_DROP:
            diag_util_printf(" %s\n",DIAG_STR_DROP);
            break;
        case L34_WAN_ROUTE_FWD_TO_CPU:
            diag_util_printf(" %s\n",DIAG_STR_TRAP2CPU);
            break;
        case L34_WAN_ROUTE_FWD:
            diag_util_printf(" %s\n",DIAG_STR_FORWARD);
            break;
        default:
            diag_util_printf(" %s based\n",DIAG_STR_INVALID);
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_wan_route */

/*
 * l34 set route_mode ( l3 | l3_l4 | disable )
 */
cparser_result_t
cparser_cmd_l34_set_route_mode_l3_l3_l4_disable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_l34_mode_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    if(strcmp(TOKEN_STR(3),"disable") == 0)
        mode = RAW_L34_L3_DISABLE_L4_DISABLE;
    else if(strcmp(TOKEN_STR(3),"l3_l4") == 0)
        mode = RAW_L34_L3_ENABLE_L4_ENABLE;
    else
        mode = RAW_L34_L3_ENABLE_L4_DISABLE;
    
 
    DIAG_UTIL_ERR_CHK(apollo_raw_l34_mode_set(mode), ret);
    
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_route_mode_l3_l3_l4_disable */

/*
 * l34 get route_mode
 */
cparser_result_t
cparser_cmd_l34_get_route_mode(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_l34_mode_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_mode_get(&mode), ret);

    diag_util_printf("%-10s %-10s","L3","L4");

    switch(mode)
    {
        case RAW_L34_L3_DISABLE_L4_DISABLE:
            diag_util_printf("%-10s %-10s",DIAG_STR_DISABLE,DIAG_STR_DISABLE);
            break;
        case RAW_L34_L3_ENABLE_L4_DISABLE:
            diag_util_printf("%-10s %-10s",DIAG_STR_ENABLE,DIAG_STR_DISABLE);
            break;
        case RAW_L34_L3_DISABLE_L4_ENABLE:
            diag_util_printf("%-10s %-10s",DIAG_STR_DISABLE,DIAG_STR_ENABLE);
            break;
        case RAW_L34_L3_ENABLE_L4_ENABLE:
            diag_util_printf("%-10s %-10s",DIAG_STR_ENABLE,DIAG_STR_ENABLE);
            break;
        default:
            diag_util_printf("%-10s %-10s",DIAG_STR_INVALID,DIAG_STR_INVALID);
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_route_mode */


/*
 * l34 get pppoe_trf
 */
cparser_result_t
cparser_cmd_l34_get_pppoe_trf(
    cparser_context_t *context)
{
    uint8 pppoeTrf;
    int32 index;
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l34_pppoeTrf_get(&pppoeTrf), ret);

    diag_util_mprintf("%5s %3s\n","Valid","Hit");
    
    for(index = 0;index < L34_PPPOE_TABLE_MAX_IDX;index++)
    {
        diag_util_mprintf("%-5d %-3d\n",index,((pppoeTrf & (1<<index))) >> (L34_PPPOE_TABLE_MAX_IDX - index));
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_pppoe_trf */

/*
 * l34 get arp_trf idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_arp_trf_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_arp_trf_idx_table_idx */

/*
 * l34 get arp_trf
 */
cparser_result_t
cparser_cmd_l34_get_arp_trf(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_arp_trf */

/*
 * l34 reset arp_trf ( table0 | table1 )
 */
cparser_result_t
cparser_cmd_l34_reset_arp_trf_table0_table1(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_reset_arp_trf_table0_table1 */

/*
 * l34 select arp_trf ( table0 | table1 )
 */
cparser_result_t
cparser_cmd_l34_select_arp_trf_table0_table1(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_select_arp_trf_table0_table1 */

/*
 * l34 get l4_trf idx <UINT:table_idx>
 */
cparser_result_t
cparser_cmd_l34_get_l4_trf_idx_table_idx(
    cparser_context_t *context,
    uint32_t  *table_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_l4_trf_idx_table_idx */

/*
 * l34 get l4_trf
 */
cparser_result_t
cparser_cmd_l34_get_l4_trf(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_l4_trf */

/*
 * l34 reset l4_trf ( table0 | table1 )
 */
cparser_result_t
cparser_cmd_l34_reset_l4_trf_table0_table1(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_reset_l4_trf_table0_table1 */

/*
 * l34 select l4_trf ( table0 | table1 )
 */
cparser_result_t
cparser_cmd_l34_select_l4_trf_table0_table1(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_select_l4_trf_table0_table1 */

/*
 * l34 set hsb l2bridge <UINT:l2bridbe> ipfrags <UINT:ipfrags> ipmf <UINT:ipmf> l4chk_ok <UINT:l4chk_ok> l3chk_ok <UINT:l3chk_ok> direct_tx <UINT:direct_tx> udp_no_chksum <UINT:udp_no_chk> parse_fail <UINT:parse_fail> pppoe_if <UINT:pppoe_if> svlan_if <UINT:svlan_if> ttls <UINT:ttls> pkt_type <UINT:pkt_type> tcp_flag <UINT:tcp_flag> cvlan_if <UINT:cvlan_if> spa <UINT:source_port> cvid <UINT:cvid> packet_len <UINT:packet_len> dport <UINT:dport> pppoe_id <UINT:pppoe> dip <IPV4ADDR:dip_addr> sip <IPV4ADDR:sip_addr> s_port <UINT:s_port> dmac <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l34_set_hsb_l2bridge_l2bridbe_ipfrags_ipfrags_ipmf_ipmf_l4chk_ok_l4chk_ok_l3chk_ok_l3chk_ok_direct_tx_direct_tx_udp_no_chksum_udp_no_chk_parse_fail_parse_fail_pppoe_if_pppoe_if_svlan_if_svlan_if_ttls_ttls_pkt_type_pkt_type_tcp_flag_tcp_flag_cvlan_if_cvlan_if_spa_source_port_cvid_cvid_packet_len_packet_len_dport_dport_pppoe_id_pppoe_dip_dip_addr_sip_sip_addr_s_port_s_port_dmac_mac(
    cparser_context_t *context,
    uint32_t  *l2bridbe_ptr,
    uint32_t  *ipfrags_ptr,
    uint32_t  *ipmf_ptr,
    uint32_t  *l4chk_ok_ptr,
    uint32_t  *l3chk_ok_ptr,
    uint32_t  *direct_tx_ptr,
    uint32_t  *udp_no_chk_ptr,
    uint32_t  *parse_fail_ptr,
    uint32_t  *pppoe_if_ptr,
    uint32_t  *svlan_if_ptr,
    uint32_t  *ttls_ptr,
    uint32_t  *pkt_type_ptr,
    uint32_t  *tcp_flag_ptr,
    uint32_t  *cvlan_if_ptr,
    uint32_t  *source_port_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *packet_len_ptr,
    uint32_t  *dport_ptr,
    uint32_t  *pppoe_ptr,
    uint32_t  *dip_addr_ptr,
    uint32_t  *sip_addr_ptr,
    uint32_t  *s_port_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_l34_hsb_t hsb_data;
    diag_portlist_t portlist;
    uint32_t spa_data;
            
    DIAG_UTIL_PARAM_CHK();
    memset(&hsb_data,0x0,sizeof(apollo_raw_l34_hsb_t));

    hsb_data.l2bridge            = *l2bridbe_ptr;
    hsb_data.ipfrag_s            = *ipfrags_ptr;
    hsb_data.ipmf                = *ipmf_ptr;
    hsb_data.l4_chksum_ok        = *l4chk_ok_ptr;
    hsb_data.l3_chksum_ok        = *l3chk_ok_ptr;
    hsb_data.cpu_direct_tx       = *direct_tx_ptr;
    hsb_data.udp_no_chksum       = *udp_no_chk_ptr;
    hsb_data.parse_fail          = *parse_fail_ptr;
    hsb_data.pppoe_if            = *pppoe_if_ptr;
    hsb_data.svlan_if            = *svlan_if_ptr;
    hsb_data.ttls                = *ttls_ptr;
    hsb_data.cvid                = *cvid_ptr;
    hsb_data.len                 = *packet_len_ptr;
    hsb_data.cvlan_if            = *cvlan_if_ptr;
    hsb_data.dport_l4chksum      = *dport_ptr;
    hsb_data.pppoe_id            = *pppoe_ptr;
    hsb_data.sport_icmpid_chksum = *s_port_ptr;
    hsb_data.tcp_flag            = *tcp_flag_ptr;


     
    /*DIP address*/
    if((ret = diag_util_str2ip((uint32 *)&hsb_data.dip,(uint8 *)TOKEN_STR(42))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*SIP address*/
    if((ret = diag_util_str2ip((uint32 *)&hsb_data.sip,(uint8 *)TOKEN_STR(44))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }


    hsb_data.spa = *source_port_ptr;

    /*destination mac*/
    osal_memcpy((uint8 *)&hsb_data.dmac,(uint8 *)mac_ptr,sizeof(cparser_macaddr_t));
 

    /*call hsb API*/
    if((ret = apollo_raw_l34_hsbData_set(&hsb_data)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_hsb_l2bridge_l2bridbe_ipfrags_ipfrags_ipmf_ipmf_l4chk_ok_l4chk_ok_l3chk_ok_l3chk_ok_direct_tx_direct_tx_udp_no_chksum_udp_no_chk_parse_fail_parse_fail_pppoe_if_pppoe_if_svlan_if_svlan_if_ttls_ttls_pkt_type_pkt_type_tcp_flag_tcp_flag_cvlan_if_cvlan_if_spa_port_lost_ext_port_extport_list_vc_port_vcport_list_cvid_cvid_packet_len_packet_len_dport_dport_pppoe_pppoe_dip_dip_addr_sip_sip_addr_s_port_s_port_dmac_mac */


/*
 * l34 get hsb
 */
cparser_result_t
cparser_cmd_l34_get_hsb(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_l34_hsb_t hsb_data;
    uint32  ip_tmp_buf[UTIL_IP_TMP_BUFFER_LENGTH];
    uint32_t spa_data;

    memset(&hsb_data,0x0,sizeof(apollo_raw_l34_hsb_t));
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    /*call hsb API*/
    if((ret = apollo_raw_l34_hsbData_get(&hsb_data)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_printf("\n l2bridbe            :%-4d",hsb_data.l2bridge);
    diag_util_printf("\n ipfrags             :%-4d",hsb_data.ipfrag_s);
    diag_util_printf("\n ipmf                :%-4d",hsb_data.ipmf);
    diag_util_printf("\n l4chk               :%-4d",hsb_data.l4_chksum_ok);
    diag_util_printf("\n l3chk               :%-4d",hsb_data.l3_chksum_ok);
    diag_util_printf("\n direct_tx           :%-4d",hsb_data.cpu_direct_tx);
    diag_util_printf("\n udp_no_chk          :%-4d",hsb_data.udp_no_chksum);
    diag_util_printf("\n parse_fail          :%-4d",hsb_data.parse_fail);
    diag_util_printf("\n pppoe_if            :%-4d",hsb_data.pppoe_if);
    diag_util_printf("\n svlan_if            :%-4d",hsb_data.svlan_if);
    diag_util_printf("\n ttls                :%-4d",hsb_data.ttls);
    diag_util_printf("\n cvlan_if            :%-4d",hsb_data.cvlan_if);
    diag_util_printf("\n SPA                 :%-4d",hsb_data.spa);   
    diag_util_printf("\n cvid                :%-4d",hsb_data.cvid);
    diag_util_printf("\n len                 :%-4d",hsb_data.len);
    diag_util_printf("\n dport_l4chksum      :%-4d",hsb_data.dport_l4chksum);
    diag_util_printf("\n pppoe_id            :%-4d",hsb_data.pppoe_id);
    diag_util_printf("\n sport_icmpid_chksum :%-4d",hsb_data.sport_icmpid_chksum);
    diag_util_printf("\n tcp_flag            :0x%-4x",hsb_data.tcp_flag);

    /*DIP*/    
    diag_util_ip2str((uint8*)ip_tmp_buf, (uint32)hsb_data.dip);
    diag_util_printf("\n DIP                 :%s",ip_tmp_buf);

    /*SIP*/    
    diag_util_ip2str((uint8*)ip_tmp_buf, (uint32)hsb_data.sip);
    diag_util_printf("\n DIP                 :%s",ip_tmp_buf);
 
    /*DMAC*/    
    diag_util_printf("\n DMAC                :%s \n",diag_util_inet_mactoa((uint8 *)&hsb_data.dmac));   
  
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_hsb */

/*
 * l34 get hsa
 */
cparser_result_t
cparser_cmd_l34_get_hsa(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_l34_hsa_t hsa_data;
    uint32  ip_tmp_buf[UTIL_IP_TMP_BUFFER_LENGTH];
    uint32_t spa_data;

    memset(&hsa_data,0x0,sizeof(apollo_raw_l34_hsa_t));
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    /*call API*/
    if((ret = apollo_raw_l34_hsaData_get(&hsa_data)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    
    diag_util_printf("\n l34trans            :%-4d",hsa_data.l34trans);    
    diag_util_printf("\n l2trans             :%-4d",hsa_data.l2trans);    
    diag_util_printf("\n inter_if            :%-4d",hsa_data.inter_if);    
    diag_util_printf("\n l4_pri_valid        :%-4d",hsa_data.l4_pri_valid);    
    diag_util_printf("\n frag                :%-4d",hsa_data.frag);    
    diag_util_printf("\n pppoe_if            :%-4d",hsa_data.pppoe_if);    
    diag_util_printf("\n action              :%-4d",hsa_data.action);    
    diag_util_printf("\n pppid_idx           :%-4d",hsa_data.pppid_idx);    
    diag_util_printf("\n difid               :%-4d",hsa_data.difid);    
    diag_util_printf("\n l4_pri_sel          :%-4d",hsa_data.l4_pri_sel);    
    diag_util_printf("\n dvid                :%-4d",hsa_data.dvid);    
    diag_util_printf("\n reason              :%-4d",hsa_data.reason);
    diag_util_printf("\n nexthop_mac_idx     :%-4d",hsa_data.nexthop_mac_idx);    
    diag_util_printf("\n l4_chksum           :0x%-6x",hsa_data.l4_chksum);    
    diag_util_printf("\n l3_chksum           :0x%-4x",hsa_data.l3_chksum);    
    diag_util_printf("\n port                :%-4d",hsa_data.port);    
    /*IP address*/
    diag_util_ip2str((uint8*)ip_tmp_buf, (uint32)hsa_data.ip);
    diag_util_printf("\n IP                  :%s\n",ip_tmp_buf);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_hsa */


/*
 * l34 set hsba log_mode <UINT:mode>
 */
cparser_result_t
cparser_cmd_l34_set_hsba_log_mode_mode(
    cparser_context_t *context,
    uint32_t  *mode_ptr)
{
    int32 ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_PARAM_RANGE_CHK((*mode_ptr > 5),RT_ERR_INPUT);
    
    /*call API*/
    if((ret = apollo_raw_l34_hsba_mode_set(*mode_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l34_set_hsba_log_mode_mode */


/*
 * l34 get hsba log_mode 
 */
cparser_result_t
cparser_cmd_l34_get_hsba_log_mode(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32_t  mode;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

   
    /*call API*/
    if((ret = apollo_raw_l34_hsba_mode_get(&mode)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_printf(" hsba log mode             :%d\n",mode);
    return CPARSER_OK;
}    /* end of cparser_cmd_l34_get_hsba_log_mode */

