void *rtk_rg_malloc(int NBYTES);
void rtk_rg_free(void *APTR);

#include <rtk_rg_define.h>
#include <rtk_rg_internal.h>


/*add for move code from liteRomeDriver Boyce 2014-10-15*/
#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_callback.h>
#if defined(CONFIG_APOLLO)
//apollo
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#include <rtk_rg_xdsl_extAPI.h>
#endif

// OS-dependent defination
#ifndef CONFIG_APOLLO_MODEL
#ifdef __linux__
#include <linux/slab.h>
#include <linux/skbuff.h>

#ifdef CONFIG_DEFAULTS_KERNEL_3_18
#include <linux/kmod.h>
#endif


//Support Protocol Stack Rx packet dump.
#if defined(CONFIG_APOLLO)
int DumpProtocolStackRx_debug = 0;
#endif
int DumpProtocolStackTx_debug = 0;


int assert_eq(int func_return,int expect_return,const char *func,int line)
{
	if (func_return != expect_return) 
	{
		rtlglue_printf("\033[31;43m%s(%d): func_return=0x%x expect_return=0x%x, fail, so abort!\033[m\n",func, line,func_return,expect_return);
#ifdef CONFIG_RG_DEBUG		
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN)
		{
			rtlglue_printf("\033[1;33;41m");
			_rtk_rg_dump_stack();
			rtlglue_printf("\033[0m\n");
		}
#endif		
		
		return func_return; 
	}
	return 0;
}

void *rtk_rg_malloc(int NBYTES) 
{
	if(NBYTES==0) return NULL;
	return (void *)kmalloc(NBYTES,GFP_ATOMIC);
}
void rtk_rg_free(void *APTR) 
{
	kfree(APTR);
}

void _rtk_rg_dev_kfree_skb_any(struct sk_buff *skb)
{	
	if(skb==NULL)//the skb didn't allocate
		return;
	
	if((skb)&&(rg_db.systemGlobal.fwdStatistic))
	{
		rg_db.systemGlobal.statistic.perPortCnt_skb_free[rg_db.pktHdr->ingressPort]++;
	}
	dev_kfree_skb_any(skb);	
}

struct sk_buff *_rtk_rg_dev_alloc_skb(unsigned int length)
{
	struct sk_buff *skb;
#if RTK_RG_SKB_PREALLOCATE
	skb=_rtk_rg_getAlloc(SKB_BUF_SIZE);//get skb from pre-allocated pool	
#else	
	skb=dev_alloc_skb(length);
	if((skb)&&(rg_db.systemGlobal.fwdStatistic))
	{
		rg_db.systemGlobal.statistic.perPortCnt_skb_alloc[rg_db.pktHdr->ingressPort]++;
	}
#endif
	return skb;
}

struct sk_buff *_rtk_rg_getAlloc(unsigned int size)
{
	struct sk_buff *skb;
	skb=re8670_getAlloc(size);
	if((skb)&&(rg_db.systemGlobal.fwdStatistic))
	{
		rg_db.systemGlobal.statistic.perPortCnt_skb_pre_alloc_for_uc[rg_db.pktHdr->ingressPort]++;
	}
	return skb;	
}

struct sk_buff *_rtk_rg_skb_copy(const struct sk_buff *skb, unsigned gfp_mask)
{
	struct sk_buff *ret_skb;
	ret_skb=skb_copy(skb,(gfp_t)gfp_mask);
	if((ret_skb)&&(rg_db.systemGlobal.fwdStatistic))
	{
		rg_db.systemGlobal.statistic.perPortCnt_skb_alloc[rg_db.pktHdr->ingressPort]++;
	}
	return ret_skb; 
}

struct sk_buff *_rtk_rg_skb_clone(struct sk_buff *skb, unsigned gfp_mask)
{
	struct sk_buff *ret_skb;
	ret_skb=skb_clone(skb,(gfp_t)gfp_mask);
	if((ret_skb)&&(rg_db.systemGlobal.fwdStatistic))
	{
		rg_db.systemGlobal.statistic.perPortCnt_skb_alloc[rg_db.pktHdr->ingressPort]++;
	}
	return ret_skb; 
}

//This function is not copy SKB, only be used for TX path.
struct sk_buff *rtk_rg_skbCopyToPreAllocSkb(struct sk_buff *skb)
{
	struct sk_buff *new_skb;
#if RTK_RG_SKB_PREALLOCATE
	if(skb->data[0]&1)
	{
		if(skb->data[0]==0xff)			
			new_skb=re8670_getBcAlloc(SKB_BUF_SIZE);
		else
			new_skb=re8670_getMcAlloc(SKB_BUF_SIZE);		

		if(new_skb==NULL) return NULL;
		if(rg_db.systemGlobal.fwdStatistic)
		{	
			rg_db.systemGlobal.statistic.perPortCnt_skb_pre_alloc_for_mc_bc[rg_db.pktHdr->ingressPort]++;
		}		
	}	
	else
	{
		new_skb=_rtk_rg_getAlloc(SKB_BUF_SIZE);
		if(new_skb==NULL) return NULL;
	}
		

	skb_put(new_skb, (skb->len <= SKB_BUF_SIZE) ? skb->len : SKB_BUF_SIZE);
	memcpy(new_skb->data,skb->data,(skb->len <= SKB_BUF_SIZE) ? skb->len : SKB_BUF_SIZE);
#else
	//bcSkb = dev_alloc_skb(skb->len);
	new_skb=_rtk_rg_skb_copy(skb,GFP_ATOMIC);
#endif
	return new_skb;
}


#endif


int32 _rtk_rg_platform_function_register_failed(void){
	FIXME("pf.function hasn't registered! (Be careful the bootup WARNING message.)\n");
	RETURN_ERR(RT_ERR_RG_DRIVER_NOT_SUPPORT);
}


int32 _rtk_rg_platform_function_register_check(struct platform *pf){
	int i;
	for(i=0;i < sizeof(struct platform)/sizeof(uint32);i++){
		if((uint32)(*((uint32*)pf+i))==0x0){
			*((uint32*)pf+i) = (uint32)&_rtk_rg_platform_function_register_failed;
			FIXME("pf.function[%d] hasn't registered!!!",i);
		}
	}
	
	return (RT_ERR_RG_OK);
}

#if 1
//#ifdef CONFIG_APOLLO_RLE0371
#define IP_PROTO_TCP 6
#define IP_PROTO_UDP 17
#define _RTK_RG_CHM _rtk_rg_standard_chksum

static u16
_rtk_rg_standard_chksum(u8 *dataptr, u16 len)
{
  u32 acc;
  u16 src;
  u8 *octetptr;

  acc = 0;

  octetptr = (u8*)dataptr;
  while (len > 1)
  {

    src = (*octetptr) << 8;
    octetptr++;
    src |= (*octetptr);
    octetptr++;
    acc += src;
    len -= 2;
  }

  if (len > 0)
  {
    src = (*octetptr) << 8;
    acc += src;
  }

  acc = (acc >> 16) + (acc & 0x0000ffffUL);
  while ((acc & 0xffff0000) != 0) {
    acc = (acc >> 16) + (acc & 0x0000ffffUL);
  }

  return htons((u16)acc);
}

u16 inet_chksum(u8 *dataptr, u16 len)
{
  u32 acc;

  acc = _RTK_RG_CHM(dataptr, len);
  while (acc >> 16) {
    acc = (acc & 0xffff) + (acc >> 16);
  }
  return (u16)~(acc & 0xffff);
}

u16 inet_chksum_pseudo(u8 *tcphdr, u16 tcplen,
       u32 srcip, u32 destip,  u8 proto)
{
	u32 acc;
	u8 swapped;

	acc = 0;
	swapped = 0;
	/* iterate through all pbuf in chain */

	acc += _RTK_RG_CHM(tcphdr, tcplen);

	while (acc >> 16) 
	{
		acc = (acc & 0xffffUL) + (acc >> 16);
	}
	if (tcplen % 2 != 0) 
	{
		swapped = 1 - swapped;
		acc = ((acc & 0xff) << 8) | ((acc & 0xff00UL) >> 8);
	}

	if (swapped) 
	{
		acc = ((acc & 0xff) << 8) | ((acc & 0xff00UL) >> 8);
	}

	acc += (srcip & 0xffffUL);
	acc += ((srcip >> 16) & 0xffffUL);
	acc += (destip & 0xffffUL);
	acc += ((destip>> 16) & 0xffffUL);	
	acc += (u32)htons((u16)proto);
	acc += (u32)htons(tcplen);
	while (acc >> 16) 
	{
		acc = (acc & 0xffffUL) + (acc >> 16);
	}
	return (u16)~(acc & 0xffffUL);

}


//Checksum update functions
uint16 _rtk_rg_fwdengine_L4checksumUpdate(uint8 acktag, uint16 ori_checksum, uint32 ori_ip, uint16 ori_port, uint32 ori_seq, uint32 ori_ack, uint32 new_ip, uint16 new_port, uint32 new_seq, uint32 new_ack)
{
	uint32 tmp_chksum;
	//uint16 ori_ttlProto,new_ttlProto;
	//DEBUG("ori chksum = %x",ori_checksum);

	//IP
	if(((ori_ip&0xffff0000)^(new_ip&0xffff0000))!=0)
	{
		//DEBUG("1");
		tmp_chksum = ((~ori_checksum)&0xffff) + (((~ori_ip)&0xffff0000)>>16) + (new_ip>>16);
		//DEBUG("chksum = %x",tmp_chksum);
		tmp_chksum += (((~ori_ip)&0xffff) + (new_ip&0xffff));
		//DEBUG("chksum = %x",tmp_chksum);
	}
	else 
	{
		//DEBUG("2");
		tmp_chksum = ((~ori_checksum)&0xffff) + ((~ori_ip)&0xffff) + (new_ip&0xffff);
		//DEBUG("chksum = %x",tmp_chksum);
	}

	//SEQ
	if(ori_seq!=new_seq)
	{
		if(((ori_seq&0xffff0000)^(new_seq&0xffff0000))!=0)
		{
			//DEBUG("1");
			tmp_chksum += (((~ori_seq)&0xffff0000)>>16) + (new_seq>>16);
			//DEBUG("chksum = %x",tmp_chksum);
			tmp_chksum += (((~ori_seq)&0xffff) + (new_seq&0xffff));
			//DEBUG("chksum = %x",tmp_chksum);
		}
		else 
		{
			//DEBUG("2");
			tmp_chksum += ((~ori_seq)&0xffff) + (new_seq&0xffff);
			//DEBUG("chksum = %x",tmp_chksum);
		}
	}

	//ACK
	if(acktag && ori_ack!=new_ack)
	{
		if(((ori_ack&0xffff0000)^(new_ack&0xffff0000))!=0)
		{
			//DEBUG("1");
			tmp_chksum += (((~ori_ack)&0xffff0000)>>16) + (new_ack>>16);
			//DEBUG("chksum = %x",tmp_chksum);
			tmp_chksum += (((~ori_ack)&0xffff) + (new_ack&0xffff));
			//DEBUG("chksum = %x",tmp_chksum);
		}
		else 
		{
			//DEBUG("2");
			tmp_chksum += ((~ori_ack)&0xffff) + (new_ack&0xffff);
			//DEBUG("chksum = %x",tmp_chksum);
		}
	}
	
	//PORT
	if(ori_port!=new_port)
	{
		//DEBUG("3");
		tmp_chksum += (((~ori_port)&0xffff) + new_port);
		//DEBUG("chksum = %x",tmp_chksum);
	}
	//L4Chksum didn't contain TTL field!!
	while (tmp_chksum >> 16) 
	{
		tmp_chksum = (tmp_chksum & 0xffffUL) + (tmp_chksum >> 16);
	}
	
	return ~(tmp_chksum&0xffff);
}

