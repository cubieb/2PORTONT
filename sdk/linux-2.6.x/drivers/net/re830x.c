/*	
 *	re830x.c: RTL830x driver
*/

#ifdef CONFIG_EXT_SWITCH
#include "re830x.h"
#include "re_smi.h"
#ifdef CONFIG_RE8306_API
#include "./rtl8306/Rtl8306_types.h"
#ifdef CONFIG_ETHWAN
#include "./rtl8306/rtl8306e_asicdrv.h"
#endif //CONFIG_ETHWAN
#include "./rtl8306/Rtl8306_AsicDrv.h"
#include "./rtl8306/Rtl8306_Driver_s.h"
#include "./rtl8306/Rtl8306_Driver_sd.h"
#endif

#define SUCCESS 	0
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

unsigned short chipver = RTL830X;

void select_page(unsigned int npage)
{
	unsigned short rdata;
	
	miiar_read(0, 16, &rdata);
	switch (npage) {
		case 0:
			miiar_write(0, 16, (rdata & 0x7FFF) | 0x0002);
			break;
		case 1:
			miiar_write(0, 16, rdata | 0x8002 );
			break;
		case 2:
			miiar_write(0, 16, rdata & 0x7FFD);
			break;
		case 3:
			miiar_write(0, 16, (rdata & 0xFFFD) | 0x8000);
			break;
		default: // default page 0
			miiar_write(0, 16, (rdata & 0x7FFF) | 0x0002);
	}
}

void set_8305_phy(struct r8305_struc *info)
{
	int i;
	
	for (i=0; i<=SW_PORT_NUM; i++) {
		rtl8305sc_setVLANTagInsertRemove(virt2phy[i], info->phy[virt2phy[i]].egtag);
		rtl8305sc_setAsicPortVlanIndex(virt2phy[i], info->phy[virt2phy[i]].vlanIndex);
	}
}

void set_8305_vlan(struct r8305_struc *info)
{
	int i;
	
	for (i=0; i<=SW_PORT_NUM; i++) {
		rtl8305sc_setAsicVlan(i, info->vlan[i].vid, info->vlan[i].member);
	}
}

void set_8305(struct r8305_struc *info)
{
	set_8305_vlan(info);
	set_8305_phy(info);
}

int rtl8305sc_setPower(char port,char enabled) {
	miiar_WriteBit(port, 0, 11, enabled==FALSE?1:0);
	return SUCCESS;
}

//Configure switch to be VLAN switch
int rtl8305sc_setAsicVlanEnable(char enabled) {
	miiar_WriteBit(0, 18, 8, enabled==FALSE?1:0);
	return SUCCESS;
}

//Configure switch to be VLAN tag awared
int rtl8305sc_setAsicVlanTagAware(char enabled) {
	miiar_WriteBit(0, 16, 10, enabled==FALSE?1:0);
	return SUCCESS;
}

//VLAN tag insert and remove
//11:do not insert or remove		10:add
//01:remove						00:remove and add new tags
void rtl8305sc_setVLANTagInsertRemove(char port, char option){
	unsigned short regValue;
	miiar_read(port,22,&regValue);
	regValue=(regValue&0xfffc)|option;  //add tag for frames outputed from port 1
	miiar_write(port,22,regValue);
}

//VLAN tag 802.1p priority disable
// 1: disable priority classification		0: enable priority classification
void rtl8305sc_setVLANTag1pProirity(char port, char option){
	unsigned short regValue;
	miiar_read(port,22,&regValue);
	regValue=(regValue&0xfbff)|((option&0x01)<<10);
	miiar_write(port,22,regValue);
}

int rtl8305sc_setAsicBroadcastInputDrop(char enabled) {
	miiar_WriteBit(0, 18, 13, enabled==TRUE?1:0);
	return SUCCESS;
}

