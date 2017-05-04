/*
 * re_igmp.c: A Linux Ethernet IGMP driver for Realtek 867x.
 */

#include <linux/if.h>
#include <linux/netdevice.h>
#include "re867x.h"
#include "re830x.h"
#include "re_smi.h"
#include "re_igmp.h"
#include <linux/ip.h>

// Mason Yu. MLD snooping
#ifdef CONFIG_IPV6
int timer_running_mld=0;
struct timer_list seq_timer_mld;
struct timer_arg timeout_arg_mld;
int MLD_SNP_registered=0;
int enable_MLD_SNP=0;
#endif

int enable_IGMP_SNP=0;
int debug_igmp=0;
int  wlan_igmp_tag = 0x1f; 
#ifdef CONFIG_EXT_SWITCH
#define MAX_IGMP_SNOOPING_GROUP		6
#define IGMP_SNOOPING_PVID_START	10
#undef IGMP_FAST_LEAVE

int DropUnknownMulticast=1; //when IGMP snooping is enabled, 0:allow all multicast traffic  1:drop unknown multicast traffic
int REST_IGMP_SNOOPING_GROUP=MAX_IGMP_SNOOPING_GROUP;
int current_snooping_group=0;
int IGMP_SNP_registered=0;
#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
struct IGMP_SNOOPING snooping_table[MAX_IGMP_SNOOPING_GROUP];
#else // IGMP_CPU_TAG
struct IGMP_SNOOPING *snooping_pool;
#endif
struct timer_list seq_timer;
struct timer_arg timeout_arg;
int timer_running=0;

struct IGMP_SNOOPING{
	//unsigned char VLAN_index;
	unsigned int multicast_addr;
	unsigned char multicast_addr_v6[16];	// Mason Yu. MLD snooping
	unsigned char member_port;
	unsigned char member_wlan;
	unsigned short VLANID;
	unsigned char tmp_member_port;
	unsigned char tmp_member_wlan;
	struct timer_list	snp_timer;
	unsigned char timer_running;
	struct IGMP_SNOOPING *next;
	unsigned long expires[6];//0-3 for lan0-lan3, 4 for usb, 5 for wlan
};

struct timer_arg{
	unsigned char count;
	struct sk_buff *skb;
	struct sk_buff *old_skb;
	int expire;
};

#ifdef IGMP_FAST_LEAVE
unsigned int handle_leave = 0, leave_query_count = 0;
unsigned char leave_port = 0;
unsigned int leave_multicast_addr;
int handle_IGMP_leave(int grp, struct sk_buff *skb);
#endif

static void remove_IGMP_Group(struct IGMP_SNOOPING *igmpGroup)
{
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	igmpGroup->multicast_addr=0xc0000000;
	igmpGroup->member_port =0;
	#else // IGMP_CPU_TAG
	struct IGMP_SNOOPING *igmp_group,*igmp_group_prev;	
	
	for(igmp_group_prev=igmp_group=snooping_pool; igmp_group; igmp_group_prev=igmp_group,igmp_group = igmp_group->next)
	{
		if(igmp_group->multicast_addr == igmpGroup->multicast_addr)
		{
			if(igmp_group_prev == igmp_group)/*remove the head*/
				snooping_pool = igmp_group->next;
			else
				igmp_group_prev->next= igmp_group->next;
			kfree(igmpGroup);
			break;
			
		}
	}
	#endif
}