uint16 _rtk_rg_fwdengine_L4checksumUpdateForMss(uint16 ori_checksum, uint16 ori_mss, uint16 new_mss)
{
	uint32 tmp_chksum=((~ori_checksum)&0xffff);
	if(ori_mss!=new_mss)
	{
		tmp_chksum += (((~ori_mss)&0xffff) + new_mss);
		while (tmp_chksum >> 16) 
		{
			tmp_chksum = (tmp_chksum & 0xffffUL) + (tmp_chksum >> 16);
		}
		return ~(tmp_chksum&0xffff);
	}
	return ori_checksum;
	
}

uint16 _rtk_rg_fwdengine_L3checksumUpdate(uint16 ori_checksum, uint32 ori_sip, uint8 ori_ttl, uint8 ori_protocol, uint32 new_sip, uint8 new_ttl)
{
	uint32 tmp_chksum;
	uint16 ori_ttlProto,new_ttlProto;
	//DEBUG("ori chksum = %x, oriSip is %x, oriTTL is %x, oriProto is %x, newSip is %x, newTTL is %x",ori_checksum,ori_sip,ori_ttl,ori_protocol,new_sip,new_ttl);

	if(((ori_sip&0xffff0000)^(new_sip&0xffff0000))!=0)
	{
		//DEBUG("1");
		tmp_chksum = ((~ori_checksum)&0xffff) + ((~ori_sip)>>16) + (new_sip>>16);
		//DEBUG("chksum = %x",tmp_chksum);
		tmp_chksum += (((~ori_sip)&0xffff) + (new_sip&0xffff));
		//DEBUG("chksum = %x",tmp_chksum);
	}
	else
	{
		//DEBUG("2");
		tmp_chksum = ((~ori_checksum)&0xffff) + ((~ori_sip)&0xffff) + (new_sip&0xffff);
		//DEBUG("chksum = %x",tmp_chksum);
	}

	if(ori_ttl!=new_ttl)
	{
		//DEBUG("3");
		ori_ttlProto=(ori_ttl<<8)+ori_protocol;
		new_ttlProto=(new_ttl<<8)+ori_protocol;
		tmp_chksum += ((~ori_ttlProto)&0xffff);
		tmp_chksum += new_ttlProto;
		//DEBUG("chksum = %x",tmp_chksum);
	}

	while (tmp_chksum >> 16) 
	{
		tmp_chksum = (tmp_chksum & 0xffffUL) + (tmp_chksum >> 16);
	}
	
	return ~(tmp_chksum&0xffff);
}

uint16 _rtk_rg_fwdengine_L3checksumUpdateDSCP(uint16 ori_checksum, uint8 header_length, uint8 ori_DSCP, uint8 new_DSCP)
{
	uint32 tmp_chksum;
	uint16 ori_version_IHL_DSCP,new_version_IHL_DSCP;
	//DEBUG("ori chksum = %x, oriSip is %x, oriTTL is %x, oriProto is %x, newSip is %x, newTTL is %x",ori_checksum,ori_sip,ori_ttl,ori_protocol,new_sip,new_ttl);

	if(ori_DSCP!=new_DSCP)
	{
		//DEBUG("3");
		ori_version_IHL_DSCP=0x4000+(header_length<<6)+ori_DSCP;
		new_version_IHL_DSCP=0x4000+(header_length<<6)+new_DSCP;
		tmp_chksum = ((~ori_checksum)&0xffff) + ((~ori_version_IHL_DSCP)&0xffff) + (new_version_IHL_DSCP&0xffff);
		//DEBUG("chksum = %x",tmp_chksum);
	}
	else
		return ori_checksum;

	while (tmp_chksum >> 16) 
	{
		tmp_chksum = (tmp_chksum & 0xffffUL) + (tmp_chksum >> 16);
	}
	
	return ~(tmp_chksum&0xffff);
}

#endif


int _rtk_rg_wlanDeviceCount_dec(int wlan_idx, unsigned char *macAddr, int *dev_idx)
{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	int i,idx;
#endif
	if(wlan_idx==0)
	{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		for(i=rg_db.wlanMbssidHeadIdx;i<rg_db.wlanMbssidHeadIdx+MAX_WLAN_MBSSID_SW_TABLE_SIZE;i++)
		{
			idx=i%MAX_WLAN_MBSSID_SW_TABLE_SIZE;
			if(memcmp(rg_db.wlanMbssid[idx].mac.octet,macAddr,ETHER_ADDR_LEN)==0) //the MAC is finded in table.
			{
				atomic_dec(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[rg_db.wlanMbssid[idx].wlan_dev_idx]);
				if(dev_idx!=NULL)*dev_idx=rg_db.wlanMbssid[idx].wlan_dev_idx;
				return (RT_ERR_RG_OK);			
			}
		}
#endif
	}
	
	return (RT_ERR_RG_OK);
}

//do callback in workqueue
#ifdef __KERNEL__
void wq_do_initByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_initByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_initByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.initByHwCallBack();
	kfree(p_callback_work);
}
void wq_do_arpAddByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_arpAddByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_arpAddByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.arpAddByHwCallBack(&p_callback_work->arpInfo);
	kfree(p_callback_work);
}
void wq_do_arpDelByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_arpDelByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_arpDelByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.arpDelByHwCallBack(&p_callback_work->arpInfo);
	kfree(p_callback_work);

}
void wq_do_macAddByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_macAddByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_macAddByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.macAddByHwCallBack(&p_callback_work->macInfo);
	kfree(p_callback_work);
}
void wq_do_macDelByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_macDelByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_macDelByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.macDelByHwCallBack(&p_callback_work->macInfo);
	kfree(p_callback_work);
}
void wq_do_routingAddByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_routingAddByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_routingAddByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.routingAddByHwCallBack(&p_callback_work->v4RoutingInfo);
	kfree(p_callback_work);
}
void wq_do_routingDelByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_routingDelByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_routingDelByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.routingDelByHwCallBack(&p_callback_work->v4RoutingInfo);
	kfree(p_callback_work);
}
void wq_do_naptAddByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_naptAddByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_naptAddByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.naptAddByHwCallBack(&p_callback_work->naptInfo);
	kfree(p_callback_work);
}
void wq_do_naptDelByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_naptDelByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_naptDelByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.naptDelByHwCallBack(&p_callback_work->naptInfo);
	kfree(p_callback_work);
}
void wq_do_bindingAddByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_bindingAddByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_bindingAddByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.bindingAddByHwCallBack(&p_callback_work->bindInfo);
	kfree(p_callback_work);
}
void wq_do_bindingDelByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_bindingDelByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_bindingDelByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.bindingDelByHwCallBack(&p_callback_work->bindInfo);
	kfree(p_callback_work);
}
void wq_do_interfaceAddByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_interfaceAddByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_interfaceAddByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.interfaceAddByHwCallBack(&p_callback_work->intfInfo,&p_callback_work->intfIdx);
	kfree(p_callback_work);
}
void wq_do_interfaceDelByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_interfaceDelByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_interfaceDelByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.interfaceDelByHwCallBack(&p_callback_work->intfInfo,&p_callback_work->intfIdx);
	kfree(p_callback_work);
}
void wq_do_neighborAddByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_neighborAddByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_neighborAddByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.neighborAddByHwCallBack(&p_callback_work->neighborInfo);
	kfree(p_callback_work);
}
void wq_do_neighborDelByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_neighborDelByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_neighborDelByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.neighborDelByHwCallBack(&p_callback_work->neighborInfo);
	kfree(p_callback_work);
}
void wq_do_v6RoutingAddByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_v6RoutingAddByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_v6RoutingAddByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.v6RoutingAddByHwCallBack(&p_callback_work->v6RoutingInfo);
	kfree(p_callback_work);
}
void wq_do_v6RoutingDelByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_v6RoutingDelByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_v6RoutingDelByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.v6RoutingDelByHwCallBack(&p_callback_work->v6RoutingInfo);
	kfree(p_callback_work);
}
void wq_do_pppoeBeforeDiagByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_pppoeBeforeDiagByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_pppoeBeforeDiagByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.pppoeBeforeDiagByHwCallBack(&p_callback_work->pppoeBeforeInfo,&p_callback_work->wanIdx);
	kfree(p_callback_work);
}
void wq_do_dhcpRequestByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_dhcpRequestByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_dhcpRequestByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.dhcpRequestByHwCallBack(&p_callback_work->wanIdx);
	kfree(p_callback_work);
}
void wq_do_pptpBeforeDialByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_pptpBeforeDialByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_pptpBeforeDialByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.pptpBeforeDialByHwCallBack(&p_callback_work->pptpBeforeInfo,&p_callback_work->wanIdx);
	kfree(p_callback_work);
}
void wq_do_l2tpBeforeDialByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_l2tpBeforeDialByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_l2tpBeforeDialByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.l2tpBeforeDialByHwCallBack(&p_callback_work->l2tpBeforeInfo,&p_callback_work->wanIdx);
	kfree(p_callback_work);
}
void wq_do_pppoeDsliteBeforeDialByHwCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_pppoeDsliteBeforeDialByHwCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_pppoeDsliteBeforeDialByHwCallBack_t, work);
	rg_db.systemGlobal.initParam.pppoeDsliteBeforeDialByHwCallBack(&p_callback_work->pppoeDsliteBeforeInfo,&p_callback_work->wanIdx);
	kfree(p_callback_work);
}
#if 0
void wq_do_softwareNaptAddCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_softwareNaptAddCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_softwareNaptAddCallBack_t, work);
	rg_db.systemGlobal.initParam.softwareNaptInfoAddCallBack(&p_callback_work->naptInfo);
	kfree(p_callback_work);
}
void wq_do_softwareNaptDelCallBack(struct work_struct *p_work)
{
	rtk_rg_wq_softwareNaptDelCallBack_t *p_callback_work = container_of(p_work, rtk_rg_wq_softwareNaptDelCallBack_t, work);
	rg_db.systemGlobal.initParam.softwareNaptInfoDeleteCallBack(&p_callback_work->naptInfo);
	kfree(p_callback_work);
}
#endif
#endif

