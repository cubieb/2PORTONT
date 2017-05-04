/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.                                                    
* 
* Program : just for driver debug
* Abstract :                                                           
* Author : Hyking Liu (Hyking_liu@realsil.com.tw)               
* -------------------------------------------------------
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "rtl865x_proc_debug.h"
#include <linux/kernel.h>
#include <linux/delay.h>

#include <net/rtl/rtl865x_netif.h>
#include "common/rtl865x_netif_local.h"
#include "common/rtl865x_eventMgr.h"
#include "common/rtl_utils.h"

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include <net/rtl/rtl865x_ppp.h>
#include "l3Driver/rtl865x_ppp_local.h"
#include "l3Driver/rtl865x_route.h"
#include "l3Driver/rtl865x_nexthop.h"
#endif
#include <net/rtl/rtl865x_arp_api.h>
#include "testModel/testModel.h"



#include "AsicDriver/rtl865x_asicBasic.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"	
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
#include "AsicDriver/rtl865x_asicL3.h"
#endif
#if defined(CONFIG_RTL_LAYERED_ASIC_DRIVER_L4)
#include "AsicDriver/rtl865x_asicL4.h"
#endif

#include "AsicDriver/rtl865xc_asicregs.h"
#include "AsicDriver/rtl865xC_hs.h"
#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <net/rtl/rtl865x_igmpsnooping.h>
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
#include <net/rtl/rtl865x_outputQueue.h>
#endif



#include "rtl865xc_swNic.h"
#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if.h>
#include <net/rtl/rtl_nic.h>
#endif

#ifdef CONFIG_RTL_8367B
#include "../rtl8367b/port.h"
#endif
struct proc_dir_entry *rtl865x_proc_dir=NULL;
static struct proc_dir_entry *vlan_entry,*netif_entry,*l2_entry, *l2_all_entry,*arp_entry,*testModel_entry,
		*nexthop_entry,*l3_entry,*ip_entry,*pppoe_entry,*napt_entry,
		*acl_entry,*acl_all_entry,*storm_control,*dump_trap2CPU_pkt,*dump_swNicTxRx_pkt,
#ifdef CONFIG_RTL_MULTI_ETH_WAN
		*sw_netif, *sw_l3,
#endif
		*sw_arp_entry,*sw_nexthop_entry,*sw_extip_entry,
#if defined(RTL_DEBUG_NIC_SKB_BUFFER)
		*nic_skb_buff, 
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER
		*acl_chains_entry, 
#endif
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
		*qos_rule_entry, *sw_priority_decision_entry,
#endif
		*rxRing_entry, *txRing_entry, *mbuf_entry, 
		*hs_entry, *pvid_entry, *mirrorPort_entry, 
		*mem_entry, *diagnostic_entry, 
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
		*sw_napt_entry, 
#endif
		*port_bandwidth_entry, *queue_bandwidth_entry, 
		*port_status_entry, *priority_decision_entry,*hwMCast_entry,
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)		
		*swMCast_entry,
#endif		
#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
		*rateLimit_entry,
#endif
#if defined(CONFIG_RTL_8367B)
		*mib_8367b_entry, *rtl8367b_reg,
#endif
		*asicCnt_entry,*phyReg_entry;

static uint32 queue_bandwidth_record_portmask = 0;

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
int32 rtl_dumpSwMulticastInfo(void);
#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL865X_EVENT_PROC_DEBUG)
static struct proc_dir_entry *eventMgr_entry;
#endif
/* //move to net/bridge/br.c
#if defined (CONFIG_RTL_IGMP_SNOOPING) 
static struct proc_dir_entry *igmp_entry;
#endif
*/
#define	PROC_READ_RETURN_VALUE		0

extern int32 rtl865x_sw_napt_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data );
extern int32  rtl865x_sw_napt_proc_write( struct file *filp, const char *buff,unsigned long len, void *data );


#ifdef CONFIG_RTL_LAYERED_DRIVER
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
static struct proc_dir_entry *sw_l2_entry;
extern int32 rtl865x_sw_l2_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data );
extern int32 rtl865x_sw_l2_proc_write( struct file *filp, const char *buff,unsigned long len, void *data );
#endif
#endif

#ifdef CONFIG_RTL865X_ROMEPERF
static struct proc_dir_entry *perf_dump;
extern int32 rtl865x_perf_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data );
extern int32 rtl865x_perf_proc_write(struct file *file, const char *buffer, unsigned long count, void *data);
#endif


#if 0
int vlan_show(void)
{
		int i, j;
		
		for ( i = 0; i < RTL865XC_VLAN_NUMBER; i++ )
		{
			rtl865x_tblAsicDrv_vlanParam_t vlan;

			if ( rtl8651_getAsicVlan( i, &vlan ) == FAILED )
				continue;
			
			printk("  VID[%d] ", i);
			printk("\n\tmember ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					printk( "%d ", j);
			}

			printk("\n\tUntag member ports:"); 			

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if( vlan.untagPortMask & ( 1 << j ) )
					printk("%d ", j);
			}

			printk("\n\tFID:\t%d\n",vlan.fid);
		}

		return SUCCESS;

}
#endif

//static int32 vlan_read( char *page, char **start, off_t off, int count, int *eof, void *data )
static int32 vlan_single_show(struct seq_file *s, void *v)

{
	seq_printf(s,"%s\n", "ASIC VLAN Table:");

	{
		int i, j;
		
		for ( i = 0; i < RTL865XC_VLAN_NUMBER; i++ )
		{
			rtl865x_tblAsicDrv_vlanParam_t vlan;

#ifndef CONFIG_RTL_8196D
			if ( rtl8651_getAsicVlan( i, &vlan ) == FAILED )
				continue;
			
			seq_printf(s, "  VID[%d] ", i);
#else
			rtl865xc_tblAsic_vlanTable_t entry;
			_rtl8651_readAsicEntry(TYPE_VLAN_TABLE, i, &entry);
			if ((entry.extMemberPort | entry.memberPort) == 0)
				continue;
			vlan.memberPortMask = (entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;
			vlan.untagPortMask = (entry.extEgressUntag<<RTL8651_PORT_NUMBER) |entry.egressUntag;
			vlan.fid = entry.fid;
			seq_printf(s, "  VID[%d] ", entry.vid);
#endif
			seq_printf(s, "\n\tmember ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					seq_printf(s,"%d ", j);
			}

			seq_printf(s,"\n\tUntag member ports:");				

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if( vlan.untagPortMask & ( 1 << j ) )
					seq_printf(s,"%d ", j);
			}

			seq_printf(s,"\n\tFID:\t%d\n",vlan.fid);
		}

	}	

	return 0;
}

#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)
extern int switch_iram(uint32 addr);
#endif

int vlan_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, vlan_single_show, NULL));
}



struct file_operations vlan_single_seq_file_operations = {
        .open           = vlan_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};


#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#define RTL8651_ACLTBL_DROP_ALL RTL865X_ACLTBL_DROP_ALL
#define RTL8651_ACLTBL_PERMIT_ALL RTL865X_ACLTBL_PERMIT_ALL
#define RTL8651_ACLTBL_ALL_TO_CPU RTL865X_ACLTBL_ALL_TO_CPU
#endif

static int dump_trap2CPU_pkt_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
      int len;
	  extern int DumpTrapCPUpkt_debug;
      len = sprintf(page, "Dump pkt trapped to CPU  : %s\n", DumpTrapCPUpkt_debug?"Enable":"Disable");

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int dump_trap2CPU_pkt_write(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{

    unsigned char chartmp; 
	extern int DumpTrapCPUpkt_debug;
	  
    if (count > 1) {	//call from shell
      	if (buffer && !copy_from_user(&chartmp, buffer, 1)) 
		{
	    	DumpTrapCPUpkt_debug = chartmp - '0';				
			printk("Dump pkt trapped to CPU  : %s\n", DumpTrapCPUpkt_debug?"Enable":"Disable");	
      	}
			
	}
	else{	
		printk("write fail\n");		
		return -EFAULT;
	}
	return count;
}

static int dump_swNicTxRx_pkt_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
      int len;
	  extern int DumpSwNicTxRx_debug;
      len = sprintf(page, "Dump swNIC RxTx Info  : %s\n", DumpSwNicTxRx_debug?"Enable":"Disable");

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int dump_swNicTxRx_pkt_write(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{

    unsigned char chartmp; 
	extern int DumpSwNicTxRx_debug;
	  
    if (count > 1) {	//call from shell
      	if (buffer && !copy_from_user(&chartmp, buffer, 1)) 
		{
	    	DumpSwNicTxRx_debug = chartmp - '0';				
			printk("Dump swNIC RxTx Info  : %s\n", DumpSwNicTxRx_debug?"Enable":"Disable");	
      	}
			
	}
	else{	
		printk("write fail\n");		
		return -EFAULT;
	}
	return count;
}


static int32 netif_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	int8	*pst[] = { "DIS/BLK",  "LIS", "LRN", "FWD" };
	uint8 *mac;
	int32 i, j;

	len = sprintf(page, "%s\n", "ASIC Network Interface Table:");
	for ( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
	{
		rtl865x_tblAsicDrv_intfParam_t intf;
		rtl865x_tblAsicDrv_vlanParam_t vlan;

		if ( rtl8651_getAsicNetInterface( i, &intf ) == FAILED )
			continue;

		if ( intf.valid )
		{
			mac = (uint8 *)&intf.macAddr.octet[0];
			len += sprintf(page+len,"[%d]  VID[%d] %x:%x:%x:%x:%x:%x", 
				i, intf.vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			len += sprintf(page+len,"  Routing %s \n",
				intf.enableRoute==TRUE? "enabled": "disabled" );

			len += sprintf(page+len,"      ingress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.inAclStart )
			{
				if ( intf.inAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					len += sprintf(page+len,"permit all,");
				if ( intf.inAclStart == RTL8651_ACLTBL_ALL_TO_CPU )
					len += sprintf(page+len,"all to cpu,");
				if ( intf.inAclStart == RTL8651_ACLTBL_DROP_ALL )
					len += sprintf(page+len,"drop all,");
			}
			else
				len += sprintf(page+len,"ACL %d-%d, ", intf.inAclStart, intf.inAclEnd);

			len += sprintf(page+len,"  egress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.outAclStart )
			{
				if ( intf.outAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					len += sprintf(page+len,"permit all,");
				if ( intf.outAclStart==RTL8651_ACLTBL_ALL_TO_CPU )
					len += sprintf(page+len,"all to cpu,");
				if ( intf.outAclStart==RTL8651_ACLTBL_DROP_ALL )
					len += sprintf(page+len,"drop all,");
			}
			else
				len += sprintf(page+len,"ACL %d-%d, ", intf.outAclStart, intf.outAclEnd);

			len += sprintf(page+len, "\n      %d MAC Addresses, MTU %d Bytes\n", intf.macAddrNumber, intf.mtu);

			rtl8651_getAsicVlan( intf.vid, &vlan );

			len += sprintf(page+len,"\n      Untag member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.untagPortMask & ( 1 << j ) )
					len += sprintf(page+len,"%d ", j);
			}
			len += sprintf(page+len, "\n      Active member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					len += sprintf(page+len, "%d ", j);
			}
			
			len += sprintf(page+len, "\n      Port state(");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( ( vlan.memberPortMask & ( 1 << j ) ) == 0 )
					continue;
				if ((( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET ) > 4 )
					len += sprintf(page+len, "--- ");
				else
					len += sprintf(page+len, "%d:%s ", j, pst[(( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET )]);

			}
			len += sprintf(page+len, ")\n");

			len += sprintf(page+len, "\n");
		}

	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 netif_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#ifdef CONFIG_RTL_MULTI_ETH_WAN
static int32 sw_netif_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	rtl865x_netif_local_t *tbl;
	int len;
	int i;
	extern rtl865x_netif_local_t * getNetifTbl(void);

	tbl = getNetifTbl();
	len = sprintf(page, "%s\n", "SW Network Interface Table:");
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{
		if (tbl[i].valid) {
			len += sprintf(page+len,"%3d vid %-4d %-6s %-7s ", i, tbl[i].vid, (tbl[i].if_type==IF_ETHER)?"ETH":"PPPOE", 
				tbl[i].name);
			if (tbl[i].is_slave)
				len += sprintf(page+len, "SLAVE dev, MASTER is %s. ", tbl[i].master?(char *)(tbl[i].master->name):"NULL");
			else
				len += sprintf(page+len, "MASTER dev. (asic idx:%d)",tbl[i].asicIdx);
			if (tbl[i].is_wan)
				len += sprintf(page+len, "WAN  ");
			else
				len += sprintf(page+len, "LAN  ");
			if (tbl[i].is_defaultWan)
				len += sprintf(page+len, "(default WAN)  ");
			len += sprintf(page+len, "ACL chain list head addr:%p  "
				,tbl[i].NetifAclChainListHEAD_ptr);			
			if (tbl[i].enableRoute)
				len += sprintf(page+len, "enableRoute : TRUE \n\n");
			else
				len += sprintf(page+len, "enableRoute : FALSE \n\n");
		}
	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}


static int32 sw_netif_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
#endif

#if 0
static int32 acl_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	int8 *actionT[] = { "permit", "redirect to ether", "drop", "to cpu", "legacy drop", 
					"drop for log", "mirror", "redirect to pppoe", "default redirect", "mirror keep match", 
					"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","priority "
					};
#ifdef CONFIG_RTL_LAYERED_DRIVER
	rtl865x_AclRule_t asic_acl;
#else
	_rtl8651_tblDrvAclRule_t asic_acl;
#endif
	rtl865x_tblAsicDrv_intfParam_t asic_intf;
	uint32 acl_start, acl_end;

	uint16 vid;
	int8 outRule;


	
	len = sprintf(page, "%s\n", "ASIC ACL Table:");
	for(vid=0; vid<8; vid++ ) 
	{
		/* Read VLAN Table */
		if (rtl8651_getAsicNetInterface(vid, &asic_intf) == FAILED)
			continue;
		if (asic_intf.valid==FALSE)
			continue;

		outRule = FALSE;
		acl_start = asic_intf.inAclStart; acl_end = asic_intf.inAclEnd;
		len += sprintf(page+len, "\nacl_start(%d), acl_end(%d)", acl_start, acl_end);
	again:
		if (outRule == FALSE)
			len += sprintf(page+len, "\n<<Ingress Rule for Netif  %d: (VID %d)>>\n", vid,asic_intf.vid);
		else
			len += sprintf(page+len, "\n<<Egress Rule for Netif %d (VID %d)>>:\n", vid,asic_intf.vid);

#ifdef CONFIG_RTL_LAYERED_DRIVER

		for(; acl_start<= acl_end;acl_start++)
		{
			if ( _rtl865x_getAclFromAsic(acl_start, &asic_acl) == FAILED)
				rtlglue_printf("=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);
		
			switch(asic_acl.ruleType_)
			{
			case RTL865X_ACL_MAC:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Ethernet", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);
				
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
				break;

			case RTL865X_ACL_IP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);
				
				len += sprintf(page+len, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				len += sprintf(page+len, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_); 
					break;
					
			case RTL865X_ACL_IP_RANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP Range", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
						);
				len += sprintf(page+len, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
					len += sprintf(page+len, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_); 
					break;			
			case RTL865X_ACL_ICMP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_, 
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL865X_ACL_ICMP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_, 
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL865X_ACL_IGMP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;


			case RTL865X_ACL_IGMP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;

			case RTL865X_ACL_TCP:
					len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP", actionT[asic_acl.actionType_]);
					len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
				len += sprintf(page+len, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
						);
				break;
			case RTL865X_ACL_TCP_IPRANGE:
					len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP IP RANGE", actionT[asic_acl.actionType_]);
					len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
					len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
					len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
					len += sprintf(page+len, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
				break;

			case RTL865X_ACL_UDP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start,"UDP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
						);
				break;				
			case RTL865X_ACL_UDP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
				break;				

			
			case RTL865X_ACL_SRCFILTER:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2], 
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_SRCFILTER_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter(IP RANGE)", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2], 
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_DSTFILTER:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2], 
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_, 
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)), 
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
						);
				break;
			case RTL865X_ACL_DSTFILTER_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter(IP Range)", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2], 
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_, 
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)), 
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
				break;

				default:
					len += sprintf(page+len, "asic_acl.ruleType_(0x%x)\n", asic_acl.ruleType_);

		}


				/* Action type */
		switch (asic_acl.actionType_) 
		{

			case RTL865X_ACL_PERMIT:
			case RTL865X_ACL_REDIRECT_ETHER:
			case RTL865X_ACL_DROP:
			case RTL865X_ACL_TOCPU:
			case RTL865X_ACL_LEGACY_DROP:
			case RTL865X_ACL_DROPCPU_LOG:
			case RTL865X_ACL_MIRROR:
			case RTL865X_ACL_REDIRECT_PPPOE:
			case RTL865X_ACL_MIRROR_KEEP_MATCH:
				len += sprintf(page+len, "\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", asic_acl.netifIdx_, asic_acl.pppoeIdx_, asic_acl.L2Idx_);
				break;

			case RTL865X_ACL_PRIORITY: 
				len += sprintf(page+len, "\tprioirty: %d   ", asic_acl.priority_) ;
				break;
				
			case RTL865X_ACL_DEFAULT_REDIRECT:
				len += sprintf(page+len,"\tnextHop:%d  ", asic_acl.nexthopIdx_);
				break;

			case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
			case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
				len += sprintf(page+len, "\tratelimitIdx: %d  ", asic_acl.ratelimtIdx_);
				break;
			default: 
				;
			
			}
			len += sprintf(page+len, "pktOpApp: %d\n", asic_acl.pktOpApp_);
			
		}
#else
		for( ; acl_start<=acl_end; acl_start++) 
		{
			if (rtl8651_getAsicAclRule(acl_start, &asic_acl) == FAILED)
				rtlglue_printf("=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);
		
			switch(asic_acl.ruleType_)
			{
			case RTL8651_ACL_MAC:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Ethernet", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);
				
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
				break;

			case RTL8651_ACL_IP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);
				
				len += sprintf(page+len, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				len += sprintf(page+len, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_); 
					break;
					
			case RTL8652_ACL_IP_RANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP Range", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
						);
				len += sprintf(page+len, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
					len += sprintf(page+len, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_); 
					break;			
			case RTL8651_ACL_ICMP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_, 
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL8652_ACL_ICMP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_, 
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL8651_ACL_IGMP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;


			case RTL8652_ACL_IGMP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;

			case RTL8651_ACL_TCP:
					len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP", actionT[asic_acl.actionType_]);
					len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
				len += sprintf(page+len, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
						);
				break;
			case RTL8652_ACL_TCP_IPRANGE:
					len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP IP RANGE", actionT[asic_acl.actionType_]);
					len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
					len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
					len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
					len += sprintf(page+len, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
				break;

			case RTL8651_ACL_UDP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start,"UDP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
						);
				break;				
			case RTL8652_ACL_UDP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
				break;				

			case RTL8651_ACL_IFSEL:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tgidxSel: %x\n", asic_acl.gidxSel_);
				break;
			case RTL8651_ACL_SRCFILTER:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2], 
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL8652_ACL_SRCFILTER_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter(IP RANGE)", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2], 
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL8651_ACL_DSTFILTER:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2], 
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_, 
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)), 
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
						);
				break;
			case RTL8652_ACL_DSTFILTER_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter(IP Range)", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2], 
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_, 
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)), 
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
				break;

				default:
					len += sprintf(page+len, "asic_acl.ruleType_(0x%x)\n", asic_acl.ruleType_);

		}


				/* Action type */
		switch (asic_acl.actionType_) 
		{

			case RTL8651_ACL_PERMIT: /* 0x00 */
			case RTL8651_ACL_REDIRECT: /* 0x01 */
			case RTL8651_ACL_CPU: /* 0x03 */
			case RTL8651_ACL_DROP: /* 0x02, 0x04 */
			case RTL8651_ACL_DROP_LOG: /* 0x05 */
			case RTL8651_ACL_MIRROR: /* 0x06 */
			case RTL8651_ACL_REDIRECT_PPPOE: /* 0x07 */
			case RTL8651_ACL_MIRROR_KEEP_MATCH: /* 0x09 */
				len += sprintf(page+len, "\tdvidx: %d   hp: %d   pppoeIdx: %d   nxtHop:%d  ", asic_acl.dvid_, asic_acl.priority_,
						asic_acl.pppoeIdx_, asic_acl.nextHop_);
				break;

			case RTL8651_ACL_POLICY: /* 0x08 */
				len += sprintf(page+len, "\thp: %d   nxtHopIdx: %d  ", asic_acl.priority_, asic_acl.nhIndex);
				break;

			case RTL8651_ACL_PRIORITY: /* 0x08 */
				len += sprintf(page+len, "\tprioirty: %d   ", asic_acl.priority) ;
				break;

			case RTL8651_ACL_DROP_RATE_EXCEED_PPS: /* 0x0a */
			case RTL8651_ACL_LOG_RATE_EXCEED_PPS: /* 0x0b */
			case RTL8651_ACL_DROP_RATE_EXCEED_BPS: /* 0x0c */
			case RTL8651_ACL_LOG_RATE_EXCEED_BPS: /* 0x0d */
				len += sprintf(page+len, "\trlIdx: %d  ", asic_acl.rlIndex);
				break;
			default: 
				;
			
			}
			len += sprintf(page+len, "pktOpApp: %d\n", asic_acl.pktOpApp);
			
		}

