#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#endif

#include <rtk_rg_sip.h>
#include <rtk_rg_alg_tool.h>

static int32 init_sip = 0;
static uint8 *_rtl8651_l4_sip_alias_buf[1];
static uint16 o_port=0,i_port=0;

int num_memcmp(void *src,void *dest,int num)
{
	int r;
	char *pSrc=(char*)src;
	r=memcmp(src,dest,num);
	if(r!=0) return r;
	if((pSrc[num]>='0')&&(pSrc[num]<='9')) return -1; //ex: '192.168.1.1' hit '192.168.1.111' is not allow.
	return 0;
}

static char localip[16];
static int8 localip_len;
static char globalip[16];
static int8 globalip_len;
static char globalport[16];
static int8 globalport_len;
static char localport[16];
static int8 localport_len;
static char trueport[16];
static int8 trueport_len=0;
static char rtpport[16];
static int8 rtpport_len=0;
//char controlport[16];
//int8 controlport_len=0;
//char controlextport[16];
//int8 controlextport_len=0;
static char remoteip[16];
static int8 remoteip_len;
static char key[32];
static int8 key_len;
static char keyword[32];
static int8 keyword_len;
static char ipkey[32];
static int8 ipkey_len;
static uint32 trueip=0;
static char realip[32];
static int8 realip_len=0;
static int8 old_cl_len=0; //old content-length len
static int8 new_cl_len=0; //new content-length len
//uint32 extIP;
static uint16 extPort;
static uint16 control_port = 0;
//uint16 control_extport = 0;
//uint16 rtcpPort = 0;
static int8 SameSubnet = FALSE;

//for inbound
static char viakey[32];
static int8 viakey_len=0;
static char rportkey[32];
static int8 rportkey_len;
static int8 SameGw = TRUE;
static char realrtpport[32];
static int8 realrtpport_len = 0;
static char realport[32];
static int8 realport_len = 0;
static uint32 truelocalip = 0;
static uint16 trueport_num = 0;

