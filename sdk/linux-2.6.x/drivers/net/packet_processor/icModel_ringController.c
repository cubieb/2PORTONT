
#include <linux/netdevice.h>

//#include <types.h>
//#include "crc32.h"

#include "hsModel.h"
#include "rtl8672_asicregs.h"
#include "rtl_utils.h"
#include "rtl8672_Proto.h"
#include "rtl8672_tblAsicDrv.h"

#include "icModel_ringController.h"
#include "icModel_ALE.h"
#include "icModel_Packet.h"
#include "icModel_pktParser.h"
#include "icModel_pktTranslator.h"
#include "icTest_ringController.h"

#include "modelTrace.h"
#include "icTest.h"


// packet processor first desc pointer 
struct mac_pRx *first_mac_prx[MAC_RX_INTFS];
struct sar_pRx *first_sar_prx[SAR_INTFS];
struct ext_Rx *first_ext_prx[EXT_INTFS];

struct mac_pTx *first_mac_ptx[MAC_TX_INTFS];
struct sar_pTx*first_sar_ptx[SAR_INTFS];
struct ext_Tx *first_ext_ptx[EXT_INTFS];

struct mac_vRx *first_mac_vrx[MAC_RX_INTFS];
struct sar_vRx *first_sar_vrx[SAR_INTFS];
struct ext_Rx *first_ext_vrx[EXT_INTFS];

struct mac_vTx *first_mac_vtx[MAC_TX_INTFS];
struct sar_vTx*first_sar_vtx[SAR_INTFS];
struct ext_Tx *first_ext_vtx[EXT_INTFS];

struct sp_pRx *first_sp_to_mac[MAC_TX_INTFS];
struct sp_pRx *first_sp_to_sar[SAR_INTFS];
struct sp_pRx *first_sp_to_ext[EXT_INTFS];


// asic current pointer
struct mac_pRx *current_mac_prx[MAC_RX_INTFS];
struct sar_pRx *current_sar_prx[SAR_INTFS];
struct ext_Rx *current_ext_prx[EXT_INTFS]; /* FIXME */

struct mac_pTx *current_mac_ptx[MAC_TX_INTFS];
struct sar_pTx*current_sar_ptx[SAR_INTFS];
struct ext_Tx *current_ext_ptx[EXT_INTFS];  /* FIXME */

//struct mac_vRx *current_mac_vrx[MAC_RX_INTFS];
//struct sar_vRx *current_sar_vrx[SAR_INTFS];
//struct ext_Rx *current_ext_vrx[EXT_INTFS];

//struct mac_vTx *current_mac_vtx[MAC_TX_INTFS];
//struct sar_vTx*current_sar_vtx[SAR_INTFS];
//struct ext_Tx *current_ext_vtx[EXT_INTFS];

struct sp_pRx *current_sp_to_mac[MAC_TX_INTFS];
struct sp_pRx *current_sp_to_sar[SAR_INTFS];
struct sp_pRx *current_sp_to_ext[EXT_INTFS];


// packet processor pp pointer 
struct mac_pRx *pp_mac_prx[MAC_RX_INTFS];
struct sar_pRx *pp_sar_prx[SAR_INTFS];
struct ext_Rx *pp_ext_prx[EXT_INTFS];

struct mac_pTx *pp_mac_ptx[MAC_TX_INTFS];
struct sar_pTx*pp_sar_ptx[SAR_INTFS];
struct ext_Tx *pp_ext_ptx[EXT_INTFS];

struct mac_vRx *pp_mac_vrx[MAC_RX_INTFS];
struct sar_vRx *pp_sar_vrx[SAR_INTFS];
struct ext_Rx *pp_ext_vrx[EXT_INTFS];

struct mac_vTx *pp_mac_vtx[MAC_TX_INTFS];
struct sar_vTx*pp_sar_vtx[SAR_INTFS];
struct ext_Tx *pp_ext_vtx[EXT_INTFS];

struct sp_pRx *pp_sp_to_mac[MAC_TX_INTFS];
struct sp_pRx *pp_sp_to_sar[SAR_INTFS];
struct sp_pRx *pp_sp_to_ext[EXT_INTFS];


/* ring used count : only used for model code */
int	mac_prx_ring_used[MAC_RX_INTFS]={0};  // if used == 0 means the ring is empty
int	mac_ptx_ring_used[MAC_TX_INTFS]={0};
int	sar_prx_ring_used[SAR_INTFS]={0};
int	sar_ptx_ring_used[SAR_INTFS]={0};
int	ext_prx_ring_used[EXT_INTFS]={0};
int	ext_ptx_ring_used[EXT_INTFS]={0};

/* ring size : used by model code and real mode */
int	mac_prx_ring_size[MAC_RX_INTFS]={0};
int	mac_ptx_ring_size[MAC_TX_INTFS]={0};
int	sar_prx_ring_size[SAR_INTFS]={0};
int	sar_ptx_ring_size[SAR_INTFS]={0};
int	ext_prx_ring_size[EXT_INTFS]={0};
int	ext_ptx_ring_size[EXT_INTFS]={0};
int	mac_vrx_ring_size[MAC_RX_INTFS]={0};
int	mac_vtx_ring_size[MAC_TX_INTFS]={0};
int	sar_vrx_ring_size[SAR_INTFS]={0};
int	sar_vtx_ring_size[SAR_INTFS]={0};
int	ext_vrx_ring_size[EXT_INTFS]={0};
int	ext_vtx_ring_size[EXT_INTFS]={0};

/* model code only */
int	mac_prx_ring_empty[MAC_RX_INTFS]={0};
int	mac_ptx_ring_full[MAC_TX_INTFS]={0};
int	sar_prx_ring_empty[SAR_INTFS]={0};
int	sar_ptx_ring_full[SAR_INTFS]={0};

/* counter : only used for model code */
int	mac_prx_drop[MAC_RX_INTFS]={0};
int	mac_vrx_recv[MAC_RX_INTFS]={0};
int	mac_vtx_fwd[MAC_TX_INTFS]={0};
int	sar_prx_drop[SAR_INTFS]={0};
int	sar_vrx_recv[SAR_INTFS]={0};
int	sar_vtx_fwd[SAR_INTFS]={0};
int	ext_prx_drop[EXT_INTFS]={0};
int	ext_vrx_recv[EXT_INTFS]={0};
int	ext_vtx_fwd[EXT_INTFS]={0};

int	sp_to_sar_fwd[SAR_INTFS]={0};
int	sp_to_mac_fwd[MAC_TX_INTFS]={0};
int	sp_to_ext_fwd[MAC_TX_INTFS]={0};

/* weight : only used for model code  */
uint8 sar_prx_ring_weight[SAR_INTFS]={0};
uint8 mac_prx_ring_weight[MAC_RX_INTFS]={0};
uint8 ext_prx_ring_weight[EXT_INTFS]={0};
uint8 sar_vtx_ring_weight[SAR_INTFS]={0};
uint8 mac_vtx_ring_weight[MAC_TX_INTFS]={0};
uint8 ext_vtx_ring_weight[EXT_INTFS]={0};

/* handle count : only used for model code */
uint8 sar_prx_ring_cnt[SAR_INTFS]={0};
uint8 mac_prx_ring_cnt[MAC_RX_INTFS]={0};
uint8 ext_prx_ring_cnt[EXT_INTFS]={0};
uint8 sar_vtx_ring_cnt[SAR_INTFS]={0};
uint8 mac_vtx_ring_cnt[MAC_TX_INTFS]={0};
uint8 ext_vtx_ring_cnt[EXT_INTFS]={0};

int8 ethnt_offset_map[2][5]={{0x3f,2,2,2,0x3f},{0x3f,10,10,10,0x3f}};  /* VC, LLC */

uint8 cache_linkid=0;


struct mac_pRx *mac_prx(int intf,int idx)
{
	return (struct mac_pRx *)(first_mac_prx[intf]+idx);
}

struct sar_pRx *sar_prx(int intf,int idx)
{
	return (struct sar_pRx *)(first_sar_prx[intf]+idx);
}

struct ext_Rx *ext_prx(int intf,int idx)
{
	return (struct ext_Rx *)(first_ext_prx[intf]+idx);
}

struct mac_pTx *mac_ptx(int intf,int idx)
{
	return (struct mac_pTx *)(first_mac_ptx[intf]+idx);
}

struct sar_pTx *sar_ptx(int intf,int idx)
{
	return (struct sar_pTx *)(first_sar_ptx[intf]+idx);
}

struct ext_Tx *ext_ptx(int intf,int idx)
{
	return (struct ext_Tx *)(first_ext_ptx[intf]+idx);
}

struct mac_vRx *mac_vrx(int intf,int idx)
{
	return (struct mac_vRx *)(first_mac_vrx[intf]+idx);
}

struct sar_vRx *sar_vrx(int intf,int idx)
{
	return (struct sar_vRx *)(first_sar_vrx[intf]+idx);
}

struct ext_Rx *ext_vrx(int intf,int idx)
{
	return (struct ext_Rx *)(first_ext_vrx[intf]+idx);
}

struct mac_vTx *mac_vtx(int intf,int idx)
{
	return (struct mac_vTx *)(first_mac_vtx[intf]+idx);
}

struct sar_vTx *sar_vtx(int intf,int idx)
{
	return (struct sar_vTx *)(first_sar_vtx[intf]+idx);
}

struct ext_Tx *ext_vtx(int intf,int idx)
{
	return (struct ext_Tx *)(first_ext_vtx[intf]+idx);
}

struct sp_pRx *sp_to_mac(int intf,int idx)
{
	return (struct sp_pRx *)(first_sp_to_mac[intf]+idx);
}

struct sp_pRx *sp_to_sar(int intf,int idx)
{
	return (struct sp_pRx *)(first_sp_to_sar[intf]+idx);
}

struct sp_pRx *sp_to_ext(int intf,int idx)
{
	return (struct sp_pRx *)(first_sp_to_ext[intf]+idx);
}

unsigned char *sram_buf=NULL;
#if 0
void model_interrupt(int intfidx, uint32 isr)
{
	uint32 imr;
	MT_RINGCTRL("call");
	if(intfidx<SAR_INTFS) //SAR Intf
	{
		imr=READ_MEM32(SARIMR);
		WRITE_VIR32(SARISR,READ_VIR32(SARISR)|isr);

		WRITE_VIR32(SARRDAI,READ_VIR32(SARRDAI)|(1<<intfidx));
		
		if(imr&isr)
		{
			MT_RINGCTRL_DEBUG("isr=0x%x\n",isr);
			sar_intHandler(0,NULL);
		}
		
	}
	else if(intfidx==SAR_INTFS) //MAC0 Intf
	{
		imr=READ_MEM32(MAC0IMR);

		WRITE_VIR32(MAC0ISR,READ_VIR32(MAC0ISR)|isr);		
		if(imr&isr)
		{

			mac_intHandler(0,NULL);
		}
		
	}
	else if(intfidx<SAR_INTFS+MAC_TX_INTFS) //MAC1 Intf
	{
		imr=READ_MEM32(MAC1IMR);
		WRITE_VIR32(MAC1ISR,READ_VIR32(MAC1ISR)|isr);			
		if(imr&isr)
		{
			mac_intHandler(0,NULL);
		}
		
	}
	else //EXT Intf
	{
		imr=READ_MEM32(EPIMR);
		WRITE_VIR32(EPISR,READ_VIR32(EPISR)|isr);			
		if(imr&isr)
		{
			ext_intHandler(0,NULL);
		}
		
	}

}

void model_ale(hsb_param_t *hsb, hsa_param_t *hsa)
{
	hsa_param_t *hsa_real=(hsa_param_t *)HSA_BASE;
	MT_RINGCTRL("call");	
	rtl8672_ALE(hsb,hsa);
	MT_RINGCTRL_DEBUG("AFTER ALE hsa->reason=%d dstportidx=%d tocpu=%d drop=%d\n",hsa->reason,hsa->dstportidx,hsa->tocpu,hsa->droppacket);
	IN_MT_RINGCTRL(
		dumpHSA(hsa);
	)
	memcpy(hsa_real,hsa,sizeof(hsa_param_t ));
	MT_RINGCTRL("out");
}


