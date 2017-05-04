#include <rtk_rg_xdsl_extAPI.h>
#include <rtk_rg_acl.h>
#include <rtk_rg_acl_xdsl.h>
#include <net/rtl/rtl865x_netif.h>

#ifndef ASSERT_EQ_AND_RESET
#define ASSERT_EQ_AND_RESET(value1,value2)\
do {\
		int v1;\
		int v2;\
		v1=value1;\
		v2=value2;\
        if ((v1) != (v2)) {\
                ACL("\033[31;43m%s(%d): value1=0x%x value2=0x%x, fail, so abort!\033[m\n", __FUNCTION__, __LINE__,(v1),(v2));\
                return RT_ERR_RG_FAILED; \
        }\
        v1=0;\
}while (0)
#endif

#define inet_ntoa(x) _inet_ntoa(x)

static inline int8 *_ui8tod( uint8 n, int8 *p )
{
	if( n > 99 ) *p++ = (n/100) + '0';
	if( n >  9 ) *p++ = ((n/10)%10) + '0';
	*p++ = (n%10) + '0';
	return p;
}
static int8 *_inet_ntoa(rtk_ip_addr_t ina)
{
	static int8 buf[4*sizeof "123"];
	int8 *p = buf;
	uint8 *ucp = (unsigned char *)&ina;

	p = _ui8tod( ucp[0] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[1] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[2] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[3] & 0xFF, p);
	*p++ = '\0';

	return (buf);
}
static int8 *diag_util_inet_ntoa(uint32 ina)
{	
	static int8 buf[4*sizeof "123"];    
	sprintf(buf, "%d.%d.%d.%d", ((ina>>24)&0xff), ((ina>>16)&0xff), ((ina>>8)&0xff), (ina&0xff));	
	return (buf);
}


/*(1)ACL init related APIs*/
int _rtk_rg_acl_asic_init(void)
{	
	rtl8651_clearSpecifiedAsicTable(TYPE_ACL_RULE_TABLE, RTL8651_ACLHWTBL_SIZE);  
	return (RT_ERR_RG_OK);
}
int _rtk_rg_classify_asic_init(void)	
{
	FIXME("XDSL FIXME _rtk_rg_classify_asic_init latter\n");
	return (RT_ERR_RG_OK);
}

/*(2)RG_ACL APIs and internal APIs*/
typedef enum rtk_rg_xdsl_acl_filter_fields_e
{
        xDSL_ACL_TYPE_ETHERNET_BIT=0x1,
        xDSL_ACL_TYPE_IPv4_BIT=0x2,
        xDSL_ACL_TYPE_ICMP_BIT=0x4,
        xDSL_ACL_TYPE_IGMP_BIT=0x8,
        xDSL_ACL_TYPE_TCP_BIT=0x10,
        xDSL_ACL_TYPE_UDP_BIT=0x20,
        xDSL_ACL_TYPE_SRCFILTER_BIT=0x40,
        xDSL_ACL_TYPE_DSTFILTER_BIT=0x80,
#if defined(CONFIG_RTL_8685S_HWNAT)
        xDSL_ACL_TYPE_IPv6_BIT=0x100,
#endif
} rtk_rg_xdsl_acl_filter_fields_t;

int _rtk_rg_xdsl_acl_resource_require(rtk_rg_aclFilterAndQos_t *acl_filter,uint32 *acl_type_igr_bitmask,uint32 *acl_type_egr_bitmask,uint32 *required_igr_entry_number,uint32 *required_egr_entry_number)
{
    int i;
    
    if(acl_type_igr_bitmask==NULL || acl_type_egr_bitmask==NULL || required_igr_entry_number==NULL || required_egr_entry_number==NULL)
            return FAILED;

    *acl_type_igr_bitmask = *acl_type_egr_bitmask = *required_igr_entry_number = *required_egr_entry_number = 0;

    if(acl_filter->filter_fields & INGRESS_PORT_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_SRCFILTER_BIT;
    if(acl_filter->filter_fields & INGRESS_INTF_BIT)
            ;//Could not support
    if(acl_filter->filter_fields & EGRESS_INTF_BIT)
            ;//Could not support
    if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_ETHERNET_BIT;
    if(acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
            ;//Could not support
    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
            *acl_type_igr_bitmask |= (xDSL_ACL_TYPE_SRCFILTER_BIT | xDSL_ACL_TYPE_ETHERNET_BIT);
    if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_SRCFILTER_BIT;
    if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_DSTFILTER_BIT;
    if(acl_filter->filter_fields & INGRESS_DSCP_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
#if defined(CONFIG_RTL_8685S_HWNAT)
    if(acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv6_BIT;
    if(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv6_BIT;
#endif
    if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
    if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
    if(acl_filter->filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)
            *acl_type_egr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
	if(acl_filter->filter_fields & EGRESS_IPV4_DIP_RANGE_BIT)
		   *acl_type_egr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;

	if((acl_filter->filter_fields & INGRESS_L4_TCP_BIT) && ((acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT) || (acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)) )
	{
		*acl_type_igr_bitmask |= xDSL_ACL_TYPE_TCP_BIT;
	}
	else if((acl_filter->filter_fields & INGRESS_L4_UDP_BIT) && ((acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT) || (acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)) )
	{
		*acl_type_igr_bitmask |= xDSL_ACL_TYPE_UDP_BIT;
	}
	else
	{
		if(acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT)
				*acl_type_igr_bitmask |= xDSL_ACL_TYPE_SRCFILTER_BIT;
		if(acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)
				*acl_type_igr_bitmask |= xDSL_ACL_TYPE_DSTFILTER_BIT;
	    if(acl_filter->filter_fields & INGRESS_L4_TCP_BIT)
	            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;        
	    if(acl_filter->filter_fields & INGRESS_L4_UDP_BIT)
	            *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
	}
	if(acl_filter->filter_fields & EGRESS_L4_SPORT_RANGE_BIT)
		   *acl_type_egr_bitmask |= xDSL_ACL_TYPE_SRCFILTER_BIT;
	if(acl_filter->filter_fields & EGRESS_L4_DPORT_RANGE_BIT)
		   *acl_type_egr_bitmask |= xDSL_ACL_TYPE_DSTFILTER_BIT;
	if(acl_filter->filter_fields & INGRESS_L4_ICMP_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
	if(acl_filter->filter_fields & EGRESS_CTAG_PRI_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & EGRESS_CTAG_VID_BIT)
		   *acl_type_egr_bitmask |= xDSL_ACL_TYPE_SRCFILTER_BIT;
#if defined(CONFIG_RTL_8685S_HWNAT)
	if(acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv6_BIT;
#endif
	if(acl_filter->filter_fields & INGRESS_STREAM_ID_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & INGRESS_STAG_PRI_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & INGRESS_STAG_VID_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & INGRESS_STAGIF_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & INGRESS_CTAGIF_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_ETHERNET_BIT;
	if(acl_filter->filter_fields & INGRESS_EGRESS_PORTIDX_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & INTERNAL_PRI_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
	if(acl_filter->filter_fields & INGRESS_TOS_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
#if defined(CONFIG_RTL_8685S_HWNAT)
	if(acl_filter->filter_fields & INGRESS_IPV6_TC_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv6_BIT;
	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv6_BIT;
	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv6_BIT;
#endif
	if(acl_filter->filter_fields & INGRESS_WLANDEV_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv4_BIT;
#if defined(CONFIG_RTL_8685S_HWNAT)
	if(acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv6_BIT;
	if(acl_filter->filter_fields & INGRESS_L4_ICMPV6_BIT)
		   *acl_type_igr_bitmask |= xDSL_ACL_TYPE_IPv6_BIT;
#endif
	if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT)
		   ;//Could not support
	if(acl_filter->filter_fields & INGRESS_STAG_DEI_BIT)
		   ;//Could not support

	ACL("Require xDSL ACL type igr:%x egr:%x @ %s %d \n",*acl_type_igr_bitmask,*acl_type_egr_bitmask,__func__,__LINE__);
	ACL("[ETHERNET:0x%x] [IP:0x%x] [ICMP:0x%x] [IGMP:0x%x] [TCP:0x%x] [UDP:0x%x] [SRCFILTER:0x%x] [DSTFILTER:0x%x] [IPv6:0x%x]",
		xDSL_ACL_TYPE_ETHERNET_BIT,xDSL_ACL_TYPE_IPv4_BIT,xDSL_ACL_TYPE_ICMP_BIT,
        xDSL_ACL_TYPE_IGMP_BIT,xDSL_ACL_TYPE_TCP_BIT,xDSL_ACL_TYPE_UDP_BIT,
        xDSL_ACL_TYPE_SRCFILTER_BIT,xDSL_ACL_TYPE_DSTFILTER_BIT
#if defined(CONFIG_RTL_8685S_HWNAT)
        ,xDSL_ACL_TYPE_IPv6_BIT);
#else
		,0);
#endif

	*required_igr_entry_number = *required_egr_entry_number = 0;
	for(i=0;i<9;i++)
	{
		  if(*acl_type_igr_bitmask & (0x1<<i))
				  *required_igr_entry_number += 1;
		  if(*acl_type_egr_bitmask & (0x1<<i))
				  *required_egr_entry_number += 1;
	}
	return RT_ERR_RG_OK;
}

int _rtk_rg_aclSWEntry_and_asic_chkReource(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry)
{
	int ret;
	int needs_igr_entry_number,needs_egr_entry_number;
	int acl_type_igr_bitmask=0,acl_type_egr_bitmask=0;
	int free_entry_number;
	int i;

	ACL("Enter %s @ %d \n",__func__,__LINE__);
	ret = _rtk_rg_xdsl_acl_resource_require(acl_filter,&acl_type_igr_bitmask,&acl_type_egr_bitmask,&needs_igr_entry_number,&needs_egr_entry_number);
#if defined(CONFIG_RTL_8685S_HWNAT)
	if(ret != RT_ERR_RG_OK)
#else
	if(ret != RT_ERR_RG_OK || needs_igr_entry_number >=2 || needs_egr_entry_number >= 2)
#endif
	{
		ACL("xDSL ASIC could not support this ACL rule! Leave %s @ %d\n",__func__,__LINE__);
		return RT_ERR_RG_OK;
	}

	/* ingress ACL */
	free_entry_number=MAX_ACL_IGR_ENTRY_SIZE-xDSL_ACL_IGR_RSVD_HEAD_SIZE-xDSL_ACL_IGR_RSVD_TAIL_SIZE;
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        if(_rtk_rg_aclSWEntry_get(i,aclSWEntry))
            continue;
		if(aclSWEntry->hw_aclEntry_start<MAX_ACL_IGR_ENTRY_SIZE && (aclSWEntry->hw_aclEntry_size>0))
			free_entry_number -= aclSWEntry->hw_aclEntry_size;
    }

	if(free_entry_number < needs_igr_entry_number)
	{
		ACL("Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}

	/* egress ACL */
	free_entry_number=MAX_ACL_EGR_ENTRY_SIZE-xDSL_ACL_EGR_RSVD_HEAD_SIZE-xDSL_ACL_EGR_RSVD_TAIL_SIZE;
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        if(_rtk_rg_aclSWEntry_get(i,aclSWEntry))
            continue;
		if(aclSWEntry->hw_aclEntry_start>=MAX_ACL_IGR_ENTRY_SIZE && aclSWEntry->hw_aclEntry_size>0)
			free_entry_number -= aclSWEntry->hw_aclEntry_size;
    }

	if(free_entry_number < needs_igr_entry_number)
	{
		ACL("Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}

	ACL("Leave %s \n",__func__);
	return RT_ERR_RG_OK;
}

int _rtk_rg_xdsl_search_acl_empty_Entry(int size, int* index)
{
    int i=0;
    int continue_size=0;
    rtl865x_AclRule_t aclRule;
    bzero(&aclRule, sizeof(aclRule));

    if(size == 0) //no need acl entry
    {
        return (RT_ERR_RG_OK);
    }
    for(i=MIN_ACL_ENTRY_INDEX; i<MAX_ACL_ENTRY_INDEX; i++)
    {
        if(_rtl865x_getAclFromAsic(i,&aclRule))
        {
            ACL("get acl[%d] failed",i);
            RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
        }
        if(aclRule.pktOpApp_ == 0)
        {

            continue_size++;
            if(continue_size == size)
            {
                *index = ((i+1) - size);
                ACL("get empty entry[%d] continue_size=%d index=%d",i,continue_size,*index);

                return (RT_ERR_RG_OK);
            }
            continue;
        }
        else
        {
            continue_size =0;
            continue;
        }
    }

    RETURN_ERR(RT_ERR_RG_ACL_ENTRY_FULL);
}

static rtl865x_AclRule_t aclIgrRule[MAX_ACL_TEMPLATE_SIZE];
static rtl865x_AclRule_t aclEgrRule[MAX_ACL_TEMPLATE_SIZE];

int _rtk_rg_aclSWEntry_and_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry)
{
    int i=0;
	int j=0;
	int ret;
    int acl_entry_index=0;
	int needs_igr_entry_number,needs_egr_entry_number;
	int acl_type_igr_bitmask,acl_type_egr_bitmask;


	ACL("Enter %s @ %d \n",__func__,__LINE__);
	_rtk_rg_xdsl_acl_resource_require(acl_filter,&acl_type_igr_bitmask,&acl_type_egr_bitmask,&needs_igr_entry_number,&needs_egr_entry_number);

	if(needs_igr_entry_number!=0 && needs_egr_entry_number!=0)
	{
		ACL("Count not support ingress acl & egress acl simultaneously! Leave %s @ %d\n",__func__,__LINE__);
		return (RT_ERR_RG_FAILED);
	}

#if !defined(CONFIG_RTL_8685S_HWNAT)
	if(needs_igr_entry_number>1 || needs_egr_entry_number>1)
	{
		printk("Could not support multiple ACL rules! Leave %s @ %d\n",__func__,__LINE__);
		return (RT_ERR_RG_OK);
	}
#endif

	//extport BC packet can be trapped originally, so support this kind of case but without add HW ACL rule. 
	if(acl_filter->action_type==ACL_ACTION_TYPE_TRAP || acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS)
	{
		if((acl_filter->filter_fields&INGRESS_DMAC_BIT) && 
			acl_filter->ingress_dmac.octet[0]==0xff	&&
			acl_filter->ingress_dmac.octet[1]==0xff	&&
			acl_filter->ingress_dmac.octet[2]==0xff	&&
			acl_filter->ingress_dmac.octet[3]==0xff	&&
			acl_filter->ingress_dmac.octet[4]==0xff	&&
			acl_filter->ingress_dmac.octet[5]==0xff	)
		{ 
			//do nothing , without add ACL. Just let BC packet trapped originally.
			ACL("Leave %s @ %d \n",__func__,__LINE__);
			return (RT_ERR_RG_OK);	
		}
	}

    bzero(aclIgrRule,sizeof(rtl865x_AclRule_t)*MAX_ACL_TEMPLATE_SIZE);

	/* Ingress ACL rule */
	j=0;
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_ETHERNET_BIT)
		aclIgrRule[j++].ruleType_ = RTL865X_ACL_MAC;
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_IPv4_BIT)
		aclIgrRule[j++].ruleType_ = RTL865X_ACL_IP_RANGE;
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_ICMP_BIT)
		aclIgrRule[j++].ruleType_ = RTL865X_ACL_ICMP_IPRANGE;
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_IGMP_BIT)
		aclIgrRule[j++].ruleType_ = RTL865X_ACL_IGMP_IPRANGE;
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_TCP_BIT)
		aclIgrRule[j++].ruleType_ = RTL865X_ACL_TCP_IPRANGE;
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_UDP_BIT)
		aclIgrRule[j++].ruleType_ = RTL865X_ACL_UDP_IPRANGE;
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_SRCFILTER_BIT)
		aclIgrRule[j++].ruleType_ = RTL865X_ACL_SRCFILTER;
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_DSTFILTER_BIT)
		aclIgrRule[j++].ruleType_ = RTL865X_ACL_DSTFILTER;
#ifdef CONFIG_RTL_8685S_HWNAT
	if(acl_type_igr_bitmask & xDSL_ACL_TYPE_IPv6_BIT)
	{
		if(needs_igr_entry_number<2)
			ACL("Error! Needs 2 ACL rule entry for IPv6! Leave %s @ %d\n",__func__,__LINE__);
		aclIgrRule[j].ruleType_ = RTL865X_ACL_IPV6_RANGE;
		aclIgrRule[j].ipv6ETY0_ = 1;
		aclIgrRule[j+1].ruleType_ = RTL865X_ACL_IPV6_RANGE;
		aclIgrRule[j+1].ipv6ETY0_ = 0;
		j+=2;
	}
#endif
	if(j!=needs_igr_entry_number)
	{
		ACL("Something error! Occupied ingress ACL entries is not equal to required ACL entries! Leave %s @ %d\n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}

	/* Egress ACL rule */
	j=0;
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_ETHERNET_BIT)
		aclEgrRule[j++].ruleType_ = RTL865X_ACL_MAC;
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_IPv4_BIT)
		aclEgrRule[j++].ruleType_ = RTL865X_ACL_IP_RANGE;
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_ICMP_BIT)
		aclEgrRule[j++].ruleType_ = RTL865X_ACL_ICMP_IPRANGE;
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_IGMP_BIT)
		aclEgrRule[j++].ruleType_ = RTL865X_ACL_IGMP_IPRANGE;
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_TCP_BIT)
		aclEgrRule[j++].ruleType_ = RTL865X_ACL_TCP_IPRANGE;
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_UDP_BIT)
		aclEgrRule[j++].ruleType_ = RTL865X_ACL_UDP_IPRANGE;
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_SRCFILTER_BIT)
		aclEgrRule[j++].ruleType_ = RTL865X_ACL_SRCFILTER;
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_DSTFILTER_BIT)
		aclEgrRule[j++].ruleType_ = RTL865X_ACL_DSTFILTER;
