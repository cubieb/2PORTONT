#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/skbuff.h>
#endif

#include <rtk_rg_struct.h>
#include <rtk_rg_alg_tool.h>
#include <rtk_rg_rtsp.h>
      

/* rtsp request format
 *    Request = 	Request-Line 
 * 			*( 	general-header 
 *			| 	request-header 
 *			| 	entity-header ) 
 *				CRLF
 *				[ message-body ] 
*/

#define RTSP_RESPOND_IDENTIFIER "RTSP/1.0 "

static int _rtk_rg_rtsp_nextline(unsigned char* p, unsigned int len, unsigned int * pOff, unsigned int * pLineOff, unsigned int * pLineLen) 
{ 
   	unsigned int    off = *pOff; 
   	unsigned int    physlen = 0; 
 
    if (off >= len) 
    { 
        return 0; 
    } 
 
    while (p[off] != '\n') 
    { 
        if (len-off <= 1) 
        { 
            return 0; 
        } 
 
        physlen++; 
        off++; 
    } 
 
    /* if we saw a crlf, physlen needs adjusted */ 
    if (physlen > 0 && p[off] == '\n' && p[off-1] == '\r') 
    { 
        physlen--; 
    } 
 
    /* advance past the newline */ 
    off++; 
 
    *pLineOff = *pOff; 
    *pLineLen = physlen; 
    *pOff = off; 
 
    return 1; 
} 

static int _rtk_rg_rtsp_mime_nextline(unsigned char* p, unsigned int len, unsigned int* pOff, uint* pLineOff, uint* pLineLen) 
{ 
    uint    off = *pOff; 
    uint    physlen = 0; 
    int     isFirstLine = 1; 
 
    if (off >= len) 
    { 
        return 0; 
    } 
 
    do 
    { 
        while (p[off] != '\n') 
        { 
            if (len-off <= 1) 
            { 
                return 0; 
            } 
 
            physlen++; 
            off++; 
        } 
 
        /* if we saw a crlf, physlen needs adjusted */ 
        if (physlen > 0 && p[off] == '\n' && p[off-1] == '\r') 
        { 
            physlen--; 
        } 
 
        /* advance past the newline */ 
        off++; 
 
        /* check for an empty line */ 
        if (physlen == 0) 
        { 
            break; 
        } 
 
        /* check for colon on the first physical line */ 
        if (isFirstLine) 
        { 
            isFirstLine = 0; 
            if (memchr(p+(*pOff), ':', physlen) == NULL) 
            { 
                return 0; 
            } 
        } 
    } 
    while (p[off] == ' ' || p[off] == '\t'); 
 
    *pLineOff = *pOff;
    *pLineLen = (physlen == 0) ? 0 : (off - *pOff); 
    *pOff = off; 
 
    return 1; 
} 

