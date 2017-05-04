
#include <rtk_rg_xdsl_extAPI.h>
#include <rtk_rg_acl_xdsl.h>
#include <rtk_rg_internal.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl865x_outputQueue.h>

extern int inet_aton(const char *cp, struct in_addr *addrptr);
extern __be32 in_aton(const char *str);
extern int in4_pton(const char *src, int srclen, u8 *dst, int delim, const char **end);
extern int in6_pton(const char *src, int srclen, u8 *dst, int delim, const char **end);
extern char * strstr(const char *,const char *);
extern uint8* strtomac(ether_addr_t *mac, int8 *str);
//extern uint32 inet_addr(const int8 *cp);


enum rtk_rg_parseDataType
{
   MAC=0,
   V4IP,
   V6IP,
   STRING_EN_DIS, //warning : only use for uint32* parameterGetAddr
   UNSIGNED_LONG, //uint32
   SINGED_LONG,	  //int32
   STRING,		  //warning : should sure string non-overflow
};

int32 _rtk_rg_parseGetValue(char* stringBuff,void *parameterGetAddr,char* parameterName,enum rtk_rg_parseDataType dataType)
{
	char* strPtr;
	uint32 stringSize=0;
	
	strPtr=strstr(stringBuff,parameterName);
	if(strPtr==NULL) //string not find
	{
		printk("stringBuff %s \n%s not find\n",stringBuff,parameterName);
		return FAILED;
	}
	strPtr+=strlen(parameterName);
//	else{
//		printk("\n %s \n  stringBuff %p strPtr %p \n",stringBuff,stringBuff,strPtr);
//	}
	while((*strPtr==0x20) || (*strPtr==0x2c)) //ingore " " and ","
		strPtr++;

	if(*strPtr<0x20 ||  *strPtr==0x7f)//out of ascii char range
		return FAILED;

//	printk("%s type=%d posDif=%d  sizeof:%d\n",parameterName,dataType,strPtr-stringBuff,strlen(parameterName));

	switch(dataType)
	{
		case MAC:
			strtomac(((ether_addr_t*)(parameterGetAddr)),strPtr);
			break;
		case V4IP:
			//*((uint32 *)parameterGetAddr) = (uint32)in_aton(strPtr);
			in4_pton(strPtr,-1,(uint8*)parameterGetAddr,-1,NULL);
			break;
		case V6IP:
			in6_pton(strPtr,-1,(uint8*)parameterGetAddr,-1,NULL);
			break;
		case UNSIGNED_LONG:
			*((uint32*)parameterGetAddr)= simple_strtoul(strPtr,NULL,0);
			break;
		case SINGED_LONG:
			*((int32*)parameterGetAddr)= simple_strtol(strPtr,NULL,0);
			break;
		case STRING_EN_DIS:
			if(memcmp(strPtr,"enable",sizeof("enable")-1)==0){
				*((uint32*)parameterGetAddr)=1;
			}else if(memcmp(strPtr,"disable",sizeof("disable")-1)==0){
				*((uint32*)parameterGetAddr)=0;
			}else
				return FAILED;
			break;
		case STRING:
			while(1){
				if(*(strPtr+stringSize)<=0x20 ||  *(strPtr+stringSize)==0x7f)//out of ascii char range (include space)
					break;
				stringSize++;
			}
			memcpy(parameterGetAddr,strPtr,stringSize);
			break;
		default:
			return FAILED;

	}

	return RT_ERR_OK;
}

extern int _rtk_rg_initParameterSetByHwCallBack(void);
extern int _rtk_rg_interfaceAddByHwCallBack(rtk_rg_intfInfo_t* intfInfo, int* intfIdx);
extern int _rtk_rg_interfaceDelByHwCallBack(rtk_rg_intfInfo_t* intfInfo, int* intfIdx);
extern int _rtk_rg_routingAddByHwCallBack(rtk_rg_ipv4RoutingEntry_t* routingInfo);
extern int _rtk_rg_routingDelByHwCallBack(rtk_rg_ipv4RoutingEntry_t* routingInfo);
extern int _rtk_rg_dhcpRequestByHwCallBack(int* intfIdx);
extern int _rtk_rg_pppoeBeforeDiagByHwCallBack(rtk_rg_pppoeClientInfoBeforeDial_t* before_diag, int* intfIdx);
extern int _rtk_rg_pppoeDsliteBeforeDialByHwCallBack(rtk_rg_pppoeClientInfoBeforeDial_t *before_diag, int *intfIdx);
extern int _rtk_rg_pptpBeforeDialByHwCallBack(rtk_rg_pptpClientInfoBeforeDial_t* before_dial, int* intfIdx);
extern int _rtk_rg_l2tpBeforeDialByHwCallBack(rtk_rg_l2tpClientInfoBeforeDial_t* before_dial, int* intfIdx);

extern int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);

int32 rtk_rg_xdsl_init( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtk_rg_initParams_t init_param;
	bzero(&init_param,sizeof(init_param));
	init_param.initByHwCallBack =_rtk_rg_initParameterSetByHwCallBack;
	init_param.interfaceAddByHwCallBack=_rtk_rg_interfaceAddByHwCallBack;
	init_param.interfaceDelByHwCallBack=_rtk_rg_interfaceDelByHwCallBack;
	init_param.routingAddByHwCallBack=_rtk_rg_routingAddByHwCallBack;
	init_param.routingDelByHwCallBack=_rtk_rg_routingDelByHwCallBack;
	init_param.dhcpRequestByHwCallBack=_rtk_rg_dhcpRequestByHwCallBack;
	init_param.pppoeBeforeDiagByHwCallBack=_rtk_rg_pppoeBeforeDiagByHwCallBack;
	init_param.pppoeDsliteBeforeDialByHwCallBack=_rtk_rg_pppoeDsliteBeforeDialByHwCallBack;
	init_param.pptpBeforeDialByHwCallBack=_rtk_rg_pptpBeforeDialByHwCallBack;
	init_param.l2tpBeforeDialByHwCallBack=_rtk_rg_l2tpBeforeDialByHwCallBack;

	
	assert_ok(pf.rtk_rg_initParam_set(&init_param));
	TRACE("rg_init done  %p  %p\n",rg_db.systemGlobal.initParam.initByHwCallBack,rg_db.systemGlobal.initParam.interfaceAddByHwCallBack);
	return len;
}

int32 rtk_rg_xdsl_lanInterface_add( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char *tmpbuf;	
	int ret;
	int lanIntfIdx;
	rtk_rg_lanIntfConf_t lan_info;
	
	tmpbuf=&rg_kernel.proc_parsing_buf[0];
	memset(&lan_info,0,sizeof(lan_info));
	if (buff && !copy_from_user(tmpbuf, buff, len))	
	{		
		tmpbuf[len] = '\0';				
	}else{
		goto Error;
	}
	
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.ip_version,"ip-version",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.gmac.octet[0],"gateway-mac",MAC),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.ip_addr,"ip-addr",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.ip_network_mask,"ip-mask",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.ipv6_addr.ipv6_addr[0],"ipv6-addr",V6IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.ipv6_network_mask_length,"ipv6_network_mask_length",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.port_mask.portmask,"port-mask",UNSIGNED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.untag_mask.portmask,"untag-mask",UNSIGNED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.intf_vlan_id,"intf-vlan_id",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.vlan_based_pri_enable,"vlan-based-pri-enable",STRING_EN_DIS),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.mtu,"mtu",SINGED_LONG),RT_ERR_RG_OK); if(lan_info.mtu==0) goto Error;
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&lan_info.isIVL,"isIVL",SINGED_LONG),RT_ERR_RG_OK);

{//debug
	int i=0;
	printk("\nip-version=%d gateway-mac=%02x:%02x:%02x:%02x:%02x:%02x ipaddr=%x ip-mask=%x port-mask=%x untag-mask=%x intf-vlan_id=%d vlan-based-pri-enable=%d mtu=%d isIVL=%d\n",
		lan_info.ip_version,lan_info.gmac.octet[0],lan_info.gmac.octet[1],lan_info.gmac.octet[2],lan_info.gmac.octet[3],lan_info.gmac.octet[4],lan_info.gmac.octet[5],
		lan_info.ip_addr,lan_info.ip_network_mask,lan_info.port_mask.portmask,lan_info.untag_mask.portmask,lan_info.intf_vlan_id,lan_info.vlan_based_pri_enable,lan_info.mtu,lan_info.isIVL);
	printk("ipv6_network_mask_length=%d ipv6-addr=",lan_info.ipv6_network_mask_length);
	for (i=0 ;i<8;i++)
		printk("%02x%02x ",lan_info.ipv6_addr.ipv6_addr[i*2],lan_info.ipv6_addr.ipv6_addr[i*2+1]);
	printk("\n");
}
	
	ret = pf.rtk_rg_lanInterface_add(&lan_info,&lanIntfIdx);
	printk("rtk_rg_lanInterface_add(%d) ret=%d\n",lanIntfIdx,ret);

	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT, NULL));
	return len;

Error:
	printk("invail parameter!\n");
	return len;
}
int32 rtk_rg_xdsl_wanInterface_add( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char *tmpbuf;	
	int ret;
	int wanIntfIdx;
	uint32 tempUint32;
	rtk_rg_wanIntfConf_t wan_info;
	
	tmpbuf=&rg_kernel.proc_parsing_buf[0];
	memset(&wan_info,0,sizeof(wan_info));
	if (buff && !copy_from_user(tmpbuf, buff, len)) 
	{		
		tmpbuf[len] = '\0'; 			
	}else{
		goto Error;
	}

	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wan_info.wan_type,"wan-type",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wan_info.gmac.octet[0],"gateway-mac",MAC),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wan_info.wan_port_idx,"wan-port",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wan_info.port_binding_mask.portmask,"port-binding-mask",UNSIGNED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&tempUint32,"egress-vlan-tag-on",UNSIGNED_LONG),RT_ERR_RG_OK);
	if(tempUint32) wan_info.egress_vlan_tag_on=1;	
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wan_info.egress_vlan_id,"egress-vlan-id",SINGED_LONG),RT_ERR_RG_OK); if(wan_info.egress_vlan_id==0) goto Error;
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wan_info.vlan_based_pri_enable,"vlan-based-pri-enable",STRING_EN_DIS),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&tempUint32,"isIVL",UNSIGNED_LONG),RT_ERR_RG_OK);
	if(tempUint32) wan_info.isIVL=1;



{//debug
	printk("\nwan-type=%d gateway-mac=%02x:%02x:%02x:%02x:%02x:%02x wan-port=%x port-binding-mask=%x egress-vlan-tag-on=%d  egress-vlan-id=%d vlan-based-pri-enable=%d isIVL=%d\n",
		wan_info.wan_type,wan_info.gmac.octet[0],wan_info.gmac.octet[1],wan_info.gmac.octet[2],wan_info.gmac.octet[3],wan_info.gmac.octet[4],wan_info.gmac.octet[5],
		wan_info.wan_port_idx,wan_info.port_binding_mask.portmask,wan_info.egress_vlan_tag_on,wan_info.egress_vlan_id,wan_info.vlan_based_pri_enable,wan_info.isIVL);
}


	//rtk_rg_apollo_wanInterface_add
	ret = pf.rtk_rg_wanInterface_add(&wan_info,&wanIntfIdx);
	printk("rtk_rg_wanInterface_add  wanIntfIdx=(%d) ret=%d\n",wanIntfIdx,ret);

	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT, NULL));

	return len;

Error:
	printk("invail parameter!\n");
	return len;

}
int32 rtk_rg_xdsl_staticInfo_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char *tmpbuf;	
	int ret;
	int32 wanIntfIdx=-1;
	rtk_rg_ipStaticInfo_t staticInfo;
	
	tmpbuf=&rg_kernel.proc_parsing_buf[0];
	memset(&staticInfo,0,sizeof(staticInfo));
	if (buff && !copy_from_user(tmpbuf, buff, len)) 
	{		
		tmpbuf[len] = '\0'; 			
	}else{
		goto Error;
	}

	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wanIntfIdx,"intf-index",SINGED_LONG),RT_ERR_RG_OK);
	if(wanIntfIdx==-1) goto Error;
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.ip_version,"ip-version",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.napt_enable,"napt_enable",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.ip_addr,"ip_addr",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.ip_network_mask,"ip_network_mask",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.ipv4_default_gateway_on,"ipv4_default_gateway_on",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.gateway_ipv4_addr,"gateway_ipv4_addr",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.mtu,"mtu",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.gw_mac_auto_learn_for_ipv4,"gw_mac_auto_learn_for_ipv4",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&staticInfo.gateway_mac_addr_for_ipv4.octet[0],"gateway_mac_addr_for_ipv4",MAC),RT_ERR_RG_OK);

{//debug
	printk("\nip-version=%d napt_enable=%d gateway_mac_addr_for_ipv4=%02x:%02x:%02x:%02x:%02x:%02x ip_addr=%x ip_network_mask=%x ipv4_default_gateway_on=%d gateway_ipv4_addr=%x mtu=%d gw_mac_auto_learn_for_ipv4=%d\n",
		staticInfo.ip_version,staticInfo.napt_enable,staticInfo.gateway_mac_addr_for_ipv4.octet[0],staticInfo.gateway_mac_addr_for_ipv4.octet[1],staticInfo.gateway_mac_addr_for_ipv4.octet[2],
		staticInfo.gateway_mac_addr_for_ipv4.octet[3],staticInfo.gateway_mac_addr_for_ipv4.octet[4],staticInfo.gateway_mac_addr_for_ipv4.octet[5],
		staticInfo.ip_addr,staticInfo.ip_network_mask,staticInfo.ipv4_default_gateway_on,staticInfo.gateway_ipv4_addr,staticInfo.mtu,staticInfo.gw_mac_auto_learn_for_ipv4);
}

	
//	rtk_rg_apollo_staticInfo_set
	ret = pf.rtk_rg_staticInfo_set(wanIntfIdx,&staticInfo);
	printk("rtk_rg_apollo_staticInfo_set ret=%d gw_ip=0x%x,ip=0x%x\n",ret,staticInfo.gateway_ipv4_addr,staticInfo.ip_addr);
	return len;

Error:
	printk("invail parameter!\n");
	return len;

}
int32 rtk_rg_xdsl_pppoeClientInfoBeforeDial_set( struct file *filp, const char *buff,unsigned long len, void *data )
{

	char *tmpbuf;	
	int ret;
	int32 wanIntfIdx=-1;
	rtk_rg_pppoeClientInfoBeforeDial_t pppoeBeforeDial;
	
	tmpbuf=&rg_kernel.proc_parsing_buf[0];
	memset(&pppoeBeforeDial,0,sizeof(pppoeBeforeDial));
	if (buff && !copy_from_user(tmpbuf, buff, len)) 
	{		
		tmpbuf[len] = '\0'; 			
	}else{
		goto Error;
	}

	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wanIntfIdx,"intf-index",SINGED_LONG),RT_ERR_RG_OK);
	if(wanIntfIdx==-1) goto Error;
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.username,"username",STRING),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.password,"password",STRING),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.auth_type,"auth_type",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.pppoe_proxy_enable,"pppoe_proxy_enable",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.max_pppoe_proxy_num,"max_pppoe_proxy_num",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.auto_reconnect,"auto_reconnect",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.dial_on_demond,"dial_on_demond",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.idle_timeout_secs,"idle_timeout_secs",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.stauts,"status",SINGED_LONG),RT_ERR_RG_OK);	
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.dialOnDemondCallBack,"dialOnDemondCallBack",UNSIGNED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&pppoeBeforeDial.idleTimeOutCallBack,"idleTimeOutCallBack",UNSIGNED_LONG),RT_ERR_RG_OK);

{//debug
	printk("\nintf-index=%d username=%s password=%s auth_type=%d pppoe_proxy_enable=%d max_pppoe_proxy_num=%d auto_reconnect=%d dial_on_demond=%d idle_timeout_secs=%d status=%d dialOnDemondCallBack=%p idleTimeOutCallBack=%p",
		wanIntfIdx,pppoeBeforeDial.username,pppoeBeforeDial.password,pppoeBeforeDial.auth_type,pppoeBeforeDial.pppoe_proxy_enable,pppoeBeforeDial.max_pppoe_proxy_num,pppoeBeforeDial.auto_reconnect,
		pppoeBeforeDial.dial_on_demond,pppoeBeforeDial.idle_timeout_secs,pppoeBeforeDial.stauts,pppoeBeforeDial.dialOnDemondCallBack,pppoeBeforeDial.idleTimeOutCallBack);
}	
//	rtk_rg_apollo_pppoeClientInfoBeforeDial_set
	ret = pf.rtk_rg_pppoeClientInfoBeforeDial_set(wanIntfIdx,&pppoeBeforeDial);
	return len;

Error:
	printk("invail parameter!\n");
	return len;


}
int32 rtk_rg_xdsl_pppoeClientInfoAfterDial_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char *tmpbuf;	
	int ret;
	uint32 tempUint32;
	int32 wanIntfIdx=-1;
	rtk_rg_pppoeClientInfoAfterDial_t clientPppoe_info;
	
	tmpbuf=&rg_kernel.proc_parsing_buf[0];
	memset(&clientPppoe_info,0,sizeof(clientPppoe_info));
	if (buff && !copy_from_user(tmpbuf, buff, len)) 
	{		
		tmpbuf[len] = '\0'; 			
	}else{
		goto Error;
	}

	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wanIntfIdx,"intf-index",SINGED_LONG),RT_ERR_RG_OK);
	if(wanIntfIdx==-1) goto Error;
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&tempUint32,"sessionId",UNSIGNED_LONG),RT_ERR_RG_OK);
	clientPppoe_info.sessionId =tempUint32;
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.ip_version,"ip-version",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.napt_enable,"napt_enable",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.ip_addr,"ip_addr",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.ip_network_mask,"ip_network_mask",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.ipv4_default_gateway_on,"ipv4_default_gateway_on",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.gateway_ipv4_addr,"gateway_ipv4_addr",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.mtu,"mtu",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.gw_mac_auto_learn_for_ipv4,"gw_mac_auto_learn_for_ipv4",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet[0],"gateway_mac_addr_for_ipv4",MAC),RT_ERR_RG_OK);

{//debug
	printk("\n intf-index=%d sessionId=%d \n",wanIntfIdx,clientPppoe_info.sessionId);
	printk("ip-version=%d napt_enable=%d gateway_mac_addr_for_ipv4=%02x:%02x:%02x:%02x:%02x:%02x ip_addr=%x ip_network_mask=%x ipv4_default_gateway_on=%d gateway_ipv4_addr=%x mtu=%d gw_mac_auto_learn_for_ipv4=%d\n",
		clientPppoe_info.hw_info.ip_version,clientPppoe_info.hw_info.napt_enable,clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet[0],clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet[1],clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet[2],
		clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet[3],clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet[4],clientPppoe_info.hw_info.gateway_mac_addr_for_ipv4.octet[5],
		clientPppoe_info.hw_info.ip_addr,clientPppoe_info.hw_info.ip_network_mask,clientPppoe_info.hw_info.ipv4_default_gateway_on,clientPppoe_info.hw_info.gateway_ipv4_addr,clientPppoe_info.hw_info.mtu,clientPppoe_info.hw_info.gw_mac_auto_learn_for_ipv4);
}


	
//	rtk_rg_apollo_pppoeClientInfoBeforeDial_set
	ret=pf.rtk_rg_pppoeClientInfoAfterDial_set(wanIntfIdx,&clientPppoe_info);
	return len;

Error:
	printk("invail parameter!\n");
	return len;
}
int32 rtk_rg_xdsl_dhcpRequest_set( struct file *filp, const char *buff,unsigned long len, void *data )
{

	char *tmpbuf;	
	int ret;
	int32 wanIntfIdx=-1;
	tmpbuf=&rg_kernel.proc_parsing_buf[0];
	if (buff && !copy_from_user(tmpbuf, buff, len)) 
	{		
		tmpbuf[len] = '\0'; 			
	}else{
		goto Error;
	}
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wanIntfIdx,"intf-index",SINGED_LONG),RT_ERR_RG_OK);
	if(wanIntfIdx==-1) goto Error;

