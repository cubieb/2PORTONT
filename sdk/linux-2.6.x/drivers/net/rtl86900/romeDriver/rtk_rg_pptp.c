#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/skbuff.h>
#endif

#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_pptp.h>
#include <rtk_rg_alg_tool.h>

extern rtk_rg_algDatabase_t alg_db;

int _rtk_rg_PPTPExtCallIDGetAndUse(uint16 wishCallID)
{
	int wishIdx;
	uint32 wishBitValue;
	int i;

	i=wishCallID;
	while(1)
	{
		wishIdx=i>>5; // =wishCallID/32
		wishBitValue=1<<(i&0x1f);
		
		if((alg_db.algPPTPExtCallIDEnabled[wishIdx]&wishBitValue)==0)
		{
			//DEBUG("callID %d is set!",i);
			alg_db.algPPTPExtCallIDEnabled[wishIdx]|=wishBitValue;
			return i;
		}
		
		i++;
		i&=0xffff;
		if(i==wishCallID) break;		
	}	
	return FAIL;
}

void _rtk_rg_PPTPExtCallIDFree(int callID)
{
	int idx;
	uint32 bitValue;
	int i;

	i=callID;
	idx=i>>5; // =callID/32
	bitValue=1<<(i&0x1f);
	
	if((alg_db.algPPTPExtCallIDEnabled[idx]&bitValue)>0)
		alg_db.algPPTPExtCallIDEnabled[idx]&=(~bitValue);
}


rtk_rg_alg_connection_t *  _rtk_rg_alg_PPTPconn_findByExtCallID(rtk_rg_alg_tuple_t * pTuple, unsigned short externalCallID)
{
	rtk_rg_alg_connection_t * pConn=NULL;
	DEBUG("PPTPconn_findByExt: GRE internal(%08x) ext(%08x) remote(%08x) extCallID(%d)\n", 
			pTuple->internalIp.ip,
			pTuple->extIp.ip,
			pTuple->remoteIp.ip,
			externalCallID);
	
	for (pConn = alg_db.pAlgConnectionListHead->pPrev; pConn != alg_db.pAlgConnectionListHead; pConn = pConn->pPrev)
	{		
		if((pConn->valid == 1) && (pConn->appType==ALG_CONNTYPE_PPTP) &&
			rtk_rg_alg_addr_cmp(&pConn->tuple.remoteIp, &pTuple->remoteIp) &&
				rtk_rg_alg_addr_cmp(&pConn->tuple.extIp, &pTuple->extIp) &&
				pConn->app.pptp.externalCallID==externalCallID)
					break;
	}
	if(pConn == alg_db.pAlgConnectionListHead)		//not found
		return NULL;

	return pConn;
}

rtk_rg_alg_connection_t *  _rtk_rg_alg_PPTPconn_findByRemCallID(int direct, int after, rtk_rg_alg_tuple_t * pTuple, unsigned short remoteCallID)
{
	rtk_rg_alg_connection_t * pConn=NULL;
	DEBUG("PPTPconn_findByRem: %s GRE internal(%08x) ext(%08x) remote(%08x) remCallID(%d)\n", 
			after==0?"before":"after",
			pTuple->internalIp.ip,
			pTuple->extIp.ip,
			pTuple->remoteIp.ip,
			remoteCallID);

	if(after)
	{
		//Post
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			for (pConn = alg_db.pAlgConnectionListHead->pPrev; pConn != alg_db.pAlgConnectionListHead; pConn = pConn->pPrev)
			{		
				if((pConn->valid == 1) && (pConn->appType==ALG_CONNTYPE_PPTP) &&
					rtk_rg_alg_addr_cmp(&pConn->tuple.extIp, &pTuple->extIp) &&
					rtk_rg_alg_addr_cmp(&pConn->tuple.remoteIp, &pTuple->remoteIp) &&
						pConn->app.pptp.remoteCallID==remoteCallID)
							break;
			}
		}
		else
		{
			//INBOUND
			for (pConn = alg_db.pAlgConnectionListHead->pPrev; pConn != alg_db.pAlgConnectionListHead; pConn = pConn->pPrev)
			{		
				if((pConn->valid == 1) && (pConn->appType==ALG_CONNTYPE_PPTP) &&
					rtk_rg_alg_addr_cmp(&pConn->tuple.internalIp, &pTuple->internalIp) &&
					rtk_rg_alg_addr_cmp(&pConn->tuple.remoteIp, &pTuple->remoteIp) &&
						pConn->app.pptp.remoteCallID==remoteCallID)
							break;
			}
		}
	}
	else
	{
		//Before
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			for (pConn = alg_db.pAlgConnectionListHead->pPrev; pConn != alg_db.pAlgConnectionListHead; pConn = pConn->pPrev)
			{		
				if((pConn->valid == 1) && (pConn->appType==ALG_CONNTYPE_PPTP) &&
					rtk_rg_alg_addr_cmp(&pConn->tuple.internalIp, &pTuple->internalIp) &&
					rtk_rg_alg_addr_cmp(&pConn->tuple.remoteIp, &pTuple->remoteIp) &&
						pConn->app.pptp.remoteCallID==remoteCallID)
							break;
			}
		}
		else
		{
			//INBOUND
			for (pConn = alg_db.pAlgConnectionListHead->pPrev; pConn != alg_db.pAlgConnectionListHead; pConn = pConn->pPrev)
			{		
				if((pConn->valid == 1) && (pConn->appType==ALG_CONNTYPE_PPTP) &&
					rtk_rg_alg_addr_cmp(&pConn->tuple.extIp, &pTuple->extIp) &&
					rtk_rg_alg_addr_cmp(&pConn->tuple.remoteIp, &pTuple->remoteIp) &&
						pConn->app.pptp.remoteCallID==remoteCallID)
							break;
			}
		}
	}
	if(pConn == alg_db.pAlgConnectionListHead)		//not found
		return NULL;

	return pConn;
}


