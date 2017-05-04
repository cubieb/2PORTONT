/*******************************************************************************
  acl control 
         These api can set acl rules on RTL865X_ACL_USER_USED and RTL865X_ACL_DEFAULT chain
         In static acl mode 	: all pkts are forwarded in 8676 swich (hw-acc) 
                                       	, the user can set some filter rules let them dropped or trapped to cpu
         In dynamic acl mode 	: all pkts are trapped to cpu first
         					, the user can set permit ruls let some specific flow be hw-acc in 8676 swich 
********************************************************************************/
#include "rtl865x_acl_control.h"
#include "../common/rtl_errno.h"
#include "../common/rtl865x_netif_local.h"
#include "../common/rtl865x_eventMgr.h" /*call back function....*/
#include "../l3Driver/rtl865x_ppp_local.h"
#include "../l3Driver/rtl865x_nexthop.h"
#include <net/rtl/rtl865x_fdb_api.h>


static unsigned int	ACL_MODE 			= RTL865X_ACL_Mode_Normal;
#ifdef CONFIG_RTL8676_Static_ACL
static unsigned int	MAC_FILTER_MODE 	= RTL865X_ACL_MAC_FILTER_ALL_PERMIT;
#endif

static struct list_head ChainList[RTL865X_CHAINLIST_NUMBER_PER_TBL];  
extern struct proc_dir_entry *rtl865x_proc_dir;
static struct proc_dir_entry* rtl865x_proc_acl_mode;
#ifdef CONFIG_RTL8676_Static_ACL
static struct proc_dir_entry *rtl865x_proc_acl_mac_filter_mode;
#endif


static int _rtl865x_acl_control_rewrite_chain_to_acl(void);
static int _rtl865x_acl_control_change_default_action(void);
static int _rtl865x_acl_control_free_chain(struct list_head *listHead);
static int _rtl865x_acl_control_cleanup(void);
static int 	_rtl865x_acl_control_proc_acl_mode_read(char *page, char **start, off_t off, int count, int *eof, void *data);
static int 	_rtl865x_acl_control_proc_acl_mode_write( struct file *filp, const char *buff,unsigned long len, void *data );
#ifdef CONFIG_RTL8676_Static_ACL
static int 	_rtl865x_acl_control_proc_acl_mac_filter_mode_read(char *page, char **start, off_t off, int count, int *eof, void *data);
static int 	_rtl865x_acl_control_proc_acl_mac_filter_mode_write( struct file *filp, const char *buff,unsigned long len, void *data );
#endif

int rtl865x_acl_control_set_mode(int mode);
#ifdef CONFIG_RTL8676_Static_ACL
static int _rtl865x_acl_control_set_mac_filter_mode(int mode);
#endif


static int32 rtl865x_acl_control_eventHandle_swnetif_change(void *para)
{
	int32 retval = EVENT_CONTINUE_EXECUTE;

	ACL_CONTROL_DEBUG_PRK("Enter %s\n",__func__);

	/* re-sync all sw_rule to hw acl rules */
	_rtl865x_acl_control_change_default_action();
	
	return retval;
}


static int32 _rtl865x_acl_control_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_acl_control_eventHandle_swnetif_change;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_acl_control_eventHandle_swnetif_change;
	rtl865x_registerEvent(&eventParam);
	
	return SUCCESS;

}
static int32 _rtl865x_acl_control_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_acl_control_eventHandle_swnetif_change;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_acl_control_eventHandle_swnetif_change;
	rtl865x_unRegisterEvent(&eventParam);
	
	return SUCCESS;
}
static int __init rtl865x_acl_control_init(void)
{
	int i;
	int retval=SUCCESS;
	_rtl865x_acl_control_register_event();
	
	for(i = 0; i < RTL865X_CHAINLIST_NUMBER_PER_TBL; i++)
	{
		INIT_LIST_HEAD(&ChainList[i]);
	}

	if(rtl865x_proc_dir==NULL)
		rtl865x_proc_dir = proc_mkdir(RTL865X_PROC_DIR_NAME,NULL);
	
	if(rtl865x_proc_dir)
	{	
		/*  rtl865x_proc_acl_mode  */
		{
			rtl865x_proc_acl_mode = create_proc_entry("acl_mode",0,rtl865x_proc_dir);
			if(rtl865x_proc_acl_mode != NULL)
			{
				rtl865x_proc_acl_mode->read_proc = _rtl865x_acl_control_proc_acl_mode_read;
				rtl865x_proc_acl_mode->write_proc= _rtl865x_acl_control_proc_acl_mode_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for rtl865x_proc_acl_mode");
				retval = FAILED;
				goto out;
			}
		}
		#ifdef CONFIG_RTL8676_Static_ACL
		/*  rtl865x_proc_acl_mac_filter_mode  */
		{
			rtl865x_proc_acl_mac_filter_mode = create_proc_entry("acl_mac_filter_mode",0,rtl865x_proc_dir);
			if(rtl865x_proc_acl_mac_filter_mode != NULL)
			{
				rtl865x_proc_acl_mac_filter_mode->read_proc = _rtl865x_acl_control_proc_acl_mac_filter_mode_read;
				rtl865x_proc_acl_mac_filter_mode->write_proc= _rtl865x_acl_control_proc_acl_mac_filter_mode_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for rtl865x_proc_acl_mode");
				retval = FAILED;
				goto out;
			}
		}
		#endif

	}
	else	
		retval = FAILED;
	
out:
	if(retval == FAILED)
		_rtl865x_acl_control_cleanup();
	
	return retval;	

} 

static int _rtl865x_acl_control_cleanup(void)
{
	
	if(rtl865x_proc_dir)
	{	

		if(rtl865x_proc_acl_mode!=NULL)
		{
			remove_proc_entry("rtl865x_proc_acl_mode", rtl865x_proc_dir);
		}

		#ifdef CONFIG_RTL8676_Static_ACL
		if(rtl865x_proc_acl_mac_filter_mode!=NULL)
		{
			remove_proc_entry("rtl865x_proc_acl_mac_filter_mode", rtl865x_proc_dir);
		}
		#endif
		
		remove_proc_entry(RTL865X_PROC_DIR_NAME, NULL);
		rtl865x_proc_dir = NULL;				
	}
	
	return SUCCESS;	
}