{//debug
	printk("\n intf-index=%d \n",wanIntfIdx);
}


//	rtk_rg_apollo_dhcpRequest_set
	ret=pf.rtk_rg_dhcpRequest_set(wanIntfIdx);
	return len;

Error:
	printk("invail parameter!\n");
	return len;

}
int32 rtk_rg_xdsl_dhcpClientInfo_set( struct file *filp, const char *buff,unsigned long len, void *data )
{

	char *tmpbuf;	
	int ret;
	int32 wanIntfIdx=-1;
	rtk_rg_ipDhcpClientInfo_t dhcpClient_info;
	
	tmpbuf=&rg_kernel.proc_parsing_buf[0];
	memset(&dhcpClient_info,0,sizeof(dhcpClient_info));
	if (buff && !copy_from_user(tmpbuf, buff, len)) 
	{		
		tmpbuf[len] = '\0'; 			
	}else{
		goto Error;
	}

	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&wanIntfIdx,"intf-index",SINGED_LONG),RT_ERR_RG_OK);
	if(wanIntfIdx==-1) goto Error;
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.stauts,"stauts",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.ip_version,"ip-version",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.napt_enable,"napt_enable",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.ip_addr,"ip_addr",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.ip_network_mask,"ip_network_mask",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.ipv4_default_gateway_on,"ipv4_default_gateway_on",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.gateway_ipv4_addr,"gateway_ipv4_addr",V4IP),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.mtu,"mtu",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv4,"gw_mac_auto_learn_for_ipv4",SINGED_LONG),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_parseGetValue(tmpbuf,&dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet[0],"gateway_mac_addr_for_ipv4",MAC),RT_ERR_RG_OK);

{//debug
	printk("\n intf-index=%d stauts=%d \n",wanIntfIdx,dhcpClient_info.stauts);
	printk("ip-version=%d napt_enable=%d gateway_mac_addr_for_ipv4=%02x:%02x:%02x:%02x:%02x:%02x ip_addr=%x ip_network_mask=%x ipv4_default_gateway_on=%d gateway_ipv4_addr=%x mtu=%d gw_mac_auto_learn_for_ipv4=%d\n",
		dhcpClient_info.hw_info.ip_version,dhcpClient_info.hw_info.napt_enable,dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet[0],dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet[1],dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet[2],
		dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet[3],dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet[4],dhcpClient_info.hw_info.gateway_mac_addr_for_ipv4.octet[5],
		dhcpClient_info.hw_info.ip_addr,dhcpClient_info.hw_info.ip_network_mask,dhcpClient_info.hw_info.ipv4_default_gateway_on,dhcpClient_info.hw_info.gateway_ipv4_addr,dhcpClient_info.hw_info.mtu,dhcpClient_info.hw_info.gw_mac_auto_learn_for_ipv4);
}

		
//	rtk_rg_apollo_dhcpClientInfo_set
	ret=pf.rtk_rg_dhcpClientInfo_set(wanIntfIdx,&dhcpClient_info);
	return len;

Error:
	printk("invail parameter!\n");
	return len;

}

static rtk_rg_aclFilterAndQos_t aclFilterRule;
extern int32 rtk_rg_apollo_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
extern int32 rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx);
int32 rtk_rg_xdsl_acl_cmd( struct file *filp, const char *buff,unsigned long len, void *data )
{

	char *tmpbuf;	
	char *strptr;
	char *args[10];
	int argc=0;
	int aclIdx;
	int ret;
	
	tmpbuf=&rg_kernel.proc_parsing_buf[0];
	strptr = tmpbuf;
	if (buff && !copy_from_user(tmpbuf, buff, len)) 
	{
		tmpbuf[len] = '\0'; 

		argc=0;
		while(strptr)
		{
			args[argc] = strsep(&strptr," ");
			ACL("cmd%d: %s\n",argc,args[argc]);
			argc++;
			if(argc>=8) break;
		}

		if(!strncmp(args[0],"rg",strlen(args[0])))
		{
			if(!strncmp(args[1],"clear",strlen(args[1])))
			{
				/* rg clear acl-filter */
				if(!strncmp(args[2],"acl-filter",strlen(args[2])))
					bzero(&aclFilterRule,sizeof(aclFilterRule));
			}
			else if(!strncmp(args[1],"add",strlen(args[1])))
			{
				/* rg add acl-filter entry */
				if(!strncmp(args[2],"acl-filter",strlen(args[2])) && !strncmp(args[3],"entry",strlen("entry")))
				{
					ret = rtk_rg_apollo_aclFilterAndQos_add(&aclFilterRule,&aclIdx);
					if(ret)
					{
						ACL("Add Acl rule error - %x !\n",ret);
						return ret;
					}
					ACL("Add acl rule:%d\n",aclIdx);
				}
			}
			else if(!strncmp(args[1],"del",strlen(args[1])))
			{
				/* rg add acl-filter entry */
				if(!strncmp(args[2],"acl-filter",strlen(args[2])) && !strncmp(args[3],"entry",strlen("entry")))
				{
					aclIdx = simple_strtol(args[4],NULL,0);
					if(aclIdx<0 || aclIdx>=MAX_ACL_HW_ENTRY_SIZE)
					{
						ACL("Error entry index: %d\n",aclIdx);
						return RT_ERR_RG_FAILED;
					}
					ret = rtk_rg_apollo_aclFilterAndQos_del(aclIdx);
					if(ret)
					{
						ACL("Del Acl rule error - %x !\n",ret);
						return ret;
					}
					ACL("Del acl rule:%d\n",aclIdx);
				}
			}
			else if(!strncmp(args[1],"set",strlen(args[1])))
			{
				/* rg set acl-filter ... */
				if(!strncmp(args[2],"acl-filter",strlen(args[2])))
				{
					/* rg set acl-filter action ... */
					if(!strncmp(args[3],"action",strlen(args[3])))
					{
						/* rg set acl-filter action action_type <value> */
						if(!strncmp(args[4],"action_type",strlen(args[4])))
						{
							int type;
							type = simple_strtol(args[5],NULL,0);
							switch(type)
							{
   								case 0: //Drop
   									ACL("Set acl action drop\n");
									aclFilterRule.action_type = ACL_ACTION_TYPE_DROP;
									break;
								case 1: //Permit
   									ACL("Set acl action permit\n");
									aclFilterRule.action_type = ACL_ACTION_TYPE_PERMIT;
									break;
								case 2:	//Trap to CPU
   									ACL("Set acl action traps\n");
									aclFilterRule.action_type = ACL_ACTION_TYPE_TRAP;
									break;
								case 3: //QOS/CVLAN/SVLAN/StreamID
									break;
								case 4: //Trap to PS
									break;
								case 5: //Policy route
									break;
								default:
   									ACL("Error!\n");
									break;
							}
						}
						else if(!strncmp(args[4],"qos",strlen("qos")))
						{
							if(!strncmp(args[5],"action_acl_priority",strlen("action_acl_priority")))
							{
								ACL("Set acl action assign priority\n");
								aclFilterRule.action_type = ACL_ACTION_TYPE_QOS;
								aclFilterRule.qos_actions = ACL_ACTION_ACL_PRIORITY_BIT;
								aclFilterRule.action_acl_priority = simple_strtol(args[6],NULL,0);
							}
							else if(!strncmp(args[5],"action_ingress_vid",strlen("action_ingress_vid")))
							{
								ACL("Set acl action assign priority\n");
								aclFilterRule.action_type = ACL_ACTION_TYPE_QOS;
								aclFilterRule.qos_actions = ACL_ACTION_ACL_INGRESS_VID_BIT;
								aclFilterRule.action_acl_ingress_vid = simple_strtol(args[6],NULL,0);
							}
						}
					}
					else if(!strncmp(args[3],"pattern",strlen(args[3])))
					{
						/* rg set acl-filter pattern <field> <value> */
						if(!strncmp(args[4],"ingress_dmac",strlen(args[4])))
						{
							aclFilterRule.filter_fields |= INGRESS_DMAC_BIT;
							strtomac((ether_addr_t*)(&aclFilterRule.ingress_dmac),args[5]);
						}
						else if(!strncmp(args[4],"ingress_dmac_mask",strlen(args[4])))
						{
							strtomac((ether_addr_t*)(&aclFilterRule.ingress_dmac_mask),args[5]);
						}
						else if(!strncmp(args[4],"ingress_smac",strlen(args[4])))
						{
							aclFilterRule.filter_fields |= INGRESS_SMAC_BIT;
							strtomac((ether_addr_t*)(&aclFilterRule.ingress_smac),args[5]);
						}
						else if(!strncmp(args[4],"ingress_smac_mask",strlen(args[4])))
						{
							strtomac((ether_addr_t*)(&aclFilterRule.ingress_smac_mask),args[5]);
						}
						else if(!strncmp(args[4],"ingress_ethertype",strlen(args[4])))
						{
							aclFilterRule.filter_fields |= INGRESS_ETHERTYPE_BIT;
							aclFilterRule.ingress_ethertype = simple_strtol(args[5],NULL,0);
						}
						else if(!strncmp(args[4],"ingress_ethertype_mask",strlen(args[4])))
						{
							aclFilterRule.ingress_ethertype_mask= simple_strtol(args[5],NULL,0);
						}
						else if(!strncmp(args[4],"ingress_port_idx",strlen(args[4])))
						{
							aclFilterRule.filter_fields |= INGRESS_PORT_BIT;
							aclFilterRule.ingress_port_idx = 0x1<<simple_strtol(args[5],NULL,0);
						}
						else if(!strncmp(args[4],"ingress_src_ipv4_addr_start",strlen(args[4])))
						{
							aclFilterRule.filter_fields |= INGRESS_IPV4_SIP_RANGE_BIT;
							in4_pton(args[5],-1,(uint8*)&aclFilterRule.ingress_src_ipv4_addr_start,-1,NULL);
						}
						else if(!strncmp(args[4],"ingress_src_ipv4_addr_end",strlen(args[4])))
						{
							in4_pton(args[5],-1,(uint8*)&aclFilterRule.ingress_src_ipv4_addr_end,-1,NULL);
						}
						else if(!strncmp(args[4],"ingress_src_l4_port_start",strlen(args[4])))
						{
							aclFilterRule.filter_fields |= INGRESS_L4_SPORT_RANGE_BIT;
							in4_pton(args[5],-1,(uint8*)&aclFilterRule.ingress_src_l4_port_start,-1,NULL);
						}
						else if(!strncmp(args[4],"ingress_src_l4_port_end",strlen(args[4])))
						{
							in4_pton(args[5],-1,(uint8*)&aclFilterRule.ingress_src_l4_port_end,-1,NULL);
						}
						else
						{
							ACL("pattern %s NOT supported!\n",args[4]);
							goto errout;
						}
					}
				}
			}
			else
			{
				ACL("RG command %s NOT support!\n",tmpbuf);
				goto errout;
			}
		}
	}
	else
	{
errout:
		printk("rtk_rg_acl: error input\n");
	}
	return len;
}

extern int32 rtk_rg_apollo_cvlan_add(rtk_rg_cvlan_info_t *cvlan_info);
static char		tmpbuf[512];
int32 rtk_rg_xdsl_cvlan( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char		*strptr;
	char		*tokptr;
	char		*pb_pri_en;
	uint32 vid,isIvl;
	rtk_portmask_t member,untag;
	rtk_rg_cvlan_info_t vlan;
	
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;

		/*Set cvlan. e.g. rg set cvlan vlanId 9 isIVL 0 memberPortMask 0x1f untagPortMask 0x1f vlan-based-pri-enable disable */
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"rg"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"set"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"cvlan"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"vlanId"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		vid = simple_strtol(tokptr,NULL,0);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"isIVL"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		isIvl = simple_strtol(tokptr,NULL,0);
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"memberPortMask"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		/*member*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		member.bits[0]= simple_strtol(tokptr,NULL,0);

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"untagPortMask"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		/*untag*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		untag.bits[0]= simple_strtol(tokptr,NULL,0);

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"vlan-based-pri-enable"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		pb_pri_en = strsep(&strptr," ");
		if (pb_pri_en==NULL)
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}

		printk("Set vid[%d] member[0x%x] untag[0x%x] \n",vid,member.bits[0],untag.bits[0]);
		memset(&vlan,0,sizeof(vlan));
		vlan.isIVL = isIvl;
		vlan.memberPortMask.portmask = member.bits[0];
		vlan.untagPortMask.portmask = untag.bits[0];
		vlan.vlanId = vid;
		if(!strcmp(pb_pri_en,"disable"))
			vlan.vlan_based_pri_enable = 0;
		else
			vlan.vlan_based_pri_enable = 1;

		if ( rtk_rg_apollo_cvlan_add(&vlan) != RT_ERR_OK )
			printk("Set cvlan:%d error!\n",vid);		
	}
	else
	{
errout:
		printk("error input\n");
	}

	return len;
}

extern int32 rtk_rg_apollo_qosDot1pPriRemarkByInternalPri_set(int int_pri,int rmk_dot1p);
int32 rtk_rg_xdsl_qos( struct file *filp, const char *buff,unsigned long len, void *data )
{

	char		*strptr;
	char		*tokptr;
	int	dot1pRemark,intPri;
	
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;

		/*Set cvlan. e.g. rg set cvlan vlanId 9 isIVL 0 memberPortMask 0x1f untagPortMask 0x1f vlan-based-pri-enable disable */
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"rg"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"add"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"qosDot1pPriRemarkByInternalPri"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"intPri"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		intPri = simple_strtol(tokptr,NULL,0);
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL || strcmp(tokptr,"dot1p"))
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			printk("command error ... @ %s %d\n",__func__,__LINE__);
			goto errout;
		}
		dot1pRemark = simple_strtol(tokptr,NULL,0);

		printk("Set intPri[%d] dot1p[%d] \n",intPri,dot1pRemark);

		if ( rtk_rg_apollo_qosDot1pPriRemarkByInternalPri_set(intPri,dot1pRemark) != RT_ERR_OK )
			printk("Set qos error!\n");		
	}
	else
	{
errout:
		printk("error input\n");
	}

	return len;
}


/*disable hw l2/l3/l4 trap all packet to cpu  */
int32 rtk_hwEnable(int32 enable)
{
	if(enable){
		WRITE_MEM32(MSCR,READ_MEM32(MSCR) | EN_L2 | EN_L3 | EN_L4);
		WRITE_MEM32(MSCR,READ_MEM32(MSCR) | EN_IN_ACL);
	}else{
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_L2|EN_L3|EN_L4));
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_IN_ACL));		//bypass acl and trap by l2/l3/l4 disable
	}
	return RT_ERR_OK;
}

//xdsl decide device before
struct net_device* decideRxDevice(struct re_private *cp, struct rx_info *pRxInfo)
{
	return cp->dev ;
}

void _rtk_rg_refresh_tcp_longTimeout(int32 longTimeout)
{
	int i,j,outIdx;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_naptOut;

	rtl8651_setAsicNaptTcpLongTimeout(rg_db.systemGlobal.tcp_long_timeout);
	rtl8651_setAsicNaptTcpMediumTimeout(rg_db.systemGlobal.tcp_long_timeout);
	rtl8651_setAsicNaptTcpFastTimeout(rg_db.systemGlobal.tcp_long_timeout);


	for(i=0;i<(MAX_NAPT_OUT_HW_TABLE_SIZE>>5);i++)
	{
		if(rg_db.naptValidSet[i])
		{
			for(j=0;j<32;j++)
			{
				if(rg_db.naptValidSet[i] & (0x1<<j))
				{
					outIdx = (i<<5)+j;
	
					rtl8651_getAsicNaptTcpUdpTable(outIdx,&asic_naptOut);

					if(asic_naptOut.isValid){ //hw only aging long timeout

						//update TCP
						if(asic_naptOut.isTcp){
							
							if(asic_naptOut.ageSec>rg_db.systemGlobal.tcp_long_timeout){
								asic_naptOut.ageSec=rg_db.systemGlobal.tcp_long_timeout;
								rtl8651_setAsicNaptTcpUdpTable(1,outIdx,&asic_naptOut);
							}

						}else{
//							if(asic_naptOut.ageSec>rg_db.systemGlobal.udp_long_timeout){
//								asic_naptOut.ageSec=rg_db.systemGlobal.udp_long_timeout;
//								rtl8651_setAsicNaptTcpUdpTable(outIdx,&asic_naptOut);
//							}
						}
					}
				}
			}
		}
	}


	return;
}

void _rtk_rg_refresh_udp_longTimeout(int32 longTimeout)
{
	int i,j,outIdx;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_naptOut;

	rtl8651_setAsicNaptIcmpTimeout(rg_db.systemGlobal.udp_long_timeout);
	rtl8651_setAsicNaptUdpTimeout(rg_db.systemGlobal.udp_long_timeout);

	for(i=0;i<(MAX_NAPT_OUT_HW_TABLE_SIZE>>5);i++)
	{
		if(rg_db.naptValidSet[i])
		{
			for(j=0;j<32;j++)
			{
				if(rg_db.naptValidSet[i] & (0x1<<j))
				{
					outIdx = (i<<5)+j;
	
					rtl8651_getAsicNaptTcpUdpTable(outIdx,&asic_naptOut);

					if(asic_naptOut.isValid){ //hw only aging long timeout

						//update UDP
						if(asic_naptOut.isTcp){		
//							if(asic_naptOut.ageSec>rg_db.systemGlobal.tcp_long_timeout){
//								asic_naptOut.ageSec=rg_db.systemGlobal.tcp_long_timeout;
//								rtl8651_setAsicNaptTcpUdpTable(outIdx,&asic_naptOut);
//							}
						}else{
							if(asic_naptOut.ageSec>rg_db.systemGlobal.udp_long_timeout){
								asic_naptOut.ageSec=rg_db.systemGlobal.udp_long_timeout;
								rtl8651_setAsicNaptTcpUdpTable(1,outIdx,&asic_naptOut);
							}
						}
					}
				}
			}
		}
	}

	return;
}


//pScanIdx = idx
int32 rtk_l2_nextValidAddr_get_byidx(int32 *pScanIdx,rtk_l2_ucastAddr_t  *pL2UcastData)
{

	int32 ret;
	uint32 row, column;
	uint32 tmpMask;
	int i;
	rtl865x_tblAsicDrv_l2Param_t r;
	int32 l2PortID=-1;	// P0-P8   P6P7P8=ext port
	row = *pScanIdx >>2;
	column = *pScanIdx&0x3;
	//TRACE(">>>>> pScanIdx(%d,%d)=%d\n",row,column, *pScanIdx);

	//move to next entry
	ret = rtl8651_getAsicL2Table(row,column,&r);
	if(ret!=SUCCESS)	
		return RT_ERR_FAILED;
	//fill the entry data
	bzero(pL2UcastData,sizeof(rtk_l2_ucastAddr_t));
	memcpy(pL2UcastData->mac.octet,r.macAddr.octet,sizeof(rtk_mac_t));
	pL2UcastData->fid = (uint32)r.fid;
	pL2UcastData->efid = 0; 	
	//pL2UcastData->port = r.memberPortMask&0x3F;
	pL2UcastData->port = 0;

	tmpMask = r.memberPortMask&0x7F;
	for(i=0;i<9;i++)
	{
		if( (tmpMask &(1<<i)) !=0)
		{
			l2PortID = i;	
			break;
		}
	}


	if(l2PortID ==-1){//just to cpu

		if(!r.cpu){
			DEBUG("no member but not to CPU Bug!\n");
		}
		pL2UcastData->port =RTK_RG_MAC_PORT_CPU;

	}else if(l2PortID<=5){

		pL2UcastData->port=l2PortID;
	}else{ // l2PortID>=6
		pL2UcastData->port =RTK_RG_MAC_PORT_CPU;
		pL2UcastData->ext_port= (l2PortID-5); //P6P7P8 -> P1P2P3 (extP0=CPU)

	}


	if(l2PortID ==-1){
		pL2UcastData->vid =1; //CPU
	}else{
		if(rtl8651_getAsicPVlanId(l2PortID,&(pL2UcastData->vid)) != SUCCESS){
			DEBUG("L2 Table get vid Fail\n");
		}
	}
//	FIXME("L2 vid using PVID=%d\n",pL2UcastData->vid);


	
	pL2UcastData->age = (r.ageSec/150); //FIXME:should care the uint 
	pL2UcastData->auth = (uint8)r.auth;
	pL2UcastData->flags = 0;
	//indext = nexthop idx?!
	pL2UcastData->index = *pScanIdx;
	if(r.srcBlk == TRUE)	pL2UcastData->flags |=RTK_L2_UCAST_FLAG_SA_BLOCK;
	if(r.isStatic==TRUE)	pL2UcastData->flags |=RTK_L2_UCAST_FLAG_STATIC;
//	pL2UcastData->flags |=RTK_L2_UCAST_FLAG_ARP_USED;	//always on

	return RT_ERR_OK;


}