void check_IGMP_snoop_rx(struct sk_buff *skb, int tag)
{
	int in_snp_table=0;
	#if (VPORT_USE==VPORT_VLAN_TAG)
	int i;
	#endif
	unsigned char igmp_offset=0;	
	unsigned char port;
	unsigned int multicast_addr;
	int isIgmpV3report = 0;
	int isIgmpV3leave =0;
	struct IGMP_SNOOPING *igmp_group;
	
	//tylo, for IGMP snooping
	#define IP_TYPE_OFFSET			0x0c
	#define IP_TYPE				0x0800
	#define IP_HDLEN_OFFSET			0x0e
	#define PROTOCOL_OFFSET			0x09
	#define IGMP_PROTOCOL			0x02
	#define IGMP_TYPE_REPORT_JOIN		0x16
	#define IGMPV3_TYPE_REPORT_JOIN		0x22
	#define IGMP_TYPE_REPORT_LEAVE		0x17
	#define MULTICAST_ADDR_REPORT_OFFSET	0x04
	#define MULTICAST_ADDR_REPORT_OFFSET_V3 0x0c
	#define IGMPV3_GROUPTAG_OFFSET          0x08
	#define IGMPV3_IS_INCLUDE               0x03
	#define IGMPV3_IS_EXCLUDE               0x04  


	//check if incoming frame is IGMP packet
	if(((*(unsigned char *)(skb->data+PROTOCOL_OFFSET))!=IGMP_PROTOCOL))
		return;
		
	//retrieve the input port
	#if (VPORT_USE==VPORT_CPU_TAG)
	if(tag == wlan_igmp_tag) {
		port =5;
	}
	else 
		port=tag;
	#else // VPORT_VLAN_TAG
	if(tag == wlan_igmp_tag) {
		port =5;
	}
	else {
		for(i=0;i<SW_PORT_NUM;i++)
		if (tag == rtl8305_info.vlan[rtl8305_info.phy[virt2phy[i]].vlanIndex].vid)
			break;
		port = virt2phy[i];
	} 
	#endif
	// Get multicast address
	igmp_offset=((*(unsigned char *)(skb->data))&0x0f)*4;
	if((*(unsigned char *)(skb->data+igmp_offset))==IGMPV3_TYPE_REPORT_JOIN){
		if(*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET)==IGMPV3_IS_INCLUDE)
		{
			if((*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET+1)== 0)&&(*(unsigned short *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET+2)== 0))
				isIgmpV3leave=1;
		}
		//else if(*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET)==IGMPV3_IS_EXCLUDE){
		else if(*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET)==IGMPV3_IS_EXCLUDE ||
				*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET)==0x02){
			if((*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET+1)== 0)&&(*(unsigned short *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET+2)== 0))
				isIgmpV3report=1;
		}
		
		multicast_addr=READWD(skb->data+igmp_offset+MULTICAST_ADDR_REPORT_OFFSET_V3);
	}
	else 
		multicast_addr=READWD(skb->data+igmp_offset+MULTICAST_ADDR_REPORT_OFFSET);
	
	//join group
	if(((*(unsigned char *)(skb->data+igmp_offset))==IGMP_TYPE_REPORT_JOIN ||  isIgmpV3report) &&  REST_IGMP_SNOOPING_GROUP!=0){
		// Kaohj -- bypass uPnp and reserved multicast packets
		if(multicast_addr==0xeffffffa || // uPnP
			(multicast_addr & 0xffffff00) == 0xe0000000) // Reserved multicast group
			return;
		if (debug_igmp) {
			printk("Get IGMP Report %u.%u.%u.%u from port %d\n", NIPQUAD(multicast_addr), port);
		}
		in_snp_table=0;
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		for (i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
			igmp_group = &snooping_table[i];
		#else // IGMP_CPU_TAG
		for (igmp_group = snooping_pool; igmp_group; igmp_group = igmp_group->next) {
		#endif
			if(multicast_addr==igmp_group->multicast_addr){
				in_snp_table=1;
				break;
			}
		}
		if(in_snp_table) { // Entry exists, update it.
			if(port!=5) { //port 5 for wlan
				igmp_group->tmp_member_port |= (1<<port);
				if ((igmp_group->member_port & (1<<port))==0) {
					// New member
					igmp_group->member_port |= (1<<port);
					igmp_group->tmp_member_port |= 1<<port;
					#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
					// New member, update VLAN associated to this group
					rtl8305sc_setAsicVlan(i+IGMP_SNOOPING_PVID_START,i+0x100,igmp_group->member_port);
					#endif
				}
			}
			else {
				igmp_group->tmp_member_wlan=1;
				igmp_group->member_wlan=1;
			}			
		}
		else { // find empty entry in IGMP_snooping table and create a vlan
			if(REST_IGMP_SNOOPING_GROUP>0) {
				#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
				for (i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
					if(snooping_table[i].multicast_addr==0xc0000000)
						break;
				}					
				if (i == MAX_IGMP_SNOOPING_GROUP)
					return;
				igmp_group = &snooping_table[i];
				#else // IGMP_CPU_TAG
				if ((igmp_group = (struct IGMP_SNOOPING *) kzalloc(sizeof(struct IGMP_SNOOPING), GFP_KERNEL|__GFP_NOFAIL))==NULL) {
					printk("%s alloc igmp group error",__FUNCTION__);
					return;
				}
				//memset(igmp_group,0,sizeof(struct IGMP_SNOOPING));
				//igmp_group->expires[port] = jiffies + igmpSnp_host_timeout*HZ/1000;
				//igmp_group->VLANID=0x100+i;
				#endif
				REST_IGMP_SNOOPING_GROUP--;
				igmp_group->multicast_addr=multicast_addr;
				if (port!=5) {
					igmp_group->member_port |= 1<<port;
					igmp_group->tmp_member_port |= 1<<port;
				}
				else {
					igmp_group->tmp_member_wlan=1;
					igmp_group->member_wlan=1;
				}
				#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
				if (debug_igmp) {
					printk("create VLAN:");
					printk("vlanindex:%x   vid:%x   member_port:%x  port:%x\n\n",i+9,i+0x100,igmp_group->member_port,port);
				}
				rtl8305sc_setAsicVlan(i+IGMP_SNOOPING_PVID_START,i+0x100,igmp_group->member_port);
				#else // IGMP_CPU_TAG
				// attach to the head
				igmp_group->next = snooping_pool;
				snooping_pool = igmp_group;
				#endif
			}
		}
	}
	//leave group
	if (((*(unsigned char *)(skb->data+igmp_offset))==IGMP_TYPE_REPORT_LEAVE)||isIgmpV3leave) {
		if (debug_igmp) {
			printk("Get IGMP Leave %u.%u.%u.%u\n", NIPQUAD(multicast_addr));
		}
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
			igmp_group = &snooping_table[i];
		#else // IGMP_CPU_TAG
		for (igmp_group = snooping_pool; igmp_group; igmp_group = igmp_group->next) {
		#endif
			if (multicast_addr==igmp_group->multicast_addr) {
				#ifdef IGMP_FAST_LEAVE
				handle_leave = 1;
				leave_port = port;
				leave_multicast_addr = multicast_addr;
				handle_IGMP_leave(i, skb);
				#else
				if (port!=5)
					igmp_group->member_port &= ~(1<<port);
				else
					igmp_group->member_wlan=0;
				if (igmp_group->member_port==0 && igmp_group->member_wlan==0) {
					if (debug_igmp)
						printk("Remove IGMP group %u.%u.%u.%u\n", NIPQUAD(multicast_addr));
					remove_IGMP_Group(igmp_group);
					REST_IGMP_SNOOPING_GROUP++;
				}
				else {
					if (debug_igmp)
						printk("Remove member port %x\n",port);
					#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
					rtl8305sc_setAsicVlan(i+IGMP_SNOOPING_PVID_START,i+0x100,igmp_group->member_port);
					#endif
				}
				#endif
				break;
			}
		}
		if (debug_igmp)
			printk("\n");
	}
}       
#else
void check_IGMP_snoop_rx(struct sk_buff *skb, int tag)
{
}
#endif // of CONFIG_EXT_SWITCH

int check_wlan_mcast_tx(struct sk_buff *skb)
{
	#ifdef CONFIG_EXT_SWITCH
	#define IP_HDR_DES_OFFSET		0x1e
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	unsigned int multicast_addr;
	struct IGMP_SNOOPING *igmp_group;
	
	//check if incoming frame is multicast packet
	if((skb->data[0]!=0x01) || (skb->data[1]!=0x0) || ((skb->data[2]&0xfe)!=0x5e)){
		//bypass non-multicast pkt
		return 0;
	}		
	//insert VLANID for multicast packets
	//printk("%x\n",*(unsigned int *)(skb->data+IP_HDR_DES_OFFSET));
	multicast_addr=READWD(skb->data+IP_HDR_DES_OFFSET);
	if(!((multicast_addr& 0xffffff00) == 0xe0000000)){
		//printk("multi-cast traffic!  %x\n",*(unsigned int *)(skb->data+IP_HDR_DES_OFFSET));
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
			igmp_group =  &snooping_table[i];
		#else // IGMP_CPU_TAG
		for(igmp_group = snooping_pool; igmp_group; igmp_group = igmp_group->next) {
		#endif
			if (multicast_addr==igmp_group->multicast_addr) {
				if (igmp_group->member_wlan==0)
					return 1;
				else
					return 0;
			}
		}
		
		if (skb->from_dev != NULL) {
			if ((skb->from_dev->priv_flags==IFF_DOMAIN_WAN) && DropUnknownMulticast)
				return 1;
		}
		else
			return 0;
	}
	return 0;
	#else
	return 0;
	#endif
}

#ifdef CONFIG_EXT_SWITCH
// Kaohj
/*
 *	Timeout function for IGMP Query of IGMP snooping
 *	Update multicast group status.
 */
