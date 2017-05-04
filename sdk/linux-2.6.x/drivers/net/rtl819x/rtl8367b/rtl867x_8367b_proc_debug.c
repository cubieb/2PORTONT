#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <net/rtl/rtl_types.h>

#include "rtl867x_8367b_proc_debug.h"
#include "rtl867x_8367b_acl.h"
#include "rtl867x_8367b_qos.h"

#include "../AsicDriver/rtl865x_asicCom.h"
#include "../../rtl8367b/rtl8367b_asicdrv_acl_para.h"
#include "../../rtl8367b/rtl8367b_asicdrv_vlan_para.h"

#include "../../rtl8367b/port.h"
#include "../../rtl8367b/rtl8367b_reg.h"

static struct proc_dir_entry *rtl867x_8367b_proc_dir;
static struct proc_dir_entry* rtl867x_8367b_proc_diagnostic;
static struct proc_dir_entry* rtl867x_8367b_proc_acl;
static struct proc_dir_entry* rtl867x_8367b_proc_sw_acl;
static struct proc_dir_entry* rtl867x_8367b_proc_sw_qosRule;
static struct proc_dir_entry* rtl867x_8367b_proc_priority_decision;
static struct proc_dir_entry* rtl867x_8367b_proc_sw_priority_decision;
static struct proc_dir_entry* rtl867x_8367b_l2;
static struct proc_dir_entry* rtl867x_8367b_vlan;
static struct proc_dir_entry* rtl867x_8367b_svlan;
static struct proc_dir_entry* rtl867x_8367b_pvid;
static struct proc_dir_entry* rtl867x_8367b_pvid_s;
static struct proc_dir_entry* rtl867x_8367b_port_bandwidth;


static int rtl8367b_priority_decision_show(struct seq_file *s, void *v)
{
	int i,j;

	printk("====== Port's Queue number & wrights  =====\n");	
	for(i=0;i<RTK_MAX_NUM_OF_PORT;i++)
	{
		int Index;
		rtk_qos_queue_weights_t Qweights;
		rtl8367b_getAsicOutputQueueMappingIndex(i,&Index);
		printk("Port %d  : %d  => ",i,Index);	

		rtk_qos_schedulingQueue_get(i,&Qweights);
		for(j=0;j<Index;j++)
		{
			printk("Q%d (wieght:%03d)   ",j,Qweights.weights[j]);
		}		
		printk("\n");
	}


	printk("======Priotiy <----> Queue =====\n");
	for(i=1;i<=RTK_MAX_NUM_OF_QUEUE;i++)
	{
		rtk_qos_pri2queue_t Pri2qid;
		printk("Queue number : %d ",i);	
		
		if(rtk_qos_priMap_get(i,&Pri2qid)!=RT_ERR_OK)
		{
			printk("  Get acl rule error\n");
			continue;
		}		

		printk("[ ");
		for(j=0;j<RTK_MAX_NUM_OF_PRIORITY;j++)
		{
			printk("(prio %d)->Q%d  ",j,Pri2qid.pri2queue[j]);
		}
		printk("]\n");
	}	

	printk("======Port-based priority =====\n");
	for(i=0;i<RTK_MAX_NUM_OF_PORT;i++)
	{
		rtk_pri_t Int_pri;
		rtk_qos_portPri_get(i,&Int_pri);
		printk("Port %d  : %d \n",i,Int_pri);	
	}

	return 0;
}

static int rtl8367b_sw_priority_decision_show(struct seq_file *s, void *v)
{
	rtl867x_8367b_qosShowDebugInfo();
	return 0;
}