static void __exit rtl865x_acl_control_exit(void)
{
	_rtl865x_acl_control_unRegister_event();
	_rtl865x_acl_control_cleanup();
} 


static int _rtl865x_acl_control_proc_acl_mode_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
      int len;	 

	  switch(ACL_MODE)
	  {
	  	case RTL865X_ACL_Mode_Always_Permit:
			len = sprintf(page, "0412 asic ACL mode : always PERMIT \n");
			break;
		case RTL865X_ACL_Mode_Always_Trap:
			len = sprintf(page, "0412 asic ACL mode : always TRAP \n");
			break;
		case RTL865X_ACL_Mode_Normal:
			len = sprintf(page, "0412 asic ACL mode : NORMAL \n");
			break;
		default:
			len = sprintf(page, "0412 asic ACL mode : Unknown (BUG!!) \n");
	  }      

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int32 _rtl865x_acl_control_proc_acl_mode_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 	tmpbuf[512];
	char		*strptr;	
	int32 retval = FAILED;
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;

		if(strlen(strptr)==0)
		{
			goto errout;
		}
		
		/*parse command*/
		if(strncmp(strptr, "permit",6) == 0)
		{
			retval = rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Always_Permit);
		}
		else if(strncmp(strptr, "trap",4) == 0)
		{
			retval = rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Always_Trap);
		}
		else if(strncmp(strptr, "normal",6) == 0)
		{
			retval = rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Normal);
		}
		#ifdef CONFIG_RTL8676_Dynamic_ACL
		else if(strncmp(strptr, "clean_L2",8) == 0)
		{
			retval = rtl865x_acl_control_L2_permit_clean();
		}
		else if(strncmp(strptr, "clean_L3",8) == 0)
		{
			retval = rtl865x_acl_control_L34_permit_clean();
		}
		#endif
		else
		{
			goto errout;
		}

		if(retval==SUCCESS)
			printk("write success ! \n");
		else
			printk("error : change mode fail ! \n");
		
	}
	else
	{
errout:
		#ifdef CONFIG_RTL8676_Dynamic_ACL
		printk("error input  (permit/trap/normal/clean_L2/clean_L3)\n");
		#else
		printk("error input  (permit/trap/normal)\n");
		#endif
	}

	return len;
}
#ifdef CONFIG_RTL8676_Static_ACL
static int _rtl865x_acl_control_proc_acl_mac_filter_mode_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
      int len;	  
	  
	  switch(MAC_FILTER_MODE)
	  {
	  	case RTL865X_ACL_MAC_FILTER_ALL_PERMIT:
			len = sprintf(page, "0412 asic ACL mac filter mode :	In: PERMIT	Out: PERMIT\n");
			break;
		case RTL865X_ACL_MAC_FILTER_IN_DROP_OUT_PERMIT:
			len = sprintf(page, "0412 asic ACL mac filter mode :	In: DROP		Out: PERMIT\n");
			break;
		case RTL865X_ACL_MAC_FILTER_IN_PERMIT_OUT_DROP:
			len = sprintf(page, "0412 asic ACL mac filter mode :	In: PERMIT	Out: DROP\n");
			break;
		case RTL865X_ACL_MAC_FILTER_ALL_DROP:
			len = sprintf(page, "0412 asic ACL mac filter mode :	In: DROP		Out: DROP\n");
			break;	
		default:
			len = sprintf(page, "0412 asic ACL mac filter mode : 	Unknown (BUG!!) \n");
	  }  

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int32 _rtl865x_acl_control_proc_acl_mac_filter_mode_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 	tmpbuf[512];
	char		*strptr;	
	int32 retval = FAILED;
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;

		if(strlen(strptr)==0)
		{
			goto errout;
		}
		
		/*parse command*/
		if(strncmp(strptr, "all_permit",10) == 0)
		{
			retval = _rtl865x_acl_control_set_mac_filter_mode(RTL865X_ACL_MAC_FILTER_ALL_PERMIT);
		}
		else if(strncmp(strptr, "in_drop_out_permit",18) == 0)
		{
			retval = _rtl865x_acl_control_set_mac_filter_mode(RTL865X_ACL_MAC_FILTER_IN_DROP_OUT_PERMIT);
		}
		else if(strncmp(strptr, "in_permit_out_drop",18) == 0)
		{
			retval = _rtl865x_acl_control_set_mac_filter_mode(RTL865X_ACL_MAC_FILTER_IN_PERMIT_OUT_DROP);
		}
		else if(strncmp(strptr, "all_drop",8) == 0)
		{
			retval = _rtl865x_acl_control_set_mac_filter_mode(RTL865X_ACL_MAC_FILTER_ALL_DROP);
		}
		else
		{
			goto errout;
		}

		if(retval==SUCCESS)
			printk("write success ! \n");
		else
			printk("error : change mode fail ! \n");
		
	}
	else
	{
errout:
		printk("error input  (all_permit/in_drop_out_permit/in_permit_out_drop/all_drop)\n");
	}

	return len;
}
#endif

