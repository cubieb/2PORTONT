/*
 * ALG Testing:	NetMeeting Rev: 3.01（5.1.2600.5512）with Windows XP ServicePack 3
 * Main Trunk: RG Revision:	3110
 * Last Changed Rev: 2599
 *
 *
 * ALG Testing: FTP Testing
 * Main Trunk RG Revision:	3110
 * Last Changed Rev: 2599
 * FileZilla Rev: 0.9.45 beta
 * FileZilla Serv Rev: 3.7.4.1
 * */



#include <rtk_rg_alg_tool.h>

#ifdef __KERNEL__
#include <linux/jhash.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#endif

//#include <rtk_rg_internal.h>

#define ALG_REMOTE_IP 0x1
#define ALG_REMOTE_PORT 0x2
#define ALG_INTERNAL_IP 0x4
#define ALG_INTERNAL_PORT 0x8
#define ALG_EXTERNAL_IP 0x10
#define ALG_EXTERNAL_PORT 0x20

#define EXPECT_AGING_TIME 2
#define BUCKET_OFF 2
#define BUCKET_SIZE 4
#define TCP_NUM 0x6
#define UDP_NUM 0x11

rtk_rg_algDatabase_t alg_db;

int _rtk_rg_follow_continuation(const unsigned char * pData, unsigned int * pOff, unsigned int dataLen)
{
	unsigned int off = *pOff;
	
	/* Walk past newline */
	if (++off >= dataLen)
		return 0;

	/* Skip '\n' in CR LF */
	if (pData[off-1] == '\r' && pData[off] == '\n') {
		if (++off >= dataLen)
			return 0;
	}

	/* Continuation line? */
	if (pData[off] != ' ' && pData[off] != '\t')
		return 0;

	/* skip leading whitespace */
	for (; off < dataLen; off++) {
		if (pData[off] != ' ' && pData[off] != '\t')
			break;
	}
	*pOff = off;
	return 1;
}

//includes LWS continuations
int rtk_rg_skip_wspace_lws(const unsigned char * pData, unsigned int * pOff, unsigned int dataLen)
{
	int ret;
	unsigned int off = *pOff;
	for (; off < dataLen; off++) {
		if (pData[off] == ' ')
			continue;
		if (pData[off] != '\r' && pData[off] != '\n')
			break;
		ret = _rtk_rg_follow_continuation(pData, &off, dataLen);
		if (ret < 0)
			return ret;
	}
	if(off == dataLen)
		return 0;
	*pOff = off;
	return 1;
}

//return the first free even port 
int _rtk_rg_extPortPair_get(int isTcp, uint16 port)
{
	uint16 tmpPort, extPort, extPort2;
	
	tmpPort = port & ~1;
	for (;tmpPort != 0; tmpPort += 2) 
	{
		//found a free ext Port.
#if defined(CONFIG_APOLLO)
		extPort = _rtk_rg_naptExtPortGetAndUse(FALSE,isTcp,tmpPort,NULL,FALSE);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
		extPort = _rtk_rg_naptExtPortGetAndUse(FALSE,isTcp,0,0,tmpPort,NULL,FALSE);
#endif
		if(extPort != FAIL)
		{
#if defined(CONFIG_APOLLO)
			extPort2 = _rtk_rg_naptExtPortGetAndUse(FALSE,isTcp,tmpPort+1,NULL,FALSE);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
			extPort2 = _rtk_rg_naptExtPortGetAndUse(FALSE,isTcp,0,0,tmpPort+1,NULL,FALSE);
#endif
			if(extPort2 != FAIL)
				break;
		}
	}	
	if (tmpPort == 0) 
	{	/* No port available */				
		DEBUG("rtk_rg_alg_tool: out of pair ports\n");
		return FAIL;
	}
	return extPort;
}

//wrapper function
int _rtk_rg_extPort_get(int isTcp, uint16 port)
{	
#if defined(CONFIG_APOLLO)
	return _rtk_rg_naptExtPortGetAndUse(FALSE, isTcp, port,NULL,FALSE);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
	return _rtk_rg_naptExtPortGetAndUse(FALSE,0,0,isTcp, port,NULL,FALSE);
#endif
}

