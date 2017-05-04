#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter/nf_conntrack_h323_asn1.h>
#endif

#include <rtk_rg_h323.h>
#include <rtk_rg_alg_tool.h>
#include <rtk_rg_internal.h>


int _rtk_rg_ras_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo);

int _rtk_rg_q931_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo);

int _rtk_rg_h245_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo);

int rtk_rg_algRegFunc_TCP_h245(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);

static int _rtk_rg_TPKT_parse(int direct,unsigned char ** ppData, int remainDataLen, 
										int * pDataLen,rtk_rg_alg_connection_t * pConn)
{
	uint16 tpktLen = 0;
	unsigned char * pData = *ppData;

#if 0
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{	
		int i;
		printk("\nTPKT >>>>>\n");
		for(i=0;i<32;i++)
			printk("0x%1x:",*(pData+i));
		printk("\n<<<<<<<<<<<<\n");
	}
#endif
	if((*pData == H323_TPKT_STR) && (pData[1] ==  H323_TPKT_RESERVE) )
	{
		tpktLen = *((unsigned short*)(pData+2));
		// netmeeting will send TPKT header seperately in a tcp fragment packet before Q931 data
		if( remainDataLen == 4)
		{	    
			//save this TPKT data length
		    pConn->app.h323.tpktLen[direct] = tpktLen-4;
			pConn->app.h323.receiveData[direct] =0;
		    return 0;
		}
		else 
		{
			*ppData = pData + 4;
			*pDataLen =tpktLen-4;
			pConn->app.h323.tpktLen[direct] = 0;
			pConn->app.h323.receiveData[direct] =1;
			return 1;
		}
	}
	//check if previous TPTK header is received
	else if(pConn->app.h323.receiveData[direct] == 0 && pConn->app.h323.tpktLen[direct] > 0 ) 
	{	
		*pDataLen = pConn->app.h323.tpktLen[direct];
		pConn->app.h323.tpktLen[direct] = 0;
		pConn->app.h323.receiveData[direct] =1;
		return 1;
	}
	//check if any next?!
	{
		int i;
		uint16 len;
		if((remainDataLen-*pDataLen)>4)	//still next?!
		{
			for(i=0;i<(remainDataLen-*pDataLen);i++)
			{
				if(pData[i] == H323_TPKT_STR && (pData[i+1] ==  H323_TPKT_RESERVE))
				{
					memcpy(&len,pData+i+2,sizeof(short));
					ALG("\n lock len=%d;remainDataLen=%d;pDataLen=%d\n",len,remainDataLen,*pDataLen);
					if(len > 0 && (len<=(remainDataLen-*pDataLen)))					{
						*ppData = pData + i + 4;
						*pDataLen =len-4;
						pConn->app.h323.tpktLen[direct] = 0;
						pConn->app.h323.receiveData[direct] =1;
#if 0
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{	
		pData = *ppData;
		int i;
		printk("\nTPKT ******\n");
		for(i=0;i<32;i++)
			printk("0x%1x:",*(pData+i));
		printk("\n*******\n");
	}
#endif
						return 1;
					}
				}
			}
			//else no next entry...goodbye
		}
	}
	pConn->app.h323.tpktLen[direct] = 0;
	return 0;
}

static int _rtk_rg_expect_t120(int direct, rtk_rg_alg_connection_t * pConn, rtk_rg_alg_newPort_t * pNewPort)
{
	rtk_rg_alg_tuple_t tuple;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));
	
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{							
		//t120 should use udp
		tuple.isTcp = 0;
		tuple.isIp6 = pConn->tuple.isIp6;
		if(pConn->tuple.isIp6 == 0)
		{		
			tuple.internalIp.ip = pConn->tuple.internalIp.ip;
			tuple.internalPort = pNewPort->newPort;
			tuple.extIp.ip = pConn->tuple.extIp.ip;
			tuple.extPort = pNewPort->newExtPort;
			tuple.remoteIp.ip = pConn->tuple.remoteIp.ip;		
		}
		_rtk_rg_alg_expect_add(direct, &tuple, NULL);
	}
	
	return H323_SUCCESS;
}

//rtp and rtcp are udp packets
static int _rtk_rg_expect_rtp_rtcp(int direct, rtk_rg_alg_connection_t * pConn, rtk_rg_alg_newPort_t * pNewPort)
{
//	rtk_rg_alg_tuple_t tuple;
//	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));
	
	
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{							
		//rtp or rtcp should use udp
		pConn->tuple.isTcp = 0;
		if(pConn->tuple.isIp6 == 0)
		{
			pConn->tuple.internalPort = pNewPort->newPort;
			//pConn->tuple.extIp.ip = pConn->tuple.extIp.ip;
			pConn->tuple.extPort = pNewPort->newPort;			//pNewPort->newExtPort;
			//pConn->tuple.remoteIp.ip = pConn->tuple.remoteIp.ip;	
		}
		{
			
			int ret;
			rtk_rg_upnpConnection_t upnpConn;
			rtk_rg_pktHdr_t *pPktHdr;	
			pPktHdr = (rtk_rg_pktHdr_t *)pConn->pPktHdr;
			
			upnpConn.is_tcp=pConn->tuple.isTcp;
			upnpConn.wan_intf_idx=pPktHdr->netifIdx;
			upnpConn.gateway_port=pConn->tuple.internalPort;			//no change port
			upnpConn.local_port=pConn->tuple.internalPort;
			upnpConn.local_ip=pConn->tuple.internalIp.ip;
			upnpConn.limit_remote_ip=1;
			upnpConn.limit_remote_port=0;		//can't restrict client using which port to connect
			upnpConn.remote_ip=pConn->tuple.remoteIp.ip;
			upnpConn.remote_port=0;
			upnpConn.type=UPNP_TYPE_ONESHOT;
			upnpConn.timeout=rg_db.algUserDefinedTimeout[RTK_RG_ALG_H323_TCP]; //auto time out if the client do not connect by this WAN
			
ALG("add upnp intf_idx=%d; upnp remot ip=0x%x port=%d; internal ip=0x%x port=%d",upnpConn.wan_intf_idx,pConn->tuple.remoteIp.ip,upnpConn.gateway_port,upnpConn.local_ip,upnpConn.local_port);
			assert_ok((pf.rtk_rg_upnpConnection_add)(&upnpConn,&ret));
		}
		_rtk_rg_alg_expect_add(direct, &pConn->tuple, NULL);
	}
	
	return H323_SUCCESS;
}

//return host byte order port and ipv4 ip, ipv6 ip keeps the network byte order
static int _rtk_rg_get_h245_address(unsigned char * pData, rtk_rg_alg_connection_t * pConn, H245_TransportAddress *taddr, 
										union rtk_rg_alg_addr * pAddr, uint16 * pPort, int * pOffset)
{
	int offset, len;
	if (taddr->choice != eH245_TransportAddress_unicastAddress)
		return 0;