#ifdef CONFIG_RTL_8685S_HWNAT
	if(acl_type_egr_bitmask & xDSL_ACL_TYPE_IPv6_BIT)
	{
		if(needs_egr_entry_number<2)
			ACL("Error! Needs 2 ACL rule entry for IPv6! Leave %s @ %d\n",__func__,__LINE__);
		aclIgrRule[j].ruleType_ = RTL865X_ACL_IPV6_RANGE;
		aclIgrRule[j].ipv6ETY0_ = 1;
		aclIgrRule[j+1].ruleType_ = RTL865X_ACL_IPV6_RANGE;
		aclIgrRule[j+1].ipv6ETY0_ = 0;
		j+=2;
	}
#endif
	if(j!=needs_egr_entry_number)
	{
		ACL("Something error! Occupied egress ACL entries is not equal to required ACL entries! Leave %s @ %d\n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}

	for(i=0;i<needs_igr_entry_number;i++)
	{
		aclIgrRule[i].direction_ = RTL865X_ACL_INGRESS;
		aclIgrRule[i].pktOpApp_ = RTL865X_ACL_ALL_LAYER;
#if defined(CONFIG_RTL_8685S_HWNAT)
		if(i<(needs_igr_entry_number-1))
			aclIgrRule[i].comb = 1;
#endif
	}
	for(i=0;i<needs_egr_entry_number;i++)
	{
		aclEgrRule[i].direction_ = RTL865X_ACL_INGRESS;
		aclEgrRule[i].pktOpApp_ = RTL865X_ACL_ALL_LAYER;
#if defined(CONFIG_RTL_8685S_HWNAT)
		if(i<(needs_egr_entry_number-1))
			aclEgrRule[i].comb = 1;
#endif
	}

	/* Action */
	switch(acl_filter->action_type)
	{	
		case ACL_ACTION_TYPE_DROP:
			if(needs_igr_entry_number>0)
				aclIgrRule[needs_igr_entry_number-1].actionType_ = RTL865X_ACL_DROP;
			else
				aclEgrRule[needs_egr_entry_number-1].actionType_ = RTL865X_ACL_DROP;
			break;
		case ACL_ACTION_TYPE_TRAP:
		case ACL_ACTION_TYPE_TRAP_TO_PS:
			if(needs_igr_entry_number>0)
				aclIgrRule[needs_igr_entry_number-1].actionType_ = RTL865X_ACL_TOCPU;
			else
				aclEgrRule[needs_egr_entry_number-1].actionType_ = RTL865X_ACL_TOCPU;
			break;
		case ACL_ACTION_TYPE_PERMIT:
			if(needs_igr_entry_number>0)
				aclIgrRule[needs_igr_entry_number-1].actionType_ = RTL865X_ACL_PERMIT;
			else
				aclEgrRule[needs_egr_entry_number-1].actionType_ = RTL865X_ACL_PERMIT;
			break;
		case ACL_ACTION_TYPE_POLICY_ROUTE:
			break;
		case ACL_ACTION_TYPE_QOS:
			if(needs_igr_entry_number>0)
			{
				if(acl_filter->qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)
				{
					aclIgrRule[needs_igr_entry_number-1].actionType_ = RTL865X_ACL_PRIORITY;
					aclIgrRule[needs_igr_entry_number-1].priority_ = acl_filter->action_acl_priority;
				}
				else if(acl_filter->qos_actions & ACL_ACTION_ACL_INGRESS_VID_BIT)
				{
					aclIgrRule[needs_igr_entry_number-1].actionType_ = RTL865X_ACL_VID;
					aclIgrRule[needs_igr_entry_number-1].vid_ = acl_filter->action_acl_ingress_vid;
				}
			}
			else
			{
				if(acl_filter->qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)
				{
					aclEgrRule[needs_egr_entry_number-1].actionType_ = RTL865X_ACL_PRIORITY;
					aclIgrRule[needs_egr_entry_number-1].priority_ = acl_filter->action_acl_priority;
				}
				else if(acl_filter->qos_actions & ACL_ACTION_ACL_INGRESS_VID_BIT)
				{
					aclIgrRule[needs_egr_entry_number-1].actionType_ = RTL865X_ACL_VID;
					aclIgrRule[needs_egr_entry_number-1].vid_ = acl_filter->action_acl_ingress_vid;
				}
			}
			break;
		default:
			ACL("Action type 0x%x NOT support! Leave %s @ %d\n",acl_filter->action_type,__func__,__LINE__);
			return RT_ERR_RG_FAILED;
	}

    ACL("acl_filter->filter_fields = %ull",acl_filter->filter_fields);
    if(acl_filter->filter_fields & INGRESS_INTF_BIT)
    {
        ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
    }

    if(acl_filter->filter_fields & INGRESS_PORT_BIT)
    {
    	for(i=0;i<needs_igr_entry_number;i++)
    	{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER || aclIgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER_IPRANGE)
	    	{
		        aclIgrRule[i].un_ty.srcFilterPort_ =  acl_filter->ingress_port_idx;
				aclIgrRule[i].un_ty.srcFilterPortMask_ = acl_filter->ingress_port_idx;
				aclIgrRule[i].un_ty.SRCFILTER._ignoreL3L4 = 1;
				break;
	    	}
    	}
	}

    if(acl_filter->filter_fields & EGRESS_INTF_BIT)
    {
        ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
    }
	
	if(acl_filter->filter_fields & INGRESS_STREAM_ID_BIT)
	{
		ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT)
	{
		for(i=0;i<needs_igr_entry_number;i++)
		{
			if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP)
			{
				//No pattern
				if(acl_filter->ingress_ipv4_tagif==0)//Must not be IPv4
					aclIgrRule[i].inv_flag = 1;
				break;
			}
		}
	}

#if defined(CONFIG_RTL_8685S_HWNAT)
	if(acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)
	{
		for(i=0;i<needs_igr_entry_number;i++)
		{
			if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6)
			{
				//No pattern
				if(aclIgrRule[i].ipv6ETY0_ == 1)
				{
					if(acl_filter->ingress_ipv6_tagif==0)//Must not be IPv4
						aclIgrRule[i].inv_flag = 1;
					break;
				}
			}
		}
	}
#endif
	
    if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
    {
 		for(i=0;i<needs_igr_entry_number;i++)
		{
			if(aclIgrRule[i].ruleType_ == RTL865X_ACL_MAC)
			{
				aclIgrRule[i].un_ty.MAC._typeLen = acl_filter->ingress_ethertype;
				aclIgrRule[i].un_ty.MAC._typeLenMask = acl_filter->ingress_ethertype_mask;
				break;
				break;
			}
		}
	}
    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
    {
 		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER || aclIgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER_IPRANGE)
			{
				aclIgrRule[i].un_ty.SRCFILTER._srcVlanIdx = acl_filter->ingress_ctag_vid;
				aclIgrRule[i].un_ty.SRCFILTER._srcVlanIdxMask = 0xffff;
				aclIgrRule[i].un_ty.SRCFILTER._ignoreL3L4 = 1;
				break;
			}
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_MAC)
			{
				aclIgrRule[i].un_ty.MAC._typeLen = 0x8100;
				aclIgrRule[i].un_ty.MAC._typeLenMask = 0xffff;
				break;
			}
		}
    }
    if((acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT) || (acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT))
    {
        ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
    }

	if((acl_filter->filter_fields & INGRESS_STAG_VID_BIT) || (acl_filter->filter_fields & INGRESS_STAG_PRI_BIT)
		|| (acl_filter->filter_fields & INGRESS_STAG_DEI_BIT))
	{
        ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
 	}
	if(acl_filter->filter_fields & INGRESS_STAGIF_BIT)
	{
        ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}
	
	if(acl_filter->filter_fields & INGRESS_CTAGIF_BIT)
	{
		for(i=0;i<needs_igr_entry_number;i++)
		{
			if(aclIgrRule[i].ruleType_ == RTL865X_ACL_MAC)
			{
				aclIgrRule[i].un_ty.MAC._typeLen = 0x8100;
				aclIgrRule[i].un_ty.MAC._typeLenMask = 0xffff;
				aclIgrRule[i].un_ty.SRCFILTER._ignoreL3L4 = 1;
				break;
				break;
			}
		}
	}

	if(acl_filter->filter_fields & INTERNAL_PRI_BIT)
	{
        ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}

	if(acl_filter->filter_fields & EGRESS_CTAG_PRI_BIT)
	{
        ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}
	
	if(acl_filter->filter_fields & EGRESS_CTAG_VID_BIT)
	{
		for(i=0;i<needs_egr_entry_number;i++)
		{
	    	if(aclEgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER || aclEgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER_IPRANGE)
			{
				aclEgrRule[i].un_ty.SRCFILTER._srcVlanIdx = acl_filter->ingress_ctag_vid;
				aclEgrRule[i].un_ty.SRCFILTER._srcVlanIdxMask = 0xffff;
				aclEgrRule[i].un_ty.SRCFILTER._ignoreL3L4 = 1;
				break;
			}
	    	if(aclEgrRule[i].ruleType_ == RTL865X_ACL_MAC)
			{
				aclEgrRule[i].un_ty.MAC._typeLen = 0x8100;
				aclEgrRule[i].un_ty.MAC._typeLenMask = 0xffff;
				break;
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_EGRESS_PORTIDX_BIT)
	{
        ACL("NOT support - INGRESS_INTF_BIT. Leave %s @ %d \n",__func__,__LINE__);
		return RT_ERR_RG_FAILED;
	}

    if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER || aclIgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER_IPRANGE)
			{
				memcpy(&aclIgrRule[i].un_ty.SRCFILTER._srcMac.octet[0],&acl_filter->ingress_smac.octet[0],ETH_ALEN);
				memcpy(&aclIgrRule[i].un_ty.SRCFILTER._srcMacMask.octet[0],&acl_filter->ingress_smac_mask.octet[0],ETH_ALEN);
				aclIgrRule[i].un_ty.SRCFILTER._ignoreL3L4 = 1;
				break;
			}
		}
    }
	
    if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_DSTFILTER || aclIgrRule[i].ruleType_ == RTL865X_ACL_DSTFILTER_IPRANGE)
			{
				memcpy(&aclIgrRule[i].un_ty.DSTFILTER._dstMac.octet[0],&acl_filter->ingress_dmac.octet[0],ETH_ALEN);
				memcpy(&aclIgrRule[i].un_ty.DSTFILTER._dstMacMask.octet[0],&acl_filter->ingress_dmac_mask.octet[0],ETH_ALEN);
				aclIgrRule[i].un_ty.DSTFILTER._ignoreL3L4 = 1;
				break;
			}
		}
    }
	
    if(acl_filter->filter_fields & INGRESS_DSCP_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP || aclIgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclIgrRule[i].un_ty.L3L4._tos = acl_filter->ingress_dscp<<2;
				aclIgrRule[i].un_ty.L3L4._tosMask = 0x3f<<2;
				break;
			}
		}
	}
	if(acl_filter->filter_fields & INGRESS_TOS_BIT) 
	{      
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP || aclIgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclIgrRule[i].un_ty.L3L4._tos = acl_filter->ingress_tos;
				aclIgrRule[i].un_ty.L3L4._tosMask = 0xff;
				break;
			}
		}
	}
	
#if defined(CONFIG_RTL_8685S_HWNAT)
	if(acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT)
	{
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6 || aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6_RANGE)
			{
				if(aclIgrRule[i].ipv6ETY0_ == 0)
				{
					aclIgrRule[i].un_ty.V6ETY1._ipv6TrafficClass = acl_filter->ingress_ipv6_dscp<<2;
					aclIgrRule[i].un_ty.V6ETY1._ipv6TrafficClassM = 0x3f<<2;
					break;
				}
			}
		}
	}
	
	if(acl_filter->filter_fields & INGRESS_IPV6_TC_BIT)
	{
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6 || aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6_RANGE)
			{
				if(aclIgrRule[i].ipv6ETY0_ == 0)
				{
					aclIgrRule[i].un_ty.V6ETY1._ipv6TrafficClass = acl_filter->ingress_ipv6_tc;
					aclIgrRule[i].un_ty.V6ETY1._ipv6TrafficClassM = 0xff;
					break;
				}
			}
		}
	}
#endif
    if(acl_filter->filter_fields & INGRESS_L4_TCP_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP || aclIgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclIgrRule[i].un_ty.L3L4.is.ip._proto = 6;
				aclIgrRule[i].un_ty.L3L4._tosMask = 0xff;
				break;
			}
			else if(aclIgrRule[i].ruleType_ == RTL865X_ACL_TCP || aclIgrRule[i].ruleType_ == RTL865X_ACL_TCP_IPRANGE)
			{
				aclIgrRule[i].un_ty.L3L4.is.tcp._srcPortLowerBound = acl_filter->ingress_src_l4_port_start;
				aclIgrRule[i].un_ty.L3L4.is.tcp._srcPortUpperBound = acl_filter->ingress_src_l4_port_end;
				aclIgrRule[i].un_ty.L3L4.is.tcp._dstPortLowerBound = acl_filter->ingress_dest_l4_port_start;
				aclIgrRule[i].un_ty.L3L4.is.tcp._dstPortUpperBound = acl_filter->ingress_dest_l4_port_end;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & INGRESS_L4_UDP_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP || aclIgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclIgrRule[i].un_ty.L3L4.is.ip._proto = 17;
				aclIgrRule[i].un_ty.L3L4._tosMask = 0xff;
				break;
			}
			else if(aclIgrRule[i].ruleType_ == RTL865X_ACL_UDP || aclIgrRule[i].ruleType_ == RTL865X_ACL_UDP_IPRANGE)
			{
				aclIgrRule[i].un_ty.L3L4.is.udp._srcPortLowerBound = acl_filter->ingress_src_l4_port_start;
				aclIgrRule[i].un_ty.L3L4.is.udp._srcPortUpperBound = acl_filter->ingress_src_l4_port_end;
				aclIgrRule[i].un_ty.L3L4.is.udp._dstPortLowerBound = acl_filter->ingress_dest_l4_port_start;
				aclIgrRule[i].un_ty.L3L4.is.udp._dstPortUpperBound = acl_filter->ingress_dest_l4_port_end;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & INGRESS_L4_ICMP_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP || aclIgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclIgrRule[i].un_ty.L3L4.is.ip._proto = 2;
				aclIgrRule[i].un_ty.L3L4._tosMask = 0xff;
				break;
			}
		}
    }
	
#if defined(CONFIG_RTL_8685S_HWNAT)
    if(acl_filter->filter_fields & INGRESS_L4_ICMPV6_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6 || aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6_RANGE)
			{
				if(aclIgrRule[i].ipv6ETY0_ == 0)
				{
					aclIgrRule[i].un_ty.V6ETY1._ipv6NextHeader = 58;
					aclIgrRule[i].un_ty.V6ETY1._ipv6NextHeaderM = 0xff;
					break;
				}
			}
		}
    }
#endif

	if(acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP || aclIgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclIgrRule[i].un_ty.L3L4.is.ip._proto = acl_filter->ingress_l4_protocal;
				aclIgrRule[i].un_ty.L3L4.is.ip._protoMask = 0xff;
				break;
			}
		}
	}
		
#if defined(CONFIG_RTL_8685S_HWNAT)
	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)
	{	
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6 || aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6_RANGE)
			{
				if(aclIgrRule[i].ipv6ETY0_ == 0)
				{
					aclIgrRule[i].ruleType_ = RTL865X_ACL_IPV6_RANGE;
					memcpy(&aclIgrRule[i].un_ty.dstIpV6Addr_.in6_u.u6_addr8[0],&acl_filter->ingress_dest_ipv6_addr_end[0],16);
					memcpy(&aclIgrRule[i].un_ty.dstIpV6AddrMask_.in6_u.u6_addr8[0],&acl_filter->ingress_dest_ipv6_addr_start[0],16);
					break;
				}
			}
		}
	}
	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT)
	{	
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6 || aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6_RANGE)
			{
				if(aclIgrRule[i].ipv6ETY0_ == 0)
				{
					aclIgrRule[i].ruleType_ = RTL865X_ACL_IPV6;
					memcpy(&aclIgrRule[i].un_ty.dstIpV6Addr_.in6_u.u6_addr8[0],&acl_filter->ingress_dest_ipv6_addr[0],16);
					memset(&aclIgrRule[i].un_ty.dstIpV6AddrMask_.in6_u.u6_addr8[0],0xff,16);
					break;
				}
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)
	{
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6 || aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6_RANGE)
			{
				if(aclIgrRule[i].ipv6ETY0_ == 1)
				{
					aclIgrRule[i].ruleType_ = RTL865X_ACL_IPV6_RANGE;
					memcpy(&aclIgrRule[i].un_ty.srcIpV6Addr_.in6_u.u6_addr8[0],&acl_filter->ingress_src_ipv6_addr_end[0],16);
					memcpy(&aclIgrRule[i].un_ty.srcIpV6AddrMask_.in6_u.u6_addr8[0],&acl_filter->ingress_src_ipv6_addr_start[0],16);
					break;
				}
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT)
	{
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6 || aclIgrRule[i].ruleType_ == RTL865X_ACL_IPV6_RANGE)
			{
				if(aclIgrRule[i].ipv6ETY0_ == 1)
				{
					aclIgrRule[i].ruleType_ = RTL865X_ACL_IPV6;
					memcpy(&aclIgrRule[i].un_ty.srcIpV6Addr_.in6_u.u6_addr8[0],&acl_filter->ingress_src_ipv6_addr[0],16);
					memset(&aclIgrRule[i].un_ty.srcIpV6AddrMask_.in6_u.u6_addr8[0],0xff,16);
					break;
				}
			}
		}
	}