static rtk_rg_alg_expect_t * _rtk_rg_rtsp_expect(int direct, rtk_rg_alg_connection_t * pConn, rtk_rg_alg_newPort_t * pNewPort)
{
	//rtk_rg_alg_tuple_t tuple;
	rtk_rg_alg_expect_t * pExpect = NULL;
	//memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));
#if 1	
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{							
		//rtp or rtcp should use udp
#if 0	//use upnp to add...not direct modify pConn
		pConn->tuple.isTcp = 0;
		if(pConn->tuple.isIp6 == 0)
		{
			pConn->tuple.internalPort = pNewPort->newPort;
			pConn->tuple.extPort = pNewPort->newPort;			//pNewPort->newExtPort;
		}
#endif		
		{
			
			int ret;
			rtk_rg_upnpConnection_t upnpConn;
			rtk_rg_pktHdr_t *pPktHdr;	
			pPktHdr = (rtk_rg_pktHdr_t *)pConn->pPktHdr;
			
			upnpConn.is_tcp= 0; //pConn->tuple.isTcp;
			upnpConn.wan_intf_idx=pPktHdr->netifIdx;
			upnpConn.gateway_port=	pNewPort->newPort;//pConn->tuple.internalPort;			//no change port
			upnpConn.local_port= pNewPort->newPort;//pConn->tuple.internalPort;
			upnpConn.local_ip=pConn->tuple.internalIp.ip;
			upnpConn.limit_remote_ip=0;		//1;remote source may be another server; not DMC
			upnpConn.limit_remote_port=0;		//can't restrict client using which port to connect
			upnpConn.remote_ip=pConn->tuple.remoteIp.ip;
			upnpConn.remote_port=0;
			upnpConn.type=UPNP_TYPE_ONESHOT;
			upnpConn.timeout=rg_db.algUserDefinedTimeout[RTK_RG_ALG_RTSP_TCP]; //auto time out if the client do not connect by this WAN
			
ALG("add upnp intf_idx=%d; upnp remot ip=0x%x port=%d; internal ip=0x%x port=%d\n",upnpConn.wan_intf_idx,pConn->tuple.remoteIp.ip,upnpConn.gateway_port,upnpConn.local_ip,upnpConn.local_port);
			assert_ok((pf.rtk_rg_upnpConnection_add)(&upnpConn,&ret));
		}
		pExpect = _rtk_rg_alg_expect_add(direct, &pConn->tuple, NULL);
	}
#else	
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{							
		//rtp or rtcp should use udp
		tuple.isTcp = 0;
		tuple.isIp6 = pConn->tuple.isIp6;
		if(pConn->tuple.isIp6 == 0)
		{		
			tuple.internalIp.ip = pConn->tuple.internalIp.ip;
			tuple.internalPort = pNewPort->newPort;
			tuple.extIp.ip = pConn->tuple.extIp.ip;
			tuple.extPort = pNewPort->newExtPort;
			tuple.remoteIp.ip = pConn->tuple.remoteIp.ip;	
			pExpect = _rtk_rg_alg_expect_add(direct, &tuple, NULL);								
		}
		
	}
#endif
	return pExpect;
}


static int _rtk_rg_rtsp_parse_message(unsigned char* pData, unsigned int dataLen, unsigned int * pOff, 
                   rtk_rg_rtsp_t * pRtsp) 
{ 
    unsigned int entityLen = 0; 
    unsigned int lineOff, lineLen, contentOff;  
 	unsigned char * end = NULL;
	
    if(!_rtk_rg_rtsp_nextline(pData, dataLen, pOff, &lineOff, &lineLen))
    	return 0;
    
    while(_rtk_rg_rtsp_mime_nextline(pData, dataLen, pOff, &lineOff, &lineLen)) 
	{ 
    	if(lineLen == 0) 
		{
        	if(entityLen > 0) 
            	*pOff += min(entityLen, dataLen - *pOff);            	
            break; 
        } 
        if(lineOff+lineLen > dataLen) { 
            ALG("!! overrun !!\n"); 
            break; 
        } 
 
        if(strnicmp(pData+lineOff, "CSeq:", 5) == 0) { 
            pRtsp->cseqOff = lineOff; 
            pRtsp->cseqLen = lineLen;
            ALG(">>>> cseqOff=%d;cseqLen=%d\n",lineOff,lineLen); 
        } 
 
        if(strnicmp(pData+lineOff, "Transport:", 10) == 0) { 
            pRtsp->transOff = lineOff; 
            pRtsp->transLen = lineLen;
            ALG("<<<< transOff%d;transLen=%d\n",lineOff,lineLen); 
        } 
 
        if(strnicmp(pData+lineOff, "Content-Length:", 15) == 0) { 
            contentOff = lineOff+15; 
            if(rtk_rg_skip_wspace_lws(pData, &contentOff, lineOff+lineLen))
          		entityLen = simple_strtoul(pData+contentOff, (char **)&end, 10);
        } 
    }
 
    return 1; 
} 