/*
	if hw vaild && software invaild  
		sync hw to sw
	else if hw vaild and software vaild 
		expect same entry check port mvoing
	else
		unexpecte way
*/
int32 _rtk_syncLutTb4way_FormHw2Sw(unsigned char *mac)
{
	int32 l2Idx,oriHashL2idx;
	int32 i;
	int32 ret;
	rtk_l2_addr_table_t  L2UcastData;

	// rtl8651_filterDbIndex((ether_addr_t *)mac, fid);
	oriHashL2idx = _rtk_rg_hash_mac_fid_efid(mac,LAN_FID,0) << 2;

	for(i=0;i<4;i++)
	{
		l2Idx=oriHashL2idx=i;
		bzero(&L2UcastData,sizeof(L2UcastData));
		ret=rtk_l2_nextValidAddr_get_byidx(&l2Idx,&(L2UcastData.entry.l2UcEntry));
		L2UcastData.entryType=RTK_LUT_L2UC; 

		if(ret != SUCCESS)
			continue;
#if 0   //if inefficiency try to open
		//	(only care this mac)
		if( (ret!=RT_ERR_OK) || (memcmp(mac,&(L2UcastData.entry.l2UcEntry.mac.octet[0]),ETHER_ADDR_LEN)!=0))
			continue;
#endif
		
		if( (l2Idx == L2UcastData.entry.l2UcEntry.index)  && rg_db.lut[l2Idx].valid==0)
		{ 
			//find a hw valid and software invalid entry ,sync to software 
			bzero(&rg_db.lut[l2Idx],sizeof(rtk_rg_table_lut_t));
			memcpy(&(rg_db.lut[l2Idx].rtk_lut),&L2UcastData,sizeof(L2UcastData));
			rg_db.lut[l2Idx].valid=1;
			TRACE("SYNC HW idx=[%d] to software mac=%02d:%02d:%02d:%02d:%02d:%02d",l2Idx,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		}
		else if ( (l2Idx == L2UcastData.entry.l2UcEntry.index)  && rg_db.lut[l2Idx].valid==1)
		{
			// hw valid and software valid entry
			// same entry 
			if(memcmp(&(L2UcastData.entry.l2UcEntry.mac.octet[0]),&(rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.mac.octet[0]),ETHER_ADDR_LEN)==0)
			{
				if(L2UcastData.entry.l2UcEntry.port != rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.port ||
				   L2UcastData.entry.l2UcEntry.ext_port!= rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.ext_port)
				{
					//port moving
					TRACE("L2 entry[%d] Port moving from phy %d to %d , ext %d to %d ",l2Idx,
						rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.port,L2UcastData.entry.l2UcEntry.port,
						rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.ext_port,L2UcastData.entry.l2UcEntry.ext_port);
					
					rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.ext_port= L2UcastData.entry.l2UcEntry.ext_port;
					rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.port = L2UcastData.entry.l2UcEntry.port;
				}
			}
			else
			{
				//unexpecte way
				WARNING(" unexpecte way Error %d software and hardware not sync !! ",l2Idx);
			}
		}
		else
		{
			//unexpecte way
			WARNING(" unexpecte way Error %d software and hardware not sync !! ",l2Idx);
		}

	}

	return RT_ERR_OK;
}



/* Function Name:
 *      rtk_l2_nextValidEntry_get
 * Description:
 *      using pScanIdx Get LUT next valid entry(include pScanIdx entry).
 * Input:
 *      pScanIdx - Index field in the structure.
 * Output:
 *      pL2Entry - entry content
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get next valid LUT entry.
 */
//done
int32 rtk_l2_nextValidEntry_get(int32	*pScanIdx,rtk_l2_addr_table_t     *pL2Entry)
{
	pL2Entry->entryType = RTK_LUT_L2UC; 	
	return rtk_l2_nextValidAddr_get(pScanIdx,&(pL2Entry->entry.l2UcEntry));
}


/* Function Name:
 *      rtk_l2_nextValidAddr_get
 * Description:
 *      Get next valid L2 unicast address entry from the specified device.
 * Input:
 *      pScanIdx       - currently scan index of l2 table to get next.
 *      include_static - the get type, include static mac or not.
 * Output:
 *      pL2UcastData   - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      (1) The function will skip valid l2 multicast and ip multicast entry and
 *          reply next valid L2 unicast address is based on index order of l2 table.
 *      (2) Please input 0 for get the first entry of l2 table.
 *      (3) The pScanIdx is the input and also is the output argument.
 */
int32 rtk_l2_nextValidAddr_get(int32 *pScanIdx,rtk_l2_ucastAddr_t  *pL2UcastData)
{

	int32 ret;
	uint32 row, column;
	uint32 tmpMask;
	int i;
	rtl865x_tblAsicDrv_l2Param_t r;
	int32 l2PortID=-1;	// P0-P8   P6P7P8=ext port
	row = *pScanIdx >>2;
	column = *pScanIdx&0x3;
//	TRACE(">>>>> pScanIdx(%d,%d)=%d\n",row,column, *pScanIdx);
	while(1)
	{
		//move to next entry
		ret = rtl8651_getAsicL2Table(row,column,&r);
		if(ret==SUCCESS)	break;
		
		column++;
		if(column>=RTL8651_L2TBL_COLUMN)
		{	
			row++;
			column = 0;
		}
		if(row>=RTL8651_L2TBL_ROW)	goto fail_to_find;
		*pScanIdx = row <<2 | column;
	}	

	//fill the entry data
	bzero(pL2UcastData,sizeof(rtk_l2_ucastAddr_t));
	memcpy(pL2UcastData->mac.octet,r.macAddr.octet,sizeof(rtk_mac_t));
	pL2UcastData->fid = (uint32)r.fid;
	pL2UcastData->efid = 0; 	
	//pL2UcastData->port = r.memberPortMask&0x3F;
	pL2UcastData->port = 0;

	tmpMask = r.memberPortMask&0x7F;
	for(i=0;i<9;i++)
	{
		if( (tmpMask &(1<<i)) !=0)
		{
			l2PortID = i;	
			break;
		}
	}


	if(l2PortID ==-1){//just to cpu

		if(!r.cpu){
			DEBUG("no member but not to CPU Bug!\n");
		}
		pL2UcastData->port =RTK_RG_MAC_PORT_CPU;

	}else if(l2PortID<=5){

		pL2UcastData->port=l2PortID;
	}else{ // l2PortID>=6
		pL2UcastData->port =RTK_RG_MAC_PORT_CPU;
		pL2UcastData->ext_port= (l2PortID-5); //P6P7P8 -> P1P2P3 (extP0=CPU)

	}


	if(l2PortID ==-1){
		pL2UcastData->vid =1; //CPU
	}else{
		if(rtl8651_getAsicPVlanId(l2PortID,&(pL2UcastData->vid)) != SUCCESS){
			DEBUG("L2 Table get vid Fail\n");
		}
	}
//	FIXME("L2 vid using PVID=%d\n",pL2UcastData->vid);


	
	pL2UcastData->age = (r.ageSec/150); //FIXME:should care the uint 
	pL2UcastData->auth = (uint8)r.auth;
	pL2UcastData->flags = 0;
	//indext = nexthop idx?!
	pL2UcastData->index = *pScanIdx;
	if(r.srcBlk == TRUE)	pL2UcastData->flags |=RTK_L2_UCAST_FLAG_SA_BLOCK;
	if(r.isStatic==TRUE)	pL2UcastData->flags |=RTK_L2_UCAST_FLAG_STATIC;
//	pL2UcastData->flags |=RTK_L2_UCAST_FLAG_ARP_USED;	//always on

	
	return RT_ERR_OK;
fail_to_find:
	return RT_ERR_RG_INVALID_PARAM;

}




/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get L2 entry based on specified vid and MAC address
 * Input:
 *      None
 * Output:
 *      pL2Addr - pointer to L2 entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_VLAN_VID          - invalid vlan id
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port and fid where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
 /* using mac to find hw entry */
int32 rtk_l2_addr_get(rtk_l2_ucastAddr_t *pL2Addr)
{
	int i,ret;	
	rtl865x_tblAsicDrv_l2Param_t rowl2;
	uint32 L2hash,idx;
	L2hash = rtl8651_filterDbIndex((ether_addr_t *)(&(pL2Addr->mac)),pL2Addr->fid);

	for(i=0;i<4;i++){
		ret = rtl8651_getAsicL2Table(L2hash,i,&rowl2);
		if(ret==SUCCESS && memcmp(&(rowl2.macAddr.octet),&(pL2Addr->mac.octet),6)==0){
			idx= (L2hash << 2)+ i ;
			if(rtk_l2_nextValidAddr_get_byidx(&idx,pL2Addr)==RT_ERR_OK)
				return RT_ERR_OK;
		}
	}
	return RT_ERR_L2_ENTRY_NOTFOUND;
}


/* Module Name    : L2              */
/* Sub-module Name: Unicast address */

/* Function Name:
 *      rtk_l2_addr_add
 * Description:
 *      Add L2 entry to ASIC.
 * Input:
 *      pL2_addr - L2 entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      Need to initialize L2 entry before add it.
 */
 extern int32 convert_setAsicL2Table(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, 
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag,int8 fid, int8 auth);
int32 rtk_l2_addr_add(rtk_l2_ucastAddr_t *pL2Addr)
{
	int ret=RT_ERR_FAILED;
	int l2Idx;
	uint32 nhflag=0;
	uint8 SrcBlk=0;
	uint16 isStatic=0;
	uint8 fid=0;
	uint32 mbr=0;	//it a bitmask
	uint32 cpu=0;
	uint32 aging=0;
	int count=0;
	int first_invalid=-1;
	int search_index=0,mac_exist=0;


//================== Transfer to 865x here===================

/*
//865x unsupport RTK_L2_UCAST_FLAG_FWD_PRI RTK_L2_UCAST_FLAG_LOOKUP_PRI and RTK_L2_UCAST_FLAG_IVL(SVL IVL select by fid,fid=0 SVL)

#define RTK_L2_UCAST_FLAG_SA_BLOCK          0x00000001
#define RTK_L2_UCAST_FLAG_DA_BLOCK          0x00000002
#define RTK_L2_UCAST_FLAG_STATIC            0x00000004
#define RTK_L2_UCAST_FLAG_ARP_USED          0x00000008
#define RTK_L2_UCAST_FLAG_FWD_PRI           0x00000010
#define RTK_L2_UCAST_FLAG_LOOKUP_PRI        0x00000020
#define RTK_L2_UCAST_FLAG_IVL               0x00000040
*/

//	TRACE("pL2Addr->port =%d  pL2Addr->ext_port=%d \n",pL2Addr->port,pL2Addr->ext_port);

	if(pL2Addr->port ==6){

		if(pL2Addr->ext_port==0){
			cpu=1;
			mbr=0;
		}
		else
		{
			/* ext_port==0 mean cpu port*/
			cpu=1;
			mbr= (pL2Addr->ext_port -1) << 6;

		}
	}
	else
	{
		mbr= 1<<(pL2Addr->port);
	}

	if(pL2Addr->flags & RTK_L2_UCAST_FLAG_SA_BLOCK)
	{
		SrcBlk=1;
	}

	if(pL2Addr->flags & RTK_L2_UCAST_FLAG_STATIC)
	{
		isStatic=1;
	}
	
	aging=(pL2Addr->age)*150;

	//always set fix value for nhflag
	nhflag=1;




#if 0
	if(pL2Addr->flags & RTK_L2_UCAST_FLAG_STATIC)
	{
		isStatic=1;
	}

	if(pL2Addr->flags & RTK_L2_UCAST_FLAG_ARP_USED)
	{
		nhflag=1;
	}
#endif


	if(pL2Addr->flags & RTK_L2_UCAST_FLAG_IVL)
	{
		TABLE("xdsl no IVL mode\n");
/*		//IVL
		fid=pL2Addr->fid;
		if(fid ==0){
			DEBUG("!!!WHY IVL but fid =0\n")
	}
*/

	}
	else
	{
		//SVL
		fid=0;
	}

/*we expect L2entryIndex=first_invalid  in hw  Boyce 2014-08-18*/
//find first invalid empty(=first_invalid) and check mac exit(=search_index).
	l2Idx = _rtk_rg_hash_mac_fid_efid(pL2Addr->mac.octet, pL2Addr->fid,0) << 2;

	do
	{
		search_index = l2Idx+count;
//		DEBUG("search_idx is %d",search_index);
		if(rg_db.lut[search_index].valid==0)
		{
			if(first_invalid==-1)
				first_invalid=search_index;
			//break;	//empty
			count++; //search from next entry
			continue;
		}
						
		if(rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L2UC &&
			(!memcmp(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.mac.octet,pL2Addr->mac.octet,ETHER_ADDR_LEN)))
		{
			if(((pL2Addr->flags & RTK_L2_UCAST_FLAG_IVL) && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==fid) ||
				(!(pL2Addr->flags & RTK_L2_UCAST_FLAG_IVL) && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==LAN_FID))
			{
				//DEBUG("MAC is exist!");
				mac_exist=1;
					break;
			}
		}
		else
			count++; //search from next entry
	}
	while(count < 4);



	if(mac_exist)
	{
		//	update entry
		TABLE("Update L2 Table index =%d spa=%d fid=%d  mac=%x-%x-%x-%x-%x-%x!!!!!!!!\n",
		search_index,pL2Addr->port,fid,pL2Addr->mac.octet[0],pL2Addr->mac.octet[1],pL2Addr->mac.octet[2],pL2Addr->mac.octet[3],pL2Addr->mac.octet[4],pL2Addr->mac.octet[5]);

		ret = convert_setAsicL2Table(search_index>>2,search_index&0x3,(ether_addr_t *)(&(pL2Addr->mac)),cpu,
									SrcBlk,mbr,aging,isStatic,nhflag,fid,pL2Addr->auth);
		pL2Addr->index =search_index;

	}
	else
	{
		//	sw table and hw table  synchronous ,so find invalid empty in software and force set to the HW(row,col)position
		TABLE("ADD L2 Table index =%d  spa=%d fid=%d  mac=%x-%x-%x-%x-%x-%x!!!!!!!!\n",
		first_invalid,pL2Addr->port,fid,pL2Addr->mac.octet[0],pL2Addr->mac.octet[1],pL2Addr->mac.octet[2],pL2Addr->mac.octet[3],pL2Addr->mac.octet[4],pL2Addr->mac.octet[5]);

		ret = convert_setAsicL2Table(first_invalid>>2,first_invalid&0x3,(ether_addr_t *)(&(pL2Addr->mac)),cpu,
									SrcBlk,mbr,aging,isStatic,nhflag,fid,pL2Addr->auth);
		pL2Addr->index =first_invalid;
	}

//================== Transfer to 865x end===================

	return ret;

}


/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete a L2 unicast address entry.
 * Input:
 *      pL2Addr  - L2 entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
int32 rtk_l2_addr_del(rtk_l2_ucastAddr_t *pL2Addr)
{
	int ret=RT_ERR_FAILED;
	rtl865x_tblAsicDrv_l2Param_t entry;
	//check fid and mac and delete by index
	ret=rtl8651_getAsicL2Table(pL2Addr->index >> 2,pL2Addr->index &0x3,&entry);
	if(ret==RT_ERR_OK){
		if( (pL2Addr->fid==entry.fid) && (memcmp(pL2Addr->mac.octet,entry.macAddr.octet,6)==0)){
			ret=rtl8651_delAsicL2Table(pL2Addr->index >> 2, pL2Addr->index &0x3);
		}else{
			DEBUG("L2 del fail A BUG HERE\n");
		}
	}
	return ret;
}

/* Function Name:
 *      rtk_l34_pppoeTable_set
 * Description:
 *      Set PPPoE table entry
 * Input:
 *      idx - index of PPPoE table
 *      *pppEntry -point of PPPoE entry
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_pppoeTable_set(uint32 idx, rtk_l34_pppoe_entry_t *pppEntry)
{
	rtl865x_tblAsicDrv_pppoeParam_t pppoet;
	pppoet.age=0x7;
	pppoet.sessionId= pppEntry->sessionID;
	return rtl8651_setAsicPppoe(idx, &pppoet);
}

int32 rtk_pppoe_getIdx(int32 sessionId)
{
	int i;

	for(i=0 ;i<MAX_PPPOE_SW_TABLE_SIZE;i++){
		if(rg_db.pppoe[i].rtk_pppoe.sessionID ==sessionId)
			break;
	}
	if(i==8){
		WARNING("can't find pppoe session =%d \n",sessionId);
		return FAILED;
	}else{
		return i;
	}
}


/* Function Name:
 *      rtk_l34_nexthopTable_set
 * Description:
 *      Set  Next-Hop table entry
 * Input:
 *      idx - index of  Next-Hop table
 *      *nextHopEntry -point of  Next-Hop entry
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */

//apollo idx =0-15 but 865x 0-31 
int32 rtk_l34_nexthopTable_set(uint32 idx, rtk_l34_nexthop_entry_t *entry)
{
	rtl865x_tblAsicDrv_nextHopParam_t t;
	if(idx*2+1 >= RTL8651_NEXTHOPTBL_SIZE)
		return RT_ERR_FAILED;
	
	bzero(&t,sizeof(rtl865x_tblAsicDrv_nextHopParam_t));

	t.nextHopRow = ((uint32)entry->nhIdx)>>2;
	t.nextHopColumn = ((uint32)entry->nhIdx)&0x3;
	t.isPppoe = (entry->type==L34_NH_PPPOE)? 1:0;
	if(t.isPppoe){
		t.pppoeIdx=entry->pppoeIdx;
	}
	//t.extIntIpIdx = entry->extIntIpIdx;	//: need this
	t.dvid = entry->ifIdx;	
//				= entry->keepPppoe
	
	//apollo idx =0-15 but 865x 0-31
	rtl8651_setAsicNextHopTable(idx*2,&t);
	rtl8651_setAsicNextHopTable(idx*2+1,&t);
	
	return RT_ERR_OK;

}

/* Function Name:
 *      rtk_l34_netifTable_set
 * Description:
 *      Set netif table entry
 * Input:
 *      idx - index of netif table
 *      *netifEntry -point of netif entry 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_netifTable_set(uint32 idx, rtk_l34_netif_entry_t *entry)
{

	int ret=RT_ERR_FAILED;
	rtl865x_tblAsicDrv_intfParam_t t;

	bzero(&t,sizeof(rtl865x_tblAsicDrv_intfParam_t));
	memcpy(t.macAddr.octet, entry->gateway_mac.octet,ETHER_ADDR_LEN);

	switch(entry->mac_mask) {
		case 0x7:
			t.macAddrNumber = 1;		
		break;
		case 0x6:
			t.macAddrNumber = 2;
		break;
		case 0x4:
			t.macAddrNumber = 4;
		break;
		case 0:
			t.macAddrNumber = 8;
		break;
				
		default:
		DEBUG("Not permitted macNumber value\n");
		    return RT_ERR_FAILED;//Not permitted macNumber value
				}

	t.vid = entry->vlan_id;
	t.inAclStart = 0;
	t.inAclEnd = MAX_ACL_IGR_ENTRY_SIZE-1;
	t.outAclStart = MAX_ACL_IGR_ENTRY_SIZE;
	t.outAclEnd = MAX_ACL_HW_ENTRY_SIZE-1;
	t.mtu = (uint32)entry->mtu;
	t.enableRoute = entry->enable_rounting;
	t.valid = entry->valid;
#if defined(CONFIG_RTL_8685S_HWNAT)
	t.v6mtu=entry->v6mtu;
	t.enablev6Route =entry->enable_v6Route;
#endif

	ret = rtl8651_setAsicNetInterface(idx,&t);
	TRACE("RTK_L34_NETIFTABLE_SET(%d)=%x:%x:%x:%x:%x:%x\n",idx,
	 entry->gateway_mac.octet[0], entry->gateway_mac.octet[1], entry->gateway_mac.octet[2],
	 entry->gateway_mac.octet[3], entry->gateway_mac.octet[4],  entry->gateway_mac.octet[5]);

	return ret;
}

/* Function Name:
 *      rtk_l34_routingTable_set
 * Description:
 *      Set Routing table entry
 * Input:
 *      idx - index of Routing table
 *      *routEntry -point of Routing entry
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_routingTable_set(uint32 idx, rtk_l34_routing_entry_t *entry)
{
	int ret=RT_ERR_FAILED ;
	rtl865x_tblAsicDrv_routingParam_t r;	//to fill asic entry directly

	if(entry->valid==0){
		rtl8651_delAsicRouting(idx);
		return SUCCESS;
	}
	bzero(&r,sizeof(rtl865x_tblAsicDrv_routingParam_t));
	r.ipAddr = entry->ipAddr;
	r.internal = (uint16)entry->internal;	//how to set DMZ?!
//	r.DMZFlag = entry->rt2waninf ; //unsure
	

	if(entry->ipMask==0){
		r.ipMask =0;
		}
	else{
		r.ipMask =~((1<<(31-entry->ipMask))-1);
	}

	switch(entry->process)
	{
		case L34_PROCESS_CPU: //cpu
			r.process = 0x4; 
		break; 

		case L34_PROCESS_DROP: //drop
			r.process = 0x6; 
		break; 

		case L34_PROCESS_ARP: //arp
			r.process = 0x2; 
			r.vidx = entry->netifIdx;	
			r.arpStart = entry->arpStart<<2;
			r.arpEnd   = entry->arpEnd<<2;
		break; 
	
		case L34_PROCESS_NH: //nexthop
			r.process = 0x5; 
			r.nhStart = ((uint32)entry->nhStart)<<1; //865x(0-31)32 entry ,apollo(0-15)16 entry 
			r.nhNxt = ((uint32)entry->nhNxt)<<1; 	 //only nhAlgo=0 valid ,point to nexthop table direct apollo(16) ->xdsl(32)
			r.nhAlgo = (uint32)entry->nhAlgo;
			r.ipDomain = (uint32)entry->ipDomain; //point to ip table append 8(apollo)-> 16(865x) entry
			switch(entry->nhNum)
	{
				case 0: r.nhNum = 2; break;
				case 1: r.nhNum = 4; break;
				case 2: r.nhNum = 8; break;
				case 3: r.nhNum = 16; break;
				case 4: r.nhNum = 32; break;
				default:
					return RT_ERR_FAILED;
	}

					break;
		
		case 4: 
		default: //default drop
			r.process = 6; 
					break;
				}
	
				
	ret = rtl8651_setAsicRouting(idx, &r);
	DEBUG("ADD L3 ret=%d rt2waninf=%d netifIdx=%d entry->process=%d\n",ret,entry->rt2waninf,entry->netifIdx,entry->process);	


	return ret;
}

/* Sub-module Name: External_Internal IP Table */

/* Function Name:
 *      rtk_l34_extIntIPTable_set
 * Description:
 *      Set  External_Internal IP table entry
 * Input:
 *      idx - index of  External_Internal IP table
 *      *extIpEntry -point of External_Internal IP entry
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
 
//for apollo extip table 8 entry but 865x append to 16 entry we always using entry 0-7
int32 rtk_l34_extIntIPTable_set(uint32 idx, rtk_l34_ext_intip_entry_t *entry)
{
	rtl865x_tblAsicDrv_routingParam_t routeEt;
	rtl865x_tblAsicDrv_nextHopParam_t nextHopEt;
	rtl865x_tblAsicDrv_extIntIpParam_t r;
	int ret=RT_ERR_FAILED ;
	int i;
	bzero(&r,sizeof(r));

	if( entry->valid == 0)
	{
	    ret=rtl8651_delAsicExtIntIpTable(idx);
		return ret;
	}
	
	r.extIpAddr = entry->extIpAddr;
	r.intIpAddr = entry->intIpAddr;
	r.nhIndex = (entry->nhIdx)*2; //16 entry mapping to 32 
	switch(entry->type)
	{
		case L34_EXTIP_TYPE_NAPT: 
			r.localPublic=0;
			r.nat=0;
			if(entry->intIpAddr!=0)
				DEBUG("intIpAddr should ==0 bug\n");
			break;
		case L34_EXTIP_TYPE_NAT:
			r.localPublic=0;
			r.nat=1;
			break;
		case L34_EXTIP_TYPE_LP:
			r.localPublic=1;
			r.nat=0;
			if(entry->extIpAddr != entry->intIpAddr)
				DEBUG("extIpAddr should same as intIpAddr\n");
			break;
		default:
			FIXME("what case !?\n");
			r.localPublic=1;
			r.nat=1;
			break;
	}

	//for apollo extip table 8 entry but 865x append to 16 entry 
	ASSERT_EQ(rtl8651_setAsicExtIntIpTable(idx,&r),RT_ERR_RG_OK);

	/* Set back to routing table for interface route Boyce 2015-08-25*/
	//_rtk_rg_internal_wanSet
	for(i=0 ;i<RTL8651_ROUTINGTBL_SIZE;i++){
		
		ASSERT_EQ(rtl8651_getAsicRouting(idx, &routeEt),RT_ERR_RG_OK);
		if( (routeEt.ipAddr == entry->extIpAddr) && (routeEt.process==0x2/*ARP*/) )
		{
			routeEt.arpIpIdx =idx;
			ASSERT_EQ(rtl8651_setAsicRouting(idx, &routeEt),RT_ERR_RG_OK);
		}
		else if((routeEt.ipAddr == entry->extIpAddr) && (routeEt.process==0x5/*NEXTHOP*/))
		{
			ASSERT_EQ(rtl8651_getAsicNextHopTable((routeEt.nhStart)*2,&nextHopEt),RT_ERR_RG_OK);
			nextHopEt.extIntIpIdx=idx;
			ASSERT_EQ(rtl8651_setAsicNextHopTable((routeEt.nhStart)*2+1,&nextHopEt),RT_ERR_RG_OK);
		}
		
	}


	return RT_ERR_OK;

}