static int _rtk_rg_sip_process(int direct, unsigned char * pData, unsigned int dataLen,rtk_rg_alg_connection_t * pConn)
{

	//int ret,newDelta=0;
	int ret;
	//ipaddr_t ipAddr,gwIPAddr;
	//unsigned short portNum,newPort;
	//unsigned int newLen,flowIdx;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	//rtk_rg_naptEntry_t naptEntry;
	rtk_rg_upnpConnection_t upnpConn;
	
	pPktHdr = (rtk_rg_pktHdr_t *)pConn->pPktHdr;
	skb = (struct sk_buff *)pConn->skb;

	//if(direct==NAPT_DIRECTION_OUTBOUND)
	{
		int i,j,k;
		int32 size_delta=0;

		//for new packet
		struct iphdr *spip;
		struct udphdr *sudp;
		int8 *sptr, *newpacket;

		//for data flow mapt priority
		int index;
		rtk_rg_naptFilterAndQos_t napt_filter;

		// assigned new packet buffer ; use the latter part of _rtl8651_l4_sip_alias_buf 
		newpacket =  (int8 *)_rtl8651_l4_sip_alias_buf[0];
		
		// copy out the mbuf data 
		memcpy(newpacket,skb->data,(skb->len>_RTL8651_L4_MAX_SIP_ALIAS_BUF_SIZE)?_RTL8651_L4_MAX_SIP_ALIAS_BUF_SIZE:skb->len);

		//shift pointer to IP header
		newpacket+=pPktHdr->l3Offset;
			
		// Place string pointer and beginning of data 
		spip=(struct iphdr*)newpacket;
		sudp=(struct udphdr *)((int8 *) spip + ((spip->ihl &0xf) << 2));
		sptr=(int8 *)(sudp+1);
		
		sprintf(localip,"%u.%u.%u.%u",(pConn->tuple.internalIp.ip>>24)&0xff,(pConn->tuple.internalIp.ip>>16)&0xff,(pConn->tuple.internalIp.ip>>8)&0xff,(pConn->tuple.internalIp.ip)&0xff);
		localip_len=strlen(localip);
		
		sprintf(localport, "%u", (pConn->tuple.internalPort)&0xffff);
		localport_len = strlen(localport); 

		sprintf(globalip,"%u.%u.%u.%u",(pConn->tuple.extIp.ip>>24)&0xff,(pConn->tuple.extIp.ip>>16)&0xff,(pConn->tuple.extIp.ip>>8)&0xff,(pConn->tuple.extIp.ip)&0xff);
		globalip_len=strlen(globalip);	

		sprintf(globalport, "%u", (pConn->tuple.extPort)&0xffff);
		globalport_len = strlen(globalport); 

		sprintf(remoteip,"%u.%u.%u.%u",(pConn->tuple.remoteIp.ip>>24)&0xff,(pConn->tuple.remoteIp.ip>>16)&0xff,(pConn->tuple.remoteIp.ip>>8)&0xff,(pConn->tuple.remoteIp.ip)&0xff);
		remoteip_len = strlen(remoteip); 

		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			ALG("[SIP OUT] Begin: local=%s:%s global=%s:%s remote=%s\n",localip,localport,globalip,globalport,remoteip);
		}
		else
		{
			ALG("[SIP IN] Begin: local=%s:%s global=%s:%s remote=%s\n",localip,localport,globalip,globalport,remoteip);
		}
		
		sprintf(key,"\r\nContent-Length: ");
		key_len=18;	

		/*get the rtp listening port*/
		sprintf(keyword, "audio");
		keyword_len = 5;

		/*for search the real ip which may send rtp pkt later*/
		sprintf(ipkey, "IP4");
		ipkey_len = 3;


		if(direct==NAPT_DIRECTION_INBOUND)
		{
			sprintf(viakey, "SIP/2.0/UDP");
			viakey_len = strlen(viakey);

			sprintf(rportkey, "rport=");
			rportkey_len = 6;
		}
		


		for(i=0,j=0;i<dataLen;i++,j++)
		{

			if(pData[i]<0x20&&pData[i]!=0x0d&&pData[i]!=0x0a)  return SUCCESS; //not sip packet

			if(i+4<=dataLen)
			{
				// search RTP Port number
				if(memcmp(&pData[i],"\r\nm=",4)==0)
				{
					int flag=0;
					if(direct==NAPT_DIRECTION_OUTBOUND)
						i_port=0;
					else
						o_port=0;
					
					for(k=i+4;k<dataLen;k++)
					{
						if(pData[k]==' ')
						{
							if(flag==0) flag=k;
							else
							{
								//printk("out- in_port=[%d] out_port=%d\n",i_port,o_port);
								//_rtl8651_addAlgQosUpnpMap(UPNP_PERSIST|UPNP_ALG_QOS, 0, 0, 
								//tb->insideGlobalIpAddr, i_port,
								//tb->insideLocalIpAddr, i_port,RTL8651_ALG_SIP_IDX);
								//o_port=0;
								break;
							}
						}
						else if(flag!=0)
						{
							if(direct==NAPT_DIRECTION_OUTBOUND)
								i_port=(pData[k]-'0')+i_port*10;
							else
								o_port=(pData[k]-'0')+o_port*10;
						}
					}
				}

				else if((direct==NAPT_DIRECTION_INBOUND)&&(memcmp(&pData[i],viakey,viakey_len)==0))
				{
					//search true port
					if (memcmp(&pData[i+viakey_len+1],globalip,globalip_len)==0)
					{
						for (k = i+viakey_len +globalip_len +2; k <dataLen; k++ )
						{
							if((pData[k] >= '0') && (pData[k] <= '9'))
							{	
								trueport_num=(pData[k]-'0')+trueport_num*10;
							}
							else 
								break;
						}
						ALG("[SIP IN]trueport_num=%d\n",trueport_num);
						SameGw = TRUE;	
					}
					//search remote ip 
					else if(memcmp(&pData[i+viakey_len+1],remoteip,remoteip_len)!=0)
					{
						SameGw = FALSE;
					}
				}
				else if ((direct==NAPT_DIRECTION_INBOUND)&&((memcmp(&pData[i], ipkey, ipkey_len) ==0) && (memcmp(&pData[i + ipkey_len +1 ],globalip,globalip_len)!=0)))
				{
					//	printk("not the same gateway\n");
					SameGw = FALSE;
				}

				
				
				else if((direct==NAPT_DIRECTION_OUTBOUND)&&(memcmp(&pData[i], ipkey, ipkey_len)==0))
				{
					uint16 temp=0;
					trueip=0;
					for(k= i+4; k<dataLen;k++)
					{
						if(pData[k] ==' ')
						{
							trueip = (trueip << 8) + temp;
							break;
						}
						else if (pData[k] == '.')
						{
							trueip = (trueip << 8) + temp;
							temp = 0;
						}
						else if (pData[k] >= '0' && pData[k] <= '9') 
						{	
							temp=(pData[k]-'0')+temp*10;
							if (temp > 255)
								temp = temp/10;
						}
					}
				}
				else if((direct==NAPT_DIRECTION_OUTBOUND)&&( num_memcmp(&pData[i], localip, localip_len) ==0 ) && (memcmp(&pData[i + localip_len + 1], localport, localport_len) != 0))
				{
					int flag=0;				
					control_port=0;
					for(k=i + localip_len;k<i + localip_len +6;k++)
					{
						 if (pData[k] >= '0' && pData[k] <= '9') 
						{	
							flag = 1;
						}
						 else
						 	flag = 0;
							
						if (flag ==1 )
							control_port=(pData[k]-'0')+control_port*10;

					}
					if (control_port < 1024)
						control_port = 0;
					ALG("[SIP OUT]control port is %d\n", control_port);
				}
	//			if (trueip != 0)
	//				break;
			}
		}

		if((direct==NAPT_DIRECTION_OUTBOUND)&&(i_port !=0))
		{
			

			extPort=_rtk_rg_extPort_get(0,i_port);
			//if(newPort==FAIL) return SUCCESS;	
			//add UPnP connection here!!
			
			sprintf(trueport, "%u", (i_port)&0xffff);
			trueport_len = strlen(trueport); 	

			sprintf(rtpport, "%u", (extPort)&0xffff); //control flow ext port
			rtpport_len = strlen(rtpport);


			//Add a one-shot UPnP flow for incoming connection to each L4 WAN		
			upnpConn.is_tcp=pConn->tuple.isTcp;
			upnpConn.wan_intf_idx=pPktHdr->netifIdx;
			upnpConn.gateway_port=extPort;
			upnpConn.local_ip=pConn->tuple.internalIp.ip;
			upnpConn.local_port=i_port;
			upnpConn.limit_remote_ip=1;
			upnpConn.limit_remote_port=0;		//can't restrict client using which port to connect
			upnpConn.remote_ip=pConn->tuple.remoteIp.ip;
			upnpConn.remote_port=0;
			upnpConn.type=UPNP_TYPE_ONESHOT;
			upnpConn.timeout=rg_db.algUserDefinedTimeout[RTK_RG_ALG_SIP_TCP]; //auto time out if the client do not connect by this WAN
			assert_ok((pf.rtk_rg_upnpConnection_add)(&upnpConn,&ret));	
			ALG("[SIP OUT]add UPnP connection i_port=%d(%s) extPort=%d ret=%d\n",i_port,trueport,extPort,ret);


			//data flow napt priority is followed control flow priority.
			if((pPktHdr->tagif & TCP_TAGIF)||(pPktHdr->tagif & UDP_TAGIF)){// only support by TCP or UDP (Layer4 packet).
			
				//check is outbound priority need to assigned 
				if(rg_db.naptOut[pPktHdr->naptOutboundIndx].priValid==ENABLED){
					bzero(&napt_filter,sizeof(napt_filter));
					napt_filter.direction=RTK_RG_NAPT_FILTER_OUTBOUND;
					napt_filter.filter_fields = (L4_PROTOCAL|INGRESS_SIP|INGRESS_SPORT);
					if(pPktHdr->tagif & TCP_TAGIF)
						napt_filter.ingress_l4_protocal = 0x6;
					else if(pPktHdr->tagif & UDP_TAGIF) 
						napt_filter.ingress_l4_protocal = 0x11;
					napt_filter.ingress_src_ipv4_addr = pConn->tuple.internalIp.ip;
					napt_filter.ingress_src_l4_port = i_port;
					napt_filter.action_fields = ASSIGN_NAPT_PRIORITY_BIT;
					napt_filter.assign_priority = rg_db.naptOut[pPktHdr->naptOutboundIndx].priValue;//follow control flow.
					napt_filter.ruleType = RTK_RG_NAPT_FILTER_ONE_SHOT;
					assert_ok((pf.rtk_rg_naptFilterAndQos_add)(&index,&napt_filter));		
					ALG("FTP ALG add outbound naptFilter[%d](L4_PROTOCAL=0x%x SIP=0x%x + sport=%d => priority=%d, one shot) for data flow (The priority is based on napt[%d])",
						index,napt_filter.ingress_l4_protocal,napt_filter.ingress_src_ipv4_addr,napt_filter.ingress_src_l4_port,napt_filter.assign_priority,pPktHdr->naptOutboundIndx);		
				}
		
				//check is inbound priority need to assigned 
				if(rg_db.naptIn[pPktHdr->naptrInboundIndx].priValid==ENABLED){
					bzero(&napt_filter,sizeof(napt_filter));
					napt_filter.direction=RTK_RG_NAPT_FILTER_INBOUND;
					napt_filter.filter_fields = (L4_PROTOCAL|INGRESS_DPORT|INGRESS_SIP);
					if(pPktHdr->tagif & TCP_TAGIF)
						napt_filter.ingress_l4_protocal = 0x6;
					else if(pPktHdr->tagif & UDP_TAGIF) 
						napt_filter.ingress_l4_protocal = 0x11;
					napt_filter.ingress_dest_l4_port = extPort;
					napt_filter.ingress_src_ipv4_addr = pConn->tuple.remoteIp.ip;
					napt_filter.action_fields = ASSIGN_NAPT_PRIORITY_BIT;
					napt_filter.assign_priority = rg_db.naptIn[pPktHdr->naptrInboundIndx].priValue;//follow control flow.
					napt_filter.ruleType = RTK_RG_NAPT_FILTER_ONE_SHOT;
					assert_ok((pf.rtk_rg_naptFilterAndQos_add)(&index,&napt_filter));	
					ALG("FTP ALG add inbound naptFilter[%d](L4_PROTOCAL=0x%x SIP=0x%x + dport=%d => priority=%d, one shot) for data flow (The priority is based on naptr[%d])",
						index,napt_filter.ingress_l4_protocal,napt_filter.ingress_src_ipv4_addr,napt_filter.ingress_dest_l4_port,napt_filter.assign_priority,pPktHdr->naptrInboundIndx);		
				}
			}


				
		}


		if(direct==NAPT_DIRECTION_INBOUND)
		{
			if(o_port!=0)
			{
				sprintf(rtpport, "%u", (o_port)&0xffff); //external RTP Port
				rtpport_len = strlen (rtpport);
				sprintf(realrtpport, "%u", (i_port)&0xffff); //local RTP Port
				realrtpport_len = strlen(realrtpport);
				ALG("[SIP IN] oport = %d\n", o_port);

			}
			if (trueport_num != 0)
			{
				//sip_tb = &entry;
				rtk_rg_lookupIdxReturn_t naptOutIdx;
				rtk_rg_fwdEngineReturn_t fwd_ret;
				ipaddr_t transIP;
				uint16 transPort;
				sprintf(realport, "%u", (trueport_num)&0xffff);
				realport_len = strlen (realport); 

				naptOutIdx=_rtk_rg_naptTcpUdpInHashIndexLookup(0,pPktHdr->ipv4Sip,pPktHdr->sport,pConn->tuple.extIp.ip,trueport_num);
				fwd_ret = _rtk_rg_fwdEngine_connType_lookup(pPktHdr,&transIP,&transPort);
				if(fwd_ret==RG_FWDENGINE_RET_CONTINUE) 
				{
					truelocalip = transIP;
					sprintf(realip,"%u.%u.%u.%u",(truelocalip>>24)&0xff,(truelocalip>>16)&0xff,(truelocalip>>8)&0xff,(truelocalip)&0xff);
					realip_len=strlen(realip);
					ALG("[SIP IN] trueip = %x\n", truelocalip);
				}
				else
				{
					ALG("[SIP IN] trueip not found!\n");
				}
			}
		}
		

		//change content!!
		for(i=0,j=0;i<dataLen;i++,j++)
		{	

			if((i+key_len<=dataLen)&&(memcmp(&pData[i],key,key_len)==0))
			{

				//find "\r\nContent-Length: "

				int32 datalen=0;
				int32 flag=0;
				int32 datalen_stringlen=0;
				char tempStr[16];
				int32 CntIpInMsgBody=0;
				int32 CntPortInMsgBody=0;
				int32 CntTrueIpInMsgbody=0;
				int32 CntRealIpInMsgBody=0; //for inbound
				int msg;
				
				memcpy(&sptr[j],key,key_len);
				j+=key_len;
				i+=key_len;
				for(k=i;((k<i+20)&&(k<dataLen));k++)
				{
					if((pData[k]>='0')&&(pData[k]<='9'))
					{
						datalen=pData[k]-'0'+datalen*10;
						flag = 1;
						datalen_stringlen = datalen_stringlen +1;
					}
					else
					{
						if (flag == 1)
							break;
					}
				}
				ALG("[SIP] Content-Length=%d datalen_stringlen=%d\n",datalen,datalen_stringlen);

				memcpy(&sptr[j], &pData[i], k -datalen_stringlen -i);
				j += k -datalen_stringlen -i;
				i += k -datalen_stringlen -i;
				
				old_cl_len=datalen_stringlen;
				i=k-1;
				msg=i+2;
				//find msg body
				for(k=i+2;k<=dataLen-4;k++)
				{					
					if(memcmp(&pData[k],"\r\n\r\n",4)==0)
					{
						msg=k+4;
						break;
					}					
				}

				if(direct==NAPT_DIRECTION_OUTBOUND)
				{
					// count how many ip in Message body
					for(k=msg;k<=dataLen-localip_len;k++)
					{			
						if ((num_memcmp(&pData[k],localip,localip_len)==0) /*&& (memcmp(&pData[k - ipkey_len -1], ipkey, ipkey_len) !=0)*/)
						{
							CntIpInMsgBody++;
						}
					}

					for(k=msg;k<=dataLen-trueport_len;k++)
					{

						if(i_port != 0)
						{
							if(num_memcmp(&pData[k],trueport,trueport_len)==0)
							{
								CntPortInMsgBody++;
							}
						}
					}
					if ((trueip!=0) && (SameSubnet == TRUE))
					{
						for(k=msg;k<=dataLen-realip_len;k++)
						{			
							if ((num_memcmp(&pData[k],realip,realip_len)==0) /*&& (memcmp(&pData[k - ipkey_len -1], ipkey, ipkey_len) !=0)*/)
							{
								CntTrueIpInMsgbody++;
								//printk("CntTrueIpInMsgbody = %d\n", CntTrueIpInMsgbody);
							}
						}
					}
					
					if(i_port != 0)
						new_cl_len=sprintf(tempStr,"%u",datalen+(CntIpInMsgBody*(globalip_len-localip_len)) + CntPortInMsgBody*(rtpport_len - trueport_len)
										 + (CntTrueIpInMsgbody*(globalip_len - realip_len)));
					else
						new_cl_len=sprintf(tempStr,"%u",datalen+(CntIpInMsgBody*(globalip_len-localip_len)) + (CntTrueIpInMsgbody*(globalip_len - realip_len)));
					memcpy(&sptr[j],tempStr,new_cl_len);
					j+=(new_cl_len-1);
					ALG("[SIP OUT]CntIpInMsgBody = %d CntPortInMsgBody=%d, new_cl_len=%d\n", CntIpInMsgBody,CntPortInMsgBody,new_cl_len);
				}
				else //inbound
				{

					// count how many ip in Message body
					for(k=msg;k<=dataLen-globalip_len;k++)
					{			
						if((num_memcmp(&pData[k],globalip,globalip_len)==0) && (memcmp(&pData[k - ipkey_len -1], ipkey, ipkey_len) !=0) )
						{
							CntIpInMsgBody++;
						}
					}

					for(k=msg;k<=dataLen-globalip_len;k++)
					{			
						if((num_memcmp(&pData[k],globalip,globalip_len)==0) && (memcmp(&pData[k - ipkey_len -1], ipkey, ipkey_len) ==0) && (truelocalip !=0))
						{
							CntRealIpInMsgBody++;
						}
					}

					if (SameGw == FALSE)
						new_cl_len=sprintf(tempStr,"%u",datalen+(CntIpInMsgBody*(localip_len-globalip_len))+ (CntRealIpInMsgBody*(realip_len-globalip_len)));
					else if ((o_port != 0) && (trueport_num != 0))
						new_cl_len=sprintf(tempStr,"%u",datalen+(CntIpInMsgBody*(localip_len-globalip_len))+ (CntRealIpInMsgBody*(realip_len-globalip_len)) + (realrtpport_len-rtpport_len));
					else
						new_cl_len=sprintf(tempStr,"%u",datalen + (CntIpInMsgBody*(localip_len-globalip_len)));
					memcpy(&sptr[j],tempStr,new_cl_len);
					j+=(new_cl_len-1);				
				}
			}	
			else if((direct==NAPT_DIRECTION_OUTBOUND)&&(i+localip_len<=dataLen)&&(num_memcmp(&pData[i],localip,localip_len)==0) /*&& (memcmp(&pData[i - ipkey_len -1], ipkey, ipkey_len) !=0)*/)
			{
				
				memcpy(&sptr[j],globalip,globalip_len);
				i+=(localip_len-1);
				j+=(globalip_len-1);
				size_delta+=(globalip_len-localip_len);
				ALG("[SIP OUT] match local ip address = %s-->%s size_delta=%d\n",localip,globalip,size_delta);
			}

			else if((direct==NAPT_DIRECTION_OUTBOUND)&&(i+localport_len <= dataLen) && (num_memcmp(&pData[i], localport, localport_len) == 0) && (memcmp(&pData[i -localip_len -1], localip, localip_len) == 0))
			{
				
				memcpy(&sptr[j], globalport, globalport_len);
				i+=(localport_len-1);
				j+=(globalport_len-1);
				size_delta += (globalport_len-localport_len);
				ALG("[SIP OUT] match local port & local ip address = %s-->%s size_delta=%d\n",localport,globalport,size_delta);
			}
#if 0
			else if((direct==NAPT_DIRECTION_OUTBOUND)&&(i+controlport_len <= dataLen) && (control_port != 0) && (num_memcmp(&pData[i], controlport, controlport_len) == 0) && (memcmp(&pData[i -localip_len -1], localip, localip_len) == 0))
			{			
				
				memcpy(&sptr[j], controlextport, controlextport_len);
				i+=(controlport_len-1);
				j+=(controlextport_len-1);
				size_delta += (controlextport_len-controlport_len);
				ALG("[SIP OUT] match control port & local ip address = c-port:%d-->c-extport:%d size_delta=%d dataLen=%d\n",control_port,controlextport,size_delta,dataLen);
			}
#endif			
			else if((direct==NAPT_DIRECTION_OUTBOUND)&&(i+trueport_len <= dataLen) && (num_memcmp(&pData[i], trueport, trueport_len) == 0) && (i_port != 0) /*&& ((memcmp(&pData[i -keyword_len -1], keyword, keyword_len) == 0))*/)
			{
				
				memcpy(&sptr[j], rtpport, rtpport_len);
				//printk("change rtp port\n");
				i+=(trueport_len-1);
				j+=(rtpport_len-1);
				size_delta += (rtpport_len-trueport_len);
				ALG("[SIP OUT] match trueport = %s-->%s size_delta=%d\n",trueport,rtpport,size_delta);
			}
			else if((direct==NAPT_DIRECTION_OUTBOUND)&&(i+realip_len <= dataLen) && (num_memcmp(&pData[i], realip, realip_len) == 0) && (trueip != 0)  && (SameSubnet == TRUE)  && (memcmp(&pData[i - ipkey_len -1], ipkey, ipkey_len) ==0))
			{
				
				memcpy(&sptr[j], globalip, globalip_len);
				i+=(realip_len-1);
				j+=(globalip_len-1);
				size_delta += (globalip_len-realip_len);
				ALG("[SIP OUT] match real ip = %s-->%s size_delta=%d\n",realip,globalip,size_delta);
				//printk("modify true ip\n");
			}

			else if((direct==NAPT_DIRECTION_INBOUND)&&((i+globalip_len<=dataLen))&&(num_memcmp(&pData[i],globalip,globalip_len)==0) &&(memcmp(&pData[i - ipkey_len -1], ipkey, ipkey_len) !=0) )
			{
				ALG("[SIP IN] match ip in address(without [IP4]) = %s-->%s\n",globalip,localip);
				memcpy(&sptr[j],localip,localip_len);
				i+=(globalip_len-1);
				j+=(localip_len-1);
				size_delta+=(localip_len-globalip_len); 
					
			}
			else if((direct==NAPT_DIRECTION_INBOUND)&&((i+globalip_len<=dataLen))&&(num_memcmp(&pData[i],globalip,globalip_len)==0) &&(memcmp(&pData[i- ipkey_len -1], ipkey, ipkey_len) ==0) && (truelocalip !=0))
			{
				ALG("[SIP IN] match ip in address(with [IP4]) = %s-->%s\n",globalip,realip);
				memcpy(&sptr[j],realip,realip_len);
				i+=(globalip_len-1);
				j+=(realip_len-1);
				size_delta+=(realip_len-globalip_len);						
			}

			else if((direct==NAPT_DIRECTION_INBOUND)&&(i+globalport_len <= dataLen) && (num_memcmp(&pData[i], globalport, globalport_len) == 0))
			{
				ALG("[SIP IN] match global port = %s-->%s\n",globalport,localport);
				memcpy(&sptr[j], localport, localport_len);
				i+=(globalport_len-1);
				j+=(localport_len-1);
				size_delta += (localport_len-globalport_len);
			}
			else if((direct==NAPT_DIRECTION_INBOUND)&&(i+realport_len <= dataLen) && (num_memcmp(&pData[i], realport, realport_len) == 0) && (trueport_num != 0) && ((memcmp(&pData[i-rportkey_len], rportkey, rportkey_len) != 0)))
			{
				ALG("[SIP IN] find real port by [rport=] = %s-->%s\n",realport,localport);
				memcpy(&sptr[j], localport, localport_len);
				i+=(realport_len-1);
				j+=(localport_len-1);
				size_delta += (localport_len-realport_len);
			}
			else if((direct==NAPT_DIRECTION_INBOUND)&&(i+rtpport_len <= dataLen) && (num_memcmp(&pData[i], rtpport, rtpport_len) == 0) && (o_port != 0) && ((memcmp(&pData[i -keyword_len -1], keyword, keyword_len) == 0)) && (SameGw == TRUE) && (trueport_num != 0))
			{
				ALG("[SIP IN] match RTP port by [audio] = %s-->%s\n",rtpport,realrtpport);
				memcpy(&sptr[j], realrtpport, realrtpport_len);
				//printk("change rtp port\n");
				i+=(rtpport_len-1);
				j+=(realrtpport_len-1);
				size_delta += (realrtpport_len-rtpport_len);
			}
			
			else
			{
				sptr[j]=pData[i];
			}

		}

		ALG("size_delta=%d new_cl_len=%d old_cl_len=%d\n",size_delta,new_cl_len,old_cl_len);
		spip->tot_len = htons(ntohs(spip->tot_len)+size_delta+new_cl_len-old_cl_len); 
		sudp->len = htons(ntohs(sudp->len)+size_delta+new_cl_len-old_cl_len);	

		skb_put(skb, size_delta);
		memcpy(&skb->data[pPktHdr->l3Offset],(void*)spip,ntohs(spip->tot_len));
		//skb->len=pPktHdr->l3Offset+ntohs(spip->tot_len);		

		if(pPktHdr->egressTagif&PPPOE_TAGIF)
		{
			// set pppoe length = IP total length + 2
			skb->data[pPktHdr->l3Offset-4]=htons(ntohs(spip->tot_len)+2)>>8;
			skb->data[pPktHdr->l3Offset-3]=htons(ntohs(spip->tot_len)+2)&0xff;
			
			//update l3 length
			pPktHdr->l3Len = ntohs(spip->tot_len);
			*pPktHdr->pL3Len = spip->tot_len;

			//re-cal l3 checksum
			*pPktHdr->pIpv4Checksum=0;
			*pPktHdr->pIpv4Checksum=htons(inet_chksum(skb->data+pPktHdr->l3Offset,pPktHdr->l4Offset-pPktHdr->l3Offset));
		
			//re-cal l4 checksum
			*pPktHdr->pL4Checksum=0;
			*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,pPktHdr->l3Offset+(*pPktHdr->pL3Len)-pPktHdr->l4Offset,ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));
		}
		
		//memDump(skb->data,skb->len,"new sip out");

		//checksum re-calulate by HW
		//*pPktHdr->pL4Checksum=0;
		//*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,pPktHdr->l3Offset+pPktHdr->l3Len-pPktHdr->l4Offset,ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));
	
	}


	return 1;
}