rtk_rg_fwdEngineReturn_t _rtk_rg_PPTP_GREModify(rtk_rg_naptDirection_t direct, struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_fwdEngineReturn_t ret;
	rtk_rg_alg_connection_t * pConn;
	rtk_rg_alg_tuple_t tuple;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));

	_rtk_rg_alg_init_tuple(direct, 0, pPktHdr, &tuple);
				
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{
		pConn = _rtk_rg_alg_PPTPconn_findByRemCallID(NAPT_DIRECTION_OUTBOUND, 0, &tuple, ntohs(*pPktHdr->pGRECallID));
		DEBUG("$$$$ PPTP_GREModify:  OUTBOUND packet to WAN[%d], orig SIP is %x",pPktHdr->netifIdx,pPktHdr->ipv4Sip);

		//lookup flow from remoteCallID
		if(pConn != NULL)
		{
			//Turn on action to prevent adding to shortCut
			pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
			
			ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,0, pPktHdr,skb,1,0);
			if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP

			DEBUG("SIP change to %x",ntohl(*pPktHdr->pIpv4Sip));

			return RG_FWDENGINE_RET_DIRECT_TX;
		}
	}
	else
	{
		pConn = _rtk_rg_alg_PPTPconn_findByExtCallID(&tuple, ntohs(*pPktHdr->pGRECallID));
		DEBUG("$$$$ PPTP_GREModify:  INBOUND packet from WAN[%d], orig DIP is %x",pPktHdr->netifIdx,pPktHdr->ipv4Dip);
		
		//lookup DIP from CallID in key(ExtCallID)
		if(pConn != NULL)
		{
			//Turn on action to prevent adding to shortCut
			pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;

			//Change DIP to internal IP
			*pPktHdr->pIpv4Dip=htonl(pConn->tuple.internalIp.ip);

			DEBUG("DIP change to %x, callID change from %x to %x",pConn->tuple.internalIp.ip,ntohs(*pPktHdr->pGRECallID),pConn->app.pptp.internalCallID);
			//Change CallID in key to IntCallID	
			*pPktHdr->pGRECallID=htons(pConn->app.pptp.internalCallID);
		
			ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,0,pPktHdr,skb,0,0);			
			pPktHdr->l3Modify=1;	//20150508LUKE: for wifi to check if recalculate chksum or not
			//dump_packet(skb->data,skb->len,"new");
			if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
		
			return RG_FWDENGINE_RET_DIRECT_TX;
		}
	}
		
	return RG_FWDENGINE_RET_TO_PS;
}

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_pptp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
//Attention: caller function needs to make sure it needs to do napt modification
//ipv6 address and port doesn't need to do napt modification now
#ifdef __KERNEL__
	rtk_rg_pktHdr_t *pPktHdr;
	rtk_rg_alg_connection_t * pConn;
	rtk_rg_alg_tuple_t tuple;
	int usableCallID=-1;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));

	if((pPktHdr->tagif&PPTP_TAGIF)==0)		//not PPTP packet
		return RG_FWDENGINE_ALG_RET_FAIL;