static int _rtk_rg_rtsp_parse_transport(unsigned char * pData, rtk_rg_alg_connection_t * pConn,
										rtk_rg_rtsp_t * pRtsp, unsigned int * matchOff, unsigned int * matchLen) 
{
	int 	ret = 0; 
	unsigned int off = pRtsp->transOff, lenOff, nextParamOff, nextFieldOff; 
	const unsigned char * pParamEnd, *pFieldEnd;  
	uint16 port;
	unsigned char * end = NULL;
	unsigned int portLen;
	
	lenOff = pRtsp->transOff+ pRtsp->transLen;
	
	if(pRtsp->transLen < 10  || strnicmp(pData+off, "Transport:", 10) != 0) 
	{ 
		ALG("sanity check failed\n"); 
		return 0; 
	} 
	 
	ALG("Transport: '%.*s'\n", (int)pRtsp->transLen, pData+off); 
	
	off += 10; 
	if(!rtk_rg_skip_wspace_lws(pData, &off, lenOff))
		return 0;
	 
	/* Transport: tran;field;field=val,tran;field;field=val,... */ 
	while(off < lenOff) 
	{  
		pParamEnd = memchr(pData+off, ',', lenOff-off); 
		nextParamOff = (pParamEnd == NULL) ? lenOff : (pParamEnd- pData + 1); 
	 
		while(off < nextParamOff) 
		{ 
			pFieldEnd = memchr(pData+off, ';', nextParamOff-off); 
			nextFieldOff = (pFieldEnd == NULL) ? nextParamOff : (pFieldEnd-pData+1); 
			if(strncmp(pData+off, "client_port=", 12) == 0) 
			{ 
				off += 12; 
				port = simple_strtoul(pData+off, (char **)&end, 10);
				ALG("low port found : %hu\n", port);
				if (port < 1024 || port > 65535)
					return 0;
				
				portLen = end - pData -off;
					
				if(pRtsp->lowPort != 0 && pRtsp->lowPort != port) 
				{
					ALG("multiple ports found, port %hu ignored\n", port); 
				}
				else
				{ 
					ALG("low port found : %hu\n", port);
					pRtsp->lowPort = pRtsp->highPort = port; 
					if(pData[off+portLen] == '-') 
					{  
						port = simple_strtoul(pData+off+portLen+1, (char **)&end, 10); 
						if (port < 1024 || port > 65535)
						{
							return 0; 
						}
						pRtsp->portType = PORT_RANGE;
						pRtsp->highPort = port; 
	 					
						// If we have a range, assume rtp: 
						// loport must be even, hiport must be loport+1 
						if ((pRtsp->lowPort & 0x0001) != 0 || 
							pRtsp->highPort != pRtsp->lowPort+1) 
						{ 
							ALG("incorrect range: %hu-%hu, correcting\n", pRtsp->lowPort, pRtsp->highPort); 
							pRtsp->lowPort &= ~1; 
							pRtsp->highPort = pRtsp->lowPort + 1; 
						}
						*matchLen = end - pData -off;
					} 
					else if (pData[off+portLen] == '/') 
					{ 
						port = simple_strtoul(pData+off+portLen+1, (char **)&end, 10); 
						if (port < 1024 || port > 65535)
							return 0;  
						pRtsp->portType = PORT_DISCON;						
						pRtsp->highPort = port; 
						*matchLen = end - pData -off;
					} 
					else
						*matchLen = portLen;
					
					*matchOff = off;
					ret = 1;
					ALG("****** lowport=%d; highport=%d\n",pRtsp->lowPort,pRtsp->highPort);
				} 
			} 
	 		
			off = nextFieldOff; 
		} 
	 
		off = nextParamOff; 
	} 
	 
	return ret; 

}