static void snp_query_timeout(unsigned long arg)
{
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int idx;
	#endif
	struct timer_arg *t_arg=(struct timer_arg *)arg;
	struct sk_buff *skb;
	struct IGMP_SNOOPING *igmp_group;
	#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
	struct IGMP_SNOOPING *next_group;
	#endif
	
	skb = t_arg->old_skb;
	//printk("snp_timeout\n");
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	for (idx=0;idx<MAX_IGMP_SNOOPING_GROUP;idx++) {
		igmp_group = &snooping_table[idx];
	#else // IGMP_CPU_TAG
	igmp_group = snooping_pool;
	while (igmp_group) {
		next_group = igmp_group->next;
	#endif
		if (igmp_group->multicast_addr!=0xc0000000) {
			//printk("general query:update VLAN accroding to tmp_member_port\n");
			if (igmp_group->tmp_member_port==0 && igmp_group->tmp_member_wlan==0) {
				#ifdef IGMP_FAST_LEAVE
				if (handle_leave == 1 && igmp_group->multicast_addr == leave_multicast_addr) {
					if (leave_query_count > 0 && leave_query_count < 3) {
						igmp_group->timer_running=0;
						igmp_group->member_wlan=snooping_table[idx].tmp_member_wlan;
						timer_running=0;
						handle_IGMP_leave(idx, skb);
						dev_kfree_skb(skb);
						return;
					}
					leave_query_count = 0;
					handle_leave = 0;
				}
				#endif
				if (debug_igmp)
					printk("Group %u.%u.%u.%u timeout, delete it.\n", NIPQUAD(igmp_group->multicast_addr));
				remove_IGMP_Group(igmp_group);
				REST_IGMP_SNOOPING_GROUP++;
			}
			else{
				#ifdef IGMP_FAST_LEAVE
				if (handle_leave == 1 && igmp_group->multicast_addr == leave_multicast_addr) {
					if (leave_query_count > 0 && leave_query_count < 3) {
						igmp_group->timer_running=0;
						igmp_group->member_wlan=igmp_group->tmp_member_wlan;
						timer_running=0;
						handle_IGMP_leave(idx, skb);
						dev_kfree_skb(skb);
						return;
					}
					if (debug_igmp)
						printk("Remove member port:%x\n", leave_port);
					leave_query_count = 0;
					handle_leave = 0;
					leave_port = 0;
				}
				#endif
				#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
				if (debug_igmp)
					printk("Update VLAN %x VID=%x\n",idx+IGMP_SNOOPING_PVID_START,igmp_group->VLANID);
				if(igmp_group->member_port!=igmp_group->tmp_member_port)
					rtl8305sc_setAsicVlan(idx+IGMP_SNOOPING_PVID_START,idx+0x100,igmp_group->tmp_member_port);
				#endif
				igmp_group->timer_running=0;
				igmp_group->member_port=igmp_group->tmp_member_port;
				igmp_group->member_wlan=igmp_group->tmp_member_wlan;
			}
		}
		#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
		igmp_group = next_group;
		#endif
	}
	dev_kfree_skb(skb);
	timer_running=0;
}

#ifdef IGMP_FAST_LEAVE
#include <linux/ip.h>
static char iphdr_data[20] = {0x45, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x40, 0x00,
	0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x01};
int handle_IGMP_leave(int grp, struct sk_buff *skb)
{
	#define IP_HDLEN_OFFSET			0x0e
	#define IP_HDR_DSTIP_OFFSET		0x1e
	#define IP_HDR_SRCIP_OFFSET		0x1a
	#define IGMP_TYPE_QUERY			0x11
	#define MULTICAST_ADDR_REPORT_OFFSET	0x04
	#define IGMP_PROTOCOL			0x02

	int igmp_offset;
	unsigned short tmp_csum;
	unsigned char iphdr_insert[20], dstmac[6];
	struct net_device *dev = eth_net_dev;
	struct ethhdr *eth;
	struct iphdr *iph;
	struct sk_buff *skb_query, *skb_backup;
	int *group_addr;
	short ip_check;
	struct re_private *cp = eth_net_dev->priv;

	// skb->data == IP packet; IGMP Leave received
	if (timer_running==0)
		if (snooping_table[grp].multicast_addr!=0xc0000000 && snooping_table[grp].timer_running==0) {
			if ((skb_backup = dev_alloc_skb(2048)) == NULL) {
				printk("\n%s alloc skb_backup failed!\n", __FUNCTION__);
				return 0;
			}
			memcpy(skb_put(skb_backup, skb->len),skb->data,skb->len);
			if ((skb_query = dev_alloc_skb(2048)) == NULL) {
				printk("\n%s alloc skb failed!\n", __FUNCTION__);
				return 0;
			}
			// preparing IGMP Query
			skb_put(skb_query, ETH_HLEN+20+8); // eth+ip+igmp
			// skb_query->data == Ethernet packet
			igmp_offset=((*(unsigned char *)skb->data)&0x0f)*4;
			snooping_table[grp].tmp_member_port=0;
			snooping_table[grp].tmp_member_wlan=0;

			// handle mac header
			eth = (struct ethhdr *)skb_query->data;
			eth->h_proto = __constant_htons(ETH_P_IP);
			memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
			dstmac[0] = 0x01; dstmac[1] = dstmac[3] = dstmac[4] = 0x00; dstmac[2] = 0x5e; dstmac[5] = 0x01;
			memcpy(eth->h_dest, dstmac, ETH_ALEN);
			// handle IP header
			memcpy(skb_query->data+ETH_HLEN, iphdr_data, 20);
			memcpy(skb_query->data+IP_HDR_SRCIP_OFFSET, skb->data+12, 4); // src IP
			skb_query->data[IP_HDR_SRCIP_OFFSET+3]++;
			skb_query->data[IP_HDR_DSTIP_OFFSET] = 0xe0; // dst IP: 224.0.0.1
			skb_query->data[IP_HDR_DSTIP_OFFSET+1] = 0;
			skb_query->data[IP_HDR_DSTIP_OFFSET+2] = 0;
			skb_query->data[IP_HDR_DSTIP_OFFSET+3] = 1;
			// handle IGMP Query message
			igmp_offset=ETH_HLEN+20;
			skb_query->data[igmp_offset] = IGMP_TYPE_QUERY;
			timeout_arg.expire = 1;
			//change IGMP max. response time
			skb_query->data[igmp_offset+1] = 0x0a; // 1 sec.
			//recompute IGMP checksum
			skb_query->data[igmp_offset+2] = 0;
			skb_query->data[igmp_offset+3] = 0;
			group_addr = (int *)&skb_query->data[igmp_offset+MULTICAST_ADDR_REPORT_OFFSET];
			*group_addr = snooping_table[grp].multicast_addr;
			tmp_csum = ip_compute_csum(skb_query->data+igmp_offset, 8);
			//set IGMP checksum
			skb_query->data[igmp_offset+2] = ((tmp_csum&0xff00)>>8);
			skb_query->data[igmp_offset+3] = (tmp_csum&0xff);
			skb_query->data[igmp_offset+MULTICAST_ADDR_REPORT_OFFSET+4] = '\0';
			skb_query->data[IP_HDR_DSTIP_OFFSET-6] = 0;
			skb_query->data[IP_HDR_DSTIP_OFFSET-5] = 0;
			// set IP checksum
			memcpy(iphdr_insert, skb_query->data+ETH_HLEN, 20);
			ip_check = ip_compute_csum(iphdr_insert, 20);
			(*(unsigned char *)(skb_query->data+IP_HDR_DSTIP_OFFSET-6)) = ((ip_check&0xff00)>>8);
			(*(unsigned char *)(skb_query->data+IP_HDR_DSTIP_OFFSET-5)) = (ip_check&0xff);

			timeout_arg.old_skb = skb_backup;
			timer_running=1;
			leave_query_count ++;
			// Kaohj -- send IGMP Query for fast leave
			// send out the packet
			spin_lock(&cp->lock);
			eth_net_dev->hard_start_xmit(skb_query, eth_net_dev);
			spin_unlock(&cp->lock);
			// Schedule timeout to check
			seq_timer.data=(unsigned long)&timeout_arg;
			seq_timer.expires=jiffies+timeout_arg.expire*HZ;
			seq_timer.function=snp_query_timeout;
			add_timer(&seq_timer);
		}
	return 1;
}
#endif // of IGMP_FAST_LEAVE

/*
 *	IGMP Query is going to be sent, reset status and schedule a timeout
 *	to check IGMP Report response for each LAN ports.
 *	Return value --
 *	0: pkt is IGMP query
 *      1: not IGMP query
 */
int handle_IGMP_query(struct sk_buff *skb)
{
	#define IP_TYPE_OFFSET			0x0c
	#define IP_TYPE				0x0800
	#define IP_HDLEN_OFFSET			0x0e
	#define IGMP_PROTOCOL_OFFSET		0x17
	#define IGMP_PROTOCOL			0x02
	#define IGMP_TYPE_QUERY			0x11
	#define MULTICAST_ADDR_REPORT_OFFSET	0x04
	int igmp_offset;
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	int ret;
	char mrt;
	struct sk_buff *skb_backup;
	struct IGMP_SNOOPING *igmp_group;
	
	ret = 1;
	igmp_offset=((*(unsigned char *)(skb->data+IP_HDLEN_OFFSET))&0x0f)*4+14;
	if((READHWD(skb->data+IP_TYPE_OFFSET))==IP_TYPE && 
		((*(unsigned char *)(skb->data+IGMP_PROTOCOL_OFFSET))==IGMP_PROTOCOL) && (*(unsigned char *)(skb->data+igmp_offset))==IGMP_TYPE_QUERY){
		if( (*(unsigned char *)(skb->data+igmp_offset+MULTICAST_ADDR_REPORT_OFFSET))==0){
			//general query
			ret = 0;
			if(timer_running==0){
				// reset test member before Query
				#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
				for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
					igmp_group = &snooping_table[i];
				#else // IGMP_CPU_TAG
				for(igmp_group = snooping_pool; igmp_group; igmp_group = igmp_group->next) {
				#endif
					if(igmp_group->multicast_addr!=0xc0000000){
						igmp_group->tmp_member_port=0;
						igmp_group->tmp_member_wlan=0;
					}
				}
				
				// backup skb for IGMP fast leave
				if ((skb_backup = dev_alloc_skb(2048)) == NULL) {
					printk("\n%s alloc skb_backup failed!\n", __FUNCTION__);
					return ret;
				}
				memcpy(skb_put(skb_backup, skb->len),skb->data,skb->len);
				//get max. response time
				mrt = *(unsigned char *)(skb->data+igmp_offset+1);
				if(mrt/10==0){
					timeout_arg.expire=1;
				}
				else
					timeout_arg.expire=mrt/10;
				timeout_arg.old_skb = skb_backup;
				timer_running=1;
				// Schedule timeout
				seq_timer.data=(unsigned long)&timeout_arg;
				seq_timer.expires=jiffies+timeout_arg.expire*HZ;
				seq_timer.function=snp_query_timeout;
				add_timer(&seq_timer);
			}
		}
	}
	return ret;
}

int check_IGMP_report(struct sk_buff *skb)
{
	#define IP_TYPE_OFFSET			0x0c
	#define IP_TYPE				0x0800
	#define IP_HDLEN_OFFSET			0x0e
	#define IGMP_PROTOCOL_OFFSET		0x17
	#define IGMP_PROTOCOL			0x02
	#define IGMP_TYPE_REPORT_JOIN		0x16
	#define IGMPV3_TYPE_REPORT_JOIN		0x22
	#define IGMP_TYPE_REPORT_LEAVE		0x17
	#define MULTICAST_ADDR_REPORT_OFFSET	0x04
	
	int igmp_offset;
	unsigned short tmp_ip_type;
	
	if((skb->data[0]!=0x01) || (skb->data[1]!=0x0) || ((skb->data[2]&0xfe)!=0x5e)){
		//bypass non-multicast pkt
		return 0;
	}
	
	tmp_ip_type=READHWD(skb->data+IP_TYPE_OFFSET);
	igmp_offset=((*(unsigned char *)(skb->data+IP_HDLEN_OFFSET))&0x0f)*4+14;
	if((tmp_ip_type==IP_TYPE) && 
		((*(unsigned char *)(skb->data+IGMP_PROTOCOL_OFFSET))==IGMP_PROTOCOL) && (*(unsigned char *)(skb->data+igmp_offset))==IGMP_TYPE_REPORT_JOIN) {
		return 1;
	}
	return 0;
}

/*
 * Return value:
 * 0: Unknown multicast address
 * 1: Learned multicast address
 * 2: Reserved multicast address
 * 3: not multicast address
 * vid: vlan ID or CPU tag destination port-map
 */
int IGMP_snoop_tx(struct sk_buff *skb, unsigned int *vid)
{
#define IP_HDR_DES_OFFSET		0x1e
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	unsigned int multicast_addr;
	#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
	struct IGMP_SNOOPING *igmp_group;
	#endif
	struct iphdr *iph;
	
	//check if incoming frame is IP multicast packet: 01005e of leading 23 bits
	if((skb->data[0]!=0x01) || (skb->data[1]!=0x0) || ((skb->data[2]&0xfe)!=0x5e)){
		//bypass non-multicast pkt
		return 3;
	}	
	
	//with vlan tag general query
	if(skb->data[12]==0x81 && skb->data[13]==0x00){
		// Mason Yu.	
		iph = (struct iphdr *) (skb->data + ETH_HLEN+4);		
	}               
	else {
		// Mason Yu.	
		iph = (struct iphdr *) (skb->data + ETH_HLEN);		
	}
	
	// This is not a IPv4 Packet. Bypass.
	if ( iph->version != 4)
			return 3;
	
	// This skb is local-output, Bypass it.
	// The headroom size is not enough to put VLAN tag for local-out packet.
	if(!skb->from_dev)
	{
		if (debug_igmp)
				printk("%s: This is a Local-out packet. Bypass it\n", __FUNCTION__);
		return 3;
	}
	multicast_addr=READWD((unsigned char *)&iph->daddr);
	
	//insert VLANID for multicast packets
	//printk("%x\n",*(unsigned int *)(skb->data+IP_HDR_DES_OFFSET));
	
	if(!((multicast_addr & 0xffffff00) == 0xe0000000)){ // not reserved multicast group
		//printk("multi-cast traffic!  %x\n",*(unsigned int *)(skb->data+IP_HDR_DES_OFFSET));
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
			if(multicast_addr==snooping_table[i].multicast_addr){
				// set vlan id
				*vid = snooping_table[i].VLANID & 0x0fff;
		                return 1;
			}
		}
		#else // IGMP_CPU_TAG		
		for(igmp_group = snooping_pool; igmp_group; igmp_group = igmp_group->next) {
			if(multicast_addr==igmp_group->multicast_addr) {
			
/*August 20110629 Start*/
#ifdef CONFIG_NEW_PORTMAPPING				
				if(skb->fgroup & 0x0f)
				{
					unsigned int bit_no;
					unsigned int tmp_vid;

					tmp_vid = 0;

					for(bit_no = 0; bit_no < SW_PORT_NUM; ++bit_no)
					{
						if(skb->fgroup & (1 << bit_no))
							tmp_vid |= 1 << (virt2phy[bit_no]);
					}
					*vid = tmp_vid & igmp_group->member_port;
					return 1;
				}
				else
					return 0;

#endif
/*August 20110629 End*/

				*vid = igmp_group->member_port;
				return 1;
			}
		}
		#endif
		return 0;
	}	
	return 2;
}