#endif

    if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP || aclIgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclIgrRule[i].ruleType_ = RTL865X_ACL_IP_RANGE;
				aclIgrRule[i].un_ty.srcIpAddr_ = acl_filter->ingress_src_ipv4_addr_end;
				aclIgrRule[i].un_ty.srcIpAddrMask_ = acl_filter->ingress_src_ipv4_addr_start;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_IP || aclIgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclIgrRule[i].ruleType_ = RTL865X_ACL_IP_RANGE;
				aclIgrRule[i].un_ty.dstIpAddr_ = acl_filter->ingress_dest_ipv4_addr_end;
				aclIgrRule[i].un_ty.dstIpAddrMask_ = acl_filter->ingress_dest_ipv4_addr_start;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER || aclIgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER_IPRANGE)
			{
				aclIgrRule[i].un_ty.SRCFILTER._srcPortLowerBound = acl_filter->ingress_src_l4_port_start;
				aclIgrRule[i].un_ty.SRCFILTER._srcPortUpperBound = acl_filter->ingress_src_l4_port_end;
				aclIgrRule[i].un_ty.SRCFILTER._ignoreL3L4 = 0;
				aclIgrRule[i].un_ty.SRCFILTER._ignoreL4 = 0;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)
    {
		for(i=0;i<needs_igr_entry_number;i++)
		{
	    	if(aclIgrRule[i].ruleType_ == RTL865X_ACL_DSTFILTER || aclIgrRule[i].ruleType_ == RTL865X_ACL_DSTFILTER_IPRANGE)
			{
				aclIgrRule[i].un_ty.DSTFILTER._dstPortLowerBound = acl_filter->ingress_dest_l4_port_start;
				aclIgrRule[i].un_ty.DSTFILTER._dstPortUpperBound = acl_filter->ingress_dest_l4_port_end;
				aclIgrRule[i].un_ty.DSTFILTER._ignoreL3L4 = 0;
				aclIgrRule[i].un_ty.DSTFILTER._ignoreL4 = 0;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)
    {
		for(i=0;i<needs_egr_entry_number;i++)
		{
	    	if(aclEgrRule[i].ruleType_ == RTL865X_ACL_IP || aclEgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclEgrRule[i].ruleType_ = RTL865X_ACL_IP_RANGE;
				aclEgrRule[i].un_ty.srcIpAddr_ = acl_filter->egress_src_ipv4_addr_end;
				aclEgrRule[i].un_ty.srcIpAddrMask_ = acl_filter->egress_src_ipv4_addr_start;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & EGRESS_IPV4_DIP_RANGE_BIT)
    {
		for(i=0;i<needs_egr_entry_number;i++)
		{
	    	if(aclEgrRule[i].ruleType_ == RTL865X_ACL_IP || aclEgrRule[i].ruleType_ == RTL865X_ACL_IP_RANGE)
			{
				aclEgrRule[i].ruleType_ = RTL865X_ACL_IP_RANGE;
				aclEgrRule[i].un_ty.dstIpAddr_ = acl_filter->egress_dest_ipv4_addr_end;
				aclEgrRule[i].un_ty.dstIpAddrMask_ = acl_filter->egress_dest_ipv4_addr_start;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & EGRESS_L4_SPORT_RANGE_BIT)
    {
		for(i=0;i<needs_egr_entry_number;i++)
		{
	    	if(aclEgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER || aclEgrRule[i].ruleType_ == RTL865X_ACL_SRCFILTER_IPRANGE)
			{
				aclEgrRule[i].un_ty.SRCFILTER._srcPortLowerBound = acl_filter->egress_src_l4_port_start;
				aclEgrRule[i].un_ty.SRCFILTER._srcPortUpperBound = acl_filter->egress_src_l4_port_end;
				aclEgrRule[i].un_ty.SRCFILTER._ignoreL3L4 = 0;
				aclEgrRule[i].un_ty.SRCFILTER._ignoreL4 = 0;
				break;
			}
		}
    }
    if(acl_filter->filter_fields & EGRESS_L4_DPORT_RANGE_BIT)
    {
		for(i=0;i<needs_egr_entry_number;i++)
		{
	    	if(aclEgrRule[i].ruleType_ == RTL865X_ACL_DSTFILTER || aclEgrRule[i].ruleType_ == RTL865X_ACL_DSTFILTER_IPRANGE)
			{
				aclEgrRule[i].un_ty.DSTFILTER._dstPortLowerBound = acl_filter->egress_dest_l4_port_start;
				aclEgrRule[i].un_ty.DSTFILTER._dstPortUpperBound = acl_filter->egress_dest_l4_port_end;
				aclEgrRule[i].un_ty.DSTFILTER._ignoreL3L4 = 0;
				aclEgrRule[i].un_ty.DSTFILTER._ignoreL4 = 0;
				break;
			}
		}
    }

    //setup ACL ASIC
    ASSERT_EQ(_rtk_rg_xdsl_search_acl_empty_Entry(needs_igr_entry_number, &acl_entry_index),RT_ERR_RG_OK);
    //setup aclSWEntry
    aclSWEntry->hw_aclEntry_start = acl_entry_index;
    aclSWEntry->hw_aclEntry_size = needs_igr_entry_number;
    ACL("got empty start aclEntry[%d] for %d rules",acl_entry_index,needs_igr_entry_number);

    for(i=0; i<needs_igr_entry_number; i++)
    {
		ret = _rtl865x_setAclToAsic(acl_entry_index+i,&aclIgrRule[i]);
        if(ret)
        {
            ACL("adding aclRule[%d] to aclEntry[%d] failed. ret=0x%x",i,acl_entry_index,ret);
            RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
        }
    }

    for(i=0; i<needs_egr_entry_number; i++)
    {
		ret = _rtl865x_setAclToAsic(acl_entry_index+i,&aclEgrRule[i]);
        if(ret)
        {
            ACL("adding aclRule[%d] to aclEntry[%d] failed. ret=0x%x",i,acl_entry_index,ret);
            RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
        }
    }

	ACL("Leave %s [Success]!\n",__func__);
    return RT_ERR_RG_OK;
}


int _rtk_rg_xdsl_aclSWEntry_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
    rtk_rg_aclFilterEntry_t aclSWEntry,empty_aclSWEntry;
    bzero(&aclSWEntry,sizeof(aclSWEntry));
    bzero(&empty_aclSWEntry,sizeof(empty_aclSWEntry));

    //check input parameter
    if(acl_filter == NULL
            || acl_filter_idx==NULL)
        RETURN_ERR(RT_ERR_RG_NULL_POINTER);

    if(acl_filter->filter_fields == 0x0)
        RETURN_ERR(RT_ERR_RG_INITPM_UNINIT);

        ACL("reAdd RG_ACL[%d]",*acl_filter_idx);

    //check the aclSWEntry has been used, reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_rg_aclSWEntry_get(*acl_filter_idx, &aclSWEntry),RT_ERR_RG_OK);

    if(memcmp(&aclSWEntry,&empty_aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t)))
    {
        RETURN_ERR(RT_ERR_RG_ACL_SW_ENTRY_USED);
    }


    //parse acl_filter field to setup aclSWEntry & ASIC
    
    if(acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
    {
    //skip add to HWNAT
    }
    else if(acl_filter->filter_fields & INGRESS_WLANDEV_BIT)
    {
            //skip add to HWNAT
            rg_db.systemGlobal.wlanDevPatternValidInACL = 1;
    }
    else
    {
          ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_add(acl_filter,&aclSWEntry),RT_ERR_RG_OK);
    }
    aclSWEntry.acl_filter = *acl_filter;
    aclSWEntry.valid = RTK_RG_ENABLED;
	aclSWEntry.type = ACL_USE;


    //reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_rg_aclSWEntry_set(*acl_filter_idx,aclSWEntry),RT_ERR_RG_OK);

    //_rtk_rg_aclSWEntry_dump();

    return (RT_ERR_RG_OK);
}

static int _rtk_rg_rearrange_ACL_weight(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	rtk_rg_aclFilterAndQos_t empty_aclFilter;
    bzero(&empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));
	
	//1-1. clean the sorting record of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	*accumulateIdx = 0;
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){	
		rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]=-1;
		rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action[j]=-1;
	}

	//1-2. record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
    for(i=0, j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        if(memcmp(&rg_db.systemGlobal.acl_filter_temp[i],&empty_aclFilter,sizeof(empty_aclFilter)))
        {
        	if(rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
            	//ASSERT_EQ(_rtk_rg_aclSWEntry_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = i; //record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
				j++;
				*accumulateIdx=j;
        	}
		}
    }

	//1-3. sorting the rule by weight: BubSort	
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>0; i--){
		sp=1;
	   	for (j=0; j<=i; j++){
			//check data is valid
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;
			
		  	if (rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight <
				rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight) //compare the weight
		 	{	
		 		//swap the rule index
			   	temp = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
			   	rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1];
			   	rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1] = temp;
			   	sp=0;
		 	}
	   	}
		if (sp==1) //if no swap happened, then no need to check the lower array index(they are already sorted).
			break;			
	}

	return RT_ERR_RG_OK;
}

int _rtk_rg_aclSWEntry_and_asic_rearrange(void)
{
	int i,j,accumulateIdx,ruleIdx;
	rtk_rg_aclFilterEntry_t aclSWEntry;
	rtk_rg_aclFilterAndQos_t empty_aclFilter;
	bzero(&empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));
	bzero(rg_db.systemGlobal.acl_filter_temp, sizeof(rtk_rg_aclFilterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);

	ACL("Enter %s @ %d \n",__func__,__LINE__);

	//backup all acl_filter for reAdd, and clean all aclSWEntry
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		//backup acl_filter
		ASSERT_EQ(_rtk_rg_aclSWEntry_get(i,&aclSWEntry),RT_ERR_RG_OK);
		rg_db.systemGlobal.acl_filter_temp[i] = aclSWEntry.acl_filter;
		//clean aclSWEntry
		bzero(&aclSWEntry, sizeof(rtk_rg_aclFilterEntry_t));
		ASSERT_EQ(_rtk_rg_aclSWEntry_set(i,aclSWEntry),RT_ERR_RG_OK);
	}

	//delete ingress ACL tables
	for(i=MIN_ACL_IGR_ENTRY_INDEX; i<MAX_ACL_IGR_ENTRY_INDEX; i++)
	{
		if(rtk_acl_igrRuleEntry_del(i))
		{
			RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
		}
	}

	//delete egress ACL tables
	for(i=MIN_ACL_EGR_ENTRY_INDEX; i<MAX_ACL_EGR_ENTRY_INDEX; i++)
	{
		if(rtk_classify_cfgEntry_del(i)) //xDSL support egress ACL instread of CF. We just use rtk_acl_igrRuleEntry_del() to del egress ACL entries.
		{
			RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
		}
	}

	//Follow APOLLO implementation
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);

	//re-add
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){	
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
			break;
		ASSERT_EQ(_rtk_rg_xdsl_aclSWEntry_reAdd(&rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])], &rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]),RT_ERR_RG_OK);
	}
	
	//record acl ingress_vid action valid rules for speed up internalVID decision in fwdEngine. (only type of ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET can use this action)
	for(i=0,j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)//no more rules need to check
			break;
		
		ruleIdx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];	
		if( (rg_db.systemGlobal.acl_filter_temp[ruleIdx].action_type==ACL_ACTION_TYPE_QOS) &&
			(rg_db.systemGlobal.acl_filter_temp[ruleIdx].qos_actions&ACL_ACTION_ACL_INGRESS_VID_BIT) ){
			rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action[j]=ruleIdx;
			j++;
		}
	}

	return (RT_ERR_RG_OK);
}


/*(3)literomeDriver mapping APIs*/
int32 _rtk_rg_apollo_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int ret;
	int acl_SWEntry_index=0;
	rtk_rg_aclFilterEntry_t aclSWEntry,empty_aclSWEntry;
	bzero(&aclSWEntry,sizeof(aclSWEntry));
	bzero(&empty_aclSWEntry,sizeof(empty_aclSWEntry));

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);

	//check input parameter
	if(acl_filter == NULL || acl_filter_idx == NULL)
		RETURN_ERR(RT_ERR_RG_NULL_POINTER);

	if(acl_filter->filter_fields == 0x0)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);

	if(acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	{
		DEBUG("Policy Route to WAN%d! do not add to ASIC!!",acl_filter->action_policy_route_wan);

	}
	else if(acl_filter->filter_fields & INGRESS_WLANDEV_BIT)
	{
		rg_db.systemGlobal.wlanDevPatternValidInACL = 1;//it will skip HWLOOKUP
		DEBUG("wlanDev pattern vlaid! do not rearrange ASIC!!");
	}
	else
	{
	
		ret=_rtk_rg_aclSWEntry_and_asic_chkReource(acl_filter,&aclSWEntry);
		if(ret!=RT_ERR_RG_OK) goto aclFailed;
		
		aclSWEntry.type = ACL_USE;
	}
	aclSWEntry.acl_filter = *acl_filter;
	aclSWEntry.valid = RTK_RG_ENABLED;

	ret=_rtk_rg_aclSWEntry_empty_find(&acl_SWEntry_index);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	ret=_rtk_rg_aclSWEntry_set(acl_SWEntry_index,aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;
	
	*acl_filter_idx = acl_SWEntry_index;

	//assert_ok(_rtk_rg_cvidCpri2Sidmapping_refresh()); //handel cvid, cpri => sid mapping table

	
	rg_db.systemGlobal.acl_SW_table_entry_size++;
	DEBUG("add aclSWEntry[%d]",*acl_filter_idx);

	if(acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	{
		
		DEBUG("Policy Route to WAN%d! do not rearrange ASIC!!",acl_filter->action_policy_route_wan);
	}
	else if(acl_filter->filter_fields & INGRESS_WLANDEV_BIT)
	{
		rg_db.systemGlobal.wlanDevPatternValidInACL = 1;//it will skip HWLOOKUP
		DEBUG("wlanDev pattern vlaid! do not rearrange ASIC!!");
	}

	//readd all acl rules for sorting: action  INGRESS_ALL(sort by weight) > L34 DROP > L34 SID remarking
	ret = _rtk_rg_aclSWEntry_and_asic_rearrange();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	
	ret = _rtk_rg_shortCut_clear();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	return (RT_ERR_RG_OK);

aclFailed:
	return ret;

}

static int _rtk_rg_aclSWEntry_to_asic_free(rtk_rg_aclFilterEntry_t aclSWEntry)
{
    int i,ret;
    int acl_start,acl_size,cf_start, cf_size;
    rtk_rg_aclFilterEntry_t empty_aclSWEntry;
    bzero(&empty_aclSWEntry,sizeof(empty_aclSWEntry));

    //delete acl&cf  tables
    //delete acl&cf Asic
    acl_start = aclSWEntry.hw_aclEntry_start;
    acl_size = aclSWEntry.hw_aclEntry_size;
    cf_start = aclSWEntry.hw_cfEntry_start;
    cf_size = aclSWEntry.hw_cfEntry_size;
    for(i=0; i<acl_size; i++)
    {
    	ret = rtk_acl_igrRuleEntry_del(acl_start+i);
        if(ret!=RT_ERR_RG_OK)
        {
        	DEBUG("free acl ASIC[%d] failed, ret(rtk)=%d",(acl_start+i),ret);
            RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
        }
    }

    for(i=0; i<cf_size; i++)
    {
    	ret = rtk_classify_cfgEntry_del(cf_start+i);
        if(ret!=RT_ERR_RG_OK)
        {
        	DEBUG("free cf ASIC[%d] failed, ret(rtk)=%d",(cf_size+i),ret);
            RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
        }
    }

    return (RT_ERR_RG_OK);
}

int32 _rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx)
{
	int ret;
	rtk_rg_aclFilterEntry_t aclSWEntry,empty_aclSWEntry;
	bzero(&aclSWEntry,sizeof(aclSWEntry));
	bzero(&empty_aclSWEntry,sizeof(empty_aclSWEntry));

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);

	ret=_rtk_rg_aclSWEntry_get(acl_filter_idx, &aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;
	
	ret = _rtk_rg_aclSWEntry_to_asic_free(aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//clean aclSWEntry
	ret = _rtk_rg_aclSWEntry_set(acl_filter_idx, empty_aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//rearrange the ACL & CF ASIC to avoid discontinuous entry
	ret = _rtk_rg_aclSWEntry_and_asic_rearrange();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	rg_db.systemGlobal.acl_SW_table_entry_size--;
	DEBUG("delete aclSWEntry[%d]",acl_filter_idx);
	ret = _rtk_rg_shortCut_clear();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;
	
	//_rtk_rg_aclSWEntry_dump();
	
	return (RT_ERR_RG_OK);

aclFailed:
	return ret;
	
}


int32 _rtk_rg_apollo_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx)
{
	//search for the first not empty entry after valid_idx.
	int i;
	rtk_rg_aclFilterEntry_t aclSWEntry, empty_aclSWEntry;

	bzero(&aclSWEntry, sizeof(aclSWEntry));
	bzero(&empty_aclSWEntry, sizeof(empty_aclSWEntry));

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);


	if(*valid_idx==-1){
		for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++){
			ASSERT_EQ(_rtk_rg_aclSWEntry_get(i,&aclSWEntry),RT_ERR_RG_OK);
			if(!memcmp(&(aclSWEntry.acl_filter),acl_filter,sizeof(rtk_rg_aclFilterAndQos_t)))//search the same with acl_filter
			{
				*valid_idx=i;
				*acl_filter = aclSWEntry.acl_filter;
				DEBUG("rtk_rg_aclFilterAndQos_find index=%d",*valid_idx);
				return (RT_ERR_RG_OK);
			}

		}
	}else{
		for(i=*valid_idx; i<MAX_ACL_SW_ENTRY_SIZE; i++)
		{
			ASSERT_EQ(_rtk_rg_aclSWEntry_get(i,&aclSWEntry),RT_ERR_RG_OK);

			if(memcmp(&aclSWEntry,&empty_aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t)))//search the different with empty_aclSWEntry
			{
				*valid_idx=i;
				*acl_filter = aclSWEntry.acl_filter;
				DEBUG("rtk_rg_aclFilterAndQos_find index=%d",*valid_idx);
				return (RT_ERR_RG_OK);
			}
		}
		DEBUG("rtk_rg_aclFilterAndQos_find failed");
	}
	//not found
	return (RT_ERR_RG_ACL_SW_ENTRY_NOT_FOUND);
}


int32 _rtk_rg_apollo_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter)
{
	FIXME("XDSL FIXME _rtk_rg_apollo_classifyEntry_add latter\n");
	return (RT_ERR_RG_OK);
}

int32 _rtk_rg_apollo_classifyEntry_del(int index)
{
	FIXME("XDSL FIXME _rtk_rg_apollo_classifyEntry_del latter\n");
	return (RT_ERR_RG_OK);
}

