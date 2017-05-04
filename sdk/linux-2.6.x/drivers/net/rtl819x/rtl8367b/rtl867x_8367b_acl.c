#include <linux/slab.h>
#include <net/rtl/rtl_types.h>

#include "rtl867x_8367b_acl.h"


#include "../AsicDriver/rtl865x_asicCom.h"
#include "../../rtl8367b/rtl8367b_asicdrv_acl_para.h"



int qos8367b_aclrule_uesd_num_test = 0;

static struct list_head chain_list_head;
static int __rtl867x_rtl8367b_sync_acl(void);

void rtl867x_rtl8367b_acl_fieldData_free(rtk_filter_field_t **fieldDataHead)
{
	rtk_filter_field_t *field_ptr = *fieldDataHead;
	while(field_ptr)
	{
		rtk_filter_field_t * field_ptr_nxt = field_ptr->next;
		kfree(field_ptr);
		field_ptr = field_ptr_nxt;
	}	

	*fieldDataHead = NULL;
}


/* Note.
	1. This function will 
		(1) copy input_act 	, the caller can free input_act after calling function
		(2) refer to input_cfg	, the caller can NOT free input_cfg after calling function

	TODO : I do not do duplicate check in this fuction 
*/
int rtl867x_rtl8367b_addacl(rtk_filter_cfg_t* input_cfg,rtk_filter_action_t* input_act, int chain_priority)
{
	rtl867x_8367b_acl_chain_t* chain_entry;
	rtl867x_8367b_acl_chain_rule_t* chain_aclrule_entry;

	list_for_each_entry(chain_entry,&chain_list_head,chain_list) {       
        if (chain_entry->priority == chain_priority) 
		{
			chain_aclrule_entry = kmalloc(sizeof(rtl867x_8367b_acl_chain_rule_t),GFP_KERNEL);
			if(!chain_aclrule_entry)
			{
				DBG_8367B_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
				goto FailToAdd;
			}		
		
			/* refer to cfg */
			chain_aclrule_entry->cfg = input_cfg;

			/* memcpy act */
			chain_aclrule_entry->act = kmalloc(sizeof(rtk_filter_action_t),GFP_KERNEL);
			if(!chain_aclrule_entry->act)
			{
				DBG_8367B_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
				goto FailToAdd;
			}		
			memcpy(chain_aclrule_entry->act ,input_act ,sizeof(rtk_filter_action_t));
	
			INIT_LIST_HEAD(&chain_aclrule_entry->aclrule_list);
			list_add_tail(&chain_aclrule_entry->aclrule_list , &chain_entry->aclrule_list_head);
			chain_entry->ruleCnt ++;
			
			if(__rtl867x_rtl8367b_sync_acl()!=SUCCESS)
			{
				DBG_8367B_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
				goto FailToAdd_2;
			}

		}
    }

	return SUCCESS;
	
FailToAdd_2:
	list_del(&chain_aclrule_entry->aclrule_list);

FailToAdd:	
	if(chain_aclrule_entry && chain_aclrule_entry->act)
		kfree(chain_aclrule_entry->act);
	if(chain_aclrule_entry)
		kfree(chain_aclrule_entry);
	return FAILED;

}



int rtl867x_rtl8367b_flushacl(int chain_priority)
{
	rtl867x_8367b_acl_chain_t *chain_entry,*chain_nxt;
	
	
	list_for_each_entry_safe(chain_entry, chain_nxt,&chain_list_head,chain_list) {		 
		if (chain_entry->priority == chain_priority) 
		{
			rtl867x_8367b_acl_chain_rule_t *chain_aclrule_entry,*chain_aclrule_nxt;
	
			list_for_each_entry_safe(chain_aclrule_entry, chain_aclrule_nxt,&chain_entry->aclrule_list_head,aclrule_list) { 
				list_del(&chain_aclrule_entry->aclrule_list);

				/* free act, field , cfg */
				kfree(chain_aclrule_entry->act);
				kfree(chain_aclrule_entry); 			
			}
			chain_entry->ruleCnt = 0;
		}
	}

	if(__rtl867x_rtl8367b_sync_acl()!=SUCCESS)
	{
		DBG_8367B_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}
	else
		return SUCCESS;
}


