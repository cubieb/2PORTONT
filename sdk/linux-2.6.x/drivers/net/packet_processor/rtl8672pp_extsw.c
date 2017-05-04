/*
 *	rtl8672pp_extsw.c
 */

#include <linux/atm.h>
#include <linux/atmdev.h>
#include "../re830x.h"
#include "rtl_types.h"
#include "icModel_ringController.h"

#define READWD(addr) ((unsigned char *)addr)[0]<<24 | ((unsigned char *)addr)[1]<<16 | ((unsigned char *)addr)[2]<<8 | ((unsigned char *)addr)[3]
struct sk_buff *pp_sar_send_vlan(int ch_no, struct sk_buff *skb);
int rtl8672_l2learning(unsigned char *smac,int intfidx,int vlanid);

int pp_mac_vtx_sw (struct sk_buff *skb, struct mac_vTx *vtx)
{
#ifdef CONFIG_EXT_SWITCH
	unsigned int vid;
	if (!re8670_xmit_sw(skb, &vid)) {
		dev_kfree_skb(skb);
		return 0;
	}
	else {
		if (vid) {
			// set vlan id
			vtx->vlan_vidh = ((vid & 0x00000f00)>>8);
			vtx->vlan_vidl = (vid & 0x000000ff);
			vtx->tagc=1;
		}
	}
#endif
	return 1;
}

void pp_check_igmp_snooping_rx(struct sk_buff *skb, int tag) {
#ifdef CONFIG_EXT_SWITCH
	if(enable_IGMP_SNP){
		check_IGMP_snoop_rx(skb, tag);
	}
#endif
}

int pp_mac_rx_sw(int idx, struct mac_vRx *macRx, struct sk_buff *newskb)
{
	int tag=0;
#ifdef CONFIG_EXT_SWITCH
	
	
#ifdef CONFIG_RE8306_USE_CPUTAG	
						if (*((unsigned short*)&newskb->data[12]) == 0x8899)
						{
							//   printk("skb match cpu tag\n");
							process_cpu_tag(newskb, &tag);
		  
						}
#endif
#ifndef CONFIG_RE8306_USE_CPUTAG
						if (macRx->ava) {
							tag = (macRx->vlan_vidh<<8); // VIDH
							tag |= (macRx->vlan_vidl); // VIDL
		
#ifdef CONFIG_RE8305
							if (!enable_virtual_port)
#endif	
	        					{ // if virtual-port is enabled, then the vlan tag has been replaced by switch(830x)
								// marked for 802.1p based priority
								newskb->nfmark = (macRx->vlan_prio) + 1;
							}
							//if(!enable_IGMP_SNP)
							process_eth_vlan(newskb,  &tag);
						}
#endif

						//for packet processor porting mapping
						if(enable_port_mapping)
						{
							int source_port,member_port;
							//get member
							for(source_port=0;source_port<SW_PORT_NUM;source_port++)
								if (tag == rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].vid)
									break;
							//add rules for each member
							if (source_port < SW_PORT_NUM) 
								for(member_port=0;member_port<SW_PORT_NUM;member_port++){
									if ((1<<member_port) & rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].member){
										//printk("%x   %x\n",(1<<source_port) , rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].member);
										rtl8672_l2learning(&newskb->data[6], idx+SAR_INTFS,rtl8305_info.vlan[rtl8305_info.phy[virt2phy[member_port]].vlanIndex].vid); 
									}
								}	
						}
						
						// Port-Mapping: vlan member is set at sar driver on receving packets
						//skb->vlan_member=0;
						
#endif // of CONFIG_EXT_SWITCH
	return tag;
}

int pp_mac_int_sw (struct mac_vRx *macRx, struct sk_buff *newskb)
{
	int tag=0;
#ifdef CONFIG_EXT_SWITCH
	
#ifdef CONFIG_RE8306_USE_CPUTAG	
						if (*((unsigned short*)&newskb->data[12]) == 0x8899)
						{
							//   printk("skb match cpu tag\n");
							process_cpu_tag(newskb, &tag);
		  
						}
#endif
#ifndef CONFIG_RE8306_USE_CPUTAG
						if (macRx->ava) {
							tag = (macRx->vlan_vidh<<8); // VIDH
							tag |= (macRx->vlan_vidl); // VIDL
		
#ifdef CONFIG_RE8305
							if (!enable_virtual_port)
#endif	
	        					{ // if virtual-port is enabled, then the vlan tag has been replaced by switch(830x)
								// marked for 802.1p based priority
								newskb->nfmark = (macRx->vlan_prio) + 1;
							}
							//if(!enable_IGMP_SNP)
							process_eth_vlan(newskb,  &tag);
		
						}
#endif	
						// Port-Mapping: vlan member is set at sar driver on receving packets
						//skb->vlan_member=0;
#endif    
	return tag;
}

