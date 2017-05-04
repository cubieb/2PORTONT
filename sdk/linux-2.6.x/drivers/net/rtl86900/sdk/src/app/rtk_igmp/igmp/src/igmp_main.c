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
 * Purpose : Main function of SFU IGMP Application
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) IGMP main APIs
 */

/*
* Include Files
*/
#include <igmp/inc/proto_igmp.h>
#include "pkt_redirect_user.h"
#include <rtk/init.h>

static int          igmpPktRedirect_sock;
igmp_mld_control_t  igmpCtrl;
igmp_stats_t        *p_igmp_stats = NULL;
mld_stats_t         *p_mld_stats = NULL;
uint32              gUnknown_mcast_action;

static int config_modify_by_boa(void)
{
    char boaMibVal[BOA_MIB_BUFFER_LEN];

    //pon_mode
    memset(boaMibVal, 0, BOA_MIB_BUFFER_LEN);
    if (SYS_ERR_OK == igmp_util_get_boa_mib_value_by_name("PON_MODE", boaMibVal))
    {
        igmpCtrl.ponMode = strtol(boaMibVal, NULL, 0);
    }

    //log
    memset(boaMibVal, 0, BOA_MIB_BUFFER_LEN);
    if (SYS_ERR_OK == igmp_util_get_boa_mib_value_by_name("RTK_IGMP_DBGEN", boaMibVal))
    {
        igmpCtrl.logEnable = strtol(boaMibVal, NULL, 0);
    }

    //igmp v1 behavior
    memset(boaMibVal, 0, BOA_MIB_BUFFER_LEN);
    if (SYS_ERR_OK == igmp_util_get_boa_mib_value_by_name("RTK_IGMP_DROPV1", boaMibVal))
    {
        igmpCtrl.igmpv1Drop_enable = strtol(boaMibVal, NULL, 0);
    }

    //igmp tag search behavior
    memset(boaMibVal, 0, BOA_MIB_BUFFER_LEN);
    if (SYS_ERR_OK == igmp_util_get_boa_mib_value_by_name("RTK_IGMP_TAGDECISION", boaMibVal))
    {
        igmpCtrl.igmpTagBehavior_enable = strtol(boaMibVal, NULL, 0);
    }

    //igmp query tag vid type
    memset(boaMibVal, 0, BOA_MIB_BUFFER_LEN);
    if (SYS_ERR_OK == igmp_util_get_boa_mib_value_by_name("RTK_IGMP_VIDTYPE", boaMibVal))
    {
        igmpCtrl.igmpVlanTag_type = strtol(boaMibVal, NULL, 0);
    }

    return SYS_ERR_OK;
}

static void _userStatusConfigSet(char *str, int value)
{
	if (!strcmp(str, "l2LookupMode"))
	{
		p_igmp_stats->lookup_mode = value;
	}
    else if (!strcmp(str, "igmpOperVer"))
	{
		p_igmp_stats->oper_version = value;
	}
    else if (!strcmp(str, "unknowIp4mcastAct"))
	{
		//p_igmp_stats->unknowIp4mcastAct = value;
	}
    else if (!strcmp(str, "gponMacMcastFilter"))
	{
		igmpCtrl.gmacMcastFilter_enable = value;
	}
    return;
}

/*Get config from config file, and set*/
static int userStatusConfigSet(char *filename)
{
	FILE    *pFile;
	char    buff[100], *str, *pch;
	int32   value;

    pFile = fopen(filename, "r");

    SYS_ERR_CHK((NULL == pFile), SYS_ERR_FAILED);

	while (NULL != fgets(buff, sizeof(buff), pFile))
	{
		/*string start with "#" means this line is comment*/
		if (buff[0] == '#')
			continue;

		pch = strtok(buff, "=");
		str = pch;

		pch = strtok(NULL, "=");
		value = atoi(pch);

		_userStatusConfigSet(str, value);
	}

	return SYS_ERR_OK;
}

static void config_init_default(void)
{
    // igmp_mld ctrl init
	igmpCtrl.igmpv1Drop_enable  = DISABLED;
    LOGIC_PORTMASK_CLEAR_ALL(igmpCtrl.igmpMldTrap_enable);

    // p_igmp_stats init
	p_igmp_stats->lookup_mode      = DFLT_IGMP_LOOKUP_MODE;
	p_igmp_stats->igmpsnp_enable   = DISABLED;
	p_igmp_stats->suppre_enable    = DFLT_IGMP_SUPPRE_EBL;
	p_igmp_stats->oper_version     = DFLT_IGMP_OPERATION_VER;

	gUnknown_mcast_action       = DFLT_IGMP_UNKNOWN_MCAST_ACTION;

    // p_mld_stats init
    p_mld_stats->lookup_mode   = DFLT_MLD_LOOKUP_MODE;
    p_mld_stats->mldsnp_enable = DISABLED;
    p_mld_stats->suppre_enable = DFLT_MLD_SUPPRE_EBL;
    p_mld_stats->oper_version  = DFLT_MLD_OPERATION_VER;

	return;
}

