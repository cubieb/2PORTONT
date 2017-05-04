#include <rtl8367b_asicdrv_acl.h>
#include <rtl8367b_asicdrv.h>
#include <rtl8367b_asicdrv_dot1x.h>
#include <rtl8367b_asicdrv_qos.h>
#include <rtl8367b_asicdrv_scheduling.h>
#include <rtl8367b_asicdrv_fc.h>
#include <rtl8367b_asicdrv_port.h>
#include <rtl8367b_asicdrv_phy.h>
#include <rtl8367b_asicdrv_igmp.h>
#include <rtl8367b_asicdrv_unknownMulticast.h>
#include <rtl8367b_asicdrv_rma.h>
#include <rtl8367b_asicdrv_vlan.h>
#include <rtl8367b_asicdrv_lut.h>
#include <rtl8367b_asicdrv_led.h>
#include <rtl8367b_asicdrv_svlan.h>
#include <rtl8367b_asicdrv_meter.h>
#include <rtl8367b_asicdrv_inbwctrl.h>
#include <rtl8367b_asicdrv_storm.h>
#include <rtl8367b_asicdrv_misc.h>
#include <rtl8367b_asicdrv_portIsolation.h>
#include <rtl8367b_asicdrv_cputag.h>
#include <rtl8367b_asicdrv_trunking.h>
#include <rtl8367b_asicdrv_mirror.h>
#include <rtl8367b_asicdrv_mib.h>
#include <rtl8367b_asicdrv_interrupt.h>
#include <rtl8367b_asicdrv_green.h>
#include <rtl8367b_asicdrv_eee.h>
#include <rtl8367b_asicdrv_eav.h>
#include <rtl8367b_asicdrv_hsb.h>
#include <smi.h>

#include <rtk_api.h>
#include <rtk_api_ext.h>

#include <linux/netdevice.h>

#include <rtk_error.h>

//#include <string.h>
#include <linux/slab.h>
#include "port.h"

char *mibcnt_string[]={
    "STAT_IfInOctets",
    "STAT_Dot3StatsFCSErrors",
    "STAT_Dot3StatsSymbolErrors",
    "STAT_Dot3InPauseFrames",
    "STAT_Dot3ControlInUnknownOpcodes",
    "STAT_EtherStatsFragments",
    "STAT_EtherStatsJabbers",
    "STAT_IfInUcastPkts",
    "STAT_EtherStatsDropEvents",
    "STAT_EtherStatsOctets",
    "STAT_EtherStatsUnderSizePkts",
    "STAT_EtherOversizeStats",
    "STAT_EtherStatsPkts64Octets",
    "STAT_EtherStatsPkts65to127Octets",
    "STAT_EtherStatsPkts128to255Octets",
    "STAT_EtherStatsPkts256to511Octets",
    "STAT_EtherStatsPkts512to1023Octets",
    "STAT_EtherStatsPkts1024to1518Octets",
    "STAT_EtherStatsMulticastPkts",
    "STAT_EtherStatsBroadcastPkts",
    "STAT_IfOutOctets",
    "STAT_Dot3StatsSingleCollisionFrames",
    "STAT_Dot3StatsMultipleCollisionFrames",
    "STAT_Dot3StatsDeferredTransmissions",
    "STAT_Dot3StatsLateCollisions",
    "STAT_EtherStatsCollisions",
    "STAT_Dot3StatsExcessiveCollisions",
    "STAT_Dot3OutPauseFrames",
    "STAT_Dot1dBasePortDelayExceededDiscards",
    "STAT_Dot1dTpPortInDiscards",
    "STAT_IfOutUcastPkts",
    "STAT_IfOutMulticastPkts",
    "STAT_IfOutBroadcastPkts",
    "STAT_OutOampduPkts",
    "STAT_InOampduPkts",
    "STAT_PktgenPkts",
    "STAT_InMldChecksumError",
    "STAT_InIgmpChecksumError",
    "STAT_InMldSpecificQuery",
    "STAT_InMldGeneralQuery",
    "STAT_InIgmpSpecificQuery",
    "STAT_InIgmpGeneralQuery",
    "STAT_InMldLeaves",
    "STAT_InIgmpInterfaceLeaves",
    "STAT_InIgmpJoinsSuccess",
    "STAT_InIgmpJoinsFail",
    "STAT_InMldJoinsSuccess",
    "STAT_InMldJoinsFail",
    "STAT_InReportSuppressionDrop",
    "STAT_InLeaveSuppressionDrop",
    "STAT_OutIgmpReports",
    "STAT_OutIgmpLeaves",
    "STAT_OutIgmpGeneralQuery",
    "STAT_OutIgmpSpecificQuery",
    "STAT_OutMldReports",
    "STAT_OutMldLeaves",
    "STAT_OutMldGeneralQuery",
    "STAT_OutMldSpecificQuery",
    "STAT_InKnownMulticastPkts",
    "STAT_IfInMulticastPkts",
    "STAT_IfInBroadcastPkts",
    "STAT_PORT_CNTR_END"
};