/*  chain priosity: the minimum value is the highest proirity
     if force=1, it means that this chain has to be added into acl.  
     If not ,it is maybe not added to acl when the all chains with force=1 occupy acl entry. */
int rtl867x_rtl8367b_register_aclchain(int priority, int force)
{
	struct list_head *ptr;
	rtl867x_8367b_acl_chain_t *entry;
	rtl867x_8367b_acl_chain_t *acl_chain=NULL;


	
	acl_chain = kmalloc(sizeof(rtl867x_8367b_acl_chain_t),GFP_KERNEL);	
	if(!acl_chain)
	{
		DBG_8367B_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		goto FailToAdd;
	}

	acl_chain->ruleCnt 	= 0;
	acl_chain->priority 	= priority;
	acl_chain->force 	= force;
	INIT_LIST_HEAD(&acl_chain->aclrule_list_head);
	INIT_LIST_HEAD(&acl_chain->chain_list);



	/* dulicate check */

	
	list_for_each(ptr, &chain_list_head) {
        entry = list_entry(ptr, rtl867x_8367b_acl_chain_t, chain_list);
        if (priority == entry->priority) 
		{
			DBG_8367B_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
			goto FailToAdd;
		}
    }

	

	/* find a position to insert into acl chain list */	
	list_for_each(ptr, &chain_list_head) {
        entry = list_entry(ptr, rtl867x_8367b_acl_chain_t, chain_list);
        if (priority < entry->priority ) 
        {
        	list_add_tail(&acl_chain->chain_list, ptr);
			return SUCCESS;
        }		
    }
	list_add_tail(&acl_chain->chain_list, &chain_list_head);
	return SUCCESS;

FailToAdd:	
	if(acl_chain)
		kfree(acl_chain);
	return FAILED;
}

int rtl867x_rtl8367b_unregister_aclchain(int priority)
{
	rtl867x_8367b_acl_chain_t *chain_entry,*chain_nxt;

	list_for_each_entry_safe(chain_entry, chain_nxt,&chain_list_head,chain_list) {       
        if (chain_entry->priority == priority) 
		{
			rtl867x_rtl8367b_flushacl(priority);	
			list_del(&chain_entry->chain_list);
			kfree(chain_entry);	
		}
    }

	if(__rtl867x_rtl8367b_sync_acl()!=SUCCESS)
	{
		DBG_8367B_ACL_PRK("Leave %s @ %d\n",__func__,__LINE__);
		return FAILED;
	}
	else
		return SUCCESS;
}


static int __rtl867x_rtl8367b_sync_acl(void)
{
	uint32 number_force_rule_uplimit	=0;
	uint32 number_other_rule_uplimit	=0;
	uint32 number_force_rule_used		=0;
	uint32 number_other_rule_used		=0;
	uint32 qos8367b_aclrule_uesd_num	=0;
	
	rtl867x_8367b_acl_chain_t* 		chain_entry;
	rtl867x_8367b_acl_chain_rule_t* 	chain_aclrule_entry;
	

	/* flush all hw acl rule*/
	rtk_filter_igrAcl_cfg_delAll();

	/* calculate toal number of acl force rules */
	list_for_each_entry(chain_entry,&chain_list_head,chain_list) 
	{       
        if (chain_entry->force) 
			number_force_rule_uplimit += chain_entry->ruleCnt;	
    }
	if(number_force_rule_uplimit>RL6000_ACLRULENUM)
	{
		printk("Warning! the number of force rule is exceeding the limit !!! (%s ,line %d)\n",__func__,__LINE__);
		number_force_rule_uplimit = RL6000_ACLRULENUM;
	}
	number_other_rule_uplimit = RL6000_ACLRULENUM - number_force_rule_uplimit;

	/* start to add acl rule */
	list_for_each_entry(chain_entry,&chain_list_head,chain_list) 
	{  
		list_for_each_entry(chain_aclrule_entry,&chain_entry->aclrule_list_head,aclrule_list) 
		{ 
			int retVal;
			int ruleNum;
			
			if(chain_entry->force)	
			{
				if(number_force_rule_used==number_force_rule_uplimit)
					break; /* skip this chain */
			}
			else
			{
				if(number_other_rule_used==number_other_rule_uplimit)
					break; /* skip this chain */	
			}	
			
			if ((retVal = rtk_filter_igrAcl_cfg_add(qos8367b_aclrule_uesd_num
				, chain_aclrule_entry->cfg, chain_aclrule_entry->act,&ruleNum)) != RT_ERR_OK)
			{
				DBG_8367B_ACL_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
				return FAILED;
			}
			else
			{			
				qos8367b_aclrule_uesd_num += ruleNum;
				chain_aclrule_entry->asic_used_rulenum = ruleNum;
				DBG_8367B_ACL_PRK("(%s) add RL6000    with  %d   rule  (total : %d)\n",__func__,ruleNum,qos8367b_aclrule_uesd_num);
			}		
			

			if(chain_entry->force)
				number_force_rule_used ++;
			else
				number_other_rule_used ++;
		}
	}

	return SUCCESS;
}






