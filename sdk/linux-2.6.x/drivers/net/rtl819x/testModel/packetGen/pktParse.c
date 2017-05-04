/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* Program : parse pkt
*
* Original Creator: Edward Jin-Ru Chen 2002/4/29
* $Author: ikevin362 $
*
* $Revision: 1.1 $
* $Id: pktParse.c,v 1.1 2012/10/24 04:18:57 ikevin362 Exp $
* $Log: pktParse.c,v $
* Revision 1.1  2012/10/24 04:18:57  ikevin362
* initial version
*
* Revision 1.2  2005/07/20 15:29:38  yjlou
* +: porting Model Code to Linux Kernel: check RTL865X_MODEL_KERNEL.
*
* Revision 1.1  2004/02/25 14:27:24  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/18 13:12:21  chenyl
* *** empty log message ***
*
* Revision 1.1  2004/02/13 05:18:47  chenyl
* + Pkt generating functions
*
* Revision 1.12  2003/10/06 10:16:23  waynelee
* add pptp gre support
*
* Revision 1.11  2003/09/30 09:40:55  waynelee
* support llc_other
*
* Revision 1.10  2002/10/25 11:22:12  waynelee
* change the rule of filling the content of payload
*
* Revision 1.9  2002/10/20 07:32:20  waynelee
* users can define the content of payload (but only indicate one byte)
*
* Revision 1.8  2002/10/03 13:34:54  waynelee
* support l3cks,l4cks,eflag fields
*
* Revision 1.7  2002/10/02 15:12:31  waynelee
* ipx support debug
*
* Revision 1.6  2002/10/01 07:47:33  waynelee
* support user-defined ipid, df, mf, ipoff
*
* Revision 1.5  2002/09/16 07:31:16  waynelee
* support IPX (experimental)
*
*/

#include "pktParse.h"
#include "utility.h"
// #include "pktForm.h"