void model_parser(int srcPortIdx,void* pDesc,hsb_param_t *hsb)
{
	rtl8672_tblAsicDrv_intfParam_t intf;	/* Source Interface */
	hsb_param_t *hsb_real=(hsb_param_t *)HSB_BASE;
		
	MT_RINGCTRL("call");

	rtl8672_getAsicNetInterface(srcPortIdx, &intf);	
	MT_RINGCTRL_DEBUG("Before Parser: srcPortIdx=%d intf.IfType=%d\n",srcPortIdx,intf.IfType);

	rtl8672_pktParser(srcPortIdx,pDesc,hsb,&intf);
	MT_RINGCTRL_DEBUG("AFTER Parser Reason=%d\n",hsb->reason);
	IN_MT_RINGCTRL(
		dumpHSB(hsb);
	)

	memcpy(hsb_real,hsb,sizeof(hsb_param_t ));
	MT_RINGCTRL("out");
	
}

void model_translator(void* pDesc,hsa_param_t *hsa)
{
	MT_RINGCTRL("call");
	rtl8672_pktTranslator(pDesc,hsa);
	MT_RINGCTRL("out");
}

void model_ring_controller_init(void)
{
	int i;
	MT_RINGCTRL("call");
	for(i=0;i<MAC_TX_INTFS;i++)
	{

		if(i<MAC_RX_INTFS)
		{
			current_mac_prx[i]=first_mac_prx[i];
//			current_mac_vrx[i]=&mac_vrx[i][0];
			pp_mac_prx[i]=first_mac_prx[i];
			pp_mac_vrx[i]=first_mac_vrx[i];
		}
		
		current_mac_ptx[i]=first_mac_ptx[i];
//		current_mac_vtx[i]=&mac_vtx[i][0];		
		current_sp_to_mac[i]=first_sp_to_mac[i];		
		pp_mac_ptx[i]=first_mac_ptx[i];
		pp_mac_vtx[i]=first_mac_vtx[i];
		pp_sp_to_mac[i]=first_sp_to_mac[i];
		
	}

	for(i=0;i<SAR_INTFS;i++)
	{
		current_sar_prx[i]=first_sar_prx[i];
		current_sar_ptx[i]=first_sar_ptx[i];
//		current_sar_vrx[i]=&sar_vrx[i][0];
//		current_sar_vtx[i]=&sar_vtx[i][0];
		current_sp_to_sar[i]=first_sp_to_sar[i];
		pp_sar_prx[i]=first_sar_prx[i];
		pp_sar_ptx[i]=first_sar_ptx[i];
		pp_sar_vrx[i]=first_sar_vrx[i];
		pp_sar_vtx[i]=first_sar_vtx[i];
		pp_sp_to_sar[i]=first_sp_to_sar[i];
	}

	for(i=0;i<EXT_INTFS;i++)
	{
		current_ext_prx[i]=first_ext_prx[i];
		current_ext_ptx[i]=first_ext_ptx[i];
//		current_ext_vrx[i]=&ext_vrx[i][0];
//		current_ext_vtx[i]=&ext_vtx[i][0];
		current_sp_to_ext[i]=first_sp_to_ext[i];
		pp_ext_prx[i]=first_ext_prx[i];
		pp_ext_ptx[i]=first_ext_ptx[i];
		pp_ext_vrx[i]=first_ext_vrx[i];
		pp_ext_vtx[i]=first_ext_vtx[i];
		pp_sp_to_ext[i]=first_sp_to_ext[i];
	}


}


int model_mac_rx(int intf,unsigned char *pkt,int length)
{
#if defined(RTL867X_MODEL_KERNEL) || defined(MAC_LLIP_SIM)
	struct mac_pRx *rx;
#endif

#ifndef MAC_LLIP_SIM
#ifdef RTL867X_MODEL_KERNEL
	int loopcnt = 0;	
#endif
#endif
	MT_RINGCTRL("call");
	ASSERT(intf<MAC_RX_INTFS);
	WRITE_MEM32(PPCR,READ_MEM32(PPCR)&(~PPCR_START));

#ifdef MAC_LLIP_SIM
	rx=current_mac_prx[intf];
	MT_RINGCTRL_DEBUG("rx=%x\n",(uint32)rx);

	if(rx->own==1)
	{

		rx->data_length=length;  // CRC included


#if 0
	uint32 ipv6:1;
	uint32 ipsec:1;
	uint32 e8023:1;
	uint32 pppoe:1;
	uint32 pam:1;	
#endif	


		if((pkt[0]==0xff)&&(pkt[1]==0xff)&&(pkt[2]==0xff)&&(pkt[3]==0xff)&&(pkt[4]==0xff)&&(pkt[5]==0xff))
		{
			rx->bar=1;
		}
		else if((pkt[0]&0x1)==1) 
		{
			rx->mar=1;		
		}
		else
		{
			rx->bar=0;
			rx->mar=0;
		}
		
		rx->reason=0;
		
		/* avoid resaon=1 */
		rx->ls=1; 
		rx->fs=1; 
		rx->lpkt=0;
		rx->fae=0;
		rx->runt=0;

		/* avoid reaon=2 */
		rx->crc=0;

		/* avoid check error */
		rx->pid0=1;
		rx->pid1=0;
		rx->tcpf=0;
		rx->udpf=0;
		rx->ipf=0;
		
		if(rx->sram_en == 0) //using SDRAM only
		{
			MT_RINGCTRL_DEBUG("MAC DMA to SDRAM\n");
			memcpy((void *)(rx->rx_buffer_addr+rx->rx_shift),pkt,length);
		}
		else //using SRAM + SDRAM
		{
			rtl8672_tblAsicDrv_intfParam_t input_intf;
			rtl8672_getAsicNetInterface(intf, &input_intf);

			if((rx->sram_map_of <= rx->rx_shift)&&(rx->mar!=1)&&(rx->bar!=1))
			{
				unsigned char *start_sram_dma=NULL;
				int sram_dma_size=0;
				sram_dma_size=128+rx->sram_map_of-rx->rx_shift;
				start_sram_dma=sram_buf+(128*rx->sram_map_id)+(128-sram_dma_size);				
				MT_RINGCTRL_DEBUG("start_sram_dma=0x%x sram_buf=0x%x rx->sram_map_id=%d rx->sram_map_of=%d rx->rx_shift=%d\n",(uint32)start_sram_dma,(uint32)sram_buf,rx->sram_map_id,rx->sram_map_of,rx->rx_shift);
				memcpy((void *)start_sram_dma,pkt,(length<sram_dma_size)?length:sram_dma_size); // dma to SRAM

				if(length>sram_dma_size)
				{
					memcpy((void *)(rx->rx_buffer_addr+rx->rx_shift+sram_dma_size),pkt+sram_dma_size,length-sram_dma_size); // dma to SDRAM
				}
				MT_RINGCTRL_DEBUG("MAC DMA to SRAM sram_id=%d sram_dma_size=%d sram_map_of=%d\n",rx->sram_map_id,sram_dma_size,rx->sram_map_of);
//				memDump(sram_buf+(128*rx->sram_map_id),128,"sram");
//				memDump((void*)rx->rx_buffer_addr+rx->rx_shift,256,"sdram");
				
				
			}
			else // exception: config error, using SDRAM only
			{
				MT_RINGCTRL_DEBUG("MAC DMA to SDRAM\n");
				memcpy((void *)(rx->rx_buffer_addr+rx->rx_shift),pkt,length);
			}

		}

#if 0
	uint32 vlan_vidh:4;
	uint32 vlan_cfi:1;
	uint32 vlan_prio:3;
	uint32 vlan_vidl:8;
	uint32 ava:1;
	uint32 partial_checksum:16;
	uint32 skb_header_addr;	
#endif		


		if(rx->eor==0)
		{
			current_mac_prx[intf]++;		
		}
		else
		{
			current_mac_prx[intf]=mac_prx(intf,0);
		}
		mac_prx_ring_used[intf]++;
		MT_RINGCTRL_DEBUG("MAC PRX desc=%x used=%d eor=%d datalen=%d rxbufaddr=%x rxshift=%d\n",(uint32)rx, mac_prx_ring_used[intf],rx->eor,rx->data_length,rx->rx_buffer_addr,rx->rx_shift);
		rx->own=0;
		


		WRITE_MEM32(LLIP_SIM_EN,0x1);
		WRITE_MEM32(LLIP_SIM_RX,((1<<SAR_INTFS)<<intf));
	

		
	}
	else // ring full
	{
		MT_RINGCTRL_DEBUG("MAC pRx ring full\n");
		return FAILED;
	}
#else
#ifdef RTL867X_MODEL_KERNEL
		rx=mac_prx(intf,(READ_MEM32(MPRXDESC0+(intf<<2))>>8)&0xff);
		rtl8672_send_pkt_by_rtl8139(2,pkt,length-4); /* because length is CRC inclued */
//		rtlglue_printf("Waiting MAC Port DMA...\n");
		loopcnt = 0;
		while(1)
		{
			if(rx->own==0) break;  /* hw write the own bit */
			rtlglue_printf("rx=%x rx->own=%x\n",(uint32)rx,rx->own);
			if(loopcnt > 3000){
				
				rtlglue_printf("MAC receive nothing ... rx=%x rx->own=%x rxhdr=%x\n",(uint32)rx,rx->own,*(uint32*)rx);
				rtl8672_dumpRing(0,9);
				rtlglue_printf("MAC receive nothing ... rx=%x rx->own=%x rxhdr=%x\n",(uint32)rx,rx->own,*(uint32*)rx);
				return FAILED;
			}
			loopcnt++;
		}
			
#endif
#endif

	return SUCCESS;

}