static int _rtk_rg_rtsp_mangle_transport(int direct, unsigned char ** ppData,  unsigned int * pDataLen, 
										rtk_rg_alg_connection_t * pConn, rtk_rg_rtsp_t * pRtsp,
										unsigned int  matchOff, unsigned int matchLen)
{ 
	uint16 tempPort = pRtsp->lowPort, rtpExtPort = 0, rtcpExtPort = 0;
	char buffer[sizeof("nnn.nnn.nnn.nnn")];
	unsigned int bufLen = 0, addrOff, addrLen;
	unsigned int off = pRtsp->transOff, lenOff, nextParamOff, nextFieldOff; 
	const unsigned char * pParamEnd, *pFieldEnd;  
	unsigned char * pData = *ppData;
	union rtk_rg_alg_addr addr;
	rtk_rg_alg_newPort_t newPort;
	uint32 newIp;
	
	lenOff = pRtsp->transOff+ pRtsp->transLen;

	//only do port nat modification for outbound packet 
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{	
		switch (pRtsp->portType) 
	    { 
	    case PORT_SINGLE: 
			rtpExtPort = _rtk_rg_extPort_get(FALSE, tempPort);
			if(rtpExtPort == FAIL)
				return 0;
			
			ALG("single: using port %hu\n", rtpExtPort);
	        bufLen = sprintf(buffer, "%hu", rtpExtPort);

			//add expect
			newPort.newPort = tempPort;
			newPort.newExtPort = rtpExtPort;
			_rtk_rg_rtsp_expect(direct, pConn, &newPort);
			
	        break; 
	    case PORT_RANGE: 
			rtpExtPort = _rtk_rg_extPortPair_get(FALSE, tempPort);
			if(rtpExtPort == FAIL)
				return 0;
			
			ALG("range: using ports %hu-%hu\n", rtpExtPort, rtpExtPort+1);
			bufLen = sprintf(buffer, "%hu-%hu", rtpExtPort, rtpExtPort+1); 

			//add expect
			newPort.newPort = tempPort;
			newPort.newExtPort = rtpExtPort;
			_rtk_rg_rtsp_expect(direct, pConn, &newPort);

			//add another expect
			newPort.newPort = tempPort+1;
			newPort.newExtPort = rtpExtPort+1;
			_rtk_rg_rtsp_expect(direct, pConn, &newPort);
			
	        break; 
	    case PORT_DISCON: 
	        rtpExtPort = _rtk_rg_extPort_get(FALSE, tempPort);
			if(rtpExtPort == FAIL)
				return 0;

			rtcpExtPort = _rtk_rg_extPort_get(FALSE, pRtsp->highPort);
			if(rtcpExtPort == FAIL)
				return 0;
			
			ALG("discon: using ports %hu/%hu\n", rtpExtPort, rtcpExtPort); 
			
			if(rtcpExtPort ==  rtpExtPort+1)
				bufLen = sprintf(buffer, "%hu-%hu", rtpExtPort, rtpExtPort+1);
			else
				bufLen = sprintf(buffer, "%hu/%hu", rtpExtPort, rtcpExtPort);

			//add expect
			newPort.newPort = tempPort;
			newPort.newExtPort = rtpExtPort;
			_rtk_rg_rtsp_expect(direct, pConn, &newPort);

			//add another expect
			newPort.newPort = pRtsp->highPort;
			newPort.newExtPort = rtcpExtPort;
			_rtk_rg_rtsp_expect(direct, pConn, &newPort);
			
	        break; 
	    } 
		
		if(!_rtk_rg_mangle_packet(ppData, pDataLen, pConn, matchOff, matchLen, buffer, bufLen))
			return 0;
	}
	
	//only do ipv4 address nat modification
	if(pConn->tuple.isIp6 == 1)
		return 1;
	
	while(off < lenOff) 
	{  
		pParamEnd = memchr(pData+off, ',', lenOff-off); 
		nextParamOff = (pParamEnd == NULL) ? lenOff : (pParamEnd- pData + 1); 
	 	
		while(off < nextParamOff) 
		{ 
			pFieldEnd = memchr(pData+off, ';', nextParamOff-off); 
			nextFieldOff = (pFieldEnd == NULL) ? nextParamOff : (pFieldEnd-pData+1); 
			if(strncmp(pData+off, "destination=", 12) == 0)
			{
				off += 12;
				if (!_rtk_rg_parse_addr(pData+off, &addrLen, &addr, nextParamOff-off, pConn)) 
					return 0;

				ALG("found destination= : len:%d, %8x\n",addrLen,ntohl(addr.ip)); 
				if(direct == NAPT_DIRECTION_OUTBOUND &&
					pConn->tuple.internalIp.ip == ntohl(addr.ip))
				{
					newIp = htonl(pConn->tuple.extIp.ip);							
				}else if(direct == NAPT_DIRECTION_INBOUND &&
					pConn->tuple.extIp.ip == ntohl(addr.ip))
				{
						newIp = htonl(pConn->tuple.internalIp.ip);
				}
				else
					continue;

				addrOff = off;				
				bufLen = sprintf(buffer, "%pI4", &newIp);
				if(!_rtk_rg_mangle_packet(ppData, pDataLen, pConn, addrOff, addrLen, buffer, bufLen))
						return 0;
			}
			off = nextFieldOff; 
		}
		off = nextParamOff; 
	}
	if(addrLen!=bufLen)	//there is delta length
		_rtk_rg_update_tcp_seqNdelta(pConn->direct,(rtk_rg_pktHdr_t *)pConn->pPktHdr, pConn);
 	return 1;
}
static int _rtk_rg_rtsp_reply_process(int direct, unsigned char ** ppData, unsigned int * pDataLen, 
										rtk_rg_alg_connection_t * pConn)
{
	
	unsigned int off = 0, dataLen = *pDataLen;
	unsigned char * pData = *ppData;
    rtk_rg_rtsp_t  rtsp;
	
	if (strncmp(pData, "RTSP/1.0 200 OK", 15) == 0) 	//reply only
	{
		memset(&rtsp, 0, sizeof(rtsp));
		if (!_rtk_rg_rtsp_parse_message(pData, dataLen, &off, &rtsp))
		{
        	return 0;      /* not a valid message */
        }
        if(rtsp.transLen!=0)
        {
        	int i;
	        int j,localip_len;
			char tmpip[16];
			char localip[16];
        	for(i=0;i<rtsp.transLen;i++)
        	{
        		if (strncmp(pData+rtsp.transOff+i, "destination=", 12) == 0)
        		{
        			memset(localip,0,16);
					sprintf(localip,"%u.%u.%u.%u",(pConn->tuple.internalIp.ip>>24)&0xff,(pConn->tuple.internalIp.ip>>16)&0xff,(pConn->tuple.internalIp.ip>>8)&0xff,(pConn->tuple.internalIp.ip)&0xff);
					localip_len=strlen(localip);

        			for(j=7;j<17;j++)	//"rrr.xxx.yyy.zzz" max length is 15
        			{
        				if(*(pData+rtsp.transOff+i+12+j) == 0x3b)	//0x3b = ";"
        				{
        					//int delta_len;
        					int k,tail;
        					memcpy(tmpip,pData+rtsp.transOff+i+12,j);
        					tmpip[j] = 0;
        					if(j==localip_len)	//cool!
        					{
        						ALG("#0 case(%p): oldip=%s; newip=%s\n",pConn,tmpip,localip);
        						memcpy(pData+rtsp.transOff+i+12,localip,localip_len);
        					}
        					else if(j>localip_len)
        					{
        						ALG("#1 case(%p): oldip=%s; newip=%s\n",pConn,tmpip,localip);			
        						tail = *(pDataLen) - rtsp.transOff - i - j - 1;
        						for(k=0;k<tail;k++)	//move data
        							*(pData+rtsp.transOff+i+12+localip_len+1+k)=*(pData+rtsp.transOff+i+12+j+1+k);
        						memcpy(pData+rtsp.transOff+i+12,localip,localip_len);
        						*(pData+rtsp.transOff+i+12+localip_len) = 0x3b;
        						*pDataLen += (localip_len-j);
        						
        					}
        					else //(j<localip_len)
        					{        						
        						ALG("#2 case(%p): oldip=%s; newip=%s\n",pConn,tmpip,localip);
        						tail = *(pDataLen) - rtsp.transOff - i - j -1;
        						for(k=0;k<tail;k++)	//move data
        							*(pData+dataLen+(localip_len-j)-k) = *(pData+dataLen-k);	//move delta length...not one byte only
        						memcpy(pData+rtsp.transOff+i+12,localip,localip_len);
        						*(pData+rtsp.transOff+i+12+localip_len) = 0x3b;
        						*pDataLen += (localip_len-j);
        					}
        					return 1;	//replaced
        				}
        			}
        			return 0;
        		}
        	}
        	return 0;
        }
        return 0;
	}
	return 0;
}