static int rtl8367b_l2_show(struct seq_file *s, void *v)
{
	/*Get All Lookup Table and Print the valid entry*/
	int i;
	rtk_l2_addr_table_t l2_entry;
	rtk_api_ret_t ret;

	for (i=1;i<=RTK_MAX_NUM_OF_LEARN_LIMIT;i++)
	{
		memset(&l2_entry,0,sizeof(rtk_l2_addr_table_t));
		l2_entry.index = i;
		ret = rtk_l2_entry_get(&l2_entry);
		if (ret==RT_ERR_OK)
		{
			if(l2_entry.is_ipmul)
			{
				printk("\r\nIndex SourceIP DestinationIP MemberPort State\n");
				printk("%4d ", l2_entry.index);
				printk("%u.%u.%u.%u ",NIPQUAD(l2_entry.sip));
				printk("%u.%u.%u.%u ",NIPQUAD(l2_entry.dip));
				printk("0x%X  ",l2_entry.portmask);
				printk("%s\n",(l2_entry.is_static? "Static" : "Auto"));
			}
			else if(l2_entry.mac.octet[0]&0x01)
			{
				printk("%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X 0x%X %-4d %-4s %-5s %s %d\n",
				l2_entry.index,
				l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],\
				l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],\
				l2_entry.portmask, l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),\
				(l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),\
				l2_entry.age);
			}
			else if((l2_entry.age!=0)||(l2_entry.is_static==1))
			{
				printk("[%4d] mac addr(%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x)  type(%3s)  cvid(%4d)  fid(%4d) efid(%4d) mbr(0x%x) auth(%s) block(%s) static(%s) age(%2d)\n",
				l2_entry.index,
				l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],\
				l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],\
				(l2_entry.ivl ? "IVL" : "SVL"), l2_entry.cvid , l2_entry.fid, l2_entry.efid,\
				l2_entry.portmask, (l2_entry.auth ? "Y" : "N"),\
				(l2_entry.sa_block? "Y" : "N"), (l2_entry.is_static? "Y" : "N"),\
				l2_entry.age);
			}
		}
	}
	return 0;
}

static int rtl8367b_vlan_show(struct seq_file *s, void *v)
{
	rtk_vlan_t vid;
	rtk_portmask_t mbrmsk, untagmsk;
	rtk_fid_t fid;

	for(vid=0;vid<=RL6000_VLAN_NUM;vid++)
 	{ 	
 		rtk_vlan_get(vid,&mbrmsk,&untagmsk,&fid); 	
 		printk("vid = %04d   members:0x%03X  untag:0x%03X  FID:%d\n",vid,mbrmsk.bits[0],untagmsk.bits[0],fid);
 	}
	
	return 0;
}

static int rtl8367b_svlan_show(struct seq_file *s, void *v)
{
	rtk_vlan_t svid;
	rtk_svlan_memberCfg_t Svlan_cfg;

	for(svid=0;svid<=RL6000_VLAN_NUM;svid++)
 	{ 	
 		if(rtk_svlan_memberPortEntry_get(svid,&Svlan_cfg)==RT_ERR_OK) 	
 			printk("svid = %04d   members:0x%03X  untag:0x%03X \n",svid,Svlan_cfg.memberport,Svlan_cfg.untagport);
 	}

	return 0;
}

static int rtl8367b_pvid_show(struct seq_file *s, void *v)
{	
	rtk_vlan_t vid;
	rtk_pri_t priority;
	rtk_port_t port;
	for(port=0;port<=6;port++)
	{
		rtk_vlan_portPvid_get(port,&vid,&priority);
		printk("Port %d  =>  pvid : %3d  priority:%d \n",port,vid,priority);
	}	
	return 0;
}

static int rtl8367b_pvid_s_show(struct seq_file *s, void *v)
{
	rtk_vlan_t svid;
	rtk_port_t port;
	for(port=0;port<=6;port++)
	{
		rtk_svlan_defaultSvlan_get(port, &svid);
		printk("Port %d  =>  spvid : %3d   \n",port,svid);
	}
	return 0;
}

static int rtl8367b_port_bandwidth_show(struct seq_file *s, void *v)
{
	rtk_port_t port;
	rtk_rate_t pRate; 
	rtk_enable_t pIfg_includ;
	
	for(port=0;port<=6;port++)
	{
		rtk_rate_egrBandwidthCtrlRate_get(port, &pRate,&pIfg_includ);
		printk("Port %d  =>  Rate : %7d kbps    Including IFG : %s \n",port,pRate,(pIfg_includ==1)?"Y":"N");
	}
	return 0;
}