int model_sar_rx(int intf,unsigned char *pkt,int length)
{
	struct sar_pRx *rx=NULL;
	rtl8672_tblAsicDrv_intfParam_t input_intf;
#ifdef SAR_LLIP_SIM	
	uint8 *dmac;
#endif
	MT_RINGCTRL("call");

	rtl8672_getAsicNetInterface(intf, &input_intf);	
	
	ASSERT(intf<SAR_INTFS);
	WRITE_MEM32(PPCR,READ_MEM32(PPCR)&(~PPCR_START));
	
	if(length>READ_MEM32(BUFFERSIZE))
	{
		// FIXME: not implemented yet!!
		return -1;	
	}

#ifdef SAR_LLIP_SIM	
	rx=current_sar_prx[intf];

	if(rx->own==1)
	{

		rx->data_length=length;  //not include crc
		rx->bpc = length;
		rx->crc32err=0;
		rx->ipcerr=0;		
		rx->ipv6=0;
		rx->l4csmf=0;		
		rx->bpc=length;		
#if 0
		rx->clp=0;
		rx->pti=0;
		rx->trlr=0;

		rx->tucacc=0;
		rx->wii=0;
		rx->ppi=0; /* tcp */
#endif		
		


#if 0

	uint32 ipv6:1;
	uint32 l4csmf:1;

	uint32 clp:1;
	uint32 pti:3;
	uint32 wii:1;
	uint32 frgi:1;
	uint32 ppi:2;
	uint32 ipcerr:1;
	uint32 lenerr:1;
	uint32 crc32err:1;
	uint32 atmport:1;
	uint32 bpc:16;
	uint32 trlr:16;
	uint32 tucacc:16;
	uint32 reason:5;
	uint32 skb_header_addr;

#endif	

		rx->ls=1; 
		rx->fs=1; 

		rx->bar=0;
		rx->mar=0;

		switch(input_intf.SARhdr)
		{
			case 1: //bridged
			case 2: //mer
			case 3: //pppoe
				if(input_intf.L2Encap==1) // VC
				{
					dmac=pkt+10;					
				}
				else
				{
					dmac=pkt+2;	
				}
				
				if((dmac[0]==0xff)&&(dmac[1]==0xff)&&(dmac[2]==0xff)&&(dmac[3]==0xff)&&(dmac[4]==0xff)&&(dmac[5]==0xff))
				{
					rx->bar=1;
				}
				else if((dmac[0]&0x1)==1) 
				{
					rx->mar=1;		
				}
				break;
		}
	

		if(rx->sram_en==0) //using SDRAM only
		{
			MT_RINGCTRL_DEBUG("SAR DMA to SDRAM\n");		
			memcpy((void *)(rx->rx_buffer_addr+rx->rx_shift),pkt,length);
		}
		else //using SRAM + SDRAM
		{


			if((rx->sram_map_of <= rx->rx_shift)&&(rx->bar==0)&&(rx->mar==0))
			{
				unsigned char *start_sram_dma=NULL;
				int sram_dma_size=0;
				sram_dma_size=128+rx->sram_map_of-rx->rx_shift;
				start_sram_dma=sram_buf+(128*rx->sram_map_id)+(128-sram_dma_size);

				memcpy((void *)start_sram_dma,pkt,(length<sram_dma_size)?length:sram_dma_size); // dma to SRAM

				if(length>sram_dma_size)
					memcpy((void *)(rx->rx_buffer_addr+rx->rx_shift+sram_dma_size),pkt+sram_dma_size,length-sram_dma_size); // dma to SDRAM

				MT_RINGCTRL_DEBUG("SAR dma to SRAM sram_id=%d sram_dma_size=%d\n",rx->sram_map_id,sram_dma_size);
//				memDump(sram_buf+(128*rx->sram_map_id),128,"sram");
//				memDump((void*)rx->rx_buffer_addr+rx->rx_shift,256,"sdram");
				
				
			}
			else // exception: config error, using SDRAM only
			{
				MT_RINGCTRL_DEBUG("SAR DMA to SDRAM\n");
				memcpy((void *)(rx->rx_buffer_addr+rx->rx_shift),pkt,length);
			}

		}

#if 0
	uint32 vlan_vidh:4;
	uint32 vlan_cfi:1;
	uint32 vlan_prio:3;
	uint32 vlan_vidl:8;
	uint32 ava:1;
	uint32 partial_checksum:16;
	uint32 skb_header_addr;	
#endif		

		rx->reason=0; // not to CPU
		

		if(rx->eor==0)
		{
			current_sar_prx[intf]++;		
		}
		else
		{
			current_sar_prx[intf]=first_sar_prx[intf];
		}

		sar_prx_ring_used[intf]++;
		MT_RINGCTRL_DEBUG("SAR PRX desc=%x used=%d eor=%d datalen=%d rxbufaddr=%x rxshift=%d\n",(uint32)rx, sar_prx_ring_used[intf],rx->eor,rx->data_length,rx->rx_buffer_addr,rx->rx_shift);
		
		rx->own=0;
		
#if 0
#if 0
		{
			int i;
			for(i=0;i<192000;i++)  // delay 0.5 sec
			{
				rtlglue_printf("\r");
			}
		}
#else
		while(1)
		{
			struct sar_pRx *uncache=(struct sar_pRx *)((uint32)rx|0xa0000000);
			rtlglue_printf("rx->own=%x\n",uncache->own);
			if(uncache->own==0) break;
		}
#endif
#endif

		WRITE_MEM32(LLIP_SIM_EN,0x1);
		WRITE_MEM32(LLIP_SIM_RX,(1<<intf));
	

	}
	else // ring full
	{
		return -1;
	}

#else
#ifdef RTL867X_MODEL_KERNEL
		rx=sar_prx(intf,(READ_MEM32(SPRXDESC0+(intf<<2))>>8)&0xff);
		rtl8672_send_pkt_by_rtl8139(0,pkt,length); 
		rtlglue_printf("Waiting SAR Port DMA...\n");
		while(1)
		{
			if(rx->own==0) break;  /* hw write the own bit */
			rtlglue_printf("rx=%x rx->own=%x\n",(uint32)rx,rx->own);			
		}
#endif
#endif

	return 0;
}


struct sk_buff *model_ext_rx(int intf,int length,struct sk_buff *rxskb,uint32 linkid)
{

	/* the follow code is not model code, must program the following ring contorl code in device driver(ex: 8139, 8150, 8190...).  */
	
	struct ext_Rx *rx=current_ext_prx[intf];
	struct sk_buff *result_skb=NULL;

	MT_RINGCTRL("call");
	WRITE_MEM32(PPCR,READ_MEM32(PPCR)&(~PPCR_START));

	if(length>READ_MEM32(BUFFERSIZE))
	{
		/* send this packet by pass packet processor */
		return NULL;	
	}

	if(rx->own==1) /* find a free extension port desc */
	{	
		rx->data_length=length;  //not include crc
		rx->rx_buffer_addr=(uint32)(rxskb->head);
		rx->rx_shift=(uint32)rxskb->data-(uint32)rxskb->head;
		result_skb=(struct sk_buff *)rx->skb_header_addr;
		rx->skb_header_addr=(uint32)rxskb;
		rx->linkid=linkid;
		rx->reason=0; // not to CPU
		

		if(rx->eor==0)
		{
			current_ext_prx[intf]++;		
		}
		else
		{
			current_ext_prx[intf]=first_ext_prx[intf];
		}

		ext_prx_ring_used[intf]++;		
		
		MT_RINGCTRL_DEBUG("EXT PRX desc=%x used=%d eor=%d datalen=%d rxbufaddr=%x rxshift=%d skb_eader_addr=%x linkid=%d\n",(uint32)rx, ext_prx_ring_used[intf],rx->eor,rx->data_length,rx->rx_buffer_addr,rx->rx_shift,rx->skb_header_addr,rx->linkid);
		rx->own=0;
		return result_skb;

		
	}
	else // ring full
	{
		MT_RINGCTRL_DEBUG("EXT PRX FULL\n");
		// Drop or send by CPU
	}

	return NULL;

}




void model_sar_tx(void)
{
//tx by real SAR
#if 0
	int intf,i;
	int startidx,endidx;	
	MT_RINGCTRL("call");	
	for(intf=0;intf<SAR_INTFS;intf++)
	{
		startidx=current_sar_ptx[intf]-first_sar_ptx[intf];
		endidx=(READ_MEM32(SPTXDESC0+(intf<<2))>>8)&0xff;


		if(startidx>endidx) endidx+=sar_ptx_ring_size[intf];

		if((startidx==endidx)&&((READ_MEM32(SPTXDESC0+(intf<<2))&0x10000)!=0)) endidx+=sar_ptx_ring_size[intf];

		MT_RINGCTRL_DEBUG("start=%d end=%d\n",startidx,endidx);	

		for(i=startidx;i<endidx;i++)
		{
			struct sar_pTx *ptx=sar_ptx(intf,i%sar_ptx_ring_size[intf]);
//		MT_RINGCTRL_DEBUG("intf=%d startidx=%d endidx=%d\n",intf,startidx,endidx);		
			if(ptx->own==1) /* cpu own and not put to tx yet , try to fwd to ptx */
			{
				struct sp_pRx *ptr;
				MT_RINGCTRL_DEBUG("SAR PTX intf=%x tx length=%d sramid=%d sram_sz=%d fcpu=%d orgAddr=%x\n",intf,ptx->data_length,ptx->sram_map_id,ptx->sram_size,ptx->fcpu,ptx->orgAddr<<2);

				ptr=	(struct sp_pRx *)(ptx->orgAddr<<2);
				
				if(ptx->fcpu==0)
				{
					ptr->own=1;
//					ptr->data_length=READ_VIR32(BUFFERSIZE)&0xfff;					
				}
				else
				{
					ptr->own=0;
				}
				ptr->eor=ptx->oeor;
				

				if(current_sar_ptx[intf]->eor==0)
					current_sar_ptx[intf]++;
				else
					current_sar_ptx[intf]=first_sar_ptx[intf];
				sar_ptx_ring_used[intf]--;


#ifdef SAR_LLIP_SIM
				WRITE_MEM32(LLIP_SIM_EN,0x1);
				WRITE_MEM32(LLIP_SIM_TX,(1<<intf));
#endif		


				ptx->own=0;
			}
		}
	}
#endif
}


void model_mac_tx(void)
{
	int intf,i;
	int startidx,endidx;	
	MT_RINGCTRL("call");	


	for(intf=0;intf<MAC_TX_INTFS;intf++)
	{
		startidx=current_mac_ptx[intf]-first_mac_ptx[intf];
		endidx=(READ_MEM32(MPTXDESC0+(intf<<2))>>8)&0xff;

		if(startidx>endidx) endidx+=mac_ptx_ring_size[intf];

		if((startidx==endidx)&&((READ_MEM32(MPTXDESC0+(intf<<2))&0x10000)!=0)) endidx+=mac_ptx_ring_size[intf];

		MT_RINGCTRL_DEBUG("start=%d end=%d\n",startidx,endidx);	

		for(i=startidx;i<endidx;i++)
		{
			struct mac_pTx *ptx=mac_ptx(intf,i%mac_ptx_ring_size[intf]);

		
//		MT_RINGCTRL_DEBUG("intf=%d startidx=%d endidx=%d\n",intf,startidx,endidx);		
			if(ptx->own==1) /* cpu own and not put to tx yet , try to fwd to ptx */
			{
				struct sp_pRx *ptr;
				MT_RINGCTRL_DEBUG("MAC TX intf=%x tx length=%d sramid=%d\n",intf,ptx->data_length,ptx->sram_map_id);

				ptr=	(struct sp_pRx *)(ptx->orgAddr<<2);
				
				if(ptx->fcpu==0)
				{
					ptr->own=1;
//					ptr->data_length=READ_VIR32(BUFFERSIZE)&0xfff;
				}
				else
				{
					ptr->own=0;
				}
				
				ptr->eor=ptx->oeor;


				if(current_mac_ptx[intf]->eor==0)
					current_mac_ptx[intf]++;
				else
					current_mac_ptx[intf]=first_mac_ptx[intf];

				mac_ptx_ring_used[intf]--;


#ifdef MAC_LLIP_SIM
				WRITE_MEM32(LLIP_SIM_EN,0x1);
				WRITE_MEM32(LLIP_SIM_TX,((1<<SAR_INTFS)<<intf));
#endif		


				ptx->own=0;		
				
			}
		}
	}

}

void model_ext_tx(int intf)
{

//	int i,startidx,endidx;
	MT_RINGCTRL("call");
	// FIXME: hw will trigger an interrupt to cpu when this event.
	MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);	

	model_interrupt(intf+SAR_INTFS+MAC_TX_INTFS, EXT0_PTX_OK<<intf );	  
			
#if 0	
	for(intf=0;intf<EXT_INTFS;intf++)
	{
		startidx=current_ext_ptx[intf]-first_ext_ptx[intf];
		endidx=pp_ext_ptx[intf]-first_ext_ptx[intf];


		if(startidx>endidx) endidx+=ext_ptx_ring_size[intf];

		if((startidx==endidx)&&(ext_ptx_ring_used[intf]!=0)) endidx+=ext_ptx_ring_size[intf];

		for(i=startidx;i<endidx;i++)
		{
			struct ext_Tx *ptx=ext_ptx(intf,i%ext_ptx_ring_size[intf]);
			
//		MT_RINGCTRL_DEBUG("intf=%d startidx=%d endidx=%d\n",intf,startidx,endidx);		
			if(ptx->own==1) /* cpu own and not put to tx yet , try to fwd to ptx */
			{
				struct sp_pRx *ptr;
				MT_RINGCTRL_DEBUG("EXT TX intf=%x tx length=%d\n",intf,ptx->data_length);
				ptx->own=0;
				ptr=	(struct sp_pRx *)(ptx->orgAddr<<2);
			
				if(ptx->fcpu==0)
				{
					ptr->own=1;
					ptr->data_length=READ_VIR32(BUFFERSIZE)&0xfff;									
				}
				else
				{
					ptr->own=0;
				}
				ptr->eor=ptx->oeor;


				if(current_ext_ptx[intf]->eor==0)
					current_ext_ptx[intf]++;
				else
					current_ext_ptx[intf]=first_ext_ptx[intf];

				ext_ptx_ring_used[intf]--;
//				dump_mac_prx(intf);
//				dump_sp_to_ext(intf);

					
				
			}
		}
	}
#endif	
		
}


void model_pp_swap_sar_sp(struct sar_pRx *sar,struct sp_pRx *swap,int rx_shift)
{
	struct sp_pRx cache;
	MT_RINGCTRL("call");	
	// spec step 2
	memcpy(&cache,swap,sizeof(struct sp_pRx));

	// spec step 3
	cache.own=0;

	// spec step 4
	cache.rx_buffer_addr=sar->rx_buffer_addr;

	// spec step 5
	//cache.data_length=sar->data_length;
	// spec step 6
	//cache.rx_shift=sar->rx_shift;	
	//WRITE_MEM32(XLATOR_DBG_BASE,(sar->rx_shift<<XLATOR_RXSHIFT_OFFSET)|sar->data_length);

	// spec step 7
	cache.skb_header_addr=sar->skb_header_addr;

	// spec step 8	
	cache.sram_en=sar->sram_en;		
	cache.sram_map_id=sar->sram_map_id;
	cache.sram_map_of=sar->sram_map_of;

	// spec step 9
	sar->rx_buffer_addr=swap->rx_buffer_addr;

	// spec step 10	
	sar->skb_header_addr=swap->skb_header_addr;

	// spec step 11	
	sar->sram_en=swap->sram_en;
	sar->sram_map_id=swap->sram_map_id;

	// spec step 12
	sar->data_length=READ_MEM32(BUFFERSIZE);  
	sar->rx_shift=rx_shift; //read this from interface table

	// spec step 13
	sar->own=1;

	// spec step 14
	memcpy(swap,&cache,sizeof(struct sp_pRx));	
	
}