static int _rtk_rg_rtsp_request_process(int direct, unsigned char ** ppData, unsigned int * pDataLen, 
										rtk_rg_alg_connection_t * pConn)
{
	unsigned int cmdoff, off = 0, dataLen = *pDataLen;
	unsigned char * pData = *ppData;
    rtk_rg_rtsp_t  rtsp;           
	unsigned int matchOff=0, matchLen=0;
	
	while (off < dataLen) { 
		cmdoff = off;
		memset(&rtsp, 0, sizeof(rtsp)); 
		
        if (!_rtk_rg_rtsp_parse_message(pData, dataLen, &off, &rtsp)) 
        	return 0; //break;      /* not a valid message */ 

		//printk("Found a message:'%.*s'\n", 8, pData+cmdoff);
		//method name is case-sensitive
        if (strncmp(pData+cmdoff, "SETUP ", 6) != 0) 
        	continue;   /* not a SETUP message */
 
        if(rtsp.transLen) 
		{ 
          	_rtk_rg_rtsp_parse_transport(pData, pConn, &rtsp, &matchOff, &matchLen); 
        } 

		if (rtsp.lowPort == 0) 
		{ 
            ALG("no udp transports found\n"); 
            continue;   /* no udp transports found */ 
        } 
        ALG("udp transport found, ports=(%d,%hu,%hu)\n", (int)rtsp.portType, rtsp.lowPort, rtsp.highPort);
		_rtk_rg_rtsp_mangle_transport(direct, ppData, pDataLen, pConn, &rtsp, matchOff, matchLen);
		
	}
 	return 1;
}