static int _rtl865x_acl_control_rewrite_chain_to_acl(void)
{

	xt_rule_to_acl_t *match2acl;
	int32 retval = SUCCESS;
	int32 temp_retval = SUCCESS;

	/* delete all user used acl in 8676 asic */
	rtl865x_flush_allAcl_fromChain(NULL,RTL865X_ACL_USER_USED);


	switch(ACL_MODE)
	{
		case RTL865X_ACL_Mode_Always_Permit:
		case RTL865X_ACL_Mode_Always_Trap:
			break;
		case RTL865X_ACL_Mode_Normal:
			#ifdef CONFIG_RTL8676_Static_ACL
			list_for_each_entry(match2acl,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_FORCE_TRAP],list)
			{			
				temp_retval = rtl865x_add_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED);
				if( temp_retval!=SUCCESS && temp_retval!=RTL_EENTRYALREADYEXIST)				
					retval += temp_retval; 				
			}
			list_for_each_entry(match2acl,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_NORMAL],list)
			{			
				temp_retval = rtl865x_add_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED);
				if( temp_retval!=SUCCESS && temp_retval!=RTL_EENTRYALREADYEXIST)				
					retval += temp_retval; 	
			}
			#else  /* CONFIG_RTL8676_Dynamic_ACL  */
			list_for_each_entry(match2acl,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L2_UNI_PERMIT],list)
			{							
				temp_retval = rtl865x_add_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED,1,0);
				if( temp_retval!=SUCCESS && temp_retval!=RTL_EENTRYALREADYEXIST)				
					retval += temp_retval; 	
			}
			list_for_each_entry(match2acl,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_REDIRECT],list)
			{			
				temp_retval = rtl865x_add_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED,1,0);
				if( temp_retval!=SUCCESS && temp_retval!=RTL_EENTRYALREADYEXIST)				
					retval += temp_retval; 	
			}
	
			list_for_each_entry(match2acl,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_PERMIT],list)
			{				
				temp_retval = rtl865x_add_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED,1,0);
				if( temp_retval!=SUCCESS && temp_retval!=RTL_EENTRYALREADYEXIST)				
					retval += temp_retval; 	
			}
			#endif			
			break;			
		default: //unrecognized acl mode
			return FAILED;				
	}

	/* Kevin, when there are same rules in acl chain (rtl865x_add_acl return RTL_EENTRYALREADYEXIST) , it is a acceptable case
		ex.    when drop the specified outgoing pkts in macfilter		
   		CMD: /bin/iptables -A macfilter -i eth0.2 -m mac --mac-source 00:50:fc:65:8d:39 -j RETURN
		CMD: /bin/iptables -A macfilter -i eth0.3 -m mac --mac-source 00:50:fc:65:8d:39 -j RETURN
		CMD: /bin/iptables -A macfilter -i eth0.4 -m mac --mac-source 00:50:fc:65:8d:39 -j RETURN
		CMD: /bin/iptables -A macfilter -i eth0.5 -m mac --mac-source 00:50:fc:65:8d:39 -j RETURN	

		these iptables rules will be translated in "br0" , so they are same in the acl rule
	*/	
	if(retval!=SUCCESS)		
	{
		ACL_CONTROL_DEBUG_PRK("warning ! %s return failed (error no.:%d) at line %d\n",__func__,retval,__LINE__);
		return FAILED;	
	}
	else
		return SUCCESS;	
}

static int _rtl865x_acl_control_change_default_action(void)
{	

	rtl865x_AclRule_t *rule;	
	int32 retval = SUCCESS;

	/* delete default acl in 8676 asic */
	rtl865x_flush_allAcl_fromChain(NULL,RTL865X_ACL_DEFAULT);	
	
	/*  ingress */	
	if(ACL_MODE != RTL865X_ACL_Mode_Normal)
	{
		rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
		if(!rule)
		{				
			ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
			retval = FAILED;
			goto FINISH;
		}		
		memset(rule, 0,sizeof(rtl865x_AclRule_t));		
		rule->ruleType_	= RTL865X_ACL_MAC;	

		if(ACL_MODE == RTL865X_ACL_Mode_Always_Permit)
			rule->actionType_	= RTL865X_ACL_PERMIT;	
		else  // ACL_MODE == RTL865X_ACL_Mode_Always_Trap
			rule->actionType_	= RTL865X_ACL_TOCPU;	
		
		rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
		rule->direction_ = RTL865X_ACL_INGRESS;	
		retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT,1,0);
		kfree(rule);
		if(retval!=SUCCESS)
			goto FINISH;
	}
	else // ACL_MODE == RTL865X_ACL_Mode_Normal
	{		
		#ifdef CONFIG_RTL8676_Static_ACL
		switch(MAC_FILTER_MODE)
		{
			case RTL865X_ACL_MAC_FILTER_ALL_PERMIT:
				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));		
				rule->ruleType_	= RTL865X_ACL_MAC;
				rule->actionType_	= RTL865X_ACL_PERMIT;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;	
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;
				break;
			case RTL865X_ACL_MAC_FILTER_IN_DROP_OUT_PERMIT:
				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));		
				rule->ruleType_ = RTL865X_ACL_SRCFILTER;
				rule->srcFilterIgnoreL3L4_ = 1;
				rule->srcFilterIgnoreL4_ = 1;							
				rule->srcFilterPort_ = 0x1E;  /*   port 1, 2, 3, 4  */
				rule->actionType_	= RTL865X_ACL_PERMIT;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;
				
				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));

				rule->ruleType_	= RTL865X_ACL_MAC;
				rule->typeLen_ 		= 0x0800;
				rule->typeLenMask_ 	= 0xFFFF;
				rule->actionType_	= RTL865X_ACL_DROP;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;				
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;

				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));		
				rule->ruleType_	= RTL865X_ACL_MAC;
				rule->actionType_	= RTL865X_ACL_PERMIT;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;	
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;				
				break;
			case RTL865X_ACL_MAC_FILTER_IN_PERMIT_OUT_DROP:
				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));		
				rule->ruleType_ = RTL865X_ACL_SRCFILTER;
				rule->srcFilterIgnoreL3L4_ = 1;
				rule->srcFilterIgnoreL4_ = 1;							
				rule->srcFilterPort_ = 0x01;  /*   port 0  */
				rule->actionType_	= RTL865X_ACL_PERMIT;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;
				
				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));		
				rule->ruleType_	= RTL865X_ACL_MAC;
				rule->typeLen_ 		= 0x0800;
				rule->typeLenMask_ 	= 0xFFFF;
				rule->actionType_	= RTL865X_ACL_DROP;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;	
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;

				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));		
				rule->ruleType_	= RTL865X_ACL_MAC;
				rule->actionType_	= RTL865X_ACL_PERMIT;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;	
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;	
				break;
			case RTL865X_ACL_MAC_FILTER_ALL_DROP:
				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));		
				rule->ruleType_	= RTL865X_ACL_MAC;
				rule->typeLen_ 		= 0x0800;
				rule->typeLenMask_ 	= 0xFFFF;
				rule->actionType_	= RTL865X_ACL_DROP;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;	
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;

				rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
				if(!rule)
				{				
					ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
					retval = FAILED;
					goto FINISH;
				}		
				memset(rule, 0,sizeof(rtl865x_AclRule_t));		
				rule->ruleType_	= RTL865X_ACL_MAC;
				rule->actionType_	= RTL865X_ACL_PERMIT;		
				rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
				rule->direction_ = RTL865X_ACL_INGRESS;	
				retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT);
				kfree(rule);
				if(retval!=SUCCESS)
					goto FINISH;	
				break;
				
		}
		#else /* CONFIG_RTL8676_Dynamic_ACL */
			rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
			if(!rule)
			{				
				ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
				retval = FAILED;
				goto FINISH;
			}		
			memset(rule, 0,sizeof(rtl865x_AclRule_t));		
			rule->ruleType_	= RTL865X_ACL_MAC;			
			rule->actionType_	= RTL865X_ACL_TOCPU;			
			rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
			rule->direction_ = RTL865X_ACL_INGRESS;	
			retval = rtl865x_add_acl(rule, NULL, RTL865X_ACL_DEFAULT,1,0);
			kfree(rule);
			if(retval!=SUCCESS)
				goto FINISH;
		#endif
	}

	retval = _rtl865x_acl_control_rewrite_chain_to_acl();
	