static int rtl8367b_diagnostic_show(struct seq_file *s, void *v)
{
	rtk_port_t port;
	rtk_queue_num_t queue;
	ret_t retVal;
	rtk_uint32 regValue;
	extern ret_t rtl8367b_setAsicReg(rtk_uint32 reg, rtk_uint32 value);
	extern ret_t rtl8367b_getAsicReg(rtk_uint32 reg, rtk_uint32 *pValue);

	printk(" == RTL8367b diagnostic == \n");
	for(port=0;port<=RL6000_CPU_PORT;port++)
	{
		printk(" --   port %d --  \n",port);
		retVal = rtl8367b_setAsicReg(RTL8367B_REG_FLOWCTRL_DEBUG_CTRL0, port&RTL8367B_FLOWCTRL_DEBUG_CTRL0_MASK);		
    	if(retVal != RT_ERR_OK)
    	{
			printk("read reg error\n");
			return 0;
    	}
		
		printk("\t Port descriptor : ");
		retVal = rtl8367b_getAsicReg(RTL8367B_REG_FLOWCTRL_PORT_PAGE_COUNT, &regValue);		
    	if(retVal != RT_ERR_OK)
    	{
			printk("read reg error\n");
			return 0;
    	}
		printk("Current: %5d  ",regValue);
		retVal = rtl8367b_getAsicReg(RTL8367B_REG_FLOWCTRL_PORT_MAX_PAGE_COUNT, &regValue);		
    	if(retVal != RT_ERR_OK)
    	{
			printk("read reg error\n");
			return 0;
    	}		
		printk("Max: %5d  \n",regValue);			


		for(queue=0;queue<RTK_MAX_NUM_OF_QUEUE;queue++)
		{
			printk("\t Queue %d used descriptor: ",queue);			
			retVal = rtl8367b_getAsicReg((RTL8367B_REG_FLOWCTRL_QUEUE0_PAGE_COUNT+queue), &regValue);		
	    	if(retVal != RT_ERR_OK)
	    	{
				printk("read reg error\n");
				return 0;
	    	}	
			printk("Current: %5d  ",regValue);
			retVal = rtl8367b_getAsicReg((RTL8367B_REG_FLOWCTRL_QUEUE0_MAX_PAGE_COUNT+queue), &regValue);		
	    	if(retVal != RT_ERR_OK)
	    	{
				printk("read reg error\n");
				return 0;
	    	}	
			printk("Max: %5d  \n",regValue);	
		}	
	}	
	return 0;
}

static int rtl8367b_aclrule_show(struct seq_file *s, void *v)
{
	int i;
	rtk_filter_cfg_raw_t pFilter_cfg;
	rtk_filter_action_t pAction;
	//rtk_filter_template_t aclTemplate;

	char *ip_range_type_string[]={
    "IPRANGE_UNUSED",
    "IPRANGE_IPV4_SIP",
    "IPRANGE_IPV4_DIP",
    "IPRANGE_IPV6_SIP",
    "IPRANGE_IPV6_DIP",
    "IPRANGE_END"
	};

	char *port_range_type_string[]={
    "PORTRANGE_UNUSED",
    "PORTRANGE_SPORT",
    "PORTRANGE_DPORT",
    "PORTRANGE_END"
	};

	#if 0
	printk("======RL6000 ACLTemplate=====\n");
	for(i=0;i<RTK_MAX_NUM_OF_FILTER_TYPE;i++)
	{	
		aclTemplate.index = i;
		rtk_filter_igrAcl_template_get(&aclTemplate);
		
		printk("[Template %d] : \n",i);
		for(j=0;j<RTK_MAX_NUM_OF_FILTER_FIELD;j++)
		{
			printk("  (%d) %s\n",j,fieldtype_string[(unsigned int)aclTemplate.fieldType]);
		}
	}
	#endif

	printk("======RL6000 ACL=====\n");
	for(i=0;i<RL6000_ACLRULENUM;i++)
	{
		
		if(rtk_filter_igrAcl_cfg_get(i,&pFilter_cfg,&pAction)!=RT_ERR_OK)
		{
			printk("[%2d]  Get acl rule error\n",i);
			continue;
		}

		if(!pFilter_cfg.valid)
			continue;

		printk("--Rule %02d--\n",i);
		rtl867x_rtl8367b_show_aclrule_raw(&pFilter_cfg, &pAction);
		printk("\n");
	}

	printk("======ip range=====\n");
	for(i=0;i<RL6000_ACLRANGERULENUM;i++)
	{
		rtk_filter_iprange_t Type;
		unsigned int UpperIp;
		unsigned int LowerIp;	
		
	
		if(rtk_filter_iprange_get(i,&Type,&UpperIp,&LowerIp)!=RT_ERR_OK)
		{
			printk("[%2d]  Get rule error\n",i);
			continue;
		}

		if(Type==0)
			continue;

		
		printk("[%2d]   Type:%10s  ip_start:0x%8X    ip_end:0x%8X\n",i,ip_range_type_string[Type],LowerIp,UpperIp);
	}

	printk("======port range=====\n");
	for(i=0;i<RL6000_ACLRANGERULENUM;i++)
	{
		rtk_filter_portrange_t Type;
		rtk_uint32 Upperport;
		rtk_uint32 Lowerport;		
		
		if(rtk_filter_portrange_get(i,&Type,&Upperport,&Lowerport)!=RT_ERR_OK)
		{
			printk("[%2d]  Get rule error\n",i);
			continue;
		}

		if(Type==0)
			continue;

		
		printk("[%2d]   Type:%10s  port_start:%04d    port_end:%04d\n",i,port_range_type_string[Type],Lowerport,Upperport);
	}

	return 0;
}


