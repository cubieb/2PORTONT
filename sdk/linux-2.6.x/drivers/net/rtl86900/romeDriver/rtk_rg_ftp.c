#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/skbuff.h>
#endif

#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_ftp.h>
#include <rtk_rg_alg_tool.h>


int _rtk_rg_ftp_parse_addressAndPortString(unsigned char * pData, ipaddr_t * pIpAddr, unsigned short * pPortNum)
{
	char *endCP;
	
#ifdef __KERNEL__
	*pIpAddr = simple_strtoul(pData,&endCP,0);

	*pIpAddr<<=8;

	pData=endCP+1;
	*pIpAddr += simple_strtoul(pData,&endCP,0);

	*pIpAddr<<=8;

	pData=endCP+1;
	*pIpAddr += simple_strtoul(pData,&endCP,0);

	*pIpAddr<<=8;

	pData=endCP+1;
	*pIpAddr += simple_strtoul(pData,&endCP,0);

	pData=endCP+1;
	*pPortNum = simple_strtoul(pData,&endCP,0);

	*pPortNum<<=8;

	pData=endCP+1;
	*pPortNum += simple_strtoul(pData,&endCP,0);
#endif

	return 1;
}

static char portString[256]={0};
static char preStr[64]={0};
static char postStr[64]={0};