/* Function Name:
 *      rtk_l34_arpTable_set
 * Description:
 *      Set arp table entry
 * Input:
 *      idx - index of arp table
 *      *arpEntry -point of arp entry
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_arpTable_set(uint32 idx, rtk_l34_arp_entry_t *entry)
{
	int ret=RT_ERR_FAILED;
	rtl865x_tblAsicDrv_arpParam_t t;	
	if(entry->valid==0){ //delete entry
		ret=rtl8651_delAsicArp(idx);
		return ret;
	}
	t.nextHopRow = ((uint32)entry->nhIdx)>>2;
	t.nextHopColumn = ((uint32)entry->nhIdx)&0x3;
	t.aging = 31;	//range 0-31 
	ret = rtl8651_setAsicArp(idx,&t);
	return ret;

}

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize vlan module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize vlan module before calling any vlan APIs.
 */
int32 rtk_vlan_init(void)
{
	return rtl865x_initVlanTable();
}


/* Function Name:
 *      rtk_vlan_create
 * Description:
 *      Create the vlan in the specified device.
 * Input:
 *      vid  - vlan id to be created
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_VLAN_VID   - invalid vid
 *      RT_ERR_VLAN_EXIST - vlan is exist
 * Note:
 */
int32 rtk_vlan_create(rtk_vlan_t vid)
{
	int ret;
	if(vid < 0 || vid > VLAN_NUMBER-1)
		return RT_ERR_VLAN_VID;
	ret=rtl865x_addVlan(vid);
	DEBUG("add vlan ret=%d vid= %d\n",ret,vid);
	//we always return success
	return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_port_set
 * Description:
 *      Replace the vlan members.
 * Input:
 *      vid              - vlan id
 *      pMember_portmask - member ports
 *      pUntag_portmask  - untagged member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Don't care the original vlan members and replace with new configure
 *      directly.
 *      If users specify an empty extension portmask and CPU port is set in pMember_portmask,
 *      the packets will be restricted to be forwarded to CPU.
 *      Likewise, If users specify an non-empty extension portmask and
 *      CPU port is not set in pMember_portmask, the packets will be restricted to be forwarded to CPU. too.
 */
int32 rtk_vlan_port_set(rtk_vlan_t vid,rtk_portmask_t *pMember_portmask, rtk_portmask_t *pUntag_portmask)
{

	int ret;
	uint32 portMask,untagset=0;
	int hwPortMask=0 ;
	int extPort_865x=0;

	portMask = (*pMember_portmask).bits[0];
	untagset = (*pUntag_portmask).bits[0];
	extPort_865x = (rg_db.vlan[vid].Ext_portmask.bits[0] >>1)&0x7;
	hwPortMask = (portMask&0x3f) | (extPort_865x<<6);
	untagset = untagset&0x3f;
	if(portMask & (1<<6))
		WARNING("xdsl vlan table not support cpu port \n");
	ret=rtl865x_modVlanPortMember(vid,hwPortMask,untagset);
	return ret;
}

/* Function Name:
 *      rtk_vlan_fid_set
 * Description:
 *      Set the filter id of the vlan.
 * Input:
 *      vid  - vlan id
 *      fid  - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 */
int32 rtk_vlan_fid_set(rtk_vlan_t vid, rtk_fid_t fid)
{
	return rtl865x_setVlanFilterDatabase(vid,fid);
}


/* Function Name:
 *      rtk_vlan_extPort_set
 * Description:
 *      Replace the vlan extension members.
 * Input:
 *      vid              - vlan id
 *      pExt_portmask    - extension member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Don't care the original vlan members and replace with new configure
 *      directly.
 *      If users specify an empty extension portmask and CPU port is set in pMember_portmask,
 *      the packets will be restricted to be forwarded to CPU.
 *      Likewise, If users specify an non-empty extension portmask and
 *      CPU port is not set in pMember_portmask, the packets will be restricted to be forwarded to CPU. too.
 */
int32 rtk_vlan_extPort_set(rtk_vlan_t vid,rtk_portmask_t *pExt_portmask)
{
	int ret;
	int extPort_865x =(*pExt_portmask).bits[0];
	int hwPortMask=0;
	int untagset=0;
	//ingore cpu port bit'0 and  865x only design 3 extPort  
	extPort_865x = (extPort_865x >>1)&0x7 ;

	hwPortMask = (rg_db.vlan[vid].MemberPortmask.bits[0] &0x3f) | (extPort_865x<<6) ;

	//ingore cpu bit and set extPort  always set untag
	untagset = (rg_db.vlan[vid].UntagPortmask.bits[0])&0x3f;

	ret=rtl865x_modVlanPortMember(vid,hwPortMask,untagset);
	return ret;
}



/* Function Name:
 *      rtk_vlan_destroy
 * Description:
 *      Destroy the vlan.
 * Input:
 *      vid  - vlan id to be destroyed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 * Note:
 *      None
 */
int32 rtk_vlan_destroy(rtk_vlan_t vid)
{
	/* we always return success*/
	rtl865x_delVlan(vid);
	return RT_ERR_OK;
		
}


/* Function Name:
 *      rtk_vlan_portPvid_set
 * Description:
 *      Set port default vlan id.
 * Input:
 *      port - port id
 *      pvid - port default vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */

int32 rtk_vlan_portPvid_set(rtk_port_t port, uint32 pvid)
{
	if(port<0 || port>6)
		return RT_ERR_FAILED;
	if(port == 6){
		WARNING("not support cport pvid\n");
		return RT_ERR_OK;
	}
	return rtl8651_setAsicPvid(port,pvid); /* port range 0-5 phyPort 6=ext0 7=ext1 8=ext2  pvid:0-4095 */
}


/* Function Name:
 *      rtk_vlan_extPortPvid_set
 * Description:
 *      Set extension port default vlan id.
 * Input:
 *      extPort - extension port id
 *      pvid    - extension port default vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      Configuration on EXT port 0 will be applied to physical port 6.
 */
 
/*export range 1-5*/
int32 rtk_vlan_extPortPvid_set(uint32 extPort, uint32 pvid)
{
	if(extPort<0 || extPort>5)
		return RT_ERR_PORT_ID;
	if(extPort >3){
		WARNING("not support extPort=%d > 3  \n",extPort);
		return RT_ERR_OK;
	}
	
	return rtl8651_setAsicPvid(extPort+5,pvid); /* port range 0-5 phyPort 6=ext0 7=ext1 8=ext2  pvid:0-4095 */
}



/* Function Name:
 *      rtk_l34_naptInboundTable_set
 * Description:
 *      Set  NAPTR table entry
 * Input:
 *      forced - force set to NAPTR table
 *      *naptrEntry - value of NAPTR table entry
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */ 
// if entry->valid==0  del , else set
int32 rtk_l34_naptInboundTable_set(int8 forced, uint32 idx,rtk_l34_naptInbound_entry_t *entry)
{
	int ret;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_naptIn;
	memset(&asic_naptIn,0x0,sizeof(asic_naptIn));

	if(entry->valid){

		asic_naptIn.isValid = (entry->valid>=1)?1:0;
		asic_naptIn.insideLocalIpAddr = entry->intIp;
		asic_naptIn.insideLocalPort = entry->intPort;
		asic_naptIn.isTcp = entry->isTcp;
		asic_naptIn.priValid = entry->priValid;
		asic_naptIn.priority = entry->priId;
		asic_naptIn.offset = ((entry->extPort) & 0x3f);
		asic_naptIn.selEIdx = (entry->remHash)&0x3ff; //very &0x3ff;	
		asic_naptIn.selExtIPIdx 	= ((entry->extPort) & 0x3ff) >> 6;		/* for upstream(outbound), the selExtIPIdx is useless.....*/		
		asic_naptIn.tcpFlag = 0x2;	 /*  bit0 :  0:inbound					1 : outbound	
											 bit1 :  0:the napt flow use 1 entry	1 : the napt flow use 2 entries  
											 bit2 :  0:trap SYN/FIN/RST 		1 : do not trap 
											(enhanced hash1 doesn't support outbound/inbound share one connection) */
		if(entry->isTcp){
			asic_naptIn.ageSec=rg_db.systemGlobal.tcp_long_timeout;	 
		}else{
			asic_naptIn.ageSec=rg_db.systemGlobal.udp_long_timeout;	 
		}

//defalut vlule
//		asic_naptIn.isCollision=1;		//always true
//		asic_naptIn.isCollision2=1; 	//always true
		asic_naptIn.isDedicated=0;
		asic_naptIn.isStatic = 1;
	}

	//set to asic
	ret=rtl8651_setAsicNaptTcpUdpTable(forced, idx, &asic_naptIn);
	return ret;

}

/* Function Name:
 *      rtk_l34_naptOutboundTable_set
 * Description:
 *      Set  napt table entry
 * Input:
 *      forced - force set to napt table
 *      *naptEntry - value of napt table entry
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
 // if entry->valid==0  del , else set
int32 rtk_l34_naptOutboundTable_set(int8 forced, uint32 idx,rtk_l34_naptOutbound_entry_t *entry)
{
	int ret;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_naptOut;
	memset(&asic_naptOut,0x0,sizeof(asic_naptOut));

	if(entry->valid){
		asic_naptOut.isValid = (entry->valid>=1)?1:0;
		asic_naptOut.insideLocalIpAddr = entry->intIp;
		asic_naptOut.insideLocalPort = entry->intPort;
		asic_naptOut.isTcp = entry->isTcp;
		asic_naptOut.priValid = entry->priValid;
		asic_naptOut.priority = entry->priValue;
		asic_naptOut.offset = ((entry->extPort)&0x0000ffff)>>10;
		asic_naptOut.selEIdx = ((entry->extPort)&0x3ff);	
		asic_naptOut.selExtIPIdx		= 0;		/* for upstream(outbound), the selExtIPIdx is useless.....*/		
		asic_naptOut.tcpFlag = 0x3; 	 /*  bit0 :  0:inbound					1 : outbound	
											 bit1 :  0:the napt flow use 1 entry	1 : the napt flow use 2 entries  
											 bit2 :  0:trap SYN/FIN/RST 		1 : do not trap 
											(enhanced hash1 doesn't support outbound/inbound share one connection) */
		if(entry->isTcp){
			asic_naptOut.ageSec=rg_db.systemGlobal.tcp_long_timeout;	
		}else{
			asic_naptOut.ageSec=rg_db.systemGlobal.udp_long_timeout;	
		}
#if defined(CONFIG_RTL_8685S_HWNAT)
		// SMAC/Egress VID = l4 -> Nexthop -> netif(dvid)
		// Dmac = l4 -> nexthop -> l2 Idx
		// extip = L4 -> extip(selExtIPIdx)

		//PerFlow setting
		asic_naptOut.nhIdxValid=1; //alway ture
		asic_naptOut.nhIdx = rg_db.extip[rg_db.naptIn[entry->hashIdx].rtk_naptIn.extIpIdx].rtk_extip.nhIdx ; 
		asic_naptOut.selExtIPIdx=rg_db.naptIn[entry->hashIdx].rtk_naptIn.extIpIdx ; //Per-flow point to extip table
#endif


//defalut vlule
//		asic_naptIn.isCollision=1;		//always true
//		asic_naptIn.isCollision2=1; 	//always true
		asic_naptOut.isDedicated=0;
		asic_naptOut.isStatic = 1;
	}

	//set to asic
	ret=rtl8651_setAsicNaptTcpUdpTable(forced, idx, &asic_naptOut);
	return ret;

}


extern int re865x_send_with_txInfo_and_mask(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num, struct tx_info* ptxInfoMask);
int re8686_send_with_txInfo_and_mask(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num, struct tx_info* ptxInfoMask)
{
	return re865x_send_with_txInfo_and_mask(skb,ptxInfo,ring_num,ptxInfoMask);
}

int32 rtk_switch_phyPortId_get(rtk_switch_port_name_t portName, int32 *pPortId)
{

	if(portName>=RTK_PORT_UTP0 && portName<=RTK_PORT_UTP5){
		(*pPortId)=portName;
	}else if(portName==RTK_PORT_CPU){
		(*pPortId)=6;
	}else{
		return RT_ERR_FAILED;
	}
		return RT_ERR_OK;

}


int32 rtk_l2_portLimitLearningCnt_set(rtk_port_t port, uint32 macCnt)
{
	if(port==RTK_RG_MAC_PORT_CPU){
		/*not support cport learn limitatint */
		return RT_ERR_OK;
	}
#if defined(CONFIG_RTL_8685S_HWNAT)
	return rt8198C_setAsicPortLearningLimit_MaxLimit(port,macCnt);
#else 
	WARNING("not support port limit learning Cnt\n");
	return RT_ERR_OK;
#endif
}

