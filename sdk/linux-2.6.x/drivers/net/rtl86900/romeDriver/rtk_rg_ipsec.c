#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/skbuff.h>
#endif

#include <rtk_rg_ipsec.h>
#include <rtk_rg_alg_tool.h>

static rtk_rg_isakmp_t isakmpDB[IPSEC_Max_ISAKMP];
unsigned char nat_t_md5[16] = {0x4a,0x13,0x1c,0x81,0x07,0x03,0x58,0x45,
0x5c,0x57,0x28,0xf2,0x0e,0x95,0x45,0x2f};

static rtk_rg_isakmp_t *  _rtk_rg_esp_findEspOut(uint32 local_ip,uint32 peer_ip, uint32 ospi)
{
	int i;
	rtk_rg_isakmp_t * tp = isakmpDB;
	
	for(i = 0 ; i < IPSEC_Max_ISAKMP ; i++){
		if(tp[i].peer_ip == peer_ip && tp[i].local_ip == local_ip &&
		   tp[i].ospi == ospi && tp[i].valid == 1 )
		{	
		   	tp[i].expire = jiffies + IPSEC_AGING_TIME * HZ;
		   	
		   	DEBUG("Found session #%d outbound \n", i);
		   	return &tp[i];
		}
	}
	
	return NULL;
}

static rtk_rg_isakmp_t * _rtk_rg_esp_addEsp(uint32 local_ip, uint32 peer_ip,uint32 ospi)
{
	int i;
	rtk_rg_isakmp_t * tp = isakmpDB;
	
	for(i = 0 ; i < IPSEC_Max_ISAKMP ; i++){
		if(tp[i].valid == 1 && tp->peer_ip == peer_ip && tp[i].local_ip == local_ip)
		{
			// Here comes new spi
			DEBUG("New ESP session #%d out, spi=%x -> %x\n", i, tp[i].ospi, ospi);
			tp[i].expire = jiffies + IPSEC_AGING_TIME * HZ;
			tp[i].ospi = ospi;
			tp[i].ispi = 0;
			return &tp[i];
		}
	}
	return NULL;
}

static rtk_rg_isakmp_t * _rtk_rg_esp_findEspIn(uint32 peer_ip,uint32 alias_ip,uint32 ispi){
	int i;
	rtk_rg_isakmp_t * tp = isakmpDB, *new_tp = NULL;
	
	for(i = 0 ; i < IPSEC_Max_ISAKMP ; i++)
	{
		if(tp[i].peer_ip == peer_ip && tp[i].alias_ip == alias_ip &&
		   tp[i].valid == 1 && tp[i].ispi == ispi )
		{	
			tp[i].expire = jiffies + IPSEC_AGING_TIME * HZ;	   	
		   	DEBUG("Found session #%d inbound \n", i);
			return &tp[i];
		}
		
		if(tp[i].peer_ip == peer_ip && tp[i].alias_ip == alias_ip &&
		   tp[i].ispi == 0 && tp[i].valid == 1)
		{
			DEBUG("Refresh ESP session #%d on reply (new spi)\n", i);
			new_tp = &tp[i];
	  	}
	}
	
	if(new_tp != NULL)
	{
		// Here comes new spi
		new_tp->expire = jiffies + IPSEC_AGING_TIME * HZ;	 	   	
		new_tp->ispi = ispi;
		return new_tp;
	}
	
	return NULL;
}

int _rtk_rg_esp_process(int direct, rtk_rg_alg_tuple_t * pTuple, uint32 spi, rtk_rg_isakmp_t * pIsakmp)
{
	pIsakmp = NULL;
	
	if(direct == NAPT_DIRECTION_OUTBOUND)
	{	
		DEBUG("findEspOut src %pI4, dst %pI4, ospi:%x\n", &pTuple->internalIp.ip, &pTuple->remoteIp.ip, spi);
		pIsakmp = _rtk_rg_esp_findEspOut(pTuple->internalIp.ip, pTuple->remoteIp.ip, spi);
		if(pIsakmp == NULL)
		{
        	pIsakmp=_rtk_rg_esp_addEsp(pTuple->internalIp.ip, pTuple->remoteIp.ip, spi);
			//for the esp packet whose ip pair doesn't exist in isakmp database, drop it
			if(pIsakmp == NULL)
				return 0;
		}

	}
	else
	{	
		DEBUG("findEspIn src %pI4, dst %pI4, ispi:%x\n", &pTuple->remoteIp.ip,  &pTuple->extIp.ip, spi);
		pIsakmp = _rtk_rg_esp_findEspIn(pTuple->remoteIp.ip, pTuple->extIp.ip, spi);
		if(pIsakmp != NULL)
		{		
			DEBUG("reply to localip=%x\n", pIsakmp->local_ip);
		}
		else
		{
			DEBUG("can not bind to session on reply, drop it!\n");
			return 0;
		}
	}
	
	return 1;
}