static int _rtk_rg_ftp_process(int direct, unsigned char * pData, unsigned int dataLen,rtk_rg_alg_connection_t * pConn)
{
	int ret,newDelta=0;
	ipaddr_t ipAddr,gwIPAddr;
	unsigned short portNum,newPort;
	unsigned int newLen;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_upnpConnection_t upnpConn;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pConn->pPktHdr;
	skb = (struct sk_buff *)pConn->skb;

	if(direct==NAPT_DIRECTION_OUTBOUND)
	{
		//Server In WAN's ACTIVE MODE
		if(*((unsigned int *)pData) == htonl(ALG_FTP_PORT_STR))
		{
			//DEBUG("len is %d, l4offset is %d, headerlen is %d",dataLen,pPktInfo->l4Offset, pPktInfo->headerLen);
			//DEBUG("POST_FUNCTION outbound: i am in _rtk_rg_algRegFunc_TCP_FTP!! data len is %d dport is %d",dataLen,pPktInfo->dport);
			//DEBUG("DATA is:");
			ipAddr=0;
			portNum=0;
			_rtk_rg_ftp_parse_addressAndPortString(pData+ALG_FTP_PORT_LENGTH,&ipAddr,&portNum);	//bypass string "PORT "

			newPort=_rtk_rg_extPort_get(1,portNum);
			if(newPort==FAIL) return SUCCESS;

			gwIPAddr=pConn->tuple.extIp.ip;
			snprintf(portString,sizeof(portString),"PORT %d,%d,%d,%d,%d,%d\r\n",
				(gwIPAddr>>24)&0xff,
				(gwIPAddr>>16)&0xff,
				(gwIPAddr>>8)&0xff,
				gwIPAddr&0xff,
				newPort>>8,
				newPort&0xff);

			//DEBUG("the modified port command is \"%s\"",portString);
			//DEBUG("data before len is %d, after is %d",dataLen,strlen(portString));

			newLen=strlen(portString);
			newDelta=(newLen-dataLen);
			//DEBUG("the newDelta is %d",newDelta);
			
			if(skb->tail+newDelta > skb->end)	
			{
				//need to re-alloc skb data structure
				FIXME("Overflow! we need to re-alloc skb data stucture...");
			}
			else
			{
				//copy the new string into skb and enlarge or shrink the length
				memcpy(pData,portString,newLen);

				//fix packet length
				//DEBUG("sip is %x, sport is %d",pPktInfo->ipv4Sip,pPktInfo->sport);					
				if(newDelta != 0)
				{
					skb->len += newDelta;
					skb->tail += newDelta;
					pPktHdr->l3Len += newDelta;
					*pPktHdr->pL3Len=htons(pPktHdr->l3Len);
					//20140507LUKE:if pppoe, we should add to pppoe len here!!
					if((pPktHdr->tagif&PPPOE_TAGIF)>0)
						*pPktHdr->pPppoeLength=htons(ntohs(*pPktHdr->pPppoeLength) + newDelta);

					//20140507LUKE:re-caculate IP checksum (because total len change)
					*pPktHdr->pIpv4Checksum=0;
					*pPktHdr->pIpv4Checksum=htons(inet_chksum(skb->data+pPktHdr->l3Offset,pPktHdr->l4Offset-pPktHdr->l3Offset));
				}
			}

			//20140507LUKE:re-caculate TCP checksum (because payload change)
			*pPktHdr->pL4Checksum=0;
			*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,pPktHdr->l3Offset+pPktHdr->l3Len-pPktHdr->l4Offset,ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));

			pConn->outDelta+=newDelta;	//update connection's total delta
		
			//Add a one-shot UPnP flow for incoming connection to L4 WAN		
			upnpConn.is_tcp=pConn->tuple.isTcp;
			upnpConn.wan_intf_idx=pPktHdr->netifIdx;
			upnpConn.gateway_port=newPort;
			upnpConn.local_ip=pConn->tuple.internalIp.ip;
			upnpConn.local_port=portNum;
			upnpConn.limit_remote_ip=1;
			upnpConn.limit_remote_port=0;		//can't restrict client using which port to connect
			upnpConn.remote_ip=pConn->tuple.remoteIp.ip;
			upnpConn.remote_port=0;
			upnpConn.type=UPNP_TYPE_ONESHOT;
			upnpConn.timeout=rg_db.algUserDefinedTimeout[RTK_RG_ALG_FTP_TCP];	//auto time out if the server do not connect to this WAN
			assert_ok((pf.rtk_rg_upnpConnection_add)(&upnpConn,&ret));	
		}
		else if(*((unsigned int *)pData) == htonl(ALG_FTP_PASV_RESP_STR))		//Server In LAN's PASSIVE MODE
		{
			//DEBUG("len is %d, l4offset is %d, headerlen is %d",dataLen,pPktInfo->l4Offset, pPktInfo->headerLen);
			//DEBUG("POST_FUNCTION outbound: i am in _rtk_rg_algSrvInLanRegFunc_TCP_FTP!! data len is %d dport is %d",dataLen,pPktInfo->dport);
			//DEBUG("DATA is:");
			ipAddr=0;
			portNum=0;
			_rtk_rg_ftp_parse_addressAndPortString(strchr(pData,'(')+1,&ipAddr,&portNum); //bypass string before "("
			//DEBUG("the ipAddr is %x",ipAddr);
			//DEBUG("the port is %d",portNum);		

			newPort=_rtk_rg_extPort_get(1,portNum);
			if(newPort==FAIL) return SUCCESS;

			bzero(portString,sizeof(portString));
			bzero(preStr,sizeof(preStr));
			bzero(postStr,sizeof(postStr));
			
			gwIPAddr=pConn->tuple.extIp.ip;
			strncpy(preStr,pData,(unsigned char *)strchr(pData,'(')-pData);
			strncpy(postStr,strchr(pData,')')+1,(unsigned char *)strchr(pData,'\r')-(unsigned char *)strchr(pData,')')-1);
			//DEBUG("preStr is \"%s\", len is %d",preStr,(unsigned char *)strchr(pData,'(')-pData);
			//DEBUG("postStr is \"%s\", len is %d",postStr,(unsigned char *)strchr(pData,'\r')-(unsigned char *)strchr(pData,')')-1);
			snprintf(portString,sizeof(portString),"%s(%d,%d,%d,%d,%d,%d)%s\r\n",
				preStr,
				(gwIPAddr>>24)&0xff,
				(gwIPAddr>>16)&0xff,
				(gwIPAddr>>8)&0xff,
				gwIPAddr&0xff,		
				newPort>>8,
				newPort&0xff,
				postStr);
				

			//DEBUG("the modified port response is\"%s\"",portString);
			//DEBUG("data before len is %d, after is %d",dataLen,strlen(portString));

			newLen=strlen(portString);
			newDelta=(newLen-dataLen);
			//DEBUG("the newDelta is %d",newDelta);
			
			if(skb->tail+newDelta > skb->end)	
			{
				//need to re-alloc skb data structure
				FIXME("Overflow! we need to re-alloc skb data stucture...");
			}
			else
			{
				//copy the new string into skb and enlarge or shrink the length
				memcpy(pData,portString,newLen);

				//fix packet length
				//DEBUG("sip is %x, sport is %d",pPktInfo->ipv4Sip,pPktInfo->sport);					
				if(newDelta != 0)
				{
					skb->len += newDelta;
					skb->tail += newDelta;
					pPktHdr->l3Len += newDelta;
					*pPktHdr->pL3Len=htons(pPktHdr->l3Len);
					//20140507LUKE:if pppoe, we should add to pppoe len here!!
					if((pPktHdr->tagif&PPPOE_TAGIF)>0)
						*pPktHdr->pPppoeLength=htons(ntohs(*pPktHdr->pPppoeLength) + newDelta);
					
					//20140507LUKE:re-caculate IP checksum (because total len change)
					*pPktHdr->pIpv4Checksum=0;
					*pPktHdr->pIpv4Checksum=htons(inet_chksum(skb->data+pPktHdr->l3Offset,pPktHdr->l4Offset-pPktHdr->l3Offset));
				}
			}

			//20140507LUKE:re-caculate TCP checksum (because payload change)
			*pPktHdr->pL4Checksum=0;
			*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,pPktHdr->l3Offset+pPktHdr->l3Len-pPktHdr->l4Offset,ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));

			pConn->outDelta+=newDelta;	//update connection's total delta
		
			//Add a one-shot UPnP flow for incoming connection to L4 WAN		
			upnpConn.is_tcp=pConn->tuple.isTcp;
			upnpConn.wan_intf_idx=pPktHdr->netifIdx;
			upnpConn.gateway_port=newPort;
			upnpConn.local_ip=pConn->tuple.internalIp.ip;
			upnpConn.local_port=portNum;
			upnpConn.limit_remote_ip=1;
			upnpConn.limit_remote_port=0;		//can't restrict client using which port to connect
			upnpConn.remote_ip=pConn->tuple.remoteIp.ip;
			upnpConn.remote_port=0;
			upnpConn.type=UPNP_TYPE_ONESHOT;
			upnpConn.timeout=rg_db.algUserDefinedTimeout[RTK_RG_ALG_FTP_TCP_SRV_IN_LAN];	//auto time out if the client do not connect to this WAN
			assert_ok((pf.rtk_rg_upnpConnection_add)(&upnpConn,&ret));	
			//TRACE("alg FTP upnp add success, newPort=%d  portNum=%d",newPort,portNum);
		}
	}
	return 1;
}