static char cmd_buff[CB_CMD_BUFF_SIZE];
static char env_PATH[CB_CMD_BUFF_SIZE];
int rtk_rg_callback_pipe_cmd(const char *comment, ...) {

	char * envp[]={	//element size 3
		"HOME=/",
		env_PATH,
		NULL
	};
	char * argv[]={ //element size 4
		"/bin/ash",
		"-c",
		cmd_buff,
		NULL
	};
	int idx, retval;
	va_list argList;
	va_start(argList, comment);
	snprintf( env_PATH, CB_CMD_BUFF_SIZE, "PATH=%s", CONFIG_RG_CALLBACK_ENVIRONMENT_VARIABLE_PATH);
	//sprintf( cmd_buff, comment, ##arg);
	vsprintf(cmd_buff, comment, argList);
	//rtlglue_printf("\033[1;35mcallback_pipe_cmd cmd_buff=[%s]\n\033[0m", cmd_buff);
	if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_CALLBACK){
		rtlglue_printf("[rg callback]CMD:");
		for(idx=0;argv[idx]!=NULL;idx++){rtlglue_printf("\033[1;33m%s \033[0m",argv[idx]);}
		if((retval=call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC))==0){rtlglue_printf("\n");}
		else {rtlglue_printf("\033[1;35m [Exec Failed]\033[0m, ret=%d @%s,line:%d\n",retval,__func__,__LINE__);}
	}else{
		retval=call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
	}
	va_end(argList);
	return retval;
}


//==========================  ==============================
/*move from (xdsl/apollo)_litRomeDriver.c Boyce 2014-10-24*/
//==========================  ==============================
int _rtk_rg_eiplookup(ipaddr_t ip)
{
#if 0 //CONFIG_APOLLO_MODEL
	return 0;
#else
 	int i;

	for(i=0;i<MAX_EXTIP_SW_TABLE_SIZE;i++)
	{
		if(rg_db.extip[i].rtk_extip.valid)
		{		
			if(rg_db.extip[i].rtk_extip.extIpAddr==ip)
			{
				return i; //wan interface
			}
		}
	}
	return -1; 
#endif
}


__SRAM_FWDENG_SLOWPATH
int32 _rtK_rg_checkCategoryPortmask_spa(rtk_rg_port_idx_t spa)
{		
	if(((0x1<<spa)&CONFIG_RG_ACCESSWAN_CATEGORY_PORTMASK)==0)	//unmatch
		return RT_ERR_RG_ACCESSWAN_NOT_LAN;
	
	return SUCCESS;
}

int32 _rtk_rg_checkPortNotExistByPhy(rtk_rg_port_idx_t port)
{
	return ((0x1<<port)&rg_db.systemGlobal.phyPortStatus)?0:1;
}

void _rtk_rg_cleanPortAndProtocolSettings(rtk_port_t port)
{
	int ret;
	rtk_vlan_protoVlanCfg_t protoVlanCfg;

	//Clean all Group of port-and-protocol based VID
	protoVlanCfg.valid=0;
	protoVlanCfg.vid=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
	protoVlanCfg.pri=0;			//FIXME: should I change this?
	protoVlanCfg.dei=0;
	ret = RTK_VLAN_PORTPROTOVLAN_SET(port,RG_IPV4_GROUPID,&protoVlanCfg);
	assert_ok(ret);

	protoVlanCfg.valid=0;
	protoVlanCfg.vid=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
	protoVlanCfg.pri=0;			//FIXME: should I change this?
	protoVlanCfg.dei=0;
	ret = RTK_VLAN_PORTPROTOVLAN_SET(port,RG_ARP_GROUPID,&protoVlanCfg);
	assert_ok(ret);

	protoVlanCfg.valid=0;
	protoVlanCfg.vid=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
	protoVlanCfg.pri=0;			//FIXME: should I change this?
	protoVlanCfg.dei=0;
	ret = RTK_VLAN_PORTPROTOVLAN_SET(port,RG_IPV6_GROUPID,&protoVlanCfg);
	assert_ok(ret);
}


void _rtk_rg_wanVlanTagged(rtk_rg_pktHdr_t *pPktHdr,int vlan_tag_on)
{
	int VLANId;
	VLANId=pPktHdr->egressVlanID;
	//DEBUG("the VLANId decide in _rtk_rg_interfaceVlanTagged is %d",VLANId);
	if(rg_db.vlan[VLANId].fidMode==VLAN_FID_IVL)
	{
		//tag/untag by VLAN untag setting
		if(rg_db.vlan[VLANId].UntagPortmask.bits[0]&rg_kernel.txDesc.opts3.bit.tx_portmask)
		{
			//DEBUG("IVL:untagged!");
			pPktHdr->egressVlanTagif=0;
		}
		else
		{
			//DEBUG("IVL:tagged!");
			pPktHdr->egressVlanTagif=1;
		}
	}
	else
	{
		//2 FIXME:Should we check DMAC2CVID settings here?
		//Vlan tagged or not by interface setting
		pPktHdr->egressVlanTagif=vlan_tag_on;
	}
}

#if 0
int _rtk_rg_portBindingLookup(int srcPort, int srcExtPort, int vid, rtk_rg_sipDipClassification_t sipDipClass, rtk_rg_pktHdr_t *pPktHdr)
{
	int i;
	//src port binding
	for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
	{
		if(rg_db.bind[i].valid==1)			
		{			
			int wanTypeIdx=rg_db.bind[i].rtk_bind.wanTypeIdx;
			int match=0;
			//DEBUG("srcPort is %d, bt[%d].portMask is %x, vid is %d, vidLan is %d",srcPort,i,rg_db.bind[i].rtk_bind.portMask,vid,rg_db.bind[i].rtk_bind.vidLan);
			if(srcPort==RTK_RG_PORT_CPU)
			{
				if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(1<<srcExtPort)) 
				{
					if(rg_db.bind[i].rtk_bind.vidLan==vid) match=1;
				}
			}
			else			
			{
				if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(1<<srcPort))
				{
					if(rg_db.bind[i].rtk_bind.vidLan==vid) match=1;
				}
			}

			if(match==1)
			{
				if((sipDipClass==SIP_DIP_CLASS_NAPT) && (rg_db.wantype[wanTypeIdx].rtk_wantype.wanType==L34_WAN_TYPE_L34NAT_ROUTE))
				{
					DEBUG("L4 binding look up success! nhidx = %d",rg_db.wantype[wanTypeIdx].rtk_wantype.nhIdx);
					pPktHdr->bindingDecision=RG_BINDING_LAYER4;
					return rg_db.wantype[wanTypeIdx].rtk_wantype.nhIdx; //nextHopIdx for pPktHdr->bindNextHopIdx		
				}
				else if((sipDipClass==SIP_DIP_CLASS_ROUTING) && (rg_db.wantype[wanTypeIdx].rtk_wantype.wanType==L34_WAN_TYPE_L3_ROUTE))
				{					
					DEBUG("L3 binding look up success! nhidx = %d",rg_db.wantype[wanTypeIdx].rtk_wantype.nhIdx);
					pPktHdr->bindingDecision=RG_BINDING_LAYER3;
					return rg_db.wantype[wanTypeIdx].rtk_wantype.nhIdx; //nextHopIdx for pPktHdr->bindNextHopIdx
				}	
				else
				{
					FIXME("Unmatched binding action!\n");
				}
			}
		}		
	}		
	//DEBUG("binding look up failed!!");
	pPktHdr->bindingDecision=RG_BINDING_FINISHED;
	return FAIL;
}
#endif


int _rtk_rg_broadcastWithDscpRemarkMask_get(unsigned int bcMask,unsigned int *bcWithoutDscpRemarMask,unsigned int *bcWithDscpRemarByInternalpriMask,unsigned int *bcWithDscpRemarByDscpkMask)
{
	
	/*s
	sk into two masks, onr for dscp remarking, one for no need to remarking */
	int i;
	(*bcWithDscpRemarByDscpkMask) = 0x0;
	(*bcWithDscpRemarByInternalpriMask) = 0x0;
	(*bcWithoutDscpRemarMask) = 0x0;
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++){
		if(bcMask&(1<<i)){
			if(rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[i]==ENABLED_DSCP_REMARK_AND_SRC_FROM_INTERNALPRI){
				(*bcWithDscpRemarByInternalpriMask) |= (1<<i);
			}else if(rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[i]==ENABLED_DSCP_REMARK_AND_SRC_FROM_DSCP){
				(*bcWithDscpRemarByDscpkMask) |= (1<<i);
			}else{
				(*bcWithoutDscpRemarMask) |= (1<<i);
			}
		}
	}
	return SUCCESS;
}


int _rtk_rg_broadcastWithDot1pRemarkMask_get(unsigned int bcMask,unsigned int *bcWithDot1pRemarkMask,unsigned int *bcWithoutDot1pRemarkMask)
{
	/*seperate broadcast/multicast portMask into two masks, onr for dot1p remarking, one for no need to remarking */
	int i;
	(*bcWithDot1pRemarkMask) = 0x0;
	(*bcWithoutDot1pRemarkMask) = 0x0;
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++){
		if(bcMask&(1<<i)){
			if(rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[i]){
				(*bcWithDot1pRemarkMask) |= (1<<i);
			}else{
				(*bcWithoutDot1pRemarkMask) |= (1<<i);
			}
		}
	}
	return SUCCESS;
}

#if defined(CONFIG_RG_IGMP_SNOOPING)|| defined(CONFIG_RG_MLD_SNOOPING)