#endif

		if (outRule == FALSE) 
		{
			acl_start = asic_intf.outAclStart; acl_end = asic_intf.outAclEnd;
			outRule = TRUE;
			goto again;
		}
	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 acl_write( struct file *filp, const char *buff,unsigned long len, void *data )
{

#if RTL_LAYERED_DRIVER_DEBUG
	char 	tmpbuf[32];
	int32 testNo;
	
	if (buff && !copy_from_user(tmpbuf, buff, len)) 
	{
		tmpbuf[len] = '\0';
		testNo = tmpbuf[0]-'0';		
		rtl865x_acl_test(testNo);
	}
#endif
	return len;
}

#endif

int acl_all_show(struct seq_file *s, void *v)
{
	rtl865x_AclRule_t asic_acl;
	uint32 acl_start;

	
	printk("%s\n", "ASIC ACL Table:");


	for(acl_start=0; acl_start<128 ;acl_start++)	
	{
		if ( _rtl865x_getAclFromAsic(acl_start, &asic_acl) == FAILED)
			rtlglue_printf("=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);	
		
		rtl865x_showACL(&asic_acl);		
	}

	return 0;
	
}


int acl_show(struct seq_file *s, void *v)
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
		acl_start = asic_intf.inAclStart; acl_end = asic_intf.inAclEnd;
		printk("\nacl_start(%d), acl_end(%d)", acl_start, acl_end);
	again:
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

		if (outRule == FALSE) 
		{
			acl_start = asic_intf.outAclStart; acl_end = asic_intf.outAclEnd;
			outRule = TRUE;
			goto again;
		}
	}
	return 0;
}

int acl_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, acl_show, NULL));
}

struct file_operations acl_single_seq_file_operations = {
        .open           = acl_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int acl_all_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, acl_all_show, NULL));
}


struct file_operations acl_all_single_seq_file_operations = {
        .open           = acl_all_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

//#ifdef CONFIG_RTL_IGMP_SNOOPING
#if 0 /*move to net/bridge/br.c*/
extern int igmp_show(struct seq_file *s, void *v);


int igmp_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, igmp_show, NULL));
}

struct file_operations igmp_single_seq_file_operations = {
        .open           = igmp_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

int priority_decision_show(struct seq_file *s, void *v)
{	
	uint32	regData;
	uint8 	port,queue,acl_priotity,priority_8021p;	

	printk("\n----------------------- Priority policy's weight ------------------------\n");	
	regData = READ_MEM32(QIDDPCR);
	printk("NAPT[%d] ACL[%d] DSCP[%d] 8021Q[%d] PortBase[%d]\n",
		(regData & NAPT_PRI_MASK) >> NAPT_PRI_OFFSET, 
		(regData & ACL_PRI_MASK) >> ACL_PRI_OFFSET, 
		(regData & DSCP_PRI_MASK) >> DSCP_PRI_OFFSET, 
		(regData & BP8021Q_PRI_MASK) >> BP8021Q_PRI_OFFSET, 
		(regData & PBP_PRI_MASK) >> PBP_PRI_OFFSET);

	printk("\n----------------------- Acl priority, Qidx mapping ------------------------\n");
	for(queue=0;queue<RTL8651_OUTPUTQUEUE_SIZE;queue++)
	{
		regData = READ_MEM32(UPTCMCR0+(queue<<2));
		printk("Queue number %d:\n", (queue+1));
		printk("Piority[0~7] Mapping to Queue[ %d %d %d %d %d %d %d %d ]\n",
			regData&0x7, (regData>>3)&0x7, (regData>>6)&0x7, (regData>>9)&0x7, 
			(regData>>12)&0x7, (regData>>15)&0x7, (regData>>18)&0x7, (regData>>21)&0x7);
	}

	printk("\n--------------input pkt's 802.1p , Acl priority mapping -------------\n");
	for(priority_8021p=PRI0;priority_8021p<=PRI7;priority_8021p++)
	{
		enum PRIORITYVALUE acl_priority;
		rtl8651_getAsicDot1qAbsolutelyPriority(priority_8021p,&acl_priority);	
		printk("802.1p=%d    acl_priority=%d\n",priority_8021p,acl_priority);
	}
	printk("\n");
	

	printk("\n----------------output pkt's 802.1p Remark info--------------------\n");	
	
	regData = READ_MEM32(RLRC);
	printk("Remarking layer : %s%s%s \n",(regData&RMLC_8021P_L4)?"L4 ":""
		,(regData&RMLC_8021P_L3)?"L3 ":"",(regData&RMLC_8021P_L2)?"L2":"");
	
	printk("Enable port :");
	regData = READ_MEM32(RMCR1P);
	for(port = PHY0 ; port <= PHY5 ; port++)
	{
		if(regData & (1<<RM1P_EN_OFFSET<< (port-PHY0)) )
			printk("p%d ",port-PHY0);
	}
	for(port = EXT1 ; port <= EXT3 ; port++)
	{
		if(regData & (1<<RM1P_EN_OFFSET << (PHY5-PHY0+1) << (port-EXT1)) )
			printk("ext%d ",port-EXT1+1);
	}
	printk("\n");	

	for(acl_priotity=0;acl_priotity<TOTAL_VLAN_PRIORITY_NUM;acl_priotity++)
	{
		uint8 value_8021p;
		regData = READ_MEM32(RMCR1P);
		value_8021p = (regData>>(3*acl_priotity))&0x7;
		printk("ACL priority %d:  b'%s%s%s\n",acl_priotity,(value_8021p&0x4)?"1":"0",(value_8021p&0x2)?"1":"0",(value_8021p&0x1)?"1":"0");
	}


	printk("\n----------------output pkt's DSCP Remark info--------------------\n");	
	
	regData = READ_MEM32(RLRC);
	printk("Remarking layer : %s%s%s \n",(regData&RMLC_DSCP_L4)?"L4 ":""
		,(regData&RMLC_DSCP_L3)?"L3 ":"",(regData&RMLC_DSCP_L2)?"L2":"");
	
	printk("Enable port :");
	regData = READ_MEM32(DSCPRM1);
	for(port = PHY0 ; port <= PHY5 ; port++)
	{
		if(regData & (1<<DSCPRM_EN_OFFSET<< (port-PHY0)) )
			printk("p%d ",port-PHY0);
	}
	for(port = EXT1 ; port <= EXT2 ; port++)
	{
		if(regData & (1<<DSCPRM_EN_OFFSET << (PHY5-PHY0+1) << (port-EXT1)) )
			printk("ext%d ",port-EXT1+1);
	}
	regData = READ_MEM32(DSCPRM0);
	{
		if(regData & RM1P_EN_8 )
			printk("ext%d ",EXT3-EXT1+1);
	}
	printk("\n");	
	

	for(acl_priotity=0;acl_priotity<=4;acl_priotity++)
	{
		uint8 value_dscp;
		regData = READ_MEM32(DSCPRM0);
		value_dscp = (regData>>(6*acl_priotity))&0x3f;
		printk("ACL priority %d:  b'%s%s%s%s%s%s\n",acl_priotity,(value_dscp&0x20)?"1":"0",(value_dscp&0x10)?"1":"0",(value_dscp&0x08)?"1":"0"
			,(value_dscp&0x04)?"1":"0",(value_dscp&0x02)?"1":"0",(value_dscp&0x01)?"1":"0");
	}
	for(acl_priotity=5;acl_priotity<TOTAL_VLAN_PRIORITY_NUM;acl_priotity++)
	{
		uint8 value_dscp;
		regData = READ_MEM32(DSCPRM1);
		value_dscp = (regData>>(6*(acl_priotity-5)))&0x3f;
		printk("ACL priority %d:  b'%s%s%s%s%s%s\n",acl_priotity,(value_dscp&0x20)?"1":"0",(value_dscp&0x10)?"1":"0",(value_dscp&0x08)?"1":"0"
			,(value_dscp&0x04)?"1":"0",(value_dscp&0x02)?"1":"0",(value_dscp&0x01)?"1":"0");
	}


	printk("\n----------------------- Port setting------------------------\n");	
	for(port=PHY0;port<=PHY5;port++)
	{		
		uint32 queue_number,remark_1p_enalbe,remark_dscp_enalbe;
		
		
		regData = READ_MEM32(QNUMCR);
		queue_number = (regData>>(port*3))&7;
		
		regData = READ_MEM32(RMCR1P);	
		remark_1p_enalbe = regData & (1<<RM1P_EN_OFFSET<< (port-PHY0));
		
		regData = READ_MEM32(DSCPRM1);	
		remark_dscp_enalbe = regData & (1<<DSCPRM_EN_OFFSET<< (port-PHY0));

		
		printk("[  Port  %d ] \n", port);
		printk("Queue number:	%d\n", queue_number);
		printk("802.1p Remark:	%s\n", remark_1p_enalbe?"Enable":"Disabled");
		printk("dscp    Remark:	%s\n", remark_dscp_enalbe?"Enable":"Disabled");
		printk("ACL priority    : %d\n",rtl8651_getAsicPortDefaultPriority(port));

		for(acl_priotity=0;acl_priotity<TOTAL_VLAN_PRIORITY_NUM;acl_priotity++)
		{
			int queue_idx;
			uint32 remark_8021p;
			uint32 remark_dscp;
			uint32 flow_control;
			uint32 data0, data1;
			
			regData = READ_MEM32(UPTCMCR0+((queue_number-1)<<2));			
			queue_idx = (regData>>(acl_priotity*3))&0x7;	
			
			regData = READ_MEM32(RMCR1P);
			remark_8021p = (regData>>(acl_priotity*3))&0x7;

			if(acl_priotity<=4)
			{
				regData = READ_MEM32(DSCPRM0);
				remark_dscp = (regData>>(acl_priotity*6))&0x3f;
			}
			else
			{
				regData = READ_MEM32(DSCPRM1);
				remark_dscp = (regData>>((acl_priotity-5)*6))&0x3f;
			}
			
			/* Info of acl_priority mapping */
			printk("ACL priority %d <-----> Queue idx %d   ", acl_priotity,queue_idx);
			if(remark_1p_enalbe)
				printk("802.1p Remark:0x%X   ", remark_8021p);
			if(remark_dscp_enalbe)
				printk("DSCP Remark:0x%X ", remark_dscp);
			printk("\n");	

			/* Info of Queue */
			rtl8651_getAsicQueueFlowControlConfigureRegister(port, queue_idx, &flow_control);
			printk("	==>Queue%d Flow Control %s | ", queue_idx, flow_control?"Enabled":"Disabled");

			rtl8651_getAsicQueueWeight(port, queue_idx, &data0, &data1);
			if (data0 == STR_PRIO)			
				printk("Type: STR\n");			
			else			
				printk("Type: WFQ [weight:%d]\n", data1+1);			
			
			regData = READ_MEM32(P0Q0RGCR+(queue_idx<<2)+((port*6)<<2));
			data1 = (regData&L1_MASK)>>L1_OFFSET;

			if(data1==(L1_MASK>>L1_OFFSET))			
				printk("	    BurstSize UnLimit | ");			
			else			
				printk("	    BurstSize[%dKbps] | ", data1);			

			data0 = (regData&APR_MASK)>>APR_OFFSET;
			data1 = (regData&PPR_MASK)>>PPR_OFFSET;
			if(data0!=(APR_MASK>>APR_OFFSET))
			{
				data0++;
				if (data0<16)
					printk("Engress: avgRate[%dKbps], peakRate[%d]\n", data0<<6, 1<<data1);
				else
					printk("Engress: avgRate[%d.%3dMbps], peakRate[%d]\n", data0>>4, (data0&0xf)<<6, 1<<data1);
			}
			else
				printk("Egress: avgRate & peakRateFullSpeed\n");
			
		}		
	
	}	

	return 0;
}


int priority_decision_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, priority_decision_show, NULL));
}