int _rtk_rg_init_tcp_seqNdelta(rtk_rg_pktHdr_t * pPktHdr, rtk_rg_alg_connection_t * pConn)
{
	if(pPktHdr->tagif & UDP_TAGIF)
		return 0;

	pConn->outDelta=0;
	pConn->inDelta=0;
	pConn->oldOutDelta=0;
	pConn->oldInDelta=0;
	pConn->oldOutSeq=0;
	pConn->oldInSeq=0;
	return 1;
}
//return 1 if overflow: assume window size is small than 0xffffff (16M)
static int _rtk_rg_seq_overflow_check(unsigned int bigSeq, unsigned int smallSeq)
{
	if((bigSeq & 0xff000000) == 0xff000000)	//big enough
	{
		if((smallSeq & 0xff000000) == 0)	//small enough
			return 1;
	}
	return 0;
}
int _rtk_rg_update_tcp_seqNdelta(int direct, rtk_rg_pktHdr_t * pPktHdr, rtk_rg_alg_connection_t * pConn)
{
	unsigned int nowSeq;
	unsigned short nowIpid;
	
	if(pPktHdr->tagif & UDP_TAGIF)
		return 0;

	ALG("In Delta: in=%d(%d); out=%d(%d)\n", pConn->oldOutDelta,pConn->outDelta, pConn->oldInDelta,pConn->inDelta);
	nowSeq = *pPktHdr->pTcpSeq;
	nowIpid = *pPktHdr->pIpv4Identification;
	if(direct == NAPT_DIRECTION_OUTBOUND)			//pConn->oldSeq=0 is the 1st pkt
	{
		if(nowSeq > pConn->oldOutSeq || pConn->oldOutSeq==0)
		{
			if(_rtk_rg_seq_overflow_check(nowSeq, pConn->oldOutSeq)==1)	//overflow happens: retransmiton
			{
				pConn->outDelta = pConn->oldOutDelta;
			}
			else
			{
				memcpy(&pConn->oldOutSeq,pPktHdr->pTcpSeq,sizeof(int));
				pConn->oldOutDelta = pConn->outDelta;
			}
		}
		else	//not to modify, back to old value
		{
			if(_rtk_rg_seq_overflow_check(pConn->oldOutSeq, nowSeq)==1)	//overflow happens
			{
				memcpy(&pConn->oldOutSeq,pPktHdr->pTcpSeq,sizeof(int));
				pConn->oldOutDelta = pConn->outDelta;
			}
			else
			{
				pConn->outDelta = pConn->oldOutDelta;
			}
		}
	}
	else
	{
		if(nowSeq > pConn->oldInSeq ||  pConn->oldInSeq==0)	//pConn->oldSeq=0 is the 1st pkt
		{			
			if(_rtk_rg_seq_overflow_check(nowSeq, pConn->oldInSeq)==1)	//reserved overflow happens: retransmiton
			{
				pConn->inDelta = pConn->oldInDelta;
			}
			else
			{
				memcpy(&pConn->oldInSeq,pPktHdr->pTcpSeq,sizeof(int));
				pConn->oldInDelta = pConn->inDelta;
			}
				
		}
		else	//not to modify, back to old value
		{
			if(_rtk_rg_seq_overflow_check(pConn->oldInSeq, nowSeq)==1)	//overflow happens
			{
				memcpy(&pConn->oldInSeq,pPktHdr->pTcpSeq,sizeof(int));
				pConn->oldInDelta = pConn->inDelta;
			}
			else
			{
				pConn->inDelta = pConn->oldInDelta;
			}
		}
		
	}
	ALG("Update Delta: in=%d(%d); out=%d(%d)\n", pConn->oldOutDelta,pConn->outDelta, pConn->oldInDelta,pConn->inDelta);
	return 1;
}