static int __init rtl867x_8367b_acl_init(void)
{
	rtk_api_ret_t retVal;
	#if 0
	rtk_filter_template_t acltemp;
	rtk_field_sel_t 		type;
	rtk_uint32				offset;	
	#endif

	DBG_8367B_ACL_PRK(" Enter %s\n",__func__);

	INIT_LIST_HEAD(&chain_list_head);

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B) 
	{

		/*  Init RL6000 ACL  */
		retVal = rtk_filter_igrAcl_init();
	    if(retVal!=RT_ERR_OK)
	    {
			DBG_8367B_ACL_PRK("Leave %s @ %d rtk_filter_igrAcl_init failed!\n",__func__,__LINE__);	
			goto init_failed;
	    }
		
		#if 0
		/* Configure Field Selector 7 to IPv4 Tos, offset 0 bytes (LSB 8bits) */	
		type = FORMAT_IPV4;
		offset = 0;
		retVal = rtk_filter_igrAcl_field_sel_set(7, type, offset);
		if(retVal!=RT_ERR_OK)
	    {
			DBG_8367B_ACL_PRK("Leave %s @ %d rtk_filter_igrAcl_field_sel_set failed!\n",__func__,__LINE__);	
			goto init_failed;
	    }


		/* Configure Field Selector 8 to IPv4 Protocal_number, offset 8 bytes (LSB 8bits) */	
		type = FORMAT_IPV4;
		offset = 8;
		retVal = rtk_filter_igrAcl_field_sel_set(8, type, offset);
		if(retVal!=RT_ERR_OK)
	    {
			DBG_8367B_ACL_PRK("Leave %s @ %d rtk_filter_igrAcl_field_sel_set failed!\n",__func__,__LINE__);	
			goto init_failed;
	    }

		/* Configure Field Selector 9 to NULL Filed Selector */	
		type = FORMAT_RAW;
		offset = 0;
		retVal = rtk_filter_igrAcl_field_sel_set(9, type, offset);
		if(retVal!=RT_ERR_OK)
	    {
			DBG_8367B_ACL_PRK("Leave %s @ %d rtk_filter_igrAcl_field_sel_set failed!\n",__func__,__LINE__);	
			goto init_failed;
	    }


		/* Setup Template (is is designed to be suitable for 8676 embeded ACL format )  */
		/* Template 0  : Ethernet Format */		
		memset(&acltemp, 0x00, sizeof(rtk_filter_template_t));
		acltemp.index = 0;
		acltemp.fieldType[0] = FILTER_FIELD_RAW_DMAC_15_0;
		acltemp.fieldType[1] = FILTER_FIELD_RAW_DMAC_31_16;
		acltemp.fieldType[2] = FILTER_FIELD_RAW_DMAC_47_32;
		acltemp.fieldType[3] = FILTER_FIELD_RAW_SMAC_15_0;
		acltemp.fieldType[4] = FILTER_FIELD_RAW_SMAC_31_16;
		acltemp.fieldType[5] = FILTER_FIELD_RAW_SMAC_47_32;
		acltemp.fieldType[6] = FILTER_FIELD_RAW_ETHERTYPE;	
		acltemp.fieldType[7] = FILTER_FIELD_RAW_FIELD_SELECT09;	/* NULL*/
		if(rtk_filter_igrAcl_template_set(&acltemp)!=RT_ERR_OK)
		{			
			DBG_8367B_ACL_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			goto init_failed;
		}


		/* Template  1 : IP/TCP/UDP Format */ 
		memset(&acltemp, 0x00, sizeof(rtk_filter_template_t));
		acltemp.index = 1;
		acltemp.fieldType[0] = FILTER_FIELD_RAW_IPV4_SIP_15_0;
		acltemp.fieldType[1] = FILTER_FIELD_RAW_IPV4_SIP_31_16;
		acltemp.fieldType[2] = FILTER_FIELD_RAW_IPV4_DIP_15_0;
		acltemp.fieldType[3] = FILTER_FIELD_RAW_IPV4_DIP_31_16;
		acltemp.fieldType[4] = FILTER_FIELD_RAW_IPRANGE; 		/* 16bits (ip range only 8 bit , use bitmask)*/
		acltemp.fieldType[5] = FILTER_FIELD_RAW_FIELD_SELECT07;	/* IPv4 Tos */
		acltemp.fieldType[6] = FILTER_FIELD_RAW_FIELD_SELECT08;	/* IPv4 Protocal_number */
		acltemp.fieldType[7] = FILTER_FIELD_RAW_PORTRANGE; 	/* 16bits (use bitmask)*/
		if(rtk_filter_igrAcl_template_set(&acltemp)!=RT_ERR_OK)
		{			
			DBG_8367B_ACL_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			goto init_failed;
		}	


		/* Template  2 : 802.1p  */
		memset(&acltemp, 0x00, sizeof(rtk_filter_template_t));
		acltemp.index = 2;
		acltemp.fieldType[0] = FILTER_FIELD_RAW_CTAG;		
		acltemp.fieldType[1] = FILTER_FIELD_RAW_FIELD_SELECT09;	/* NULL*/
		acltemp.fieldType[2] = FILTER_FIELD_RAW_FIELD_SELECT09;	/* NULL*/
		acltemp.fieldType[3] = FILTER_FIELD_RAW_FIELD_SELECT09;	/* NULL*/
		acltemp.fieldType[4] = FILTER_FIELD_RAW_FIELD_SELECT09;	/* NULL*/
		acltemp.fieldType[5] = FILTER_FIELD_RAW_FIELD_SELECT09;	/* NULL*/
		acltemp.fieldType[6] = FILTER_FIELD_RAW_FIELD_SELECT09;	/* NULL*/
		acltemp.fieldType[7] = FILTER_FIELD_RAW_FIELD_SELECT09;	/* NULL*/	
		if(rtk_filter_igrAcl_template_set(&acltemp)!=RT_ERR_OK)
		{			
			DBG_8367B_ACL_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			goto init_failed;
		}
		

		/* Template  3 : source filter  */
		memset(&acltemp, 0x00, sizeof(rtk_filter_template_t));
		acltemp.index = 3;
		acltemp.fieldType[0] = FILTER_FIELD_RAW_SMAC_15_0;
		acltemp.fieldType[1] = FILTER_FIELD_RAW_SMAC_31_16;
		acltemp.fieldType[2] = FILTER_FIELD_RAW_SMAC_47_32;
		acltemp.fieldType[3] = FILTER_FIELD_RAW_CTAG;
		acltemp.fieldType[4] = FILTER_FIELD_RAW_IPV4_SIP_15_0;
		acltemp.fieldType[5] = FILTER_FIELD_RAW_IPV4_SIP_31_16;
		acltemp.fieldType[6] = FILTER_FIELD_RAW_IPRANGE ; 		/* 16bits (ip range only 8 bit , use bitmask)*/
		acltemp.fieldType[7] = FILTER_FIELD_RAW_PORTRANGE 		/* 16bits (use bitmask)*/;		
		if(rtk_filter_igrAcl_template_set(&acltemp)!=RT_ERR_OK)
		{			
			DBG_8367B_ACL_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			goto init_failed;
		}	
		#endif
	}


	return SUCCESS;
	