#ifdef IPSEC_OLD_RULE	//disable this ...
static rtk_rg_isakmp_t * _rtk_rg_ipsec_findIsakmpOut(uint32 local_ip,uint32 peer_ip,uint64 icookie,uint64 rcookie)
{
	int i;
	rtk_rg_isakmp_t *tp = isakmpDB;
	
	for(i = 0 ; i < IPSEC_Max_ISAKMP; i++){
		if(tp[i].peer_ip == peer_ip &&
		   tp[i].local_ip == local_ip &&
		   tp[i].icookie == icookie &&
		   tp[i].valid == 1){		
			DEBUG("find IKE out i=%d, local %pI4 peer %pI4 icookie=%x %x\n", i, &local_ip, &peer_ip, 
						*(uint32 *)(&icookie), *((uint32 *)(&icookie)+1));
			
			return &tp[i];
		}
	}	
	return NULL;
}

static rtk_rg_isakmp_t*  _rtk_rg_ipsec_findIsakmpIn(uint32 peer_ip,uint32 alias_ip,uint64 icookie,uint64 rcookie)
{
	int i;
	rtk_rg_isakmp_t * tp = isakmpDB;
	
	for(i = 0 ; i < IPSEC_Max_ISAKMP ; i++){
		if(tp[i].peer_ip == peer_ip &&
		   tp[i].alias_ip == alias_ip &&
		   tp[i].icookie == icookie &&
		   tp[i].valid == 1){
			
			tp[i].expire = jiffies + IPSEC_AGING_TIME * HZ;	 
			tp[i].rcookie = rcookie;
			
			DEBUG("find IKE in i=%d, local_ip=%x, icookie=%x %x\n", i, tp->local_ip, 
						*(uint32 *)(&icookie), *((uint32 *)(&icookie)+1));
			return &tp[i];
		}
	}
	
	return NULL;
}

static rtk_rg_isakmp_t* _rtk_rg_ipsec_addIsakmp(uint32 local_ip,uint32 peer_ip,uint32 alias_ip,
		uint64 icookie,uint64 rcookie){
	int i, oldIndex = 0;
	rtk_rg_isakmp_t * tp = isakmpDB;

	// find the existed one or the one exists the longset time
	for(i = 0 ; i < IPSEC_Max_ISAKMP  ; i++){
		if(tp[i].peer_ip == peer_ip &&
		   tp[i].local_ip == local_ip &&
		   tp[i].valid == 1) {
		   	// session refresh
			tp[i].expire = jiffies + IPSEC_AGING_TIME * HZ;
			tp[i].icookie = icookie;
			tp[i].rcookie = rcookie;
			DEBUG("Existed session #%d been found\n", i);
			return &tp[i];
		}
		if(tp[i].expire < tp[oldIndex].expire)
			oldIndex = i;
	}
	
	// if not already exists, find a new one
	for(i = 0 ; i < IPSEC_Max_ISAKMP  ; i++){
		if(tp->valid == 0){
			memset(&tp[i], 0, sizeof(rtk_rg_isakmp_t));
			tp[i].expire = jiffies + IPSEC_AGING_TIME * HZ;	
			tp[i].peer_ip = peer_ip;
			tp[i].local_ip = local_ip;
			tp[i].alias_ip = alias_ip;
			tp[i].icookie = icookie;
			tp[i].rcookie = rcookie;
			tp[i].valid = 1;

			DEBUG("Free session #%d been found\n", i);
			return &tp[i];
		}
	}

	// replace the existing longest one if it exists more than the aging time
	if(time_after(jiffies,tp[oldIndex].expire))
	{
		//memset(&tp[i], 0, sizeof(rtk_rg_isakmp_t));
		memset(&tp[oldIndex], 0, sizeof(rtk_rg_isakmp_t));
		tp[oldIndex].expire = jiffies + IPSEC_AGING_TIME * HZ;
		tp[oldIndex].peer_ip = peer_ip;
		tp[oldIndex].local_ip = local_ip;
		tp[oldIndex].alias_ip = alias_ip;
		tp[oldIndex].icookie = icookie;
		tp[oldIndex].rcookie = rcookie;
		tp[oldIndex].valid = 1;
		DEBUG("Replace the oldest session %d by new one\n", oldIndex);
		return &tp[i];
	}
	return NULL;
}
#endif
static rtk_rg_fwdEngineAlgReturn_t _rtk_rg_ipsec_out_process(int direct, unsigned char * pData, unsigned int dataLen,rtk_rg_alg_connection_t * pConn)