int _rtk_rg_sync_tcp_ack_seq(int direct, rtk_rg_pktHdr_t * pPktHdr, rtk_rg_alg_connection_t * pConn)
{
	if(pPktHdr->tagif & UDP_TAGIF)
		return 0;
	
	
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{
		//If the same connection send SYN packet, we need to reset Delta to 0
		if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0))	
		{	
			_rtk_rg_init_tcp_seqNdelta(pPktHdr,pConn);
		}
		else 
		{
			if(pConn->outDelta != 0)
				*pPktHdr->pTcpSeq=htonl(pPktHdr->tcpSeq + pConn->outDelta);	
			if(pConn->inDelta != 0)
				*pPktHdr->pTcpAck=htonl(pPktHdr->tcpAck - pConn->inDelta);
		}
	}
	else
	{
		//If the same connection send SYN packet, we need to reset Delta to 0
		if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0))	
		{	
			pConn->outDelta=0;
			pConn->inDelta=0;
		}
		else
		{
			if(pConn->inDelta != 0)
				*pPktHdr->pTcpSeq=htonl(pPktHdr->tcpSeq + pConn->inDelta);	
			if(pConn->outDelta != 0)
				*pPktHdr->pTcpAck=htonl(pPktHdr->tcpAck - pConn->outDelta);
		}
	}
	return 1;
}
//only mangle application layer data 
int _rtk_rg_mangle_packet(unsigned char ** ppData, unsigned int * pDataLen, rtk_rg_alg_connection_t * pConn,
				  unsigned int matchOff, unsigned int matchLen,
				  const char *buffer, unsigned int bufLen)
{
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	unsigned char * pData = *ppData;	
	unsigned int dataOff = pConn->appOff;
	int delta = bufLen - matchLen;

	pPktHdr = (rtk_rg_pktHdr_t *)(pConn->pPktHdr);
	skb= (struct sk_buff *)(pConn->skb);
	
	if(delta > (skb->end - skb->tail))
	{
		DEBUG("rtk_rg_alg_tool: not enough skb room\n");
		return 0;
	}

	if(pConn->direct == NAPT_DIRECTION_OUTBOUND)
		pConn->outDelta += delta;
	else
		pConn->inDelta += delta;
	
	//DEBUG("%s: outDelta: %d, inDelta: %d, '%.*s' => '%.*s'\n",pConn->direct? "inbound":"outbound",
	//			pConn->outDelta,pConn->inDelta,matchLen,pData + matchOff, bufLen,buffer);
	
	/* move post-replacement */
	memmove(pData + matchOff + bufLen,
		pData + matchOff + matchLen,
		skb->len - (dataOff + matchOff + matchLen));

	/* insert data from buffer */
	memcpy(pData + matchOff, buffer, bufLen);
	
	*pDataLen += delta;
	skb->len += delta;
	skb->tail += delta;
	pPktHdr->l3Len += delta;
	*pPktHdr->pL3Len = htons(pPktHdr->l3Len);

	if(pPktHdr->tagif & UDP_TAGIF)
	{
		pPktHdr->l4Len += delta;
		*pPktHdr->pL4Len = htons(pPktHdr->l4Len);
	}
	if(pPktHdr->tagif & PPPOE_TAGIF)
	{
		*pPktHdr->pPppoeLength = htons(ntohs(*pPktHdr->pPppoeLength) + delta);
	}

	return 1;
}

int _rtk_rg_parse_addr(const char *cp,
                      unsigned int * size, union rtk_rg_alg_addr * addr,
                      int dataLen, rtk_rg_alg_connection_t * pConn)
{
	const char *end;
	int ret = 0;
	memset(addr, 0, sizeof(*addr));
#ifdef __KERNEL__
	//pton get a network byte order ip address
	if(pConn->tuple.isIp6 == 0)
		ret = in4_pton(cp, dataLen, (u8 *)&addr->ip, -1, &end);	
	else
		ret = in6_pton(cp, dataLen, (u8 *)&addr->ip6, -1, &end);
#endif	
	if (ret == 0 || end == cp)
		return 0;
	if (size)
		*size = end - cp;
	return 1;
}


//equal return 1, not equal return 0
int rtk_rg_alg_addr_cmp(const union rtk_rg_alg_addr *a1, 
						const union rtk_rg_alg_addr *a2)
#if 1
{
	if(a1->ip == a2->ip)	return 1;
	else if(a1->all[0] == a2->all[0] &&
		a1->all[1] == a2->all[1] &&
		a1->all[2] == a2->all[2] &&
		a1->all[3] == a2->all[3])
		return 1;
	else return 0;
}
#else
{
	return a1->all[0] == a2->all[0] &&
	       a1->all[1] == a2->all[1] &&
	       a1->all[2] == a2->all[2] &&
	       a1->all[3] == a2->all[3];
}
#endif
unsigned int _rtk_rg_alg_expect_hash(int isTcp, union rtk_rg_alg_addr * pDestIp, uint16 destPort)
{
	unsigned int hash = 0;	
#ifdef __KERNEL__
	//DEBUG("expect_hash : key(%08x, %04x, %d)\n", pDestIp->ip, destPort,isTcp ? TCP_NUM : UDP_NUM);
	hash = jhash2(pDestIp->all, ARRAY_SIZE(pDestIp->all),
		      ((destPort  << 16) |  (isTcp ? TCP_NUM : UDP_NUM)));
#endif		
	return ((u64)hash * MAX_ALG_EXPECT_HASH_SIZE) >> 32 >> BUCKET_OFF;
}