init_failed:

	return FAILED;	

} 

static void __exit rtl867x_8367b_acl_exit(void)
{

	
} 

static void _rtl867x_rtl8367b_field_display(rtk_filter_field_t* pField)
{
	const char *fieldString[FILTER_FIELD_END] = {
    	"DMAC",
    	"SMAC",
    	"ETHERTYPE",
    	"CTAG",
    	"STAG",
    	"SIP",
    	"DIP",
    	"TOS",
		"PROTOCOL",
		"IPFLAG",
		"IPOFFSET",
		"SIPV6",
		"DIPV6",
		"TRAFFIC_CLASS",
		"NEXT_HEADER",
		"TCP_SPORT",
		"TCP_DPORT",
		"TCP_FLAG",
		"UDP_SPORT",
		"UDP_DPORT",
		"ICMP_CODE",
		"ICMP_TYPE",
		"IGMP_TYPE",
		"VIDRANGE",
		"IPRANGE",
		"PORTRANGE",
		"USER00",
		"USER01",
		"USER02",
		"USER03",
		"USER04",
		"USER05",
		"USER06",
		"USER07",
		"USER08",
		"USER09",
		"USER10",
		"USER11",
		"USER12",
		"USER13",
		"USER14",
		"USER15",
        "PATTERN"
    };

	printk("Field: %s ",fieldString[pField->fieldType]);

	switch(pField->fieldType)
	{
	 case FILTER_FIELD_DMAC:
	 case FILTER_FIELD_SMAC:

		printk("[Data]%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X ",
			pField->filter_pattern_union.mac.value.octet[0],
			pField->filter_pattern_union.mac.value.octet[1],
			pField->filter_pattern_union.mac.value.octet[2],
			pField->filter_pattern_union.mac.value.octet[3],
			pField->filter_pattern_union.mac.value.octet[4],
			pField->filter_pattern_union.mac.value.octet[5]);
		printk("[Mask]%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
			pField->filter_pattern_union.mac.mask.octet[0],
			pField->filter_pattern_union.mac.mask.octet[1],
			pField->filter_pattern_union.mac.mask.octet[2],
			pField->filter_pattern_union.mac.mask.octet[3],
			pField->filter_pattern_union.mac.mask.octet[4],
			pField->filter_pattern_union.mac.mask.octet[5]);
		break;
	 case FILTER_FIELD_IPV4_SIP:
	 case FILTER_FIELD_IPV4_DIP:
			printk("[Data]%u.%u.%u.%u ", NIPQUAD(pField->filter_pattern_union.ip.value));
			printk("[Mask]%u.%u.%u.%u\n", NIPQUAD(pField->filter_pattern_union.ip.mask));
		break;
	 case FILTER_FIELD_IPV6_SIPV6:
	 case FILTER_FIELD_IPV6_DIPV6:

		printk("\n[Data]%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X\n[Mask]%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X\n",
			pField->filter_pattern_union.ipv6.value.addr[3]>>16,
			pField->filter_pattern_union.ipv6.value.addr[3]&0xFFFF,
			pField->filter_pattern_union.ipv6.value.addr[2]>>16,
			pField->filter_pattern_union.ipv6.value.addr[2]&0xFFFF,
			pField->filter_pattern_union.ipv6.value.addr[1]>>16,
			pField->filter_pattern_union.ipv6.value.addr[1]&0xFFFF,
			pField->filter_pattern_union.ipv6.value.addr[0]>>16,
			pField->filter_pattern_union.ipv6.value.addr[0]&0xFFFF,
			pField->filter_pattern_union.ipv6.mask.addr[3]>>16,
			pField->filter_pattern_union.ipv6.mask.addr[3]&0xFFFF,
			pField->filter_pattern_union.ipv6.mask.addr[2]>>16,
			pField->filter_pattern_union.ipv6.mask.addr[2]&0xFFFF,
			pField->filter_pattern_union.ipv6.mask.addr[1]>>16,
			pField->filter_pattern_union.ipv6.mask.addr[1]&0xFFFF,
			pField->filter_pattern_union.ipv6.mask.addr[0]>>16,
			pField->filter_pattern_union.ipv6.mask.addr[0]&0xFFFF);
		break;
	 case FILTER_FIELD_VID_RANGE:
	 case FILTER_FIELD_IP_RANGE:
	 case FILTER_FIELD_PORT_RANGE:
		printk("[Data]0x%X [Mask]0x%X \n",
			pField->filter_pattern_union.inData.value,
			pField->filter_pattern_union.inData.mask);

		break;
	 case FILTER_FIELD_CTAG:
	 case FILTER_FIELD_STAG:
		printk("[Data]{Pri %x,%d,Vid %3.3x} [Mask]{Pri %x,%d,Vid %3.3x}\n",
			pField->filter_pattern_union.l2tag.pri.value,
			pField->filter_pattern_union.l2tag.cfi.value,
			pField->filter_pattern_union.l2tag.vid.value,
			pField->filter_pattern_union.l2tag.pri.mask,
			pField->filter_pattern_union.l2tag.cfi.mask,
			pField->filter_pattern_union.l2tag.vid.mask);
	 	break;
	 case FILTER_FIELD_IPV4_FLAG:
		printk("[Data]{%s,%s,%s} [Mask]{%s,%s,%s}\n",
			pField->filter_pattern_union.ipFlag.xf.value?"X":"*",
			pField->filter_pattern_union.ipFlag.df.value?"D":"*",
			pField->filter_pattern_union.ipFlag.mf.value?"M":"*",
			pField->filter_pattern_union.ipFlag.xf.mask?"X":"*",
			pField->filter_pattern_union.ipFlag.df.mask?"D":"*",
			pField->filter_pattern_union.ipFlag.mf.mask?"M":"*");
	 	break;
	 case FILTER_FIELD_TCP_FLAG:
		printk("[Data]{%s,%s,%s,%s,%s,%s,%s,%s} [Mask]{%s,%s,%s,%s,%s,%s,%s,%s}\n",
			pField->filter_pattern_union.tcpFlag.cwr.value?"C":"*",
			pField->filter_pattern_union.tcpFlag.ece.value?"E":"*",
			pField->filter_pattern_union.tcpFlag.urg.value?"U":"*",
			pField->filter_pattern_union.tcpFlag.ack.value?"A":"*",
			pField->filter_pattern_union.tcpFlag.psh.value?"P":"*",
			pField->filter_pattern_union.tcpFlag.rst.value?"R":"*",
			pField->filter_pattern_union.tcpFlag.syn.value?"S":"*",
			pField->filter_pattern_union.tcpFlag.fin.value?"F":"*",
			pField->filter_pattern_union.tcpFlag.cwr.mask?"C":"*",
			pField->filter_pattern_union.tcpFlag.ece.mask?"E":"*",
			pField->filter_pattern_union.tcpFlag.urg.mask?"U":"*",
			pField->filter_pattern_union.tcpFlag.ack.mask?"A":"*",
			pField->filter_pattern_union.tcpFlag.psh.mask?"P":"*",
			pField->filter_pattern_union.tcpFlag.rst.mask?"R":"*",
			pField->filter_pattern_union.tcpFlag.syn.mask?"S":"*",
			pField->filter_pattern_union.tcpFlag.fin.mask?"F":"*");
	 	break;
	 default:
		printk("[Data]%4.4X [Mask]%4.4X\n",
			pField->filter_pattern_union.inData.value,
			pField->filter_pattern_union.inData.mask);

		break;
	}	
}