static int rtl8367b_sw_aclrule_show(struct seq_file *s, void *v)
{
	rtl867x_rtl8367b_show_aclchain();
	return 0;
}

static int rtl8367b_sw_qosRule_show(struct seq_file *s, void *v)
{
	rtl867x_8367b_qosShowSwRule();
	return 0;
}

int rtl8367b_diagnostic_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_diagnostic_show, NULL));
}

int rtl8367b_aclrule_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_aclrule_show, NULL));
}

int rtl8367b_sw_aclrule_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_sw_aclrule_show, NULL));
}

int rtl8367b_sw_qosRule_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_sw_qosRule_show, NULL));
}

int rtl8367b_priority_decision_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_priority_decision_show, NULL));
}

int rtl8367b_sw_priority_decision_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_sw_priority_decision_show, NULL));
}

int rtl8367b_l2_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_l2_show, NULL));
}

int rtl8367b_vlan_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_vlan_show, NULL));
}

int rtl8367b_svlan_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_svlan_show, NULL));
}

int rtl8367b_pvid_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_pvid_show, NULL));
}

int rtl8367b_pvid_s_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_pvid_s_show, NULL));
}

int rtl8367b_port_bandwidth_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl8367b_port_bandwidth_show, NULL));
}

struct file_operations rtl8367b_diagnostic_single_seq_file_operations = {
        .open           = rtl8367b_diagnostic_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_aclrule_single_seq_file_operations = {
        .open           = rtl8367b_aclrule_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_sw_aclrule_single_seq_file_operations = {
        .open           = rtl8367b_sw_aclrule_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_sw_qosRule_single_seq_file_operations = {
        .open           = rtl8367b_sw_qosRule_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_priority_decision_single_seq_file_operations = {
        .open           = rtl8367b_priority_decision_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_sw_priority_decision_single_seq_file_operations = {
        .open           = rtl8367b_sw_priority_decision_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_l2_single_seq_file_operations = {
        .open           = rtl8367b_l2_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_vlan_single_seq_file_operations = {
        .open           = rtl8367b_vlan_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_svlan_single_seq_file_operations = {
        .open           = rtl8367b_svlan_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_pvid_single_seq_file_operations = {
        .open           = rtl8367b_pvid_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_pvid_s_single_seq_file_operations = {
        .open           = rtl8367b_pvid_s_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations rtl8367b_port_bandwidth_single_seq_file_operations = {
        .open           = rtl8367b_port_bandwidth_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};


static int _rtl867x_8367b_proc_cleanup(void)
{	
	if(rtl867x_8367b_proc_dir)
	{
		if(rtl867x_8367b_proc_diagnostic!=NULL)
		{
			remove_proc_entry("rtl8367b_diagnostic", rtl867x_8367b_proc_dir);
		}
		if(rtl867x_8367b_proc_acl!=NULL)
		{
			remove_proc_entry("rtl8367b_acl", rtl867x_8367b_proc_dir);
		}

		if(rtl867x_8367b_proc_sw_acl!=NULL)
		{
			remove_proc_entry("rtl8367b_sw_acl", rtl867x_8367b_proc_dir);
		}
		if(rtl867x_8367b_proc_sw_qosRule!=NULL)
		{
			remove_proc_entry("rtl8367b_sw_qosRule", rtl867x_8367b_proc_dir);
		}
	
		if(rtl867x_8367b_proc_priority_decision!=NULL)
		{
			remove_proc_entry("rtl8367b_priority_decision", rtl867x_8367b_proc_dir);
		}	
		if(rtl867x_8367b_proc_sw_priority_decision!=NULL)
		{
			remove_proc_entry("sw_rtl8367b_priority_decision", rtl867x_8367b_proc_dir);
		}	

		if(rtl867x_8367b_l2!=NULL)
		{
			remove_proc_entry("rtl8367b_l2", rtl867x_8367b_proc_dir);
		}

		if(rtl867x_8367b_vlan!=NULL)
		{
			remove_proc_entry("rtl8367b_vlan", rtl867x_8367b_proc_dir);
		}

		if(rtl867x_8367b_svlan!=NULL)
		{
			remove_proc_entry("rtl8367b_svlan", rtl867x_8367b_proc_dir);
		}

		if(rtl867x_8367b_pvid!=NULL)
		{
			remove_proc_entry("rtl8367b_pvid", rtl867x_8367b_proc_dir);
		}

		if(rtl867x_8367b_pvid_s!=NULL)
		{
			remove_proc_entry("rtl8367b_pvid_s", rtl867x_8367b_proc_dir);
		}
		if(rtl867x_8367b_port_bandwidth!=NULL)
		{
			remove_proc_entry("rtl867x_8367b_port_bandwidth", rtl867x_8367b_proc_dir);
		}
	}	
	return SUCCESS;	
}

static int __init rtl867x_8367b_proc_init(void)
{
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B) 
	{
		/*  create proc  */		
		if(rtl867x_8367b_proc_dir==NULL)
			rtl867x_8367b_proc_dir = proc_mkdir(rtl867x_8367b_proc_DIR_NAME,NULL);
		
		if(rtl867x_8367b_proc_dir)
		{			
			
			rtl867x_8367b_proc_diagnostic = create_proc_entry("rtl8367b_diagnostic",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_proc_diagnostic != NULL)				
				rtl867x_8367b_proc_diagnostic->proc_fops = &rtl8367b_diagnostic_single_seq_file_operations;				
			else		
				goto init_failed;		
					
			rtl867x_8367b_proc_acl = create_proc_entry("rtl8367b_acl",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_proc_acl != NULL)				
				rtl867x_8367b_proc_acl->proc_fops = &rtl8367b_aclrule_single_seq_file_operations;				
			else		
				goto init_failed;

			rtl867x_8367b_proc_sw_acl = create_proc_entry("rtl8367b_sw_acl",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_proc_sw_acl != NULL)				
				rtl867x_8367b_proc_sw_acl->proc_fops = &rtl8367b_sw_aclrule_single_seq_file_operations;				
			else
				goto init_failed;		
					
			rtl867x_8367b_proc_sw_qosRule = create_proc_entry("rtl8367b_sw_qosRule",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_proc_sw_qosRule != NULL)				
				rtl867x_8367b_proc_sw_qosRule->proc_fops = &rtl8367b_sw_qosRule_single_seq_file_operations;				
			else
				goto init_failed;	
			
			rtl867x_8367b_proc_priority_decision = create_proc_entry("rtl8367b_priority_decision",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_proc_priority_decision != NULL)					
				rtl867x_8367b_proc_priority_decision->proc_fops = &rtl8367b_priority_decision_single_seq_file_operations;	
			else
				goto init_failed;	
				
			rtl867x_8367b_proc_sw_priority_decision = create_proc_entry("rtl8367b_sw_priority_decision",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_proc_sw_priority_decision != NULL)					
				rtl867x_8367b_proc_sw_priority_decision->proc_fops = &rtl8367b_sw_priority_decision_single_seq_file_operations;	
			else
				goto init_failed;	

			rtl867x_8367b_l2 = create_proc_entry("rtl8367b_l2",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_l2 != NULL)					
				rtl867x_8367b_l2->proc_fops = &rtl8367b_l2_single_seq_file_operations;					
			else	
				goto init_failed;

			rtl867x_8367b_vlan = create_proc_entry("rtl8367b_vlan",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_vlan != NULL)					
				rtl867x_8367b_vlan->proc_fops = &rtl8367b_vlan_single_seq_file_operations;					
			else	
				goto init_failed;

			rtl867x_8367b_svlan = create_proc_entry("rtl8367b_svlan",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_svlan != NULL)					
				rtl867x_8367b_svlan->proc_fops = &rtl8367b_svlan_single_seq_file_operations;					
			else	
				goto init_failed;

			rtl867x_8367b_pvid = create_proc_entry("rtl8367b_pvid",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_pvid != NULL)					
				rtl867x_8367b_pvid->proc_fops = &rtl8367b_pvid_single_seq_file_operations;					
			else	
				goto init_failed;
			
			rtl867x_8367b_pvid_s = create_proc_entry("rtl8367b_pvid_s",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_pvid_s != NULL)					
				rtl867x_8367b_pvid_s->proc_fops = &rtl8367b_pvid_s_single_seq_file_operations;					
			else	
				goto init_failed;

			rtl867x_8367b_port_bandwidth = create_proc_entry("rtl8367b_port_bandwidth",0,rtl867x_8367b_proc_dir);
			if(rtl867x_8367b_port_bandwidth != NULL)					
				rtl867x_8367b_port_bandwidth->proc_fops = &rtl8367b_port_bandwidth_single_seq_file_operations;					
			else	
				goto init_failed;
		}
	}

	return SUCCESS;
	
init_failed:
	_rtl867x_8367b_proc_cleanup();
	
	return FAILED;	
}

static void __exit rtl867x_8367b_proc_exit(void)
{
	_rtl867x_8367b_proc_cleanup();
} 

module_init(rtl867x_8367b_proc_init);
module_exit(rtl867x_8367b_proc_exit);