rtk_rg_alg_expect_t * _rtk_rg_alg_expect_match(int index, rtk_rg_alg_tuple_t * pTuple)
{
	rtk_rg_alg_expect_t * pEntry= &alg_db.algExpect[index];

	if(pEntry->valid == 1 && pEntry->tuple.isTcp == pTuple->isTcp)
	{
		if((pEntry->mask & ALG_INTERNAL_IP) && (pEntry->mask & ALG_EXTERNAL_IP))
		{
			if(!rtk_rg_alg_addr_cmp(&pEntry->tuple.internalIp, &pTuple->internalIp) && !rtk_rg_alg_addr_cmp(&pEntry->tuple.extIp, &pTuple->extIp))
				return NULL;
		}
		else
		{
			if((pEntry->mask & ALG_INTERNAL_IP)  && !rtk_rg_alg_addr_cmp(&pEntry->tuple.internalIp, &pTuple->internalIp))
				return NULL;
			if((pEntry->mask & ALG_EXTERNAL_IP) && !rtk_rg_alg_addr_cmp(&pEntry->tuple.extIp , &pTuple->extIp))
				return NULL;
		}
		
		if((pEntry->mask & ALG_INTERNAL_PORT) && (pEntry->mask & ALG_EXTERNAL_PORT))
		{
			if((pEntry->tuple.internalPort != pTuple->internalPort) && (pEntry->tuple.extPort != pTuple->extPort))
				return NULL;
		}
		else
		{
			if((pEntry->mask & ALG_INTERNAL_PORT) && (pEntry->tuple.internalPort != pTuple->internalPort))
				return NULL;
			if((pEntry->mask & ALG_EXTERNAL_PORT) && (pEntry->tuple.extPort != pTuple->extPort))
				return NULL;
		}
		if((pEntry->mask & ALG_REMOTE_IP) && !rtk_rg_alg_addr_cmp(&pEntry->tuple.remoteIp, &pTuple->remoteIp))
			return NULL;
		if((pEntry->mask & ALG_REMOTE_PORT) && (pEntry->tuple.remotePort != pTuple->remotePort))
			return NULL;
	}
	else
		return NULL;
	
	return pEntry;
}

int _rtk_rg_alg_expect_get_free_node(rtk_rg_alg_expect_list_node_t ** ppNode)
{
	if(alg_db.pAlgExpectFreeListHead != NULL);
	{
		*ppNode = alg_db.pAlgExpectFreeListHead;
		alg_db.pAlgExpectFreeListHead = alg_db.pAlgExpectFreeListHead->pNext;
		(*ppNode)->pNext = NULL;
		return 1;
	}
	return 0;
}

rtk_rg_alg_expect_t * _rtk_rg_alg_expect_find(int direct, rtk_rg_alg_tuple_t * pTuple)
{
	rtk_rg_alg_expect_t * pEntry=NULL;
	rtk_rg_alg_expect_list_node_t * pNode = NULL;
	unsigned int hash;
	int index, i;
	
	if(direct == NAPT_DIRECTION_OUTBOUND)
		hash = _rtk_rg_alg_expect_hash(pTuple->isTcp, &pTuple->remoteIp, pTuple->remotePort);
	else
		hash = _rtk_rg_alg_expect_hash(pTuple->isTcp, &pTuple->extIp, pTuple->extPort);

	index =  hash << BUCKET_OFF;

	//DEBUG("expect_find: start hash index %d\n", index);
	for (i= index ; i < index + BUCKET_SIZE; i++)
	{		
		if((pEntry =_rtk_rg_alg_expect_match(i, pTuple)) != NULL)
			break;
	}
	
	if(i == index + BUCKET_SIZE)		
	{
		pNode = alg_db.pAlgExpectHashList[hash];
		
		while(pNode != NULL)
		{
			if((pEntry =_rtk_rg_alg_expect_match(pNode->index, pTuple)) !=  NULL)
				break;
			pNode = pNode->pNext;
		}		
	}
	
	return pEntry;
}

void _rtk_rg_alg_expect_del(rtk_rg_alg_expect_t * pExpect)
{
	rtk_rg_alg_expect_list_node_t * pNode = NULL, * pPreNode;
	if(pExpect == NULL)
		return;
	
	pExpect->valid = 0;
	
	if(pExpect->flags & ALG_EXPECT_GET_FROM_LIST)
	{
		pPreNode = pNode = alg_db.pAlgExpectHashList[pExpect->hash];
		while(pNode != NULL)
		{
			if(pExpect->index == pNode->index)
			{	
				if(pNode == pPreNode)
					alg_db.pAlgExpectHashList[pExpect->hash] = pNode->pNext;
				else
					pPreNode->pNext = pNode->pNext;

				pNode->pNext = alg_db.pAlgExpectFreeListHead;
				alg_db.pAlgExpectFreeListHead = pNode;

				//no need to free external port since _rtk_rg_fwdEngine_inbound_fillNaptInfo()
				//will call _rtk_rg_naptExtPortGetAndUse again to use the ext port by force
				// and the free of this external port can be done by napt connection control
				//_rtk_rg_naptExtPortFree(pExpect->tuple.isTcp, pExpect->tuple.extPort);
				break;
			}
			pPreNode = pNode;
			pNode = pNode->pNext;
		}
	}
}