int _rtk_rg_ftp_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
{
	int ret = SUCCESS;
#ifdef __KERNEL__
	unsigned char * pData, *pAppData;
	unsigned int appLen=0,dataOff=0;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_alg_connection_t * pConn;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	pConn = (rtk_rg_alg_connection_t *)pConnInfo;
	skb= (struct sk_buff *)pSkb;
	
	pData=skb->data;
	if(pPktHdr->tagif&TCP_TAGIF)
		dataOff = pPktHdr->l4Offset + pPktHdr->headerLen;
	else
		dataOff = pPktHdr->l4Offset + 8; /*udp header length is 8 bytes*/
	
	appLen = skb->len - dataOff;
	pAppData = pData + dataOff;

	//ftp command length is at least 14 bytes:port_1,1,1,1,1
	if (appLen < 14)
		return FAIL;

	//do nothing before napt modification
	if(after == 0)
		return SUCCESS;
	
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{		
		if(pConn->tuple.isIp6==0)
		{
			pConn->tuple.extIp.ip = ntohl(*pPktHdr->pIpv4Sip);
			pConn->tuple.extPort = ntohs(*pPktHdr->pSport);
		}
	}

	pConn->skb=pSkb;
	pConn->pPktHdr=pPktInfo;

	ret = _rtk_rg_ftp_process(direct, pAppData, appLen, pConn);
	if(ret < 1)
		return FAIL;
#endif	
	return ret;
}