/* action define same as xdsl*/
int32 rtk_l2_portLimitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t learningAction)
{
	if(port==RTK_RG_MAC_PORT_CPU){
		/*not support cport learn limitatint*/
		return RT_ERR_OK;
	}
#if defined(CONFIG_RTL_8685S_HWNAT)
	return rt8198C_setAsicPortLearningLimitAction(port,learningAction);
#else
	WARNING("not support port limit learning Cnt\n");
	return RT_ERR_OK;
#endif
}



/* Function Name:
 *      rtk_init_without_pon
 * Description:
 *      Initialize the driver, without pon related driver
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      INIT must be initialized before using all of APIs in each modules
 */
/*
	rtk_svlan_init		->not support
	rtk_vlan_init		->done
	rtk_stp_init		
	rtk_acl_init		->done
	rtk_qos_init
	rtk_sec_init
	rtk_rate_init
	rtk_classify_init
	rtk_stat_init
	rtk_trunk_init
	rtk_l2_init			->done
	rtk_mirror_init
	rtk_cpu_init
	rtk_rldp_init
	rtk_trap_init
	rtk_gpio_init
	rtk_time_init
*/
extern int32 rtl865x_reinit_acl(void);
extern int32 rtl8651_setAsicNaptAutoAddDelete(int8 autoAdd, int8 autoDelete);
#if defined(CONFIG_RTL_8685S_HWNAT)
extern int32 _rtl819x_enableNaptPerFlow(void);
#endif

int32 rtk_init_without_pon(void)
{
	// XDSL: do most of initial this function
	
	//include init VLAN SW_table  ( rtl865x_init(void) will call rtl865x_initVlanTable to init vlan table)
	rtl865x_reinitVlantable();
	//set  l4 table oubound 4-way/enhanceHash1/PerFlow 
	_set4WayHash(1);
	_rtl8651_enableEnhancedHash1();
#if defined(CONFIG_RTL_8685S_HWNAT)
	_rtl819x_enableNaptPerFlow();
#endif

	//disable l34 auto learn and auto aging
	rtl8651_setAsicNaptAutoAddDelete(0,0);
	//865x enable napt aging function(but we set static) and disable l2 aging out ,in fact 0xbb804400 default all zero 
	rtl8651_setAsicAgingFunction(0,1);
	
	//init l2
	rtk_l2_init();
	// patch for unknow sa l2hash full ,port learning limitation action will be repleaced ,always open unknow sa to cpu
#if defined(CONFIG_RTL_8685S_HWNAT)
	WRITE_MEM32(FFCR,(READ_MEM32(FFCR)&(~( CF_UNKSA_ACT_MSK)))|2<<CF_UNKSA_ACT); /* trap to cpu and disable learning */
#endif

	//set aging time 
	rtl8651_setAsicNaptIcmpTimeout(rg_db.systemGlobal.udp_long_timeout);
	rtl8651_setAsicNaptUdpTimeout(rg_db.systemGlobal.udp_long_timeout);
	rtl8651_setAsicNaptTcpLongTimeout(rg_db.systemGlobal.tcp_long_timeout);
	rtl8651_setAsicNaptTcpMediumTimeout(rg_db.systemGlobal.tcp_long_timeout);
	rtl8651_setAsicNaptTcpFastTimeout(rg_db.systemGlobal.tcp_long_timeout);

	//init L34
	rtk_l34_init();
	//init acl	  RTL8651_ACLTBL_SIZE = RTL8651_ACLHWTBL_SIZE - RTL8651_ACLTBL_RESERV_SIZE)
	_rtk_rg_acl_asic_init();
	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP, NULL));
	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT, NULL));
	rtl865x_setDefACLForNetDecisionMiss(0,MAX_ACL_IGR_ENTRY_SIZE-1,MAX_ACL_IGR_ENTRY_SIZE,MAX_ACL_HW_ENTRY_SIZE-1);

	return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l34_init
 * Description:
 *      Initialize l34 module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Must initialize l34  module before calling any l34  APIs.
 */


/*
	xdsl:Note should init hw table, we reset all entry to zero

    L34_ROUTING_TABLE,
    L34_PPPOE_TABLE,
    L34_NEXTHOP_TABLE,
    L34_NETIF_TABLE,
    L34_INTIP_TABLE,			->ip table
    L34_ARP_TABLE,
    L34_NAPTR_TABLE,
    L34_NAPT_TABLE,
    L34_IPV6_ROUTING_TABLE,	 	
    L34_BINDING_TABLE, 			->not support
    L34_IPV6_NEIGHBOR_TABLE,
    L34_WAN_TYPE_TABLE,			->not support
    L34_FLOW_ROUTING_TABLE,		->not support
   

*/
extern int32 rtl865x_initAsicL3(void);
int32 rtk_l34_init(void)
{
//	rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, RTL8651_ROUTINGTBL_SIZE);
//	rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, RTL8651_PPPOETBL_SIZE);
//	rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_TABLE, RTL8651_NEXTHOPTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_NETINTERFACE_TABLE, RTL865XC_NETIFTBL_SIZE);
//	rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, RTL8651_IPTABLE_SIZE);
//	rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, RTL8651_ARPTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_L4_TCP_UDP_TABLE, RTL8651_TCPUDPTBL_SIZE);
	rtl865x_initAsicL3();


#if defined(CONFIG_RTL_8685S_HWNAT)
	rtl8651_clearSpecifiedAsicTable(TYPE_DS_LITE_TABLE, RTL8198C_DSLITETBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_6RD_TABLE, RTL8198C_IPv6RDTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_IPv6_ROUTING_TABLE, RTL8198C_IPv6ROUTINGTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_IPv6_NEXTHOP_TABLE, RTL8198C_V6NEXTHOPTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_IPV6_NEIGH_TABLE, RTL8198C_V6NEIGHTBL_SIZE);
#endif

	return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize l2 module before calling any l2 APIs.
 */

/*
	dal_apollomp_l2_flushLinkDownPortAddrEnable_set(ENABLED))
	dal_apollomp_l2_table_clear
	for: per port
		dal_apollomp_l2_portLimitLearningCnt_set(port, HAL_L2_LEARN_LIMIT_CNT_MAX())
		dal_apollomp_l2_portLimitLearningCntAction_set(port, LIMIT_LEARN_CNT_ACTION_FORWARD)
		dal_apollomp_l2_portAgingEnable_set(port, ENABLED))
		dal_apollomp_l2_newMacOp_set(port, HARDWARE_LEARNING, ACTION_FORWARD))
		dal_apollomp_l2_newMacOp_set(port, HARDWARE_LEARNING, ACTION_FORWARD))
	dal_apollomp_l2_limitLearningCnt_set(HAL_L2_LEARN_LIMIT_CNT_MAX()))
	dal_apollomp_l2_aging_set(RTK_L2_DEFAULT_AGING_TIME))
	dal_apollomp_l2_lookupMissAction_set(DLF_TYPE_IPMC, ACTION_FORWARD))
	dal_apollomp_l2_lookupMissAction_set(DLF_TYPE_MCAST, ACTION_FORWARD))
	dal_apollomp_l2_lookupMissAction_set(DLF_TYPE_UCAST, ACTION_FORWARD))
	dal_apollomp_l2_lookupMissFloodPortMask_set(DLF_TYPE_IPMC, &all_portmask))
	dal_apollomp_l2_lookupMissFloodPortMask_set(DLF_TYPE_BCAST, &all_portmask))
	dal_apollomp_l2_lookupMissFloodPortMask_set(DLF_TYPE_UCAST, &all_portmask))
	dal_apollomp_l2_ipmcMode_set(LOOKUP_ON_MAC_AND_VID_FID))
	dal_apollomp_l2_ipmcGroupLookupMissHash_set(HASH_DIP_ONLY))
	apollomp_raw_l2_unknReservedMcFlood_set(DISABLED))
	init IPM table
	dal_apollomp_l2_srcPortEgrFilterMask_set(&all_portmask))
*/
int32 rtk_l2_init(void)
{
/*
	XDSL:we clear multicast table andl2 table
*/
	rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);

	//default enable ipv4 multicast and disable multicast aging out
	rtl8651_setAsicMulticastEnable(TRUE);
	rtl865x_setAsicMulticastAging(FALSE);
	rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);
#if defined(CONFIG_RTL_8685S_HWNAT)
	rtl8198C_setAsicV6MulticastEnable(TRUE);
	//FIXME:diasble v6 multicast aging
	rtl8651_clearSpecifiedAsicTable(TYPE_IPv6_MULTICAST_TABLE, RTL8198C_IPv6MCASTTBL_SIZE);
#endif
	//FIXME:IPM MISS SET /L2 Tale MISS SET
	return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_lookupMissAction_set
 * Description:
 *      Set forwarding action when destination address lookup miss.
 * Input:
 *      type   - type of lookup miss
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_INPUT      - invalid type of lookup miss
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 *
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU (only for DLF_TYPE_UCAST) 
 *      - ACTION_FLOOD_IN_ALL_PORT  (only for DLF_TYPE_MCAST)
 *      - ACTION_FLOOD_IN_ROUTER_PORTS (only for DLF_TYPE_IPMC)
 */
/* nuknow da action ,only 8685s support  0b00: normal flooding     0b01: drop packet    0b10: trap to CPU*/
int32 rtk_l2_lookupMissAction_set(rtk_l2_lookupMissType_t type, rtk_action_t action)
{
	/* apollo init set unknow da to cpu*/
#if defined(CONFIG_RTL_8685S_HWNAT)
	switch(action){

		case ACTION_TRAP2CPU:
			WRITE_MEM32(FFCR,(READ_MEM32(FFCR)&(~(CF_UNKUC_ACT_MSK))) | (2<<CF_UNKUC_ACT));
			break;
		default:
			WARNING(" FIXME action not support action=%d\n",action);
			
	}
#else
	WARNING(" xdsl can mapping this function FIXME plz\n");
#endif

	return (RT_ERR_OK);
}


int32 _rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t enable)
{
	if(port <0 || port >6)
		return RT_ERR_PORT_ID;
	if(port == 6){
		WARNING("not support cport vlan filtering");
		return RT_ERR_OK;
	}
	WRITE_MEM32( VCR0, (READ_MEM32( VCR0 )&(~(1<<port))) | (1<<port));
	return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_addr_delAll
 * Description:
 *      Delete all L2 unicast address entry.
 * Input:
 *      includeStatic - include static mac or not?
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 rtk_l2_addr_delAll(uint32 includeStatic)
{
	rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_l34_lookupMode_set
 * Description:
 *      configure l34 lookup mode selection
 * Input:
 * 	  lookupMode	- mode of l34 lookup method
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_lookupMode_set(rtk_l34_lookupMode_t lookupMode)
{
	/*
		we support Macbase , vlan base and port base .parameter match with apollo 
		xdsl default support vlan base ,but apollo default mac base . FIXME:check again
	*/
	return rtl865xC_setNetDecisionPolicy(lookupMode);
}

/* Function Name:
 *      rtk_l34_ipv6NeighborTable_set
 * Description:
 *      Set neighbor table
 * Input:
 *      idx - index of neighbor table
 *      *ipv6NeighborEntry - point of neighbor data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_ipv6NeighborTable_set(uint32 idx,rtk_ipv6Neighbor_entry_t *ipv6NeighborEntry)
{
	int32 ret=RT_ERR_FAILED;
#if defined(CONFIG_RTL_8685S_HWNAT)
	rtl8198C_tblAsicDrv_v6NeighParam_t arpv6;

	if(ipv6NeighborEntry->valid==0)
	   rtl8198C_delAsicArp6(idx);
	arpv6.age =31;
	arpv6.hostID = ipv6NeighborEntry->ipv6Ifid ;
	arpv6.nextHopColumn = (ipv6NeighborEntry->l2Idx) &0x3;
	arpv6.nextHopRow = (ipv6NeighborEntry->l2Idx)>>2 ;
	arpv6.subnetIdx =ipv6NeighborEntry->ipv6RouteIdx;
	ret = rtl8198C_setAsicArp6_idx(idx,&arpv6);
#endif

	return ret;
}


extern  rtl8651_tblAsic_ethernet_t	rtl8651AsicEthernetTable[9];//  extern from asicDriver/rtl865x_asicL2.c
int32 xdslRomeDriver_LinkChange_Process(void)
{
	uint32 i, status;
	intrBcasterMsg_t	MsgData;

	MsgData.intrType=MSG_TYPE_LINK_CHANGE;
	/* Check each port. */
	for ( i = 0; i < RTL8651_MAC_NUMBER; i++ )
	{
		#ifdef CONFIG_PTMWAN
		if (RTL_PTMWANPORT_MASK == (1 << i)) // skip PTM port
			continue;
		#endif 
		/* Read Port Status Register to know the port is link-up or link-down. */
		status = READ_MEM32( PSRP0 + i * 4 );
		if ( ( status & PortStatusLinkUp ) == FALSE )
		{
			/* Link is down. */
			if(rtl8651AsicEthernetTable[i].linkUp)
			{ //link change to link down
				DEBUG("port[%d] link change to link down ",i);
				MsgData.intrSubType=INTR_STATUS_LINKDOWN;
				MsgData.intrBitMask=i;
				_rtk_rg_switchLinkChangeHandler(&MsgData);
			}

			rtl8651_setAsicEthernetLinkStatus( i, FALSE );
		}
		else
		{
			if(!(rtl8651AsicEthernetTable[i].linkUp))
			{ //link change to link up
				DEBUG("port[%d] link change to link up ",i);
				MsgData.intrSubType=INTR_STATUS_LINKUP;
				MsgData.intrBitMask=i;
				_rtk_rg_switchLinkChangeHandler(&MsgData);
			}
			/* Link is up. */
			rtl8651_setAsicEthernetLinkStatus( i, TRUE );
		}
	}

	return SUCCESS;

}

/* Function Name:
 *      rtk_port_link_get
 * Description:
 *      Get the link status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pLinkStatus - pointer to the link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The link status of the port is as following:
 *      - LINKDOWN
 *      - LINKUP
 */
int32 rtk_port_link_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus)
{
	uint32  status;
	/* Read Port Status Register to know the port is link-up or link-down. */
	status = READ_MEM32( PSRP0 + port * 4 );
	if ( ( status & PortStatusLinkUp ) == FALSE )
		*pLinkStatus=PORT_LINKDOWN;
	else
		*pLinkStatus=PORT_LINKUP;
	return RT_ERR_OK;

}

/* Function Name:
 *      rtk_port_speedDuplex_get
 * Description:
 *      Get the negotiated port speed and duplex status of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pSpeed  - pointer to the port speed
 *      pDuplex - pointer to the port duplex
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_PORT_LINKDOWN - link down port status
 * Note:
 *      (1) The speed type of the port is as following:
 *          - PORT_SPEED_10M
 *          - PORT_SPEED_100M
 *          - PORT_SPEED_1000M
 *
 *      (2) The duplex mode of the port is as following:
 *          - HALF_DUPLEX
 *          - FULL_DUPLEX
 */
int32 rtk_port_speedDuplex_get(rtk_port_t port,rtk_port_speed_t *pSpeed,rtk_port_duplex_t *pDuplex)
{
	uint32  status;
	/* Read Port Status Register to know the port is link-up or link-down. */
	status = READ_MEM32( PSRP0 + port * 4 );

#if 0
	if(( status & PortStatusLinkUp ) == FALSE)
		return RT_ERR_PORT_LINKDOWN;
#endif
	
	if ( ( status & PortStatusDuplex ) == FALSE )
		*pDuplex=PORT_HALF_DUPLEX;
	else
		*pDuplex=PORT_FULL_DUPLEX;

	if( ((status&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET)==PortStatusLinkSpeed10M )
		*pSpeed=PORT_SPEED_10M;
	else if( ((status&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET)==PortStatusLinkSpeed100M )
		*pSpeed=PORT_SPEED_100M;
	else if( ((status&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET)==PortStatusLinkSpeed1000M )
		*pSpeed=PORT_SPEED_1000M;
	else
		return RT_ERR_FAILED;
	
	return RT_ERR_OK;

}


#define FIXEME_FUNCITON 1//for sourceInsight 
#if FIXEME_FUNCITON

int32 rtk_l2_ipmcMode_get(rtk_l2_ipmcMode_t *pMode)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_classify_portRange_set
 * Description:
 *      Set Port Range check
 * Input:
 *      pRangeEntry - L4 Port Range entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      UpperPort must be larger or equal than lowerPort.
 *      This function is not supported in Test chip.
 */
int32 rtk_classify_portRange_set(rtk_classify_rangeCheck_l4Port_t *pRangeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}



/* Function Name:
 *      rtk_classify_ipRange_set
 * Description:
 *      Set IP Range check
 * Input:
 *      pRangeEntry - IP Range entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      UpperIp must be larger or equal than lowerIp.
 *      This function is not supported in Test chip.
 */
int32 rtk_classify_ipRange_set(rtk_classify_rangeCheck_ip_t *pRangeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_classify_cfPri2Dscp_set
 * Description:
 *      Set CF priority to DSCP value mapping
 * Input:
 *      pri    - priority value
 *      dscp   - DSCP value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      This function is not supported in Test chip.
 */
int32 rtk_classify_cfPri2Dscp_set(rtk_pri_t pri, rtk_dscp_t dscp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}





/* Module Name    : QoS           */
/* Sub-module Name: Egress remark */
/* Function Name:
 *      rtk_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1p remark status for a port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - status of 802.1p remark
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      The status of 802.1p remark:
 *      - DISABLED
 *      - ENABLED
 */
int32 rtk_qos_1pRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{

	pEnable =DISABLED;
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/*
	igmp and mld packet hw action default to cpu
*/
int32 rtk_trap_portIgmpMldCtrlPktAction_set(rtk_port_t port, rtk_trap_igmpMld_type_t igmpMldType, rtk_action_t action)
{
	/*FIXME: empty function */
	FIXME(" empty function igmp and mld packet hw action init default to cpu\n");
	return (RT_ERR_OK);

}


int32 rtk_intr_linkdownStatus_clear(void)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");

	return (RT_ERR_OK);

}



int32 rtk_vlan_vlanFunctionEnable_set(rtk_enable_t enable)
{
	/* set vlan egress ingress filter enable ,xdsl only support ingress vlan filtering */
	return (RT_ERR_OK);

}

/*
	XDSL: if need ,implement this function 
	apollo default RTK_L34_GLOBALSTATE_SET(L34_GLOBAL_TTLMINUS_STATE,DISABLED).

*/
int32 rtk_l34_globalState_set(rtk_l34_globalStateType_t stateType,rtk_enable_t state)
{
	switch(stateType){
		case L34_GLOBAL_L3NAT_STATE:
		break;
		case L34_GLOBAL_L4NAT_STATE:
		break;
		case L34_GLOBAL_L3CHKSERRALLOW_STATE:
		break;
		case L34_GLOBAL_L4CHKSERRALLOW_STATE:
		break;
		case L34_GLOBAL_TTLMINUS_STATE://ipv4 minus
		break;
		case L34_GLOBAL_FRAG2CPU_STATE:
		break;
		default :
			WARNING("Other type XDSL not support stateType=%d\n",stateType);
			
		break;
	}

	return (RT_ERR_OK);
}

/*xdsl only support 2 user define port and portocol base vlan group but apollo support 4 group*/
int32 rtk_vlan_protoGroup_set(uint32 protoGroupIdx,rtk_vlan_protoGroup_t *pProtoGroup)
{
	/*not support protocol group vlan */
	FIXME(" not support protocol group vlan \n");
	return (RT_ERR_OK);

}

int32 rtk_l34_bindingAction_get(rtk_l34_bindType_t bindType, rtk_l34_bindAct_t *bindAction)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

} 


int32 rtk_l34_globalState_get(rtk_l34_globalStateType_t stateType,rtk_enable_t *pState)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_l2_ipMcastAddr_add
 * Description:
 *      Add IP multicast entry to ASIC.
 * Input:
 *      pIpmcastAddr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_IPV4_ADDRESS  - Invalid IPv4 address
 *      RT_ERR_VLAN_VID      - invalid vlan id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_INPUT         - invalid input parameter
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32 rtk_l2_ipMcastAddr_add(rtk_l2_ipMcastAddr_t *pIpmcastAddr)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_l2_ipMcastAddr_del
 * Description:
 *      Delete a L2 ip multicast address entry from the specified device.
 * Input:
 *      pIpmcastAddr  - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_L2_HASH_KEY    - invalid L2 Hash key
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 * Note:
 *      (1) In vlan unaware mode (SVL), the vid will be ignore, suggest to
 *          input vid=0 in vlan unaware mode.
 *      (2) In vlan aware mode (IVL), the vid will be care.
 */
int32 rtk_l2_ipMcastAddr_del(rtk_l2_ipMcastAddr_t *pIpmcastAddr)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_l2_mcastAddr_add
 * Description:
 *      Add L2 multicast entry to ASIC.
 * Input:
 *      pMcastAddr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
int32 rtk_l2_mcastAddr_add(rtk_l2_mcastAddr_t *pMcastAddr)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_l2_mcastAddr_del
 * Description:
 *      Delete a L2 multicast address entry.
 * Input:
 *      pMcastAddr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_L2_HASH_KEY    - invalid L2 Hash key
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 * Note:
 *      None
 */
int32 rtk_l2_mcastAddr_del(rtk_l2_mcastAddr_t *pMcastAddr)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}



/* Function Name:
 *      rtk_vlan_portProtoVlan_set
 * Description:
 *      Set vlan of specificed protocol group on specified port.
 * Input:
 *      port           - port id
 *      protoGroupIdx  - protocol group index
 *      pVlanCfg       - vlan configuration of protocol group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - protocol group index is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_vlan_portProtoVlan_set(rtk_port_t port,uint32 protoGroupIdx,rtk_vlan_protoVlanCfg_t *pVlanCfg)
{
	int idx,ret,type,proto;
	printk("Set Vlan:%d valid:%d for port:%d protocol:%x - RG_IPV4_GROUPID:%d RG_ARP_GROUPID:%d RG_IPV6_GROUPID:%d\n",pVlanCfg->vid,pVlanCfg->valid,port,protoGroupIdx,RG_IPV4_GROUPID,RG_ARP_GROUPID,RG_IPV6_GROUPID);
	switch(protoGroupIdx)
	{
		case RG_IPV4_GROUPID:
			idx = RTL8651_PBV_RULE_USR1;
			type = 0;
			proto = 0x0800;
			break;
		case RG_ARP_GROUPID:
			idx = RTL8651_PBV_RULE_USR2;
			type = 0;
			proto = 0x0806;
			break;
		case RG_IPV6_GROUPID:
			idx = RTL8651_PBV_RULE_USR1;
			type = 0;
			proto = 0x86dd;
			break;
		default:
			FIXME(" not support yet  \n");
	}
	/* type - 00:ethernetII, 01:RFC-1042, 10: LLC-Other, 11:reserved */
	ret = rtl8651_defineProtocolBasedVLAN(idx,type,proto);
	if(ret)
		return (RT_ERR_FAILED);
	ret = rtl8651_setProtocolBasedVLAN(idx,port,pVlanCfg->valid,pVlanCfg->vid);
	if(ret)
		return (RT_ERR_FAILED);
	return (RT_ERR_OK);
}



/* Function Name:
 *      rtk_l34_ipv6RoutingTable_set
 * Description:
 *      Set a IPv6 routing entry by idx.
 * Input:
 *      idx     - index of ipv6 routing entry
 *      *ipv6RoutEntry - point of ipv6 routing table
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_ipv6RoutingTable_set(uint32 idx, rtk_ipv6Routing_entry_t *ipv6RoutEntry)
{
	/*FIXME: empty function */
	FIXME(" not support ipv6  \n");
	return (RT_ERR_OK);

}