rtk_rg_alg_expect_t * _rtk_rg_alg_expect_replace_or_get(int direct, rtk_rg_alg_tuple_t * pTuple)
{
	unsigned int hash;
	int index, i, oldIndex;
	rtk_rg_alg_expect_t * pExpect = NULL;
	rtk_rg_alg_expect_list_node_t * pNode = NULL;
	
	if(direct == NAPT_DIRECTION_OUTBOUND)
		hash = _rtk_rg_alg_expect_hash(pTuple->isTcp, &pTuple->remoteIp, pTuple->remotePort);
	else
		hash = _rtk_rg_alg_expect_hash(pTuple->isTcp, &pTuple->extIp, pTuple->extPort);

	oldIndex = index =  hash << BUCKET_OFF;
	
	//find a free entry or get a entry living longest time
	for (i= index ; i < index + BUCKET_SIZE; i++)
	{		
		if(alg_db.algExpect[i].valid == 0)
			break;
		if(alg_db.algExpect[i].expire < alg_db.algExpect[oldIndex].expire)
			oldIndex = i;
	}

	if(i < index + BUCKET_SIZE)
	{
		pExpect =  &alg_db.algExpect[i];
		memset(pExpect, 0, sizeof(rtk_rg_alg_expect_t));
	}
	else if(time_after(jiffies,alg_db.algExpect[oldIndex].expire))
	{
		pExpect = &alg_db.algExpect[oldIndex];
		memset(pExpect, 0, sizeof(rtk_rg_alg_expect_t));
	}
	else
	{
		//search in hash list
		if(!_rtk_rg_alg_expect_get_free_node(&pNode))
			return NULL;
		
		if(alg_db.pAlgExpectHashList[hash] == NULL)
		{
			alg_db.pAlgExpectHashList[hash] = pNode;
		}
		else
		{
			pNode->pNext = alg_db.pAlgExpectHashList[hash];
			alg_db.pAlgExpectHashList[hash] = pNode;
		}
		pExpect = &alg_db.algExpect[pNode->index];
		memset(pExpect, 0, sizeof(rtk_rg_alg_expect_t));
		pExpect->hash = hash;
		pExpect->index = pNode->index;
		pExpect->flags |= ALG_EXPECT_GET_FROM_LIST;

	}
	
	return pExpect;
}

int _rtk_rg_alg_init_tuple(int direct, int after, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_alg_tuple_t * pTuple)
{
	if(pPktHdr->tagif & IPV6_TAGIF)
		pTuple->isIp6 = 1;
	if(pPktHdr->tagif & TCP_TAGIF)
		pTuple->isTcp = 1;

	if(after == 0)
	{
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			if(pTuple->isIp6 ==0)
			{
				pTuple->internalIp.ip = pPktHdr->ipv4Sip;
				pTuple->remoteIp.ip = pPktHdr->ipv4Dip;				
			}
			else
			{
				memcpy(&pTuple->internalIp, pPktHdr->pIpv6Sip, sizeof(union rtk_rg_alg_addr));					
				memcpy(&pTuple->remoteIp, pPktHdr->pIpv6Dip, sizeof(union rtk_rg_alg_addr));			
			}
			
			pTuple->internalPort = pPktHdr->sport;
			pTuple->remotePort = pPktHdr->dport;	
		}
		else
		{
			if(pTuple->isIp6 ==0)
			{
				pTuple->extIp.ip = pPktHdr->ipv4Dip;
				pTuple->extPort = pPktHdr->dport;
				pTuple->remoteIp.ip = pPktHdr->ipv4Sip;
				pTuple->remotePort = pPktHdr->sport;
			}
			else
			{
				memcpy(&pTuple->extIp, pPktHdr->pIpv6Dip, sizeof(union rtk_rg_alg_addr));
				//ipv6 does not support nat ?
				pTuple->internalPort = pPktHdr->dport;		
				memcpy(&pTuple->remoteIp, pPktHdr->pIpv6Sip, sizeof(union rtk_rg_alg_addr));
				pTuple->remotePort = pPktHdr->sport;
			}	
		}
	}
	else
	{
	
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{		
			if(pTuple->isIp6 ==0)
			{
				pTuple->internalIp.ip = pPktHdr->ipv4Sip;
				pTuple->remoteIp.ip = pPktHdr->ipv4Dip;				
			}
			else
			{
				memcpy(&pTuple->internalIp, pPktHdr->pIpv6Sip, sizeof(union rtk_rg_alg_addr));					
				memcpy(&pTuple->remoteIp, pPktHdr->pIpv6Dip, sizeof(union rtk_rg_alg_addr));			
			}
			
			pTuple->internalPort = pPktHdr->sport;
			pTuple->remotePort = pPktHdr->dport;			
		}
		else
		{
			if(pTuple->isIp6 ==0)
			{
				//need to get external ip and port
				pTuple->internalIp.ip = ntohl(*pPktHdr->pIpv4Dip);
				pTuple->internalPort = ntohs(*pPktHdr->pDport);
				pTuple->remoteIp.ip = pPktHdr->ipv4Sip;
				pTuple->remotePort = pPktHdr->sport;
			}
			else
			{
				memcpy(&pTuple->internalIp, pPktHdr->pIpv6Dip, sizeof(union rtk_rg_alg_addr));
				//ipv6 does not support nat ?
				pTuple->internalPort = pPktHdr->dport;		
				memcpy(&pTuple->remoteIp, pPktHdr->pIpv6Sip, sizeof(union rtk_rg_alg_addr));
				pTuple->remotePort = pPktHdr->sport;
			}	
		}
	}
	return 0;
}