void model_pp_swap_mac_sp(struct mac_pRx *mac,struct sp_pRx *swap,int rx_shift)
{
	struct sp_pRx cache;
	MT_RINGCTRL("call");
	
	// spec step 2
	memcpy(&cache,swap,sizeof(struct sp_pRx));

	// spec step 3
	cache.own=0;

	// spec step 4
	cache.rx_buffer_addr=mac->rx_buffer_addr;

	// spec step 5
	//cache.data_length=mac->data_length;
	// spec step 6
	//cache.rx_shift=mac->rx_shift;
	//WRITE_MEM32(XLATOR_DBG_BASE,(mac->rx_shift<<XLATOR_RXSHIFT_OFFSET)|mac->data_length);	

	// spec step 7
	cache.skb_header_addr=mac->skb_header_addr;

	// spec step 8	
	cache.sram_en=mac->sram_en;		
	cache.sram_map_id=mac->sram_map_id;
	cache.sram_map_of=mac->sram_map_of;

	// spec step 9
	mac->rx_buffer_addr=swap->rx_buffer_addr;

	// spec step 10	
	mac->skb_header_addr=swap->skb_header_addr;

	// spec step 11	
	mac->sram_en=swap->sram_en;
	mac->sram_map_id=swap->sram_map_id;


	// spec step 12
	mac->data_length=READ_MEM32(BUFFERSIZE);  
	mac->rx_shift=rx_shift; //read this from interface table
	
	// spec step 13
	mac->own=1;

	// spec step 14
	memcpy(swap,&cache,sizeof(struct sp_pRx));	
	
}

void model_pp_swap_ext_sp(struct ext_Rx *ext,struct sp_pRx *swap,int rx_shift)
{
	struct sp_pRx cache;
	MT_RINGCTRL("call");
	
	// spec step 2
	memcpy(&cache,swap,sizeof(struct sp_pRx));

	// spec step 3
	cache.own=0;

	// spec step 4
	cache.rx_buffer_addr=ext->rx_buffer_addr;

	// spec step 5
	//cache.data_length=ext->data_length;
	// spec step 6
	//cache.rx_shift=ext->rx_shift;	
	//WRITE_MEM32(XLATOR_DBG_BASE,(ext->rx_shift<<XLATOR_RXSHIFT_OFFSET)|ext->data_length);

	// spec step 7
	cache.skb_header_addr=ext->skb_header_addr;

	// spec step 8	
	cache_linkid=ext->linkid;

	// spec step 9
	ext->rx_buffer_addr=swap->rx_buffer_addr;

	// spec step 10	
	ext->skb_header_addr=swap->skb_header_addr;

	// spec step 12 
	ext->data_length=READ_MEM32(BUFFERSIZE);  
	ext->rx_shift=rx_shift; 

	// spec step 13
	ext->linkid=0;
	ext->own=1;

	// spec step 14
	memcpy(swap,&cache,sizeof(struct sp_pRx));	
}


void model_pp_sar_to_sar(int sar_intf_idx,int desc_idx,int dst_port_idx)
{

	MT_RINGCTRL("call");
	// dump_sp_to_sar(0);
	if(pp_sp_to_sar[dst_port_idx]->own==1)							
	{
		struct sar_pRx *rsar=sar_prx(sar_intf_idx,desc_idx%sar_prx_ring_size[sar_intf_idx]);
		struct sp_pRx *swap=pp_sp_to_sar[dst_port_idx];
		struct sar_pTx *sar=pp_sar_ptx[dst_port_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;


		// SWAP pRx and share pool

		rtl8672_getAsicNetInterface(sar_intf_idx, &intf);	
		model_pp_swap_sar_sp(rsar,swap,intf.RXshift);
		MT_RINGCTRL_DEBUG("SWAP SAR_PRX[%d][%d]=%x with SAR_SP[%d][%d]=%x\n",sar_intf_idx,desc_idx%sar_prx_ring_size[sar_intf_idx],(uint32)rsar,dst_port_idx,swap-sp_to_sar(dst_port_idx,0),(uint32)swap);
		sp_to_sar_fwd[dst_port_idx]++;		

		// mapping share pool to pTx , step 15~22

		// spec step 15
		sar->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);

		// spec step 16
		sar->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);

		// spec step 17
		//sar->skb_header_addr =swap->skb_header_addr;
		
		// spec step 18
		sar->sram_map_id=swap->sram_map_id;

		// spec step 19
		{
			int size=((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET)-swap->sram_map_of;		
			if((size<=128)&&(size>=0))
			{
				sar->sram_size=128-size;
			}
			else if(size>128)
			{
				sar->sram_size=0;
			}
			else if(size<0)
			{
				// copy sram data into sdram
				memcpy((void*)swap->rx_buffer_addr+swap->sram_map_of, sram_buf+128*swap->sram_map_id,128);
				sar->sram_size=0;
			}
		}

		// spec step 20
		sar->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		sar->oeor=swap->eor;
		

		// spec step 21
		sar->fcpu=0;

		// spec 21-1 , new add
		sar->fs=1;
		sar->ls=1;
		
		// spec step 21-2 , new add
		rtl8672_getAsicNetInterface(dst_port_idx, &intf);	
		sar->atmport=intf.ATMPORT;
		sar->trlren=intf.TRLREN;
		sar->clp=intf.CLP;
		sar->pti=intf.PTI;
		sar->trlr=intf.TRLR;

		if(intf.LanFCS==0)
			sar->ethnt_offset=0x3f;
		else
			sar->ethnt_offset=ethnt_offset_map[intf.L2Encap][intf.SARhdr];
		
		// spec step 22
		sar->own=1;

	
		

		// move pTx to next entry
		if(sar->eor==0)
		{
			pp_sar_ptx[dst_port_idx]++;
		}
		else
		{
			pp_sar_ptx[dst_port_idx]=first_sar_ptx[dst_port_idx];
		}
		sar_ptx_ring_used[dst_port_idx]++;

		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_sar[dst_port_idx]++;
		}
		else
		{							
			pp_sp_to_sar[dst_port_idx]=first_sp_to_sar[dst_port_idx];
		}

		model_sar_tx();
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);
	}
}

void model_pp_sar_to_mac(int sar_intf_idx,int desc_idx,int dst_port_idx)
{
	MT_RINGCTRL("call");
	
	if(pp_sp_to_mac[dst_port_idx]->own==1)							
	{
		struct sar_pRx *sar=sar_prx(sar_intf_idx,desc_idx%sar_prx_ring_size[sar_intf_idx]);
		struct sp_pRx *swap=pp_sp_to_mac[dst_port_idx];
		struct mac_pTx *mac2=pp_mac_ptx[dst_port_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;

	
		// SWAP pRx and share pool
		rtl8672_getAsicNetInterface(sar_intf_idx, &intf);
		model_pp_swap_sar_sp(sar,swap,intf.RXshift);		
		MT_RINGCTRL_DEBUG("SWAP SAR_PRX[%d][%d]=%x with MAC_SP[%d][%d]=%x\n",sar_intf_idx,desc_idx%sar_prx_ring_size[sar_intf_idx],(uint32)sar,dst_port_idx,swap-sp_to_mac(dst_port_idx,0),(uint32)swap);
		sp_to_mac_fwd[dst_port_idx]++;		

		// mapping share pool to pTx , step 15~22

		// spec step 15
		mac2->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);

		// spec step 16
		mac2->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);

		// spec step 17
		//mac2->skb_header_addr =swap->skb_header_addr;
		
		// spec step 18
		mac2->sram_map_id=swap->sram_map_id;

		// spec step 19
		{
			int size=((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET)-swap->sram_map_of;		
			if((size<=128)&&(size>=0))
			{
				mac2->sram_size=128-size;
			}
			else if(size>128)
			{
				mac2->sram_size=0;
			}
			else if(size<0)
			{
				// copy sram data into sdram
				memcpy((void*)swap->rx_buffer_addr+swap->sram_map_of, sram_buf+128*swap->sram_map_id,128);
				mac2->sram_size=0;
			}
		}

		// spec step 20
		mac2->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		mac2->oeor=swap->eor;		

		// spec step 21
		mac2->fcpu=0;

		// spec 21-1 , new add
		mac2->fs=1;
		mac2->ls=1;
		mac2->crc=1;
		mac2->tagc=0;
		mac2->vlan_vidl=0;
		mac2->vlan_prio=0;		
		mac2->vlan_cfi=0;		
		mac2->vlan_vidh=0;		
		

		// spec step 22
		mac2->own=1;
		

		// move pTx to next entry
		if(mac2->eor==0)
		{
			pp_mac_ptx[dst_port_idx]++;
		}
		else
		{
			pp_mac_ptx[dst_port_idx]=first_mac_ptx[dst_port_idx];
		}
		mac_ptx_ring_used[dst_port_idx]++;

		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_mac[dst_port_idx]++;
		}
		else
		{							
			pp_sp_to_mac[dst_port_idx]=first_sp_to_mac[dst_port_idx];
		}

//		model_mac_tx();
		if(dst_port_idx==0)
		{
			model_interrupt(dst_port_idx+SAR_INTFS,MAC_PTOK0);
		}
		else
		{
			model_interrupt(dst_port_idx+SAR_INTFS,MAC_PTOK0<<(dst_port_idx-1));
		}
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);
	}	
}

void model_pp_sar_to_ext(int sar_intf_idx,int desc_idx,int ext_intf_idx)
{
	MT_RINGCTRL("call");
	
	if(pp_sp_to_ext[ext_intf_idx]->own==1)							
	{
		struct sar_pRx *sar=sar_prx(sar_intf_idx,desc_idx%sar_prx_ring_size[sar_intf_idx]);	
		struct sp_pRx *swap=pp_sp_to_ext[ext_intf_idx];
		struct ext_Tx *ext=pp_ext_ptx[ext_intf_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;

		// SWAP pRx and share pool
		rtl8672_getAsicNetInterface(sar_intf_idx, &intf);		
		model_pp_swap_sar_sp(sar,swap,intf.RXshift);
		MT_RINGCTRL_DEBUG("SWAP SAR_PRX[%d][%d]=%x with EXT_SP[%d][%d]=%x\n",sar_intf_idx,desc_idx%sar_prx_ring_size[sar_intf_idx],(uint32)sar,ext_intf_idx,swap-sp_to_ext(ext_intf_idx,0),(uint32)swap);	
		sp_to_ext_fwd[ext_intf_idx]++;

		// mapping share pool to pTx , step 15~22

		// spec step 15
		ext->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);							

		// spec step 16
		ext->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);

		// spec step 17
		ext->skb_header_addr =swap->skb_header_addr;

		// spec step 18
		ext->linkid=0;

		// spec step 19
		if(swap->sram_en==1)
		{
			// copy sram data into sdram
			memcpy((void*)swap->rx_buffer_addr+swap->sram_map_of, sram_buf+128*swap->sram_map_id,128);
		}


		// spec step 20
		ext->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		ext->oeor=swap->eor;		

		// spec step 21
		ext->fcpu=0;

		// spec step 22
		ext->own=1;
		

		// move pTx to next entry
		if(ext->eor==0)
		{
			pp_ext_ptx[ext_intf_idx]++;
		}
		else
		{
			pp_ext_ptx[ext_intf_idx]=first_ext_ptx[ext_intf_idx];
		}

		ext_ptx_ring_used[ext_intf_idx]++;
		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_ext[ext_intf_idx]++;
		}
		else
		{							
			pp_sp_to_ext[ext_intf_idx]=first_sp_to_ext[ext_intf_idx];
		}

		model_ext_tx(ext_intf_idx);
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);

	}	

}