FINISH:
	return retval;	
}

int rtl865x_acl_control_set_mode(int mode)
{
	if(	mode!=RTL865X_ACL_Mode_Always_Permit 
		&& mode!=RTL865X_ACL_Mode_Always_Trap
		&& mode!=RTL865X_ACL_Mode_Normal)
		return FAILED;	

	ACL_MODE = mode;

	return _rtl865x_acl_control_change_default_action();
}




#ifdef CONFIG_RTL8676_Static_ACL
int _rtl865x_acl_control_set_mac_filter_mode(int mode)
{
	if(		mode!=RTL865X_ACL_MAC_FILTER_ALL_PERMIT 
		&& mode!=RTL865X_ACL_MAC_FILTER_IN_DROP_OUT_PERMIT
		&& mode!=RTL865X_ACL_MAC_FILTER_IN_PERMIT_OUT_DROP
		&& mode!=RTL865X_ACL_MAC_FILTER_ALL_DROP)
		return FAILED;	

	MAC_FILTER_MODE = mode;

	return _rtl865x_acl_control_change_default_action();
}

int rtl865x_acl_control_filter_rule_add(xt_rule_to_acl_t* node,int force_trap)
{
	if(force_trap)
		list_add_tail(&node->list,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_FORCE_TRAP]);
	else
		list_add_tail(&node->list,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_NORMAL]);

	if(_rtl865x_acl_control_rewrite_chain_to_acl()!=SUCCESS)
		return FAILED;
	else
		return SUCCESS;
}
int rtl865x_acl_control_filter_rule_clean(void)
{
	int i;
	for(i = 0; i < RTL865X_CHAINLIST_NUMBER_PER_TBL; i++)
	{
		_rtl865x_acl_control_free_chain(&ChainList[i]);
	}
	return 0;
}
#else /* CONFIG_RTL8676_Dynamic_ACL */
int rtl865x_acl_control_L2_permit_add(unsigned char* src_mac,unsigned char* dst_mac, char *acl_in_ifname)
{
	xt_rule_to_acl_t *list_node=NULL;
	rtl865x_AclRule_t *rule=NULL;	
	xt_rule_to_acl_t *match2acl;


	ACL_CONTROL_DEBUG_PRK("Enter %s (src mac:%02X:%02X:%02X:%02X:%02X:%02X    dst mac:%02X:%02X:%02X:%02X:%02X:%02X)\n",__func__
			,src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]
			,dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5]);


	//duplicate check
	list_for_each_entry(match2acl,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L2_UNI_PERMIT],list)
	{			
		if(!memcmp(match2acl->aclRule->un_ty.srcMac_.octet,src_mac,ETH_ALEN) && !memcmp(match2acl->aclRule->un_ty.dstMac_.octet,dst_mac,ETH_ALEN))
			return SUCCESS;
	}	

	list_node = kmalloc(sizeof(xt_rule_to_acl_t),GFP_KERNEL);	
	if(!list_node)
	{
		ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
		goto FailToAdd;
	}		
	rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
	if(!rule)
	{		
		ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
		goto FailToAdd;
	}
	
	memset(list_node, 0,sizeof(xt_rule_to_acl_t));
	memset(rule, 0,sizeof(rtl865x_AclRule_t));
	rule->ruleType_ = RTL865X_ACL_MAC;
	rule->actionType_	= RTL865X_ACL_PERMIT;		
	rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
	rule->direction_ = RTL865X_ACL_INGRESS;
	memcpy(rule->un_ty.srcMac_.octet, src_mac, ETH_ALEN);
	memcpy(rule->un_ty.dstMac_.octet, dst_mac, ETH_ALEN);
	memset(rule->un_ty.srcMacMask_.octet, 0xff, ETH_ALEN);
	memset(rule->un_ty.dstMacMask_.octet, 0xff, ETH_ALEN);
	list_node->aclRule = rule;
	strcpy(list_node->iniface, acl_in_ifname);

	/* If acl mode is normal , take effect immediately */
	if(ACL_MODE == RTL865X_ACL_Mode_Normal)
	{
		if(rtl865x_add_acl(rule, list_node->iniface, RTL865X_ACL_USER_USED,1,0)!=SUCCESS)
			goto FailToAdd;
	}
	list_add_tail(&list_node->list,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L2_UNI_PERMIT]);	


		ACL_CONTROL_DEBUG_PRK("(%s)add succcess! (src mac:%02X:%02X:%02X:%02X:%02X:%02X    dst mac:%02X:%02X:%02X:%02X:%02X:%02X)\n",__func__
			,src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]
			,dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5]);	
		return SUCCESS;