int32 _rtk_rg_apollo_classifyEntry_find(int index, rtk_rg_classifyEntry_t *classifyFilter)
{
	FIXME("XDSL FIXME _rtk_rg_apollo_classifyEntry_find latter\n");
	return (RT_ERR_RG_OK);
}


/*(4)reserved ACL related APIs*/
int _rtk_rg_aclAndCfReservedRuleHeadReflash(void)
{
	uint32 type;
	uint32 aclIdx=0;
	uint32 cfIdx=0;
	int addRuleFailedFlag=0;
	
	rtl865x_AclRule_t aclRule;

	//reflash th rules
	uint32 i;
	for(i=0;i<rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(i));
	}
	for(i=0;i<rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary;i++){
		assert_ok(rtk_classify_cfgEntry_del(i));
	}

	for(type=0;type<RTK_RG_ACLANDCF_RESERVED_HEAD_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){
				case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_ALL_TRAP @ acl[%d]",aclIdx);						
						bzero(&aclRule,sizeof(aclRule));

						/* Ingress ACL */
						aclRule.actionType_ = RTL865X_ACL_TOCPU;
						aclRule.direction_ = RTL865X_ACL_INGRESS;
						aclRule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
						aclRule.ruleType_ = RTL865X_ACL_MAC;
						if(_rtl865x_setAclToAsic(aclIdx,&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ALL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;
						
						/* Egress ACL */
						aclRule.actionType_ = RTL865X_ACL_TOCPU;
						aclRule.direction_ = RTL865X_ACL_INGRESS;
						aclRule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
						aclRule.ruleType_ = RTL865X_ACL_MAC;
						if(_rtl865x_setAclToAsic(cfIdx,&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ALL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						cfIdx++;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP:
					{
						char bc_mac[ETHER_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP @ acl[%d]",aclIdx);						
						bzero(&aclRule,sizeof(aclRule));
				
						/* Ingress ACL */
						memset(&aclRule, 0,sizeof(rtl865x_AclRule_t));	
						aclRule.ruleType_ = RTL865X_ACL_MAC;
						aclRule.actionType_ = RTL865X_ACL_PERMIT;
						aclRule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
						aclRule.direction_ = RTL865X_ACL_INGRESS;
						memcpy(aclRule.un_ty.dstMac_.octet,bc_mac,ETHER_ADDR_LEN);
						memcpy(aclRule.un_ty.dstMacMask_.octet,bc_mac,ETHER_ADDR_LEN);

						if(_rtl865x_setAclToAsic(aclIdx,&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;
					}
					break;
				default:
					break;
			}
			
		}
	}
	
	rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary=aclIdx;
	rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary=cfIdx;

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	//reflash user ACL
//	ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_rearrange(),RT_ERR_RG_OK);
	

	return (RT_ERR_RG_OK);
}

int _rtk_rg_aclAndCfReservedRuleTailReflash(void)
{
	uint32 type;
	uint32 aclIdx, cfIdx;
	//rule used varibles	
	rtl865x_AclRule_t aclRule;

	int addRuleFailedFlag=0;

	//reflash th rules
	uint32 i;
	for(i=0;i<xDSL_ACL_IGR_RSVD_TAIL_SIZE;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary+i));
	}
	for(i=0;i<xDSL_ACL_EGR_RSVD_TAIL_SIZE;i++){
		assert_ok(rtk_classify_cfgEntry_del(rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary+i));
	}

	aclIdx = MAX_ACL_IGR_ENTRY_SIZE - 1;
	cfIdx = MAX_ACL_EGR_ENTRY_SIZE - 1;
	for(type=RTK_RG_ACLANDCF_RESERVED_HEAD_END;type<RTK_RG_ACLANDCF_RESERVED_TAIL_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){				
				case RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT @ acl[%d]",aclIdx);
					bzero(&aclRule,sizeof(aclRule));

					/* Ingress ACL */
					aclRule.actionType_ = RTL865X_ACL_PERMIT;
					aclRule.direction_ = RTL865X_ACL_INGRESS;
					aclRule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
					aclRule.ruleType_ = RTL865X_ACL_MAC;
					if(_rtl865x_setAclToAsic(aclIdx,&aclRule))
					{
						ACL_RSV("add reserved ingress RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					//point to next ruleIdx
					aclIdx--;

					/* Egress ACL */
					if(_rtl865x_setAclToAsic(cfIdx+MAX_ACL_IGR_ENTRY_SIZE,&aclRule))
					{
						ACL_RSV("add reserved egress ACL RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					//point to next ruleIdx
					cfIdx--;
					break;
					
				default:
					break;
			}
		}
	}

	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary=aclIdx;
	rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary=cfIdx;

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	return (RT_ERR_RG_OK);

}

int _rtk_rg_aclAndCfReservedRuleAddCheck(uint32 aclRsvSize, uint32 cfRsvSize)
{
	uint32 i;
	rtl865x_AclRule_t aclRule;

	//check rest empty acl rules in enough
	bzero(&aclRule,sizeof(aclRule));
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary;i>(rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary-aclRsvSize);i--){
		assert_ok(_rtl865x_getAclFromAsic(i,&aclRule));
		if(aclRule.pktOpApp_!=0){
			WARNING("ACL rest rules for reserved is not enough! i=%d aclUpperBoundary=%d aclRsvSize=%d",i,rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary,aclRsvSize);
			return (RT_ERR_RG_FAILED);
		}
	}

	//check rest empty cf rules in enough
	bzero(&aclRule,sizeof(aclRule));
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary;i>(rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary-cfRsvSize);i--){
		assert_ok(_rtl865x_getAclFromAsic(i,&aclRule));
		if(aclRule.pktOpApp_!=0){
			WARNING("CF rest rules for reserved is not enough!");
			return (RT_ERR_RG_FAILED);
		}
	}

	return (RT_ERR_RG_OK);

}

int _rtk_rg_aclReservedEntry_init(void)
{
	rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary = xDSL_ACL_IGR_RSVD_HEAD_SIZE;
	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary = (MAX_ACL_IGR_ENTRY_SIZE-xDSL_ACL_IGR_RSVD_TAIL_SIZE)-1; //MAX_INDEX is  MAX_SIZE-1
	//xDSL support egress ACL instead of CF. We just use rg_db cfLowerBoundary and cfUpperBoundary to store egress ACL table range.
	rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary = MAX_ACL_IGR_ENTRY_SIZE + xDSL_ACL_EGR_RSVD_HEAD_SIZE;
	rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary = (MAX_ACL_IGR_ENTRY_SIZE + MAX_ACL_EGR_ENTRY_SIZE - xDSL_ACL_EGR_RSVD_TAIL_SIZE)-1; //MAX_INDEX is	MAX_SIZE-1
	return (RT_ERR_RG_OK);
}


int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
	int i;
	ACL_RSV("#####Reserved ACL reflash!(add reserved ACL rsvType=%d)#####",rsvType);

	//init used parameter in fwdEngine dataPath, all rules will be readd again
	for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++){
		rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[i]=FAIL;
	}

	switch(rsvType){
		case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP: 
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ALL_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,1));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ALL_PERMIT]=ENABLED; 
			break;

		default:
			ACL_RSV("!!! xDSL driver not support !!\n");
			break;
	}

	_rtk_rg_aclAndCfReservedRuleHeadReflash();
	_rtk_rg_aclAndCfReservedRuleTailReflash();
	
	return (RT_ERR_RG_OK);
}

int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType)
{
	ACL_RSV("#####Reserved ACL reflash!(del reserved ACL rsvType=2)#####",rsvType);

	rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=DISABLED;

	_rtk_rg_aclAndCfReservedRuleHeadReflash();
	_rtk_rg_aclAndCfReservedRuleTailReflash();
	return (RT_ERR_RG_OK);
}


/*(5)fwdEngine datapath used APIs*/
int _rtk_rg_aclDecisionClear(rtk_rg_pktHdr_t *pPktHdr)
{
	pPktHdr->aclDecision.l34CFRuleHit=0;
	pPktHdr->aclDecision.l2CFHitAction=0;
	pPktHdr->aclDecision.action_type=0;
	pPktHdr->aclDecision.qos_actions=0; 

	pPktHdr->aclDecision.action_dot1p_remarking_pri=0; 
	pPktHdr->aclDecision.action_ip_precedence_remarking_pri=0; 
	pPktHdr->aclDecision.action_dscp_remarking_pri=0; 
	pPktHdr->aclDecision.action_queue_id=0; 
	pPktHdr->aclDecision.action_share_meter=0; 
	pPktHdr->aclDecision.action_stream_id_or_llid=0; 
	pPktHdr->aclDecision.action_acl_priority=0; 
	pPktHdr->aclDecision.action_redirect_portmask=0;
	pPktHdr->aclDecision.action_acl_egress_internal_priority=0;
	bzero(&pPktHdr->aclDecision.action_acl_cvlan,sizeof(rtk_rg_cvlan_tag_action_t));
	bzero(&pPktHdr->aclDecision.action_acl_svlan,sizeof(rtk_rg_svlan_tag_action_t));

	bzero(&pPktHdr->aclDecision.us_action_field,sizeof(rtk_rg_cf_us_action_type_t));
	bzero(&pPktHdr->aclDecision.ds_action_field,sizeof(rtk_rg_cf_ds_action_type_t));
	bzero(&pPktHdr->aclDecision.action_cvlan,sizeof(rtk_rg_cvlan_tag_action_t));
	bzero(&pPktHdr->aclDecision.action_svlan,sizeof(rtk_rg_svlan_tag_action_t));
	bzero(&pPktHdr->aclDecision.action_cfpri,sizeof(rtk_rg_cfpri_action_t));
	bzero(&pPktHdr->aclDecision.action_sid_or_llid,sizeof(rtk_rg_sid_llid_action_t));
	bzero(&pPktHdr->aclDecision.action_dscp,sizeof(rtk_rg_dscp_action_t));
	bzero(&pPktHdr->aclDecision.action_log,sizeof(rtk_rg_log_action_t));
	bzero(&pPktHdr->aclDecision.action_uni,sizeof(rtk_rg_uni_action_t));
	return SUCCESS;

}