int32 _rtk_rg_igmpReport_ingress_portmask_check_and_limit(rtk_rg_pktHdr_t *pPktHdr, uint32* egress_port_mask){
#if 0
	//patch for user force control IGMP report forward ports.  
	// *egress_port_mask should bring the original egress portmask, and here will return the check result. If result set to 0x0 means no need to send!
	if ( pPktHdr != NULL  && rg_db.systemGlobal.initParam.igmpSnoopingEnable && (pPktHdr->tagif&IGMP_TAGIF)  &&  (pPktHdr->IGMPType == 0x12/*IGMPv2 report*/  ||  pPktHdr->IGMPType == 0x16/*IGMPv3 report*/) ) 
	{	
#ifdef CONFIG_DUALBAND_CONCURRENT
		if(pPktHdr->internalVlanID
		if(pPktHdr->egressVlanTagif==1 && 
			pPktHdr->egressVlanID==CONFIG_DEFAULT_TO_SLAVE_GMAC_VID &&
			pPktHdr->egressPriority==CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI){
			//check packet to slave wifi
			if((rg_db.systemGlobal.igmpReportPortmask&(1<<RTK_RG_EXT_PORT1))==0x0){
				TRACE("IGMP report forward to slave wifi is limited");
				*egress_port_mask = 0x0;
				return SUCCESS;
			}
		}
		else
		{
#endif
			//check packet to other physical port or master wifi
			*egress_port_mask &= rg_db.systemGlobal.igmpReportPortmask;//modified the portmask by proc/rg/igmp_report_filter_portmask
			TRACE("IGMP report forward port limit to 0x%x",*egress_port_mask);
			return SUCCESS;
#ifdef CONFIG_DUALBAND_CONCURRENT			
		}
#endif
	}	
#endif
	return SUCCESS;
}


int32 _rtk_rg_igmpReport_portmask_check_and_limit(rtk_rg_pktHdr_t *pPktHdr, uint32* egress_port_mask){

	//patch for user force control IGMP report forward ports.  
	// *egress_port_mask should bring the original egress portmask, and here will return the check result. If result set to 0x0 means no need to send!
	if ( pPktHdr != NULL  && rg_db.systemGlobal.initParam.igmpSnoopingEnable && (pPktHdr->tagif&IGMP_TAGIF)  &&  (pPktHdr->IGMPType == 0x12/*IGMPv2 report*/  ||  pPktHdr->IGMPType == 0x16/*IGMPv3 report*/) ) 
	{	
#ifdef CONFIG_DUALBAND_CONCURRENT		
		if(pPktHdr->egressVlanTagif==1 && 
			pPktHdr->egressVlanID==CONFIG_DEFAULT_TO_SLAVE_GMAC_VID &&
			pPktHdr->egressPriority==CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI){
			//check packet to slave wifi
			if((rg_db.systemGlobal.igmpReportPortmask&(1<<RTK_RG_EXT_PORT1))==0x0){
				TRACE("IGMP report forward to slave wifi is limited");
				*egress_port_mask = 0x0;
				return SUCCESS;
			}
		}
		else
		{
#endif
			//check packet to other physical port or master wifi
			*egress_port_mask &= rg_db.systemGlobal.igmpReportPortmask;//modified the portmask by proc/rg/igmp_report_filter_portmask
			TRACE("IGMP report forward port limit to 0x%x",*egress_port_mask);
			return SUCCESS;
#ifdef CONFIG_DUALBAND_CONCURRENT			
		}
#endif
	}	

	return SUCCESS;
}

#endif


#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
int32 _rtk_rg_igmpMldQuery_portmask_check_and_limit(rtk_rg_pktHdr_t *pPktHdr, uint32* egress_port_mask){

	//patch for user force control IGMP/MLD query from igmpSnooping forward ports.  
	// *egress_port_mask should bring the original egress portmask, and here will return the check result. If result set to 0x0 means no need to send!
	if ( (pPktHdr != NULL)  && 
		(rg_db.systemGlobal.initParam.igmpSnoopingEnable) && 
		(pPktHdr->ingressLocation==RG_IGR_IGMP_OR_MLD) && //only from igmpSnooping module
		(((pPktHdr->tagif&IGMP_TAGIF)&&(pPktHdr->IGMPType == 0x11/*IGMP query*/)) || ((pPktHdr->tagif&IPV6_MLD_TAGIF)&&(pPktHdr->IGMPv6Type==130/*Multicast Listener Query*/)))
		)
	{	
#ifdef CONFIG_DUALBAND_CONCURRENT		
		if(pPktHdr->egressVlanTagif==1 && 
			pPktHdr->egressVlanID==CONFIG_DEFAULT_TO_SLAVE_GMAC_VID &&
			pPktHdr->egressPriority==CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI){
			//check packet to slave wifi
			if((rg_db.systemGlobal.igmpMldQueryPortmask&(1<<RTK_RG_EXT_PORT1))==0x0){
				TRACE("IGMP Query forward to slave wifi is limited");
				*egress_port_mask = 0x0;
				return SUCCESS;
			}
		}
		else
		{
#endif
			//check packet to other physical port or master wifi
			*egress_port_mask &= rg_db.systemGlobal.igmpMldQueryPortmask;//modified the portmask by proc/rg/igmp_report_filter_portmask
			TRACE("IGMP Query forward port limit to 0x%x",*egress_port_mask);
			return SUCCESS;
#ifdef CONFIG_DUALBAND_CONCURRENT			
		}
#endif
	}	
	
	else
	{
		//IGMP("Not IGMP or MLD query!");
	}


	return SUCCESS;
}

#endif

int _rtk_rg_getPortLinkupStatus(void)
{
	// call func from _rtk_rg_initParam_set, all procedure has been protected by spinlock(rgApiLock).
	rtk_rg_portStatusInfo_t portInfo;
	int i = 0;

	if(pf.rtk_rg_portStatus_get != NULL)
	{
		for (i = 0; i< RTK_RG_MAC_PORT_MAX; i++){
			if((rg_db.systemGlobal.phyPortStatus & (1<<i)) == 0)	// phisical port is not exist
				continue;
			//rtk_rg_portStatus_get(i, &portInfo);
			pf.rtk_rg_portStatus_get(i, &portInfo);
			rg_db.portLinkupMask |= (portInfo.linkStatus<<i);
		}
		rg_db.portLinkStatusInitDone = TRUE;
	}
	DEBUG("Port linkup status: init done(%s), portmask = 0x%x", rg_db.portLinkStatusInitDone?"O":"X", rg_db.portLinkupMask);
	return RT_ERR_OK;
}


/* Return FAIL: means no needed to do transmition, so skip furthur process*/
int _rtk_rg_egressPortMaskCheck(rtk_rg_pktHdr_t *pPktHdr, unsigned int bcTxMask, unsigned int allDestPortMask)
{
	if (pPktHdr->egressUniPortmask != 0)	// if hit ACL/CF force forward
	{
		unsigned int filterPortMask = 0;
		unsigned int oriUniPortMask = pPktHdr->egressUniPortmask;
		pPktHdr->egressUniPortmask &= ~bcTxMask;
		filterPortMask = pPktHdr->egressUniPortmask & allDestPortMask;
		pPktHdr->egressUniPortmask = oriUniPortMask & (~filterPortMask) & bcTxMask;
		if (pPktHdr->egressUniPortmask == 0){		// force forward ports didn't include self, skip transmission because force forward ports will handle Tx by self.
			TRACE("Portmask [0x%x] **STOP** sending packet. Update Egress port mask from 0x%x to 0x%x.", bcTxMask, oriUniPortMask, pPktHdr->egressUniPortmask);
			return FAIL;
		}else
			TRACE("Portmask [0x%x] Update Egress port mask from 0x%x to 0x%x", bcTxMask, oriUniPortMask, pPktHdr->egressUniPortmask);
	} else {	// Normal case: SKIP sending if Tx port is not belong to member set
		if(pPktHdr->fwdDecision==RG_FWD_DECISION_PPPOE_MC){
			if ((pPktHdr->multicastMacPortMask.portmask & bcTxMask) == 0) {
				TRACE("Portmask [0x%x] **STOP** sending packet. PPPoE Multicast, member set = 0x%x", bcTxMask, pPktHdr->multicastMacPortMask.portmask);
				return FAIL;
			}
		} else if ( ((rg_db.vlan[pPktHdr->internalVlanID].MemberPortmask.bits[0] & bcTxMask) == 0)) {
			// Not Forcefwd && specific port is not belong to member port.
			TRACE("Portmask [0x%x] **STOP** sending packet. VLAN ID = %d, member set = 0x%x", bcTxMask, pPktHdr->internalVlanID, rg_db.vlan[pPktHdr->internalVlanID].MemberPortmask.bits[0]);
			return FAIL;
		}
	}
	return SUCCESS;
}

int _rtk_rg_egressPacketDoQosRemarkingDecision(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, struct sk_buff *bcSkb, unsigned int dpMask, unsigned int internalVlanID)
{
	/* MUST decide egress tagif first [pPktHdr->egressVlanTagif] before calling this Qos remarking function */
	unsigned int bcWithDot1pRemarkMask, bcWithoutDot1pRemarkMask;
	unsigned int bcWithoutDscpRemarMask,bcWithDscpRemarByInternalpriMask,bcWithDscpRemarByDscpkMask;
	int dscp_off;
	unsigned char doQosDot1pRemark = FALSE;				// enable 1P remark or not
	unsigned char doQosDSCPRemark = DISABLED_DSCP_REMARK;	// enable DSCP remark or not



	// 1.1. Decide: with or without dot1p remarking
	if(pPktHdr!=NULL && pPktHdr->egressVlanTagif){
		_rtk_rg_broadcastWithDot1pRemarkMask_get(dpMask,&bcWithDot1pRemarkMask,&bcWithoutDot1pRemarkMask);
		if(bcWithDot1pRemarkMask != 0x0)
			doQosDot1pRemark = TRUE;
		else if(bcWithoutDot1pRemarkMask != 0x0)
			doQosDot1pRemark = FALSE;
	}else{
		//if pPktHdr is NULL, it doesn't need to do remarking.
		doQosDot1pRemark = FALSE;
	}
	
	// 1.2. Decide: with or without dscp remarking
	if(pPktHdr!=NULL){
		_rtk_rg_broadcastWithDscpRemarkMask_get(dpMask,&bcWithoutDscpRemarMask,&bcWithDscpRemarByInternalpriMask,&bcWithDscpRemarByDscpkMask);		
		if (bcWithoutDscpRemarMask != 0x0)
			doQosDSCPRemark = DISABLED_DSCP_REMARK;
		else if (bcWithDscpRemarByInternalpriMask)
			doQosDSCPRemark = ENABLED_DSCP_REMARK_AND_SRC_FROM_INTERNALPRI;
		else if (bcWithDscpRemarByDscpkMask)
			doQosDSCPRemark = ENABLED_DSCP_REMARK_AND_SRC_FROM_DSCP;
	}else{
		doQosDSCPRemark= DISABLED_DSCP_REMARK;
	}

	//if (doQosDot1pRemark || doQosDSCPRemark) // mask to always show trace log
		TRACE("Portmask [0x%x] Qos remark: 1P remarking(%s), DSCP remarking(%s)", dpMask, doQosDot1pRemark?"O":"X", doQosDSCPRemark?"O":"X");
	
	// 2.1. set up dot1p remarking
	if (doQosDot1pRemark)	{
		pPktHdr->egressPriority=rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority]&0x7;
		TRACE("Dot1P remark by InternalPri[%d] => Dot1P[%d]", pPktHdr->internalPriority, pPktHdr->egressPriority);
	}

	// 2.2. set up dscp remarking
	if(doQosDSCPRemark && (pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
		dscp_off = (pPktHdr->pTos)-(skb->data);
		_rtk_rg_dscpRemarkToSkb(doQosDSCPRemark,pPktHdr,bcSkb,dscp_off);
	}

	//DEBUG("BC Qos remark portmask [0x%x]: [%s] 1P remarking, [%s] DSCP remarking", dpMask, doQosDot1pRemark?"with":"without", doQosDSCPRemark?"with":"without");

	return RT_ERR_RG_OK;
}

int32 _rtk_rg_sendBroadcastToWan(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *bcSkb, int wanIdx, unsigned int dpMask)
{
	int egressPort;
	uint32 lanInternalVlan = 0;


	//DEBUG("wan port is %d",rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.wan_intf_conf.wan_port_idx);
	rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;
	rg_kernel.txDesc.opts3.bit.tx_portmask=dpMask;
	
	//decision of VLAN tagging
	lanInternalVlan = pPktHdr->internalVlanID; //back up lan internalVlan to avoid _rtk_rg_interfaceVlanIDPriority() change it to wan interfcae vlan.
	_rtk_rg_interfaceVlanIDPriority(pPktHdr,&rg_db.systemGlobal.interfaceInfo[wanIdx].storedInfo,&rg_kernel.txDesc,&rg_kernel.txDescMask);
	pPktHdr->internalVlanID = lanInternalVlan;//rowback pPktHdr->internalVlanID to lan interfcae vlan

	_rtk_rg_wanVlanTagged(pPktHdr,rg_db.systemGlobal.interfaceInfo[wanIdx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on);

	//Qos remarking: Chuck
	egressPort=rg_db.systemGlobal.interfaceInfo[wanIdx].storedInfo.wan_intf.wan_intf_conf.wan_port_idx;
	if(rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[egressPort]==RTK_RG_ENABLED){
		TRACE("QoS dop1p Remarking by port[%d]: internalPri %d => CRI %d",egressPort,pPktHdr->internalPriority,rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority]);		
		pPktHdr->egressPriority=rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority];						
	}

	if((pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF))
		_rtk_rg_qosDscpRemarking(egressPort,pPktHdr,bcSkb);

	_rtk_rg_modifyPacketByACLAction(bcSkb,pPktHdr,rg_db.systemGlobal.interfaceInfo[wanIdx].storedInfo.wan_intf.wan_intf_conf.wan_port_idx);
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
	if(rg_db.systemGlobal.interfaceInfo[wanIdx].storedInfo.wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD)
	{
		TRACE("Send to Master WIFI vxd intf");
		pPktHdr->egressWlanDevIdx=rg_db.systemGlobal.interfaceInfo[wanIdx].storedInfo.wan_intf.wirelessWan;	//20151210LUKE: keep idx for rate limit
		_rtk_rg_splitJumboSendToMasterWifi(pPktHdr,bcSkb,wlan_vxd_netdev);
	}
	else if(rg_db.systemGlobal.interfaceInfo[wanIdx].storedInfo.wan_intf.wirelessWan==RG_WWAN_WLAN1_VXD)
	{
		TRACE("Send to Slave WIFI vxd intf");
		pPktHdr->egressWlanDevIdx=rg_db.systemGlobal.interfaceInfo[wanIdx].storedInfo.wan_intf.wirelessWan;	//20151210LUKE: keep idx for rate limit
		_rtk_rg_splitJumboSendToMasterWifi(pPktHdr,bcSkb,wlan1_vxd_netdev);
	}
	else
#endif
	{
		//Transfer mirror packet with dedicated VLAN and tagging to each WAN interface
		//TRACE("_rtk_rg_egressPacketSend");
		_rtk_rg_egressPacketSend(bcSkb,pPktHdr);
	}

	return RG_FWDENGINE_RET_CONTINUE;

}