//for the first inbound packet which is not found in napt and naptr table
//get the internal ip/port if it matches an expect entry
int rtk_rg_fwdEngine_connection_inbound_check(void *data,ipaddr_t *transIP,uint16 *transPort)
{
	rtk_rg_alg_expect_t * pExpect=NULL;
	rtk_rg_pktHdr_t *pPktHdr=(rtk_rg_pktHdr_t *)data;
	rtk_rg_alg_tuple_t tuple;
	
	memset(&tuple, 0 , sizeof(rtk_rg_alg_tuple_t));
	
	_rtk_rg_alg_init_tuple(NAPT_DIRECTION_INBOUND,0, pPktHdr, &tuple);

	//only deal with inbound ipv4 address
	if(tuple.isIp6 == 1)
		return -1;
	
	pExpect = _rtk_rg_alg_expect_find(NAPT_DIRECTION_INBOUND, &tuple);

	if(pExpect == NULL)
	{
		DEBUG("rtk_rg_alg_tool: not find inbound expect src %08x,%04x dest %08x,%04x\n",
			tuple.remoteIp.ip,tuple.remotePort,
			tuple.extIp.ip,tuple.extPort);
		return -1;
	}
	
	*transIP = pExpect->tuple.internalIp.ip;
	*transPort = pExpect->tuple.internalPort;
	//napt information should be added to rg napt/naptr table, so delete this expect entry
	_rtk_rg_alg_expect_del(pExpect);
	return 0;
}

rtk_rg_alg_connection_t *  _rtk_rg_alg_connection_find(rtk_rg_alg_tuple_t * pTuple)
{
	rtk_rg_alg_connection_t * pConn=NULL;
	DEBUG("connection_find: %s internal(%08x,%04x) ext(%08x,%04x) remote(%08x,%04x)\n", 
		    pTuple->isTcp==1?"TCP":"UDP",
			pTuple->internalIp.ip,pTuple->internalPort,
			pTuple->extIp.ip,pTuple->extPort,
			pTuple->remoteIp.ip,pTuple->remotePort);
	
	for (pConn = alg_db.pAlgConnectionListHead->pPrev; pConn != alg_db.pAlgConnectionListHead; pConn = pConn->pPrev)
	{		
		if((pConn->valid == 1) && (pConn->tuple.isTcp==pTuple->isTcp) &&
			rtk_rg_alg_addr_cmp(&pConn->tuple.remoteIp, &pTuple->remoteIp) &&
			(pConn->tuple.remotePort == pTuple->remotePort))
			{
				if(rtk_rg_alg_addr_cmp(&pConn->tuple.internalIp, &pTuple->internalIp) &&
					pConn->tuple.internalPort == pTuple->internalPort)
					break;
				/*
				if(rtk_rg_alg_addr_cmp(&pConn->tuple.extIp, &pTuple->extIp) &&
					pConn->tuple.extPort == pTuple->extPort)
					break;
				*/
			}		 	
	}
	if(pConn == alg_db.pAlgConnectionListHead)		//not found
		return NULL;

	return pConn;
}

