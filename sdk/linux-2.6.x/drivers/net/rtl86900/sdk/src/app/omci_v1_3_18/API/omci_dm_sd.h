/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of Dual Management shared define
 */

#ifndef __OMCI_DM_DS_H__
#define __OMCI_DM_DS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define WAN_PONMAC_QUEUE_MAX		(4)
#define OMCI_IPV6_ADDR_LEN   		16
#define	OMCI_USERNAME_PASSWORD_LEN	25
#define	OMCI_ACS_URL_LEN 			375


typedef enum mgmt_cfg_op_e
{
	OP_RESET_ALL,
	OP_SET_IF,
	OP_RESET_ACS,
	OP_SET_ACS
}mgmt_cfg_op_t;

typedef enum if_service_e
{
	IF_SERVICE_DATA			= (1 << 0),
	IF_SERVICE_TR69			= (1 << 1),
	IF_SERVICE_SIP			= (1 << 2),
	IF_SERVICE_ALL			= (IF_SERVICE_DATA | IF_SERVICE_TR69 | IF_SERVICE_SIP)		
}if_service_t;

typedef unsigned int omci_ipv4_addr_t;

typedef struct omci_ipv6_addr_s
{
    unsigned char ipv6_addr[OMCI_IPV6_ADDR_LEN];
} omci_ipv6_addr_t;

typedef struct if_info_s
{
	unsigned int if_id;
    unsigned int if_is_ipv6_B;
	unsigned int if_is_DHCP_B; 
	unsigned int if_is_ip_stack_B;
	if_service_t if_service_type; 
	unsigned short if_tci;

    union
    {
        omci_ipv4_addr_t ipv4_addr;
        omci_ipv6_addr_t ipv6_ddr;
    } ip_addr;
	
	union
    {
        omci_ipv4_addr_t ipv4_mask_addr;
        omci_ipv6_addr_t ipv6_mask_addr;
    } mask_addr;
	
	union
    {
        omci_ipv4_addr_t ipv4_gateway_addr;
        omci_ipv6_addr_t ipv6_gateway_addr;
    } gateway_addr;
	
	union
    {
        omci_ipv4_addr_t ipv4_primary_dns_addr;
        omci_ipv6_addr_t ipv6_primary_dns_addr;
    } primary_dns_addr;
	
	union
    {
        omci_ipv4_addr_t ipv4_second_dns_addr;
        omci_ipv6_addr_t ipv6_second_dns_addr;
    } second_dns_addr;
	
}if_info_t;

typedef struct acs_info_s
{
	unsigned int 	related_if_id;
	unsigned char	acs_url[OMCI_ACS_URL_LEN];
	unsigned char   username[OMCI_USERNAME_PASSWORD_LEN];
	unsigned char   password[OMCI_USERNAME_PASSWORD_LEN];
}acs_info_t;

typedef struct mgmt_cfg_s
{
	if_info_t if_entry;
	acs_info_t acs;
	//others mgmt service info: sip
}mgmt_cfg_t;

typedef struct mgmt_cfg_msg_s
{
	mgmt_cfg_op_t op_id;
	mgmt_cfg_t cfg;
}mgmt_cfg_msg_t;


unsigned int omci_cfg_set (mgmt_cfg_msg_t *pParam, unsigned int len);


#ifdef __cplusplus
}
#endif

#endif