	switch (taddr->unicastAddress.choice) 
	{
	case eUnicastAddress_iPAddress:
		if(pConn->tuple.isIp6)
			return 0;
		offset= taddr->unicastAddress.iPAddress.network;
		len = 4;
		break;
	case eUnicastAddress_iP6Address:
		if(!pConn->tuple.isIp6)
			return 0;
		offset = taddr->unicastAddress.iP6Address.network;
		len = 16;
		break;
	default:
		return 0;
	}
	if(pConn->tuple.isIp6 == 0)
		pAddr->ip = ntohl(*(uint32*)(&pData[offset]));
	else
		memcpy(pAddr, pData+offset, len);
	
	*pPort = ntohs(*(uint16*)(&pData[offset+len]));
	*pOffset = offset;
	return 1;
}

//return host byte order port and ipv4 ip, ipv6 ip keeps the network byte order
static int _rtk_rg_get_h225_address(unsigned char * pData, rtk_rg_alg_connection_t * pConn, TransportAddress  *taddr,
									union rtk_rg_alg_addr * pAddr, uint16 * pPort, int * pOffset)
{
	int offset, len;
	switch (taddr->choice) 
	{
	case eTransportAddress_ipAddress:
		if(pConn->tuple.isIp6)
			return 0;
		offset = taddr->ipAddress.ip;
		len = 4;
		break;
	case eTransportAddress_ip6Address:
		if(!pConn->tuple.isIp6)
			return 0;
		offset = taddr->ip6Address.ip;
		len = 16;
		break;
	default:
		return 0;
	}
	if(pConn->tuple.isIp6 == 0)
		pAddr->ip = ntohl(*(uint32*)(&pData[offset]));
	else
		memcpy(pAddr, pData+offset, len);
	
	*pPort = ntohs(*(uint16*)(&pData[offset+len]));
#if 1	//
if(*pPort != rg_db.algUserDefinedPort[RTK_RG_ALG_H323_TCP])
{	
	_rtk_rg_algDynamicPort_set(rtk_rg_algRegFunc_TCP_h245,0,0, *pPort,1,rg_db.systemGlobal.tcp_long_timeout);
}
#endif
	*pOffset = offset;
	return 1;
}
static int _rtk_rg_set_address_nocheck(unsigned char * pData, rtk_rg_alg_connection_t * pConn, 
								union rtk_rg_alg_addr * pAddr, uint16 port, int offset)
{
	if(pConn->tuple.isIp6 == 0)
	{
		*(uint32*)(&pData[offset]) = htonl(pAddr->ip);
		*(uint16*)(&pData[offset+4])= htons(port);
	}
	return 1;
}
static int _rtk_rg_set_address(int direct, unsigned char * pData, rtk_rg_alg_connection_t * pConn, 
								union rtk_rg_alg_addr * pAddr, uint16 port, int offset)
{
	if((direct == NAPT_DIRECTION_OUTBOUND) && 
	(rtk_rg_alg_addr_cmp(pAddr,&pConn->tuple.internalIp)) && (port == pConn->tuple.internalPort))
	{	    				
		if(pConn->tuple.isIp6 == 0)
		{
#if 0		//
			uint32 tmpIp;
			uint16 tmpPort;
			tmpIp = htonl(pConn->tuple.extIp.ip);
			tmpPort = htons(pConn->tuple.extPort);
			memcpy(pData+offset,&tmpIp,4);
			memcpy(pData+offset+4,&tmpPort,2);
#else
			*(uint32*)(&pData[offset]) = htonl(pConn->tuple.extIp.ip);
			*(uint16*)(&pData[offset+4])= htons(pConn->tuple.extPort);
#endif
		}
		return 1;				
	}
	else if(direct == NAPT_DIRECTION_INBOUND && 
		rtk_rg_alg_addr_cmp(pAddr, &pConn->tuple.extIp) && port == pConn->tuple.extPort)
	{
		if(pConn->tuple.isIp6 == 0)
		{
#if 0		//
			uint32 tmpIp;
			uint16 tmpPort;
			tmpIp = htonl(pConn->tuple.internalIp.ip);
			tmpPort = htons(pConn->tuple.internalPort);
			memcpy(pData+offset,&tmpIp,4);
			memcpy(pData+offset+4,&tmpPort,2);
#else
			*(uint32*)(&pData[offset]) = htonl(pConn->tuple.internalIp.ip);
			*(uint16*)(&pData[offset+4])= htons(pConn->tuple.internalPort);
#endif
		}
		return 1;
	}
	return 0;
}


static int _rtk_rg_set_ras_addr(int direct, unsigned char * pData, rtk_rg_alg_connection_t * pConn, 
								TransportAddress *taddr, int count)
{
	int i, offset;
	uint16 port;
	union rtk_rg_alg_addr addr;

	for (i = 0; i < count; i++) {
		if(_rtk_rg_get_h225_address(pData, pConn, &taddr[i], &addr, &port, &offset))
	   	{
			if( _rtk_rg_set_address(direct, pData, pConn, &addr, port, offset))
				break;
		}
	}
	return 0;
}