void pktParse(int8 * pkt, pktParse_t * result, uint32 totalLen) {
	uint32	type, len, lenb;
	uint16 tmp, csum;
	int8 * nextPtr;
	uint8 nh, exhdrlen;
	uint8 dip[16];
	hopbyhopHdr_t * hopPtr;
	DesHdr_t * desPtr;
	DesHdr_t * desPtr2;
	RouHdr_t * rouPtr;
	FraHdr_t * fraPtr;
	AutHdr_t * autPtr;
	uint8 segmentleft=0;

	memset(result, 0, sizeof(pktParse_t));

	// total length
	result->totalLen = totalLen;

	//Parse layer2 headers
	result->ether2 = (_etherHdr_t *)pkt;
	nextPtr = pkt + sizeof(_etherHdr_t);
	type = ntohs(result->ether2->etherType);

	if(ntohs(result->ether2->etherType) == 0x8100) {
		result->vlan = (_vlanHdr_t *)nextPtr;
		result->pktFlag |= L2_VLAN;
		nextPtr += sizeof(_vlanHdr_t);
		type = ntohs(result->vlan->etherType);
	}

	if(type < 0x0600) {//802.3-2000 p.79
		result->llc = (_llcHdr_t *)nextPtr;
		result->pktFlag |= L2_SNAP;
		nextPtr += sizeof(_llcHdr_t);
		type = ntohs(result->llc->etherType);
	}

	if(type == 0x8864 || type == 0x8863 ) {
		result->pppoe = (_pppoeHdr_t *)nextPtr;
		result->pktFlag |= L2_PPPoE;
		nextPtr += sizeof(_pppoeHdr_t);
		type = ntohs(result->pppoe->proto);
	}

	//Parse layer3 headers, IP, ARP
	if( ((result->pktFlag & L2_PPPoE) == L2_PPPoE && type == 0x0021) ||
		((result->pktFlag & L2_PPPoE) == 0 && 
		  (((result->pktFlag & L2_SNAP) == L2_SNAP && result->llc->llc_dsap == 0xAA &&	result->llc->llc_ssap == 0xAA && result->llc->ctrl == 0x03) || 
		   (result->pktFlag & L2_SNAP) == 0) &&
			type == 0x0800) ) {
		result->ip = (ipHdr_t *)nextPtr;
		result->pktFlag = 0;
		result->pktFlag |= L3_IP;
		nextPtr += result->ip->ip_hl*4;
		type = result->ip->ip_p;
		len = ntohs(result->ip->ip_len) - result->ip->ip_hl*4;
		// store ip data length to dataLen
		result->dataLen = len;
		// ip payload
		result->payload = (uint8 *)nextPtr;
		if(ipcsum((uint16 *)result->ip, result->ip->ip_hl*4, 0) == 0)
			result->l3Flag |= PKT_PARSE_L3_CKSUM_OK;
		else {
			uint16 orgCksum = result->ip->ip_sum;
			
			// printf("Find l3 header error\n");
			result->l3Flag |= PKT_PARSE_L3_CKSUM_ERR;
			result->ip->ip_sum = 0;
			result->l3Cksum = ~ipcsum((uint16 *)result->ip, result->ip->ip_hl*4, 0);
			result->ip->ip_sum = orgCksum;
		}
	}
	else if  ((result->pktFlag & L2_PPPoE) == 0 && (type == 0x0806 || type == 0x8035)) {
		result->arp = (arpHdr_t *)nextPtr;
		result->pktFlag |= L3_ARP;
		nextPtr = NULL;//Arp do not parse futher
		return;
	}
	else if  (type == 0x8137) {
		result->ipx = (ipxHdr_t *)nextPtr;
		nextPtr = NULL;
		return;
	}
	//added by liujuan;
	else if ( ((result->pktFlag & L2_PPPoE) == L2_PPPoE && type == 0x0057) ||
                ((result->pktFlag & L2_PPPoE) == 0 &&
                  (((result->pktFlag & L2_SNAP) == L2_SNAP && result->llc->llc_dsap == 0xAA &&  result->llc->llc_ssap == 0xAA && result->llc->ctrl == 0x03) ||
                   (result->pktFlag & L2_SNAP) == 0) && type == 0x86DD)) {
		result->ipv6 =(ip6Hdr_t *)nextPtr;
		result->pktFlag |= L3_IPV6;

		nh=result->ipv6->ip_next;
		nextPtr= nextPtr+40;

	//	rtlglue_printf("nh is [%d]\n", nh);
		exhdrlen=0;
		while(1) {
			if(nh==0) {
				result->exheader |= hopbyhopHdr;
		//		nextPtr= nextPtr+40;
				hopPtr=(hopbyhopHdr_t *)nextPtr;
				nh=hopPtr->nexthdr;
				nextPtr=nextPtr+16;
				exhdrlen=exhdrlen+16;
			}
			else if (nh==60) {
				if(result->exheader & DesHdr) {
					result->exheader |=DesHdr2;
			//		nextPtr=nextPtr + 16;
					desPtr2=(DesHdr_t *)nextPtr;
					nh=desPtr2->nexthdr;
					nextPtr= nextPtr+16;
					exhdrlen=exhdrlen+16;
				}
				else {
					result->exheader |= DesHdr;
					desPtr=(DesHdr_t *)nextPtr;
					nh=desPtr->nexthdr;
					nextPtr=nextPtr+16;
					exhdrlen=exhdrlen+16;
				}
			}
			else if(nh==43) {
				result->exheader |= RouHdr;
				rouPtr = (RouHdr_t *)nextPtr;
				nh=rouPtr->nexthdr;
				segmentleft=rouPtr->segmentleft;
				memcpy(dip, nextPtr-8+rouPtr->hdrlen*8, 16);
				nextPtr=nextPtr + ((rouPtr->hdrlen * 8) +8);
				exhdrlen=exhdrlen+rouPtr->hdrlen*8+8;
		//		dip=rouPtr->dstip;
			}
			else if(nh==44) {
				result->exheader |= FraHdr;
				fraPtr=(FraHdr_t *)nextPtr;
				nh=fraPtr->nexthdr;
				nextPtr=nextPtr+8;
				exhdrlen=exhdrlen+8;
			}
			else if(nh==51) {
				result->exheader |= AutHdr;
				autPtr=(AutHdr_t *)nextPtr;
				nh=autPtr->nexthdr;
				nextPtr=nextPtr+20;
				exhdrlen=exhdrlen+20;
			}
			else {
		//		rtlglue_printf("IPV6 packet with error next header\n");
				break;
			}
		}  
		//if (result->ipv6->ip_next==hopbyhopHdr) {
			//result->exheader |= hopbyhopHdr;
	//		nextPtr = nextPtr + 40;
		//	hopPtr =(hopbyhopHdr_t *)nextPtr;
//		}

		if (nh==6)
			type=0x06;
		else if (nh==17)
			type=0x11;
		else if (nh==58)
			type=0x3a;
		else if (nh==59) {
			nextPtr=NULL;
			return;
		}
	//	rtlglue_printf("the ipv6 iplength is [%d]\n", result->ipv6->ip_len);
	//	rtlglue_printf("the ipv6 ip_len is [%d]\n", result->ipv6->ip_len);
	//	rtlglue_printf("the length of extension header is [%d]\n", exhdrlen);
		len=ntohs(result->ipv6->ip_len) - exhdrlen;
		//len=26-exhdrlen;
	//	rtlglue_printf("the len is [%d]\n", len);
		result->dataLen=len;
		
	}		
	else {
		result->payload = (uint8 *)nextPtr;
		// caculate the data len
		result->dataLen = totalLen - sizeof(_etherHdr_t);
		if((result->pktFlag & L2_VLAN) == L2_VLAN)
			result->dataLen -= sizeof(_vlanHdr_t);
		if((result->pktFlag & L2_SNAP) == L2_SNAP)
			result->dataLen -= sizeof(_llcHdr_t);
		if((result->pktFlag & L2_PPPoE) == L2_PPPoE)
			result->dataLen -= sizeof(_pppoeHdr_t);
		nextPtr = NULL;
		return;
	}

	//Parse layer4 headers
	if((result->pktFlag & L3_IP) == L3_IP) {
		uint16 orgCksum, phCksum;

		result->l4Flag = 0;
		switch(type) {
		case IP_PROTO_PPTP:
			result->pptp = (greHdr_t *) nextPtr;
			nextPtr += sizeof(greHdr_t);
			result->payload = (uint8 *)nextPtr;
			result->dataLen -= sizeof(greHdr_t);
			result->pktFlag |= L4_PPTP;
			break;
		case IP_PROTO_ICMP:
			result->icmp = (icmpHdr_t *) nextPtr;
			if(ipcsum((uint16 *)result->icmp, len, 0)==0)
				result->l4Flag |= PKT_PARSE_L4_CKSUM_OK;
			else {
				orgCksum = result->icmp->icmp_cksum;

				result->l4Flag |= PKT_PARSE_L4_CKSUM_ERR;
				result->icmp->icmp_cksum = 0;
				result->l4Cksum = ~ipcsum((uint16 *)result->icmp, len, 0);
				result->icmp->icmp_cksum = orgCksum;
			}
			nextPtr += sizeof(icmpHdr_t);
			result->payload = (uint8 *)nextPtr;
			result->dataLen -= sizeof(icmpHdr_t);
			result->pktFlag |= L4_ICMP;
			break;
		case IP_PROTO_IGMP:
			result->igmp = (igmpHdr_t *) nextPtr;
			if(ipcsum((uint16 *)result->igmp, len, 0)==0)
				result->l4Flag |= PKT_PARSE_L4_CKSUM_OK;
			else {
				orgCksum = result->igmp->igmp_cksum;
				result->l4Flag |= PKT_PARSE_L4_CKSUM_ERR;
				result->igmp->igmp_cksum = 0;
				result->l4Cksum = ~ipcsum((uint16 *)result->igmp, len, 0);
				result->igmp->igmp_cksum = orgCksum;
			}
			nextPtr += sizeof(igmpHdr_t);
//			result->payload = (uint8 *)nextPtr;
//			result->dataLen -= sizeof(igmpHdr_t);
			result->pktFlag |= L4_IGMP;
			break;
		case IP_PROTO_TCP:
			result->tcp = (tcpHdr_t *) nextPtr;
			phCksum = ipcsum((uint16 *)&result->ip->ip_src, 8, 0);
			tmp = htons(IP_PROTO_TCP);
			phCksum = ipcsum(&tmp, 2, phCksum);
			tmp = len;
//			printf("tmp %x ",tmp);
			tmp = htons(tmp);
//			printf("tmp after %x\n",tmp);
			phCksum = ipcsum(&tmp, 2, phCksum);
			csum = ipcsum((uint16 *)result->tcp, len, phCksum);
			if(csum == 0)
				result->l4Flag |= PKT_PARSE_L4_CKSUM_OK;
			else {
				orgCksum = result->tcp->th_sum;
				result->l4Flag |= PKT_PARSE_L4_CKSUM_ERR;
				result->tcp->th_sum = 0;
				result->l4Cksum = ~ipcsum((uint16 *)result->tcp, len, phCksum);
				result->tcp->th_sum = orgCksum;
			}
			// nextPtr += (sizeof(tcpHdr_t) + result->tcp->th_off*4);
			nextPtr += result->tcp->th_off*4;
			result->pktFlag |= L4_TCP;
			result->dataLen -= result->tcp->th_off*4;
			result->payload = (uint8 *)nextPtr;
			break;
		case IP_PROTO_UDP:
			result->udp = (udpHdr_t *) nextPtr;
			if((uint16)len != ntohs(result->udp->uh_ulen))
				result->l4Flag |= PKT_PARSE_UDP_LEN_ERR;
			phCksum = ipcsum((uint16 *)&result->ip->ip_src, 8, 0);
			tmp = htons(IP_PROTO_UDP);
			phCksum = ipcsum(&tmp, 2, phCksum);
			phCksum = ipcsum((uint16 *)&result->udp->uh_ulen, 2, phCksum);
			csum = ipcsum((uint16 *)result->udp, len, phCksum);
			if(result->udp->uh_sum == 0)
				result->l4Flag |= PKT_PARSE_UDP_NO_CKSUM;
			else if(csum == 0)
				result->l4Flag |= PKT_PARSE_L4_CKSUM_OK;
			else {
				orgCksum = result->udp->uh_sum;
				result->l4Flag |= PKT_PARSE_L4_CKSUM_ERR;
				result->udp->uh_sum = 0;
				result->l4Cksum = ~ipcsum((uint16 *)result->udp, len, phCksum);
				result->udp->uh_sum = orgCksum;
			}
			nextPtr += sizeof(udpHdr_t);
			result->pktFlag |= L4_UDP;
			result->dataLen -= sizeof(udpHdr_t);
			result->payload = (uint8 *)nextPtr;
			break;
		}
	}
	else if((result->pktFlag & L3_IPV6)==L3_IPV6) {
		uint16 orgCksum, phCksum;
		result->l4Flag=0;
		switch (type) {
		case IP_PROTO_V6ICMP:
			result->icmp = (icmpHdr_t *) nextPtr;
			if((result->exheader & RouHdr) && (segmentleft !=0)) {
				csum=ipcsum((uint16 *)result->ipv6->ip_src, 16, 0);
				csum=ipcsum((uint16 *)dip, 16, csum);
				lenb=htonl(len);
				csum=ipcsum((uint16 *)&lenb, 4, csum);
				tmp=58;
				tmp=htons(tmp);
				csum=ipcsum(&tmp, 2, csum);
				phCksum=ipcsum((uint16 *)result->icmp, len, csum);
			}
			else {
				csum=ipcsum((uint16 *) result->ipv6->ip_src, 32, 0);
				lenb=htonl(len);
				csum=ipcsum((uint16 *)&lenb, 4, csum);
				tmp=58;
				tmp=htons(tmp);
				csum=ipcsum(&tmp, 2, csum);
				phCksum=ipcsum((uint16 *)result->icmp, len, csum);
			}
			if(phCksum==0)
				result->l4Flag |= PKT_PARSE_L4_CKSUM_OK;
			else {
				orgCksum = result->icmp->icmp_cksum;

				result->l4Flag |= PKT_PARSE_L4_CKSUM_ERR;
				result->icmp->icmp_cksum = 0;
				result->l4Cksum = ~ipcsum((uint16 *)result->icmp, len, csum);
				result->icmp->icmp_cksum = orgCksum;
			}
			nextPtr += sizeof(icmpHdr_t);
			result->payload = (uint8 *)nextPtr;
			result->dataLen -= sizeof(icmpHdr_t);
			result->pktFlag |= L4_ICMP; 
			break; 
		case IP_PROTO_TCP:
			result->tcp = (tcpHdr_t *) nextPtr;

/*			nh=6;
        	//	rtlglue_printf("hello!! in the form tcp checksum!!!\n\n");
        		if (result->exheader & RouHdr) {
               			 csum=ipcsum((uint16 *)&result->ipv6->ip_src, 16, 0);
             			 csum=ipcsum((uint16 *)dip, 16, csum);
       			 }
       			 else
               			 csum=ipcsum((uint16 *)&result->ipv6->ip_src, 32, 0);
       			 tmp=htons(len);
      			 rtlglue_printf("\n\nthe tmp is [%d]\n\n", tmp);
        //tmp=30;
       			 csum=ipcsum(&tmp, 4, csum);
       			 tmp=nh;
       			 csum=ipcsum(&tmp, 1, csum);
			 phCksum=ipcsum((uint16 *)result->tcp, len, csum);
       			 result->tcp->th_sum=0;
        	//	rtlglue_printf("              hi             ");
       			 csum=ipcsum((uint16 *)result->tcp, len, csum);
      			 rtlglue_printf("the tcp checksum is [%d] \n\n", csum);*/

			if ((result->exheader & RouHdr)&& (segmentleft !=0)) {
				csum=ipcsum((uint16 *)&result->ipv6->ip_src, 16, 0);
				csum=ipcsum((uint16 *) dip, 16, csum);
			//	rtlglue_printf("             here\n");
			}
			else
				csum=ipcsum((uint16 *)&result->ipv6->ip_src, 32, 0);
	//		rtlglue_printf("the csum1  is  [%d]\n", csum);
	//		rtlglue_printf("the ip address is [%d]\n", result->ipv6->ip_src[0]);
	//		rtlglue_printf("the ip address is [%d]\n", result->ipv6->ip_src[1]);
	//		rtlglue_printf("the ip address is [%d]\n", result->ipv6->ip_src[2]);
	//		rtlglue_printf("the ip address is [%d]\n", result->ipv6->ip_src[3]);
	//		rtlglue_printf("the ip address is [%d]\n", result->ipv6->ip_src[4]); 
		//	tmp=htons(len);
			lenb=htonl(len);
	//		rtlglue_printf("the lenb is [%d]\n", lenb);
			csum=ipcsum((uint16 *)&lenb, 4, csum);
	//		rtlglue_printf("the csum2 is [%d]\n", csum);
			tmp=6;
			tmp=htons(tmp);
			csum=ipcsum(&tmp, 2, csum);
	//		rtlglue_printf("the csum3 is [%d]\n", csum);
	//		rtlglue_printf("the checksum in tcp packet is [%d]\n ",htons(result->tcp->th_sum));
			
			phCksum=ipcsum((uint16 *) result->tcp, len, csum);
			
	//		result->tcp->th_sum=0;
	//		csum= ipcsum((uint16 *)result->tcp, len, csum);
	//		rtlglue_printf("the csum4 is [%d]\n", csum);
	//		rtlglue_printf("the checksum without th_sum is [%d]\n ", csum);			  
	//		phCksum = ipcsum((uint16 *)&result->ip->ip_src, 8, 0);
//			tmp = htons(IP_PROTO_TCP);
//			phCksum = ipcsum(&tmp, 2, phCksum);
//			tmp = len;
//			printf("tmp %x ",tmp);
//			tmp = htons(tmp);
//			printf("tmp after %x\n",tmp);
//			phCksum = ipcsum(&tmp, 2, phCksum);
//			csum = ipcsum((uint16 *)result->tcp, len, phCksum);
			if(phCksum == 0)
				result->l4Flag |= PKT_PARSE_L4_CKSUM_OK;
			else {
				orgCksum = result->tcp->th_sum;
				result->l4Flag |= PKT_PARSE_L4_CKSUM_ERR;
				result->tcp->th_sum = 0;
				result->l4Cksum = ~ipcsum((uint16 *)result->tcp, len, csum);
				result->tcp->th_sum = orgCksum;
		//		rtlglue_printf("the error checksum result is [%d]\n", phCksum);
			}
			// nextPtr += (sizeof(tcpHdr_t) + result->tcp->th_off*4);
			nextPtr += result->tcp->th_off*4;
			result->pktFlag |= L4_TCP;
			result->dataLen -= result->tcp->th_off*4;
			result->payload = (uint8 *)nextPtr;
			break; 
		case IP_PROTO_UDP:
			result->udp = (udpHdr_t *) nextPtr;
			if((uint16)len != ntohs(result->udp->uh_ulen))
				result->l4Flag |= PKT_PARSE_UDP_LEN_ERR;
	//		phCksum = ipcsum((uint16 *)&result->ip->ip_src, 8, 0);
	//		tmp = htons(IP_PROTO_UDP);
	//		phCksum = ipcsum(&tmp, 2, phCksum);
	//		phCksum = ipcsum((uint16 *)&result->udp->uh_ulen, 2, phCksum);
	//		csum = ipcsum((uint16 *)result->udp, len, phCksum);
			if((result->exheader & RouHdr)&& (segmentleft!=0)) {
				csum=ipcsum((uint16 *)result->ipv6->ip_src, 16, 0);
				csum=ipcsum((uint16 *)dip, 16, csum);
			}
			else
				csum=ipcsum((uint16 *)result->ipv6->ip_src, 32, 0);
			tmp=htons(len);
			csum=ipcsum(&tmp, 2, csum);
			tmp=17;
			tmp=htons(tmp);
			csum=ipcsum(&tmp, 2, csum);
			phCksum=ipcsum((uint16 *)result->udp, len, csum);
			if(result->udp->uh_sum == 0)
				result->l4Flag |= PKT_PARSE_UDP_NO_CKSUM;
			else if(phCksum == 0)
				result->l4Flag |= PKT_PARSE_L4_CKSUM_OK;
			else {
				orgCksum = result->udp->uh_sum;
				result->l4Flag |= PKT_PARSE_L4_CKSUM_ERR;
				result->udp->uh_sum = 0;
				result->l4Cksum = ~ipcsum((uint16 *)result->udp, len, csum);
				result->udp->uh_sum = orgCksum;
			}
			nextPtr += sizeof(udpHdr_t);
			result->pktFlag |= L4_UDP;
			result->dataLen -= sizeof(udpHdr_t);
			result->payload = (uint8 *)nextPtr;
			break;
		}
		
	}
}