int _rtk_rg_ingressACLPatternCheck(rtk_rg_pktHdr_t *pPktHdr, int ingressCvidRuleIdxArray[])
{

	int i;
	uint8 port;
	rtk_rg_aclFilterEntry_t* pChkRule;
	int ipv6_upper_range_check,ipv6_lowwer_range_check;
	int port_check;
	uint8 tos;
	int handleValidRuleCounter=0;
	int hitRuleCounter=0;
	
	if(ingressCvidRuleIdxArray==NULL){
		if(pPktHdr->aclDecision.aclIgrRuleChecked==1){
			return RT_ERR_RG_OK;
		}
		else{
			//normal _rtk_rg_ingressACLPatternCheck gas been called. the result is recored in pPktHdr->aclDecision, no need to check whole function again!
			pPktHdr->aclDecision.aclIgrRuleChecked = 1;
		}
	}
	
	pPktHdr->aclPriority = -1; //initial acl priority
	pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter]=-1; //empty from first rule until the end

	if(rg_db.systemGlobal.acl_SW_table_entry_size<=0)//no rule need to verify
		return RT_ERR_RG_OK;	

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//DEBUG("CHECK INGRESS ACL[%d]:",i);
		if(handleValidRuleCounter >= rg_db.systemGlobal.acl_SW_table_entry_size)//no valid rule need to check, skip rest for loop
			break;
			
		if(ingressCvidRuleIdxArray==NULL){
			if(rg_db.systemGlobal.acl_SW_table_entry[i].valid!=RTK_RG_ENABLED)//skip empty rule
				continue;
			pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[i]);
		}else{
			if(ingressCvidRuleIdxArray[i]==-1)
				break;
			pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[ingressCvidRuleIdxArray[i]]);
		}
		
		handleValidRuleCounter++; //a valid rule is going to check

		if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP)//trap to fwdEngine ignor
			continue;

		ACL("Check ingress pattern of ACLRule[%d]:		  handleValidRuleCounter=%d",i,handleValidRuleCounter);

		/*ingress rule check*/
		port_check = FAIL;
		if(pChkRule->acl_filter.filter_fields&INGRESS_PORT_BIT){
			for(port=0;port<RTK_RG_PORT_MAX;port++) //including extport
			{
				if(pChkRule->acl_filter.ingress_port_idx & (1<<pPktHdr->ingressPort))
					port_check = SUCCESS;						
			}
			if(port_check!=SUCCESS){
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT rule_pmask=0x%x spa=%d ",i,pChkRule->acl_filter.ingress_port_mask.portmask,pPktHdr->ingressPort);
				continue;
			}
		}
		
		if(pChkRule->acl_filter.filter_fields&INGRESS_INTF_BIT){
			ACL("RG_ACL[%d] INGRESS_INTF_BIT NOT support. ",i);
			continue;
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STREAM_ID_BIT){
			ACL("RG_ACL[%d] INGRESS_STREAM_ID_BIT NOT support. ",i);
			continue;
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_TAGIF_BIT){
			if(pChkRule->acl_filter.ingress_ipv4_tagif){//must have ip header
				if(pPktHdr->tagif&IPV4_TAGIF){
					//hit do nothing
					//ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT must tag hit ",i);
				}else{
					ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT UNHIT ",i);
					continue;
				}	
			}else{//must not have IP header
				if(pPktHdr->tagif&IPV4_TAGIF){
					ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT UNHIT ",i);
					continue;
				}else{
					//hit do nothing
					//ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT must untag hit ",i);
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_TAGIF_BIT){
			if(pChkRule->acl_filter.ingress_ipv6_tagif){//must have ip header
				if(pPktHdr->tagif&IPV6_TAGIF){
					//hit do nothing
					//ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT must tag hit ",i);
				}else{
					ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT UNHIT ",i);
					continue;
				}
			}else{//must not have IP header
				if(pPktHdr->tagif&IPV6_TAGIF){
					ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT UNHIT ",i);
					continue;
				}else{
					//hit do nothing
					//ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT must untag hit ",i);
				}
			}
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_STAGIF_BIT){
			if(pChkRule->acl_filter.ingress_stagIf){//must have stag
				if((pPktHdr->tagif&SVLAN_TAGIF)==0x0){
					ACL("RG_ACL[%d] INGRESS_STAGIF_BIT UNHIT ",i);
					continue;
				}
			}else{//must not have stag
				if(pPktHdr->tagif&SVLAN_TAGIF){
					ACL("RG_ACL[%d] INGRESS_STAGIF_BIT UNHIT ",i);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAGIF_BIT){
			if(pChkRule->acl_filter.ingress_ctagIf){//must have ctag
				if((pPktHdr->tagif&CVLAN_TAGIF)==0x0){
					ACL("RG_ACL[%d] INGRESS_CTAGIF_BIT UNHIT ",i);
					continue;
				}	
			}else{//must not have ctag
				if(pPktHdr->tagif&CVLAN_TAGIF){
					ACL("RG_ACL[%d] INGRESS_CTAGIF_BIT UNHIT ",i);
					continue;
				}
			}
		}

		
		if(pChkRule->acl_filter.filter_fields&INGRESS_ETHERTYPE_BIT){
			if((pChkRule->acl_filter.ingress_ethertype & pChkRule->acl_filter.ingress_ethertype_mask)!=(pPktHdr->etherType & pChkRule->acl_filter.ingress_ethertype_mask)){
				ACL("RG_ACL[%d] INGRESS_ETHERTYPE UNHIT ",i);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_PRI_BIT){
			if(pChkRule->acl_filter.ingress_stag_pri!=pPktHdr->stagPri){
				ACL("RG_ACL[%d] INGRESS_STAG_PRI UNHIT ",i);
				continue;
			}
		}
		
		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_VID_BIT){
			if(pChkRule->acl_filter.ingress_stag_vid!=pPktHdr->stagVid){
				ACL("RG_ACL[%d] INGRESS_STAG_VID UNHIT ",i);
				continue;
			}
		}
		
		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_DEI_BIT){
			if(pChkRule->acl_filter.ingress_stag_dei!=pPktHdr->stagDei){
				ACL("RG_ACL[%d] INGRESS_STAG_DEI UNHIT ",i);
				continue;
			}
		}
		
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_PRI_BIT){
			if(pChkRule->acl_filter.ingress_ctag_pri!=pPktHdr->ctagPri){
				ACL("RG_ACL[%d] INGRESS_CTAG_PRI UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_VID_BIT){
			if(pChkRule->acl_filter.ingress_ctag_vid!=pPktHdr->ctagVid){
				ACL("RG_ACL[%d] INGRESS_CTAG_VID UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_CFI_BIT){
			if(pChkRule->acl_filter.ingress_ctag_cfi!=pPktHdr->ctagCfi){
				ACL("RG_ACL[%d] INGRESS_CTAG_CFI UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_SMAC_BIT){
			if((pChkRule->acl_filter.ingress_smac.octet[0] & pChkRule->acl_filter.ingress_smac_mask.octet[0])!=(pPktHdr->pSmac[0]& pChkRule->acl_filter.ingress_smac_mask.octet[0]) ||
				(pChkRule->acl_filter.ingress_smac.octet[1] & pChkRule->acl_filter.ingress_smac_mask.octet[1])!=(pPktHdr->pSmac[1]& pChkRule->acl_filter.ingress_smac_mask.octet[1])||
				(pChkRule->acl_filter.ingress_smac.octet[2] & pChkRule->acl_filter.ingress_smac_mask.octet[2])!=(pPktHdr->pSmac[2]& pChkRule->acl_filter.ingress_smac_mask.octet[2])||
				(pChkRule->acl_filter.ingress_smac.octet[3] & pChkRule->acl_filter.ingress_smac_mask.octet[3])!=(pPktHdr->pSmac[3]& pChkRule->acl_filter.ingress_smac_mask.octet[3])||
				(pChkRule->acl_filter.ingress_smac.octet[4] & pChkRule->acl_filter.ingress_smac_mask.octet[4])!=(pPktHdr->pSmac[4]& pChkRule->acl_filter.ingress_smac_mask.octet[4])||
				(pChkRule->acl_filter.ingress_smac.octet[5] & pChkRule->acl_filter.ingress_smac_mask.octet[5])!=(pPktHdr->pSmac[5]& pChkRule->acl_filter.ingress_smac_mask.octet[5])
				){
				//memDump(pPktHdr->pSmac,6,"pktHdr pSmac");
				//memDump(pChkRule->acl_filter.ingress_smac.octet,6,"acl Smac");
				ACL("RG_ACL[%d] INGRESS_SMAC UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_DMAC_BIT){
			if((pChkRule->acl_filter.ingress_dmac.octet[0] & pChkRule->acl_filter.ingress_dmac_mask.octet[0])!=(pPktHdr->pDmac[0] & pChkRule->acl_filter.ingress_dmac_mask.octet[0]) ||
				(pChkRule->acl_filter.ingress_dmac.octet[1] & pChkRule->acl_filter.ingress_dmac_mask.octet[1])!=(pPktHdr->pDmac[1] & pChkRule->acl_filter.ingress_dmac_mask.octet[1])||
				(pChkRule->acl_filter.ingress_dmac.octet[2] & pChkRule->acl_filter.ingress_dmac_mask.octet[2])!=(pPktHdr->pDmac[2] & pChkRule->acl_filter.ingress_dmac_mask.octet[2])||
				(pChkRule->acl_filter.ingress_dmac.octet[3] & pChkRule->acl_filter.ingress_dmac_mask.octet[3])!=(pPktHdr->pDmac[3] & pChkRule->acl_filter.ingress_dmac_mask.octet[3])||
				(pChkRule->acl_filter.ingress_dmac.octet[4] & pChkRule->acl_filter.ingress_dmac_mask.octet[4])!=(pPktHdr->pDmac[4] & pChkRule->acl_filter.ingress_dmac_mask.octet[4])||
				(pChkRule->acl_filter.ingress_dmac.octet[5] & pChkRule->acl_filter.ingress_dmac_mask.octet[5])!=(pPktHdr->pDmac[5] & pChkRule->acl_filter.ingress_dmac_mask.octet[5])
				){
				//memDump(pPktHdr->pDmac,6,"pktHdr pDmac");
				//memDump(pChkRule->acl_filter.ingress_dmac.octet,6,"acl Dmac");
				ACL("RG_ACL[%d] INGRESS_DMAC UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_DSCP_BIT){
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				ACL("RG_ACL[%d] INGRESS_DSCP UNHIT ",i);
				continue;
			}else{
				tos = *(pPktHdr->pTos);
				if(pChkRule->acl_filter.ingress_dscp!=(tos>>2)){
					ACL("RG_ACL[%d] INGRESS_DSCP UNHIT ",i);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_TOS_BIT){
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				continue;
			}else{
				tos = *(pPktHdr->pTos);
				if(pChkRule->acl_filter.ingress_tos!=tos){
					ACL("RG_ACL[%d] INGRESS_TOS UNHIT");
					continue;
				}
			}
		}
		
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DSCP_BIT){
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				ACL("RG_ACL[%d] INGRESS_IPV6_DSCP_BIT UNHIT ",i);
				continue;
			}else{
				tos = (*(pPktHdr->pTos))<<4 & 0xf0;
				tos |= (*((pPktHdr->pTos)+1))>>4 & 0xf; 
				if(pChkRule->acl_filter.ingress_ipv6_dscp!=(tos>>2)){
					ACL("RG_ACL[%d] INGRESS_IPV6_DSCP UNHIT ",i);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_TC_BIT){
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				continue;
			}else{
				tos = (*(pPktHdr->pTos))<<4 & 0xf0;
				tos |= (*((pPktHdr->pTos)+1))>>4 & 0xf; 
				if(pChkRule->acl_filter.ingress_ipv6_tc!=tos){
					ACL("RG_ACL[%d] INGRESS_IPV6_TC UNHIT");
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_TCP_BIT){
			if( !(pPktHdr->tagif&TCP_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_TCP UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_UDP_BIT){
			if( !(pPktHdr->tagif&UDP_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_UDP UNHIT ",i);
				continue;
			}
		}		
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_ICMP_BIT){
			if(!(pPktHdr->tagif&ICMP_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_ICMP UNHIT ",i);
				continue;
			}
		}	

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_ICMPV6_BIT){
			if(!(pPktHdr->tagif&ICMPV6_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_ICMPV6_BIT UNHIT ",i);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_POROTCAL_VALUE_BIT){
			if((pPktHdr->ipProtocol)!=(pChkRule->acl_filter.ingress_l4_protocal)){
				ACL("RG_ACL[%d] INGRESS_L4_POROTCAL_VALUE_BIT UNHIT ",i);
				continue;
			}
		}
		
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
			ipv6_upper_range_check = -1;
			ipv6_lowwer_range_check = -1;
			if(pPktHdr->pIpv6Sip != NULL){
				ipv6_upper_range_check = memcmp(&(pChkRule->acl_filter.ingress_src_ipv6_addr_end[0]),pPktHdr->pIpv6Sip,16);
				 ipv6_lowwer_range_check = memcmp(pPktHdr->pIpv6Sip,&(pChkRule->acl_filter.ingress_src_ipv6_addr_start[0]),16);
			}
			if(!(ipv6_upper_range_check>=0 && ipv6_lowwer_range_check>=0 )){
				ACL("RG_ACL[%d] INGRESS_IPV6_SIP_RANGE UNHIT ",i);
				continue;
			}
		}		
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_SIP_BIT){
			if(pPktHdr->pIpv6Sip != NULL){
				if((pPktHdr->pIpv6Sip[0]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[0])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[0]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[0]) ||
					(pPktHdr->pIpv6Sip[1]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[1])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[1]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[1]) ||
					(pPktHdr->pIpv6Sip[2]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[2])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[2]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[2]) ||
					(pPktHdr->pIpv6Sip[3]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[3])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[3]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[3]) ||
					(pPktHdr->pIpv6Sip[4]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[4])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[4]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[4]) ||
					(pPktHdr->pIpv6Sip[5]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[5])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[5]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[5]) ||
					(pPktHdr->pIpv6Sip[6]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[6])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[6]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[6]) ||
					(pPktHdr->pIpv6Sip[7]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[7])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[7]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[7]) ||
					(pPktHdr->pIpv6Sip[8]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[8])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[8]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[8]) ||
					(pPktHdr->pIpv6Sip[9]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[9])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[9]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[9]) ||
					(pPktHdr->pIpv6Sip[10]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[10])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[10]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[10]) ||
					(pPktHdr->pIpv6Sip[11]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[11])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[11]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[11]) ||
					(pPktHdr->pIpv6Sip[12]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[12])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[12]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[12]) ||
					(pPktHdr->pIpv6Sip[13]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[13])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[13]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[13]) ||
					(pPktHdr->pIpv6Sip[14]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[14])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[14]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[14]) ||
					(pPktHdr->pIpv6Sip[15]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[15])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[15]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[15])
					){
						ACL("RG_ACL[%d] INGRESS_IPV6_SIP_BIT UNHIT ",i);
						continue;
				}
			}else{
				ACL("RG_ACL[%d] INGRESS_IPV6_SIP_BIT UNHIT ",i);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DIP_BIT){
			if(pPktHdr->pIpv6Dip != NULL){
				if((pPktHdr->pIpv6Dip[0]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[0])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[0]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[0]) ||
					(pPktHdr->pIpv6Dip[1]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[1])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[1]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[1]) ||
					(pPktHdr->pIpv6Dip[2]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[2])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[2]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[2]) ||
					(pPktHdr->pIpv6Dip[3]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[3])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[3]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[3]) ||
					(pPktHdr->pIpv6Dip[4]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[4])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[4]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[4]) ||
					(pPktHdr->pIpv6Dip[5]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[5])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[5]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[5]) ||
					(pPktHdr->pIpv6Dip[6]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[6])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[6]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[6]) ||
					(pPktHdr->pIpv6Dip[7]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[7])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[7]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[7]) ||
					(pPktHdr->pIpv6Dip[8]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[8])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[8]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[8]) ||
					(pPktHdr->pIpv6Dip[9]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[9])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[9]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[9]) ||
					(pPktHdr->pIpv6Dip[10]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[10])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[10]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[10]) ||
					(pPktHdr->pIpv6Dip[11]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[11])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[11]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[11]) ||
					(pPktHdr->pIpv6Dip[12]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[12])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[12]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[12]) ||
					(pPktHdr->pIpv6Dip[13]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[13])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[13]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[13]) ||
					(pPktHdr->pIpv6Dip[14]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[14])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[14]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[14]) ||
					(pPktHdr->pIpv6Dip[15]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[15])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[15]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[15])
					){
						ACL("RG_ACL[%d] INGRESS_IPV6_DIP_BIT UNHIT ",i);
						continue;
				}

				
			}else{
				ACL("RG_ACL[%d] INGRESS_IPV6_DIP_BIT UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DIP_RANGE_BIT){
			ipv6_upper_range_check = -1;
			ipv6_lowwer_range_check = -1;
			if(pPktHdr->pIpv6Dip!=NULL){
				ipv6_upper_range_check = memcmp(&(pChkRule->acl_filter.ingress_dest_ipv6_addr_end[0]),pPktHdr->pIpv6Dip,16);
				ipv6_lowwer_range_check = memcmp(pPktHdr->pIpv6Dip,&(pChkRule->acl_filter.ingress_dest_ipv6_addr_start[0]),16);
			}
			if(!(ipv6_upper_range_check>=0 && ipv6_lowwer_range_check>=0 )){
				ACL("RG_ACL[%d] INGRESS_IPV6_DIP_RANGE UNHIT ",i);
				continue;
			}
			
		}		
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
			if( !((pChkRule->acl_filter.ingress_src_ipv4_addr_end >= pPktHdr->ipv4Sip ) &&
				(pChkRule->acl_filter.ingress_src_ipv4_addr_start<= pPktHdr->ipv4Sip ))){
				ACL("RG_ACL[%d] INGRESS_IPV4_SIP_RANGE UNHIT ",i);
				continue;
			}
		}		
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
			if( !((pChkRule->acl_filter.ingress_dest_ipv4_addr_end >= pPktHdr->ipv4Dip ) &&
				(pChkRule->acl_filter.ingress_dest_ipv4_addr_start<= pPktHdr->ipv4Dip ))){
				ACL("RG_ACL[%d] INGRESS_IPV4_DIP_RANGE UNHIT ",i);
				continue;
			}
		}		
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
			if( !((pChkRule->acl_filter.ingress_src_l4_port_end >= pPktHdr->sport ) &&
				(pChkRule->acl_filter.ingress_src_l4_port_start<= pPktHdr->sport ))){
				ACL("RG_ACL[%d] INGRESS_L4_SPORT_RANGE UNHIT ",i);
				continue;
			}
		}		
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
			if( !((pChkRule->acl_filter.ingress_dest_l4_port_end >= pPktHdr->dport ) &&
				(pChkRule->acl_filter.ingress_dest_l4_port_start<= pPktHdr->dport ))){
				ACL("RG_ACL[%d] INGRESS_L4_DPORT_RANGE UNHIT ",i);
				continue;
			}
		}	

		
		if(pChkRule->acl_filter.filter_fields&INGRESS_WLANDEV_BIT){
			
			if(pChkRule->acl_filter.ingress_wlanDevMask&(1<<pPktHdr->wlan_dev_idx)){
				//hit! 
			}else{
				ACL("RG_ACL[%d] INGRESS_WLANDEV_BIT UNHIT ingress_wlanDevMask=0x%x pPktHdr->wlan_dev_idx=%d",i,pChkRule->acl_filter.ingress_wlanDevMask,pPktHdr->wlan_dev_idx);
				continue;
			}
		}
			
#if 0
		//do in _rtk_rg_ingressACLAction
		//record acl priority for internal priority decision
		if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_QOS &&
			(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT) &&
			pPktHdr->aclPriority==-1/*acl internal pri action not do before.*/){
			
			pPktHdr->aclPriority = pChkRule->acl_filter.action_acl_priority;
		}

#endif
		if(pPktHdr->aclPolicyRoute==FAIL && pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
		{
			ACL("RG_ACL[%d] HIT: Policy Route! use WAN[%d] as egress intf!!",i,pChkRule->acl_filter.action_policy_route_wan);
			//Keep policy route indicated egress WAN interface index in pktHdr
			pPktHdr->aclPolicyRoute=pChkRule->acl_filter.action_policy_route_wan;
		}
		//record ACL hit rule
		//ACL("HIT ACL Rule[%d]",i);
		ACL("RG_ACL[%d] Ingress Part Hit!",i);
		if(ingressCvidRuleIdxArray==NULL){
			pPktHdr->aclDecision.aclIgrHitMask[(i>>5)]|=(1<<(i&0x1f));
			pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter]=i; //record current rule
			pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter+1]=-1;//empty next rule until the end
			hitRuleCounter++;
		}else{
			//ingress_cvid action can only be single hit
			pPktHdr->internalVlanID =  pChkRule->acl_filter.action_acl_ingress_vid;
			TRACE("RG_ACL[%d] HIT: internalVlanID modify to %d",i,pPktHdr->internalVlanID);
			break;
		}

	}
	
	return RT_ERR_RG_OK;
}


rtk_rg_fwdEngineReturn_t _rtk_rg_ingressACLAction(rtk_rg_pktHdr_t *pPktHdr)
{
	/*
	*	This API is using for do pure ingress ACL actions such as: ingress_cvid,  ingress_svid, acl_priority, drop, trap_to_PS
	*	remarking actions will be done after _rtk_rg_egressACLAction() & _rtk_rg_modifyPacketByACLAction()
	*/

	//ingress_cvid,  ingress_svid have not supported right now.
	uint32 i, aclIdx=0;
	rtk_rg_aclFilterEntry_t* pChkRule;
	int permit=0;
	TRACE("Ingress ACL Action:");
	
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		aclIdx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];

		if(aclIdx==-1){ 
			// no more SW_acl rules
			break;
		}
		else if(rg_db.systemGlobal.acl_SW_table_entry[aclIdx].acl_filter.fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
			if(rg_db.systemGlobal.check_acl_priority_action_for_rg_acl_of_l34_type==ENABLED){
				if((pPktHdr->aclDecision.aclIgrHitMask[(aclIdx>>5)]&(1<<(aclIdx&0x1f))))
				{
					pChkRule= &rg_db.systemGlobal.acl_SW_table_entry[aclIdx];
					
					if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_QOS &&
						(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT) &&
						pPktHdr->aclPriority==-1/*acl internal pri action not do before.*/){
						
						pPktHdr->aclPriority = pChkRule->acl_filter.action_acl_priority;
						ACL("RG_ACL[%d] do ACL_PRIORITY to %d", aclIdx, pPktHdr->aclPriority);
					}
				}
				//TRACE("#####check_acl_priority_action_for_rg_acl_of_l34_type=%d (slow)#####",rg_db.systemGlobal.check_acl_priority_action_for_rg_acl_of_l34_type);
			}
			else //no acl priority action rules in	fwding_type_and_direction=ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_XXXXX, speed up.
			{
				//TRACE("#####check_acl_priority_action_for_rg_acl_of_l34_type=%d (fast)#####",rg_db.systemGlobal.check_acl_priority_action_for_rg_acl_of_l34_type);
				//pure ingress ACL rule finished, rest rules is includ egress acl 
				break;
			}
		}else{
			pChkRule= &rg_db.systemGlobal.acl_SW_table_entry[aclIdx];
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
				&& (pPktHdr->aclDecision.aclIgrHitMask[(aclIdx>>5)]&(1<<(aclIdx&0x1f)))){

				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_QOS &&
					(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT) &&
					pPktHdr->aclPriority==-1/*acl internal pri action not do before.*/){
					
					pPktHdr->aclPriority = pChkRule->acl_filter.action_acl_priority;
					ACL("RG_ACL[%d] do ACL_PRIORITY to %d", aclIdx, pPktHdr->aclPriority);
				}

				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_PERMIT)
				{
					ACL("RG_ACL[%d]: PERMIT",aclIdx);
					TRACE("ACL[%d] ACT:PERMIT",i);
					permit=1;
				}

				if((permit==0)&&(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP)){
					ACL(" RG_ACL[%d]: DROP",aclIdx);
					TRACE("ACL[%d] ACT:DROP",i);
					return RG_FWDENGINE_RET_DROP;
				}				

				if((permit==0)&&(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_TO_PS)&&(pPktHdr->pRxDesc->opts3.bit.dst_port_mask!=0x20)){
					ACL(" RG_ACL[%d] TRAP to PS",aclIdx);
					TRACE("ACL[%d] ACT:TRAP to PS",i);
					return RG_FWDENGINE_RET_ACL_TO_PS;
				}
			}
		}
	}

	return RG_FWDENGINE_RET_CONTINUE;

}