#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
static void IGMP_Snooping_config(void){
	IGMP_SNP_registered = 1;
	rtl8305_info.vlan_tag_aware = 1;
	rtl8305sc_setAsicVlanTagAware(1);	//enable tag aware
	rtl8305sc_setAsicBroadcastInputDrop(1);	//Broadcast output drop
	rtl8305sc_setAsicVlanEnable(1);	//VLAN enable
	
        set_8305(&rtl8305_info);
	RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled

#ifdef CONFIG_RE8305
	//VLAN F member = Port 0 and Port 4 (MII), VID of VLAN F = 005
	rtl8305sc_setAsicVlan(5, 5, 0x11);	
	//VLAN G member = Port 1 and Port 4 (MII), VID of VLAN F = 006
	rtl8305sc_setAsicVlan(6, 6, 0x12);	
	//VLAN H member = Port 2 and Port 4 (MII), VID of VLAN F = 007
	rtl8305sc_setAsicVlan(7, 7, 0x14);	
	//VLAN I member = Port 3 and Port 4 (MII), VID of VLAN F = 008
	rtl8305sc_setAsicVlan(8, 8, 0x18);
#endif
#ifdef CONFIG_RE8306
	// Kaohj -- originally for sequential send; use 8306 IGMP snooping and CPU port function instead.
	#if 0
	//VLAN F member = Port 0 and Port 4 (MII), VID of VLAN F = 005
	rtl8305sc_setAsicVlan(5, 5, 0x21);	
	//VLAN G member = Port 1 and Port 4 (MII), VID of VLAN F = 006
	rtl8305sc_setAsicVlan(6, 6, 0x22);	
	//VLAN H member = Port 2 and Port 4 (MII), VID of VLAN F = 007
	rtl8305sc_setAsicVlan(7, 7, 0x24);	
	//VLAN I member = Port 3 and Port 4 (MII), VID of VLAN F = 008
	rtl8305sc_setAsicVlan(8, 8, 0x28);
	#endif
#endif

}
#endif