static int _rtk_rg_set_signal_addr(int direct, unsigned char * pData, rtk_rg_alg_connection_t * pConn, 
								TransportAddress *taddr, int count)
{
	int i, offset, tmpOffset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	
	for (i = 0; i < count; i++) 
	{
		if(_rtk_rg_get_h225_address(pData, pConn, &taddr[i], &addr, &port, &offset))
	   	{		
			if(direct == NAPT_DIRECTION_OUTBOUND && rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.internalIp) &&
				port == pConn->app.h323.signal_port[direct])
			{
				/* Fix for Gnomemeeting */
				if (i > 0 && _rtk_rg_get_h225_address(pData, pConn, &taddr[0],
							  &addr, &port, &tmpOffset) &&
					(ntohl(addr.ip) & 0xff000000) == 0x7f000000)
				{
					offset = tmpOffset;
				}
				_rtk_rg_set_address_nocheck(pData, pConn, &pConn->tuple.extIp, pConn->app.h323.signal_port[!direct], offset);
				break;
			}
			else if(direct == NAPT_DIRECTION_INBOUND && rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.extIp) &&
					port == pConn->app.h323.signal_port[direct])
			{
				_rtk_rg_set_address_nocheck(pData, pConn, &pConn->tuple.internalIp, pConn->app.h323.signal_port[!direct], offset);
			}
		}
	}
	
	return 0;
}
static int _rtk_rg_rtp_rtcp_process(int direct, unsigned char * pData, rtk_rg_alg_connection_t * pConn, 
									H245_TransportAddress *taddr)
{
	int offset = 0;
	uint16 port;
	union rtk_rg_alg_addr addr;
	rtk_rg_alg_newPort_t newPort;
	uint16 rtpPort, rtcpPort, rtpExtPort = 0,rtcpExtPort = 0;
	if(!_rtk_rg_get_h245_address(pData, pConn, taddr, &addr, &port, &offset)) 
	{		
		return 0;
	}
	memset(&newPort, 0 , sizeof(rtk_rg_alg_newPort_t));
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{
		if(rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.internalIp) == 0 || port == 0)
		{
			return 0;
		}
		if(port != pConn->tuple.internalPort )
		{
#if 0
//rtp-rtcp format is ip-port-0x00-ip-port
			//do not change rtp/rtcp port
			rtpPort = port;
			rtpExtPort = port;
			if(pConn->tuple.isIp6 == 0)
			{
				int tmpIP;
				tmpIP = htonl(pConn->tuple.extIp.ip);
				*(uint32*)(&pData[offset]) = tmpIP;	//rtp ip changed
				//*(uint16*)(&pData[offset+4])= htons(rtpExtPort);	//do not change rtp port			
				memcpy(&pData[offset+7],(void *)&tmpIP,4);	//change rtcp ip				
				memcpy(&rtcpPort,&pData[offset+7+4],2);	//get rtcp port				
				rtcpExtPort	= rtcpPort;
				
ALG("_rtk_rg_rtp_rtcp_process get rtp ip/port=%x/%d; rtcp ip/port=%x:%d\n",tmpIP,rtcpPort);
			}
#else			
			if((port%2)==0)	/* RTP port is even */
			{
				rtpPort = port;
				rtcpPort = port + 1;		
				rtpExtPort = _rtk_rg_extPortPair_get(FALSE, port);
				if(rtpExtPort == FAIL)
				{
					return -1;
				}
				rtcpExtPort = rtpExtPort+1;
				if(pConn->tuple.isIp6 == 0)
				{
					int tmpIP;
					tmpIP = htonl(pConn->tuple.extIp.ip);
					*(uint32*)(&pData[offset]) = tmpIP;	//rtp ip changed
					*(uint16*)(&pData[offset+4])= htons(rtpExtPort);	//rtp is changed
					//open rtp
					newPort.newPort = rtpPort;
					newPort.newExtPort = rtpExtPort;
					_rtk_rg_expect_rtp_rtcp(direct, pConn, &newPort);
								
					memcpy(&pData[offset+7],(void *)&tmpIP,4);	//change rtcp ip				
					memcpy(&pData[offset+7+4],&rtcpExtPort,2);	//get rtcp ext port
					//open rtcp
					newPort.newPort = rtcpPort;
					newPort.newExtPort = rtcpExtPort;
					_rtk_rg_expect_rtp_rtcp(direct, pConn, &newPort);
				}
			}
			else	/* only RTCP ...this should be odd*/
			{
				//since re-assign port, need to modify port also
				rtcpExtPort = _rtk_rg_extPort_get(FALSE, port);
				if(pConn->tuple.isIp6 == 0)
				{
					*(uint32*)(&pData[offset]) = htonl(pConn->tuple.extIp.ip);
					*(uint16*)(&pData[offset+4])= htons(rtcpExtPort);	//rtp is changed
				}
			}
			
#endif		
		}
		else
		{
			//since re-assign port, need to modify port also
			rtcpExtPort = _rtk_rg_extPort_get(FALSE, port);
			if(pConn->tuple.isIp6 == 0)
			{
				*(uint32*)(&pData[offset]) = htonl(pConn->tuple.extIp.ip);
				*(uint16*)(&pData[offset+4])= htons(rtcpExtPort);	//rtp is changed
			}
		}
	}
	
	return 0;
}

static int _rtk_rg_t120_process(int direct, unsigned char * pData, rtk_rg_alg_connection_t * pConn, 
								H245_TransportAddress *taddr)
{
	int offset = 0;
	uint16 port;
	union rtk_rg_alg_addr addr;
	uint16 extPort;
	rtk_rg_alg_newPort_t newPort;
		
	if(!_rtk_rg_get_h245_address(pData, pConn, taddr, &addr, &port, &offset)) 
	{
		return 0;
	}
	memset(&newPort, 0 , sizeof(rtk_rg_alg_newPort_t));
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{
		if(rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.internalIp) == 0 || port == 0)
			return 0;
					
		if(port != pConn->tuple.internalPort) 
		{		
			extPort=_rtk_rg_extPort_get(FALSE,port);
			if(extPort==FAIL) return 0;	

			if(pConn->tuple.isIp6 == 0)
			{
				*(uint32*)(&pData[offset]) = htonl(pConn->tuple.extIp.ip);
				*(uint16*)(&pData[offset+4])= htons(extPort);
			}
			newPort.newPort = port;						
			newPort.newExtPort = extPort;	
					
			_rtk_rg_expect_t120(direct,pConn, &newPort);
		}				
		
	}
	
	return 0;
}

static int _rtk_rg_h245_channel_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, 
											H2250LogicalChannelParameters * channel)
{
	int ret;

	if (channel->options & eH2250LogicalChannelParameters_mediaChannel) {
		/* RTP */
		
		ret = _rtk_rg_rtp_rtcp_process(direct, pData, pConn, &channel->mediaChannel);
		if (ret < 0)
			return -1;
	
	}

	
	if (channel->
 options & eH2250LogicalChannelParameters_mediaControlChannel) 
		{
		/* RTCP */
		ret = _rtk_rg_rtp_rtcp_process(direct, pData, pConn, &channel->mediaControlChannel);
		if (ret < 0)
			return -1;
	
	}

	
	return 0;
}

//open logical channel process
static int _rtk_rg_h245_olc_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, OpenLogicalChannel *olc)
{	
	int ret;
	if (olc->forwardLogicalChannelParameters.multiplexParameters.choice ==
		eOpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters_h2250LogicalChannelParameters)
	{
		ret = _rtk_rg_h245_channel_process(direct, pData, pConn,
				&olc->forwardLogicalChannelParameters.multiplexParameters.h2250LogicalChannelParameters);
		if (ret < 0)
				return -1;
	}
	
	if ((olc->options & eOpenLogicalChannel_reverseLogicalChannelParameters) &&
			(olc->reverseLogicalChannelParameters.options &
			 eOpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters)
			&& (olc->reverseLogicalChannelParameters.multiplexParameters.choice ==
			eOpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters_h2250LogicalChannelParameters))
	{
		ret = _rtk_rg_h245_channel_process(direct, pData, pConn,
						 &olc->
						 reverseLogicalChannelParameters.
						 multiplexParameters.
						 h2250LogicalChannelParameters);
		if (ret < 0)
				return -1;
	}
	
	if ((olc->options & eOpenLogicalChannel_separateStack) &&
		 olc->forwardLogicalChannelParameters.dataType.choice == eDataType_data &&
		 olc->forwardLogicalChannelParameters.dataType.data.application.choice == 
		 eDataApplicationCapability_application_t120 &&
		 olc->forwardLogicalChannelParameters.dataType.data.application.t120.choice == 
		 eDataProtocolCapability_separateLANStack &&
		 olc->separateStack.networkAddress.choice ==
			eNetworkAccessParameters_networkAddress_localAreaAddress) 
	{
		ret = _rtk_rg_t120_process(direct, pData, pConn,
					  &olc->separateStack.networkAddress.localAreaAddress);
		if (ret < 0)
				return -1;
	}
	
	return H323_SUCCESS;
}