struct file_operations priority_decision_single_seq_file_operations = {
        .open           = priority_decision_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};


#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
int sw_priority_decision_show(struct seq_file *s, void *v)
{	
	rtl865x_qosShowDebugInfo();			
	return 0;
}


int sw_priority_decision_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_priority_decision_show, NULL));
}


struct file_operations sw_priority_decision_single_seq_file_operations = {
        .open           = sw_priority_decision_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif



#ifdef CONFIG_RTL_LAYERED_DRIVER
int aclChains_show(struct seq_file *s, void *v)
{
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	rtl865x_show_allAclChains();
#endif
	return 0;
}

int aclChains_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, aclChains_show, NULL));
}

struct file_operations aclChains_single_seq_file_operations = {
        .open           = aclChains_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
int qosRule_show(struct seq_file *s, void *v)
{
	rtl865x_show_allQosAcl();
	return 0;
}

int qosRule_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, qosRule_show, NULL));
}

struct file_operations qosRule_single_seq_file_operations = {
        .open           = qosRule_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#endif

static int32 hs_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int	len;	
	hsb_param_t *hsb_r, dummy_hsb_r;
	hsa_param_t *hsa_r, dummy_hsa_r;
	ipaddr_t addr;
	char addr_s[100];

	len = 0;
	hsb_r = &dummy_hsb_r;
	hsa_r = &dummy_hsa_r;
	memset((void*)hsb_r,0,sizeof(hsb_r));
	memset((void*)hsa_r,0,sizeof(hsa_r));
	
	rtl865xC_virtualMacGetHsb( hsb_r );
	{
		len += sprintf(page+len,"HSB(");
		len += sprintf(page+len,"\ttype:%d",hsb_r->type);
		
		len += sprintf(page+len,"\tspa:%d",hsb_r->spa);
		len += sprintf(page+len,"\tlen:%d",hsb_r->len);
		len += sprintf(page+len,"\tvid :%d\n",hsb_r->vid);
		len += sprintf(page+len,"\tpppoe:%d",hsb_r->pppoeif);
		
		/* Protocol contents */
		len += sprintf(page+len,"\ttagif:%d\tpppoeId:%d",hsb_r->tagif,hsb_r->pppoeid);
		len += sprintf(page+len,"\tethrtype:0x%04x\n",hsb_r->ethtype);
		len += sprintf(page+len,"\tllc_other:%d\tsnap:%d\n",hsb_r->llcothr,hsb_r->snap);
		len += sprintf(page+len,"\tda:%02x-%02x-%02x-%02x-%02x-%02x",hsb_r->da[0],hsb_r->da[1],hsb_r->da[2],hsb_r->da[3],hsb_r->da[4],hsb_r->da[5]);
		len += sprintf(page+len,"\tsa:%02x-%02x-%02x-%02x-%02x-%02x\n",hsb_r->sa[0],hsb_r->sa[1],hsb_r->sa[2],hsb_r->sa[3],hsb_r->sa[4],hsb_r->sa[5]);
		
		addr = ntohl( hsb_r->sip);
		inet_ntoa_r(addr, addr_s);
		len += sprintf(page+len,"\tsip:%s(hex:%08x)   ",addr_s,hsb_r->sip);
		len += sprintf(page+len,"\tsprt:%d (hex:%x)\n ",(int)hsb_r->sprt,hsb_r->sprt);
		addr  = ntohl(hsb_r->dip);
		inet_ntoa_r(addr, addr_s);
		len += sprintf(page+len,"\tdip:%s(hex:%08x) ",addr_s,hsb_r->dip);;		
		len += sprintf(page+len,"\tdprt:%d(hex:%08x)\n",hsb_r->dprt,hsb_r->dprt);
		
		len += sprintf(page+len,"\tipptl:%d,",(int)hsb_r->ipptl);
		len += sprintf(page+len,"\tipflg:%d,",hsb_r->ipfg);
		len += sprintf(page+len,"\tiptos:%d,",hsb_r->iptos);
		len += sprintf(page+len,"\ttcpflg:%d\n",hsb_r->tcpfg);
		
		len += sprintf(page+len,"\tdirtx:%d,",hsb_r->dirtx);
		len += sprintf(page+len,"\tprtnmat:%d",hsb_r->patmatch);
	       
		len += sprintf(page+len,"\tudp_nocs:%d",hsb_r->udpnocs);
		len += sprintf(page+len,"\tttlst:0x%x\n",hsb_r->ttlst);

		
		len += sprintf(page+len,"\thp:%d",hsb_r->hiprior);
		len += sprintf(page+len,"\tl3csok:%d\tl4csok:%d\tipfragif:%d\n",hsb_r->l3csok,hsb_r->l4csok,hsb_r->ipfo0_n);
		
	 	len += sprintf(page+len,"\textspa:%d",hsb_r->extspa);
		len += sprintf(page+len,"\turlmch:%d\n)\n",hsb_r->urlmch);
	}

	rtl865xC_virtualMacGetHsa( hsa_r );
	{
		len += sprintf(page+len,("HSA("));
		len += sprintf(page+len,"\tmac:%02x-%02x-%02x-%02x-%02x-%02x\n",hsa_r->nhmac[0],hsa_r->nhmac[1],hsa_r->nhmac[2],hsa_r->nhmac[3],hsa_r->nhmac[4],hsa_r->nhmac[5]);

		addr =ntohl( hsa_r->trip);
		inet_ntoa_r(addr, addr_s);
		len += sprintf(page+len,"\ttrip:%s(hex:%08x)",addr_s,hsa_r->trip);	
		len += sprintf(page+len,"\tprt:%d\tipmcast:%d\n",hsa_r->port,hsa_r->ipmcastr);
		len += sprintf(page+len,"\tl3cs:%d",hsa_r->l3csdt);
		len += sprintf(page+len,"\tl4cs:%d",hsa_r->l4csdt);
		len += sprintf(page+len,"\tInternal NETIF:%d",hsa_r->egif);
		len += sprintf(page+len,"\tl2tr:%d,\n ",hsa_r->l2tr);
		len += sprintf(page+len,"\tl34tr:%d",hsa_r->l34tr);
		len += sprintf(page+len,"\tdirtx:%d",hsa_r->dirtxo);
		len += sprintf(page+len,"\ttype:%d",hsa_r->typeo);
		len += sprintf(page+len,"\tsnapo:%d",hsa_r->snapo);
		len += sprintf(page+len,"\twhy2cpu 0x%x (%d)\n",hsa_r->why2cpu,hsa_r->why2cpu);
		len += sprintf(page+len,"\tpppif:%d",hsa_r->pppoeifo);
		len += sprintf(page+len,"\tpppid:%d",hsa_r->pppidx);
		len += sprintf(page+len,"\tttl_1:0x%x",hsa_r->ttl_1if);
		len += sprintf(page+len,"\tdpc:%d,",hsa_r->dpc);

		len += sprintf(page+len,"\tleno:%d(0x%x)\n",hsa_r->leno,hsa_r->leno);

		len += sprintf(page+len,"\tl3CrcOk:%d",hsa_r->l3csoko);
		len += sprintf(page+len,"\tl4CrcOk:%d",hsa_r->l4csoko);
		len += sprintf(page+len,"\tfrag:%d",hsa_r->frag);
		len += sprintf(page+len,"\tlastFrag:%d\n",hsa_r->lastfrag);



		len += sprintf(page+len,"\tsvid:0x%x",hsa_r->svid);
		len += sprintf(page+len,"\tdvid:%d(0x%x)",hsa_r->dvid,hsa_r->dvid);
		len += sprintf(page+len,"\tdestination interface :%d\n",hsa_r->difid);
		len += sprintf(page+len,"\trxtag:%d",hsa_r->rxtag);
		len += sprintf(page+len,"\tdvtag:0x%x",hsa_r->dvtag);
		len += sprintf(page+len,"\tspa:%d",hsa_r->spao);
		len += sprintf(page+len,"\tdpext:0x%x\thwfwrd:%d\n",hsa_r->dpext,hsa_r->hwfwrd);
		len += sprintf(page+len,"\tspcp:%d",hsa_r->spcp);
		len += sprintf(page+len,"\tpriority:%d",hsa_r->priority);
		
		len += sprintf(page+len,"\tdp:0x%x\n",hsa_r->dp);
		len += sprintf(page+len,")\n");
	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 hs_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#if defined(RTL_DEBUG_NIC_SKB_BUFFER)
int nic_mbuf_show(struct seq_file *s, void *v)
{
	rtl819x_debug_skb_memory();
	return 0;
}

int nic_mbuf_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, nic_mbuf_show, NULL));
}


struct file_operations nic_mbuf_single_seq_file_operations = {
        .open           = nic_mbuf_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

int rxRing_show(struct seq_file *s, void *v)
{
	rtl_dumpRxRing();
	return 0;
}

int rxRing_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rxRing_show, NULL));
}

struct file_operations rxRing_single_seq_file_operations = {
        .open           = rxRing_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int txRing_show(struct seq_file *s, void *v)
{
	rtl_dumpTxRing();
	return 0;
}

int txRing_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, txRing_show, NULL));
}

struct file_operations txRing_single_seq_file_operations = {
        .open           = txRing_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int mbuf_show(struct seq_file *s, void *v)
{
	rtl_dumpMbufRing();
	return 0;
}

int mbuf_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, mbuf_show, NULL));
}

struct file_operations mbuf_single_seq_file_operations = {
        .open           = mbuf_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int32 pvid_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	uint32 vidp[9];
	int32  i;
	int	len;
	char *rule_string[]={"","IPX","NetBIOS","PPPoE Control","PPPoE Session","user-defined 1","user-defined 2"};

	len = 0;
	for(i=0; i<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; i++)
	{
		if (rtl8651_getAsicPVlanId(i, &vidp[i]) != SUCCESS)
		{
			len += sprintf(page+len,"ASIC PVID get failed.\n");
		}
	}
	len += sprintf(page+len,">> PVID Reg:\n");
	for(i=0; i<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; i++)
		len += sprintf(page+len,"p%d: %d,", i, vidp[i]);
	len += sprintf(page+len,"\n");


	len += sprintf(page+len,">> Port and protocol based Reg:\n");
	for(i=1;i<RTL8651_PBV_RULE_MAX;i++)
	{
		int port;	
		uint32 vid;
		uint8 valid;
		len += sprintf(page+len," --  %s  --\n",rule_string[i]);

		for(port=0;port<RTL8651_AGGREGATOR_NUMBER;port++)
		{
			rtl8651_getProtocolBasedVLAN(i,port,&valid,&vid);
			len += sprintf(page+len,"port %d  \t valid:%d \t vid:%d \n",port,valid,vid);
		}	
	}
		

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 pvid_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return SUCCESS;
}

static int32 mirrorPort_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	uint32 mirrorRx, mirrorTx, mirrorPort;
	int	len;

	len = 0;
	rtl8651_getAsicPortMirror(&mirrorRx, &mirrorTx, &mirrorPort);
	len += sprintf(page+len,">>Mirror Control Register:\n\n");
	len += sprintf(page+len,"  Mirror Rx: 0x%x\n", mirrorRx);
	len += sprintf(page+len,"  Mirror Tx: 0x%x\n", mirrorTx);
	len += sprintf(page+len,"  Mirror Port: 0x%x\n", mirrorPort);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 mirrorPort_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char		tmpbuf[64];
	//uint32	*mem_addr, mem_data, mem_len;
	uint32	rx_mask,tx_mask,port_mask;
	char		*strptr, *cmd_addr;
	char		*tokptr;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		printk("cmd %s\n", cmd_addr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "mirror", 6))
		{
			rx_mask=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			tx_mask = simple_strtol(tokptr,NULL,0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			port_mask = simple_strtol(tokptr, NULL, 0);
			printk("mirror rx port mask(0x%x) tx port mask(0x%x), mirror port mask(0x%x)\n",rx_mask,tx_mask,port_mask);
			rtl8651_setAsicPortMirror(rx_mask,tx_mask,port_mask);
		}		
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("Mirror port configuration only support \"mirror\"as the first parameter\n");		
		printk("mirror: \"mirror rx_portmask tx_portmask mirror_portmask\"\n");
	}

	return len;
}