static void _rtl867x_rtl8367b_careTag_display(rtk_filter_care_tag_t* careTag)
{
	if(careTag->tagType[CARE_TAG_CTAG].value
	|| careTag->tagType[CARE_TAG_STAG].value
	|| careTag->tagType[CARE_TAG_PPPOE].value
	|| careTag->tagType[CARE_TAG_IPV4].value
	|| careTag->tagType[CARE_TAG_IPV6].value)
	{
		printk("Care Tag :  %s %s %s %s %s\n",
		(careTag->tagType[CARE_TAG_CTAG].value?"Ctag":""),
		(careTag->tagType[CARE_TAG_STAG].value?"Stag":""),
		(careTag->tagType[CARE_TAG_PPPOE].value?"PPPoE":""),
		(careTag->tagType[CARE_TAG_IPV4].value?"IPv4":""),
		(careTag->tagType[CARE_TAG_IPV6].value?"IPv6":""));
	}
}

static void _rtl867x_rtl8367b_activeport_display(rtk_filter_value_t* activeport)
{
    printk("  Active port : 0x%04X(Mask:0x%04X)\n"	
		,activeport->value
		,activeport->mask);
}


void rtl867x_rtl8367b_show_aclrule_Datafiled(rtk_filter_cfg_t* pFilter_cfg)
{
	rtk_filter_field_t  *field_ptr = pFilter_cfg->fieldHead;
	int count=0;
	 
	while(field_ptr)	
	{
		printk(" --  Field %d  --\n ",count);		
		_rtl867x_rtl8367b_field_display(field_ptr);
		
		field_ptr = field_ptr->next;
		count ++;		
	}
	printk("\n");
	_rtl867x_rtl8367b_careTag_display(&pFilter_cfg->careTag);
	_rtl867x_rtl8367b_activeport_display(&pFilter_cfg->activeport);
	
}