//open logical channel ack process
static int _rtk_rg_h245_olca_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, OpenLogicalChannelAck *olca)
{
	H2250LogicalChannelAckParameters *ack;
	int ret;

	if ((olca->options & eOpenLogicalChannelAck_reverseLogicalChannelParameters) &&

	    (olca->reverseLogicalChannelParameters.options & eOpenLogicalChannelAck_reverseLogicalChannelParameters_multiplexParameters) &&
	    (olca->reverseLogicalChannelParameters.multiplexParameters.choice == eOpenLogicalChannelAck_reverseLogicalChannelParameters_multiplexParameters_h2250LogicalChannelParameters)
	    )
	
	    {
		ret = _rtk_rg_h245_channel_process(direct, pData, pConn, &olca->reverseLogicalChannelParameters.multiplexParameters.h2250LogicalChannelParameters);
	    	if (ret < 0)
	    	return -1;
	
	    }

	
	if ((olca->options & eOpenLogicalChannelAck_forwardMultiplexAckParameters) &&
	    (olca->forwardMultiplexAckParameters.choice == eOpenLogicalChannelAck_forwardMultiplexAckParameters_h2250LogicalChannelAckParameters)
	    )
	
	    {
		
	    	ack = &olca->forwardMultiplexAckParameters.h2250LogicalChannelAckParameters;
			if (ack->options & eH2250LogicalChannelAckParameters_mediaChannel) 
			{
			/* RTP */
			
			ret = _rtk_rg_rtp_rtcp_process(direct, pData, pConn, &ack->mediaChannel);
			if (ret < 0)
				return -1;
		
			}

		
			if (ack->options & eH2250LogicalChannelAckParameters_mediaControlChannel) 
			{
			/* RTCP */
			
			ret = _rtk_rg_rtp_rtcp_process(direct, pData, pConn,&ack->mediaControlChannel);
			if (ret < 0)
				return -1;
			}
		}

	
		if ((olca->options & eOpenLogicalChannelAck_separateStack) &&
			olca->separateStack.networkAddress.choice == eNetworkAccessParameters_networkAddress_localAreaAddress) 
			{
		
				ret = _rtk_rg_t120_process(direct, pData, pConn,&olca->separateStack.networkAddress.localAreaAddress);
				if (ret < 0)
					return -1;
	
			}

	
			return 0;
}

static int _rtk_rg_MultimediaSystemControlMessage_process(int direct, unsigned char * pData, 
									rtk_rg_alg_connection_t * pConn, MultimediaSystemControlMessage *mscm)
{	
	int ret = 0;
	
	switch (mscm->choice) {
	case eMultimediaSystemControlMessage_request:
		
		if (mscm->request.choice == eRequestMessage_openLogicalChannel) 
		{		
			ALG("eRequestMessage_openLogicalChannel\n");
			ret = _rtk_rg_h245_olc_process(direct, pData, pConn, &mscm->request.openLogicalChannel);
		}
		break;
	case eMultimediaSystemControlMessage_response:
		if (mscm->response.choice == eResponseMessage_openLogicalChannelAck) 
		{	
			ALG("eResponseMessage_openLogicalChannelAck\n");
			ret = _rtk_rg_h245_olca_process(direct, pData, pConn, &mscm->response.openLogicalChannelAck);
		}
		break;
	default:
		ret = -1;
		break;
	}
	return ret;
}

static int _rtk_rg_h245_process(int direct,unsigned char * pAppData, int appLen,rtk_rg_alg_connection_t * pConn)
{
	static MultimediaSystemControlMessage mscm;
	int ret,remainDataLen, h245DataLen;
	unsigned char  * pData = pAppData;
	unsigned char  * pDataEnd = pAppData + appLen;
	
	while(((remainDataLen = pDataEnd- pData) >=4) && _rtk_rg_TPKT_parse(direct, &pData, remainDataLen, &h245DataLen, pConn))
	{
		ret = DecodeMultimediaSystemControlMessage(pData, h245DataLen, &mscm);
		if (ret < 0) 
		{
			ALG("rtk_rg_h323: h245 decoding error: %s\n", ret == H323_ERROR_BOUND ?
						 "out of bound" : "out of range");
			break;
		}
		ret = _rtk_rg_MultimediaSystemControlMessage_process(direct, pData, pConn, &mscm);

		pData += h245DataLen;
	}
	return H323_SUCCESS;
}

int _rtk_rg_h245_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
{
	int ret = SUCCESS;
#ifdef __KERNEL__
	char *pData, *pAppData;
	int appLen=0,dataOff=0;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_alg_connection_t * pConn;
	
	
	if(pSkb == NULL || pPktInfo == NULL)
		return -1;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	pConn = (rtk_rg_alg_connection_t *)pConnInfo;
	skb= (struct sk_buff *)pSkb;
	
	pData=skb->data;	
	if(pPktHdr->tagif&TCP_TAGIF)
		dataOff = pPktHdr->l4Offset + pPktHdr->headerLen;
	else
		dataOff = pPktHdr->l4Offset + 8; /*udp header length is 8 bytes*/	
#if 1
	appLen = pPktHdr->l3Len + pPktHdr->l3Offset - dataOff;
	ALG("_rtk_rg_h245_handler appLen(%d-%d-%d) = %d\n",pPktHdr->l3Len,pPktHdr->l3Offset,dataOff,appLen);
#else
	appLen = skb->len - dataOff;
#endif
	pAppData = pData + dataOff;
	//not valid H245 packet, just return
	if(appLen < 4)
		return FAIL;
	
	if(after == 0)
	{
		//do nothing
	}
	else
	{
		if(direct == NAPT_DIRECTION_OUTBOUND)
		{
			if(pConn->tuple.isIp6 == 0)
			{
				pConn->tuple.extIp.ip = ntohl(*pPktHdr->pIpv4Sip);
				pConn->tuple.extPort = ntohs(*pPktHdr->pSport);
			}			
			ret = _rtk_rg_h245_process(direct,pAppData,appLen,pConn);
		}
		else
		{
			ret = _rtk_rg_h245_process(direct,pAppData,appLen,pConn);	
		}
	}
#endif
	return ret;
}