rtk_rg_alg_connection_t *  _rtk_rg_alg_connection_add(rtk_rg_alg_tuple_t * pTuple)
{
	rtk_rg_alg_connection_t * pConn=NULL;
	
	DEBUG("connection_add: internal(%08x, %04x) remote(%08x, %04x)\n",
		pTuple->internalIp.ip, pTuple->internalPort, pTuple->remoteIp.ip,pTuple->remotePort);
	pConn = alg_db.pAlgConnectionListHead;
	
	pConn->valid = 1;
	pConn->appHandler = NULL;
	pConn->inDelta = 0;
	pConn->outDelta = 0;
	pConn->appType = ALG_CONNTYPE_START;
	
	memcpy(&pConn->tuple, pTuple, sizeof(rtk_rg_alg_tuple_t));
	
	//move to next one
	alg_db.pAlgConnectionListHead = alg_db.pAlgConnectionListHead->pNext;	
	return pConn;
}


rtk_rg_alg_expect_t * _rtk_rg_alg_expect_add(int direct, rtk_rg_alg_tuple_t * pTuple, int * isNew)
{
	rtk_rg_alg_expect_t * pExpect=NULL;
	int reverseDirect = !direct;
	
	//DEBUG("expect_add: %s  (%08x, %04x) (%08x, %04x) (%08x, %04x)\n", 
	//		pTuple->isTcp? "tcp":"udp", pTuple->internalIp.ip, pTuple->internalPort,
	//		pTuple->extIp.ip, pTuple->extPort,
	//		pTuple->remoteIp.ip, pTuple->remotePort);
	
	pExpect = _rtk_rg_alg_expect_find(reverseDirect, pTuple);
	if(pExpect == NULL)
	{
		pExpect = _rtk_rg_alg_expect_replace_or_get(reverseDirect, pTuple);

		//should not be NULL
		if(pExpect == NULL)
			return NULL;
			
		pExpect->valid = 1;
		pExpect->tuple.isTcp = pTuple->isTcp;
		pExpect->appHandler = NULL;
		
		if(pTuple->internalIp.ip)
		{
			memcpy(&pExpect->tuple.internalIp, &pTuple->internalIp, sizeof(union rtk_rg_alg_addr));
			pExpect->mask |= ALG_INTERNAL_IP;
		}
		if(pTuple->internalPort)
		{
			pExpect->tuple.internalPort = pTuple->internalPort;
			pExpect->mask |= ALG_INTERNAL_PORT;
		}
		if(pTuple->remoteIp.ip)
		{
			memcpy(&pExpect->tuple.remoteIp, &pTuple->remoteIp,sizeof(union rtk_rg_alg_addr));
			pExpect->mask |= ALG_REMOTE_IP;
		}
		if(pTuple->remotePort)
		{
			pExpect->tuple.remotePort = pTuple->remotePort;
			pExpect->mask |= ALG_REMOTE_PORT;
		}
		if(pTuple->extIp.ip)
		{
			memcpy(&pExpect->tuple.extIp, &pTuple->extIp, sizeof(union rtk_rg_alg_addr));
			pExpect->mask |= ALG_EXTERNAL_IP;
		}
		if(pTuple->extPort)
		{
			pExpect->tuple.extPort = pTuple->extPort;
			pExpect->mask |= ALG_EXTERNAL_PORT;
		}
		pExpect->expire = jiffies + EXPECT_AGING_TIME * HZ;
		if(isNew)
			*isNew = 1;
	}
	else
	{
		//refresh timer
		pExpect->expire = jiffies + EXPECT_AGING_TIME * HZ;
		if(isNew)
			*isNew = 0;
	}
	return pExpect;
}

int rtk_rg_alg_expect_forward(int direct, int after, unsigned char *pSkb,unsigned char * pPktInfo)
{
	int ret = 0;
	rtk_rg_pktHdr_t * pPktHdr;
	rtk_rg_alg_expect_t * pExpect=NULL;
	rtk_rg_alg_tuple_t tuple;
	rtk_rg_alg_connection_t * pConn=NULL;
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));

	if(after == 0)
	{
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
		}
		else
		{
			//inbound packet's external ip is lost after napt modification
			//so we need to check the expect before napt modification
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			
			pExpect = _rtk_rg_alg_expect_find(direct, &tuple);
			if(pExpect != NULL && pExpect->appHandler != NULL)
			{
				memcpy(&tuple.internalIp, &pExpect->tuple.internalIp, sizeof(union rtk_rg_alg_addr));
				tuple.internalPort = pExpect->tuple.internalPort;
				pConn = _rtk_rg_alg_connection_find(&tuple);
				if(pConn == NULL)
				{
					pConn = _rtk_rg_alg_connection_add(&tuple);
					pConn->appHandler = pExpect->appHandler;
					_rtk_rg_alg_expect_del(pExpect);
				}
			}
			
		}
	}
	else
	{
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			
			pConn = _rtk_rg_alg_connection_find(&tuple);
			if(pConn == NULL)
			{
				pExpect = _rtk_rg_alg_expect_find(direct, &tuple);
				if(pExpect != NULL && pExpect->appHandler != NULL)
				{
					pConn = _rtk_rg_alg_connection_add(&tuple);	
					pConn->appHandler = pExpect->appHandler;
					_rtk_rg_alg_expect_del(pExpect);
				}
			}
			
			if(pConn != NULL && pConn->appHandler != NULL)
			{
				ret = pConn->appHandler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
			}
		}
		else
		{	
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			
			pConn = _rtk_rg_alg_connection_find(&tuple);		
			
			if(pConn != NULL && pConn->appHandler != NULL)
			{
				ret = pConn->appHandler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
			}
		}
	}
	return ret;
}