{
#ifdef IPSEC_OLD_RULE	//disable this ...
	rtk_rg_isakmp_t * pIsakmp;
#endif
	uint64 icookie, rcookie;
	int i;
	unsigned char is_natt = 0;	//not nat-t
	
	if(dataLen<(IPSEC_HEADER_LEN))		return  RG_FWDENGINE_ALG_RET_FAIL;	
	//icookie= *(uint64*)pData;
	//rcookie= *(uint64*)(pData+8);
	memcpy((void *)&icookie,pData,8);
	memcpy((void *)&rcookie,pData+8,8);
	DEBUG("out-bound: s_addr=%pI4, d_addr=%pI4, icookie=%x %x,rcookie=%x %x\n", 
				&pConn->tuple.internalIp.ip, &pConn->tuple.remoteIp.ip,
				*(uint32 *)(&icookie), *((uint32 *)(&icookie)+1), *(uint32 *)(&rcookie), *((uint32 *)(&rcookie)+1));
	
	if(rcookie==0L)	//1st pkt of initiator
	{
		if(dataLen > (IPSEC_HEADER_LEN+IPSEC_NATT_MD5_LEN+4))
		{
			for(i=IPSEC_HEADER_LEN;i<dataLen;i++)
			{
				if((pData[i] == nat_t_md5[0]) && (pData[i+1] == nat_t_md5[1]) && (pData[i+2] == nat_t_md5[2]) && (pData[i+3] == nat_t_md5[3])
				&& (pData[i+4] == nat_t_md5[4]) && (pData[i+5] == nat_t_md5[5]) && (pData[i+6] == nat_t_md5[6]) && (pData[i+7] == nat_t_md5[7])
				&& (pData[i+8] == nat_t_md5[8]) && (pData[i+9] == nat_t_md5[9]) && (pData[i+10] == nat_t_md5[10]) && (pData[i+11] == nat_t_md5[11])
				&& (pData[i+12] == nat_t_md5[12]) && (pData[i+13] == nat_t_md5[13]) && (pData[i+14] == nat_t_md5[14]) && (pData[i+15] == nat_t_md5[15]))
				{
					is_natt = 1;	//is nat-t
				}
			}
		}
		DEBUG(">>>> natt=%d\n",is_natt);
		if((is_natt == 1) && (rg_db.systemGlobal.ipsec_passthru==PASS_DISABLE))
		{
			DEBUG(">>>>> is nat-t=1 drop\n");
			return RG_FWDENGINE_ALG_RET_DROP;
		}
		if((is_natt == 0) && (rg_db.systemGlobal.ipsec_passthru==PASS_ENABLE))
		{
			DEBUG(">>>>> is nat-t=0 drop\n");
			return RG_FWDENGINE_ALG_RET_DROP;
		}
	}
	
#ifdef IPSEC_OLD_RULE	//disable this ...
	//if(rcookie==0 || icookie==0)
	if(rcookie==0L)	//1st pkt of initiator
	{
		pIsakmp = _rtk_rg_ipsec_findIsakmpOut(pConn->tuple.internalIp.ip, pConn->tuple.remoteIp.ip, icookie, rcookie);			
		if(pIsakmp == NULL)
		{
			rtk_rg_isakmp_t* pRet;
			DEBUG("This is ORIGINAL dir packet, finding in OUT way\n");
			pRet = _rtk_rg_ipsec_addIsakmp(pConn->tuple.internalIp.ip, pConn->tuple.remoteIp.ip, pConn->tuple.extIp.ip, icookie, rcookie);
			if(pRet==NULL)	//no available entry now
			{
				DEBUG("In OUT way, but TABLE FULL DROP\n");
				return RG_FWDENGINE_ALG_RET_DROP;
			}
			else
				return RG_FWDENGINE_ALG_RET_SUCCESS;
		}
		else
			DEBUG("Found entry=0x%x in OUT way\n",(int)pIsakmp);
		
	}
#endif
	return RG_FWDENGINE_ALG_RET_SUCCESS;
}