static int32 proc_mem_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return PROC_READ_RETURN_VALUE;
}

static int32 proc_mem_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[64];
	uint32	*mem_addr, mem_data, mem_len;
	char		*strptr, *cmd_addr;
	char		*tokptr;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		printk("cmd %s\n", cmd_addr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "read", 4))
		{
			mem_addr=(uint32*)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_len=simple_strtol(tokptr, NULL, 0);
			memDump(mem_addr, mem_len, "");
		}
		else if (!memcmp(cmd_addr, "write", 5))
		{
			mem_addr=(uint32*)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_data=simple_strtol(tokptr, NULL, 0);
			WRITE_MEM32(mem_addr, mem_data);
			printk("Write memory 0x%p dat 0x%x: 0x%x\n", mem_addr, mem_data, READ_MEM32(mem_addr));
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("Memory operation only support \"read\" and \"write\" as the first parameter\n");
		printk("Read format:	\"read mem_addr length\"\n");
		printk("Write format:	\"write mem_addr mem_data\"\n");
	}

	return len;
}


static int32 l2_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
 	uint32 row, col, port, m=0;
	
	len = sprintf(page, "%s\n", "ASIC L2 Table:");
	for(row=0x0; row<RTL8651_L2TBL_ROW; row++)
	{
		for(col=0; col<RTL8651_L2TBL_COLUMN; col++)
		{
			memset((void*)&asic_l2, 0, sizeof(asic_l2));
			if (rtl8651_getAsicL2Table(row, col, &asic_l2) == FAILED)
			{
				continue;
			}
			
			if (asic_l2.isStatic && asic_l2.ageSec==0 && asic_l2.cpu && asic_l2.memberPortMask == 0 &&asic_l2.auth==0)
			{
				continue;
			}

			len += sprintf(page + len, "%4d.[%3d,%d] %02x:%02x:%02x:%02x:%02x:%02x FID:%x mbr(",m, row, col, 
					asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], 
					asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5],asic_l2.fid
			);

			m++;

			for (port = 0 ; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ; port ++)
			{
				if (asic_l2.memberPortMask & (1<<port))
				{
					len += sprintf(page + len,"%d ", port);
				}
			}

			len += sprintf(page + len,")");
			len += sprintf(page + len,"%s %s %s %s age:%d ",asic_l2.cpu?"CPU":"FWD", asic_l2.isStatic?"STA":"DYN",  asic_l2.srcBlk?"BLK":"", asic_l2.nhFlag?"NH":"", asic_l2.ageSec);

			if (asic_l2.auth)
			{
				len += sprintf(page + len,"AUTH:%d",asic_l2.auth);
			}
			else
			{
				len += sprintf(page + len,"AUTH:0");
			}
			
			len += sprintf(page + len,"\n");
		}
	}

	return len;

}
static int32 l2_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

extern uint32 _rtl865x_getAddMcastOpCnt(void);
extern uint32 _rtl865x_getDelMcastOpCnt(void);
extern uint32 _rtl865x_getForceAddMcastOpCnt(void);
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
static int32 rtl865x_proc_hw_mcast_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;
	rtl865x_tblAsicDrv_multiCastParam_t asic;
	uint32 entry;
	
	#if 1
	printk("%s\n", "ASIC Multicast Table:");
	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				/*printk("\t[%d]  (INVALID)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
					asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff), 
					asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
					asic.mbr);
				printk("\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
					asic.age, asic.cpu);*/
				continue;
			}
			else
			{
				printk("\t[%d]  (OK)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff), 
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
				printk("\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
			}
	}
	printk("\n\t TotalOpCnt:AddMcastOpCnt:%d\tDelMcastOpCnt:%d\tForceAddMcastOpCnt:%d\t \n", _rtl865x_getAddMcastOpCnt(),_rtl865x_getDelMcastOpCnt(),_rtl865x_getForceAddMcastOpCnt());
	#else
	len = sprintf(page, "%s\n", "ASIC Multicast Table:");

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				len +=sprintf(page+len,"\t[%d]  (Invalid Entry)\n", entry);
				continue;
			}
			len += sprintf(page+len, "\t[%d]  dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff), 
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
			len +=sprintf(page+len,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
	}
	#endif
	
	if (len <= off+count) 
	{
		*eof = 1;
	}
	
	*start = page + off;
	len -= off;
	
	if (len>count)
	{
		len = count;
	}
	
	if (len<0)
	{
	  	len = 0;
	}

	return len;
}

static int32 rtl865x_proc_hw_mcast_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[512];
	char		*strptr;
	char		*tokptr;
	char		*dotPtr;
	rtl865xc_tblAsic_ipMulticastTable_t entry;		
	int16 age;
	uint32 idx;
	uint32 sip,dip;
	uint32 srcPort,svid,mbr;
	int32	i;

	
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		bzero(&entry, sizeof(entry));
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;

		/*valid*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		entry.valid = simple_strtol(tokptr, NULL, 0);
		
		/*destination ip*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		dip=0;
		for(i=0;i<4;i++)
		{
			dotPtr=strsep(&tokptr,".");
			if(dotPtr==NULL)
			{
				break;
			}
			dip=(dip<<8)|simple_strtol(dotPtr, NULL, 0);
		}
		
		entry.destIPAddrLsbs= dip & 0xfffffff;
		
		
		/*source ip*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		
		sip=0;
		for(i=0;i<4;i++)
		{
			dotPtr=strsep(&tokptr,".");
			if(dotPtr==NULL)
			{
				break;
			}
			sip=(sip<<8)|simple_strtol(dotPtr, NULL, 0);
		}
		
		entry.srcIPAddr=sip;
		
		
		/*mbr*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		mbr= simple_strtol(tokptr, NULL, 0);
		entry.portList 			= mbr & (RTL8651_PHYSICALPORTMASK);
#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) 
#else
		entry.extPortList 		= mbr >> RTL8651_PORT_NUMBER;
#endif

		/*svid*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		svid= simple_strtol(tokptr, NULL, 0);
#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) 
#else
		entry.srcVidH 			= ((svid)>>4) &0xff;
		entry.srcVidL 			= (svid)&0xf;
#endif

		/*spa*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}		
		srcPort= simple_strtol(tokptr, NULL, 0);
#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) 
#else

		if (srcPort>= RTL8651_PORT_NUMBER) 
		{

			/* extension port */
			entry.srcPortExt = 1;
			entry.srcPort 			= (srcPort-RTL8651_PORT_NUMBER);
		}
		else 
		{
			entry.srcPortExt = 0;
			entry.srcPort 			= srcPort;
		}
#endif
		/*extIP*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		entry.extIPIndex = simple_strtol(tokptr, NULL, 0);
		
		/*age*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		age=simple_strtol(tokptr, NULL, 0);
	
		
		entry.ageTime			= 0;
		while ( age > 0 )
		{
			if ( (++entry.ageTime) == 7)
				break;
			age -= 5;
		}

		/*to cpu*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		entry.toCPU = simple_strtol(tokptr, NULL, 0);

		idx = rtl8651_ipMulticastTableIndex(sip,dip);
		_rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);
			
			
	}
	else
	{
errout:
		printk("error input\n");
	}

	return len;
}

static int32 rtl865x_proc_sw_mcast_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;
		
	rtl_dumpSwMulticastInfo();
	
	return len;
}

static int32 rtl865x_proc_sw_mcast_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	
	return len;
}
#endif

int l2_all_show(struct seq_file *s, void *v)
{
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
 	uint32 row, col, port, m=0;
	
	printk("ASIC L2 (ALL) Table:\n");
	for(row=0x0; row<RTL8651_L2TBL_ROW; row++)
	{
		for(col=0; col<RTL8651_L2TBL_COLUMN; col++)
		{
			memset((void*)&asic_l2, 0, sizeof(asic_l2));
			if (rtl8651_getAsicL2Table_ALL(row, col, &asic_l2) == FAILED)
			{
				continue;
			}
			
			if (asic_l2.isStatic && asic_l2.ageSec==0 && asic_l2.cpu && asic_l2.memberPortMask == 0 &&asic_l2.auth==0)
			{
				continue;
			}

				printk("%4d.[%3d,%d] %02x:%02x:%02x:%02x:%02x:%02x FID:%x mbr(",m, row, col, 
					asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], 
					asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5],asic_l2.fid
			);

			m++;

			for (port = 0 ; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ; port ++)
			{
				if (asic_l2.memberPortMask & (1<<port))
				{
					printk("%d ", port);
				}
			}

			printk(")");
			printk("%s %s %s %s age:%d ",asic_l2.cpu?"CPU":"FWD", asic_l2.isStatic?"STA":"DYN",  asic_l2.srcBlk?"BLK":"", asic_l2.nhFlag?"NH":"", asic_l2.ageSec);

			if (asic_l2.auth)
			{		
				printk("AUTH:%d",asic_l2.auth);
			}
			else
			{
				printk("AUTH:0");
			}
			
			printk("\n");
		}
	}

	return 0;
}

int l2_all_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, l2_all_show, NULL));
}


struct file_operations l2_all_single_seq_file_operations = {
        .open           = l2_all_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};



#ifdef CONFIG_RTL_LAYERED_DRIVER_L3

static uint8 arp_hit[RTL8651_SW_ARPTBL_SIZE];	
int sw_arp_show(struct seq_file *s, void *v)
{
	rtl865x_route_t *head;
	int i;
	
	struct rtl865x_arp_table* ARPTables = rtl865x_getswARPTable();	

	printk("SW ARP Table:\n");	
	/* we use sw l3 table to get all related sw arp entries... */
	memset(&arp_hit,0,RTL8651_SW_ARPTBL_SIZE*sizeof(uint8));
	head = rtl865x_getRouteList();
	while (head)
	{
		if(head->process == RT_ARP)
		{
			for(i=head->un.arp.arpsta;i<(head->un.arp.arpend+8);i++)
			{
				ipaddr_t rt_ip;

				if(ARPTables->mappings[i].ip!=0)	
				{
					rt_ip = (head->ipAddr&head->ipMask)+(i-head->un.arp.arpsta);
					printk("[%u.%u.%u.%u]  : ",NIPQUAD(rt_ip));
					arp_hit[i]=1;
				}
				else
					continue;

				if(ARPTables->mappings[i].ip!= rt_ip)
				{
					printk("(BUG, ip is not matching, in sw_arp table is %u.%u.%u.%u) \n",NIPQUAD(ARPTables->mappings[i].ip));
					continue;
				}

				printk("%02X:%02X:%02X:%02X:%02X:%02X \n"
					,ARPTables->mappings[i].mac.octet[0],ARPTables->mappings[i].mac.octet[1],ARPTables->mappings[i].mac.octet[2]
					,ARPTables->mappings[i].mac.octet[3],ARPTables->mappings[i].mac.octet[4],ARPTables->mappings[i].mac.octet[5]);
			}
		}
		head = head->next;
	}

	printk("\n");
	/* For trace bug, we search that whether there is any sw_arp entry is disconnected with sw_l3 entry */
	for(i=0;i<RTL8651_SW_ARPTBL_SIZE;i++)		
	{
		if(ARPTables->mappings[i].ip!=0 && arp_hit[i]==0)
			printk("We find a bug in sw_arp table (%u.%u.%u.%u , %02X:%02X:%02X:%02X:%02X:%02X ) is disconnected with sw_l3 entry \n"
			,NIPQUAD(ARPTables->mappings[i].ip)
			,ARPTables->mappings[i].mac.octet[0],ARPTables->mappings[i].mac.octet[1],ARPTables->mappings[i].mac.octet[2]
			,ARPTables->mappings[i].mac.octet[3],ARPTables->mappings[i].mac.octet[4],ARPTables->mappings[i].mac.octet[5]);
	}


	return 0;
}

int sw_arp_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_arp_show, NULL));
}


struct file_operations sw_arp_single_seq_file_operations = {
        .open           = sw_arp_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};






int sw_nexthop_show(struct seq_file *s, void *v)
{
	int entryIdx;
	rtl865x_nextHopEntry_t *entry;
	rtl865x_nextHopEntry_t* NexthopTables = rtl865x_getNexthopList();	

	int8 *NextHopT[] = { "IF_NONE", "IF_ETHER", "IF_PPPOE", "IF_PPTP", "IF_L2TP"};
	

	printk("SW Nexthop Table:\n");	

	for(entryIdx = 0; entryIdx < NXTHOP_ENTRY_NUM; entryIdx++)
	{
		if(NexthopTables[entryIdx].valid != 0)
		{
			entry = &NexthopTables[entryIdx];

			if(entry->nextHopType==IF_ETHER)
				printk("[%d] asic_idx:%d   Type:%s   nexthop_ip:%u.%u.%u.%u  "
				,entryIdx,entry->entryIndex,NextHopT[entry->nextHopType],NIPQUAD(entry->nexthop));
			else
				printk("[%d] asic_idx:%d   Type:%s   sesstion_id:%u  "
				,entryIdx,entry->entryIndex,NextHopT[entry->nextHopType],entry->nexthop);


			printk("attr:%s   netif:%s   srcIp:%u.%u.%u.%u  refCnt:%d\n"
				,(entry->flag== NEXTHOP_L3)?"For Routing":"For ACL",entry->dstNetif,NIPQUAD(entry->srcIp),entry->refCnt);
				
		}
	}
	return 0;
}

int sw_extip_show(struct seq_file *s, void *v)
{
	rtl865x_extIP_show();
	return 0;
}


int sw_nexthop_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_nexthop_show, NULL));
}


struct file_operations sw_nexthop_single_seq_file_operations = {
        .open           = sw_nexthop_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int sw_extip_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_extip_show, NULL));
}

struct file_operations sw_extip_single_seq_file_operations = {
        .open           = sw_extip_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};


static int32 testModel_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return 0;
}


static int32 testModel_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 	tmpbuf[512];
	char		*strptr;
	char		*cmdptr;
	uint32	test_id=0;
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;

		if(strlen(strptr)==0)
		{
			goto errout;
		}
		
		cmdptr = strsep(&strptr," ");
		if (cmdptr==NULL)
		{
			goto errout;
		}
		
		/*parse command*/	
		if(strncmp(cmdptr, "test",4) == 0)
		{
			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL)
			{
				goto errout;
			}					
			test_id = simple_strtol(cmdptr, NULL, 0);

			
			rtl_testModel_run(test_id);
		
		}
		else
		{
			goto errout;
		}
		
	}
	else
	{
errout:
		printk("error input  ()\n");
	}

	return len;
}