void rtl867x_rtl8367b_show_aclrule_raw_Datafiled(rtk_filter_cfg_raw_t* pFilter_cfg)
{
	int i;
	char *rawfieldtype_string[]={
    "FILTER_FIELD_RAW_UNUSED", 		/* 0x00 */
    "FILTER_FIELD_RAW_DMAC_15_0", 	/* 0x01 */
    "FILTER_FIELD_RAW_DMAC_31_16",
    "FILTER_FIELD_RAW_DMAC_47_32",
    "FILTER_FIELD_RAW_SMAC_15_0",
    "FILTER_FIELD_RAW_SMAC_31_16",
    "FILTER_FIELD_RAW_SMAC_47_32",
    "FILTER_FIELD_RAW_ETHERTYPE",
    "FILTER_FIELD_RAW_STAG",
    "FILTER_FIELD_RAW_CTAG", 			/* 0x09 */
	"NULL", 							/* 0x0A */
	"NULL", 							/* 0x0B */
	"NULL", 							/* 0x0C */
	"NULL", 							/* 0x0D */
	"NULL", 							/* 0x0E */
	"NULL", 							/* 0x0F */
	
    "FILTER_FIELD_RAW_IPV4_SIP_15_0",	/* 0x10 */
    "FILTER_FIELD_RAW_IPV4_SIP_31_16",
    "FILTER_FIELD_RAW_IPV4_DIP_15_0",
    "FILTER_FIELD_RAW_IPV4_DIP_31_16",	/* 0x13 */
	"NULL", 							/* 0x14 */
	"NULL", 							/* 0x15 */
	"NULL", 							/* 0x16 */
	"NULL", 							/* 0x17 */
	"NULL", 							/* 0x18 */
	"NULL", 							/* 0x19 */
	"NULL", 							/* 0x1A */
	"NULL", 							/* 0x1B */
	"NULL", 							/* 0x1C */
	"NULL", 							/* 0x1D */
	"NULL", 							/* 0x1E*/
	"NULL", 							/* 0x1F */
	
    "FILTER_FIELD_RAW_IPV6_SIP_15_0",	/* 0x20 */
    "FILTER_FIELD_RAW_IPV6_SIP_31_16",    
    "FILTER_FIELD_RAW_IPV6_DIP_15_0",
    "FILTER_FIELD_RAW_IPV6_DIP_31_16",
    "NULL", 							/* 0x24 */
	"NULL", 							/* 0x25 */
	"NULL", 							/* 0x26 */
	"NULL", 							/* 0x27 */
	"NULL", 							/* 0x28 */
	"NULL", 							/* 0x29 */
	"NULL", 							/* 0x2A */
	"NULL", 							/* 0x2B */
	"NULL", 							/* 0x2C */
	"NULL", 							/* 0x2D */
	"NULL", 							/* 0x2E*/
	"NULL", 							/* 0x2F */
    
    "FILTER_FIELD_RAW_VIDRANGE",		/* 0x30 */
    "FILTER_FIELD_RAW_IPRANGE",
    "FILTER_FIELD_RAW_PORTRANGE",
    "NULL", 							/* 0x33 */
    "NULL", 							/* 0x34 */
	"NULL", 							/* 0x35 */
	"NULL", 							/* 0x36 */
	"NULL", 							/* 0x37 */
	"NULL", 							/* 0x38 */
	"NULL", 							/* 0x39 */
	"NULL", 							/* 0x3A */
	"NULL", 							/* 0x3B */
	"NULL", 							/* 0x3C */
	"NULL", 							/* 0x3D */
	"NULL", 							/* 0x3E*/
	"NULL", 							/* 0x3F */
	
    "FILTER_FIELD_SELECT00",			/* 0x40 */
    "FILTER_FIELD_SELECT01",
    "FILTER_FIELD_SELECT02",
    "FILTER_FIELD_SELECT03",
    "FILTER_FIELD_SELECT04",
    "FILTER_FIELD_SELECT05",
    "FILTER_FIELD_SELECT06",
    "FILTER_FIELD_SELECT07",
    "FILTER_FIELD_SELECT08",
    "FILTER_FIELD_SELECT09",
    "FILTER_FIELD_SELECT10",
    "FILTER_FIELD_SELECT11",
    "FILTER_FIELD_SELECT12",
    "FILTER_FIELD_SELECT13",
    "FILTER_FIELD_SELECT14",
    "FILTER_FIELD_SELECT15",
    "FILTER_FIELD_RAW_END"
	};

	for(i=0;i<RTK_FILTER_RAW_FIELD_NUMBER;i++)
	{
		printk("  Filed %d [%20s]\t data : 0x%04X  Mask :  0x%04X \n",i
			,rawfieldtype_string[pFilter_cfg->fieldRawType[i]]
			,pFilter_cfg->dataFieldRaw[i]
			,pFilter_cfg->careFieldRaw[i]);
	}
	
	printk("\n");
	_rtl867x_rtl8367b_careTag_display(&pFilter_cfg->careTag);
	_rtl867x_rtl8367b_activeport_display(&pFilter_cfg->activeport);
}