//Configure switch VLAN ID and corresponding member ports
int rtl8305sc_setAsicVlan(unsigned short vlanIndex, unsigned short vid, unsigned short memberPortMask) {
	unsigned short regValue;
	
	// Mason Yu. change CPU prot.
	// (1) In order to avoid rubbing off old value, we should read old value first then set new value into switch.
	// (2) We should set 6 bits for membership.
	switch(vlanIndex) {
		case 0:/* VLAN[A] */
		miiar_read(0, 25, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(0, 25, regValue);
		miiar_read(0, 24, &regValue);		
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(0, 24, regValue);
		break;
		case 1:/* VLAN[B] */
		miiar_read(1, 25, &regValue);		
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(1, 25, regValue);
		miiar_read(1, 24, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(1, 24, regValue);
	
		break;
		case 2:/* VLAN[C] */
		miiar_read(2, 25, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(2, 25, regValue);
		miiar_read(2, 24, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(2, 24, regValue);
		break;
		case 3:/* VLAN[D] */
		miiar_read(3, 25, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(3, 25, regValue);
		miiar_read(3, 24, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(3, 24, regValue);
		break;
		case 4:/* VLAN[E] */
		miiar_read(4, 25, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(4, 25, regValue);
		miiar_read(4, 24, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(4, 24, regValue);
		break;
		case 5:/* VLAN[F] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */		
		miiar_read(0, 27, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(0, 27, regValue);		
		miiar_read(0, 26, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(0, 26, regValue);
		break;
		case 6:/* VLAN[G] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(1, 27, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(1, 27, regValue);
		miiar_read(1, 26, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(1, 26, regValue);
		break;
		case 7:/* VLAN[H] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(2, 27, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(2, 27, regValue);
		miiar_read(2, 26, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(2, 26, regValue);
		break;
		case 8:/* VLAN[I] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(3, 27, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(3, 27, regValue);
		miiar_read(3, 26, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(3, 26, regValue);
		break;
		case 9:/* VLAN[J] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(4, 27, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(4, 27, regValue);
		miiar_read(4, 26, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(4, 26, regValue);
		break;
		case 10:/* VLAN[K] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(0, 29, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(0, 29, regValue);
		miiar_read(0, 28, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(0, 28, regValue);
		break;
		case 11:/* VLAN[L] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(1, 29, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(1, 29, regValue);
		miiar_read(1, 28, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(1, 28, regValue);
		break;
		case 12:/* VLAN[M] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(2, 29, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(2, 29, regValue);
		miiar_read(2, 28, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(2, 28, regValue);
		break;
		case 13:/* VLAN[N] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(3, 29, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(3, 29, regValue);
		miiar_read(3, 28, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(3, 28, regValue);
		break;
		case 14:/* VLAN[O] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(4, 29, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(4, 29, regValue);
		miiar_read(4, 28, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(4, 28, regValue);
		break;
		case 15:/* VLAN[P] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(0, 31, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(0, 31, regValue);
		miiar_read(0, 30, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		miiar_write(0, 30, regValue);
		break;
		default:
			return -1;
	}
	return SUCCESS;
}

//Configure switch port un-tagged packet belonged VLAN index
int rtl8305sc_setAsicPortVlanIndex(unsigned short port, unsigned short vlanIndex) {
	unsigned short regValue;

	if(port>=MAX_SWITCH_PORT)
		return -1;
	if(port==6){
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(0, 26, &regValue);	
		regValue = (regValue & 0xFFF) | (vlanIndex<<12);		
		miiar_write(0, 26, regValue);	
		miiar_WriteBit(0, 16, 15, 0); /* Change to page 0 */
	}
	else{
		miiar_read(port, 24, &regValue);
		regValue = (regValue & 0xFFF) | (vlanIndex<<12);
		miiar_write(port, 24, regValue);
	}
	return SUCCESS;
}

void detect_switch_version()
{
#ifdef CONFIG_RE8306_API
	//write PHY 0 page 0 reg16.11=1
	rtl8306_setAsicPhyRegBit(0, 16, 11, 0, 1);
	//write PHY 4 page 0 reg23.5=1
	rtl8306_setAsicPhyRegBit(4, 23, 5, 0, 1);
	//read PHY 4 page 0 reg30
	miiar_read(4,30,&chipver);
#else
	chipver = RTL830X;
#endif
}

#ifdef CONFIG_RE8306_API
int32 rtl8306_getAsicQosQueueFlowControl(uint32 queue, uint32 set, uint32 *enabled);
void dump_8306_qos()
{
	uint32 enabled;
	uint32 ip, mask;
	uint32 port, queue, quemask, num, set, hisize, losize, level, prio, qid;
	uint32 burstsize;
	uint32 regValue, weight, rate, i;
	uint32 pktOn, pktOff, dscOn, dscOff, qentry[4];
	
	printk("[Qos bandwidth control configration]\n");
	printk("[Gloabal configration]\n");
	rtl8306_getAsicPhyReg(5, 28, 3, &regValue);
	regValue = regValue & 0xFF;
	printk("Leaky bucket Tick = %3d, ", regValue);
	rtl8306_getAsicPhyReg(5, 27, 3, &regValue);
	printk("Token = %3d, ", (regValue >> 8) & 0xFF);
	printk("LBTHR = %3d, ", regValue & 0xFF);
	rtl8306_getAsicPhyReg(5, 29, 3, &regValue);
	printk("Current LBCNT = 0X%3X\n", regValue & 0x7FF);
	rtl8306_getAsicQosRxRateGlobalControl(&hisize, &losize, &enabled);
	printk("Rx LB   Highsize=%2d*1KB, Lowsize=%2d*1KB, Preamble %s\n", hisize, losize, enabled == TRUE ? "Include":"Not Include");
	rtl8306_getAsicQosPortQueueNum(&num);
	printk("Port queue number -- %d queue\n", num);
	printk("[Tx queue bandwidth control]\n");		
	for (set = 0; set < 2; set ++) {
		printk("Queue configuration [set %d]\n", set);
		for (queue = 0; queue < 4; queue ++) {
			rtl8306_getAsicQosTxQueueWeight(queue, &weight, set);
			if ((queue == RTL8306_QUEUE2) || (queue == RTL8306_QUEUE3)) {
				rtl8306_getAsicQosTxQueueStrictPriority(queue, set, &enabled);
				rtl8306_getAsicQosTxQueueLeakyBucket(queue, set, &burstsize, &rate);
				printk("Q%d Weight=%3d, Burstsize=%2d*1KB, Rate=%4d*64Kbps, %8s strict PRISCH\n",
					   queue, weight, burstsize, rate, enabled == TRUE ? "Enabled":"Disabled");
			} else 
				printk("Q%d Weight=%3d\n", queue, weight);		
		}						
	}
	printk("[Per port bandwidth control]\n");
	printk("%-8s%-18s%-18s%-11s%-10s%-10s\n", "", "Rx Rate(*64kbps)", "Tx Rate(*64kbps)", "Sche mode", "Q3 LB", "Q2 LB");
	for (port = 0; port < RTL8306_PORT_NUMBER; port ++) {
		rtl8306_getAsicQosPortRate(port, &rate, RTL8306_PORT_RX, &enabled);
		printk("Port%-4d(%-4d,%-12s", port, rate, enabled == TRUE ? "Enabled)":"Disabled)");
		rtl8306_getAsicQosPortRate(port, &rate, RTL8306_PORT_TX, &enabled);
		printk("(%-4d,%-12s", rate, enabled == TRUE ? "Enabled)":"Disabled)");
		rtl8306_getAsicQosPortScheduleMode(port, &set, &quemask);
		printk("set%-8d%-10s%-10s\n", set, (quemask & 0x8) ? "Enabled":"Disabled", (quemask & 0x4) ? "Enabled":"Disabled");																	
	}

	/*dump qos priority assignment configuration*/
	printk("\n");
	printk("[Qos priority assignment ]\n");
	printk("Priority to QID mapping (PRI--QID):\n");
	for (prio = 0; prio <= RTL8306_PRIO3; prio++) {
		rtl8306_getAsicQosPrioritytoQIDMapping(prio, &qid);
		printk("P%d--Q%d, ", prio, qid);

	}
	printk("\n");
	rtl8306_getAsicQosPktPriorityAssign(RTL8306_ACL_PRIO, &level);
	printk("Different Priority type arbitration level:\nACL-BASED = %d, ", level);
	rtl8306_getAsicQosPktPriorityAssign(RTL8306_DSCP_PRIO, &level);	
	printk("DSCP-BASED = %d, ", level);
	rtl8306_getAsicQosPktPriorityAssign(RTL8306_1QBP_PRIO, &level);		
	printk("1Q_BASED = %d, ", level);
	rtl8306_getAsicQosPktPriorityAssign(RTL8306_PBP_PRIO, &level);		
	printk("Port-Based = %d\n", level);
	printk("1Q tag priority to 2-bit priority mapping(1Q-2bit)\n");
	for (i = 0; i <= RTL8306_1QTAG_PRIO7; i++) {
		rtl8306_getAsicQos1QtagPriorityto2bitPriority(i, &prio);
		printk("0x%1x-%1d, ", i, prio);	
	}
	printk("\n");
	printk("DSCP priority to 2-bit priority mapping(DSCP-2bit):\n");
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_EF, &prio);
	printk("DSCP_EF  -%d, ", prio);
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFL1, &prio);
	printk("DSCP_AFL1-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFM1, &prio);
	printk("DSCP_AFM1-%d, ", prio);	
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFH1, &prio);
	printk("DSCP_AFH1-%d\n", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFL2, &prio);
	printk("DSCP_AFL2-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFM2, &prio);
	printk("DSCP_AFM2-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFH2, &prio);
	printk("DSCP_AFH2-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFL3, &prio);
	printk("DSCP_AFL3-%d\n", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFM3, &prio);
	printk("DSCP_AFM3-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFH3, &prio);
	printk("DSCP_AFH3-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFL4, &prio);
	printk("DSCP_AFL4-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFM4, &prio);
	printk("DSCP_AFM4-%d\n", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFH4, &prio);
	printk("DSCP_AFH4-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_NC, &prio);
	printk("DSCP_NC  -%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_REG_PRI, &prio);
	printk("DSCP_REG -%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_BF, &prio);
	printk("DSCP_BF  -%d\n", prio);		
	rtl8306_getAsicQosDSCPUserAssignPriority(RTL8306_DSCP_USERA, &prio, &enabled);
	printk("User specify DSCP code A 0x%2x, %s\n", prio, enabled== TRUE ? "Enabled" : "Disabled");
	rtl8306_getAsicQosDSCPUserAssignPriority(RTL8306_DSCP_USERB, &prio, &enabled);
	printk("User specify DSCP code B 0x%2x, %s\n", prio, enabled== TRUE ? "Enabled" : "Disabled");	
	rtl8306_getAsicQosIPAddressPriority(&prio);
	printk("IP Address priority = %d\n", prio);
	rtl8306_getAsicQosIPAddress(RTL8306_IPADD_A, &ip, &mask, &enabled);
	printk("IP priority Address[A]--%8s, ip %d:%d:%d:%d, mask %d:%d:%d:%d\n",
		   enabled == TRUE ? "Enabled":"Disabled",
		   (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF,
		   (mask >> 24) & 0xFF, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask &0xFF);
	
	rtl8306_getAsicQosIPAddress(RTL8306_IPADD_B, &ip, &mask, &enabled);
	printk("IP priority Address[B]--%8s, ip %d:%d:%d:%d, mask %d:%d:%d:%d\n",
		   enabled == TRUE ? "Enabled":"Disabled",
		   (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF,
		   (mask >> 24) & 0xFF, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask &0xFF);

	printk("[Per Port Priority Configuration]\n");
	printk("%8s%-12s%-12s%-12s%-12s\n", "","DSCP-based", "1Q-based", "Port-based", "CPU TAG-based");
	for (port = 0 ; port < RTL8306_PORT_NUMBER; port ++) {
		rtl8306_getAsicQosPriorityEnable(port, RTL8306_DSCP_PRIO, &enabled);
		printk("Port%d   %-12s", port, enabled == TRUE ? "Enabled" :"Disabled");
		rtl8306_getAsicQosPriorityEnable(port, RTL8306_1QBP_PRIO, &enabled);
		rtl8306_getAsicQos1QBasedPriority(port, &prio);
		printk("%8s,%-3d ", enabled == TRUE ? "Enabled" :"Disabled", prio);
		rtl8306_getAsicQosPriorityEnable(port, RTL8306_PBP_PRIO, &enabled);
		rtl8306_getAsicQosPortBasedPriority(port, &prio);
		printk("%8s,%-3d", enabled == TRUE ? "Enabled" :"Disabled", prio);
		rtl8306_getAsicQosPriorityEnable(port, RTL8306_CPUTAG_PRIO, &enabled);
		printk("%12s\n", enabled == TRUE ? "Enabled" :"Disabled");
	}
	printk("\n");
	printk("[Flow control]\n");
	printk("%-5s%-9s%-9s%-9s%-9s\n", "", "Q0", "Q1", "Q2", "Q3");
	for (i = 0 ; i < 2; i ++) {
		printk("Set%-2d", i);
		for (queue = 0; queue < 4; queue ++) {
			rtl8306_getAsicQosQueueFlowControl(queue, i, &enabled);				
			printk("%-9s", enabled == TRUE ? "Enable" : "Disable");
		}
		printk("\n");
	}
	for (port = 0; port < 6; port ++) {
		rtl8306_getAsicQosPortFlowControlMode(port, &set);
		printk("P%d - Set%d, ", port, set);
	}
	printk("\n");
	rtl8306_getAsicQosSystemRxFlowControl(&enabled);
	printk("System Rx Flow control %s\n", enabled == TRUE ? "Enabled":"Disabled");															
	/*dump flow control threshold*/
	rtl8306_getAsicPhyReg(5, 30, 2, &regValue);
	qentry[0] = (regValue >> 8) & 0x3F;
	qentry[1] = regValue &0x3F;
	rtl8306_getAsicPhyReg(5, 31, 2, &regValue);
	qentry[2] = (regValue >> 8) & 0x3F;	
	qentry[3] = 128 - qentry[0] - qentry[1]- qentry[2];
	for (set = 0 ; set < 2; set ++) {
		printk ("QFL Set %d threshold:\n", set);
		printk("%-4s%-10s%-10s%-10s%-10s%-10sSet%d\n", "","pktOn", "pktOff", "dscOn", "dscOff", "entry", set);
		for (i = 0; i < 4; i ++)  {
			rtl8306_getAsicQosQueueFlowControlThr(i, RTL8306_FCO_QLEN, RTL8306_FCON, set, &pktOn, &enabled);
			rtl8306_getAsicQosQueueFlowControlThr(i, RTL8306_FCO_QLEN, RTL8306_FCOFF, set, &pktOff, &enabled);		
			rtl8306_getAsicQosQueueFlowControlThr(i, RTL8306_FCO_DSC, RTL8306_FCON, set,  &dscOn,  &enabled);
			rtl8306_getAsicQosQueueFlowControlThr(i, RTL8306_FCO_DSC, RTL8306_FCOFF, set, &dscOff, &enabled);
			printk("Q%-3d%-10d%-10d%-10d%-10d%-10d\n", i, pktOn, pktOff, dscOn, dscOff, qentry[i]);
		}

	}
	printk("%-8s%-10s%-10s\n", "", "dscOn", "dscOff");
	for (port = 0 ; port < 6; port ++) {
		rtl8306_getAsicQosPortFLowControlThr(port, &dscOn, &dscOff, RTL8306_PORT_TX);
		printk("Port%-4d%-10d%-10d\n", port,  dscOn, dscOff);			
	}			
}		
	
int32 rtl8306_getAsicQosQueueFlowControl(uint32 queue, uint32 set, uint32 *enabled)
{
	uint32 bitValue;
	
	if ((queue > RTL8306_QUEUE3) || (set > RTL8306_FCO_SET1) || (enabled == NULL))
		return FAILED;
	switch (queue) {
	case RTL8306_QUEUE0:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 6, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 6, 2, &bitValue);			
		break;
	case RTL8306_QUEUE1:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 7, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 7, 2, &bitValue);					
		break;
	case RTL8306_QUEUE2:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 8, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 8, 2, &bitValue);					
		break;
	case RTL8306_QUEUE3:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 9, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 9, 2, &bitValue);					
		break;
	default:
		return FAILED;		
	}			
	*enabled = (bitValue == 0 ? TRUE:FALSE	);	
	return SUCCESS;	
}

#define RTL8306_MIB_CNT1			0
#define RTL8306_MIB_CNT2			1
#define RTL8306_MIB_CNT3			2
#define RTL8306_MIB_CNT4			3
#define RTL8306_MIB_CNT5			4
#define RTL8306_MIB_RESET		0
#define RTL8306_MIB_START		1
void reset_8306_counter()
{
	int i;
	
     for (i=0;i<=5; i++)
            rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_RESET);

      for (i=0;i<=5; i++)
            rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_START);
}

void dump_8306_counter()
{
	uint32 port, unit;
	uint32 value;
	
	for (port = 0; port < RTL8306_PORT_NUMBER; port ++) {
		printk("Port%d:\n", port);
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT1, &value);												
		rtl8306_getAsicMibCounterUnit(port, RTL8306_MIB_CNT1, &unit);
		printk("TX COUNT = %d (%s; ", value, unit == RTL8306_MIB_BYTE ? "byte)" : "pkt)");
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT2, &value);
		rtl8306_getAsicMibCounterUnit(port, RTL8306_MIB_CNT2, &unit);
		printk("RX COUNT = %d (%s; ", value, unit == RTL8306_MIB_BYTE ? "byte)" : "pkt)");
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT3, &value);
		printk("RX Drop = %d(pkt);\n", value);	
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT4, &value);
		printk("RX CRCERR =%d(pkt); ", value);
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT5, &value);
		printk("RX ERR =%d(pkt)\n", value);								
	}	
}

/*Set QOS queue flow control threshold*/
int32 _rtl8306_setAsicQosQueueFlowControlThr(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32 value, uint32 enabled) {
    uint32 regValue, mask;
    uint32 selection;
    uint32 reg, shift;
	
    if ((queue > RTL8306_QUEUE3) || (type > RTL8306_FCO_QLEN) || (onoff > RTL8306_FCON) || (set > RTL8306_FCO_SET1))
        return FAILED;
    
    selection = (set << 2) | (onoff <<1) |type;
    value &= 0x7f;
    switch (selection) 
    {
        case 0 : 		/*set 0, turn off, DSC*/
	      if (queue == RTL8306_QUEUE0) {
		    reg = 17;
		    mask = 0xFFF0;
		    shift = 0;
		} else if  (queue == RTL8306_QUEUE1 ) {
                  reg = 17;
                  mask = 0xF0FF;
                  shift = 8;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 20;
			mask = 0xFFF0;
			shift = 0;
		} else  {
			reg = 20;
			mask = 0xF0FF;
			shift = 8;
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & mask) | ((value & 0xf)  << shift);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);	
        
             /*flc bit[6:4]*/
		if (queue == RTL8306_QUEUE0) 
            {
                rtl8306_getAsicPhyReg(1, 26, 3, &regValue);  
                regValue &= 0xfff8;
                regValue |= (value & 0x70) >> 4;
                rtl8306_setAsicPhyReg(1, 26, 3, regValue);                
             } 
             else if  (queue == RTL8306_QUEUE1 ) 
		{
                rtl8306_getAsicPhyReg(1, 26, 3, &regValue);  
                regValue &= 0xfe3f;
                regValue |= ((value & 0x70) >> 4) << 6;
                rtl8306_setAsicPhyReg(1, 26, 3, regValue);
		}
             else if (queue == RTL8306_QUEUE2 ) 
		{
                rtl8306_getAsicPhyReg(1, 26, 3, &regValue);  
                regValue &= 0x8fff;
                regValue |=   ((value & 0x70) >> 4) << 12;
                rtl8306_setAsicPhyReg(1, 26, 3, regValue);                
		} 
             else  
		{
                rtl8306_getAsicPhyReg(1, 27, 3, &regValue);  
                regValue &= 0xffe3;
                regValue |=   ((value & 0x70) >> 4) << 2;
                rtl8306_setAsicPhyReg(1, 27, 3, regValue);                                                		
		}             
		break;
        
	case 1 :		/*set 0, turn off, QLEN*/
		if (queue == RTL8306_QUEUE0) {
			reg = 17;
			mask = 0xFF0F;
			shift = 4;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 17;
			mask = 0x0FFF;
			shift = 12;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 20;
			mask = 0xFF0F;
			shift = 4;
		} else  {
			reg = 20;
			mask = 0x0FFF;
			shift = 12;
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & mask) | ((value & 0xf) << shift);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);		

             /*bit[6:4]*/
		if (queue == RTL8306_QUEUE0) 
             {
                rtl8306_getAsicPhyReg(1, 26, 3, &regValue);  
                regValue &= 0xffc7;
                regValue |= ((value & 0x70) >> 4) << 3;
                rtl8306_setAsicPhyReg(1, 26, 3, regValue);                                   
             } 
             else if  (queue == RTL8306_QUEUE1 ) 
		{
                rtl8306_getAsicPhyReg(1, 26, 3, &regValue);  
                regValue &= 0xf1ff;
                regValue |= ((value & 0x70) >> 4) << 9;
                rtl8306_setAsicPhyReg(1, 26, 3, regValue);                                                   
		}
             else if (queue == RTL8306_QUEUE2 ) 
		{
                /*bit[4]*/
                rtl8306_getAsicPhyReg(1, 26, 3, &regValue);                  
                regValue &= 0x7fff;
                regValue |= (((value & 0x10) >> 4) << 15);
                rtl8306_setAsicPhyReg(1, 26, 3, regValue);   

                /*bit[6:5]*/
                rtl8306_getAsicPhyReg(1, 27, 3, &regValue);         
                regValue &= 0xfffc;
                regValue |= ((value & 0x60) >> 5) ;
                rtl8306_setAsicPhyReg(1, 27, 3, regValue);                  		    
		} 
             else  
		{
                rtl8306_getAsicPhyReg(1, 27, 3, &regValue);       
                regValue &= 0xff1f;
                regValue |= ((value & 0x70) >> 4) << 5;
                rtl8306_setAsicPhyReg(1, 27, 3, regValue);                  		                    		
		}             
                     
		break;
	case 2 :		/*set 0, turn on, DSC*/
		if (queue == RTL8306_QUEUE0) 
			reg = 18;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg = 19;
		else if (queue == RTL8306_QUEUE2 )
			reg = 21;
		else  
			reg = 22;
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & 0xFFC0) | (value & 0x3f);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);		

             /*bit[6]*/
		if (queue == RTL8306_QUEUE0) 
            {
		    rtl8306_setAsicPhyRegBit(5, 21, 7, 2, (value & 0x40) ? 1:0);            
            }      
		else if  (queue == RTL8306_QUEUE1 ) 
            {
		    rtl8306_setAsicPhyRegBit(5, 21, 15, 2, (value & 0x40) ? 1:0);            
            }      	
		else if (queue == RTL8306_QUEUE2 )
            {
		    rtl8306_setAsicPhyRegBit(5, 24, 7, 2, (value & 0x40) ? 1:0);                        
            }      	
		else 
            {
		    rtl8306_setAsicPhyRegBit(5, 24, 15, 2, (value & 0x40) ? 1:0);                        
            }      
	                
		break;
	case 3:		/*set 0, turn  on, QLEN*/
		if (queue == RTL8306_QUEUE0) 
			reg = 18;
		 else if  (queue == RTL8306_QUEUE1 ) 
			reg = 19;
		 else if (queue == RTL8306_QUEUE2 ) 
			reg = 21;
		 else  
			reg = 22;	
		if (queue != RTL8306_QUEUE3)  
            {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			regValue = (regValue & 0xC0FF) | ((value & 0x3f) << 8);
			rtl8306_setAsicPhyReg(5, reg, 2, regValue);
		}  else 
		{
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			regValue = (regValue & 0x3FF) | ((value & 0x3f) << 10);
			rtl8306_setAsicPhyReg(5, reg, 2, regValue);			
		}	
        
             /*bit[6]*/
		if (queue == RTL8306_QUEUE0) 
            {
		    rtl8306_setAsicPhyRegBit(5, 21, 6, 2, (value & 0x40) ? 1:0);    
            }      
		else if  (queue == RTL8306_QUEUE1 ) 
            {
		    rtl8306_setAsicPhyRegBit(5, 21, 14, 2, (value & 0x40) ? 1:0);     
                 
            }      	
		else if (queue == RTL8306_QUEUE2 )
            {
		    rtl8306_setAsicPhyRegBit(5, 24, 6, 2, (value & 0x40) ? 1:0);                        
            }      	
		else 
            {
		    rtl8306_setAsicPhyRegBit(5, 24, 14, 2, (value & 0x40) ? 1:0);                        
            }                   
		break;
        
	case 4:		/*set 1, turn off, DSC*/
		if (queue == RTL8306_QUEUE0) 
            {
			reg = 23;
			mask = 0xFFF0;
			shift =0;
		} else if  (queue == RTL8306_QUEUE1 ) 
		{
			reg = 23;
			mask = 0xF0FF;
			shift =8;
		} else if (queue == RTL8306_QUEUE2 )
		{
			reg = 26;
			mask = 0xFFF0;
			shift =0;
		} else 
		{
			reg = 26;
			mask = 0xF0FF;
			shift =8;		
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & mask) | ((value & 0xf) << shift);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);		

             /*flc bit[6:4]*/
		if (queue == RTL8306_QUEUE0) 
            {
                rtl8306_getAsicPhyReg(1, 27, 3, &regValue);  
                regValue &= 0xf8ff;
                regValue |= ((value & 0x70) >> 4) << 8;
                rtl8306_setAsicPhyReg(1, 27, 3, regValue);                
             } 
             else if  (queue == RTL8306_QUEUE1 ) 
		{
                
                rtl8306_getAsicPhyReg(1, 27, 3, &regValue);  
                regValue &= 0x3fff;
                regValue |= ((value & 0x30) >> 4) << 14;
                rtl8306_setAsicPhyReg(1, 27, 3, regValue);           
                rtl8306_setAsicPhyRegBit(1, 28, 0, 3, (value & 0x40) ? 1:0);
                    
		}
             else if (queue == RTL8306_QUEUE2 ) 
		{
                rtl8306_getAsicPhyReg(1, 28, 3, &regValue);  		
                regValue &= 0xff8f;
                regValue |= ((value & 0x70) >> 4) << 4;
                rtl8306_setAsicPhyReg(1, 28, 3, regValue);
		} 
             else  
		{
                rtl8306_getAsicPhyReg(1, 28, 3, &regValue);  		
                regValue &= 0xe3ff;
                regValue |= ((value & 0x70) >> 4) << 10;
                rtl8306_setAsicPhyReg(1, 28, 3, regValue);                
		}             
           
		break;
	case 5:		/*set 1, turn off, QLEN*/
		if (queue == RTL8306_QUEUE0) {
			reg = 23;
			mask = 0xFF0F;
			shift = 4;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 23;
			mask = 0x0FFF;
			shift = 12;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 26;
			mask = 0xFF0F;
			shift = 4;
		} else  {
			reg = 26;
			mask = 0x0FFF;
			shift = 12;		
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & mask) | ((value & 0xf) << shift);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);	

             /*bit[6:4]*/
		if (queue == RTL8306_QUEUE0) 
             {
                rtl8306_getAsicPhyReg(1, 27, 3, &regValue);  
                regValue &= 0xc7ff;
                regValue |= ((value & 0x70) >> 4) << 11;
		   rtl8306_setAsicPhyReg(1, 27, 3, regValue);	                                
             } 
             else if  (queue == RTL8306_QUEUE1 ) 
		{
                rtl8306_getAsicPhyReg(1, 28, 3, &regValue);  
                regValue &= 0xfff1;
                regValue |= ((value & 0x70) >> 4) << 1;
                rtl8306_setAsicPhyReg(1, 28, 3, regValue);                                
		}
             else if (queue == RTL8306_QUEUE2 ) 
		{
                rtl8306_getAsicPhyReg(1, 28, 3, &regValue);  
                regValue &= 0xfc7f;
                regValue |= ((value & 0x70) >> 4) << 7;
                rtl8306_setAsicPhyReg(1, 28, 3, regValue);                                
		} 
             else  
		{
                rtl8306_getAsicPhyReg(1, 28, 3, &regValue);
                regValue &= 0x1fff;
                regValue |= ((value & 0x70) >> 4) << 13;
                rtl8306_setAsicPhyReg(1, 28, 3, regValue);                                                
		}                                 
		break;
        
	case 6:		/*set 1, turn on, DSC*/
		if (queue == RTL8306_QUEUE0) 
			reg = 24;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg =25;
		else if (queue == RTL8306_QUEUE2 ) 
			reg = 27;
		else  
			reg = 28;		
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & 0xFFC0) | (value & 0x3f);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);						

             /*bit[6]*/
		if (queue == RTL8306_QUEUE0) 
            {
		    rtl8306_setAsicPhyRegBit(5, 25, 7, 2, (value & 0x40) ? 1:0);            
            }      
		else if  (queue == RTL8306_QUEUE1 ) 
            {
		    rtl8306_setAsicPhyRegBit(5, 25, 15, 2, (value & 0x40) ? 1:0);            
            }      	
		else if (queue == RTL8306_QUEUE2 )
            {
		    rtl8306_setAsicPhyRegBit(5, 27, 7, 2, (value & 0x40) ? 1:0);            
            }      	
		else 
            {
		    rtl8306_setAsicPhyRegBit(5, 27, 15, 2, (value & 0x40) ? 1:0);            
            }      
        
		break;
        
	case 7:		/*set 1, turn  on, QLEN*/
		if (queue == RTL8306_QUEUE0) 
			reg = 24;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg =25;
		else if (queue == RTL8306_QUEUE2 ) 
			reg = 27;
		else  
			reg = 28;		
		if (queue != RTL8306_QUEUE3)  
            {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			regValue = (regValue & 0xC0FF) | ((value & 0x3f) << 8);
			rtl8306_setAsicPhyReg(5, reg, 2, regValue);
		}  else 
		{
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			regValue = (regValue & 0x3FF) | ((value & 0x3f) << 10);
			rtl8306_setAsicPhyReg(5, reg, 2, regValue);			
		}	

             /*bit[6]*/
		if (queue == RTL8306_QUEUE0) 
            {
		    rtl8306_setAsicPhyRegBit(5, 25, 6, 2, (value & 0x40) ? 1:0);            
            }      
		else if  (queue == RTL8306_QUEUE1 ) 
            {
		    rtl8306_setAsicPhyRegBit(5, 25, 14, 2, (value & 0x40) ? 1:0);            
            }      	
		else if (queue == RTL8306_QUEUE2 )
            {
		    rtl8306_setAsicPhyRegBit(5, 27, 6, 2, (value & 0x40) ? 1:0);                        
            }      	
		else 
            {
		    rtl8306_setAsicPhyRegBit(5, 27, 14, 2, (value & 0x40) ? 1:0);                        
            }              
		break;
		
	default:
		return FAILED;
	}

	/*Enable/Disable Flow control of the specified queue*/
	switch (queue) {
	case RTL8306_QUEUE0:
		if (set == RTL8306_FCO_SET0)
			rtl8306_setAsicPhyRegBit(5, 22, 6, 2, enabled ? 0:1);
		else 
			rtl8306_setAsicPhyRegBit(5, 28, 6, 2, enabled ? 0:1);			
		break;
	case RTL8306_QUEUE1:
		if (set == RTL8306_FCO_SET0)
			rtl8306_setAsicPhyRegBit(5, 22, 7, 2, enabled ? 0:1);
		else 
			rtl8306_setAsicPhyRegBit(5, 28, 7, 2, enabled ? 0:1);					
		break;
	case RTL8306_QUEUE2:
		if (set == RTL8306_FCO_SET0)
			rtl8306_setAsicPhyRegBit(5, 22, 8, 2, enabled ? 0:1);
		else 
			rtl8306_setAsicPhyRegBit(5, 28, 8, 2, enabled ? 0:1);					
		break;
	case RTL8306_QUEUE3:
		if (set == RTL8306_FCO_SET0)
			rtl8306_setAsicPhyRegBit(5, 22, 9, 2, enabled ? 0:1);
		else 
			rtl8306_setAsicPhyRegBit(5, 28, 9, 2, enabled ? 0:1);					
		break;
	default:
		return FAILED;
	}			
	return SUCCESS;
}