FailToAdd:	
	if(list_node)
		kfree(list_node);
	if(rule);
		kfree(rule);
		
	return FAILED;

}
int rtl865x_acl_control_L2_permit_del(unsigned char* del_mac)
{

	xt_rule_to_acl_t *match2acl;
	xt_rule_to_acl_t *nxt;
	int findout=false;	


	list_for_each_entry_safe(match2acl,nxt,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L2_UNI_PERMIT],list)
	{			
		if(!memcmp(match2acl->aclRule->un_ty.srcMac_.octet,del_mac,ETH_ALEN) || !memcmp(match2acl->aclRule->un_ty.dstMac_.octet,del_mac,ETH_ALEN))
		{
			findout=true;
			if(rtl865x_del_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED)!=SUCCESS)
				return FAILED;								
			list_del(&match2acl->list);
			kfree(match2acl->aclRule);
			kfree(match2acl);
		}
		
	}

	if(findout==true)
	{	
			ACL_CONTROL_DEBUG_PRK("(%s)del from acl L2 permit : (mac:%02X:%02X:%02X:%02X:%02X:%02X)\n",__func__
				,del_mac[0],del_mac[1],del_mac[2],del_mac[3],del_mac[4],del_mac[5]);
		}

		return SUCCESS;

}
int rtl865x_acl_control_L2_permit_del_pair(unsigned char* src_mac,unsigned char* dst_mac)
{

	xt_rule_to_acl_t *match2acl;
	xt_rule_to_acl_t *nxt;
	int findout=false;	


	list_for_each_entry_safe(match2acl,nxt,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L2_UNI_PERMIT],list)
	{			
		if(!memcmp(match2acl->aclRule->un_ty.srcMac_.octet,src_mac,ETH_ALEN) && !memcmp(match2acl->aclRule->un_ty.dstMac_.octet,dst_mac,ETH_ALEN))
		{
			findout=true;
			if(rtl865x_del_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED)!=SUCCESS)
				return FAILED;
			list_del(&match2acl->list);
			kfree(match2acl->aclRule);
			kfree(match2acl);
		}
		
	}

	if(findout==true)
	{	
			ACL_CONTROL_DEBUG_PRK("(%s)del from acl L2 permit : (src mac:%02X:%02X:%02X:%02X:%02X:%02X    dst mac:%02X:%02X:%02X:%02X:%02X:%02X)\n",__func__
				,src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]
				,dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5]);
		}

		return SUCCESS;

}

int rtl865x_acl_control_L2_permit_clean(void)
{

	_rtl865x_acl_control_free_chain(&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L2_UNI_PERMIT]);

	if(_rtl865x_acl_control_rewrite_chain_to_acl()!=SUCCESS)
		return FAILED;
	else
		return SUCCESS;
	
}
int rtl865x_acl_control_L34_permit_add(__u32 src_ip,__u16 src_port,__u32 dst_ip,__u16 dst_port,__u8 protocol,char* in_netifname)
{

	xt_rule_to_acl_t *list_node=NULL;
	rtl865x_AclRule_t *rule=NULL;
	xt_rule_to_acl_t *match2acl;

	ACL_CONTROL_DEBUG_PRK("Enter %s : add into acl L34 permit : (src ip:%u.%u.%u.%u:%u  dst ip:%u.%u.%u.%u:%u  in_if:%s  protocol:%s)\n",__func__
			,NIPQUAD(src_ip),src_port,NIPQUAD(dst_ip),dst_port,in_netifname,protocol==IPPROTO_TCP?"TCP":"UDP");

	if( protocol!=IPPROTO_TCP && protocol!=IPPROTO_UDP )	
		goto FailToAdd;
	

	if(!rtl865x_netifExist(in_netifname))  /*  the interface is in asic netif table  */		
		goto FailToAdd;


	//duplicate check
	list_for_each_entry(match2acl,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_PERMIT],list)
	{
		if(protocol==IPPROTO_TCP && match2acl->aclRule->ruleType_==RTL865X_ACL_TCP)
		{
			if( match2acl->aclRule->un_ty.srcIpAddr_==src_ip &&  match2acl->aclRule->un_ty.tcpSrcPortLB_==src_port && match2acl->aclRule->un_ty.tcpSrcPortUB_==src_port &&
			    match2acl->aclRule->un_ty.dstIpAddr_==dst_ip &&  match2acl->aclRule->un_ty.tcpDstPortLB_==dst_port && match2acl->aclRule->un_ty.tcpDstPortUB_==dst_port &&
			    !strcmp(match2acl->iniface,in_netifname))
				return SUCCESS;
		}

		if(protocol==IPPROTO_UDP && match2acl->aclRule->ruleType_==RTL865X_ACL_UDP)
		{
			if( match2acl->aclRule->un_ty.srcIpAddr_==src_ip &&  match2acl->aclRule->un_ty.udpSrcPortLB_==src_port && match2acl->aclRule->un_ty.udpSrcPortUB_==src_port &&
			    match2acl->aclRule->un_ty.dstIpAddr_==dst_ip &&  match2acl->aclRule->un_ty.udpDstPortLB_==dst_port && match2acl->aclRule->un_ty.udpDstPortUB_==dst_port &&
			    !strcmp(match2acl->iniface,in_netifname))
				return SUCCESS;
		}
	}

	list_node = kmalloc(sizeof(xt_rule_to_acl_t),GFP_KERNEL);	
	if(!list_node)
	{
		ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
		goto FailToAdd;
	}
		
	rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
	if(!rule)
	{		
		ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
		goto FailToAdd;
	}

	
	memset(list_node, 0,sizeof(xt_rule_to_acl_t));
	memset(rule, 0,sizeof(rtl865x_AclRule_t));
	rule->actionType_	= RTL865X_ACL_PERMIT;		
	rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
	rule->direction_ = RTL865X_ACL_INGRESS;
	
	if(protocol==IPPROTO_TCP)
	{
		rule->ruleType_ = RTL865X_ACL_TCP;
		rule->un_ty.srcIpAddr_ 		= src_ip;
		rule->un_ty.srcIpAddrMask_	= 0xFFFFFFFF;
		rule->un_ty.dstIpAddr_		= dst_ip;
		rule->un_ty.dstIpAddrMask_	= 0xFFFFFFFF;
		rule->un_ty.tcpSrcPortLB_	= src_port;
		rule->un_ty.tcpSrcPortUB_	= src_port;
		rule->un_ty.tcpDstPortLB_	= dst_port;
		rule->un_ty.tcpDstPortUB_	= dst_port;	
	}
	else if (protocol==IPPROTO_UDP)
	{
		rule->ruleType_ = RTL865X_ACL_UDP;	
		rule->un_ty.srcIpAddr_ 		= src_ip;
		rule->un_ty.srcIpAddrMask_	= 0xFFFFFFFF;
		rule->un_ty.dstIpAddr_		= dst_ip;
		rule->un_ty.dstIpAddrMask_	= 0xFFFFFFFF;
		rule->un_ty.udpSrcPortLB_	= src_port;
		rule->un_ty.udpSrcPortUB_	= src_port;
		rule->un_ty.udpDstPortLB_	= dst_port;
		rule->un_ty.udpDstPortUB_	= dst_port;
	}
	
	list_node->aclRule = rule;
	strcpy(list_node->iniface,in_netifname);
	/* If acl mode is normal , take effect immediately */
	if(ACL_MODE == RTL865X_ACL_Mode_Normal)
	{
		if(rtl865x_add_acl(rule, list_node->iniface, RTL865X_ACL_USER_USED,1,0)!=SUCCESS)
			goto FailToAdd;	
	}	
	list_add_tail(&list_node->list,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_PERMIT]);	


	
		ACL_CONTROL_DEBUG_PRK("(%s)add into acl L34 permit : (src ip:%u.%u.%u.%u:%u  dst ip:%u.%u.%u.%u:%u  in_if:%s  protocol:%s)\n",__func__
			,NIPQUAD(src_ip),src_port,NIPQUAD(dst_ip),dst_port,in_netifname,protocol==IPPROTO_TCP?"TCP":"UDP");
		return SUCCESS;

	
	