static int32 arp_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	rtl865x_tblAsicDrv_arpParam_t asic_arp;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	
	uint32	i, j, port;
	ipaddr_t ipAddr;
	int8 ipBuf[sizeof"255.255.255.255"];

		
	len = sprintf(page, "%s\n", "ASIC Arp Table:\n");
	for(i=0; i<RTL8651_ARPTBL_SIZE; i++) 
	{
		if (rtl8651_getAsicArp(i,  &asic_arp) == FAILED)
			continue;
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++) 
		{
			if (rtl8651_getAsicRouting(j, &asic_l3) == FAILED || asic_l3.process!= 0x02 /*RT_ARP*/)
				continue;
			if(asic_l3.arpStart <= (i>>3) &&  (i>>3) <= asic_l3.arpEnd)
			{
				ipAddr = (asic_l3.ipAddr & asic_l3.ipMask) + (i - (asic_l3.arpStart<<3));
				if(rtl8651_getAsicL2Table_Patch(asic_arp.nextHopRow, asic_arp.nextHopColumn, &asic_l2) == FAILED)
				{
					inet_ntoa_r(ipAddr, ipBuf);
					len += sprintf(page + len,"%-16s [%3d,%d] ", ipBuf, asic_arp.nextHopRow, asic_arp.nextHopColumn);
				}
				else 
				{
					inet_ntoa_r(ipAddr, ipBuf);
					len += sprintf(page + len,"%-16s %02x-%02x-%02x-%02x-%02x-%02x (", ipBuf, asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5]);
					for(port=0; port< RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; port++){
						if(asic_l2.memberPortMask& (1<<port))
							len += sprintf(page + len,"%d ", port);
						else
							len += sprintf(page + len,"  ");
					}							
					len += sprintf(page + len,") %us", asic_l2.ageSec);
				}
				continue;
			}
		}

		len += sprintf(page + len," ARP:%3d  L2:%3d,%d,aging:%d\n", i, asic_arp.nextHopRow, asic_arp.nextHopColumn,asic_arp.aging);

	}

	return len;		
}

static int32 arp_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

static int32 nexthop_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;	
	rtl865x_tblAsicDrv_nextHopParam_t asic_nxthop;

	uint32 idx, refcnt, rt_flag;

	len = sprintf(page, "%s\n", "ASIC Next Hop Table:\n");
	for(idx=0; idx<RTL8651_NEXTHOPTBL_SIZE; idx++) {
		refcnt = rt_flag = 0;
		if (rtl8651_getAsicNextHopTable(idx, &asic_nxthop) == FAILED)
			continue;
		len += sprintf(page + len,"  [%d]  type(%s) IPIdx(%d) dstVid(%d) pppoeIdx(%d) nextHop(%d,%d) rf(%d) rt(%d)\n", idx,
			(asic_nxthop.isPppoe==TRUE? "pppoe": "ethernet"), asic_nxthop.extIntIpIdx, 
			asic_nxthop.dvid, asic_nxthop.pppoeIdx, asic_nxthop.nextHopRow,asic_nxthop.nextHopColumn, refcnt, rt_flag);
	}

	return len;
}

static int32 nexthop_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

static int32 l3_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;	
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
	int8 *strNetType[] = { "WAN", "DMZ", "LAN",  "RLAN"};
	uint32 idx, mask;
	int netIdx;

	len = sprintf(page, "%s\n", "ASIC L3 Routing Table:\n");
	for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++)
	{
		if (rtl8651_getAsicRouting(idx, &asic_l3) == FAILED)
		{
			len += sprintf(page + len,"\t[%d]  (Invalid)\n", idx);
			continue;
		}
		if (idx == RTL8651_ROUTINGTBL_SIZE-1)
			mask = 0;
		else for(mask=32; !(asic_l3.ipMask&0x01); asic_l3.ipMask=asic_l3.ipMask>>1)
				mask--;
		netIdx = asic_l3.internal<<1|asic_l3.DMZFlag;
		len += sprintf(page + len,"\t[%d]  %d.%d.%d.%d/%d process(%s) %s \n", idx, (asic_l3.ipAddr>>24),
			((asic_l3.ipAddr&0x00ff0000)>>16), ((asic_l3.ipAddr&0x0000ff00)>>8), (asic_l3.ipAddr&0xff), 
			mask, str[asic_l3.process],strNetType[netIdx]);
		
		switch(asic_l3.process) 
		{
		case 0x00:	/* PPPoE */
			len += sprintf(page + len,"\t           dvidx(%d)  pppidx(%d) nxthop(%d,%d)\n", asic_l3.vidx, asic_l3.pppoeIdx, asic_l3.nextHopRow,asic_l3.nextHopColumn);
			break;
			
		case 0x01:	/* L2 */
			len += sprintf(page + len,"              dvidx(%d) nexthop(%d)\n", asic_l3.vidx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x02:	/* ARP */
			len += sprintf(page + len,"             dvidx(%d) ARPSTA(%d) ARPEND(%d) IPIDX(%d)\n", asic_l3.vidx, asic_l3.arpStart<<3, asic_l3.arpEnd<<3, asic_l3.arpIpIdx);
			break;

		case 0x03:	/* Reserved */
			;

		case 0x04:	/* CPU */
			len += sprintf(page + len,"             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x05:	/* NAPT Next Hop */
			len += sprintf(page + len,"              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d) IPDOMAIN(%d)\n", asic_l3.nhStart,
				asic_l3.nhNum, asic_l3.nhNxt, asic_l3.nhAlgo, asic_l3.ipDomain);
			break;

		case 0x06:	/* DROP */
			len += sprintf(page + len,"             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x07:	/* Reserved */
			/* pass through */
		default: 
		;
		}
	}

	return len;
}

static int32 l3_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#ifdef CONFIG_RTL_MULTI_ETH_WAN
static int32 sw_l3_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;	
	rtl865x_route_t *head;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
	uint32 mask;
	ipaddr_t ipMask;

	len = sprintf(page, "%s\n", "SW L3 Routing Table:\n");
	head = rtl865x_getRouteList();

	while (head)
	{
		ipMask = head->ipMask;
		
		if (ipMask)
		{
			for(mask=32; !(ipMask&0x01); ipMask=ipMask>>1)
				mask--;
		}
		else
			mask = 0;

		len += sprintf(page + len,"\t[%d]  %d.%d.%d.%d/%d srcIP:%d.%d.%d.%d process(%s)  needInAsic:%d  netif:%s  addr:0x%x\n", 
			head->asicIdx, (head->ipAddr>>24), ((head->ipAddr&0x00ff0000)>>16), ((head->ipAddr&0x0000ff00)>>8), (head->ipAddr&0xff), 
			mask, (head->srcIp>>24), ((head->srcIp&0x00ff0000)>>16), ((head->srcIp&0x0000ff00)>>8), (head->srcIp&0xff), 
			str[head->process], head->needInAsic, head->dstNetif,(unsigned int)head);

	
		head = head->next;
	}

	return len;
}

static int32 sw_l3_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
#endif

static int32 ip_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;

	rtl865x_tblAsicDrv_extIntIpParam_t asic_ip;
	int32	i;
	int8 intIpBuf[sizeof"255.255.255.255"];
	int8 extIpBuf[sizeof"255.255.255.255"];

	len = sprintf(page, "%s\n", "ASIC IP Table:\n");

	for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
	{
		if (rtl8651_getAsicExtIntIpTable(i,  &asic_ip) == FAILED)
		{
			len += sprintf(page + len,"  [%d] (Invalid)\n", i);
			continue;
		}
		inet_ntoa_r(asic_ip.intIpAddr, intIpBuf);
		inet_ntoa_r(asic_ip.extIpAddr,extIpBuf);			
		len += sprintf(page + len,"  [%d] intip(%-14s) extip(%-14s) type(%s) nhIdx(%d)\n",
					i, intIpBuf,extIpBuf,
					(asic_ip.localPublic==TRUE? "LP" : (asic_ip.nat==TRUE ? "NAT" : "NAPT")), asic_ip.nhIndex);
	}

	return len;
}

static int32 ip_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

static int32 pppoe_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;	
	rtl865x_tblAsicDrv_pppoeParam_t asic_pppoe;
	int32	i;

	len = sprintf(page, "%s\n", "ASIC PPPOE Table:\n");
	for(i=0; i<RTL8651_PPPOETBL_SIZE; i++) 
	{
		if (rtl8651_getAsicPppoe(i,  &asic_pppoe) == FAILED)
			continue;
		len += sprintf(page + len,"\t[%d]  sessionID(%d)  ageSec(%d)\n", i, asic_pppoe.sessionId, asic_pppoe.age);
	}
	return len;
}

static int32 pppoe_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
int32 napt_show(struct seq_file *s, void *v)
{
	int len;	
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_tcpudp;
	uint32 idx, entry=0;

	len = seq_printf(s, "%s\n", "ASIC NAPT TCP/UDP Table:\n");

	for(idx=0; idx<RTL8651_TCPUDPTBL_SIZE; idx++) {
		if (rtl8651_getAsicNaptTcpUdpTable(idx, &asic_tcpudp) == FAILED)
			continue;

		if (asic_tcpudp.isValid == 1 || asic_tcpudp.isDedicated == 1 ) {
			len += seq_printf(s, "[%4d] %d.%d.%d.%d:%d {V,D}={%d,%d} col1(%d) col2(%d) static(%d) tcp(%d)\n",
			       idx,
			       asic_tcpudp.insideLocalIpAddr>>24, (asic_tcpudp.insideLocalIpAddr&0x00ff0000) >> 16,
			       (asic_tcpudp.insideLocalIpAddr&0x0000ff00)>>8, asic_tcpudp.insideLocalIpAddr&0x000000ff,
			       asic_tcpudp.insideLocalPort, 
			       asic_tcpudp.isValid, asic_tcpudp.isDedicated,
			       asic_tcpudp.isCollision, asic_tcpudp.isCollision2, asic_tcpudp.isStatic, asic_tcpudp.isTcp );

			len += seq_printf(s, "   age(%d) offset(%d) tcpflag(%d) SelEIdx(%d) SelIPIdx(%d) priValid:%d pri(%d)\n",
			        asic_tcpudp.ageSec, asic_tcpudp.offset<<10, asic_tcpudp.tcpFlag, 
			        asic_tcpudp.selEIdx, asic_tcpudp.selExtIPIdx,asic_tcpudp.priValid,asic_tcpudp.priority );
			entry++;
		}
	}
	len += seq_printf(s, "Total entry: %d\n", entry);	
	
	return 0;
}

int napt_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, napt_show, NULL));
}

struct file_operations napt_single_seq_file_operations = {
        .open           = napt_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
extern int32 rtl865x_sw_napt_seq_read(struct seq_file *s, void *v);
extern int32 rtl865x_sw_napt_seq_write( struct file *filp, const char *buff,unsigned long len, loff_t *off );

int sw_napt_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl865x_sw_napt_seq_read, NULL));
}

static ssize_t sw_napt_proc_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return rtl865x_sw_napt_seq_write(file, userbuf,count, off);
}

struct file_operations sw_napt_single_seq_file_operations = {
        .open           = sw_napt_single_open,
	 .write		=sw_napt_proc_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif


static int32 diagnostic_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData, regData1;
	int		port, regIdx;
	uint32	mask, offset;

	len = sprintf(page, "Diagnostic Register Info:\n");

	regData = READ_MEM32(GDSR0);
	len += sprintf(page + len, "MaxUsedDescriptor: %d CurUsed Descriptor: %d\n", 
		(regData&MaxUsedDsc_MASK)>>MaxUsedDsc_OFFSET, 
		(regData&USEDDSC_MASK)>>USEDDSC_OFFSET);
	len += sprintf(page+len, "DescRunOut: %s TotalDescFC: %s ShareBufFC: %s\n", 
		(regData&DSCRUNOUT)?"YES":"NO", (regData&TotalDscFctrl_Flag)?"YES":"NO", (regData&SharedBufFCON_Flag)?"YES":"NO");

	for(regIdx = 0; regIdx<2; regIdx++)
	{
		regData = READ_MEM32(PCSR0+(regIdx<<2));

		for(port=0; port<4; port++)
		{
			switch(port)
			{
				case 0:
					mask = P0OQCgst_MASK;
					offset = P0OQCgst_OFFSET;
					break;
				case 1:
					mask = P1OQCgst_MASK;
					offset = P1OQCgst_OFFSET;
					break;
				case 2:
					mask = P2OQCgst_MASK;
					offset = P2OQCgst_OFFSET;
					break;
				default:
					mask = P3OQCgst_MASK;
					offset = P3OQCgst_OFFSET;
					break;
			}
			regData1 = (regData&mask)>>offset;
			if (regData1==0)
				len += sprintf(page+len, "Port%d not congestion\n", port+(regIdx<<2));
			else
			{
				len += sprintf(page+len, "Port%d queue congestion mask 0x%x\n", port+(regIdx<<2), regData1);
			}
		}
	}
	
	for(port=0;port<=CPU;port++)
	{
		len += sprintf(page+len, "Port%d each queue used descriptor: Queue[0~5]: [ ", port);
		for(regIdx=0; regIdx<3; regIdx++)
		{
			regData = READ_MEM32(P0_DCR0+(port<<4)+(regIdx<<2));
			len += sprintf(page+len, "%d %d ", 
				((regData&Pn_EQDSCR_MASK)>>Pn_EVEN_OQDSCR_OFFSET), 
				((regData&Pn_OQDSCR_MASK)>>Pn_ODD_OQDSCR_OFFSET));
		}

		regData = READ_MEM32(P0_DCR3+(port<<4));
		len += sprintf(page+len, "]  Input queue [%d]\n", 
				((regData&Pn_EQDSCR_MASK)>>Pn_EVEN_OQDSCR_OFFSET));
	}
	
	return len;
}

static int32 diagnostic_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

static int32 port_bandwidth_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0, data1;
	int		port;
	
	len = sprintf(page, "Dump Port Bandwidth Info:\n");

	for(port=0;port<=CPU;port++)
	{
		regData = READ_MEM32(IBCR0+((port>>1)<<2));
		if(port&1)
		{
			data0 = (regData&IBWC_ODDPORT_MASK)>>IBWC_ODDPORT_OFFSET;
		}
		else
		{
			data0 = (regData&IBWC_EVENPORT_MASK)>>IBWC_EVENPORT_OFFSET;
		}
		
		regData = READ_MEM32(WFQRCRP0+((port*3)<<2));
		data1 = (regData&APR_MASK)>>APR_OFFSET;

		data0++;
		
		if (data0)
		{
			if (data0<64)
				len += sprintf(page+len, "Port%d Ingress:[%dKbps]	", port, (data0<<4)/1000);
			else
				len += sprintf(page+len, "Port%d Ingress:[%d.%03dMbps]	", port, (data0<<14)/1000000, ((data0<<14)%1000000)/1000);
		}
		else
		{
			len += sprintf(page+len, "Port%d Ingress:FullSpeed	", port);
		}

		if(data1!=(APR_MASK>>APR_OFFSET))
		{
			data1++;
			
			if (data1<16)
				len += sprintf(page+len, "Engress:[%dKbps]\n", (data1<<16)/1000);
			else
				len += sprintf(page+len, "Engress:[%d.%03dMbps]\n", (data1<<16)/1000000, ((data1<<16)%1000000)/1000);

		}
		else
			len += sprintf(page+len, "Egress:FullSpeed\n");
	}
	
	return len;
}