#ifdef CONFIG_ETHWAN
void setIGMPSnooping(int enable)
{
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	unsigned short regValue;
	#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
	struct IGMP_SNOOPING *igmp_group;
	struct IGMP_SNOOPING *next_group;
	#endif
	if (enable) {
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		if (!IGMP_SNP_registered) {
			IGMP_SNP_registered = 1;
			rtl8305_info.vlan_tag_aware = 1;
		}
		rtl8305_info.phy[virt2phy[4]].egtag=TAG_REPLACE;
		for (i=0; i<SW_PORT_NUM; i++)
			rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
		#else // IGMP_CPU_TAG
		#endif
		enable_IGMP_SNP=1;
	}
	else {
		enable_IGMP_SNP=0;
		// flush snooping table
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
			remove_IGMP_Group(&snooping_table[i]);
		}
		#else // IGMP_CPU_TAG
		igmp_group = snooping_pool;
		while (igmp_group) {
			next_group = igmp_group->next;
			remove_IGMP_Group(igmp_group);
			igmp_group = next_group;
		}
		#endif
		REST_IGMP_SNOOPING_GROUP=MAX_IGMP_SNOOPING_GROUP;
	}
}
#else
void setIGMPSnooping(int enable)
{
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	unsigned short regValue;
	#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
	struct IGMP_SNOOPING *igmp_group;
	struct IGMP_SNOOPING *next_group;
	#endif
	if (enable) {
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		if (!IGMP_SNP_registered) {
			IGMP_Snooping_config();
		}
		rtl8305_info.phy[virt2phy[4]].egtag=TAG_REPLACE;
		for (i=0; i<SW_PORT_NUM; i++)
			rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
		set_8305(&rtl8305_info);
		#else // IGMP_CPU_TAG
		// remove cpu tag
		select_page(3);
		miiar_read(2, 21, &regValue);
		miiar_write(2, 21, regValue | 0x0800);
		select_page(0);
		// enable cputag aware
		miiar_read(4, 21, &regValue);
		miiar_write(4, 21, regValue | 0x0080);
		#endif
		// Enable CPU port, IGMP snooping function.
		select_page(3);
		miiar_read(2, 21, &regValue);
		miiar_write(2, 21, (regValue & 0x7FFF) | 0x4000);
		select_page(0);
		// Set CPU port
		miiar_read(4, 21, &regValue);
		miiar_write(4, 21, (regValue & 0xFFF8) | CONFIG_CPU_PORT);
		enable_IGMP_SNP=1;
	}
	else {
		enable_IGMP_SNP=0;
		// flush snooping table
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
			remove_IGMP_Group(&snooping_table[i]);
		}
		#else // IGMP_CPU_TAG
		igmp_group = snooping_pool;
		while (igmp_group) {
			next_group = igmp_group->next;
			remove_IGMP_Group(igmp_group);
			igmp_group = next_group;
		}
		#endif
		REST_IGMP_SNOOPING_GROUP=MAX_IGMP_SNOOPING_GROUP;
		// Disable IGMP snooping function
		select_page(3);
		miiar_read(2, 21, &regValue);
		miiar_write(2, 21, regValue & 0xBFFF);
		select_page(0);
	}
}
#endif

/*
 * init IGMP Snooping
 */
void initIGMPSnooping(void)
{
	int i;
	struct IGMP_SNOOPING *igmp_group;
	
	init_timer(&seq_timer);
#ifdef CONFIG_IPV6
	init_timer(&seq_timer_mld);
#endif
	i=0;
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
		igmp_group = &snooping_table[i];
	#else // IGMP_CPU_TAG
	for(igmp_group = snooping_pool; igmp_group; igmp_group = igmp_group->next) {
	#endif
		//snooping_table[i].VLAN_index=-1;
		igmp_group->multicast_addr=0xc0000000;
		igmp_group->member_port=0x0;
		igmp_group->member_wlan=0;
		igmp_group->tmp_member_port=0x0;
		igmp_group->tmp_member_wlan=0;
		igmp_group->VLANID=0x100+i;
		igmp_group->timer_running=0;
	}
}

void dump_igmp_info(void)
{
	int i;
	struct IGMP_SNOOPING *igmp_group;
	
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		printk("IGMP_TAG: VLAN_TAG\t");
	#else // IGMP_CPU_TAG
		printk("IGMP_TAG: CPU_TAG\t");
	#endif
	#if (VPORT_USE==VPORT_VLAN_TAG)
		printk("VPORT_TAG: VLAN_TAG\n");
	#else // VPORT_CPU_TAG
		printk("VPORT_TAG: CPU_TAG\n");
	#endif
	printk("IGMP Snooping group table: %d Max., %d active\n\n", MAX_IGMP_SNOOPING_GROUP, MAX_IGMP_SNOOPING_GROUP-REST_IGMP_SNOOPING_GROUP);
	i=-1;
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
		igmp_group = &snooping_table[i];
	#else // IGMP_CPU_TAG
	for(igmp_group = snooping_pool; igmp_group; igmp_group = igmp_group->next) {
		i++;
	#endif
		printk("Group[%d]=%u.%u.%u.%u, mbr=0x%x\n", i, NIPQUAD(igmp_group->multicast_addr), igmp_group->member_port);
	}
	printk("\n");
}