int _rtk_rg_dscpRemarkToSkb(rtk_rg_qosDscpRemarkSrcSelect_t dscpSrc,rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, int dscp_off){
	unsigned char tos;
	if(dscpSrc==ENABLED_DSCP_REMARK_AND_SRC_FROM_INTERNALPRI){
		if(pPktHdr->pTos!=NULL){//packet may not have IP header
			TRACE("DSCP remark by InternalPri[%d] => DSCP[%d]",pPktHdr->internalPriority,rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByInternalPri[pPktHdr->internalPriority]);
			if(pPktHdr->tagif&IPV6_TAGIF)
			{
				//dscp is the MSB 6 bits of traffic class
				tos = rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByInternalPri[pPktHdr->internalPriority]>>0x2;	//dscp MSB 4 bits
				tos |= (*pPktHdr->pTos)&0xf0;		//keep version 4 bits
				*pPktHdr->pTos=tos;
				*(skb->data+dscp_off)=tos;

				tos = (rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByInternalPri[pPktHdr->internalPriority]&0x3)<<0x6;	//dscp LSB 2 bits
				tos |= (*(pPktHdr->pTos+1))&0x3f;		//keep original traffic label LSB 2 bits and flow label MSB 4 bits
				*(skb->data+dscp_off+1)=tos;
			}
			else if(pPktHdr->tagif&IPV4_TAGIF)
			{
				tos = rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByInternalPri[pPktHdr->internalPriority]<<0x2;
				tos |= (*pPktHdr->pTos)&0x3;		//keep 2 bits from LSB
				*(skb->data+dscp_off)=tos; 	//remarking tos of packet
			}

			//record egressDSCP
			pPktHdr->egressDSCP=rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByInternalPri[pPktHdr->internalPriority];
		}

	}else if(dscpSrc==ENABLED_DSCP_REMARK_AND_SRC_FROM_DSCP){
		if(pPktHdr->pTos!=NULL){//packet may not have IP header
			TRACE("DSCP remark by DSCP[%d] => DSCP[%d]",(*(pPktHdr->pTos)>>0x2),rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByDscp[*(pPktHdr->pTos)>>0x2]); 
			if(pPktHdr->tagif&IPV6_TAGIF)
			{
				//dscp is the MSB 6 bits of traffic class
				tos = rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByDscp[*(pPktHdr->pTos)>>0x2]>>0x2;	//dscp MSB 4 bits
				tos |= (*pPktHdr->pTos)&0xf0;		//keep version 4 bits
				*(skb->data+dscp_off)=tos;

				tos = (rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByDscp[*(pPktHdr->pTos)>>0x2]&0x3)<<0x6; //dscp LSB 2 bits
				tos |= (*(pPktHdr->pTos+1))&0x3f;		//keep original traffic label LSB 2 bits and flow label MSB 4 bits
				*(skb->data+dscp_off+1)=tos;
			}
			else if(pPktHdr->tagif&IPV4_TAGIF)
			{
				tos = rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByDscp[*(pPktHdr->pTos)>>0x2]<<0x2;
				tos |= (*pPktHdr->pTos)&0x3;		//keep 2 bits from LSB
				*(skb->data+dscp_off)=tos; 	//remarking tos of packet
			}

			//record egressDSCP
			pPktHdr->egressDSCP=rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByDscp[*(pPktHdr->pTos)>>0x2];
		}

	}

	return RT_ERR_RG_OK;
}