int _rtk_rg_rtsp_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
{
	int ret = 0; //= SUCCESS;
#ifdef __KERNEL__
	unsigned char * pData, *pAppData;
	unsigned int appLen=0,dataOff=0;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_alg_connection_t * pConn;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	pConn = (rtk_rg_alg_connection_t *)pConnInfo;
	skb=(struct sk_buff *)pSkb;
	
	pData=skb->data;
	if(pPktHdr->tagif&TCP_TAGIF)
		dataOff = pPktHdr->l4Offset + pPktHdr->headerLen;
	else
		dataOff = pPktHdr->l4Offset + 8; /*udp header length is 8 bytes*/

#if 1
	appLen = pPktHdr->l3Len + pPktHdr->l3Offset - dataOff;
	ALG(" _rtk_rg_rtsp_handle appLen(%d-%d-%d) = %d\n",pPktHdr->l3Len,pPktHdr->l3Offset,dataOff,appLen);
#else
	appLen = skb->len - dataOff;
#endif
	pAppData = pData + dataOff;	
	
	if (appLen < 0) //may be zero, since KMplayer will send interleave rtsp pkt
		return FAIL;
	
	//do nothing before napt modification
	if(after == 0)
		return SUCCESS;
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{		
		if(pConn->tuple.isIp6 == 0)
		{
			pConn->tuple.extIp.ip = ntohl(*pPktHdr->pIpv4Sip);
			pConn->tuple.extPort = ntohs(*pPktHdr->pSport);
		}
	}

	pConn->skb = pSkb;
	pConn->pPktHdr = pPktInfo;
	pConn->appOff = dataOff;
	pConn->direct = direct;
	//RTSP_RESPOND_IDENTIFIER: RTSP/1.0
	if (strnicmp(pAppData, RTSP_RESPOND_IDENTIFIER, strlen(RTSP_RESPOND_IDENTIFIER)) != 0)
	{
		ret = _rtk_rg_rtsp_request_process(direct, &pAppData, &appLen, pConn);
	}
	else
	{
		if(direct==NAPT_DIRECTION_INBOUND)	//RTSP RESPONSE only
		{
			int len;
			len = appLen;
			ret = _rtk_rg_rtsp_reply_process(direct, &pAppData, &len, pConn);
			if((ret == 1) && (len != appLen))
			{
				ALG("\nlen=%d;appLen=%d\n",len,appLen);
				skb->len += (len - appLen);
				skb->tail += (len - appLen);
				pPktHdr->l3Len += (len - appLen);
				*pPktHdr->pL3Len=htons(pPktHdr->l3Len);
				ALG("before pConn->inDelta=%d\n",pConn->inDelta);
				pConn->inDelta+=(len - appLen);
				_rtk_rg_update_tcp_seqNdelta(pConn->direct, pPktHdr, pConn);
				ALG("after pConn->inDelta=%d\n",pConn->inDelta);

				if(pPktHdr->tagif & PPPOE_TAGIF)
				{
					*pPktHdr->pPppoeLength = htons(ntohs(*pPktHdr->pPppoeLength) + (len - appLen));
				}
			}
		}
	}
	//rtsp reponse message is not necessary to be processed	
#endif	
	return ret;
}