/*
 * : A Linux Ethernet MLD driver for Realtek 867x. Start
 */
 // Mason Yu. MLD snooping
void printkIPv6( unsigned char * multicast_addr_v6) {
 printk("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
			multicast_addr_v6[0],multicast_addr_v6[1],multicast_addr_v6[2],multicast_addr_v6[3],
			multicast_addr_v6[4],multicast_addr_v6[5],multicast_addr_v6[6],multicast_addr_v6[7],
			multicast_addr_v6[8],multicast_addr_v6[9],multicast_addr_v6[10],multicast_addr_v6[11],
			multicast_addr_v6[12],multicast_addr_v6[13],multicast_addr_v6[14],multicast_addr_v6[15]);
			
}

#ifdef CONFIG_IPV6
#include <linux/ipv6.h>
#include <linux/in6.h>
#define IN6_IS_ADDR_MULTICAST(a)        ((a)->s6_addr[0] == 0xff)
#include <linux/etherdevice.h> /* eth_type_trans */

static void remove_MLD_Group(struct IGMP_SNOOPING *mldGroup)
{	
	struct IGMP_SNOOPING *mld_group,*mld_group_prev;	
	
	for(mld_group_prev=mld_group=snooping_pool; mld_group; mld_group_prev=mld_group,mld_group = mld_group->next)
	{		
		if (memcmp(mld_group->multicast_addr_v6, mldGroup->multicast_addr_v6, sizeof(struct in6_addr)) == 0 )
		{
			if(mld_group_prev == mld_group)/*remove the head*/
				snooping_pool = mld_group->next;
			else
				mld_group_prev->next= mld_group->next;
			kfree(mldGroup);
			break;
			
		}
	}	
}

void check_MLD_snoop_rx(struct sk_buff *skb, int tag)
{
	struct ipv6hdr *ipv6h;
	char* protoType;
	unsigned char port;
	struct IGMP_SNOOPING *mld_group;
	int in_snp_table=0;
	#if (VPORT_USE==VPORT_VLAN_TAG)
	int i;
	#endif
	
	// We do not use the skb_reset_network_header(), we	use offset.
	//skb_reset_network_header(skb);
	//ipv6h = ipv6_hdr(skb);
	
	/* check IPv6 header information */
	// This is a IP packet, so we offset 0.
	ipv6h = (struct ipv6hdr *) (skb->data + 0);	
	if(ipv6h->version != 6){	
		//printk("%s: ipv6h->version != 6\n", __FUNCTION__);
		return;
	}

	/*Next header: IPv6 hop-by-hop option (0x00)*/
	if(ipv6h->nexthdr == 0)	{
		protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ipv6hdr) );	
	}else{
		//printk("s: ipv6h->nexthdr != 0\n", __FUNCTION__);
		return;
	}	
	
	/* This is not icmpv6 packet. Drop it */
	if(protoType[0] != 0x3a){
		return;
	}
	
	//retrieve the input port	
	#if (VPORT_USE==VPORT_CPU_TAG)
	if(tag == wlan_igmp_tag) {
		port =5;
	}
	else 
		port=tag;
	#else // VPORT_VLAN_TAG
	if(tag == wlan_igmp_tag) {
		port =5;
	}
	else {
		for(i=0;i<SW_PORT_NUM;i++)
		if (tag == rtl8305_info.vlan[rtl8305_info.phy[virt2phy[i]].vlanIndex].vid)
			break;
		port = virt2phy[i];
	} 
	#endif 	
	
	if(protoType[0] == 0x3a){		
		//printk("recv icmpv6 packet\n");
		struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);
		unsigned char* icmpv6_McastAddr ;
		unsigned char McastAddr[16];
	
		if(icmpv6h->icmp6_type != ICMPV6_MGM_REPORT &&
		   icmpv6h->icmp6_type != ICMPV6_MLD2_REPORT &&
		   icmpv6h->icmp6_type != ICMPV6_MGM_REDUCTION){
			if (debug_igmp) {
				printk("%s: Type=0x%x (unknow type)\n",__FUNCTION__, icmpv6h->icmp6_type);
			}		
			return;
		}
		
		// Join Group
		if(icmpv6h->icmp6_type == ICMPV6_MGM_REPORT || icmpv6h->icmp6_type ==ICMPV6_MLD2_REPORT){
			if(icmpv6h->icmp6_type == ICMPV6_MGM_REPORT)
				icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8);
			if(icmpv6h->icmp6_type == ICMPV6_MLD2_REPORT)
				icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 + 4);
				
			memcpy(McastAddr, icmpv6_McastAddr, sizeof(struct in6_addr));
			if (debug_igmp) {					
				printk("%s: Type=0x%x (Multicast listener report)\n",__FUNCTION__, icmpv6h->icmp6_type);
			}		
			
			in_snp_table=0;
			#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
			for (i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
				mld_group = &snooping_table[i];
			#else // IGMP_CPU_TAG
			for (mld_group = snooping_pool; mld_group; mld_group = mld_group->next) {
			#endif				
				if ( memcmp(mld_group->multicast_addr_v6, McastAddr, sizeof(struct in6_addr)) == 0 ) {
					in_snp_table=1;
					break;
				}
			}
			
			if(in_snp_table) { // Entry exists, update it.
				if(port!=5) { //port 5 for wlan
					mld_group->tmp_member_port |= (1<<port);
					if ((mld_group->member_port & (1<<port))==0) {
						// New member
						mld_group->member_port |= (1<<port);
						mld_group->tmp_member_port |= 1<<port;
						#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
						// New member, update VLAN associated to this group
						rtl8305sc_setAsicVlan(i+IGMP_SNOOPING_PVID_START,i+0x100,mld_group->member_port);
						#endif
					}
				}
				else {
					mld_group->tmp_member_wlan=1;
					mld_group->member_wlan=1;
				}			
			}
			else { // find empty entry in IGMP_snooping table and create a vlan
				if(REST_IGMP_SNOOPING_GROUP>0) {
					#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
					for (i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
						//if(snooping_table[i].multicast_addr==0xc0000000)
						if ( memcmp(snooping_table[i].multicast_addr_v6, 0x0, sizeof(struct in6_addr)) == 0 )
							break;
					}					
					if (i == MAX_IGMP_SNOOPING_GROUP)
						return;
					mld_group = &snooping_table[i];
					#else // IGMP_CPU_TAG
					if ((mld_group = (struct IGMP_SNOOPING *) kzalloc(sizeof(struct IGMP_SNOOPING), GFP_KERNEL|__GFP_NOFAIL))==NULL) {
						printk("%s alloc igmp group error",__FUNCTION__);
						return;
					}
					//memset(mld_group,0,sizeof(struct IGMP_SNOOPING));
					//mld_group->expires[port] = jiffies + igmpSnp_host_timeout*HZ/1000;
					//mld_group->VLANID=0x100+i;
					#endif
					REST_IGMP_SNOOPING_GROUP--;
					//mld_group->multicast_addr=multicast_addr;
					memcpy(mld_group->multicast_addr_v6, icmpv6_McastAddr, sizeof(struct in6_addr));
					
					// Init the IPv4 multicast_addr
					mld_group->multicast_addr =0xc0000000;
					
					if (port!=5) {
						mld_group->member_port |= 1<<port;
						mld_group->tmp_member_port |= 1<<port;
					}
					else {
						mld_group->tmp_member_wlan=1;
						mld_group->member_wlan=1;
					}
					#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
					if (debug_igmp) {
						printk("create VLAN:");
						printk("vlanindex:%x   vid:%x   member_port:%x  port:%x\n\n",i+9,i+0x100,mld_group->member_port,port);
					}
					rtl8305sc_setAsicVlan(i+IGMP_SNOOPING_PVID_START,i+0x100,mld_group->member_port);
					#else // IGMP_CPU_TAG
					// attach to the head
					mld_group->next = snooping_pool;
					snooping_pool = mld_group;
					#endif
				}
			}					
		}else if(icmpv6h->icmp6_type == ICMPV6_MGM_REDUCTION){
			// Leave group
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 );
			memcpy(McastAddr, icmpv6_McastAddr, sizeof(struct in6_addr));
			if (debug_igmp) {					
				printk("%s: Type=0x%x (Multicast listener done ) \n",__FUNCTION__, icmpv6h->icmp6_type);
			}			
			
			#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
			for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
				mld_group = &snooping_table[i];
			#else // IGMP_CPU_TAG
			for (mld_group = snooping_pool; mld_group; mld_group = mld_group->next) {
			#endif				
				if (memcmp(mld_group->multicast_addr_v6, McastAddr, sizeof(struct in6_addr)) == 0 ) {					
					if (port!=5)
						mld_group->member_port &= ~(1<<port);
					else
						mld_group->member_wlan=0;
					if (mld_group->member_port==0 && mld_group->member_wlan==0) {
						if (debug_igmp) {
							printk("%s: Remove IGMP group:", __FUNCTION__);
							printkIPv6(mld_group->multicast_addr_v6);
						}
						remove_MLD_Group(mld_group);
						REST_IGMP_SNOOPING_GROUP++;
					}
					else {
						if (debug_igmp)
							printk("%s: Remove member port %x\n",__FUNCTION__, port);
						#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
						rtl8305sc_setAsicVlan(i+IGMP_SNOOPING_PVID_START,i+0x100,mld_group->member_port);
						#endif
					}					
					break;
				}
			}
					
		}				
	}		
	
}