int32 _rtk_rg_sendBroadcastToVlanBindingLan(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, unsigned int internalVlanID, unsigned int dpMask)
{
	struct sk_buff *bcSkb=NULL, *cpInt2DscpRemakSkb=NULL, *cpDscp2DscpRemakSkb=NULL;
	unsigned int bcWithDot1pRemarkMask=0, bcWithoutDot1pRemarkMask;
	unsigned int bcWithoutDscpRemarMask,bcWithDscpRemarByInternalpriMask,bcWithDscpRemarByDscpkMask;
	int aclRet;
	int dscp_off;
		
	if(pPktHdr!=NULL){
		_rtk_rg_broadcastWithDot1pRemarkMask_get(dpMask,&bcWithDot1pRemarkMask,&bcWithoutDot1pRemarkMask);
	}else{
		//if pPktHdr is NULL, it doesn't need to do remarking.
		bcWithoutDot1pRemarkMask = dpMask;
	}
	
	/*handel without Cpri remarking packets*/
	if(bcWithoutDot1pRemarkMask!=0x0){
		//Qos DSCP remarking:chuck
		if(pPktHdr!=NULL){
			_rtk_rg_broadcastWithDscpRemarkMask_get(bcWithoutDot1pRemarkMask,&bcWithoutDscpRemarMask,&bcWithDscpRemarByInternalpriMask,&bcWithDscpRemarByDscpkMask);		
		}else{
			bcWithoutDscpRemarMask = dpMask;
		}
		if(bcWithoutDscpRemarMask!=0x0){			
			bcSkb=rtk_rg_skbCopyToPreAllocSkb(skb);
			if(bcSkb==NULL) goto OUT_OF_MEM;

			//printk("s4 %02x-%02x-%02x-%02x-%02x-%02x ref=%d usr=%d\n",bcSkb->data[0],bcSkb->data[1],bcSkb->data[2],bcSkb->data[3],bcSkb->data[4],bcSkb->data[5], atomic_read(&skb_shinfo(bcSkb)->dataref),atomic_read(&bcSkb->users));
		
			//memcpy(bcSkb->data,skb->data,skb->len);
			//bcSkb->len=skb->len;
			pPktHdr->egressVlanID=internalVlanID;
			pPktHdr->egressVlanTagif=1;

			rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;;
			rg_kernel.txDesc.opts3.bit.tx_portmask=bcWithoutDscpRemarMask; //send to port not need to remarking by internalPri: Chuck
		
			//set up priority
			if(rg_db.vlan[internalVlanID].priorityEn==1)
			{
				pPktHdr->egressPriority=rg_db.vlan[internalVlanID].priority&0x7;
			}
			TRACE("Broadcast to tagged LAN(VLAN %d, CPRI %d) portmask %x",internalVlanID,rg_kernel.txDesc.opts2.bit.prio,bcWithoutDscpRemarMask);

			//Do ACL egress check
			assert_ok(_rtk_rg_egressACLPatternCheck(RG_FWD_DECISION_BRIDGING,0,pPktHdr,bcSkb,0,0,-1));
			aclRet = _rtk_rg_egressACLAction(RG_FWD_DECISION_BRIDGING,pPktHdr);
			if(aclRet==RG_FWDENGINE_RET_DROP){
				if(bcSkb) _rtk_rg_dev_kfree_skb_any(bcSkb); 
				return RG_FWDENGINE_RET_DROP;
			}else{
				_rtk_rg_modifyPacketByACLAction(bcSkb,pPktHdr,-1);
				if(rg_db.systemGlobal.gponDsBCModuleEnable && (pPktHdr->ingressPort==RTK_RG_PORT_PON) && (((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)||(pPktHdr->pDmac[0]==0x01 && pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e)) && (rg_db.systemGlobal.initParam.wanPortGponMode==1))//must be GPON, BC, from PON
					_rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(bcSkb,pPktHdr,0);
				else
					_rtk_rg_egressPacketSend(bcSkb,pPktHdr);
			}
		}

		if(bcWithDscpRemarByInternalpriMask!=0x0){
			cpInt2DscpRemakSkb=rtk_rg_skbCopyToPreAllocSkb(skb);
			if(cpInt2DscpRemakSkb==NULL) goto OUT_OF_MEM;

			pPktHdr->egressVlanID=internalVlanID;
			pPktHdr->egressVlanTagif=1;
			
			rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;;
			rg_kernel.txDesc.opts3.bit.tx_portmask=bcWithDscpRemarByInternalpriMask; //send to port not need to remarking by internalPri: Chuck
			
			if((pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
				dscp_off = (pPktHdr->pTos)-(skb->data);
				_rtk_rg_dscpRemarkToSkb(ENABLED_DSCP_REMARK_AND_SRC_FROM_INTERNALPRI,pPktHdr,cpInt2DscpRemakSkb,dscp_off);
				TRACE("Broadcast to tagged LAN(VLAN %d, CPRI %d) with DSCP remarking(internalPri[%d]=>dscp[%d]) portmask %x",
						internalVlanID,rg_kernel.txDesc.opts2.bit.prio,pPktHdr->internalPriority,rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByInternalPri[pPktHdr->internalPriority],bcWithDscpRemarByInternalpriMask);
			}
			//dump_packet(bcSkb->data,bcSkb->len,"broadcast packet");
			
			//Do ACL egress check
			assert_ok(_rtk_rg_egressACLPatternCheck(RG_FWD_DECISION_BRIDGING,0,pPktHdr,cpInt2DscpRemakSkb,0,0,-1));
			aclRet = _rtk_rg_egressACLAction(RG_FWD_DECISION_BRIDGING,pPktHdr);
			if(aclRet==RG_FWDENGINE_RET_DROP){
				if(cpInt2DscpRemakSkb) _rtk_rg_dev_kfree_skb_any(cpInt2DscpRemakSkb); 
				return RG_FWDENGINE_RET_DROP;
			}else{
				_rtk_rg_modifyPacketByACLAction(cpInt2DscpRemakSkb,pPktHdr,-1);
				if(rg_db.systemGlobal.gponDsBCModuleEnable && (pPktHdr->ingressPort==RTK_RG_PORT_PON) && (((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)||(pPktHdr->pDmac[0]==0x01 && pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e)) && (rg_db.systemGlobal.initParam.wanPortGponMode==1))//must be GPON, BC, from PON
					_rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(cpInt2DscpRemakSkb,pPktHdr,0);
				else				
					_rtk_rg_egressPacketSend(cpInt2DscpRemakSkb,pPktHdr);
			}

		}
	
		if(bcWithDscpRemarByDscpkMask!=0x0){
			cpDscp2DscpRemakSkb=rtk_rg_skbCopyToPreAllocSkb(skb);
			if(cpDscp2DscpRemakSkb==NULL) goto OUT_OF_MEM;

			pPktHdr->egressVlanID=internalVlanID;
			pPktHdr->egressVlanTagif=1;
	
			rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;;
			rg_kernel.txDesc.opts3.bit.tx_portmask=bcWithDscpRemarByDscpkMask; //send to port not need to remarking by internalPri: Chuck

			if((pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
				dscp_off = (pPktHdr->pTos)-(skb->data);
				_rtk_rg_dscpRemarkToSkb(ENABLED_DSCP_REMARK_AND_SRC_FROM_DSCP,pPktHdr,cpDscp2DscpRemakSkb,dscp_off);
				TRACE("Broadcast to tagged LAN(VLAN %d, CPRI %d) with DSCP remarking(dscp[%d]=>dscp[%d]) portmask %x",
									internalVlanID,rg_kernel.txDesc.opts2.bit.prio,(*pPktHdr->pTos)>>2,rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByDscp[(*pPktHdr->pTos)>>2],bcWithDscpRemarByDscpkMask);
			}
			//dump_packet(bcSkb->data,bcSkb->len,"broadcast packet");
			
			//Do ACL egress check
			assert_ok(_rtk_rg_egressACLPatternCheck(RG_FWD_DECISION_BRIDGING,0,pPktHdr,cpDscp2DscpRemakSkb,0,0,-1));	
			aclRet = _rtk_rg_egressACLAction(RG_FWD_DECISION_BRIDGING,pPktHdr);
			if(aclRet==RG_FWDENGINE_RET_DROP){
				if(cpDscp2DscpRemakSkb) _rtk_rg_dev_kfree_skb_any(cpDscp2DscpRemakSkb); 
				return RG_FWDENGINE_RET_DROP;
			}else{
				
				_rtk_rg_modifyPacketByACLAction(cpDscp2DscpRemakSkb,pPktHdr,-1);
				if(rg_db.systemGlobal.gponDsBCModuleEnable && (pPktHdr->ingressPort==RTK_RG_PORT_PON) && (((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)||(pPktHdr->pDmac[0]==0x01 && pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e)) && (rg_db.systemGlobal.initParam.wanPortGponMode==1))//must be GPON, BC, from PON
					_rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(cpDscp2DscpRemakSkb,pPktHdr,0);
				else
					_rtk_rg_egressPacketSend(cpDscp2DscpRemakSkb,pPktHdr);
			}

		}
	}


	/*handel with Cpri remarking packets*/
	if(bcWithDot1pRemarkMask!=0x0){//some port need to remark:chuck

		//Qos DSCP remarking:chuck
		_rtk_rg_broadcastWithDscpRemarkMask_get(bcWithDot1pRemarkMask,&bcWithoutDscpRemarMask,&bcWithDscpRemarByInternalpriMask,&bcWithDscpRemarByDscpkMask);		

		if(bcWithoutDscpRemarMask!=0x0){		
			bcSkb=rtk_rg_skbCopyToPreAllocSkb(skb);
			if(bcSkb==NULL) goto OUT_OF_MEM;


			pPktHdr->egressVlanID=internalVlanID;
			pPktHdr->egressVlanTagif=1;

			//set up dot1p priority
			pPktHdr->egressPriority=rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority]&0x7;

			rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;
			rg_kernel.txDesc.opts3.bit.tx_portmask=bcWithoutDscpRemarMask;
								
			TRACE("Broadcast to tagged LAN(VLAN %d, CPRI%d) portmask %x",internalVlanID,rg_kernel.txDesc.opts2.bit.prio,bcWithoutDscpRemarMask);

			//Do ACL egress check
			assert_ok(_rtk_rg_egressACLPatternCheck(RG_FWD_DECISION_BRIDGING,0,pPktHdr,bcSkb,0,0,-1));	
			aclRet = _rtk_rg_egressACLAction(RG_FWD_DECISION_BRIDGING,pPktHdr);
			if(aclRet==RG_FWDENGINE_RET_DROP){
				if(bcSkb) _rtk_rg_dev_kfree_skb_any(bcSkb); 
				return RG_FWDENGINE_RET_DROP;
			}else{
				_rtk_rg_modifyPacketByACLAction(bcSkb,pPktHdr,-1);
				if(rg_db.systemGlobal.gponDsBCModuleEnable && (pPktHdr->ingressPort==RTK_RG_PORT_PON) && (((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)||(pPktHdr->pDmac[0]==0x01 && pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e)) && (rg_db.systemGlobal.initParam.wanPortGponMode==1))//must be GPON, BC, from PON
					_rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(bcSkb,pPktHdr,0);
				else
					_rtk_rg_egressPacketSend(bcSkb,pPktHdr);
			}
		}


		
		if(bcWithDscpRemarByInternalpriMask!=0x0){
			cpInt2DscpRemakSkb=rtk_rg_skbCopyToPreAllocSkb(skb);
			if(cpInt2DscpRemakSkb==NULL) goto OUT_OF_MEM;

			pPktHdr->egressVlanID=internalVlanID;
			pPktHdr->egressVlanTagif=1;

			//set up dot1p priority
			pPktHdr->egressPriority=rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority]&0x7;
			
			rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;;
			rg_kernel.txDesc.opts3.bit.tx_portmask=bcWithDscpRemarByInternalpriMask;
			
			if((pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
				dscp_off = (pPktHdr->pTos)-(skb->data);
				_rtk_rg_dscpRemarkToSkb(ENABLED_DSCP_REMARK_AND_SRC_FROM_INTERNALPRI,pPktHdr,cpInt2DscpRemakSkb,dscp_off);
				TRACE("Broadcast to tagged LAN(VLAN %d, CPRI %d) with DSCP remarking(internalPri[%d]=>dscp[%d]) portmask %x",
					internalVlanID,rg_kernel.txDesc.opts2.bit.prio,pPktHdr->internalPriority,rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByInternalPri[pPktHdr->internalPriority],bcWithDscpRemarByInternalpriMask);
			}

			//dump_packet(bcSkb->data,bcSkb->len,"broadcast packet");

			//Do ACL egress check
			assert_ok(_rtk_rg_egressACLPatternCheck(RG_FWD_DECISION_BRIDGING,0,pPktHdr,cpInt2DscpRemakSkb,0,0,-1));		
			aclRet = _rtk_rg_egressACLAction(RG_FWD_DECISION_BRIDGING,pPktHdr);
			if(aclRet==RG_FWDENGINE_RET_DROP){
				if(cpInt2DscpRemakSkb) _rtk_rg_dev_kfree_skb_any(cpInt2DscpRemakSkb); 
				return RG_FWDENGINE_RET_DROP;
			}else{
				_rtk_rg_modifyPacketByACLAction(cpInt2DscpRemakSkb,pPktHdr,-1);
				if(rg_db.systemGlobal.gponDsBCModuleEnable && (pPktHdr->ingressPort==RTK_RG_PORT_PON) && (((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)||(pPktHdr->pDmac[0]==0x01 && pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e)) && (rg_db.systemGlobal.initParam.wanPortGponMode==1))//must be GPON, BC, from PON
					_rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(cpInt2DscpRemakSkb,pPktHdr,0);
				else
					_rtk_rg_egressPacketSend(cpInt2DscpRemakSkb,pPktHdr);
			}

		}
	
		if(bcWithDscpRemarByDscpkMask!=0x0){
			cpDscp2DscpRemakSkb=rtk_rg_skbCopyToPreAllocSkb(skb);
			if(cpDscp2DscpRemakSkb==NULL) goto OUT_OF_MEM;

			pPktHdr->egressVlanID=internalVlanID;
			pPktHdr->egressVlanTagif=1;

			//set up dot1p priority
			pPktHdr->egressPriority=rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority]&0x7;

			rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;;
			rg_kernel.txDesc.opts3.bit.tx_portmask=bcWithDscpRemarByDscpkMask;
			
			if((pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
				dscp_off = (pPktHdr->pTos)-(skb->data);
				_rtk_rg_dscpRemarkToSkb(ENABLED_DSCP_REMARK_AND_SRC_FROM_DSCP,pPktHdr,cpDscp2DscpRemakSkb,dscp_off);
				TRACE("Broadcast to tagged LAN(VLAN %d, CPRI %d) with DSCP remarking(dscp[%d]=>dscp[%d]) portmask %x",
									internalVlanID,rg_kernel.txDesc.opts2.bit.prio,(*pPktHdr->pTos)>>2,rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkByDscp[(*pPktHdr->pTos)>>2],bcWithDscpRemarByDscpkMask);
			}

			//dump_packet(bcSkb->data,bcSkb->len,"broadcast packet");

			assert_ok(_rtk_rg_egressACLPatternCheck(RG_FWD_DECISION_BRIDGING,0,pPktHdr,cpDscp2DscpRemakSkb,0,0,-1));
			aclRet = _rtk_rg_egressACLAction(RG_FWD_DECISION_BRIDGING,pPktHdr);
			if(aclRet==RG_FWDENGINE_RET_DROP){
				if(cpDscp2DscpRemakSkb) _rtk_rg_dev_kfree_skb_any(cpDscp2DscpRemakSkb);
				return RG_FWDENGINE_RET_DROP;
			}else{
				_rtk_rg_modifyPacketByACLAction(cpDscp2DscpRemakSkb,pPktHdr,-1);
				if(rg_db.systemGlobal.gponDsBCModuleEnable && (pPktHdr->ingressPort==RTK_RG_PORT_PON) && (((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)||(pPktHdr->pDmac[0]==0x01 && pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e)) && (rg_db.systemGlobal.initParam.wanPortGponMode==1))//must be GPON, BC, from PON
					_rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(cpDscp2DscpRemakSkb,pPktHdr,0);
				else
					_rtk_rg_egressPacketSend(cpDscp2DscpRemakSkb,pPktHdr);
			}
		}

	}

	return (RG_FWDENGINE_RET_CONTINUE);
OUT_OF_MEM:

#if RTK_RG_SKB_PREALLOCATE	
	FIXME("Out of pre-alloc memory(%s:%d)\n",__FUNCTION__,__LINE__);
#else
	FIXME("Out of memory(%s:%d)\n",__FUNCTION__,__LINE__);
#endif
	if(bcSkb) _rtk_rg_dev_kfree_skb_any(bcSkb);

	return (RG_FWDENGINE_RET_DROP);
}



int _rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr, int toMasterWifiOrCPU)
{
	int i;
	rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule;
	unsigned int candidatePortMask;
	unsigned int sendPortMask = 0x0;
	struct sk_buff *cpSkb=NULL;
	struct tx_info tmp_txDesc,tmp_txDescMask;

	if(toMasterWifiOrCPU==1)//ro wifi
	{
		 candidatePortMask = (1<<RTK_RG_EXT_PORT0);
	}
	else if(toMasterWifiOrCPU==2)//to cpu
	{
		candidatePortMask = (1<<RTK_RG_PORT_CPU);
	}
	else{//to physical ports (not support for slave wifi)
		 candidatePortMask = rg_kernel.txDesc.opts3.bit.tx_portmask;
	}
	
	//check rule
	for(i=0;i<MAX_GPON_DS_BC_FILTER_SW_ENTRY_SIZE;i++){

		if(candidatePortMask ==0x0)//no port needs to send
			break;

		//get valid rule
		if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].valid==RTK_RG_ENABLED)
			filterRule = &rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule;
		else
			continue;

		DEBUG("check rule[%d]",i);

		//pattern check
		if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT)
		{
			if(filterRule->ingress_stream_id!=pPktHdr->pRxDesc->opts2.bit.pon_stream_id){
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT unhit",i);
				continue;
			}
		}

		if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT)
		{
			if(filterRule->ingress_stagIf==1 && (pPktHdr->tagif&SVLAN_TAGIF)==0x0){//must stag
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT unhit",i);
				continue;
			}else if(filterRule->ingress_stagIf==0 && (pPktHdr->tagif&SVLAN_TAGIF)){//must un-stag
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT unhit",i);
				continue;
			}
		}

		if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT)
		{
			if(filterRule->ingress_ctagIf==1 && (pPktHdr->tagif&CVLAN_TAGIF)==0x0){//must ctag
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT unhit",i);
				continue;
			}else if(filterRule->ingress_ctagIf==0 && (pPktHdr->tagif&CVLAN_TAGIF)){//must un-ctag
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT unhit",i);
				continue;
			}

		}

		if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_SVID_BIT)
		{
			if(filterRule->ingress_stag_svid!=pPktHdr->stagVid){
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_INGRESS_SVID_BIT unhit",i);
				continue;
			}
		}

		if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_CVID_BIT)
		{
			if(filterRule->ingress_ctag_cvid!=pPktHdr->ctagVid){
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_INGRESS_SVID_BIT unhit",i);
				continue;
			}
		}
		
		if( (toMasterWifiOrCPU==0 || toMasterWifiOrCPU==1)&& filterRule->filter_fields & GPON_DS_BC_FILTER_EGRESS_PORT_BIT)
		{
			if((filterRule->egress_portmask.portmask & rg_kernel.txDesc.opts3.bit.tx_portmask)==0x0){
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_EGRESS_PORT_BIT unhit",i);
				continue;
			}
		}else if(toMasterWifiOrCPU==2){
			if((filterRule->egress_portmask.portmask & (1<<RTK_RG_PORT_CPU))==0x0){
				DEBUG("[GPON BC] rule[%d] GPON_DS_BC_FILTER_EGRESS_PORT_BIT unhit",i);
				continue;
			}
		}
		
		DEBUG("[GPON BC] rule[%d] hit!!!",i);
		//rule hit, send to assigned egress port!
		sendPortMask = candidatePortMask & filterRule->egress_portmask.portmask;
		if(sendPortMask){ 

			if(toMasterWifiOrCPU==0 || toMasterWifiOrCPU==1){//to lan port or to wifi need to copy a new skb to send
				//make another copy for none CF port in this case.
				cpSkb=rtk_rg_skbCopyToPreAllocSkb(skb);
				if(cpSkb==NULL){
					if(cpSkb) _rtk_rg_dev_kfree_skb_any(cpSkb);
					//can not send
					DEBUG("[GPON BC] allocate skb failed! Drop!",i);
					return RG_FWDENGINE_RET_DROP;		
				} 
			}

			//backup tx_desc & pktHdr
			memcpy(&tmp_txDesc,&rg_kernel.txDesc,sizeof(tmp_txDesc));
			memcpy(&tmp_txDescMask,&rg_kernel.txDescMask,sizeof(tmp_txDescMask));
			memcpy(&rg_db.systemGlobal.pktHeader_broadcast,pPktHdr,sizeof(rtk_rg_pktHdr_t));
			
			if(filterRule->ctag_action.ctag_decision==RTK_RG_GPON_BC_FORCE_UNATG){
				pPktHdr->egressVlanTagif = 0;
				DEBUG("[GPON BC] rule[%d] Hit! Remove Ctag to portmask 0x%x",i,sendPortMask);
			}else if(filterRule->ctag_action.ctag_decision==RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID){
				pPktHdr->egressVlanTagif = 1;
				pPktHdr->egressVlanID = filterRule->ctag_action.assigned_ctag_cvid;
				DEBUG("[GPON BC] rule[%d] Hit! Add Ctag(%d,not assign) to portmask 0x%x",i,pPktHdr->egressVlanID,sendPortMask);
			}else if(filterRule->ctag_action.ctag_decision==RTK_RG_GPON_BC_FORCE_TAGGIN_WITH_CVID_CPRI){
				pPktHdr->egressVlanTagif = 1;
				pPktHdr->egressVlanID = filterRule->ctag_action.assigned_ctag_cvid;
				pPktHdr->egressPriority = filterRule->ctag_action.assigned_ctag_cpri;			
				DEBUG("[GPON BC] rule[%d] Hit! Add Ctag(%d,%d) to portmask 0x%x",i,pPktHdr->egressVlanID,pPktHdr->egressPriority,sendPortMask);
			}	

			if(toMasterWifiOrCPU==1){
				#ifdef CONFIG_RG_WLAN_HWNAT_ACCELERATION
					rtk_rg_mbssidDev_t intf_idx;
					pPktHdr->gponDsBcModuleRuleHit = 1;
						
					intf_idx=_rtk_master_wlan_mbssid_tx(pPktHdr,cpSkb);

					if(intf_idx==RG_RET_MBSSID_NOT_FOUND)//send failed, free the skb
					{
						if(cpSkb) _rtk_rg_dev_kfree_skb_any(cpSkb);
					}
					
					if(intf_idx==RG_RET_MBSSID_FLOOD_ALL_INTF)
					{
						TRACE("Broadcast to master WLAN(flooding)");
					}
					else
					{
						TRACE("Broadcast to master WLAN(intf=%d)",intf_idx);
					}
				#else
					TRACE("No Wifi Acceleration");
				#endif
			}
			else if(toMasterWifiOrCPU==2)//to CPU
			{
				//no need to call _rtk_rg_egressPacketSend
				//and no need to free the skb, because it is not copied.
			}
			else
			{
				rg_kernel.txDesc.opts3.bit.tx_portmask= sendPortMask;
				_rtk_rg_egressPacketSend(cpSkb,pPktHdr);
			}
				
			candidatePortMask &= ~(sendPortMask); //remove send ports from candidatePortMask

			if(toMasterWifiOrCPU==0 || toMasterWifiOrCPU==1){//after send packet,  tx_desc & pktHdr need to restore
				//restore tx_desc & pktHdr
				memcpy(&rg_kernel.txDesc,&tmp_txDesc,sizeof(tmp_txDesc));
				memcpy(&rg_kernel.txDescMask,&tmp_txDescMask,sizeof(tmp_txDescMask));
				memcpy(pPktHdr,&rg_db.systemGlobal.pktHeader_broadcast,sizeof(rtk_rg_pktHdr_t));
			}
		}

	}


	if(toMasterWifiOrCPU==0 || toMasterWifiOrCPU==1){//to CPU, the SKB is not copied one, so need to free 

		if(candidatePortMask) //some port without hit gponDsBcRules should send as original.
		{
			rg_kernel.txDesc.opts3.bit.tx_portmask= candidatePortMask;
			_rtk_rg_egressPacketSend(skb,pPktHdr);
		}
		else
		{
			//all candidatePortMask is send, handle(free) the incoming skb as _rtk_rg_egressPacketSend()
			if(skb) _rtk_rg_dev_kfree_skb_any(skb);
		}
	}
	
	//after for loop, the rest ports of candidatePortMask should be discard by no-body send(unhit drop as CF)
	//but return RG_FWDENGINE_RET_CONTINUE to let it goto PS.
	return RG_FWDENGINE_RET_CONTINUE;	

}