int32 _rtl8306_setAsicQosPortFlowControlThr(uint32 port, uint32 onthr, uint32 offthr, uint32 direction ) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) || (direction > 1))
		return FAILED;
	regValue = (offthr << 8) + onthr;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  	
	if (direction == RTL8306_PORT_TX) 
		rtl8306_setAsicPhyReg(port, 20, 2, regValue);
	else 
		rtl8306_setAsicPhyReg(port, 19, 3, regValue);
	return SUCCESS;
}


//check switch version and apply correct qos parameter for better performance
void _rtl8306_QoS_parameter_patch(void){	

	if(chipver == RTL8306G){
		int port;
		//printk("apply 8306G Qos parameters\n");
		for (port = 0; port < 5; port ++)
            	{
			rtl8306_setAsicPhyReg(port, 4, 0, 0x01E1);
			rtl8306_setAsicPhyRegBit(port, 0, 9, 0, 1);
		}
		rtl8306_setAsicPhyReg(1, 20, 0, 0xBC);
		rtl8306_setAsicPhyReg(5, 17, 2, 0xFDFF);
		rtl8306_setAsicPhyReg(5, 18, 2, 0x0F0F);
		rtl8306_asicSoftReset();
	}
	else if(chipver == RTL8306N){

		/*----------------------add patch for 6167B------------------------*/
		//printk("apply 6167B patch\n");   
		
		/*setSETO_THRESH][9:0]*/
		rtl8306_setAsicPhyReg(2, 20, 0, 0x178);
		
		 /*set SETF_THRESH[9:0]*/
		rtl8306_setAsicPhyReg(1, 20, 0, 0xDC);
	
		 /*set CLRF_THRESH[9:0]*/
		rtl8306_setAsicPhyReg(1, 21, 0, 0xDC);
	 
		 /*set CLRO_THRESH[9:0]*/
		rtl8306_setAsicPhyReg(2, 21, 0, 0x170);
	 	
		/*set port 0 queue flow control mode to be set 1*/ 
		rtl8306_setAsicPhyRegBit(0, 18, 12, 2, 1);

		#ifdef CONFIG_ETHWAN
		/*set set 0 queue flow control threshold*/ 
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE2, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 24, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE2, RTL8306_FCO_QLEN, RTL8306_FCOFF, 0, 24, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE2, RTL8306_FCO_DSC, RTL8306_FCON, 0, 90, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE2, RTL8306_FCO_DSC, RTL8306_FCOFF, 0, 90, TRUE);
			
		/*set set 1 queue flow control threshold*/	
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE2, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 24, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE2, RTL8306_FCO_QLEN, RTL8306_FCOFF, 1, 15, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE2, RTL8306_FCO_DSC, RTL8306_FCON, 1, 13,  TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE2, RTL8306_FCO_DSC, RTL8306_FCOFF, 1, 6,  TRUE);
		#else
		/*set set 0 queue flow control threshold*/ 
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE0, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 24, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE0, RTL8306_FCO_QLEN, RTL8306_FCOFF, 0, 24, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE0, RTL8306_FCO_DSC, RTL8306_FCON, 0, 90, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE0, RTL8306_FCO_DSC, RTL8306_FCOFF, 0, 90, TRUE);
	
		/*set set 1 queue flow control threshold*/	
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE0, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 24, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE0, RTL8306_FCO_QLEN, RTL8306_FCOFF, 1, 15, TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE0, RTL8306_FCO_DSC, RTL8306_FCON, 1, 13,  TRUE);
		_rtl8306_setAsicQosQueueFlowControlThr(RTL8306_QUEUE0, RTL8306_FCO_DSC, RTL8306_FCOFF, 1, 6,  TRUE);
		#endif
	
	
		 /*set per port flow control threshold*/ 
		_rtl8306_setAsicQosPortFlowControlThr(RTL8306_PORT0, 90, 90,  RTL8306_PORT_TX);
		_rtl8306_setAsicQosPortFlowControlThr(RTL8306_PORT1, 90, 90,  RTL8306_PORT_TX);
		_rtl8306_setAsicQosPortFlowControlThr(RTL8306_PORT2, 90, 90,  RTL8306_PORT_TX);
		_rtl8306_setAsicQosPortFlowControlThr(RTL8306_PORT3, 90, 90,  RTL8306_PORT_TX);
		_rtl8306_setAsicQosPortFlowControlThr(RTL8306_PORT4, 90, 90,  RTL8306_PORT_TX);
		_rtl8306_setAsicQosPortFlowControlThr(RTL8306_PORT5, 90, 90,  RTL8306_PORT_TX);
	 
		/*enable new flow control method*/			   
		rtl8306_setAsicPhyRegBit(5, 29, 15, 2, 1);
	
		rtl8306_asicSoftReset();					   

	}
	else{
		//printk("non 8306n swithc\n");	
	}
}
#endif // of CONFIG_RE8306_API

#endif // of CONFIG_EXT_SWITCH
