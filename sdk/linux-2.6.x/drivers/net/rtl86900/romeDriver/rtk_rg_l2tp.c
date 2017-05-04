#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/skbuff.h>
#endif

#include <rtk_rg_l2tp.h>
#include <rtk_rg_alg_tool.h>

#define L2TP_FLAG_TYPE  0x80
#define L2TP_FLAG_LEN   0x40
#define L2TP_FLAG_SEQ   0x08
#define L2TP_FLAG_OFF   0x02
#define L2TP_FLAG_PRI   0x01

#define L2TP_CONTROL_SCCRQ 1

#define L2TP_AVP_MESSAGE_TYPE 0

#define L2TP_AVP_ASSIGNED_TUNNELID 9
#define L2TP_AVP_MIN_LENGTH 6

#define L2TP_IS_CONTROL(x)  ((x) & L2TP_FLAG_TYPE)


/*The initiator of an L2TP tunnel picks an available source UDP port (which may or may not be 1701), 
 * and sends to the desired destination address at port 1701.
 * the recipient picks a free port on its own system (which may or may not be 1701)
 * a control connection will create a tunnel
 * a data transport will create a session within a tunnel (use the same connection tuple), no need to add an expect
*/
static rtk_rg_alg_expect_t * _rtk_rg_l2tp_expect(int direct, rtk_rg_alg_connection_t * pConn)
{
	rtk_rg_alg_tuple_t tuple;
	rtk_rg_alg_expect_t * pExpect = NULL;

	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));
	
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{							
		tuple.isTcp = pConn->tuple.isTcp;
		tuple.isIp6 = pConn->tuple.isIp6;
		if(pConn->tuple.isIp6 == 0)
		{		
			tuple.internalIp.ip = pConn->tuple.internalIp.ip;
			tuple.internalPort = pConn->tuple.internalPort;
			tuple.extIp.ip = pConn->tuple.extIp.ip;
			tuple.extPort = pConn->tuple.extPort;
			tuple.remoteIp.ip = pConn->tuple.remoteIp.ip;	
			pExpect = _rtk_rg_alg_expect_add(direct, &tuple, NULL);								
		}
	}
	return pExpect;
}

//check if the message has the assigned tunnel id avp, return 1 if found
static int _rtk_rg_l2tp_parse_avp(unsigned char * pData, rtk_rg_l2tp_header_t * pl2tpHeader, rtk_rg_l2tp_avp_t ** ppAvp)
{
	unsigned int end, off;
	rtk_rg_l2tp_avp_t * avp;
	uint16 hiLength;
	
	//not parse l2tp data message
	if(!(pl2tpHeader->code & L2TP_FLAG_TYPE))
		return 0;

	end = pl2tpHeader->length;
	off = pl2tpHeader->dataOff;

	//the first avp must be control message type AVP, attribute type is 0
	avp = (rtk_rg_l2tp_avp_t *)(&pData[off]);
	if(ntohs(avp->attrType) != L2TP_AVP_MESSAGE_TYPE)
		return 0;
	pl2tpHeader->controlType = ntohs(*(uint16 *)(avp->attrValue));
	off += avp->length;
	
	while(off < end)
	{
		avp = (rtk_rg_l2tp_avp_t *)(&pData[off]);
		if(avp->length < L2TP_AVP_MIN_LENGTH)
			return 0;
		
		if(ntohs(avp->attrType) == L2TP_AVP_ASSIGNED_TUNNELID)
		{
			*ppAvp = avp;
			break;
		}
		/* the length field is actually 10 bits */
		hiLength = (avp->mask & 0x03) << 8;
		off += hiLength + avp->length;
	}
	if(off < end)
		return 1;
	return 0;
}