void model_pp_mac_to_sar(int mac_intf_idx,int desc_idx,int dst_port_idx)
{
	MT_RINGCTRL("call");
	// dump_sp_to_sar(0);
	if(pp_sp_to_sar[dst_port_idx]->own==1)							
	{
		struct mac_pRx *mac=mac_prx(mac_intf_idx,desc_idx%mac_prx_ring_size[mac_intf_idx]);
		struct sp_pRx *swap=pp_sp_to_sar[dst_port_idx];
		struct sar_pTx *sar=pp_sar_ptx[dst_port_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;


		// SWAP pRx and share pool
		rtl8672_getAsicNetInterface(SAR_INTFS+mac_intf_idx, &intf);
		model_pp_swap_mac_sp(mac,swap,intf.RXshift);		
		MT_RINGCTRL_DEBUG("SWAP MAC_PRX[%d][%d]=%x with SAR_SP[%d][%d]=%x\n",mac_intf_idx,desc_idx%mac_prx_ring_size[mac_intf_idx],(uint32)mac,dst_port_idx,swap-sp_to_sar(dst_port_idx,0),(uint32)swap);
		sp_to_sar_fwd[dst_port_idx]++;		

		// mapping share pool to pTx , step 15~22

		// spec step 15
		sar->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);

		// spec step 16
		sar->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);
		
		// spec step 17
		//sar->skb_header_addr =swap->skb_header_addr;
		
		// spec step 18
		sar->sram_map_id=swap->sram_map_id;

		// spec step 19
		{
			int size=((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET)-swap->sram_map_of;
			if((size<=128)&&(size>=0))
			{
				sar->sram_size=128-size;
			}
			else if(size>128)
			{
				sar->sram_size=0;
			}
			else if(size<0)
			{
				// copy sram data into sdram
				memcpy((void*)swap->rx_buffer_addr+swap->sram_map_of, sram_buf+128*swap->sram_map_id,128);
				sar->sram_size=0;
			}

		}


		// spec step 20
		sar->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		sar->oeor=swap->eor;		

		// spec step 21
		sar->fcpu=0;

		// spec 21-1 , new add
		sar->fs=1;
		sar->ls=1;
		
		// spec step 21-2 , new add
		rtl8672_getAsicNetInterface(dst_port_idx, &intf);
		sar->atmport=intf.ATMPORT;
		sar->trlren=intf.TRLREN;
		sar->clp=intf.CLP;
		sar->pti=intf.PTI;
		sar->trlr=intf.TRLR;

		if(intf.LanFCS==0)
			sar->ethnt_offset=0x3f;
		else			
			sar->ethnt_offset=ethnt_offset_map[intf.L2Encap][intf.SARhdr];

		// spec step 22
		sar->own=1;


		
		
		

		// move pTx to next entry
		if(sar->eor==0)
		{
			pp_sar_ptx[dst_port_idx]++;
		}
		else
		{
			pp_sar_ptx[dst_port_idx]=first_sar_ptx[dst_port_idx];
		}
		sar_ptx_ring_used[dst_port_idx]++;

		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_sar[dst_port_idx]++;
		}
		else
		{							
			pp_sp_to_sar[dst_port_idx]=first_sp_to_sar[dst_port_idx];
		}

		model_sar_tx();
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);
	}

}

void model_pp_mac_to_mac(int mac_intf_idx,int desc_idx,int dst_port_idx)
{
	MT_RINGCTRL("call");
	if(pp_sp_to_mac[dst_port_idx]->own==1)							
	{
		struct mac_pRx *mac=mac_prx(mac_intf_idx,desc_idx%mac_prx_ring_size[mac_intf_idx]);
		struct sp_pRx *swap=pp_sp_to_mac[dst_port_idx];
		struct mac_pTx *mac2=pp_mac_ptx[dst_port_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;

		
		
		// SWAP pRx and share pool
		rtl8672_getAsicNetInterface(SAR_INTFS+mac_intf_idx, &intf);		
		
		model_pp_swap_mac_sp(mac,swap,intf.RXshift);				
		MT_RINGCTRL_DEBUG("SWAP MAC_PRX[%d][%d]=%x with MAC_SP[%d][%d]=%x\n",mac_intf_idx,desc_idx%mac_prx_ring_size[mac_intf_idx],(uint32)mac,dst_port_idx,swap-sp_to_mac(dst_port_idx,0),(uint32)swap);		
		sp_to_mac_fwd[dst_port_idx]++;		

		// mapping share pool to pTx , step 15~22

		// spec step 15
		mac2->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);

		// spec step 16
		mac2->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);

		// spec step 17
		//mac2->skb_header_addr =swap->skb_header_addr;
		
		// spec step 18
		mac2->sram_map_id=swap->sram_map_id;

		// spec step 19
		{
			int size=((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET)-swap->sram_map_of;		
			if((size<=128)&&(size>=0))
			{
				mac2->sram_size=128-size;
			}
			else if(size>128)
			{
				mac2->sram_size=0;
			}
			else if(size<0)
			{
				// copy sram data into sdram
				memcpy((void*)swap->rx_buffer_addr+swap->sram_map_of, sram_buf+128*swap->sram_map_id,128);
				mac2->sram_size=0;
			}
		}


		// spec step 20
		mac2->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		mac2->oeor=swap->eor;		

		// spec step 21
		mac2->fcpu=0;

		// spec 21-1 , new add
		mac2->fs=1;
		mac2->ls=1;
		mac2->crc=1;
		mac2->tagc=0;
		mac2->vlan_vidl=0;
		mac2->vlan_prio=0;		
		mac2->vlan_cfi=0;		
		mac2->vlan_vidh=0;				

		// spec step 22
		mac2->own=1;
		

		// move pTx to next entry
		if(mac2->eor==0)
		{
			pp_mac_ptx[dst_port_idx]++;
		}
		else
		{
			pp_mac_ptx[dst_port_idx]=first_mac_ptx[dst_port_idx];
		}
		mac_ptx_ring_used[dst_port_idx]++;

		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_mac[dst_port_idx]++;
		}
		else
		{							
			pp_sp_to_mac[dst_port_idx]=first_sp_to_mac[dst_port_idx];
		}

		//model_mac_tx();
		if(dst_port_idx==0)
		{
			model_interrupt(dst_port_idx+SAR_INTFS,MAC_PTOK0);
		}
		else
		{
			model_interrupt(dst_port_idx+SAR_INTFS,MAC_PTOK0<<(dst_port_idx-1));
		}
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);
	}	
}



void model_pp_mac_to_ext(int mac_intf_idx,int desc_idx,int ext_intf_idx)
{
	MT_RINGCTRL("call");
	if(pp_sp_to_ext[ext_intf_idx]->own==1)							
	{
		struct mac_pRx *mac=mac_prx(mac_intf_idx,desc_idx%mac_prx_ring_size[mac_intf_idx]);	
		struct sp_pRx *swap=pp_sp_to_ext[ext_intf_idx];
		struct ext_Tx *ext=pp_ext_ptx[ext_intf_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;

		
		// SWAP pRx and share pool
		rtl8672_getAsicNetInterface(SAR_INTFS+mac_intf_idx, &intf);				
		model_pp_swap_mac_sp(mac,swap,intf.RXshift);

		MT_RINGCTRL_DEBUG("SWAP MAC_PRX[%d][%d]=%x with EXT_SP[%d][%d]=%x\n",mac_intf_idx,desc_idx%mac_prx_ring_size[mac_intf_idx],(uint32)mac,ext_intf_idx,swap-sp_to_ext(ext_intf_idx,0),(uint32)swap);		
		sp_to_ext_fwd[ext_intf_idx]++;		

		// mapping share pool to pTx , step 15~22

		// spec step 15
		ext->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);							

		// spec step 16
		ext->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);

		// spec step 17
		ext->skb_header_addr =swap->skb_header_addr;

		// spec step 18
		ext->linkid=0;

		// spec step 19
		if(swap->sram_en==1)
		{
			// copy sram data into sdram
			memcpy((void*)swap->rx_buffer_addr+swap->sram_map_of, sram_buf+128*swap->sram_map_id,128);
		}


		// spec step 20
		ext->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		ext->oeor=swap->eor;		

		// spec step 21
		ext->fcpu=0;

		// spec step 22
		ext->own=1;
		

		// move pTx to next entry
		if(ext->eor==0)
		{
			pp_ext_ptx[ext_intf_idx]++;
		}
		else
		{
			pp_ext_ptx[ext_intf_idx]=first_ext_ptx[ext_intf_idx];
		}

		ext_ptx_ring_used[ext_intf_idx]++;
		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_ext[ext_intf_idx]++;
		}
		else
		{							
			pp_sp_to_ext[ext_intf_idx]=first_sp_to_ext[ext_intf_idx];
		}

		model_ext_tx(ext_intf_idx);
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);

	}	
}

void model_pp_ext_to_sar(int ext_intf_idx,int desc_idx,int dst_port_idx)
{
	MT_RINGCTRL("call");
	if(pp_sp_to_sar[dst_port_idx]->own==1)							
	{
		struct ext_Rx *ext=ext_prx(ext_intf_idx,desc_idx%ext_prx_ring_size[ext_intf_idx]);
		struct sp_pRx *swap=pp_sp_to_sar[dst_port_idx];
		struct sar_pTx *sar=pp_sar_ptx[dst_port_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;

		
		// SWAP pRx and share pool
		rtl8672_getAsicNetInterface(SAR_INTFS+MAC_TX_INTFS+ext_intf_idx, &intf);		
		model_pp_swap_ext_sp(ext,swap,intf.RXshift);
		MT_RINGCTRL_DEBUG("SWAP EXT_PRX[%d][%d]=%x with SAR_SP[%d][%d]=%x\n",ext_intf_idx,desc_idx%ext_prx_ring_size[ext_intf_idx],(uint32)ext,dst_port_idx,swap-sp_to_sar(dst_port_idx,0),(uint32)swap);
		sp_to_sar_fwd[dst_port_idx]++;

		// mapping share pool to pTx , step 15~22

		// spec step 15
		sar->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);

		// spec step 16
		sar->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);

		// spec step 19
		sar->sram_size=0;

		// spec step 20
		sar->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		sar->oeor=swap->eor;		

		// spec step 21
		sar->fcpu=0;

		// spec 21-1 , new add
		sar->fs=1;
		sar->ls=1;
				
		// spec step 21-2 , new add
		rtl8672_getAsicNetInterface(dst_port_idx, &intf);			
		sar->atmport=intf.ATMPORT;
		sar->trlren=intf.TRLREN;
		sar->clp=intf.CLP;
		sar->pti=intf.PTI;
		sar->trlr=intf.TRLR;

		if(intf.LanFCS==0)
			sar->ethnt_offset=0x3f;
		else			
			sar->ethnt_offset=ethnt_offset_map[intf.L2Encap][intf.SARhdr];
		
		// spec step 22
		sar->own=1;

		
		

		// move pTx to next entry
		if(sar->eor==0)
		{
			pp_sar_ptx[dst_port_idx]++;
		}
		else
		{
			pp_sar_ptx[dst_port_idx]=first_sar_ptx[dst_port_idx];
		}
		sar_ptx_ring_used[dst_port_idx]++;

		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_sar[dst_port_idx]++;
		}
		else
		{							
			pp_sp_to_sar[dst_port_idx]=first_sp_to_sar[dst_port_idx];
		}

		model_sar_tx();
	
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);
	}
}