static int32 port_bandwidth_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

static int32 queue_bandwidth_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0, data1;
	int		port, queue;

	if (queue_bandwidth_record_portmask==0)
	{
		len = sprintf(page, "Please set the dump mask firstly.\n");
		return len;
	}
	len = sprintf(page, "Dump Queue Bandwidth Info:\n");

	for(port=PHY0;port<=CPU;port++)
	{
		if ((queue_bandwidth_record_portmask&(1<<port))==0)
			continue;
		
		regData = READ_MEM32(QNUMCR);
		data0 = (regData>>(port*3))&7;
		len += sprintf(page+len, "Port%d Queue number:	%d\n", port, data0);
		for(queue=QUEUE0;queue<=QUEUE5;queue++)
		{
			rtl8651_getAsicQueueFlowControlConfigureRegister(port, queue, &data0);
			len += sprintf(page+len, "	==>Queue%d FC %s | ", queue, data0?"Enabled":"Disabled");

			rtl8651_getAsicQueueWeight(port, queue, &data0, &data1);
			if (data0 == STR_PRIO)
			{
				len += sprintf(page+len, "Type: STR\n");
			}
			else
			{
				len += sprintf(page+len, "Type: WFQ [weight:%d]\n", data1+1);
			}
			
			regData = READ_MEM32(P0Q0RGCR+(queue<<2)+((port*6)<<2));
			data1 = (regData&L1_MASK)>>L1_OFFSET;

			if(data1==(L1_MASK>>L1_OFFSET))
			{
				len += sprintf(page+len, "	    BurstSize UnLimit | ");
			}
			else
			{
				len += sprintf(page+len, "	    BurstSize[%dKbps] | ", data1);
			}

			data0 = (regData&APR_MASK)>>APR_OFFSET;
			data1 = (regData&PPR_MASK)>>PPR_OFFSET;
			if(data0!=(APR_MASK>>APR_OFFSET))
			{
				data0++;
				if (data0<16)
					len += sprintf(page+len, "Engress: avgRate[%dKbps], peakRate[%d]\n", data0<<6, 1<<data1);
				else
					len += sprintf(page+len, "Engress: avgRate[%d.%3dMbps], peakRate[%d]\n", data0>>4, (data0&0xf)<<6, 1<<data1);

			}
			else
				len += sprintf(page+len, "Egress: avgRate & peakRateFullSpeed\n");
			
		}		
	}
	
	return len;
}

static int32 queue_bandwidth_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char tmpbuf[16], *tokptr, *strptr;
	int	port;

	memset(tmpbuf, 0, 16);
	
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr = tmpbuf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			return len;
		}
		queue_bandwidth_record_portmask=(uint32)simple_strtol(tokptr, NULL, 0);

		printk("Dump info of: ");
		for(port=PHY0;port<=CPU;port++)
		{
			if ((1<<port)&queue_bandwidth_record_portmask)
				printk("port%d ", port);
		}
		printk("\n");
	}

	return len;
}

static int32 port_status_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0;
	int		port;
	
	len = sprintf(page, "Dump Port Status:\n");

	for(port=PHY0;port<=CPU;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));

		if (port==CPU)
			len += sprintf(page+len, "CPUPort ");
		else
			len += sprintf(page+len, "Port%d ", port);
		data0 = regData & PortStatusLinkUp;
		
		if (data0)
			len += sprintf(page+len, "LinkUp | ");
		else
		{
			len += sprintf(page+len, "LinkDown\n\n");
			continue;
		}
		data0 = regData & PortStatusNWayEnable;
		len += sprintf(page+len, "NWay Mode %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusRXPAUSE;
		len += sprintf(page+len, "	RXPause %s | ", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusTXPAUSE;
		len += sprintf(page+len, "TXPause %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusDuplex;
		len += sprintf(page+len, "	Duplex %s | ", data0?"Enabled":"Disabled");
		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		len += sprintf(page+len, "Speed %s\n\n", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"Unkown")));
	}
	
	return len;
}

static int32 port_status_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char		tmpbuf[64];
	uint32	port_mask;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int 	type;
	int 		port;
	int forceMode = 0;
	int forceLink = 0;
	int forceLinkSpeed = 0;
	int forceDuplex = 0;
	uint32 advCapability = 0;

#define SPEED10M 	0
#define SPEED100M 	1
#define SPEED1000M 	2

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len-1] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "port", 4))
		{
			port_mask=simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			
			if(strcmp(tokptr,"10_half") == 0)
				type = HALF_DUPLEX_10M;
			else if(strcmp(tokptr,"100_half") == 0)
				type = HALF_DUPLEX_100M;
			else if(strcmp(tokptr,"10_full") == 0)
				type = DUPLEX_10M;
			else if(strcmp(tokptr,"100_full") == 0)
				type = DUPLEX_100M;
			else
				type = PORT_AUTO;

			switch(type)
			{
				case HALF_DUPLEX_10M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED10M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_10M);
					break;
				}
				case HALF_DUPLEX_100M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED100M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_100M);
					break;
				}				
				case DUPLEX_10M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED10M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_10M);
					break;
				}
				case DUPLEX_100M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED100M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_100M);
					break;
				}				
				default:	
				{
					forceMode=FALSE;
					forceLink=TRUE;
					/*all capality*/
					advCapability=(1<<PORT_AUTO);		
				}
			}
				
			
			for(port = 0; port < CPU; port++)
			{
				if((1<<port) & port_mask)
				{
					rtl865xC_setAsicEthernetForceModeRegs(port, forceMode, forceLink, forceLinkSpeed, forceDuplex);
							
					/*Set PHY Register*/
					rtl8651_setAsicEthernetPHYSpeed(port,forceLinkSpeed);
					rtl8651_setAsicEthernetPHYDuplex(port,forceDuplex);
					rtl8651_setAsicEthernetPHYAutoNeg(port,forceMode?FALSE:TRUE);
					rtl8651_setAsicEthernetPHYAdvCapality(port,advCapability);
					rtl8651_restartAsicEthernetPHYNway(port);
				}
			}
			
		}		
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("port status only support \"port\" as the first parameter\n");
		printk("format: \"port port_mask 10_half/100_half/10_full/100_full/1000_full/auto\"\n");
	}
	return len;
}

#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
static int32 rate_limit_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	rtl865x_tblAsicDrv_rateLimitParam_t asic_rl;
	uint32 entry;

	len = sprintf(page, "Dump rate limit table:\n");
	for(entry=0; entry<RTL8651_RATELIMITTBL_SIZE; entry++) {
		if (rtl8651_getAsicRateLimitTable(entry, &asic_rl) == SUCCESS) {
			len += sprintf(page+len, " [%d]  Token(%u)  MaxToken(%u)  remainTime Unit(%u)  \n\trefillTimeUnit(%u)  refillToken(%u)\n",
				entry, asic_rl.token, asic_rl.maxToken, asic_rl.t_remainUnit, asic_rl.t_intervalUnit, asic_rl.refill_number);
		}
		else len += sprintf(page+len, " [%d]  Invalid entry\n", entry);
	}
	len += sprintf(page+len, "\n");

	return len;
}

static int32 rate_limit_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
#endif

static int32 rtl865x_proc_mibCounter_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;
	
	rtl865xC_dumpAsicDiagCounter();
	
	return len;
}


static int32 rtl865x_proc_mibCounter_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[512];
	char		*strptr;
	char		*cmdptr;
	uint32	portNum=0xFFFFFFFF;
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr=tmpbuf;

		if(strlen(strptr)==0)
		{
			goto errout;
		}
		
		cmdptr = strsep(&strptr," ");
		if (cmdptr==NULL)
		{
			goto errout;
		}
		
		/*parse command*/
		if(strncmp(cmdptr, "clear",5) == 0)
		{
			rtl8651_clearAsicCounter();
		}
		else if(strncmp(cmdptr, "dump",4) == 0)
		{
			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL)
			{
				goto errout;
			}
			
			if(strncmp(cmdptr, "port",4) != 0)
			{
				goto errout;
			}
			
			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL)
			{
				goto errout;
			}
			portNum = simple_strtol(cmdptr, NULL, 0);
		
				
			if((portNum>=0) && (portNum<=RTL8651_PORT_NUMBER))
			{
				extern int32 rtl8651_returnAsicCounter(uint32 offset); 
				extern uint64 rtl865xC_returnAsicCounter64(uint32 offset);
				uint32 addrOffset_fromP0 = portNum * MIB_ADDROFFSETBYPORT;
				
				if ( portNum == RTL8651_PORT_NUMBER )
					printk("<CPU port (extension port included)>\n");
				else
					printk("<Port: %d>\n", portNum);

				printk("Rx counters\n");
				printk("   Rcv %llu bytes, Drop %u pkts, CRCAlignErr %u, FragErr %u, JabberErr %u\n", 
					rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
				printk("   Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n", 
					rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ));
				printk("   < 64: %u pkts, 64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n", 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0 ));
				printk("   256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n", 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0 ) );
				printk("   oversize: %u pkts, Control unknown %u pkts, Pause %u pkts\n", 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ));
				
				printk("Output counters\n");
				printk("   Snd %llu bytes, Unicast %u pkts, Multicast %u pkts\n",
					rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ));
				printk("   Broadcast %u pkts, Late collision %u, Deferred transmission %u \n",
					rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
				printk("   Collisions %u Single collision %u Multiple collision %u pause %u\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ));
				
			}
			else
			{
				goto errout;
			}
		}
		else
		{
			goto errout;
		}
		
	}
	else
	{
errout:
		printk("error input\n");
	}

	return len;
}

static int32 proc_phyReg_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return PROC_READ_RETURN_VALUE;
}

#ifdef CONFIG_RTL_8198
void getPhyByPortPage(int port, int page)
{
	uint32	regData;
	int reg;

	//change page num
	//rtl8651_setAsicEthernetPHYReg(port, 31, page);
	if (page>=31)
	{	
		rtl8651_setAsicEthernetPHYReg( port, 31, 7  );
		rtl8651_setAsicEthernetPHYReg( port, 30, page  );
	}
	else if (page>0)
	{
		rtl8651_setAsicEthernetPHYReg( port, 31, page  );
	}
					
	for(reg=0;reg<32;reg++)
	{
		rtl8651_getAsicEthernetPHYReg( port, reg, &regData);	
		printk("port:%d,page:%d,regId:%d,regData:0x%x\n",port,page,reg,regData);
	}
	//if(page!=3)
	//{
		printk("------------------------------------------\n");
	//}

	//change back to page 0
	rtl8651_setAsicEthernetPHYReg(port, 31, 0 );

}

static const int _8198_phy_page[] = {	0, 1, 2, 3,     		4, 5, 6, 32, 
								  	33, 34, 35, 36,	40, 44, 45, 46, 
								  	64, 65, 66, 69,	70, 80, 81, 161 };
#endif


static int32 proc_phyReg_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[64];
	uint32	phyId, pageId,regId, regData;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int32 	ret;
	int 		i, j;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
	
		if (!memcmp(cmd_addr, "read", 4))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);
			
			ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData); 
			if(ret==SUCCESS)
			{
				printk("read phyId(%d), regId(%d),regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				printk("error input!\n");
			}
		}
		else if (!memcmp(cmd_addr, "write", 5))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);
		
			ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);
			if(ret==SUCCESS)
			{
				printk("Write phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				printk("error input!\n");
			}
		}
#ifdef CONFIG_8198_PORT5_RGMII
		else if (!memcmp(cmd_addr, "8370read", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);
			ret = rtl8370_getAsicReg(regId, &regData); 
			
			if(ret==0)
			{
				printk("rtl8370_getAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				printk("get fail %d\n", ret);
			}
		}
		else if (!memcmp(cmd_addr, "8370write", 9))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);
		
			ret = rtl8370_setAsicReg(regId, regData);

			if(ret==0)
			{
				printk("rtl8370_setAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				printk("set fail %d\n", ret);
			}
		}
#endif
		else if (!memcmp(cmd_addr, "extRead", 7))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			pageId=simple_strtol(tokptr, NULL, 0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			//switch page
			if (pageId>=31)
			{	
				rtl8651_setAsicEthernetPHYReg( phyId, 31, 7  );
				rtl8651_setAsicEthernetPHYReg( phyId, 30, pageId  );
			}
			else if (pageId>0)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
			}
	
			ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);
			
			if(ret==SUCCESS)
			{
				printk("extRead phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId,pageId, regId, regData);
			}
			else
			{
				printk("error input!\n");
			}

			//change back to page 0
			rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);
			
		}
		else if (!memcmp(cmd_addr, "extWrite", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			pageId=simple_strtol(tokptr, NULL, 0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);

			//switch page
			if (pageId>=31)
			{	
				rtl8651_setAsicEthernetPHYReg( phyId, 31, 7  );
				rtl8651_setAsicEthernetPHYReg( phyId, 30, pageId  );
			}
			else if (pageId>0)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
			}
	
			ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);
			
			if(ret==SUCCESS)
			{
				printk("extWrite phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId, pageId, regId, regData);
			}
			else
			{
				printk("error input!\n");
			}
			
			//change back to page 0
			rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);
		}
		else if (!memcmp(cmd_addr, "snr", 3))
		{
			uint32 	sum;
			for (i=0;i<5;i++) 
			{
				if (REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) 
				{
					for (j=0, sum=0;j<10;j++) 
					{
						rtl8651_getAsicEthernetPHYReg(i, 29, &regData);
						sum += regData;
						mdelay(10);
					}
					sum /= 10;
					//db = -(10 * log(sum/262144));
					//printk("  port %d SNR = %d dB\n", i, db);
					printk("  port %d SUM = %d\n", i, sum);
				}
				else 
				{
					printk("  port %d is link down\n", i);
				}
			}
		}
		else if (!memcmp(cmd_addr, "dumpAll", 7))
		{
			int port;
#ifdef CONFIG_RTL_8196C
			int page,reg;
#endif
			for (port=0; port<5; port++)
			{
				printk("==========================================\n");
				
#ifdef CONFIG_RTL_8196C
				for(page=0;page<4;page++)
				{
					//change page num
					rtl8651_setAsicEthernetPHYReg(port, 31, page);
					for(reg=0;reg<32;reg++)
					{
						rtl8651_getAsicEthernetPHYReg( port, reg, &regData);	
						printk("port:%d,page:%d,regId:%d,regData:0x%x\n",port,page,reg,regData);
					}
					if(page!=3)
					{
						printk("------------------------------------------\n");
					}
					//change back to page 0
					rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
				}
#elif defined(CONFIG_RTL_8198)
				//set pageNum {0 1 2 3 4 5 6 32 33 34 35 36 40 44 45 46 64 65 66 69 70 80 81 161}       
				for (i=0; i<24; i++)
					getPhyByPortPage(port,  _8198_phy_page[i]);
#endif				
			}
		}
	}
	else
	{
errout:
		printk("error input!\n");
	}

	return len;
}