#define FID7	7
#define FID8	8
#define FID9	9


int RL6000_vlan_set(void){
	rtk_portmask_t mbrmsk, untag;
	rtk_api_ret_t retVal;
	int i;
	
	retVal=rtk_switch_init();
	if(retVal!=RT_ERR_OK)
		printk("rtk_switch_init failed!\n");
		
	retVal=rtk_vlan_init();
	if(retVal!=RT_ERR_OK)
       		printk("rtk_vlan_init failed!\n");

 	for(i=0;i<4096;i++)
 	{
 		if(i%100 == 0)
 			printk("--set RL6000 vlan %d--\n",i);
 	
 		if(i==7 || i==8 ||i==9)
 		{
 			mbrmsk.bits[0] = RL6000_ALL_PORT_BITMAP;
 			untag.bits[0] = RL6000_ALL_PORT_BITMAP;
 		}
 		else
 		{
 			mbrmsk.bits[0] = RL6000_ALL_PORT_BITMAP;
 			untag.bits[0] = 0;
 		}
 	
 		retVal=rtk_vlan_set(i, mbrmsk, untag, 0); //all vlan's fid is 0 
 		if(retVal!=RT_ERR_OK)
 			printk("vlan %d set failed!\n",i);
 	}

	/* set pvid :  wan:8   lan:9  */
	retVal=rtk_vlan_portPvid_set(RL6000_WAN_PORT, 8,0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set port %d failed!\n",RL6000_WAN_PORT);      
	
	for(i=0;i<RL6000_LAN_PORT_NUM;i++)
	{
		int lan_port_idx = RL6000_LAN_PORT_START + i;
		retVal=rtk_vlan_portPvid_set(lan_port_idx, 9,0);
		if(retVal!=RT_ERR_OK)
			printk("rtk_vlan_portPvid_set port %d failed!\n",lan_port_idx);
	}       
	
	retVal=rtk_vlan_portPvid_set(RL6000_CPU_PORT, 0,0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set port %d failed!\n",RL6000_CPU_PORT);   
	
	/* set wan port efid=1, other ports efid=2 */
	retVal = rtk_port_efid_set(RL6000_WAN_PORT,1);
	if(retVal!=RT_ERR_OK)
		printk("rtk_port_efid_set port %d failed!\n",RL6000_WAN_PORT);     
	for(i=0;i<RL6000_LAN_PORT_NUM;i++)
	{
		int lan_port_idx = RL6000_LAN_PORT_START + i;
		retVal = rtk_port_efid_set(lan_port_idx,2);
		if(retVal!=RT_ERR_OK)
			printk("rtk_port_efid_set port %d failed!\n",lan_port_idx);
	}
	
	/* disable cpu port's mac addr learning ability */
	rtl8367b_setAsicLutLearnLimitNo(RL6000_CPU_PORT,0);
	
	/* disable unknown unicast/mcast/bcast flooding between LAN ports */
	smi_write(RTL8367B_REG_UNDA_FLOODING_PMSK, RL6000_CPU_PORT_BITMAP);
	smi_write(RTL8367B_REG_UNMCAST_FLOADING_PMSK, RL6000_CPU_PORT_BITMAP);
	smi_write(RTL8367B_REG_BCAST_FLOADING_PMSK, RL6000_CPU_PORT_BITMAP);
	
	{
		rtk_portmask_t mbrmsk2, untag2;
		int i;
		rtk_fid_t pFid;
		rtk_vlan_t pPvid;
		rtk_pri_t pPriority;
		rtk_vlan_get(9, &mbrmsk2, &untag2, &pFid);
		printk("vid:%d  portmask:0x%x untagmask:0x%x  fid:%d\n",9, mbrmsk2.bits[0], untag2.bits[0],pFid);
		rtk_vlan_get(8, &mbrmsk2, &untag2, &pFid);
		printk("vid:%d  portmask:0x%x untagmask:0x%x  fid:%d\n",8, mbrmsk2.bits[0], untag2.bits[0],pFid);
	
		for(i=0 ;i<5; i++){
			rtk_vlan_portPvid_get(i,&pPvid,&pPriority);
			printk("port:%d  pvid:%d priority:%d\n", i, pPvid, pPriority);
		}
	}
	return 0;

}

int RL6000_wanport_bandwidth_set(int bps)
{
	//int qid = 0;

	if(rtk_rate_egrBandwidthCtrlRate_set(RL6000_CPU_PORT, bps/1000,  1)!=RT_ERR_OK)
	{
		printk("RL6000_wanport_bandwidth_set %d failed!\n",bps);
		return 1;
	}	

	#if 0
	/* when we limit WAN port output's bandwidth, disable its flow contorl for stopping it to pause 8676 switch send out downstream packets */
	for(qid=0;qid<RTL8367B_QIDMAX;qid++)
	{
		if(rtl8367b_setAsicFlowControlQueueEgressEnable(RL6000_WAN_PORT,qid,0)!=RT_ERR_OK)
		{
			printk("rtl8367b_setAsicFlowControlQueueEgressEnable qid %d failed!\n",qid);
			return 1;
		}	
	}
	#endif


	return 0;
}

int RL6000_wanport_bandwidth_flush(void)
{
	//int qid = 0;
	
	if(rtk_rate_egrBandwidthCtrlRate_set(RL6000_CPU_PORT, RTL8367B_QOS_RATE_INPUT_MAX,  1)!=RT_ERR_OK)
	{
		printk("RL6000_wanport_bandwidth_flush  failed!\n");
		return 1;
	}

	#if 0
	for(qid=0;qid<RTL8367B_QIDMAX;qid++)
	{
		if(rtl8367b_setAsicFlowControlQueueEgressEnable(RL6000_WAN_PORT,qid,1)!=RT_ERR_OK)
		{
			printk("rtl8367b_setAsicFlowControlQueueEgressEnable qid %d failed!\n",qid);
			return 1;
		}	
	}
	#endif

	return 0;
}
void RL6000_getPortStatus(void){

rtk_portmask_t mbrmsk2, untag2;
		int i;
		rtk_fid_t pFid;
		rtk_vlan_t pPvid;
		rtk_pri_t pPriority;
		rtk_vlan_get(9, &mbrmsk2, &untag2, &pFid);
		printk("vid:%d  portmask:0x%x untagmask:0x%x  fid:%d\n",9, mbrmsk2.bits[0], untag2.bits[0],pFid);

		rtk_vlan_get(8, &mbrmsk2, &untag2, &pFid);
		printk("vid:%d  portmask:0x%x untagmask:0x%x  fid:%d\n",8, mbrmsk2.bits[0], untag2.bits[0],pFid);

		rtk_vlan_get(7, &mbrmsk2, &untag2, &pFid);
		printk("vid:%d  portmask:0x%x untagmask:0x%x  fid:%d\n",7, mbrmsk2.bits[0], untag2.bits[0],pFid);

		for(i=0 ;i<5; i++){
			rtk_vlan_portPvid_get(i,&pPvid,&pPriority);
			printk("port:%d  pvid:%d priority:%d\n", i, pPvid, pPriority);
		}


}

#ifdef CONFIG_RTL_8367B
int RL6000_vlan_setAll(void){
	rtk_portmask_t mbrmsk, untag;
	rtk_api_ret_t retVal;
	
	retVal=rtk_switch_init();
	if(retVal!=RT_ERR_OK)
		printk("rtk_switch_init failed!\n");
	
	retVal=rtk_vlan_init();
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_init failed!\n");


    /*port0~port4 and port 6 in VLAN 7,8,9 */
	mbrmsk.bits[0] = 0x05f;
	untag.bits[0]  = 0x05f;
	retVal=rtk_vlan_set(7, mbrmsk, untag, FID7);
	if(retVal!=RT_ERR_OK)
		printk("vlan 7 set failed!\n");

    retVal=rtk_vlan_set(8, mbrmsk, untag, FID8);
	if(retVal!=RT_ERR_OK)
		printk("vlan 8 set failed!\n");

    retVal=rtk_vlan_set(9, mbrmsk, untag, FID9);
	if(retVal!=RT_ERR_OK)
		printk("vlan 9 set failed!\n");    		


	retVal=rtk_vlan_portPvid_set(0, 7, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 0 failed!\n");
	retVal=rtk_vlan_portPvid_set(1, 7, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 1 failed!\n");
	retVal=rtk_vlan_portPvid_set(2, 7, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 2 failed!\n");
	retVal=rtk_vlan_portPvid_set(3, 7, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 3 failed!\n");
	retVal=rtk_vlan_portPvid_set(4, 7, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 4 failed!\n");


    retVal=rtk_vlan_portPvid_set(0, 8, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 0 failed!\n");
	retVal=rtk_vlan_portPvid_set(1, 8, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 1 failed!\n");
	retVal=rtk_vlan_portPvid_set(2, 8, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 2 failed!\n");
	retVal=rtk_vlan_portPvid_set(3, 8, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 3 failed!\n");
	retVal=rtk_vlan_portPvid_set(4, 8, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 4 failed!\n");		


    retVal=rtk_vlan_portPvid_set(0, 9, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 0 failed!\n");
	retVal=rtk_vlan_portPvid_set(1, 9, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 1 failed!\n");
	retVal=rtk_vlan_portPvid_set(2, 9, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 2 failed!\n");
	retVal=rtk_vlan_portPvid_set(3, 9, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 3 failed!\n");
	retVal=rtk_vlan_portPvid_set(4, 9, 0);
	if(retVal!=RT_ERR_OK)
		printk("rtk_vlan_portPvid_set 4 failed!\n");		

	{
		rtk_portmask_t mbrmsk2, untag2;
		int i;
		rtk_fid_t pFid;
		rtk_vlan_t pPvid;
		rtk_pri_t pPriority;
		rtk_vlan_get(9, &mbrmsk2, &untag2, &pFid);
		printk("vid:%d  portmask:0x%x untagmask:0x%x  fid:%d\n",9, mbrmsk2.bits[0], untag2.bits[0],pFid);

		rtk_vlan_get(8, &mbrmsk2, &untag2, &pFid);
		printk("vid:%d  portmask:0x%x untagmask:0x%x  fid:%d\n",8, mbrmsk2.bits[0], untag2.bits[0],pFid);

        rtk_vlan_get(7, &mbrmsk2, &untag2, &pFid);
		printk("vid:%d  portmask:0x%x untagmask:0x%x  fid:%d\n",7, mbrmsk2.bits[0], untag2.bits[0],pFid);
        
		for(i=0 ;i<5; i++){
			rtk_vlan_portPvid_get(i,&pPvid,&pPriority);
			printk("port:%d  pvid:%d priority:%d\n", i, pPvid, pPriority);
		}
	}
	
	return 0;
}







void RL6000_port_isolation_setAll(void){

    rtk_portmask_t portmask;
    portmask.bits[0] = 0x40;

    int port;
    for(port=0;port<5;port++)
    {
        rtk_port_isolation_set(port , portmask);
    }	
   // rtk_port_isolation_set(RL6000_CPU_PORT , portmask);
}

void RL6000_vlan_portIgrFilterEnable_setAll(void){

    rtk_enable_t igr_filter;
    igr_filter=ENABLED;
    int port=0;
//    for(port=0;port<5;port++)
    {
        rtk_vlan_portIgrFilterEnable_set(port , igr_filter);
        
    }	

}

void RL6000_l2_limitLearningCnt_setAll(void){

int port;
   for(port=0;port<5;port++)
   {
       rtl8367b_setAsicLutLearnLimitNo(port , 0);
   } 

     rtl8367b_setAsicLutLearnLimitNo(RL6000_CPU_PORT , 0);   
}



void RL6000_cpu_tag(int enable){
	if(enable){
		smi_write(RTL8367B_REG_CPU_PORT_MASK,1<<RL6000_CPU_PORT); //set CPU port
		smi_write(RTL8367B_REG_CPU_CTRL,0x281|(RL6000_CPU_PORT)<<RTL8367B_CPU_TRAP_PORT_OFFSET);
	}
	else{
		smi_write(RTL8367B_REG_CPU_CTRL,0x280|(RL6000_CPU_PORT)<<RTL8367B_CPU_TRAP_PORT_OFFSET);
	}
}


void RL6000_RGMII(void){
	unsigned int rdata;
	smi_write(RTL8367B_REG_DIGITAL_INTERFACE_SELECT, 0x10);
	smi_read(RTL8367B_REG_DIGITAL_INTERFACE_SELECT,&rdata);
	//printk("reg.0x%x:%x\n",RTL8367B_REG_DIGITAL_INTERFACE_SELECT , rdata);
	smi_write(RTL8367B_REG_DIGITAL_INTERFACE1_FORCE, 0x1076);
	smi_write(RTL8367B_REG_EXT1_RGMXF, 0x2);
}

void RL6000_LED(void)
{
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_LINK_ACT);
	rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LINK_ACT);
	rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_LINK_ACT);
}

unsigned int RL6000_set_phy(unsigned int port, unsigned int reg, unsigned int regData, unsigned int page)
{
	unsigned int ret;

	if (port > RTK_PHY_ID_MAX)
        return RT_ERR_PORT_ID;

	if ((ret = rtl8367b_setAsicPHYReg(port, RTL8367B_PHY_PAGE_ADDRESS, page))) {
		printk("page select err!, err=0x%x\n", ret);
		return ret;
	}

	if ((ret = rtl8367b_setAsicPHYReg(port, reg, regData))) {
		printk("set phy register err!, err=0x%x\n", ret);
		return ret;
	}

	if ((ret = rtl8367b_setAsicPHYReg(port, RTL8367B_PHY_PAGE_ADDRESS, 0))) {
		printk("page select err!, err=0x%x\n", ret);
		return ret;
	}

	return ret;
}

unsigned int RL6000_get_phy(unsigned int port, unsigned int reg, unsigned int *pData, unsigned int page)
{
	unsigned int ret;
	
	if (port > RTK_PHY_ID_MAX)
		return RT_ERR_PORT_ID;

	if ((ret = rtl8367b_setAsicPHYReg(port, RTL8367B_PHY_PAGE_ADDRESS, page))) {
		printk("page select err!, err=0x%x\n", ret);
		return ret;
	}

	if ((ret = rtl8367b_getAsicPHYReg(port, reg, pData))) {
		printk("get phy register err!, err=0x%x\n", ret);
		return ret;
	}

	if ((ret = rtl8367b_setAsicPHYReg(port, RTL8367B_PHY_PAGE_ADDRESS, 0))) {
		printk("page select err!, err=0x%x\n", ret);
		return ret;
	}

	return ret;
}

void rtl8367b_accumulate_port_stats(unsigned int portnum, struct net_device_stats *net_stats)
{
    unsigned long long pCntr;
    
    if(rtk_stat_port_get(portnum, STAT_IfInUcastPkts, &pCntr)==RT_ERR_OK)
        net_stats->rx_packets +=pCntr;
    if(rtk_stat_port_get(portnum, STAT_EtherStatsMulticastPkts, &pCntr)==RT_ERR_OK)
        net_stats->rx_packets +=pCntr;
    if(rtk_stat_port_get(portnum, STAT_EtherStatsBroadcastPkts, &pCntr)==RT_ERR_OK)
        net_stats->rx_packets +=pCntr;    
    
    if(rtk_stat_port_get(portnum, STAT_IfOutUcastPkts, &pCntr)==RT_ERR_OK)    
        net_stats->tx_packets +=pCntr;
    if(rtk_stat_port_get(portnum, STAT_IfOutMulticastPkts, &pCntr)==RT_ERR_OK)    
        net_stats->tx_packets +=pCntr;
    if(rtk_stat_port_get(portnum, STAT_IfOutBroadcastPkts, &pCntr)==RT_ERR_OK)    
        net_stats->tx_packets +=pCntr;    

    if(rtk_stat_port_get(portnum, STAT_IfInOctets, &pCntr)==RT_ERR_OK)    
        net_stats->rx_bytes +=pCntr;
    if(rtk_stat_port_get(portnum, STAT_IfOutOctets, &pCntr)==RT_ERR_OK)            
        net_stats->tx_bytes +=pCntr;
        
    if(rtk_stat_port_get(portnum, STAT_Dot3StatsFCSErrors, &pCntr)==RT_ERR_OK)        
        net_stats->rx_errors +=pCntr;
    if(rtk_stat_port_get(portnum, STAT_EtherStatsJabbers, &pCntr)==RT_ERR_OK)        
        net_stats->rx_errors +=pCntr;
    if(rtk_stat_port_get(portnum, STAT_EtherOversizeStats, &pCntr)==RT_ERR_OK)        
        net_stats->rx_errors +=pCntr;        

    if(rtk_stat_port_get(portnum, STAT_EtherStatsMulticastPkts, &pCntr)==RT_ERR_OK)            
        net_stats->multicast +=pCntr;    
        
    if(rtk_stat_port_get(portnum, STAT_EtherStatsCollisions, &pCntr)==RT_ERR_OK)                    
        net_stats->collisions +=pCntr; 
        
    if(rtk_stat_port_get(portnum, STAT_Dot3StatsFCSErrors, &pCntr)==RT_ERR_OK)         
        net_stats->rx_crc_errors +=pCntr;   

}


#endif

void reset_rtl8367b_PortStat(void){
	rtk_stat_global_reset();
}

void show_rtl8367b_PortStat(unsigned int port){
	unsigned long long pCntr;
	int i;
	printk("dump mib counters for port:%d\n",port);
	
	for(i=STAT_IfInOctets;i<STAT_PORT_CNTR_END;i++){
		if((i!=STAT_Dot3StatsFCSErrors) 
				&& (i!=STAT_IfInUcastPkts) && (i!=STAT_IfInMulticastPkts) && (i!=STAT_IfInBroadcastPkts) 
				&& (i!=STAT_IfOutUcastPkts) && (i!=STAT_IfOutMulticastPkts) && (i!=STAT_IfOutBroadcastPkts)
				&& (i!=STAT_EtherStatsDropEvents) 	&& (i!=STAT_Dot1dTpPortInDiscards))
			continue;
		if(rtk_stat_port_get(port, i, &pCntr)!=RT_ERR_OK)
			printk("get %s failed!\n",mibcnt_string[i]);
		else
			printk("%s:%llu\n",mibcnt_string[i], pCntr);	
	}
	printk("\n");	
}


