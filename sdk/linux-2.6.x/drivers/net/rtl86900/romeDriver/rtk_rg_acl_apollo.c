#include <rtk_rg_acl_apollo.h>
#include <rtk_rg_acl.h>

//#include <rtk_rg_struct.h>
//#include <rtk_rg_internal.h>
#include <dal/apollomp/raw/apollomp_raw_hwmisc.h>

extern int32 ioal_mem32_read(uint32 addr, uint32 *pVal);
extern int32 ioal_mem32_write(uint32 addr, uint32 val);

int _dump_rg_acl_entry_content(struct seq_file *s, rtk_rg_aclFilterAndQos_t *aclPara);

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

rtk_rg_intfInfo_t ingress_intf_info, egress_intf_info, empty_intf_info;
int flag_add_aclRule[MAX_ACL_TEMPLATE_SIZE]={DISABLE};
rtk_acl_ingress_entry_t aclRule[MAX_ACL_TEMPLATE_SIZE];
rtk_acl_field_t aclField[RTK_MAX_NUM_OF_ACL_RULE_FIELD*MAX_ACL_TEMPLATE_SIZE];
rtk_acl_igr_act_t acl_action;
rtk_acl_field_t empty_aclField;
rtk_classify_field_t classifyField_1[CLASSIFY_FIELD_END];
rtk_classify_field_t classifyField_2[CLASSIFY_FIELD_END];
rtk_classify_cfg_t cfRule_1, cfRule_2; //cfRule_1 for upstream. cfRule_2 for downstream
rtk_classify_field_t empty_classifyField;
rtk_rg_aclFilterEntry_t empty_aclSWEntry;
rtk_acl_rangeCheck_ip_t aclSIPv4RangeEntry,aclDIPv4RangeEntry,aclSIPv6RangeEntry,aclDIPv6RangeEntry;
rtk_acl_rangeCheck_l4Port_t aclSportRangeEntry,aclDportRangeEntry;
rtk_classify_rangeCheck_ip_t cfIpRangeEntry;
rtk_classify_rangeCheck_l4Port_t cfPortRangeEntry;
rtk_rg_aclFilterEntry_t aclSWEntry;
rtk_rg_aclFilterAndQos_t empty_aclFilter;


#if 0
rtk_rg_flow_direction_t flow_direction;
#else
rtk_rg_acl_fwding_type_direction_t flow_direction;
#endif
rtk_classify_field_t pClassifyField_acl;


#if 1 /*****(1)ACL init related APIs*****/
#endif


#if defined(CONFIG_APOLLO)
int _rtk_rg_acl_asic_init(void)
{
    int i=0,ret;

	rtk_filter_unmatch_action_type_t pAction;
	rtk_acl_template_t aclTemplate;
    rtk_acl_rangeCheck_ip_t iprangeEntry;
    rtk_acl_rangeCheck_l4Port_t prtRangeEntry;
#ifdef CONFIG_APOLLO_MODEL
#else
		rtk_acl_field_entry_t fieldSel;
#endif

#if defined(CONFIG_RTL9600_SERIES)
	rtk_acl_igr_rule_mode_t pMode;

    //set ACL_MODE
    pMode = ACL_IGR_RULE_MODE_0;
    ASSERT_EQ(rtk_acl_igrRuleMode_set(pMode),RT_ERR_OK);
#endif	

    //set ACL_EN
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT0,ENABLED),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT1,ENABLED),RT_ERR_OK);
#if !defined(CONFIG_RTL9602C_SERIES)	
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT2,ENABLED),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT3,ENABLED),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT_RGMII,ENABLED),RT_ERR_OK);	
#endif	
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT_PON,ENABLED),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT_CPU,ENABLED),RT_ERR_OK);

    //set ACL_PERMIT
    pAction = FILTER_UNMATCH_PERMIT;
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT0,pAction),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT1,pAction),RT_ERR_OK);
#if !defined(CONFIG_RTL9602C_SERIES)	
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT2,pAction),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT3,pAction),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT_RGMII,pAction),RT_ERR_OK);
#endif	
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT_PON,pAction),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT_CPU,pAction),RT_ERR_OK);

 
    /*init TEMPLATE & field selector*/

#ifdef CONFIG_APOLLO_MODEL
#else

#ifdef CONFIG_EPON_FEATURE
	if(!rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//setup field selector[03] for the 2 byte after ethertype (EPON ISSUE)
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 3;
		fieldSel.format = ACL_FORMAT_RAW;
		fieldSel.offset = 14;//pass DA, SA , Ether_Type,
		if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
			return ret;
	}
	else
#endif
	{ 	
		//setup field selector[03] for IPv6 Src[128:121]
	    bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 3;
		fieldSel.format = ACL_FORMAT_IPV6;
		fieldSel.offset = 8;
		if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
			return ret;
	} 

	//setup field selector[04] for aclFilter "DSCP"
    bzero(&fieldSel,sizeof(fieldSel));
    fieldSel.index = 4;
    fieldSel.format = ACL_FORMAT_IPV4;
    fieldSel.offset = 0;//will get 16 bits (version(4) + headerLength(4) + Tos(8))
    if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
        return ret;

#ifdef CONFIG_RG_PPPOE_PASSTHROUGH
	//setup field selector[15] for liteRomeDriver "PPPoE SessionID"
	bzero(&fieldSel,sizeof(fieldSel));
	fieldSel.index = 15;
	fieldSel.format = ACL_FORMAT_RAW;
	fieldSel.offset = 16;//pass DA, SA , Ether_Type, PPPoE Header(0x8864), PPPoE Ethernet frame
	if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
		return ret;
#else
	//If PPPoE Passthrought  no  needed, setup field selector[15] for IPv6 DSCP
	bzero(&fieldSel,sizeof(fieldSel));
	fieldSel.index = 15;
	fieldSel.format = ACL_FORMAT_IPV6;
	fieldSel.offset = 0;//will get 16 bits (version(4) + TC(8) + Flow Lebel[19:16](4))
	if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
		return ret;
#endif


#endif


#ifdef CONFIG_RG_ACL_V6SIP_FILTER_BUT_DISABLE_V6ROUTING
	//setup field selector[8~13] from IPv6 DIP[127:32] to SIP[127:32]
	//Warning: if open CONFIG_RG_ACL_V6SIP_FILTER_BUT_DISABLE_V6ROUTING, the IPv6 Routing can not be used because HW without IPv6 DIP information!
	bzero(&fieldSel,sizeof(fieldSel));
	fieldSel.index = 8;
	fieldSel.format = ACL_FORMAT_IPV6;
	fieldSel.offset = 18;
	if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
		return ret;

	bzero(&fieldSel,sizeof(fieldSel));
	fieldSel.index = 9;
	fieldSel.format = ACL_FORMAT_IPV6;
	fieldSel.offset = 16;
	if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
		return ret;

	bzero(&fieldSel,sizeof(fieldSel));
	fieldSel.index = 10;
	fieldSel.format = ACL_FORMAT_IPV6;
	fieldSel.offset = 14;
	if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
		return ret;

	bzero(&fieldSel,sizeof(fieldSel));
	fieldSel.index = 11;
	fieldSel.format = ACL_FORMAT_IPV6;
	fieldSel.offset = 12;
	if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
		return ret;

	bzero(&fieldSel,sizeof(fieldSel));
	fieldSel.index = 12;
	fieldSel.format = ACL_FORMAT_IPV6;
	fieldSel.offset = 10;
	if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
		return ret;

	bzero(&fieldSel,sizeof(fieldSel));
	fieldSel.index = 13;
	fieldSel.format = ACL_FORMAT_IPV6;
	fieldSel.offset = 8;
	if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
		return ret;

#ifdef CONFIG_EPON_FEATURE
	if(!rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//setup field selector[03] for the 2 byte after ethertype (EPON ISSUE)
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 3;
		fieldSel.format = ACL_FORMAT_RAW;
		fieldSel.offset = 14;//pass DA, SA , Ether_Type,
		if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
			return ret;
	}
	else
#endif
	{
		//setup field selector[03] for IPv6 Dest[128:121]
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 3;
		fieldSel.format = ACL_FORMAT_IPV6;
		fieldSel.offset = 24;
		if((ret = rtk_acl_fieldSelect_set(&fieldSel))!= RT_ERR_OK)
			return ret;
	} 
#endif




    //aclTemplate[0]
    bzero((void*) &aclTemplate, sizeof(aclTemplate));
    aclTemplate.index = 0;
    aclTemplate.fieldType[0] = ACL_FIELD_DMAC0;
    aclTemplate.fieldType[1] = ACL_FIELD_DMAC1;
    aclTemplate.fieldType[2] = ACL_FIELD_DMAC2;
	aclTemplate.fieldType[3] = ACL_FIELD_EXT_PORTMASK;
    aclTemplate.fieldType[4] = ACL_FIELD_SMAC0;
    aclTemplate.fieldType[5] = ACL_FIELD_SMAC1;
    aclTemplate.fieldType[6] = ACL_FIELD_SMAC2;
    aclTemplate.fieldType[7] = ACL_FIELD_ETHERTYPE;
    if((ret = rtk_acl_template_set(&aclTemplate))!= RT_ERR_OK)
        return ret;

	//rtlglue_printf("%s(%d):set template[0] done \n",__func__,__LINE__);



    //aclTemplate[1]
    bzero((void*) &aclTemplate, sizeof(aclTemplate));
    aclTemplate.index = 1;
    aclTemplate.fieldType[0] = ACL_FIELD_USER_DEFINED01;//l4_dport
    aclTemplate.fieldType[1] = ACL_FIELD_IPV4_SIP0;
    aclTemplate.fieldType[2] = ACL_FIELD_IPV4_SIP1;
    aclTemplate.fieldType[3] = ACL_FIELD_USER_DEFINED00;//l4_sport
    aclTemplate.fieldType[4] = ACL_FIELD_USER_DEFINED14;//IP_protocal
    aclTemplate.fieldType[5] = ACL_FIELD_PORT_RANGE;
    aclTemplate.fieldType[6] = ACL_FIELD_IPV4_DIP0;
    aclTemplate.fieldType[7] = ACL_FIELD_IPV4_DIP1;
    if((ret = rtk_acl_template_set(&aclTemplate))!= RT_ERR_OK)
        return ret;
	
    //rtlglue_printf("%s(%d):set template[1] done \n",__func__,__LINE__);



#ifdef CONFIG_APOLLO_MODEL
#else
	//aclTemplate[2]
    bzero((void*) &aclTemplate, sizeof(aclTemplate));
    aclTemplate.index = 2;
    aclTemplate.fieldType[0] = ACL_FIELD_CTAG;
    aclTemplate.fieldType[1] = ACL_FIELD_GEMPORT; //Abel team patch
    aclTemplate.fieldType[2] = ACL_FIELD_STAG;
    aclTemplate.fieldType[3] = ACL_FIELD_USER_DEFINED03;//SIPv6[127:112]
    aclTemplate.fieldType[4] = ACL_FIELD_USER_DEFINED04;//DSCP
#ifdef CONFIG_RG_ACL_V6SIP_FILTER_BUT_DISABLE_V6ROUTING 
	aclTemplate.fieldType[5] = ACL_FIELD_IPV6_DIP1;//DIPv6[31:16]
	aclTemplate.fieldType[6] = ACL_FIELD_IPV6_DIP0;//DIPv6[15:0]
#else
	aclTemplate.fieldType[5] = ACL_FIELD_IPV6_SIP1;//SIPv6[31:16]
	aclTemplate.fieldType[6] = ACL_FIELD_IPV6_SIP0;//SIPv6[15:0]
#endif
    aclTemplate.fieldType[7] = ACL_FIELD_USER_DEFINED15;//SessionID
    if((ret = rtk_acl_template_set(&aclTemplate))!= RT_ERR_OK)
        return ret;

#endif

#ifdef CONFIG_APOLLO_MODEL
#else
	//aclTemplate[3]
    bzero((void*) &aclTemplate, sizeof(aclTemplate));
    aclTemplate.index = 3;
    aclTemplate.fieldType[0] = ACL_FIELD_USER_DEFINED08;//DIPv6[47:32]
    aclTemplate.fieldType[1] = ACL_FIELD_USER_DEFINED09;//DIPv6[63:48]
    aclTemplate.fieldType[2] = ACL_FIELD_USER_DEFINED10;//DIPv6[79:64]
    aclTemplate.fieldType[3] = ACL_FIELD_USER_DEFINED11;//DIPv6[95:80]
    aclTemplate.fieldType[4] = ACL_FIELD_USER_DEFINED12;//DIPv6[111:96]
    aclTemplate.fieldType[5] = ACL_FIELD_USER_DEFINED13;//DIPv6[127:112] 
#ifdef CONFIG_RG_ACL_V6SIP_FILTER_BUT_DISABLE_V6ROUTING 
	aclTemplate.fieldType[6] = ACL_FIELD_IPV6_SIP1;//SIPv6[31:16]
    aclTemplate.fieldType[7] = ACL_FIELD_IPV6_SIP0;//SIPv6[15:0] 
#else
    aclTemplate.fieldType[6] = ACL_FIELD_IPV6_DIP1;//DIPv6[31:16]
    aclTemplate.fieldType[7] = ACL_FIELD_IPV6_DIP0;//DIPv6[15:0] 
#endif
    if((ret = rtk_acl_template_set(&aclTemplate))!= RT_ERR_OK)
        return ret;
#endif

#if 0

	rtlglue_printf("%s(%d):dump fieldSelect:\n",__func__,__LINE__);
	for(i=0;i<16;i++){
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = i;
        if((ret = rtk_acl_fieldSelect_get(&fieldSel))!= RT_ERR_OK)
            return ret;
		else
			rtlglue_printf("%s(%d):fieldSelect[%d]:format=%d  offset=%d \n",__func__,__LINE__,fieldSel.index,fieldSel.format,fieldSel.offset);
	}
#endif


    /*init IP_RNG_TABLE*/
    bzero((void*) &iprangeEntry, sizeof(iprangeEntry));
    for(i =0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
    {
        iprangeEntry.index=i;
        if((ret = rtk_acl_ipRange_set(&iprangeEntry))!= RT_ERR_OK)
            return ret;
    }


    /*init PORT_RNG_TABLE*/
    bzero((void*) &prtRangeEntry, sizeof(prtRangeEntry));
    for(i =0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
    {
        prtRangeEntry.index=i;
        if((ret = rtk_acl_portRange_set(&prtRangeEntry))!= RT_ERR_OK)
            return ret;
    }

#if 0
#ifdef __KERNEL__
//#ifdef CONFIG_RG_PPPOE_AND_VALN_ISSUE_PATCH
	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
	{
		assert_ok(_rtk_rg_acl_reserved_pppoeCvidIssue_svid2IngressCvid(RTK_RG_PORT_PON));
	}
//#endif
#endif
#endif

	//assert_ok(_rtk_rg_acl_reserved_multicastVidTranslate());
#if 0
	/*init default rule for change multicast packet to vid=1 (Let it across lan/wan)*/
	{
		rtk_acl_ingress_entry_t aclRule;
		rtk_acl_field_t aclField1;
		rtk_acl_field_t aclField2;

		//add acl[50] for reamrking multicast packet to vid=1 
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField1,sizeof(aclField1));
		bzero(&aclField2,sizeof(aclField2));
		aclRule.valid=ENABLED;
		aclRule.index=RESERVED_ACL_MULTICAST_VID_TRANSLATE;
			
		aclField1.fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField1.fieldUnion.pattern.fieldIdx = 1;//template[0],field[1] = DA[31:16]
		aclField1.fieldUnion.data.value=0x5e00;
		aclField1.fieldUnion.data.mask=0xff00;
		assert_ok(rtk_acl_igrRuleField_add(&aclRule, &aclField1));

		aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField2.fieldUnion.pattern.fieldIdx = 2;//template[0],field[2] = DA[47:32]
		aclField2.fieldUnion.data.value=0x0100;
		aclField2.fieldUnion.data.mask=0xffff;
		assert_ok(rtk_acl_igrRuleField_add(&aclRule, &aclField2));

			
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
		aclRule.templateIdx=0; /*use DA: template[],field[0~2]*/
		aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
		aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
		aclRule.act.cvlanAct.cvid=1; //remark to 1
		assert_ok(rtk_acl_igrRuleEntry_add(&aclRule));		

	}
#endif

    return (RT_ERR_RG_OK);

}


int _rtk_rg_classify_asic_init(void)
{
#if defined(CONFIG_RTL9600_SERIES)	

    int i,ret;
	rtk_classify_rangeCheck_l4Port_t prtRngEntry;
    rtk_classify_rangeCheck_ip_t ipRngEntry;
    /*clear port_range_check table*/
	
    bzero((void*) &prtRngEntry, sizeof(prtRngEntry));
    for(i =0; i<MAX_CF_PORTRANGETABLE_SIZE; i++)
    {
        prtRngEntry.index=i;
        prtRngEntry.lowerPort= 0xffff;
        if((ret = rtk_classify_portRange_set(&prtRngEntry))!= RT_ERR_OK)
            return ret;
    }


    /*clear ip_range_check table*/
    bzero((void*) &ipRngEntry, sizeof(ipRngEntry));
    ipRngEntry.lowerIp=0xffffffff;
    for(i =0; i<MAX_CF_IPRANGETABLE_SIZE; i++)
    {
        ipRngEntry.index=i;

        if((ret = rtk_classify_ipRange_set(&ipRngEntry))!= RT_ERR_OK)
            return ret;
    }

	/*clear cfPri2Dscp table*/
	for(i =0; i<MAX_CF_DSCPTABLE_SIZE; i++)
	{		   
		if((ret = rtk_classify_cfPri2Dscp_set(i,0))!= RT_ERR_OK)
			return ret;
	}

#else
	FIXME("must clear egress ip/port range when initial.");
	FIXME("apollo fe don't have cfPri2Dscp");
#endif		

    return (RT_ERR_RG_OK);
}
#endif

#if 1 /*****(2)RG_ACL APIs and internal APIs*****/
#endif

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

static int _rtk_rg_search_acl_empty_Entry(int size, int* index)
{
    int i=0;
    int continue_size=0;
    rtk_acl_ingress_entry_t aclRule;
    bzero(&aclRule, sizeof(aclRule));

    if(size == 0) //no need acl entry
    {
        return (RT_ERR_RG_OK);
    }
    for(i=MIN_ACL_ENTRY_INDEX; i<MAX_ACL_ENTRY_INDEX; i++)
    {
        aclRule.index = i;
        if(rtk_acl_igrRuleEntry_get(&aclRule))
        {
            DEBUG("get acl[%d] failed",i);
            RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
        }
        if(aclRule.valid==DISABLED)
        {

            continue_size++;
            if(continue_size == size)
            {
                *index = ((i+1) - size);
                //DEBUG("get empty entry[%d] continue_size=%d index=%d",i,continue_size,*index);

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

static int _rtk_rg_search_cf_empty_Entry(int size,int* index)
{
    int i,j;
    rtk_classify_cfg_t cfRule;
    bzero(&cfRule, sizeof(cfRule));
    for(i=RESERVED_CF_BEFORE; i<RESERVED_CF_AFTER; i++)
    {
        cfRule.index = i;
        if(rtk_classify_cfgEntry_get(&cfRule))
        {
            RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
        }
	
        if(cfRule.valid==DISABLED)
        {
            *index = i;
			for(j=0;j<size;j++){
				cfRule.index = i+j;
				if(rtk_classify_cfgEntry_get(&cfRule))
				{
					RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
				}
				if(cfRule.valid==DISABLED){
					//DEBUG("cf need size=%d get cf[%d] is empty",size,cfRule.index);
				   continue;
				}else{
					RETURN_ERR(RT_ERR_RG_CF_ENTRY_FULL);
				}
			}
			return (RT_ERR_RG_OK);
        }

    }
   	return (RT_ERR_RG_CF_ENTRY_FULL);
}

#if 0
static int _rtk_rg_search_cf_64to511_empty_Entry(int size,int* index)
{
    int i;
    rtk_classify_cfg_t cfRule;
    bzero(&cfRule, sizeof(cfRule));
    for(i=MIN_CF_64TO511_ENTRY_SIZE; i<MAX_CF_64TO511_ENTRY_SIZE; i++)
    {
        cfRule.index = i;
        if(rtk_classify_cfgEntry_get(&cfRule))
        {
            RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
        }
        if(size==1)
        {
            if(cfRule.valid==DISABLED)
            {
                *index = i;
                return (RT_ERR_RG_OK);
            }
            else
            {
                continue;
            }
        }
        else if(size==2)
        {
            if(cfRule.valid==DISABLED)
            {
                cfRule.index = i+1;
                if(rtk_classify_cfgEntry_get(&cfRule))
                {
                    RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
                }
                if(cfRule.valid==DISABLED)
                {
                    *index = i;
                    return (RT_ERR_RG_OK);
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }

    }
    RETURN_ERR(RT_ERR_RG_CF_ENTRY_FULL);
}
#endif
static int _rtk_rg_search_acl_empty_portTableEntry(int* index)
{
    int i;
    rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
    bzero(&aclPortRangeEntry, sizeof(aclPortRangeEntry));

    for(i=0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
    {
        aclPortRangeEntry.index = i;
        if(rtk_acl_portRange_get(&aclPortRangeEntry))
        {
            RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
        }
        else
        {
            if(aclPortRangeEntry.type==PORTRANGE_UNUSED && aclPortRangeEntry.upper_bound==0x0 && aclPortRangeEntry.lower_bound==0x0)
            {
                *index = i;
                return (RT_ERR_RG_OK);
            }
            else
            {
                continue;
            }
        }
    }
    RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_FULL);

}
#if 0
static int _rtk_rg_search_acl_empty_ipTableEntry(int* index)
{
    int i;
    rtk_acl_rangeCheck_ip_t aclIpRangeEntry;
    bzero(&aclIpRangeEntry, sizeof(aclIpRangeEntry));
    for(i=0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
    {
        aclIpRangeEntry.index = i;
        if(rtk_acl_ipRange_get(&aclIpRangeEntry))
        {
            RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
        }
        else
        {
            if(aclIpRangeEntry.type==IPRANGE_UNUSED && aclIpRangeEntry.upperIp==0x0 && aclIpRangeEntry.lowerIp==0x0)
            {
                *index = i;
                return (RT_ERR_RG_OK);
            }
            else
            {
                continue;
            }
        }
    }
    RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_FULL);

}
#endif
static int _rtk_rg_search_cf_empty_portTableEntry(int* index)
{
    int i;
    rtk_classify_rangeCheck_l4Port_t cfPortRangeEntry;
    bzero(&cfPortRangeEntry, sizeof(cfPortRangeEntry));

    for(i=0; i<MAX_CF_PORTRANGETABLE_SIZE; i++)
    {
        cfPortRangeEntry.index= i;
        if(rtk_classify_portRange_get(&cfPortRangeEntry))
        {
            RETURN_ERR(RT_ERR_RG_CF_PORTTABLE_ACCESS_FAILED);
        }
        else
        {
            if(cfPortRangeEntry.type==0x0 && cfPortRangeEntry.lowerPort==0xffff && cfPortRangeEntry.upperPort==0x0)
            {
                *index = i;
                return (RT_ERR_RG_OK);
            }
            else
            {
                continue;
            }
        }
    }
    RETURN_ERR(RT_ERR_RG_CF_PORTTABLE_FULL);

}

static int _rtk_rg_search_cf_empty_ipTableEntry(int* index)
{
    int i;
    rtk_classify_rangeCheck_ip_t cfIpRangeEntry;
    bzero(&cfIpRangeEntry, sizeof(cfIpRangeEntry));

    for(i=0; i<MAX_CF_IPRANGETABLE_SIZE; i++)
    {
        cfIpRangeEntry.index= i;
        if(rtk_classify_ipRange_get(&cfIpRangeEntry))
        {
            RETURN_ERR(RT_ERR_RG_CF_IPTABLE_ACCESS_FAILED);
        }
        else
        {
            if(cfIpRangeEntry.type==0x0 && cfIpRangeEntry.lowerIp==0xffffffff && cfIpRangeEntry.upperIp==0x0)
            {
                *index = i;
                return (RT_ERR_RG_OK);
            }
            else
            {
                continue;
            }
        }
    }
    RETURN_ERR(RT_ERR_RG_CF_IPTABLE_FULL);

}

static int _rtk_rg_search_cf_empty_dscpTableEntry(int* index)
{
    int i,ret;
	rtk_dscp_t dscp;
    for(i=0; i<8; i++)
    {
        ret = rtk_classify_cfPri2Dscp_get(i,&dscp);
        if(ret!= RT_ERR_RG_OK)
            RETURN_ERR(RT_ERR_RG_CF_DSCPTABLE_ACCESS_FAILED);

        if(dscp == 0)
        {
            *index = i;
            return (RT_ERR_RG_OK);
        }
    }
    RETURN_ERR(RT_ERR_RG_CF_DSCPTABLE_FULL);
}

static int _rtk_rg_conflictField_check(rtk_rg_aclFilterAndQos_t* acl_filter)
{

    int ingress_intf_idx=0, i=0, ret=0;
	//int  egress_intf_idx=0;
    int ingress_intf_type=UNKNOW_INTF;
	//int egress_intf_type=UNKNOW_INTF;
    rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk;

    bzero(&ingress_intf_info,sizeof(rtk_rg_intfInfo_t));
    bzero(&egress_intf_info,sizeof(rtk_rg_intfInfo_t));
    bzero(&empty_intf_info,sizeof(rtk_rg_intfInfo_t));


	ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);


	//TCP & UDP can not enable at the same time
	if((acl_filter->filter_fields & INGRESS_L4_TCP_BIT) &&(acl_filter->filter_fields & INGRESS_L4_UDP_BIT))
		RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);

    //ipv6 & ipv4  ip can not use at the same time
    if( ((acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)||(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT))&&
            ((acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)||(acl_filter->filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)||(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)||(acl_filter->filter_fields & EGRESS_IPV4_DIP_RANGE_BIT)))
    {   
    	DEBUG("IPV4/IPV6 conflict");
		DEBUG("INGRESS_IPV6_SIP_RANGE_BIT =0x%llx",acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT);
		DEBUG("INGRESS_IPV6_DIP_RANGE_BIT =0x%llx",acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT);
		DEBUG("INGRESS_IPV4_SIP_RANGE_BIT =0x%llx",acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT);
		DEBUG("EGRESS_IPV4_SIP_RANGE_BIT =0x%llx",acl_filter->filter_fields & EGRESS_IPV4_SIP_RANGE_BIT);
		DEBUG("INGRESS_IPV4_DIP_RANGE_BIT =0x%llx",acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT);
		DEBUG("EGRESS_IPV4_DIP_RANGE_BIT =0x%llx",acl_filter->filter_fields & EGRESS_IPV4_DIP_RANGE_BIT);
        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
    }
    //CF do not change the source destination (ip/port) at the same time(NAPT only change one side of src/dest)
    if( (acl_filter->filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)&&(acl_filter->filter_fields & EGRESS_IPV4_DIP_RANGE_BIT))
        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);

    if( (acl_filter->filter_fields & EGRESS_L4_SPORT_RANGE_BIT)&&(acl_filter->filter_fields & EGRESS_L4_DPORT_RANGE_BIT))
        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);


    //ingress_netif CVID, DMAC, ingress_portmask should sync with acl_filter assigned!
    if(acl_filter->filter_fields & INGRESS_INTF_BIT)
    {
        ingress_intf_idx = acl_filter->ingress_intf_idx;
        ret = rtk_rg_apollo_intfInfo_find(&ingress_intf_info, &ingress_intf_idx);
        if(ret!=RT_ERR_RG_OK)
        {
            return ret;
        }
        if(ingress_intf_idx != acl_filter->ingress_intf_idx)
        {
            //did not get the assigned interface
            RETURN_ERR(RT_ERR_RG_INTF_GET_FAIL);
        }

        if(ingress_intf_info.is_wan) //ingress interface is wan
        {

            //get ingress_netif is wan/lan
            ingress_intf_type = WAN_INTF;

            //Wan Interface
            if(ingress_intf_info.wan_intf.wan_intf_conf.wan_type == RTK_RG_BRIDGE)
            {
                //wan bridge(L2) mode: judge CVID
                if(ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
                {
                    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
                    {
                        //prevent error setting: user set CVID is not same with ingress_wan_interface VID=> conflict!!!
                        if(acl_filter->ingress_ctag_vid!=ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id)
                            RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    }
                }
                else
                {
                    //prevent error setting: user set CVID but infress_wan_interface do not need to tag on=> conflict!!!
                    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                	if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT)
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                }
            }
            else
            {
                //wan route(L34) mode: judge CVID + DMAC
                if(ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
                {
                    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
                    {
                        //prevent error setting: user set CVID is not same with ingress_wan_interface VID=> conflict!!!
                        if(acl_filter->ingress_ctag_vid!=ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id)
                            RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    }

                    if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
                    {
                        //prevent error setting: user set DMAC is not same with ingress_wan_interface gmac=> conflict!!!
                        if(memcmp(&(acl_filter->ingress_dmac),&(ingress_intf_info.wan_intf.wan_intf_conf.gmac),sizeof(rtk_mac_t)))
                            RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    }
                }
                else
                {
                    //prevent error setting: user set CVID but ingress_wan_interface do not need to tag on=> conflict!!!
                    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT)
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);

                    if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
                    {
                        //prevent error setting: user set DMAC is not same with ingress_wan_interface gmac=> conflict!!!
                        if(memcmp(&(acl_filter->ingress_dmac),&(ingress_intf_info.wan_intf.wan_intf_conf.gmac),sizeof(rtk_mac_t)))
                            RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    }
                }
            }
        }
        else//ingress interface is lan
        {
            //get ingress_netif is wan/lan
            ingress_intf_type = LAN_INTF;

            //prevent error setting: user set portmask is not in lan_intf.port_mask=> conflict!!!
            if(acl_filter->filter_fields & INGRESS_PORT_BIT)
            {
                for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
                {
                    if( ( ((mac_pmsk.bits[0]>>i)&0x1)==0x1)  &&  (((ingress_intf_info.lan_intf.port_mask.portmask>>i)&0x1)==0x0) )
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                }
            }
        }
    }

    return (RT_ERR_RG_OK);
}

static int _rtk_rg_free_cf_portTableEntry(int index)
{
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)


    rtk_classify_rangeCheck_l4Port_t cfPortRangeEntry;
    bzero(&cfPortRangeEntry,sizeof(cfPortRangeEntry));
    cfPortRangeEntry.index=index;
    cfPortRangeEntry.lowerPort = 0xffff;

    if( rtk_classify_portRange_set(&cfPortRangeEntry))
    {
        DEBUG("free cf porttable failed");
        RETURN_ERR(RT_ERR_RG_CF_PORTTABLE_ACCESS_FAILED);
    }
#elif defined(CONFIG_RTL9602C_SERIES)	
	FIXME("9602BVB _rtk_rg_free_cf_portTableEntry(%d)",index);
#endif
    return (RT_ERR_RG_OK);
}
static int _rtk_rg_free_cf_ipTableEntry(int index)
{
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)

    rtk_classify_rangeCheck_ip_t cfIpRangeEntry;
    bzero(&cfIpRangeEntry,sizeof(cfIpRangeEntry));
    cfIpRangeEntry.index=index;
    cfIpRangeEntry.lowerIp=0xffffffff;

    if( rtk_classify_ipRange_set(&cfIpRangeEntry))
    {
        DEBUG("free cf iptable failed");
        RETURN_ERR(RT_ERR_RG_CF_IPTABLE_ACCESS_FAILED);
    }
#elif defined(CONFIG_RTL9602C_SERIES)

	FIXME("9602BVB _rtk_rg_free_cf_ipTableEntry(%d)",index);
#endif
    return (RT_ERR_RG_OK);	
	
}


static int _rtk_rg_free_cf_dscpTableEntry(int index)
{
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)

    int ret;
    ret = rtk_classify_cfPri2Dscp_set(index,0);
    if(ret != RT_ERR_RG_OK)
        RETURN_ERR(RT_ERR_RG_CF_DSCPTABLE_ACCESS_FAILED);
#elif defined(CONFIG_RTL9602C_SERIES)	
		FIXME("9602BVB _rtk_rg_free_cf_dscpTableEntry(%d)",index);
#endif

    return (RT_ERR_RG_OK);
}

static int _rtk_rg_free_acl_portTableEntry(int index)
{
	int ret;
    rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
    bzero(&aclPortRangeEntry,sizeof(aclPortRangeEntry));
    aclPortRangeEntry.index=index;
	ret = rtk_acl_portRange_set(&aclPortRangeEntry);
    if(ret!= RT_ERR_RG_OK)
    {
        DEBUG("free acl porttable failed, ret(rtk)=%d",ret);
        RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
    }
    return (RT_ERR_RG_OK);
}

static int _rtk_rg_free_acl_ipTableEntry(int index)
{
	int ret;
    rtk_acl_rangeCheck_ip_t aclIpRangeEntry;
    bzero(&aclIpRangeEntry,sizeof(aclIpRangeEntry));
    aclIpRangeEntry.index=index;
	ret = rtk_acl_ipRange_set(&aclIpRangeEntry);
    if(ret!= RT_ERR_RG_OK)
    {
        DEBUG("free acl iptable failed, ret(rtk)=%d",ret);
        RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
    }
    return (RT_ERR_RG_OK);
}


static int _rtk_rg_get_larger_ipv4_alignment_range(ipaddr_t ipv4_addr_start, ipaddr_t ipv4_addr_end, ipaddr_t* ipv4_align_range, uint32* ipv4_align_mask)
{

	int i;
	u32 mask=0x0;
	for(i=0;i<32;i++){
		if(((ipv4_addr_start<<i)&0x80000000)!=((ipv4_addr_end<<i)&0x80000000)){
			break;
		}else{
			mask |= (1<<(31-i));
		}
	}
	*ipv4_align_range = (ipv4_addr_start&mask);	
	*ipv4_align_mask = mask;
	return (RT_ERR_RG_OK);
	
}

static int _rtk_rg_maskLength_get_by_ipv4_range(ipaddr_t ipv4_addr_start, ipaddr_t ipv4_addr_end, int* maskLength)
{
	int i;
	int length=0;
	
	for(i=0;i<32;i++){
		if(((ipv4_addr_start<<i)&0x80000000)!=((ipv4_addr_end<<i)&0x80000000)){
			length = i;
			break;
		}
	}

	//check rest bit in range
	for(i=length;i<32;i++){
		if(((ipv4_addr_start<<i)&0x80000000)!=0){rtlglue_printf("IP Range not in mask alignment \n"); RETURN_ERR(RT_ERR_RG_FAILED);}
		if(((ipv4_addr_end<<i)&0x80000000)!=0x80000000){rtlglue_printf("IP Range not in mask alignment \n"); RETURN_ERR(RT_ERR_RG_FAILED);}
	}
	
	*maskLength = (32-length);
	
	return (RT_ERR_RG_OK);
	
}

static int _rtk_rg_maskLength_get_by_ipv6_range(uint8* ipv6_start, uint8* ipv6_end, int* maskLength)
{
	int i,j,k;
	int length;

	if(ipv6_start[0]==ipv6_end[0] && ipv6_start[1]==ipv6_end[1] &&
		ipv6_start[2]==ipv6_end[2] && ipv6_start[3]==ipv6_end[3] &&
		ipv6_start[4]==ipv6_end[4] && ipv6_start[5]==ipv6_end[5] &&
		ipv6_start[6]==ipv6_end[6] && ipv6_start[7]==ipv6_end[7] &&
		ipv6_start[8]==ipv6_end[8] && ipv6_start[9]==ipv6_end[9] &&
		ipv6_start[10]==ipv6_end[10] && ipv6_start[11]==ipv6_end[11] &&
		ipv6_start[12]==ipv6_end[12] && ipv6_start[13]==ipv6_end[13] &&
		ipv6_start[14]==ipv6_end[14] && ipv6_start[15]==ipv6_end[15]){
		//SINGLE IP
		*maskLength = 0;

	}else{ //RANGE IP
		for(i=0;i<16;i++){
			if(memcmp(&ipv6_start[i],&ipv6_end[i],1)){//i: get the first different byte
				break;
			}
		}
		for(j=0;j<8;j++){ // j: get the first different bit of the byte
			if(((ipv6_start[i]<<j)&0x80) != ((ipv6_end[i]<<j)&0x80))
				break;
		}
		length = (i*8)+j;
		(*maskLength) = 128 - length;


		

		/*make sure the IP range is mask aligned*/
		for(k=0;k<8;k++){
			if(k>j){
				if((((ipv6_start[i]<<k)&0x80)!=0x0)){rtlglue_printf("IP Range not in mask alignment(1) \n"); RETURN_ERR(RT_ERR_RG_FAILED);}//rest bit should be 0
				if((((ipv6_end[i]<<k)&0x80)!=0x80)){rtlglue_printf("IP Range not in mask alignment(2) \n"); RETURN_ERR(RT_ERR_RG_FAILED);}//rest bit should be 1
			}
		}
		
		for(k=i+1;k<16;k++){
			if(ipv6_start[k]!=0x0){rtlglue_printf("IP Range not in mask alignment(3) \n"); RETURN_ERR(RT_ERR_RG_FAILED);}//rest bytes should be 0x00
			if(ipv6_end[k]!=0xff){rtlglue_printf("IP Range not in mask alignment(4) \n"); RETURN_ERR(RT_ERR_RG_FAILED);}//rest bytes should be 0xff
		}
	}
	return (RT_ERR_RG_OK);
	
}
#if 0
static int _rtk_rg_asic_defaultDropEntry_setup(void)
{




    rtk_acl_ingress_entry_t aclRule;
    bzero(&aclRule,sizeof(aclRule));


	bzero(&cfUsRule,sizeof(cfUsRule));
    bzero(&cfDsRule,sizeof(cfDsRule));


    aclRule.index = RESERVED_ACL_DEFAULT_ENTRY;
    aclRule.valid = ENABLE;
    aclRule.activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK;
    aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
    aclRule.act.forwardAct.act = ACL_IGR_FORWARD_REDIRECT_ACT;
    aclRule.act.forwardAct.portMask.bits[0]= 0x0;


    if(rtk_acl_igrRuleEntry_add(&aclRule))
    {
        DEBUG("adding default aclRule[%d] drop failed",RESERVED_ACL_DEFAULT_ENTRY);
        RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
    }


    cfUsRule.index = RESERVED_CF_US_DEFAULT_ENTRY;
    cfUsRule.valid = ENABLE;
    cfUsRule.direction = CLASSIFY_DIRECTION_US;
    cfUsRule.act.usAct.drop = CLASSIFY_DROP_ACT_ENABLE;
    if(RTK_CLASSIFY_CFGENTRY_ADD(&cfUsRule))
    {
        DEBUG("adding default cfUsEntry[%d] drop failed",RESERVED_CF_US_DEFAULT_ENTRY);
        RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
    }

    cfDsRule.index = RESERVED_CF_DS_DEFAULT_ENTRY;
    cfDsRule.valid = ENABLE;
    cfDsRule.direction = CLASSIFY_DIRECTION_DS;
    cfDsRule.act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_FORCE_FORWARD; //DS drop
    cfDsRule.act.dsAct.uniMask.bits[0] = 0x0;
    if(RTK_CLASSIFY_CFGENTRY_ADD(&cfDsRule))
    {
        DEBUG("adding default cfDsEntry[%d] drop failed",RESERVED_CF_DS_DEFAULT_ENTRY);
        RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
    }


    return (RT_ERR_RG_OK);
}
#endif

static int _rtk_rg_asic_defaultDropEntry_remove(void)
{
#if 0

    if(rtk_acl_igrRuleEntry_del(RESERVED_ACL_DEFAULT_ENTRY))
    {
        DEBUG("remove default aclRule[%d] drop failed",RESERVED_ACL_DEFAULT_ENTRY);
        RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
    }

    if(rtk_classify_cfgEntry_del(RESERVED_CF_US_DEFAULT_ENTRY))
    {
        DEBUG("remove default cfUsEntry[%d] drop failed",RESERVED_CF_US_DEFAULT_ENTRY);
        RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
    }

    if(rtk_classify_cfgEntry_del(RESERVED_CF_DS_DEFAULT_ENTRY))
    {
        DEBUG("remove default cfDsEntry[%d] drop failed",RESERVED_CF_DS_DEFAULT_ENTRY);
        RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
    }

#endif

    return (RT_ERR_RG_OK);

}


#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
static int _rtk_rg_rearrange_ACL_weight_for_egress_wan(void)
{
	int i,j;
	int temp,sp,p_idx,n_idx,p_type,n_type,p_weight,n_weight;

	//1-4. sorting the rule by weight: BubSort
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>0; i--){
		sp=1;
		for (j=0; j<=i; j++){
			//check data is valid
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

			//we change the order if the weight is the same but the type has different
			p_weight=rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight;
			n_weight=rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight;
			if(p_weight!=n_weight)continue;
			
			p_idx=rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].egress_intf_idx;
			p_type=RG_ACL_TRANS_NONE;
			n_idx=rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].egress_intf_idx;
			n_type=RG_ACL_TRANS_NONE;

			//compare decision:vlanB_l34>portB_l34>vlanB_l2>portB_l2>intf_ro>df_ro
			if((rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].filter_fields&EGRESS_INTF_BIT)&&
				(rtk_rg_apollo_intfInfo_find(&egress_intf_info, &p_idx)==RT_ERR_RG_OK)&&
				(p_idx==rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].egress_intf_idx)&&
				egress_intf_info.is_wan){
				if(egress_intf_info.wan_intf.wan_intf_conf.vlan_binding_mask.portmask)
					p_type=RG_ACL_TRANS_L2_VLAN_BIND;
				else if(egress_intf_info.wan_intf.wan_intf_conf.port_binding_mask.portmask)
					p_type=RG_ACL_TRANS_L2_PORT_BIND;
				
				if(egress_intf_info.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE){
					if(p_type!=RG_ACL_TRANS_NONE)
						p_type+=2;	//L34
					else{
						switch(rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ip_version){
							case IPVER_V4ONLY:
								if(rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ipv4_default_gateway_on)
									p_type=RG_ACL_TRANS_v4_OTHER;
								else
									p_type=RG_ACL_TRANS_v4_SUBNET;
								break;
							case IPVER_V6ONLY:
								if(rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ipv6_default_gateway_on)
									p_type=RG_ACL_TRANS_v6_OTHER;
								else
									p_type=RG_ACL_TRANS_v6_SUBNET;
								break;
							default:
								if(rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ipv4_default_gateway_on)
									p_type=RG_ACL_TRANS_v4_OTHER_v6_OTHER;
								else
									p_type=RG_ACL_TRANS_v4_SUBNET_v6_OTHER;
								if(!rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ipv6_default_gateway_on)
									p_type+=1;
								break;
						}
					}
				}
			}
			if((rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].filter_fields&EGRESS_INTF_BIT)&&
				(rtk_rg_apollo_intfInfo_find(&egress_intf_info, &n_idx)==RT_ERR_RG_OK)&&
				(n_idx==rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].egress_intf_idx)&&
				egress_intf_info.is_wan){
				if(egress_intf_info.wan_intf.wan_intf_conf.vlan_binding_mask.portmask)
					n_type=RG_ACL_TRANS_L2_VLAN_BIND;
				else if(egress_intf_info.wan_intf.wan_intf_conf.port_binding_mask.portmask)
					n_type=RG_ACL_TRANS_L2_PORT_BIND;
				
				if(egress_intf_info.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE){
					if(n_type!=RG_ACL_TRANS_NONE)
						n_type+=2;	//L34
					else{
						switch(rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ip_version){
							case IPVER_V4ONLY:
								if(rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ipv4_default_gateway_on)
									n_type=RG_ACL_TRANS_v4_OTHER;
								else
									n_type=RG_ACL_TRANS_v4_SUBNET;
								break;
							case IPVER_V6ONLY:
								if(rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ipv6_default_gateway_on)
									n_type=RG_ACL_TRANS_v6_OTHER;
								else
									n_type=RG_ACL_TRANS_v6_SUBNET;
								break;
							default:
								if(rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ipv4_default_gateway_on)
									n_type=RG_ACL_TRANS_v4_OTHER_v6_OTHER;
								else
									n_type=RG_ACL_TRANS_v4_SUBNET_v6_OTHER;
								if(!rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ipv6_default_gateway_on)
									n_type+=1;
								break;
						}
					}
				}
			}
			//20150826LUKE: Mix egress WAN pattern rule with non-egress-WAN pattern rule at same weight should return fail.
			if((p_type==RG_ACL_TRANS_NONE && n_type>p_type) || (n_type==RG_ACL_TRANS_NONE && p_type>n_type))
				return RT_ERR_RG_ACL_EGRESS_WAN_MIX;

			//compare the type while acl_weight is the same
			if (p_type<n_type){
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
#endif


static int _rtk_rg_aclDuplicatedIngressRuleCheck(int chkAclSWEntryIdx,rtk_rg_aclFilterAndQos_t *chkAclSWEntry,int *duIdx)
{
	/*for cheching is there any duplicated pure ingress rule. 
	If the duplicated rule exist it doesn't need to add HW ACL again.
	We just shared the same HW ACL rule index*/

	int i,cmpIdx;
	rtk_rg_aclFilterEntry_t *cmpAclSWEntry;

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//get elder rules
		cmpIdx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];
		if(cmpIdx==-1){//rest rules haven't set.
			ACL("no rest rule need to check!\n");
			break;
		}
					
		cmpAclSWEntry = (&rg_db.systemGlobal.acl_SW_table_entry[cmpIdx]);
		ACL("Current RG_ACL cmp with RG_ACL[%d] cmpAclSWEntry.filter_fields=0x%llx chkAclSWEntry.filter_fields=0x%llx \n",cmpIdx,cmpAclSWEntry->acl_filter.filter_fields,chkAclSWEntry->filter_fields);

		if(chkAclSWEntryIdx==cmpIdx){//this is chkAclSWEntry self rule.
			ACL("self rule, skip!\n");
			continue;
		}

		if(cmpAclSWEntry->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){//pure acl without cf_latch action, can not be shared.
			ACL("pure acl rule, skip!\n");
			continue;
		}

		//compared the ingress part with elder rule
		if((chkAclSWEntry->filter_fields&PURE_ACL_PATTERN_BITS)==(cmpAclSWEntry->acl_filter.filter_fields&PURE_ACL_PATTERN_BITS)){

			if(chkAclSWEntry->filter_fields&INGRESS_PORT_BIT){
				if(chkAclSWEntry->ingress_port_mask.portmask!=cmpAclSWEntry->acl_filter.ingress_port_mask.portmask){
					ACL("INGRESS_PORT_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_INTF_BIT){
				if(chkAclSWEntry->ingress_intf_idx!=cmpAclSWEntry->acl_filter.ingress_intf_idx){
					ACL("INGRESS_INTF_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_ETHERTYPE_BIT){
				if(chkAclSWEntry->ingress_ethertype!=cmpAclSWEntry->acl_filter.ingress_ethertype){
					ACL("INGRESS_ETHERTYPE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_ethertype_mask!=cmpAclSWEntry->acl_filter.ingress_ethertype_mask){
					ACL("INGRESS_ETHERTYPE_BIT not the same\n");
					continue;
				}	
			}
			if(chkAclSWEntry->filter_fields&INGRESS_CTAG_PRI_BIT){
				if(chkAclSWEntry->ingress_ctag_pri!=cmpAclSWEntry->acl_filter.ingress_ctag_pri){
					ACL("INGRESS_CTAG_PRI_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_CTAG_VID_BIT){
				if(chkAclSWEntry->ingress_ctag_vid!=cmpAclSWEntry->acl_filter.ingress_ctag_vid){
					ACL("INGRESS_CTAG_VID_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_CTAG_CFI_BIT){
				if(chkAclSWEntry->ingress_ctag_cfi!=cmpAclSWEntry->acl_filter.ingress_ctag_cfi){
					ACL("INGRESS_CTAG_CFI_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_SMAC_BIT){
				if(memcmp(chkAclSWEntry->ingress_smac.octet,cmpAclSWEntry->acl_filter.ingress_smac.octet,ETHER_ADDR_LEN)){
					ACL("INGRESS_SMAC_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_smac_mask.octet,cmpAclSWEntry->acl_filter.ingress_smac_mask.octet,ETHER_ADDR_LEN)){
					ACL("INGRESS_SMAC_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_DMAC_BIT){
				if(memcmp(chkAclSWEntry->ingress_dmac.octet,cmpAclSWEntry->acl_filter.ingress_dmac.octet,ETHER_ADDR_LEN)){
					ACL("INGRESS_DMAC_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_dmac_mask.octet,cmpAclSWEntry->acl_filter.ingress_dmac_mask.octet,ETHER_ADDR_LEN)){
					ACL("INGRESS_DMAC_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_DSCP_BIT){
				if(chkAclSWEntry->ingress_dscp!=cmpAclSWEntry->acl_filter.ingress_dscp){
					ACL("INGRESS_DSCP_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
				if(memcmp(chkAclSWEntry->ingress_src_ipv6_addr_start,cmpAclSWEntry->acl_filter.ingress_src_ipv6_addr_start,16)){
					ACL("INGRESS_IPV6_SIP_RANGE_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_src_ipv6_addr_end,cmpAclSWEntry->acl_filter.ingress_src_ipv6_addr_end,16)){
					ACL("INGRESS_IPV6_SIP_RANGE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_DIP_RANGE_BIT){
				if(memcmp(chkAclSWEntry->ingress_dest_ipv6_addr_start,cmpAclSWEntry->acl_filter.ingress_dest_ipv6_addr_start,16)){
					ACL("INGRESS_IPV6_DIP_RANGE_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_dest_ipv6_addr_end,cmpAclSWEntry->acl_filter.ingress_dest_ipv6_addr_end,16)){
					ACL("INGRESS_IPV6_DIP_RANGE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
				if(chkAclSWEntry->ingress_src_ipv4_addr_start!=cmpAclSWEntry->acl_filter.ingress_src_ipv4_addr_start){
					ACL("INGRESS_IPV4_SIP_RANGE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_src_ipv4_addr_end!=cmpAclSWEntry->acl_filter.ingress_src_ipv4_addr_end){
					ACL("INGRESS_IPV4_SIP_RANGE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
				if(chkAclSWEntry->ingress_dest_ipv4_addr_start!=cmpAclSWEntry->acl_filter.ingress_dest_ipv4_addr_start){
					ACL("INGRESS_IPV4_DIP_RANGE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_dest_ipv4_addr_end!=cmpAclSWEntry->acl_filter.ingress_dest_ipv4_addr_end){
					ACL("INGRESS_IPV4_DIP_RANGE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
				if(chkAclSWEntry->ingress_src_l4_port_start!=cmpAclSWEntry->acl_filter.ingress_src_l4_port_start){
					ACL("INGRESS_L4_SPORT_RANGE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_src_l4_port_end!=cmpAclSWEntry->acl_filter.ingress_src_l4_port_end){
					ACL("INGRESS_L4_SPORT_RANGE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
				if(chkAclSWEntry->ingress_dest_l4_port_start!=cmpAclSWEntry->acl_filter.ingress_dest_l4_port_start){
					ACL("INGRESS_L4_DPORT_RANGE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_dest_l4_port_end!=cmpAclSWEntry->acl_filter.ingress_dest_l4_port_end){
					ACL("INGRESS_L4_DPORT_RANGE_BIT not the same\n");
					continue;
				}
			}

			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_DSCP_BIT){
				if(chkAclSWEntry->ingress_ipv6_dscp!=cmpAclSWEntry->acl_filter.ingress_ipv6_dscp){
					ACL("INGRESS_IPV6_DSCP_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_STREAM_ID_BIT){
				if(chkAclSWEntry->ingress_stream_id!=cmpAclSWEntry->acl_filter.ingress_stream_id){
					ACL("INGRESS_STREAM_ID_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_stream_id_mask!=cmpAclSWEntry->acl_filter.ingress_stream_id_mask){
					ACL("INGRESS_STREAM_ID_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_STAG_PRI_BIT){
				if(chkAclSWEntry->ingress_stag_pri!=cmpAclSWEntry->acl_filter.ingress_stag_pri){
					ACL("INGRESS_STAG_PRI_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_STAG_VID_BIT){
				if(chkAclSWEntry->ingress_stag_vid!=cmpAclSWEntry->acl_filter.ingress_stag_vid){
					ACL("INGRESS_STAG_VID_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_STAG_DEI_BIT){
				if(chkAclSWEntry->ingress_stag_dei!=cmpAclSWEntry->acl_filter.ingress_stag_dei){
					ACL("INGRESS_STAG_DEI_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_STAGIF_BIT){
				if(chkAclSWEntry->ingress_stagIf!=cmpAclSWEntry->acl_filter.ingress_stagIf){
					ACL("INGRESS_STAGIF_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_CTAGIF_BIT){
				if(chkAclSWEntry->ingress_ctagIf!=cmpAclSWEntry->acl_filter.ingress_ctagIf){
					ACL("INGRESS_CTAGIF_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_L4_POROTCAL_VALUE_BIT){
				if(chkAclSWEntry->ingress_l4_protocal!=cmpAclSWEntry->acl_filter.ingress_l4_protocal){
					ACL("INGRESS_L4_POROTCAL_VALUE_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_TOS_BIT){
				if(chkAclSWEntry->ingress_tos!=cmpAclSWEntry->acl_filter.ingress_tos){
					ACL("INGRESS_TOS_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_TC_BIT){
				if(chkAclSWEntry->ingress_ipv6_tc!=cmpAclSWEntry->acl_filter.ingress_ipv6_tc){
					ACL("INGRESS_IPV6_TC_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_SIP_BIT){
				if(memcmp(chkAclSWEntry->ingress_src_ipv6_addr,cmpAclSWEntry->acl_filter.ingress_src_ipv6_addr,16)){
					ACL("INGRESS_IPV6_SIP_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_src_ipv6_addr_mask,cmpAclSWEntry->acl_filter.ingress_src_ipv6_addr_mask,16)){
					ACL("INGRESS_IPV6_SIP_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_DIP_BIT){
				if(memcmp(chkAclSWEntry->ingress_dest_ipv6_addr,cmpAclSWEntry->acl_filter.ingress_dest_ipv6_addr,16)){
					ACL("INGRESS_IPV6_DIP_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_dest_ipv6_addr_mask,cmpAclSWEntry->acl_filter.ingress_dest_ipv6_addr_mask,16)){
					ACL("INGRESS_IPV6_DIP_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_WLANDEV_BIT){
				if(chkAclSWEntry->ingress_wlanDevMask!=cmpAclSWEntry->acl_filter.ingress_wlanDevMask){
					ACL("INGRESS_WLANDEV_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV4_TAGIF_BIT){
				if(chkAclSWEntry->ingress_ipv4_tagif!=cmpAclSWEntry->acl_filter.ingress_ipv4_tagif){
					ACL("INGRESS_IPV4_TAGIF_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_TAGIF_BIT){
				if(chkAclSWEntry->ingress_ipv6_tagif!=cmpAclSWEntry->acl_filter.ingress_ipv6_tagif){
					ACL("INGRESS_IPV6_TAGIF_BIT not the same\n");
					continue;
				}

			}

			ACL("Ingress Part is the same with RG_ACL[%d], share the HW ACL[%d] \n",cmpIdx,cmpAclSWEntry->hw_aclEntry_start);
			*duIdx = cmpIdx;
			break;
		}
	}

	return (RT_ERR_RG_OK);
}



/*This API is using for saving ACL resources if all patterns can be handled in CF0~63*/
static int _rtk_rg_checkCFAsicPatternOnly(rtk_rg_aclFilterAndQos_t *acl_filter){

	unsigned long long int acl_rule_must_need_pattern_mask = PURE_ACL_PATTERN_BITS;
	unsigned long long int cf_pattern_mask = PURE_CF_PATTERN_BITS;

	acl_rule_must_need_pattern_mask &= (~(INGRESS_STAGIF_BIT|INGRESS_CTAGIF_BIT));//stagif/ctagif can be checked by cf

	//any ACL patterns are included
	if(acl_filter->filter_fields & acl_rule_must_need_pattern_mask){		
		return NEED_CF_ASIC_RULE_ONLY_FLASE;
	}

	//double check for the rule have CF patterns
	if(acl_filter->filter_fields & cf_pattern_mask){
		return NEED_CF_ASIC_RULE_ONLY_TRUE;
	}
	 
	return NEED_CF_ASIC_RULE_ONLY_FLASE;
}

static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx)
{
    int i=0, j=0;
	int ret;
    int table_index=0;
    int acl_entry_index=0,acl_entry_index_tmp=0;
    int acl_entry_size=0;
    int flag_acl_first_entry;
    int cf_entry_index=0;
    int flag_add_cfRule=DISABLE;
    int ingress_intf_idx=0, egress_intf_idx=0;
	int ipv6_unmask_length=0;
	int ipv4_unmask_length=0;
	int flag_ipv6_sip_need_to_trap=DISABLE;
	int flag_ipv6_dip_need_to_trap=DISABLE;
	int flag_ipv4_iprange_need_to_trap=DISABLE;
	
	//support for pppoe+vlan issue: while downstream 0x8100 will parse as Stag, 
	//if original ACL rule include Ctag pattern, then ACL also need to add duplicate rule for Stag
	int flag_cvlan_has_duplicate_to_svlan=DISABLED;

#define CONFIG_RG_ACL_EGRESS_CPRI_PATTERN_SUPPORT 1
#ifdef CONFIG_RG_ACL_EGRESS_CPRI_PATTERN_SUPPORT
	int flag_egress_cpri_supported_by_multiple_cfRule_without_qosRemark=DISABLE;
	int flag_egress_cpri_supported_by_multiple_cfRule_with_qosRemark=DISABLE;
	
#endif

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	int first_keep=0, binding_mask=0,temp_mask=0,LAN_num=0,LAN_count=0;
	unsigned int bit_mask=0;
	rtk_rg_acl_transform_type_t transform_type=RG_ACL_TRANS_NONE;
	rtk_rg_portmask_t egress_port_binding_mask;
	unsigned long long int saved_filter_fields=acl_filter->filter_fields;
#endif

	rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk;
	int cf_only=0;


cvlan_duplicate_to_svlan:
	ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);
	//extport BC packet can be trapped originally, so support this kind of case but without add HW ACL rule. 
	if((ext_pmsk.bits[0]!=0x0) && (acl_filter->action_type==ACL_ACTION_TYPE_TRAP || acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS)){
		if((acl_filter->filter_fields&INGRESS_DMAC_BIT) && 
			acl_filter->ingress_dmac.octet[0]==0xff	&&
			acl_filter->ingress_dmac.octet[1]==0xff	&&
			acl_filter->ingress_dmac.octet[2]==0xff	&&
			acl_filter->ingress_dmac.octet[3]==0xff	&&
			acl_filter->ingress_dmac.octet[4]==0xff	&&
			acl_filter->ingress_dmac.octet[5]==0xff	){ //from extPort but BC case is supported
			//do nothing , without add ACL. Just let BC packet trapped originally.
			return (RT_ERR_RG_OK);	
		}
	}

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	if((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) && (acl_filter->filter_fields&EGRESS_INTF_BIT))
	{
		//Check which WAN model we have:
		//if wanType is bridge:
		// 1: with VLAN-binding, ingress SPA+VID
		// 2: with Port-binding, ingress SPA
		//else
		// 1: with VLAN-binding, ingress SPA+VID+DMAC=GMAC
		// 2: with Port-binding, ingress SPA+DMAC=GMAC
		// 3: with non-default route interface, DIP sunbnet+DMAC=GMAC
		// 4: other, ingress SPA without WAN port+DMAC=GMAC
		rg_db.systemGlobal.acl_SW_egress_intf_type_zero_num++;
		egress_intf_idx = acl_filter->egress_intf_idx;
		ASSERT_EQ(rtk_rg_apollo_intfInfo_find(&egress_intf_info, &egress_intf_idx),RT_ERR_RG_OK);
		if((egress_intf_idx==acl_filter->egress_intf_idx) && egress_intf_info.is_wan)
		{
			memcpy(&egress_port_binding_mask,&egress_intf_info.wan_intf.wan_intf_conf.port_binding_mask,sizeof(rtk_rg_portmask_t));
			DEBUG("WAN intf[%d] vlanbpmsk is %x, portbpmsk is %x!",egress_intf_idx,egress_intf_info.wan_intf.wan_intf_conf.vlan_binding_mask.portmask,egress_port_binding_mask.portmask);
			if(egress_intf_info.wan_intf.wan_intf_conf.vlan_binding_mask.portmask)
			{
				transform_type=RG_ACL_TRANS_L2_VLAN_BIND;
				for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
				{
					if(rg_db.bind[i].valid && rg_db.bind[i].rtk_bind.vidLan!=0 && egress_intf_idx==rg_db.nexthop[rg_db.wantype[rg_db.bind[i].rtk_bind.wanTypeIdx].rtk_wantype.nhIdx].rtk_nexthop.ifIdx)
						binding_mask|=0x1<<i;
				}
				temp_mask=binding_mask;
			}
			else if(egress_port_binding_mask.portmask)
				transform_type=RG_ACL_TRANS_L2_PORT_BIND;
			
			if(egress_intf_info.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE)
			{
				LAN_num=rg_db.systemGlobal.lanIntfTotalNum;
				LAN_count=0;
				if(transform_type!=RG_ACL_TRANS_NONE)
					transform_type+=2;	//L34
				else
				{
					switch(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_version)
					{
						case IPVER_V4ONLY:
							if(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv4_default_gateway_on)
								transform_type=RG_ACL_TRANS_v4_OTHER;
							else
								transform_type=RG_ACL_TRANS_v4_SUBNET;
							break;
						case IPVER_V6ONLY:
							if(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_default_gateway_on)
								transform_type=RG_ACL_TRANS_v6_OTHER;
							else
								transform_type=RG_ACL_TRANS_v6_SUBNET;
							break;
						default:
							if(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv4_default_gateway_on)
								transform_type=RG_ACL_TRANS_v4_OTHER_v6_OTHER;
							else
								transform_type=RG_ACL_TRANS_v4_SUBNET_v6_OTHER;
							if(!rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_default_gateway_on)
								transform_type+=2;
							break;
					}
				}
			}
TRANSFORM_BEGIN:
			DEBUG("transform begin, type is %d, filter_fields:%llx, LAN_num is %d, LAN_count is %d",transform_type,acl_filter->filter_fields,LAN_num,LAN_count);
			switch(transform_type)
			{
				case RG_ACL_TRANS_L34_VLAN_BIND:
					//DMAC=LGMAC
					if(LAN_num>0)
					{
						acl_filter->filter_fields|=INGRESS_DMAC_BIT;
						memcpy(acl_filter->ingress_dmac.octet,rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN);
						acl_filter->ingress_dmac_mask.octet[0]=0xff;
						acl_filter->ingress_dmac_mask.octet[1]=0xff;
						acl_filter->ingress_dmac_mask.octet[2]=0xff;
						acl_filter->ingress_dmac_mask.octet[3]=0xff;
						acl_filter->ingress_dmac_mask.octet[4]=0xff;
						acl_filter->ingress_dmac_mask.octet[5]=0xff;
					}
					else
					{
						temp_mask=0;
						break;
					}
				case RG_ACL_TRANS_L2_VLAN_BIND:
					//ingress SPA+VID
					for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
					{
						if(temp_mask&(0x1<<i))
						{
							temp_mask&=(~(0x1<<i));	//turn off one bit while all LANGMAC had added
							acl_filter->filter_fields|=INGRESS_PORT_BIT;						
							acl_filter->ingress_port_mask.portmask=0x0;
							acl_filter->ingress_port_mask.portmask|=rg_db.bind[i].rtk_bind.portMask.bits[0];
							acl_filter->ingress_port_mask.portmask|=rg_db.bind[i].rtk_bind.extPortMask.bits[0]<<RTK_RG_PORT_CPU;
							if(transform_type==RG_ACL_TRANS_L34_VLAN_BIND && !(acl_filter->ingress_port_mask.portmask&rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->port_mask.portmask))
								goto skipAclAsicSetting;
							acl_filter->filter_fields|=INGRESS_CTAG_VID_BIT;
							acl_filter->ingress_ctag_vid=rg_db.bind[i].rtk_bind.vidLan;
							DEBUG("add spa %x vid %d",acl_filter->ingress_port_mask.portmask,acl_filter->ingress_ctag_vid);
							break;
						}
					}				
					break;
				case RG_ACL_TRANS_L34_PORT_BIND:
					//DMAC=LGMAC
					if(LAN_num>0)
					{
						acl_filter->filter_fields|=INGRESS_DMAC_BIT;
						memcpy(acl_filter->ingress_dmac.octet,rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN);
						acl_filter->ingress_dmac_mask.octet[0]=0xff;
						acl_filter->ingress_dmac_mask.octet[1]=0xff;
						acl_filter->ingress_dmac_mask.octet[2]=0xff;
						acl_filter->ingress_dmac_mask.octet[3]=0xff;
						acl_filter->ingress_dmac_mask.octet[4]=0xff;
						acl_filter->ingress_dmac_mask.octet[5]=0xff;
						if(!(egress_port_binding_mask.portmask&rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->port_mask.portmask))goto skipAclAsicSetting;
					}
					else
						break;
				case RG_ACL_TRANS_L2_PORT_BIND:
					//ingress SPA
					acl_filter->filter_fields|=INGRESS_PORT_BIT;
					acl_filter->ingress_port_mask.portmask=egress_port_binding_mask.portmask;
					break;
				case RG_ACL_TRANS_v6_SUBNET:			
				case RG_ACL_TRANS_v4_OTHER_v6_SUBNET:
					//IPv6 DIP sunbnet
					acl_filter->filter_fields|=INGRESS_IPV6_DIP_RANGE_BIT;
					memcpy(acl_filter->ingress_dest_ipv6_addr_start,rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(acl_filter->ingress_dest_ipv6_addr_end,rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr,IPV6_ADDR_LEN);
					if((rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length&0x7)==0)
						bit_mask=0;
					else
						bit_mask=(0xff<<(8-(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length&0x7)))&0xff;
					i=rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length>>3;
					acl_filter->ingress_dest_ipv6_addr_start[i]&=bit_mask;
					acl_filter->ingress_dest_ipv6_addr_end[i]|=((~bit_mask)&0xff);
					memset(&acl_filter->ingress_dest_ipv6_addr_start[i+1],0,IPV6_ADDR_LEN-i-1);
					memset(&acl_filter->ingress_dest_ipv6_addr_end[i+1],0xff,IPV6_ADDR_LEN-i-1);
					DEBUG("ipv6 start %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
						acl_filter->ingress_dest_ipv6_addr_start[0],acl_filter->ingress_dest_ipv6_addr_start[1],acl_filter->ingress_dest_ipv6_addr_start[2],acl_filter->ingress_dest_ipv6_addr_start[3],
						acl_filter->ingress_dest_ipv6_addr_start[4],acl_filter->ingress_dest_ipv6_addr_start[5],acl_filter->ingress_dest_ipv6_addr_start[6],acl_filter->ingress_dest_ipv6_addr_start[7],
						acl_filter->ingress_dest_ipv6_addr_start[8],acl_filter->ingress_dest_ipv6_addr_start[9],acl_filter->ingress_dest_ipv6_addr_start[10],acl_filter->ingress_dest_ipv6_addr_start[11],
						acl_filter->ingress_dest_ipv6_addr_start[12],acl_filter->ingress_dest_ipv6_addr_start[13],acl_filter->ingress_dest_ipv6_addr_start[14],acl_filter->ingress_dest_ipv6_addr_start[15]);
					DEBUG("ipv6 end %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
						acl_filter->ingress_dest_ipv6_addr_end[0],acl_filter->ingress_dest_ipv6_addr_end[1],acl_filter->ingress_dest_ipv6_addr_end[2],acl_filter->ingress_dest_ipv6_addr_end[3],
						acl_filter->ingress_dest_ipv6_addr_end[4],acl_filter->ingress_dest_ipv6_addr_end[5],acl_filter->ingress_dest_ipv6_addr_end[6],acl_filter->ingress_dest_ipv6_addr_end[7],
						acl_filter->ingress_dest_ipv6_addr_end[8],acl_filter->ingress_dest_ipv6_addr_end[9],acl_filter->ingress_dest_ipv6_addr_end[10],acl_filter->ingress_dest_ipv6_addr_end[11],
						acl_filter->ingress_dest_ipv6_addr_end[12],acl_filter->ingress_dest_ipv6_addr_end[13],acl_filter->ingress_dest_ipv6_addr_end[14],acl_filter->ingress_dest_ipv6_addr_end[15]);
				case RG_ACL_TRANS_v4_SUBNET:
				case RG_ACL_TRANS_v4_SUBNET_v6_OTHER:
				case RG_ACL_TRANS_v4_SUBNET_v6_SUBNET:
					if(transform_type!=RG_ACL_TRANS_v6_SUBNET && transform_type!=RG_ACL_TRANS_v4_OTHER_v6_SUBNET)
					{
						//IPv4 DIP sunbnet
						acl_filter->filter_fields|=INGRESS_IPV4_DIP_RANGE_BIT;
						acl_filter->ingress_dest_ipv4_addr_start=(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_addr&rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_network_mask);
						acl_filter->ingress_dest_ipv4_addr_end=(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_addr|(~rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_network_mask));
						DEBUG("ipv4 start %x",acl_filter->ingress_dest_ipv4_addr_start);
						DEBUG("ipv4 end %x",acl_filter->ingress_dest_ipv4_addr_end);
					}
				case RG_ACL_TRANS_v4_OTHER:
				case RG_ACL_TRANS_v6_OTHER:
				case RG_ACL_TRANS_v4_OTHER_v6_OTHER:
					if(transform_type==RG_ACL_TRANS_v4_OTHER || transform_type==RG_ACL_TRANS_v4_OTHER_v6_OTHER)
					{
						acl_filter->filter_fields|=INGRESS_IPV4_TAGIF_BIT;
						acl_filter->ingress_ipv4_tagif=1;
					}
					if(transform_type==RG_ACL_TRANS_v6_OTHER || transform_type==RG_ACL_TRANS_v4_OTHER_v6_OTHER)
					{
						acl_filter->filter_fields|=INGRESS_IPV6_TAGIF_BIT;
						acl_filter->ingress_ipv6_tagif=1;
					}
					if(LAN_num-->0)
					{
						//ingress SPA without WAN port+DMAC=LGMAC
						acl_filter->filter_fields|=INGRESS_PORT_BIT;
						acl_filter->ingress_port_mask.portmask=rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->port_mask.portmask;
						acl_filter->filter_fields|=INGRESS_DMAC_BIT;
						memcpy(acl_filter->ingress_dmac.octet,rg_db.systemGlobal.lanIntfGroup[LAN_count++].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN);
						acl_filter->ingress_dmac_mask.octet[0]=0xff;
						acl_filter->ingress_dmac_mask.octet[1]=0xff;
						acl_filter->ingress_dmac_mask.octet[2]=0xff;
						acl_filter->ingress_dmac_mask.octet[3]=0xff;
						acl_filter->ingress_dmac_mask.octet[4]=0xff;
						acl_filter->ingress_dmac_mask.octet[5]=0xff;
						DEBUG("ingress pmsk=%x, dmac is %02x:%02x:%02x:%02x:%02x:%02x",acl_filter->ingress_port_mask.portmask,
							acl_filter->ingress_dmac.octet[0],acl_filter->ingress_dmac.octet[1],acl_filter->ingress_dmac.octet[2],
							acl_filter->ingress_dmac.octet[3],acl_filter->ingress_dmac.octet[4],acl_filter->ingress_dmac.octet[5]);
					}
					break;
				default:
					//Do nothing
					break;
			}
			ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);
		}
	}
#endif


	//check if pattern are all can be handle in one CF rule. (saving ACL resource)
	cf_only = _rtk_rg_checkCFAsicPatternOnly(acl_filter);
	

	for(i=0;i<MAX_ACL_TEMPLATE_SIZE;i++){
		flag_add_aclRule[i]=DISABLE;
	}


    //rtk_acl_ingress_entry_t aclRule[MAX_ACL_TEMPLATE_SIZE];
    bzero(aclRule,sizeof(rtk_acl_ingress_entry_t)*MAX_ACL_TEMPLATE_SIZE);
    bzero(aclField,sizeof(rtk_acl_field_t)*RTK_MAX_NUM_OF_ACL_RULE_FIELD*MAX_ACL_TEMPLATE_SIZE);


    //rtk_classify_field_t classifyField_1[CLASSIFY_FIELD_END];	//up
    bzero(classifyField_1, CLASSIFY_FIELD_END*sizeof(rtk_classify_field_t));


    //rtk_classify_field_t classifyField_2[CLASSIFY_FIELD_END];	//down
    bzero(classifyField_2, CLASSIFY_FIELD_END*sizeof(rtk_classify_field_t));

    //for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
    //bzero(&aclRule[i], sizeof(aclRule[i]));

    bzero(&acl_action,sizeof(acl_action));
    bzero(&empty_aclField,sizeof(empty_aclField));
    //bzero(aclField,sizeof(rtk_acl_field_t)*RTK_MAX_NUM_OF_ACL_RULE_FIELD*MAX_ACL_TEMPLATE_SIZE);
    bzero(&cfRule_1,sizeof(cfRule_1));
    bzero(&cfRule_2,sizeof(cfRule_2));
    bzero(&empty_classifyField,sizeof(empty_classifyField));

    for(i=0; i<CLASSIFY_FIELD_END; i++)
    {
        //bzero(&classifyField_1[i],sizeof(classifyField_1[i]));
        //bzero(&classifyField_2[i],sizeof(classifyField_2[i]));
    }
    bzero(&empty_aclSWEntry,sizeof(empty_aclSWEntry));

    bzero(&aclSIPv4RangeEntry, sizeof(aclSIPv4RangeEntry));
    bzero(&aclDIPv4RangeEntry, sizeof(aclDIPv4RangeEntry));
    bzero(&aclSIPv6RangeEntry, sizeof(aclSIPv6RangeEntry));
    bzero(&aclDIPv6RangeEntry, sizeof(aclDIPv6RangeEntry));
    bzero(&aclSportRangeEntry, sizeof(aclSportRangeEntry));
    bzero(&aclDportRangeEntry, sizeof(aclDportRangeEntry));
    bzero(&cfIpRangeEntry, sizeof(cfIpRangeEntry));
    bzero(&cfPortRangeEntry, sizeof(cfPortRangeEntry));
    bzero(&ingress_intf_info, sizeof(ingress_intf_info));
    bzero(&egress_intf_info, sizeof(egress_intf_info));
    bzero(&flow_direction, sizeof(flow_direction));

    //DEBUG("acl_filter->filter_fields = %x",acl_filter->filter_fields);
    //check the acl conflict field & get flow direction!
    ASSERT_EQ(_rtk_rg_conflictField_check(acl_filter),RT_ERR_RG_OK);
	/*The flow direction is assigned by user*/
	flow_direction = acl_filter->fwding_type_and_direction;

	//force DROP to add a cfRule, sync HW and SW action_drop
	if(flow_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
		flow_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP)
		flag_add_cfRule =  ENABLE;

    if(acl_filter->filter_fields & INGRESS_INTF_BIT)
    {
        //use intf idex to get  Datastruct(for check is_wan or is_lan)
        ingress_intf_idx = acl_filter->ingress_intf_idx;
        ASSERT_EQ(rtk_rg_apollo_intfInfo_find(&ingress_intf_info, &ingress_intf_idx),RT_ERR_RG_OK);

        if(ingress_intf_info.is_wan)
        {
            //Wan Interface
            if(ingress_intf_info.wan_intf.wan_intf_conf.wan_type == RTK_RG_BRIDGE)//wan bridge(L2) mode
            {

                if(ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
                {
                    //this bridge wan have ctag => pkt must have ctag & compare CVID
                    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
                    {
                        aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 1;
                        aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
                    }
                    //setup Ctag
                    aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
                    aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = TEMPLATE_CTAG;
                    aclField[TEMPLATE_CTAG].fieldUnion.data.value = ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id;
                    aclField[TEMPLATE_CTAG].fieldUnion.data.mask = 0x0fff; //do not compare with CPRI[15:3] & CFI[12]

                }
                else
                {
                    //this bridge wan do not have ctag =>  pkt must must not have ctag
                    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
                    {
                        aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 0;
                        aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
                    }
                }


            }
            else //wan route(L34) mode
            {

                if(ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
                {
                    //this route wan have ctag
                    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
                    {
                        aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 1;
                        aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
                    }

                    //setup Ctag
                    aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
                    aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = TEMPLATE_CTAG;
                    aclField[TEMPLATE_CTAG].fieldUnion.data.value = ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id;
                    aclField[TEMPLATE_CTAG].fieldUnion.data.mask = 0x0fff; //do not compare with CPRI[15:3] & CFI[12]


                    //setup gmac
                    aclField[TEMPLATE_DMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
                    aclField[TEMPLATE_DMAC0].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC0;
                    aclField[TEMPLATE_DMAC0].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[4]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[5]);
                    aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = 0xffff;

                    aclField[TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
                    aclField[TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC1;
                    aclField[TEMPLATE_DMAC1].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[2]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[3]);
                    aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = 0xffff;

                    aclField[TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
                    aclField[TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC2;
                    aclField[TEMPLATE_DMAC2].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[0]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[1]);
                    aclField[TEMPLATE_DMAC2].fieldUnion.data.mask = 0xffff;

                }
                else
                {
                    //this route wan do not have ctag
                    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
                    {
                        aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 0;
                        aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
                    }

                    //setup gmac
                    aclField[TEMPLATE_DMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
                    aclField[TEMPLATE_DMAC0].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC0;
                    aclField[TEMPLATE_DMAC0].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[4]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[5]);
                    aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = 0xffff;

                    aclField[TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
                    aclField[TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC1;
                    aclField[TEMPLATE_DMAC1].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[2]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[3]);
                    aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = 0xffff;

                    aclField[TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
                    aclField[TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC2;
                    aclField[TEMPLATE_DMAC2].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[0]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[1]);
                    aclField[TEMPLATE_DMAC2].fieldUnion.data.mask = 0xffff;
                }

            }

        }
        else
        {
            //lan Interface  => lan intf by judge active port
            for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
            {
            
            	for(j=0;j<RTK_RG_ALL_MAC_PORTMASK;j++){
					if((ingress_intf_info.lan_intf.port_mask.portmask&(1<<j)))
					{
						aclRule[i].activePorts.bits[0]|=(1<<j);
					}
				}
            }
        }
    }

    //assigned INGRESS_PORT is prior than INGRESS_INTF related ports
    if(acl_filter->filter_fields & INGRESS_PORT_BIT)
    {
    	//add active port in all related(0~3) aclRule
        for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
        {
            aclRule[i].activePorts = mac_pmsk;			
        }
		//if there are any ext_port add ext_port pattern
		if(ext_pmsk.bits[0]){	
			aclField[TEMPLATE_EXTPORTMASK].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_EXTPORTMASK].fieldUnion.pattern.fieldIdx = TEMPLATE_EXTPORTMASK;
			aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.value = 0x0;
			for(i=0;i<RTK_RG_MAX_EXT_PORT;i++){
				if(!(ext_pmsk.bits[0]&(1<<i))){
					aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.mask |= (1<<i); //band not allowed ext_port										
				}
			}
		}
    }else{
    	//default should enable all port (acll EXT_PORT are include in CPU_PORT)
    	/*
        for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
        {
            aclRule[i].activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK;//(1<<APOLLO_PORT0)|(1<<APOLLO_PORT1)|(1<<APOLLO_PORT2)|(1<<APOLLO_PORT3)|(1<<APOLLO_PORT_PON)|(1<<APOLLO_PORT_RGMII)|(1<<APOLLO_PORT_CPU);
        }
        	*/
        if(cf_only==NEED_CF_ASIC_RULE_ONLY_FLASE) //if any ACL rule are needed, we show the warning!
        	WARNING("The ACL rule without assigning any port!!! filter_fields=0x%llx",acl_filter->filter_fields);	
	
	}

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	if((acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) && (acl_filter->filter_fields&EGRESS_INTF_BIT))
#else
    if(acl_filter->filter_fields & EGRESS_INTF_BIT)
#endif		
    {
        egress_intf_idx = acl_filter->egress_intf_idx;
        //ASSERT_EQ(rtk_rg_apollo_intfInfo_find(&egress_intf_info, &egress_intf_idx),RT_ERR_RG_OK);
		ret = rtk_rg_apollo_intfInfo_find(&egress_intf_info, &egress_intf_idx);
		if(ret!=RT_ERR_RG_OK){
			//WARNING("intf[%d] is not exist! (filter_fields=0x%llx, qos_actions=0x%x, action_type=%d)",acl_filter->egress_intf_idx,acl_filter->filter_fields,acl_filter->qos_actions,acl_filter->action_type);
			return ret;
		}
		else if(egress_intf_idx!=acl_filter->egress_intf_idx)//make sure the found first valid interfcae idx is the same as rule assigned idx.
		{
			//WARNING("intf[%d] is not exist!  (filter_fields=0x%llx, qos_actions=0x%x, action_type=%d)",acl_filter->egress_intf_idx,acl_filter->filter_fields,acl_filter->qos_actions,acl_filter->action_type); 
			return RT_ERR_RG_ENTRY_NOT_EXIST;
		}


        classifyField_1[CLASSIFY_FIELD_WAN_IF].fieldType =CLASSIFY_FIELD_WAN_IF;
        classifyField_1[CLASSIFY_FIELD_WAN_IF].classify_pattern.fieldData.value=acl_filter->egress_intf_idx;
        classifyField_1[CLASSIFY_FIELD_WAN_IF].classify_pattern.fieldData.mask=0x7;

        classifyField_2[CLASSIFY_FIELD_WAN_IF].fieldType =CLASSIFY_FIELD_WAN_IF;
        classifyField_2[CLASSIFY_FIELD_WAN_IF].classify_pattern.fieldData.value=acl_filter->egress_intf_idx;
        classifyField_2[CLASSIFY_FIELD_WAN_IF].classify_pattern.fieldData.mask=0x7;

    }
	
	if(acl_filter->filter_fields & INGRESS_STREAM_ID_BIT){
        aclField[TEMPLATE_GEMPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_GEMPORT].fieldUnion.pattern.fieldIdx = TEMPLATE_GEMPORT;
        aclField[TEMPLATE_GEMPORT].fieldUnion.data.value = acl_filter->ingress_stream_id;
		if(acl_filter->ingress_stream_id_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
        	aclField[TEMPLATE_GEMPORT].fieldUnion.data.mask = 0xffff;
			acl_filter->ingress_stream_id_mask = 0xffff;
		}else{
			aclField[TEMPLATE_GEMPORT].fieldUnion.data.mask = acl_filter->ingress_stream_id_mask;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT)
	{
		for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
		{
			aclRule[i].careTag.tags[ACL_CARE_TAG_IPV4].value = acl_filter->ingress_ipv4_tagif;
			aclRule[i].careTag.tags[ACL_CARE_TAG_IPV4].mask = 0xffff;
		}	
	}
	
	if(acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)
	{
		for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
		{
			aclRule[i].careTag.tags[ACL_CARE_TAG_IPV6].value = acl_filter->ingress_ipv6_tagif;
			aclRule[i].careTag.tags[ACL_CARE_TAG_IPV6].mask = 0xffff;		
		}	
	}
	
    if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
    {
        aclField[TEMPLATE_ETHERTYPE].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_ETHERTYPE].fieldUnion.pattern.fieldIdx = TEMPLATE_ETHERTYPE;
        aclField[TEMPLATE_ETHERTYPE].fieldUnion.data.value = acl_filter->ingress_ethertype;
		if(acl_filter->ingress_ethertype_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
        	aclField[TEMPLATE_ETHERTYPE].fieldUnion.data.mask = 0xffff;
			acl_filter->ingress_ethertype_mask = 0xffff;
		}else{
			aclField[TEMPLATE_ETHERTYPE].fieldUnion.data.mask = acl_filter->ingress_ethertype_mask;
		}
		
	}
    if((acl_filter->filter_fields & INGRESS_CTAG_VID_BIT) || (acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
    	|| (acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT))
    {
		if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
		{
			/*PPPoE+CVLAN issue: PPPoE+VLAN wan side have to config Ctag as Stag*/
			if(flag_cvlan_has_duplicate_to_svlan==DISABLED){		
		        aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		        aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = TEMPLATE_CTAG;

		        if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
		        {
		            aclField[TEMPLATE_CTAG].fieldUnion.data.value |= (acl_filter->ingress_ctag_vid);
		            aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0x0fff;//do not care CFI[13]
		        }

		        if(acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
		        {
		            aclField[TEMPLATE_CTAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_pri)<<13);
		            aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0xe000;
		        }

		        if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT)
		        {
		            aclField[TEMPLATE_CTAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_cfi)<<12);
		            aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0x1000;
		        }

				for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
				{
					aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 1;
					aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
				}		
				
			}else if(flag_cvlan_has_duplicate_to_svlan==ENABLED){
				//Duplicate Ctag information to Stag
				aclField[TEMPLATE_STAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		        aclField[TEMPLATE_STAG].fieldUnion.pattern.fieldIdx = TEMPLATE_STAG;

		        if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
		        {
		            aclField[TEMPLATE_STAG].fieldUnion.data.value |= (acl_filter->ingress_ctag_vid);
		            aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0x0fff;//do not care CFI[13]
		        }

		        if(acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
		        {
		            aclField[TEMPLATE_STAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_pri)<<13);
		            aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0xe000;
		        }

		        if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT)
		        {
		            aclField[TEMPLATE_STAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_cfi)<<12);
		            aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0x1000;
		        }

				for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
				{
					aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].value = 1;
					aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].mask = 0xffff;
				}	
			}
		}
		else
		{
			/*normal version*/
			aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = TEMPLATE_CTAG;

	        if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
	        {
	            aclField[TEMPLATE_CTAG].fieldUnion.data.value |= (acl_filter->ingress_ctag_vid);
	            aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0x0fff;//do not care CFI[13]
	        }

	        if(acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
	        {
	            aclField[TEMPLATE_CTAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_pri)<<13);
	            aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0xe000;
	        }
	        
	        if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT)
	        {
	            aclField[TEMPLATE_CTAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_cfi)<<12);
	            aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0x1000;
	        }
		}

    }

	if((acl_filter->filter_fields & INGRESS_STAG_VID_BIT) || (acl_filter->filter_fields & INGRESS_STAG_PRI_BIT)
		|| (acl_filter->filter_fields & INGRESS_STAG_DEI_BIT)){
		aclField[TEMPLATE_STAG].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_STAG].fieldUnion.pattern.fieldIdx = TEMPLATE_STAG;

        if(acl_filter->filter_fields & INGRESS_STAG_VID_BIT)
        {
            aclField[TEMPLATE_STAG].fieldUnion.data.value |= (acl_filter->ingress_stag_vid);
            aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0x0fff;//do not care CFI[13]
        }

        if(acl_filter->filter_fields & INGRESS_STAG_PRI_BIT)
        {
            aclField[TEMPLATE_STAG].fieldUnion.data.value |= ((acl_filter->ingress_stag_pri)<<13);
            aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0xe000;
        }
        
        if(acl_filter->filter_fields & INGRESS_STAG_DEI_BIT)
        {
            aclField[TEMPLATE_STAG].fieldUnion.data.value |= ((acl_filter->ingress_stag_dei)<<12);
            aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0x1000;
        }
	}


	if(acl_filter->filter_fields & INGRESS_STAGIF_BIT){
		if(cf_only){//this pattern can be handled by CF for saving ACl rules
			classifyField_1[CLASSIFY_FIELD_IS_STAG].fieldType =CLASSIFY_FIELD_IS_STAG;
			classifyField_1[CLASSIFY_FIELD_IS_STAG].classify_pattern.fieldData.value=acl_filter->ingress_stagIf;
			classifyField_1[CLASSIFY_FIELD_IS_STAG].classify_pattern.fieldData.mask=0x1;

			classifyField_2[CLASSIFY_FIELD_IS_STAG].fieldType =CLASSIFY_FIELD_IS_STAG;
			classifyField_2[CLASSIFY_FIELD_IS_STAG].classify_pattern.fieldData.value=acl_filter->ingress_stagIf;
			classifyField_2[CLASSIFY_FIELD_IS_STAG].classify_pattern.fieldData.mask=0x1;
		}else{
		    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
	        {
	        	if(acl_filter->ingress_stagIf)
	            	aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].value=1;
				else
					aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].value=0;
				
				aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].mask=0xffff;
	        }
		}
	}
	if(acl_filter->filter_fields & INGRESS_CTAGIF_BIT){
		if(cf_only){//this pattern can be handled by CF for saving ACl rules
			classifyField_1[CLASSIFY_FIELD_IS_CTAG].fieldType =CLASSIFY_FIELD_IS_CTAG;
			classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.value=acl_filter->ingress_ctagIf;
			classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.mask=0x1;

			classifyField_2[CLASSIFY_FIELD_IS_CTAG].fieldType =CLASSIFY_FIELD_IS_CTAG;
			classifyField_2[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.value=acl_filter->ingress_ctagIf;
			classifyField_2[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.mask=0x1;
		}
		else{
	        for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
	        {
	        	if(acl_filter->ingress_ctagIf)
	            	aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value=1;
				else
					aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value=0;
				
	            aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;
	        }
		}

	}

	if(acl_filter->filter_fields & INTERNAL_PRI_BIT){
		classifyField_1[CLASSIFY_FIELD_INTER_PRI].fieldType =CLASSIFY_FIELD_INTER_PRI;
		classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.value=acl_filter->internal_pri;
		classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.mask=0x7;

		classifyField_2[CLASSIFY_FIELD_INTER_PRI].fieldType =CLASSIFY_FIELD_INTER_PRI;
		classifyField_2[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.value=acl_filter->internal_pri;
		classifyField_2[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.mask=0x7;
	}

	if(acl_filter->filter_fields & EGRESS_CTAG_PRI_BIT){

#ifdef CONFIG_RG_ACL_EGRESS_CPRI_PATTERN_SUPPORT		
		rtk_enable_t enable;
		int ret;
		ret=rtk_qos_1pRemarkEnable_get(RTK_RG_MAC_PORT_PON, &enable);
		assert_ok(ret);
		if(enable==DISABLED){//PON port without remarking
			//now support this pattern by two rules, 
			//(1) one assigned CF pattern  CLASSIFY_FIELD_TAG_PRI=0 && CLASSIFY_FIELD_IS_CTAG=0  for untag pkt. (untag p-bit will follow port-based-pri)
			//(2) another one assigned CF patternCLASSIFY_FIELD_TAG_PRI==assigned_pri && && CLASSIFY_FIELD_IS_CTAG=1 for ctagged pkt. (do this support after current rule added!)
			flag_egress_cpri_supported_by_multiple_cfRule_without_qosRemark=ENABLED;

			//must untagged
			classifyField_1[CLASSIFY_FIELD_IS_CTAG].fieldType =CLASSIFY_FIELD_IS_CTAG;
			classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.value=0;
			classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.mask=0x1;

			//just reserved this pattern first
			classifyField_1[CLASSIFY_FIELD_TAG_PRI].fieldType =CLASSIFY_FIELD_TAG_PRI;
			classifyField_1[CLASSIFY_FIELD_TAG_PRI].classify_pattern.fieldData.value=0;
			classifyField_1[CLASSIFY_FIELD_TAG_PRI].classify_pattern.fieldData.mask=0x0;		

		}else{//PON port with remarking			
			flag_egress_cpri_supported_by_multiple_cfRule_with_qosRemark=ENABLED;
			//just reserved this pattern first, do each CLASSIFY_FIELD_INTER_PRI pattern later 
			classifyField_1[CLASSIFY_FIELD_INTER_PRI].fieldType =CLASSIFY_FIELD_INTER_PRI;
			classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.value=acl_filter->egress_ctag_pri;
			classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.mask=0x7;

		}
#else
		WARNING("CF have no egress_stag_pri pattern, using intPri to instead!");
		classifyField_1[CLASSIFY_FIELD_INTER_PRI].fieldType =CLASSIFY_FIELD_INTER_PRI;
		classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.value=acl_filter->egress_ctag_pri;
		classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.mask=0x7;

		classifyField_2[CLASSIFY_FIELD_INTER_PRI].fieldType =CLASSIFY_FIELD_INTER_PRI;
		classifyField_2[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.value=acl_filter->egress_ctag_pri;
		classifyField_2[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.mask=0x7;
#endif

	}
	if(acl_filter->filter_fields & EGRESS_CTAG_VID_BIT){

		if(acl_filter->egress_ctag_vid_mask==0x0)
		{	//for compitible of non-mask version (mask not set means all care)
			acl_filter->egress_ctag_vid_mask = 0xfff;
		}
		
		classifyField_1[CLASSIFY_FIELD_TAG_VID].fieldType =CLASSIFY_FIELD_TAG_VID;
		classifyField_1[CLASSIFY_FIELD_TAG_VID].classify_pattern.fieldData.value=acl_filter->egress_ctag_vid;
		classifyField_1[CLASSIFY_FIELD_TAG_VID].classify_pattern.fieldData.mask=acl_filter->egress_ctag_vid_mask;
		
		classifyField_2[CLASSIFY_FIELD_TAG_VID].fieldType =CLASSIFY_FIELD_TAG_VID;
		classifyField_2[CLASSIFY_FIELD_TAG_VID].classify_pattern.fieldData.value=acl_filter->egress_ctag_vid;
		classifyField_2[CLASSIFY_FIELD_TAG_VID].classify_pattern.fieldData.mask=acl_filter->egress_ctag_vid_mask;
	}


	if(acl_filter->filter_fields & EGRESS_IP4MC_IF){
		classifyField_1[CLASSIFY_FIELD_IP4_MC].fieldType =CLASSIFY_FIELD_IP4_MC;
		classifyField_1[CLASSIFY_FIELD_IP4_MC].classify_pattern.fieldData.value=acl_filter->egress_ip4mc_if;
		classifyField_1[CLASSIFY_FIELD_IP4_MC].classify_pattern.fieldData.mask=0x1;
		
		classifyField_2[CLASSIFY_FIELD_IP4_MC].fieldType =CLASSIFY_FIELD_IP4_MC;
		classifyField_2[CLASSIFY_FIELD_IP4_MC].classify_pattern.fieldData.value=acl_filter->egress_ip4mc_if;
		classifyField_2[CLASSIFY_FIELD_IP4_MC].classify_pattern.fieldData.mask=0x1;
	}

	if(acl_filter->filter_fields & EGRESS_IP6MC_IF){
		classifyField_1[CLASSIFY_FIELD_IP6_MC].fieldType =CLASSIFY_FIELD_IP6_MC;
		classifyField_1[CLASSIFY_FIELD_IP6_MC].classify_pattern.fieldData.value=acl_filter->egress_ip6mc_if;
		classifyField_1[CLASSIFY_FIELD_IP6_MC].classify_pattern.fieldData.mask=0x1;
		
		classifyField_2[CLASSIFY_FIELD_IP6_MC].fieldType =CLASSIFY_FIELD_IP6_MC;
		classifyField_2[CLASSIFY_FIELD_IP6_MC].classify_pattern.fieldData.value=acl_filter->egress_ip6mc_if;
		classifyField_2[CLASSIFY_FIELD_IP6_MC].classify_pattern.fieldData.mask=0x1;
	}


	if(acl_filter->filter_fields & INGRESS_EGRESS_PORTIDX_BIT){ //mapping to CF pattern [2:0]UNI
		
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
			acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN){
			//US, uni bit means SPA
			classifyField_1[CLASSIFY_FIELD_UNI].fieldType =CLASSIFY_FIELD_UNI;
			classifyField_1[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.value=acl_filter->ingress_port_idx;
			if(acl_filter->ingress_port_idx_mask==0x0){
				//for compitible of non-mask version (mask not set means all care)
				classifyField_1[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=0x7;
				acl_filter->ingress_port_idx_mask = 0x7;
			}else{
				classifyField_1[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=acl_filter->ingress_port_idx_mask;
			}
				
			classifyField_2[CLASSIFY_FIELD_UNI].fieldType =CLASSIFY_FIELD_UNI;
			classifyField_2[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.value=acl_filter->ingress_port_idx;
			if(acl_filter->ingress_port_idx_mask==0x0){
				//for compitible of non-mask version (mask not set means all care)
				classifyField_2[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=0x7;
				acl_filter->ingress_port_idx_mask = 0x7;
			}else{
				classifyField_2[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=acl_filter->ingress_port_idx_mask;
			}	

		}
		else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN){
			//DS, uni bit means LUT DA
			classifyField_1[CLASSIFY_FIELD_UNI].fieldType =CLASSIFY_FIELD_UNI;
			classifyField_1[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.value=acl_filter->egress_port_idx;
			if(acl_filter->egress_port_idx_mask==0x0){
				//for compitible of non-mask version (mask not set means all care)
				classifyField_1[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=0x7;
				acl_filter->egress_port_idx_mask = 0x7;
			}
			else{
				classifyField_1[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=acl_filter->egress_port_idx_mask;
			}
				
			classifyField_2[CLASSIFY_FIELD_UNI].fieldType =CLASSIFY_FIELD_UNI;
			classifyField_2[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.value=acl_filter->egress_port_idx;
			if(acl_filter->egress_port_idx_mask==0x0){
				//for compitible of non-mask version (mask not set means all care)
				classifyField_2[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=0x7;
				acl_filter->egress_port_idx_mask = 0x7;
			}else{
				classifyField_2[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=acl_filter->egress_port_idx_mask;
			}	

		}
		else{
			WARNING("ingress_port_idx/egress_port_idx not supported in type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
		}

	}

    if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
    {
        aclField[TEMPLATE_SMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_SMAC0].fieldUnion.pattern.fieldIdx = TEMPLATE_SMAC0;
        aclField[TEMPLATE_SMAC0].fieldUnion.data.value = ((acl_filter->ingress_smac.octet[4]<<8) | (acl_filter->ingress_smac.octet[5]));
        aclField[TEMPLATE_SMAC0].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask.octet[4]<<8) | (acl_filter->ingress_smac_mask.octet[5]));

        aclField[TEMPLATE_SMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_SMAC1].fieldUnion.pattern.fieldIdx = TEMPLATE_SMAC1;
        aclField[TEMPLATE_SMAC1].fieldUnion.data.value =((acl_filter->ingress_smac.octet[2]<<8) | (acl_filter->ingress_smac.octet[3]));
        aclField[TEMPLATE_SMAC1].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask.octet[2]<<8) | (acl_filter->ingress_smac_mask.octet[3]));

        aclField[TEMPLATE_SMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_SMAC2].fieldUnion.pattern.fieldIdx = TEMPLATE_SMAC2;
        aclField[TEMPLATE_SMAC2].fieldUnion.data.value =((acl_filter->ingress_smac.octet[0]<<8) | (acl_filter->ingress_smac.octet[1]));
        aclField[TEMPLATE_SMAC2].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask.octet[0]<<8) | (acl_filter->ingress_smac_mask.octet[1]));

		//for compitible of non-mask version (mask not set means all care)
		if((acl_filter->ingress_smac_mask.octet[0] |
			acl_filter->ingress_smac_mask.octet[1] |
			acl_filter->ingress_smac_mask.octet[2] |
			acl_filter->ingress_smac_mask.octet[3] |
			acl_filter->ingress_smac_mask.octet[4] |
			acl_filter->ingress_smac_mask.octet[5]) ==0x0 ){
			aclField[TEMPLATE_SMAC0].fieldUnion.data.mask = 0xffff;
			aclField[TEMPLATE_SMAC1].fieldUnion.data.mask = 0xffff;
			aclField[TEMPLATE_SMAC2].fieldUnion.data.mask = 0xffff;

			acl_filter->ingress_smac_mask.octet[0]=0xff; 
			acl_filter->ingress_smac_mask.octet[1]=0xff;
			acl_filter->ingress_smac_mask.octet[2]=0xff;
			acl_filter->ingress_smac_mask.octet[3]=0xff;
			acl_filter->ingress_smac_mask.octet[4]=0xff;
			acl_filter->ingress_smac_mask.octet[5]=0xff;
		}


    }
    if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
    {

        aclField[TEMPLATE_DMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_DMAC0].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC0;
        aclField[TEMPLATE_DMAC0].fieldUnion.data.value =((acl_filter->ingress_dmac.octet[4]<<8) | (acl_filter->ingress_dmac.octet[5]));
        aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = ((acl_filter->ingress_dmac_mask.octet[4]<<8) | (acl_filter->ingress_dmac_mask.octet[5]));

        aclField[TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC1;
        aclField[TEMPLATE_DMAC1].fieldUnion.data.value =((acl_filter->ingress_dmac.octet[2]<<8) | (acl_filter->ingress_dmac.octet[3]));
        aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = ((acl_filter->ingress_dmac_mask.octet[2]<<8) | (acl_filter->ingress_dmac_mask.octet[3]));

        aclField[TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC2;
        aclField[TEMPLATE_DMAC2].fieldUnion.data.value =((acl_filter->ingress_dmac.octet[0]<<8) | (acl_filter->ingress_dmac.octet[1]));
        aclField[TEMPLATE_DMAC2].fieldUnion.data.mask =((acl_filter->ingress_dmac_mask.octet[0]<<8) | (acl_filter->ingress_dmac_mask.octet[1]));

		//for compitible of non-mask version (mask not set means all care)	
		if((acl_filter->ingress_dmac_mask.octet[0]|
			acl_filter->ingress_dmac_mask.octet[1]|
			acl_filter->ingress_dmac_mask.octet[2]|
			acl_filter->ingress_dmac_mask.octet[3]|
			acl_filter->ingress_dmac_mask.octet[4]|
			acl_filter->ingress_dmac_mask.octet[5])==0x0 ){
			aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = 0xffff;
			aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = 0xffff;
			aclField[TEMPLATE_DMAC2].fieldUnion.data.mask = 0xffff;

			acl_filter->ingress_dmac_mask.octet[0]=0xff;
			acl_filter->ingress_dmac_mask.octet[1]=0xff;
			acl_filter->ingress_dmac_mask.octet[2]=0xff;
			acl_filter->ingress_dmac_mask.octet[3]=0xff;
			acl_filter->ingress_dmac_mask.octet[4]=0xff;
			acl_filter->ingress_dmac_mask.octet[5]=0xff;				
		}	
    }
	
    if(acl_filter->filter_fields & INGRESS_DSCP_BIT)
    {
        aclField[TEMPLATE_FIELDSELECT04].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_FIELDSELECT04].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT04;
        aclField[TEMPLATE_FIELDSELECT04].fieldUnion.data.value = (acl_filter->ingress_dscp)<<2;
        aclField[TEMPLATE_FIELDSELECT04].fieldUnion.data.mask = 0x00fc;

	}
	if(acl_filter->filter_fields & INGRESS_TOS_BIT) 
	{      
		aclField[TEMPLATE_FIELDSELECT04].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT04].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT04;
		aclField[TEMPLATE_FIELDSELECT04].fieldUnion.data.value = acl_filter->ingress_tos;
		aclField[TEMPLATE_FIELDSELECT04].fieldUnion.data.mask = 0x00ff;
	}
	
#ifdef CONFIG_RG_PPPOE_PASSTHROUGH
	if(acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT){
		WARNING("IPv6 DSCP not supported while PPPoE Passthrought is enabled!");
		RETURN_ERR(RT_ERR_RG_FAILED);
	}
	
	if(acl_filter->filter_fields & INGRESS_IPV6_TC_BIT){
		WARNING("IPv6 TC not supported while PPPoE Passthrought is enabled!");
		RETURN_ERR(RT_ERR_RG_FAILED);
	}

	
#else
	if(acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT)
	{
        aclField[TEMPLATE_FIELDSELECT15].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_FIELDSELECT15].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT15;
        aclField[TEMPLATE_FIELDSELECT15].fieldUnion.data.value = (acl_filter->ingress_ipv6_dscp)<<6; //TC 2bits & Flow Lebel 4 bits shift(because using FS[15])
        aclField[TEMPLATE_FIELDSELECT15].fieldUnion.data.mask = 0x0fc0;
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_TC_BIT)
	{        
		aclField[TEMPLATE_FIELDSELECT15].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT15].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT15;
		aclField[TEMPLATE_FIELDSELECT15].fieldUnion.data.value = (acl_filter->ingress_ipv6_tc)<<4; //TC 2bits & Flow Lebel 4 bits shift(because using FS[15])
		aclField[TEMPLATE_FIELDSELECT15].fieldUnion.data.mask = 0x0ff0;
	}	
#endif
	
    if(acl_filter->filter_fields & INGRESS_L4_TCP_BIT)
    {
        for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
        {
            aclRule[i].careTag.tags[ACL_CARE_TAG_TCP].value=1;
            aclRule[i].careTag.tags[ACL_CARE_TAG_TCP].mask=0xffff;
        }

    }
    if(acl_filter->filter_fields & INGRESS_L4_UDP_BIT)
    {
        for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
        {
            aclRule[i].careTag.tags[ACL_CARE_TAG_UDP].value=1;
            aclRule[i].careTag.tags[ACL_CARE_TAG_UDP].mask=0xffff;
        }
    }
    if(acl_filter->filter_fields & INGRESS_L4_ICMP_BIT)
    {
		aclField[TEMPLATE_FIELDSELECT14].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT14;
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.data.value =0x01; //ICMP protocal
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.data.mask =0xff;//mask 8 bit only
    }
	
    if(acl_filter->filter_fields & INGRESS_L4_ICMPV6_BIT)
    {
		aclField[TEMPLATE_FIELDSELECT14].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT14;
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.data.value =0x3a; //ICMPv6 nextheader value
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.data.mask =0xff;//mask 8 bit only
    }

	if(acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		rtlglue_printf("INGRESS_L4_POROTCAL_VALUE_BIT");
		aclField[TEMPLATE_FIELDSELECT14].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT14;
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.data.value =acl_filter->ingress_l4_protocal; //ICMP protocal
		aclField[TEMPLATE_FIELDSELECT14].fieldUnion.data.mask =0xffff;
	}
		

#ifdef CONFIG_RG_ACL_V6SIP_FILTER_BUT_DISABLE_V6ROUTING
	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)
	{	
		ipv6_unmask_length = 0;
		ASSERT_EQ(_rtk_rg_maskLength_get_by_ipv6_range(acl_filter->ingress_dest_ipv6_addr_start,acl_filter->ingress_dest_ipv6_addr_end,&ipv6_unmask_length),RT_ERR_RG_OK);
		if(ipv6_unmask_length > 112){
			rtlglue_printf("ipv6 dest range too wide (only supported maskLength less than 112bit)!!!\n");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
		
		if(ipv6_unmask_length ==0){
			//compare DIPv6[31:0]
			aclField[TEMPLATE_IPv6DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_IPv6DIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP0;
			aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[14]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[15]);
			aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.mask = 0xffff;

			aclField[TEMPLATE_IPv6DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_IPv6DIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP1;
			aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[12]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[13]);
			aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.mask = 0xffff;

		}
		else if(ipv6_unmask_length <=16){
			//compare DIPv6[31:16]
			aclField[TEMPLATE_IPv6DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_IPv6DIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP1;
			aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[12]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[13]);
			aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.mask = 0xffff;

		}
		else if(ipv6_unmask_length <= 32){//IPv6 DIP[31:0] can be compared
			//mask range is larger than 32bit, don't care DIP[31:0]
		}
#ifdef CONFIG_EPON_FEATURE
		if(!rg_db.systemGlobal.initParam.wanPortGponMode)//EPON, FS[3] is using for patching ethertype
		{
			//IPv6 DIP[128:113] should alway be compared. 
			aclField[TEMPLATE_FIELDSELECT03].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_FIELDSELECT03].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT03;
			aclField[TEMPLATE_FIELDSELECT03].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[0]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[1]);
			aclField[TEMPLATE_FIELDSELECT03].fieldUnion.data.mask = 0xffff;
		}
#endif
		flag_ipv6_dip_need_to_trap = ENABLE;

	}
	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT)
	{	
    	//all mask are zero means all care!
		if(acl_filter->ingress_dest_ipv6_addr_mask[0]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[1]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[2]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[3]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[4]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[5]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[6]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[7]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[8]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[9]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[10]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[11]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[12]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[13]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[14]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[15]==0x0 ){

				acl_filter->ingress_dest_ipv6_addr_mask[0] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[1] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[2] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[3] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[4] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[5] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[6] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[7] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[8] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[9] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[10] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[11] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[12] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[13] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[14] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[15] = 0xff;
		}

		//compare DIPv6[31:0]
		aclField[TEMPLATE_IPv6DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_IPv6DIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP0;
		aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[14]<<8)|(acl_filter->ingress_dest_ipv6_addr[15]);
		aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[14]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[15]);
		
		aclField[TEMPLATE_IPv6DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_IPv6DIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP1;
		aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[12]<<8)|(acl_filter->ingress_dest_ipv6_addr[13]);
		aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[12]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[13]);

		//IPv6 DIP[128:113] should alway be compared. 
		aclField[TEMPLATE_FIELDSELECT03].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT03].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT03;
		aclField[TEMPLATE_FIELDSELECT03].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[0]<<8)|(acl_filter->ingress_dest_ipv6_addr[1]);
		aclField[TEMPLATE_FIELDSELECT03].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[0]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[1]);
		
		flag_ipv6_dip_need_to_trap = ENABLE;

	}

	
	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)
	{
		uint16 sipv6_first_diff_byte_mask=0;
		ipv6_unmask_length = 0; 
		ASSERT_EQ(_rtk_rg_maskLength_get_by_ipv6_range(acl_filter->ingress_src_ipv6_addr_start,acl_filter->ingress_src_ipv6_addr_end,&ipv6_unmask_length),RT_ERR_RG_OK);
		if(ipv6_unmask_length > 127){
			rtlglue_printf("ipv6 src range too wide (only supported maskLength less than 112bit)!!!\n");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}

		//set each SIP pattern
		if(ipv6_unmask_length < 16){
			aclField[TEMPLATE_IPv6SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_IPv6SIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP0;
			aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[14]<<8)|(acl_filter->ingress_src_ipv6_addr_end[15]);//v6SIP[15:0]
			aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 32){
			aclField[TEMPLATE_IPv6SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP1;
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[12]<<8)|(acl_filter->ingress_src_ipv6_addr_end[13]);//v6SIP[31:16]
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.mask = 0xffff;
			
		}


		//IPv6 DIP[127:32], use field selector 8~13,		
		if(ipv6_unmask_length < 48){
			aclField[TEMPLATE_FIELDSELECT08].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_FIELDSELECT08].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT08;
			aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[10]<<8)|(acl_filter->ingress_src_ipv6_addr_end[11]);//v6SIP[47:32]
			aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 64){
			aclField[TEMPLATE_FIELDSELECT09].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_FIELDSELECT09].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT09;
			aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[8]<<8)|(acl_filter->ingress_src_ipv6_addr_end[9]);//v6SIP[63:48]
			aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 80){
			aclField[TEMPLATE_FIELDSELECT10].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_FIELDSELECT10].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT10;
			aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[6]<<8)|(acl_filter->ingress_src_ipv6_addr_end[7]);//v6SIP[79:64]
			aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 96){
			aclField[TEMPLATE_FIELDSELECT11].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_FIELDSELECT11].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT11;
			aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[4]<<8)|(acl_filter->ingress_src_ipv6_addr_end[5]);//v6SIP[95:80]
			aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 112){
			aclField[TEMPLATE_FIELDSELECT12].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_FIELDSELECT12].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT12;
			aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[2]<<8)|(acl_filter->ingress_src_ipv6_addr_end[3]);//v6SIP[111:96]
			aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 127){
			aclField[TEMPLATE_FIELDSELECT13].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_FIELDSELECT13].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT13;
			aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[0]<<8)|(acl_filter->ingress_src_ipv6_addr_end[1]);//v6SIP[127:112]
			aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.mask = 0xffff;
		}
		
		//set first different SIP pattern mask
		for(i=0;i<16;i++){
			if(i>=(ipv6_unmask_length%16))
				sipv6_first_diff_byte_mask |= (1<<i);
		}
		
		if(0<=ipv6_unmask_length && ipv6_unmask_length<16){
			aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.mask = sipv6_first_diff_byte_mask;
		}else if(16<=ipv6_unmask_length && ipv6_unmask_length<32){
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.mask = sipv6_first_diff_byte_mask;
		}else if(32<=ipv6_unmask_length && ipv6_unmask_length<48){
			aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.mask = sipv6_first_diff_byte_mask;
		}else if(48<=ipv6_unmask_length && ipv6_unmask_length<64){
			aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.mask = sipv6_first_diff_byte_mask;
		}else if(64<=ipv6_unmask_length && ipv6_unmask_length<80){
			aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.mask = sipv6_first_diff_byte_mask;
		}else if(80<=ipv6_unmask_length && ipv6_unmask_length<96){
			aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.mask = sipv6_first_diff_byte_mask;
		}else if(96<=ipv6_unmask_length && ipv6_unmask_length<112){
			aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.mask = sipv6_first_diff_byte_mask;
		}else if(112<=ipv6_unmask_length && ipv6_unmask_length<127){
			aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.mask = sipv6_first_diff_byte_mask;
		}
		

	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT)
	{
	
		//all mask are zero means all care!
		if(acl_filter->ingress_src_ipv6_addr_mask[0]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[1]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[2]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[3]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[4]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[5]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[6]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[7]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[8]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[9]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[10]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[11]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[12]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[13]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[14]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[15]==0x0 ){

			acl_filter->ingress_src_ipv6_addr_mask[0] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[1] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[2] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[3] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[4] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[5] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[6] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[7] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[8] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[9] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[10] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[11] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[12] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[13] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[14] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[15] = 0xff;
		}
		aclField[TEMPLATE_IPv6SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_IPv6SIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP0;
		aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[14]<<8)|(acl_filter->ingress_src_ipv6_addr[15]);//v6SIP[15:0]
		aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[14]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[15]);//v6SIP[15:0];
		
		aclField[TEMPLATE_IPv6SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_IPv6SIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP1;
		aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[12]<<8)|(acl_filter->ingress_src_ipv6_addr[13]);//v6SIP[31:16]
		aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[12]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[13]);
		
		aclField[TEMPLATE_FIELDSELECT08].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT08].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT08;
		aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[10]<<8)|(acl_filter->ingress_src_ipv6_addr[11]);//v6SIP[47:32]
		aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[10]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[11]);;
	
		aclField[TEMPLATE_FIELDSELECT09].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT09].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT09;
		aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[8]<<8)|(acl_filter->ingress_src_ipv6_addr[9]);//v6SIP[63:48]
		aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[8]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[9]);;
		
		aclField[TEMPLATE_FIELDSELECT10].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT10].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT10;
		aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[6]<<8)|(acl_filter->ingress_src_ipv6_addr[7]);//v6SIP[79:64]
		aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[6]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[7]);;
	
		aclField[TEMPLATE_FIELDSELECT11].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT11].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT11;
		aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[4]<<8)|(acl_filter->ingress_src_ipv6_addr[5]);//v6SIP[95:80]
		aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[4]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[5]);;

		aclField[TEMPLATE_FIELDSELECT12].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT12].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT12;
		aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[2]<<8)|(acl_filter->ingress_src_ipv6_addr[3]);//v6SIP[111:96]
		aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[2]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[3]);;
		
		aclField[TEMPLATE_FIELDSELECT13].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT13].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT13;
		aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[0]<<8)|(acl_filter->ingress_src_ipv6_addr[1]);//v6SIP[127:112]
		aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[0]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[1]);
	
	}

#else
    if(acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)
    {
    	ipv6_unmask_length = 0;
		ASSERT_EQ(_rtk_rg_maskLength_get_by_ipv6_range(acl_filter->ingress_src_ipv6_addr_start,acl_filter->ingress_src_ipv6_addr_end,&ipv6_unmask_length),RT_ERR_RG_OK);
		if(ipv6_unmask_length > 112){
			rtlglue_printf("ipv6 src range too wide (only supported maskLength less than 112bit)!!!\n");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
		
		if(ipv6_unmask_length ==0){
			//compare SIPv6[31:0]			
			aclField[TEMPLATE_IPv6SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_IPv6SIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP0;
			aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[14]<<8)|(acl_filter->ingress_src_ipv6_addr_end[15]);
			aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.mask = 0xffff;

			aclField[TEMPLATE_IPv6SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP1;
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[12]<<8)|(acl_filter->ingress_src_ipv6_addr_end[13]);
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.mask = 0xffff;
	
        }
		else if(ipv6_unmask_length <=16){
			//compare SIPv6[31:16]
			aclField[TEMPLATE_IPv6SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP1;
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[12]<<8)|(acl_filter->ingress_src_ipv6_addr_end[13]);
			aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.mask = 0xffff;

		}
		else if(ipv6_unmask_length <= 32){//IPv6 SIP[31:0] can be compared
			//mask range is larger than 32bit, don't care SIP[31:0]
        }
#ifdef CONFIG_EPON_FEATURE
		if(!rg_db.systemGlobal.initParam.wanPortGponMode)//EPON, FS[3] is using for patching ethertype
		{
			//IPv6 SIP[128:113] should alway be compared. 
			aclField[TEMPLATE_FIELDSELECT03].fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField[TEMPLATE_FIELDSELECT03].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT03;
			aclField[TEMPLATE_FIELDSELECT03].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr_end[0]<<8)|(acl_filter->ingress_src_ipv6_addr_end[1]);
			aclField[TEMPLATE_FIELDSELECT03].fieldUnion.data.mask = 0xffff;
		}
#endif
		flag_ipv6_sip_need_to_trap = ENABLE;

    }

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT)
    {
    	//all mask are zero means all care!
		if(acl_filter->ingress_src_ipv6_addr_mask[0]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[1]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[2]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[3]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[4]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[5]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[6]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[7]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[8]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[9]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[10]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[11]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[12]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[13]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[14]==0x0 && 
			acl_filter->ingress_src_ipv6_addr_mask[15]==0x0 ){

			acl_filter->ingress_src_ipv6_addr_mask[0] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[1] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[2] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[3] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[4] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[5] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[6] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[7] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[8] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[9] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[10] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[11] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[12] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[13] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[14] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[15] = 0xff;
		}
		//compare SIPv6[31:0]			
		aclField[TEMPLATE_IPv6SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_IPv6SIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP0;
		aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[14]<<8)|(acl_filter->ingress_src_ipv6_addr[15]);
		aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[14]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[15]);

		aclField[TEMPLATE_IPv6SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_IPv6SIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6SIP1;
		aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[12]<<8)|(acl_filter->ingress_src_ipv6_addr[13]);
		aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[12]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[13]);

		//IPv6 SIP[128:113] should alway be compared. 
		aclField[TEMPLATE_FIELDSELECT03].fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField[TEMPLATE_FIELDSELECT03].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT03;
		aclField[TEMPLATE_FIELDSELECT03].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[0]<<8)|(acl_filter->ingress_src_ipv6_addr[1]);
		aclField[TEMPLATE_FIELDSELECT03].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[0]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[1]);

		flag_ipv6_sip_need_to_trap = ENABLE;
    }
    if(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)
    {
		uint16 dipv6_first_diff_byte_mask=0;
    	ipv6_unmask_length = 0;	
		ASSERT_EQ(_rtk_rg_maskLength_get_by_ipv6_range(acl_filter->ingress_dest_ipv6_addr_start,acl_filter->ingress_dest_ipv6_addr_end,&ipv6_unmask_length),RT_ERR_RG_OK);
		if(ipv6_unmask_length > 127){
			rtlglue_printf("ipv6 dest range too wide (only supported maskLength less than 112bit)!!!\n");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}

		//set each  DIP pattern
		if(ipv6_unmask_length < 16){
	        aclField[TEMPLATE_IPv6DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_IPv6DIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP0;
	        aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[14]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[15]);//v6DIP[15:0]
			aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 32){
	        aclField[TEMPLATE_IPv6DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_IPv6DIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP1;
	        aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[12]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[13]);//v6DIP[31:16]
			aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.mask = 0xffff;	
		}


        //IPv6 DIP[127:32], use field selector 8~13,		
		if(ipv6_unmask_length < 48){
	        aclField[TEMPLATE_FIELDSELECT08].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_FIELDSELECT08].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT08;
	        aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[10]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[11]);//v6DIP[47:32]
			aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.mask = 0xffff;
			
		}

		if(ipv6_unmask_length < 64){
	        aclField[TEMPLATE_FIELDSELECT09].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_FIELDSELECT09].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT09;
	        aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[8]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[9]);//v6DIP[63:48]
			aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 80){
	        aclField[TEMPLATE_FIELDSELECT10].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_FIELDSELECT10].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT10;
	        aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[6]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[7]);//v6DIP[79:64]
			aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 96){
	        aclField[TEMPLATE_FIELDSELECT11].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_FIELDSELECT11].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT11;
	        aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[4]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[5]);//v6DIP[95:80]
			aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length < 112){
	        aclField[TEMPLATE_FIELDSELECT12].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_FIELDSELECT12].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT12;
	        aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[2]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[3]);//v6DIP[111:96]
			aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.mask = 0xffff;
		}

		if(ipv6_unmask_length<127){
			//IPv6 DIP[127:112]alway be compared
	        aclField[TEMPLATE_FIELDSELECT13].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_FIELDSELECT13].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT13;
	        aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr_end[0]<<8)|(acl_filter->ingress_dest_ipv6_addr_end[1]);//v6DIP[127:112]
			aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.mask = 0xffff;
			
		}

		
		//set first different DIP pattern mask
		for(i=0;i<16;i++){
			if(i>=(ipv6_unmask_length%16))
				dipv6_first_diff_byte_mask |= (1<<i);
		}
		
		if(0<=ipv6_unmask_length && ipv6_unmask_length<16){
			aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.mask = dipv6_first_diff_byte_mask;
		}else if(16<=ipv6_unmask_length && ipv6_unmask_length<32){
			aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.mask = dipv6_first_diff_byte_mask;
		}else if(32<=ipv6_unmask_length && ipv6_unmask_length<48){
			aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.mask = dipv6_first_diff_byte_mask;
		}else if(48<=ipv6_unmask_length && ipv6_unmask_length<64){
			aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.mask = dipv6_first_diff_byte_mask;
		}else if(64<=ipv6_unmask_length && ipv6_unmask_length<80){
			aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.mask = dipv6_first_diff_byte_mask;
		}else if(80<=ipv6_unmask_length && ipv6_unmask_length<96){
			aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.mask = dipv6_first_diff_byte_mask;
		}else if(96<=ipv6_unmask_length && ipv6_unmask_length<112){
			aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.mask = dipv6_first_diff_byte_mask;
		}else if(112<=ipv6_unmask_length && ipv6_unmask_length<127){
			aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.mask = dipv6_first_diff_byte_mask;
		}
		
    }

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT)
    {
    	//all mask are zero means all care!
		if(acl_filter->ingress_dest_ipv6_addr_mask[0]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[1]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[2]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[3]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[4]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[5]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[6]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[7]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[8]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[9]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[10]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[11]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[12]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[13]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[14]==0x0 && 
			acl_filter->ingress_dest_ipv6_addr_mask[15]==0x0 ){

				acl_filter->ingress_dest_ipv6_addr_mask[0] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[1] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[2] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[3] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[4] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[5] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[6] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[7] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[8] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[9] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[10] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[11] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[12] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[13] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[14] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[15] = 0xff;
		}

        aclField[TEMPLATE_IPv6DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_IPv6DIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP0;
        aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[14]<<8)|(acl_filter->ingress_dest_ipv6_addr[15]);//v6DIP[15:0]
		aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[14]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[15]);
	
        aclField[TEMPLATE_IPv6DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_IPv6DIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP1;
        aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[12]<<8)|(acl_filter->ingress_dest_ipv6_addr[13]);//v6DIP[31:16]
		aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[12]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[13]);

        aclField[TEMPLATE_FIELDSELECT08].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_FIELDSELECT08].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT08;
        aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[10]<<8)|(acl_filter->ingress_dest_ipv6_addr[11]);//v6DIP[47:32]
		aclField[TEMPLATE_FIELDSELECT08].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[10]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[11]);
			
        aclField[TEMPLATE_FIELDSELECT09].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_FIELDSELECT09].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT09;
        aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[8]<<8)|(acl_filter->ingress_dest_ipv6_addr[9]);//v6DIP[63:48]
		aclField[TEMPLATE_FIELDSELECT09].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[8]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[9]);
	
        aclField[TEMPLATE_FIELDSELECT10].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_FIELDSELECT10].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT10;
        aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[6]<<8)|(acl_filter->ingress_dest_ipv6_addr[7]);//v6DIP[79:64]
		aclField[TEMPLATE_FIELDSELECT10].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[6]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[7]);
	
        aclField[TEMPLATE_FIELDSELECT11].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_FIELDSELECT11].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT11;
        aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[4]<<8)|(acl_filter->ingress_dest_ipv6_addr[5]);//v6DIP[95:80]
		aclField[TEMPLATE_FIELDSELECT11].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[4]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[5]);
	
        aclField[TEMPLATE_FIELDSELECT12].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_FIELDSELECT12].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT12;
        aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[2]<<8)|(acl_filter->ingress_dest_ipv6_addr[3]);//v6DIP[111:96]
		aclField[TEMPLATE_FIELDSELECT12].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[2]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[3]);
	
        aclField[TEMPLATE_FIELDSELECT13].fieldType = ACL_FIELD_PATTERN_MATCH;
        aclField[TEMPLATE_FIELDSELECT13].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT13;
        aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[0]<<8)|(acl_filter->ingress_dest_ipv6_addr[1]);//v6DIP[127:112]
		aclField[TEMPLATE_FIELDSELECT13].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[0]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[1]);
    }
#endif
    if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
    {

		if(acl_filter->ingress_src_ipv4_addr_start == acl_filter->ingress_src_ipv4_addr_end ){
			//SINGLE IP
	        aclField[TEMPLATE_IPv4SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_IPv4SIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4SIP0;
	        aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff);//SIP[15:0]
	        aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.mask = 0xffff;
			
	        aclField[TEMPLATE_IPv4SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_IPv4SIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4SIP1;
	        aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff0000)>>16;//SIP[31:16]
	        aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.mask = 0xffff;

		}else{
			//IP RANGE


			if((rg_db.systemGlobal.aclDropIpRangeBySwEnable==ENABLED && acl_filter->action_type==ACL_ACTION_TYPE_DROP) ||
				(rg_db.systemGlobal.aclPermitIpRangeBySwEnable==ENABLED && acl_filter->action_type==ACL_ACTION_TYPE_PERMIT))
			{	//force IP range handled by SW, by proc/rg/acl_drop(permit)_ip_range_rule_handle_by_sw
				ipaddr_t ipv4_aligned_range=0; 
				uint32 ipv4_aligned_mask=0;
				_rtk_rg_get_larger_ipv4_alignment_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&ipv4_aligned_range,&ipv4_aligned_mask);

				aclField[TEMPLATE_IPv4SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField[TEMPLATE_IPv4SIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4SIP0;
				aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.value = (ipv4_aligned_range & 0xffff);//SIP[15:0]
				aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.mask = (ipv4_aligned_mask&0x0000ffff);
				
				aclField[TEMPLATE_IPv4SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField[TEMPLATE_IPv4SIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4SIP1;
				aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.value = (ipv4_aligned_range & 0xffff0000)>>16;//SIP[31:16]
				aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.mask = (ipv4_aligned_mask >> 16);

				flag_ipv4_iprange_need_to_trap = ENABLE;
			}
			else //normal case
			{
				int i;
				uint32 mask=0;
				ASSERT_EQ(_rtk_rg_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&ipv4_unmask_length),RT_ERR_RG_OK);
			
				//build IPv4 Range mask
				for(i=0;i<32;i++){
					if(i>=ipv4_unmask_length){mask|=(1<<i);}
				}
		
				aclField[TEMPLATE_IPv4SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField[TEMPLATE_IPv4SIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4SIP0;
				aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff);//SIP[15:0]
				aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.mask = (mask&0x0000ffff);

				aclField[TEMPLATE_IPv4SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField[TEMPLATE_IPv4SIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4SIP1;
				aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff0000)>>16;//SIP[31:16]
				aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.mask = (mask >> 16);
			}
		}
    }
    if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
    {

		if(acl_filter->ingress_dest_ipv4_addr_start == acl_filter->ingress_dest_ipv4_addr_end ){
			//SINGLE IP
	        aclField[TEMPLATE_IPv4DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_IPv4DIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4DIP0;
	        aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff);//DIP[15:0]
	        aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.mask = 0xffff;
			
	        aclField[TEMPLATE_IPv4DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_IPv4DIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4DIP1;
	        aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff0000)>>16;//DIP[31:16]
	        aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.mask = 0xffff;

		}else{
			//IP RANGE

			if((rg_db.systemGlobal.aclDropIpRangeBySwEnable==ENABLED && acl_filter->action_type==ACL_ACTION_TYPE_DROP) ||
				(rg_db.systemGlobal.aclPermitIpRangeBySwEnable==ENABLED && acl_filter->action_type==ACL_ACTION_TYPE_PERMIT))
			{	//force IP range handled by SW, by proc/rg/acl_drop(permit)_ip_range_rule_handle_by_sw
				ipaddr_t ipv4_aligned_range=0; 
				uint32 ipv4_aligned_mask=0;
				_rtk_rg_get_larger_ipv4_alignment_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&ipv4_aligned_range,&ipv4_aligned_mask);

				aclField[TEMPLATE_IPv4DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField[TEMPLATE_IPv4DIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4DIP0;
				aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.value = (ipv4_aligned_range & 0xffff);//DIP[15:0]
				aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.mask = (ipv4_aligned_mask&0x0000ffff);
				
				aclField[TEMPLATE_IPv4DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField[TEMPLATE_IPv4DIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4DIP1;
				aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.value = (ipv4_aligned_range & 0xffff0000)>>16;//DIP[31:16]
				aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.mask = (ipv4_aligned_mask >> 16);

				flag_ipv4_iprange_need_to_trap = ENABLE;
			}
			else
			{
				int i;
				uint32 mask=0;
				ASSERT_EQ(_rtk_rg_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&ipv4_unmask_length),RT_ERR_RG_OK);
			
				//build IPv4 Range mask
				for(i=0;i<32;i++){
					if(i>=ipv4_unmask_length){mask|=(1<<i);}
				}
		
				aclField[TEMPLATE_IPv4DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField[TEMPLATE_IPv4DIP0].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4DIP0;
				aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff);//DIP[15:0]
				aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.mask = (mask&0x0000ffff);

				aclField[TEMPLATE_IPv4DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField[TEMPLATE_IPv4DIP1].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv4DIP1;
				aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff0000)>>16;//DIP[31:16]
				aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.mask = (mask >> 16);
			}
		}
    }
    if(acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT)
    {

		if(acl_filter->ingress_src_l4_port_start == acl_filter->ingress_src_l4_port_end ){
			//SINGLE IPort
	        aclField[TEMPLATE_FIELDSELECT01].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_FIELDSELECT01].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT01;
	        aclField[TEMPLATE_FIELDSELECT01].fieldUnion.data.value = acl_filter->ingress_src_l4_port_start;
	        aclField[TEMPLATE_FIELDSELECT01].fieldUnion.data.mask = 0xffff;

		}else{
			//Range Port
	        ASSERT_EQ(_rtk_rg_search_acl_empty_portTableEntry(&table_index),RT_ERR_RG_OK);

	        bzero(&aclSportRangeEntry, sizeof(aclSportRangeEntry));
	        aclSportRangeEntry.index = table_index;
	        aclSportRangeEntry.upper_bound=acl_filter->ingress_src_l4_port_end;
	        aclSportRangeEntry.lower_bound=acl_filter->ingress_src_l4_port_start;
	        aclSportRangeEntry.type = PORTRANGE_SPORT;


	        aclField[TEMPLATE_PORTRANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_PORTRANGE].fieldUnion.pattern.fieldIdx = TEMPLATE_PORTRANGE;
	        aclField[TEMPLATE_PORTRANGE].fieldUnion.data.value |= (1<<table_index);
	        aclField[TEMPLATE_PORTRANGE].fieldUnion.data.mask |= (1<<table_index);

	        aclSWEntry->hw_used_table |= ACL_USED_PORTTABLE_SPORT;
	        aclSWEntry->hw_used_table_index[ACL_USED_PORTTABLE_SPORT_INDEX] = table_index;


	        if(rtk_acl_portRange_set(&aclSportRangeEntry))
	        {
	            DEBUG("adding acl SportRange table failed");
	            RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
	        }
	        else
	        {
	            //DEBUG("adding acl SportRange to porttable[%d]",aclSportRangeEntry.index);
	        }
		}

    }
    if(acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)
    {

		if(acl_filter->ingress_dest_l4_port_start == acl_filter->ingress_dest_l4_port_end ){
			//SINGLE IPort
	        aclField[TEMPLATE_FIELDSELECT00].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_FIELDSELECT00].fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT00;
	        aclField[TEMPLATE_FIELDSELECT00].fieldUnion.data.value = acl_filter->ingress_dest_l4_port_start;
	        aclField[TEMPLATE_FIELDSELECT00].fieldUnion.data.mask = 0xffff;

		}else{
	        ASSERT_EQ(_rtk_rg_search_acl_empty_portTableEntry(&table_index),RT_ERR_RG_OK);

	        bzero(&aclDportRangeEntry, sizeof(aclDportRangeEntry));
	        aclDportRangeEntry.index = table_index;
	        aclDportRangeEntry.upper_bound=acl_filter->ingress_dest_l4_port_end;
	        aclDportRangeEntry.lower_bound=acl_filter->ingress_dest_l4_port_start;
	        aclDportRangeEntry.type = PORTRANGE_DPORT;

	        aclField[TEMPLATE_PORTRANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
	        aclField[TEMPLATE_PORTRANGE].fieldUnion.pattern.fieldIdx = TEMPLATE_PORTRANGE;
	        aclField[TEMPLATE_PORTRANGE].fieldUnion.data.value |= (1<<table_index);
	        aclField[TEMPLATE_PORTRANGE].fieldUnion.data.mask |= (1<<table_index);

	        aclSWEntry->hw_used_table |= ACL_USED_PORTTABLE_DPORT;
	        aclSWEntry->hw_used_table_index[ACL_USED_PORTTABLE_DPORT_INDEX] = table_index;

	        if(rtk_acl_portRange_set(&aclDportRangeEntry))
	        {
	            DEBUG("adding acl DportRange table failed");
	            RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
	        }
	        else
	        {
	            //DEBUG("adding acl DportRange to porttable[%d]",aclDportRangeEntry.index);
	        }
		}

    }
    if(acl_filter->filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)
    {
        ASSERT_EQ(_rtk_rg_search_cf_empty_ipTableEntry(&table_index),RT_ERR_RG_OK);

        bzero(&cfIpRangeEntry, sizeof(cfIpRangeEntry));
        cfIpRangeEntry.index = table_index;
        cfIpRangeEntry.upperIp=acl_filter->egress_src_ipv4_addr_end;
        cfIpRangeEntry.lowerIp=acl_filter->egress_src_ipv4_addr_start;
        cfIpRangeEntry.type = CLASSIFY_IPRANGE_IPV4_SIP;

        classifyField_1[CLASSIFY_FIELD_IP_RANGE].fieldType =CLASSIFY_FIELD_IP_RANGE;
        classifyField_1[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.value=((1<<3) | table_index); //(1<<3) is the valid bit
        classifyField_1[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.mask=0xf;


        classifyField_2[CLASSIFY_FIELD_IP_RANGE].fieldType =CLASSIFY_FIELD_IP_RANGE;
        classifyField_2[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.value=((1<<3) | table_index); //(1<<3) is the valid bit
        classifyField_2[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.mask=0xf;

        aclSWEntry->hw_used_table |= CF_USED_IPTABLE_IPV4SIP;
        aclSWEntry->hw_used_table_index[CF_USED_IPTABLE_IPV4SIP_INDEX] = table_index;
    }
    if(acl_filter->filter_fields & EGRESS_IPV4_DIP_RANGE_BIT)
    {
        ASSERT_EQ(_rtk_rg_search_cf_empty_ipTableEntry(&table_index),RT_ERR_RG_OK);

        bzero(&cfIpRangeEntry, sizeof(cfIpRangeEntry));
        cfIpRangeEntry.index = table_index;
        cfIpRangeEntry.upperIp=acl_filter->egress_dest_ipv4_addr_end;
        cfIpRangeEntry.lowerIp=acl_filter->egress_dest_ipv4_addr_start;
        cfIpRangeEntry.type = CLASSIFY_IPRANGE_IPV4_DIP;

        classifyField_1[CLASSIFY_FIELD_IP_RANGE].fieldType =CLASSIFY_FIELD_IP_RANGE;
        classifyField_1[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.value=((1<<3) | table_index); //(1<<3) is the valid bit
        classifyField_1[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.mask=0xf;

        classifyField_2[CLASSIFY_FIELD_IP_RANGE].fieldType =CLASSIFY_FIELD_IP_RANGE;
        classifyField_2[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.value=((1<<3) | table_index); //(1<<3) is the valid bit
        classifyField_2[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.mask=0xf;

        aclSWEntry->hw_used_table |= CF_USED_IPTABLE_IPV4DIP;
        aclSWEntry->hw_used_table_index[CF_USED_IPTABLE_IPV4DIP_INDEX] = table_index;
    }
    if(acl_filter->filter_fields & EGRESS_L4_SPORT_RANGE_BIT)
    {
        ASSERT_EQ(_rtk_rg_search_cf_empty_portTableEntry(&table_index),RT_ERR_RG_OK);

        bzero(&cfPortRangeEntry, sizeof(cfPortRangeEntry));
        cfPortRangeEntry.index = table_index;
        cfPortRangeEntry.upperPort=acl_filter->egress_src_l4_port_end;
        cfPortRangeEntry.lowerPort=acl_filter->egress_src_l4_port_start;
        cfPortRangeEntry.type = CLASSIFY_PORTRANGE_SPORT;


        classifyField_1[CLASSIFY_FIELD_PORT_RANGE].fieldType = CLASSIFY_FIELD_PORT_RANGE;
        classifyField_1[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.value = ((1<<3) | table_index); //(1<<3) is the valid bit
        classifyField_1[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.mask=0xf;

        classifyField_2[CLASSIFY_FIELD_PORT_RANGE].fieldType = CLASSIFY_FIELD_PORT_RANGE;
        classifyField_2[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.value = ((1<<3) | table_index); //(1<<3) is the valid bit
        classifyField_2[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.mask=0xf;

        aclSWEntry->hw_used_table |= CF_USED_PORTTABLE_SPORT;
        aclSWEntry->hw_used_table_index[CF_USED_PORTTABLE_SPORT_INDEX] = table_index;

    }
    if(acl_filter->filter_fields & EGRESS_L4_DPORT_RANGE_BIT)
    {
        ASSERT_EQ(_rtk_rg_search_cf_empty_portTableEntry(&table_index),RT_ERR_RG_OK);

        bzero(&cfPortRangeEntry, sizeof(cfPortRangeEntry));
        cfPortRangeEntry.index = table_index;
        cfPortRangeEntry.upperPort=acl_filter->egress_dest_l4_port_end;
        cfPortRangeEntry.lowerPort=acl_filter->egress_dest_l4_port_start;
        cfPortRangeEntry.type = CLASSIFY_PORTRANGE_DPORT;

        classifyField_1[CLASSIFY_FIELD_PORT_RANGE].fieldType = CLASSIFY_FIELD_PORT_RANGE;
        classifyField_1[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.value = ((1<<3) | table_index); //(1<<3) is the valid bit
        classifyField_1[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.mask=0xf;

        classifyField_2[CLASSIFY_FIELD_PORT_RANGE].fieldType = CLASSIFY_FIELD_PORT_RANGE;
        classifyField_2[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.value = ((1<<3) | table_index); //(1<<3) is the valid bit
        classifyField_2[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.mask=0xf;

        aclSWEntry->hw_used_table |= CF_USED_PORTTABLE_DPORT;
        aclSWEntry->hw_used_table_index[CF_USED_PORTTABLE_DPORT_INDEX] = table_index;

    }



    //fieldadd field[0~7], for aclRule_0.
    for(i=0; i<RTK_MAX_NUM_OF_ACL_RULE_FIELD; i++)
    {	
        if(memcmp(&aclField[i],&empty_aclField,sizeof(rtk_acl_field_t)))
        {
            //DEBUG("adding field[%d] to aclRule[0]",i);
            flag_add_aclRule[0] = ENABLE;
            if(rtk_acl_igrRuleField_add(&aclRule[0], &aclField[i]))
            {

                //DEBUG("adding aclRule[0] field[%d] failed",i);
                RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
            }
        }
    }
    // field[8~15] for aclRule_1,
    for(i=RTK_MAX_NUM_OF_ACL_RULE_FIELD; i<RTK_MAX_NUM_OF_ACL_RULE_FIELD*2; i++)
    {
        if(memcmp(&aclField[i],&empty_aclField,sizeof(rtk_acl_field_t)))
        {
            //DEBUG("adding field[%d] to aclRule[1]",i);
            flag_add_aclRule[1] = ENABLE;
            //fix the aclRule_1 shift field index
            aclField[i].fieldUnion.pattern.fieldIdx = aclField[i].fieldUnion.pattern.fieldIdx - RTK_MAX_NUM_OF_ACL_RULE_FIELD;
            if(rtk_acl_igrRuleField_add(&aclRule[1], &aclField[i]))
            {
                //DEBUG("adding aclRule[1] field[%d] failed",i);
                RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
            }
        }
    }
    // field[16~23] for aclRule_2
    for(i=RTK_MAX_NUM_OF_ACL_RULE_FIELD*2; i<RTK_MAX_NUM_OF_ACL_RULE_FIELD*3; i++)
    {
        if(memcmp(&aclField[i],&empty_aclField,sizeof(rtk_acl_field_t)))
        {
            //DEBUG("adding field[%d] to aclRule[2]",i);
            flag_add_aclRule[2] = ENABLE;
            //fix the aclRule_2 shift field index
            aclField[i].fieldUnion.pattern.fieldIdx = aclField[i].fieldUnion.pattern.fieldIdx - RTK_MAX_NUM_OF_ACL_RULE_FIELD*2;
            if(rtk_acl_igrRuleField_add(&aclRule[2], &aclField[i]))
            {
                //DEBUG("adding aclRule[2] field[%d] failed",i);
                RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
            }
        }
    }

    // field[24~31] for aclRule_3
    for(i=RTK_MAX_NUM_OF_ACL_RULE_FIELD*3; i<RTK_MAX_NUM_OF_ACL_RULE_FIELD*4; i++)
    {
        if(memcmp(&aclField[i],&empty_aclField,sizeof(rtk_acl_field_t)))
        {
            //DEBUG("adding field[%d] to aclRule[3]",i);
            flag_add_aclRule[3] = ENABLE;
            //fix the aclRule_3 shift field index
            aclField[i].fieldUnion.pattern.fieldIdx = aclField[i].fieldUnion.pattern.fieldIdx - RTK_MAX_NUM_OF_ACL_RULE_FIELD*3;
            if(rtk_acl_igrRuleField_add(&aclRule[3], &aclField[i]))
            {
                //DEBUG("adding aclRule[3] field[%d] failed",i);
                RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
            }
        }
    }

	//if only Activeport or FrameType are valid, set in aclRule_1.
	if(flag_add_aclRule[0]==DISABLE && flag_add_aclRule[1]==DISABLE && flag_add_aclRule[2]==DISABLE && flag_add_aclRule[3]==DISABLE){
		//any Activeport are valid
		if(aclRule[0].activePorts.bits[0]!=0x0)
			flag_add_aclRule[0] = ENABLE;

		//any FrameType are valid
		for(i=0;i<ACL_CARE_TAG_END;i++){
			if(aclRule[0].careTag.tags[i].value==1)
				flag_add_aclRule[0] = ENABLE;
		}
	}
	


    //fieldadd classifyField[all] for cfRule
    for(i=0; i<CLASSIFY_FIELD_END; i++)
    {
        if(memcmp(&classifyField_1[i],&empty_classifyField,sizeof(rtk_classify_field_t)) || memcmp(&classifyField_2[i],&empty_classifyField,sizeof(rtk_classify_field_t)))
        {
            //DEBUG("adding field[%d] to cfRule_1 & cfRule_2",i);
            flag_add_cfRule = ENABLE;

            if(i == CLASSIFY_FIELD_PORT_RANGE)
            {
                if(rtk_classify_portRange_set( &cfPortRangeEntry))
                {
                    DEBUG("adding cf portRange table failed");
                    RETURN_ERR(RT_ERR_RG_CF_PORTTABLE_ACCESS_FAILED);
                }
                else
                {
                    //DEBUG("adding cf portRange table [%d]",cfPortRangeEntry.index);
                }
            }
            if(i == CLASSIFY_FIELD_IP_RANGE)
            {
                if(rtk_classify_ipRange_set( &cfIpRangeEntry))
                {
                    DEBUG("adding cf ipRange table failed");
                    RETURN_ERR(RT_ERR_RG_CF_IPTABLE_ACCESS_FAILED);
                }
                else
                {
                    //DEBUG("adding cf ipRange table [%d]",cfIpRangeEntry.index);
                }
            }

			if(i == CLASSIFY_FIELD_ACL_HIT){
				continue;
			}

            if(rtk_classify_field_add(&cfRule_1, &classifyField_1[i]))
            {
                DEBUG("adding cfRule_1 field failed");
                RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
            }

            if(rtk_classify_field_add(&cfRule_2, &classifyField_2[i]))
            {
                DEBUG("adding cfRule_2 field failed");
                RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
            }



        }
    }


    //setup action
	if(flag_ipv6_dip_need_to_trap==ENABLE ||flag_ipv6_sip_need_to_trap==ENABLE){//support for IPv6 SIP range check: not enought field selector, so trap to CPU
		if(flag_add_cfRule)
		{
			acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
			acl_action.aclLatch = ENABLED;
		
			cfRule_1.act.usAct.log = CLASSIFY_US_LOG_ACT_ENABLE;
			cfRule_1.act.usAct.logCntIdx = 0; //[fix me] which logging counter index?
		
			//[fix me]use UNI force to CPU port, but need to verify the CPU tag reason is 0?
			cfRule_2.act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_FORCE_FORWARD;
			cfRule_2.act.dsAct.uniMask.bits[0]= (1<<RTK_RG_MAC_PORT_CPU);
		}
		else
		{
			//add log counter
			acl_action.enableAct[ACL_IGR_LOG_ACT] = ENABLE;
			acl_action.logAct.act= ACL_IGR_LOG_MIB_ACT;
			acl_action.logAct.mib=0; //[fix me] choose mib0 or another?
			//trap to CPU
			acl_action.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
			acl_action.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;		
		}
	}
	else if(flag_ipv4_iprange_need_to_trap==ENABLE)
	{
		//only support by pure ACL
		acl_action.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		acl_action.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
	}
	else
	{//other normal case
	    switch(acl_filter->action_type)
	    {
	    case ACL_ACTION_TYPE_DROP:		//F
	        if(flag_add_cfRule)
	        {
	            acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
	            acl_action.aclLatch = ENABLED;

	            cfRule_1.act.usAct.drop = CLASSIFY_DROP_ACT_ENABLE;
	            cfRule_2.act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_FORCE_FORWARD; //DS drop
	            cfRule_2.act.dsAct.uniMask.bits[0] = 0x0;
	        }
	        else
	        {
	            acl_action.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
	            acl_action.forwardAct.act = ACL_IGR_FORWARD_REDIRECT_ACT;
	            acl_action.forwardAct.portMask.bits[0]= 0x0;
	        }
	        break;
	    case ACL_ACTION_TYPE_PERMIT:	//F
	        //need to add a default drop aclRule in last entry!
	        //=> but should consider when to delete this entry (currently, reAdd will not add this default entry while no acl permit rule)

	        //setup default drop asic entry: marked by current request. if need this feature, enable downstaire code
	        //ASSERT_EQ(_rtk_rg_asic_defaultDropEntry_setup(),RT_ERR_RG_OK);

			if(flag_add_cfRule)
			{
				acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
				acl_action.aclLatch = ENABLED;

				//avoid hit other drop action rule!!!
		        acl_action.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		        acl_action.forwardAct.act= ACL_IGR_FORWARD_COPY_ACT;
				acl_action.forwardAct.portMask.bits[0]= 0x0;

				//Permit, CF do not need to do any action.
			}else{
				//avoid hit drop action by default rule!!!
		        acl_action.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		        acl_action.forwardAct.act= ACL_IGR_FORWARD_COPY_ACT;
				acl_action.forwardAct.portMask.bits[0]= 0x0;
			}
	        break;
	    case ACL_ACTION_TYPE_TRAP:
		case ACL_ACTION_TYPE_TRAP_TO_PS:
	        if(flag_add_cfRule)
	        {
	            acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
	            acl_action.aclLatch = ENABLED;
#ifdef CONFIG_DUALBAND_CONCURRENT
				//if have slave wifi, force internal pri to zero for avoding pkt directly sent to slave CPU. 
				acl_action.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
				acl_action.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				acl_action.priAct.aclPri = 0;
#endif
				//US action can not trap to CPU
	            cfRule_1.act.usAct.log = CLASSIFY_US_LOG_ACT_ENABLE;
	            cfRule_1.act.usAct.logCntIdx = 0; //[fix me] which logging counter index?

	            //[fix me]use UNI force to CPU port, but have CPU tag reason!
	            cfRule_2.act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_FORCE_FORWARD;
	            cfRule_2.act.dsAct.uniMask.bits[0]= (1<<RTK_RG_MAC_PORT_CPU);
	        }
	        else
	        {
				//trap to CPU
				acl_action.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
				acl_action.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;	
#ifdef CONFIG_DUALBAND_CONCURRENT
				//if have slave wifi, force internal pri to zero for avoding pkt directly sent to slave CPU. 
				acl_action.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
				acl_action.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				acl_action.priAct.aclPri = 0;
#endif

	        }
	        break;
	    case ACL_ACTION_TYPE_QOS: //need to prevent conflict actions
	        //*****ACL action allot:*****//
	        //***CACT=>1p remarking, SACT=>IP or DSCP remarking, PoliceACT=> sharemeter, PRIACT(aclPri)=>QID***//
	        for(i=0; i<32 ; i++)
	        {
	            switch((1<<i)&acl_filter->qos_actions)
	            {
	            case ACL_ACTION_NOP_BIT:
	                break;
	            case ACL_ACTION_DS_UNIMASK_BIT:
	            	if(flag_add_cfRule && (acl_filter->downstream_uni_portmask!=0))
	            	{
	            		cfRule_2.act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK; //DS permit(uni)
	            		cfRule_2.act.dsAct.uniMask.bits[0] = acl_filter->downstream_uni_portmask;
	            	}
	            	break;
	            case ACL_ACTION_1P_REMARKING_BIT:		//C
	                if(flag_add_cfRule)
	                {
	                    acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
	                    acl_action.aclLatch = ENABLED;

	                    //[fix me]: for CF US case , because we don't know whethere QOS_1P_remarking Enable or not
	                    //so, we both set CACT 1p reamrking, and cfPri
	                    cfRule_1.act.usAct.cAct = CLASSIFY_US_CACT_ADD_CTAG_8100;
	                    cfRule_1.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_INTERNAL;
	                    cfRule_1.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_ASSIGN;
	                    cfRule_1.act.usAct.cTagPri = acl_filter->action_dot1p_remarking_pri;

	                    cfRule_1.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
	                    cfRule_1.act.usAct.cfPri = acl_filter->action_dot1p_remarking_pri;


	                    cfRule_2.act.dsAct.cAct = CLASSIFY_DS_CACT_ADD_CTAG_8100;
	                    //[fix me] apolloMP CF DS do not have CVID from internal, so use LUT_MAC_LEARN this option.  but this may assign wrong cvid while not binding.
	                    cfRule_2.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_LUT;
	                    cfRule_2.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_ASSIGN;
	                    cfRule_2.act.dsAct.cTagPri = acl_filter->action_dot1p_remarking_pri;

	                }
	                else
	                {
	                    acl_action.enableAct[ACL_IGR_CVLAN_ACT] = ENABLE;
	                    acl_action.cvlanAct.act= ACL_IGR_CVLAN_1P_REMARK_ACT;
	                    acl_action.cvlanAct.dot1p=acl_filter->action_dot1p_remarking_pri;
	                }
	                break;
	            case ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT:
	                //[fix me] IP_PRECEDENCE_REMARKING as dscp remarking problem
	                if(flag_add_cfRule)
	                {
	                    acl_action.enableAct[ACL_IGR_INTR_ACT] = 1;
	                    acl_action.aclLatch = 1;


	                    ASSERT_EQ(_rtk_rg_search_cf_empty_dscpTableEntry(&table_index),RT_ERR_RG_OK);

	                    //record to aclSWEntry
	                    aclSWEntry->hw_used_table |= CF_USED_DSCPTABLE;
	                    aclSWEntry->hw_used_table_index[CF_USED_DSCPTABLE_INDEX] = table_index;

	                    //setup to CF
	                    if(rtk_classify_cfPri2Dscp_set(table_index, (acl_filter->action_ip_precedence_remarking_pri<<3)))
	                    {
	                        RETURN_ERR(RT_ERR_RG_CF_DSCPTABLE_ACCESS_FAILED);
	                    }


						DEBUG("set dscp_entry[%d] acl_filter->action_ip_precedence_remarking_pri=%x (acl_filter->action_ip_precedence_remarking_pri<<3)=%x",table_index,acl_filter->action_ip_precedence_remarking_pri,(acl_filter->action_ip_precedence_remarking_pri<<3));
						
	                    cfRule_1.act.usAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
	                    cfRule_1.act.usAct.cfPri= table_index;

	                    cfRule_2.act.dsAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
	                    cfRule_2.act.dsAct.cfPri = table_index;

	                }
	                else
	                {
	                    acl_action.enableAct[ACL_IGR_SVLAN_ACT] = ENABLE;
	                    acl_action.svlanAct.act = ACL_IGR_SVLAN_DSCP_REMARK_ACT;
	                    acl_action.svlanAct.dscp = (acl_filter->action_ip_precedence_remarking_pri << 3);
	                }
	                break;
	            case ACL_ACTION_DSCP_REMARKING_BIT:
	                if(flag_add_cfRule)
	                {
	                    acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
	                    acl_action.aclLatch = ENABLED;

	                    ASSERT_EQ(_rtk_rg_search_cf_empty_dscpTableEntry(&table_index),RT_ERR_RG_OK);

	                    //record to aclSWEntry
	                    aclSWEntry->hw_used_table |= CF_USED_DSCPTABLE;
	                    aclSWEntry->hw_used_table_index[CF_USED_DSCPTABLE_INDEX] = table_index;

	                    //setup to CF
	                    if(rtk_classify_cfPri2Dscp_set(table_index, acl_filter->action_dscp_remarking_pri))
	                    {
	                        RETURN_ERR(RT_ERR_RG_CF_DSCPTABLE_ACCESS_FAILED);
	                    }
	                    cfRule_1.act.usAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
	                    cfRule_1.act.usAct.cfPri = table_index;

	                    cfRule_2.act.dsAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
	                    cfRule_2.act.dsAct.cfPri = table_index;

	                }
	                else
	                {
	                    acl_action.enableAct[ACL_IGR_SVLAN_ACT] = ENABLE;
	                    acl_action.svlanAct.act = ACL_IGR_SVLAN_DSCP_REMARK_ACT;
	                    acl_action.svlanAct.dscp = acl_filter->action_dscp_remarking_pri;
	                }
	                break;
	            case ACL_ACTION_QUEUE_ID_BIT:
	                if(flag_add_cfRule)
	                {

	                    acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
	                    acl_action.aclLatch = ENABLED;
		
	          	
						//US by QID_ACT
						//cfRule_1.act.usAct.sidQidAct= CLASSIFY_US_SQID_ACT_ASSIGN_QID; //do not have this action!!!
	                    //cfRule_1.act.usAct.sidQid = acl_filter->action_queue_id;
						cfRule_1.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
						cfRule_1.act.usAct.cfPri =  acl_filter->action_queue_id;
	
						//DS by cfPriACT
	                    cfRule_2.act.dsAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
	                    cfRule_2.act.dsAct.cfPri = acl_filter->action_queue_id;
						
	                }
	                else
	                {
						acl_action.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
						acl_action.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						acl_action.priAct.aclPri = acl_filter->action_queue_id;

	                }
	                break;
	            case ACL_ACTION_SHARE_METER_BIT:
	                if(flag_add_cfRule)
	                {
						//can not use in cf, return error
						DEBUG("sharemeter action should not use with cf pattern, please check the pattern do not include egress patterns!");
						RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
	                }
	                else
	                {
						acl_action.enableAct[ACL_IGR_LOG_ACT] = ENABLE;
						acl_action.logAct.act = ACL_IGR_LOG_POLICING_ACT;
						acl_action.logAct.meter = acl_filter->action_share_meter;
	                }
	                break;

				case ACL_ACTION_STREAM_ID_OR_LLID_BIT:
					if(flag_add_cfRule)
	                {
	                    acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
	                    acl_action.aclLatch = ENABLED;
						//US by SID_ACT
						cfRule_1.act.usAct.sidQidAct = CLASSIFY_US_SQID_ACT_ASSIGN_SID;
						cfRule_1.act.usAct.sidQid =  acl_filter->action_stream_id_or_llid;
						//DS without SID, don't care
					}  
					else
	                {
						acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
						acl_action.extendAct.act = ACL_IGR_EXTEND_SID_ACT;
						acl_action.extendAct.index= acl_filter->action_stream_id_or_llid;
	                }
					break;				
				case ACL_ACTION_ACL_PRIORITY_BIT:
					if(flag_add_cfRule)
					{
						//user should avoid using this action for QoS p-bit remarking.
						//US by cfPriACT
						cfRule_1.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
						cfRule_1.act.usAct.cfPri =  acl_filter->action_acl_priority;
	
						//DS by cfPriACT
						cfRule_2.act.dsAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
						cfRule_2.act.dsAct.cfPri = acl_filter->action_acl_priority;
					}
					else
					{
						acl_action.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
						acl_action.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						acl_action.priAct.aclPri = acl_filter->action_acl_priority;
					}

	          		break;

				case ACL_ACTION_REDIRECT_BIT:
					if(flag_add_cfRule)
	                {
						
						DEBUG("ACL_ACTION_REDIRECT_BIT only support in type==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
						RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
	                }
	                else
	                {
						acl_action.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						acl_action.forwardAct.act = ACL_IGR_FORWARD_REDIRECT_ACT;
						acl_action.forwardAct.portMask.bits[0]= acl_filter->redirect_portmask;
						
						WARNING("ACL_ACTION_REDIRECT_BIT could make later ACL rule DROP/TRAP action unsless if packet hit this rule.");
	                }
					break;
				case ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT:
					flag_add_cfRule=ENABLED; //have to add cf rule
					if(flag_add_cfRule)
					{
						acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
						acl_action.aclLatch = ENABLED;
				
						cfRule_1.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
						cfRule_1.act.usAct.cfPri =	acl_filter->egress_internal_priority;
		
						cfRule_2.act.dsAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
						cfRule_2.act.dsAct.cfPri = acl_filter->egress_internal_priority;
					}
					else
					{
						//CF Pri rule must with Classify
					}
					break;

				case ACL_ACTION_ACL_CVLANTAG_BIT:
					flag_add_cfRule=ENABLED; //have to add cf rule
					acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
	                acl_action.aclLatch = ENABLED;
					
					if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
					{
						//US cvlan action
						switch(acl_filter->action_acl_cvlan.cvlanTagIfDecision){
							case ACL_CVLAN_TAGIF_TAGGING: 
								cfRule_1.act.usAct.cAct = CLASSIFY_US_CACT_ADD_CTAG_8100;

								//CVID decision
								switch(acl_filter->action_acl_cvlan.cvlanCvidDecision){
									case ACL_CVLAN_CVID_ASSIGN: 
										cfRule_1.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_ASSIGN; 
										cfRule_1.act.usAct.cTagVid = acl_filter->action_acl_cvlan.assignedCvid;
										break;
									case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: cfRule_1.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_1ST_TAG; break;
									case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: cfRule_1.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_2ND_TAG; break;
									case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: cfRule_1.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_INTERNAL; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}	
								//CPRI decision
								switch(acl_filter->action_acl_cvlan.cvlanCpriDecision){
									case ACL_CVLAN_CPRI_ASSIGN: 
										cfRule_1.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_ASSIGN;
										cfRule_1.act.usAct.cTagPri = acl_filter->action_acl_cvlan.assignedCpri;
										break;
									case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: cfRule_1.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_1ST_TAG; break;
									case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: cfRule_1.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_2ND_TAG; break;
									case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: cfRule_1.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_INTERNAL; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;

								}
							break;
							case ACL_CVLAN_TAGIF_TAGGING_WITH_C2S: cfRule_1.act.usAct.cAct = CLASSIFY_US_CACT_TRANSLATION_C2S; break;
							case ACL_CVLAN_TAGIF_UNTAG: cfRule_1.act.usAct.cAct = CLASSIFY_US_CACT_DEL_CTAG; break;
							case ACL_CVLAN_TAGIF_TRANSPARENT: cfRule_1.act.usAct.cAct = CLASSIFY_US_CACT_TRANSPARENT; break;
							case ACL_CVLAN_TAGIF_NOP: cfRule_1.act.usAct.cAct = CLASSIFY_US_CACT_NOP; break;
							default:
								RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
								break;
						}
					}
					else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
					{
						//DS cvlan action
						switch(acl_filter->action_acl_cvlan.cvlanTagIfDecision){
							case ACL_CVLAN_TAGIF_TAGGING: 
								cfRule_2.act.dsAct.cAct = CLASSIFY_DS_CACT_ADD_CTAG_8100;
								//CVID decision
							    switch(acl_filter->action_acl_cvlan.cvlanCvidDecision){
									case ACL_CVLAN_CVID_ASSIGN: 
										cfRule_2.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_ASSIGN; 
										cfRule_2.act.dsAct.cTagVid = acl_filter->action_acl_cvlan.assignedCvid;
										break;
									case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: cfRule_2.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_1ST_TAG; break;
									case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: cfRule_2.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_2ND_TAG; break;
									case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: cfRule_2.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_LUT; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								//CPRI decision
								switch(acl_filter->action_acl_cvlan.cvlanCpriDecision){
									case ACL_CVLAN_CPRI_ASSIGN:
										cfRule_2.act.dsAct.cPriAct = CLASSIFY_US_PRI_ACT_ASSIGN;
										cfRule_2.act.dsAct.cTagPri = acl_filter->action_acl_cvlan.assignedCpri;
										break;								
									case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: cfRule_2.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG; break;
									case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: cfRule_2.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG; break;
									case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: cfRule_2.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_INTERNAL; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								break;
							case ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C: cfRule_2.act.dsAct.cAct = CLASSIFY_DS_CACT_TRANSLATION_SP2C; break;
							case ACL_CVLAN_TAGIF_UNTAG: cfRule_2.act.dsAct.cAct = CLASSIFY_DS_CACT_DEL_CTAG; break;
							case ACL_CVLAN_TAGIF_TRANSPARENT: cfRule_2.act.dsAct.cAct = CLASSIFY_DS_CACT_TRANSPARENT; break;
							case ACL_CVLAN_TAGIF_NOP: cfRule_2.act.dsAct.cAct = CLASSIFY_DS_CACT_NOP; break;
							default:
								RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
								break;
						}

						
					}
					else
					{
						WARNING("ACL_ACTION_ACL_CVLANTAG_BIT only supported by type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_SVLAN_SVLAN or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_SVLAN_SVLAN");
						RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
					}

					break;//End of ACL_ACTION_ACL_CVLANTAG_BIT

					
				case ACL_ACTION_ACL_SVLANTAG_BIT:
					flag_add_cfRule=ENABLED; //have to add cf rule
					acl_action.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
	                acl_action.aclLatch = ENABLED;
					if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
					{
						switch(acl_filter->action_acl_svlan.svlanTagIfDecision){
							case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: 
								cfRule_1.act.usAct.csAct = CLASSIFY_US_CSACT_ADD_TAG_VS_TPID;
								//SVID decision
							    switch(acl_filter->action_acl_svlan.svlanSvidDecision){
									case ACL_SVLAN_SVID_ASSIGN: 
										cfRule_1.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_ASSIGN;
										cfRule_1.act.usAct.sTagVid = acl_filter->action_acl_svlan.assignedSvid;
										break;
									case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: cfRule_1.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_FROM_1ST_TAG; break;
									case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: cfRule_1.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_FROM_2ND_TAG; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								//SPRI decision
							    switch(acl_filter->action_acl_svlan.svlanSpriDecision){
									case ACL_SVLAN_SPRI_ASSIGN: 
										cfRule_1.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_ASSIGN;
										cfRule_1.act.usAct.sTagPri = acl_filter->action_acl_svlan.assignedSpri;
										break;
									case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: cfRule_1.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_1ST_TAG; break;
									case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: cfRule_1.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_2ND_TAG; break;
									case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: cfRule_1.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_INTERNAL; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								break;
							case ACL_SVLAN_TAGIF_TAGGING_WITH_8100: 
								cfRule_1.act.usAct.csAct = CLASSIFY_US_CSACT_ADD_TAG_8100;
								//SVID decision
							    switch(acl_filter->action_acl_svlan.svlanSvidDecision){
									case ACL_SVLAN_SVID_ASSIGN: 
										cfRule_1.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_ASSIGN;
										cfRule_1.act.usAct.sTagVid = acl_filter->action_acl_svlan.assignedSvid;
										break;
									case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: cfRule_1.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_FROM_1ST_TAG; break;
									case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: cfRule_1.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_FROM_2ND_TAG; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								//SPRI decision
							    switch(acl_filter->action_acl_svlan.svlanSpriDecision){
									case ACL_SVLAN_SPRI_ASSIGN: 
										cfRule_1.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_ASSIGN;
										cfRule_1.act.usAct.sTagPri = acl_filter->action_acl_svlan.assignedSpri;
										break;
									case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: cfRule_1.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_1ST_TAG; break;
									case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: cfRule_1.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_2ND_TAG; break;
									case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: cfRule_1.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_INTERNAL; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								break;
							case ACL_SVLAN_TAGIF_UNTAG: cfRule_1.act.usAct.csAct = CLASSIFY_US_CSACT_DEL_STAG; break;
							case ACL_SVLAN_TAGIF_TRANSPARENT: cfRule_1.act.usAct.csAct = CLASSIFY_US_CSACT_TRANSPARENT; break;
							case ACL_SVLAN_TAGIF_NOP: cfRule_1.act.usAct.csAct = CLASSIFY_US_CSACT_NOP; break;
							default:
								RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
								break;
						}
					}
					else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
					{
						switch(acl_filter->action_acl_svlan.svlanTagIfDecision){
							case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: 
								cfRule_2.act.dsAct.csAct = CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID;
								//SVID decision
							    switch(acl_filter->action_acl_svlan.svlanSvidDecision){
									case ACL_SVLAN_SVID_ASSIGN: 
										cfRule_2.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_ASSIGN;
										cfRule_2.act.dsAct.sTagVid = acl_filter->action_acl_svlan.assignedSvid;;
										break;										
									case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: cfRule_2.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_FROM_1ST_TAG; break;
									case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: cfRule_2.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_FROM_2ND_TAG; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								//SPRI decision
							    switch(acl_filter->action_acl_svlan.svlanSpriDecision){
									case ACL_SVLAN_SPRI_ASSIGN: 
										cfRule_2.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_ASSIGN;
										cfRule_2.act.dsAct.sTagPri = acl_filter->action_acl_svlan.assignedSpri;;
										break;				
									case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: cfRule_2.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG; break;
									case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: cfRule_2.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG; break;
									case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: cfRule_2.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_INTERNAL; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								break;

							case ACL_SVLAN_TAGIF_TAGGING_WITH_8100: 
								cfRule_2.act.dsAct.csAct = CLASSIFY_DS_CSACT_ADD_TAG_8100;
								//SVID decision
							    switch(acl_filter->action_acl_svlan.svlanSvidDecision){
									case ACL_SVLAN_SVID_ASSIGN: 
										cfRule_2.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_ASSIGN;
										cfRule_2.act.dsAct.sTagVid = acl_filter->action_acl_svlan.assignedSvid;;
										break;										
									case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: cfRule_2.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_FROM_1ST_TAG; break;
									case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: cfRule_2.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_FROM_2ND_TAG; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								//SPRI decision
							    switch(acl_filter->action_acl_svlan.svlanSpriDecision){
									case ACL_SVLAN_SPRI_ASSIGN: 
										cfRule_2.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_ASSIGN;
										cfRule_2.act.dsAct.sTagPri = acl_filter->action_acl_svlan.assignedSpri;;
										break;				
									case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: cfRule_2.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG; break;
									case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: cfRule_2.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG; break;
									case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: cfRule_2.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_INTERNAL; break;
									default:
										RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
										break;
								}
								break;
							case ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C: cfRule_2.act.dsAct.csAct = CLASSIFY_DS_CSACT_SP2C; break;
							case ACL_SVLAN_TAGIF_UNTAG: cfRule_2.act.dsAct.csAct = CLASSIFY_DS_CSACT_DEL_STAG; break;
							case ACL_SVLAN_TAGIF_TRANSPARENT: cfRule_2.act.dsAct.csAct = CLASSIFY_DS_CSACT_TRANSPARENT; break;
							case ACL_SVLAN_TAGIF_NOP: cfRule_2.act.dsAct.csAct = CLASSIFY_DS_CSACT_NOP; break;
							default:
								RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
								break;

						}
					}
					else
					{
						WARNING("ACL_ACTION_ACL_SVLANTAG_BIT only supported by type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_SVLAN_SVLAN or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_SVLAN_SVLAN");
						RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
					}	
					break;//End of ACL_ACTION_ACL_SVLANTAG_BIT


				case ACL_ACTION_ACL_INGRESS_VID_BIT:
#if 0	//mark for ingress ACL and egress CF acts at the same pkt
					if(flag_add_cfRule)
					{
						//can not use in cf, return error
						DEBUG("ingress_vid action should not use with cf pattern, please check the pattern do not include egress patterns!");
						RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
					}
					else
#endif
					{
						if(acl_action.enableAct[ACL_IGR_CVLAN_ACT] == ENABLE){
							DEBUG("ingress_vid action and 1p_remarking action are conflict! CVLAN_ACT can only support one of them at the same time");
							RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
						}
						acl_action.enableAct[ACL_IGR_CVLAN_ACT] = ENABLE;
						acl_action.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						acl_action.cvlanAct.cvid = (acl_filter->action_acl_ingress_vid & 0xfff);
					}	
					break;
					
	           	default:
	                break;
	            }

	        }
	        break;

	
	    default:
	        break;
	    }
	}

	//the same acl patterns has set before
	if(shareHwAclWithSWAclIdx!=FAIL){
		aclSWEntry->hw_aclEntry_start = rg_db.systemGlobal.acl_SW_table_entry[shareHwAclWithSWAclIdx].hw_aclEntry_start;
		aclSWEntry->hw_aclEntry_size = rg_db.systemGlobal.acl_SW_table_entry[shareHwAclWithSWAclIdx].hw_aclEntry_size;
		acl_entry_index = rg_db.systemGlobal.acl_SW_table_entry[shareHwAclWithSWAclIdx].hw_aclEntry_start;
		goto skipAclAsicSetting;
	}

    //setup ACL ASIC
    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
    {
        if(flag_add_aclRule[i])
            acl_entry_size++;
    }
    ASSERT_EQ(_rtk_rg_search_acl_empty_Entry(acl_entry_size, &acl_entry_index),RT_ERR_RG_OK);
    //setup aclSWEntry
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	if(!first_keep)
	{
		aclSWEntry->hw_aclEntry_start = acl_entry_index;
    	aclSWEntry->hw_aclEntry_size = acl_entry_size;
		first_keep=1;
	}
	else
		aclSWEntry->hw_aclEntry_size += acl_entry_size;

	DEBUG("hw_aclEntry_start is %d, hw_aclEntry_size is %d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);
#else
    aclSWEntry->hw_aclEntry_start = acl_entry_index;
    aclSWEntry->hw_aclEntry_size = acl_entry_size;
#endif
    //DEBUG("got empty start aclEntry[%d] for %d rules",acl_entry_index,acl_entry_size);

    flag_acl_first_entry = ENABLE;
	acl_entry_index_tmp = acl_entry_index;
	
    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
    {
        if(flag_add_aclRule[i])
        {
            if(flag_acl_first_entry)
            {
                //only first acl entry have to set action
                flag_acl_first_entry = DISABLE;
                aclRule[i].act = acl_action;
            }
            aclRule[i].index = acl_entry_index_tmp;
            aclRule[i].valid = ENABLE;
            aclRule[i].templateIdx = i;
			if((acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT) || (acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)){
				aclRule[i].careTag.tags[ACL_CARE_TAG_IPV4].value=ENABLED;
				aclRule[i].careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
			}
			if((acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT) || (acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)){
				aclRule[i].careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED;
				aclRule[i].careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
			}
			
            acl_entry_index_tmp++;
			ret = rtk_acl_igrRuleEntry_add(&aclRule[i]);
            if(ret)
            {
                DEBUG("adding aclRule[%d] to aclEntry[%d] failed. ret=0x%x",i,acl_entry_index_tmp,ret);
                RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
            }
        }
    }


skipAclAsicSetting:
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	acl_entry_size=0;
	if(temp_mask)goto TRANSFORM_BEGIN;	//if more bind have to add

	acl_filter->filter_fields=saved_filter_fields;
	switch(transform_type)
	{
		case RG_ACL_TRANS_v4_OTHER_v6_SUBNET:
		case RG_ACL_TRANS_v4_SUBNET_v6_OTHER:
		case RG_ACL_TRANS_v4_SUBNET_v6_SUBNET:
			transform_type-=5;
			LAN_num=rg_db.systemGlobal.lanIntfTotalNum;
			LAN_count=0;
			goto TRANSFORM_BEGIN;
		case RG_ACL_TRANS_L34_VLAN_BIND:
			temp_mask=binding_mask;
		case RG_ACL_TRANS_L34_PORT_BIND:
			LAN_num--;
			LAN_count++;
			if(LAN_num>0)goto TRANSFORM_BEGIN;		//if more LAN have to add	
			break;
		default:
			break;
	}
#endif
    bzero(&pClassifyField_acl,sizeof(pClassifyField_acl));
    // judge is there ACL latch here => fieldadd classifyField[all]
    if((flag_add_aclRule[0] || flag_add_aclRule[1] || flag_add_aclRule[2] || flag_add_aclRule[3]) && flag_add_cfRule)
    {

        classifyField_1[CLASSIFY_FIELD_ACL_HIT].fieldType = CLASSIFY_FIELD_ACL_HIT;
        classifyField_1[CLASSIFY_FIELD_ACL_HIT].classify_pattern.fieldData.value = ((1<<7) | acl_entry_index); //(1<<7) is the valid bit
        classifyField_1[CLASSIFY_FIELD_ACL_HIT].classify_pattern.fieldData.mask=0xff;


        classifyField_2[CLASSIFY_FIELD_ACL_HIT].fieldType = CLASSIFY_FIELD_ACL_HIT;
        classifyField_2[CLASSIFY_FIELD_ACL_HIT].classify_pattern.fieldData.value = ((1<<7) | acl_entry_index); //(1<<7) is the valid bit
        classifyField_2[CLASSIFY_FIELD_ACL_HIT].classify_pattern.fieldData.mask=0xff;

        pClassifyField_acl.fieldType =CLASSIFY_FIELD_ACL_HIT;
        pClassifyField_acl.classify_pattern.fieldData.value=0x81;
        pClassifyField_acl.classify_pattern.fieldData.mask=0xff;

        if(rtk_classify_field_add(&cfRule_1, &classifyField_1[CLASSIFY_FIELD_ACL_HIT]))
        {
            DEBUG("adding cfRule_1 field[CLASSIFY_FIELD_ACL_HIT] failed");
            RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
        }

        if(rtk_classify_field_add(&cfRule_2, &classifyField_2[CLASSIFY_FIELD_ACL_HIT]))
        {
            DEBUG("adding cfRule_2 field[CLASSIFY_FIELD_ACL_HIT] failed");
            RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
        }

    }



    //setup CF ASIC
    if(flag_add_cfRule==ENABLE)
    {
        switch(flow_direction)
        {
		case ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET: 
			//This type should not include CF patterns.
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET not support egress_pattern");
			break;
			
		case ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP: 
		case ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN:
#ifdef CONFIG_RG_ACL_EGRESS_CPRI_PATTERN_SUPPORT	
			if(flag_egress_cpri_supported_by_multiple_cfRule_without_qosRemark==ENABLED){
				int ret;
				//without qosRemarking, egress_ctag_pri could form taggedPri or port-based-pri
				ASSERT_EQ(_rtk_rg_search_cf_empty_Entry(2,&cf_entry_index),RT_ERR_RG_OK); //one for intPri pattern, one for CtagPri pattern

				aclSWEntry->hw_cfEntry_start = cf_entry_index;
				aclSWEntry->hw_cfEntry_size = 2;

				DEBUG("got empty cfEntry[%d] for cfRule",cf_entry_index);
				DEBUG("got empty cfEntry[%d] for cfRule",cf_entry_index+1);
				cfRule_1.index = cf_entry_index;
				cfRule_1.valid = 1;
				cfRule_1.direction = CLASSIFY_DIRECTION_US;

				ret=RTK_CLASSIFY_CFGENTRY_ADD(&cfRule_1);//rule for unctagged packet
				if(ret)
				{
					DEBUG("adding cfEntry(%d) failed(ret=0x%x)",cf_entry_index,ret);
					RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
				}


				/*following are the support of second rule for egress_cpri_pattern*/
				cfRule_1.index =(cf_entry_index+1);
				//change the compare pattern for ctagged packet egress_cpri

				//must be ctagged
				classifyField_1[CLASSIFY_FIELD_IS_CTAG].fieldType =CLASSIFY_FIELD_IS_CTAG;
				classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.value=1;
				classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.mask=0x1;

				//egress_ctag_pri will be taggged pri;
				classifyField_1[CLASSIFY_FIELD_TAG_PRI].fieldType =CLASSIFY_FIELD_TAG_PRI;
				classifyField_1[CLASSIFY_FIELD_TAG_PRI].classify_pattern.fieldData.value=acl_filter->egress_ctag_pri;;
				classifyField_1[CLASSIFY_FIELD_TAG_PRI].classify_pattern.fieldData.mask=0x7;
				if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule_1))//rule for ctagged packet
				{
					DEBUG("adding cfEntry(%d) failed",cf_entry_index);
					RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
				}
				
			}
			else if(flag_egress_cpri_supported_by_multiple_cfRule_with_qosRemark==ENABLED){
				//with qosRemarking, egress_ctag_pri could from any internalPri that remarking to this egress_ctag_pri
				int ruleSize;
				int duplicateSize;
				int intPri[8];		
				assert_ok(_rtk_rg_dot1pPriRemarking2InternalPri_search(acl_filter->egress_ctag_pri, &duplicateSize,intPri));
				ASSERT_EQ(_rtk_rg_search_cf_empty_Entry(duplicateSize,&cf_entry_index),RT_ERR_RG_OK); //size depend on how many intPri remark to this egress_ctag_pri

				for(ruleSize=0;ruleSize<duplicateSize;ruleSize++){
					DEBUG("got empty cfEntry[%d] for cfRule",cf_entry_index+ruleSize);

					cfRule_1.index = cf_entry_index+ruleSize;
					cfRule_1.valid = 1;
					cfRule_1.direction = CLASSIFY_DIRECTION_US;

					/*Need to set default action for avoiding hit CF 64-511*/
					cfRule_1.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
					cfRule_1.act.usAct.cfPri = intPri[ruleSize];
					
					classifyField_1[CLASSIFY_FIELD_INTER_PRI].fieldType =CLASSIFY_FIELD_INTER_PRI;
					classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.value=intPri[ruleSize];
					classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.mask=0x7;
					if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule_1))
					{
						DEBUG("adding cfEntry(%d) failed",cf_entry_index);
						RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
					}
				}

			}else{
				//case without assigned egress_ctag_pri
				ASSERT_EQ(_rtk_rg_search_cf_empty_Entry(1,&cf_entry_index),RT_ERR_RG_OK); //one for intPri pattern, one for CtagPri pattern

				aclSWEntry->hw_cfEntry_start = cf_entry_index;
				aclSWEntry->hw_cfEntry_size = 1;
				DEBUG("got empty cfEntry[%d] for cfRule",cf_entry_index);
				cfRule_1.index = cf_entry_index;
				cfRule_1.valid = 1;
				cfRule_1.direction = CLASSIFY_DIRECTION_US;
				if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule_1))
				{
					DEBUG("adding cfEntry(%d) failed",cf_entry_index);
					RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
				}
			}
#else
			ASSERT_EQ(_rtk_rg_search_cf_empty_Entry(1,&cf_entry_index),RT_ERR_RG_OK);

			aclSWEntry->hw_cfEntry_start = cf_entry_index;
			aclSWEntry->hw_cfEntry_size = 1;

			DEBUG("got empty cfEntry[%d] for cfRule",cf_entry_index);
			cfRule_1.index = cf_entry_index;
			cfRule_1.valid = 1;
			cfRule_1.direction = CLASSIFY_DIRECTION_US;

			//rtlglue_printf("adding cfEntry(%d) CLASSIFY_DIRECTION_US\n",cf_entry_index);
			if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule_1))
			{
				DEBUG("adding cfEntry(%d) failed",cf_entry_index);
				RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
			}
#endif
			
			break;


		case ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP: 
		case ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN: 
			//DS without egress_ctag_pri pattern, no need to support!
            ASSERT_EQ(_rtk_rg_search_cf_empty_Entry(1,&cf_entry_index),RT_ERR_RG_OK);

            aclSWEntry->hw_cfEntry_start = cf_entry_index;
            aclSWEntry->hw_cfEntry_size = 1;

            //DEBUG("got empty cfEntry[%d] for cfRule",cf_entry_index);

            cfRule_2.index = cf_entry_index;
            cfRule_2.valid = 1;
            cfRule_2.direction = CLASSIFY_DIRECTION_DS;
			//rtlglue_printf("adding cfEntry(%d) CLASSIFY_DIRECTION_DS\n",cf_entry_index);
            if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule_2))
            {
                DEBUG("adding cfEntry(%d) failed",cf_entry_index);
                RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
            }			
			break;

        default:
            RETURN_ERR(RT_ERR_RG_ACL_CF_FLOW_DIRECTION_ERROR);
            break;

        }
    }


	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
	{
		if(((acl_filter->filter_fields & INGRESS_CTAG_VID_BIT) || (acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT))&&flag_cvlan_has_duplicate_to_svlan==DISABLED){
				flag_cvlan_has_duplicate_to_svlan=ENABLED;
				//rtlglue_printf("Do CF cvlan_duplicate_to_svlan patch\n");
				goto cvlan_duplicate_to_svlan;
		}
	}

    return (RT_ERR_RG_OK);
}


static int _rtk_rg_aclSWEntry_to_asic_free(rtk_rg_aclFilterEntry_t aclSWEntry)
{
    int i,ret;
    int acl_start,acl_size,cf_start, cf_size;
    rtk_rg_aclFilterEntry_t empty_aclSWEntry;
    bzero(&empty_aclSWEntry,sizeof(empty_aclSWEntry));


    //delete acl&cf  tables
    if(aclSWEntry.hw_used_table & ACL_USED_IPTABLE_IPV4SIP)
        _rtk_rg_free_acl_ipTableEntry(aclSWEntry.hw_used_table_index[ACL_USED_IPTABLE_IPV4SIP_INDEX]);

    if(aclSWEntry.hw_used_table & ACL_USED_IPTABLE_IPV4DIP)
        _rtk_rg_free_acl_ipTableEntry(aclSWEntry.hw_used_table_index[ACL_USED_IPTABLE_IPV4DIP_INDEX]);

    if(aclSWEntry.hw_used_table & ACL_USED_IPTABLE_IPV6SIP)
        _rtk_rg_free_acl_ipTableEntry(aclSWEntry.hw_used_table_index[ACL_USED_IPTABLE_IPV6SIP_INDEX]);

    if(aclSWEntry.hw_used_table & ACL_USED_IPTABLE_IPV6DIP)
        _rtk_rg_free_acl_ipTableEntry(aclSWEntry.hw_used_table_index[ACL_USED_IPTABLE_IPV6DIP_INDEX]);

    if(aclSWEntry.hw_used_table & ACL_USED_PORTTABLE_SPORT)
        _rtk_rg_free_acl_portTableEntry(aclSWEntry.hw_used_table_index[ACL_USED_PORTTABLE_SPORT_INDEX]);

    if(aclSWEntry.hw_used_table & ACL_USED_PORTTABLE_DPORT)
        _rtk_rg_free_acl_portTableEntry(aclSWEntry.hw_used_table_index[ACL_USED_PORTTABLE_DPORT_INDEX]);

    if(aclSWEntry.hw_used_table & CF_USED_IPTABLE_IPV4SIP)
        _rtk_rg_free_cf_ipTableEntry(aclSWEntry.hw_used_table_index[CF_USED_IPTABLE_IPV4SIP_INDEX]);

    if(aclSWEntry.hw_used_table & CF_USED_IPTABLE_IPV4DIP)
        _rtk_rg_free_cf_ipTableEntry(aclSWEntry.hw_used_table_index[CF_USED_IPTABLE_IPV4DIP_INDEX]);

    if(aclSWEntry.hw_used_table & CF_USED_PORTTABLE_SPORT)
        _rtk_rg_free_cf_portTableEntry(aclSWEntry.hw_used_table_index[CF_USED_PORTTABLE_SPORT_INDEX]);

    if(aclSWEntry.hw_used_table & CF_USED_PORTTABLE_DPORT)
        _rtk_rg_free_cf_portTableEntry(aclSWEntry.hw_used_table_index[CF_USED_PORTTABLE_DPORT_INDEX]);

    if(aclSWEntry.hw_used_table & CF_USED_DSCPTABLE)
        _rtk_rg_free_cf_dscpTableEntry(aclSWEntry.hw_used_table_index[CF_USED_DSCPTABLE_INDEX]);

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

static int _rtk_rg_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int duIdx;
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
	else{
		duIdx = FAIL;//init with none-duplicated acl rule index
		if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){ //only latch to cf needs to check.
			_rtk_rg_aclDuplicatedIngressRuleCheck(*acl_filter_idx,acl_filter,&duIdx);
		}
		ASSERT_EQ(_rtk_rg_aclSWEntry_to_asic_add(acl_filter,&aclSWEntry,duIdx),RT_ERR_RG_OK);
	}
    aclSWEntry.acl_filter = *acl_filter;
	aclSWEntry.valid = RTK_RG_ENABLED;
    aclSWEntry.type = ACL_USE;


    //reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_rg_aclSWEntry_set(*acl_filter_idx,aclSWEntry),RT_ERR_RG_OK);

    //_rtk_rg_aclSWEntry_dump();

    return (RT_ERR_RG_OK);
}

int _rtk_rg_aclSWEntry_and_asic_rearrange(void)
{
    int i,j,accumulateIdx,ruleIdx;
	
#ifdef CONFIG_EPON_FEATURE
	int reg_bak;	
	//backup the original t-con bandwidth  reg value.
	assert_ok(ioal_mem32_read(0x23280,&reg_bak));
	
	if(rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//Enlarge PON threshold
		assert_ok(ioal_mem32_write(0x23280,0x595));
		RGAPI("reg[0x23280]=0x%x , set to val=0x595",reg_bak);	
	}
#endif
	
    bzero(&aclSWEntry, sizeof(rtk_rg_aclFilterEntry_t));
    bzero(&empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));
    bzero(rg_db.systemGlobal.acl_filter_temp, sizeof(rtk_rg_aclFilterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);

	//clear gloable settings, if there is wlanDev pattern rule, the wlanDevPatternValidInACL will be valid by _rtk_rg_aclSWEntry_reAdd()
	rg_db.systemGlobal.wlanDevPatternValidInACL = 0;

	//if no acl priority action in RG ACL L34 rule, no need to check l34 rules with this action  for speeding up.
	rg_db.systemGlobal.check_acl_priority_action_for_rg_acl_of_l34_type = DISABLED;

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	//20141224LUKE: clear egress intf idx
	rg_db.systemGlobal.acl_SW_egress_intf_type_zero_num = 0;
#endif
	
    //backup all acl_filter for reAdd, and clean all aclSWEntry
    for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        //backup acl_filter
        ASSERT_EQ(_rtk_rg_aclSWEntry_get(i, &aclSWEntry),RT_ERR_RG_OK);
        rg_db.systemGlobal.acl_filter_temp[i] = aclSWEntry.acl_filter;
        //clean aclSWEntry
        bzero(&aclSWEntry, sizeof(rtk_rg_aclFilterEntry_t));
        ASSERT_EQ(_rtk_rg_aclSWEntry_set(i, aclSWEntry),RT_ERR_RG_OK);
    }

    //delete ACL & CF tables
    for(i=MIN_ACL_ENTRY_INDEX; i<=MAX_ACL_ENTRY_INDEX; i++)
    {
        if(rtk_acl_igrRuleEntry_del(i))
        {
            RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
        }
    }

	//delete CF(0-63) tables, not include reserved entries
    for(i=RESERVED_CF_BEFORE; i<RESERVED_CF_AFTER; i++)
    {
        if(rtk_classify_cfgEntry_del(i))
        {
            RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
        }
    }
	//delete CF(64-511) tables, not include reserved entries => handel by API rtk_rg_classify_cfgEntry_add/del
	/*
	for(i=MIN_CF_64TO511_ENTRY_SIZE; i<MAX_CF_64TO511_ENTRY_SIZE; i++)
    {
        if(rtk_classify_cfgEntry_del(i))
        {
            RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
        }
    }
    */

    for(i=0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
    {
        if(_rtk_rg_free_acl_ipTableEntry(i))
        {
            RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
        }
    }
    for(i=0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
    {
        if(_rtk_rg_free_acl_portTableEntry(i))
        {
            RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
        }
    }

    for(i=0; i<MAX_CF_IPRANGETABLE_SIZE; i++)
    {
        if(_rtk_rg_free_cf_ipTableEntry(i))
        {
            RETURN_ERR(RT_ERR_RG_CF_IPTABLE_ACCESS_FAILED);
        }
    }

    for(i=0; i<MAX_CF_PORTRANGETABLE_SIZE; i++)
    {
        if(_rtk_rg_free_cf_portTableEntry(i))
        {
            RETURN_ERR(RT_ERR_RG_CF_PORTTABLE_ACCESS_FAILED);
        }
    }
    for(i=0; i<MAX_CF_DSCPTABLE_SIZE; i++)
    {
        if(_rtk_rg_free_cf_dscpTableEntry(i))
        {
            RETURN_ERR(RT_ERR_RG_CF_DSCPTABLE_ACCESS_FAILED);
        }
    }

    //the default drop asic entry will be  readd, if there are any permit aclSWEntry
    ASSERT_EQ(_rtk_rg_asic_defaultDropEntry_remove(),RT_ERR_RG_OK);

    //reAdd all reserve aclSWEntry:  type priority as following
		//1. ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET,
		//2. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP,
		//2. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP,
		//3. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID,
	
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight_for_egress_wan(),RT_ERR_RG_OK);

	//1-5. readd the rules of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){	
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
			break;
		ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])], &rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]),RT_ERR_RG_OK);
	}
#else
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);

	//1-4. readd the rules of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){	
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
			break;
		ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])], &rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]),RT_ERR_RG_OK);
	}
#endif	

	
	// 2. add rule of ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        if(memcmp(&rg_db.systemGlobal.acl_filter_temp[i],&empty_aclFilter,sizeof(empty_aclFilter)))
        {
        	if(rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || 
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP)
			{
					if(rg_db.systemGlobal.acl_filter_temp[i].action_type==ACL_ACTION_TYPE_QOS &&(rg_db.systemGlobal.acl_filter_temp[i].qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)){
						rg_db.systemGlobal.check_acl_priority_action_for_rg_acl_of_l34_type = ENABLED;
					}
					
		            ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
					rg_db.systemGlobal.acl_SWindex_sorting_by_weight[accumulateIdx]=i;
					accumulateIdx++;					
        	}
        }
    }
	
	// 3 add rule of type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
    for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        if(memcmp(&rg_db.systemGlobal.acl_filter_temp[i],&empty_aclFilter,sizeof(empty_aclFilter)))
        {
        	if(	rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
					if(rg_db.systemGlobal.acl_filter_temp[i].action_type==ACL_ACTION_TYPE_QOS &&(rg_db.systemGlobal.acl_filter_temp[i].qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)){
						rg_db.systemGlobal.check_acl_priority_action_for_rg_acl_of_l34_type = ENABLED;
					}
					
		            ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
					rg_db.systemGlobal.acl_SWindex_sorting_by_weight[accumulateIdx]=i;
					accumulateIdx++;
        	}
		}
    }


	//record acl ingress_vid action valid rules for speed up internalVID decision in fwdEngine. (only type of ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET can use this action)
	for(i=0,j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)//no more rules need to check
			break;
		
		ruleIdx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];	
		if(	(rg_db.systemGlobal.acl_filter_temp[ruleIdx].action_type==ACL_ACTION_TYPE_QOS) &&
			(rg_db.systemGlobal.acl_filter_temp[ruleIdx].qos_actions&ACL_ACTION_ACL_INGRESS_VID_BIT) ){
			rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action[j]=ruleIdx;
			j++;
		}
	}

#ifdef CONFIG_EPON_FEATURE
	if(rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//set PON threshold back
		assert_ok(ioal_mem32_write(0x23280,reg_bak));
		RGAPI("reg[0x23280]=0x595 , set back to val=0x%x",reg_bak); 
	}
#endif	

    return (RT_ERR_RG_OK);
}


static int32 _rtk_rg_acl_multicastTempPermit_enable(void){
	//we reserved acl[0] for permit multicast packet
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField_dmac0;

	bzero(&aclRule,sizeof(aclRule));
	bzero(&aclField_dmac0,sizeof(aclField_dmac0));

	//DEBUG("add ACL for MC temp permit");
	
	//setup MC DMAC care bit 0x01
	aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
	aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
	aclField_dmac0.fieldUnion.data.value = 0x100;
	aclField_dmac0.fieldUnion.data.mask = 0x100;
	if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
	{
		DEBUG("setup ACL for MC temporary trap failed!!!");
		return RT_ERR_RG_FAILED;
	}
	
	aclRule.valid=ENABLED;
	aclRule.index=rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx;
	aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;		
	aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
	aclRule.act.forwardAct.act= ACL_IGR_FORWARD_COPY_ACT;		
	aclRule.act.forwardAct.portMask.bits[0]=0x0;
	if(rtk_acl_igrRuleEntry_add(&aclRule))
	{
		WARNING("setup ACL for MC temporary trap failed!!!");
		return RT_ERR_RG_FAILED;
	}

	return RT_ERR_RG_OK;
}

static int32 _rtk_rg_acl_multicastTempPermit_disable(void){

	//DEBUG("remove ACL for MC temp permit");

	if(rtk_acl_igrRuleEntry_del(rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx))
	{
		WARNING("setup ACL for MC temporary trap failed!!!");
		return RT_ERR_RG_FAILED;
	}
	return RT_ERR_RG_OK;
}
#if 0
static int _rtk_rg_aclFilterAndQos_check_testChip_feature(rtk_rg_aclFilterAndQos_t *acl_filter)
{
	rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk;

	if((acl_filter->filter_fields &EGRESS_INTF_BIT)|| 
		(acl_filter->filter_fields &EGRESS_IPV4_SIP_RANGE_BIT)||
		(acl_filter->filter_fields &EGRESS_IPV4_DIP_RANGE_BIT)||
		(acl_filter->filter_fields &EGRESS_L4_SPORT_RANGE_BIT)||
		(acl_filter->filter_fields &EGRESS_L4_DPORT_RANGE_BIT)){
		DEBUG("TEST CHIP NOT SUPPORT CLASSIFY FIELD(EGRESS_IP, EGRESS_L4_PORT)!!!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if((acl_filter->filter_fields &INGRESS_PORT_BIT)){		
		ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);
		if(ext_pmsk.bits[0]!=0){
			DEBUG("TEST CHIP NOT SUPPORT ACL EXT_PORT!!!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	return (RT_ERR_RG_OK);
}
#endif

static int _rtk_rg_aclFilterAndQos_check_notSupport_feature(rtk_rg_aclFilterAndQos_t *acl_filter){
	rtk_portmask_t mac_pmsk, ext_pmsk;

	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP 
		|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP
		|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT
		|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
	{
		WARNING("egress trap is not supported in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	//make sure the CF action vid/pri decision is supported by apollo
	if(acl_filter->action_acl_svlan.svlanTagIfDecision>=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2)
	{
		WARNING("svlanTagIfDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(acl_filter->action_acl_svlan.svlanSvidDecision>=ACL_SVLAN_SVID_NOP)
	{
		WARNING("svlanSvidDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(acl_filter->action_acl_svlan.svlanSpriDecision>=ACL_SVLAN_SPRI_NOP)
	{
		WARNING("svlanSpriDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(acl_filter->action_acl_cvlan.cvlanTagIfDecision>=ACL_CVLAN_TAGIF_END)
	{
		WARNING("cvlanTagIfDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(acl_filter->action_acl_cvlan.cvlanCvidDecision>=ACL_CVLAN_CVID_NOP)
	{
		WARNING("cvlanCvidDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(acl_filter->action_acl_cvlan.cvlanCpriDecision>=ACL_CVLAN_CPRI_NOP)
	{
		WARNING("cvlanCpriDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	//20160701LUKE: disable CF modify STAG if layer2LookupMissFlood2CPU is enabled.
	if(rg_kernel.layer2LookupMissFlood2CPU==RTK_RG_ENABLED && acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN &&
		(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID||
		acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_8100||
		acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C)&&
		(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_ASSIGN||
		acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_1ST_TAG||
		acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_2ND_TAG)){
		WARNING("svlanSvidDecision not support when layer2LookupMissFlood2CPU in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if((acl_filter->action_type==ACL_ACTION_TYPE_TRAP) &&((acl_filter->filter_fields &EGRESS_INTF_BIT)||(acl_filter->filter_fields &EGRESS_IPV4_SIP_RANGE_BIT)||(acl_filter->filter_fields &EGRESS_IPV4_DIP_RANGE_BIT)||(acl_filter->filter_fields &EGRESS_L4_SPORT_RANGE_BIT)||(acl_filter->filter_fields &EGRESS_L4_DPORT_RANGE_BIT))){
		WARNING("%s(%d):TRAP TO CPU NOT SUPPORT EGRESS FEATURE!!!",__func__,__LINE__);
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if((acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE)&&(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)){
		WARNING("%s(%d):POLICY ROUTE CAN USE DIRECTION ALL PACKET ONLY!!!",__func__,__LINE__);
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(((acl_filter->filter_fields &INGRESS_IPV6_SIP_RANGE_BIT)||(acl_filter->filter_fields &INGRESS_IPV6_DIP_RANGE_BIT))&&((acl_filter->filter_fields &EGRESS_INTF_BIT)||(acl_filter->filter_fields &EGRESS_IPV4_SIP_RANGE_BIT)||(acl_filter->filter_fields &EGRESS_IPV4_DIP_RANGE_BIT)||(acl_filter->filter_fields &EGRESS_L4_SPORT_RANGE_BIT)||(acl_filter->filter_fields &EGRESS_L4_DPORT_RANGE_BIT))){
		WARNING("%s(%d):IPV6 SRC/DEST IP NOT SUPPORT WITH EGRESS PATTERN!!!",__func__,__LINE__);
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

#ifdef CONFIG_RG_PPPOE_PASSTHROUGH
	//if CONFIG_RG_PPPOE_PASSTHROUGH enabled, FS[15] is used for PPPoE SessionId => DSCPv6 is not supported.
	if((acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT)){
		WARNING("IPv6 DSCP is not supported when PPPoE Passthrought is Enabled!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if((acl_filter->filter_fields & INGRESS_IPV6_TC_BIT)){
		WARNING("IPv6 TC is not supported when PPPoE Passthrought is Enabled!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}		
#endif

	if(acl_filter->filter_fields&EGRESS_INTF_BIT){
		if(acl_filter->egress_intf_idx==7){
			WARNING("%s(%d):EGRESS_INTF pattern should not be 0 or 7!!!",__func__,__LINE__);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
	if((acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) &&
		((acl_filter->filter_fields&EGRESS_L4_DPORT_RANGE_BIT) || (acl_filter->filter_fields&EGRESS_L4_SPORT_RANGE_BIT) ||
		(acl_filter->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT) || (acl_filter->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT))
		)
		{
			WARNING("\n%s(%d):NOT SUPPORT IP/PORT range in CF(EGRESS) pattern!!!\n",__func__,__LINE__);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
#if 0
	if((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN) 
		&& (acl_filter->filter_fields&EGRESS_L4_DPORT_RANGE_BIT))
	{
		rtlglue_printf("%s(%d):EGRESS_DPORT should set as ACL INGRESS_DPORT pattern!!!",__func__,__LINE__);
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#endif
#if 0 //ACL_FWD_TYPE_DIR_INGRESS_L2_UP/DOWN type is access direct by rtk_classify_api
	//limit CF ACL_FWD_TYPE_DIR_INGRESS_L2_UP/DOWN to the CF only pattern
	if((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_L2_UP || acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_L2_DOWN)&&
		((acl_filter->filter_fields&INGRESS_PORT_BIT)||
			(acl_filter->filter_fields&INGRESS_INTF_BIT)||
			(acl_filter->filter_fields&INGRESS_SMAC_BIT)||
			(acl_filter->filter_fields&INGRESS_DMAC_BIT)||
			(acl_filter->filter_fields&INGRESS_L4_TCP_BIT)||
			(acl_filter->filter_fields&INGRESS_L4_UDP_BIT)||
			(acl_filter->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT)||
			(acl_filter->filter_fields&INGRESS_IPV6_DIP_RANGE_BIT)||
			(acl_filter->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT)||
			(acl_filter->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT)||
			(acl_filter->filter_fields&INGRESS_L4_SPORT_RANGE_BIT)||
			(acl_filter->filter_fields&INGRESS_L4_DPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_SPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_DPORT_RANGE_BIT)||
			(acl_filter->filter_fields&INGRESS_L4_ICMP_BIT)
			/* following are L2_CF supported types
			EGRESS_INTF_BIT=0x4,
			INGRESS_ETHERTYPE_BIT=0x8,
			INGRESS_CTAG_PRI_BIT=0x10,
			INGRESS_CTAG_VID_BIT=0x20,
			INGRESS_DSCP_BIT=0x100,*/
		)
	){
		WARNING("PATTERN NOT SUPPORTED BY ACL_FWD_TYPE_DIR_INGRESS_L2_UP/DOWN TYPE!!!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#endif
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	//limit CF ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET to the ACL only pattern, policy_route can assign egress_intf only
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
		if((acl_filter->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_SPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_DPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_CTAG_PRI_BIT)||
			(acl_filter->filter_fields&EGRESS_CTAG_VID_BIT))
		{
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET support ingress pattern only!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		else if(acl_filter->filter_fields&EGRESS_INTF_BIT)
		{
			if((acl_filter->filter_fields&INGRESS_CTAG_VID_BIT)||
				(acl_filter->filter_fields&INGRESS_PORT_BIT)||
				(acl_filter->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT)||
				(acl_filter->filter_fields&INGRESS_DMAC_BIT))
			{
				WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET for Egress Intf conflict ingress pattern!");
				RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
			}
		}
	}
#else
	//limit CF ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET to the ACL only pattern, policy_route can assign egress_intf only
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
		if((acl_filter->filter_fields&EGRESS_INTF_BIT)||
			(acl_filter->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_SPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_DPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_CTAG_PRI_BIT)||
			(acl_filter->filter_fields&EGRESS_CTAG_VID_BIT))
		{
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET support ingress pattern only!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
#endif

	//limit ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP/DOWN_DROP to the CF: drop action
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET ){
		if((acl_filter->filter_fields & EGRESS_IP4MC_IF) || (acl_filter->filter_fields & EGRESS_IP6MC_IF)){
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET not support EGRESS_IP4MC_IF or EGRESS_IP6MC_IF!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}



	//limit ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP/DOWN_DROP to the CF: drop action
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || 
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP){
		if(acl_filter->action_type!=ACL_ACTION_TYPE_DROP){
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP/DOWN_DROP support drop action only!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	//limit ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP to the CF: do not support egress_cat_vid & egress_ctag_pri
	//because its hard to check lan side egress port remarking or not in fwdEngine while do egress ACl check.
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP){
		if((acl_filter->filter_fields&EGRESS_CTAG_VID_BIT) || (acl_filter->filter_fields&EGRESS_CTAG_PRI_BIT)){
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP not support egress_ctag_vid and egress_ctag_pri!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	//limit ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID to the CF: streamID action
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN){
#ifdef CONFIG_GPON_FEATURE
		if(acl_filter->action_type!=ACL_ACTION_TYPE_QOS || (acl_filter->qos_actions&ACL_ACTION_STREAM_ID_OR_LLID_BIT)==0x0){
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN must assign streamID action!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
#endif	

		if(((acl_filter->filter_fields&EGRESS_INTF_BIT)==0x0)){
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN must assigned egress_intf_idx!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}

		if((acl_filter->action_acl_cvlan.cvlanTagIfDecision)==ACL_CVLAN_TAGIF_NOP){
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID must assigned action_acl_cvlan!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	//ingress or egress rule could not allow action_permit
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN){
		if(acl_filter->action_type==ACL_ACTION_TYPE_PERMIT){
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN must assigned action_acl_cvlan!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
	//while CF drop, check if any pure ACL action_permit... permit may not work
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || 
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP){
		int i;
		rtk_rg_aclFilterAndQos_t *aclPara;
		if(acl_filter->action_type==ACL_ACTION_TYPE_DROP)	//check re-arranged SW ACL with action==PERMIT
		{
			for(i=0;i<rg_db.systemGlobal.acl_SW_table_entry_size;i++)
			{
				if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED){
					aclPara = &(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter);
					if(aclPara->action_type==ACL_ACTION_TYPE_PERMIT && aclPara->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
						WARNING("type ACL_FWD_TYPE_DIR_EGRESS_L34_DROP, ACL has one entry assigned ACL_ACTION_TYPE_PERMIT; PERMIT may be no use");
						break;
					}
				}
			}
		}
	}
	//while pure ACL permit, check if any ACL_CF drop, action_permit may not work
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
			int i;
			rtk_rg_aclFilterAndQos_t *aclPara;
		if(acl_filter->action_type==ACL_ACTION_TYPE_PERMIT)	//check re-arranged SW ACL with action==DROP
		{
			for(i=0;i<rg_db.systemGlobal.acl_SW_table_entry_size;i++)
			{
				if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED){
					aclPara = &(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter);
					if(aclPara->fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
						aclPara->fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP){
							WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, ACL has one entry assigned US/DS ACL_CF DROP; PERMIT may be no use");
							break;						
					}
				}
			}
		}
	}

	//only ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET have weight
	if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET && acl_filter->acl_weight!=0){
		WARNING("only type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET can assign acl_weight!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#if 0	//
	if((acl_filter->qos_actions&ACL_ACTION_ACL_INGRESS_VID_BIT) && (acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)){
		rtlglue_printf("%s(%d):ACL_ACTION_ACL_INGRESS_VID_BIT only support in type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET !!!",__func__,__LINE__);
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#endif
#ifdef CONFIG_EPON_FEATURE
	if(!rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//FS[3] used for EPON
#ifdef CONFIG_RG_ACL_V6SIP_FILTER_BUT_DISABLE_V6ROUTING
		//V6DIP[127:112] can not be support
		if((acl_filter->filter_fields&INGRESS_IPV6_DIP_RANGE_BIT)||(acl_filter->filter_fields&INGRESS_IPV6_DIP_BIT))
		{
			WARNING("IPV6 DIP can not support with EPON!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
#else
		//V6SIP[127:112] can not be support
		if((acl_filter->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT)||(acl_filter->filter_fields&INGRESS_IPV6_SIP_BIT))
		{
			WARNING("IPV6 SIP can not support with EPON!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
#endif
	}
#endif

	//comes from slave CPU with trap action will leading RX_Desc extPortmask message removed!
	//reserved ACL (RTK_RG_ACLANDCF_RESERVED_WIFI_MASTER_EXTPORT_PATCH/ RTK_RG_ACLANDCF_RESERVED_WIFI_SLAVE_EXTPORT_PATCH) will failed, so this kind of rules not support
	//However, extPort BC packet can be trapped originally, so support this kind of case but without add HW ACL rule. 
	ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);
	if((ext_pmsk.bits[0]&(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU))/*EXT1*/) &&(acl_filter->action_type==ACL_ACTION_TYPE_TRAP || acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS)){
		
		if((acl_filter->ingress_port_mask.portmask&RTK_RG_ALL_MAC_PORTMASK)!=0x0){
			WARNING("Macport and extPort should be seperate as two ACL rule!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		
		if(	(acl_filter->filter_fields&INGRESS_DMAC_BIT) && 
			acl_filter->ingress_dmac.octet[0]==0xff	&&
			acl_filter->ingress_dmac.octet[1]==0xff	&&
			acl_filter->ingress_dmac.octet[2]==0xff	&&
			acl_filter->ingress_dmac.octet[3]==0xff	&&
			acl_filter->ingress_dmac.octet[4]==0xff	&&
			acl_filter->ingress_dmac.octet[5]==0xff	){ //from extPort (can not include macPort) but BC case is supported
			//do nothing , without add ACL. Just let BC packet trapped originally.
		}
		else{
			WARNING("ACL not support packet comes from slave TRAP!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
	
	if((acl_filter->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT)!=0x0 && (acl_filter->filter_fields&INGRESS_IPV6_SIP_BIT)!=0x0){
		WARNING("ACL not support INGRESS_IPV6_SIP_RANGE_BIT and INGRESS_IPV6_SIP_BIT at the same time !");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	
	if((acl_filter->filter_fields&INGRESS_IPV6_DIP_RANGE_BIT)!=0x0 && (acl_filter->filter_fields&INGRESS_IPV6_DIP_BIT)!=0x0){
		WARNING("ACL not support INGRESS_IPV6_DIP_RANGE_BIT and INGRESS_IPV6_DIP_BIT at the same time !");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->qos_actions & ACL_ACTION_QUEUE_ID_BIT){ //queueID action just supported in SW(fwdEngine)
		if(acl_filter->ingress_port_mask.portmask!=(1<<RTK_RG_PORT_CPU)){
			WARNING("ACL ACL_ACTION_QUEUE_ID_BIT only supported in CPU port");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);	
		}
	}
	
	if(acl_filter->qos_actions & ACL_ACTION_REDIRECT_BIT){ //queueID action just supported in SW(fwdEngine)
		if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
			WARNING("ACL action ACL_ACTION_REDIRECT_BIT only supported in type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);	
		}
	}

	if(acl_filter->qos_actions & ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT){ //cfpri action just supported in ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_XXXX (type1~4)
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
			WARNING("ACL action ACL_ACTION_REDIRECT_BIT only supported in type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_XXXX");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);	
		}
	}

	if(acl_filter->qos_actions & ACL_ACTION_ACL_PRIORITY_BIT){ //acl priority action just supported in ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET (type0)
		if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
			WARNING("ACL action ACL_ACTION_ACL_PRIORITY_BIT only supported in type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);	
		}
	}

	if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
		if((acl_filter->qos_actions & ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT)&&((acl_filter->qos_actions & ACL_ACTION_DSCP_REMARKING_BIT)||(acl_filter->qos_actions & ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT)))
		{
			//both dscp remarking and cfpri assign is using cfpri_register.		
			WARNING("ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT and ACL_ACTION_DSCP_REMARKING_BIT can not support at the same time!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);	
		}
	}
		
	if(acl_filter->filter_fields&INGRESS_WLANDEV_BIT){
		if(acl_filter->ingress_port_mask.portmask!=(1<<RTK_RG_EXT_PORT0)){
			WARNING("ingress_wlanDev only supported by ingress_portmask 0x%x",(1<<RTK_RG_EXT_PORT0));
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	//CF latched by ACL
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN){
		if(acl_filter->qos_actions == ACL_ACTION_ACL_CVLANTAG_BIT){	//qos action: CTAG
			rtk_rg_enable_t en;
			int i;
			
			//for(i=0;i<=RTK_RG_MAC_PORT3;i++){
#if defined(CONFIG_RTL9602C_SERIES)			
			i=RTK_RG_MAC_PORT_PON;
#else				
			for(i=RTK_RG_MAC_PORT_PON;i<=RTK_RG_MAC_PORT_RGMII;i++)
#endif				
			{ //consider wan only
				rtk_rg_apollo_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(i,&en);
				if(en!=0){
					if(acl_filter->action_acl_cvlan.cvlanCpriDecision!=ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI){
						WARNING("CPRI may be replaced by Internal PRI");
						RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
					}
				}
			}
		}
	}
	
	if(acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE){
		int i;
		i = acl_filter->action_policy_route_wan;
		if(i>=MAX_NETIF_SW_TABLE_SIZE){	//check wan idx number 1st
			WARNING("PolicyRoute wan idx should be less than %d",MAX_NETIF_SW_TABLE_SIZE);
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
		if((rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_STATIC && rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.static_info.gateway_ipv4_addr==0) ||	//static ip
		(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP && rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr == 0)  	//dhcp
		){
			WARNING("PolicyRoute will fail while wan(%d) has no nexthop and DIP is not in ARP table!",i);
			//RETURN_ERR(RT_ERR_RG_POLICYROUTE_NO_NH);
		}
	}

	//EGRESS_CTAG_PRI_BIT for unatgged LAN case should force all LAN port-based priority to zero. (because unatg Lan => tag Wan, the p-bit will comes from port-based priority.) 
	if(acl_filter->filter_fields&EGRESS_CTAG_PRI_BIT){
		int pri;
		//make sure all Lan port-based priority set to zero.
		rtk_rg_apollo_qosPortBasedPriority_get(RTK_RG_MAC_PORT0,&pri);
		if(pri!=0){
			WARNING("ingress_wctag_pri only supported with all LAN port-based priority are zero. port[%d] port-baed vid=%d",RTK_RG_MAC_PORT0,pri);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}

		rtk_rg_apollo_qosPortBasedPriority_get(RTK_RG_MAC_PORT1,&pri);
		if(pri!=0){
			WARNING("ingress_wctag_pri only supported with all LAN port-based priority are zero. port[%d] port-baed vid=%d",RTK_RG_MAC_PORT1,pri);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}

#if !defined(CONFIG_RTL9602C_SERIES)

		rtk_rg_apollo_qosPortBasedPriority_get(RTK_RG_MAC_PORT2,&pri);
		if(pri!=0){
			WARNING("ingress_wctag_pri only supported with all LAN port-based priority are zero. port[%d] port-baed vid=%d",RTK_RG_MAC_PORT2,pri);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}

		rtk_rg_apollo_qosPortBasedPriority_get(RTK_RG_MAC_PORT3,&pri);
		if(pri!=0){
			WARNING("ingress_wctag_pri only supported with all LAN port-based priority are zero. port[%d] port-baed vid=%d",RTK_RG_MAC_PORT3,pri);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
#endif
		
	}


	if(rg_db.systemGlobal.aclDropIpRangeBySwEnable){		
		if((acl_filter->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT)||(acl_filter->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT)){//if trigger the IP range trap, make sure it only support by pure ACL and action must be drop
			if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
				if(((acl_filter->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT) &&
					(acl_filter->ingress_src_ipv4_addr_start!=acl_filter->ingress_src_ipv4_addr_end)) ||
				  ((acl_filter->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT) &&
					(acl_filter->ingress_dest_ipv4_addr_start!=acl_filter->ingress_dest_ipv4_addr_end)))
				{
				WARNING("IP range drop can only support with type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
				RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
			}
		}
	}
	}

	if(rg_db.systemGlobal.aclPermitIpRangeBySwEnable){		
		if((acl_filter->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT)||(acl_filter->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT)){//if trigger the IP range trap, make sure it only support by pure ACL and action must be permit
			if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
				if(((acl_filter->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT) &&
					(acl_filter->ingress_src_ipv4_addr_start!=acl_filter->ingress_src_ipv4_addr_end)) ||
				  ((acl_filter->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT) &&
					(acl_filter->ingress_dest_ipv4_addr_start!=acl_filter->ingress_dest_ipv4_addr_end)))
				{
				WARNING("IP range permit can only support with type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
				RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
			}
				
			}
		}
	}

	#if 0 //20151221, Cheney: remove this limitation because some customers may use MacBasedDecision + force forward.
	//not support force forward ports when macBasedDecision enabled. 
	//Because downstream binding may have more than one packet to lan port, the force forward can not define the behavior.
	if(rg_db.systemGlobal.initParam.macBasedTagDecision==1)
	{
		if(acl_filter->qos_actions&ACL_ACTION_DS_UNIMASK_BIT)
		{
			WARNING("ACL_ACTION_DS_UNIMASK_BIT is not supported when macBasedTagDecision enabled");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		
		if(acl_filter->qos_actions&ACL_ACTION_REDIRECT_BIT)
		{
			WARNING("ACL_ACTION_REDIRECT_BIT is not supported when macBasedTagDecision enabled");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
	#endif
	
	return (RT_ERR_RG_OK);
}

static int32 _rtk_rg_apollo_classifyEntry_notSupport(rtk_rg_classifyEntry_t *classifyFilter){

	//make sure the CF action vid/pri decision is supported by apollo
	if(classifyFilter->action_svlan.svlanTagIfDecision>=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2)
	{
		WARNING("svlanTagIfDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(classifyFilter->action_svlan.svlanSvidDecision>=ACL_SVLAN_SVID_NOP)
	{
		WARNING("svlanSvidDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(classifyFilter->action_svlan.svlanSpriDecision>=ACL_SVLAN_SPRI_NOP)
	{
		WARNING("svlanSpriDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(classifyFilter->action_cvlan.cvlanTagIfDecision>=ACL_CVLAN_TAGIF_END)
	{
		WARNING("cvlanTagIfDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(classifyFilter->action_cvlan.cvlanCvidDecision>=ACL_CVLAN_CVID_NOP)
	{
		WARNING("cvlanCvidDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(classifyFilter->action_cvlan.cvlanCpriDecision>=ACL_CVLAN_CPRI_NOP)
	{
		WARNING("cvlanCpriDecision not support in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	//20160701LUKE: disable CF modify STAG if layer2LookupMissFlood2CPU is enabled.
	if(rg_kernel.layer2LookupMissFlood2CPU==RTK_RG_ENABLED && classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM &&
		(classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID||
		classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_8100||
		classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C)&&
		(classifyFilter->action_svlan.svlanSvidDecision==ACL_SVLAN_SVID_ASSIGN||
		classifyFilter->action_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_1ST_TAG||
		classifyFilter->action_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_2ND_TAG)){
		WARNING("svlanSvidDecision not support when layer2LookupMissFlood2CPU in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	
	//fwdEngine only support action of  STAG/CTAG/SID/DROP
	if((classifyFilter->us_action_field & (CF_US_ACTION_CFPRI_BIT|CF_US_ACTION_DSCP_BIT|CF_US_ACTION_LOG_BIT))!=0x0){
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if((classifyFilter->ds_action_field & (CF_DS_ACTION_CFPRI_BIT|CF_DS_ACTION_DSCP_BIT))){
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	//pure CF
	if((classifyFilter->us_action_field & CF_US_ACTION_CTAG_BIT)){
		rtk_rg_enable_t en;
		int i;
		//for(i=0;i<=RTK_RG_MAC_PORT3;i++){
#if defined(CONFIG_RTL9602C_SERIES)		
		i=RTK_RG_MAC_PORT_PON;
#else
		for(i=RTK_RG_MAC_PORT_PON;i<=RTK_RG_MAC_PORT_RGMII;i++)
#endif
		{		//consider WAN only?!
			rtk_rg_apollo_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(i,&en);
			if(en!=0){
				if(classifyFilter->action_cvlan.cvlanCpriDecision!=ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI){
					WARNING("CPRI may be replaced by Internal PRI");
					RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
				}
			}
		}
	}
	return (RT_ERR_RG_OK);
}



#if 1 /*****(3)literomeDriver mapping APIs*****/
#endif
int32 _rtk_rg_apollo_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int i,ret;
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


	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("RG ACL parameter check:\n");
		_dump_rg_acl_entry_content(NULL, acl_filter);
	}


	//let multicast packet permit while H/W ACL rearrange to make sure IPTV flow smooth
	ret = _rtk_rg_acl_multicastTempPermit_enable();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;


	ret = _rtk_rg_aclFilterAndQos_check_notSupport_feature(acl_filter);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	
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
		ret=_rtk_rg_aclSWEntry_to_asic_add(acl_filter,&aclSWEntry,FAIL);
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

	
	//count the total aclSWEntry size
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED)
			rg_db.systemGlobal.acl_SW_table_entry_size++;
	}
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

	//delete the MC temp permit
	_rtk_rg_acl_multicastTempPermit_disable();

	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL[%d] success!\n",*acl_filter_idx);
	}
    return (RT_ERR_RG_OK);

aclFailed:
	//delete the MC temp permit
	_rtk_rg_acl_multicastTempPermit_disable();
	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL Faild! ret= 0x%x\n",ret);
	}
	
	return ret;
	
}

int32 _rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx)
{
	int i,ret;
    rtk_rg_aclFilterEntry_t aclSWEntry,empty_aclSWEntry;
    bzero(&aclSWEntry,sizeof(aclSWEntry));
    bzero(&empty_aclSWEntry,sizeof(empty_aclSWEntry));

	//Check rg has been init
    if(rg_db.systemGlobal.vlanInit==0)
        RETURN_ERR(RT_ERR_RG_NOT_INIT);

	//let multicast packet permit while H/W ACL rearrange to make sure IPTV flow smooth
	ret = _rtk_rg_acl_multicastTempPermit_enable();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

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

	//count the total aclSWEntry size
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED)
			rg_db.systemGlobal.acl_SW_table_entry_size++;
	}
	DEBUG("delete aclSWEntry[%d]",acl_filter_idx);
	ret = _rtk_rg_shortCut_clear();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;
	
    //_rtk_rg_aclSWEntry_dump();
    
    //delete the MC temp permit
	_rtk_rg_acl_multicastTempPermit_disable();
    return (RT_ERR_RG_OK);

aclFailed:
	//delete the MC temp permit
	_rtk_rg_acl_multicastTempPermit_disable();
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

static rtk_classify_field_t classifyField[CF_PATTERN_END];
int32 _rtk_rg_apollo_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter){

	int i;
	int ret;
	rtk_classify_cfg_t cfRule;
	rtk_classify_field_t empty_classifyField;

	bzero(&cfRule,sizeof(rtk_classify_cfg_t));
	bzero(&empty_classifyField,sizeof(rtk_classify_field_t));
	bzero(classifyField,sizeof(rtk_classify_field_t)*CF_PATTERN_END);

	ASSERT_EQ(_rtk_rg_apollo_classifyEntry_notSupport(classifyFilter),RT_ERR_RG_OK);

	//check index range
	if(classifyFilter->index < CF_FOR_L2_FLOW_INDEX_START || classifyFilter->index > CF_FOR_L2_FLOW_INDEX_END){
		rtlglue_printf("Invalid classify rule index, range should be in %d~%d\n",CF_FOR_L2_FLOW_INDEX_START,CF_FOR_L2_FLOW_INDEX_END);
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}
	
	//check flow & action
	if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
		//ds should not have action
		if(classifyFilter->ds_action_field) RETURN_ERR(RT_ERR_RG_INVALID_PARAM);		
	}else if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
		//us should not have action
		if(classifyFilter->us_action_field) RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}else{
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	//set index
	cfRule.valid = ENABLED;
	cfRule.index = classifyFilter->index;

	//set direction
	if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
		cfRule.direction = CLASSIFY_DIRECTION_US;
	}else{
		cfRule.direction = CLASSIFY_DIRECTION_DS;
	}

	//set patterns
	if(classifyFilter->filter_fields & EGRESS_ETHERTYPR_BIT){
		classifyField[CF_PATTERN_ETHERTYPE].fieldType=CLASSIFY_FIELD_ETHERTYPE;
		classifyField[CF_PATTERN_ETHERTYPE].classify_pattern.fieldData.value=(classifyFilter->etherType & 0xffff);
		if(classifyFilter->etherType_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
			classifyField[CF_PATTERN_ETHERTYPE].classify_pattern.fieldData.mask=0xffff;		
		}else{
			classifyField[CF_PATTERN_ETHERTYPE].classify_pattern.fieldData.mask=(classifyFilter->etherType_mask & 0xffff);	
		}
	}
	if(classifyFilter->filter_fields & EGRESS_GEMIDX_BIT){
		classifyField[CF_PATTERN_GEMIDX_OR_LLID].fieldType=CLASSIFY_FIELD_TOS_DSIDX;
		classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.value=(classifyFilter->gemidx & 0xff);
		if(classifyFilter->gemidx_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
			classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.mask=0xff;	
		}else{
			classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.mask=(classifyFilter->gemidx_mask&0xff);	
		}
	}

	if(classifyFilter->filter_fields & EGRESS_LLID_BIT){
		classifyField[CF_PATTERN_GEMIDX_OR_LLID].fieldType=CLASSIFY_FIELD_TOS_DSIDX;
		classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.value=(classifyFilter->llid & 0xf); //llid 4 bits only
		classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.mask=0xf;	
	}
	if(classifyFilter->filter_fields & EGRESS_TAGVID_BIT){
		classifyField[CF_PATTERN_OUTTERTAGVID].fieldType=CLASSIFY_FIELD_TAG_VID;
		classifyField[CF_PATTERN_OUTTERTAGVID].classify_pattern.fieldData.value=(classifyFilter->outterTagVid & 0xfff);
		classifyField[CF_PATTERN_OUTTERTAGVID].classify_pattern.fieldData.mask=0xfff;
	}
	if(classifyFilter->filter_fields & EGRESS_TAGPRI_BIT){
		classifyField[CF_PATTERN_OUTTERTAGPRI].fieldType=CLASSIFY_FIELD_TAG_PRI;
		classifyField[CF_PATTERN_OUTTERTAGPRI].classify_pattern.fieldData.value=(classifyFilter->outterTagPri & 0x7);
		classifyField[CF_PATTERN_OUTTERTAGPRI].classify_pattern.fieldData.mask=0x7;
	}
	if(classifyFilter->filter_fields & EGRESS_INTERNALPRI_BIT){
		classifyField[CF_PATTERN_INTERNALPRI].fieldType=CLASSIFY_FIELD_INTER_PRI;
		classifyField[CF_PATTERN_INTERNALPRI].classify_pattern.fieldData.value=(classifyFilter->internalPri & 0x7);
		classifyField[CF_PATTERN_INTERNALPRI].classify_pattern.fieldData.mask=0x7;
	}
	if(classifyFilter->filter_fields & EGRESS_STAGIF_BIT){
		classifyField[CF_PATTERN_STAGFLAG].fieldType=CLASSIFY_FIELD_IS_STAG;
		classifyField[CF_PATTERN_STAGFLAG].classify_pattern.fieldData.value=(classifyFilter->stagIf & 0x1);
		classifyField[CF_PATTERN_STAGFLAG].classify_pattern.fieldData.mask=0x1;
	}
	if(classifyFilter->filter_fields & EGRESS_CTAGIF_BIT){
		classifyField[CF_PATTERN_CTAGFLAG].fieldType=CLASSIFY_FIELD_IS_CTAG;
		classifyField[CF_PATTERN_CTAGFLAG].classify_pattern.fieldData.value=(classifyFilter->ctagIf & 0x1);
		classifyField[CF_PATTERN_CTAGFLAG].classify_pattern.fieldData.mask=0x1;
	}
	if(classifyFilter->filter_fields & EGRESS_UNI_BIT){
		classifyField[CF_PATTERN_UNI].fieldType=CLASSIFY_FIELD_UNI;
		classifyField[CF_PATTERN_UNI].classify_pattern.fieldData.value=(classifyFilter->uni & 0x7);
		if(classifyFilter->uni_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
			classifyField[CF_PATTERN_UNI].classify_pattern.fieldData.mask=0x7;	
		}else{
			classifyField[CF_PATTERN_UNI].classify_pattern.fieldData.mask=(classifyFilter->uni_mask&0x7);	
		}
		
	}

	for(i=0;i<CF_PATTERN_END;i++){
		if(memcmp(&classifyField[i],&empty_classifyField,sizeof(rtk_classify_field_t))){
			ret = rtk_classify_field_add(&cfRule,&classifyField[i]);	
			if(ret!=RT_ERR_OK){
				DEBUG("add classify_field[%d] failed!",i);
				RETURN_ERR(ret);
			}
		}
	}
		
	//set action
	if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
		//Upstream actions
		for(i=0;i<64;i++){
			switch((classifyFilter->us_action_field&(1ULL<<i))){
				//Stag
				case CF_US_ACTION_STAG_BIT:
					//StagIf decision
					switch(classifyFilter->action_svlan.svlanTagIfDecision){
						case ACL_SVLAN_TAGIF_NOP: break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_ADD_TAG_VS_TPID; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_8100: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_ADD_TAG_8100; break;
						case ACL_SVLAN_TAGIF_UNTAG: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_DEL_STAG; break;
						case ACL_SVLAN_TAGIF_TRANSPARENT: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_TRANSPARENT; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					//Svid decision
					switch(classifyFilter->action_svlan.svlanSvidDecision){
						case ACL_SVLAN_SVID_ASSIGN: 
							cfRule.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_ASSIGN;
							cfRule.act.usAct.sTagVid = classifyFilter->action_svlan.assignedSvid;
							break;
						case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: cfRule.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_FROM_1ST_TAG; break;
						case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: cfRule.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_FROM_2ND_TAG; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					//Spri decision
					switch(classifyFilter->action_svlan.svlanSpriDecision){
						case ACL_SVLAN_SPRI_ASSIGN: 
							cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_ASSIGN;
							cfRule.act.usAct.sTagPri = classifyFilter->action_svlan.assignedSpri; 
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_1ST_TAG; break;
						case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_2ND_TAG;break;
						case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_INTERNAL; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					
					break;

				//Ctag
				case CF_US_ACTION_CTAG_BIT:
					//CtagIf decision 
					switch(classifyFilter->action_cvlan.cvlanTagIfDecision){
						case ACL_CVLAN_TAGIF_NOP: break;
						case ACL_CVLAN_TAGIF_TAGGING: cfRule.act.usAct.cAct = CLASSIFY_US_CACT_ADD_CTAG_8100; break;
						case ACL_CVLAN_TAGIF_TAGGING_WITH_C2S: cfRule.act.usAct.cAct = CLASSIFY_US_CACT_TRANSLATION_C2S; break;//upstream only 
						case ACL_CVLAN_TAGIF_UNTAG: cfRule.act.usAct.cAct = CLASSIFY_US_CACT_DEL_CTAG; break;
						case ACL_CVLAN_TAGIF_TRANSPARENT: cfRule.act.usAct.cAct = CLASSIFY_US_CACT_TRANSPARENT; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					//Cvid decision
					switch(classifyFilter->action_cvlan.cvlanCvidDecision){
						case ACL_CVLAN_CVID_ASSIGN: 
							cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_ASSIGN;
							cfRule.act.usAct.cTagVid = classifyFilter->action_cvlan.assignedCvid;
							break;
						case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_1ST_TAG; break;
						case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_2ND_TAG; break;
						case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_INTERNAL; break;//upstream only 
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					//Cpri decision
					switch(classifyFilter->action_cvlan.cvlanCpriDecision){
						case ACL_CVLAN_CPRI_ASSIGN: 
							cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_ASSIGN;
							cfRule.act.usAct.cTagPri = classifyFilter->action_cvlan.assignedCpri; 
							break;
						case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_1ST_TAG; break;
						case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_2ND_TAG; break;
						case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_INTERNAL; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					
					break;

					
				//CFPRI
				case CF_US_ACTION_CFPRI_BIT: 
					cfRule.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
					cfRule.act.usAct.cfPri = classifyFilter->action_cfpri.assignedCfPri;	
					break;
				
				//DSCP
				case CF_US_ACTION_DSCP_BIT: 
					cfRule.act.usAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
					cfRule.act.usAct.cfPri = classifyFilter->action_dscp.assignedDscp;
					//FIXME: DSCP have to use DSCP table and needs to sync with RG ACL
					RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);

					break;
				
				//SID
				case CF_US_ACTION_SID_BIT: 
					cfRule.act.usAct.sidQidAct = CLASSIFY_US_SQID_ACT_ASSIGN_SID;
					cfRule.act.usAct.sidQid = classifyFilter->action_sid_or_llid.assignedSid_or_llid;	
					break;
				
				//DROP
				case CF_US_ACTION_DROP_BIT: 
					cfRule.act.usAct.drop = CLASSIFY_DROP_ACT_ENABLE;
					break;
				
				//LOG
				case CF_US_ACTION_LOG_BIT: 
					cfRule.act.usAct.log = CLASSIFY_US_LOG_ACT_ENABLE;
					cfRule.act.usAct.logCntIdx = classifyFilter->action_log.assignedCounterIdx;
					break;

				default:
					break;
			}
		}
	}else{
		//Downstream actions
		for(i=0;i<64;i++){
			switch((classifyFilter->ds_action_field&(1ULL<<i))){		
				//Stag
				case CF_DS_ACTION_STAG_BIT:
					//StagIf decision
					switch(classifyFilter->action_svlan.svlanTagIfDecision){
						case ACL_SVLAN_TAGIF_NOP: break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_8100: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_ADD_TAG_8100; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_SP2C; break; //downstream only
						case ACL_SVLAN_TAGIF_UNTAG: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_DEL_STAG; break;
						case ACL_SVLAN_TAGIF_TRANSPARENT: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_TRANSPARENT; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}

					//Svid decision
					switch(classifyFilter->action_svlan.svlanSvidDecision){
						case ACL_SVLAN_SVID_ASSIGN: 
							cfRule.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_ASSIGN;
							cfRule.act.dsAct.sTagVid = classifyFilter->action_svlan.assignedSvid;
							break;
						case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: cfRule.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_FROM_1ST_TAG; break;
						case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: cfRule.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_FROM_2ND_TAG; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;

					}

					//Spri decision
					switch(classifyFilter->action_svlan.svlanSpriDecision){
						case ACL_SVLAN_SPRI_ASSIGN: 
							cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_ASSIGN;
							cfRule.act.dsAct.sTagPri = classifyFilter->action_svlan.assignedSpri;	
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG; break;
						case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG; break;
						case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_INTERNAL; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					break;

				//Ctag
				case CF_DS_ACTION_CTAG_BIT:
					//CtagIf decision
					switch(classifyFilter->action_cvlan.cvlanTagIfDecision){
						case ACL_CVLAN_TAGIF_NOP: break;
						case ACL_CVLAN_TAGIF_TAGGING: cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_ADD_CTAG_8100; break;
						case ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C: cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_TRANSLATION_SP2C; break; //downstream only
						case ACL_CVLAN_TAGIF_UNTAG: cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_DEL_CTAG; break;
						case ACL_CVLAN_TAGIF_TRANSPARENT: cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_TRANSPARENT; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;

					}
					//Cvid decision
					switch(classifyFilter->action_cvlan.cvlanCvidDecision){
						case ACL_CVLAN_CVID_ASSIGN: 
							cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_ASSIGN;
							cfRule.act.dsAct.cTagVid = classifyFilter->action_cvlan.assignedCvid;
							break;
						case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_1ST_TAG; break;
						case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_2ND_TAG; break;
						case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_LUT; break;//downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;

					}
					//Cpri decision
					switch(classifyFilter->action_cvlan.cvlanCpriDecision){
						case ACL_CVLAN_CPRI_ASSIGN: 
							cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_ASSIGN;
							cfRule.act.dsAct.cTagPri = classifyFilter->action_cvlan.assignedCpri;
							break;
						case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG; break;
						case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG; break;
						case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_INTERNAL; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;

					}
					break;

				//CFPRI
				case CF_DS_ACTION_CFPRI_BIT:
					cfRule.act.dsAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
					cfRule.act.dsAct.cfPri = classifyFilter->action_cfpri.assignedCfPri;	
					break;
				
				//DSCP: have side effect, can not use!
				case CF_DS_ACTION_DSCP_BIT: 
					cfRule.act.dsAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
					cfRule.act.dsAct.cfPri = classifyFilter->action_dscp.assignedDscp;
					//FIXME: DSCP have to use DSCP table and needs to sync with RG ACL
					RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
					break;
							
				//UNI
				case CF_DS_ACTION_UNI_MASK_BIT: 
					switch(classifyFilter->action_uni.uniActionDecision){
						case ACL_UNI_FWD_TO_PORTMASK_ONLY: cfRule.act.dsAct.uniAct=CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK; break;
						case ACL_UNI_FORCE_BY_MASK: cfRule.act.dsAct.uniAct=CLASSIFY_DS_UNI_ACT_FORCE_FORWARD; break;
						default: break;
					}
					cfRule.act.dsAct.uniMask.bits[0]= classifyFilter->action_uni.assignedUniPortMask;	
					break;
					
				case CF_DS_ACTION_DROP_BIT: 
					cfRule.act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_FORCE_FORWARD;
					cfRule.act.dsAct.uniMask.bits[0]= 0x0;	
					break;

				default:
					break;
			}

		}
	}

	assert_ok(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule));
	assert_ok(_rtk_rg_classifySWEntry_set(cfRule.index, *classifyFilter));

	rg_db.systemGlobal.cf_valid_mask[cfRule.index>>5]|=(1<<(cfRule.index&0x1f)); //record valid rules

	
	return (RT_ERR_RG_OK);

}

int32 _rtk_rg_apollo_classifyEntry_del(int index){
	rtk_rg_classifyEntry_t empty_classifyFilter;
	bzero(&empty_classifyFilter,sizeof(rtk_rg_classifyEntry_t));
	
	assert_ok(rtk_classify_cfgEntry_del(index));
	assert_ok(_rtk_rg_classifySWEntry_set(index, empty_classifyFilter));

	rg_db.systemGlobal.cf_valid_mask[index>>5]&=(~(1<<(index&0x1f))); //remove valid rules

	
	return (RT_ERR_RG_OK);
}

int32 _rtk_rg_apollo_classifyEntry_find(int index, rtk_rg_classifyEntry_t *classifyFilter){
	assert_ok(_rtk_rg_classifySWEntry_get(index, classifyFilter));
	return (RT_ERR_RG_OK);
}


#if 1  /*****(4)reserved ACL related APIs*****/
#endif

int _rtk_rg_aclReservedEntry_init(void){

#ifdef CONFIG_EPON_FEATURE
	if(!rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//one rule for HWNAT trap, two rule for EPON drop
		rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary = 3;
		rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary = MAX_ACL_ENTRY_SIZE -1; //MAX_INDEX is  MAX_SIZE-1
		rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary = 0;
		rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary = MAX_CF0To63_ENTRY_SIZE-1; //MAX_INDEX is  MAX_SIZE-1
	}
	else
#endif
	{
		//one rule for HWNAT trap
		rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary = 1;
		rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary = MAX_ACL_ENTRY_SIZE-1;	//MAX_INDEX is  MAX_SIZE-1
		rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary = 0;
		rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary = MAX_CF0To63_ENTRY_SIZE-1;	//MAX_INDEX is  MAX_SIZE-1
	} 

	return (RT_ERR_RG_OK);
}


#if defined(CONFIG_APOLLO)
int _rtk_rg_vlanExistInRgCheck(int vid)
{
	/*check the VLAN has been created in rg_db.
	every acl ingress cvid action should do this check for avoiding unsync between H/W & rg_db,
	because RTK ACL api will created H/W vlan automatically*/
	
	if(rg_db.vlan[vid].valid==ENABLED)
		return SUCCESS;
	else
		return FAIL;
}

int _rtk_rg_aclTemp2Field2Decision(void)
{
	/*check the template[2] field[2] should keep as original Stag, or should be redefine as TCP flags*/

	rtk_acl_template_t aclTemplate;
	
	if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP]==ENABLED ||
		rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY]==ENABLED){

		/*use FS[2] instead of Stag pattern for SYN flag*/
		bzero(&aclTemplate,sizeof(aclTemplate));
		aclTemplate.index=2; //Stag is in template[2],field[2]
		rtk_acl_template_get(&aclTemplate);
		//make sure we are sacrifying Stag pattern
	
		aclTemplate.fieldType[2]=ACL_FIELD_USER_DEFINED02;
		if(rtk_acl_template_set(&aclTemplate)){
			ACL_RSV("Set Template[2] Field[2] to user TCP_FLAG failed!!!");
			return FAIL;
		}
		ACL_RSV("Set Template[2] Field[2] to user TCP_FLAG Success!!!");	
	
	}else{
		/*use Template[2]Field[2] as original Stag*/
		bzero(&aclTemplate,sizeof(aclTemplate));
		aclTemplate.index=2; //Stag is in template[2],field[2]
		rtk_acl_template_get(&aclTemplate);
	
		aclTemplate.fieldType[2]=ACL_FIELD_STAG;
		if(rtk_acl_template_set(&aclTemplate)){
			ACL_RSV("Set Template[2] Field[2] to user STAG failed!!!");
			return FAIL;
		}
		ACL_RSV("Set Template[2] Field[2] to user STAG Success!!!");	
	}

	return SUCCESS;
}


static rtk_acl_field_t aclField_ar[8];
int _rtk_rg_aclAndCfReservedRuleHeadReflash(void)
{
	uint32 type;
	uint32 aclIdx=0;
	uint32 cfIdx=0;
	int addRuleFailedFlag=0;
	rtk_mac_t gmac,mac;
	
	rtk_ipv6_addr_t ipv6_addr;
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField,aclField2;
	rtk_acl_field_t	aclField_dmac0,aclField_dmac1,aclField_dmac2;
	uint32 i;

#ifdef CONFIG_EPON_FEATURE
	int reg_bak;

	//backup the original t-con bandwidth  reg value.
	assert_ok(ioal_mem32_read(0x23280,&reg_bak));
	
	if(rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//Enlarge PON threshold
		assert_ok(ioal_mem32_write(0x23280,0x595));
		RGAPI("reg[0x23280]=0x%x , set to val=0x595",reg_bak);	
	}
#endif

	//reflash th rules
	for(i=0;i<rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(i));
	}
	for(i=0;i<rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary;i++){
		assert_ok(rtk_classify_cfgEntry_del(i));
	}

	/*decide ACL template[2] Field[2]*/
	_rtk_rg_aclTemp2Field2Decision();


	for(type=0;type<RTK_RG_ACLANDCF_RESERVED_HEAD_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){
				case RTK_RG_ACLANDCF_RESERVED_STPBLOCKING:
					{
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac1,sizeof(aclField_dmac1));
						bzero(&aclField_dmac2,sizeof(aclField_dmac2));
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac2.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[0]
						aclField_dmac2.fieldUnion.data.value = 0x0180;
						aclField_dmac2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
						aclField_dmac1.fieldUnion.data.value = 0xc200;
						aclField_dmac1.fieldUnion.data.mask = 0xff00;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;		
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.stpBlockingPortmask.portmask;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_REDIRECT_ACT;
						aclRule.act.forwardAct.portMask.bits[0]= 0x0;	//drop
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}
					
						//point to next ruleIdx
						aclIdx++;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 3; //FS[3] in template[2]:field[3]
						aclField.fieldUnion.data.value = 0x0002; //two byte 0x0002 after ethertype for EPON
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 2;//FS[3] in template[2]:field[3]
						aclRule.activePorts.bits[0] = (1<<RTK_RG_MAC_PORT_PON); 	
						//DROP & Interrupt action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_REDIRECT_ACT;
						aclRule.act.forwardAct.portMask.bits[0]= 0x0;
						aclRule.act.enableAct[ACL_IGR_INTR_ACT] = ENABLE;
						aclRule.act.aclInterrupt = ENABLED;
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT failed!!!");
							addRuleFailedFlag=1;
							break;
						}
					
						//point to next ruleIdx
						aclIdx++;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7; //ethertype in template[0]:field[7]
						aclField.fieldUnion.data.value = 0x8808; //ethertype 0x8808 for EPON
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 0;//ethertype in template[0]:field[7]
						aclRule.activePorts.bits[0] = (1<<RTK_RG_MAC_PORT_PON);		
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT failed!!!");
							addRuleFailedFlag=1;
							break;
						}
					
						//point to next ruleIdx
						aclIdx++;
					}
					break;
					
				case RTK_RG_ACLANDCF_RESERVED_EPON_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_EPON_ASSIGN_PRIORITY @ acl[%d]",aclIdx,aclIdx);

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7; //ethertype in template[0]:field[7]
						aclField.fieldUnion.data.value = 0x8809; //ethertype 0x8808 for EPON
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_EPON_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 0;//ethertype in template[0]:field[7]
						aclRule.activePorts.bits[0] = (1<<RTK_RG_MAC_PORT_PON); 

						aclRule.act.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
						aclRule.act.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri = 7;
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_EPON_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						//point to next ruleIdx
						aclIdx++;

					}
					break;	

				case RTK_RG_ACLANDCF_RESERVED_INTF7_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF7_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[7].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[7]=aclIdx; //record former rule index
					goto dhcpTrapRule;
		
				case RTK_RG_ACLANDCF_RESERVED_INTF6_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF6_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[6].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[6]=aclIdx; //record former rule index
					goto dhcpTrapRule;
					
				case RTK_RG_ACLANDCF_RESERVED_INTF5_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF5_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[5].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[5]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF4_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF4_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[4].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[4]=aclIdx; //record former rule index
					goto dhcpTrapRule;
							
				case RTK_RG_ACLANDCF_RESERVED_INTF3_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF3_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[3].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[3]=aclIdx; //record former rule index
					goto dhcpTrapRule;
			
				case RTK_RG_ACLANDCF_RESERVED_INTF2_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF2_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[2].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[2]=aclIdx; //record former rule index
					goto dhcpTrapRule;
			
				case RTK_RG_ACLANDCF_RESERVED_INTF1_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF1_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[1].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[1]=aclIdx; //record former rule index
					goto dhcpTrapRule;
			
				case RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[0].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[0]=aclIdx; //record former rule index
					goto dhcpTrapRule;

dhcpTrapRule:
					/*start to set ACL rule*/
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField_dmac0,sizeof(aclField_dmac0));
					bzero(&aclField_dmac1,sizeof(aclField_dmac1));
					bzero(&aclField_dmac2,sizeof(aclField_dmac2));
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.templateIdx=0; //dmac
					//setup gmac
					aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
					aclField_dmac2.fieldUnion.data.value = (gmac.octet[4]<<8) | (gmac.octet[5]);
					aclField_dmac2.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}			
					aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
					aclField_dmac1.fieldUnion.data.value = (gmac.octet[2]<<8) | (gmac.octet[3]);
					aclField_dmac1.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
					aclField_dmac0.fieldUnion.data.value = (gmac.octet[0]<<8) | (gmac.octet[1]);
					aclField_dmac0.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
					//other mode trap to CPU to keep original
					aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
					
#ifdef CONFIG_DUALBAND_CONCURRENT
					aclRule.act.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
					aclRule.act.priAct.act= ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT; 	
					aclRule.act.priAct.aclPri = ((CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1)&0x7);
#endif			
					if(rtk_acl_igrRuleEntry_add(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}					
					//point to next ruleIdx
					aclIdx++;			


					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));
					bzero(&aclField2,sizeof(aclField2));
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.templateIdx=1; //l4_sport, l4_dport
					//setup l4_sport
					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 3; //sport: template[1] field[3]
					aclField.fieldUnion.data.value = 67; //DHCP l4_port
					aclField.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}	
					//setup l4_dport
					aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField2.fieldUnion.pattern.fieldIdx = 0; //sport: template[1] field[3]
					aclField2.fieldUnion.data.value = 68; //DHCP l4_port
					aclField2.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					if(rtk_acl_igrRuleEntry_add(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx++;	

					break;


				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP @ acl[%d]",aclIdx);


						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));

						//trap 224.0.0.X
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 6; //DIP[15:0]
						aclField.fieldUnion.data.value = 0x0; 
						aclField.fieldUnion.data.mask = 0xff00;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 7; //DIP[31:16]
						aclField2.fieldUnion.data.value = 0xe000; 
						aclField2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value= 1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;	
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;		
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
					{
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));

						//trap 239.255.255.250
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 6; //DIP[15:0]
						aclField.fieldUnion.data.value = 0xfffa; 
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 7; //DIP[31:16]
						aclField2.fieldUnion.data.value = 0xefff; 
						aclField2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value= 1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;	
						aclRule.activePorts.bits[0]=RTK_RG_ALL_LAN_PORTMASK;		
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP @ acl[%d]",aclIdx);

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP:
					{
						
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP @ acl[%d]",aclIdx);
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac0,sizeof(aclField_dmac0));
						bzero(&aclField_dmac1,sizeof(aclField_dmac1));
						bzero(&aclField_dmac2,sizeof(aclField_dmac2));

						//setup broadcast DMAC
						aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
						aclField_dmac2.fieldUnion.data.value = 0xffff;
						aclField_dmac2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}				
						aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
						aclField_dmac1.fieldUnion.data.value = 0xffff;
						aclField_dmac1.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
						aclField_dmac0.fieldUnion.data.value = 0xffff;
						aclField_dmac0.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;		
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
#if 0	
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=0;
#endif
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

						//keep this index in rg_db. while user ACL rearrange this index will be used.
						rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx = aclIdx;
						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_ALL_TRAP @ acl[%d]",aclIdx);						
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;		
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
#if 0	
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=0;
#endif
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ALL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;

						//keep this index in rg_db. while user ACL rearrange this index will be used.
						rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx = aclIdx;
						//point to next ruleIdx
						aclIdx++;	
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP @ acl[%d]",aclIdx);
				
						//Set ACL rule for trap all SYN packet
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.templateIdx=2; /*use :FS[2] => template[2],field[2]*/
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 2;/*template[2],field[2]*/
						aclField.fieldUnion.data.value = 0x2; //SYN flag
						aclField.fieldUnion.data.mask = 0x2;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						//force fwd to CPU port (avoid egress vlan filter useless)
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						//point to next ruleIdx
						aclIdx++;

					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY @ acl[%d]",aclIdx);
						
						//Set ACL rule for trap all SYN packet
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.templateIdx=2; /*use :FS[2] => template[2],field[2]*/
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 2;/*template[2],field[2]*/
						aclField.fieldUnion.data.value = 0x10; //ACK flag
						aclField.fieldUnion.data.mask = 0x10;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						//force fwd to CPU port (avoid egress vlan filter useless)
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_assign_priority.priority;
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						//point to next ruleIdx
						aclIdx++;						
					}
					break;
					
				case RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH:
					{
						if(_rtk_rg_vlanExistInRgCheck(rg_db.systemGlobal.aclAndCfReservedRule.wifiDadapathM2SPatchPara.tranCvid)!=SUCCESS){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH failed!!! Assigned Vid[%d] not exist",rg_db.systemGlobal.aclAndCfReservedRule.wifiDadapathM2SPatchPara.tranCvid);
							addRuleFailedFlag=1;
							break;
						}

						
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH @ acl[%d]",aclIdx);
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_CPU);//filter packets from CPU port only(case for cpu1 sent to cpu2)
						aclRule.templateIdx=2; /*use :ACL_FIELD_CTAG => template[1],field[0]*/
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 0;/*template[2],field[0]*/
						aclField.fieldUnion.data.value = (rg_db.systemGlobal.aclAndCfReservedRule.wifiDadapathM2SPatchPara.igrCpri<<13)|rg_db.systemGlobal.aclAndCfReservedRule.wifiDadapathM2SPatchPara.igrCvid;
						aclField.fieldUnion.data.mask = 0xefff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						
						//force fwd to CPU port (avoid egress vlan filter useless)
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_REDIRECT_ACT;
						aclRule.act.forwardAct.portMask.bits[0]=(1<<RTK_RG_PORT_CPU);
							
						//translate cvid
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.aclAndCfReservedRule.wifiDadapathM2SPatchPara.tranCvid;
						
						//translate internal pri (by aclPri)
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.wifiDadapathM2SPatchPara.tranCpri;
						
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

					}
					break;	

				case RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[0].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[0].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF1_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[1].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[1].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;					
				case RTK_RG_ACLANDCF_RESERVED_INTF2_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[2].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[2].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;					
				case RTK_RG_ACLANDCF_RESERVED_INTF3_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[3].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[3].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;					
				case RTK_RG_ACLANDCF_RESERVED_INTF4_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[4].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[4].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;					
				case RTK_RG_ACLANDCF_RESERVED_INTF5_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[5].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[5].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;					
				case RTK_RG_ACLANDCF_RESERVED_INTF6_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[6].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[6].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;					
				case RTK_RG_ACLANDCF_RESERVED_INTF7_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[7].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[7].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;					

dsliteTrapRule:
					/*start to set ACL rule*/
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField_ar[0],sizeof(rtk_acl_field_t));
					bzero(&aclField_ar[1],sizeof(rtk_acl_field_t));
					bzero(&aclField_ar[2],sizeof(rtk_acl_field_t));
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.templateIdx=0; //smac
					//setup smac
					aclField_ar[2].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[2].fieldUnion.pattern.fieldIdx = 4; //SA[15:0]: template[0] field[0]
					aclField_ar[2].fieldUnion.data.value = (mac.octet[4]<<8) | (mac.octet[5]);
					aclField_ar[2].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[2])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}			
					aclField_ar[1].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[1].fieldUnion.pattern.fieldIdx = 5; //SA[31:16]: template[0] field[1]
					aclField_ar[1].fieldUnion.data.value = (mac.octet[2]<<8) | (mac.octet[3]);
					aclField_ar[1].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[1])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_ar[0].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[0].fieldUnion.pattern.fieldIdx = 6; //SA[47:32]: template[0] field[2]
					aclField_ar[0].fieldUnion.data.value = (mac.octet[0]<<8) | (mac.octet[1]);
					aclField_ar[0].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[0])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
					aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
#ifdef CONFIG_DUALBAND_CONCURRENT
					aclRule.act.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
					aclRule.act.priAct.act= ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT; 	
					aclRule.act.priAct.aclPri = ((CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1)&0x7);
#endif			
					if(rtk_acl_igrRuleEntry_add(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}					
					//point to next ruleIdx
					aclIdx++;			


					bzero(&aclRule,sizeof(aclRule));
					bzero(aclField_ar,sizeof(rtk_acl_field_t)*8);
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.templateIdx=3; //ipv6_dip
					//setup DIPv6
					aclField_ar[0].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[0].fieldUnion.pattern.fieldIdx = 0;
					aclField_ar[0].fieldUnion.data.value = (ipv6_addr.ipv6_addr[10]<<8|ipv6_addr.ipv6_addr[11]); //DIP[]
					aclField_ar[0].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[0])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}	
					aclField_ar[1].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[1].fieldUnion.pattern.fieldIdx = 1;
					aclField_ar[1].fieldUnion.data.value = (ipv6_addr.ipv6_addr[8]<<8|ipv6_addr.ipv6_addr[9]); //DIP[]
					aclField_ar[1].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[1])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_ar[2].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[2].fieldUnion.pattern.fieldIdx = 2;
					aclField_ar[2].fieldUnion.data.value = (ipv6_addr.ipv6_addr[6]<<8|ipv6_addr.ipv6_addr[7]); //DIP[]
					aclField_ar[2].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[2])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_ar[3].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[3].fieldUnion.pattern.fieldIdx = 3;
					aclField_ar[3].fieldUnion.data.value = (ipv6_addr.ipv6_addr[4]<<8|ipv6_addr.ipv6_addr[5]); //DIP[]
					aclField_ar[3].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[3])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_ar[4].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[4].fieldUnion.pattern.fieldIdx = 4;
					aclField_ar[4].fieldUnion.data.value = (ipv6_addr.ipv6_addr[2]<<8|ipv6_addr.ipv6_addr[3]); //DIP[]
					aclField_ar[4].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[4])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_ar[5].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[5].fieldUnion.pattern.fieldIdx = 5;
					aclField_ar[5].fieldUnion.data.value = (ipv6_addr.ipv6_addr[0]<<8|ipv6_addr.ipv6_addr[1]); //DIP[]
					aclField_ar[5].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[5])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_ar[6].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[6].fieldUnion.pattern.fieldIdx = 6;
					aclField_ar[6].fieldUnion.data.value = (ipv6_addr.ipv6_addr[12]<<8|ipv6_addr.ipv6_addr[13]); //DIP[]
					aclField_ar[6].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[6])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_ar[7].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[7].fieldUnion.pattern.fieldIdx = 7;
					aclField_ar[7].fieldUnion.data.value = (ipv6_addr.ipv6_addr[14]<<8|ipv6_addr.ipv6_addr[15]); //DIP[]
					aclField_ar[7].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[7])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
										
					if(rtk_acl_igrRuleEntry_add(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx++;	
					break;


			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[0].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF1_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF1_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[1].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF2_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF2_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[2].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF3_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF3_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[3].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF4_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF4_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[4].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF5_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF5_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[5].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF6_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF6_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[6].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF7_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF7_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[7].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
pppoeMcRoutingTrapRule:

				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){//rule add  only when proc enabled
	
					/*start to set ACL rule*/
					bzero(&aclRule,sizeof(aclRule));
					bzero(aclField_ar,sizeof(rtk_acl_field_t)*8);
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value=1;
					aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
					aclRule.templateIdx=0; //dmac + ethertype
					//setup gmac
					aclField_ar[2].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[2].fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
					aclField_ar[2].fieldUnion.data.value = (gmac.octet[4]<<8) | (gmac.octet[5]);
					aclField_ar[2].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[2])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}			
					aclField_ar[1].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[1].fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
					aclField_ar[1].fieldUnion.data.value = (gmac.octet[2]<<8) | (gmac.octet[3]);
					aclField_ar[1].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[1])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_ar[0].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[0].fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
					aclField_ar[0].fieldUnion.data.value = (gmac.octet[0]<<8) | (gmac.octet[1]);
					aclField_ar[0].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[0])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					//setup ethertype
					aclField_ar[3].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[3].fieldUnion.pattern.fieldIdx = 7; //ethertype: template[0] field[2]
					aclField_ar[3].fieldUnion.data.value = 0x8864;
					aclField_ar[3].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[3])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
					aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
#ifdef CONFIG_DUALBAND_CONCURRENT
					aclRule.act.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
					aclRule.act.priAct.act= ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT; 	
					aclRule.act.priAct.aclPri = ((CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1)&0x7);
#endif			
					if(rtk_acl_igrRuleEntry_add(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}					
					//point to next ruleIdx
					aclIdx++;		

					
					bzero(&aclRule,sizeof(aclRule));
					bzero(aclField_ar,sizeof(rtk_acl_field_t)*8);
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value=1;
					aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
					aclRule.templateIdx=1; //dip
					//setup DIPv6
					aclField_ar[0].fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_ar[0].fieldUnion.pattern.fieldIdx = 7; //DIP[0:15]
					aclField_ar[0].fieldUnion.data.value = 0xe000; //multicast IP leading with 1110 ...
					aclField_ar[0].fieldUnion.data.mask = 0xf000;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_ar[0])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}	
					if(rtk_acl_igrRuleEntry_add(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					//point to next ruleIdx
					aclIdx++;	
				}
				break;
				
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[0].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule; 			
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF1_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF1_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[1].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule; 	
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF2_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF2_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[2].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule; 
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF3_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF3_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[3].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule; 
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF4_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF4_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[4].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule; 
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF5_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF5_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[5].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule; 
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF6_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF6_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[6].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule; 
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF7_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF7_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[7].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule; 

linkLocalTrapRule:
				bzero(&aclRule,sizeof(aclRule));
				bzero(&aclField_dmac2,sizeof(aclField_dmac2));
				bzero(&aclField_dmac1,sizeof(aclField_dmac1));
				bzero(&aclField_dmac0,sizeof(aclField_dmac0));

				/*set rule for gateway mac*/
				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
				aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED; 
				aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
				aclRule.templateIdx=0; /*IPv6 DIP[127:112]: template[3],field[5]*/
				
				//setup gmac
				aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
				aclField_dmac2.fieldUnion.data.value = (gmac.octet[4]<<8) | (gmac.octet[5]);
				aclField_dmac2.fieldUnion.data.mask = 0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				
				aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
				aclField_dmac1.fieldUnion.data.value = (gmac.octet[2]<<8) | (gmac.octet[3]);
				aclField_dmac1.fieldUnion.data.mask = 0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				
				aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
				aclField_dmac0.fieldUnion.data.value = (gmac.octet[0]<<8) | (gmac.octet[1]);
				aclField_dmac0.fieldUnion.data.mask = 0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				//trap to cpu action
				aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
				aclRule.act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;		

				//continuous hit, no need to set action
				if(rtk_acl_igrRuleEntry_add(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}	
				
				//point to next ruleIdx
				aclIdx++;

				/*set rule for link local IP*/
				bzero(&aclRule,sizeof(aclRule));
				bzero(&aclField,sizeof(aclField));
				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
				aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED; 
				aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
				aclRule.templateIdx=3; /*IPv6 DIP[127:112]: template[3],field[5]*/
				
				aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField.fieldUnion.pattern.fieldIdx = 5;//template[3],field[5] = IPv6 DIP[127:112]
				aclField.fieldUnion.data.value=0xfe80; //IPv6 DIP[127:112]
				aclField.fieldUnion.data.mask=0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				if(rtk_acl_igrRuleEntry_add(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}	
				//point to next ruleIdx
				aclIdx++;
				break;
							
				default:
					break;
		
			}
			
		}
	}
	
	rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary=aclIdx;
	rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary=cfIdx;

#ifdef CONFIG_EPON_FEATURE
	if(rg_db.systemGlobal.initParam.wanPortGponMode)	
	{
		//set PON threshold back
		assert_ok(ioal_mem32_write(0x23280,reg_bak));
		RGAPI("reg[0x23280]=0x595 , set back to val=0x%x",reg_bak);	
	}
#endif	

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	//reflash user ACL
	ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_rearrange(),RT_ERR_RG_OK);
	

	return (RT_ERR_RG_OK);
}


int _rtk_rg_aclAndCfReservedRuleTailReflash(void)
{
	uint32 type;
	uint32 aclIdx=63, cfIdx=63;
	uint32 vlan,port;
	//rule used varibles	
	int spri,intpri,wanPort;
#if 1
	//uint16 sessionId;
	rtk_classify_cfg_t cfRule;
	rtk_classify_field_t cfField;
	rtk_classify_ds_act_t dsAct;
	rtk_mac_t gmac;
	rtk_acl_field_t aclField_dmac0,aclField_dmac1,aclField_dmac2;
#endif
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField;

	int addRuleFailedFlag=0;

	uint32 i;

#ifdef CONFIG_EPON_FEATURE
	int reg_bak;

	//backup the original t-con bandwidth  reg value.
	assert_ok(ioal_mem32_read(0x23280,&reg_bak));
	
	if(rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//Enlarge PON threshold
		assert_ok(ioal_mem32_write(0x23280,0x595));
		RGAPI("reg[0x23280]=0x%x , set to val=0x595",reg_bak);	
	}
#endif	

	//reflash th rules
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary;i<64;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(i));
	}
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary;i<64;i++){
		assert_ok(rtk_classify_cfgEntry_del(i));
	}

	
	for(type=RTK_RG_ACLANDCF_RESERVED_HEAD_END;type<RTK_RG_ACLANDCF_RESERVED_TAIL_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){				
#if 0	
				case RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP @ acl[%d]",aclIdx);
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 0; //portRange in template[1]:field[0]
						aclField.fieldUnion.data.value = 80; //dport:80 in rangeTable[15]
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 1;
						aclRule.activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK & (~(1<<RTK_RG_MAC_PORT_CPU));
						//trap to cpu action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;		
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					
					}
					break;
#endif
				case RTK_RG_ACLANDCF_RESERVED_IGMP_TO_SLAVE_WIFI_BLOCK:
					{
#ifdef CONFIG_DUALBAND_CONCURRENT
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_IGMP_TO_SLAVE_WIFI_BLOCK @ acl[%d]",aclIdx);
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 4; //l4_protocal(FS[14]) in template[1]:field[4]
						aclField.fieldUnion.data.value = 0x2; //IGMP protocal value = 0x2
						aclField.fieldUnion.data.mask = 0xff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IGMP_TO_SLAVE_WIFI_BLOCK failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 1;
						aclRule.activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK;
						aclRule.act.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
						aclRule.act.priAct.act= ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;		
						aclRule.act.priAct.aclPri = ((CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1)&0x7);
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IGMP_TO_SLAVE_WIFI_BLOCK failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx--;
#endif
					}
					break;
					
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
					{

					/*new default policy, drop unknownDA UDP multicast, trap else multicast*/
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT @ acl[%d] & acl[%d]",aclIdx,aclIdx-1);

						//rule for trap else multicast packet (such as unknownDA ICMP...etc)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5; //dip[127:112] in template[3]:field[5]
						aclField.fieldUnion.data.value = 0xff00; //dip[127:112]
						aclField.fieldUnion.data.mask = 0xff00;

						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 3;
						aclRule.activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK & (~(1<<RTK_RG_MAC_PORT_CPU));
						//trap to cpu action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;		
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;	


						//rule for drop unknownDA UDP multicast (use permit action, if unknownDA will dropped by HW reason 207)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5; //dip[127:112] in template[3]:field[5]
						aclField.fieldUnion.data.value = 0xff0e; //dip[127:112]
						aclField.fieldUnion.data.mask = 0xff0f;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 3;//dip[127:112] in template[3]:field[0]
						aclRule.activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK & (~(1<<RTK_RG_MAC_PORT_CPU));
						//aclRule.careTag.tags[ACL_CARE_TAG_UDP].value=ENABLED;
						//aclRule.careTag.tags[ACL_CARE_TAG_UDP].mask=0xffff;
						//permit, used to avoid next trap action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_COPY_ACT;		
						aclRule.act.forwardAct.portMask.bits[0]=0x0;
						if(rtk_acl_igrRuleEntry_add(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;					
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT:
					{
					
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT @ acl[%d]",aclIdx);
						//translate ipv6 multicast to assigned vid 
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;//template[3],field[5] = v6DIP[127:112] use ACL_FIELD_USER_DEFINED13
						aclField.fieldUnion.data.value=0xff00; //v6DIP start with 1111 1111 ...
						aclField.fieldUnion.data.mask=0xff00;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT failed!!!");
							addRuleFailedFlag=1;
							break;
						}
							
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);//PON only
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
						aclRule.templateIdx=3; /*use DIP[127:120]: template[3],field[5]*/
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=DEFAULT_CPU_VLAN; 
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					}
					break;

#if 0					
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6:
					{
						
						if(_rtk_rg_vlanExistInRgCheck(rg_db.systemGlobal.aclAndCfReservedRule.multicastVidTranslateForIpv6.vid)!=SUCCESS){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6 failed!!! Assigned Vid[%d] not exist",rg_db.systemGlobal.aclAndCfReservedRule.multicastVidTranslateForIpv6.vid);
							addRuleFailedFlag=1;
							break;
						}
						
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6 @ acl[%d]",aclIdx);
						//translate ipv6 multicast to assigned vid 
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;//template[3],field[5] = v6DIP[127:112] use ACL_FIELD_USER_DEFINED13
						aclField.fieldUnion.data.value=0xff00; //v6DIP start with 1111 1111 ...
						aclField.fieldUnion.data.mask=0xff00;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
							
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);//PON only
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
						//must untag
						aclRule.careTag.tags[ACL_CARE_TAG_STAG].value=DISABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_STAG].mask=0xffff;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=DISABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;
						aclRule.templateIdx=3; /*use DIP[127:120]: template[3],field[5]*/
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.aclAndCfReservedRule.multicastVidTranslateForIpv6.vid; 
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6 failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV4:
					{
						
						if(_rtk_rg_vlanExistInRgCheck(rg_db.systemGlobal.aclAndCfReservedRule.multicastVidTranslateForIpv4.vid)!=SUCCESS){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV4 failed!!! Assigned Vid[%d] not exist",rg_db.systemGlobal.aclAndCfReservedRule.multicastVidTranslateForIpv4.vid);
							addRuleFailedFlag=1;
							break;
						}
						
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV4 @ acl[%d]",aclIdx);
						//translate ipv4 multicast to assigned vid
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7;//template[1],field[7] = DIP[31:16]
						aclField.fieldUnion.data.value=0xe000; //DIP start with 1110 ...
						aclField.fieldUnion.data.mask=0xf000;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV4 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
							
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);//PON only
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
						//must untag
						aclRule.careTag.tags[ACL_CARE_TAG_STAG].value=DISABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_STAG].mask=0xffff;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=DISABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;

						aclRule.templateIdx=1; /*use DIP[31:16]: template[1],field[7]*/
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.aclAndCfReservedRule.multicastVidTranslateForIpv4.vid; 
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV4 failed!!!");
							addRuleFailedFlag=1;
							break;
						}		

						//point to next ruleIdx
						aclIdx--;
					}
					break;
#endif
#if 1	
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864:
					{
						if(_rtk_rg_vlanExistInRgCheck(rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.remarkVid)!=SUCCESS){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!! Assigned Vid[%d] not exist",rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.remarkVid);
							addRuleFailedFlag=1;
							break;
						}
	
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 @ acl[%d~%d] & cf[%d]",aclIdx,(aclIdx-2),cfIdx);
						
						//for remarking ethertype 0x8864 to bridged vid.(downstream)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
							//just filter pppoe packet, do not care sessionID
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.wanPmsk;		//filter packets from WAN port only
						aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].mask=0xffff;

						//because per PPPoEWAN no need to check sessionID, so it doesnt have to concern vlan tag makes FS[15] parsing sessionID problem.
						//aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=DISABLE;
						//aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;
						aclRule.templateIdx=3; /*use fieldSelector[15]: template[3],field[7]*/
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.remarkVid;
						aclRule.act.enableAct[ACL_IGR_INTR_ACT] = ENABLE; //latch to cf
						aclRule.act.aclLatch = ENABLED; 
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}


						
						//add cf[RESERVED_CF_PPPOE_PASSTHROUGH_DOWNSTREAM_DMAC2CVID_ENTRY] for force all downstream packet DMAC=>CVID action (downstream)	
						bzero(&cfRule,sizeof(cfRule));
						bzero(&dsAct,sizeof(dsAct));
						dsAct.cAct=CLASSIFY_DS_CACT_ADD_CTAG_8100;
						dsAct.cVidAct=CLASSIFY_DS_VID_ACT_FROM_LUT;
						dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_FROM_INTERNAL;
						cfRule.index=cfIdx;
						cfRule.direction=CLASSIFY_DIRECTION_DS;
						cfRule.valid=ENABLED;
						cfRule.act.dsAct=dsAct;
						//latch by ACL[RESERVED_ACL_PPPoE_SESSIONID_REMARKING_ENTRY_BASE_FOR_ETHTER_8864]
						cfField.fieldType = CLASSIFY_FIELD_ACL_HIT;
						cfField.classify_pattern.fieldData.value = ((1<<7) | aclIdx); //(1<<7) is the valid bit
						cfField.classify_pattern.fieldData.mask=0xff;
						if(rtk_classify_field_add(&cfRule, &cfField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx--;	
						cfIdx--;
						
						
						//add acl[59] for trap all ethertype=0x8864 to fwdEngine (upstream)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
							//just filter pppoeTag packet
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.lanPmsk&(~(0x1<<RTK_RG_MAC_PORT_CPU)); 	//filter packets from LAN port only, exclude CPU port
						aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].mask=0xffff;
						aclRule.templateIdx=3; //unuse any field.
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;//ACL_IGR_FORWARD_COPY_ACT;
						//aclRule.act.forwardAct.portMask.bits[0]=wan_pmsk;
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx--;	
						
						//add acl[60] for trap all ethertype=0x8863 to fwdEngine (upstream and downstream)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
							//Ethertype 0x8863
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7;//template[0],field[7]
						aclField.fieldUnion.data.value=0x8863;
						aclField.fieldUnion.data.mask=0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK&(~(0x1<<RTK_RG_MAC_PORT_CPU));		//from CPU port should not trap again
						aclRule.templateIdx=0; /*use Ethertype: template[0],field[7]*/
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;	
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx--;
			
					}
					break;
#endif
#if 1	//since RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN is for RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 patch
//however, this one should add "PPPoE Session" in pattern check(add but not test yet)
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF7ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF7ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[7].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[7].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF6ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF6ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[6].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[6].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF5ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF5ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[5].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[5].sessionId,2); //sizeof(uint16)=2					
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF4ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF4ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[4].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[4].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF3ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF3ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[3].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[3].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF2ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF2ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[2].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[2].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF1ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF1ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[1].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[1].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[0].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[0].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
					
setPPPoEwanRule:
					/*start to set ACL rule*/
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField_dmac0,sizeof(aclField_dmac0));
					bzero(&aclField_dmac1,sizeof(aclField_dmac1));
					bzero(&aclField_dmac2,sizeof(aclField_dmac2));
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
					aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].value=ENABLED; //filter 0x8864 
					aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].mask=0xffff;
					aclRule.templateIdx=0; 
#if 0		//PPPoE sessionId check: need test

					bzero(&aclField,sizeof(aclField));
					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = TEMPLATE_FIELDSELECT15;
					aclField.fieldUnion.data.value = sessionId;
					aclField.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
							break;
					}
					
#endif
					//setup gmac
					aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
					aclField_dmac2.fieldUnion.data.value = (gmac.octet[4]<<8) | (gmac.octet[5]);
					aclField_dmac2.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
					aclField_dmac1.fieldUnion.data.value = (gmac.octet[2]<<8) | (gmac.octet[3]);
					aclField_dmac1.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
					aclField_dmac0.fieldUnion.data.value = (gmac.octet[0]<<8) | (gmac.octet[1]);
					aclField_dmac0.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					
					aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
					aclRule.act.cvlanAct.act = ACL_IGR_CVLAN_MIB_ACT;
					aclRule.act.cvlanAct.mib = 32; //assigned a unused counter
					
					if(rtk_acl_igrRuleEntry_add(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					//point to next ruleIdx
					aclIdx--;

					break;
#endif
				case RTK_RG_ACLANDCF_RESERVED_WIFI_MASTER_EXTPORT_PATCH:
					{
						int j;
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_WIFI_MASTER_EXTPORT_PATCH @ acl[%d]",aclIdx);
						
						//add acl bring extSpa message from igr_extPMask  to egr_extPmsk
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_CPU);//filter packets from CPU port only
						aclRule.templateIdx=0; /*use :ACL_FIELD_EXT_PORTMASK => template[0],field[3]*/
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 3;
						aclField.fieldUnion.data.value = 0x0;
						for(j=0;j<RTK_RG_MAX_EXT_PORT;j++){
							if(!(rg_db.systemGlobal.aclAndCfReservedRule.wifiMasterExtportPatchPara.igrPmsk&(1<<j))){
								aclField.fieldUnion.data.mask |= (1<<j); //band not allowed ext_port										
							}
						}
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI_MASTER_EXTPORT_PATCH failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						
						aclRule.act.enableAct[ACL_IGR_INTR_ACT]=ENABLED;
						aclRule.act.aclInterrupt=ENABLED;
						aclRule.act.extendAct.act=ACL_IGR_EXTEND_EXT_ACT;
						aclRule.act.extendAct.portMask.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.wifiMasterExtportPatchPara.egrPmsk;
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI_MASTER_EXTPORT_PATCH failed!!!");
							addRuleFailedFlag=1;
							break;
						}
											
						//point to next ruleIdx
						aclIdx--;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_WIFI_SLAVE_EXTPORT_PATCH:
					{
						int j;
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_WIFI_SLAVE_EXTPORT_PATCH @ acl[%d]",aclIdx);
				
						//add acl bring extSpa message from igr_extPMask  to egr_extPmsk
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_CPU);//filter packets from CPU port only
						aclRule.templateIdx=0; /*use :ACL_FIELD_EXT_PORTMASK => template[0],field[3]*/
						
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 3;
						aclField.fieldUnion.data.value = 0x0;
						for(j=0;j<RTK_RG_MAX_EXT_PORT;j++){
							if(!(rg_db.systemGlobal.aclAndCfReservedRule.wifiSlaveExtportPatchPara.igrPmsk&(1<<j))){
								aclField.fieldUnion.data.mask |= (1<<j); //band not allowed ext_port										
							}
						}
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI_SLAVE_EXTPORT_PATCH failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						
						aclRule.act.enableAct[ACL_IGR_INTR_ACT]=ENABLED;
						aclRule.act.aclInterrupt=ENABLED;
						aclRule.act.extendAct.act=ACL_IGR_EXTEND_EXT_ACT;
						aclRule.act.extendAct.portMask.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.wifiSlaveExtportPatchPara.egrPmsk;
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI_SLAVE_EXTPORT_PATCH failed!!!");
							addRuleFailedFlag=1;
							break;
						}
											
						//point to next ruleIdx
						aclIdx--;
					}
					break;

//#ifdef CONFIG_RG_PPPOE_AND_VALN_ISSUE_PATCH
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH:
					{	
						if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
						{
							ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH @ acl[%d] & acl[%d]",aclIdx,aclIdx-1);

							//first rule for translate Stag to Ctag, and translate SVID to 1
							bzero(&aclRule,sizeof(aclRule));
		
							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=(1<<rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSvid2CvidPatchPara.wanPort);
							//aclRule.templateIdx=2; /*STag: template[2],field[2] => no need to compare Stag now.*/
							//must have stag
							aclRule.careTag.tags[ACL_CARE_TAG_STAG].value=ENABLED; //check have 0x8100 or 0x8864 tag. 
							aclRule.careTag.tags[ACL_CARE_TAG_STAG].mask=0xffff;
							//translate svid to ingress cvid
							aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
							aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_DS_SVID_ACT;
							//translate svid to 1
							aclRule.act.enableAct[ACL_IGR_SVLAN_ACT]=ENABLED;
							aclRule.act.svlanAct.act=ACL_IGR_SVLAN_IGR_SVLAN_ACT;
							aclRule.act.svlanAct.svid=1; //SVID[1] include all memberport
							if(rtk_acl_igrRuleEntry_add(&aclRule)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//point to next ruleIdx
							aclIdx--;

#if 0
							//Second rule for Trap broadcast pkt from WAN. 
							//Because broadcast dataPath should go by fwdEngine, and should keep original SVID (Avoiding H/W bug: can not keep original Stag information when trap)
							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField_dmac0,sizeof(aclField_dmac0));
							bzero(&aclField_dmac1,sizeof(aclField_dmac1));
							bzero(&aclField_dmac2,sizeof(aclField_dmac2));
							
							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=(1<<rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSvid2CvidPatchPara.wanPort);
							aclRule.templateIdx=0;
							//setup broadcast DMAC
							aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
							aclField_dmac2.fieldUnion.data.value = 0xffff;
							aclField_dmac2.fieldUnion.data.mask = 0xffff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							
							aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
							aclField_dmac1.fieldUnion.data.value = 0xffff;
							aclField_dmac1.fieldUnion.data.mask = 0xffff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							
							aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
							aclField_dmac0.fieldUnion.data.value = 0xffff;
							aclField_dmac0.fieldUnion.data.mask = 0xffff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							//action trap & remarking aclPri to zero
							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
							aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
							aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
							aclRule.act.priAct.aclPri=0;
							if(rtk_acl_igrRuleEntry_add(&aclRule)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//point to next ruleIdx
							aclIdx--;
#endif						
						}
					}
					break;
//#endif

//#ifdef CONFIG_RG_PPPOE_AND_VALN_ISSUE_PATCH
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI7REMAPTOINTERNALPRI_PATCH:
					if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI7REMAPTOINTERNALPRI_PATCH @ acl[%d]",aclIdx);
						spri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[7].spri;
						intpri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[7].intpri;
						wanPort=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[7].wanPort;
						goto setPPPoECvidIssueRule;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI6REMAPTOINTERNALPRI_PATCH:	
					if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI6REMAPTOINTERNALPRI_PATCH @ acl[%d]",aclIdx);
						spri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[6].spri;
						intpri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[6].intpri;
						wanPort=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[6].wanPort;
						goto setPPPoECvidIssueRule;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI5REMAPTOINTERNALPRI_PATCH:
					if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI5REMAPTOINTERNALPRI_PATCH @ acl[%d]",aclIdx);
						spri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[5].spri;
						intpri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[5].intpri;
						wanPort=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[5].wanPort;
						goto setPPPoECvidIssueRule;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI4REMAPTOINTERNALPRI_PATCH:
					if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI4REMAPTOINTERNALPRI_PATCH @ acl[%d]",aclIdx);
						spri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[4].spri;
						intpri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[4].intpri;
						wanPort=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[4].wanPort;
						goto setPPPoECvidIssueRule;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI3REMAPTOINTERNALPRI_PATCH:
					if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI3REMAPTOINTERNALPRI_PATCH @ acl[%d]",aclIdx);
						spri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[3].spri;
						intpri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[3].intpri;
						wanPort=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[3].wanPort;
						goto setPPPoECvidIssueRule;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI2REMAPTOINTERNALPRI_PATCH:
					if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI2REMAPTOINTERNALPRI_PATCH @ acl[%d]",aclIdx);
						spri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[2].spri;
						intpri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[2].intpri;
						wanPort=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[2].wanPort;
						goto setPPPoECvidIssueRule;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI1REMAPTOINTERNALPRI_PATCH:		
					if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI1REMAPTOINTERNALPRI_PATCH @ acl[%d]",aclIdx);
						spri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[1].spri;
						intpri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[1].intpri;
						wanPort=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[1].wanPort;
						goto setPPPoECvidIssueRule;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI0REMAPTOINTERNALPRI_PATCH:
					if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI0REMAPTOINTERNALPRI_PATCH @ acl[%d]",aclIdx);
						spri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[0].spri;
						intpri=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[0].intpri;
						wanPort=rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[0].wanPort;					
setPPPoECvidIssueRule:					
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=(1<<wanPort);
						aclRule.templateIdx=2; /*STag: template[2],field[2]*/

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 2;//template[2],field[2] = Stag
						aclField.fieldUnion.data.value=(spri<<13); //Spri
						aclField.fieldUnion.data.mask=0xe000;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRIxREMAPTOINTERNALPRI_PATCH failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						
						//must have stag
						aclRule.careTag.tags[ACL_CARE_TAG_STAG].value=ENABLED; //check have 0x8100 or 0x8864 tag. 
						aclRule.careTag.tags[ACL_CARE_TAG_STAG].mask=0xffff;

						//translate svid to 1
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=intpri; //assigned internal priority
						if(rtk_acl_igrRuleEntry_add(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRIxREMAPTOINTERNALPRI_PATCH failed!!!");
							addRuleFailedFlag=1;
							break;
						}	

						//point to next ruleIdx
						aclIdx--;
					}
					break;
//#endif
#if 0
				case RTK_RG_ACLANDCF_RESERVED_PON_INTFDEFAULTSSIDREMAP_PATCH:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTFDEFAULTSSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[8].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[8].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[8].vid;
					goto ponSsidRemarkRule; 			
				case RTK_RG_ACLANDCF_RESERVED_PON_INTF7SSIDREMAP_PATCH: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTF7SSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[7].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[7].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[7].vid;
					goto ponSsidRemarkRule; 				
				case RTK_RG_ACLANDCF_RESERVED_PON_INTF6SSIDREMAP_PATCH: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTF6SSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[6].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[6].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[6].vid;
					goto ponSsidRemarkRule;
				case RTK_RG_ACLANDCF_RESERVED_PON_INTF5SSIDREMAP_PATCH: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTF5SSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[5].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[5].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[5].vid;
					goto ponSsidRemarkRule; 			
				case RTK_RG_ACLANDCF_RESERVED_PON_INTF4SSIDREMAP_PATCH: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTF4SSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[4].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[4].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[4].vid;
					goto ponSsidRemarkRule; 		
				case RTK_RG_ACLANDCF_RESERVED_PON_INTF3SSIDREMAP_PATCH: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTF3SSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[3].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[3].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[3].vid;
					goto ponSsidRemarkRule; 
				case RTK_RG_ACLANDCF_RESERVED_PON_INTF2SSIDREMAP_PATCH: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTF2SSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[2].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[2].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[2].vid;
					goto ponSsidRemarkRule; 			
				case RTK_RG_ACLANDCF_RESERVED_PON_INTF1SSIDREMAP_PATCH: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTF1SSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[1].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[1].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[1].vid;
					goto ponSsidRemarkRule; 					
				case RTK_RG_ACLANDCF_RESERVED_PON_INTF0SSIDREMAP_PATCH: 
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PON_INTF0SSIDREMAP_PATCH @ cf[%d]",cfIdx);
					intfIdx=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[0].intfIdx;
					ssid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[0].ssid;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[0].vid;
					goto ponSsidRemarkRule;

ponSsidRemarkRule:
					bzero(&cfRule,sizeof(cfRule));
					bzero(&cfField,sizeof(cfField));
					bzero(&usAct,sizeof(usAct));
			
					cfRule.valid=ENABLED;
					cfRule.index=cfIdx;
					cfRule.direction=CLASSIFY_DIRECTION_US;
					cfField.fieldType = CLASSIFY_FIELD_WAN_IF;
					cfField.classify_pattern.fieldData.value = intfIdx;
					cfField.classify_pattern.fieldData.mask=0x7; //3bits only
					if(rtk_classify_field_add(&cfRule, &cfField))
					{
						RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
					}

					
					if(vid!=-1)
					{//force tagged 
						usAct.cAct=CLASSIFY_US_CACT_ADD_CTAG_8100;
						usAct.cVidAct=CLASSIFY_US_VID_ACT_ASSIGN;
						usAct.cTagVid=vid;
						//FIXME: dot1p forcing using internal will cause some problem while QoS Remarking is Enabled!!! 
						usAct.cPriAct=CLASSIFY_US_PRI_ACT_FROM_INTERNAL;
					}else{//vid==-1 means untagged
						//force remove tag
						usAct.cAct=CLASSIFY_US_CACT_DEL_CTAG;
					}
					
					usAct.sidQidAct=CLASSIFY_US_SQID_ACT_ASSIGN_SID;
					usAct.sidQid=ssid;
					cfRule.act.usAct=usAct;
					if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PON_INTFxSSIDREMAP_PATCH failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					
					//point to next ruleIdx
					cfIdx--;

					break;
					

#endif
					
				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT0].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT0].assigned_vid;
					goto portBasedAclRule;
				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT1_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT1_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT1].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT1].assigned_vid;
					goto portBasedAclRule;

#if defined(CONFIG_RTL9600_SERIES)

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT2_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT2_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT2].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT2].assigned_vid;
					goto portBasedAclRule;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT3_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT3_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT3].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT3].assigned_vid;
					goto portBasedAclRule;
#endif

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT4_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT4_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_PON].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_PON].assigned_vid;
					goto portBasedAclRule;

#if defined(CONFIG_RTL9600_SERIES)

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT5_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT5_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_RGMII].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_RGMII].assigned_vid;
					goto portBasedAclRule;
#endif

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT6_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT6_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_CPU].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_CPU].assigned_vid;
					goto portBasedAclRule;

portBasedAclRule:				
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));
					
					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=(1<<port);
					aclRule.templateIdx=0; 
					aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=DISABLED;
					aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;
					
					//translate CVID
					aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
					aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
					aclRule.act.cvlanAct.cvid=vlan; 
					if(rtk_acl_igrRuleEntry_add(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORTn_WITHOUT_FILTER failed!!!");
						addRuleFailedFlag=1;
						break;
					}	

					//point to next ruleIdx
					aclIdx--;

					break;
				case RTK_RG_ACLANDCF_RESERVED_TAIL_END:	
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_TAIL_END @ acl[%d]",aclIdx);
					break;


					
				default:
					break;
			}
		}
	}

	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary=aclIdx;
	rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary=cfIdx;

#ifdef CONFIG_EPON_FEATURE
	if(rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		//set PON threshold back
		assert_ok(ioal_mem32_write(0x23280,reg_bak));
		RGAPI("reg[0x23280]=0x595 , set back to val=0x%x",reg_bak); 
	}
#endif


	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	return (RT_ERR_RG_OK);

}

int _rtk_rg_aclAndCfReservedRuleAddCheck(uint32 aclRsvSize, uint32 cfRsvSize)
{
	uint32 i;
	rtk_acl_ingress_entry_t aclRule;
	rtk_classify_cfg_t cfRule;;

	//check rest empty acl rules in enough
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary;i>(rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary-aclRsvSize);i--){
		aclRule.index=i;
		assert_ok(rtk_acl_igrRuleEntry_get(&aclRule));
		if(aclRule.valid!=DISABLED){
			WARNING("ACL rest rules for reserved is not enough! i=%d aclUpperBoundary=%d aclRsvSize=%d",i,rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary,aclRsvSize);
			return (RT_ERR_RG_FAILED);
		}
	}

	//check rest empty cf rules in enough
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary;i>(rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary-cfRsvSize);i--){
		cfRule.index=i;
		assert_ok(rtk_classify_cfgEntry_get(&cfRule));
		if(cfRule.valid!=DISABLED){
			WARNING("CF rest rules for reserved is not enough!");
			return (RT_ERR_RG_FAILED);
		}
	}

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
		case RTK_RG_ACLANDCF_RESERVED_STPBLOCKING:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_STPBLOCKING]=ENABLED;
			break;
		case RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_EPON_DROP_AND_INTERRUPT]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_EPON_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_EPON_ASSIGN_PRIORITY]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0)); //reserved one more for MC temporary permit while ACL rearrange
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP: 
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));//reserved one more for MC temporary permit while ACL rearrange
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ALL_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP]=ENABLED; 
			break;	

		case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_assign_priority,(rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY]=ENABLED; 
			break;			

		case RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[0],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF1_DSLITE_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[1],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF1_DSLITE_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF2_DSLITE_TRAP:			
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[2],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF2_DSLITE_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF3_DSLITE_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[3],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF3_DSLITE_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF4_DSLITE_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[4],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF4_DSLITE_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF5_DSLITE_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[5],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF5_DSLITE_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF6_DSLITE_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[6],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF6_DSLITE_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF7_DSLITE_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[7],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF7_DSLITE_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[0],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF1_MC_ROUTING_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[1],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF1_MC_ROUTING_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF2_MC_ROUTING_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[2],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF2_MC_ROUTING_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF3_MC_ROUTING_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[3],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF3_MC_ROUTING_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF4_MC_ROUTING_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[4],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF4_MC_ROUTING_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF5_MC_ROUTING_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[5],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF5_MC_ROUTING_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF6_MC_ROUTING_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[6],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF6_MC_ROUTING_TRAP]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF7_MC_ROUTING_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[7],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF7_MC_ROUTING_TRAP]=ENABLED; 
			break;		

		case RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.wifiDadapathM2SPatchPara,(rtk_rg_aclAndCf_reserved_WifiDatapathMaster2SlavePatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_WifiDatapathMaster2SlavePatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_INTF7_DHCP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[7],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF7_DHCP_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF6_DHCP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[6],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF6_DHCP_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF5_DHCP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[5],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF5_DHCP_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF4_DHCP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[4],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF4_DHCP_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF3_DHCP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[3],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF3_DHCP_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF2_DHCP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[2],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF2_DHCP_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF1_DHCP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[1],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF1_DHCP_TRAP]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[0],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP]=ENABLED; 
			break;
#if 0
		case RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP: 
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP]=ENABLED; 
			break;
#endif		
		case RTK_RG_ACLANDCF_RESERVED_IGMP_TO_SLAVE_WIFI_BLOCK: 
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IGMP_TO_SLAVE_WIFI_BLOCK]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT]=ENABLED; 
			break;
#if 0		
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6: 
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.multicastVidTranslateForIpv6,(rtk_rg_aclAndCf_reserved_multicastVidTranslateForIpv6_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_multicastVidTranslateForIpv6_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV4: 
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.multicastVidTranslateForIpv4,(rtk_rg_aclAndCf_reserved_multicastVidTranslateForIpv4_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_multicastVidTranslateForIpv4_t));	
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV4]=ENABLED;
			break;
#endif
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864: 
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(3,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara,(rtk_rg_aclAndCf_reserved_pppoepassthroughtDefaulTrapRuletPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtDefaulTrapRuletPatch_t));			
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF7ISPPPOEWAN:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[7],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF7ISPPPOEWAN]=ENABLED;	
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF6ISPPPOEWAN:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[6],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF6ISPPPOEWAN]=ENABLED;	
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF5ISPPPOEWAN:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[5],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF5ISPPPOEWAN]=ENABLED; 	
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF4ISPPPOEWAN:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[4],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF4ISPPPOEWAN]=ENABLED;	
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF3ISPPPOEWAN:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[3],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF3ISPPPOEWAN]=ENABLED;	
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF2ISPPPOEWAN:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[2],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF2ISPPPOEWAN]=ENABLED; 	
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF1ISPPPOEWAN:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[1],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF1ISPPPOEWAN]=ENABLED;	
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[0],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN]=ENABLED;	
			break;

		case RTK_RG_ACLANDCF_RESERVED_WIFI_MASTER_EXTPORT_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.wifiMasterExtportPatchPara,(rtk_rg_aclAndCf_reserved_WifiMasterExtportPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_WifiMasterExtportPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_WIFI_MASTER_EXTPORT_PATCH]=ENABLED;	
			break;
		case RTK_RG_ACLANDCF_RESERVED_WIFI_SLAVE_EXTPORT_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.wifiSlaveExtportPatchPara,(rtk_rg_aclAndCf_reserved_WifiSlaveExtportPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_WifiSlaveExtportPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_WIFI_SLAVE_EXTPORT_PATCH]=ENABLED;
			break;

//#ifdef CONFIG_RG_PPPOE_AND_VALN_ISSUE_PATCH
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
#if 0		//remove broadcast trap, only need 1 ACL
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
#else
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
#endif
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSvid2CvidPatchPara,(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSvidCopy2CvidPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSvidCopy2CvidPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH]=ENABLED; 
			}
			break;
//#endif

//#ifdef CONFIG_RG_PPPOE_AND_VALN_ISSUE_PATCH
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI7REMAPTOINTERNALPRI_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[7],(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI7REMAPTOINTERNALPRI_PATCH]=ENABLED; 
			}
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI6REMAPTOINTERNALPRI_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[6],(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI6REMAPTOINTERNALPRI_PATCH]=ENABLED; 
			}
			break;		
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI5REMAPTOINTERNALPRI_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[5],(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI5REMAPTOINTERNALPRI_PATCH]=ENABLED; 
			}
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI4REMAPTOINTERNALPRI_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[4],(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI4REMAPTOINTERNALPRI_PATCH]=ENABLED;
			}
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI3REMAPTOINTERNALPRI_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[3],(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI3REMAPTOINTERNALPRI_PATCH]=ENABLED;
			}
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI2REMAPTOINTERNALPRI_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[2],(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI2REMAPTOINTERNALPRI_PATCH]=ENABLED;
			}
			break;
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI1REMAPTOINTERNALPRI_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[1],(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI1REMAPTOINTERNALPRI_PATCH]=ENABLED; 
			}
			break;		
		case RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI0REMAPTOINTERNALPRI_PATCH:
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
				memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoeCvidIssueSpriRemap2IntpriPara[0],(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t));				
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI0REMAPTOINTERNALPRI_PATCH]=ENABLED; 
			}
			break;
//#endif			
#if 0
		case RTK_RG_ACLANDCF_RESERVED_PON_INTFDEFAULTSSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[8],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTFDEFAULTSSIDREMAP_PATCH]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_PON_INTF7SSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[7],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTF7SSIDREMAP_PATCH]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_PON_INTF6SSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[6],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTF6SSIDREMAP_PATCH]=ENABLED;
			break;
		case RTK_RG_ACLANDCF_RESERVED_PON_INTF5SSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[5],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTF5SSIDREMAP_PATCH]=ENABLED;
			break;
		case RTK_RG_ACLANDCF_RESERVED_PON_INTF4SSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[4],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTF4SSIDREMAP_PATCH]=ENABLED;
			break;
		case RTK_RG_ACLANDCF_RESERVED_PON_INTF3SSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[3],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTF3SSIDREMAP_PATCH]=ENABLED; 
			break; 
		case RTK_RG_ACLANDCF_RESERVED_PON_INTF2SSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[2],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTF2SSIDREMAP_PATCH]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_PON_INTF1SSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[1],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTF1SSIDREMAP_PATCH]=ENABLED;
			break;
		case RTK_RG_ACLANDCF_RESERVED_PON_INTF0SSIDREMAP_PATCH:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(0,1));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ponIntfSsidRemapPatchPara[0],(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ponIntfSsidRemapPatch_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_PON_INTF0SSIDREMAP_PATCH]=ENABLED; 
			break;
#endif

		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP]=ENABLED; 
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[0],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));				
			break;
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF1_LINK_LOCAL_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IPV6_INTF1_LINK_LOCAL_TRAP]=ENABLED; 
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[1],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));				
			break;
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF2_LINK_LOCAL_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IPV6_INTF2_LINK_LOCAL_TRAP]=ENABLED; 
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[2],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));				
			break;
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF3_LINK_LOCAL_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IPV6_INTF3_LINK_LOCAL_TRAP]=ENABLED; 
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[3],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));				
			break;
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF4_LINK_LOCAL_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IPV6_INTF4_LINK_LOCAL_TRAP]=ENABLED; 
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[4],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));				
			break;
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF5_LINK_LOCAL_TRAP:			
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IPV6_INTF5_LINK_LOCAL_TRAP]=ENABLED; 
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[5],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));				
			break;
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF6_LINK_LOCAL_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IPV6_INTF6_LINK_LOCAL_TRAP]=ENABLED; 
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[6],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));				
			break;
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF7_LINK_LOCAL_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IPV6_INTF7_LINK_LOCAL_TRAP]=ENABLED; 
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[7],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));				
			break;
			
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[0],(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT1_WITHOUT_FILTER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[1],(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT1_WITHOUT_FILTER]=ENABLED; 
			break;
		
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT2_WITHOUT_FILTER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[2],(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT2_WITHOUT_FILTER]=ENABLED; 
			break;
			
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT3_WITHOUT_FILTER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[3],(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT3_WITHOUT_FILTER]=ENABLED; 
			break;
			
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT4_WITHOUT_FILTER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[4],(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT4_WITHOUT_FILTER]=ENABLED; 
			break;
			
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT5_WITHOUT_FILTER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[5],(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT5_WITHOUT_FILTER]=ENABLED; 
			break;
			
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT6_WITHOUT_FILTER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[6],(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));				
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT6_WITHOUT_FILTER]=ENABLED; 
			break;

		case RTK_RG_ACLANDCF_RESERVED_TAIL_END:
			break;
			
		default:
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


#endif //end of  #if defined(CONFIG_APOLLO)


#if 1  /*****(5)fwdEngine datapath used APIs*****/
#endif

char *name_of_sact_Decision[]={
	"ACL_SVLAN_TAGIF_NOP",
	"ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID",
	"ACL_SVLAN_TAGIF_TAGGING_WITH_8100",
	"ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C",
	"ACL_SVLAN_TAGIF_UNTAG",
	"ACL_SVLAN_TAGIF_TRANSPARENT",
	"ACL_SVLAN_TAGIF_END"
};

char *name_of_svid_Decision[]={
	"ACL_SVLAN_SVID_ASSIGN",
	"ACL_SVLAN_SVID_COPY_FROM_1ST_TAG",
	"ACL_SVLAN_SVID_COPY_FROM_2ND_TAG",
	"ACL_SVLAN_SVID_END"
};
char *name_of_spri_Decision[]={
	"ACL_SVLAN_SPRI_ASSIGN",
	"ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG",
	"ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG",
	"ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI",
	"ACL_SVLAN_SPRI_END"

};
char *name_of_cact_Decision[]={
	"ACL_SVLAN_TAGIF_NOP",
	"ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID",
	"ACL_SVLAN_TAGIF_TAGGING_WITH_8100",
	"ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C", //downstream only
	"ACL_SVLAN_TAGIF_UNTAG",
	"ACL_SVLAN_TAGIF_TRANSPARENT",
	"ACL_SVLAN_TAGIF_END"

};
char *name_of_cvid_Decision[]={
	"ACL_CVLAN_CVID_ASSIGN",
	"ACL_CVLAN_CVID_COPY_FROM_1ST_TAG",
	"ACL_CVLAN_CVID_COPY_FROM_2ND_TAG",
	"ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID",
	"ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID",
	"ACL_CVLAN_CVID_END"
};
char *name_of_cpri_Decision[]={
	"ACL_CVLAN_CPRI_ASSIGN",
	"ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG",
	"ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG",
	"ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI",
	"ACL_CVLAN_CPRI_END"
};

char *name_of_uni_Decision[]={
	"ACL_UNI_FWD_TO_PORTMASK_ONLY",
	"ACL_UNI_FORCE_BY_MASK"
};

__SRAM_FWDENG_SLOWPATH
int _rtk_rg_aclDecisionClear(rtk_rg_pktHdr_t *pPktHdr)
{
#ifdef CONFIG_GPON_FEATURE
	pPktHdr->streamID=0; //SYNC to HW if no CF SID action hit.
#endif
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

//ingressCvidRuleIdxArray is NULL: all SW_acl rules ingress pattern check
//ingressCvidRuleIdxArray have value: check the ingress action rules only and replace pktHdr->internalVlanID if hit.
__SRAM_FWDENG_SLOWPATH
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

		ACL("Check ingress pattern of ACLRule[%d]:        handleValidRuleCounter=%d",i,handleValidRuleCounter);


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
		else //upstream
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
		}

		
		/*ingress rule check*/
		port_check = FAIL;
		if(pChkRule->acl_filter.filter_fields&INGRESS_PORT_BIT){
			for(port=0;port<RTK_RG_PORT_MAX;port++){//including extport
				if(pChkRule->acl_filter.ingress_port_mask.portmask & (1<<port)){
					if(port==pPktHdr->ingressPort){
						port_check = SUCCESS;						
					}
				}
			}
			if(port_check!=SUCCESS){
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT rule_pmask=0x%x spa=%d ",i,pChkRule->acl_filter.ingress_port_mask.portmask,pPktHdr->ingressPort);
				continue;
			}
		}


		
		if(pChkRule->acl_filter.filter_fields&INGRESS_INTF_BIT){
			//FIXME:judge rule is same as HWNAT, however may be not enought for ever case.
			//check this intf exist
			if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].valid==DISABLED){
				ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",i);
				continue;
			}


			if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.is_wan==1){//wan intf
				if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE){//bridge wan
					//check vlan
					if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on==1){//vlan should tagged
						if(pPktHdr->ctagVid!=rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id){
							ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",i);
							continue;
						}
					}else{//vlan should untagged
						if(pPktHdr->tagif & CVLAN_TAGIF){
							ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",i);
							continue;
						}		
					}
				}else{//L34 Wan
					if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on==1){//vlan should tagged
						//vid==gw_vid && da==gw_mac
						if(pPktHdr->ctagVid!=rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id){
							ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",i);
							continue;
						}
						if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[0]!=pPktHdr->pDmac[0] ||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[1]!=pPktHdr->pDmac[1]||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[2]!=pPktHdr->pDmac[2]||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[3]!=pPktHdr->pDmac[3]||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[4]!=pPktHdr->pDmac[4]||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[5]!=pPktHdr->pDmac[5]
							){
								ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",i);
								continue;
						}
					}else{//vlan should untagged & da==gw_mac
						if(pPktHdr->tagif & CVLAN_TAGIF){
							ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",i);
							continue;
						}	
						if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[0]!=pPktHdr->pDmac[0] ||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[1]!=pPktHdr->pDmac[1]||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[2]!=pPktHdr->pDmac[2]||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[3]!=pPktHdr->pDmac[3]||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[4]!=pPktHdr->pDmac[4]||
							rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[5]!=pPktHdr->pDmac[5]
							){
								ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",i);
								continue;
						}
					}
					
				}

			}else{//lan intf
				//check spa is in lan_port_mask
				port_check = FAIL;
				for(port=0;port<RTK_RG_MAX_MAC_PORT;port++){
					if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.lan_intf.port_mask.portmask & (1<<port)){
						if(port==pPktHdr->pRxDesc->opts3.bit.src_port_num){
							port_check = SUCCESS;
						}
					}
				}
				if(port_check!=SUCCESS){
					ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",i);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STREAM_ID_BIT){
			if((pChkRule->acl_filter.ingress_stream_id&pChkRule->acl_filter.ingress_stream_id_mask)!=(pPktHdr->pRxDesc->opts2.bit.pon_stream_id&pChkRule->acl_filter.ingress_stream_id_mask)){
				ACL("RG_ACL[%d] INGRESS_STREAM_ID_BIT UNHIT ",i);
				continue;
			}
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
			if(	!(pPktHdr->tagif&TCP_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_TCP UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_UDP_BIT){
			if(	!(pPktHdr->tagif&UDP_TAGIF)){
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


__SRAM_FWDENG_SLOWPATH
rtk_rg_fwdEngineReturn_t _rtk_rg_ingressACLAction(rtk_rg_pktHdr_t *pPktHdr)
{
	/*
	*   This API is using for do pure ingress ACL actions such as: ingress_cvid,  ingress_svid, acl_priority, drop, trap_to_PS
	*   remarking actions will be done after _rtk_rg_egressACLAction() & _rtk_rg_modifyPacketByACLAction()
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
			else //no acl priority action rules in  fwding_type_and_direction=ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_XXXXX, speed up.
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

__SRAM_FWDENG_SLOWPATH
int _rtk_rg_egressACLPatternCheck(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify, rtk_rg_port_idx_t egressPort)
{
	//egressPort used for confirm packet egress to CF port, egressPort==-1 represent BC to lan
	uint32 i,index;
	rtk_rg_aclFilterEntry_t* pChkRule;
	ipaddr_t sipModify,dipModify;
	uint16 sportModify,dportModify;
	int dipL3Idx,netifIdx=-1,nexthopIdx;
	uint32 flowDecision; 
	uint32 is_ip4mc_check=0,is_ip6mc_check=0;
	
	
	rtk_rg_classifyEntry_t* pL2CFChkRule;

	//reset the egressHitMask & final actions, because in broacast dataPath each Interfcaes will call this API individually 

	for(i=0;i<((MAX_ACL_SW_ENTRY_SIZE/32)+1);i++){
		pPktHdr->aclDecision.aclEgrHitMask[i]=0;
	}

	for(i=0;i<((TOTAL_CF_ENTRY_SIZE/32)+1);i++){
		pPktHdr->aclDecision.aclEgrL2HitMask[i]=0;
	}

	_rtk_rg_aclDecisionClear(pPktHdr);



	if((egressPort == RTK_RG_PORT_PON)&&(rg_db.systemGlobal.ponPortUnmatchCfDrop))
		pPktHdr->unmatched_cf_act = IDX_UNHIT_DROP;	//to PON port, only upstream; not need to check if any downstream pkt
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


	//flow decision
	if(pPktHdr->ingressPort==RTK_RG_PORT_PON) //downstream
	{			
		flowDecision = CF_DOWNSTREAM; 
		ACL("flowDecision: CF_DOWNSTREAM");
	}
	else //maybe upstream, 
	{
		if(egressPort==RTK_RG_PORT_PON)//upstream
		{
			flowDecision = CF_UPSTREAM; 
			ACL("flowDecision: CF_UPSTREAM");
		}
		else //lan to lan
		{
			flowDecision= CF_LAN_TO_LAN;
			ACL("flowDecision: CF_LAN_TO_LAN");
		}
	}
	

	//check L34 pattern: CF 0-63	
	//for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
	for(index=0;index<MAX_ACL_SW_ENTRY_SIZE;index++){
		i = pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[index];
		if(i==-1)//no rest rule need to check
			break;
	

		//DEBUG("CHECK EGRESS ACL[%d]:",i);
		pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[i]);

		//direction check
		if(flowDecision == CF_DOWNSTREAM) //downstream
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
		else if(flowDecision == CF_UPSTREAM)//maybe upstream, 
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

		}
		else //lan-to-lan
		{
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
				ACL("RG_ACL[%d] fwdtype = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, speedup by skip egress pattern check ",i);
				goto skipEgressPatternCheck;
			}
			else //fwdtype = 1~4
			{
				//consider drop
				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP){
					//let it continue check. The Drop action will alwaye be Execute(if hit) even packet is not related to CF port.		
				}else{
					ACL("RG_ACL[%d] Lan-to-Lan, none-related to CF port",i);
					continue;
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
							if(priority!=pChkRule->acl_filter.egress_ctag_pri){//HW limitation, untagged case egress_ctag_pri will be port-based pri, and now we force all port-based priority to zero!
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
			}else if(pPktHdr->ingressPort==RTK_RG_PORT_PON){//CF downstream(from PON),check da
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


		if(pChkRule->acl_filter.filter_fields&EGRESS_IP4MC_IF){
			if((pPktHdr->pDmac[0]==0x01&& pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e) && (pPktHdr->tagif&IGMP_TAGIF)==0x0){
				is_ip4mc_check = 1; //IP4MC(not include IGMP)
			}

			if(pChkRule->acl_filter.egress_ip4mc_if!=is_ip4mc_check){
				ACL("RG_ACL[%d] EGRESS_IP4MC_IF UNHIT ",i);
						continue;
			}
		}

		
		if(pChkRule->acl_filter.filter_fields&EGRESS_IP6MC_IF){
			if((pPktHdr->pDmac[0]==0x33 && pPktHdr->pDmac[1]==0x33) && (pPktHdr->tagif&IPV6_MLD_TAGIF)==0x0){
				is_ip6mc_check = 1; //IP6MC(not include MLD)
			}

			if(pChkRule->acl_filter.egress_ip6mc_if!=is_ip6mc_check){
				ACL("RG_ACL[%d] EGRESS_IP6MC_IF UNHIT ",i);
						continue;
			}

		}


		//record CF hit rule
		//ACL("HIT CF0-63[%d]",i);
		pPktHdr->unmatched_cf_act = IDX_UNHIT_PASS;
skipEgressPatternCheck:
		ACL("RG_ACL[%d] Egress Part Hit!",i);
		pPktHdr->aclDecision.aclEgrHitMask[i>>5]|=(1<<(i&0x1f));

	}


	//check L2 pattern: CF 64-511	
	//ACL("Check CF 64-511: l3Modify=%d l4Modify=%d ingressLocation=%d",l3Modify,l4Modify, pPktHdr->ingressLocation);
	for(i=CF_FOR_L2_FLOW_INDEX_START;i<=CF_FOR_L2_FLOW_INDEX_END;i++){

		if(rg_db.systemGlobal.cf_valid_mask[i>>5]==0x0)
		{
			i+=31; //skip none valid rules for 32 rules per run.
			continue;
		}		
		if((rg_db.systemGlobal.cf_valid_mask[i>>5]>>(((i>>3)&0x3)<<3))&0xff)
		{
			//there is valid rule need to check in these 8 rules
		}
		else
		{
			i+=7; //skip none valid rules for 8 rules per run.
			continue;
		}
	
		//L2 CF64-511 rule is valid if index is none zero
		if(rg_db.systemGlobal.classify_SW_table_entry[i].index!=0){
			pL2CFChkRule = &(rg_db.systemGlobal.classify_SW_table_entry[i]);
		}else{
			continue;
		}
		//ACL("CF64-511 check pattern of CF[%d]",i);
		
		//1 FIXME: now just consider PON as CF port, 
		//here check ingress port. Egress port will check while action modify in each port.
		if(pPktHdr->ingressPort==RTK_RG_PORT_PON){
			if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
				//spa is PON, just need to consider DS rules.
				ACL("RG_CF[%d] DIRECTION UNHIT UPSTREAM ",i);
				continue;
			}	
		}else{
			//consider drop
			if(pL2CFChkRule->us_action_field&CF_US_ACTION_DROP_BIT){
				//let it continue check. The Drop action will alwaye be Execute(if hit) even packet is not related to CF port.
				
			}else{
#if defined(CONFIG_RTL9602C_SERIES)
				if(egressPort!=RTK_RG_PORT_PON)
#else
				if((egressPort!=RTK_RG_PORT_PON) && (egressPort!=RTK_RG_PORT_RGMII))
#endif							
				{//make sure its upstream, must egress to CF port
					ACL("RG_CF[%d] DIRECTION UNHIT UPSTREAM egressPort=%d ",i,egressPort);
					continue;	
				} 	
			
				if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
					//spa is none PON, just need to consider US rules.
					ACL("RG_CF[%d] DIRECTION UNHIT DOWNSTREAM ",i);
					continue;
				}
			}
		}

		if(pL2CFChkRule->filter_fields & EGRESS_ETHERTYPR_BIT){ //support mask
			if((pL2CFChkRule->etherType & pL2CFChkRule->etherType_mask)!= (pPktHdr->etherType& pL2CFChkRule->etherType_mask)){
				ACL("RG_CF[%d] EGRESS_ETHERTYPR_BIT UNHIT ",i);
				continue;
			}
		}
		
		if(pL2CFChkRule->filter_fields & EGRESS_GEMIDX_BIT){ //support mask
			/*where is the source? Rxdesc?*/
			if((pL2CFChkRule->gemidx & pL2CFChkRule->gemidx_mask)!= (pPktHdr->pRxDesc->opts2.bit.pon_stream_id & pL2CFChkRule->gemidx_mask)){
				ACL("RG_CF[%d] EGRESS_GEMIDX_BIT UNHIT ",i);
				continue;
			}
		}

		if(pL2CFChkRule->filter_fields & EGRESS_LLID_BIT){
			/*where is the source? Rxdesc?*/
			if(pL2CFChkRule->llid != pPktHdr->pRxDesc->opts2.bit.pon_stream_id){
				ACL("RG_CF[%d] EGRESS_LLID_BIT UNHIT ",i);
				continue;
			}
		}
		
		if(pL2CFChkRule->filter_fields & EGRESS_TAGVID_BIT){
			if(pPktHdr->tagif&SVLAN_TAGIF){
				if(pL2CFChkRule->outterTagVid != pPktHdr->stagVid){
					ACL("RG_CF[%d] EGRESS_TAGVID_BIT UNHIT ",i);
					continue;
				}
			}else if(pPktHdr->tagif&CVLAN_TAGIF){
				if(pL2CFChkRule->outterTagVid != pPktHdr->ctagVid){
					ACL("RG_CF[%d] EGRESS_TAGVID_BIT UNHIT ",i);
					continue;
				}
			}else{
				ACL("RG_CF[%d] EGRESS_TAGVID_BIT UNHIT ",i);
				continue;
			}
		}
		
		if(pL2CFChkRule->filter_fields & EGRESS_TAGPRI_BIT){
			if(pPktHdr->tagif&SVLAN_TAGIF){
				if(pL2CFChkRule->outterTagPri != pPktHdr->stagPri){
					ACL("RG_CF[%d] EGRESS_TAGPRI_BIT UNHIT ",i);
					continue;
				}
			}else if(pPktHdr->tagif&CVLAN_TAGIF){
				if(pL2CFChkRule->outterTagPri != pPktHdr->ctagPri){
					ACL("RG_CF[%d] EGRESS_TAGPRI_BIT UNHIT ",i);
					continue;
				}
			}else{
				ACL("RG_CF[%d] EGRESS_TAGPRI_BIT UNHIT ",i);
				continue;
			}
		}
		
		if(pL2CFChkRule->filter_fields & EGRESS_INTERNALPRI_BIT){
			if(pL2CFChkRule->internalPri != pPktHdr->internalPriority){
				ACL("RG_CF[%d] EGRESS_INTERNALPRI_BIT UNHIT ",i);
				continue;
			}
		}
		
		if(pL2CFChkRule->filter_fields & EGRESS_STAGIF_BIT){
			if((pL2CFChkRule->stagIf ==0 && (pPktHdr->tagif&SVLAN_TAGIF)) ||
				(pL2CFChkRule->stagIf ==1 && !(pPktHdr->tagif&SVLAN_TAGIF))){
				ACL("RG_CF[%d] EGRESS_STAGIF_BIT UNHIT ",i);
				continue;
			}
		}
		
		if(pL2CFChkRule->filter_fields & EGRESS_CTAGIF_BIT){
			if((pL2CFChkRule->ctagIf ==0 && (pPktHdr->tagif&CVLAN_TAGIF)) ||
				(pL2CFChkRule->ctagIf ==1 && !(pPktHdr->tagif&CVLAN_TAGIF))){
				ACL("RG_CF[%d] EGRESS_CTAGIF_BIT UNHIT ",i);
				continue;
			}
		}
		
		if(pL2CFChkRule->filter_fields & EGRESS_UNI_BIT){ //support mask
			//upstream
			if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
				if((pL2CFChkRule->uni & pL2CFChkRule->uni_mask)!= (pPktHdr->ingressPort & pL2CFChkRule->uni_mask)){
					ACL("RG_CF[%d] EGRESS_UNI_BIT UNHIT ",i);
					continue;
				}
			}
			//downstream : should decide by DMAC lookup
			if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
				if (l3Modify){//NAPT or V4V6Routing, ingress DA lookup should always be CPU port because DA==Gateway MAC 
					if((pL2CFChkRule->uni & pL2CFChkRule->uni_mask)!=RTK_RG_MAC_PORT_CPU){
						ACL("RG_CF[%d] EGRESS_UNI_BIT UNHIT ",i);
						continue;
					}
				}
				else{//Bridge  
					//TRACE("uni=%d uni_mask=%d egressPort=%d",pL2CFChkRule->uni,pL2CFChkRule->uni_mask,pPktHdr->egressMACPort);
					if((pL2CFChkRule->uni & pL2CFChkRule->uni_mask)!= (pPktHdr->egressMACPort & pL2CFChkRule->uni_mask)){
						ACL("RG_CF[%d] EGRESS_UNI_BIT UNHIT ",i);
						continue;
					}
				}
			}					
		}

		pPktHdr->aclDecision.direction = pL2CFChkRule->direction;
		pPktHdr->aclDecision.aclEgrL2HitMask[(i>>5)]|=(1<<(i&0x1f));
		pPktHdr->aclDecision.cf64to511RuleHit = 1;
		ACL("RG_CF[%d] Hit!!!",i);

		pPktHdr->unmatched_cf_act = IDX_UNHIT_PASS;
		//FIXME:CF can just hit one rule! So we just check until first hit.
		break;
		
	}
	return RT_ERR_RG_OK;

}

__SRAM_FWDENG_SLOWPATH
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
							
							ACL("RG_ACL[%d] do CVLANTAG to %s %s %s vid=%d pri=%d", 
								acl_idx, 
								name_of_cact_Decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision],
								name_of_cvid_Decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCvidDecision],
								name_of_cpri_Decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCpriDecision],
								pPktHdr->aclDecision.action_acl_cvlan.assignedCvid,
								pPktHdr->aclDecision.action_acl_cvlan.assignedCpri
								);
						}
						else
						{
							ACL("RG_ACL[%d] skip CVLANTAG to %s %s %s vid=%d pri=%d, because CF[0-64] is already done!", 
								acl_idx, 
								name_of_cact_Decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision],
								name_of_cvid_Decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCvidDecision],
								name_of_cpri_Decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCpriDecision],
								pPktHdr->aclDecision.action_acl_cvlan.assignedCvid,
								pPktHdr->aclDecision.action_acl_cvlan.assignedCpri
								);
						}
					}
				}
				
				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_SVLANTAG_BIT){
					if(!(pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_SVLANTAG_BIT)){//ACL_ACTION_ACL_SVLANTAG_BIT have not been set

						if(pPktHdr->aclDecision.l34CFRuleHit == 0){
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_ACL_SVLANTAG_BIT;
							pPktHdr->aclDecision.action_acl_svlan= pChkRule->acl_filter.action_acl_svlan;
							if(pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision!=ACL_SVLAN_TAGIF_NOP){//nop will not effect CF[64-511] hit
								pPktHdr->aclDecision.l2CFHitAction |=(1<<EGR_CSACT_DONE);
							}
						
							ACL("RG_ACL[%d] do SVLANTAG to %s %s %s svid=%d spri=%d", 
								acl_idx, 
								name_of_sact_Decision[pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision],
								name_of_svid_Decision[pPktHdr->aclDecision.action_acl_svlan.svlanSvidDecision],
								name_of_spri_Decision[pPktHdr->aclDecision.action_acl_svlan.svlanSpriDecision],
								pPktHdr->aclDecision.action_acl_svlan.assignedSvid,
								pPktHdr->aclDecision.action_acl_svlan.assignedSpri
								);
						}
						else
						{
							ACL("RG_ACL[%d] skip SVLANTAG to %s %s %s svid=%d spri=%d, because CF[0-64] is already done!", 
								acl_idx, 
								name_of_sact_Decision[pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision],
								name_of_svid_Decision[pPktHdr->aclDecision.action_acl_svlan.svlanSvidDecision],
								name_of_spri_Decision[pPktHdr->aclDecision.action_acl_svlan.svlanSpriDecision],
								pPktHdr->aclDecision.action_acl_svlan.assignedSvid,
								pPktHdr->aclDecision.action_acl_svlan.assignedSpri
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


	//check CF[64-511].
	{
		//just need to do first rule
		for(i=CF_FOR_L2_FLOW_INDEX_START;i<=CF_FOR_L2_FLOW_INDEX_END;i++){

			if(rg_db.systemGlobal.cf_valid_mask[i>>5]==0x0)
			{
				i+=31; //skip none valid rules for 32 rules per run.
				continue;
			}
			
			if((rg_db.systemGlobal.cf_valid_mask[i>>5]>>(((i>>3)&0x3)<<3))&0xff)
			{
				//there is valid rule need to check in these 8 rules
			}
			else
			{
				i+=7; //skip none valid rules for 8 rules per run.
				continue;
			}

			
			if(pPktHdr->aclDecision.aclEgrL2HitMask[(i>>5)]&(1<<(i&0x1f))){
				//ACL(" Do CF[%d] actions",i);
				//copy the rule action to final aclDecision
				if(rg_db.systemGlobal.classify_SW_table_entry[i].direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
					pPktHdr->aclDecision.us_action_field = rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field;

					//Check is that drop action
					if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_DROP_BIT){

						if(pPktHdr->aclDecision.l34CFRuleHit != 0)//drop(UNI) action is already be excuted, skip this drop and continue.
						{
							if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_UNI_ACT_DONE))!=0) 
								return RG_FWDENGINE_RET_CONTINUE;
						}
						else
						{
							TRACE("RG_CF[%d] DROP",i);
							return RG_FWDENGINE_RET_DROP;
						}
					}

				}else{
					pPktHdr->aclDecision.ds_action_field = rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field;

					//Check is that drop action
					if(rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_DROP_BIT){

						if(pPktHdr->aclDecision.l34CFRuleHit != 0)//drop(UNI) action is already be excuted, skip this drop and continue.
						{
							if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_UNI_ACT_DONE))!=0) 
								return RG_FWDENGINE_RET_CONTINUE;
						}
						else
						{
							TRACE("RG_CF[%d] DROP",i);
							return RG_FWDENGINE_RET_DROP;
						}
					}	

				}
				//ACL("Copy CF[%d] to final action decision",i);
				
				pPktHdr->aclDecision.direction=rg_db.systemGlobal.classify_SW_table_entry[i].direction;
				pPktHdr->aclDecision.us_action_field=rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field;
				pPktHdr->aclDecision.ds_action_field=rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field;
				if(direct==RG_FWD_DECISION_NAPTR && rg_db.pktHdr->pRxDesc->opts3.bit.src_port_num==RTK_RG_MAC_PORT_PON){//"NAPT downstream" CF64-511 shoule not effect NAT vlan decision (final vlan decisuin will be decide by NAT, now patch by ignor the CF64-511 C-ation) 
					ACL("RG_CF[%d] Downstrean NATP flow: CF64-511 will not effect cvlan_action. This CVLAN will be decided by NAT",i);
				}else{
					if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_CACT_DONE))==0) //CACT already_hit_if check
					{	
						pPktHdr->aclDecision.action_cvlan=rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan;
					}
					else
					{
						pPktHdr->aclDecision.us_action_field &= (~CF_US_ACTION_CTAG_BIT);
						pPktHdr->aclDecision.ds_action_field &= (~CF_DS_ACTION_CTAG_BIT);
						ACL("RG_CF[%d] CACT has been executed\n",i);
					}
				}
#if 0					
				if(pPktHdr->aclDecision.l34CFRuleHit == 0 || pPktHdr->aclDecision.l2CFHitAction==0)
				{
					pPktHdr->aclDecision.action_svlan=rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan;
					pPktHdr->aclDecision.action_cfpri=rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri;
					pPktHdr->aclDecision.action_sid_or_llid=rg_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid;
					pPktHdr->aclDecision.action_dscp=rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp;
					pPktHdr->aclDecision.action_log=rg_db.systemGlobal.classify_SW_table_entry[i].action_log;
					pPktHdr->aclDecision.action_uni=rg_db.systemGlobal.classify_SW_table_entry[i].action_uni;
				}
				else
#endif
				{
					if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_CSACT_DONE))==0)//SACT already_hit_if check
					{	
						pPktHdr->aclDecision.action_svlan=rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan;
					}
					else
					{	
						pPktHdr->aclDecision.us_action_field &= (~CF_US_ACTION_STAG_BIT);
						pPktHdr->aclDecision.ds_action_field &= (~CF_DS_ACTION_STAG_BIT);
						ACL("RG_CF[%d] CSACT has been executed\n",i);
					}
					
					if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_CFPRI_ACT_DONE))==0)//CFACT already_hit_if check				
					{	
						pPktHdr->aclDecision.action_cfpri=rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri;
					}
					else
					{
						pPktHdr->aclDecision.us_action_field &= (~CF_US_ACTION_CFPRI_BIT);
						pPktHdr->aclDecision.ds_action_field &= (~CF_DS_ACTION_CFPRI_BIT);
						ACL("RG_CF[%d] CFPRI has been executed\n",i);
					}

					if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_SID_ACT_DONE))==0)//SID already_hit_if check						
					{	
						pPktHdr->aclDecision.action_sid_or_llid=rg_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid;
					}
					else
					{
						pPktHdr->aclDecision.us_action_field &= (~CF_US_ACTION_SID_BIT);
						ACL("RG_CF[%d] SID has been executed\n",i);
					}
					
					if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_DSCP_REMARK_DONE))==0)//DSCP already_hit_if check							
					{
						pPktHdr->aclDecision.action_dscp=rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp;
					}
					else
					{
						pPktHdr->aclDecision.us_action_field &= (~CF_US_ACTION_DSCP_BIT);
						pPktHdr->aclDecision.ds_action_field &= (~CF_DS_ACTION_DSCP_BIT);
						ACL("RG_CF[%d] DSCP has been executed\n",i);
					}
					
					if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_LOG_ACT_DONE))==0)//LOG already_hit_if check							
					{	
						pPktHdr->aclDecision.action_log=rg_db.systemGlobal.classify_SW_table_entry[i].action_log;
					}
					else
					{
						pPktHdr->aclDecision.us_action_field &= (~CF_US_ACTION_LOG_BIT);
						ACL("RG_CF[%d] LOG has been executed\n",i);
					}
					if((pPktHdr->aclDecision.l2CFHitAction & (1<<EGR_UNI_ACT_DONE))==0)//UNI already_hit_if check							
					{
						pPktHdr->aclDecision.action_uni=rg_db.systemGlobal.classify_SW_table_entry[i].action_uni;
					}
					else
					{
						pPktHdr->aclDecision.ds_action_field &= (~CF_DS_ACTION_UNI_MASK_BIT);
						ACL("RG_CF[%d] UNI(DROP) has been executed\n",i);
					}
				}
#if 0		
				if(pPktHdr->aclDecision.direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){//updtream
					if((pPktHdr->aclDecision.us_action_field & CF_US_ACTION_STAG_BIT)!=0x0){
						ACL("RG_CF[%d] do SVLANTAG to %s %s %s svid=%d spri=%d", 
						i, 
						name_of_sact_Decision[pPktHdr->aclDecision.action_svlan.svlanTagIfDecision],
						name_of_svid_Decision[pPktHdr->aclDecision.action_svlan.svlanSvidDecision],
						name_of_spri_Decision[pPktHdr->aclDecision.action_svlan.svlanSpriDecision],
						pPktHdr->aclDecision.action_svlan.assignedSvid,
						pPktHdr->aclDecision.action_svlan.assignedSpri
						);}
					if((pPktHdr->aclDecision.us_action_field & CF_US_ACTION_CTAG_BIT)!=0x0){
						ACL("RG_CF[%d] do CVLANTAG to %s %s %s vid=%d pri=%d", 
						i, 
						name_of_cact_Decision[pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision],
						name_of_cvid_Decision[pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision],
						name_of_cpri_Decision[pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision],
						pPktHdr->aclDecision.action_cvlan.assignedCvid,
						pPktHdr->aclDecision.action_cvlan.assignedCpri
						);}
					if((pPktHdr->aclDecision.us_action_field & CF_US_ACTION_CFPRI_BIT)!=0x0){
						ACL("RG_CF[%d] do CFPRI to %d",i,pPktHdr->aclDecision.action_cfpri.assignedCfPri);}
					if((pPktHdr->aclDecision.us_action_field & CF_US_ACTION_DSCP_BIT)!=0x0){
						ACL("RG_CF[%d] do DSCP to %d",i,pPktHdr->aclDecision.action_dscp.assignedDscp);}
					if((pPktHdr->aclDecision.us_action_field & CF_US_ACTION_SID_BIT)!=0x0){
						ACL("RG_CF[%d] do SID to %d",i,pPktHdr->aclDecision.action_sid_or_llid.assignedSid_or_llid);}
					if((pPktHdr->aclDecision.us_action_field & CF_US_ACTION_DROP_BIT)!=0x0){
						ACL("RG_CF[%d] do Drop",i);}
					if((pPktHdr->aclDecision.us_action_field & CF_US_ACTION_LOG_BIT)!=0x0){
						ACL("RG_CF[%d] do LOG to Counter[%d]",i,pPktHdr->aclDecision.action_log.assignedCounterIdx);}		
				}else{//downstream
					if((pPktHdr->aclDecision.ds_action_field & CF_DS_ACTION_STAG_BIT)!=0x0){
						ACL("RG_CF[%d] do SVLANTAG to %s %s %s svid=%d spri=%d", 
						i, 
						name_of_sact_Decision[pPktHdr->aclDecision.action_svlan.svlanTagIfDecision],
						name_of_svid_Decision[pPktHdr->aclDecision.action_svlan.svlanSvidDecision],
						name_of_spri_Decision[pPktHdr->aclDecision.action_svlan.svlanSpriDecision],
						pPktHdr->aclDecision.action_svlan.assignedSvid,
						pPktHdr->aclDecision.action_svlan.assignedSpri
						);}
					if((pPktHdr->aclDecision.ds_action_field & CF_DS_ACTION_CTAG_BIT)!=0x0){
						ACL("RG_CF[%d] do CVLANTAG to %s %s %s vid=%d pri=%d", 
						i, 
						name_of_cact_Decision[pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision],
						name_of_cvid_Decision[pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision],
						name_of_cpri_Decision[pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision],
						pPktHdr->aclDecision.action_cvlan.assignedCvid,
						pPktHdr->aclDecision.action_cvlan.assignedCpri
						);}
					if((pPktHdr->aclDecision.ds_action_field & CF_DS_ACTION_CFPRI_BIT)!=0x0){
						ACL("RG_CF[%d] do CFPRI to %d",i,pPktHdr->aclDecision.action_cfpri.assignedCfPri);}
					if((pPktHdr->aclDecision.ds_action_field & CF_DS_ACTION_DSCP_BIT)!=0x0){
						ACL("RG_CF[%d] do DSCP to %d",i,pPktHdr->aclDecision.action_dscp.assignedDscp);}
					if((pPktHdr->aclDecision.ds_action_field & CF_DS_ACTION_UNI_MASK_BIT)!=0x0){
						ACL("RG_CF[%d] do  %s to 0x%x",i,name_of_uni_Decision[pPktHdr->aclDecision.action_uni.uniActionDecision],pPktHdr->aclDecision.action_uni.assignedUniPortMask);}
					if((pPktHdr->aclDecision.ds_action_field & CF_DS_ACTION_DROP_BIT)!=0x0){
						ACL("RG_CF[%d] do Drop",i);}
				}
#endif
				
				//just need to do first rule
				break;
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

/*
*  modified ACL Qos decistion to pktHdr->(final CVLAN/SVLAN decition)
*/
int _rtk_rg_modifyPacketByACLAction(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr,rtk_rg_port_idx_t egressPort) //egressPort==-1 or RTK_RG_MAC_PORT_MAX, means broadcast to Lan
{
#if defined(CONFIG_APOLLO)
	unsigned char tos;		
#ifdef CONFIG_DUALBAND_CONCURRENT	
	if(pPktHdr->egressVlanTagif==1 &&
		pPktHdr->egressVlanID==CONFIG_DEFAULT_TO_SLAVE_GMAC_VID && 
		pPktHdr->egressPriority==CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI)
	{
		//wifi slave datapath do not modified cvlan/svlan
		
	}
	else
#endif
	{
		//1 FIXME: at the moment, we only care QoS actions.  shareMeter didn't handeled.

		/* do ACL+CF0-63 actions */
		if(pPktHdr->aclDecision.action_type==ACL_ACTION_TYPE_QOS)
		{		
			if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_1P_REMARKING_BIT)>0)
			{
				pPktHdr->egressPriority = pPktHdr->aclDecision.action_dot1p_remarking_pri;
				TRACE("Modify by ACL_CF[0-63] ACT:1P_REMARKING egreesPri=%d",pPktHdr->aclDecision.action_dot1p_remarking_pri);
			}
			
			if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT)>0)
			{
				if(pPktHdr->pTos==NULL)goto ACL_RET;		//packet may not have IP header
				if(!(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)) goto ACL_RET;
				
				if(pPktHdr->tagif&IPV6_TAGIF)
				{
					//ip precedence is the MSB 3 bits of traffic class
					tos = pPktHdr->aclDecision.action_ip_precedence_remarking_pri<<0x1;
					tos |= (*pPktHdr->pTos)&0xf1;		//keep version 4 bits and original traffic class MSB 4th bit
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;
				}
				else
				{	
					tos = pPktHdr->aclDecision.action_ip_precedence_remarking_pri<<0x5;
					tos |= (*pPktHdr->pTos)&0x1f;		//keep 5 bits from LSB
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;		//remarking tos of packet					
				}
				TRACE("Modify by ACL_CF[0-63] ACT:IP_PRECEDENCE_REMARKING ToS=%d",tos);
			}
			else if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_DSCP_REMARKING_BIT)>0)
			{
				if(pPktHdr->pTos==NULL)goto ACL_RET;		//packet may not have IP header
				if(!(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)) goto ACL_RET;
				
				if(pPktHdr->tagif&IPV6_TAGIF)
				{
					//dscp is the MSB 6 bits of traffic class
					tos = pPktHdr->aclDecision.action_dscp_remarking_pri>>0x2;	//dscp MSB 4 bits
					tos |= (*pPktHdr->pTos)&0xf0;		//keep version 4 bits
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;

					tos = (pPktHdr->aclDecision.action_dscp_remarking_pri&0x3)<<0x6;	//dscp LSB 2 bits
					tos |= (*(pPktHdr->pTos+1))&0x3f;		//keep original traffic label LSB 2 bits and flow label MSB 4 bits
					*((skb->data + (pPktHdr->pTos-pPktHdr->skb->data))+1)=tos;
				}
				else
				{
					tos = pPktHdr->aclDecision.action_dscp_remarking_pri<<0x2;
					tos |= (*pPktHdr->pTos)&0x3;		//keep 2 bits from LSB
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;		//remarking tos of packet
				}
				pPktHdr->egressDSCP = pPktHdr->aclDecision.action_dscp_remarking_pri;
				TRACE("Modify by ACL_CF[0-63] ACT:DSCP_REMARKING DSCP=%d",pPktHdr->egressDSCP);
			}

			if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT)>0){
				pPktHdr->internalPriority= pPktHdr->aclDecision.action_acl_egress_internal_priority;	
				TRACE("Modify by ACL_CF[0-63] ACT:CFPRI Egress_internal_priority=%d",pPktHdr->internalPriority);
			}

			if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_QUEUE_ID_BIT)>0){
				pPktHdr->internalPriority= pPktHdr->aclDecision.action_queue_id;	
				TRACE("Modify by ACL_CF[0-63] ACT:QUEUE_ID QueueID=%d (use internal priority)",pPktHdr->internalPriority);
			}


			if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_STREAM_ID_OR_LLID_BIT)>0){
#ifdef CONFIG_GPON_FEATURE
				if(rg_db.systemGlobal.initParam.wanPortGponMode){
					pPktHdr->streamID = pPktHdr->aclDecision.action_stream_id_or_llid&0x7f;				
					TRACE("Modify by ACL_CF[0-63] ACT:STREAM_ID_OR_LLID StreamID=%d",pPktHdr->aclDecision.action_stream_id_or_llid&0x7f);
				}else{
					TRACE("Modify by ACL_CF[0-63] Weired!!!  wanPortGponMode=%d but assigned streamID action rule, skip this streamID action.",rg_db.systemGlobal.initParam.wanPortGponMode);
				}
#endif				
			}

			if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_REDIRECT_BIT)>0){
				pPktHdr->egressUniPortmask = pPktHdr->aclDecision.action_redirect_portmask;	
				TRACE("Modify by ACL_CF[0-63] ACT:REDIRCT egressPortMask=0x%x",pPktHdr->egressUniPortmask);
			}
		

			if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_CVLANTAG_BIT)>0){
				
				switch(pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision){
					case ACL_CVLAN_TAGIF_TAGGING: 
						pPktHdr->egressVlanTagif = 1; //force tagging
						//cvid decision
						switch(pPktHdr->aclDecision.action_acl_cvlan.cvlanCvidDecision){
							case ACL_CVLAN_CVID_ASSIGN: 
								pPktHdr->egressVlanID = pPktHdr->aclDecision.action_acl_cvlan.assignedCvid; 
								pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
								TRACE("Modify by ACL_CF[0-63] ACT:CVID_ASSIGN CVID=%d",pPktHdr->egressVlanID);
								break;
							case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: 
								if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
									pPktHdr->egressVlanID = pPktHdr->stagVid; 
									pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
									TRACE("Modify by ACL_CF[0-63] ACT:CVID_COPY_FROM_1ST_TAG(S) CVID=%d",pPktHdr->egressVlanID);
								}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
									pPktHdr->egressVlanID = pPktHdr->ctagVid;
									pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
									TRACE("Modify by ACL_CF[0-63] ACT:CVID_COPY_FROM_1ST_TAG(C) CVID=%d",pPktHdr->egressVlanID);
								}else{
									//ingress without tag, keep original decision
								}
								break;
							case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: 
								if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
									pPktHdr->egressVlanID = pPktHdr->ctagVid; 
									pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
									TRACE("Modify by ACL_CF[0-63] ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
								}else{
									//ingress without double tag, keep original decision
								}
								break;
							case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: 
								pPktHdr->egressVlanID = pPktHdr->internalVlanID;
								pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
								TRACE("Modify by ACL_CF[0-63] ACT:CVID_COPY_FROM_INTERNAL_VID CVID=%d",pPktHdr->egressVlanID);
								break;
							case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: 
								WARNING("CVLAN action(CVID_CPOY_FROM_DMAC2CVID) is not supported in fwdEngine."); 
								break;
							default:
								break;
						}
						
						//cpri decision
						if(egressPort==RTK_RG_PORT_PON &&
							((pPktHdr->aclDecision.qos_actions&ACL_ACTION_1P_REMARKING_BIT)>0 || rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[egressPort]==RTK_RG_ENABLED)){//upstream ACL p-bit remarking > QoS p-bit remarking > CF p-bit remarking
							//if (upstream) and (hit ACL remarking or have QoS remarking), skip CF p-bit action
							TRACE("Modify by ACL_CF[0-63] ACT:CPRI_INVALID because of ACL-pbit remarking or Port-Based p-bit remarking");
						}else{
							switch(pPktHdr->aclDecision.action_acl_cvlan.cvlanCpriDecision){
								case ACL_CVLAN_CPRI_ASSIGN: 
									pPktHdr->egressPriority = pPktHdr->aclDecision.action_acl_cvlan.assignedCpri;
									TRACE("Modify by ACL_CF[0-63] ACT:CPRI_ASSIGN CPRI=%d",pPktHdr->egressPriority);
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressPriority = pPktHdr->stagPri;
										TRACE("Modify by ACL_CF[0-63] ACT:CPRI_COPY_FROM_1ST_TAG(S) CPRI=%d",pPktHdr->egressPriority);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressPriority = pPktHdr->ctagPri; 
										TRACE("Modify by ACL_CF[0-63] ACT:CPRI_COPY_FROM_1ST_TAG(C) CPRI=%d",pPktHdr->egressPriority);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG:
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressPriority = pPktHdr->ctagPri; 
										TRACE("Modify by ACL_CF[0-63] ACT:CPRI_COPY_FROM_2ND_TAG CPRI=%d",pPktHdr->egressPriority);
									}else{
										//ingress without double tag, keep original decision
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: 
									pPktHdr->egressPriority = pPktHdr->internalPriority;
									TRACE("Modify by ACL_CF[0-63] ACT:CPRI_COPY_FROM_INTERNAL CPRI=%d",pPktHdr->egressPriority);
									break;
								default:
									break;

							}
						}

						break;
					case ACL_CVLAN_TAGIF_TAGGING_WITH_C2S: WARNING("CVLAN action(TAGGING_WITH_C2S) is not supported in fwdEngine."); break;
					case ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C: WARNING("CVLAN action(TAGGING_WITH_SP2C) is not supported in fwdEngine."); break;
					case ACL_CVLAN_TAGIF_UNTAG: 
						pPktHdr->egressVlanTagif = 0; 
						TRACE("Modify by ACL_CF[0-63] ACT:UN-CTAG");
						break;

					case ACL_CVLAN_TAGIF_NOP:
						/*do nothing, follow switch-core*/
						break;
					case ACL_CVLAN_TAGIF_TRANSPARENT: 
						/*keep ingress ctag(untag)*/
						pPktHdr->dmac2VlanID = FAIL;
						if(pPktHdr->tagif & CVLAN_TAGIF){
							pPktHdr->egressVlanTagif = 1; //force tagging
							pPktHdr->egressVlanID = pPktHdr->ctagVid;
							pPktHdr->egressPriority = pPktHdr->ctagPri; 
							
						}else{//untag
							pPktHdr->egressVlanTagif = 0; //force untag
						}	
						break;
					default:
						break;
				}
			}


			if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_SVLANTAG_BIT)>0){

				switch(pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision){
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: 
						pPktHdr->egressServiceVlanTagif = 1; //force tagging with tpid
						TRACE("Modify by ACL_CF[0-63] ACT:SVLANTAG with TPID");
						goto svidSpriDecision;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_8100: 
						pPktHdr->egressServiceVlanTagif = 2; //force tagging with 0x8100
						TRACE("Modify by ACL_CF[0-63] ACT:SVLANTAG with 8100");
svidSpriDecision:
						//svid decision
						switch(pPktHdr->aclDecision.action_acl_svlan.svlanSvidDecision){
							case ACL_SVLAN_SVID_ASSIGN: 
								pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_acl_svlan.assignedSvid;
								TRACE("Modify by ACL_CF[0-63] ACT:SVID_ASSIGN SVID=%d",pPktHdr->egressServiceVlanID);
								break;
							case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: 
								if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
									pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
									TRACE("Modify by ACL_CF[0-63] ACT:SVID_COPY_FROM_1ST_TAG(S) SVID=%d",pPktHdr->egressServiceVlanID);
								}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
									pPktHdr->egressServiceVlanID = pPktHdr->ctagVid; 
									TRACE("Modify by ACL_CF[0-63] ACT:SVID_COPY_FROM_1ST_TAG(C) SVID=%d",pPktHdr->egressServiceVlanID);
								}else{
									//ingress without tag, keep original decision
								}
								break;
							case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: 
								if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
									pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
									TRACE("Modify by ACL_CF[0-63] ACT:SVID_COPY_FROM_2ND_TAG SVID=%d",pPktHdr->egressServiceVlanID);
								}else{
									//ingress without double tag, keep original decision
								}
								break;
							default:
								break;
						}

						//spri decision
						switch(pPktHdr->aclDecision.action_acl_svlan.svlanSpriDecision){
							case ACL_SVLAN_SPRI_ASSIGN: 
								pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_acl_svlan.assignedSpri;
								TRACE("Modify by ACL_CF[0-63] ACT:SPRI_ASSIGN SPRI=%d",pPktHdr->egressServicePriority);
								break;
							case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: 
								if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
									pPktHdr->egressServicePriority = pPktHdr->stagPri;
									TRACE("Modify by ACL_CF[0-63] ACT:SPRI_COPY_FROM_1ST_TAG(S) SPRI=%d",pPktHdr->egressServicePriority);
								}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
									pPktHdr->egressServicePriority = pPktHdr->ctagPri; 
									TRACE("Modify by ACL_CF[0-63] ACT:SPRI_COPY_FROM_1ST_TAG(C) SPRI=%d",pPktHdr->egressServicePriority);
								}else{
									//ingress without tag, keep original decision
								}
								break;
							case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: 
								if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
									pPktHdr->egressServicePriority = pPktHdr->ctagPri;
									TRACE("Modify by ACL_CF[0-63] ACT:SPRI_COPY_FROM_2ND_TAG SPRI=%d",pPktHdr->egressServicePriority);
								}else{
									//ingress without double tag, keep original decision
								}
								break;
							case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: 
								pPktHdr->egressServicePriority = pPktHdr->internalPriority;
								TRACE("Modify by ACL_CF[0-63] ACT:SPRI_COPY_FROM_INTERNAL_PRI SPRI=%d",pPktHdr->egressServicePriority);
								break;
							default: 
								break;
						}
						break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C: WARNING("SVLAN action(TAGGING_WITH_SP2C) is not supported in fwdEngine."); break; //downstream only
					case ACL_SVLAN_TAGIF_UNTAG: 
						pPktHdr->egressServiceVlanTagif = 0; 
						TRACE("Modify by ACL_CF[0-63] ACT:UN-STAG");
						break;
					case ACL_SVLAN_TAGIF_NOP:
						break;
					case ACL_SVLAN_TAGIF_TRANSPARENT:
						/*keep ingress stag(untag)*/
						if(pPktHdr->tagif & SVLAN_TAGIF){
							pPktHdr->egressServiceVlanTagif = 1; //force tagging
							pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
							pPktHdr->egressServicePriority = pPktHdr->stagPri; 			
						}else{//untag
							pPktHdr->egressServiceVlanTagif = 0; //force untag
						}
						break;
					default: 
						break;

				}
				
			}


		}


		/* do CF64-511 actions */
		//If CF:0-63 have no rule hit, then do Cf 64-511 action
		//if( pPktHdr->aclDecision.l34CFRuleHit==0)
		{
			//ACL("Do CF64-511 actions:");
			if(pPktHdr->aclDecision.direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
				//check US action

				//SID action
				if(pPktHdr->aclDecision.us_action_field & CF_US_ACTION_SID_BIT){			
#if 0		//those should be filled in _rtk_rg_egressPacketSend; only fill pPktHdr here
					rg_kernel.txDescMask.opts1.bit.cputag_psel=1;
					rg_kernel.txDescMask.opts3.bit.tx_dst_stream_id=0x7f;
					rg_kernel.txDesc.opts1.bit.cputag_psel=1;
					rg_kernel.txDesc.opts3.bit.tx_dst_stream_id=pPktHdr->aclDecision.action_sid_or_llid.assignedSid_or_llid&0x7f;
					TRACE("Modify by CF_UP[64-511] ACT:StreamID SID=%d",rg_kernel.txDesc.opts3.bit.tx_dst_stream_id);
#endif
#ifdef CONFIG_GPON_FEATURE
					pPktHdr->streamID = pPktHdr->aclDecision.action_sid_or_llid.assignedSid_or_llid&0x7f;
					TRACE("Modify by CF_UP[64-511] ACT:StreamID SID=%d",pPktHdr->streamID);
#endif				
					//ACL(" CF enable CF_US_ACTION_SID_BIT sid=%d",pPktHdr->aclDecision.action_sid_or_llid.assignedSid_or_llid);
					
				}
				
				//Stag action
				if((pPktHdr->aclDecision.us_action_field&CF_US_ACTION_STAG_BIT)>0){
	
					switch(pPktHdr->aclDecision.action_svlan.svlanTagIfDecision){
						case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: 
							pPktHdr->egressServiceVlanTagif = 1; //force tagging with tpid
							TRACE("Modify by CF_UP[64-511] ACT:SVLAN_TAGIF_TAGGING_WITH_VSTPID");
							goto svidSpriDecisionForUsCF;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_8100: 
							pPktHdr->egressServiceVlanTagif = 2; //force tagging with 0x8100
							TRACE("Modify by CF_UP[64-511] ACT:SVLAN_TAGIF_TAGGING_WITH_8100");
svidSpriDecisionForUsCF:
							//svid decision
							switch(pPktHdr->aclDecision.action_svlan.svlanSvidDecision){
								case ACL_SVLAN_SVID_ASSIGN: 
									pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_svlan.assignedSvid;
									TRACE("Modify by CF_UP[64-511] ACT:SVID_ASSIGN SVID=%d",pPktHdr->egressServiceVlanID);
									break;
								case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressServiceVlanID = pPktHdr->stagVid; 
										TRACE("Modify by CF_UP[64-511] ACT:SVID_COPY_FROM_1ST_TAG(S) SVID=%d",pPktHdr->egressServiceVlanID);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
										TRACE("Modify by CF_UP[64-511] ACT:SVID_COPY_FROM_1ST_TAG(C) SVID=%d",pPktHdr->egressServiceVlanID);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: 
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressServiceVlanID = pPktHdr->ctagVid; 
										TRACE("Modify by CF_UP[64-511] ACT:SVID_COPY_FROM_2ND_TAG SVID=%d",pPktHdr->egressServiceVlanID);
									}else{
										//ingress without double tag, keep original decision
									}
									break;
								default:
									break;
							}
	
							//spri decision
							switch(pPktHdr->aclDecision.action_svlan.svlanSpriDecision){
								case ACL_SVLAN_SPRI_ASSIGN: 
									pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_svlan.assignedSpri;
									TRACE("Modify by CF_UP[64-511] ACT:SPRI_ASSIGN SPRI=%d",pPktHdr->egressServicePriority);
									break;
								case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressServicePriority = pPktHdr->stagPri; 
										TRACE("Modify by CF_UP[64-511] ACT:SPRI_COPY_FROM_1ST_TAG(S) SPRI=%d",pPktHdr->egressServicePriority);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressServicePriority = pPktHdr->ctagPri; 
										TRACE("Modify by CF_UP[64-511] ACT:SPRI_COPY_FROM_1ST_TAG(C) SPRI=%d",pPktHdr->egressServicePriority);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: 
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressServicePriority = pPktHdr->ctagPri; 
										TRACE("Modify by CF_UP[64-511] ACT:SPRI_COPY_FROM_2ND_TAG SPRI=%d",pPktHdr->egressServicePriority);
									}else{
										//ingress without double tag, keep original decision
									}
									break;
								case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: 
									pPktHdr->egressServicePriority = pPktHdr->internalPriority;
									TRACE("Modify by CF_UP[64-511] ACT:SPRI_COPY_FROM_INTERNAL_PRI SPRI=%d",pPktHdr->egressServicePriority);
									break;
								default: 
									break;
							}
							break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C: WARNING("SVLAN action(TAGGING_WITH_SP2C) is not supported in fwdEngine."); break; //downstream only
						case ACL_SVLAN_TAGIF_UNTAG: 
							pPktHdr->egressServiceVlanTagif = 0; 
							TRACE("Modify by CF_UP[64-511] ACT:UN-STAG");
							break;
						case ACL_SVLAN_TAGIF_NOP:
							break;
						case ACL_SVLAN_TAGIF_TRANSPARENT:
							/*keep ingress stag(untag)*/
							if(pPktHdr->tagif & SVLAN_TAGIF){
								pPktHdr->egressServiceVlanTagif = 1; //force tagging
								pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
								pPktHdr->egressServicePriority = pPktHdr->stagPri; 			
							}else{//untag
								pPktHdr->egressServiceVlanTagif = 0; //force untag
							}
							break;
						default: 
							break;
	
					}
					
				}

				//Ctag action
				if((pPktHdr->aclDecision.us_action_field&CF_US_ACTION_CTAG_BIT)>0){
					
					switch(pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision){
						case ACL_CVLAN_TAGIF_TAGGING: 
							pPktHdr->egressVlanTagif = 1; //force tagging
							TRACE("Modify by CF_UP[64-511] ACT:CVLAN_TAGIF_TAGGING");
							//cvid decision
							switch(pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision){
								case ACL_CVLAN_CVID_ASSIGN: 
									pPktHdr->egressVlanID = pPktHdr->aclDecision.action_cvlan.assignedCvid;
									pPktHdr->dmac2VlanID = FAIL;
									TRACE("Modify by CF_UP[64-511] ACT:CVID_ASSIGN CVID=%d",pPktHdr->egressVlanID);
									break;
								case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressVlanID = pPktHdr->stagVid;
										pPktHdr->dmac2VlanID = FAIL;
										TRACE("Modify by CF_UP[64-511] ACT:CVID_COPY_FROM_1ST_TAG(S) CVID=%d",pPktHdr->egressVlanID);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressVlanID = pPktHdr->ctagVid;
										pPktHdr->dmac2VlanID = FAIL;
										TRACE("Modify by CF_UP[64-511] ACT:CVID_COPY_FROM_1ST_TAG(C) CVID=%d",pPktHdr->egressVlanID);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: 
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressVlanID = pPktHdr->ctagVid;
										pPktHdr->dmac2VlanID = FAIL;
										TRACE("Modify by CF_UP[64-511] ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
									}else{
										//ingress without double tag, keep original decision
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: 
									pPktHdr->egressVlanID = pPktHdr->internalVlanID;
									pPktHdr->dmac2VlanID = FAIL;
									TRACE("Modify by CF_UP[64-511] ACT:CVID_COPY_FROM_INTERNAL CVID=%d",pPktHdr->egressVlanID);
									break;
								case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: 
									WARNING("CVLAN action(CVID_CPOY_FROM_DMAC2CVID) is not supported in fwdEngine."); 
									break;
								default:
									break;
							}
							
							//cpri decision
							switch(pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision){
								case ACL_CVLAN_CPRI_ASSIGN: 
									pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
									TRACE("Modify by CF_UP[64-511] ACT:CPRI_ASSIGN CPRI=%d",pPktHdr->egressPriority);
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressPriority = pPktHdr->stagPri;
										TRACE("Modify by CF_UP[64-511] ACT:CPRI_COPY_FROM_1ST_TAG(S) CPRI=%d",pPktHdr->egressPriority);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressPriority = pPktHdr->ctagPri; 
										TRACE("Modify by CF_UP[64-511] ACT:CPRI_COPY_FROM_1ST_TAG(C) CPRI=%d",pPktHdr->egressPriority);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG:
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressPriority = pPktHdr->ctagPri; 
										TRACE("Modify by CF_UP[64-511] ACT:CPRI_COPY_FROM_2ND_TAG CPRI=%d",pPktHdr->egressPriority);
									}else{
										//ingress without double tag, keep original decision
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: 
									pPktHdr->egressPriority = pPktHdr->internalPriority;
									TRACE("Modify by CF_UP[64-511] ACT:CPRI_COPY_FROM_INTERNAL_PRI CPRI=%d",pPktHdr->egressPriority);
									break;
								default:
									break;

							}
							break;
						case ACL_CVLAN_TAGIF_TAGGING_WITH_C2S: WARNING("CVLAN action(TAGGING_WITH_C2S) is not supported in fwdEngine."); break;
						case ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C: WARNING("CVLAN action(TAGGING_WITH_SP2C) is not supported in fwdEngine."); break;
						case ACL_CVLAN_TAGIF_UNTAG: 
							pPktHdr->egressVlanTagif = 0; 
							TRACE("Modify by CF_UP[64-511] ACT:UN-CTAG");
							break;

						case ACL_CVLAN_TAGIF_NOP:
							/*do nothing, follow switch-core*/
							break;
							
						case ACL_CVLAN_TAGIF_TRANSPARENT: 
							/*keep ingress ctag(untag)*/
							pPktHdr->dmac2VlanID = FAIL;
							if(pPktHdr->tagif & CVLAN_TAGIF){
								pPktHdr->egressVlanTagif = 1; //force tagging
								pPktHdr->egressVlanID = pPktHdr->ctagVid;
								pPktHdr->egressPriority = pPktHdr->ctagPri; 			
							}else{//untag
								pPktHdr->egressVlanTagif = 0; //force untag
							}
							break;
						default:
							break;
					}
				}


	
				//1 FIXME: at the moment, we only care Stag/Ctag SID & DROP actions on CF64~511.
				if(pPktHdr->aclDecision.us_action_field & ~(CF_US_ACTION_SID_BIT|CF_US_ACTION_DROP_BIT|CF_US_ACTION_STAG_BIT|CF_US_ACTION_CTAG_BIT)){
					WARNING("fwdEngin CF:64-511 can just support US STAG / CTAG / SID / DROP action.");
				}
				
			}else{
				//check DS action
				if((pPktHdr->aclDecision.ds_action_field&CF_DS_ACTION_STAG_BIT)>0){
	
					switch(pPktHdr->aclDecision.action_svlan.svlanTagIfDecision){
						case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID:
							pPktHdr->egressServiceVlanTagif = 1; //force tagging with tpid
							TRACE("Modify by CF_DOWN[64-511] ACT:SVLAN_TAGIF_TAGGING_WITH_VSTPID");
							goto svidSpriDecisionForDsCF;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_8100: 
							pPktHdr->egressServiceVlanTagif = 2; //force tagging with 0x8100
							TRACE("Modify by CF_DOWN[64-511] ACT:SVLAN_TAGIF_TAGGING_WITH_8100");
svidSpriDecisionForDsCF:
							//svid decision
							switch(pPktHdr->aclDecision.action_svlan.svlanSvidDecision){
								case ACL_SVLAN_SVID_ASSIGN: 
									pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_svlan.assignedSvid;
									TRACE("Modify by CF_DOWN[64-511] ACT:SVID_ASSIGN SVID=%d",pPktHdr->egressServiceVlanID);
									break;
								case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
										TRACE("Modify by CF_DOWN[64-511] ACT:COPY_FROM_1ST_TAG(S) SVID=%d",pPktHdr->egressServiceVlanID);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressServiceVlanID = pPktHdr->ctagVid; 
										TRACE("Modify by CF_DOWN[64-511] ACT:COPY_FROM_1ST_TAG(C) SVID=%d",pPktHdr->egressServiceVlanID);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: 
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressServiceVlanID = pPktHdr->ctagVid; 
										TRACE("Modify by CF_DOWN[64-511] ACT:COPY_FROM_2ND_TAG SVID=%d",pPktHdr->egressServiceVlanID);
									}else{
										//ingress without double tag, keep original decision
									}									
									break;
									// why do not have copy from internal??
								default:
									break;
							}
	
							//spri decision
							switch(pPktHdr->aclDecision.action_svlan.svlanSpriDecision){
								case ACL_SVLAN_SPRI_ASSIGN: 
									pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_svlan.assignedSpri; 
									TRACE("Modify by CF_DOWN[64-511] ACT:SPRI_ASSIGN SPRI=%d",pPktHdr->egressServicePriority);
									break;
								case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressServicePriority = pPktHdr->stagPri; 
										TRACE("Modify by CF_DOWN[64-511] ACT:SPRI_COPY_FROM_1ST_TAG(S) SPRI=%d",pPktHdr->egressServicePriority);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressServicePriority = pPktHdr->ctagPri; 
										TRACE("Modify by CF_DOWN[64-511] ACT:SPRI_COPY_FROM_1ST_TAG(C) SPRI=%d",pPktHdr->egressServicePriority);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: 
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressServicePriority = pPktHdr->ctagPri; 
										TRACE("Modify by CF_DOWN[64-511] ACT:SPRI_COPY_FROM_2ND_TAG SPRI=%d",pPktHdr->egressServicePriority);
									}else{
										//ingress without double tag, keep original decision
									}
									break;
								case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: 
									pPktHdr->egressServicePriority = pPktHdr->internalPriority;
									TRACE("Modify by CF_DOWN[64-511] ACT:SPRI_COPY_FROM_INTERNAL_PRI SPRI=%d",pPktHdr->egressServicePriority);
									break;
								default: 
									break;
							}
							break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C: WARNING("SVLAN action(TAGGING_WITH_SP2C) is not supported in fwdEngine."); break; //downstream only
						case ACL_SVLAN_TAGIF_UNTAG: 
							pPktHdr->egressServiceVlanTagif = 0; 
							TRACE("Modify by CF_DOWN[64-511] ACT:UN-STAG");
							break;
						case ACL_SVLAN_TAGIF_NOP:
							break;
						case ACL_SVLAN_TAGIF_TRANSPARENT:
							/*keep ingress stag(untag)*/
							if(pPktHdr->tagif & SVLAN_TAGIF){
								pPktHdr->egressServiceVlanTagif = 1; //force tagging
								pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
								pPktHdr->egressServicePriority = pPktHdr->stagPri; 			
							}else{//untag
								pPktHdr->egressServiceVlanTagif = 0; //force untag
							}
							break;
						default: 
							break;
	
					}
					
				}
				
				
				if((pPktHdr->aclDecision.ds_action_field&CF_DS_ACTION_CTAG_BIT)>0){
					
					switch(pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision){
						case ACL_CVLAN_TAGIF_TAGGING: 
							pPktHdr->egressVlanTagif = 1; //force tagging
							TRACE("Modify by CF_DOWN[64-511] ACT:CVLAN_TAGGING");
							//cvid decision
							switch(pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision){
								case ACL_CVLAN_CVID_ASSIGN: 
									pPktHdr->egressVlanID = pPktHdr->aclDecision.action_cvlan.assignedCvid; 
									pPktHdr->dmac2VlanID = FAIL;
									TRACE("Modify by CF_DOWN[64-511] ACT:CVID_ASSIGN CVID=%d",pPktHdr->egressVlanID);
									break;
								case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressVlanID = pPktHdr->stagVid;
										TRACE("Modify by CF_DOWN[64-511] ACT:CVID_COPY_FROM_1ST_TAG(S) CVID=%d",pPktHdr->egressVlanID);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressVlanID = pPktHdr->ctagVid; 
										pPktHdr->dmac2VlanID = FAIL;
										TRACE("Modify by CF_DOWN[64-511] ACT:CVID_COPY_FROM_1ST_TAG(C) CVID=%d",pPktHdr->egressVlanID);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: 
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressVlanID = pPktHdr->ctagVid; 
										pPktHdr->dmac2VlanID = FAIL;
										TRACE("Modify by CF_DOWN[64-511] ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
									}else{
										//ingress without double tag, keep original decision
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: 
									pPktHdr->egressVlanID = pPktHdr->internalVlanID;
									pPktHdr->dmac2VlanID = FAIL;
									TRACE("Modify by CF_DOWN[64-511] ACT:CVID_COPY_FROM_INTERNAL CVID=%d",pPktHdr->egressVlanID);
									break;
								case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: 
									WARNING("CVLAN action(CVID_CPOY_FROM_DMAC2CVID) is not supported in fwdEngine."); 
									break;
								default:
									break;
							}
							
							//cpri decision
							switch(pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision){
								case ACL_CVLAN_CPRI_ASSIGN: 
									pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
									TRACE("Modify by CF_DOWN[64-511] ACT:CPRI_ASSIGN CPRI=%d",pPktHdr->egressPriority);
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: 
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressPriority = pPktHdr->stagPri;
										TRACE("Modify by CF_DOWN[64-511] ACT:CPRI_COPY_FROM_1ST_TAG(S) CPRI=%d",pPktHdr->egressPriority);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressPriority = pPktHdr->ctagPri; 
										TRACE("Modify by CF_DOWN[64-511] ACT:CPRI_COPY_FROM_1ST_TAG(C) CPRI=%d",pPktHdr->egressPriority);
									}else{
										//ingress without tag, keep original decision
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG:
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressPriority = pPktHdr->ctagPri; 
										TRACE("Modify by CF_DOWN[64-511] ACT:CPRI_COPY_FROM_2ND_TAG CPRI=%d",pPktHdr->egressPriority);
									}else{
										//ingress without double tag, keep original decision
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: 
									pPktHdr->egressPriority = pPktHdr->internalPriority;
									TRACE("Modify by CF_DOWN[64-511] ACT:CPRI_COPY_FROM_INTERNAL CPRI=%d",pPktHdr->egressPriority);
									break;
								default:
									break;
	
							}
							break;
						case ACL_CVLAN_TAGIF_TAGGING_WITH_C2S: WARNING("CVLAN action(TAGGING_WITH_C2S) is not supported in fwdEngine."); break;
						case ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C: WARNING("CVLAN action(TAGGING_WITH_SP2C) is not supported in fwdEngine."); break;
						case ACL_CVLAN_TAGIF_UNTAG: 
							pPktHdr->egressVlanTagif = 0; 
							TRACE("Modify by CF_DOWN[64-511] ACT:UN-CTAG");
							break;
	
						
						case ACL_CVLAN_TAGIF_TRANSPARENT: 
							/*keep ingress ctag(untag)*/
							pPktHdr->dmac2VlanID = FAIL;
							if(pPktHdr->tagif & CVLAN_TAGIF){
								pPktHdr->egressVlanTagif = 1; //force tagging
								pPktHdr->egressVlanID = pPktHdr->ctagVid;
								pPktHdr->egressPriority = pPktHdr->ctagPri; 			
							}else{//untag
								pPktHdr->egressVlanTagif = 0; //force untag
							}
							break;
							
						case ACL_CVLAN_TAGIF_NOP:
							/*do nothing*/ break;
						default:
							break;
					}
				}
		
							
				if((pPktHdr->aclDecision.ds_action_field&CF_DS_ACTION_UNI_MASK_BIT)>0){
					switch(pPktHdr->aclDecision.action_uni.uniActionDecision){
						case ACL_UNI_FWD_TO_PORTMASK_ONLY: 
							pPktHdr->aclDecision.action_uni.assignedUniPortMask &= rg_kernel.txDesc.opts3.bit.tx_portmask;
							ACL(" egressPmsk change to 0x%x",pPktHdr->aclDecision.action_uni.assignedUniPortMask);
							TRACE("Modify by CF_DOWN[64-511] ACT:UNI_FWD_TO_PORTMASK_ONLY finalPortMask=0x%x",pPktHdr->aclDecision.action_uni.assignedUniPortMask);
							break;
						case ACL_UNI_FORCE_BY_MASK: 
							ACL(" egressPmsk change to 0x%x",pPktHdr->aclDecision.action_uni.assignedUniPortMask);
							TRACE("Modify by CF_DOWN[64-511] ACT:UNI_FORCE_BY_MASK finalPortMask=0x%x",pPktHdr->aclDecision.action_uni.assignedUniPortMask);
							break;
						default:
							break;						
					}
					pPktHdr->egressUniPortmask = pPktHdr->aclDecision.action_uni.assignedUniPortMask;
				}


				//1 FIXME: at the moment, we only care Stag/Ctag SID & DROP actions on CF64~511.
				if(pPktHdr->aclDecision.ds_action_field & ~(CF_DS_ACTION_STAG_BIT|CF_DS_ACTION_CTAG_BIT|CF_DS_ACTION_DROP_BIT|CF_DS_ACTION_UNI_MASK_BIT)){
					WARNING("fwdEngin CF:64-511 can just support DS STAG / CTAG / DROP /UNI action.");
				}

			}
		}

	}

	
ACL_RET:

#endif	//#if defined(CONFIG_APOLLO)

	return RT_ERR_RG_OK;
}


#if 1 /*(6)debug tool APIs*/
#endif

/*internal utils*/
int _dump_rg_acl_entry_content(struct seq_file *s, rtk_rg_aclFilterAndQos_t *aclPara)
{
	rtk_rg_aclFilterAndQos_t *acl_parameter;

	PROC_PRINTF("fwd_type: %d\n",aclPara->fwding_type_and_direction);
	PROC_PRINTF("acl_weight: %d\n",aclPara->acl_weight);


	PROC_PRINTF("[Patterns]: \n");	
	PROC_PRINTF("filter_fields:0x%llx\n", aclPara->filter_fields);	
	if(aclPara->filter_fields&INGRESS_PORT_BIT) PROC_PRINTF("ingress_port_mask:0x%x\n", aclPara->ingress_port_mask.portmask);	
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

	//acl_parameter = &rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter;
	acl_parameter = aclPara;
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

	if(acl_parameter->filter_fields & EGRESS_IP4MC_IF){
		PROC_PRINTF("egress_ip4mc_if: %s \n",acl_parameter->egress_ip4mc_if?"Must be IP4MC":"Must not be IP4MC");
	}
	if(acl_parameter->filter_fields & EGRESS_IP6MC_IF){
		PROC_PRINTF("egress_ip6mc_if: %s \n",acl_parameter->egress_ip6mc_if?"Must be IP6MC":"Must not be IP6MC");
	}
	
	if(acl_parameter->filter_fields & INGRESS_L4_TCP_BIT){
		PROC_PRINTF("l4-protocal: tcp \n");
	}else if(acl_parameter->filter_fields & INGRESS_L4_UDP_BIT){
		PROC_PRINTF("l4-protocal: udp \n");
	}else if(acl_parameter->filter_fields & INGRESS_L4_ICMP_BIT){
		PROC_PRINTF("l4-protocal: icmp \n");
	}else{
		//PROC_PRINTF("l4-protocal: not care \n");
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
	return 0;
}

int _dump_rg_acl(struct seq_file *s)
{
#if 1
	int i;
	rtk_rg_aclFilterAndQos_t *aclPara;
	//rtk_rg_aclFilterAndQos_t *acl_parameter;

	PROC_PRINTF("acl_SW_table_entry_size:%d\n",rg_db.systemGlobal.acl_SW_table_entry_size);

	PROC_PRINTF("aclSW rule index sorting:\n");
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;
		PROC_PRINTF("ACL[%d]:w(%d)",rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i],
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

			PROC_PRINTF("[Using range tables]: \n");
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&ACL_USED_IPTABLE_IPV4SIP_INDEX) PROC_PRINTF("ACL_SIP4_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[ACL_USED_IPTABLE_IPV4SIP_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&ACL_USED_IPTABLE_IPV4DIP_INDEX) PROC_PRINTF("ACL_DIP4_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[ACL_USED_IPTABLE_IPV4DIP_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&ACL_USED_IPTABLE_IPV6SIP_INDEX) PROC_PRINTF("ACL_SIP6_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[ACL_USED_IPTABLE_IPV6SIP_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&ACL_USED_IPTABLE_IPV6DIP_INDEX) PROC_PRINTF("ACL_DIP6_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[ACL_USED_IPTABLE_IPV6DIP_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&ACL_USED_PORTTABLE_SPORT_INDEX) PROC_PRINTF("ACL_SPORT_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[ACL_USED_PORTTABLE_SPORT_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&ACL_USED_PORTTABLE_DPORT_INDEX) PROC_PRINTF("ACL_DPORT_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[ACL_USED_PORTTABLE_DPORT_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&CF_USED_IPTABLE_IPV4SIP_INDEX) PROC_PRINTF("CF_SIP4_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[CF_USED_IPTABLE_IPV4SIP_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&CF_USED_IPTABLE_IPV4DIP_INDEX) PROC_PRINTF("CF_DIP4_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[CF_USED_IPTABLE_IPV4DIP_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&CF_USED_PORTTABLE_SPORT_INDEX) PROC_PRINTF("CF_SPORT_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[CF_USED_PORTTABLE_SPORT_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&CF_USED_PORTTABLE_DPORT_INDEX) PROC_PRINTF("CF_DPORT_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[CF_USED_PORTTABLE_DPORT_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&CF_USED_DSCPTABLE_INDEX) PROC_PRINTF("CF_DSCP_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[CF_USED_DSCPTABLE_INDEX]);

			_dump_rg_acl_entry_content(s, aclPara);
			
		}

	}
#endif


	return RT_ERR_RG_OK;
}
int _dump_rg_cf(struct seq_file *s)
{
	/*dump CF[64-511]*/
#if 1
	int i;
	rtk_rg_classifyEntry_t* cf_rule;
	for(i=MIN_CF_64TO511_ENTRY_SIZE;i<=MAX_CF_64TO511_ENTRY_SIZE;i++){//only show L2 classify range
		if(rg_db.systemGlobal.classify_SW_table_entry[i].index==i){//if empty, the index should be zero
			cf_rule = &rg_db.systemGlobal.classify_SW_table_entry[i];
			PROC_PRINTF("========cfRule[%d]=========\n",i);
			PROC_PRINTF("direction: %s \n",(cf_rule->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM)?"upstream":"downstream");

			PROC_PRINTF("[patterns]: \n");
			
			if(cf_rule->filter_fields & EGRESS_ETHERTYPR_BIT)
				PROC_PRINTF("ETHERTYPR:0x%x \tMASK:0x%x \n",cf_rule->etherType,cf_rule->etherType_mask);
			if(cf_rule->filter_fields & EGRESS_GEMIDX_BIT)
				PROC_PRINTF("GEMIDX:%d \tMASK:0x%x \n",cf_rule->gemidx,cf_rule->gemidx_mask);
			if(cf_rule->filter_fields & EGRESS_LLID_BIT)
				PROC_PRINTF("LLID:%d \n",cf_rule->llid);
			if(cf_rule->filter_fields & EGRESS_TAGVID_BIT)
				PROC_PRINTF("TAGVID:%d \n",cf_rule->outterTagVid);
			if(cf_rule->filter_fields & EGRESS_TAGPRI_BIT)
				PROC_PRINTF("TAGPRI:%d \n",cf_rule->outterTagPri);
			if(cf_rule->filter_fields & EGRESS_INTERNALPRI_BIT)
				PROC_PRINTF("INTERNALPRI:%d \n",cf_rule->internalPri);
			if(cf_rule->filter_fields & EGRESS_STAGIF_BIT)
				PROC_PRINTF("STAGIF:%s \n",cf_rule->stagIf?"must have Stag":"must not have Stag");
			if(cf_rule->filter_fields & EGRESS_CTAGIF_BIT)
				PROC_PRINTF("CTAGIF:%s \n",cf_rule->ctagIf?"must have Ctag":"must not have Ctag");
			if(cf_rule->filter_fields & EGRESS_UNI_BIT)
				PROC_PRINTF("UNI:%d \tMASK:0x%x \n",cf_rule->uni,cf_rule->uni_mask);
	

			PROC_PRINTF("[valid actions]: \n");
			if(cf_rule->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){//upstream
				if(cf_rule->us_action_field & CF_US_ACTION_STAG_BIT) PROC_PRINTF("SVLAN \n");
				if(cf_rule->us_action_field & CF_US_ACTION_CTAG_BIT) PROC_PRINTF("CVLAN \n");
				if(cf_rule->us_action_field & CF_US_ACTION_CFPRI_BIT) PROC_PRINTF("CFPRI \n");
				if(cf_rule->us_action_field & CF_US_ACTION_DSCP_BIT) PROC_PRINTF("DSCP \n");
				if(cf_rule->us_action_field & CF_US_ACTION_SID_BIT) PROC_PRINTF("SID \n");
				if(cf_rule->us_action_field & CF_US_ACTION_DROP_BIT) PROC_PRINTF("DROP \n");
				if(cf_rule->us_action_field & CF_US_ACTION_LOG_BIT) PROC_PRINTF("LOG \n");
					
			}else{//downstream
				if(cf_rule->ds_action_field & CF_DS_ACTION_STAG_BIT) PROC_PRINTF("SVLAN \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_CTAG_BIT) PROC_PRINTF("CVLAN \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_CFPRI_BIT) PROC_PRINTF("CFPRI \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_DSCP_BIT) PROC_PRINTF("DSCP \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_UNI_MASK_BIT) PROC_PRINTF("UNI \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_DROP_BIT) PROC_PRINTF("DROP \n");
			}
						
		}
	}
#endif

	return RT_ERR_RG_OK;
}


int32 _dump_acl_portRangeTable(struct seq_file *s)
{

	int i;
	rtk_acl_rangeCheck_l4Port_t portRangeEntry;
	
	PROC_PRINTF("------------ ACL PORT RANGE TABLES -------------\n");
	for(i=0; i<16; i++)
	{
		memset(&portRangeEntry,0,sizeof(portRangeEntry));
		portRangeEntry.index=i;
		rtk_acl_portRange_get(&portRangeEntry);
		PROC_PRINTF("\tPORTRANGE[%d] upper:%d lower:%d type:0x%x\n",i,portRangeEntry.upper_bound,portRangeEntry.lower_bound,portRangeEntry.type);
	}

	return RT_ERR_RG_OK;
}

int32 _dump_acl_ipRangeTable(struct seq_file *s)
{
	int i;
	rtk_acl_rangeCheck_ip_t ipRangeEntry;
	
	PROC_PRINTF("------------ ACL IP RANGE TABLES -------------\n");
	for(i=0; i<8; i++)
	{
		memset(&ipRangeEntry,0,sizeof(ipRangeEntry));
		ipRangeEntry.index=i;
		rtk_acl_ipRange_get(&ipRangeEntry);
		PROC_PRINTF("\tIPRANGE[%d] upper:0x%x lower:0x%x type:0x%x\n",i,ipRangeEntry.upperIp,ipRangeEntry.lowerIp,ipRangeEntry.type);
	}
	return RT_ERR_RG_OK;
}




char *name_of_acl_field[]={
	"",
	"DMAC0[15:0]", // 1
	"DMAC1[31:16]",
	"DMAC2[47:32]",
	"SMAC0[15:0]",
	"SMAC1[31:16]",
	"SMAC2[47:32]",
	"ETHERTYPE", //7
	"STAG",
	"CTAG",
	"GEMIDX/LLIDX", //0xa
	"",	"",	"",	"",	"", //0xb~0xf
	"IP4SIP[15:0]", //0x10
	"IP4SIP[31:16]",
	"IP4DIP[15:0]", 
	"IP4DIP[31:16]",
	"","","","","","","","","","","","",//0x14~0x1f
	"IP6SIP[15:0]", //0x20
	"IP6SIP[31:16]",
	"","","","","","", //0x22~0x27
	"IP6DIP[15:0]", //0x28
	"IP6DIP[31:16]", //0x29
	"IP6NH", //0x2a
	"","","","","",//0x2b~0x2f
	"VIDRANGE",//x0x30
	"IPRANGE",
	"PORTRANGE",
	"PKTLENRANGE",
	"FIELD_VALID",
	"EXT_PORT_MASK",
	"","","","","","","","","","",//0x36~0x3f
	"FIELD_SEL0",//0x40
	"FIELD_SEL1", 
	"FIELD_SEL2", 
	"FIELD_SEL3", 
	"FIELD_SEL4", 
	"FIELD_SEL5", 
	"FIELD_SEL6", 
	"FIELD_SEL7", 
	"FIELD_SEL8", 
	"FIELD_SEL9", 
	"FIELD_SEL10", 
	"FIELD_SEL11", 
	"FIELD_SEL12", 
	"FIELD_SEL13", 	
	"FIELD_SEL14", 	
	"FIELD_SEL15"	
};


char *name_of_cact_idx[]={
	"Ingress CVLAN action",
	"Egress CVLAN action",
	"Using SVID",
	"Policing",
	"Logging",
	"1P remark"
};

char *name_of_sact_idx[]={
	"Ingress SVLAN action",
	"Egress SVLAN action",
	"Using CVID",
	"Policing",
	"Logging",
	"1P remark",
	"DSCP remark",
	"Policy route"
};

char *name_of_policyact_idx[]={
	"Policing",
	"Logging",
};


char *name_of_fwdact_idx[]={
	"Copy frame with ACLPMSK",
	"Redirect frame with ACLPMSK",
	"Ingress mirror to ACLPMSK",
	"Trap to ACL Trap port"
};

char *name_of_priact_idx[]={
	"ACL Priority",
	"DSCP Remarking",
	"1P Remarking",
	"Policing",
	"Logging",
	"Policy Route"
};

char *name_of_cfact_idx[]={
	"None",
	"Stream ID assign",
	"LLID",
	"EXT Fwd Pmask"
};

int32 _dump_acl_template(struct seq_file *s)
{
    int i;
    rtk_acl_template_t aclTemplate;

#ifdef CONFIG_APOLLO_RLE0371
#else
	int j;
    uint32 val;
    PROC_PRINTF("\n----ACL TEMPLATE(ASIC VALUE by ASIC API)-----\n");
	j=0;val=0;
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			for(i=0; i<4; i++)
		    {
		    	PROC_PRINTF("    TEMPLATE[%d]:\n",i);
		        for(j=0; j<8; j++)
		        {
				reg_array_field_read(APOLLOMP_ACL_TEMPLATE_CTRLr, i,j, APOLLOMP_FIELDf, &val);
					//PROC_PRINTF("[%d:0x%x]",j,val&0x7f);
					PROC_PRINTF("\t{%d:0x%02x:%s}\n",j,val&0x7f,name_of_acl_field[val&0x7f]);
		        }
				PROC_PRINTF("\n");
		    }
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif
    PROC_PRINTF("--------------- ACL TEMPLATES(API VALUE) ----------------\n");
    for(i=0; i<4; i++)
    {
        memset(&aclTemplate,0,sizeof(aclTemplate));
        aclTemplate.index=i;
        rtk_acl_template_get(&aclTemplate);
        PROC_PRINTF("\tTEMPLATE[%d]: [0:0x%x][1:0x%x][2:0x%x][3:0x%x][4:0x%x][5:0x%x][6:0x%x][7:0x%x]\n",i,aclTemplate.fieldType[0],aclTemplate.fieldType[1],aclTemplate.fieldType[2],aclTemplate.fieldType[3],aclTemplate.fieldType[4],aclTemplate.fieldType[5],aclTemplate.fieldType[6],aclTemplate.fieldType[7]);
    }
	
	return RT_ERR_RG_OK;
}


int32 _dump_acl(struct seq_file *s)
{
    int i,j;
    uint32 val;
    rtk_acl_ingress_entry_t aclRule;
	char* actionString;
	
	i=0;j=0;val=0;

    PROC_PRINTF("--------------- ACL TABLES ----------------\n");
    for(i=0; i<64; i++)
    {
        memset(&aclRule,0,sizeof(aclRule));
        aclRule.index=i;
        rtk_acl_igrRuleEntry_get(&aclRule);
        if(aclRule.valid)
        {
        	int tag_care=0;
            PROC_PRINTF("  --- ACL TABLE[%d] ---\n",i);
            PROC_PRINTF("\tvalid:%x\n",aclRule.valid);


			for(j=0; j<8; j++)
			{
				if(aclRule.readField.fieldRaw[j].mask) 
				{
					PROC_PRINTF("\tfield[%d]:0x%04x  mask[%d]:0x%04x\t",j,aclRule.readField.fieldRaw[j].value,j,aclRule.readField.fieldRaw[j].mask);
					reg_array_field_read(APOLLOMP_ACL_TEMPLATE_CTRLr, aclRule.templateIdx,j, APOLLOMP_FIELDf, &val);
					PROC_PRINTF("{0x%02x:%s}\n",val&0x7f,name_of_acl_field[val&0x7f]);
				}				
			}

			PROC_PRINTF("\tactive portmask:0x%x\n",aclRule.activePorts.bits[0]);

			if(aclRule.careTag.tags[6].mask) tag_care|=(1<<6);
			if(aclRule.careTag.tags[5].mask) tag_care|=(1<<5);
			if(aclRule.careTag.tags[4].mask) tag_care|=(1<<4);
			if(aclRule.careTag.tags[3].mask) tag_care|=(1<<3);
			if(aclRule.careTag.tags[2].mask) tag_care|=(1<<2);
			if(aclRule.careTag.tags[1].mask) tag_care|=(1<<1);
			if(aclRule.careTag.tags[0].mask) tag_care|=(1<<0);
			if(tag_care)
			{
				PROC_PRINTF("\ttag_care:");
				if(tag_care&(1<<6))PROC_PRINTF("%s",aclRule.careTag.tags[6].value?"[UDP:O]":"[UDP:X]");
				if(tag_care&(1<<5))PROC_PRINTF("%s",aclRule.careTag.tags[5].value?"[TCP:O]":"[TCP:X]");
				if(tag_care&(1<<4))PROC_PRINTF("%s",aclRule.careTag.tags[4].value?"[IPv6:O]":"[IPv6:X]");
				if(tag_care&(1<<3))PROC_PRINTF("%s",aclRule.careTag.tags[3].value?"[IPv4:O]":"[IPv4:X]");
				if(tag_care&(1<<2))PROC_PRINTF("%s",aclRule.careTag.tags[2].value?"[Stag:O]":"[Stag:X]");
				if(tag_care&(1<<1))PROC_PRINTF("%s",aclRule.careTag.tags[1].value?"[Ctag:O]":"[Ctag:X]");
				if(tag_care&(1<<0))PROC_PRINTF("%s",aclRule.careTag.tags[0].value?"[PPPoE:O]":"[PPPoE:X]");
				PROC_PRINTF("\n");
			}
			
            PROC_PRINTF("\ttemplateIdx:%x\n",aclRule.templateIdx);

#if 1
			{
				PROC_PRINTF("\taction bits:");

				//if(aclRule.act.enableAct[6]) PROC_PRINTF("[POLICY ROUTE]");
				if(aclRule.act.enableAct[5]) PROC_PRINTF("[INT/CF]");
				if(aclRule.act.enableAct[4]) PROC_PRINTF("[FWD]");
				if(aclRule.act.enableAct[3]) PROC_PRINTF("[POLICY/LOG]");
				if(aclRule.act.enableAct[2]) PROC_PRINTF("[PRI]");
				if(aclRule.act.enableAct[1]) PROC_PRINTF("[SVLAN]");
				if(aclRule.act.enableAct[0]) PROC_PRINTF("[CVLAN]");
				PROC_PRINTF("\n");

				if(aclRule.act.enableAct[0])
				{
					switch(aclRule.act.cvlanAct.act)
					{
						case ACL_IGR_CVLAN_IGR_CVLAN_ACT: actionString="Ingress CVLAN action"; break;
					    case ACL_IGR_CVLAN_EGR_CVLAN_ACT: actionString="Egress CVLAN action";break;
					    case ACL_IGR_CVLAN_DS_SVID_ACT: actionString="Using SVID";break;
					    case ACL_IGR_CVLAN_POLICING_ACT: actionString="Policing";break;
					    case ACL_IGR_CVLAN_1P_REMARK_ACT: actionString="1P remark";break;
						case ACL_IGR_CVLAN_BW_METER_ACT: actionString="Bandwidth Metering"; break;    
					   	default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[CVLAN_ACTIDX:%x(%s)] cvid:%d dot1p:%x\n",aclRule.act.cvlanAct.act,actionString,aclRule.act.cvlanAct.cvid,aclRule.act.cvlanAct.dot1p);
				}

				if(aclRule.act.enableAct[1])
				{
					switch(aclRule.act.svlanAct.act)
					{
						case ACL_IGR_SVLAN_IGR_SVLAN_ACT: actionString="Ingress SVLAN action"; break;
						case ACL_IGR_SVLAN_EGR_SVLAN_ACT:  actionString="Egress SVLAN action"; break;
						case ACL_IGR_SVLAN_US_CVID_ACT:  actionString="Using CVID"; break;
						case ACL_IGR_SVLAN_POLICING_ACT:  actionString="Policing"; break;
						case ACL_IGR_SVLAN_1P_REMARK_ACT:  actionString="1P remark"; break;
						case ACL_IGR_SVLAN_DSCP_REMARK_ACT:  actionString="DSCP remark"; break;
						case ACL_IGR_SVLAN_ROUTE_ACT:  actionString="policy Route(not support in apolloFE)"; break;
						case ACL_IGR_SVLAN_BW_METER_ACT:  actionString="Bandwidth Metering"; break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[SVLAN_ACTIDX:%x(%s)] svid:%d dot1p:%x dscp:%d nexthop:%x\n",aclRule.act.svlanAct.act,actionString,aclRule.act.svlanAct.svid,aclRule.act.svlanAct.dot1p,aclRule.act.svlanAct.dscp,aclRule.act.svlanAct.nexthop);
				}

				if(aclRule.act.enableAct[4])
				{
					switch(aclRule.act.forwardAct.act)
					{
						case ACL_IGR_FORWARD_COPY_ACT: actionString="Forward frame with ACLPMSK only (& filtering)"; break;
						case ACL_IGR_FORWARD_REDIRECT_ACT: actionString="Redirect frame with ACLPMSK"; break;
						case ACL_IGR_FORWARD_IGR_MIRROR_ACT: actionString="Ingress mirror to ACLPMSK"; break;
						case ACL_IGR_FORWARD_TRAP_ACT: actionString="Trap to CPU"; break;
						default: actionString="unKnown action"; break;
					}
				
					PROC_PRINTF("\t[FWD_ACTIDX:%x(%s)] portMask:0x%x\n",aclRule.act.forwardAct.act,actionString,aclRule.act.forwardAct.portMask.bits[0]);
				}

				if(aclRule.act.enableAct[2])
				{	
					switch(aclRule.act.priAct.act)
					{
						case ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT: actionString="ACL Priority";break;
						case ACL_IGR_PRI_DSCP_REMARK_ACT: actionString="DSCP Remarking";break;
						case ACL_IGR_PRI_1P_REMARK_ACT: actionString="1P Remarking";break;
						case ACL_IGR_PRI_POLICING_ACT: actionString="Policing";break;
						case ACL_IGR_PRI_MIB_ACT: actionString="Logging";break;
						case ACL_IGR_PRI_ROUTE_ACT: actionString="policy Route(not support in apolloFE)";break;
						case ACL_IGR_PRI_BW_METER_ACT: actionString="Bandwidth Metering";break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[PRI_ACTIDX:%x(%s)] aclPri:%x dot1p:%x dscp:%d nexthop:%x\n",aclRule.act.priAct.act,actionString,aclRule.act.priAct.aclPri,aclRule.act.priAct.dot1p,aclRule.act.priAct.dscp,aclRule.act.priAct.nexthop);
				}

				if(aclRule.act.enableAct[3])
				{	
					switch(aclRule.act.logAct.act)
					{
					  	case ACL_IGR_LOG_POLICING_ACT: actionString="Policing"; break;
					    case ACL_IGR_LOG_MIB_ACT: actionString="Logging"; break;
						case ACL_IGR_LOG_BW_METER_ACT: actionString="Bandwidth Metering"; break;
						case ACL_IGR_LOG_1P_REMARK_ACT: actionString="1P remark"; break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[POLICY/LOG_ACTIDX:%x(%s)] meteridx:%d\n",aclRule.act.logAct.act,actionString, aclRule.act.logAct.meter);
				}

				if(aclRule.act.enableAct[5])
				{		
					switch(aclRule.act.extendAct.act)
					{
						case ACL_IGR_EXTEND_NONE_ACT: actionString="None"; break;
					    case ACL_IGR_EXTEND_SID_ACT: actionString="Stream ID assign"; break;
					    case ACL_IGR_EXTEND_LLID_ACT: actionString="LLID"; break;
					    case ACL_IGR_EXTEND_EXT_ACT: actionString="Ext Act(not support in apolloFE)"; break;
						case ACL_IGR_EXTEND_1P_REMARK_ACT: actionString="1P Remarking"; break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[INT/CF_ACTIDX:%x(%s)] CFHITLATCH:%x INT:%x index(stream_id or llid):0x%x, pmask:0x%x\n",aclRule.act.extendAct.act,actionString,aclRule.act.aclLatch, aclRule.act.aclInterrupt,aclRule.act.extendAct.index,aclRule.act.extendAct.portMask.bits[0]);
				}

			}

#else

            PROC_PRINTF("\taction bits:");

			if(aclRule.act.enableAct[5]) PROC_PRINTF("[INT/CF]");
			if(aclRule.act.enableAct[4]) PROC_PRINTF("[FWD]");
			if(aclRule.act.enableAct[3]) PROC_PRINTF("[POLICY/LOG]");
			if(aclRule.act.enableAct[2]) PROC_PRINTF("[PRI]");
			if(aclRule.act.enableAct[1]) PROC_PRINTF("[SVLAN]");
			if(aclRule.act.enableAct[0]) PROC_PRINTF("[CVLAN]");
			PROC_PRINTF("\n");

			if(aclRule.act.enableAct[0])
			{
			    PROC_PRINTF("\t[CVLAN_ACTIDX:%x(%s)] cvid:%d dot1p:%x\n",aclRule.act.cvlanAct.act,name_of_cact_idx[aclRule.act.cvlanAct.act],aclRule.act.cvlanAct.cvid,aclRule.act.cvlanAct.dot1p);
			}

			if(aclRule.act.enableAct[1])
			{
#ifdef CONFIG_APOLLO_RLE0371
			    PROC_PRINTF("\t[SVLAN_ACTIDX:%x] svid:%d dot1p:%x dscp:%d \n",aclRule.act.svlanAct.act,aclRule.act.svlanAct.svid,aclRule.act.svlanAct.dot1p,aclRule.act.svlanAct.dscp);
#else
				PROC_PRINTF("\t[SVLAN_ACTIDX:%x(%s)] svid:%d dot1p:%x dscp:%d nexthop:%x\n",aclRule.act.svlanAct.act,name_of_sact_idx[aclRule.act.svlanAct.act],aclRule.act.svlanAct.svid,aclRule.act.svlanAct.dot1p,aclRule.act.svlanAct.dscp,aclRule.act.svlanAct.nexthop);
#endif
			}

			if(aclRule.act.enableAct[4])
			{
	            PROC_PRINTF("\t[FWD_ACTIDX:%x(%s)] portMask:0x%x\n",aclRule.act.forwardAct.act,name_of_fwdact_idx[aclRule.act.forwardAct.act],aclRule.act.forwardAct.portMask.bits[0]);
			}

			if(aclRule.act.enableAct[2])
			{			
#ifdef CONFIG_APOLLO_RLE0371
            	PROC_PRINTF("\t[PRIACT:%x] aclPri:%x dot1p:%x dscp:%d \n",aclRule.act.priAct.act,aclRule.act.priAct.aclPri,aclRule.act.priAct.dot1p,aclRule.act.priAct.dscp);
#else
	        	PROC_PRINTF("\t[PRI_ACTIDX:%x(%s)] aclPri:%x dot1p:%x dscp:%d nexthop:%x\n",aclRule.act.priAct.act,name_of_priact_idx[aclRule.act.priAct.act],aclRule.act.priAct.aclPri,aclRule.act.priAct.dot1p,aclRule.act.priAct.dscp,aclRule.act.priAct.nexthop);
#endif
			}

			if(aclRule.act.enableAct[3])
			{			
	            PROC_PRINTF("\t[POLICY/LOG_ACTIDX:%x(%s)] meteridx:%d\n",aclRule.act.logAct.act,name_of_policyact_idx[aclRule.act.logAct.act], aclRule.act.logAct.meter);
			}

			if(aclRule.act.enableAct[5])
			{			
	            PROC_PRINTF("\t[INT/CF_ACTIDX:%x(%s)] CFHITLATCH:%x INT:%x index(stream_id or llid):0x%x, pmask:0x%x\n",aclRule.act.extendAct.act,name_of_cfact_idx[aclRule.act.extendAct.act],aclRule.act.aclLatch, aclRule.act.aclInterrupt,aclRule.act.extendAct.index,aclRule.act.extendAct.portMask.bits[0]);
			}
#endif

       }
    }


#if 1//def FORCE_PROBE_APOLLOMP
	//use the ASIC API
    PROC_PRINTF("--------------- ACL HIT OINFO----------------\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			for(i=0; i<6; i++)
		    {
		        reg_array_field_read(APOLLOMP_STAT_ACL_REASONr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_ACL_HIT_INFOf, &val);
		        switch(i)
		        {
		        case 0:
		            PROC_PRINTF("[CACT:%s]: hit rule %d\n",val&0x80?"O":"X",val&0x7f);
		            break;
		        case 1:
		            PROC_PRINTF("[SACT:%s]: hit rule %d\n",val&0x80?"O":"X",val&0x7f);
		            break;
		        case 2:
		            PROC_PRINTF("[PRI:%s]: hit rule %d\n",val&0x80?"O":"X",val&0x7f);
		            break;
		        case 3:
		            PROC_PRINTF("[POLICE:%s]: hit rule %d\n",val&0x80?"O":"X",val&0x7f);
		            break;
		        case 4:
		            PROC_PRINTF("[INT:%s]: hit rule %d\n",val&0x80?"O":"X",val&0x7f);
		            break;
		        case 5:
		            PROC_PRINTF("[FWD:%s]: hit rule %d\n",val&0x80?"O":"X",val&0x7f);
		            break;

		        }
		    }
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif  

	return RT_ERR_RG_OK;
}


char *name_of_us_csAct_idx[]={
	"SVLAN NOP",
	"SVLAN ADD TAG AS VS_TPID",
	"SVLAN ADD TAG AS 0x8100",
	"SVLAN DEL TAG",
	"SVLAN TRANSPARENT",
	"SVLAN ADD TAG AS SP2C",
};
char *name_of_us_csVid_idx[]={
    "SVID ASSIGN",
    "SVID FROM 1ST TAG",
    "SVID FROM 2ND TAG",
    "SVID FROM INTERNAL",
};
char *name_of_us_csPri_idx[]={
	"SPRI ASSIGN",
	"SPRI FROM 1ST TAG",
    "SPRI FROM 2ND TAG",
    "SPRI FROM INTERNAL",
};

char *name_of_us_cAct_idx[]={
	"CVLAN NOP",
	"CVLAN ADD TAG",
	"CVLAN ADD TAG BY C2S",
	"CVLAN DEL TAG",
	"CVLAN TRANSPARENT",
};

char *name_of_us_cVid_idx[]={
    "CVID ASSIGN",
    "CVID FROM 1ST TAG",
    "CVID FROM 2ND TAG",
    "CVID FROM INTERNAL",
};
char *name_of_us_cPri_idx[]={
    "CVID ASSIGN",
    "CVID FROM 1ST TAG",
    "CVID FROM 2ND TAG",
    "CVID FROM INTERNAL",
};
char *name_of_us_sidQid_idx[]={
    "SQID NOP",
    "SID ASSIGN",
    "QID ASSIGN",
};


char *name_of_ds_csAct_idx[]={
	"SVLAN NOP",
	"SVLAN ADD TAG AS VS_TPID",
	"SVLAN ADD TAG AS 0x8100",
	"SVLAN DEL TAG",
	"SVLAN TRANSPARENT",
	"SVLAN ADD TAG AS SP2C",
};

char *name_of_ds_csVid_idx[]={
    "SVID NOP",
    "SVID ASSIGN",
    "SVID FROM 1ST TAG",
    "SVID FROM 2ND TAG",
    "SVID FROM LUT(DMAC2CVID)",
};

char *name_of_ds_csPri_idx[]={
	"SPRI NOP",
	"SPRI ASSIGN",
	"SPRI FROM 1ST TAG",
    "SPRI FROM 2ND TAG",
    "SPRI FROM INTERNAL",
};

char *name_of_ds_cAct_idx[]={
	"CVLAN NOP",
	"CVLAN ADD TAG",
	"CVLAN TRANSLATE WITH SP2C",
	"CVLAN DEL",
	"CVLAN TRANSPARENT",
};
char *name_of_ds_cVid_idx[]={
    "CVID ASSIGN",
    "CVID FROM 1ST TAG",
    "CVID FROM 2ND TAG",
    "CVID FROM LUT(DMAC2CVID)",
};
char *name_of_ds_cPri_idx[]={
	"CPRI ASSIGN",
	"CPRI FROM 1ST TAG",
    "CPRI FROM 2ND TAG",
    "CPRI FROM INTERNAL",
};
char *name_of_ds_uni_idx[]={
	"UNI NOP",
	"UNI MASK BY UNIMASK",
	"UNI FORCE FORWARD",
};

int32 _dump_cf(struct seq_file *s)
{
	uint32 val;
	
	int i;
	rtk_dscp_t dscp_entry;
	int dscp=0;
	rtk_classify_cfg_t classifyCfg;
	rtk_classify_rangeCheck_l4Port_t prtRangeEntry;
	rtk_classify_rangeCheck_ip_t ipRangeEntry;
	char *decisionString, *vidDecisionString, *priDecisionString;
	val=0;

	PROC_PRINTF("------------ CF PORT RANGE TABLE -------------\n");
	for(i=0; i<8; i++)
	{
		memset(&prtRangeEntry,0,sizeof(prtRangeEntry));
		prtRangeEntry.index=i;
		rtk_classify_portRange_get(&prtRangeEntry);
		PROC_PRINTF("\tPORTRANGE[%d] upper:%d lower:%d type:0x%x\n",i,prtRangeEntry.upperPort,prtRangeEntry.lowerPort,prtRangeEntry.type);

	}

	PROC_PRINTF("------------ CF IP RANGE TABLE -------------\n");
	for(i=0; i<8; i++)
	{
		memset(&ipRangeEntry,0,sizeof(ipRangeEntry));
		ipRangeEntry.index=i;
		rtk_classify_ipRange_get(&ipRangeEntry);		
		PROC_PRINTF("\tIPRANGE[%d] upper:0x%x lower:0x%x type:0x%x\n",i,ipRangeEntry.upperIp,ipRangeEntry.lowerIp,ipRangeEntry.type);
	}

	PROC_PRINTF("------------ CF DSCP TABLE -------------\n");
	for(i=0; i<8; i++)
	{
		rtk_classify_cfPri2Dscp_get(i,&dscp_entry);
		PROC_PRINTF("\tDSCP[%d]:0x%x\n",i,dscp_entry);
	}

	PROC_PRINTF("------------ CF TABLE -------------\n");
	for(i=0; i<512; i++)
	{
		memset(&classifyCfg,0,sizeof(classifyCfg));
		classifyCfg.index=i;
		rtk_classify_cfgEntry_get(&classifyCfg);
		if(classifyCfg.valid)
		{
			PROC_PRINTF("  ------ CF TABLE[%d] ------\n",i);
			//PROC_PRINTF("\tvalid:%x\n",classifyCfg.valid);


			//parse U/D
			PROC_PRINTF("\tDIRECTION: %s\n",classifyCfg.direction?"DOWNSTREAM":"UPSTREAM");

			//PROC_PRINTF("\tfield[2]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[2],classifyCfg.field.readField.careFieldRaw[2]);
			//PROC_PRINTF("\tfield[1]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[1],classifyCfg.field.readField.careFieldRaw[1]);
			//PROC_PRINTF("\tfield[0]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[0],classifyCfg.field.readField.careFieldRaw[0]);


			if(i<64){//L34 patterns
				//parse PortRange
				if((classifyCfg.field.readField.dataFieldRaw[2]>>12)&0x8)
				{
					PROC_PRINTF("\tPORT: must hit TABLE[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>12)&0x7);
				}
			  

				//parse IpRange
				if((classifyCfg.field.readField.dataFieldRaw[2]>>8)&0x8)
				{
					PROC_PRINTF("\tIP: must hit TABLE[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>8)&0x7);
				}
			
				//parse Acl Hit
				if((classifyCfg.field.readField.dataFieldRaw[2]>>0)&0x80)
				{
					PROC_PRINTF("\tACL: must hit ACL[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>0)&0x7f);
				}
		   
				//parse Wan Interface
				PROC_PRINTF("\tWAN_IF:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>12)&0x7,(classifyCfg.field.readField.careFieldRaw[1]>>12)&0x7);
		  

				//parse IPv6_MC
				if((classifyCfg.field.readField.careFieldRaw[1]>>11)&0x1)
					PROC_PRINTF("\tIPv6_MC:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>11)&0x1)?"Must":"Must Not");
				//parse IPv4_MC
				if( (classifyCfg.field.readField.careFieldRaw[1]>>10)&0x1)
					PROC_PRINTF("\tIPv4_MC:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>10)&0x1)?"Must":"Must Not");
				//parse MLD
				if( (classifyCfg.field.readField.careFieldRaw[1]>>9)&0x1)
					PROC_PRINTF("\tMLD:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>9)&0x1)?"Must":"Must Not");
				//parse IGMP
				if((classifyCfg.field.readField.careFieldRaw[1]>>8)&0x1)
					PROC_PRINTF("\tIGMP:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>8)&0x1)?"Must":"Must Not");
				//parse DEI
				if((classifyCfg.field.readField.careFieldRaw[1]>>7)&0x1){
					PROC_PRINTF("\tDEI:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>7)&0x1,(classifyCfg.field.readField.careFieldRaw[1]>>7)&0x1);
				}
			}else{//L2 patterns
				//parse Ethertype
				if(classifyCfg.field.readField.careFieldRaw[2]){
					 PROC_PRINTF("\tEhtertype:0x%x \t(Mask:0x%x)\n",classifyCfg.field.readField.dataFieldRaw[2],classifyCfg.field.readField.careFieldRaw[2]);

				}
				//parse GemIdx or LLID
				if((classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff){
					 PROC_PRINTF("\tGemIdx/LLID/TOS:0x%x \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>7)&0xff,(classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff);
				}

			}

		 
			//parse VID
			if((((classifyCfg.field.readField.careFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.careFieldRaw[0])>>11 &0x1f)){
				PROC_PRINTF("\tVID:%d \t(Mask:0x%x)\n",(((classifyCfg.field.readField.dataFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.dataFieldRaw[0])>>11 &0x1f),(((classifyCfg.field.readField.careFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.careFieldRaw[0])>>11 &0x1f));
			}

			//parse PRI
			if((classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7){
				PROC_PRINTF("\tPRI:%d \t\t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[0]>>8)&0x7,(classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7);
			}

			//parse INTER_PRI
			if((classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7){
				PROC_PRINTF("\tINTER_PRI:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[0]>>5)&0x7,(classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7);
			}

			//parse STAG, CTAG
			if((classifyCfg.field.readField.careFieldRaw[0]>>4)&0x1)
				PROC_PRINTF("\tSTAG_IF:%s \n",((classifyCfg.field.readField.dataFieldRaw[0]>>4)&0x1)?"Must Stagged":"Must don't have Stag");

			if((classifyCfg.field.readField.careFieldRaw[0]>>3)&0x1)
				PROC_PRINTF("\tCTAG_IF:%s \n",((classifyCfg.field.readField.dataFieldRaw[0]>>3)&0x1)?"Must Ctagged":"Must don't have Ctag");
			
  
			//parse UNI
			if((classifyCfg.field.readField.careFieldRaw[0]>>0)&0x7){
				PROC_PRINTF("\tUNI:%d \t\t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[0]>>0)&0x7,(classifyCfg.field.readField.careFieldRaw[0]>>0)&0x7);
			}

#if 1
			if(classifyCfg.direction) //downstream
			{

				//[SACT]
				switch(classifyCfg.act.dsAct.csAct)
				{
					case CLASSIFY_DS_CSACT_NOP: decisionString="NOP"; break;
					case CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID: decisionString="ADD_TAG_VS_TPID"; break;
					case CLASSIFY_DS_CSACT_ADD_TAG_8100: decisionString="ADD_TAG_8100"; break;
					case CLASSIFY_DS_CSACT_DEL_STAG: decisionString="DEL_STAG"; break;
					case CLASSIFY_DS_CSACT_TRANSPARENT: decisionString="TRANSPARENT"; break;
					case CLASSIFY_DS_CSACT_SP2C: decisionString="SP2C"; break;
					case CLASSIFY_DS_CSACT_ADD_TAG_STAG_TPID: decisionString="ADD_TAG_STAG_TPID"; break;
					case CLASSIFY_DS_CSACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
					case CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID2: decisionString="ADD_TAG_VS_TPID2"; break;
					default:
						decisionString="unKnown decision"; 
						break;
				}
				switch(classifyCfg.act.dsAct.csVidAct)
				{
					case CLASSIFY_DS_VID_ACT_ASSIGN: vidDecisionString="ASSIGN"; break;
					case CLASSIFY_DS_VID_ACT_FROM_1ST_TAG: vidDecisionString="FROM_1ST_TAG"; break;
					case CLASSIFY_DS_VID_ACT_FROM_2ND_TAG: vidDecisionString="FROM_2ND_TAG"; break;
					case CLASSIFY_DS_VID_ACT_FROM_LUT: vidDecisionString="FROM_LUT"; break;
					case CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C: vidDecisionString="FROM_SP2C"; break;
					case CLASSIFY_DS_VID_ACT_NOP: vidDecisionString="NOP"; break;
					default:
						vidDecisionString="unKnown decision"; 
						break;
				}
				switch(classifyCfg.act.dsAct.csPriAct)
				{
					case CLASSIFY_DS_PRI_ACT_ASSIGN: priDecisionString="ASSIGN"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG: priDecisionString="FROM_1ST_TAG"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG: priDecisionString="FROM_2ND_TAG"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_INTERNAL: priDecisionString="FROM_INTERNAL"; break;
					case CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C: priDecisionString="FROM_SP2C"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_DSCP: priDecisionString="FROM_DSCP"; break;
					case CLASSIFY_DS_PRI_ACT_NOP: priDecisionString="NOP"; break;
					default:
						priDecisionString="unKnown decision"; 
						break;
				
				}
				
				if(classifyCfg.act.dsAct.csAct!=CLASSIFY_DS_CSACT_ACTCTRL_DISABLE){
					PROC_PRINTF("\t[SACT]:%s \n\t\tSVID_ACT:%s \tSVID:%d \n\t\tSPRI_ACT:%s \tSPRI:%d \n",
					decisionString,
					vidDecisionString,classifyCfg.act.dsAct.sTagVid,
					priDecisionString,classifyCfg.act.dsAct.sTagPri);
				}


				//[CACT]
				switch(classifyCfg.act.dsAct.cAct)
				{
					case CLASSIFY_DS_CACT_NOP: decisionString="NOP"; break;
					case CLASSIFY_DS_CACT_ADD_CTAG_8100: decisionString="ADD_CTAG_8100"; break;
					case CLASSIFY_DS_CACT_TRANSLATION_SP2C: decisionString="SP2C"; break;
					case CLASSIFY_DS_CACT_DEL_CTAG: decisionString="DEL_CTAG"; break;
					case CLASSIFY_DS_CACT_TRANSPARENT: decisionString="TRANSPARENT"; break;
					case CLASSIFY_DS_CACT_ACTCTRL_DISABLE: decisionString="TRANSPARENT"; break;
					default:
						decisionString="unKnown decision"; 
						break;
				}
				switch(classifyCfg.act.dsAct.cVidAct)
				{
					case CLASSIFY_DS_VID_ACT_ASSIGN: vidDecisionString="ASSIGN"; break;
					case CLASSIFY_DS_VID_ACT_FROM_1ST_TAG: vidDecisionString="FROM_1ST_TAG"; break;
					case CLASSIFY_DS_VID_ACT_FROM_2ND_TAG: vidDecisionString="FROM_2ND_TAG"; break;
					case CLASSIFY_DS_VID_ACT_FROM_LUT: vidDecisionString="FROM_LUT"; break;
					case CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C: vidDecisionString="SP2C"; break;
					case CLASSIFY_DS_VID_ACT_NOP: vidDecisionString="NOP"; break;
					default:
						vidDecisionString="unKnown decision"; 
						break;
				}
				switch(classifyCfg.act.dsAct.cPriAct)
				{
					case CLASSIFY_DS_PRI_ACT_ASSIGN: priDecisionString="ASSIGN"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG: priDecisionString="FROM_1ST_TAG"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG: priDecisionString="FROM_2ND_TAG"; break; 
					case CLASSIFY_DS_PRI_ACT_FROM_INTERNAL: priDecisionString="FROM_INTERNAL"; break;
					case CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C: priDecisionString="SP2C"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_DSCP: priDecisionString="FROM_DSCP"; break;
					case CLASSIFY_DS_PRI_ACT_NOP: priDecisionString="NOP"; break;
					default:
						priDecisionString="unKnown decision"; 
						break;
				}
				
				if(classifyCfg.act.dsAct.cAct!=CLASSIFY_DS_CACT_ACTCTRL_DISABLE)
				{
					PROC_PRINTF("\t[CACT]:%s \n\t\tCVID_ACT:%s \tVID:%d \n\t\tCPRI_ACT:%s \tPRI:%d \n",
					decisionString,
					vidDecisionString,classifyCfg.act.dsAct.cTagVid,
					priDecisionString,classifyCfg.act.dsAct.cTagPri);
				}

				//[CFPRI]
				switch(classifyCfg.act.dsAct.interPriAct)
				{
					case CLASSIFY_CF_PRI_ACT_NOP: decisionString="NOP"; break;
					case CLASSIFY_CF_PRI_ACT_ASSIGN: decisionString="ASSIGN"; break;
					case CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
					default:
						decisionString="unKnown decision"; 
						break;
				}

				if(classifyCfg.act.dsAct.interPriAct!=CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE)
				{
				PROC_PRINTF("\t[CFPRI]:%s \n\t\tCFPRI:%d\n",
				decisionString,
				classifyCfg.act.dsAct.cfPri);
			}
			
			//[DSCP]
			switch(classifyCfg.act.dsAct.dscp)
			{
				case CLASSIFY_DSCP_ACT_DISABLE: decisionString="DISABLE"; break;
				case CLASSIFY_DSCP_ACT_ENABLE: decisionString="ENABLE"; break;
				case CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision"; 
					break;
			}
			
			if(classifyCfg.act.dsAct.dscp!=CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE)
			{
				rtk_classify_cfPri2Dscp_get(classifyCfg.act.usAct.cfPri, &dscp);
				PROC_PRINTF("\t[DSCP]:%s \n\t\tDSCP:%d \n",
				decisionString,
				dscp);
			}
			
			//[UNI]
			switch(classifyCfg.act.dsAct.uniAct)
			{
				case CLASSIFY_DS_UNI_ACT_NOP: decisionString="NOP"; break;
				case CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK: decisionString="MASK_BY_UNIMASK (& filtering)"; break;
				case CLASSIFY_DS_UNI_ACT_FORCE_FORWARD: decisionString="FORCE_FORWARD"; break;
				case CLASSIFY_DS_UNI_ACT_TRAP: decisionString="TRAP"; break;
				case CLASSIFY_DS_UNI_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision"; 
					break;
			}	
			if(classifyCfg.act.dsAct.uniAct!=CLASSIFY_DS_UNI_ACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[UNI]:%s \n\t\tUNI_MASK:0x%x\n",
				decisionString,
				classifyCfg.act.dsAct.uniMask.bits[0]);
			}
		}
		else
		{
			//[SACT]
			switch(classifyCfg.act.usAct.csAct)
			{
				case CLASSIFY_US_CSACT_NOP: decisionString="NOP"; break;
				case CLASSIFY_US_CSACT_ADD_TAG_VS_TPID: decisionString="ADD_TAG_VS_TPID"; break;
				case CLASSIFY_US_CSACT_ADD_TAG_8100: decisionString="ADD_TAG_8100"; break;
				case CLASSIFY_US_CSACT_DEL_STAG: decisionString="DEL_STAG"; break;
				case CLASSIFY_US_CSACT_TRANSPARENT: decisionString="TRANSPARENT"; break;
				case CLASSIFY_US_CSACT_ADD_TAG_STAG_TPID: decisionString="ADD_TAG_STAG_TPID"; break;	  
				case CLASSIFY_US_CSACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				case CLASSIFY_US_CSACT_ADD_TAG_VS_TPID2: decisionString="ADD_TAG_VS_TPID2"; break;
				default:
					decisionString="unKnown decision"; 
					break;
			}
			switch(classifyCfg.act.usAct.csVidAct)
			{
				case CLASSIFY_US_VID_ACT_ASSIGN: vidDecisionString="ASSIGN"; break;
				case CLASSIFY_US_VID_ACT_FROM_1ST_TAG: vidDecisionString="FROM_1ST_TAG"; break;
				case CLASSIFY_US_VID_ACT_FROM_2ND_TAG: vidDecisionString="FROM_2ND_TAG"; break;
				case CLASSIFY_US_VID_ACT_FROM_INTERNAL: vidDecisionString="FROM_INTERNAL"; break;
				case CLASSIFY_US_VID_ACT_NOP: vidDecisionString="NOP"; break;
				default:
					vidDecisionString="unKnown decision"; 
					break;
			}
			switch(classifyCfg.act.usAct.csPriAct)
			{
				case CLASSIFY_US_PRI_ACT_ASSIGN: priDecisionString="ASSIGN"; break;
				case CLASSIFY_US_PRI_ACT_FROM_1ST_TAG: priDecisionString="FROM_1ST_TAG"; break;
				case CLASSIFY_US_PRI_ACT_FROM_2ND_TAG: priDecisionString="FROM_2ND_TAG"; break; 
				case CLASSIFY_US_PRI_ACT_FROM_INTERNAL: priDecisionString="FROM_INTERNAL"; break;
				case CLASSIFY_US_PRI_ACT_FROM_DSCP: priDecisionString="FROM_DSCP"; break;
				case CLASSIFY_US_PRI_ACT_NOP: priDecisionString="NOP"; break;
				default:
					priDecisionString="unKnown decision"; 
					break;
			}

			if(classifyCfg.act.usAct.csAct!=CLASSIFY_US_CSACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[SACT]:%s \n\t\tSVID_ACT:%s	\tSVID:%d \n\t\tSPRI_ACT:%s \tSPRI:%d \n",
				decisionString,
				vidDecisionString,classifyCfg.act.usAct.sTagVid,
				priDecisionString,classifyCfg.act.usAct.sTagPri);
			}

			
			//[CACT]
			switch(classifyCfg.act.usAct.cAct)
			{
				case CLASSIFY_US_CACT_NOP: decisionString="NOP"; break;
				case CLASSIFY_US_CACT_ADD_CTAG_8100: decisionString="ADD_CTAG_8100"; break;
				case CLASSIFY_US_CACT_TRANSLATION_C2S: decisionString="C2S"; break;
				case CLASSIFY_US_CACT_DEL_CTAG: decisionString="DEL_CTAG"; break;
				case CLASSIFY_US_CACT_TRANSPARENT: decisionString="TRANSPARENT"; break;
				case CLASSIFY_US_CACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision"; 
					break;
			}

			switch(classifyCfg.act.usAct.cVidAct)
			{
				case CLASSIFY_US_VID_ACT_ASSIGN: vidDecisionString="ASSIGN"; break;
				case CLASSIFY_US_VID_ACT_FROM_1ST_TAG: vidDecisionString="FROM_1ST_TAG"; break;
				case CLASSIFY_US_VID_ACT_FROM_2ND_TAG: vidDecisionString="FROM_2ND_TAG"; break;
				case CLASSIFY_US_VID_ACT_FROM_INTERNAL: vidDecisionString="FROM_INTERNAL"; break;
				case CLASSIFY_US_VID_ACT_NOP: vidDecisionString="NOP"; break;
				default:
					vidDecisionString="unKnown decision"; 
					break;
			}

			switch(classifyCfg.act.usAct.cPriAct)
			{
				case CLASSIFY_US_PRI_ACT_ASSIGN: priDecisionString="ASSIGN"; break;
				case CLASSIFY_US_PRI_ACT_FROM_1ST_TAG: priDecisionString="FROM_1ST_TAG"; break;
				case CLASSIFY_US_PRI_ACT_FROM_2ND_TAG: priDecisionString="FROM_2ND_TAG"; break;
				case CLASSIFY_US_PRI_ACT_FROM_INTERNAL: priDecisionString="FROM_INTERNAL"; break;
				case CLASSIFY_US_PRI_ACT_FROM_DSCP: priDecisionString="FROM_DSCP"; break;
				case CLASSIFY_US_PRI_ACT_NOP: priDecisionString="NOP"; break;
				default:
					priDecisionString="unKnown decision"; 
					break;
			}

			if(classifyCfg.act.usAct.cAct!=CLASSIFY_US_CACT_ACTCTRL_DISABLE){
				PROC_PRINTF("\t[CACT]:%s \n\t\tCVID_ACT:%s \tVID:%d \n\t\tCPRI_ACT:%s \tPRI:%d \n",
				decisionString,
				vidDecisionString,classifyCfg.act.usAct.cTagVid,
				priDecisionString,classifyCfg.act.usAct.cTagPri);
			}

			//[CFACT]
			switch(classifyCfg.act.usAct.interPriAct)
			{
				case CLASSIFY_CF_PRI_ACT_NOP: decisionString="NOP"; break;
				case CLASSIFY_CF_PRI_ACT_ASSIGN: decisionString="ASSIGN"; break;
				case CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision"; 
					break;
			}
			if(classifyCfg.act.usAct.interPriAct!=CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[CFPRI]:%s \n\t\tCFPRI:%d\n",
					decisionString,
					classifyCfg.act.usAct.cfPri);
			}
			
			//[DSCP]
			switch(classifyCfg.act.usAct.dscp)
			{
				case CLASSIFY_DSCP_ACT_DISABLE: decisionString="DISABLE"; break;
				case CLASSIFY_DSCP_ACT_ENABLE: decisionString="ENABLE"; break;
				case CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision"; 
					break;
			}
			if(classifyCfg.act.usAct.dscp!=CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE)
			{
				rtk_classify_cfPri2Dscp_get(classifyCfg.act.usAct.cfPri, &dscp);
				PROC_PRINTF("\t[DSCP]:%s \tDSCP:%d \n",
				decisionString,
				dscp);
			}

			
			//[DROP]
			switch(classifyCfg.act.usAct.drop)
			{
				case CLASSIFY_DROP_ACT_NONE: decisionString="NONE"; break;
				case CLASSIFY_DROP_ACT_ENABLE: decisionString="DROP"; break;
				case CLASSIFY_DROP_ACT_TRAP: decisionString="TRAP"; break;
				case CLASSIFY_DROP_ACT_DROP_PON: decisionString="DROP_PON"; break;
				case CLASSIFY_DROP_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision"; 
					break; 
			}		
			if(classifyCfg.act.usAct.drop!=CLASSIFY_DROP_ACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[DROP]:%s\n",
				decisionString);
			}
			
			//[SID]
			switch(classifyCfg.act.usAct.sidQidAct)
			{
				case CLASSIFY_US_SQID_ACT_ASSIGN_NOP: decisionString="NOP"; break;
				case CLASSIFY_US_SQID_ACT_ASSIGN_SID: decisionString="ASSIGN_SID"; break;
				case CLASSIFY_US_SQID_ACT_ASSIGN_QID: decisionString="ASSIGN_QID"; break;
				case CLASSIFY_US_SQID_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision"; 
					break;
			}
			if(classifyCfg.act.usAct.sidQidAct!=CLASSIFY_US_SQID_ACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[SID]:%s \n\t\tstream_id=%d \n",
					decisionString,
					classifyCfg.act.usAct.sidQid);
			}
		}

#else
			if(classifyCfg.direction) //downstream
			{
				rtk_classify_cfPri2Dscp_get(classifyCfg.act.dsAct.cfPri, &dscp);

				//[SACT]
				if(classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID || classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_ADD_TAG_8100 ){
					PROC_PRINTF("\t[SACT]:%s \n\t\tSVID_ACT:%s \tSVID:%d \n\t\tSPRI_ACT:%s \tSPRI:%d \n",
					name_of_ds_csAct_idx[classifyCfg.act.dsAct.csAct],
					name_of_ds_csVid_idx[classifyCfg.act.dsAct.csVidAct],classifyCfg.act.dsAct.sTagVid,
					name_of_ds_csPri_idx[classifyCfg.act.dsAct.csPriAct],classifyCfg.act.dsAct.sTagPri);
				}else if (classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_DEL_STAG || classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_TRANSPARENT || classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_SP2C){
					PROC_PRINTF("\t[SACT]:%s \n",name_of_ds_csAct_idx[classifyCfg.act.dsAct.csAct]);

				}

				//[CACT]
				if(classifyCfg.act.dsAct.cAct==CLASSIFY_DS_CACT_ADD_CTAG_8100){
					PROC_PRINTF("\t[CACT]:%s \n\t\tCVID_ACT:%s \tVID:%d \n\t\tCPRI_ACT:%s \tPRI:%d \n",
						name_of_ds_cAct_idx[classifyCfg.act.dsAct.cAct],
						name_of_ds_cVid_idx[classifyCfg.act.dsAct.cVidAct],classifyCfg.act.dsAct.cTagVid,
						name_of_ds_cPri_idx[classifyCfg.act.dsAct.cPriAct],classifyCfg.act.dsAct.cTagPri);
				}else if(classifyCfg.act.dsAct.cAct==CLASSIFY_DS_CACT_TRANSLATION_SP2C || classifyCfg.act.dsAct.cAct==CLASSIFY_DS_CACT_DEL_CTAG || CLASSIFY_DS_CACT_TRANSPARENT){
					PROC_PRINTF("\t[CACT]:%s\n",name_of_ds_cAct_idx[classifyCfg.act.dsAct.cAct]);
				}

				//[CFPRI]
				if(classifyCfg.act.dsAct.interPriAct==CLASSIFY_CF_PRI_ACT_ASSIGN)
					PROC_PRINTF("\t[CFPRI]:CFPRI ASSIGN \n\t\tCFPRI:%d\n",classifyCfg.act.dsAct.cfPri);
				
				//[DSCP]
				if(classifyCfg.act.dsAct.dscp==CLASSIFY_DSCP_ACT_ENABLE)
					PROC_PRINTF("\t[DSCP]:DSCP ASSIGN \n\t\tDSCP:%d \n",dscp);

				//[UNI]
				if(classifyCfg.act.dsAct.uniAct!=CLASSIFY_DS_UNI_ACT_NOP)
					PROC_PRINTF("\t[UNI]:%s \n\t\tUNI_MASK:0x%x\n",name_of_ds_uni_idx[classifyCfg.act.dsAct.uniAct],classifyCfg.act.dsAct.uniMask.bits[0]);
			}
			else
			{
				rtk_classify_cfPri2Dscp_get(classifyCfg.act.usAct.cfPri, &dscp);

				//[SACT]
				if(classifyCfg.act.usAct.csAct==CLASSIFY_US_CSACT_ADD_TAG_VS_TPID ||classifyCfg.act.usAct.csAct==CLASSIFY_US_CSACT_ADD_TAG_8100){
					PROC_PRINTF("\t[SACT]:%s \n\t\tSVID_ACT:%s	\tSVID:%d \n\t\tSPRI_ACT:%s \tSPRI:%d \n",
					name_of_us_csAct_idx[classifyCfg.act.usAct.csAct],
					name_of_us_csVid_idx[classifyCfg.act.usAct.csVidAct],classifyCfg.act.usAct.sTagVid,
					name_of_us_csPri_idx[classifyCfg.act.usAct.csPriAct],classifyCfg.act.usAct.sTagPri);
				}else if (classifyCfg.act.usAct.csAct==CLASSIFY_US_CSACT_DEL_STAG || classifyCfg.act.usAct.csAct==CLASSIFY_US_CSACT_TRANSPARENT){
					PROC_PRINTF("\t[SACT]:%s \n",name_of_us_csAct_idx[classifyCfg.act.usAct.csAct]);
				}

				//[CACT]
				if(classifyCfg.act.usAct.cAct==CLASSIFY_US_CACT_ADD_CTAG_8100){
					PROC_PRINTF("\t[CACT]:%s \n\t\tCVID_ACT:%s \tVID:%d \n\t\tCPRI_ACT:%s \tPRI:%d \n",
						name_of_us_cAct_idx[classifyCfg.act.usAct.cAct],
						name_of_us_cVid_idx[classifyCfg.act.usAct.cVidAct],classifyCfg.act.usAct.cTagVid,
						name_of_us_cPri_idx[classifyCfg.act.usAct.cPriAct],classifyCfg.act.usAct.cTagPri);
				}else if(classifyCfg.act.usAct.cAct==CLASSIFY_US_CACT_TRANSLATION_C2S||classifyCfg.act.usAct.cAct==CLASSIFY_US_CACT_DEL_CTAG||classifyCfg.act.usAct.cAct==CLASSIFY_US_CACT_TRANSPARENT){
					PROC_PRINTF("\t[CACT]:%s\n",name_of_us_cAct_idx[classifyCfg.act.usAct.cAct]);
				}

				//[CFACT]
				if(classifyCfg.act.usAct.interPriAct==CLASSIFY_CF_PRI_ACT_ASSIGN)
					PROC_PRINTF("\t[CFPRI]:CFPRI ASSIGN \n\t\tCFPRI:%d\n",classifyCfg.act.usAct.cfPri);

				//[DSCP]
				if(classifyCfg.act.usAct.dscp==CLASSIFY_DSCP_ACT_ENABLE)
					PROC_PRINTF("\t[DSCP]:DSCP ASSIGN \tDSCP:%d \n",dscp);

				//[DROP]
				if(classifyCfg.act.usAct.drop==CLASSIFY_DROP_ACT_ENABLE)
					PROC_PRINTF("\t[DROP]\n");

				//[DROP]
				if(classifyCfg.act.usAct.sidQidAct==CLASSIFY_US_SQID_ACT_ASSIGN_SID || classifyCfg.act.usAct.sidQidAct==CLASSIFY_US_SQID_ACT_ASSIGN_QID)
					PROC_PRINTF("\t[SID]:%s \n\t\tSidQid=%d \n",name_of_us_sidQid_idx[classifyCfg.act.usAct.sidQidAct],classifyCfg.act.usAct.sidQid);

				//[LOG]
				if(classifyCfg.act.usAct.log==CLASSIFY_US_LOG_ACT_ENABLE)
					PROC_PRINTF("\t[LOG] \tConterIdx:%d\n",classifyCfg.act.usAct.logCntIdx);
			}
#endif
		}
	}

#if 1
//#ifdef FORCE_PROBE_APOLLOMP

	PROC_PRINTF("--------------- CF HIT INFO----------------\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			for(i=0; i<2; i++)
			{
				reg_array_field_read(APOLLOMP_STAT_CF_REASONr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_CF_HIT_INFOf, &val);
				switch(i)
				{
				case 0:
					if(val&0x200)
						{PROC_PRINTF("[RULE(64-511):%s]: hit rule %d\n",val&0x200?"O":"X",val&0x1ff);}
					else
						{PROC_PRINTF("[RULE(64-511):%s]: unhit\n",val&0x200?"O":"X");}
					break;
				case 1:
					if(val&0x200)
						{PROC_PRINTF("[RULE(0-63):%s]: hit rule %d\n",val&0x200?"O":"X",val&0x1ff);}
					else
						{PROC_PRINTF("[RULE(0-63):%s]: unhit\n",val&0x200?"O":"X");}
					break;
				}
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif

	return RT_ERR_RG_OK;
}


static char tmpStr[255];
int _dump_rg_acl_and_cf_diagshell(struct seq_file *s)
{
#if 1
	int i;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//rtk_rg_acl_filter_fields_e check 1st
		if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields!=0x0){ 	//no more rules need to check			
		uint8	setFlag=0;
		memset(tmpStr,0,255);

	PROC_PRINTF("rg clear acl-filter\n");
				if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight!=0)	//not shown while zero
	PROC_PRINTF("rg set acl-filter acl_weight %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight);

	PROC_PRINTF("rg set acl-filter fwding_type_and_direction %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction);

	//action
	PROC_PRINTF("rg set acl-filter action action_type %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type);		
				if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	PROC_PRINTF("rg set acl-filter action policy-route egress_intf_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_policy_route_wan);

	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_QOS)
	{
					
	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)!=0)	
	{
		PROC_PRINTF("rg set acl-filter action qos action_acl_priority %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_priority);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_1P_REMARKING_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_dot1p_remarking_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_dot1p_remarking_pri);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_DSCP_REMARKING_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_dscp_remarking_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_dscp_remarking_pri);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_ip_precedence_remarking_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_ip_precedence_remarking_pri);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_QUEUE_ID_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_queue_id %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_queue_id);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_egress_internal_priority %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_internal_priority);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_STREAM_ID_OR_LLID_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_stream_id %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_stream_id_or_llid);
	}
					
	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_INGRESS_VID_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_ingress_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_ingress_vid);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_REDIRECT_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_redirect 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.redirect_portmask);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_DS_UNIMASK_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_downstream_uni_portmask portmask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.downstream_uni_portmask);
	}
	
	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_SHARE_METER_BIT)!=0)
	{
		PROC_PRINTF("rg set acl-filter action qos action_share_meter %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_share_meter);
	}

	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_CVLANTAG_BIT)!=0)
	{
		switch(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanTagIfDecision){
		case ACL_CVLAN_TAGIF_TAGGING:
			sprintf(tmpStr,"tagging cvidDecision %d cpriDecision %d cvid %d cpri %d",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanCvidDecision,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanCpriDecision,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.assignedCvid,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.assignedCpri);
			setFlag = 1;
			break;
		case ACL_CVLAN_TAGIF_TAGGING_WITH_C2S:		//upstream only
			sprintf(tmpStr,"tagWithC2S");
			setFlag = 1;
			break;
		case ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C: 	//downstream only
			sprintf(tmpStr,"tagWithSP2C");
			setFlag = 1;
			break;
		case ACL_CVLAN_TAGIF_UNTAG:
			sprintf(tmpStr,"untag");
			setFlag = 1;
			break;
		case ACL_CVLAN_TAGIF_TRANSPARENT:
			sprintf(tmpStr,"transparent");
			setFlag = 1;
			break;
		case ACL_CVLAN_TAGIF_NOP:
		default:
			setFlag = 0;
			break;
		}
		if(setFlag == 1)
		{
			PROC_PRINTF("rg set acl-filter action qos action_ctag %s\n",tmpStr);
		}
	}
	
	if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_SVLANTAG_BIT)!=0){
		switch(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanTagIfDecision){
		case ACL_SVLAN_TAGIF_TAGGING_WITH_8100:
			sprintf(tmpStr,"tagWith8100 svidDecision %d spriDecision %d svid %d spri %d",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanSvidDecision,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanSpriDecision,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.assignedSvid,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.assignedSpri);
			setFlag = 1;
			break;
		case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID:		//upstream only
			sprintf(tmpStr,"tagWithTPID %d spriDecision %d svid %d spri %d",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanSvidDecision,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanSpriDecision,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.assignedSvid,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.assignedSpri);
			setFlag = 1;
			break;
		case ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C: 	//downstream only
			sprintf(tmpStr,"tagWithSP2C");
			setFlag = 1;
			break;
		case ACL_SVLAN_TAGIF_UNTAG:
			sprintf(tmpStr,"untag");
			setFlag = 1;
			break;
		case ACL_SVLAN_TAGIF_TRANSPARENT:
			sprintf(tmpStr,"transparent");
			setFlag = 1;
			break;
		case ACL_SVLAN_TAGIF_NOP:
		default:
			setFlag = 0;
			break;
		}				

		if(setFlag == 1)
		{
			PROC_PRINTF("rg set acl-filter action qos action_stag %s\n",tmpStr);
			setFlag = 0;
		}
	}
}
		//pattern: egress
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_CTAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_pri);
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_CTAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_vid);
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_vid_mask %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_vid_mask);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IPV4_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dest_ipv4_addr_start %d.%d.%d.%d engress_dest_ipv4_addr_end %d.%d.%d.%d\n",
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff),
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff));
		}		
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_INTF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_intf_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_intf_idx);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_EGRESS_PORTIDX_BIT)!=0)
		{
			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx_mask);
			}
			else if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
			{
				PROC_PRINTF("rg set acl-filter pattern ingress_port_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern ingress_port_idx_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_idx_mask);
			}
		}
	
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_src_ipv4_addr_start %d.%d.%d.%d egress_src_ipv4_addr_end %d.%d.%d.%d\n",
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff),
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff));
		}

		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_L4_SPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_src_l4_port_start %d egress_src_l4_port_end %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_l4_port_start,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_l4_port_end);
		}		
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_L4_DPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dest_l4_port_start %d egress_dest_l4_port_end %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_l4_port_start,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_l4_port_end);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IP4MC_IF)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ip4mc_if %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ip4mc_if);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IP6MC_IF)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ip6mc_if %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ip6mc_if);
		}

		
		//pattern: ingress
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_TAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv4_tagif %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv4_tagif);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_TAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_tagif %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_tagif);
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_pri);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_vid);
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_CFI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_cfi %d set\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_cfi);
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv4_addr_start %d.%d.%d.%d ingress_dest_ipv4_addr_end %d.%d.%d.%d\n",
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff),
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff));
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DIP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[15]);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr_start %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ingress_dest_ipv6_addr_end %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[15],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[15]);
			}
					
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DIP_BIT)!=0)	//mask for IPv6; all zero = all "1"(all mask)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr_mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[15]);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dmac %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[5]);
		}
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dmac_mask %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[5]);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DSCP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dscp %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dscp);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_ETHERTYPE_BIT)!=0){
			PROC_PRINTF("rg set acl-filter pattern ingress_ethertype 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ethertype);
			PROC_PRINTF("rg set acl-filter pattern ingress_ethertype_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ethertype_mask);
		}
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_INTF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_intf_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_intf_idx);
		}
	
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DSCP_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_dscp %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_dscp);
				
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_UDP_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 0\n");

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_TCP_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 1\n");
			
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_ICMP_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 2\n");
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal_value 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_l4_protocal);
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_PORT_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_port_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_mask.portmask);
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_SMAC_BIT)!=0){
PROC_PRINTF("rg set acl-filter pattern ingress_smac %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[5]);
			
PROC_PRINTF("rg set acl-filter pattern ingress_smac_mask %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[5]);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv4_addr_start %d.%d.%d.%d ingress_src_ipv4_addr_end %d.%d.%d.%d\n",
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff),
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff));
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_SIP_BIT)!=0){
PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[15]); 
			
PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr_mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[15]);
				}
				
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr_start %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ingress_src_ipv6_addr_end %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[5],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[6],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[7],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[8],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[9],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[10],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[11],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[12],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[13],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[14],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[15],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[5],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[6],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[7],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[8],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[9],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[10],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[11],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[12],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[13],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[14],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[15]);

		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_SPORT_RANGE_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_src_l4_port_start %d ingress_src_l4_port_end %d\n",
		rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_l4_port_start,
		rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_l4_port_end);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_DPORT_RANGE_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_dest_l4_port_start %d ingress_dest_l4_port_end %d\n",
		rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_l4_port_start,
		rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_l4_port_end);

		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAGIF_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_stagIf %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stagIf);
		
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAGIF_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_ctagIf %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctagIf);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_PRI_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_stag_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_pri);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_VID_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_stag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_vid);
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_DEI_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_stag_dei %d set\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_dei);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STREAM_ID_BIT)!=0){
PROC_PRINTF("rg set acl-filter pattern ingress_stream_id %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stream_id);		
PROC_PRINTF("rg set acl-filter pattern ingress_stream_id_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stream_id_mask);
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_WLANDEV_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern ingress_wlanDevMask %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_wlanDevMask);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INTERNAL_PRI_BIT)!=0)
PROC_PRINTF("rg set acl-filter pattern internal_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.internal_pri);

PROC_PRINTF("rg add acl-filter entry\n");
		}
	}
	PROC_PRINTF("#add classify-filter\n");	//action(*), entry and pattern(*)
	for(i=0;i<TOTAL_CF_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields!=0 || 
			rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field!=0 ||
			rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field!=0
			 ){
		uint8	setFlag=0;

		memset(tmpStr,0,255);
PROC_PRINTF("rg clear classify-filter\n");
		
		if(rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field!=0x0){
			
			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_STAG_BIT)!=0){
				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanTagIfDecision)
				{
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: 
						sprintf(tmpStr,"tagging tagWithTPID svidDecision %d spriDecision %d svid %d spri %d",
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSvidDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSpriDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSpri);
						setFlag = 1;
						break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_8100:
						sprintf(tmpStr,"tagWith8100 svidDecision %d spriDecision %d svid %d spri %d",
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSvidDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSpriDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSpri);
						setFlag = 1;
						break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C:
						sprintf(tmpStr,"tagWithSP2C");
						setFlag = 1;
						break; //downstream only
					case ACL_SVLAN_TAGIF_UNTAG:
						sprintf(tmpStr,"unTag");
						setFlag = 1;
						break;
					case ACL_SVLAN_TAGIF_TRANSPARENT: 
						sprintf(tmpStr,"transparent");
						setFlag = 1;
						break;
					case ACL_SVLAN_TAGIF_NOP:
					default:
						setFlag = 0;
							break;
				}
				if(setFlag == 1)
PROC_PRINTF("rg set classify-filter action direction downstream action_stag %s\n",tmpStr);
			}
			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_CTAG_BIT)!=0){
				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanTagIfDecision)
				{
					case ACL_CVLAN_TAGIF_TAGGING:
						sprintf(tmpStr,"tagging cvidDecision %d cpriDecision %d cvid %d cpri %d",
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCvidDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCpriDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCpri);
						setFlag = 1;
						break;
					case ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C:
						sprintf(tmpStr,"tagWithSP2C");
						break; //downstream only
					case ACL_CVLAN_TAGIF_UNTAG:
						sprintf(tmpStr,"unTag");
						setFlag = 1;
						break;
					case ACL_CVLAN_TAGIF_TRANSPARENT: 
						sprintf(tmpStr,"transparent");
						setFlag = 1;
						break;
					case ACL_CVLAN_TAGIF_NOP:
					default:
						setFlag = 0;
							break;
				}
				if(setFlag == 1)
PROC_PRINTF("rg set classify-filter action direction downstream action_ctag %s\n",tmpStr);
			}
			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_CFPRI_BIT)!=0){
PROC_PRINTF("rg set classify-filter action direction downstream action_cfpri %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.assignedCfPri);
			}
			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_DSCP_BIT)!=0){
PROC_PRINTF("rg set classify-filter action direction downstream action_dscp %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp.assignedDscp);
			}
			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_UNI_MASK_BIT)!=0){ 			
PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision %d portmask 0x%x\n",
			rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision,
			rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.assignedUniPortMask);
			}
			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_DROP_BIT)!=0){
PROC_PRINTF("rg set classify-filter action direction downstream action_drop\n");
			}			
		}
			//upstreaming
		if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field != 0){
			
			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_STAG_BIT)!=0){
switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanTagIfDecision)
				{
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: 
						sprintf(tmpStr,"tagging tagWithTPID svidDecision %d spriDecision %d svid %d spri %d",
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSvidDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSpriDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSpri);
						setFlag = 1;
						break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_8100:
						sprintf(tmpStr,"tagWith8100 svidDecision %d spriDecision %d svid %d spri %d",
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSvidDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSpriDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSpri);
						setFlag = 1;
						break;
					case ACL_SVLAN_TAGIF_UNTAG:
						sprintf(tmpStr,"unTag");
						setFlag = 1;
						break;					
					case ACL_SVLAN_TAGIF_TRANSPARENT: 
						sprintf(tmpStr,"transparent");
						setFlag = 1;
						break;
					case ACL_SVLAN_TAGIF_NOP:
					default:
						setFlag = 0;
							break;
				}
				if(setFlag == 1)
PROC_PRINTF("rg set classify-filter action direction upstream action_stag %s\n",tmpStr);
			}
			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_CTAG_BIT)!=0){
				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanTagIfDecision)
				{
					case ACL_CVLAN_TAGIF_TAGGING:
						sprintf(tmpStr,"tagging cvidDecision %d cpriDecision %d cvid %d cpri %d",
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCvidDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCpriDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCpri);
						setFlag = 1;
						break;
					case ACL_CVLAN_TAGIF_TAGGING_WITH_C2S:
						sprintf(tmpStr,"tagWithC2S");
						break; //downstream only
					case ACL_CVLAN_TAGIF_UNTAG:
						sprintf(tmpStr,"unTag");
						setFlag = 1;
						break;
					case ACL_CVLAN_TAGIF_TRANSPARENT: 
						sprintf(tmpStr,"transparent");
						setFlag = 1;
						break;
					case ACL_CVLAN_TAGIF_NOP:
					default:
						setFlag = 0;
							break;
				}
				if(setFlag == 1)
PROC_PRINTF("rg set classify-filter action direction upstream action_ctag %s\n",tmpStr);
			}
			
			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_DROP_BIT)!=0)
PROC_PRINTF("rg set classify-filter action direction upstream action_drop\n");

			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_CFPRI_BIT)!=0)
PROC_PRINTF("rg set classify-filter action direction upstream action_cfpri %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.assignedCfPri);

			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_DSCP_BIT)!=0)
PROC_PRINTF("rg set classify-filter action direction upstream action_dscp %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp.assignedDscp);

			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_SID_BIT)!=0)
PROC_PRINTF("rg set classify-filter action direction upstream action_sid %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid.assignedSid_or_llid);

			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_LOG_BIT)!=0)
PROC_PRINTF("rg set classify-filter action direction upstream action_log %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_log.assignedCounterIdx);
		}

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_CTAGIF_BIT)!=0)
PROC_PRINTF("rg set classify-filter pattern ctagIf %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].ctagIf);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_ETHERTYPR_BIT)!=0){
PROC_PRINTF("rg set classify-filter pattern etherType 0x%x\n",rg_db.systemGlobal.classify_SW_table_entry[i].etherType);
PROC_PRINTF("rg set classify-filter pattern etherType_mask 0x%x\n",rg_db.systemGlobal.classify_SW_table_entry[i].etherType_mask);
		}

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_GEMIDX_BIT)!=0){
PROC_PRINTF("rg set classify-filter pattern gemidx %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].gemidx);
PROC_PRINTF("rg set classify-filter pattern gemidx_mask 0x%x\n",rg_db.systemGlobal.classify_SW_table_entry[i].gemidx_mask);
		}
		
		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_INTERNALPRI_BIT)!=0)
PROC_PRINTF("rg set classify-filter pattern internalPri %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].internalPri);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_LLID_BIT)!=0)
PROC_PRINTF("rg set classify-filter pattern llid %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].llid);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_TAGPRI_BIT)!=0)
PROC_PRINTF("rg set classify-filter pattern outterTagPri %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].outterTagPri);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_TAGVID_BIT)!=0)
PROC_PRINTF("rg set classify-filter pattern outterTagVid %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].outterTagVid);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_STAGIF_BIT)!=0)
PROC_PRINTF("rg set classify-filter pattern stagIf %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].stagIf);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_UNI_BIT)!=0){
PROC_PRINTF("rg set classify-filter pattern uni %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].uni);
PROC_PRINTF("rg set classify-filter pattern uni_mask 0x%x\n",rg_db.systemGlobal.classify_SW_table_entry[i].uni_mask);
		}
PROC_PRINTF("rg set classify-filter entry %d\n",i);
PROC_PRINTF("rg add classify-filter entry\n");
		}
	}

#endif
return RT_ERR_RG_OK;
}


#if 1 /*(7)external APIs need for this ACl module*/
#endif


#if 1  /*****(8)Apollo patch used APIs *****/
#endif
int32 _rtk_rg_AclEgressPriorityPattern_Check(void){
	//if any egress_ctag_pri pattern rule exist in rg_db, return FAIL.
	int i;

    //check the acl_SW_Entry has been allocate
    if(_rtk_rg_is_aclSWEntry_init())
    {
        if(_rtk_rg_aclSWEntry_init())
            RETURN_ERR(RT_ERR_RG_NULL_POINTER);
    }

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED){
			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_CTAG_PRI_BIT)
				return FAIL;
		}
	}
	return (RT_ERR_RG_OK);
}

#ifdef RTK_RG_INGRESS_QOS_TEST_PATCH
int _rtk_rg_qos_acl_flush(void)
{
	int i,j;
	for(i=0;i<RTK_RG_MAC_PORT_PON;i++)
	{
		for(j=8;j>=0;j--)
		{
			if(rg_db.systemGlobal.qos_acl_patch[i][j])
			{
				DEBUG("DEL ACL index:%d\n",rg_db.systemGlobal.qos_acl_patch[i][j]-1);
				rtk_rg_apollo_aclFilterAndQos_del(rg_db.systemGlobal.qos_acl_patch[i][j]-1);
				rg_db.systemGlobal.qos_acl_patch[i][j]=0;
			}
		}
	}
	rg_db.systemGlobal.qos_acl_total_patch=0;
	return (RT_ERR_RG_OK);
}


int _rtk_rg_qos_acl_patch(rtk_rg_mac_port_idx_t port, uint32 rate)
{
	rg_db.systemGlobal.qos_ingress_total_rate-=rg_db.systemGlobal.qos_ingress_rate[port];
	rg_db.systemGlobal.qos_ingress_rate[port]=rate;
	rg_db.systemGlobal.qos_ingress_total_rate+=rate;

	//If total ingress rate < 1000 Mbps, clear all patch
	DEBUG("[QOS PATCH]Total rate:%d\n",rg_db.systemGlobal.qos_ingress_total_rate);

	if((rg_db.systemGlobal.qos_type==RTK_RG_INGRESS_QOS_ORIGINAL) || (port>=RTK_RG_MAC_PORT_PON))
	{
		//Normal ingress bandwidth control
		DEBUG("[QoS PATCH]Nomal mode.");
		ASSERT_EQ(rtk_rate_portIgrBandwidthCtrlRate_set(port,rate),SUCCESS);
	}
	else if(rg_db.systemGlobal.qos_type==RTK_RG_INGRESS_QOS_ALL_HIGH_QUEUE)
	{	
		//All queue of rate limit port re-map to high queue
		rtk_rg_aclFilterAndQos_t acl_filter;
		int aclIdx;

		DEBUG("[QoS PATCH]Patch 1...");
		if(rg_db.systemGlobal.qos_acl_patch[port][0])
		{
			DEBUG("DEL ACL index:%d\n",rg_db.systemGlobal.qos_acl_patch[port][0]-1);
			ASSERT_EQ(rtk_rg_apollo_aclFilterAndQos_del(rg_db.systemGlobal.qos_acl_patch[port][0]-1),RT_ERR_RG_OK);
			rg_db.systemGlobal.qos_acl_patch[port][0]=0;
			rg_db.systemGlobal.qos_acl_total_patch--;
		}
		if((rate<250000) && (rg_db.systemGlobal.qos_acl_total_patch<3))
		{
			DEBUG("[QoS PATCH 1]...");
			//if(rg_db.systemGlobal.qos_acl_total_patch==4) goto NORMAL_QOS;
			memset(&acl_filter,0,sizeof(rtk_rg_aclFilterAndQos_t));
			acl_filter.filter_fields = INGRESS_PORT_BIT;
			acl_filter.ingress_port_mask.portmask=(0x1<<port);
			acl_filter.action_type=ACL_ACTION_TYPE_QOS;
			acl_filter.qos_actions=ACL_ACTION_QUEUE_ID_BIT;
			//acl_filter.action_queue_id=6-rg_db.systemGlobal.qos_acl_total_patch;
			acl_filter.action_queue_id=3;
			ASSERT_EQ(rtk_rg_apollo_aclFilterAndQos_add(&acl_filter,&aclIdx),RT_ERR_RG_OK);
			rg_db.systemGlobal.qos_acl_patch[port][0]=aclIdx+1; //0 for invalid
			rg_db.systemGlobal.qos_acl_total_patch++;
		}
		ASSERT_EQ(rtk_rate_portIgrBandwidthCtrlRate_set(port,rate),SUCCESS);
	}

	return SUCCESS;
}
#endif


//patch for stag won't do cvlan filter and ttl_minus when cvlan is untag
int32 _rtk_rg_acl_reserved_stag_ingressCVidFromPVID(uint32 in_pvid, uint32 in_port)
{
	rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t parameter;
	bzero(&parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));

	_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER+in_port);
	parameter.service_port = in_port;
	parameter.assigned_vid = in_pvid;
	_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER+in_port, &parameter);

	return (RT_ERR_RG_OK);
}

#ifdef __KERNEL__ 
//model code skips HW patch
#if defined(CONFIG_APOLLO)
int _rtk_rg_acl_reserved_pppoeCvidIssue_svid2IngressCvid(int wan_port)
{
	//if packet comes from Wan port & have stag(0x8100), tanslate SVID to ingress CVID => patch for PPPoE+CVID & link-100Mbps bug!	
	rtk_portmask_t svlanPortmask;
	rtk_portmask_t svlanUntagPortmask;

	rtk_rg_aclAndCf_reserved_pppoeCvidIssueSvidCopy2CvidPatch_t 	pppoeCvidIssueSvid2CvidPatch;
	bzero(&pppoeCvidIssueSvid2CvidPatch,sizeof(pppoeCvidIssueSvid2CvidPatch));
	pppoeCvidIssueSvid2CvidPatch.wanPort=wan_port;
	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_SVIDCOPY2CVID_PATCH, &pppoeCvidIssueSvid2CvidPatch));


	//Create SVID[1], member=all, untag=all
	//svlan set service-port 4
	assert_ok(RTK_SVLAN_SERVICEPORT_SET(wan_port, ENABLED));	
	//svlan set tpid 0x8100
	assert_ok(RTK_SVLAN_TPIDENTRY_SET(0, 0x8100));	
	//svlan set svlan-table svid 1 member all,6
	//svlan set svlan-table svid 1 untag-member all,6
	svlanPortmask.bits[0] = RTK_RG_ALL_MAC_PORTMASK;
	svlanUntagPortmask.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
	assert_ok(RTK_SVLAN_MEMBERPORT_SET(1, &svlanPortmask, &svlanUntagPortmask));	

	//Assign FID to WAN_FID
	assert_ok(rtk_svlan_fidEnable_set(1,ENABLED));
	assert_ok(rtk_svlan_fid_set(1,WAN_FID));

	//Set SVLAN untag action to assign SVLAN ID 1
	assert_ok(rtk_svlan_untagAction_set(SVLAN_ACTION_SVLAN, 1));
		
	return (RT_ERR_RG_OK);
}
#endif
#endif //end of #ifdef __KERNEL__ 

#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)
int _rtk_rg_acl_reserved_pppoeCvidIssue_spriRemap2InternalPri(int wan_port, int spri, int intpri)
{
	rtk_rg_aclAndCf_reserved_pppoeCvidIssueSpriRemap2InternalPriPatch_t pppoeCvidIssueSpriRemap2IntpriPara;	
	bzero(&pppoeCvidIssueSpriRemap2IntpriPara,sizeof(pppoeCvidIssueSpriRemap2IntpriPara));
	pppoeCvidIssueSpriRemap2IntpriPara.wanPort=wan_port;
	pppoeCvidIssueSpriRemap2IntpriPara.spri=spri;
	pppoeCvidIssueSpriRemap2IntpriPara.intpri=intpri;
	if(spri==0){assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI0REMAPTOINTERNALPRI_PATCH, &pppoeCvidIssueSpriRemap2IntpriPara));}
	else if(spri==1){assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI1REMAPTOINTERNALPRI_PATCH, &pppoeCvidIssueSpriRemap2IntpriPara));}
	else if(spri==2){assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI2REMAPTOINTERNALPRI_PATCH, &pppoeCvidIssueSpriRemap2IntpriPara));}
	else if(spri==3){assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI3REMAPTOINTERNALPRI_PATCH, &pppoeCvidIssueSpriRemap2IntpriPara));}
	else if(spri==4){assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI4REMAPTOINTERNALPRI_PATCH, &pppoeCvidIssueSpriRemap2IntpriPara));}
	else if(spri==5){assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI5REMAPTOINTERNALPRI_PATCH, &pppoeCvidIssueSpriRemap2IntpriPara));}
	else if(spri==6){assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI6REMAPTOINTERNALPRI_PATCH, &pppoeCvidIssueSpriRemap2IntpriPara));}
	else if(spri==7){assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PPPoECVIDISSUE_QOSSPRI7REMAPTOINTERNALPRI_PATCH, &pppoeCvidIssueSpriRemap2IntpriPara));}

	return (RT_ERR_RG_OK);
}
#endif

#ifdef CONFIG_DUALBAND_CONCURRENT
/* patch for Slave GMAC packets recvice by special 1Q VID and PRI */
int32 _rtk_rg_acl_reserved_wifi_internalVidPriTranslateForSlave(uint32 in_cvid, uint32 in_cpri, uint32 tran_cvid, uint32 tran_pri)
{
	rtk_rg_aclAndCf_reserved_WifiDatapathMaster2SlavePatch_t wifiDadapathM2SPatchPara;
	bzero(&wifiDadapathM2SPatchPara,sizeof(wifiDadapathM2SPatchPara));
	wifiDadapathM2SPatchPara.igrCvid=in_cvid;
	wifiDadapathM2SPatchPara.igrCpri=in_cpri;
	wifiDadapathM2SPatchPara.tranCvid=tran_cvid;
	wifiDadapathM2SPatchPara.tranCpri=tran_pri;
 	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_WIFI_DATAPATH_MASTER2SLAVE_PATCH, &wifiDadapathM2SPatchPara));
	
	return (RT_ERR_RG_OK);	
}
#endif