/*
	PPTP_StartCtrlConnRequest	= 1,
	PPTP_StartCtrlConnReply 	= 2,
	PPTP_StopCtrlConnRequest	= 3,
	PPTP_StopCtrlConnReply	= 4,
	PPTP_EchoRequest			= 5,
	PPTP_EchoReply			= 6,
	PPTP_OutCallRequest 		= 7,
	PPTP_OutCallReply			= 8,
	PPTP_InCallRequest		= 9,
	PPTP_InCallReply			= 10,
	PPTP_InCallConn 			= 11,
	PPTP_CallClearRequest		= 12,
	PPTP_CallDiscNotify 		= 13,
	PPTP_WanErrorNotify 		= 14,
	PPTP_SetLinkInfo			= 15
*/
	_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
	
	if(after==0)
	{
		//Pre function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			pConn = _rtk_rg_alg_connection_find(&tuple);
			switch(pPktHdr->pptpCtrlType)
			{
				case PPTP_OutCallRequest:
					//find ExtCallID for use
					//Keep internalIpAddr and MAC
					//Keep internalCallID
					DEBUG("$$$$ PRE PPTP_OutCallRequest  pPktInfo->netifIdx is %d, pPktHdr->pPptpCallId->cid1 is %d",pPktHdr->netifIdx,pPktHdr->pPptpCallId->cid1);
					if(pConn == NULL || pConn->appType!=ALG_CONNTYPE_PPTP)
					{
						pConn = _rtk_rg_alg_connection_add(&tuple);	
						pConn->appType=ALG_CONNTYPE_PPTP;
					
						usableCallID=_rtk_rg_PPTPExtCallIDGetAndUse(pPktHdr->pPptpCallId->cid1);
						if(usableCallID==FAIL)
						{
							DEBUG("PPTP_OutCallRequest: Thers is no usable CallID now...");
							return RG_FWDENGINE_ALG_RET_FAIL;
						}

						pConn->app.pptp.internalCallID=pPktHdr->pPptpCallId->cid1;
						pConn->app.pptp.externalCallID=usableCallID;
						DEBUG("################ internalCallID is %d, extCallID is %d",pConn->app.pptp.internalCallID,pConn->app.pptp.externalCallID);
					}

					//Replace internal CallID to external CallID
					pPktHdr->pPptpCallId->cid1=pConn->app.pptp.externalCallID;
					break;
				case PPTP_CallClearRequest:
					DEBUG("$$$$ PRE PPTP_CallClearRequest  pPktInfo->netifIdx is %d",pPktHdr->netifIdx);
					if(pConn == NULL || pConn->appType!=ALG_CONNTYPE_PPTP)
					{
						DEBUG("PPTP_CallClearRequest: Error..we can not find the GRE entry ..");
						return RG_FWDENGINE_ALG_RET_FAIL;
					}
					
					//Replace internal CallID to external CallID
					pPktHdr->pPptpCallId->cid1=pConn->app.pptp.externalCallID;

					//patch for ubuntu pptpd won't send CallDisconnectNotify when receive CallClearRequest
					_rtk_rg_PPTPExtCallIDFree(pConn->app.pptp.externalCallID);
					break;
				default:
					break;
			}
		}
		else
		{
			//INBOUND
			switch(pPktHdr->pptpCtrlType)
			{
				case PPTP_OutCallReply:
					pConn = _rtk_rg_alg_PPTPconn_findByExtCallID(&tuple,pPktHdr->pPptpCallId->cid2);
					//Keep remoteCallID
					DEBUG("$$$$ PPTP_OutCallReply  pPktInfo->netifIdx is %d, extCallID is %d",pPktHdr->netifIdx,pPktHdr->pPptpCallId->cid2);
					if(pConn == NULL || pConn->appType!=ALG_CONNTYPE_PPTP)
					{
						DEBUG("PPTP_OutCallReply: Error..we can not find the GRE entry from ExternalCallID %d..",pPktHdr->pPptpCallId->cid2);
						return RG_FWDENGINE_ALG_RET_FAIL;
					}
					pConn->app.pptp.remoteCallID=pPktHdr->pPptpCallId->cid1;
					DEBUG("@@@ the GreEntry has been set in PRE:retmoteCallID is %d, extCallID is %d, intCallID is %d",pConn->app.pptp.remoteCallID,pConn->app.pptp.externalCallID,pConn->app.pptp.internalCallID);
					//Replace externalCallID by internalCallID
					pPktHdr->pPptpCallId->cid2=pConn->app.pptp.internalCallID;
					break;
				case PPTP_WanErrorNotify:
					pConn = _rtk_rg_alg_PPTPconn_findByExtCallID(&tuple,pPktHdr->pPptpCallId->cid1);
					//replace peer's CallID to internalCallID
					if(pConn == NULL || pConn->appType!=ALG_CONNTYPE_PPTP)
					{
						DEBUG("PPTP_WanErrorNotify: Error..we can not find the GRE entry from ExternalCallID %d..",pPktHdr->pPptpCallId->cid1);
						return RG_FWDENGINE_ALG_RET_FAIL;
					}
					//Replace externalCallID by internalCallID
					pPktHdr->pPptpCallId->cid1=pConn->app.pptp.internalCallID;
					break;
				default:
					break;
			}
		}
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			pConn = _rtk_rg_alg_connection_find(&tuple);
			switch(pPktHdr->pptpCtrlType)
			{
				case PPTP_OutCallRequest:
					//use ExtCallID to find match entry
					DEBUG("$$$$ POST PPTP_OutCallRequest  pPktInfo->netifIdx is %d",pPktHdr->netifIdx);
					if(pConn == NULL || pConn->appType!=ALG_CONNTYPE_PPTP)
					{
						DEBUG("PPTP_OutCallRequest: Error..we can not find the GRE entry from ExternalCallID %d..",pPktHdr->pPptpCallId->cid1);
						return RG_FWDENGINE_ALG_RET_FAIL;
					}
					//keep external IP and port
					if(pPktHdr->tagif & IPV6_TAGIF)
						memcpy(&pConn->tuple.extIp, pPktHdr->pIpv6Sip, sizeof(union rtk_rg_alg_addr));
					else
						pConn->tuple.extIp.ip=ntohl(*pPktHdr->pIpv4Sip);
					pConn->tuple.extPort=ntohs(*pPktHdr->pSport);
					
					DEBUG("@@@ the GreEntry has been set in PRE:remoteIP is %08x",pConn->tuple.remoteIp.ip);
					break;
				case PPTP_CallDiscNotify:
					//pConn = _rtk_rg_alg_PPTPconn_findByRemCallID(after, &tuple,pPktHdr->pPptpCallId->cid1);
					//release GRE entry if we receive CallDisconnectNotify from WAN interface
					DEBUG("$$$$ PPTP_CallDiscNotify  pPktInfo->netifIdx is %d",pPktHdr->netifIdx);
					if(pConn == NULL)
					{
						DEBUG("PPTP_CallDiscNotify: Error..we can not find the GRE entry for IntCallID %d..",pPktHdr->pPptpCallId->cid1);
						return RG_FWDENGINE_ALG_RET_FAIL;
					}
					pConn->valid=0;
					_rtk_rg_PPTPExtCallIDFree(pConn->app.pptp.externalCallID);
					break;
				default:
					break;
			}
		}
		else
		{
			//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
			//INBOUND
			switch(pPktHdr->pptpCtrlType)
			{
				case PPTP_CallDiscNotify:
					pConn = _rtk_rg_alg_PPTPconn_findByRemCallID(direct, after, &tuple,pPktHdr->pPptpCallId->cid1);
					//release GRE entry if we receive CallDisconnectNotify from LAN interface
					DEBUG("$$$$ PPTP_CallDiscNotify  pPktInfo->netifIdx is %d",pPktHdr->netifIdx);
					if(pConn == NULL || pConn->appType!=ALG_CONNTYPE_PPTP)
					{
						DEBUG("PPTP_CallDiscNotify: Error..we can not find the GRE entry from RemoteCallID %d..",pPktHdr->pPptpCallId->cid1);
						return RG_FWDENGINE_ALG_RET_FAIL;
					}
					pConn->valid=0;
					_rtk_rg_PPTPExtCallIDFree(pConn->app.pptp.externalCallID);
					break;
				default:
					break;
			}
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