int MLD_snoop_tx(struct sk_buff *skb, unsigned int *vid)
{
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	unsigned char multicast_addr[16];
	#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
	struct IGMP_SNOOPING *mld_group;
	#endif
	
	struct ipv6hdr *ipv6h;
	char* protoType;
	
	// We can not remove mac header, we use offset.
	//skb->protocol = eth_type_trans (skb, skb->dev);
	//skb_reset_network_header(skb);
	//ipv6h = ipv6_hdr(skb);
	
	ipv6h = (struct ipv6hdr *) (skb->data + ETH_HLEN);
	
	/* check IPv6 header information */		
	if(ipv6h->version != 6){	
		//printk("%s: ipv6h->version != 6\n", __FUNCTION__);
		return 3;
	}	
	
	/*Next header: IPv6 hop-by-hop option (0x00)*/
	// Mason Yu.
	#if 0
	if(ipv6h->nexthdr == 0)	{
		protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ipv6hdr) );		
	
		/* If it is icmpv6 packet. Drop it */	
		if(protoType[0] == 0x3a){
			struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);			
			if (debug_igmp)
				printk("%s: This is a icmpv6 packet. Type=%x. Bypass it\n", __FUNCTION__, icmpv6h->icmp6_type);			
			return 3;
		}		
	}
	#endif
	
	// This skb is local-output, Bypass it.
	// The headroom size is not enough to put VLAN tag for local-out packet.
	if(!skb->from_dev)
	{
		if (debug_igmp)
				printk("%s: This is a Local-out packet. Bypass it\n", __FUNCTION__);
		return 3;
	}
	
	//check if incoming frame is IP multicast packet:
	if (!IN6_IS_ADDR_MULTICAST(&ipv6h->daddr))
	{
		/* printk("%s: Address is not multicast!\n", __FUNCTION__); */
		return 3;
	}	
	memcpy(multicast_addr, &ipv6h->daddr, sizeof(struct in6_addr));	
		
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){			
		if ( memcmp(multicast_addr, snooping_table[i].multicast_addr_v6, sizeof(struct in6_addr) == 0) {
			// set vlan id
			*vid = snooping_table[i].VLANID & 0x0fff;
		               return 1;
		}
	}
	#else // IGMP_CPU_TAG		
	for(mld_group = snooping_pool; mld_group; mld_group = mld_group->next) {			
		if(memcmp(multicast_addr, mld_group->multicast_addr_v6, sizeof(struct in6_addr)) == 0) {			
/*August 20110629 Start*/
#ifdef CONFIG_NEW_PORTMAPPING				
				if(skb->fgroup & 0x0f)
				{
					unsigned int bit_no;
					unsigned int tmp_vid;

					tmp_vid = 0;

					for(bit_no = 0; bit_no < SW_PORT_NUM; ++bit_no)
					{
						if(skb->fgroup & (1 << bit_no))
							tmp_vid |= 1 << (virt2phy[bit_no]);
					}
					*vid = tmp_vid & mld_group->member_port;
					return 1;
				}
				else
					return 0;

#endif
/*August 20110629 End*/	
			*vid = mld_group->member_port;			
			return 1;
		}
	}
	#endif
	return 0;	
}

/*
 *	Timeout function for MLD Query of MLD snooping
 *	Update multicast group status.
 */