void model_pp_ext_to_mac(int ext_intf_idx,int desc_idx,int dst_port_idx)
{
	MT_RINGCTRL("call");
	if(pp_sp_to_mac[dst_port_idx]->own==1)							
	{
		struct ext_Rx *ext=ext_prx(ext_intf_idx,desc_idx%ext_prx_ring_size[ext_intf_idx]);
		struct sp_pRx *swap=pp_sp_to_mac[dst_port_idx];
		struct mac_pTx *mac=pp_mac_ptx[dst_port_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;

		
		// SWAP pRx and share pool
		rtl8672_getAsicNetInterface(SAR_INTFS+MAC_TX_INTFS+ext_intf_idx, &intf);
		model_pp_swap_ext_sp(ext,swap,intf.RXshift);
		
		MT_RINGCTRL_DEBUG("SWAP EXT_PRX[%d][%d]=%x with MAC_SP[%d][%d]=%x\n",ext_intf_idx,desc_idx%ext_prx_ring_size[ext_intf_idx],(uint32)ext,dst_port_idx,swap-sp_to_mac(dst_port_idx,0),(uint32)swap);
		sp_to_mac_fwd[dst_port_idx]++;

		// mapping share pool to pTx , step 15~22

		// spec step 15
		mac->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);

		// spec step 16
		mac->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);

		// spec step 18
		mac->sram_map_id=swap->sram_map_id;

		// spec step 19
		mac->sram_size=0;

		// spec step 20
		mac->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		mac->oeor=swap->eor;		

		// spec step 21
		mac->fcpu=0;

		// spec 21-1 , new add
		mac->fs=1;
		mac->ls=1;
		mac->crc=1;
		mac->tagc=0;
		mac->vlan_vidl=0;
		mac->vlan_prio=0;		
		mac->vlan_cfi=0;		
		mac->vlan_vidh=0;				

		// spec step 22
		mac->own=1;
		

		// move pTx to next entry
		if(mac->eor==0)
		{
			pp_mac_ptx[dst_port_idx]++;
		}
		else
		{
			pp_mac_ptx[dst_port_idx]=first_mac_ptx[dst_port_idx];
		}
		mac_ptx_ring_used[dst_port_idx]++;

		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_mac[dst_port_idx]++;
		}
		else
		{							
			pp_sp_to_mac[dst_port_idx]=first_sp_to_mac[dst_port_idx];
		}

		//model_mac_tx();	
		if(dst_port_idx==0)
		{
			model_interrupt(dst_port_idx+SAR_INTFS,MAC_PTOK0);
		}
		else
		{
			model_interrupt(dst_port_idx+SAR_INTFS,MAC_PTOK0<<(dst_port_idx-1));
		}
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);
	}

}

void model_pp_ext_to_ext(int ext_intf_idx,int desc_idx,int dst_port_idx)
{
	MT_RINGCTRL("call");
	if(pp_sp_to_mac[dst_port_idx]->own==1)							
	{
		struct ext_Rx *ext=ext_prx(ext_intf_idx,desc_idx%ext_prx_ring_size[ext_intf_idx]);
		struct sp_pRx *swap=pp_sp_to_ext[dst_port_idx];
		struct ext_Tx *ext2=pp_ext_ptx[dst_port_idx];
		rtl8672_tblAsicDrv_intfParam_t intf;

		
		// SWAP pRx and share pool
		rtl8672_getAsicNetInterface(SAR_INTFS+MAC_TX_INTFS+ext_intf_idx, &intf);		
		model_pp_swap_ext_sp(ext,swap,intf.RXshift);
		MT_RINGCTRL_DEBUG("SWAP EXT_PRX[%d][%d]=%x with EXT_SP[%d][%d]=%x\n",ext_intf_idx,desc_idx%ext_prx_ring_size[ext_intf_idx],(uint32)ext,dst_port_idx,swap-sp_to_ext(dst_port_idx,0),(uint32)swap);
		sp_to_ext_fwd[dst_port_idx]++;		

		// mapping share pool to pTx , step 15~22

		// spec step 15
		ext2->tx_buffer_addr=swap->rx_buffer_addr+((READ_MEM32(XLATOR_DBG_BASE)&XLATOR_RXSHIFT_MASK)>>XLATOR_RXSHIFT_OFFSET);

		// spec step 16
		ext2->data_length=(READ_MEM32(XLATOR_DBG_BASE)&XLATOR_DATALEN_MASK);

		// spec step 17
		ext2->skb_header_addr=swap->skb_header_addr;

		// spec step 18
		ext2->linkid=cache_linkid;

		// spec step 20
		ext2->orgAddr=((uint32)swap)>>2;

		// spec step 20-1
		ext2->oeor=swap->eor;

		// spec step 21
		ext2->fcpu=0;

		// spec step 22
		ext2->own=1;
		

		// move pTx to next entry
		if(ext2->eor==0)
		{
			pp_ext_ptx[dst_port_idx]++;
		}
		else
		{
			pp_ext_ptx[dst_port_idx]=first_ext_ptx[dst_port_idx];
		}
		ext_ptx_ring_used[dst_port_idx]++;

		

		// move share pool to next entry
		if(swap->eor==0)
		{
			pp_sp_to_ext[dst_port_idx]++;
		}
		else
		{							
			pp_sp_to_ext[dst_port_idx]=first_sp_to_ext[dst_port_idx];
		}

		model_ext_tx(dst_port_idx);	
		
	}
	else // assert nerver go here
	{
		MT_RINGCTRL_DEBUG("Exception: %s %d\n",__FUNCTION__,__LINE__);
	}
}


void model_pp_drop_mac_prx(int mac_intf_idx,int desc_idx)
{
	struct mac_pRx *prx=mac_prx(mac_intf_idx,desc_idx%mac_prx_ring_size[mac_intf_idx]);
	rtl8672_tblAsicDrv_intfParam_t intf;

	MT_RINGCTRL("call");	
	MT_RINGCTRL_DEBUG("Drop MAC pRx packet: %s %d\n",__FUNCTION__,__LINE__);	

	rtl8672_getAsicNetInterface(SAR_INTFS+mac_intf_idx, &intf);	
	prx->rx_shift=intf.RXshift;
	prx->data_length=READ_MEM32(BUFFERSIZE);
	prx->own=1;
	prx->fs=0;
	prx->ls=0;	

	mac_prx_drop[mac_intf_idx]++;
}

void model_pp_drop_sar_prx(int sar_intf_idx,int desc_idx)
{
	struct sar_pRx *prx=sar_prx(sar_intf_idx,desc_idx%sar_prx_ring_size[sar_intf_idx]);
	rtl8672_tblAsicDrv_intfParam_t intf;
	
	MT_RINGCTRL("call");	
	MT_RINGCTRL_DEBUG("Drop SAR pRx packet: %s %d\n",__FUNCTION__,__LINE__);	

	rtl8672_getAsicNetInterface(sar_intf_idx, &intf);	
	prx->rx_shift=intf.RXshift;
	prx->data_length=READ_MEM32(BUFFERSIZE);
	prx->own=1;
	prx->fs=0;
	prx->ls=0;

	sar_prx_drop[sar_intf_idx]++;
}


void model_pp_drop_ext_prx(int ext_intf_idx,int desc_idx)
{
	struct ext_Rx *prx=ext_prx(ext_intf_idx,desc_idx%ext_prx_ring_size[ext_intf_idx]);
	rtl8672_tblAsicDrv_intfParam_t intf;
	
	MT_RINGCTRL("call");	
	MT_RINGCTRL_DEBUG("Drop EXT pRx packet: %s %d\n",__FUNCTION__,__LINE__);	

	rtl8672_getAsicNetInterface(SAR_INTFS+MAC_TX_INTFS+ext_intf_idx, &intf);	
	prx->rx_shift=intf.RXshift;
	prx->data_length=READ_MEM32(BUFFERSIZE);
	prx->own=1;

	ext_prx_drop[ext_intf_idx]++;
}


void model_pp_to_vrx_sar(int intf,int prx_idx,hsa_param_t *hsa)
{
	struct sar_pRx *prx;
	struct sar_vRx *vrx;
	struct sar_pRx prx_tmp;	
	struct sar_vRx vrx_tmp;
	MT_RINGCTRL("call");	
	prx=sar_prx(intf,prx_idx);
	vrx=pp_sar_vrx[intf];

#if 0 //move to translator
	//dma SRAM to SDRAM
	if(prx->sram_en==1)
	{
		// copy sram data into sdram
		memcpy((void*)prx->rx_buffer_addr+prx->sram_map_of, sram_buf+128*prx->sram_map_id,128);
	}	
#endif	

	
	// cache 2 buffer
	memcpy(&vrx_tmp,prx,sizeof(struct sar_vRx));
	memcpy(&prx_tmp,vrx,sizeof(struct sar_vRx));
	// spec step 1
	vrx_tmp.eor=vrx->eor;
	
	// spec step 2
	vrx_tmp.reason=hsa->reason;

	// spec step 3	
	memcpy(vrx,&vrx_tmp,sizeof(struct sar_vRx));

	// move SAR vRx to next
	if(vrx->eor==0)
	{
		pp_sar_vrx[intf]++;
	}
	else
	{
		pp_sar_vrx[intf]=first_sar_vrx[intf];
	}
	

	// spec step 4
	prx_tmp.eor=prx->eor;
	prx_tmp.sram_en=prx->sram_en;
	prx_tmp.sram_map_id=prx->sram_map_id;
	prx_tmp.sram_map_of=prx->sram_map_of;

	// spec step 5

	memcpy(prx,&prx_tmp,sizeof(struct sar_pRx));

	//spec step 6 (SW)
#if 0	
	prx->data_length=READ_MEM32(BUFFERSIZE);
	prx->rx_shift=intf_tbl[intf].rx_shift;
	prx->eor=1;
#endif

	sar_vrx_recv[intf]++;
	model_translator(prx,hsa);
	MT_RINGCTRL_DEBUG("RX to CPU SAR_PRX[%d][%d]=%x , SAR_VRX[%d][%d]=%x\n",intf,prx_idx,(uint32)prx,intf,vrx-first_sar_vrx[intf],(uint32)vrx);

	model_interrupt(intf,SAR_RDA);


}

void model_pp_to_vrx_mac(int intf,int prx_idx,hsa_param_t *hsa)
{
	struct mac_pRx *prx;
	struct mac_vRx *vrx;
	struct mac_pRx prx_tmp;	
	struct mac_vRx vrx_tmp;
	MT_RINGCTRL("call");	
	prx=mac_prx(intf,prx_idx);
	vrx=pp_mac_vrx[intf];

#if 0 //move to translator
	//dma SRAM to SDRAM
	if(prx->sram_en==1)
	{
		// copy sram data into sdram
		memcpy((void*)prx->rx_buffer_addr+prx->sram_map_of, sram_buf+128*prx->sram_map_id,128);
	}	
#endif	

	
	// cache 2 buffer
	memcpy(&vrx_tmp,prx,sizeof(struct mac_vRx));
	memcpy(&prx_tmp,vrx,sizeof(struct mac_vRx));
	

	// spec step 1
	vrx_tmp.eor=vrx->eor;
	
	// spec step 2
	vrx_tmp.reason=hsa->reason;

	// spec step 3	
	memcpy(vrx,&vrx_tmp,sizeof(struct mac_vRx));


	// move mac vRx to next
	if(vrx->eor==0)
	{
		pp_mac_vrx[intf]++;
	}
	else
	{
		pp_mac_vrx[intf]=first_mac_vrx[intf];
	}
	
		
	// FIXME: trigger an interrupt here

	// spec step 4
	prx_tmp.eor=prx->eor;
	prx_tmp.sram_en=prx->sram_en;
	prx_tmp.sram_map_id=prx->sram_map_id;
	prx_tmp.sram_map_of=prx->sram_map_of;

	// spec step 5
	memcpy(prx,&prx_tmp,sizeof(struct mac_pRx));

	//spec step 6 (SW)
#if 0	
	prx->data_length=READ_MEM32(BUFFERSIZE);
	prx->rx_shift=intf_tbl[intf].rx_shift;
	prx->eor=1;
#endif

	mac_vrx_recv[intf]++;

	model_translator(prx,hsa);

	MT_RINGCTRL_DEBUG("RX to CPU MAC_PRX[%d][%d]=%x , MAC_VRX[%d][%d]=%x\n",intf,prx_idx,(uint32)prx,intf,vrx-first_mac_vrx[intf],(uint32)vrx);
	model_interrupt(intf+SAR_INTFS,MAC_ROK);	

}

void model_pp_to_vrx_ext(int intf,int prx_idx,hsa_param_t *hsa)
{
	struct ext_Rx *prx;
	struct ext_Rx *vrx;
	struct ext_Rx prx_tmp;	
	struct ext_Rx vrx_tmp;
	MT_RINGCTRL("call");	
	prx=ext_prx(intf,prx_idx);
	vrx=pp_ext_vrx[intf];


	MT_RINGCTRL_DEBUG("RX to CPU EXT_PRX[%d][%d]=%x(skb=%x) , EXT_VRX[%d][%d]=%x(skb=%x)\n",intf,prx_idx,(uint32)prx,(uint32)prx->skb_header_addr,intf,vrx-first_ext_vrx[intf],(uint32)vrx,(uint32)vrx->skb_header_addr);	
	
	// cache 2 buffer
	memcpy(&vrx_tmp,prx,sizeof(struct ext_Rx));
	memcpy(&prx_tmp,vrx,sizeof(struct ext_Rx));

	// spec step 1
	vrx_tmp.eor=vrx->eor;
	
	// spec step 2
	vrx_tmp.reason=hsa->reason;

	// spec step 3	
	memcpy(vrx,&vrx_tmp,sizeof(struct ext_Rx));


	// move ext vRx to next
	if(vrx->eor==0)
	{
		pp_ext_vrx[intf]++;
	}
	else
	{
		pp_ext_vrx[intf]=first_ext_vrx[intf];
	}

	// spec step 4
	prx_tmp.eor=prx->eor;

	// spec step 5
	memcpy(prx,&prx_tmp,sizeof(struct ext_Rx));


	//spec step 6 (SW)
#if 0	
	prx->data_length=READ_MEM32(BUFFERSIZE);
	prx->rx_shift=intf_tbl[intf].rx_shift;
	prx->eor=1;
#endif

	ext_vrx_recv[intf]++;

	model_translator(prx,hsa);

	model_interrupt(intf+SAR_INTFS+MAC_TX_INTFS,EXT0_VRX_OK<<intf);
}