static rtk_rg_fwdEngineAlgReturn_t _rtk_rg_ipsec_in_process(int direct, unsigned char * pData, unsigned int dataLen,rtk_rg_alg_connection_t * pConn)
{
#ifdef IPSEC_OLD_RULE	//disable this
	rtk_rg_isakmp_t * pIsakmp;
#endif
	uint64 icookie, rcookie;	

	if(dataLen<(IPSEC_HEADER_LEN))		return  RG_FWDENGINE_ALG_RET_FAIL;	
	
	icookie= *(uint64*)pData;
	rcookie= *(uint64*)(pData+8);
	
	
#ifdef IPSEC_OLD_RULE	//disable this
	DEBUG("In-bound: s_addr=%pI4, d_addr=%pI4, icookie=%x %x,rcookie=%x %x\n", 
				&pConn->tuple.remoteIp.ip, &pConn->tuple.extIp.ip,
				*(uint32 *)(&icookie), *((uint32 *)(&icookie)+1), *(uint32 *)(&rcookie), *((uint32 *)(&rcookie)+1));
	//if(rcookie!=0 || icookie==0)
	if(rcookie!=0L)	//reply pkt should have rcookie
	{		
						
		pIsakmp = _rtk_rg_ipsec_findIsakmpIn(pConn->tuple.remoteIp.ip, pConn->tuple.extIp.ip, icookie, rcookie);
		if(pIsakmp == NULL)
		{
			DEBUG("In IN way, but NO EXISTED ENTRY DROP\n");
			return RG_FWDENGINE_ALG_RET_DROP; /* it is not an expect reply packet, should drop it*/
		}
		else
			DEBUG("Found entry=0x%x in IN way\n",(int)pIsakmp);
	}
#endif
	return RG_FWDENGINE_ALG_RET_SUCCESS;
}


rtk_rg_fwdEngineAlgReturn_t _rtk_rg_isakmp_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
{
	rtk_rg_fwdEngineAlgReturn_t ret = RG_FWDENGINE_ALG_RET_SUCCESS;
#ifdef __KERNEL__
	unsigned char * pData, *pAppData;
	unsigned int appLen=0,dataOff=0;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_alg_connection_t * pConn;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	pConn = (rtk_rg_alg_connection_t *)pConnInfo;
	skb= (struct sk_buff *)pSkb;

	//do nothing before napt modification
	if(after == 0)
		return RG_FWDENGINE_ALG_RET_SUCCESS;
	
	//only support udp packet
	if(pPktHdr->tagif&TCP_TAGIF)
		return RG_FWDENGINE_ALG_RET_FAIL;
	
	pData=skb->data;

	if(pPktHdr->tagif&TCP_TAGIF)
		dataOff = pPktHdr->l4Offset + pPktHdr->headerLen;
	else
		dataOff = pPktHdr->l4Offset + 8; /*udp header length is 8 bytes*/
#if 1
	appLen = pPktHdr->l3Len + pPktHdr->l3Offset - dataOff;
	DEBUG("_rtk_rg_isakmp_handler appLen(%d-%d-%d) = %d\n",pPktHdr->l3Len,pPktHdr->l3Offset,dataOff,appLen);
#else
	appLen = skb->len - dataOff;
#endif
	pAppData = pData + dataOff;

	
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{		
		if(pConn->tuple.isIp6 == 0)
		{
			pConn->tuple.extIp.ip = ntohl(*pPktHdr->pIpv4Sip);
			pConn->tuple.extPort = ntohs(*pPktHdr->pSport);
		}
	}

	if(direct == NAPT_DIRECTION_OUTBOUND)
		ret = _rtk_rg_ipsec_out_process(direct, pAppData, appLen, pConn);
	else
		ret = _rtk_rg_ipsec_in_process(direct, pAppData, appLen, pConn);
#endif	
	return ret;
}