FailToAdd:	
	if(list_node)
		kfree(list_node);
	if(rule);
		kfree(rule);	
	return FAILED;
}
int rtl865x_acl_control_L34_permit_del(__u32 src_ip,__u16 src_port,__u32 dst_ip,__u16 dst_port,__u8 protocol)
{
	xt_rule_to_acl_t *match2acl;
	xt_rule_to_acl_t *nxt;
	int findout=false;
	


	list_for_each_entry_safe(match2acl,nxt,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_PERMIT],list)
	{			
		if(protocol==IPPROTO_TCP && match2acl->aclRule->ruleType_==RTL865X_ACL_TCP)
		{
			if( match2acl->aclRule->un_ty.srcIpAddr_==src_ip &&  match2acl->aclRule->un_ty.tcpSrcPortLB_==src_port && match2acl->aclRule->un_ty.tcpSrcPortUB_==src_port &&
			    match2acl->aclRule->un_ty.dstIpAddr_==dst_ip &&  match2acl->aclRule->un_ty.tcpDstPortLB_==dst_port && match2acl->aclRule->un_ty.tcpDstPortUB_==dst_port)
			{
				findout=true;
				if(rtl865x_del_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED)!=SUCCESS)
					return FAILED;
				list_del(&match2acl->list);
				kfree(match2acl->aclRule);
				kfree(match2acl);
			}
		}

		if(protocol==IPPROTO_UDP && match2acl->aclRule->ruleType_==RTL865X_ACL_UDP)
		{
			if( match2acl->aclRule->un_ty.srcIpAddr_==src_ip &&  match2acl->aclRule->un_ty.udpSrcPortLB_==src_port && match2acl->aclRule->un_ty.udpSrcPortUB_==src_port &&
			    match2acl->aclRule->un_ty.dstIpAddr_==dst_ip &&  match2acl->aclRule->un_ty.udpDstPortLB_==dst_port && match2acl->aclRule->un_ty.udpDstPortUB_==dst_port)
			{
				findout=true;
				if(rtl865x_del_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED)!=SUCCESS)
					return FAILED;
				list_del(&match2acl->list);
				kfree(match2acl->aclRule);
				kfree(match2acl);
			}
		}
		
	}

	if(findout==true)
	{
			ACL_CONTROL_DEBUG_PRK("(%s)del from acl L34 permit : (src ip:%u.%u.%u.%u:%u  dst ip:%u.%u.%u.%u:%u   protocol:%s)\n",__func__
				,NIPQUAD(src_ip),src_port,NIPQUAD(dst_ip),dst_port,protocol==IPPROTO_TCP?"TCP":"UDP");	
		}
	
		return SUCCESS;
	
}
int rtl865x_acl_control_L34_permit_clean(void)
{

	_rtl865x_acl_control_free_chain(&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_PERMIT]);

	if(_rtl865x_acl_control_rewrite_chain_to_acl()!=SUCCESS)
		return FAILED;
	else
		return SUCCESS;
	
}