int _rtk_rg_BroadcastPacketToLanWithEgressACLModification(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *bcSkb,int l3Modify,int l4Modify, unsigned int bcTxMask, unsigned int allDestPortMask, rtk_rg_port_idx_t egressPort)
{
		/* egressPort==0 means caller is normal case, function will replace it by checking if bcTxMask belongs to CF port 
		     egressPort==-1, means use -1 to do ACL pattern check. It is used in vlan binding case.*/

		struct sk_buff *cpSkb=NULL;
		int aclRet= RG_FWDENGINE_RET_CONTINUE;
		unsigned int allPortmask = RTK_RG_ALL_MAC_PORTMASK;
#if defined(CONFIG_RTL9602C_SERIES)		
		unsigned int cfPortMask = (1<<RTK_RG_PORT_PON);	
#else
		unsigned int cfPortMask = ((1<<RTK_RG_PORT_PON)|(1<<RTK_RG_PORT_RGMII));	
#endif
		unsigned int toCfPortMask = (bcTxMask & cfPortMask);
		unsigned int toOtherPortMask =(bcTxMask & (allPortmask&=(~cfPortMask)) );

		//ACL("mask cfPortMask=0x%x  toCfPortMask=0x%x  toOtherPortMask=0x%x",cfPortMask,toCfPortMask,toOtherPortMask);

		if (egressPort != -1){
			egressPort = (toCfPortMask!= 0x0)?(RTK_RG_PORT_PON):(RTK_RG_PORT_MAX);
		}
		
		// Cheney: abnormal case handler: to prevent use same skb source. (for BC path1)
		if(toCfPortMask!= 0x0 && toOtherPortMask!=0x0)
		{		
			//make a copy for original skb, (and using this copy in this sub-function)		
			cpSkb=rtk_rg_skbCopyToPreAllocSkb(bcSkb);		
			if(cpSkb==NULL){			
				if(cpSkb) _rtk_rg_dev_kfree_skb_any(cpSkb); 			
				return RG_FWDENGINE_RET_DROP;	
			}	
		}
			
		if(toCfPortMask!=0x0){//send to CF ports only
			rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;
			rg_kernel.txDesc.opts3.bit.tx_portmask=toCfPortMask;

			_rtk_rg_modifyPacketByACLAction(bcSkb,pPktHdr,RTK_RG_PORT_PON);
			
			if (_rtk_rg_egressPortMaskCheck(pPktHdr,bcTxMask, allDestPortMask) == FAIL){
				aclRet = RG_FWDENGINE_RET_DROP;
				goto SKIPPACKETSEND;
			}
			
			//TRACE("Portmask [0x%x] Send to CF pmsk only",toCfPortMask);
			_rtk_rg_egressPacketSend(bcSkb,pPktHdr);

		}
		if(toOtherPortMask!=0x0){//send to none-CF ports only
			if(toCfPortMask!=0x0){// Cheney: abnormal case handler: to prevent use same skb source. (for BC path1)
				// original bcSkb has been transmitted! Use the backup one
				bcSkb = cpSkb;
			}
			rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;
			rg_kernel.txDesc.opts3.bit.tx_portmask=toOtherPortMask;

			_rtk_rg_modifyPacketByACLAction(bcSkb,pPktHdr,RTK_RG_PORT_MAX);
			
			if (_rtk_rg_egressPortMaskCheck(pPktHdr,bcTxMask, allDestPortMask) == FAIL){
				aclRet = RG_FWDENGINE_RET_DROP;
				goto SKIPPACKETSEND;
			}
			
			//TRACE("Portmask [0x%x] Send to none-CF pmsk only",toOtherPortMask);				
			if(rg_db.systemGlobal.gponDsBCModuleEnable && (pPktHdr->ingressPort==RTK_RG_PORT_PON) && (((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)||(pPktHdr->pDmac[0]==0x01 && pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e)) && (rg_db.systemGlobal.initParam.wanPortGponMode==1))//must be GPON, BC, from PON
				_rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(bcSkb,pPktHdr,0);
			else
				_rtk_rg_egressPacketSend(bcSkb,pPktHdr);

		}
SKIPPACKETSEND:
		return aclRet;

		
}

//collect napt info for callback from NAPT and NAPTR table
void _rtk_rg_naptInfoCollectForCallback(int naptOutIdx, rtk_rg_naptInfo_t *naptInfo)
{	
	int naptInIdx=rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
	
	memset(naptInfo,0,sizeof(rtk_rg_naptInfo_t));	
	naptInfo->naptTuples.is_tcp = rg_db.naptIn[naptInIdx].rtk_naptIn.isTcp;
	naptInfo->naptTuples.local_ip = rg_db.naptIn[naptInIdx].rtk_naptIn.intIp;
	naptInfo->naptTuples.local_port = rg_db.naptIn[naptInIdx].rtk_naptIn.intPort;
#if 0		
	naptInfo->naptTuples.remote_ip = rg_db.naptIn[naptInIdx].remoteIp;
	naptInfo->naptTuples.remote_port = rg_db.naptIn[naptInIdx].remotePort;
#else
	naptInfo->naptTuples.remote_ip = rg_db.naptOut[naptOutIdx].remoteIp;
	naptInfo->naptTuples.remote_port = rg_db.naptOut[naptOutIdx].remotePort;
#endif
	naptInfo->naptTuples.wan_intf_idx = rg_db.nexthop[rg_db.extip[rg_db.naptIn[naptInIdx].rtk_naptIn.extIpIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx;;
	naptInfo->naptTuples.inbound_pri_valid = rg_db.naptIn[naptInIdx].rtk_naptIn.priValid;
	naptInfo->naptTuples.inbound_priority = rg_db.naptIn[naptInIdx].rtk_naptIn.priId;		
	naptInfo->naptTuples.external_port = rg_db.naptOut[naptOutIdx].extPort;
	naptInfo->naptTuples.outbound_pri_valid = rg_db.naptOut[naptOutIdx].rtk_naptOut.priValid;
	naptInfo->naptTuples.outbound_priority = rg_db.naptOut[naptOutIdx].rtk_naptOut.priValue;
	naptInfo->idleSecs = rg_db.naptOut[naptOutIdx].idleSecs;
	naptInfo->state = rg_db.naptOut[naptOutIdx].state;
}

/* LAN NET INFO declaration string */
const char* rg_lanNet_phone_type[] = {
	"ANDROID",
	"IPHONE",
	"IPAD",
	"WINDOWS PHONE",
	NULL
};

const char* rg_lanNet_computer_type[] = {
	"WINDOWS NT",
	"MACINTOSH",
	"LINUX",
	NULL
};

const char* rg_lanNet_brand[][MAX_LANNET_SUB_BRAND_SIZE] = {
	{"OTHER", NULL},					//RG_BRAND_OTHER
	{"HUAWEI", NULL},					//RG_BRAND_HUAWEI
	{"XIAOMI", NULL},					//RG_BRAND_XIAOMI
	{"MEIZU", NULL},					//RG_BRAND_MEIZU
	{"IPHONE", NULL},					//RG_BRAND_IPHONE
	{"NOKIA", NULL},					//RG_BRAND_NOKIA
	{"SAMSUNG", NULL},					//RG_BRAND_SAMSUNG
	{"SONY", "XPERIA", NULL},			//RG_BRAND_SONY
	{"ERICSSON", NULL},					//RG_BRAND_ERICSSON
	{"MOT", NULL},						//RG_BRAND_MOT
	{"HTC", NULL},						//RG_BRAND_HTC
	{"SGH", NULL},						//RG_BRAND_SGH
	{"LG", NULL},						//RG_BRAND_LG
	{"SHARP", NULL},					//RG_BRAND_SHARP
	{"PHILIPS", NULL},					//RG_BRAND_PHILIPS
	{"PANASONIC", NULL},				//RG_BRAND_PANASONIC
	{"ALCATEL", NULL},					//RG_BRAND_ALCATEL
	{"LENOVO", NULL},					//RG_BARND_LENOVO
	{NULL}
};

const char* rg_lanNet_os[][MAX_LANNET_SUB_OS_SIZE] = {
	{"OTHER", NULL},					//RG_OS_OTHER
	{"WINDOWS NT", NULL},				//RG_OS_WINDOWS_NT
	{"MACINTOSH", "MAC OS X", NULL},	//RG_OS_MACINTOSH
	{"IOS", "IPHONE OS", NULL},			//RG_OS_IPHONE_OS
	{"ANDROID", NULL},					//RG_OS_ANDROID
	{"WINDOWS PHONE", NULL},			//RG_OS_WINDOWS_PHONE
	{"LINUX", NULL},					//RG_OS_LINUX
	{NULL}
};

const char* rg_http_request_cmd[] = {
	"GET",
	"POST",
	"PUT",
	"OPTIONS",
	"HEAD",
	"DELETE",
	"TRACE",
	NULL
};

#else //APOLLO_MODEL CODE
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

#include <fcntl.h>
#include <unistd.h>

void *rtk_rg_malloc(int NBYTES)
{
	return malloc(NBYTES);
}
void rtk_rg_free(void *APTR){
	free(APTR);
}

#endif