void model_pp_vtx_sar(int intf)
{
	MT_RINGCTRL("call");
	if(pp_sar_ptx[intf]->own==0) // have free ptx desc
	{
		struct sar_pTx cache;
		memcpy(&cache,pp_sar_vtx[intf],sizeof(struct sar_vTx));

		//spec step 1
		cache.oeor=pp_sar_vtx[intf]->eor;

		//spec step 2
		cache.eor=pp_sar_ptx[intf]->eor;

		//spec step 3
		cache.sram_map_id=0;
		cache.sram_size=0;

		//spec step 4
		memcpy(pp_sar_ptx[intf],&cache,sizeof(struct sar_pTx));
		
		// move pp vtx to next entry
		if(pp_sar_vtx[intf]->eor==0)
		{
			pp_sar_vtx[intf]++;
		}
		else
		{
			pp_sar_vtx[intf]=first_sar_vtx[intf];
		}
//		sar_vtx_ring_used[intf]--;

		// move pTx to next entry
		if(pp_sar_ptx[intf]->eor==0)
		{
			pp_sar_ptx[intf]++;
		}
		else
		{
			pp_sar_ptx[intf]=first_sar_ptx[intf];
		}
		
		sar_ptx_ring_used[intf]++;
		sar_vtx_fwd[intf]++;
		model_interrupt(intf,SAR_VTDF);

		//FIXME: trigger pTx working here
		model_sar_tx();
	}
	else
	{
		sar_ptx_ring_full[intf]=1;
	}
}


void model_pp_vtx_mac(int intf)
{

	MT_RINGCTRL("call");
	if(pp_mac_ptx[intf]->own==0) // have free ptx desc
	{
		struct mac_pTx cache;
		memcpy(&cache,pp_mac_vtx[intf],sizeof(struct mac_vTx));

		//spec step 1
		cache.oeor=pp_mac_vtx[intf]->eor;

		//spec step 2
		cache.eor=pp_mac_ptx[intf]->eor;

		//spec step 3
		cache.sram_map_id=0;
		cache.sram_size=0;

		//spec step 4
		memcpy(pp_mac_ptx[intf],&cache,sizeof(struct mac_pTx));
		
		// move pp vtx to next entry
		if(pp_mac_vtx[intf]->eor==0)
		{
			pp_mac_vtx[intf]++;

		}
		else
		{
			pp_mac_vtx[intf]=first_mac_vtx[intf];
		}
//		mac_vtx_ring_used[intf]--;

		// move pTx to next entry
		if(pp_mac_ptx[intf]->eor==0)
		{
			pp_mac_ptx[intf]++;
		}
		else
		{
			pp_mac_ptx[intf]=first_mac_ptx[intf];
		}
		mac_ptx_ring_used[intf]++;
		mac_vtx_fwd[intf]++;

		model_interrupt(intf+SAR_INTFS,MAC_VTOK);

		//FIXME: trigger pTx working here
		if(intf==0)
		{
			model_interrupt(intf+SAR_INTFS,MAC_PTOK0);
		}
		else
		{
			model_interrupt(intf+SAR_INTFS,MAC_PTOK0<<(intf-1));
		}
//		model_mac_tx();


	}
	else
	{
		mac_ptx_ring_full[intf]=1;
	}	
}


void model_pp_vtx_ext(int intf)
{
	MT_RINGCTRL("call");
	if(pp_ext_ptx[intf]->own==0) // have free ptx desc
	{
		struct ext_Tx cache;
		memcpy(&cache,pp_ext_vtx[intf],sizeof(struct ext_Tx));

		//spec step 1
		cache.oeor=pp_ext_vtx[intf]->eor;

		//spec step 2
		cache.eor=pp_ext_ptx[intf]->eor;

		//spec step 3
		memcpy(pp_ext_ptx[intf],&cache,sizeof(struct ext_Tx));
		
		// move pp vtx to next entry
		if(pp_ext_vtx[intf]->eor==0)
		{
			pp_ext_vtx[intf]++;
		}
		else
		{
			pp_ext_vtx[intf]=first_ext_vtx[intf];
		}
//		ext_vtx_ring_used[intf]--;

		// move pTx to next entry
		if(pp_ext_ptx[intf]->eor==0)
		{
			pp_ext_ptx[intf]++;
		}
		else
		{
			pp_ext_ptx[intf]=first_ext_ptx[intf];
		}
		ext_ptx_ring_used[intf]++;
		ext_vtx_fwd[intf]++;

		model_ext_tx(intf);
	}
}