//protocol value in IP header is ESP and before napt modification
rtk_rg_fwdEngineReturn_t rtk_rg_alg_ESP(int direct, unsigned char *pSkb,unsigned char *pPktInfo, rtk_rg_isakmp_t * pIsakmp)
{
#ifdef __KERNEL__
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_alg_tuple_t tuple;
	unsigned char * pData;
	int len;
	uint32 spi;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	skb= (struct sk_buff *)pSkb;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));

	/*Ipv6 doesn't support ip address nat modification*/
	if(pPktHdr->tagif & IPV6_TAGIF)
		return RG_FWDENGINE_RET_TO_PS;
	
	if(direct==NAPT_DIRECTION_OUTBOUND)
	{
		
		tuple.internalIp.ip = pPktHdr->ipv4Sip;
		tuple.remoteIp.ip = pPktHdr->ipv4Dip;		
	}
	else
	{
		tuple.extIp.ip = pPktHdr->ipv4Dip;
		tuple.remoteIp.ip = pPktHdr->ipv4Sip;
	}

		
	pData = skb->data + pPktHdr->l4Offset;
	len = skb->len - pPktHdr->l4Offset;

	/*esp header
	 * Security Parameters Index (SPI):4bytes
	 * Sequence Number : 4bytes
	*/
	if(len < 8)
		return RG_FWDENGINE_RET_DROP;
	
	spi = *(uint32 *)pData;

	if(_rtk_rg_esp_process(direct, &tuple, spi, pIsakmp))
		return RG_FWDENGINE_RET_CONTINUE;
	else
		return RG_FWDENGINE_RET_DROP;
#endif
	return RG_FWDENGINE_RET_TO_PS;
}


//if udp port is 500, it is a isakmp packet
//for IPsec transport mode, need to make sure the internal port and external port are the same.
//for IPsec tunnel mode, can't support it since the tcp/udp headr is protected by ESP protocol and it is not visible
rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_IPsec(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
//Attention: caller function needs to make sure it needs to do napt modification
//ipv6 address and port doesn't need to do napt modification now
#ifdef __KERNEL__
	rtk_rg_fwdEngineAlgReturn_t ret=0;
	rtk_rg_pktHdr_t *pPktHdr;
	rtk_rg_alg_connection_t * pConn;
	rtk_rg_alg_tuple_t tuple;
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));
	
	if(after==0)
	{
		//Pre function
		//rtk_rg_isakmp_system_proc_init();	//coudl be called here?!
		//DEBUG("***************** rtk_rg_algRegFunc_IPsec is initialized\n");
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
				return RG_FWDENGINE_ALG_RET_FAIL;
			
			ret = _rtk_rg_isakmp_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);	
			
		}
		else
		{
			//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
	
			pConn = _rtk_rg_alg_connection_find(&tuple);		
			if(pConn == NULL)
				return RG_FWDENGINE_ALG_RET_FAIL;
			
			ret = _rtk_rg_isakmp_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
		}
	}

#endif
	return ret;
}


/*udp port is 4500, it is NAT-Traversal (NAT-T).
 * 1. if value of the first 4 bytes after outer udp header are zero, it is a isakmp packet
 *   
 * 2. if udp checksum is 0, it is a esp packet. 
 *     we can change the ip address and port in outer header by _rtk_rg_fwdEngine_shortCutNaptPacketModify
 *     so don't need to handle it here
 *
int rtk_rg_algRegFunc_UDP_4500_IPsec(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
	return SUCCESS;
}
*/

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