/* 
	ACL match field : int_ip , int_port , rem_ip , rem_port , protocol

	This acl rule will add at "acl_in_ifname" netif
	 
	The following fields is for redirecting
		redir_scr_ip 		: After napt and policy routing , the packet's scr ip
		redir_dst_ip 		: After napt and policy routing , the packet's nexthop's ip addr
		                                (Attention : this is not packet's dest ip, it is used for fill packet L2 dest mac addr)
		redir_out_netif	: After napt and policy routing,  the asic netif that packet will be routed to 
*/
int rtl865x_acl_control_L34_redirect_add(__u32 int_ip,__u16 int_port,__u32 rem_ip,__u16 rem_port,__u8 protocol,char *acl_in_ifname
													,__u32 redir_scr_ip,__u32 redir_dst_ip,char *redir_out_netif)
{

	xt_rule_to_acl_t *list_node=NULL;
	rtl865x_AclRule_t *rule=NULL;
	xt_rule_to_acl_t *match2acl;
	rtl865x_netif_local_t *redir_out_netIf;	
		


	ACL_CONTROL_DEBUG_PRK("Enter %s : add into acl L34 redirect : (int ip:%u.%u.%u.%u:%u  rem ip:%u.%u.%u.%u:%u  protocol:%s in_if:%s)\n\
						apt_scr_ip:%u.%u.%u.%u   nexthop's ip:%u.%u.%u.%u  redir_out_netif:%s",__func__
			,NIPQUAD(int_ip),int_port,NIPQUAD(rem_ip),rem_port,protocol==IPPROTO_TCP?"TCP":"UDP",acl_in_ifname,NIPQUAD(redir_scr_ip),NIPQUAD(redir_dst_ip),redir_out_netif); /* hp_hack */
	

	if( protocol!=IPPROTO_TCP && protocol!=IPPROTO_UDP )
		goto FailToAdd;
	

	//duplicate check
	list_for_each_entry(match2acl,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_REDIRECT],list)
	{
		if(protocol==IPPROTO_TCP && match2acl->aclRule->ruleType_==RTL865X_ACL_TCP)
		{
			if( match2acl->aclRule->un_ty.srcIpAddr_==int_ip &&  match2acl->aclRule->un_ty.tcpSrcPortLB_==int_port && match2acl->aclRule->un_ty.tcpSrcPortUB_==int_port &&
				match2acl->aclRule->un_ty.dstIpAddr_==rem_ip &&  match2acl->aclRule->un_ty.tcpDstPortLB_==rem_port && match2acl->aclRule->un_ty.tcpDstPortUB_==rem_port)
				return SUCCESS;
		}

		if(protocol==IPPROTO_UDP && match2acl->aclRule->ruleType_==RTL865X_ACL_UDP)
		{
			if( match2acl->aclRule->un_ty.srcIpAddr_==int_ip &&  match2acl->aclRule->un_ty.udpSrcPortLB_==int_port && match2acl->aclRule->un_ty.udpSrcPortUB_==int_port &&
				match2acl->aclRule->un_ty.dstIpAddr_==rem_ip &&  match2acl->aclRule->un_ty.udpDstPortLB_==rem_port && match2acl->aclRule->un_ty.udpDstPortUB_==rem_port)
				return SUCCESS;
		}
	}

	list_node = kmalloc(sizeof(xt_rule_to_acl_t),GFP_KERNEL);
	if(!list_node)
	{
		ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
		goto FailToAdd;
	}
	
	rule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_KERNEL);
	if(!rule)
	{
		ACL_CONTROL_DEBUG_PRK("\n!!!!!!%s(%d): No memory freed for kmalloc!!!",__FUNCTION__,__LINE__);
		goto FailToAdd;
	}
	memset(list_node, 0,sizeof(xt_rule_to_acl_t));
	memset(rule, 0,sizeof(rtl865x_AclRule_t));

	
	/* redirect to ethernet or ppppoe will cause SNAT fail. redirect to nextgop table instead */
	#if 0 /* the original */
	if (out_ifname[0] == 'p')//pppoe interface
		rule->actionType_ = RTL865X_ACL_REDIRECT_PPPOE;
	else
		rule->actionType_ = RTL865X_ACL_REDIRECT_ETHER;
	#else
	rule->actionType_ = RTL865X_ACL_DEFAULT_REDIRECT;
	#endif


	rule->pktOpApp_ 	= RTL865X_ACL_ALL_LAYER;
	rule->direction_ = RTL865X_ACL_INGRESS;
	
	if(protocol==IPPROTO_TCP)
	{
		rule->ruleType_ = RTL865X_ACL_TCP;
		rule->un_ty.srcIpAddr_		= int_ip;
		rule->un_ty.srcIpAddrMask_	= 0xFFFFFFFF;
		rule->un_ty.dstIpAddr_		= rem_ip;
		rule->un_ty.dstIpAddrMask_	= 0xFFFFFFFF;
		rule->un_ty.tcpSrcPortLB_ = int_port;
		rule->un_ty.tcpSrcPortUB_ = int_port;
		rule->un_ty.tcpDstPortLB_ = rem_port;
		rule->un_ty.tcpDstPortUB_ = rem_port; 
	}
	else
	{
		rule->ruleType_ = RTL865X_ACL_UDP;	
		rule->un_ty.srcIpAddr_		= int_ip;
		rule->un_ty.srcIpAddrMask_	= 0xFFFFFFFF;
		rule->un_ty.dstIpAddr_		= rem_ip;
		rule->un_ty.dstIpAddrMask_	= 0xFFFFFFFF;
		rule->un_ty.udpSrcPortLB_ = int_port;
		rule->un_ty.udpSrcPortUB_ = int_port;
		rule->un_ty.udpDstPortLB_ = rem_port;
		rule->un_ty.udpDstPortUB_ = rem_port;
	}
	

	redir_out_netIf = _rtl865x_getSWNetifByName(redir_out_netif);
	if(!redir_out_netIf)
		goto FailToAdd;



	/*  case 1.  pppoe wan 
				redirect to the nexthop table that pppoe table use directly  
	*/
	if(redir_out_netIf->if_type == IF_PPPOE)
	{
		int nextHopIdx;
		rtl865x_ppp_t *ppp;		
	
		/* get session ID from pppoe table */
		ppp = rtl865x_getPppByNetifName(redir_out_netIf->name);		
		if (!ppp)	
			goto FailToAdd;	
		
		nextHopIdx=rtl865x_getNxtHopIdx(NEXTHOP_DEFREDIRECT_ACL, redir_out_netIf->name, ppp->sessionId,redir_scr_ip);

		if(nextHopIdx==-1)
			goto FailToAdd;	

		rule->nexthopIdx_ = nextHopIdx;			
	}
	/*  case 2.  ether wan 
				we have to create a new nexthop table for redirecting by myself  
	*/
	else if(redir_out_netIf->if_type == IF_ETHER)
	{
		int nextHopIdx;
	


		if ((nextHopIdx = rtl865x_getNxtHopIdx(NEXTHOP_DEFREDIRECT_ACL, redir_out_netIf->name, redir_dst_ip,redir_scr_ip)) == -1)
		{
			if ( rtl865x_addNxtHop(NEXTHOP_DEFREDIRECT_ACL, NULL, redir_out_netIf->name, redir_dst_ip, redir_scr_ip) != SUCCESS) 
				goto FailToAdd;		
		}			

		nextHopIdx = rtl865x_getNxtHopIdx(NEXTHOP_DEFREDIRECT_ACL, redir_out_netIf->name, redir_dst_ip,redir_scr_ip);

		if(nextHopIdx==-1)
			goto FailToAdd;		

		rule->nexthopIdx_ = nextHopIdx;
	}
	else
		goto FailToAdd;	
	



	list_node->aclRule = rule;
	strcpy(list_node->iniface,acl_in_ifname);
	/* If acl mode is normal , take effect immediately */
	if(ACL_MODE == RTL865X_ACL_Mode_Normal)
	{
		if(rtl865x_add_acl(rule, list_node->iniface, RTL865X_ACL_USER_USED,1,0)!=SUCCESS)
			goto FailToAdd;	
	}
	list_add_tail(&list_node->list,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_REDIRECT]); 


		ACL_CONTROL_DEBUG_PRK("(%s)add into acl L34 redirect : (src ip:%u.%u.%u.%u:%u  dst ip:%u.%u.%u.%u:%u  in_if:%s  protocol:%s)\n",__func__
			,NIPQUAD(int_ip),int_port,NIPQUAD(rem_ip),rem_port,acl_in_ifname,protocol==IPPROTO_TCP?"TCP":"UDP");		
		return SUCCESS;
	