int _rtk_rg_egressACLPatternCheck(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify, rtk_rg_port_idx_t egressPort)
{
	//egressPort used for confirm packet egress to CF port, egressPort==-1 represent BC to lan
	uint32 i,index;
	rtk_rg_aclFilterEntry_t* pChkRule;
	ipaddr_t sipModify,dipModify;
	uint16 sportModify,dportModify;
	int dipL3Idx,netifIdx=-1,nexthopIdx;
	
	//reset the egressHitMask & final actions, because in broacast dataPath each Interfcaes will call this API individually 

	for(i=0;i<((MAX_ACL_SW_ENTRY_SIZE/32)+1);i++){
		pPktHdr->aclDecision.aclEgrHitMask[i]=0;
	}

	for(i=0;i<((TOTAL_CF_ENTRY_SIZE/32)+1);i++){
		pPktHdr->aclDecision.aclEgrL2HitMask[i]=0;
	}

	_rtk_rg_aclDecisionClear(pPktHdr);



	if((egressPort == RTK_RG_PORT_PON)&&(rg_db.systemGlobal.ponPortUnmatchCfDrop))
		pPktHdr->unmatched_cf_act = IDX_UNHIT_DROP; //to PON port, only upstream; not need to check if any downstream pkt
	else
		pPktHdr->unmatched_cf_act = IDX_UNHIT_PASS;

/*
	if(rg_db.systemGlobal.acl_SW_table_entry_size<=0)//no rule need to verify
		return RT_ERR_RG_OK;
*/
	//get original info
	sipModify = pPktHdr->ipv4Sip;
	dipModify = pPktHdr->ipv4Dip;
	sportModify = pPktHdr->sport; 
	dportModify = pPktHdr->dport;
	//ACL("Egress Pattern check: direct=%d naptIdx=%d l3Modify=%d l4Modify=%d netifIdx=%d  fwdDecision=%d",direct,naptIdx,l3Modify,l4Modify,pPktHdr->netifIdx,pPktHdr->fwdDecision);


	//get L34 info (translate SIP/DIP  SPORT/DPORT)
	if(direct==RG_FWD_DECISION_NAPT)
	{

		//fill SIP
		if(l3Modify)
			sipModify=rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr;

		//fill SPORT
		if(l4Modify)
			sportModify=rg_db.naptOut[naptIdx].extPort;

		//pPktHdr->netifIdx has been set by _rtk_rg_routingDecisionTablesLookup(pPktHdr,SIP_DIP_CLASS_NAPT);
		netifIdx = pPktHdr->netifIdx;

	}
	else if(direct==RG_FWD_DECISION_NAPTR)//INBOUND
	{					
		//fill DIP
		if(l3Modify)
			dipModify=rg_db.naptIn[naptIdx].rtk_naptIn.intIp;
		
		//fill DPORT
		if(l4Modify)
			dportModify=rg_db.naptIn[naptIdx].rtk_naptIn.intPort;

		//set egress intf index to pktHdr
		//_rtk_rg_routingDecisionTablesLookup(pPktHdr,SIP_DIP_CLASS_NAPTR); => calling this API call will make some of pktHdr information changed!we just need to decide  egress netifIdx.
		dipL3Idx = _rtk_rg_l3lookup(dipModify);
		if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_ARP)
		{
			netifIdx=rg_db.l3[dipL3Idx].rtk_l3.netifIdx;
		}
		else if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_NH)
		{
			nexthopIdx=rg_db.l3[dipL3Idx].rtk_l3.nhStart; 
			netifIdx=rg_db.nexthop[nexthopIdx].rtk_nexthop.ifIdx;
		}
		else if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_CPU)
		{

			if(rg_db.l3[dipL3Idx].rtk_l3.ipAddr > 0)
			{
				netifIdx=rg_db.l3[dipL3Idx].rtk_l3.netifIdx;
			}
			else
			{
				netifIdx = -1; //netifIdx not found
			}
		}

	}else{
		//bridge mode or IPv6 routing mode: will be Fail(-1) => can not hit egress_CVID, or egress_CPRI pattern.
		//or binding: pPktHdr->netifIdx will be the wanIntf index
		//or from protocal-stack:  pPktHdr->netifIdx will be the wanIntf index.
		netifIdx = pPktHdr->netifIdx;
	}

	//check L34 pattern: CF 0-63	
	//for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
	for(index=0;index<MAX_ACL_SW_ENTRY_SIZE;index++){
		i = pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[index];
		if(i==-1)//no rest rule need to check
			break;
	
		//if(!(pPktHdr->aclDecision.aclIgrHitMask[(i>>5)]&(1<<(i&0x1f))))//ignor ingress not hit rules
			//continue;
		//DEBUG("CHECK EGRESS ACL[%d]:",i);
	
		pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[i]);
		//ACL("Check egress pattern of ACLRule[%d]:",i);

		if(pPktHdr->ingressPort==RTK_RG_PORT_PON) //downstream
		{			
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP)
			{
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT DOWNSTREAM ",i);
				continue;
			}
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
			{
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT DOWNSTREAM ",i);
				continue;			
			}
		}
		else //maybe upstream, 
		{
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP)
			{
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT UPSTREAM ",i);
				continue;
			}
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT UPSTREAM ",i);
				continue;			
			}
			//egressPort will be check later because DROP action will be hit even it is not

			//consider drop
			if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP){
				//let it continue check. The Drop action will alwaye be Execute(if hit) even packet is not related to CF port.
				
			}else{
				if(pChkRule->acl_filter.filter_fields & PURE_CF_PATTERN_BITS){//if any cf pattern is valid, we have to confirm it is egress to CF port. (If pure ACL rule, let it pass)

#if defined(CONFIG_RTL9602C_SERIES)
					if(egressPort!=RTK_RG_PORT_PON)
#else
					if((egressPort!=RTK_RG_PORT_PON) && (egressPort!=RTK_RG_PORT_RGMII))
#endif						
					{//make sure its upstream, must egress to CF port
						ACL("RG_ACL[%d] EGRESS_PORT UNHIT UPSTREAM egressPort=%d ",i,egressPort);
						continue;	
					} 
				}
			}
		}



		if(pChkRule->acl_filter.filter_fields&EGRESS_INTF_BIT){

			//L2, the egress_Intf is always zero (H/W behavior), but in fwdEngine pktHrd->netifIdx init with FAIL(-1)
			//[FIXME] apollo_FE may change this behavior. 
			if(direct==RG_FWD_DECISION_BRIDGING){	
				//normal bridge
				if(netifIdx==-1){
					if(pChkRule->acl_filter.egress_intf_idx != 0){//synwith HWNAT bebavior, bridge: the user should fill egress_intf_idx to zero.
						ACL("RG_ACL[%d] EGRESS_INTF UNHIT ",i);
						continue;
					}
				}
				//binding, or from PS(fron PS case will change pktHdr->netifIdx to ingress interface)
				else if(pChkRule->acl_filter.egress_intf_idx != netifIdx){
					ACL("RG_ACL[%d] EGRESS_INTF UNHIT rule_intfIdx=%x Decision_netifIdx=%d ",i,pChkRule->acl_filter.egress_intf_idx ,netifIdx);
					continue;
				}
			}else{
				if(rg_db.netif[pChkRule->acl_filter.egress_intf_idx].rtk_netif.valid==DISABLED){
					ACL("RG_ACL[%d] EGRESS_INTF UNHIT ",i);
					continue;
				}
				if(pChkRule->acl_filter.egress_intf_idx != netifIdx){
					ACL("RG_ACL[%d] EGRESS_INTF UNHIT ",i);
					continue;
				}	
			}
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_IPV4_SIP_RANGE_BIT){
			if( !((pChkRule->acl_filter.egress_src_ipv4_addr_end >= sipModify ) &&
				(pChkRule->acl_filter.egress_src_ipv4_addr_start<= sipModify ))){
				//DEBUG(" sipModify=0x%x",sipModify);
				//ACL("EGRESS_src_ipv4_addr_start=0x%x egress_src_ipv4_addr_end=0x%x",pChkRule->acl_filter.egress_src_ipv4_addr_start,pChkRule->acl_filter.egress_src_ipv4_addr_end);
				ACL("RG_ACL[%d] EGRESS_IPV4_SIP_RANGE UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
			if( !((pChkRule->acl_filter.egress_dest_ipv4_addr_end >= dipModify ) &&
				(pChkRule->acl_filter.egress_dest_ipv4_addr_start<= dipModify))){
				//DEBUG(" dipModify=0x%x",dipModify);
				//ACL("EGRESS_dest_ipv4_addr_start=0x%x egress_dest_ipv4_addr_end=0x%x",pChkRule->acl_filter.egress_dest_ipv4_addr_start,pChkRule->acl_filter.egress_dest_ipv4_addr_end);
				ACL("RG_ACL[%d] EGRESS_IPV4_DIP_RANGE UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_L4_SPORT_RANGE_BIT){
			if( !((pChkRule->acl_filter.egress_src_l4_port_end >= sportModify ) &&
				(pChkRule->acl_filter.egress_src_l4_port_start<= sportModify ))){			
				ACL("RG_ACL[%d] EGRESS_L4_SPORT_RANGE UNHIT ",i);
				continue;
			}

		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_L4_DPORT_RANGE_BIT){
			if( !((pChkRule->acl_filter.egress_dest_l4_port_end >= dportModify ) &&
				(pChkRule->acl_filter.egress_dest_l4_port_start<= dportModify))){
				ACL("RG_ACL[%d] EGRESS_L4_DPORT_RANGE UNHIT ",i);
				continue;
			}
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_VID_BIT){

				//L2, the egress_VID is equal to internalVID (H/W behavior)
				if(direct==RG_FWD_DECISION_BRIDGING){
					if(netifIdx==FAIL){//nomal Bridge
						if((pPktHdr->tagif&CVLAN_TAGIF)==0x0){ //L2 for this pattern must have Ctag
							ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  packet without Ctag ",i);
							continue;
						}
						if((pChkRule->acl_filter.egress_ctag_vid & pChkRule->acl_filter.egress_ctag_vid_mask)!= (pPktHdr->internalVlanID & pChkRule->acl_filter.egress_ctag_vid_mask)){
							ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  pChkRule->egress_ctag_vid=%d pPktHdr->egressVlanID=%d, pChkRule->acl_filter.egress_ctag_vid_mask=0x%x",i,pChkRule->acl_filter.egress_ctag_vid,pPktHdr->internalVlanID,pChkRule->acl_filter.egress_ctag_vid_mask);
							continue;
						}
					}else{//binding or from PS, the egress_ctag_vid should checked by intf
							if((pChkRule->acl_filter.egress_ctag_vid& pChkRule->acl_filter.egress_ctag_vid_mask)!=(rg_db.netif[netifIdx].rtk_netif.vlan_id & pChkRule->acl_filter.egress_ctag_vid_mask)){
							ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  pChkRule->egress_ctag_vid=%d rg_db.netif[%d].vlan_id=%d, pChkRule->acl_filter.egress_ctag_vid_mask=0x%x ",i,pChkRule->acl_filter.egress_ctag_vid,netifIdx,rg_db.netif[netifIdx].rtk_netif.vlan_id,pChkRule->acl_filter.egress_ctag_vid_mask);
							continue;
						}
					}
				}else{
					/*L34 should always egress with Interfaace VLANID*/
					if(rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.is_wan){
						//Wan intf
						if((pChkRule->acl_filter.egress_ctag_vid&pChkRule->acl_filter.egress_ctag_vid_mask)!=(rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id&pChkRule->acl_filter.egress_ctag_vid_mask)){
							ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT ",i);
							continue;
						}
					}else{
						//Lan intf
						if((pChkRule->acl_filter.egress_ctag_vid&pChkRule->acl_filter.egress_ctag_vid_mask)!=(rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.lan_intf.intf_vlan_id&pChkRule->acl_filter.egress_ctag_vid_mask)){
							ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT ",i);
							continue;
						}
					}
				}
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_PRI_BIT){

			//L2, the egress_cpri is ingress_cpri (if ctag), or egress_cpri is internal-cpri(if untag)
			//[FIXME] apollo_FE may change this behavior. 
			if(direct==RG_FWD_DECISION_BRIDGING){
				if(pChkRule->acl_filter.egress_ctag_pri!=pPktHdr->egressPriority){
					ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT ",i);
					continue;
				}
			}else{
				/*L34 have upstream can find wan Interface*/
				// this pattern just consider upstream (egress port==PON port), downstream have not enough information about egress port.
				if(rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.is_wan){
					rtk_enable_t enable;
					int ret=rtk_qos_1pRemarkEnable_get(RTK_RG_MAC_PORT_PON, &enable);
					assert_ok(ret);
					if(enable==ENABLED){//PON remarking: check internal priority
						int i;
						int flag_pattern_hit=DISABLED;
						int duplicateSize;
						int intPri[8];		
						assert_ok(_rtk_rg_dot1pPriRemarking2InternalPri_search(pChkRule->acl_filter.egress_ctag_pri, &duplicateSize,intPri));
						for(i=0;i<duplicateSize;i++){
							if(intPri[i]==pPktHdr->internalPriority){
								flag_pattern_hit=ENABLED; //pattern hit
								break;
							}
						}
						if(flag_pattern_hit==DISABLED){
							ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT ",i);
							continue;
						}
					}else{//PON not remarking:
						if(pPktHdr->tagif & CVLAN_TAGIF){//tagged pkt: check tagged cpri
							if(pChkRule->acl_filter.egress_ctag_pri!=pPktHdr->ctagPri){
								ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT ",i);
								continue;
							}
						}else{//untagged pkt:check port-based priority
							int priority=0;
							if(pPktHdr->ingressPort!=RTK_RG_PORT_PON){
								if(pPktHdr->ingressPort<RTK_RG_PORT_PON)
								{
									(pf.rtk_rg_qosPortBasedPriority_get)(pPktHdr->ingressPort,&priority);
								}else{//ext port follow CPU port-based priority
									(pf.rtk_rg_qosPortBasedPriority_get)(RTK_RG_PORT_CPU,&priority);
								}
							}
							if(priority!=0){//HW limitation, untagged case egress_ctag_pri will be port-based pri, and now we force all port-based priority to zero!
								ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT ",i);
								continue;
							}
						}
					}
				}else{
					ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT: downstream not support this pattern! ",i);
					continue;
				}
			}
		}



		if(pChkRule->acl_filter.filter_fields&INTERNAL_PRI_BIT){
			if(pChkRule->acl_filter.internal_pri!=pPktHdr->internalPriority){
				ACL("RG_ACL[%d] INTERNAL_PRI_BIT UNHIT ",i);
				continue;
			}
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_EGRESS_PORTIDX_BIT){//uni pattern
			if(pPktHdr->egressMACPort==RTK_RG_MAC_PORT_PON){//CF upstream(to PON), check spa
				if( (pChkRule->acl_filter.ingress_port_idx & pChkRule->acl_filter.ingress_port_idx_mask)!=(pPktHdr->ingressPort & pChkRule->acl_filter.ingress_port_idx_mask)){
					ACL("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
					continue;
				}
			}else if(pPktHdr->ingressPort==(rtk_rg_port_idx_t)RTK_RG_MAC_PORT_PON){//CF downstream(from PON),check da
				if(l3Modify){//NAPT or V4V6Routing, ingress DA lookup should always be CPU port because DA==Gateway MAC 
					if((pChkRule->acl_filter.egress_port_idx & pChkRule->acl_filter.egress_port_idx_mask)!=RTK_RG_MAC_PORT_CPU){//DA lookup should always be CPU port
						ACL("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
						continue;
					}
				}else{//Bridge
					if( (pChkRule->acl_filter.egress_port_idx & pChkRule->acl_filter.egress_port_idx_mask)!=(pPktHdr->egressMACPort & pChkRule->acl_filter.egress_port_idx_mask)){
						ACL("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
						continue;
					}
				}
			}else{
				//not CF case.
			}
		}



		//record CF hit rule
		//ACL("HIT CF0-63[%d]",i);
		ACL("RG_ACL[%d] Egress Part Hit!",i);
		pPktHdr->aclDecision.aclEgrHitMask[i>>5]|=(1<<(i&0x1f));
		pPktHdr->unmatched_cf_act = IDX_UNHIT_PASS;
	}
	
	return RT_ERR_RG_OK;

}


int _rtk_rg_egressACLAction(int direct, rtk_rg_pktHdr_t *pPktHdr)
{
	uint32 i,acl_idx;
	rtk_rg_aclFilterEntry_t* pChkRule;
	int permit=0;
	TRACE("Egress ACL Action:");
	
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){

		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;	//no more SW_ACL rules
		
		acl_idx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];
		if((pPktHdr->aclDecision.aclIgrHitMask[(acl_idx>>5)]&(1<<(acl_idx&0x1f))) && (pPktHdr->aclDecision.aclEgrHitMask[(acl_idx>>5)]&(1<<(acl_idx&0x1f)))){

			ACL("RG_ACL[%d] Hit!!!",acl_idx);
			if((rg_db.systemGlobal.acl_SW_table_entry[acl_idx].acl_filter.action_type==ACL_ACTION_TYPE_QOS && rg_db.systemGlobal.acl_SW_table_entry[acl_idx].acl_filter.qos_actions==ACL_ACTION_STREAM_ID_OR_LLID_BIT) ||
				rg_db.systemGlobal.acl_SW_table_entry[acl_idx].acl_filter.action_type==ACL_ACTION_TYPE_POLICY_ROUTE){
				//if rule is L34 and only SID action, we let it add to shortcut
				//if rule is L34 policy route action, we let it add to shortcut
			}else{
				pPktHdr->aclHit = 1; //any rule hit with other actions, then can not add this flow to short cut.
			}

			pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[acl_idx]);


			if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP){
				if(permit==0 
					|| (pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
					pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP)		//CF DROP included
					)
				{

					TRACE("RG_ACL[%d] DROP!",acl_idx);
					return RG_FWDENGINE_RET_DROP;	
				}
			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_PERMIT){
//				DEBUG(" ACL[%d] CONTINUE",i);
				//return RG_FWDENGINE_RET_CONTINUE;
				permit=1;
			}else{ //Qos Type
				pPktHdr ->aclDecision.action_type = ACL_ACTION_TYPE_QOS;

				/*chuck:
				   Here we support ACL different action hit in different rule, 
				   The prority of any_DROP=any_trap>CF>ACL, 
				   and sigle CF can be hit should take care in RG ACL api by forcing action assign & sorting the rules*/
				
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_1P_REMARKING_BIT){

					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_1P_REMARKING_BIT)){//ACL_ACTION_1P_REMARKING_BIT have not been set
						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_1P_REMARKING_BIT;
							pPktHdr->aclDecision.action_dot1p_remarking_pri = pChkRule->acl_filter.action_dot1p_remarking_pri;
							pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_CACT_DONE);
							ACL("RG_ACL[%d] do 1P_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dot1p_remarking_pri);
						}
						else
						{	
							ACL("RG_ACL[%d] skip 1P_REMARKING to %d, because CF[0-64] is already done!", acl_idx, pPktHdr->aclDecision.action_dot1p_remarking_pri);
						}
					}
				}
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT || pPktHdr->aclDecision.qos_actions&ACL_ACTION_DSCP_REMARKING_BIT)){//both  ACL_ACTION_1P_REMARKING_BIT & ACL_ACTION_DSCP_REMARKING_BIT have not been set
						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT;
							pPktHdr->aclDecision.action_ip_precedence_remarking_pri= pChkRule->acl_filter.action_ip_precedence_remarking_pri;
							pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_DSCP_REMARK_DONE);
							ACL("RG_ACL[%d] do IP_PRECEDENCE_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
						}
						else
						{
							ACL("RG_ACL[%d] skip IP_PRECEDENCE_REMARKING to %d, because CF[0-64] is already done!", acl_idx, pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
						}
					}	
				}
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_DSCP_REMARKING_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT || pPktHdr->aclDecision.qos_actions&ACL_ACTION_DSCP_REMARKING_BIT)){//both  ACL_ACTION_1P_REMARKING_BIT & ACL_ACTION_DSCP_REMARKING_BIT have not been set
						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_DSCP_REMARKING_BIT;
							pPktHdr->aclDecision.action_dscp_remarking_pri= pChkRule->acl_filter.action_dscp_remarking_pri;
							pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_DSCP_REMARK_DONE);
							ACL("RG_ACL[%d] do DSCP_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dscp_remarking_pri);
						}
						else
						{
							ACL("RG_ACL[%d] skip DSCP_REMARKING to %d, because CF[0-64] is already done!", acl_idx, pPktHdr->aclDecision.action_dscp_remarking_pri);
						}
					}
				}
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_QUEUE_ID_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_QUEUE_ID_BIT)){//ACL_ACTION_QUEUE_ID_BIT have not been set
						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_QUEUE_ID_BIT;
							pPktHdr->aclDecision.action_queue_id= pChkRule->acl_filter.action_queue_id;
							pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_CFPRI_ACT_DONE);					
							ACL("RG_ACL[%d] do QUEUE_ID to %d", acl_idx, pPktHdr->aclDecision.action_queue_id);
						}
						else
						{
							ACL("RG_ACL[%d] skip QUEUE_ID to %d, because CF[0-64] is already done!", acl_idx, pPktHdr->aclDecision.action_queue_id);
						}
					}
				}
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_SHARE_METER_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_SHARE_METER_BIT)){//ACL_ACTION_QUEUE_ID_BIT have not been set
						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_SHARE_METER_BIT;
							pPktHdr->aclDecision.action_share_meter= pChkRule->acl_filter.action_share_meter;
							pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_LOG_ACT_DONE);
							ACL("RG_ACL[%d] do SHARE_METER to %d", acl_idx, pPktHdr->aclDecision.action_share_meter);
						}
						else
						{
							ACL("RG_ACL[%d] skip SHARE_METER to %d, because CF[0-64] is already done!", acl_idx, pPktHdr->aclDecision.action_share_meter);
						}
					}
				}
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_STREAM_ID_OR_LLID_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_STREAM_ID_OR_LLID_BIT)){//ACL_ACTION_QUEUE_ID_BIT have not been set
						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_STREAM_ID_OR_LLID_BIT;
							pPktHdr->aclDecision.action_stream_id_or_llid= pChkRule->acl_filter.action_stream_id_or_llid;
							pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_SID_ACT_DONE);
							ACL("RG_ACL[%d] do STREAM_ID_OR_LLID to %d", acl_idx, pPktHdr->aclDecision.action_stream_id_or_llid);
						}
						else
						{
							ACL("RG_ACL[%d] skip STREAM_ID_OR_LLID to %d, because CF[0-64] is already done!", acl_idx, pPktHdr->aclDecision.action_stream_id_or_llid);
						}

					}
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT){
					//aclPriority is used for internalPriority decision, no need to use after this function anymore.
				}		
				
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT)){//ACL_ACTION_QUEUE_ID_BIT have not been set
						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT;
							pPktHdr->aclDecision.action_acl_egress_internal_priority= pChkRule->acl_filter.egress_internal_priority;
							pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_CFPRI_ACT_DONE);					
							ACL("RG_ACL[%d] assign CFPRI to %d", acl_idx, pPktHdr->aclDecision.action_acl_egress_internal_priority);
						}
						else
						{
							ACL("RG_ACL[%d] skip CFPRI to %d, because CF[0-64] is already done!", acl_idx, pPktHdr->aclDecision.action_queue_id);
						}
					}
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_REDIRECT_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_REDIRECT_BIT)){//ACL_ACTION_REDIRECT_BIT have not been set
						permit=1; //this action will permit following drop/trap.
						


						//This action only supported by ACL, not supported in CF, no need to check pPktHdr->aclDecision.l34CFRuleHit
						//if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_REDIRECT_BIT;
							pPktHdr->aclDecision.action_redirect_portmask= pChkRule->acl_filter.redirect_portmask;
						//}
						//DEBUG("Redirect to 0x%x",pPktHdr->aclDecision.action_redirect_portmask);
					}		
				}


				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_CVLANTAG_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_CVLANTAG_BIT)){//ACL_ACTION_ACL_CVLANTAG_BIT have not been set
						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_ACL_CVLANTAG_BIT;
							pPktHdr->aclDecision.action_acl_cvlan= pChkRule->acl_filter.action_acl_cvlan;

							if(pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision!=ACL_CVLAN_TAGIF_NOP){//nop will not effect CF[64-511] hit
								pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_CACT_DONE);
							}
							
							ACL("RG_ACL[%d] do CVLANTAG to vid=%d", 
								acl_idx, 
								pPktHdr->aclDecision.action_acl_cvlan.assignedCvid
								);
						}
						else
						{
							ACL("RG_ACL[%d] skip CVLANTAG to vid=%d !", 
								acl_idx, 
								pPktHdr->aclDecision.action_acl_cvlan.assignedCvid
								);
						}
					}
				}
				
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_DS_UNIMASK_BIT){
					if(pPktHdr->aclDecision.l34CFRuleHit == 0){
						pPktHdr->aclDecision.action_uni.uniActionDecision = ACL_UNI_FWD_TO_PORTMASK_ONLY;
						pPktHdr->aclDecision.action_uni.assignedUniPortMask = pChkRule->acl_filter.downstream_uni_portmask;
						pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_UNI_ACT_DONE);
						ACL("RG_ACL[%d] do UNIPORT MASK=0x%x\n",acl_idx,pPktHdr->aclDecision.action_uni.assignedUniPortMask);
					}
					else
					{
						ACL("RG_ACL[%d] skip UNIPORT MASK=0x%x, because CF[0-64] is already done!\n",acl_idx,pPktHdr->aclDecision.action_uni.assignedUniPortMask);
					}
				}
			}

			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN){
			
				//CF:0-63 first rule is hit
				pPktHdr->aclDecision.l34CFRuleHit = 1;
			}
		}
	}

