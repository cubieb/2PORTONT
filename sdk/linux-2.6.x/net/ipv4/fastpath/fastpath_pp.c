/*
 *	fastpath_pp.c
 */

#include "fastpath_core.h"
#include "../../../drivers/net/packet_processor/rtl_types.h"
#include "../../../drivers/net/packet_processor/rtl8672_tblDrv.h"
#include "../../../drivers/net/packet_processor/icModel_ringController.h"

// Kaohj -- exception port list
struct exception_list {
	int proto;
	uint16 dst_port;
};
struct exception_list not_expected[5] = {
	{IPPROTO_TCP, 80}, {0, 0}
};

extern struct Arp_List_Entry *FastPath_Find_ARP_Entry(ipaddr_t ipaddr);
extern struct Route_List_Entry *rtl867x_lookupRoute(ipaddr_t ip);

static int isAllowed(int proto, uint16 org_dst_port)
{
	int i;

	i=0;
	while (not_expected[i].proto!=0) {
		if (not_expected[i].proto==proto && not_expected[i].dst_port==org_dst_port)
			return 0;
		i++;
	}
	return 1;
}
unsigned char* get_ppp_mac(struct net_device *dev);
struct net_device* get_ppp_dev(struct net_device *dev);
unsigned short get_ppp_sid(struct net_device *dev);
unsigned char get_pppoatm_ch(struct net_device *dev);
enum LR_RESULT
rtl867x_addNaptConnection(int isTCP, uint32 org_src_ip,uint16 org_src_port,uint32 org_dst_ip,uint16 org_dst_port,
										uint32 reply_src_ip,uint16 reply_src_port,uint32 reply_dst_ip,uint16 reply_dst_port,
										uint16 in_vlan_id,uint16 out_vlan_id, struct sk_buff* skb)
{
	rtl8672_tblAsicDrv_l4Param_t l4entry;
	struct Arp_List_Entry *ep;
	struct Route_List_Entry *rt;
	unsigned char isPPPoE=0;
	
	if (!isAllowed(isTCP, org_dst_port))
		return LR_FAILED;
	
	if( skb && skb->dev && !strncmp("ppp8",skb->dev->name,4) )
	{
		//printk( "it's 3g ppp8, skb->dev->name=%s,  ==> return LR_FAILED\n",  skb->dev->name );
		return LR_FAILED;
	}
	
	l4entry.Valid=1;
	l4entry.PriorityID=0;
	l4entry.Tos=0;
	l4entry.Age=0; // Software write to zero, hw write to one... if software check the value is zero, delete the entry

	l4entry.Proto=isTCP;
	
	l4entry.SessionID= 0;
	l4entry.TtlDe= 1;
	l4entry.PrecedRemr=0;
	l4entry.TosRemr=0;
	l4entry.Dot1PRemr=0;
	l4entry.OTagIf=0;
	l4entry.IPppoeIf=0;
	l4entry.OPppoeIf=0;
	l4entry.IVlanIDChk=0;
	
	l4entry.SrcIP=org_src_ip;
	l4entry.SrcPort=(org_src_port);
	
	l4entry.DstIP=org_dst_ip;
	l4entry.DstPort=(org_dst_port);
	
	l4entry.NewSrcIP=reply_dst_ip;
	l4entry.NewSrcPort=(reply_dst_port);
	
	l4entry.NewDstIP=reply_src_ip;				
	l4entry.NewDstPort=(reply_src_port);

	rt=rtl867x_lookupRoute(l4entry.DstIP);

	/*xl_yue:20091120 return LR_FAILED if rt==0*/
	if(!rt){
		//printk("no route!\n");
		return LR_FAILED;
	}
	/*xl_yue:20091120 end*/

	if(rt->gateway==0)
	{
		ep=FastPath_Find_ARP_Entry(l4entry.DstIP);
	}
	else
	{
		//printk("match default route, gateway ip=%x\n",rt->gateway);
		ep=FastPath_Find_ARP_Entry(rt->gateway);
	}
	if(ep!=NULL)
	{
		rtl8672_tblAsicDrv_l2Param_t l2;
		ether_t* mac=&ep->mac;
		l4entry.NHMAC[0]=mac->octet[0];
		l4entry.NHMAC[1]=mac->octet[1];
		l4entry.NHMAC[2]=mac->octet[2];
		l4entry.NHMAC[3]=mac->octet[3];
		l4entry.NHMAC[4]=mac->octet[4];
		l4entry.NHMAC[5]=mac->octet[5];
		l4entry.IVlanID=in_vlan_id;
		l4entry.OVlanID=out_vlan_id;


		if((l4entry.OVlanID!=0) && (l4entry.IVlanID!=0))
		{
			int result;
			rtl8672_L2EntryLookup(l4entry.NHMAC,out_vlan_id,&l2);
			l4entry.DstPortIdx=l2.Port;							
			//printk("set ip1=%x mac ok = %02x-%02x-%02x-%02x-%02x-%02x portidx=%d out-vlan=%d\n",l4entry.DstIP,l4entry.NHMAC[0],l4entry.NHMAC[1],l4entry.NHMAC[2],l4entry.NHMAC[3],l4entry.NHMAC[4],l4entry.NHMAC[5],l4entry.DstPortIdx,l4entry.OVlanID);
			result=rtl8672_addAsicL4Entry(&l4entry, 0);
			/*
			if(result==FAILED){
				printk("add l4 entry failed!\n");
			}
			else
				printk("SUCCESS!\n");
			*/
		}
		else
		{
			return LR_FAILED;
		}

	}
	else
	{
		struct net_device* dst_dev = (struct net_device *)__dev_get_by_name(rt->ifname);
		//printk("dst_dev->name:%s   rt->ifname:%s\n",dst_dev->name,rt->ifname);
		if(!strncmp("ppp",dst_dev->name,3)){
			l4entry.IVlanID=in_vlan_id;
			l4entry.OVlanID=out_vlan_id;
			if((l4entry.OVlanID!=0) && (l4entry.IVlanID!=0))
			{
				int result=0,dstPortIdx;
				dstPortIdx = index_dev2ch(get_ppp_dev(dst_dev));
				//printk("dst dev:%x dstPortIdx:%d\n",(uint32)get_ppp_dev(dst_dev),dstPortIdx);
				if(dstPortIdx!=-1){ //pppoe
					l4entry.DstPortIdx=dstPortIdx;
					if(l4entry.DstPortIdx == -1){
						//printk("error in addNaptConnection %d\n",__LINE__);
						return LR_FAILED;
					}
					isPPPoE=1;
					l4entry.IPppoeIf=0;
					l4entry.OPppoeIf=1;
					l4entry.SessionID = (uint32)get_ppp_sid(dst_dev);
					if(l4entry.SessionID==0 || get_ppp_mac(dst_dev)==NULL){
						//printk("dst dev find error in %s %d\n",__FUNCTION__,__LINE__);
						return LR_FAILED;
					}
					memcpy(l4entry.NHMAC, get_ppp_mac(dst_dev), 6);
					result=rtl8672_addAsicL4Entry(&l4entry, 0);
				}
				else{//pppoa
					l4entry.DstPortIdx=get_pppoatm_ch(dst_dev);
					if(l4entry.DstPortIdx == -1)
						return LR_FAILED;
					result=rtl8672_addAsicL4Entry(&l4entry, 0);
				}
			}
			else
			{
				return LR_FAILED;
			}
		}else{//rt1483
			l4entry.IVlanID=in_vlan_id;
			l4entry.OVlanID=out_vlan_id;
			if((l4entry.OVlanID!=0) && (l4entry.IVlanID!=0))
			{
				int result;
				l4entry.DstPortIdx=index_dev2ch(dst_dev); //for rt1483
				if(l4entry.DstPortIdx == -1){
					//printk("error in addNaptConnection %d\n",__LINE__);
					return LR_FAILED;
				}
				result=rtl8672_addAsicL4Entry(&l4entry, 0);
			}
			else
			{
				//printk("OVlanID==0!\n");
				return LR_FAILED;
			}
		}
	}


	ep=FastPath_Find_ARP_Entry(l4entry.SrcIP);
	if(ep!=NULL)
	{

		ether_t* mac=&ep->mac;
		
		l4entry.SrcIP=reply_src_ip;
		l4entry.SrcPort=(reply_src_port);
		
		l4entry.DstIP=reply_dst_ip;
		l4entry.DstPort=(reply_dst_port);
		
		l4entry.NewSrcIP=org_dst_ip;
		l4entry.NewSrcPort=(org_dst_port);
		
		l4entry.NewDstIP=org_src_ip;
		l4entry.NewDstPort=(org_src_port);
	
		l4entry.NHMAC[0]=mac->octet[0];
		l4entry.NHMAC[1]=mac->octet[1];
		l4entry.NHMAC[2]=mac->octet[2];
		l4entry.NHMAC[3]=mac->octet[3];
		l4entry.NHMAC[4]=mac->octet[4];
		l4entry.NHMAC[5]=mac->octet[5];
		l4entry.IVlanID=out_vlan_id;
		l4entry.OVlanID=in_vlan_id;					
		if(isPPPoE)
			l4entry.IPppoeIf=1;
		else
			l4entry.IPppoeIf=0;
		l4entry.OPppoeIf=0;

		if((l4entry.OVlanID!=0) && (l4entry.IVlanID!=0))
		{
			rtl8672_tblAsicDrv_l2Param_t l2;
			int result;
			rtl8672_L2EntryLookup(l4entry.NHMAC,in_vlan_id,&l2);
			l4entry.DstPortIdx=l2.Port;	
			//printk("set ip2=%x mac ok = %02x-%02x-%02x-%02x-%02x-%02x portidx=%d out-vlan=%d\n",l4entry.DstIP,l4entry.NHMAC[0],l4entry.NHMAC[1],l4entry.NHMAC[2],l4entry.NHMAC[3],l4entry.NHMAC[4],l4entry.NHMAC[5],l4entry.DstPortIdx,l4entry.OVlanID);
			result=rtl8672_addAsicL4Entry(&l4entry, 0);
			/*
			if(result==FAILED){
				printk("add l4 entry failed!\n");
			}
			else
				printk("SUCCESS!\n");
			*/
		}
		else
		{
			return LR_FAILED;
		}

	}
	else
	{
		//printk("2can't found IP:%x's MAC\n",l4entry.SrcIP);
		return LR_NONEXIST;
	}

	return LR_SUCCESS;
}