static rtk_rg_alg_expect_t * _rtk_rg_init_expect(int direct, rtk_rg_alg_connection_t * pConn, rtk_rg_alg_newPort_t * pNewPort)
{
	rtk_rg_alg_tuple_t tuple;
	rtk_rg_alg_expect_t * pEntry=NULL;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));

	tuple.isTcp = pConn->tuple.isTcp;
	tuple.isIp6 = pConn->tuple.isIp6;

	if(direct == NAPT_DIRECTION_OUTBOUND)
	{								
		if(pConn->tuple.isIp6 == 0)
		{			
			tuple.internalIp.ip = pConn->tuple.internalIp.ip;
			tuple.internalPort = pNewPort->newPort;
			tuple.extIp.ip = pConn->tuple.extIp.ip;
			tuple.extPort = pNewPort->newExtPort;
			tuple.remoteIp.ip = pConn->tuple.remoteIp.ip;	
		}
		else
		{	
			//ipv6
			memcpy(&tuple.internalIp, &pConn->tuple.internalIp, sizeof(union rtk_rg_alg_addr));
			tuple.internalPort = pNewPort->newPort;
			memcpy(&tuple.remoteIp, &pConn->tuple.remoteIp, sizeof(union rtk_rg_alg_addr));
		}
		pEntry = _rtk_rg_alg_expect_add(direct, &tuple, NULL);
	}
	else
	{			
		if(pConn->tuple.isIp6 == 0)
		{
			tuple.internalIp.ip = pConn->tuple.internalIp.ip;
			tuple.remoteIp.ip = pConn->tuple.remoteIp.ip;
			tuple.remotePort = pNewPort->newPort;
		}
		else
		{	
			//ipv6
			memcpy(&tuple.internalIp, &pConn->tuple.internalIp, sizeof(union rtk_rg_alg_addr));
			memcpy(&tuple.remoteIp, &pConn->tuple.remoteIp, sizeof(union rtk_rg_alg_addr));
			tuple.remotePort = pNewPort->newPort;
		}
		pEntry = _rtk_rg_alg_expect_add(direct, &tuple, NULL);
	}
	return pEntry;
}

static int _rtk_rg_expect_h245(int direct, rtk_rg_alg_connection_t * pConn, rtk_rg_alg_newPort_t * pNewPort)
{
	rtk_rg_alg_expect_t * pExpect;
	pExpect = _rtk_rg_init_expect(direct, pConn, pNewPort);
	if(pExpect != NULL)
	{
		pExpect->appHandler = _rtk_rg_h245_handler;
	}
	return H323_SUCCESS;
}

int _rtk_rg_ras_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo);

int _rtk_rg_q931_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo);


static int _rtk_rg_expect_ras(int direct, rtk_rg_alg_connection_t * pConn, rtk_rg_alg_newPort_t * pNewPort)
{
	rtk_rg_alg_expect_t * pExpect;
	pExpect = _rtk_rg_init_expect(direct, pConn, pNewPort);
	
	if(pExpect != NULL)
	{
		pExpect->appHandler = _rtk_rg_ras_handler;
	}
	return H323_SUCCESS;
}

static int _rtk_rg_expect_q931(int direct, rtk_rg_alg_connection_t * pConn, rtk_rg_alg_newPort_t * pNewPort)
{
	rtk_rg_alg_expect_t * pExpect;
	pExpect = _rtk_rg_init_expect(direct, pConn, pNewPort);
	if(pExpect != NULL)
	{
		pExpect->appHandler = _rtk_rg_q931_handler;
	}
	return H323_SUCCESS;
}

static int _rtk_rg_h245Address_process(int direct, unsigned char * pData, rtk_rg_alg_connection_t * pConn, union rtk_rg_alg_addr * pAddr, uint16 port, int offset)
{
	uint16  extPort;
	rtk_rg_alg_newPort_t newPort;

	memset(&newPort, 0 , sizeof(rtk_rg_alg_newPort_t));		
	
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{
		if(rtk_rg_alg_addr_cmp(pAddr, &pConn->tuple.internalIp) && port != pConn->tuple.internalPort)
		{				
			if(pConn->tuple.isIp6 == 0)
			{
				extPort=_rtk_rg_extPort_get(pConn->tuple.isTcp, port);
				if(extPort==FAIL) return 0;	

				*(uint32*)(&pData[offset]) = htonl(pConn->tuple.extIp.ip);
				*(uint16*)(&pData[offset+4])= htons(extPort);
				
				newPort.newPort = port;						
				newPort.newExtPort = extPort;
			}
				
			_rtk_rg_expect_h245(direct,pConn, &newPort);
		}				
		
	}
	else
	{
		if (rtk_rg_alg_addr_cmp(pAddr, &pConn->tuple.remoteIp) && (port != pConn->tuple.remotePort))
		{
			newPort.newPort = port;		
			_rtk_rg_expect_h245(direct,pConn, &newPort);
		}
	}
	return 0;

}

static int _rtk_rg_h225_callforwarding(int direct, unsigned char * pData, rtk_rg_alg_connection_t * pConn, union rtk_rg_alg_addr * pAddr, uint16 port, int offset)
{
	//may use a different ip address, so need to create a new napt connection, not support now
	return H323_SUCCESS;
}