static void rtl867x_rtl8367b_show_action(rtk_filter_action_t* pAction)
{
	if(pAction->actEnable[FILTER_ENACT_PRIORITY])	
		printk("  Priority : %d \n",pAction->filterPriority);	
	else
		printk("  Priority : (NONE) \n");	
}

void rtl867x_rtl8367b_show_aclrule(rtk_filter_cfg_t* pFilter_cfg,rtk_filter_action_t* pAction)
{
	rtl867x_rtl8367b_show_aclrule_Datafiled(pFilter_cfg);
	rtl867x_rtl8367b_show_action(pAction);
}


void rtl867x_rtl8367b_show_aclrule_raw(rtk_filter_cfg_raw_t* pFilter_cfg,rtk_filter_action_t* pAction)
{
	rtl867x_rtl8367b_show_aclrule_raw_Datafiled(pFilter_cfg);
	rtl867x_rtl8367b_show_action(pAction);
}

void rtl867x_rtl8367b_show_aclchain(void)
{
	rtl867x_8367b_acl_chain_t* chain_entry;
	rtl867x_8367b_acl_chain_rule_t *chain_aclrule_entry;	
	int asic_hw_index = 0;

	list_for_each_entry(chain_entry,&chain_list_head,chain_list) 
	{       	
		int sw_idx=0;	
		
		printk("====ACL Chain (priority:%d , force:%d, rule_cnt:%d)====\n"
			,chain_entry->priority,chain_entry->force,chain_entry->ruleCnt);	
		
		list_for_each_entry(chain_aclrule_entry,&chain_entry->aclrule_list_head,aclrule_list) 
		{
			printk("-- Rule %02d (used rule num in hw : %d    index : %d ~ %d)--\n",sw_idx,chain_aclrule_entry->asic_used_rulenum
				,asic_hw_index,asic_hw_index+chain_aclrule_entry->asic_used_rulenum-1);
			rtl867x_rtl8367b_show_aclrule(chain_aclrule_entry->cfg,chain_aclrule_entry->act);	
			printk("\n");
			
			sw_idx++;
			asic_hw_index += chain_aclrule_entry->asic_used_rulenum;		
		}
		
    }
}

module_init(rtl867x_8367b_acl_init);
module_exit(rtl867x_8367b_acl_exit);