int _rtk_rg_server_in_wan_passive_mode_naptPriority_assign(unsigned char *pPktInfo, rtk_rg_alg_connection_t * pConn){
	//This API is used for parsing the remote server selected port, and assign naprPriorty to data flow.

#ifdef __KERNEL__
	int index;
	rtk_rg_pktHdr_t *pPktHdr;
	rtk_rg_naptFilterAndQos_t napt_filter;

	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;

	if((pPktHdr->tagif&IPV4_TAGIF)&&((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))){//FTP only translated by TCP or UDP.
		if(pPktHdr->pL4Payload!=NULL && (*((unsigned int *)pPktHdr->pL4Payload) == htonl(ALG_FTP_PASV_RESP_STR))){ //The PASV request packet start with 227(ASCii) in payload.
			//check is outbound priority need to assigned 
			if(rg_db.naptOut[pPktHdr->naptOutboundIndx].priValid==ENABLED){
				bzero(&napt_filter,sizeof(napt_filter));
				napt_filter.direction=RTK_RG_NAPT_FILTER_OUTBOUND;
				napt_filter.filter_fields = (INGRESS_DIP|INGRESS_DPORT);
				napt_filter.ingress_dest_ipv4_addr = pConn->tuple.remoteIp.ip;
				napt_filter.ingress_dest_l4_port = pConn->tuple.remotePort;
				napt_filter.action_fields = ASSIGN_NAPT_PRIORITY_BIT;
				napt_filter.assign_priority = rg_db.naptOut[pPktHdr->naptOutboundIndx].priValue;//follow control flow.
				napt_filter.ruleType = RTK_RG_NAPT_FILTER_ONE_SHOT;
				assert_ok((pf.rtk_rg_naptFilterAndQos_add)(&index,&napt_filter));		
				TRACE("FTP ALG add outbound naptFilter[%d](DIP=0x%x + dport=%d => priority=%d, one shot) for data flow (The priority is based on napt[%d])",index,pConn->tuple.remoteIp,pConn->tuple.remotePort,rg_db.naptOut[pPktHdr->naptOutboundIndx].priValue,pPktHdr->naptOutboundIndx);		
			}

			//check is inbound priority need to assigned 
			if(rg_db.naptIn[pPktHdr->naptrInboundIndx].priValid==ENABLED){
				bzero(&napt_filter,sizeof(napt_filter));
				napt_filter.direction=RTK_RG_NAPT_FILTER_INBOUND;
				napt_filter.filter_fields = (INGRESS_SIP|INGRESS_SPORT);
				napt_filter.ingress_src_ipv4_addr = pConn->tuple.remoteIp.ip;
				napt_filter.ingress_src_l4_port = pConn->tuple.remotePort;
				napt_filter.action_fields = ASSIGN_NAPT_PRIORITY_BIT;
				napt_filter.assign_priority = rg_db.naptIn[pPktHdr->naptrInboundIndx].priValue;//follow control flow.
				napt_filter.ruleType = RTK_RG_NAPT_FILTER_ONE_SHOT;
				assert_ok((pf.rtk_rg_naptFilterAndQos_add)(&index,&napt_filter)); 	
				TRACE("FTP ALG add inbound naptFilter[%d](SIP=0x%x + sport=%d => priority=%d, one shot) for data flow (The priority is based on naptr[%d])",index,pConn->tuple.remoteIp,pConn->tuple.remotePort,rg_db.naptIn[pPktHdr->naptrInboundIndx].priValue,pPktHdr->naptrInboundIndx);		
			}
		}
	}
#endif	
	
	return SUCCESS;
}


rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_ftp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
//Attention: caller function needs to make sure it needs to do napt modification
//ipv6 address and port doesn't need to do napt modification now
#ifdef __KERNEL__
	int ret;
	rtk_rg_pktHdr_t *pPktHdr;
	rtk_rg_alg_connection_t * pConn;
	rtk_rg_alg_tuple_t tuple;
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));
	
	if(after==0)
	{
		//Pre function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			
			pConn = _rtk_rg_alg_connection_find(&tuple);
			if(pConn == NULL)
			{
				pConn = _rtk_rg_alg_connection_add(&tuple);	
			}
			else
				_rtk_rg_sync_tcp_ack_seq(direct, pPktHdr, pConn);
		}
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{		
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			
			pConn = _rtk_rg_alg_connection_find(&tuple);		
			if(pConn == NULL)
				return RG_FWDENGINE_ALG_RET_FAIL;
			
			ret = _rtk_rg_ftp_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);	
		}
		else
		{
			//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
	
			pConn = _rtk_rg_alg_connection_find(&tuple);		
			if(pConn == NULL)
				return RG_FWDENGINE_ALG_RET_FAIL;

			_rtk_rg_sync_tcp_ack_seq(direct, pPktHdr, pConn);

			_rtk_rg_server_in_wan_passive_mode_naptPriority_assign(pPktInfo, pConn);
		}
	}
#endif
	return RG_FWDENGINE_ALG_RET_SUCCESS;
}

#ifdef __KERNEL__
static int __init init(void)
{
	return SUCCESS;
}


static void __exit exit(void)
{
}

module_init(init);
module_exit(exit);

#endif