FailToAdd:	
	if(list_node)
		kfree(list_node);
	if(rule);
		kfree(rule);	
	return FAILED;
}


int rtl865x_acl_control_L34_redirect_del(__u32 src_ip,__u16 src_port,__u32 dst_ip,__u16 dst_port,__u8 protocol)
{
	xt_rule_to_acl_t *match2acl;
	xt_rule_to_acl_t *nxt;
	int findout=false;



	list_for_each_entry_safe(match2acl,nxt,&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_REDIRECT],list)
	{			
		if(protocol==IPPROTO_TCP && match2acl->aclRule->ruleType_==RTL865X_ACL_TCP)
		{
			if( match2acl->aclRule->un_ty.srcIpAddr_==src_ip &&  match2acl->aclRule->un_ty.tcpSrcPortLB_==src_port && match2acl->aclRule->un_ty.tcpSrcPortUB_==src_port &&
				match2acl->aclRule->un_ty.dstIpAddr_==dst_ip &&  match2acl->aclRule->un_ty.tcpDstPortLB_==dst_port && match2acl->aclRule->un_ty.tcpDstPortUB_==dst_port)
			{
				findout=true;
				if(rtl865x_del_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED)!=SUCCESS)
					return FAILED;
				list_del(&match2acl->list);
				kfree(match2acl->aclRule);
				kfree(match2acl);
								
			}
		}

		if(protocol==IPPROTO_UDP && match2acl->aclRule->ruleType_==RTL865X_ACL_UDP)
		{
			if( match2acl->aclRule->un_ty.srcIpAddr_==src_ip &&  match2acl->aclRule->un_ty.udpSrcPortLB_==src_port && match2acl->aclRule->un_ty.udpSrcPortUB_==src_port &&
				match2acl->aclRule->un_ty.dstIpAddr_==dst_ip &&  match2acl->aclRule->un_ty.udpDstPortLB_==dst_port && match2acl->aclRule->un_ty.udpDstPortUB_==dst_port)
			{
				findout=true;
				if(rtl865x_del_acl(match2acl->aclRule, match2acl->iniface, RTL865X_ACL_USER_USED)!=SUCCESS)
					return FAILED;
				list_del(&match2acl->list);
				kfree(match2acl->aclRule);
				kfree(match2acl);
			}
		}
		
	}

	if(findout==true)
	{
			ACL_CONTROL_DEBUG_PRK("(%s)del from acl L34 redirect : (src ip:%u.%u.%u.%u:%u  dst ip:%u.%u.%u.%u:%u   protocol:%s)\n",__func__
				,NIPQUAD(src_ip),src_port,NIPQUAD(dst_ip),dst_port,protocol==IPPROTO_TCP?"TCP":"UDP");
		}

		return SUCCESS;
}

int rtl865x_acl_control_L34_redirect_clean(void)
{

	_rtl865x_acl_control_free_chain(&ChainList[RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_REDIRECT]);

	if(_rtl865x_acl_control_rewrite_chain_to_acl()!=SUCCESS)
		return FAILED;
	else
		return SUCCESS;
}


#endif




static int _rtl865x_acl_control_free_chain(struct list_head *listHead)
{
	/*free all xtmatch rule*/
	xt_rule_to_acl_t *match2acl,*nxt;
	list_for_each_entry_safe(match2acl,nxt,listHead,list)
	{
		list_del(&match2acl->list);
		kfree(match2acl->aclRule);
		kfree(match2acl);			
	}

	return 0;
}



module_init(rtl865x_acl_control_init);
module_exit(rtl865x_acl_control_exit);