static int _rtk_rg_l2tp_parse_header(unsigned char * pData, unsigned int dataLen, rtk_rg_l2tp_header_t * pl2tpHeader)
{
	unsigned int off = 0;
	uint8 code = *pData;

	memset(pl2tpHeader, 0, sizeof(*pl2tpHeader));
	
	pl2tpHeader->code = pData[off++];
	pl2tpHeader->version= pData[off++];
	
	if(code & L2TP_FLAG_LEN)
		pl2tpHeader->length = *(uint16*)(&pData[off]);
	off +=2;
	
	pl2tpHeader->tunnelId = ntohs(*(uint16*)(&pData[off]));
	off +=2;
	pl2tpHeader->sessionId = ntohs(*(uint16*)(&pData[off]));
	off +=2;

	if(off >= dataLen)
		return 0;
	
	if(code & L2TP_FLAG_SEQ)
	{
		pl2tpHeader->Ns = *(uint16*)(&pData[off]);
		off +=2;
		pl2tpHeader->Nr = *(uint16*)(&pData[off]);
		off +=2;
	}

	if(off >= dataLen)
		return 0;
	
	if(code & L2TP_FLAG_OFF)
	{
		pl2tpHeader->offsetSize = ntohs(*(uint16*)(&pData[off]));
		pl2tpHeader->dataOff = pl2tpHeader->offsetSize;
	}
	else
		pl2tpHeader->dataOff = off;

	return 1;
}

static int _rtk_rg_l2tp_process(int direct, unsigned char * pData, unsigned int dataLen,rtk_rg_alg_connection_t * pConn)
{
	rtk_rg_l2tp_header_t l2tpHeader;
	rtk_rg_l2tp_avp_t * pAvp = NULL;
	int ret;
	ret = _rtk_rg_l2tp_parse_header(pData, dataLen, &l2tpHeader);
	if(ret < 1)
		return 0;

	DEBUG("l2tp header %s message: length(%d), tunnelId(%d), sessionId(%d)\n", 
				L2TP_IS_CONTROL(l2tpHeader.code)?"control":"data", l2tpHeader.length,
				l2tpHeader.tunnelId, l2tpHeader.sessionId);
	
	if(L2TP_IS_CONTROL(l2tpHeader.code) && l2tpHeader.length == 0)
		return 0;

	if(_rtk_rg_l2tp_parse_avp(pData, &l2tpHeader, &pAvp))
	{
		DEBUG("l2tp type(%d), %s\n", l2tpHeader.controlType,
			pAvp == NULL ? "don't have assigned tunnelId avp": "has assigned tunnelId avp");

		if(L2TP_IS_CONTROL(l2tpHeader.code) && l2tpHeader.tunnelId == 0 &&
			l2tpHeader.controlType == L2TP_CONTROL_SCCRQ)
		{
			//it is the start-control-connection-request message
			if(direct == NAPT_DIRECTION_OUTBOUND)
			{
				DEBUG("l2tp sccrq message: add an expect\n");
				_rtk_rg_l2tp_expect(direct, pConn);
			}
		}
	}
	return 1;
}

int _rtk_rg_l2tp_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
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

	//l2tp header lengh is at least 6 bytes 
	if (appLen < 6)
		return FAIL;

	//do nothing before napt modification
	if(after == 0)
		return SUCCESS;
	
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{		
		if(pConn->tuple.isIp6 == 0)
		{
			pConn->tuple.extIp.ip = ntohs(*pPktHdr->pIpv4Sip);
			pConn->tuple.extPort = ntohs(*pPktHdr->pSport);
		}
	}

	ret = _rtk_rg_l2tp_process(direct, pAppData, appLen, pConn);
	if(ret < 1)
		return FAIL;
#endif	
	return ret;
}

int rtk_rg_algRegFunc_l2tp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
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
				return FAIL;
			
			ret = _rtk_rg_l2tp_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);	
			
		}
		else
		{
			//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
	
			pConn = _rtk_rg_alg_connection_find(&tuple);		
			if(pConn == NULL)
				return FAIL;
			
			ret = _rtk_rg_l2tp_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
		}
	}

#endif
	return SUCCESS;
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