static int _rtk_rg_h225_setup_process(int direct, unsigned char * pData, rtk_rg_alg_connection_t * pConn, Setup_UUIE *setup)
{
	int ret, i, offset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	
	ALG("_rtk_rg_h225_setup_process\n");
	if (setup->options & eSetup_UUIE_h245Address) {

		
		if(_rtk_rg_get_h225_address(pData, pConn, &setup->h245Address, &addr, &port, &offset))
	   	{
	   		ret = _rtk_rg_h245Address_process(direct, pData, pConn, &addr, port, offset);
			if (ret < 0)
				return -1;
		}	
	}

	if (setup->options & eSetup_UUIE_destCallSignalAddress)
	{
	   	if(_rtk_rg_get_h225_address(pData, pConn, &setup->destCallSignalAddress, &addr, &port, &offset))
	   	{
			ret = _rtk_rg_set_address(direct, pData, pConn, &addr, port, offset);
			if (ret < 0)
				return -1;
	   	}		
	}

	if (setup->options & eSetup_UUIE_sourceCallSignalAddress) 
	{
		if(_rtk_rg_get_h225_address(pData, pConn, &setup->sourceCallSignalAddress, &addr, &port, &offset))
	   	{
			ret = _rtk_rg_set_address(direct, pData, pConn, &addr, port, offset);
			if (ret < 0)
				return -1;
	   	}
	}

	if (setup->options & eSetup_UUIE_fastStart) {
		for (i = 0; i < setup->fastStart.count; i++) {
			ret = _rtk_rg_h245_olc_process(direct, pData, pConn,
					  &setup->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

static int _rtk_rg_h225_callproceeding_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, CallProceeding_UUIE *callproc)
{
	int ret, i, offset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	if (callproc->options & eCallProceeding_UUIE_h245Address) {
		

		if(_rtk_rg_get_h225_address(pData, pConn, &callproc->h245Address, &addr, &port, &offset))
	   	{
	   		ret = _rtk_rg_h245Address_process(direct, pData, pConn, &addr, port, offset);
			if (ret < 0)
				return -1;
		}	
	}

	if (callproc->options & eCallProceeding_UUIE_fastStart) {
		for (i = 0; i < callproc->fastStart.count; i++) {
			ret = _rtk_rg_h245_olc_process(direct, pData, pConn,
					  &callproc->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

static int _rtk_rg_h225_connect_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, Connect_UUIE *connect)
{
	int ret, i, offset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	
	if (connect->options & eConnect_UUIE_h245Address) {
		
		if(_rtk_rg_get_h225_address(pData, pConn, &connect->h245Address, &addr, &port, &offset))
	   	{
	   		ret = _rtk_rg_h245Address_process(direct, pData, pConn, &addr, port, offset);
			if (ret < 0)
				return -1;
		}	
	}

	if (connect->options & eConnect_UUIE_fastStart) {
		for (i = 0; i < connect->fastStart.count; i++) {
			ret = _rtk_rg_h245_olc_process(direct, pData, pConn,
					  &connect->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}

	}

	return 0;
}

static int _rtk_rg_h225_alerting_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, Alerting_UUIE *alert)
{
	int ret, i, offset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	
	if (alert->options & eAlerting_UUIE_h245Address) {
		

		if(_rtk_rg_get_h225_address(pData, pConn, &alert->h245Address, &addr, &port, &offset))
	   	{
	   		ret = _rtk_rg_h245Address_process(direct, pData, pConn, &addr, port, offset);
			if (ret < 0)
				return -1;
		}
	}

	if (alert->options & eAlerting_UUIE_fastStart) {
		for (i = 0; i < alert->fastStart.count; i++) {
			ret = _rtk_rg_h245_olc_process(direct, pData, pConn,
					  &alert->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

static int _rtk_rg_h225_facility_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, Facility_UUIE *facility)
{
	int ret, i, offset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	
	if ((facility->reason.choice == eFacilityReason_callForwarded) &&
		(facility->options & eFacility_UUIE_alternativeAddress))
	{
		if(_rtk_rg_get_h225_address(pData, pConn, &facility->alternativeAddress, &addr, &port, &offset))
		{
			//use another ip:port to open a new h225 connection
			_rtk_rg_h225_callforwarding(direct, pData, pConn, &addr, port, offset);
		}
	}

	

	if (facility->options & eFacility_UUIE_h245Address) {

		if(_rtk_rg_get_h225_address(pData, pConn, &facility->h245Address, &addr, &port, &offset))
	   	{
	   		ret = _rtk_rg_h245Address_process(direct, pData, pConn, &addr, port, offset);
			if (ret < 0)
				return -1;
		}
	}

	if (facility->options & eFacility_UUIE_fastStart) {
		for (i = 0; i < facility->fastStart.count; i++) {
			ret = _rtk_rg_h245_olc_process(direct, pData, pConn,
					  &facility->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

static int _rtk_rg_h225_progress_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, Progress_UUIE *progress)
{
	int ret, i, offset;
	uint16 port;
	union rtk_rg_alg_addr addr;


	if (progress->options & eProgress_UUIE_h245Address) {
		
		
		if(_rtk_rg_get_h225_address(pData, pConn, &progress->h245Address, &addr, &port, &offset))
	   	{
	   		ret = _rtk_rg_h245Address_process(direct, pData, pConn, &addr, port, offset);
			if (ret < 0)
				return -1;
		}
	}

	if (progress->options & eProgress_UUIE_fastStart) {
		for (i = 0; i < progress->fastStart.count; i++) {
			ret = _rtk_rg_h245_olc_process(direct, pData, pConn,
					  &progress->fastStart.item[i]);
			if (ret < 0)
				return -1;
		}
	}

	return 0;
}

static int _rtk_rg_h225_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, Q931 * pQ931)
{
	H323_UU_PDU * pdu = &pQ931->UUIE.h323_uu_pdu;
	int i;
	int ret = 0;


	ALG("_rtk_rg_h225_process choice=%d(%d)\n",pdu->h323_message_body.choice,eH323_UU_PDU_h323_message_body_setup);
	switch (pdu->h323_message_body.choice) {
	
		case eH323_UU_PDU_h323_message_body_setup:
		
		ret = _rtk_rg_h225_setup_process(direct, pData, pConn,&pdu->h323_message_body.setup);
		break;
	
		case eH323_UU_PDU_h323_message_body_callProceeding:
		
		ret = _rtk_rg_h225_callproceeding_process(direct, pData, pConn,&pdu->h323_message_body.callProceeding);
		break;
		case eH323_UU_PDU_h323_message_body_connect:
		
		ret = _rtk_rg_h225_connect_process(direct, pData, pConn,&pdu->h323_message_body.connect);
		break;
	
		case eH323_UU_PDU_h323_message_body_alerting:
		
		ret = _rtk_rg_h225_alerting_process(direct, pData, pConn,&pdu->h323_message_body.alerting);
		
		break;
	
		case eH323_UU_PDU_h323_message_body_facility:
		
		ret = _rtk_rg_h225_facility_process(direct, pData, pConn,&pdu->h323_message_body.facility);
		
		break;
	
		case eH323_UU_PDU_h323_message_body_progress:
		
		ret = _rtk_rg_h225_progress_process(direct, pData, pConn,&pdu->h323_message_body.progress);
		
		break;
	
		default:
		break;
	
	}

	
	if (ret < 0)
		return -1;

	if (pdu->options & eH323_UU_PDU_h245Control) 
	{
		
		for (i = 0; i < pdu->h245Control.count; i++) {
			
			ret = _rtk_rg_MultimediaSystemControlMessage_process(direct, pData, pConn,&pdu->h245Control.item[i]);
			if (ret < 0) return -1;
		}
	
	}
	return 0;	
}

static int _rtk_rg_q931_process(int direct,unsigned char * pAppData, int appLen,rtk_rg_alg_connection_t * pConn)
{
	int ret, remainDataLen, q931DataLen = 0;
	unsigned char  * pData = pAppData;
	unsigned char  * pDataEnd = pAppData + appLen;
	static Q931 q931;
	ret = H323_SUCCESS;//
	
	while(((remainDataLen = pDataEnd- pData) >=4) && _rtk_rg_TPKT_parse(direct, &pData, remainDataLen, &q931DataLen, pConn))
	{
		/* Decode Q.931 signal */
		ret = DecodeQ931(pData, q931DataLen, &q931);
		if (ret < 0) {
			ALG("rtk_rg_h323 : q931 decoding error: %s\n",
				 ret == H323_ERROR_BOUND ?
				 "out of bound" : "out of range");
			ret = H323_FORMAT_ERROR;//
			break;
		}
		_rtk_rg_h225_process(direct, pData, pConn, &q931);
		pData += q931DataLen;
	}
	return ret;	//
}

static int _rtk_rg_ras_grq_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, GatekeeperRequest *grq)
{
	return _rtk_rg_set_ras_addr(direct, pData, pConn, &grq->rasAddress, 1);
}

static int _rtk_rg_ras_gcf_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, GatekeeperConfirm *gcf)
{
	int offset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	rtk_rg_alg_newPort_t newPort;
	
	if (!_rtk_rg_get_h225_address(pData, pConn, &gcf->rasAddress, &addr, &port, &offset))
		return 0;	
	
	//gatekeeper should be in public network, so direct should be NAPT_DIRECTION_INBOUND
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{
		return 0;	
	}
	else if(direct == NAPT_DIRECTION_INBOUND && rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.remoteIp) )
	{	
		/* Registration port is the same as discovery port */
		if(port == pConn->tuple.remotePort)
			return 0;
		
		newPort.newPort = port;
		_rtk_rg_expect_ras(direct, pConn, &newPort);
	}	
	return 1;
}

static int _rtk_rg_ras_rrq_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, RegistrationRequest *rrq)
{
	int i, offset, count;
	TransportAddress *taddr;
	uint16 port, extPort;
	union rtk_rg_alg_addr addr;
	rtk_rg_alg_newPort_t newPort;
	
	taddr = rrq->callSignalAddress.item;
	count = rrq->callSignalAddress.count;
	for(i = 0; i< count; i++)
	{	
		if(_rtk_rg_get_h225_address(pData, pConn, &taddr[i], &addr, &port, &offset))
	   	{
	   		if(direct == NAPT_DIRECTION_OUTBOUND && rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.internalIp) 
				&& port > 0 && port != pConn->tuple.internalPort)
				break;
		}
	}
	if (i >= count)		/* Not found */
		return 0;

	//gatekeeper should be in public network, so direct should be NAPT_DIRECTION_OUTBOUND
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{
		extPort=_rtk_rg_extPort_get(FALSE, port);
		if(extPort==FAIL) return 0;	
				
		newPort.newPort = port;
		newPort.newExtPort = extPort;
		_rtk_rg_expect_q931(direct, pConn, &newPort);
		
		if(_rtk_rg_set_address_nocheck(pData, pConn, &pConn->tuple.extIp, extPort, offset))
		{
			/* Save ports */
			pConn->app.h323.signal_port[direct] = port;
			pConn->app.h323.signal_port[!direct] = extPort;
			
			/* Fix for Gnomemeeting */
			if (i > 0 && _rtk_rg_get_h225_address(pData, pConn, &taddr[0], &addr, &port, &offset) &&
			    (ntohl(addr.ip) & 0xff000000) == 0x7f000000) 
			{
				_rtk_rg_set_address_nocheck(pData, pConn, &pConn->tuple.extIp, extPort, offset);
			}
		}
	}

	return _rtk_rg_set_ras_addr(direct, pData, pConn, rrq->rasAddress.item, rrq->rasAddress.count);
	
}

static int _rtk_rg_ras_rcf_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, RegistrationConfirm *rcf)
{
	return _rtk_rg_set_signal_addr(direct, pData, pConn,
					rcf->callSignalAddress.item, rcf->callSignalAddress.count);	
}

static int _rtk_rg_ras_urq_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, UnregistrationRequest *urq)
{
	 _rtk_rg_set_signal_addr(direct, pData, pConn,urq->callSignalAddress.item,
				   			urq->callSignalAddress.count);
	 //need to remove  expects related to this connection
	 return 1;
}

static int _rtk_rg_ras_arq_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, AdmissionRequest *arq)
{
	int offset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	
	if ((arq->options & eAdmissionRequest_destCallSignalAddress) &&
	    _rtk_rg_get_h225_address(pData, pConn, &arq->destCallSignalAddress, &addr, &port, &offset) &&
	    port == pConn->app.h323.signal_port[direct]) 
	{
		/* Answering ARQ */
		if(direct == NAPT_DIRECTION_OUTBOUND && rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.internalIp))
			return _rtk_rg_set_address_nocheck(pData, pConn, &pConn->tuple.extIp, pConn->app.h323.signal_port[!direct], offset);
		else if(direct == NAPT_DIRECTION_INBOUND && rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.extIp))
			return _rtk_rg_set_address_nocheck(pData, pConn, &pConn->tuple.internalIp, pConn->app.h323.signal_port[!direct], offset);
	}

	if ((arq->options & eAdmissionRequest_srcCallSignalAddress) &&
	    _rtk_rg_get_h225_address(pData, pConn, &arq->srcCallSignalAddress, &addr, &port, &offset)) 
	{
		/* Calling ARQ */
		return _rtk_rg_set_address(direct, pData, pConn, &addr, port, offset);
	}
	return 0;
}

static int _rtk_rg_ras_acf_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, AdmissionConfirm *acf)
{
	int offset;
	uint16 port;
	union rtk_rg_alg_addr addr;
	
	if (!_rtk_rg_get_h225_address(pData, pConn, &acf->destCallSignalAddress,
			   &addr, &port, &offset))
		return 0;
	
	/* Answering ACF */
	if(direct == NAPT_DIRECTION_OUTBOUND && rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.remoteIp))
	{
		return 0;	
	}
	else if(direct == NAPT_DIRECTION_INBOUND && rtk_rg_alg_addr_cmp(&addr, &pConn->tuple.extIp) )
	{	
		return  _rtk_rg_set_signal_addr(direct, pData, pConn, &acf->destCallSignalAddress, 1);
	}

	//new ip address, need to add a expect
	return 0;
}