/*According to Linux netfilter nf_conntrack_rtsp.c implementation, 
 * only process the content of header field "Transport" in setup request message
*/
rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_rtsp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
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
				_rtk_rg_alg_init_tuple(direct, 1, pPktHdr, &tuple);
				
				pConn = _rtk_rg_alg_connection_find(&tuple);
				if(pConn == NULL)
				{
					pConn = _rtk_rg_alg_connection_add(&tuple);
				}
				else
				{
					pConn->pPktHdr = pPktInfo;
					_rtk_rg_sync_tcp_ack_seq(direct, pPktHdr, pConn);
				}				
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
				ret = _rtk_rg_rtsp_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
				
			}
			else
			{
				//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
				_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
		
				pConn = _rtk_rg_alg_connection_find(&tuple);		
				if(pConn == NULL)
					return RG_FWDENGINE_ALG_RET_FAIL;
				pConn->pPktHdr = pPktInfo;
				ret = _rtk_rg_rtsp_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
				if(ret == 0)	//while pkt is modified, do not update seq-ack number
				{
					_rtk_rg_sync_tcp_ack_seq(direct, pPktHdr, pConn);
				}
			}
			
		}
	
#endif

	if((pPktHdr->egressTagif&PPPOE_TAGIF) && (after==1))
	{
		struct sk_buff *skb;
		skb= (struct sk_buff *)pSkb;
			// set pppoe length = IP total length + 2
			//skb->data[pPktHdr->l3Offset-4]=htons(ntohs(spip->tot_len)+2)>>8;
			//skb->data[pPktHdr->l3Offset-3]=htons(ntohs(spip->tot_len)+2)&0xff;

			//re-cal l3 checksum
			*pPktHdr->pIpv4Checksum=0;
			*pPktHdr->pIpv4Checksum=htons(inet_chksum(skb->data+pPktHdr->l3Offset,pPktHdr->l4Offset-pPktHdr->l3Offset));
		
			//re-cal l4 checksum
			*pPktHdr->pL4Checksum=0;
			*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,pPktHdr->l3Offset+(*pPktHdr->pL3Len)-pPktHdr->l4Offset,
			ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));
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