int32 igmp_mld_send_pkt(uint8 *pMsg, uint32 len, rtk_portmask_t portmask, uint32 sid)
{
    uint8 is_unkown = 0;
    uint8 data[MAX_PAYLOAD];
    uint32 portMask = 0, realDataLen = 0;
    rtk_portmask_t zero_mask, ether_pmk;

    SYS_ERR_CHK(((NULL == pMsg) || (0 == len) || (MAX_PAYLOAD - sizeof(uint32) - sizeof(uint32) < len)), SYS_ERR_NULL_POINTER);

    RTK_PORTMASK_RESET(zero_mask);

    SYS_DBG(LOG_DBG_IGMP,
        "%s()@%d: origial  physical port mask=%u, sid=%u\n", __FUNCTION__, __LINE__, portmask.bits[0], sid);

    is_unkown = (((RTK_PORTMASK_COMPARE(portmask, zero_mask)) || (RTK_PORTMASK_IS_PORT_SET(portmask, HAL_GET_PON_PORT()))) ? 0 : 1);

    if (is_unkown)
    {
        RTK_PORTMASK_RESET(ether_pmk);
        HAL_GET_ETHER_PORTMASK(ether_pmk); /* include pon portmask  */
        RTK_PORTMASK_PORT_CLEAR(ether_pmk, HAL_GET_PON_PORT());
        portMask = ether_pmk.bits[0];
    }
    else
    {
        portMask = portmask.bits[0];
    }

    realDataLen = ((len + sizeof(uint32) * 2) > MAX_PAYLOAD ? MAX_PAYLOAD : (len + sizeof(uint32) * 2));
    osal_memset(data, 0, MAX_PAYLOAD);
    // portMask + sid + pMsg
    osal_memcpy(data, (unsigned char *)&portMask, sizeof(uint32));
    osal_memcpy(data + sizeof(uint32), (unsigned char *)&sid, sizeof(uint32));
    osal_memcpy(data + sizeof(uint32) * 2, pMsg, len);

    SYS_DBG(LOG_DBG_IGMP,
        "%s()@%d, is_unkown=%u pkt fwd port mask=%u, realDataLen=%u\n", __FUNCTION__, __LINE__, is_unkown, portMask, realDataLen);


    if (0 > ptk_redirect_userApp_sendPkt(igmpPktRedirect_sock, PR_KERNEL_UID_IGMPCTRL, 0, realDataLen, (unsigned char *)&data))
    {
        printf("Send packet failed by PR %s() @%d\n", __FUNCTION__, __LINE__);
        return SYS_ERR_FAILED;
    }

    return SYS_ERR_OK;
}

static int32 igmp_mld_recv_pkt(void)
{
	unsigned short dataLen  = 0;
	unsigned char *pPayload = NULL;

	pPayload = (unsigned char *)osal_alloc(MAX_PAYLOAD * sizeof(char));

    SYS_PARAM_CHK((!pPayload), SYS_ERR_NULL_POINTER);

	while (1)
    {
        if (ptk_redirect_userApp_recvPkt(igmpPktRedirect_sock, MAX_PAYLOAD, &dataLen, pPayload) > 0)
        {
			mcast_recv(pPayload, dataLen);
		}
	}
	return SYS_ERR_OK;
}

int32 igmp_mld_create_pkt_dev(void)
{
    if (-1 == (igmpPktRedirect_sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK)))
	{
		printf("Create Socket Fail!\n");
	    return SYS_ERR_FAILED;
	}

    if (0 > ptk_redirect_userApp_reg(igmpPktRedirect_sock, PR_USER_UID_IGMPCTRL, MAX_PAYLOAD))
	{
		printf("Register Packet Redirect Fail.\n");
		return SYS_ERR_FAILED;
	}
    return SYS_ERR_OK;
}

int main(int argc, char *argv[])
{
	int cmd = 1;
	char filename[50];

	/* global control resource init */
    osal_memset(&igmpCtrl, 0, sizeof(igmp_mld_control_t));

    SYS_ERR_CHK((SYS_ERR_OK != rtk_core_init()), SYS_ERR_FAILED);

    /* TBD for max port id definition */
    igmpCtrl.sys_max_port_num = (HAL_GET_CPU_PORT() + 1);

    printf("igmpd is running:  sys_max_port_num =%u .....\n", igmpCtrl.sys_max_port_num);

    /* igmp and mld statistics resource init */
    SYS_ERR_CHK((SYS_ERR_OK != mcast_igmp_mld_statistics_init()), SYS_ERR_FAILED);

    /* set default value for global configuration */
	config_init_default();

    /* set configuration by BOA MIB */
    SYS_ERR_CHK((SYS_ERR_OK != config_modify_by_boa()), SYS_ERR_FAILED);

	/* parser input parameters for user setting */
	while (cmd < argc)
	{
		if (!strcmp("-f", argv[cmd]))
		{
			cmd++;
			strcpy(filename, argv[cmd]);
			cmd++;
		}
        else
		{
			cmd++;
		}
	}

    /* set configuration by user configuration file */
    SYS_ERR_CHK((SYS_ERR_OK != userStatusConfigSet(filename)), SYS_ERR_FAILED);

	SYS_ERR_CHK((SYS_ERR_OK != mcast_init()), SYS_ERR_FAILED);

    /* start to receive control packets */
    SYS_ERR_CHK((SYS_ERR_OK != igmp_mld_recv_pkt()), SYS_ERR_FAILED);

	return SYS_ERR_OK;
}