void model_pp_polling(void)
{

	//handle pRx
	int startidx,endidx;
	int intf,i;	
	MT_RINGCTRL("call");
	
	//SAR pRx
	for(intf=0;intf<SAR_INTFS;intf++)
	{

		startidx=pp_sar_prx[intf]-first_sar_prx[intf];
		endidx=current_sar_prx[intf]-first_sar_prx[intf];

		if(startidx>endidx) endidx+=sar_prx_ring_size[intf];

		if((startidx==endidx)&&(sar_prx_ring_used[intf]!=0)) endidx+=sar_prx_ring_size[intf];

		for(i=startidx;i<endidx;i++)
		{
			struct sar_pRx *prx=sar_prx(intf,i%sar_prx_ring_size[intf]);


			/* run the follow case by ring weight */
			if(sar_prx_ring_cnt[intf] >= sar_prx_ring_weight[intf])
			{
				sar_prx_ring_cnt[intf]=0;
				break;
			}
			else
			{
				sar_prx_ring_cnt[intf]++;				
			}

			if(prx->own==0) /* cpu own and not put to tx yet , try to fwd to ptx */
			{
				int dst_port_idx;
				hsb_param_t rhsb;
				hsa_param_t rhsa;
				model_parser(intf,(void*)prx,&rhsb);
				model_ale(&rhsb,&rhsa);
				dst_port_idx=rhsa.dstportidx;
				if(rhsa.tocpu==0) // to pTx
				{				

					if(dst_port_idx<8)	// SAR to SAR
					{
						if(pp_sar_ptx[dst_port_idx]->own==0)
						{

							model_translator((void*)prx,&rhsa);
							model_pp_sar_to_sar(intf,i,dst_port_idx);
						}
						else // pTx full, drop SAR pRx
						{
							sar_ptx_ring_full[dst_port_idx]=1;
							MT_RINGCTRL_DEBUG("SAR PTX FULL\n");						
							model_pp_drop_sar_prx(intf,i);
						}
					}
					else if(dst_port_idx<(MAC_TX_INTFS+SAR_INTFS))	//SAR to MAC
					{
						if(pp_sar_ptx[dst_port_idx-SAR_INTFS]->own==0)
						{

							model_translator((void*)prx,&rhsa);						
							model_pp_sar_to_mac(intf,i,dst_port_idx-SAR_INTFS);
						}
						else // pTx full, drop SAR pRx
						{
							mac_ptx_ring_full[dst_port_idx-SAR_INTFS]=1;	
							MT_RINGCTRL_DEBUG("MAC PTX FULL\n");						
							model_pp_drop_sar_prx(intf,i);
						}
					}
					else // SAR to Extension port
					{
						if(((READ_VIR32(EPISR)&(EXT0_PTX_FULL<<(dst_port_idx-MAC_TX_INTFS-SAR_INTFS)))==0)&&
							(pp_ext_ptx[dst_port_idx-MAC_TX_INTFS-SAR_INTFS]->own==0))
						{
							model_translator((void*)prx,&rhsa);						
							model_pp_sar_to_ext(intf,i,dst_port_idx-MAC_TX_INTFS-SAR_INTFS);
						}
						else // pTx full, drop SAR pRx
						{
							MT_RINGCTRL_DEBUG("EXT PTX FULL\n");
							model_pp_drop_sar_prx(intf,i);
							model_interrupt(dst_port_idx,EXT0_PTX_FULL<<(dst_port_idx-MAC_TX_INTFS-SAR_INTFS));  
						}
					}
				}
				else // SAR vRx
				{
					if(pp_sar_vrx[intf]->own==1)
					{
//						rtlglue_printf("vrx->fs=%d\n",pp_sar_vrx[intf]->fs);
						model_pp_to_vrx_sar(intf,i,&rhsa);
					}
					else // vRx full, drop SAR pRx
					{
						MT_RINGCTRL_DEBUG("SAR vRx Full, drop SAR pRx pkt\n");
						model_pp_drop_sar_prx(intf,i);
						model_interrupt(intf,SAR_RBF);
					}
				}

				// move pp prx to next entry
				if(pp_sar_prx[intf]->eor==0)
				{
					pp_sar_prx[intf]++;
				}
				else
				{
					pp_sar_prx[intf]=first_sar_prx[intf];
				}				
				sar_prx_ring_used[intf]--;

			}

		}
	}


	//MAC pRx
	for(intf=0;intf<MAC_RX_INTFS;intf++)
	{
		startidx=pp_mac_prx[intf]-first_mac_prx[intf];
		endidx=current_mac_prx[intf]-first_mac_prx[intf];

		if(startidx>endidx) endidx+=mac_prx_ring_size[intf];

		if((startidx==endidx)&&(mac_prx_ring_used[intf]!=0)) endidx+=mac_prx_ring_size[intf];

		for(i=startidx;i<endidx;i++)
		{
			struct mac_pRx *prx;

			/* run the follow case by ring weight */
			if(mac_prx_ring_cnt[intf] >= mac_prx_ring_weight[intf])
			{
				mac_prx_ring_cnt[intf]=0;
				break;
			}
			else
			{
				mac_prx_ring_cnt[intf]++;				
			}

		
			prx=mac_prx(intf,i%mac_prx_ring_size[intf]);
			if(prx->own==0) /* cpu own and not put to tx yet , try to fwd to ptx */
			{
				int dst_port_idx;
				hsb_param_t rhsb;
				hsa_param_t rhsa;
				memset(&rhsb,0,sizeof(hsb_param_t));
				MT_RINGCTRL_DEBUG("prx->fs=%d prx->ls=%d\n",prx->fs,prx->ls);
				model_parser(intf+SAR_INTFS,(void*)prx,&rhsb);
//				dumpHSB(&rhsb);
				model_ale(&rhsb,&rhsa);
				dst_port_idx=rhsa.dstportidx;

				if(rhsa.droppacket==0) 
				{
					if(rhsa.tocpu==0) // to pTx
					{

						if(dst_port_idx<8)	// MAC to SAR
						{
							if(pp_sar_ptx[dst_port_idx]->own==0)
							{
								model_translator((void*)prx,&rhsa);			

								model_pp_mac_to_sar(intf,i,dst_port_idx);
							}
							else // pTx full, drop MAC pRx
							{
								sar_ptx_ring_full[dst_port_idx]=1;
								MT_RINGCTRL_DEBUG("SAR PTX FULL\n");						
								model_pp_drop_mac_prx(intf,i);
							}
						}
						else if(dst_port_idx<(MAC_TX_INTFS+SAR_INTFS))	//MAC to MAC
						{
							if(pp_mac_ptx[dst_port_idx-SAR_INTFS]->own==0)
							{
								model_translator((void*)prx,&rhsa);						
								model_pp_mac_to_mac(intf,i,dst_port_idx-SAR_INTFS);
							
							}
							else // pTx full, drop MAC pRx
							{
								mac_ptx_ring_full[dst_port_idx-SAR_INTFS]=1;							
								MT_RINGCTRL_DEBUG("MAC PTX FULL\n");
								model_pp_drop_mac_prx(intf,i);
							
							}
						}
						else // MAC to Extension port
						{
							if(((READ_VIR32(EPISR)&(EXT0_PTX_FULL<<(dst_port_idx-MAC_TX_INTFS-SAR_INTFS)))==0)&&
							(pp_ext_ptx[dst_port_idx-MAC_TX_INTFS-SAR_INTFS]->own==0))	
							{
								model_translator((void*)prx,&rhsa);						
								model_pp_mac_to_ext(intf,i,dst_port_idx-MAC_TX_INTFS-SAR_INTFS);
							}
							else // pTx full, drop MAC pRx
							{
								MT_RINGCTRL_DEBUG("EXT PTX FULL\n");
								model_pp_drop_mac_prx(intf,i);								
								model_interrupt(dst_port_idx,EXT0_PTX_FULL<<(dst_port_idx-MAC_TX_INTFS-SAR_INTFS));  
							}
						}
					}
					else
					{
						if(pp_mac_vrx[intf]->own==1)
						{
							model_pp_to_vrx_mac(intf,i,&rhsa);
						
						}
						else // vRx full, drop MAC pRx
						{
							MT_RINGCTRL_DEBUG("MAC vRx Full, drop MAC pRx pkt\n");						
							model_pp_drop_mac_prx(intf,i);
							model_interrupt(intf+SAR_INTFS,MAC_RER_OVF);
						
						}
					}
				}
				else
				{
					model_pp_drop_mac_prx(intf,i);
				
				}

				// move pp prx to next entry
				if(pp_mac_prx[intf]->eor==0)
				{

					pp_mac_prx[intf]++;
				}
				else
				{

					pp_mac_prx[intf]=first_mac_prx[intf];
				}				
				mac_prx_ring_used[intf]--;
			}

		}
	}

	
	
	//EXT pRx
	for(intf=0;intf<EXT_INTFS;intf++)
	{

		startidx=pp_ext_prx[intf]-first_ext_prx[intf];
		endidx=current_ext_prx[intf]-first_ext_prx[intf];

		if(startidx>endidx) endidx+=ext_prx_ring_size[intf];

		if((startidx==endidx)&&(ext_prx_ring_used[intf]!=0)) endidx+=ext_prx_ring_size[intf];

		for(i=startidx;i<endidx;i++)
		{
			struct ext_Rx *prx=ext_prx(intf,i%ext_prx_ring_size[intf]);


			/* run the follow case by ring weight */
			if(ext_prx_ring_cnt[intf] >= ext_prx_ring_weight[intf])
			{
				ext_prx_ring_cnt[intf]=0;
				break;
			}
			else
			{
				ext_prx_ring_cnt[intf]++;				
			}
			

			if(prx->own==0) /* cpu own and not put to tx yet , try to fwd to ptx */
			{

				int dst_port_idx;
				hsb_param_t rhsb;
				hsa_param_t rhsa;
				model_parser(intf+SAR_INTFS+MAC_TX_INTFS,(void*)prx,&rhsb);
				model_ale(&rhsb,&rhsa);
				dst_port_idx=rhsa.dstportidx;

				if(rhsa.tocpu==0) // to pTx
				{				

					if(dst_port_idx<8)	// ext to SAR
					{
						if(pp_sar_ptx[dst_port_idx]->own==0)
						{
							model_translator((void*)prx,&rhsa);
							model_pp_ext_to_sar(intf,i,dst_port_idx);
						}
						else // pTx full, drop ext pRx
						{
							sar_ptx_ring_full[dst_port_idx]=1;
							MT_RINGCTRL_DEBUG("SAR PTX FULL\n");						
							model_pp_drop_ext_prx(intf,i);
						}
					}
					else if(dst_port_idx<(MAC_TX_INTFS+SAR_INTFS))	//ext to MAC
					{
						if(pp_mac_ptx[dst_port_idx-SAR_INTFS]->own==0)
						{
							model_translator((void*)prx,&rhsa);						
							model_pp_ext_to_mac(intf,i,dst_port_idx-SAR_INTFS);
						}
						else // pTx full, drop ext pRx
						{
							mac_ptx_ring_full[dst_port_idx-SAR_INTFS]=1;						
							MT_RINGCTRL_DEBUG("MAC PTX FULL\n");						
							model_pp_drop_ext_prx(intf,i);
						}
					}
					else // ext to Extension port
					{
						if(((READ_VIR32(EPISR)&(EXT0_PTX_FULL<<(dst_port_idx-MAC_TX_INTFS-SAR_INTFS)))==0)&&
						(pp_ext_ptx[dst_port_idx-MAC_TX_INTFS-SAR_INTFS]->own==0))
						{
							model_translator((void*)prx,&rhsa);						
							model_pp_ext_to_ext(intf,i,dst_port_idx-MAC_TX_INTFS-SAR_INTFS);
						}
						else // pTx full, drop ext pRx
						{
							MT_RINGCTRL_DEBUG("EXT PTX FULL\n");						
							model_pp_drop_ext_prx(intf,i);							
							model_interrupt(dst_port_idx,EXT0_PTX_FULL<<(dst_port_idx-MAC_TX_INTFS-SAR_INTFS));  
						}
					}
				}
				else // EXT vRx
				{
					if(((READ_VIR32(EPISR)&(EXT0_VRX_FULL<<intf))==0)&&(pp_ext_vrx[intf]->own==1))
					{
						model_pp_to_vrx_ext(intf,i,&rhsa);
					}
					else // vRx full, drop EXT pRx
					{
						MT_RINGCTRL_DEBUG("EXT vRx Full or EPISR full bit is not clear, drop EXT pRx pkt\n");						
						model_pp_drop_ext_prx(intf,i);
						model_interrupt(intf+SAR_INTFS+MAC_TX_INTFS,EXT0_VRX_FULL<<intf);
					}				
				}

				// move pp prx to next entry
				if(pp_ext_prx[intf]->eor==0)
				{
					pp_ext_prx[intf]++;
				}
				else
				{
					pp_ext_prx[intf]=first_ext_prx[intf];
				}				
				ext_prx_ring_used[intf]--;

			}

		}


		startidx=pp_ext_prx[intf]-first_ext_prx[intf];
		endidx=current_ext_prx[intf]-first_ext_prx[intf];
		if((startidx==endidx)&&(ext_prx_ring_used[intf]==0)) 
		{
			WRITE_VIR32(PENDINGRING,READ_VIR32(PENDINGRING)&(~(1<<(16+intf)))); /* clear the pending flag */
		}
		
	}


	//SAR vTx
	for(intf=0;intf<SAR_INTFS;intf++)
	{
		int i=pp_sar_vtx[intf]-first_sar_vtx[intf];
		int pending=READ_VIR32(PENDINGRING);

		if(pending&(1<<intf)) /* when SW is writed this bit to one, HW must to do some EXT vTx job here */
		{		
			while(1)
			{
				struct sar_vTx *vtx=sar_vtx(intf,i);

				/* run the follow case by ring weight */
				if(sar_vtx_ring_cnt[intf] >= sar_vtx_ring_weight[intf])
				{
					sar_vtx_ring_cnt[intf]=0;
					break;
				}
				else
				{
					sar_vtx_ring_cnt[intf]++;				
				}
				
				if(vtx->own==0) 
				{
					WRITE_VIR32(PENDINGRING,READ_VIR32(PENDINGRING)&(~(1<<intf))); /* clear the pending flag */
					break;
				}
				model_pp_vtx_sar(intf);					
				i=(i+1)%sar_vtx_ring_size[intf];
			}
		}
	}

	//MAC vTx
	for(intf=0;intf<MAC_TX_INTFS;intf++)
	{
		int i=pp_mac_vtx[intf]-first_mac_vtx[intf];
		int pending=READ_VIR32(PENDINGRING);
		
		if(pending&(1<<(SAR_INTFS+intf))) /* when SW is writed this bit to one, HW must to do some EXT vTx job here */
		{		
			while(1)
			{		
				struct mac_vTx *vtx=mac_vtx(intf,i);

				/* run the follow case by ring weight */
				if(mac_vtx_ring_cnt[intf] >= mac_vtx_ring_weight[intf])
				{
					mac_vtx_ring_cnt[intf]=0;
					break;
				}
				else
				{
					mac_vtx_ring_cnt[intf]++;				
				}
				
				if(vtx->own==0)
				{
					WRITE_VIR32(PENDINGRING,READ_VIR32(PENDINGRING)&(~(1<<(SAR_INTFS+intf)))); /* clear the pending flag */		
					model_interrupt(intf+SAR_INTFS,MAC_TDU);
					break;			
				}
				model_pp_vtx_mac(intf);
				i=(i+1)%mac_vtx_ring_size[intf];
			}
		}

	}

	//EXT vTx (form PP's index to own bit = 0)	
	for(intf=0;intf<EXT_INTFS;intf++)
	{	
		int i=pp_ext_vtx[intf]-first_ext_vtx[intf];
		int pending=READ_VIR32(PENDINGRING);

		if(pending&(1<<(SAR_INTFS+MAC_TX_INTFS+intf))) /* when SW is writed this bit to one, HW must to do some EXT vTx job here */
		{
			while(1)
			{
				struct ext_Tx *vtx=ext_vtx(intf,i);
				
				/* run the follow case by ring weight */
				if(ext_vtx_ring_cnt[intf] >= ext_vtx_ring_weight[intf])
				{
					ext_vtx_ring_cnt[intf]=0;
					break;
				}
				else
				{
					ext_vtx_ring_cnt[intf]++;				
				}

				if(vtx->own==0) 
				{
					WRITE_VIR32(PENDINGRING,READ_VIR32(PENDINGRING)&(~(1<<(SAR_INTFS+MAC_TX_INTFS+intf)))); /* clear the pending flag */
					model_interrupt(intf+SAR_INTFS+MAC_TX_INTFS,EXT0_VTX_EMPTY<<intf);
					break;
				}
				model_pp_vtx_ext(intf);
				i=(i+1)%ext_vtx_ring_size[intf];
			}
		}
	}


}


void model_pp(void)
{
	int i,j;
	enum IC_TYPE type;
	MT_RINGCTRL("call");

	model_getTestTarget(&type);

	WRITE_MEM32(PPCR,READ_MEM32(PPCR)|(PPCR_START));


//	WRITE_MEM32(LLIP_SIM_RX,0xffffffff);
	j=0;


	if(type==IC_TYPE_MODEL)
	{
		if((READ_MEM32(PPCR)&(PPCR_START|PPCR_ENABLE))==(PPCR_START|PPCR_ENABLE)) /* when pp is enabled and started */
		{
			for(i=0;i<=MAC_PRX_RING_SIZE+SAR_PRX_RING_SIZE+EXT_PRX_RING_SIZE;i++)
			{		
				model_pp_polling();
			}
		}
	}
	else
	{
		//FIXME: because Packet Processor is work by polling mode. If descriptor own bit is change to Packet-Processor owned,
		//           packet-processor will work immediately. so, we must start and stop the packet-processor here. to make sure
		//		the test case is able to run step-by-step.
		
	
		// waiting for HW to finish the job
		WRITE_MEM32(DEBUG_CTL,0x23);
		while(1)
		{
			/* 
			[14:11]	rxring_ctl.cstate_cs	FSM, 
			0=IDLE, 1=Sel rx port state, 2=calculate address state, 
			3=reading state, 4=not belong to me, clear valid , 
			5=wait state(one cycle), 6=send to parser, 
			7=wait the txring response, 8=write descriptor state, 
			9=update point state
			*/
			rtlglue_printf("DBG_SIGNAL=%x\n",(uint32)DBG_SIGNAL);
			if((READ_MEM32(DBG_SIGNAL)&0x7800)==0) break; 
		}
#ifdef RTL867X_MODEL_KERNEL	
#if 1
		if(READ_MEM32(PP_TOTAL_CNT) == 0){
			rtlglue_printf("HW fwd Nothing!\n");
		}else{
			rtlglue_printf("HW fwd OK-last cycles:%d, total:%d, total pkts:%d, avg:%d\n"
						,READ_MEM32(PP_SINGLE_CYCLE)
						,READ_MEM32(PP_TOTAL_CYCLE)
						,READ_MEM32(PP_TOTAL_CNT)
						,READ_MEM32(PP_TOTAL_CYCLE)/READ_MEM32(PP_TOTAL_CNT));
		}
#endif
#endif		
	}
	
}
#endif