void _rtk_rg_alg_resetAllDataBase(void)
{
	int i;

	//Clear all ALG related database
	memset(&alg_db,0,sizeof(rtk_rg_algDatabase_t));
	
	//siyuan init alg connection link list
	alg_db.pAlgConnectionListHead = &alg_db.algConnectionList[0];
	for(i=0; i<MAX_ALG_CONNECTION_SIZE; i++)
	{
		if(i==MAX_ALG_CONNECTION_SIZE-1)
			alg_db.algConnectionList[i].pNext=&alg_db.algConnectionList[0];		//ring-buffer
		else
			alg_db.algConnectionList[i].pNext=&alg_db.algConnectionList[i+1];

		if(i==0)
			alg_db.algConnectionList[i].pPrev=&alg_db.algConnectionList[MAX_ALG_CONNECTION_SIZE-1];
		else
			alg_db.algConnectionList[i].pPrev=&alg_db.algConnectionList[i-1];
	}

	//siyuan init alg expect link list
	alg_db.pAlgExpectFreeListHead = &alg_db.algExpectFreeList[0];
	for(i=MAX_ALG_EXPECT_HASH_SIZE; i< MAX_ALG_EXPECT_SIZE; i++)
	{
		alg_db.algExpectFreeList[i-MAX_ALG_EXPECT_HASH_SIZE].index=i;
		if(i+1 < MAX_ALG_EXPECT_SIZE)
			alg_db.algExpectFreeList[i-MAX_ALG_EXPECT_HASH_SIZE].pNext=&alg_db.algExpectFreeList[i-MAX_ALG_EXPECT_HASH_SIZE+1];
		else
			alg_db.algExpectFreeList[i-MAX_ALG_EXPECT_HASH_SIZE].pNext=NULL;
	}
	for(i=0; i<(MAX_ALG_EXPECT_HASH_SIZE>>2); i++)
		alg_db.pAlgExpectHashList[i] = NULL;
}

int _rtk_rg_alg_displayAllDataBase(struct seq_file *s, void *v)
{
	int wishIdx;
	uint32 wishBitValue;
	int i;
	rtk_rg_alg_connection_t * pConn=NULL;

	rtlglue_printf("<<ALG Connection information:\n");
	for (pConn = alg_db.pAlgConnectionListHead->pPrev; pConn != alg_db.pAlgConnectionListHead; pConn = pConn->pPrev)
	{
		if(pConn->valid)
		{
			rtlglue_printf("%s internal(%08x,%04x) ext(%08x,%04x) remote(%08x,%04x)\n", 
			    pConn->tuple.isTcp==1?"TCP":"UDP or non-TCP",
				pConn->tuple.internalIp.ip,pConn->tuple.internalPort,
				pConn->tuple.extIp.ip,pConn->tuple.extPort,
				pConn->tuple.remoteIp.ip,pConn->tuple.remotePort);

			switch(pConn->appType)
			{
				case ALG_CONNTYPE_H323:
					break;
				case ALG_CONNTYPE_SIP:
					break;
				case ALG_CONNTYPE_PPTP:
					rtlglue_printf("    APP-PPTP:intCallID(%d),extCallID(%d),remCallID(%d)\n",
						pConn->app.pptp.internalCallID,
						pConn->app.pptp.externalCallID,
						pConn->app.pptp.remoteCallID);
					break;
				default:
					break;
			}
		}
	}

	//Display PPTP CallID use bit
	rtlglue_printf("<<PPTP external CallID use bit:\n");
	i=0;
	while(1)
	{
		wishIdx=i>>5; // =wishCallID/32
		wishBitValue=1<<(i&0x1f);
		
		if((alg_db.algPPTPExtCallIDEnabled[wishIdx]&wishBitValue)>0)
			rtlglue_printf("    callID %d is set\n",i);
		
		i++;
		i&=0xffff;
		if(i==0) break;		
	}

	return 0;
}