int _rtk_rg_sip_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
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


	//do nothing before napt modification
	if(after == 0)
	{
		if(direct==NAPT_DIRECTION_INBOUND)
		{
			if(pConn->tuple.isIp6==0)
			{
				pConn->tuple.extIp.ip = ntohl(*pPktHdr->pIpv4Dip);
				pConn->tuple.extPort = ntohs(*pPktHdr->pDport);
			}		
		}
		return SUCCESS;
	}


	//extIP= egress gateway ip 
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

	ret = _rtk_rg_sip_process(direct, pAppData, appLen, pConn);
	if(ret < 1)
		return FAIL;
#endif	
	return ret;
}

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_sip(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
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
			//else
				//_rtk_rg_sync_tcp_ack_seq(direct, pPktHdr, pConn);
		}
	}
	else //after=1
	{
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{		
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
			
			pConn = _rtk_rg_alg_connection_find(&tuple);		
			if(pConn == NULL)
				return RG_FWDENGINE_ALG_RET_FAIL;
			
			ret = _rtk_rg_sip_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);	
		}
		else
		{
			//Attention, pPktHdr->ipv4Dip is not the original external ip after napt modification, it is the internal ip
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);
	
			pConn = _rtk_rg_alg_connection_find(&tuple);
			if(pConn == NULL)
				return RG_FWDENGINE_ALG_RET_FAIL;

			ret = _rtk_rg_sip_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);

			//_rtk_rg_sync_tcp_ack_seq(direct, pPktHdr, pConn);
		}
	}

#endif
	return RG_FWDENGINE_ALG_RET_SUCCESS;
}

#ifdef __KERNEL__
static int __init init(void)
{
	/* malloc global variable */
	if(init_sip)
		return SUCCESS;

	_rtl8651_l4_sip_alias_buf[0] = (uint8 *)rtk_rg_malloc(_RTL8651_L4_MAX_SIP_ALIAS_BUF_SIZE);
	//assert(_rtl8651_l4_sip_alias_buf[0]);

	init_sip = 1;
	return SUCCESS;
}


static void __exit exit(void)
{
}

module_init(init);
module_exit(exit);

#endif