static int _rtk_rg_ras_result_process(int direct,unsigned char * pData, rtk_rg_alg_connection_t * pConn, RasMessage *ras)
{
	switch (ras->choice) {
	case eRasMessage_gatekeeperRequest:
		return _rtk_rg_ras_grq_process(direct, pData, pConn, &ras->gatekeeperRequest);
	case eRasMessage_gatekeeperConfirm:
		return _rtk_rg_ras_gcf_process(direct, pData, pConn, &ras->gatekeeperConfirm);
	case eRasMessage_registrationRequest:
		return _rtk_rg_ras_rrq_process(direct, pData, pConn, &ras->registrationRequest);
	case eRasMessage_registrationConfirm:
		return _rtk_rg_ras_rcf_process(direct, pData, pConn, &ras->registrationConfirm);
	case eRasMessage_unregistrationRequest:
		return _rtk_rg_ras_urq_process(direct, pData, pConn, &ras->unregistrationRequest);
	case eRasMessage_admissionRequest:
		return _rtk_rg_ras_arq_process(direct, pData, pConn, &ras->admissionRequest);
	case eRasMessage_admissionConfirm:
		return _rtk_rg_ras_acf_process(direct, pData, pConn, &ras->admissionConfirm);
	default:
		break;
	}
	
	return 0;
}


static int _rtk_rg_ras_process(int direct,unsigned char * pAppData, int appLen,rtk_rg_alg_connection_t * pConn)
{
	int ret;
	RasMessage ras;
	unsigned char  * pData = pAppData;
	
	/* Decode RAS message */
	ret = DecodeRasMessage(pAppData, appLen, &ras);
	if (ret < 0) {
		DEBUG("rtk_rg_h323: ras message decoding error: %s\n",
			 ret == H323_ERROR_BOUND ?
			 "out of bound" : "out of range");
		return H323_STOP;
	}
	/* Process RAS message */
	if (_rtk_rg_ras_result_process(direct, pData, pConn, &ras) < 0)
		return H323_FAIL;
	
	return H323_SUCCESS;
}