static void mld_snp_query_timeout(unsigned long arg)
{
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int idx;
	#endif	
	struct IGMP_SNOOPING *mld_group;	
	#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
	struct IGMP_SNOOPING *next_group;
	#endif
	
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	for (idx=0;idx<MAX_IGMP_SNOOPING_GROUP;idx++) {
		mld_group = &snooping_table[idx];
	#else // IGMP_CPU_TAG
	mld_group = snooping_pool;
	while (mld_group) {
		next_group = mld_group->next;
	//for (mld_group = snooping_pool; mld_group; mld_group = mld_group->next) {
	#endif
		if (mld_group->multicast_addr!=0xc0000000) {
			//printk("general query:update VLAN accroding to tmp_member_port\n");
			if (mld_group->tmp_member_port==0 && mld_group->tmp_member_wlan==0) {				
				if (debug_igmp)
					printk("%s: Group %u.%u.%u.%u timeout, delete it.\n", __FUNCTION__, NIPQUAD(mld_group->multicast_addr));
				remove_MLD_Group(mld_group);
				REST_IGMP_SNOOPING_GROUP++;
			}
			else{				
				#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
				if (debug_igmp)
					printk("%s: Update VLAN %x VID=%x\n",__FUNCTION__, idx+IGMP_SNOOPING_PVID_START,mld_group->VLANID);
				if(mld_group->member_port!=mld_group->tmp_member_port)
					rtl8305sc_setAsicVlan(idx+IGMP_SNOOPING_PVID_START,idx+0x100,mld_group->tmp_member_port);
				#endif
				mld_group->timer_running=0;
				mld_group->member_port=mld_group->tmp_member_port;
				mld_group->member_wlan=mld_group->tmp_member_wlan;
			}
		}
		#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
		mld_group = next_group;
		#endif
	}	
	timer_running_mld=0;
}

/*
 *	MLD Query is going to be sent, reset status and schedule a timeout
 *	to check MLD Report response for each LAN ports.
 *	Return value --
 *	0: pkt is MLD query
 *  1: not MLD query
 */
int handle_MLD_query(struct sk_buff *skb)
{	
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	int ret;		
	struct IGMP_SNOOPING *mld_group;
	
	struct ipv6hdr *ipv6h;
	char* protoType;	
	struct icmp6hdr* icmpv6h;
	
	ipv6h = (struct ipv6hdr *) (skb->data + ETH_HLEN);
	ret = 1;
	/* check IPv6 header information */		
	if(ipv6h->version != 6){	
		//printk("handle_MLD_query:ipv6h->version != 6\n");
		return 1;
	}
	
	/*Next header: IPv6 hop-by-hop option (0x00)*/
	if(ipv6h->nexthdr == 0)	{
		protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ipv6hdr) );	
	}else{
		//printk("handle_MLD_query:ipv6h->nexthdr != 0\n");
		return 1;
	}	
	
	/* This is not icmpv6 packet. Drop it */
	if(protoType[0] != 0x3a){
		return 1;
	}
	
	icmpv6h = (struct icmp6hdr*)(protoType + 8);	
	// Query
	if(icmpv6h->icmp6_type == ICMPV6_MGM_QUERY){	
			//general query
			if (debug_igmp)
				printk("%s: Type=0x%x (Multicast Listener Query) \n",__FUNCTION__, icmpv6h->icmp6_type);
			ret = 0;
			if(timer_running_mld==0){
				// reset test member before Query
				#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
				for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
					mld_group = &snooping_table[i];
				#else // IGMP_CPU_TAG
				for(mld_group = snooping_pool; mld_group; mld_group = mld_group->next) {
				#endif
					if(mld_group->multicast_addr!=0xc0000000){
						mld_group->tmp_member_port=0;
						mld_group->tmp_member_wlan=0;
					}
				}
								
				//get max. response time				
				// in units of milliseconds
				if(icmpv6h->icmp6_maxdelay/1000==0){
					timeout_arg_mld.expire=1;
				}
				else
					timeout_arg_mld.expire=icmpv6h->icmp6_maxdelay/1000;				
				timer_running_mld=1;								
				// Schedule timeout				
				seq_timer_mld.data=(unsigned long)&timeout_arg_mld;
				seq_timer_mld.expires=jiffies+timeout_arg_mld.expire*HZ;
				seq_timer_mld.function=mld_snp_query_timeout;
				add_timer(&seq_timer_mld);				
			}		
	}
	return ret;
}

int check_MLD_report(struct sk_buff *skb)
{	
	struct ipv6hdr *ipv6h;
	char* protoType;
	struct icmp6hdr* icmpv6h;
	
	ipv6h = (struct ipv6hdr *) (skb->data + ETH_HLEN);
	
	/* check IPv6 header information */		
	if(ipv6h->version != 6){			
		return 0;
	}
	
	/*Next header: IPv6 hop-by-hop option (0x00)*/
	if(ipv6h->nexthdr == 0)	{
		protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ipv6hdr) );	
	}else{		
		return 0;
	}	
	
	/* This is not icmpv6 packet. Drop it */
	if(protoType[0] != 0x3a){
		return 0;
	}
	
	icmpv6h = (struct icmp6hdr*)(protoType + 8);	
	// MLD Report or MLDv2 Report
	if(icmpv6h->icmp6_type == ICMPV6_MGM_REPORT || icmpv6h->icmp6_type ==ICMPV6_MLD2_REPORT){
		if (debug_igmp)
			printk("check_MLD_report: This is a MLDv1/v2 report. Type=%x. Drop it.\n", icmpv6h->icmp6_type);
		return 1;
	}	
	return 0;
}

#ifdef CONFIG_ETHWAN
void setMLDSnooping(int enable)
{
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	unsigned short regValue;
	#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
	struct IGMP_SNOOPING *mld_group;
	struct IGMP_SNOOPING *next_group;
	#endif
	if (enable) {
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		if (!MLD_SNP_registered) {
			MLD_SNP_registered = 1;
			rtl8305_info.vlan_tag_aware = 1;
		}
		rtl8305_info.phy[virt2phy[4]].egtag=TAG_REPLACE;
		for (i=0; i<SW_PORT_NUM; i++)
			rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
		#else // IGMP_CPU_TAG
		#endif
		enable_MLD_SNP=1;
	}
	else {
		enable_MLD_SNP=0;
		// flush snooping table
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
			remove_MLD_Group(&snooping_table[i]);
		}
		#else // IGMP_CPU_TAG
		mld_group = snooping_pool;
		while (mld_group) {
			next_group = mld_group->next;
			remove_MLD_Group(mld_group);
			mld_group = next_group;
		}
		#endif
		REST_IGMP_SNOOPING_GROUP=MAX_IGMP_SNOOPING_GROUP;
	}
}
#else
void setMLDSnooping(int enable)
{
	#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
	int i;
	#endif
	unsigned short regValue;
	#if (IGMP_SNOOPING_USE==IGMP_CPU_TAG)
	struct IGMP_SNOOPING *mld_group;
	#endif
	if (enable) {
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		if (!MLD_SNP_registered) {
			IGMP_Snooping_config();
		}
		rtl8305_info.phy[virt2phy[4]].egtag=TAG_REPLACE;
		for (i=0; i<SW_PORT_NUM; i++)
			rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
		set_8305(&rtl8305_info);
		#else // IGMP_CPU_TAG
		// remove cpu tag
		select_page(3);
		miiar_read(2, 21, &regValue);
		miiar_write(2, 21, regValue | 0x0800);
		select_page(0);
		// enable cputag aware
		miiar_read(4, 21, &regValue);
		miiar_write(4, 21, regValue | 0x0080);
		#endif
		// Enable CPU port, IGMP snooping function.
		select_page(3);
		miiar_read(2, 21, &regValue);
		miiar_write(2, 21, (regValue & 0x7FFF) | 0x4000);
		select_page(0);
		// Set CPU port
		miiar_read(4, 21, &regValue);
		miiar_write(4, 21, (regValue & 0xFFF8) | CONFIG_CPU_PORT);
		enable_MLD_SNP=1;
	}
	else {
		enable_MLD_SNP=0;
		// flush snooping table
		#if (IGMP_SNOOPING_USE==IGMP_VLAN_TAG)
		for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++) {
			remove_MLD_Group(&snooping_table[i]);
		}
		#else // IGMP_CPU_TAG
		for(mld_group = snooping_pool; mld_group; mld_group = mld_group->next)
			remove_MLD_Group(mld_group);
		#endif
		REST_IGMP_SNOOPING_GROUP=MAX_IGMP_SNOOPING_GROUP;
		// Disable IGMP snooping function
		select_page(3);
		miiar_read(2, 21, &regValue);
		miiar_write(2, 21, regValue & 0xBFFF);
		select_page(0);
	}
}
#endif //#ifdef CONFIG_ETHWAN
#endif // #ifdef CONFIG_IPV6
#endif // of CONFIG_EXT_SWITCH