static int32 storm_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32 port;
	uint32 totalExtPortNum=3;	
	len = sprintf(page, "Dump storm control info:\n");

	regData = READ_MEM32(BSCR);
	len += sprintf(page+len, "rate(%d)\n",regData*100/30360);

	for ( port = 0; port < RTL8651_PORT_NUMBER + totalExtPortNum; port++ )
	{
		regData = READ_MEM32(PCRP0+port*4);
		len+= sprintf(page+len,"port%d, %s BCSC, %s BC, %s MC\n", port,regData&ENBCSC?"enable":"disable",regData&BCSC_ENBROADCAST?"enable":"disable",
					regData&BCSC_ENMULTICAST?"enable":"disable");
	}

	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	return len;
}

static int32 storm_write(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	uint32 tmpBuf[32];
	uint32 stormCtrlType=0x3;//mc & bc
	uint32 enableStormCtrl=FALSE;
	uint32 percentage=0;
	uint32 uintVal;

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		uintVal=simple_strtoul((char *)tmpBuf, NULL, 0);
		printk("%s(%d) uintval(%u) \n",__FUNCTION__,__LINE__,uintVal);//Added for test
		if(uintVal>100)
		{
			enableStormCtrl=FALSE;
			percentage=0;
		}
		else
		{
			enableStormCtrl=TRUE;
			percentage=uintVal;
		}
		//printk("%s(%d),enableStormCtrl=%d,percentage=%d\n",__FUNCTION__,__LINE__,
		//	enableStormCtrl,percentage);//Added for test
		rtl865x_setStormControl(stormCtrlType,enableStormCtrl,percentage);
		return count;
	}
	return -EFAULT;
}

#if defined(CONFIG_RTL_8367B)
static int32 rtl8367b_proc_mibCounter_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;
	show_rtl8367b_PortStat(0);
	show_rtl8367b_PortStat(1);
	show_rtl8367b_PortStat(2);
	show_rtl8367b_PortStat(3);
	show_rtl8367b_PortStat(4);
	show_rtl8367b_PortStat(6);//cpu port
	
	return len;
}

static int32 rtl8367b_proc_mibCounter_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	reset_rtl8367b_PortStat();
	return len;
}

static int32 rtl8367b_proc_reg_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return PROC_READ_RETURN_VALUE;
}

static int32 rtl8367b_proc_reg_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char		tmpbuf[64];
	uint32		regId, regData;
	char		*strptr, *cmd_addr;
	char		*tokptr;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
	
		if (!memcmp(cmd_addr, "read", 4))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);
			
			RL6000_read(regId, &regData);
			printk("rtl8367b read REG0x%x: 0x%04X\n", regId, regData);
		}
		else if (!memcmp(cmd_addr, "write", 5))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);
		
			RL6000_write(regId, regData&0xffff);
			printk("rtl8367b write REG0x%x: 0x%04X\n", regId, regData);
			RL6000_read(regId, &regData);
			printk("rtl8367b read REG0x%x: 0x%04X\n", regId, regData);
		}
	}
	else
	{
errout:
		printk("error input!\n");
	}

	return len;
}

#endif

int32 rtl865x_proc_debug_init(void)
{
	int32 retval;
	if(rtl865x_proc_dir==NULL)
		rtl865x_proc_dir = proc_mkdir(RTL865X_PROC_DIR_NAME,NULL);
	if(rtl865x_proc_dir)
	{

		/*  dump_trap2CPU_pkt  */
		{
			dump_trap2CPU_pkt = create_proc_entry("dump_trap2CPU_pkt",0,rtl865x_proc_dir);
			if(dump_trap2CPU_pkt != NULL)
			{
				dump_trap2CPU_pkt->read_proc = dump_trap2CPU_pkt_read;
				dump_trap2CPU_pkt->write_proc= dump_trap2CPU_pkt_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for dump_trap2CPU_pkt");
				retval = FAILED;
				goto out;
			}
		}

		/* dump_swNicTxRx_pkt */
		{
			dump_swNicTxRx_pkt = create_proc_entry("dump_swNicTxRx_pkt",0,rtl865x_proc_dir);
			if(dump_swNicTxRx_pkt != NULL)
			{
				dump_swNicTxRx_pkt->read_proc = dump_swNicTxRx_pkt_read;
				dump_swNicTxRx_pkt->write_proc= dump_swNicTxRx_pkt_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for dump_swNicTxRx_pkt");
				retval = FAILED;
				goto out;
			}
		}

		/*vlan*/
		{
			vlan_entry = create_proc_entry("vlan",0,rtl865x_proc_dir);
			if(vlan_entry != NULL)
			{
				//vlan_entry->read_proc = vlan_read;
				//vlan_entry->write_proc= vlan_write;				
				vlan_entry->proc_fops = &vlan_single_seq_file_operations;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for vlan");
				retval = FAILED;
				goto out;
			}
		}

		/*netif*/
		{
			netif_entry = create_proc_entry("netif",0,rtl865x_proc_dir);
			if(netif_entry != NULL)
			{
				netif_entry->read_proc = netif_read;
				netif_entry->write_proc= netif_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for netif");
				retval = FAILED;
				goto out;
			}
		}

#ifdef CONFIG_RTL_MULTI_ETH_WAN
		/*sw_netif*/
		{
			sw_netif = create_proc_entry("sw_netif",0,rtl865x_proc_dir);
			if(sw_netif != NULL)
			{
				sw_netif->read_proc = sw_netif_read;
				sw_netif->write_proc= sw_netif_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for netif");
				retval = FAILED;
				goto out;
			}
		}
#endif

		/*acl*/
		{
			acl_entry = create_proc_entry("acl",0,rtl865x_proc_dir);
			if(acl_entry != NULL)
			{
				/*acl_entry->read_proc = acl_read;
				acl_entry->write_proc= acl_write;
				acl_entry->owner = THIS_MODULE;
				*/
				acl_entry->proc_fops = &acl_single_seq_file_operations;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for acl");
				retval = FAILED;
				goto out;
			}
		}

		/*acl_all*/
		{
			acl_all_entry = create_proc_entry("acl_all",0,rtl865x_proc_dir);
			if(acl_all_entry != NULL)
			{
				/*acl_entry->read_proc = acl_read;
				acl_entry->write_proc= acl_write;
				acl_entry->owner = THIS_MODULE;
				*/
				acl_all_entry->proc_fops = &acl_all_single_seq_file_operations;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for acl_all");
				retval = FAILED;
				goto out;
			}
		}

		/*storm control*/
		{
			storm_control = create_proc_entry("storm_control",0,rtl865x_proc_dir);
			if(storm_control != NULL)
			{
				storm_control->read_proc = storm_read;
				storm_control->write_proc= storm_write;
			}
			else
			{
				rtlglue_printf("can't create proc entry for storm control");
				retval = FAILED;
				goto out;
			}
		}

#ifdef CONFIG_RTL_LAYERED_DRIVER
		/*soft acl chains*/
		{
			acl_chains_entry = create_proc_entry("sw_aclChains",0,rtl865x_proc_dir);
			if(acl_chains_entry != NULL)
			{
				/*acl_entry->read_proc = acl_read;
				acl_entry->write_proc= acl_write;
				acl_entry->owner = THIS_MODULE;
				*/
				acl_chains_entry->proc_fops = &aclChains_single_seq_file_operations;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for aclChains");
				retval = FAILED;
				goto out;
			}
		}
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
		/*qos rules*/
		{
			qos_rule_entry = create_proc_entry("soft_qosRules",0,rtl865x_proc_dir);
			if(qos_rule_entry != NULL)
			{
				/*acl_entry->read_proc = acl_read;
				acl_entry->write_proc= acl_write;
				acl_entry->owner = THIS_MODULE;
				*/
				qos_rule_entry->proc_fops = &qosRule_single_seq_file_operations;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for aclChains");
				retval = FAILED;
				goto out;
			}
		}
#endif
		/*hs*/
		{
			hs_entry = create_proc_entry("hs",0,rtl865x_proc_dir);
			if(hs_entry != NULL)
			{
				hs_entry->read_proc = hs_read;
				hs_entry->write_proc= hs_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for hs");
				retval = FAILED;
				goto out;
			}
		}
#if defined(RTL_DEBUG_NIC_SKB_BUFFER)
		/*nic mbuf*/
		{
			nic_skb_buff = create_proc_entry("nic_mbuf",0,rtl865x_proc_dir);
			if(nic_skb_buff != NULL)
			{
				nic_skb_buff->proc_fops = &nic_mbuf_single_seq_file_operations;
				retval = SUCCESS;
			}
			else
			{
				printk("can't create nic_mbuf entry for hs");
				retval = FAILED;
				goto out;
			}
		}
#endif
		/*rx ring*/
		{
			rxRing_entry = create_proc_entry("rxRing",0,rtl865x_proc_dir);
			if(rxRing_entry != NULL)
			{
				rxRing_entry->proc_fops = &rxRing_single_seq_file_operations;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create rxRing entry for hs");
				retval = FAILED;
				goto out;
			}
		}

		/*tx ring*/
		{
			txRing_entry = create_proc_entry("txRing",0,rtl865x_proc_dir);
			if(txRing_entry != NULL)
			{
				txRing_entry->proc_fops = &txRing_single_seq_file_operations;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create txRing entry for hs");
				retval = FAILED;
				goto out;
			}
		}

		/*rx ring*/
		{
			mbuf_entry = create_proc_entry("mbufRing",0,rtl865x_proc_dir);
			if(mbuf_entry != NULL)
			{
				mbuf_entry->proc_fops = &mbuf_single_seq_file_operations;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create mbuf entry for hs");
				retval = FAILED;
				goto out;
			}
		}
		
		/*pvid*/
		{
			pvid_entry = create_proc_entry("pvid",0,rtl865x_proc_dir);
			if(pvid_entry != NULL)
			{
				pvid_entry->read_proc = pvid_read;
				pvid_entry->write_proc= pvid_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for pvid");
				retval = FAILED;
				goto out;
			}
		}

			/*pvid*/
		{
			mirrorPort_entry = create_proc_entry("mirrorPort",0,rtl865x_proc_dir);
			if(mirrorPort_entry != NULL)
			{
				mirrorPort_entry->read_proc = mirrorPort_read;
				mirrorPort_entry->write_proc= mirrorPort_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for mirrorPort");
				retval = FAILED;
				goto out;
			}
		}

		/*memory*/
		{
			mem_entry = create_proc_entry("memory",0,rtl865x_proc_dir);
			if(mem_entry != NULL)
			{
				mem_entry->read_proc = proc_mem_read;
				mem_entry->write_proc= proc_mem_write;

				retval = SUCCESS;				
			}
			else
			{
				rtlglue_printf("can't create proc entry for memory");
				retval = FAILED;
				goto out;
			}
		}
			
		/*l2*/
		{
			l2_entry = create_proc_entry("l2", 0, rtl865x_proc_dir);
			if(l2_entry != NULL)
			{
				l2_entry->read_proc = l2_read;
				l2_entry->write_proc = l2_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for l2");
				retval = FAILED;
				goto out;
			}
		}

		/*l2_all*/
		{
			l2_all_entry = create_proc_entry("l2_all", 0, rtl865x_proc_dir);
			if(l2_all_entry != NULL)
			{		
				l2_all_entry->proc_fops = &l2_all_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for l2_all");
				retval = FAILED;
				goto out;
			}
		}
		
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)		
		hwMCast_entry=create_proc_entry("hwMCast", 0, rtl865x_proc_dir);
		if(hwMCast_entry != NULL)
		{
			hwMCast_entry->read_proc = rtl865x_proc_hw_mcast_read;
			hwMCast_entry->write_proc = rtl865x_proc_hw_mcast_write;
			retval = SUCCESS;
		}
		
		swMCast_entry=create_proc_entry("swMCast", 0, rtl865x_proc_dir);
		if(swMCast_entry != NULL)
		{
			swMCast_entry->read_proc = rtl865x_proc_sw_mcast_read;
			swMCast_entry->write_proc = rtl865x_proc_sw_mcast_write;
			retval = SUCCESS;
		}
#endif	

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3

		/*sw_arp*/
		{
			sw_arp_entry = create_proc_entry("sw_arp", 0, rtl865x_proc_dir);
			if(sw_arp_entry != NULL)
			{		
				sw_arp_entry->proc_fops = &sw_arp_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_arp");
				retval = FAILED;
				goto out;
			}
		}

		/*sw_nexthop*/
		{
			sw_nexthop_entry = create_proc_entry("sw_nexthop", 0, rtl865x_proc_dir);
			if(sw_nexthop_entry != NULL)
			{		
				sw_nexthop_entry->proc_fops = &sw_nexthop_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_nexthop");
				retval = FAILED;
				goto out;
			}
		}

		/*sw_extip*/
		{
			sw_extip_entry = create_proc_entry("sw_ip", 0, rtl865x_proc_dir);
			if(sw_extip_entry != NULL)
			{		
				sw_extip_entry->proc_fops = &sw_extip_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_ip");
				retval = FAILED;
				goto out;
			}
		}


		/*arp*/
		{
			arp_entry = create_proc_entry("arp", 0, rtl865x_proc_dir);
			if(arp_entry != NULL)
			{
				arp_entry->read_proc = arp_read;
				arp_entry->write_proc = arp_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for arp");
				retval = FAILED;
				goto out;
			}
		}
		
		/*nextHop*/
		{
			nexthop_entry= create_proc_entry("nexthop", 0, rtl865x_proc_dir);
			if(nexthop_entry != NULL)
			{
				nexthop_entry->read_proc = nexthop_read;
				nexthop_entry->write_proc = nexthop_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for nexthop");
				retval = FAILED;
				goto out;
			}
		}

		/*l3*/
		{
			l3_entry= create_proc_entry("l3", 0, rtl865x_proc_dir);
			if(l3_entry != NULL)
			{
				l3_entry->read_proc = l3_read;
				l3_entry->write_proc = l3_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for l3");
				retval = FAILED;
				goto out;
			}
		}


		/* test_model */
		{
			testModel_entry = create_proc_entry("testModel", 0, rtl865x_proc_dir);
			if(testModel_entry != NULL)
			{
				testModel_entry->read_proc = testModel_read;
				testModel_entry->write_proc = testModel_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for testModel");
				retval = FAILED;
				goto out;
			}
		}
		
		#ifdef CONFIG_RTL_MULTI_ETH_WAN
		/*sw_l3*/
		{
			sw_l3 = create_proc_entry("sw_l3", 0, rtl865x_proc_dir);
			if(sw_l3 != NULL)
			{
				sw_l3->read_proc = sw_l3_read;
				sw_l3->write_proc = sw_l3_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_l3");
				retval = FAILED;
				goto out;
			}
		}
		#endif
		/*ip*/
		{
			ip_entry= create_proc_entry("ip", 0, rtl865x_proc_dir);
			if(ip_entry != NULL)
			{
				ip_entry->read_proc = ip_read;
				ip_entry->write_proc = ip_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for ip");
				retval = FAILED;
				goto out;
			}
		}

		/*pppoe*/
		{
			pppoe_entry= create_proc_entry("pppoe", 0, rtl865x_proc_dir);
			if(pppoe_entry != NULL)
			{
				pppoe_entry->read_proc = pppoe_read;
				pppoe_entry->write_proc = pppoe_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for pppoe");
				retval = FAILED;
				goto out;
			}
		}

#endif
//#if defined(CONFIG_RTL_LAYERED_DRIVER_L4) && defined(CONFIG_RTL_8198)
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
		/*napt*/
		{
			napt_entry= create_proc_entry("napt", 0, rtl865x_proc_dir);
			if(napt_entry != NULL)
			{
				#if 1
				napt_entry->proc_fops = &napt_single_seq_file_operations;
				#else
				napt_entry->read_proc = napt_read;
				napt_entry->write_proc = napt_write;
				napt_entry->owner = THIS_MODULE;
				#endif
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for napt");
				retval = FAILED;
				goto out;
			}
		}
		
		/*software napt*/
//#if defined(CONFIG_RTL_LAYERED_DRIVER_L4) && defined(CONFIG_RTL_8198)
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
		{
			sw_napt_entry= create_proc_entry("sw_napt", 0, rtl865x_proc_dir);
			if(sw_napt_entry != NULL)
			{
				#ifdef CONFIG_RTL_LAYERED_DRIVER
				sw_napt_entry->proc_fops= &sw_napt_single_seq_file_operations;
				#else
				sw_napt_entry->read_proc = rtl865x_sw_napt_proc_read;
				sw_napt_entry->write_proc = rtl865x_sw_napt_proc_write;
				#endif
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_napt");
				retval = FAILED;
				goto out;
			}
		}
#endif

#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
		/*software l2*/
		{
			sw_l2_entry= create_proc_entry("sw_l2", 0, rtl865x_proc_dir);
			if(sw_l2_entry != NULL)
			{
				sw_l2_entry->read_proc = rtl865x_sw_l2_proc_read;
				sw_l2_entry->write_proc = rtl865x_sw_l2_proc_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_l2");
				retval = FAILED;
				goto out;
			}
		}
#endif

#ifdef CONFIG_RTL865X_ROMEPERF
		/*rome perf dump*/
		{
			rtl8651_romeperfInit();
			perf_dump= create_proc_entry("perf_dump", 0, rtl865x_proc_dir);
			if(perf_dump != NULL)
			{
				perf_dump->read_proc = rtl865x_perf_proc_read;
				perf_dump->write_proc = rtl865x_perf_proc_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for dump_perf");
				retval = FAILED;
				goto out;
			}
		}
#endif
		/*diagnostic*/
		{
			diagnostic_entry= create_proc_entry("diagnostic", 0, rtl865x_proc_dir);
			if(diagnostic_entry != NULL)
			{
				diagnostic_entry->read_proc = diagnostic_read;
				diagnostic_entry->write_proc = diagnostic_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for diagnostic");
				retval = FAILED;
				goto out;
			}
		}

		/*port_bandwidth*/
		{
			port_bandwidth_entry= create_proc_entry("port_bandwidth", 0, rtl865x_proc_dir);
			if(port_bandwidth_entry != NULL)
			{
				port_bandwidth_entry->read_proc = port_bandwidth_read;
				port_bandwidth_entry->write_proc = port_bandwidth_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for port_bandwidth");
				retval = FAILED;
				goto out;
			}
		}

		/*queue_bandwidth*/
		{
			queue_bandwidth_entry= create_proc_entry("queue_bandwidth", 0, rtl865x_proc_dir);
			if(queue_bandwidth_entry != NULL)
			{
				queue_bandwidth_entry->read_proc = queue_bandwidth_read;
				queue_bandwidth_entry->write_proc = queue_bandwidth_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for queue_bandwidth");
				retval = FAILED;
				goto out;
			}
		}
		
		/*port_status*/
		{
			port_status_entry= create_proc_entry("port_status", 0, rtl865x_proc_dir);
			if(port_status_entry != NULL)
			{
				port_status_entry->read_proc = port_status_read;
				port_status_entry->write_proc = port_status_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for port_status");
				retval = FAILED;
				goto out;
			}
		}

		/*phy*/
		{
			phyReg_entry= create_proc_entry("phyReg", 0, rtl865x_proc_dir);
			if(phyReg_entry != NULL)
			{
				phyReg_entry->read_proc = proc_phyReg_read;
				phyReg_entry->write_proc = proc_phyReg_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for phyTeg");
				retval = FAILED;
				goto out;
			}
		}

			/*priority_decision*/
		{
			priority_decision_entry= create_proc_entry("priority_decision", 0, rtl865x_proc_dir);
			if(priority_decision_entry != NULL)
			{
				//priority_decision_entry->read_proc = priority_decision_read;
				//priority_decision_entry->write_proc = priority_decision_write;
				priority_decision_entry->proc_fops = &priority_decision_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for priority_decision");
				retval = FAILED;
				goto out;
			}
		}

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
		/*sw_priority_decision*/
		{
			sw_priority_decision_entry= create_proc_entry("sw_priority_decision", 0, rtl865x_proc_dir);
			if(priority_decision_entry != NULL)
			{
				//priority_decision_entry->read_proc = priority_decision_read;
				//priority_decision_entry->write_proc = priority_decision_write;
				sw_priority_decision_entry->proc_fops = &sw_priority_decision_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_priority_decision");
				retval = FAILED;
				goto out;
			}
		}
#endif

#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
		/* rate limit table */
		{
			rateLimit_entry= create_proc_entry("rateLimit", 0, rtl865x_proc_dir);
			if(rateLimit_entry != NULL)
			{
				rateLimit_entry->read_proc = rate_limit_read;
				rateLimit_entry->write_proc = rate_limit_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for rate limit table");
				retval = FAILED;
				goto out;
			}
		}
#endif
		#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL865X_EVENT_PROC_DEBUG)
		eventMgr_entry=create_proc_entry("eventMgr", 0, rtl865x_proc_dir);
		if(eventMgr_entry != NULL)
		{
			eventMgr_entry->read_proc = rtl865x_event_proc_read;
			eventMgr_entry->write_proc = rtl865x_event_proc_write;
			retval = SUCCESS;
		}
		#endif
			

		asicCnt_entry=create_proc_entry("asicCounter", 0, rtl865x_proc_dir);
		if(asicCnt_entry != NULL)
		{
			asicCnt_entry->read_proc = rtl865x_proc_mibCounter_read;
			asicCnt_entry->write_proc = rtl865x_proc_mibCounter_write;
			retval = SUCCESS;
		}

#if 0// defined CONFIG_RTL_IGMP_SNOOPING
		igmp_entry=create_proc_entry("igmp", 0, rtl865x_proc_dir);
		if(igmp_entry != NULL)
		{
			/*
			igmp_entry->read_proc = rtl865x_proc_igmpsnooping_read;
			igmp_entry->write_proc = rtl865x_proc_igmpsnooping_write;
			igmp_entry->owner = THIS_MODULE;
			*/
			igmp_entry->proc_fops = &igmp_single_seq_file_operations;
			retval = SUCCESS;
		}
#endif

#if defined(CONFIG_RTL_8367B)
		mib_8367b_entry=create_proc_entry("rtl8367b_counter", 0, rtl865x_proc_dir);
		if(mib_8367b_entry != NULL)
		{
			mib_8367b_entry->read_proc = rtl8367b_proc_mibCounter_read;
			mib_8367b_entry->write_proc = rtl8367b_proc_mibCounter_write;
			retval = SUCCESS;
		}

		rtl8367b_reg = create_proc_entry("rtl8367b_reg", 0, rtl865x_proc_dir);
		if (rtl8367b_reg != NULL)
		{
			rtl8367b_reg->read_proc = rtl8367b_proc_reg_read;
			rtl8367b_reg->write_proc = rtl8367b_proc_reg_write;
			retval = SUCCESS;
		}
#endif

	}
	else
	{
		retval = FAILED;
	}