#if 0	
	DEBUG("===ACL ACTION FINAL DECISION:===");
	DEBUG(" 1P_REMARKING[%s]: vid=%d",pPktHdr->aclDecision.qos_actions&ACL_ACTION_1P_REMARKING_BIT?"O":"X",pPktHdr->aclDecision.action_dot1p_remarking_pri);
	DEBUG(" IP_PRECEDENCE_REMARKING[%s]: ip_pre=0x%x",pPktHdr->aclDecision.qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT?"O":"X",pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
	DEBUG(" DSCP_REMARKING[%s]: dscp=%d",pPktHdr->aclDecision.qos_actions&ACL_ACTION_DSCP_REMARKING_BIT?"O":"X",pPktHdr->aclDecision.action_dscp_remarking_pri);
	DEBUG(" QUEUE_ID[%s]: qid=%d",pPktHdr->aclDecision.qos_actions&ACL_ACTION_QUEUE_ID_BIT?"O":"X",pPktHdr->aclDecision.action_queue_id);
	DEBUG(" SHARE_METER[%s]: shmeter=%d",pPktHdr->aclDecision.qos_actions&ACL_ACTION_SHARE_METER_BIT?"O":"X",pPktHdr->aclDecision.action_share_meter);
	DEBUG(" STREAM_ID[%s]: sid=%d",pPktHdr->aclDecision.qos_actions&ACL_ACTION_STREAM_ID_OR_LLID_BIT?"O":"X",pPktHdr->aclDecision.action_stream_id_or_llid);
#endif

	if(pPktHdr->unmatched_cf_act == IDX_UNHIT_DROP)
	{
		//drop L2 un-matched-CF pkt
		TRACE("RG_CF Un-matched Drop");
		return RG_FWDENGINE_RET_DROP;
	}

	return RG_FWDENGINE_RET_CONTINUE;
}


int _rtk_rg_modifyPacketByACLAction(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr,rtk_rg_port_idx_t egressPort)
{
	/* do ACL+CF0-63 actions */
	if(pPktHdr->aclDecision.action_type==ACL_ACTION_TYPE_QOS)
	{		
		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT)>0){
			pPktHdr->internalPriority= pPktHdr->aclDecision.action_acl_egress_internal_priority;	
			TRACE("Modify by ACL_CF[0-63] ACT:CFPRI Egress_internal_priority=%d",pPktHdr->internalPriority);
		}
	}

	return RT_ERR_RG_OK;
}





/*(6)debug tool APIs*/

char * strFilterFields[]=
{
	"INGRESS_PORT_BIT",
	"INGRESS_INTF_BIT",
	"EGRESS_INTF_BIT",		//not support in 0371 testchip
	"INGRESS_ETHERTYPE_BIT",
	"INGRESS_CTAG_PRI_BIT",
	"INGRESS_CTAG_VID_BIT",
	"INGRESS_SMAC_BIT",
	"INGRESS_DMAC_BIT",
	"INGRESS_DSCP_BIT",
	"INGRESS_L4_TCP_BIT",
	"INGRESS_L4_UDP_BIT",	
	"INGRESS_IPV6_SIP_RANGE_BIT",
	"INGRESS_IPV6_DIP_RANGE_BIT",
	"INGRESS_IPV4_SIP_RANGE_BIT",
	"INGRESS_IPV4_DIP_RANGE_BIT",
	"INGRESS_L4_SPORT_RANGE_BIT",
	"INGRESS_L4_DPORT_RANGE_BIT",
	"EGRESS_IPV4_SIP_RANGE_BIT",	//not support in 0371 testchip
	"EGRESS_IPV4_DIP_RANGE_BIT",	//not support in 0371 testchip
	"EGRESS_L4_SPORT_RANGE_BIT",	//not support in 0371 testchip
	"EGRESS_L4_DPORT_RANGE_BIT",	//not support in 0371 testchip
	"INGRESS_L4_ICMP_BIT",	//not support in 0371 testchip,  IPv4 ICMP only
	"EGRESS_CTAG_PRI_BIT",
	"EGRESS_CTAG_VID_BIT",
	"INGRESS_IPV6_DSCP_BIT",			//Only support while PPPoE Passthrought disabled. 
	"INGRESS_STREAM_ID_BIT",
	"INGRESS_STAG_PRI_BIT",
	"INGRESS_STAG_VID_BIT",
	"INGRESS_STAGIF_BIT",
	"INGRESS_CTAGIF_BIT",
	"INGRESS_EGRESS_PORTIDX_BIT",	//cf [2:0]uni pattern: ingress_port_idx for US, egress_port_idx for DS
	"INTERNAL_PRI_BIT",	//cf [7:5]IntPri pattern
	"INGRESS_L4_POROTCAL_VALUE_BIT",
	"INGRESS_TOS_BIT",
	"INGRESS_IPV6_TC_BIT",
	"INGRESS_IPV6_SIP_BIT",
	"INGRESS_IPV6_DIP_BIT",
	"INGRESS_WLANDEV_BIT",	//only supported in fwdEngine, and limit ingress_port_mask is (1<<RTK_RG_EXT_PORT0)
	"INGRESS_IPV4_TAGIF_BIT",
	"INGRESS_IPV6_TAGIF_BIT",
	"INGRESS_L4_ICMPV6_BIT",	//IPv6 ICMPv6 only
	"INGRESS_CTAG_CFI_BIT",
	"INGRESS_STAG_DEI_BIT",
};

