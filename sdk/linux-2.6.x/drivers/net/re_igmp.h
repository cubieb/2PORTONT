/*	
 *	re_igmp.h
*/
#ifndef _RE_IGMP_H_
#define _RE_IGMP_H_

#define IGMP_CPU_TAG		0
#define IGMP_VLAN_TAG		1
#define IGMP_SNOOPING_USE IGMP_CPU_TAG
extern int debug_igmp;

int handle_IGMP_query(struct sk_buff *skb);
int check_IGMP_report(struct sk_buff *skb);
int IGMP_snoop_tx(struct sk_buff *skb, unsigned int *vid);
void setIGMPSnooping(int enable);
void initIGMPSnooping(void);

#endif /*_RE_IGMP_H_*/

// Mason Yu. MLD snooping
#ifdef CONFIG_IPV6
int handle_MLD_query(struct sk_buff *skb);
int check_MLD_report(struct sk_buff *skb);
void check_MLD_snoop_rx(struct sk_buff *skb, int tag);
int MLD_snoop_tx(struct sk_buff *skb, unsigned int *vid);
void setMLDSnooping(int enable);
extern int enable_MLD_SNP;
#endif