out:
	if(retval == FAILED)
		rtl865x_proc_debug_cleanup();
	
	return retval;	
	
}

int32 rtl865x_proc_debug_cleanup(void)
{
	if(rtl865x_proc_dir)
	{
		if(vlan_entry)
		{
			remove_proc_entry("vlan",rtl865x_proc_dir);
			vlan_entry = NULL;
		}

		if(netif_entry)
		{
			remove_proc_entry("netif", rtl865x_proc_dir);
			netif_entry = NULL;
		}

		if(acl_entry)
		{
			remove_proc_entry("acl", rtl865x_proc_dir);
			acl_entry = NULL;
		}
#ifdef CONFIG_RTL_LAYERED_DRIVER
		if(acl_chains_entry)
		{
			remove_proc_entry("sw_aclChains", rtl865x_proc_dir);
			acl_chains_entry = NULL;
		}
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
		if(qos_rule_entry)
		{
			remove_proc_entry("soft_qosRules", rtl865x_proc_dir);
			qos_rule_entry = NULL;
		}
#endif
		if(hs_entry)
		{
			remove_proc_entry("hs", rtl865x_proc_dir);
			hs_entry = NULL;
		}
#if defined(RTL_DEBUG_NIC_SKB_BUFFER)
		if(nic_skb_buff)
		{
			remove_proc_entry("nic_mbuf", rtl865x_proc_dir);
			nic_skb_buff = NULL;
		}
#endif
		if(rxRing_entry)
		{
			remove_proc_entry("rxRing", rtl865x_proc_dir);
			rxRing_entry = NULL;
		}

		if(txRing_entry)
		{
			remove_proc_entry("txRing", rtl865x_proc_dir);
			txRing_entry = NULL;
		}

		if(mbuf_entry)
		{
			remove_proc_entry("mbufRing", rtl865x_proc_dir);
			mbuf_entry = NULL;
		}

		if(l2_entry)
		{
			remove_proc_entry("l2", rtl865x_proc_dir);
			l2_entry = NULL;
		}

		if(l2_all_entry)
		{
			remove_proc_entry("l2_all", rtl865x_proc_dir);
			l2_all_entry = NULL;
		}

		if(mem_entry)
		{
			remove_proc_entry("memory", mem_entry);
			mem_entry = NULL;
		}

		if(arp_entry)
		{
			remove_proc_entry("arp", rtl865x_proc_dir);
			arp_entry = NULL;
		}

		if(testModel_entry)
		{
			remove_proc_entry("testModel", rtl865x_proc_dir);
			testModel_entry = NULL;
		}

		if(sw_arp_entry)
		{
			remove_proc_entry("sw_arp", rtl865x_proc_dir);
			sw_arp_entry = NULL;
		}

		if(nexthop_entry)
		{
			remove_proc_entry("nexthop", rtl865x_proc_dir);
			nexthop_entry = NULL;
		}

		if(sw_nexthop_entry)
		{
			remove_proc_entry("sw_nexthop", rtl865x_proc_dir);
			sw_nexthop_entry = NULL;
		}

		
		if(sw_extip_entry)
		{
			remove_proc_entry("sw_ip", rtl865x_proc_dir);
			sw_extip_entry = NULL;
		}

		if(l3_entry)
		{
			remove_proc_entry("l3", rtl865x_proc_dir);
			l3_entry = NULL;
		}
		
#ifdef CONFIG_RTL_MULTI_ETH_WAN
		if (sw_netif)
		{
			remove_proc_entry("sw_netif", rtl865x_proc_dir);
			sw_netif = NULL;
		}

		if (sw_l3)
		{
			remove_proc_entry("sw_l3", rtl865x_proc_dir);
			sw_l3 = NULL;
		}
#endif
		if(ip_entry)
		{
			remove_proc_entry("ip", rtl865x_proc_dir);
			ip_entry = NULL;
		}

		if(pppoe_entry)
		{
			remove_proc_entry("pppoe", rtl865x_proc_dir);
			pppoe_entry = NULL;
		}

		if(napt_entry)
		{
			remove_proc_entry("napt", rtl865x_proc_dir);
		}

		if (diagnostic_entry)
		{
			remove_proc_entry("diagnostic", rtl865x_proc_dir);
		}

		if (port_bandwidth_entry)
		{
			remove_proc_entry("port_bandwidth", rtl865x_proc_dir);
		}

		if (queue_bandwidth_entry)
		{
			remove_proc_entry("queue_bandwidth", rtl865x_proc_dir);
		}

		if (port_status_entry)
		{
			remove_proc_entry("port_status", rtl865x_proc_dir);
		}

		if (phyReg_entry)
		{
			remove_proc_entry("phyReg", rtl865x_proc_dir);
		}
		if (priority_decision_entry)
		{
			remove_proc_entry("priority_decision", rtl865x_proc_dir);
		}
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
		if(sw_napt_entry)
		{
			remove_proc_entry("sw_napt", rtl865x_proc_dir);
		}
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
		if(sw_l2_entry)
		{
			remove_proc_entry("sw_l2", rtl865x_proc_dir);
		}
#endif

#ifdef CONFIG_RTL865X_ROMEPERF
		if(perf_dump)
		{
			remove_proc_entry("perf_dump", rtl865x_proc_dir);
		}
#endif

		#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL865X_EVENT_PROC_DEBUG)
		if(eventMgr_entry != NULL)
		{
			remove_proc_entry("eventMgr", rtl865x_proc_dir);
		}
		#endif

		if(hwMCast_entry != NULL)
		{
			remove_proc_entry("hwMCast", rtl865x_proc_dir);
		}
		
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(swMCast_entry != NULL)
		{
			remove_proc_entry("swMCast", rtl865x_proc_dir);
		}
#endif
		if(asicCnt_entry != NULL)
		{
			remove_proc_entry("asicCounter", rtl865x_proc_dir);
		}
/* //move to net/bridge/br.c
#if defined (CONFIG_RTL_IGMP_SNOOPING)
		if(igmp_entry!=NULL)
		{
			remove_proc_entry("igmp", rtl865x_proc_dir);
		}
#endif
*/
#if defined(CONFIG_RTL_8367B)
		if(mib_8367b_entry!=NULL)
		{
			remove_proc_entry("rtl8367b_counter", rtl865x_proc_dir);
		}
#endif

#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
		if(rateLimit_entry != NULL)
		{
			remove_proc_entry("rateLimit", rtl865x_proc_dir);
		}
#endif
		remove_proc_entry(RTL865X_PROC_DIR_NAME, NULL);
		rtl865x_proc_dir = NULL;
				
	}

	return SUCCESS;	
}