int32 _dump_rg_acl(struct seq_file *s)
{

#if 1
	int i;
	int fieldIdx;
	rtk_rg_aclFilterAndQos_t *aclPara;
	rtk_rg_aclFilterAndQos_t *acl_parameter;

	PROC_PRINTF("acl_SW_table_entry_size:%d\n",rg_db.systemGlobal.acl_SW_table_entry_size);

	PROC_PRINTF("aclSW rule index sorting:\n");
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;
		PROC_PRINTF("ACL[%d]:weight(%d)",rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i],
			rg_db.systemGlobal.acl_SW_table_entry[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i])].acl_filter.acl_weight);
		if(i+1!=MAX_ACL_SW_ENTRY_SIZE && rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i+1]>=0)
			PROC_PRINTF(" > ");
	}
	PROC_PRINTF("\n");
	
	
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields!=0x0){
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED){
			aclPara = &(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter);
			PROC_PRINTF("========================RG_ACL[%d]===========================\n",i);
			PROC_PRINTF("[hw_acl_start:%d(continue:%d) hw_cf_start:%d(continue:%d)] \n"
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_start
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_size
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_cfEntry_start
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_cfEntry_size);
			PROC_PRINTF("acl_weight: %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight);

			for(fieldIdx=0;fieldIdx<64;fieldIdx++)
			{
				if(aclPara->filter_fields && (aclPara->filter_fields & (0x1<<fieldIdx)))
					break;
			}
			PROC_PRINTF("[Patterns]: \n");	
			PROC_PRINTF("filter_fields:0x%llx - %s\n", aclPara->filter_fields, strFilterFields[fieldIdx]);	
			if(aclPara->filter_fields&INGRESS_PORT_BIT) PROC_PRINTF("ingress_port_mask:0x%x\n", aclPara->ingress_port_idx);	
			if(aclPara->filter_fields&INGRESS_EGRESS_PORTIDX_BIT) PROC_PRINTF("ingress_port_idx:  %d  ingress_port_idx_mask:	%d\n", aclPara->ingress_port_idx,aclPara->ingress_port_idx_mask);
			if(aclPara->filter_fields&INGRESS_EGRESS_PORTIDX_BIT) PROC_PRINTF("egress_port_idx:  %d  egress_port_idx_mask:	%d\n", aclPara->egress_port_idx,aclPara->egress_port_idx_mask);
			if(aclPara->filter_fields&INGRESS_DSCP_BIT) PROC_PRINTF("ingress_dscp:	   %d\n", aclPara->ingress_dscp);			
			if(aclPara->filter_fields&INGRESS_INTF_BIT) PROC_PRINTF("ingress_intf_idx: %d\n", aclPara->ingress_intf_idx);	
			if(aclPara->filter_fields&EGRESS_INTF_BIT) PROC_PRINTF("egress_intf_idx:  %d\n", aclPara->egress_intf_idx); 
			if(aclPara->filter_fields&INGRESS_STREAM_ID_BIT) PROC_PRINTF("ingress_stream_id:  %d\n	ingress_stream_id_mask:  0x%x\n", aclPara->ingress_stream_id, aclPara->ingress_stream_id_mask); 
			if(aclPara->filter_fields&INGRESS_ETHERTYPE_BIT) PROC_PRINTF("ingress_ethertype:0x%x  ingress_ethertype_mask:0x%x\n", aclPara->ingress_ethertype, aclPara->ingress_ethertype_mask); 		
			if(aclPara->filter_fields&INGRESS_CTAG_VID_BIT) PROC_PRINTF("ingress_ctag_vid: %d\n", aclPara->ingress_ctag_vid);	
			if(aclPara->filter_fields&INGRESS_CTAG_PRI_BIT) PROC_PRINTF("ingress_ctag_pri: %d\n", aclPara->ingress_ctag_pri);	
			if(aclPara->filter_fields&INGRESS_CTAG_CFI_BIT) PROC_PRINTF("ingress_ctag_cfi: %d\n", aclPara->ingress_ctag_cfi);
			if(aclPara->filter_fields&INGRESS_STAG_VID_BIT) PROC_PRINTF("ingress_stag_vid: %d\n", aclPara->ingress_stag_vid);	
			if(aclPara->filter_fields&INGRESS_STAG_PRI_BIT) PROC_PRINTF("ingress_stag_pri: %d\n", aclPara->ingress_stag_pri);
			if(aclPara->filter_fields&INGRESS_STAG_DEI_BIT) PROC_PRINTF("ingress_stag_dei: %d\n", aclPara->ingress_stag_dei);
			if(aclPara->filter_fields&INGRESS_SMAC_BIT) PROC_PRINTF("ingress_smac: %02X:%02X:%02X:%02X:%02X:%02X ingress_smac_mask: %02X:%02X:%02X:%02X:%02X:%02X\n", 
																			aclPara->ingress_smac.octet[0],
																			aclPara->ingress_smac.octet[1],
																			aclPara->ingress_smac.octet[2],
																			aclPara->ingress_smac.octet[3],
																			aclPara->ingress_smac.octet[4],
																			aclPara->ingress_smac.octet[5],
																			aclPara->ingress_smac_mask.octet[0],
																			aclPara->ingress_smac_mask.octet[1],
																			aclPara->ingress_smac_mask.octet[2],
																			aclPara->ingress_smac_mask.octet[3],
																			aclPara->ingress_smac_mask.octet[4],
																			aclPara->ingress_smac_mask.octet[5]);			
			if(aclPara->filter_fields&INGRESS_DMAC_BIT) PROC_PRINTF("ingress_dmac: %02X:%02X:%02X:%02X:%02X:%02X  ingress_dmac_mask: %02X:%02X:%02X:%02X:%02X:%02X\n",
																			aclPara->ingress_dmac.octet[0],
																			aclPara->ingress_dmac.octet[1],
																			aclPara->ingress_dmac.octet[2],
																			aclPara->ingress_dmac.octet[3],
																			aclPara->ingress_dmac.octet[4],
																			aclPara->ingress_dmac.octet[5],
																			aclPara->ingress_dmac_mask.octet[0],
																			aclPara->ingress_dmac_mask.octet[1],
																			aclPara->ingress_dmac_mask.octet[2],
																			aclPara->ingress_dmac_mask.octet[3],
																			aclPara->ingress_dmac_mask.octet[4],
																			aclPara->ingress_dmac_mask.octet[5]);

			acl_parameter = &rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter;
			if(aclPara->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
				PROC_PRINTF("ingress_sip_low_bound: %s	", inet_ntoa(acl_parameter->ingress_src_ipv4_addr_start));
				PROC_PRINTF("ingress_sip_up_bound:	%s \n", inet_ntoa(acl_parameter->ingress_src_ipv4_addr_end));
			}
			if(aclPara->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
				PROC_PRINTF("ingress_dip_low_bound: %s	", inet_ntoa(acl_parameter->ingress_dest_ipv4_addr_start));
				PROC_PRINTF("ingress_dip_up_bound:	%s \n", inet_ntoa(acl_parameter->ingress_dest_ipv4_addr_end));
			}

			if(aclPara->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
				PROC_PRINTF("ingress_src_ipv6_addr_start: %02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[0],acl_parameter->ingress_src_ipv6_addr_start[1]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[2],acl_parameter->ingress_src_ipv6_addr_start[3]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[4],acl_parameter->ingress_src_ipv6_addr_start[5]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[6],acl_parameter->ingress_src_ipv6_addr_start[7]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[8],acl_parameter->ingress_src_ipv6_addr_start[9]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[10],acl_parameter->ingress_src_ipv6_addr_start[11]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[12],acl_parameter->ingress_src_ipv6_addr_start[13]);
				PROC_PRINTF(":%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr_start[14],acl_parameter->ingress_src_ipv6_addr_start[15]);

				PROC_PRINTF("ingress_src_ipv6_addr_end: %02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[0],acl_parameter->ingress_src_ipv6_addr_end[1]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[2],acl_parameter->ingress_src_ipv6_addr_end[3]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[4],acl_parameter->ingress_src_ipv6_addr_end[5]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[6],acl_parameter->ingress_src_ipv6_addr_end[7]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[8],acl_parameter->ingress_src_ipv6_addr_end[9]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[10],acl_parameter->ingress_src_ipv6_addr_end[11]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[12],acl_parameter->ingress_src_ipv6_addr_end[13]);
				PROC_PRINTF(":%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr_end[14],acl_parameter->ingress_src_ipv6_addr_end[15]);
			}

			if(aclPara->filter_fields&INGRESS_IPV6_DIP_RANGE_BIT){
				PROC_PRINTF("ingress_dest_ipv6_addr_start: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[0],acl_parameter->ingress_dest_ipv6_addr_start[1]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[2],acl_parameter->ingress_dest_ipv6_addr_start[3]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[4],acl_parameter->ingress_dest_ipv6_addr_start[5]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[6],acl_parameter->ingress_dest_ipv6_addr_start[7]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[8],acl_parameter->ingress_dest_ipv6_addr_start[9]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[10],acl_parameter->ingress_dest_ipv6_addr_start[11]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[12],acl_parameter->ingress_dest_ipv6_addr_start[13]);
				PROC_PRINTF(":%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_start[14],acl_parameter->ingress_dest_ipv6_addr_start[15]);

				PROC_PRINTF("ingress_dest_ipv6_addr_end: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[0],acl_parameter->ingress_dest_ipv6_addr_end[1]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[2],acl_parameter->ingress_dest_ipv6_addr_end[3]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[4],acl_parameter->ingress_dest_ipv6_addr_end[5]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[6],acl_parameter->ingress_dest_ipv6_addr_end[7]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[8],acl_parameter->ingress_dest_ipv6_addr_end[9]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[10],acl_parameter->ingress_dest_ipv6_addr_end[11]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[12],acl_parameter->ingress_dest_ipv6_addr_end[13]);
				PROC_PRINTF(":%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_end[14],acl_parameter->ingress_dest_ipv6_addr_end[15]);
			}

			if(aclPara->filter_fields&INGRESS_IPV6_SIP_BIT){
				PROC_PRINTF("ingress_src_ipv6_addr: %02X%02X:",acl_parameter->ingress_src_ipv6_addr[0],acl_parameter->ingress_src_ipv6_addr[1]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr[2],acl_parameter->ingress_src_ipv6_addr[3]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr[4],acl_parameter->ingress_src_ipv6_addr[5]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr[6],acl_parameter->ingress_src_ipv6_addr[7]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr[8],acl_parameter->ingress_src_ipv6_addr[9]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr[10],acl_parameter->ingress_src_ipv6_addr[11]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr[12],acl_parameter->ingress_src_ipv6_addr[13]);
				PROC_PRINTF(":%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr[14],acl_parameter->ingress_src_ipv6_addr[15]);

				PROC_PRINTF("ingress_src_ipv6_addr_mask: %02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[0],acl_parameter->ingress_src_ipv6_addr_mask[1]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[2],acl_parameter->ingress_src_ipv6_addr_mask[3]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[4],acl_parameter->ingress_src_ipv6_addr_mask[5]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[6],acl_parameter->ingress_src_ipv6_addr_mask[7]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[8],acl_parameter->ingress_src_ipv6_addr_mask[9]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[10],acl_parameter->ingress_src_ipv6_addr_mask[11]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[12],acl_parameter->ingress_src_ipv6_addr_mask[13]);
				PROC_PRINTF(":%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr_mask[14],acl_parameter->ingress_src_ipv6_addr_mask[15]);
			}

			if(aclPara->filter_fields&INGRESS_IPV6_DIP_BIT){
				PROC_PRINTF("ingress_dest_ipv6_addr: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr[0],acl_parameter->ingress_dest_ipv6_addr[1]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[2],acl_parameter->ingress_dest_ipv6_addr[3]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[4],acl_parameter->ingress_dest_ipv6_addr[5]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[6],acl_parameter->ingress_dest_ipv6_addr[7]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[8],acl_parameter->ingress_dest_ipv6_addr[9]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[10],acl_parameter->ingress_dest_ipv6_addr[11]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[12],acl_parameter->ingress_dest_ipv6_addr[13]);
				PROC_PRINTF(":%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr[14],acl_parameter->ingress_dest_ipv6_addr[15]);

				PROC_PRINTF("ingress_dest_ipv6_addr_mask: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[0],acl_parameter->ingress_dest_ipv6_addr_mask[1]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[2],acl_parameter->ingress_dest_ipv6_addr_mask[3]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[4],acl_parameter->ingress_dest_ipv6_addr_mask[5]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[6],acl_parameter->ingress_dest_ipv6_addr_mask[7]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[8],acl_parameter->ingress_dest_ipv6_addr_mask[9]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[10],acl_parameter->ingress_dest_ipv6_addr_mask[11]);
				PROC_PRINTF(":%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[12],acl_parameter->ingress_dest_ipv6_addr_mask[13]);
				PROC_PRINTF(":%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_mask[14],acl_parameter->ingress_dest_ipv6_addr_mask[15]);
			}		

			if(aclPara->filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
				PROC_PRINTF("ingress_src_l4_port_low_bound: %d	", acl_parameter->ingress_src_l4_port_start);
				PROC_PRINTF("ingress_src_l4_port_up_bound:	%d \n", acl_parameter->ingress_src_l4_port_end);
			}
			if(aclPara->filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
				PROC_PRINTF("ingress_dest_l4_port_low_bound: %d  ", acl_parameter->ingress_dest_l4_port_start);
				PROC_PRINTF("ingress_dest_l4_port_up_bound:  %d \n", acl_parameter->ingress_dest_l4_port_end);
			}

			if(aclPara->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT){
				PROC_PRINTF("egress_sip_low_bound: %s  ", diag_util_inet_ntoa(acl_parameter->egress_src_ipv4_addr_start));
				PROC_PRINTF("egress_sip_up_bound:  %s \n", diag_util_inet_ntoa(acl_parameter->egress_src_ipv4_addr_end));
			}
			if(aclPara->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
				PROC_PRINTF("egress_dip_low_bound: %s  ", diag_util_inet_ntoa(acl_parameter->egress_dest_ipv4_addr_start));
				PROC_PRINTF("egress_dip_up_bound:  %s \n", diag_util_inet_ntoa(acl_parameter->egress_dest_ipv4_addr_end));
			}
			if(aclPara->filter_fields&EGRESS_L4_SPORT_RANGE_BIT){
				PROC_PRINTF("egress_src_l4_port_low_bound: %d  ", acl_parameter->egress_src_l4_port_start);
				PROC_PRINTF("egress_src_l4_port_up_bound:  %d \n", acl_parameter->egress_src_l4_port_end);
			}
			if(aclPara->filter_fields&EGRESS_L4_DPORT_RANGE_BIT){
				PROC_PRINTF("egress_dest_l4_port_low_bound: %d	", acl_parameter->egress_dest_l4_port_start);
				PROC_PRINTF("egress_dest_l4_port_up_bound:	%d \n", acl_parameter->egress_dest_l4_port_end);
			}

			if(aclPara->filter_fields&EGRESS_CTAG_VID_BIT){ 
				PROC_PRINTF("egress_ctag_vid:  %d \n", acl_parameter->egress_ctag_vid);
				PROC_PRINTF("egress_ctag_vid_mask:	%d \n", acl_parameter->egress_ctag_vid_mask);
			}
			
			if(aclPara->filter_fields&EGRESS_CTAG_PRI_BIT) PROC_PRINTF("egress_ctag_pri:  %d \n", acl_parameter->egress_ctag_pri);


			if(acl_parameter->filter_fields & INGRESS_IPV4_TAGIF_BIT){
				PROC_PRINTF("ipv4_tagIf: %s \n",acl_parameter->ingress_ipv4_tagif?"Must be IPv4":"Must not be IPv4");
			}

			if(acl_parameter->filter_fields & INGRESS_IPV6_TAGIF_BIT){
				PROC_PRINTF("ipv6_tagIf: %s \n",acl_parameter->ingress_ipv6_tagif?"Must be IPv6":"Must not be IPv6");
			}

			
			if(acl_parameter->filter_fields & INGRESS_L4_TCP_BIT){
				PROC_PRINTF("l4-protocal: tcp \n");
			}else if(acl_parameter->filter_fields & INGRESS_L4_UDP_BIT){
				PROC_PRINTF("l4-protocal: udp \n");
			}else if(acl_parameter->filter_fields & INGRESS_L4_ICMP_BIT){
				PROC_PRINTF("l4-protocal: icmp \n");
			}else{
				PROC_PRINTF("l4-protocal: not care \n");
			}

			if(acl_parameter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT){
				PROC_PRINTF("l4-protocal-value: %d \n",acl_parameter->ingress_l4_protocal);
			}

			if(acl_parameter->filter_fields & INGRESS_STAGIF_BIT){
				if(aclPara->ingress_stagIf){
					PROC_PRINTF("stagIf: must have Stag \n");
				}else{
					PROC_PRINTF("stagIf: must not have Stag \n");
				}
			}
			if(acl_parameter->filter_fields & INGRESS_CTAGIF_BIT){
				if(aclPara->ingress_ctagIf){
					PROC_PRINTF("ctagIf: must have Ctag \n");
				}else{
					PROC_PRINTF("ctagIf: must not have Ctag \n");
				}
			}

			if(acl_parameter->filter_fields & INGRESS_WLANDEV_BIT){
				PROC_PRINTF("ingress_wlanDevMask: %d \n",acl_parameter->ingress_wlanDevMask);
			}
			
			PROC_PRINTF("[Actions]: \n");
			switch(acl_parameter->action_type){
				case ACL_ACTION_TYPE_DROP:
					PROC_PRINTF("action_type: ACL_ACTION_TYPE_DROP \n");
					break;
				case ACL_ACTION_TYPE_PERMIT:
					PROC_PRINTF("action type: ACL_ACTION_TYPE_PERMIT \n");
					break;
				case ACL_ACTION_TYPE_TRAP:
					PROC_PRINTF("action type: ACL_ACTION_TYPE_TRAP \n");
					break;
				case ACL_ACTION_TYPE_TRAP_TO_PS:
					PROC_PRINTF("action type: ACL_ACTION_TYPE_TRAP_TO_PS \n");
					break;
				case ACL_ACTION_TYPE_QOS:
					PROC_PRINTF("action type: ACL_ACTION_TYPE_QOS \n");
					PROC_PRINTF("qos_actions_bits: 0x%x\n",acl_parameter->qos_actions);
					if(acl_parameter->qos_actions&ACL_ACTION_1P_REMARKING_BIT) PROC_PRINTF("dot1p_remarking: %d \n",acl_parameter->action_dot1p_remarking_pri);
					if(acl_parameter->qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT) PROC_PRINTF("ip_precedence_remarking: %d\n",acl_parameter->action_ip_precedence_remarking_pri);
					if(acl_parameter->qos_actions&ACL_ACTION_DSCP_REMARKING_BIT) PROC_PRINTF("dscp_remarking: %d\n",acl_parameter->action_dscp_remarking_pri);
					if(acl_parameter->qos_actions&ACL_ACTION_QUEUE_ID_BIT) PROC_PRINTF("queue_id: %d\n",acl_parameter->action_queue_id);
					if(acl_parameter->qos_actions&ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT) PROC_PRINTF("egress_internal_priority(cfpri): %d\n",acl_parameter->egress_internal_priority);
					if(acl_parameter->qos_actions&ACL_ACTION_SHARE_METER_BIT) PROC_PRINTF("share_meter: %d\n",acl_parameter->action_share_meter);	
					if(acl_parameter->qos_actions&ACL_ACTION_STREAM_ID_OR_LLID_BIT) PROC_PRINTF("stream id: %d\n",acl_parameter->action_stream_id_or_llid); 
					if(acl_parameter->qos_actions&ACL_ACTION_ACL_PRIORITY_BIT) PROC_PRINTF("acl priority: %d\n",acl_parameter->action_acl_priority);	
					if(acl_parameter->qos_actions&ACL_ACTION_ACL_INGRESS_VID_BIT) PROC_PRINTF("ingress cvid: %d\n",acl_parameter->action_acl_ingress_vid);	
					if(acl_parameter->qos_actions&ACL_ACTION_REDIRECT_BIT) PROC_PRINTF("redirect to portmaks: 0x%x\n",acl_parameter->redirect_portmask);	
					if(acl_parameter->qos_actions&ACL_ACTION_ACL_CVLANTAG_BIT){//only print CVID/CPRI  assign or internal case
						PROC_PRINTF("modify CVLAN: ");
						if(acl_parameter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING){
							PROC_PRINTF("[add CTag] ");
							//CVID
							if(acl_parameter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_ASSIGN){
								PROC_PRINTF("assign CVID=%d, ",acl_parameter->action_acl_cvlan.assignedCvid);
							}else if(acl_parameter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID){
								PROC_PRINTF("assign CVID from internal, ");
							}
							//CPRI
							if(acl_parameter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_ASSIGN){
								PROC_PRINTF("assign CPRI=%d\n",acl_parameter->action_acl_cvlan.assignedCpri);
							}else if(acl_parameter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI){
								PROC_PRINTF("assign CPRI from internal\n");
							}
						}
					}
					if(acl_parameter->qos_actions&ACL_ACTION_ACL_SVLANTAG_BIT){//only print SVID/SPRI  assign case
						PROC_PRINTF("modify SVLAN: ");
						if(acl_parameter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID || acl_parameter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_8100){
							//SVID
							if(acl_parameter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_ASSIGN){
								PROC_PRINTF("assign SVID=%d, ",acl_parameter->action_acl_svlan.assignedSvid);
							}
							
							//SPRI
							if(acl_parameter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_ASSIGN){
								PROC_PRINTF("assign SPRI=%d\n",acl_parameter->action_acl_svlan.assignedSpri);
							}else if(acl_parameter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI){
								PROC_PRINTF("assign SPRI from internal\n");
							}
						}
					}
					break;
				case ACL_ACTION_TYPE_POLICY_ROUTE:
					PROC_PRINTF("action type: ACL_ACTION_TYPE_POLICY_ROUTE \n");
					PROC_PRINTF("policy_route_wan: 0x%x\n",acl_parameter->action_policy_route_wan);
				default:
					break;
			}
		
		}

	}
#endif

	return RT_ERR_RG_OK;
}
int _dump_rg_cf(struct seq_file *s)
{
	FIXME("XDSL FIXME _dump_rg_cf latter\n");
	/*dump CF[64-511]*/
	return RT_ERR_RG_OK;
}

int _dump_rg_acl_and_cf_diagshell(struct seq_file *s)
{
	FIXME("XDSL FIXME _dump_rg_acl_and_cf_diagshell latter\n");
	return RT_ERR_RG_OK;
}

#include <net/rtl/rtl865x_netif.h>
extern void rtl865x_showACL(rtl865x_AclRule_t *rule);
int32 _dump_xdsl_acl(void)
{
	rtl865x_AclRule_t asic_acl;
	rtl865x_tblAsicDrv_intfParam_t asic_intf;
	uint32 acl_start, acl_end;

	uint16 vid;
	int8 outRule;
	
	printk("%s\n", "ASIC ACL Table:");
	for(vid=0; vid<8; vid++ ) 
	{
		/* Read VLAN Table */
		if (rtl8651_getAsicNetInterface(vid, &asic_intf) == FAILED)
			continue;
		if (asic_intf.valid==FALSE)
			continue;

		outRule = FALSE;
showACL:
		if(outRule)
		{
			acl_start = asic_intf.outAclStart; acl_end = asic_intf.outAclEnd;
		}
		else
		{
			acl_start = asic_intf.inAclStart; acl_end = asic_intf.inAclEnd;
		}
		printk("	\nacl_start(%d), acl_end(%d)", acl_start, acl_end);

		if (outRule == FALSE)
			printk("\n<<Ingress Rule for Netif  %d: (VID %d)>>\n", vid,asic_intf.vid);
		else
			printk("\n<<Egress Rule for Netif %d (VID %d)>>:\n", vid,asic_intf.vid);

		for(; acl_start<= acl_end;acl_start++)
		{
			if ( _rtl865x_getAclFromAsic(acl_start, &asic_acl) == FAILED)
				rtlglue_printf("=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);
			if(asic_acl.pktOpApp_ == 0)
				continue;

			rtl865x_showACL(&asic_acl);
		}	

		if(outRule==FALSE)
		{
			outRule = TRUE;
			goto showACL;
		}
		else
			break;
	}
	return 0;
}