/* Function Name:
 *      rtk_l2_ipmcGroup_add
 * Description:
 *      Add an entry to IPMC Group Table.
 * Input:
 *      gip         - Group IP
 *      pPortmask   - Group member port mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_ENTRY_FULL   - entry is full
 * Note:
 *      None
 */
int32 rtk_l2_ipmcGroup_add(ipaddr_t gip, rtk_portmask_t *pPortmask)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_l2_ipmcGroup_del
 * Description:
 *      Delete an entry from IPMC Group Table.
 * Input:
 *      gip         - Group IP
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
int32 rtk_l2_ipmcGroup_del(ipaddr_t gip)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_qos_1pPriRemapGroup_set
 * Description:
 *      Set remapped internal priority of dot1p priority on specified dot1p priority remapping group.
 * Input:
 *      grpIdx   - index of dot1p remapping group
 *      dot1pPri - dot1p priority
 *      intPri   - internal priority
 *      dp        - drop precedence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32 rtk_qos_1pPriRemapGroup_set(uint32 grpIdx,rtk_pri_t dot1pPri,rtk_pri_t intPri,uint32 dp)
{
	/*FIXME: empty function */
	FIXME(" Qos setting FIXME latter \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_qos_dscpPriRemapGroup_set
 * Description:
 *      Set remapped internal priority of DSCP on specified DSCP remapping group.
 * Input:
 *      grpIdx  - index of dscp remapping group
 *      dscp    - DSCP
 *      intPri - internal priority
 *      dp      - drop precedence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE   - invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Apollo only support group 0
 */
int32 rtk_qos_dscpPriRemapGroup_set(uint32 grpIdx,uint32 dscp,rtk_pri_t intPri,uint32 dp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_qos_portPri_set
 * Description:
 *      Get internal priority of one port.
 * Input:
 *      port     - port id
 *      intPri  - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                 the highest prioirty)
 * Output:
 *    None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    None
 */
int32 rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t intPri)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_qos_priSelGroup_set
 * Description:
 *      Set weight of each priority assignment on specified priority selection group.
 * Input:
 *      grpIdx          - index of priority selection group
 *      pWeightOfPriSel - weight of each priority assignment
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Apollo only support group 0 
 */
int32 rtk_qos_priSelGroup_set(uint32 grpIdx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{
	if(grpIdx!=0)
	{
		printk("xDSL do not support multiple priority selection group!\n");
		return (RT_ERR_OK);
	}
	rtl8651_setAsicPriorityDecision(pWeightOfPriSel->weight_of_saBaed,pWeightOfPriSel->weight_of_dot1q,pWeightOfPriSel->weight_of_dscp,pWeightOfPriSel->weight_of_acl,pWeightOfPriSel->weight_of_l4Based);
	return (RT_ERR_OK);
}




/* Function Name:
 *      rtk_acl_template_get
 * Description:
 *      Get template of ingress ACL.
 * Input:
 *      template - Ingress ACL template
 * Output:
 *      template - Ingress ACL template
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_INPUT           - Invalid input parameters.
 * Note:
 *      This function get ACL template.
 */
int32  rtk_acl_template_get(rtk_acl_template_t *aclTemplate)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_acl_igrRuleEntry_del
 * Description:
 *      Delete an ACL configuration from ASIC
 * Input:
 *      pAclrule   - ACL ingress filter rule configuration.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_ENTRY_INDEX 						- Invalid entryIdx .
 * Note:
 *      None
 */
int32 rtk_acl_igrRuleEntry_del(uint32 index)
{
	rtl865x_AclRule_t rule;

	DEBUG("Enter %s (index:%d)\n",__func__,index);

	if(index<0 || index>=MAX_ACL_IGR_ENTRY_SIZE)
	{
		DEBUG("Leave %s @ %d\n",__func__,__LINE__);
		return RT_ERR_FAILED;
	}

	/* create a dummy acl rule (this acl rule will be skipped when lookup)*/
	memset(&rule, 0,sizeof(rtl865x_AclRule_t));		

	if(_rtl865x_setAclToAsic(index, &rule)!=SUCCESS)
	{
		DEBUG("Del ACL Failed! Leave %s @ %d\n",__func__,__LINE__);
		return RT_ERR_FAILED;
	}
	
	DEBUG("Del ACL Success!\n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_classify_cfgEntry_del
 * Description:
 *      Delete an classification configuration from ASIC
 * Input:
 *      entryIdx    - index of classification entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_ENTRY_INDEX 		- Invalid classification index .
 * Note:
 *      None.
 */
int32 rtk_classify_cfgEntry_del(uint32 entryIdx)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_acl_igrRuleEntry_add
 * Description:
 *      Add an ACL configuration to ASIC
 * Input:
 *      pAclRule   - ACL ingress filter rule configuration.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pAclrule point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 *      RT_ERR_ENTRY_INDEX 						- Invalid entryIdx .
 * Note:
 *      None
 */
int32 rtk_acl_igrRuleEntry_add(rtk_acl_ingress_entry_t *pAclRule)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_classify_cfgEntry_add
 * Description:
 *      Add an classification entry to ASIC
 * Input:
 *      entryIdx       - index of classification entry.
 *      pClassifyCfg     - The classification configuration that this function will add comparison rule
 *      pClassifyAct     - Action(s) of classification configuration.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pClassifyCfg point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None.
 */
int32 rtk_classify_cfgEntry_add(rtk_classify_cfg_t *pClassifyCfg)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_acl_template_set
 * Description:
 *      Set template of ingress ACL.
 * Input:
 *      template - Ingress ACL template
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_INPUT           - Invalid input parameters.
 * Note:
 *      This function set ACL template.
 */
int32 rtk_acl_template_set(rtk_acl_template_t *aclTemplate)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_acl_igrRuleField_add
 * Description:
 *      Add comparison rule to an ACL configuration
 * Input:
 *      pAclEntry     - The ACL configuration that this function will add comparison rule
 *      pAclField   - The comparison rule that will be added.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_NULL_POINTER    	- Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 * Note:
 *      This function add a comparison rule (*pAclField) to an ACL configuration (*pAclEntry).
 *      Pointer pFilter_cfg points to an ACL configuration structure, this structure keeps multiple ACL
 *      comparison rules by means of linked list. Pointer pAclField will be added to linked
 *      list keeped by structure that pAclEntry points to.
 *      caller should not free (*pAclField) before rtk_acl_igrRuleEntry_add is called
 */
int32 rtk_acl_igrRuleField_add(rtk_acl_ingress_entry_t *pAclRule, rtk_acl_field_t *pAclField)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}


/* Function Name:
 *      rtk_acl_igrRuleEntry_get
 * Description:
 *      Get an ACL entry from ASIC
 * Input:
 *      None.
 * Output:
 *      pAclRule     - The ACL configuration that this function will add comparison rule
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pAclRule point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      use this API to get rule entry the field data will return in raw format
 *      raw data is return in pAclRule->field.readField
 */
int32 rtk_acl_igrRuleEntry_get(rtk_acl_ingress_entry_t *pAclRule)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_classify_cfgEntry_get
 * Description:
 *      Gdd an classification entry from ASIC
 * Input:
 *      None.
 * Output:
 *      pClassifyCfg     - The classification configuration that this function will add comparison rule
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pClassifyCfg point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None.
 */
int32 rtk_classify_cfgEntry_get(rtk_classify_cfg_t *pClassifyCfg)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_acl_igrRuleMode_set
 * Description:
 *      Set ingress ACL rule mode
 * Input:
 *      mode - ingress ACL rule mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_INPUT           - Input error
 * Note:
 *          - ACL_IGR_RULE_MODE_0, 64  rules, the size each rule is 16x8 bits 
 *          - ACL_IGR_RULE_MODE_1, 128 rules, 
 *          -               the size each rule is 16x4 bits(entry 0~63)
 *          -               the size each rule is 16x3 bits(entry 64~127) 
 */
int32 rtk_acl_igrRuleMode_set(rtk_acl_igr_rule_mode_t mode)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_acl_igrState_set
 * Description:
 *      Set state of ingress ACL.
 * Input:
 *      port    - Port id.
 *      state   - Ingress ACL state.
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
int32 rtk_acl_igrState_set(rtk_port_t port, rtk_enable_t state)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_acl_igrUnmatchAction_set
 * Description:
 *      Apply action to packets when no ACL configuration match
 * Input:
 *      port    - Port id.
 *      action - Action.
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
int32 rtk_acl_igrUnmatchAction_set(rtk_port_t port, rtk_filter_unmatch_action_type_t action)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_acl_fieldSelect_set
 * Description:
 *      Set user defined field selectors in HSB
 * Input:
 *      pFieldEntry 	- pointer of field selector entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 * Note:
 *      System support 16 user defined field selctors.
 * 		Each selector can be enabled or disable.
 *      User can defined retrieving 16-bits in many predefiend
 * 		standard l2/l3/l4 payload.
 */
int32 rtk_acl_fieldSelect_set(rtk_acl_field_entry_t *pFieldEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_acl_portRange_set
 * Description:
 *      Set Port Range check
 * Input:
 *      pRangeEntry - L4 Port Range entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      upper Port must be larger or equal than lowerPort.
 */
int32 rtk_acl_portRange_set(rtk_acl_rangeCheck_l4Port_t *pRangeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_switch_version_get
 * Description:
 *      Get chip version
 * Input:
 *      pChipId    - chip id
 *      pRev       - revision id
 *      pSubtype   - sub type
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK 
 *      RT_ERR_FAILED  
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer 
 */
int32 rtk_switch_version_get(uint32 *pChipId, uint32 *pRev, uint32 *pSubtype)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}




/* Function Name:
 *      rtk_rate_shareMeterBucket_set
 * Description:
 *      Set meter Bucket Size
 * Input:
 *      index        - shared meter index
 *      bucketSize   - Bucket Size
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can set shared meter bucket size.
 */
int32 rtk_rate_shareMeterBucket_set(uint32 index, uint32 bucketSize)
{
	/*FIXME: empty function */
	FIXME(" empty function plz check\n");
	return (RT_ERR_OK);
}




/* Function Name:
 *      rtk_acl_ipRange_set
 * Description:
 *      Set IP Range check
 * Input:
 *      pRangeEntry - IP Range entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      upper Ip must be larger or equal than lowerIp.
 */
int32 rtk_acl_ipRange_set(rtk_acl_rangeCheck_ip_t *pRangeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_port_isolationCtagPktConfig_set
 * Description:
 *      Isolation configuration selection for ingress Ctag packets
 * Input:
 *      mode    - Isolation configuration selection
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32 rtk_port_isolationCtagPktConfig_set(rtk_port_isoConfig_t mode)
{
	/* not support port isolation config mode*/
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_port_isolationL34PktConfig_set
 * Description:
 *      Isolation configuration selection for ingress Ctag packets
 * Input:
 *      mode    - Isolation configuration selection
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32 rtk_port_isolationL34PktConfig_set(rtk_port_isoConfig_t mode)
{
	/* not support port isolation config mode*/
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_port_isolationIpmcLeaky_set
 * Description:
 *      Set the ip multicast leaky state of the port isolation
 * Input:
 *      port      - port id
 *      enable    - status of port isolation leaky for ip multicast packets
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      none
 */
int32 rtk_port_isolationIpmcLeaky_set(rtk_port_t port, rtk_enable_t enable)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_port_isolationEntry_set
 * Description:
 *      Set Port isolation portmask
 * Input:
 *      mode            - Configuration 0 or 1
 *      port            - Ingress port
 *      pPortmask       - Isolation portmask for specified ingress port.
 *      pExtPortmask    - Isolation extension portmask for specified ingress port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      pExtPortmask is the extension egress portmask toward CPU port.
 *      If users specify an empty extension portmask and CPU port is set in pPortmask,
 *      the packets will be restricted to be forwarded to CPU.
 *      Likewise, If users specify an non-empty extension portmask and CPU port is not set in pPortmask,
 *      the packets will be restricted to be forwarded to CPU. too.
 */
int32 rtk_port_isolationEntry_set(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_port_isolationEntryExt_set
 * Description:
 *      Set Port isolation portmask
 * Input:
 *      mode            - Configuration 0 or 1
 *      port            - Ingress port
 *      pPortmask       - Isolation portmask for specified ingress port.
 *      pExtPortmask    - Isolation extension portmask for specified ingress port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      pExtPortmask is the extension egress portmask toward CPU port.
 *      If users specify an empty extension portmask and CPU port is set in pPortmask,
 *      the packets will be restricted to be forwarded to CPU.
 *      Likewise, If users specify an non-empty extension portmask and CPU port is not set in pPortmask,
 *      the packets will be restricted to be forwarded to CPU. too.
 */
int32 rtk_port_isolationEntryExt_set(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}







/* Function Name:
 *      rtk_classify_cfSel_set
 * Description:
 *      Set CF port selection, only pon port and RGMII port can be set
 * Input:
 *      port    - port id, only pon port and RGMII port can be set.
 *      cfSel   - CF port selection.
 * Output:
 *      pRangeEntry - IP Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      Only accept pon port and RGMII port.
 *      This function is not supported in Test chip.
 */
int32 rtk_classify_cfSel_set(rtk_port_t port, rtk_classify_cf_sel_t cfSel)
{
	/*XDSL not support cf*/
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_l34_wanRoutMode_set
 * Description:
 *      set wan route mode 
 * Input:
 * 	  wanRouteMode	- mode of wan routed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_wanRoutMode_set(rtk_l34_wanRouteMode_t wanRouteMode)
{
	/* what is waRoutMode !?*/
	FIXME("not support wanRoutMode\n");
	return (RT_ERR_OK);
}




/* Function Name:
 *      rtk_l34_hsabMode_set
 * Description:
 *      Set L34 hsab mode
 * Input:
 *      hsabMode - L34 hsab 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_hsabMode_set(rtk_l34_hsba_mode_t hsabMode)
{
	WARNING(" not support hsa Mode \n");
	return (RT_ERR_OK);
}



/* Function Name:
 *      rtk_port_macForceAbility_get
 * Description:
 *      Set MAC forece ability
 * Input:
 *      port - the ports for get ability
 *      macAbility - mac ability value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_port_macForceAbility_get(rtk_port_t port,rtk_port_macAbility_t *pMacAbility)
{
	if(port<0 || port >RTK_RG_MAC_PORT_CPU)
		return RT_ERR_FAILED;
	if(port==RTK_RG_MAC_PORT_CPU){
		WARNING("not support set mac force ability to Cport\n");
		return RT_ERR_OK; 
	}
	FIXME(" empty function rtk_port_macForceAbility_get FIXME plz\n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_port_macForceAbility_set
 * Description:
 *      Set MAC forece ability
 * Input:
 *      port - the ports for set ability
 *      macAbility - mac ability value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_port_macForceAbility_set(rtk_port_t port,rtk_port_macAbility_t macAbility)
{
	if(port<0 || port >RTK_RG_MAC_PORT_CPU)
		return RT_ERR_FAILED;
	if(port==RTK_RG_MAC_PORT_CPU){
		WARNING("not support set mac force ability to Cport\n");
		return RT_ERR_OK; 
	}
	FIXME(" empty function rtk_port_macForceAbility_get FIXME plz\n");
	return (RT_ERR_OK);;
}


/* Function Name:
 *      rtk_port_macForceAbilityState_set
 * Description:
 *      Set MAC forece ability state
 * Input:
 *      port - the ports for set ability
 *      state - mac ability state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_port_macForceAbilityState_set(rtk_port_t port,rtk_enable_t state)
{
	if(port<0 || port >RTK_RG_MAC_PORT_CPU)
		return RT_ERR_FAILED;
	if(port==RTK_RG_MAC_PORT_CPU){
		WARNING("not support set mac force ability to Cport\n");
		return RT_ERR_OK; 
	}
	FIXME(" empty function rtk_port_macForceAbility_get FIXME plz\n");
	return (RT_ERR_OK);

}




/* Function Name:
 *      rtk_l2_lookupMissFloodPortMask_set
 * Description:
 *      Set flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      type            - type of lookup miss
 *      pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      DLF_TYPE_IPMC, DLF_TYPE_IP6MC & DLF_TYPE_MCAST shares the same configuration.
 */
int32 rtk_l2_lookupMissFloodPortMask_set(rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
	//unknow multicat to cpu
	//unknow broadcast to cpu
	WARNING(" unknow multicat to cpu , unknow broadcast to cpu not implement \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_l2_ipmcMode_set
 * Description:
 *      Set lookup mode of layer2 ip multicast switching.
 * Input:
 *      mode - lookup mode of layer2 ip multicast switching
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_MAC_AND_VID_FID
 *      - LOOKUP_ON_DPI_AND_VID
 */
int32 rtk_l2_ipmcMode_set(rtk_l2_ipmcMode_t mode)
{
	WARNING(" not support multicast mode setting\n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_l2_ipmcGroupLookupMissHash_set
 * Description:
 *      Set Hash operation of IPv4 multicast packet which is not in IPMC Group Table.
 * Input:
 *      ipmcHash - Hash operation of IPv4 multicast packet which is not in IPMC Group Table
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32 rtk_l2_ipmcGroupLookupMissHash_set(rtk_l2_ipmcHashOp_t ipmcHash)
{
	WARNING("not support rtk_l2_ipmcGroupLookupMissHash_set\n");
	return (RT_ERR_OK);
}



/* Function Name:
 *      rtk_l2_srcPortEgrFilterMask_set
 * Description:
 *      Set source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      pFilter_portmask - source port egress filtering configuration when packet egress.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected
 */
int32 rtk_l2_srcPortEgrFilterMask_set(rtk_portmask_t *pFilter_portmask)
{
	/*FIXME: empty function */
	WARNING("not support  rtk_l2_srcPortEgrFilterMask_set \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_l2_extPortEgrFilterMask_set
 * Description:
 *      Set extension port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      pExt_portmask - extension port egress filtering configuration when packet egress.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected
 */
int32 rtk_l2_extPortEgrFilterMask_set(rtk_portmask_t *pExt_portmask)
{
	/*FIXME: empty function */
	WARNING(" not support rtk_l2_extPortEgrFilterMask_set\n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_fwd2CpuPriRemap_set
 * Description:
 *      Set remapped internal priority of DSCP on specified DSCP remapping group.
 * Input:
 *      intPri  -  internal priority
 *      rempPri - remapping priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 */
int32 rtk_qos_fwd2CpuPriRemap_set(rtk_pri_t intPri,rtk_pri_t rempPri)
{
	FIXME(" Qos setting FIXME latter \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_portPriMap_set
 * Description:
 *      Set the value of internal priority to QID mapping table on specified port.
 * Input:
 *      port  - port id
 *      index - index to priority to queue table
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
int32 rtk_qos_portPriMap_set(rtk_port_t port, uint32 group)
{
	/*FIXME: empty function */
	FIXME(" Qos setting FIXME latter \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_classify_unmatchAction_set
 * Description:
 *      Apply action to packets when no classfication configuration match
 * Input:
 *      None
 * Output:
 *      action - unmatch action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no classfication configruation matches.
 */
int32 rtk_classify_unmatchAction_set(rtk_classify_unmatch_action_t action)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_l34_nexthopTable_get
 * Description:
 *      Get Next-Hop table entry
 * Input:
 *      idx - index of Next-Hop table
 * Output:
 *      *nextHopEntry -point of Next-Hop entry result
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_nexthopTable_get(uint32 idx, rtk_l34_nexthop_entry_t *nextHopEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_acl_portRange_get
 * Description:
 *      Set Port Range check
 * Input:
 *      None
 * Output:
 *      pRangeEntry - L4 Port Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      None.
 */
int32 rtk_acl_portRange_get(rtk_acl_rangeCheck_l4Port_t *pRangeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_acl_ipRange_get
 * Description:
 *      Set IP Range check
 * Input:
 *      None.
 * Output:
 *      pRangeEntry - IP Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      None.
 */
int32 rtk_acl_ipRange_get(rtk_acl_rangeCheck_ip_t *pRangeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_classify_portRange_get
 * Description:
 *      Set Port Range check
 * Input:
 *      None
 * Output:
 *      pRangeEntry - L4 Port Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      This function is not supported in Test chip.
 */
int32 rtk_classify_portRange_get(rtk_classify_rangeCheck_l4Port_t *pRangeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_classify_ipRange_get
 * Description:
 *      Set IP Range check
 * Input:
 *      None.
 * Output:
 *      pRangeEntry - IP Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      This function is not supported in Test chip.
 */
int32 rtk_classify_ipRange_get(rtk_classify_rangeCheck_ip_t *pRangeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_classify_cfPri2Dscp_get
 * Description:
 *      Get CF priority to DSCP value mapping
 * Input:
 *      pri    - priority value
 * Output:
 *      pDscp  - pointer of DSCP value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_NULL_POINTER    - Pointer pClassifyCfg point to NULL.
 * Note:
 *      This function is not supported in Test chip.
 */
int32 rtk_classify_cfPri2Dscp_get(rtk_pri_t pri, rtk_dscp_t *pDscp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_classify_field_add
 * Description:
 *      Add comparison field to an classfication configuration
 * Input:
 *      pClassifyEntry     - The classfication configuration that this function will add comparison rule
 *      pClassifyField     - The comparison rule that will be added.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_NULL_POINTER    	- Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 * Note:
 *      This function add a comparison rule (*pClassifyField) to an ACL configuration (*pClassifyEntry).
 *      Pointer pFilter_cfg points to an ACL configuration structure, this structure keeps multiple ACL
 *      comparison rules by means of linked list. Pointer pAclField will be added to linked
 *      list keeped by structure that pAclEntry points to.
 *      - caller should not free (*pClassifyField) before rtk_classify_cfgEntry_add is called
 */
int32 rtk_classify_field_add(rtk_classify_cfg_t *pClassifyEntry, rtk_classify_field_t *pClassifyField)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_vlan_extPortPvid_get
 * Description:
 *      Get extension port default vlan id.
 * Input:
 *      extPort - Extension port id
 * Output:
 *      pPvid - pointer buffer of port default vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Configuration on EXT port 0 will be get from physical port 6.
 */
int32 rtk_vlan_extPortPvid_get(uint32 extPort, uint32 *pPvid)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get port default vlan id.
 * Input:
 *      port  - port id
 * Output:
 *      pPvid - pointer buffer of port default vlan id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_vlan_portPvid_get(rtk_port_t port, uint32 *pPvid)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_l34_ipmcTransTable_set
 * Description:
 *      Set  IPMC Transfer table entry
 * Input:
 *      forced - force set to IPMC Transfer table
 *      *ipmcEntry - value of  IPMC Transfer entry
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_ipmcTransTable_set(uint32 idx, rtk_l34_ipmcTrans_entry_t *ipmcEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}



/* Function Name:
 *      rtk_mirror_portBased_set
 * Description:
 *      Set port mirror function.
 * Input:
 *      mirroringPort         - Monitor port.
 *      pMirroredRxPortmask   - Rx mirror port mask.
 *      pMirroredTxPortmask   - Tx mirror port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      The API is to set mirror function of source port and mirror port.
 *      The mirror port can only be set to one port and the TX and RX mirror ports
 *      should be identical.
 */
int32 rtk_mirror_portBased_set(rtk_port_t mirroringPort, rtk_portmask_t *pMirroredRxPortmask, rtk_portmask_t *pMirroredTxPortmask)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}



/* Function Name:
 *      rtk_mirror_portBased_get
 * Description:
 *      Get port mirror function.
 * Input:
 *      None
 * Output:
 *      pMirroringPort         - Monitor port.
 *      pMirroredRxPortmask   - Rx mirror port mask.
 *      pMirroredTxPortmask   - Tx mirror port mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API is to get mirror function of source port and mirror port.
 */
int32 rtk_mirror_portBased_get(rtk_port_t *pMirroringPort, rtk_portmask_t *pMirroredRxPortmask, rtk_portmask_t *pMirroredTxPortmask)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      port - port id
 *      rate - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */
int32 rtk_rate_portEgrBandwidthCtrlRate_set(rtk_port_t port, uint32 rate)
{
	int ret;

	ret = rtl865x_qosSetBandwidth(port,rate<<10);

	if(ret)
		return (RT_ERR_FAILED);

	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlRate_set
 * Description:
 *      Set the ingress bandwidth control rate.
 * Input:
 *      port - port id
 *      rate - ingress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in apollo is 8Kbps.
 */
int32 rtk_rate_portIgrBandwidthCtrlRate_set(rtk_port_t port, uint32 rate)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate.
 * Input:
 *      port  - port id
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in Apollo is 8Kbps.
 */
int32 rtk_rate_portEgrBandwidthCtrlRate_get(rtk_port_t port, uint32 *pRate)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlRate_get
 * Description:
 *      Get the ingress bandwidth control rate.
 * Input:
 *      port  - port id
 * Output:
 *      pRate - ingress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity in apollo is 8Kbps.
 */
int32 rtk_rate_portIgrBandwidthCtrlRate_get(rtk_port_t port, uint32 *pRate)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      port  - port id
 *      page  - page id
 *      reg   - reg id
 * Output:
 *      pData - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid page id
 *      RT_ERR_PHY_REG_ID   - invalid reg id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_port_phyReg_get(rtk_port_t port,uint32 page,rtk_port_phy_reg_t  reg,uint32 *pData)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      port - port id
 *      page - page id
 *      reg  - reg id
 *      data - reg data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_PHY_PAGE_ID - invalid page id
 *      RT_ERR_PHY_REG_ID  - invalid reg id
 * Note:
 *      None
 */
int32 rtk_port_phyReg_set(rtk_port_t port,uint32 page,rtk_port_phy_reg_t  reg,uint32 data)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set PHY auto negotiation ability of the specific port
 * Input:
 *      port     - port id
 *      pAbility - pointer to the PHY ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      You can set these abilities no matter which mode PHY currently stays on
 */
int32 rtk_port_phyAutoNegoAbility_set(rtk_port_t port,rtk_port_phy_ability_t  *pAbility)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY auto negotiation ability of the specific port
 * Input:
 *      port    - port id
 * Output:
 *      pAbility - pointer to the PHY ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_port_phyAutoNegoAbility_get(rtk_port_t port,rtk_port_phy_ability_t  *pAbility)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_port_macForceAbilityState_get
 * Description:
 *      Get MAC forece ability state
 * Input:
 *      port - the ports for get ability
 *      pState - mac ability state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_port_macForceAbilityState_get(rtk_port_t port,rtk_enable_t *pState)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_rate_stormControlEnable_get
 * Description:
 *      Get enable status of storm control on specified port.
 * Input:
 *      rtk_rate_storm_group_ctrl_t  - storm group type enable control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
*      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 */
int32 rtk_rate_stormControlEnable_get(rtk_rate_storm_group_ctrl_t  *stormCtrl)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_rate_stormControlEnable_set
 * Description:
 *      Set enable status of storm control on specified port.
 * Input:
 *      None
 * Output:
 *      rtk_rate_storm_group_ctrl_t  - storm group type enable control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 *    total 4 storm type can be enabled.
 *      - if total enable group exceed 4 system will return RT_ERR_ENTRY_FULL
 *
 *      - when global storm type set to disable the per port setting for this 
 *        storm type will also set to disable for all port.   
 */
int32 rtk_rate_stormControlEnable_set(rtk_rate_storm_group_ctrl_t  *stormCtrl)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_rate_stormControlPortEnable_set
 * Description:
 *      Set enable status of storm control on specified port.
 * Input:
 *      port       - port id
 *      stormType  - storm group type
 *      enable     - enable status of storm control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_ENTRY_NOTFOUND    - The global strom group is not enable for this group
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 *    - Before call this API must make sure the global strom gruop for given group is enabled,
 *      otherwise this API will return RT_ERR_ENTRY_NOTFOUND  
 */
int32 rtk_rate_stormControlPortEnable_set(rtk_port_t port,rtk_rate_storm_group_t  stormType,rtk_enable_t enable)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_rate_stormControlMeterIdx_set
 * Description:
 *      Set the storm control meter index.
 * Input:
 *      port       - port id
 *      storm_type - storm group type
 *      index       - storm control meter index.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_ENTRY_NOTFOUND    - The global strom group is not enable for this group
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 *      RT_ERR_RATE    - Invalid input bandwidth
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *    - STORM_GROUP_DHCP
 *    - STORM_GROUP_ARP
 *    - STORM_GROUP_IGMP_MLD
 *    - Before call this API must make sure the global strom gruop for given group is enabled,
 *      otherwise this API will return RT_ERR_ENTRY_NOTFOUND
 */
int32 rtk_rate_stormControlMeterIdx_set(rtk_port_t port,rtk_rate_storm_group_t  stormType,uint32 index)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_rate_shareMeter_set
 * Description:
 *      Set meter configuration
 * Input:
 *      index       - shared meter index
 *      rate        - rate of share meter
 *      ifgInclude  - include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 *      RT_ERR_RATE             - Invalid rate
 *      RT_ERR_INPUT            - Invalid input parameters
 * Note:
 *      The API can set shared meter rate and ifg include for each meter.
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k.
 *      The granularity of rate is 8 kbps. The ifg_include parameter is used
 *      for rate calculation with/without inter-frame-gap and preamble.
 */
int32 rtk_rate_shareMeter_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_rate_shareMeter_get
 * Description:
 *      Get meter configuration
 * Input:
 *      index        - shared meter index
 * Output:
 *      pRate        - pointer of rate of share meter
 *      pIfgInclude  - include IFG or not, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can get shared meter rate and ifg include for each meter.
 *      The rate unit is 1 kbps and the granularity of rate is 8 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble
 */
int32 rtk_rate_shareMeter_get(uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of egress bandwidth control includes IFG or not.
 * Input:
 *      ifgInclude - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32 rtk_rate_egrBandwidthCtrlIncludeIfg_set(rtk_enable_t ifgInclude)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_ponmac_queue_get
 * Description:
 *      get queue setting
 * Input:
 *      pQueue         - queue id and scheduler id for ths queue.
 * Output:
 *      pQueueCfg     - queue configuration
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueueList point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32 rtk_ponmac_queue_get(rtk_ponmac_queue_t *pQueue, rtk_ponmac_queueCfg_t *pQueueCfg)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_ponmac_queue_add
 * Description:
 *      Add queue to given scheduler id and apply queue setting
 * Input:
 *      pQueue         - queue id and scheduler id for ths queue.
 *      pQueueCfg     - queue configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pQueueList point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None
 */
int32 rtk_ponmac_queue_add(rtk_ponmac_queue_t *pQueue, rtk_ponmac_queueCfg_t *pQueueCfg)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_schedulingQueue_set
 * Description:
 *      Set the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      port      - port id
 *      pQweights - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - Invalid port id
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight
 * Note:
 *      The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *      If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
int32 rtk_qos_schedulingQueue_set(rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_schedulingQueue_get
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      port      - port id
 * Output:
 *      pQweights - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *      If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
int32 rtk_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_sec_portAttackPreventState_set
 * Description:
 *      Per port set attack prevention confi state
 * Input:
 *      port   - port id.
 *      enable - status attack prevention
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 * Note:
 *      The status attack prevention:
 *      - DISABLED
 *      - ENABLED
 */
int32 rtk_sec_portAttackPreventState_set(rtk_port_t port, rtk_enable_t enable)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_sec_portAttackPreventState_get
 * Description:
 *      Per port get attack prevention confi state
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - status attack prevention
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      The status attack prevention:
 *      - DISABLED
 *      - ENABLED
 */
int32 rtk_sec_portAttackPreventState_get(rtk_port_t port, rtk_enable_t *pEnable)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_sec_attackPrevent_set
 * Description:
 *      Set action for each kind of attack.
 * Input:
 *      attack_type - type of attack
 *      action      - action for attack
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 *      RT_ERR_INPUT      - invalid input parameter
 * Note:
 *      Action is as following:
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_FORWARD
 */
int32 rtk_sec_attackPrevent_set(rtk_sec_attackType_t attackType,rtk_action_t action)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_sec_attackPrevent_get
 * Description:
 *      Get action for each kind of attack on specified port.
 * Input:
 *      attackType - type of attack
 * Output:
 *      pAction     - pointer to action for attack
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Action is as following:
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_FORWARD
 */
int32 rtk_sec_attackPrevent_get(rtk_sec_attackType_t attackType,rtk_action_t *pAction)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_sec_attackFloodThresh_set
 * Description:
 *      Set  flood threshold, time unit 1ms.
 * Input:
 *      floodThresh - flood threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Flood type is as following:
 *      - SEC_ICMPFLOOD
 *      - SEC_SYNCFLOOD
 *      - SEC_FINFLOOD
 */
int32 rtk_sec_attackFloodThresh_set(rtk_sec_attackFloodType_t type, uint32 floodThresh)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      port        - port id
 * Output:
 *      pPortCntrs - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
int32 rtk_stat_port_getAll(rtk_port_t port, rtk_stat_port_cntr_t *pPortCntrs)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset the specified port counters in the specified device.
 * Input:
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
int32 rtk_stat_port_reset(rtk_port_t port)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_priMap_get
 * Description:
 *      Get the entry of internal priority to QID mapping table.
 * Input:
 *      group     - the group of priority to Queue id map(0~3).
 *      pPri2qid  - array of internal priority on a queue
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_ENTRY_INDEX  - Invalid group index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *     None 
 */
int32 rtk_qos_priMap_get(uint32 group, rtk_qos_pri2queue_t *pPri2qid)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_qos_priMap_set
 * Description:
 *      Set the entry of internal priority to QID mapping table.
 * Input:
 *      group     - the group of priority to Queue id map(0~3).
 *      pPri2qid  - array of internal priority on a queue
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_ENTRY_INDEX  - Invalid group index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Below is an example of internal priority to QID mapping table.
 *      -
 *      -              Priority
 *      -  group        0   1   2   3   4   5   6   7
 *      -              ================================ 
 *      -        0      0   1   2   3   4   5   6   7
 *      -        1      0   0   0   0   0   0   0   0
 *      -        2      0   0   0   0   6   6   6   6
 *      -        3      0   0   0   1   1   2   2   3
 *      -for table index 0
 *      -    pPri2qid[0] = 0   internal priority 0 map to queue 0       
 *      -    pPri2qid[1] = 1   internal priority 1 map to queue 1 
 *      -    pPri2qid[2] = 2   internal priority 2 map to queue 2 
 *      -    pPri2qid[3] = 3   internal priority 3 map to queue 3 
 *      -    pPri2qid[4] = 4   internal priority 4 map to queue 4 
 *      -    pPri2qid[5] = 5   internal priority 5 map to queue 5 
 *      -    pPri2qid[6] = 6   internal priority 6 map to queue 6  
 *      -    pPri2qid[7] = 7   internal priority 7 map to queue 7 
 */
int32 rtk_qos_priMap_set(uint32 group, rtk_qos_pri2queue_t *pPri2qid)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_qos_priSelGroup_get
 * Description:
 *      Get weight of each priority assignment on specified priority selection group.
 * Input:
 *      grpIdx         - index of priority selection group
 * Output:
 *      pWeightOfPriSel - pointer to weight of each priority assignment
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Apollo only support group 0 
 */
int32 rtk_qos_priSelGroup_get(uint32 grpIdx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_dscpPriRemapGroup_get
 * Description:
 *      Get remapped internal priority of DSCP on specified DSCP remapping group.
 * Input:
 *      grpIdx   - index of dscp remapping group
 *      dscp     - DSCP
 * Output:
 *      pIntPri  - pointer to internal priority
 *      pDp      - pointer to drop precedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE - invalid DSCP value
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_INPUT          - invalid input parameter
 * Note:
 *      Apollo only support group 0
 */
int32 rtk_qos_dscpPriRemapGroup_get(uint32 grpIdx,uint32 dscp,rtk_pri_t *pIntPri,uint32 *pDp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_1pPriRemapGroup_get
 * Description:
 *      Get remapped internal priority of dot1p priority on specified dot1p priority remapping group.
 * Input:
 *      grpIdx   - index of outer dot1p remapping group
 *      dot1pPri - dot1p priority
 * Output:
 *      pIntPri  - pointer to internal priority
 *      pDp       - pointer to drop precedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY - invalid dot1p priority
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_INPUT           - invalid input parameter
 * Note:
 *      Apollo only support group 0
 */
int32 rtk_qos_1pPriRemapGroup_get(uint32 grpIdx,rtk_pri_t dot1pPri,rtk_pri_t *pIntPri,uint32 *pDp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_set
 * Description:
 *      Set DSCP remark status for a port
 * Input:
 *      port   - port id
 *      enable - status of DSCP remark
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 * Note:
 *      The status of DSCP remark:
 *      - DISABLED
 *      - ENABLED
 */
int32 rtk_qos_dscpRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_portDscpRemarkSrcSel_set
 * Description:
 *      Set remarking source of DSCP remarking.
 * Input:
 *      port                 - port id
 *      type      - remarking source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter

 * Note:
 *      The API can configure DSCP remark functionality to map original DSCP value or internal
 *      priority to TX DSCP value.
 */
int32 rtk_qos_portDscpRemarkSrcSel_set(rtk_port_t port, rtk_qos_dscpRmkSrc_t type)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_get
 * Description:
 *      Get DSCP remark status for a port
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - status of DSCP remark
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      The status of DSCP remark:
 *      - DISABLED
 *      - ENABLED
 */
int32 rtk_qos_dscpRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_portDscpRemarkSrcSel_get
 * Description:
 *      Get remarking source of DSCP remarking.
 * Input:
 *      port                 - port id
 * Output:
 *      pType      - remarking source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer

 * Note:
 *      None
 */
int32 rtk_qos_portDscpRemarkSrcSel_get(rtk_port_t port, rtk_qos_dscpRmkSrc_t *pType)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_dscpRemarkGroup_set
 * Description:
 *      Set remarked DSCP of internal priority on specified dscp remark group.
 * Input:
 *      grpIdx - index of dot1p remarking group
 *      intPri - internal priority value (range from 0 ~ 7)
 *      dp      - drop precedence
 *      dscp    - DSCP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE   - invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter

 * Note:
 *      (1) The valid range of grp_idx is 0 for apollo
 *      (2) dp(drop precedence) is not implement in Apollo  
 */
int32 rtk_qos_dscpRemarkGroup_set(uint32 grpIdx,rtk_pri_t intPri,uint32 dp,uint32 dscp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_dscpRemarkGroup_get
 * Description:
 *      Get remarked DSCP of internal priority on specified dscp remark group.
 * Input:
 *      grpIdx - index of dot1p remapping group
 *      intPri - internal priority
 *      dp      - drop precedence
 * Output:
 *      pDscp   - pointer to DSCP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      (1) The valid range of grp_idx is 0 for apollo
 */
int32 rtk_qos_dscpRemarkGroup_get(uint32 grpIdx,rtk_pri_t intPri,uint32 dp,uint32 *pDscp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_dscp2DscpRemarkGroup_set
 * Description:
 *      Set DSCP to remarked DSCP mapping.
 * Input:
 *      grpIdx  - group index
 *      dscp    - DSCP value
 *      rmkDscp - remarked DSCP value
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 * Note:
 *      dscp parameter can be DSCP value or internal priority according to configuration of API 
 *      dal_apollo_qos_dscpRemarkSrcSel_set(), because DSCP remark functionality can map original DSCP 
 *      value or internal priority to TX DSCP value.
 */
int32 rtk_qos_dscp2DscpRemarkGroup_set(uint32 grpIdx,uint32 dscp, uint32 rmkDscp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_qos_dscp2DscpRemarkGroup_get
 * Description:
 *      Get DSCP to remarked DSCP mapping.
 * Input:
 *      dscp    - DSCP value
 *      grpIdx  - group index
 * Output:
 *      pDscp   - remarked DSCP value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      None.
 */
int32 rtk_qos_dscp2DscpRemarkGroup_get(uint32 grpIdx, uint32 dscp, uint32 *pDscp)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1p remark status for a port
 * Input:
 *      port   - port id.
 *      enable - status of 802.1p remark
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 * Note:
 *      The status of 802.1p remark:
 *      - DISABLED
 *      - ENABLED
 */
int32 rtk_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_1pRemarkGroup_set
 * Description:
 *      Set remarked dot1p priority of internal priority on specified dot1p remark group.
 * Input:
 *      grpIdx   - index of dot1p remark group
 *      intPri   - internal priority
 *      dp       - drop precedence
 *      dot1pPri - dot1p priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32 rtk_qos_1pRemarkGroup_set(uint32 grpIdx,rtk_pri_t intPri,uint32 dp,rtk_pri_t dot1pPri)
{
	uint32 regValue;
	
	/* Invalid input parameter */
	if ((intPri < 0) || (intPri > 7) || (dot1pPri < 0) || (dot1pPri > 7))
		return FAILED; 

	rtl8651_EnablePortRemark_8021p(0);
	rtl8651_EnablePortRemark_8021p(1);
	rtl8651_EnablePortRemark_8021p(2);
	rtl8651_EnablePortRemark_8021p(3);
	rtl8651_EnablePortRemark_8021p(4);

	regValue = READ_MEM32(RMCR1P);
	switch (intPri)
	{
		case PRI0:
			WRITE_MEM32(RMCR1P, (regValue & ~(RM0_1P_MASK)) | (dot1pPri << RM0_1P_OFFSET));  break;
		case PRI1:
			WRITE_MEM32(RMCR1P, (regValue & ~(RM1_1P_MASK)) | (dot1pPri << RM1_1P_OFFSET));  break;
		case PRI2:
			WRITE_MEM32(RMCR1P, (regValue & ~(RM2_1P_MASK)) | (dot1pPri << RM2_1P_OFFSET));  break;
		case PRI3:
			WRITE_MEM32(RMCR1P, (regValue & ~(RM3_1P_MASK)) | (dot1pPri << RM3_1P_OFFSET));  break;
		case PRI4:
			WRITE_MEM32(RMCR1P, (regValue & ~(RM4_1P_MASK)) | (dot1pPri << RM4_1P_OFFSET));  break;
		case PRI5:
			WRITE_MEM32(RMCR1P, (regValue & ~(RM5_1P_MASK)) | (dot1pPri << RM5_1P_OFFSET));  break;
		case PRI6:
			WRITE_MEM32(RMCR1P, (regValue & ~(RM6_1P_MASK)) | (dot1pPri << RM6_1P_OFFSET));  break;
		case PRI7:
			WRITE_MEM32(RMCR1P, (regValue & ~(RM7_1P_MASK)) | (dot1pPri << RM7_1P_OFFSET));  break;
	}
	
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_1pRemarkGroup_get
 * Description:
 *      Get remarked dot1p priority of internal priority on specified dot1p remark group.
 * Input:
 *      grpIdx    - index of dot1p remark group
 *      intPri    - internal priority
 *      dp         - drop precedence
 * Output:
 *      pDot1pPri - pointer to dot1p priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_qos_1pRemarkGroup_get(uint32 grpIdx,rtk_pri_t intPri,uint32 dp,rtk_pri_t *pDot1pPri)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_qos_portPri_get
 * Description:
 *      Get internal priority of one port.
 * Input:
 *      port     - port id
 * Output:
 *      pIntPri  - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                 the highest prioirty)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    None
 */
int32 rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pIntPri)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}



/* Function Name:
 *      rtk_sec_attackFloodThresh_get
 * Description:
 *      Get flood threshold, time unit 1ms.
 * Input:
 *      None
 * Output:
 *      pFloodThresh - pointer to flood threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Flood type is as following:
 *      - SEC_ICMPFLOOD
 *      - SEC_SYNCFLOOD
 *      - SEC_FINFLOOD
 */
int32 rtk_sec_attackFloodThresh_get(rtk_sec_attackFloodType_t type, uint32 *pFloodThresh)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

int intr_bcaster_notifier_cb_register(intrBcasterNotifier_t     *pRegNotifier)
{
#if 0
    struct notifier_block   *pNb;
    intrBcasterNotifier_t   *pNotifier;

    if (!pRegNotifier)
        return RT_ERR_FAILED;

    pNb = kmalloc(sizeof(struct notifier_block) +
                    sizeof(intrBcasterNotifier_t), GFP_KERNEL);
    if (!pNb)
        return RT_ERR_FAILED;

    pNotifier = (intrBcasterNotifier_t *)(pNb + 1);

    // fill data
    pNb->notifier_call = intr_bcaster_notifier_dispatch_cb;
    pNotifier->notifierCb = pRegNotifier->notifierCb;
    pNotifier->notifyType = pRegNotifier->notifyType;

    // keep nb addr
    pRegNotifier->pNbAddr = pNb;

    if (blocking_notifier_chain_register(&gNotifierHead, pNb))
    {
        kfree(pNb);

        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
#endif
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);


}


int re8670_rx_skb (struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}

struct sk_buff *re8670_getAlloc(unsigned int size)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);

}



#endif //FIXEME_FUNCITON




#define NOT_SUPPORT_FUNCTION 1
#if NOT_SUPPORT_FUNCTION

/* Function Name:
 *      rtk_l34_hsdState_set
 * Description:
 *      Set L34 hsd state
 * Input:
 *      hsdState - L34 hsd state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_hsdState_set(rtk_enable_t hsdState)
{
/*
	Not support HSD but it must call by init.
	don't printk any message just return RT_ERR_OK
*/
	return (RT_ERR_OK);
}

int32 rtk_l34_hwL4TrfWrkTbl_Clear(rtk_l34_l4_trf_t l4TrfTable)
{
/*
	Not support hwL4TrfWrkTbl but it must call by init.
	don't printk any message just return RT_ERR_OK
*/
	return (RT_ERR_OK);
}



/*xdsl not support svlan*/

/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Get service ports from the specified device.
 * Input:
 *      port        - port id
 * Output:
 *      pEnable     - status of service port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_svlan_servicePort_get(rtk_port_t port, rtk_enable_t *pEnable)
{
	*pEnable = DISABLE;
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}
/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get the svlan TPID.
 * Input:
 *      svlanIndex   - index of tpid entry
 * Output:
 *      pSvlanTagId - pointer buffer of svlan TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Only support pSvlanTagId 0 in Apollo.
 */

int32 rtk_svlan_tpidEntry_get(uint32 svlanIndex, uint32 *pSvlanTagId)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}


int32 rtk_svlan_deiKeepState_set(rtk_enable_t enable){

	WARNING("svlan not support\n");
	return (RT_ERR_OK);

}


/* Function Name:
 *      rtk_svlan_dmacVidSelState_set
 * Description:
 *      Set DMAC CVID selection status
 * Input:
 *      port    - Port
 *      enable  - state of DMAC CVID Selection
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_PORT_ID      			- Invalid port id.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can set DMAC CVID Selection state
 */
int32 rtk_svlan_dmacVidSelState_set(rtk_port_t port, rtk_enable_t enable)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}
/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Set the svlan TPID.
 * Input:
 *      svlan_index  - index of svlan table
 *      svlan_tag_id - svlan TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Only support pSvlan_tag_id 0 in Apollo.
 */
int32 rtk_svlan_tpidEntry_set(uint32 svlan_index, uint32 svlan_tag_id)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_svlan_servicePort_set
 * Description:
 *      Set service ports to the specified device.
 * Input:
 *      port       - port id
 *      enable     - status of service port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 rtk_svlan_servicePort_set(rtk_port_t port, rtk_enable_t enable)
{
	/*FIXME: empty function */
	WARNING("svlan not support\n");
	return (RT_ERR_OK);

}

/* Function Name:
 *      rtk_svlan_memberPort_set
 * Description:
 *      Replace the svlan members.
 * Input:
 *      svid            - svlan id
 *      pSvlanPortmask - svlan member ports
 *      pSvlanUntagPortmask - svlan untag member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SVLAN_ENTRY_INDEX     - invalid svid entry no
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found
 * Note:
 *      (1) Don't care the original svlan members and replace with new configure
 *          directly.
 *      (2) svlan portmask only for svlan ingress filter checking
 */
int32 rtk_svlan_memberPort_set(rtk_vlan_t svid, rtk_portmask_t *pSvlanPortmask, rtk_portmask_t *pSvlanUntagPortmask)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}


/* Function Name:
 *      rtk_svlan_fidEnable_set
 * Description:
 *      Set svlan based fid assignment status.
 * Input:
 *      svid  - svlan id
 *      enable - svlan based fid assignment status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 *      RT_ERR_VLAN_VID
 *      RT_ERR_SVLAN_NOT_EXIST
 * Note:
 *      None
 */
int32 rtk_svlan_fidEnable_set(rtk_vlan_t svid, rtk_enable_t enable)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_svlan_fid_set
 * Description:
 *      Set the filter id of the svlan.
 * Input:
 *      svid  - svlan id
 *      fid  - filter id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 *      RT_ERR_VLAN_VID
 *      RT_ERR_SVLAN_NOT_EXIST
 * Note:
 *      The FID is effective only in VLAN SVL mode. 
 */
int32 rtk_svlan_fid_set(rtk_vlan_t svid, rtk_fid_t fid)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}
/* Function Name:
 *      rtk_svlan_untagAction_set
 * Description:
 *      Configure Action of downstream UnStag packet
 * Input:
 *      action  - Action for UnStag
 *      svid    - The SVID assigned to UnStag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also supported by this API. The parameter of svid is
 *      only referenced when the action is set to UNTAG_ASSIGN
 */
int32 rtk_svlan_untagAction_set(rtk_svlan_action_t action, rtk_vlan_t svid)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}
/* Function Name:
 *      rtk_svlan_sp2cUnmatchCtagging_set
 * Description:
 *      Set unmatch sp2c egress ctagging state
 * Input:
 *      state      - unmatch cvlan tagging state
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 rtk_svlan_sp2cUnmatchCtagging_set(rtk_enable_t state)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_svlan_create
 * Description:
 *      Create the svlan.
 * Input:
 *      svid - svlan id to be created
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SVLAN_EXIST - SVLAN entry is exist
 * Note:
 *      None
 */
int32 rtk_svlan_create(rtk_vlan_t svid)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_svlan_unmatchAction_set
 * Description:
 *      Configure Action of downstream Unmatch packet
 * Input:
 *      action  - Action for Unmatch
 *      svid    - The SVID assigned to Unmatch packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also supported by this API. The parameter add svid is
 *      only refernced when the action is set to UNMATCH_ASSIGN
 */
int32 rtk_svlan_unmatchAction_set(rtk_svlan_action_t action, rtk_vlan_t svid)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_svlan_dmacVidSelForcedState_set
 * Description:
 *      Set DMAC CVID selection status
 * Input:
 *      port    - Port
 *      enable  - state of DMAC CVID Selection
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can set DMAC CVID Selection forced state
 */
int32 rtk_svlan_dmacVidSelForcedState_set(rtk_enable_t enable)
{
	WARNING("svlan not support\n");
	return (RT_ERR_OK);
}

/* XDSL not support bind function*/
rtk_rg_fwdEngineReturn_t _rtk_rg_bindingRuleCheck(rtk_rg_pktHdr_t *pPktHdr, int *wanGroupIdx)
{
	return 0;
}
int32 _rtk_rg_updatingVlanBind(int wanIdx,int v6wanTypeIdx)
{
	return (RT_ERR_RG_OK);
}
int32 _rtk_rg_addBindFromPortmask(unsigned int pmsk, unsigned int expmsk, int intfIdx, int wantypeIdx, int v6WantypeIdx)
{
	return (RT_ERR_RG_OK);
}
int32 _rtk_rg_updateBindWanIntf(rtk_rg_wanIntfConf_t *wanintf)
{
	return (RT_ERR_RG_OK);
}
int32 _rtk_rg_updateNoneBindingPortmask(uint32 wanPmsk)
{
	return (RT_ERR_RG_OK);
}
void _rtk_rg_deletingPortBindFromInterface(int intfIdx)
{
	return ;
}
int _rtk_rg_portBindingByProtocal_change( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return (RT_ERR_RG_OK);
}
int _rtk_rg_portBindingByProtocal_state(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	return (RT_ERR_RG_OK);
}
int32 _rtk_rg_updateBindOtherWanPortBasedVID(rtk_rg_wanIntfConf_t *otherWanIntf)
{
	return (RT_ERR_RG_OK);
}



/* Function Name:
 *      rtk_l34_bindingTable_set
 * Description:
 *      Set binding table
 * Input:
 *      idx - index of binding table
 *      *bindEntry - point of binding data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_bindingTable_set(uint32 idx,rtk_binding_entry_t *bindEntry)
{
	WARNING("xdsl not binding\n");
	return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l34_bindingAction_set
 * Description:
 *     Set binding action
 * Input:
 *      bindType - binding type
 *      bindAction - binding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_bindingAction_set(rtk_l34_bindType_t bindType, rtk_l34_bindAct_t bindAction)
{
	WARNING("xdsl not support binding\n");
	return RT_ERR_OK;
}



/* Function Name:
 *      rtk_vlan_priorityEnable_set
 * Description:
 *      Set vlan based priority assignment status.
 * Input:
 *      vid    - vlan id
 *      enable - vlan based priority assignment status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      None
 */
int32 rtk_vlan_priorityEnable_set(rtk_vlan_t vid, rtk_enable_t enable)
{
	WARNING("xdsl not support vlan priority ENABLE\n");
	return RT_ERR_OK;
}


/* Function Name:
 *      rtk_vlan_priority_set
 * Description:
 *      Set VLAN priority for each CVLAN.
 * Input:
 *      vid  - vlan id
 *      priority - 802.1p priority for the PVID.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_VLAN_PRIORITY    - Invalid priority.
 * Note:
 *      This API is used to set priority per VLAN.
 */
int32 rtk_vlan_priority_set(rtk_vlan_t vid, rtk_pri_t priority)
{
	WARNING("xdsl not support vlan priority \n");
	return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_fidMode_set
 * Description:
 *      Set the filter id mode of the vlan.
 * Input:
 *      vid   - vlan id
 *      mode  - filter id mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_VLAN_VID             - invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - specified vlan entry not found
 *      RT_ERR_OUT_OF_RANGE         - input parameter out of range
 * Note:
 *      mode can be: -VLAN__FID_IVL
 *                   -VLAN__FID_SVL
 */
int32 rtk_vlan_fidMode_set(rtk_vlan_t vid, rtk_fidMode_t mode)
{
	int ret;
	if(mode==VLAN_FID_IVL){
		ret=RT_ERR_OK;
		WARNING("xdsl not support IVL mode\n");
	}else
	{
		//we only SVL mode , no need to set
		ret=RT_ERR_OK;
	}
	return ret;
}


/* Function Name:
 *      rtk_l34_wanTypeTable_set
 * Description:
 *      Set WAN type entry by idx.
 * Input:
 *      idx - index of wan type table for binding
 *      *wanTypeEntry - point of wan type table entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 rtk_l34_wanTypeTable_set(uint32 idx, rtk_wanType_entry_t *wanTypeEntry)
{
	/*FIXME: empty function */
	FIXME(" empty function \n");
	return (RT_ERR_OK);
}

/* Function Name:
 *      rtk_l2_portLookupMissAction_set
 * Description:
 *      Set forwarding action of specified port when destination address lookup miss.
 * Input:
 *      port    - port id
 *      type    - type of lookup miss
 *      action  - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid type of lookup miss
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_MCAST
 *      - DLF_TYPE_IP6MC
 *
 *      Forwarding action is as following:
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_FORWARD
 *      - ACTION_DROP_EXCLUDE_RMA (Only for DLF_TYPE_MCAST)
 */
int32 rtk_l2_portLookupMissAction_set(rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
	/*FIXME: empty function */
	FIXME("unknow DA not support per port setting \n");
	return (RT_ERR_OK);
}


int32 rtk_l2_portAgingEnable_set(rtk_port_t port, rtk_enable_t enable)
{
	//FIXME
	return (RT_ERR_OK);
}

#endif //EMPTY_FUNCTION