int _rtk_rg_ras_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
{
	int ret = SUCCESS;
#ifdef __KERNEL__
	unsigned char *pData, *pAppData;
	int appLen=0,dataOff=0;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_alg_connection_t * pConn;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	pConn = (rtk_rg_alg_connection_t *)pConnInfo;
	skb= (struct sk_buff *)pSkb;
	
	pData=skb->data;	
	//udp header length is 8 bytes
	dataOff = pPktHdr->l4Offset + 8;	
#if 1	//
	appLen = pPktHdr->l3Len + pPktHdr->l3Offset - dataOff;
	ALG("_rtk_rg_ras_handler appLen(%d-%d-%d) = %d\n",pPktHdr->l3Len,pPktHdr->l3Offset,dataOff,appLen);
#else
	appLen = skb->len - dataOff;
#endif
	pAppData = pData + dataOff;
	if(after==0)
	{
		//do nothing
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{		
			if(pConn->tuple.isIp6 == 0)
			{
				pConn->tuple.extIp.ip = ntohl(*pPktHdr->pIpv4Sip);
				pConn->tuple.extPort = ntohs(*pPktHdr->pSport);
			}
			ret = _rtk_rg_ras_process(direct, pAppData, appLen, pConn);				
		}
		else
		{
			ret = _rtk_rg_ras_process(direct, pAppData, appLen, pConn);
		}
	}
#endif
	return ret;
}

int _rtk_rg_q931_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
{
	int ret = SUCCESS;
#ifdef __KERNEL__
	unsigned char *pData, *pAppData;
	int appLen=0,dataOff=0;
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
	
#if 1	//
	appLen = pPktHdr->l3Len + pPktHdr->l3Offset - dataOff;
	ALG("_rtk_rg_q931_handler appLen(%d-%d-%d) = %d\n",pPktHdr->l3Len,pPktHdr->l3Offset,dataOff,appLen);
#else
	appLen = skb->len - dataOff;
#endif
	pAppData = pData + dataOff;
	//not valid q931 packet, just return
	if(appLen < 4)
		return H323_FAIL;
	if(after==0)
	{
		//do nothing
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{		
			if(pConn->tuple.isIp6 == 0)
			{
				pConn->tuple.extIp.ip = ntohl(*pPktHdr->pIpv4Sip);
				pConn->tuple.extPort = ntohs(*pPktHdr->pSport);
			}
			ret = _rtk_rg_q931_process(direct, pAppData, appLen, pConn);
		}
		else
		{
			ret = _rtk_rg_q931_process(direct, pAppData, appLen, pConn);
		}
	}
#endif
	return ret;
}

int rtk_rg_algRegFunc_TCP_h245(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
//Attention: caller function needs to make sure it needs to do napt modification
//ipv6 address and port doesn't need to do napt modification now
#ifdef __KERNEL__
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
			pConn->pPktHdr = pPktInfo;			
		}
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{		
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);

			pConn = _rtk_rg_alg_connection_find(&tuple);
			//entrance
			if(pConn == NULL)
				return FAIL;
			
			pConn->pPktHdr = pPktInfo;
			_rtk_rg_h245_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
		}
		else
		{
			//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			pConn = _rtk_rg_alg_connection_find(&tuple);
			
			if(pConn == NULL)
				return FAIL;
			pConn->pPktHdr = pPktInfo;
			_rtk_rg_h245_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
		}

	}
#endif
	if(pPktHdr->egressTagif&PPPOE_TAGIF)
	{
		struct sk_buff *skb;
		skb= (struct sk_buff *)pSkb;
			//re-cal l3 checksum
			*pPktHdr->pIpv4Checksum=0;
			*pPktHdr->pIpv4Checksum=htons(inet_chksum(skb->data+pPktHdr->l3Offset,pPktHdr->l4Offset-pPktHdr->l3Offset));
		
			//re-cal l4 checksum
			*pPktHdr->pL4Checksum=0;
			*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,pPktHdr->l3Offset+(*pPktHdr->pL3Len)-pPktHdr->l4Offset,ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));
	}
  return SUCCESS;
}

//tcp port 1720
rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_TCP_h323(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
//Attention: caller function needs to make sure it needs to do napt modification
//ipv6 address and port doesn't need to do napt modification now
#ifdef __KERNEL__
	int ret=0;
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
			pConn->pPktHdr = pPktInfo;			
		}
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{		
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);

			pConn = _rtk_rg_alg_connection_find(&tuple);
			//entrance
			if(pConn == NULL)
				return RG_FWDENGINE_ALG_RET_FAIL;
			
			pConn->pPktHdr = pPktInfo;
			ret = _rtk_rg_q931_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
			//if(ret == H323_FORMAT_ERROR)	//not q931
				//_rtk_rg_h245_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
		}
		else
		{
			//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			pConn = _rtk_rg_alg_connection_find(&tuple);
			
			if(pConn == NULL)
				return RG_FWDENGINE_ALG_RET_FAIL;
			pConn->pPktHdr = pPktInfo;
			ret = _rtk_rg_q931_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
			//if(ret == H323_FORMAT_ERROR)	//not q931
				//_rtk_rg_h245_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
		}

	}
#endif
	if(pPktHdr->egressTagif&PPPOE_TAGIF)
	{
		struct sk_buff *skb;
		skb= (struct sk_buff *)pSkb;
			//re-cal l3 checksum
			*pPktHdr->pIpv4Checksum=0;
			*pPktHdr->pIpv4Checksum=htons(inet_chksum(skb->data+pPktHdr->l3Offset,pPktHdr->l4Offset-pPktHdr->l3Offset));
		
			//re-cal l4 checksum
			*pPktHdr->pL4Checksum=0;
			*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,pPktHdr->l3Offset+(*pPktHdr->pL3Len)-pPktHdr->l4Offset,ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));
	}
  return RG_FWDENGINE_ALG_RET_SUCCESS;
}

//udp port 1719
rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_UDP_ras(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
//Attention: caller function needs to make sure it needs to do napt modification
//ipv6 address and port doesn't need to do napt modification now
#ifdef __KERNEL__
	int ret=0;
	rtk_rg_pktHdr_t *pPktHdr;
	rtk_rg_alg_connection_t *pConn;
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
			pConn->pPktHdr = pPktInfo;
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
			
			pConn->pPktHdr = pPktInfo;
			ret = _rtk_rg_ras_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);			
		}
		else
		{
			//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			
			pConn = _rtk_rg_alg_connection_find(&tuple);		
			if(pConn == NULL)
				return RG_FWDENGINE_ALG_RET_FAIL;
			
			pConn->pPktHdr = pPktInfo;
			ret = _rtk_rg_ras_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
		}

	}
#endif
	if(pPktHdr->egressTagif&PPPOE_TAGIF)
	{
		struct sk_buff *skb;
		skb= (struct sk_buff *)pSkb;
			//re-cal l3 checksum
			*pPktHdr->pIpv4Checksum=0;
			*pPktHdr->pIpv4Checksum=htons(inet_chksum(skb->data+pPktHdr->l3Offset,pPktHdr->l4Offset-pPktHdr->l3Offset));
		
			//re-cal l4 checksum
			*pPktHdr->pL4Checksum=0;
			*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,pPktHdr->l3Offset+(*pPktHdr->pL3Len)-pPktHdr->l4Offset,ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));
	}
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

