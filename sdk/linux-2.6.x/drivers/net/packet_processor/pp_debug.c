/*
* Copyright (c) Realtek Semiconductor Corporation, 2010
* All rights reserved.
* 
* Program : RTL8672 Packet Processor Driver
* Abstract : 
* Author : Ethan Yi Sheng Leu (ysleu@realtek.com)
* $Id: pp_debug.c,v 1.1 2012/07/12 08:37:47 czyao Exp $
*/

//#include "rtl_pplinux.h"
#include "rtl_types.h"
#include "rtl_utils.h"
//#include "rtl8672_pp.h"
#include "rtl8672_tblAsicDrv.h"
#include "rtl8672_tblDrv.h"
#include "rtl8672_asicregs.h"
//#include "rtl8672pp_extsw.h"
#include "../../867x_sar/ra867x_pp.h"
#include "../re830x.h"
//#include "rtl8672pp_sarsw.h"
#include "icModel_ringController.h"
#include <net/net_namespace.h>

#define SAR_INTFS 8
#define MAC_TX_INTFS 5
#define MAC_RX_INTFS 2
#define EXT_INTFS 3
extern struct mac_pRx *user_ptm_prx_idx(int intf,int ring);
extern struct mac_vRx *user_ptm_vrx_idx(int intf,int ring);
extern struct mac_pRx *user_mac_prx_idx(int intf,int ring);
extern struct mac_vRx *user_mac_vrx_idx(int intf,int ring);
extern struct mac_vTx *user_ptm_vtx_idx(int intf,int ring);
extern struct mac_pTx *user_ptm_ptx_idx(int intf,int ring);
extern struct sp_pRx  *user_sp_to_ptm_idx(int intf,int ring);

extern struct mac_vTx *user_mac_vtx_idx(int intf,int ring);
extern struct mac_pTx *user_mac_ptx_idx(int intf,int ring);
extern struct sp_pRx  *user_sp_to_mac_idx(int intf,int ring);



#if 0
struct net_device *ext_dev,*mac_dev[2],*sarPP_dev;

extern int pp_trace_enable;
extern int pp_trace_debug_enable;

/* Packet Processor descriptors */
extern uint8 pp_mac_vrx_ring_size[MAC_RX_INTFS];
extern int cpu_sar_vrx_idx[SAR_INTFS];
extern int cpu_mac_vrx_idx[MAC_RX_INTFS];
extern int cpu_ext_vrx_idx[EXT_INTFS];
extern int cpu_mac_vtx_idx[MAC_TX_INTFS];
extern int cpu_ext_ptx_idx[EXT_INTFS];
extern uint32 cpu_mac_vtx_skb[MAC_TX_INTFS][MAC_VTX_RING_SIZE]; // for vtx resource recycle

extern struct rtl8672_intf_tbl intf_table[];
extern void rtl8672_dumpHSA(rtl8672_tblAsicDrv_hsaParam_t *hsa);
extern void rtl8672_dumpHSB(rtl8672_tblAsicDrv_hsbParam_t *hsb);

/* Packet Processor device */
#ifdef SAR_USE_TASKLET
struct tasklet_struct ppsar_rx_tasklets;
#endif
#ifdef MAC_USE_TASKLET
struct tasklet_struct ppmac_rx_tasklets;
#endif

#endif

/* /proc file system */
#define PPL2_PROC_NAME "ppl2_table"
#define PPL4_PROC_NAME "ppl4_table"
#define PPINTF_PROC_NAME "ppintf_table"
#define PPHSA_PROC_NAME "pphsa_table"
#define PPHSB_PROC_NAME "pphsb_table"

struct generic_pp_table {
	u32	sram_idx;
	u32	sram_way;
	u32 dram_bit;
	u32 dram_way;

	u32 max_pos;
	loff_t pos;
	size_t data_size;
	
	int (*func_hashIdexBits)(u32 *);
	int (*func_getWay)(u32 *);
	int (*func_sram_get)(u32, u32, void *);
	int (*func_get)(u32, u32, void *);
	int (*func_get2) (u32,void*);
	int (*func_get3) (void*);
};

static void *pp_seq_start3(struct seq_file *seq, loff_t *pos) {

	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	u32 max_pos;
	void *data;

	max_pos = 1;
	if (*pos >= max_pos) {
		return NULL;
	}
	gen->max_pos = max_pos;
	gen->pos = *pos;
		
	data = kmalloc(gen->data_size, GFP_KERNEL);
	if (data) {
		gen->func_get3(data);
	}
	return data;
}

static void *pp_seq_next3(struct seq_file *seq, void *v, loff_t *pos) {

	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	++*pos;
	gen->pos = *pos;

	if (*pos < gen->max_pos) {
		gen->func_get3(v);
	} else {
		return NULL;
	}

	return v;
}

static void *pp_seq_start2(struct seq_file *seq, loff_t *pos) {

	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	u32 max_pos;
	void *data;

	max_pos = SAR_INTFS+MAC_TX_INTFS+EXT_INTFS;
	if (*pos >= max_pos) {
		return NULL;
	}

	printk("[%s, line %d] gen->pos = %d\n",__func__,__LINE__,(unsigned int)gen->pos);
	
	gen->max_pos = max_pos;
	gen->pos = *pos;

	printk("[%s, line %d] gen->pos = %d\n",__func__,__LINE__,(unsigned int)gen->pos);
		
	data = kmalloc(gen->data_size, GFP_KERNEL);
	if (data) {
		gen->func_get2(0,data);
	}
	return data;
}

static void *pp_seq_next2(struct seq_file *seq, void *v, loff_t *pos) {

	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	
	++*pos;
	gen->pos = *pos;

	if (*pos < gen->max_pos) {
		gen->func_get2(*pos,v);
	} else {
		return NULL;
	}

	return v;
}

static void *pp_seq_start(struct seq_file *seq, loff_t *pos) 
{
	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	u32 hash, max_pos;
	void *data;

	gen->func_hashIdexBits(&gen->dram_bit);
	gen->func_getWay(&gen->dram_way);
	hash = 0x1 << gen->dram_bit;

	//printk("[%s, line %d] gen->pos = %d\n",__func__,__LINE__,(unsigned int)gen->pos);

	max_pos = (hash * gen->dram_way) + (gen->sram_idx * gen->sram_way);	
	if (*pos >= max_pos) {
		return NULL;
	}
	gen->max_pos = max_pos;
	gen->pos = *pos;

	//printk("[%s, line %d] gen->pos = %d\n",__func__,__LINE__,(unsigned int)gen->pos);
		
	data = kmalloc(gen->data_size, GFP_KERNEL);
	if (data) {
		gen->func_sram_get(0,0,data);
	}
	return data;
}

static void *pp_seq_next(struct seq_file *seq, void *v, loff_t *pos) {

	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	//u32 sram_size = gen->sram_idx * gen->sram_way;
	u32 hash, way;
	
	++*pos;
	gen->pos = *pos;

#if 0
	if (*pos < sram_size) {
		hash = (unsigned int)(*pos) / gen->sram_way;
		way  = (unsigned int)(*pos) % gen->sram_way;
		gen->func_sram_get(hash, way, v);
	} else if (*pos < gen->max_pos) {
		hash = (unsigned int)(*pos) / gen->dram_way;
		way  = (unsigned int)(*pos) % gen->dram_way;
		gen->func_get(hash, way, v);
	} else {
		return NULL;
	}
#else
	if (*pos < gen->max_pos) {
		hash = (unsigned int)(*pos) / gen->dram_way;
		way  = (unsigned int)(*pos) % gen->dram_way;
		gen->func_get(hash, way, v);
	} else {
		return NULL;
	}
#endif


	return v;
}


static void pp_seq_stop(struct seq_file *seq, void *v) {	
	if (v)
		kfree(v);	
}

/* HSA Table show */
static int pphsa_seq_show(struct seq_file *seq, void *v) {
	rtl8672_tblAsicDrv_hsaParam_t *hsa = v;
		seq_printf(seq, "----- [Dump HSA] -------------------------------------------------------------\n");

		seq_printf(seq, "[HSA] outiftype=%01x   outl2encap=%01x   outlanfcs=%01x   pppprotolen=%01x   outsarhdr=%01x \n\
[HSA] droppacket=%01x  pif=%01x     l3change=%01x  l4change=%01x    fromcpu=%01x  tocpu=%01x \n\
[HSA] iniftype=%01x    tif=%01x     linkid=%01x    dstportidx=%01x  srcportidx=%01x\n\
[HSA] framectrl=%03x   ethtp=%04x   pid=%01x    cfi=%01x    vid=%03x    vidremark=%01x	reason=%02x \n\
[HSA] dmac=%02x:%02x:%02x:%02x:%02x:%02x   smac=%02x:%02x:%02x:%02x:%02x:%02x   sid=%02x \n\
[HSA] mac3=%02x:%02x:%02x:%02x:%02x:%02x   mac4=%02x:%02x:%02x:%02x:%02x:%02x   l3type=%01x \n\
[HSA] tosremr=%01x   tos=%02x  ttl=%02x   inl3offset=%04x   inl3len=%04x  l3cs=%04x \n\
[HSA] sip=%08x  dip=%08x   sport=%04x  dport=%04x  l4cs=%04x  l4proto=%02x \n\
------------------------------------------------------------------------------\n",
			hsa->outiftype, hsa->outl2encap, hsa->outlanfcs, hsa->pppprotolen, hsa->outsarhdr,
			hsa->droppacket, hsa->pif, hsa->l3change, hsa->l4change, hsa->fromcpu, hsa->tocpu,
			hsa->iniftype, hsa->tif, hsa->linkid, hsa->dstportidx, hsa->srcportidx,
			hsa->framectrl, hsa->ethtp, hsa->pid, hsa->cfi, hsa->vid, hsa->vidremark, hsa->reason,
			hsa->dmac[0],hsa->dmac[1],hsa->dmac[2],hsa->dmac[3],hsa->dmac[4],hsa->dmac[5],
			hsa->smac[0],hsa->smac[1],hsa->smac[2],hsa->smac[3],hsa->smac[4],hsa->smac[5],
			hsa->sid,
			hsa->mac3[0],hsa->mac3[1],hsa->mac3[2],hsa->mac3[3],hsa->mac3[4],hsa->mac3[5],
			hsa->mac4[0],hsa->mac4[1],hsa->mac4[2],hsa->mac4[3],hsa->mac4[4],hsa->mac4[5],
			hsa->l3type,
			hsa->tosremr, hsa->tos, hsa->ttl, hsa->inl3offset, hsa->inl3len, hsa->l3cs,
			hsa->sip, hsa->dip, hsa->sport, hsa->dport, hsa->l4cs,hsa->l4proto);

	return 0;
}

/* HSB Table show */
static int pphsb_seq_show(struct seq_file *seq, void *v) {
	rtl8672_tblAsicDrv_hsbParam_t *hsb = v;
		seq_printf(seq, "----- [Dump HSB] -------------------------------------------------------------\n\
[HSB] fromcpu=%01x    hasmac=%01x    tif=%01x    pif=%01x     srcportidx=%01x   l3hwfwdip=%01x \n\
[HSB] linkid=%01x     pid=%01x       cfi=%01x    vid=%03x   pppCompable=%01x  framectrl=%03x  ethtp=%04x \n\
[HSB] dmac=%02x:%02x:%02x:%02x:%02x:%02x   smac=%02x:%02x:%02x:%02x:%02x:%02x   sid=%02x \n\
[HSB] mac3=%02x:%02x:%02x:%02x:%02x:%02x   mac4=%02x:%02x:%02x:%02x:%02x:%02x   l3cs=%02x \n\
[HSB] l3len=%04x   sip=%08x   dip=%08x   tos=%02x   l3offset=%04x \n\
[HSB] ttl=%02x     l3type=%01x      ipfrag=%01x    l3csok=%01x   udpnocs=%01x   l4csok=%01x \n\
[HSB] reason=%02x  l4proto=%04x  tcpflag=%02x  l4cs=%04x  sport=%04x  dport=%04x \n\
-----------------------------------------------------------------------------\n",
		hsb->fromcpu, hsb->hasmac, hsb->tif, hsb->pif, hsb->srcportidx, hsb->l3hwfwdip,
		hsb->linkid, hsb->pid, hsb->cfi, hsb->vid, hsb->pppcompable, hsb->framectrl, hsb->ethtp,
		hsb->dmac[0],hsb->dmac[1],hsb->dmac[2],hsb->dmac[3],hsb->dmac[4],hsb->dmac[5],
		hsb->smac[0],hsb->smac[1],hsb->smac[2],hsb->smac[3],hsb->smac[4],hsb->smac[5],hsb->sid,
		hsb->mac3[0],hsb->mac3[1],hsb->mac3[2],hsb->mac3[3],hsb->mac3[4],hsb->mac3[5],
		hsb->mac4[0],hsb->mac4[1],hsb->mac4[2],hsb->mac4[3],hsb->mac4[4],hsb->mac4[5],
		hsb->l3cs,
		hsb->l3len, hsb->sip, hsb->dip, hsb->tos, hsb->l3offset,
		hsb->ttl, hsb->l3type, hsb->ipfrag, hsb->l3csok, hsb->udpnocs, hsb->l4csok,
		hsb->reason, hsb->l4proto, hsb->tcpflag, hsb->l4cs, hsb->sport, hsb->dport);

	return 0;
}

/* Interface Table show */
static int ppintf_seq_show(struct seq_file *seq, void *v) {
	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	rtl8672_tblAsicDrv_intfParam_t *intfp = v;
	if (!gen->pos) {
		seq_printf(seq, "[index] ATM TEN CLP PTI  TRLR         GMAC        Encap FCS CPPP Tpe SARHdr RxSft   GIP   Tag UnT PID srcFl L2En 1QRe PVID LID MTU VIDRe  \n");
	}

		seq_printf(seq, "[Intf %1x] %01x   %01x   %01x   %01x   %04x   %02x-%02x-%02x-%02x-%02x-%02x   %01x    %01x    %01x   %01x    %01x     %02x  %08x  %01x   %01x   %01x    %01x    %01x    %01x    %03x  %01x  %03x  %03x\n",						
						(unsigned int)gen->pos,
						intfp->ATMPORT, 
						intfp->TRLREN,
						intfp->CLP,
						intfp->PTI,
						intfp->TRLR,
						intfp->GMAC[0], intfp->GMAC[1], intfp->GMAC[2], intfp->GMAC[3], intfp->GMAC[4], intfp->GMAC[5],
						intfp->L2Encap,
						intfp->LanFCS,
						intfp->CompPPP,
						intfp->IfType,
						intfp->SARhdr,
						intfp->RXshift,
						intfp->GIP,
						intfp->AcceptTagged,
						intfp->AcceptUntagged,
						intfp->PortPriorityID,
						intfp->SrcPortFilter,
						intfp->L2BridgeEnable,
						intfp->Dot1QRemr,
						intfp->PortVlanID,
					  	intfp->LogicalID,
						intfp->MTU,
						intfp->VlanIDRemr);

	return 0;
}

/* L2 table show */
static int ppl2_seq_show(struct seq_file *seq, void *v) {
	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	rtl8672_tblAsicDrv_l2Param_t *l2p = v;

	//printk("[%s] gen->pos = %d\n",__func__,(unsigned int)gen->pos);
	
	if (!gen->pos) {
		seq_printf(seq, "          v MAC               VID Port Age SADrop oTagIf 1PRemr PIDR APMAC             \n");
	}

	if (l2p->Valid)
	seq_printf(seq, "[L2 %04x] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",						
						(unsigned int)gen->pos,
						l2p->Valid,
						l2p->MAC[0], l2p->MAC[1], l2p->MAC[2], l2p->MAC[3], l2p->MAC[4], l2p->MAC[5],
						l2p->VlanID,
						l2p->Port,
						l2p->Age,
						l2p->SADrop,
						l2p->OTagIf,
						l2p->Dot1PRemr,
						l2p->PriorityIDRemr,
						l2p->APMAC[0], l2p->APMAC[1], l2p->APMAC[2], l2p->APMAC[3], l2p->APMAC[4], l2p->APMAC[5]);
				
	return 0;
}

/* L4 table show */
static int ppl4_seq_show(struct seq_file *seq, void *v) {
	struct generic_pp_table *gen = (struct generic_pp_table *)seq->private;
	rtl8672_tblAsicDrv_l4Param_t *l4p = v;
		
	if (!gen->pos) {
		seq_printf(seq, "         v Pt SrcIP:Port    DstIP:Port    Age NewSrcIP:Port NewDstIP:Port DPI NextHop-MAC       Td Pr Tr TOS VC iVID oVID oTI 1Pr PID iPI oPI SID  \n");		
	}

	if (l4p->Valid)
	seq_printf(seq, "[L4 %04x] %01x %02x %08x:%04x %08x:%04x %01x   %08x:%04x %08x:%04x %01x   %02x:%02x:%02x:%02x:%02x:%02x %01x  %01x  %01x  %02x  %01x  %03x  %03x  %01x   %01x   %01x   %01x   %01x   %04x \n",
						(unsigned int)gen->pos,		
						l4p->Valid,
						l4p->Proto,
						l4p->SrcIP,
						l4p->SrcPort,
						l4p->DstIP,
						l4p->DstPort,
						l4p->Age,
						l4p->NewSrcIP,
						l4p->NewSrcPort,
						l4p->NewDstIP,
						l4p->NewDstPort,
						l4p->DstPortIdx,
						l4p->NHMAC[0], l4p->NHMAC[1], l4p->NHMAC[2], l4p->NHMAC[3], l4p->NHMAC[4], l4p->NHMAC[5],
						l4p->TtlDe,
						l4p->PrecedRemr,
						l4p->TosRemr,
						l4p->Tos,
						l4p->IVlanIDChk,
						l4p->IVlanID,
						l4p->OVlanID,
						l4p->OTagIf,
						l4p->Dot1PRemr,
						l4p->PriorityID,
						l4p->IPppoeIf,
						l4p->OPppoeIf,
						l4p->SessionID);			
	return 0;
}

/* HSB table operation */
static struct seq_operations pphsb_seq_ops = {
	.start	= pp_seq_start3,
	.next	= pp_seq_next3,
	.stop	= pp_seq_stop,
	.show	= pphsb_seq_show,
};

static int pphsb_seq_open(struct inode *inode, struct file *file) {	
	struct seq_file *seq;
	int rc = -EAGAIN;
	struct generic_pp_table *g = kmalloc(sizeof(struct generic_pp_table), GFP_KERNEL);

	if (!g)
		goto out;
	rc = seq_open(file, &pphsb_seq_ops);
	if (rc)
		goto out_kfree;


	g->sram_idx = 0;
	g->sram_way = 0;

	g->data_size = sizeof(rtl8672_tblAsicDrv_hsbParam_t);
	g->func_hashIdexBits 	= NULL;
	g->func_getWay		= NULL;
	g->func_sram_get	= NULL;
	g->func_get		= NULL;
	g->func_get2		= NULL;
	g->func_get3		= (int(*)(void *))rtl8672_getAsicHSB;

	seq = file->private_data;
	seq->private = (void *)g;
out:
	return rc;

out_kfree:
	kfree(g);
	goto out;
}

static struct file_operations pphsb_seq_fops = {
	.owner		= THIS_MODULE,
	.open       = pphsb_seq_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release	= seq_release_private,
};

/* HSA table operation */
static struct seq_operations pphsa_seq_ops = {
	.start	= pp_seq_start3,
	.next	= pp_seq_next3,
	.stop	= pp_seq_stop,
	.show	= pphsa_seq_show,
};

static int pphsa_seq_open(struct inode *inode, struct file *file) {	
	struct seq_file *seq;
	int rc = -EAGAIN;
	struct generic_pp_table *g = kmalloc(sizeof(struct generic_pp_table), GFP_KERNEL);

	if (!g)
		goto out;
	rc = seq_open(file, &pphsa_seq_ops);
	if (rc)
		goto out_kfree;


	g->sram_idx = 0;
	g->sram_way = 0;

	g->data_size = sizeof(rtl8672_tblAsicDrv_hsaParam_t);
	g->func_hashIdexBits 	= NULL;
	g->func_getWay		= NULL;
	g->func_sram_get	= NULL;
	g->func_get		= NULL;
	g->func_get2		= NULL;
	g->func_get3		= (int(*)(void *))rtl8672_getAsicHSA;

	seq = file->private_data;
	seq->private = (void *)g;
out:
	return rc;

out_kfree:
	kfree(g);
	goto out;
}

static struct file_operations pphsa_seq_fops = {
	.owner		= THIS_MODULE,
	.open       = pphsa_seq_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release	= seq_release_private,
};

/* interface table operation */
static struct seq_operations ppintf_seq_ops = {
	.start	= pp_seq_start2,
	.next	= pp_seq_next2,
	.stop	= pp_seq_stop,
	.show	= ppintf_seq_show,
};

static int pp_dumpPTMPRXRing2( char *page, char **start, off_t off, int count, int *eof, void *data )
{
        int i=0;
       // int idx=2;
       int idx = 0;
        int pp,ip;
        struct mac_pRx *prx=user_ptm_prx_idx(idx,i);

        while(1)
        {
                pp=(READ_MEM32(MPRXDESC0+(idx<<2))>>16)&0xfff;
                ip=READ_MEM32(MPRXDESC0+(idx<<2))&0xfff;

		rtlglue_printf("%08x %s%s OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d PID1/PID0=%01x/%01x S_EN=%d ID=%d OF=%d FS/LS=%d/%d FAT/LPKT/RUNT=%d/%d/%d RES/CRC=%d/%d\n",(uint32)prx,(pp==i)?"P":" ",(ip==i)?"I":" ",prx->own,prx->eor,prx->data_length,prx->rx_buffer_addr,prx->rx_shift,prx->pid1,prx->pid0,prx->sram_en,prx->sram_map_id,prx->sram_map_of,prx->fs,prx->ls,prx->fae,prx->lpkt,prx->runt,prx->res,prx->crc);

		if(prx->eor) break;
                prx=user_ptm_prx_idx(idx,++i);
        }

        rtlglue_printf("-----------------------------------------------------------------------------\n");
        return SUCCESS;
}

static int pp_dumpPTMVRXRing2( char *page, char **start, off_t off, int count, int *eof, void *data )
{
        int i=0;
        int idx=2;
        int pp;
        struct mac_vRx *vrx=(struct mac_vRx*)(void *)user_ptm_vrx_idx(idx,i);

        while(1)
        {


		pp=READ_MEM32(SVRXDESC0+(idx<<2))&0xfff;
		rtlglue_printf("%08x %s OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d \n",(uint32)vrx,(pp==i)?"I":" ",vrx->own,vrx->eor,vrx->data_length,vrx->rx_buffer_addr,vrx->rx_shift);
		if(vrx->eor) break;
                vrx=(struct mac_vRx*)(void *)user_ptm_vrx_idx(idx,++i);
        }

        rtlglue_printf("-----------------------------------------------------------------------------\n");
        return SUCCESS;
}

static int pp_dumpPTMVTXRing2( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=2;
	int pp;

	struct mac_vTx *vtx=user_ptm_vtx_idx(idx,i);
	while(1)
	{
		pp=READ_MEM32(SVTXDESC0+(idx<<2))&0xfff;
		rtlglue_printf("%s  0x%x OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x\n",(pp==i)?"P":" ",(uint32)vtx,vtx->own,vtx->eor,vtx->data_length,vtx->tx_buffer_addr,(vtx->orgAddr<<2));
		if(vtx->eor) break;
		vtx=user_ptm_vtx_idx(idx,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}


static int pp_dumpPTMPTXRing2( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=2;
	int ip,pp;

	struct mac_pTx *ptx=user_ptm_ptx_idx(idx,i);
	while(1)
	{
		pp=(READ_MEM32(SPTXDESC0+(idx<<2))>>16)&0xfff;
		ip=READ_MEM32(SPTXDESC0+(idx<<2))&0xfff;
		rtlglue_printf("%08x %s%s OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x OEOR=%d S_ID=%03d S_SIZE=%03d\n",(uint32)ptx,(pp==i)?"P":" ",(ip==i)?"I":" ",ptx->own,ptx->eor,ptx->data_length,ptx->tx_buffer_addr,(ptx->orgAddr<<2),ptx->oeor,ptx->sram_map_id,ptx->sram_size);
		if(ptx->eor) break;
		ptx=user_ptm_ptx_idx(idx,++i);
	}




	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpPTMSPRing2( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=2;
	int pp;

	struct sp_pRx *prx=user_sp_to_ptm_idx(idx,i);
	while(1)
	{
		pp=READ_MEM32(SPSAR0DESC+(idx<<2))&0xfff;
		rtlglue_printf("%s  DESC=%08x OWN=%d EOR=%d BUF=%08x SKB=%08x S_EN=%d ID=%d OF=%d\n",(pp==i)?"P":" ",(uint32)prx,prx->own,prx->eor,prx->rx_buffer_addr,prx->skb_header_addr,prx->sram_en,prx->sram_map_id,prx->sram_map_of);
		if(prx->eor) break;
		prx=user_sp_to_ptm_idx(idx,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpPRXRing8( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=8;
	int pp,ip;
	struct mac_pRx *prx=user_mac_prx_idx(idx-SAR_INTFS,i);

	while(1)
	{
		pp=(READ_MEM32(MPRXDESC0+((idx-SAR_INTFS)<<2))>>16)&0xfff;
		ip=READ_MEM32(MPRXDESC0+((idx-SAR_INTFS)<<2))&0xfff;
		rtlglue_printf("%08x %s%s OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d PID1/PID0=%01x/%01x S_EN=%d ID=%d OF=%d FS/LS=%d/%d FAT/LPKT/RUNT=%d/%d/%d RES/CRC=%d/%d\n",(uint32)prx,(pp==i)?"P":" ",(ip==i)?"I":" ",prx->own,prx->eor,prx->data_length,prx->rx_buffer_addr,prx->rx_shift,prx->pid1,prx->pid0,prx->sram_en,prx->sram_map_id,prx->sram_map_of,prx->fs,prx->ls,prx->fae,prx->lpkt,prx->runt,prx->res,prx->crc);
		if(prx->eor) break;
		prx=user_mac_prx_idx(idx-SAR_INTFS,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpVRXRing8( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=8;
	int ip;
	struct mac_vRx *vrx=user_mac_vrx_idx(idx-SAR_INTFS,i);

	while(1)
	{
		ip=READ_MEM32(MVRXDESC0+((idx-SAR_INTFS)<<2))&0xfff;
		rtlglue_printf("%08x %s OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d \n",(uint32)vrx,(ip==i)?"I":" ",vrx->own,vrx->eor,vrx->data_length,vrx->rx_buffer_addr,vrx->rx_shift);
		if(vrx->eor) break;
		vrx=user_mac_vrx_idx(idx-SAR_INTFS,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpPRXRing9( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=9;
	int pp,ip;
	struct mac_pRx *prx=user_mac_prx_idx(idx-SAR_INTFS,i);

	while(1)
	{
		pp=(READ_MEM32(MPRXDESC0+((idx-SAR_INTFS)<<2))>>16)&0xfff;
		ip=READ_MEM32(MPRXDESC0+((idx-SAR_INTFS)<<2))&0xfff;
		rtlglue_printf("%08x %s%s OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d PID1/PID0=%01x/%01x S_EN=%d ID=%d OF=%d FS/LS=%d/%d FAT/LPKT/RUNT=%d/%d/%d RES/CRC=%d/%d\n",(uint32)prx,(pp==i)?"P":" ",(ip==i)?"I":" ",prx->own,prx->eor,prx->data_length,prx->rx_buffer_addr,prx->rx_shift,prx->pid1,prx->pid0,prx->sram_en,prx->sram_map_id,prx->sram_map_of,prx->fs,prx->ls,prx->fae,prx->lpkt,prx->runt,prx->res,prx->crc);
		if(prx->eor) break;
		prx=user_mac_prx_idx(idx-SAR_INTFS,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpVRXRing9( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=9;
	int ip;
	struct mac_vRx *vrx=user_mac_vrx_idx(idx-SAR_INTFS,i);

	while(1)
	{
		ip=READ_MEM32(MVRXDESC0+((idx-SAR_INTFS)<<2))&0xfff;
		rtlglue_printf("%08x %s OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d \n",(uint32)vrx,(ip==i)?"I":" ",vrx->own,vrx->eor,vrx->data_length,vrx->rx_buffer_addr,vrx->rx_shift);
		if(vrx->eor) break;
		vrx=user_mac_vrx_idx(idx-SAR_INTFS,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpVTXRing9( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=9;
	int pp;

	struct mac_vTx *vtx=user_mac_vtx_idx(idx-SAR_INTFS,i);
	while(1)
	{
		pp=READ_MEM32(MVTXDESC0+((idx-SAR_INTFS)<<2))&0xfff;
		rtlglue_printf("%08x %s  OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x\n",(uint32)vtx,(pp==i)?"P":" ",vtx->own,vtx->eor,vtx->data_length,vtx->tx_buffer_addr,(vtx->orgAddr<<2));
		if(vtx->eor) break;
		vtx=user_mac_vtx_idx(idx-SAR_INTFS,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpPTXRing9( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=9;
	int ip,pp;

	struct mac_pTx *ptx=user_mac_ptx_idx(idx-SAR_INTFS,i);
	while(1)
	{
		pp=(READ_MEM32(MPTXDESC0+((idx-SAR_INTFS)<<2))>>16)&0xfff;
		ip=READ_MEM32(MPTXDESC0+((idx-SAR_INTFS)<<2))&0xfff;
		rtlglue_printf("%08x %s%s OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x OEOR=%d S_ID=%03d S_SIZE=%03d CRC=%d VTAG=%d VPRI=%d\n",(uint32)ptx,(pp==i)?"P":" ",(ip==i)?"I":" ",ptx->own,ptx->eor,ptx->data_length,ptx->tx_buffer_addr,(ptx->orgAddr<<2),ptx->oeor,ptx->sram_map_id,ptx->sram_size,ptx->crc,ptx->tagc,ptx->vlan_prio);
		if(ptx->eor) break;
		ptx=user_mac_ptx_idx(idx-SAR_INTFS,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpSPRing9( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int i=0;
	int idx=9;
	int pp;

	struct sp_pRx *prx=user_sp_to_mac_idx(idx-SAR_INTFS,i);
	while(1)
	{
		pp=READ_MEM32(SPMAC0DESC+((idx-SAR_INTFS)<<2))&0xfff;
		rtlglue_printf("%08x %s  OWN=%d EOR=%d BUF=%08x SKB=%08x S_EN=%d ID=%d OF=%d\n",(uint32)prx,(pp==i)?"P":" ",prx->own,prx->eor,prx->rx_buffer_addr,prx->skb_header_addr,prx->sram_en,prx->sram_map_id,prx->sram_map_of);
		if(prx->eor) break;
		prx=user_sp_to_mac_idx(idx-SAR_INTFS,++i);
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;
}

static int pp_dumpCounter( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	unsigned int i=0,intf;
	rtlglue_printf("[Idx] pRxDrop vRxRecv vTxFwd spFwd\n");
	for(intf=0;intf<SAR_INTFS+MAC_TX_INTFS+EXT_INTFS;intf++){
		if(intf<SAR_INTFS){
			i=intf;
			rtlglue_printf("[ %01x ]   %04x   %04x   %04x   %04x   \n",intf,READ_MEM32(SPRXDROPCNT0+(i<<2))&0xffff,READ_MEM32(SVRXRECVCNT0+(i<<2))&0xffff,READ_MEM32(SVTXFWDCNT0+(i<<2)),READ_MEM32(SPSAR0FWDCNT+(i<<2)));
		}else if(intf<SAR_INTFS+MAC_RX_INTFS){
			i=intf-SAR_INTFS;
			rtlglue_printf("[ %01x ]   %04x   %04x   %04x   %04x   \n",intf,READ_MEM32(MPRXDROPCNT0+(i<<2))&0xffff,READ_MEM32(MVRXRECVCNT0+(i<<2))&0xffff,READ_MEM32(MVTXFWDCNT0+(i<<2))&0xffff,READ_MEM32(SPMAC0FWDCNT+(i<<2))&0xffff);
		}else if(intf<SAR_INTFS+MAC_TX_INTFS){
			i=intf-SAR_INTFS;
			rtlglue_printf("[ %01x ]   ----   ----   %04x   %04x   \n",intf,READ_MEM32(MVTXFWDCNT0+(i<<2))&0xffff,READ_MEM32(SPMAC0FWDCNT+(i<<2))&0xffff);
		}else{
			i=intf-SAR_INTFS-MAC_TX_INTFS;
			rtlglue_printf("[ %01x ]   %04x   %04x   %04x   %04x   \n",intf,READ_MEM32(EPRXDROPCNT0+(i<<2))&0xffff,READ_MEM32(EVRXRECVCNT0+(i<<2))&0xffff,READ_MEM32(EVTXFWDCNT0+(i<<2))&0xffff,READ_MEM32(SPEXT0FWDCNT+(i<<2))&0xffff);
		}
	}
	return SUCCESS;
}

static int dump_pp_hsab_read(char *page, char **start, off_t off,
                int count, int *eof, void *data)
{
      int len;
      extern int Dump_ppTrapCPU_hsab_debug;
      len = sprintf(page, "Dump pkt trapped to CPU  : %s\n", Dump_ppTrapCPU_hsab_debug?"Enable":"Disable");

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int dump_pp_hsab_write(struct file *file, const char *buffer,unsigned long count, void *data)
{
   	unsigned char chartmp;
        extern int Dump_ppTrapCPU_hsab_debug;

	if (count > 1) {    //call from shell
		if (buffer && !copy_from_user(&chartmp, buffer, 1))
		{
		        Dump_ppTrapCPU_hsab_debug = chartmp - '0';
	                printk("Dump pkt trapped to CPU  : %s\n", Dump_ppTrapCPU_hsab_debug?"Enable":"Disable");
		}
	}
        else{
                printk("write fail\n");
                return -EFAULT;
        }
        return count;
	
}


static int dump_pp_packet2CPU_read(char *page, char **start, off_t off,
                int count, int *eof, void *data)
{
      int len;
      extern int Dump_ppTrapCPUpkt_debug;
      len = sprintf(page, "Dump pkt trapped to CPU  : %s\n", Dump_ppTrapCPUpkt_debug?"Enable":"Disable");

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int dump_pp_packet2CPU_write(struct file *file, const char *buffer,unsigned long count, void *data)
{
   	unsigned char chartmp;
        extern int Dump_ppTrapCPUpkt_debug;

	if (count > 1) {    //call from shell
		if (buffer && !copy_from_user(&chartmp, buffer, 1))
		{
		        Dump_ppTrapCPUpkt_debug = chartmp - '0';
	                printk("Dump pkt trapped to CPU  : %s\n", Dump_ppTrapCPUpkt_debug?"Enable":"Disable");
		}
	}
        else{
                printk("write fail\n");
                return -EFAULT;
        }
        return count;
	
}


static int ppintf_seq_open(struct inode *inode, struct file *file) {	
	struct seq_file *seq;
	int rc = -EAGAIN;
	struct generic_pp_table *g = kmalloc(sizeof(struct generic_pp_table), GFP_KERNEL);

	if (!g)
		goto out;
	rc = seq_open(file, &ppintf_seq_ops);
	if (rc)
		goto out_kfree;


	g->sram_idx = 0;
	g->sram_way = 0;

	g->data_size = sizeof(rtl8672_tblAsicDrv_intfParam_t);
	g->func_hashIdexBits 	= NULL;
	g->func_getWay		= NULL;
	g->func_sram_get	= NULL;
	g->func_get		= NULL;
	g->func_get2		= (int(*)(u32,void *))rtl8672_getAsicNetInterface;

	seq = file->private_data;
	seq->private = (void *)g;
out:
	return rc;

out_kfree:
	kfree(g);
	goto out;
}

static struct file_operations ppintf_seq_fops = {
	.owner		= THIS_MODULE,
	.open       = ppintf_seq_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release	= seq_release_private,
};

static struct seq_operations ppl2_seq_ops = {
	.start	= pp_seq_start,
	.next	= pp_seq_next,
	.stop	= pp_seq_stop,
	.show	= ppl2_seq_show,
};

static int ppl2_seq_open(struct inode *inode, struct file *file) {	
	struct seq_file *seq;
	int rc = -EAGAIN;
	struct generic_pp_table *g = kmalloc(sizeof(struct generic_pp_table), GFP_KERNEL);

	if (!g)
		goto out;
	rc = seq_open(file, &ppl2_seq_ops);
	if (rc)
		goto out_kfree;


	g->sram_idx = ALE_L2TABLE_SRAM_IDX;
	g->sram_way = ALE_L2TABLE_SRAM_WAY;

	g->data_size = sizeof(rtl8672_tblAsicDrv_l2Param_t);
	g->func_hashIdexBits 	= rtl8672_getAsicL2HashIdxBits;
	g->func_getWay 		= rtl8672_getAsicL2Way;
	g->func_sram_get		= (int(*)(u32,u32,void *))rtl8672_getAsicL2Table_Sram;
	g->func_get			= (int(*)(u32,u32,void *))rtl8672_getAsicL2Table;

	seq = file->private_data;
	seq->private = (void *)g;
out:
	return rc;

out_kfree:
	kfree(g);
	goto out;
}


static struct file_operations ppl2_seq_fops = {
	.owner		= THIS_MODULE,
	.open       = ppl2_seq_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release	= seq_release_private,
};



static struct seq_operations ppl4_seq_ops = {
	.start	= pp_seq_start,
	.next	= pp_seq_next,
	.stop	= pp_seq_stop,
	.show	= ppl4_seq_show,
};



static int ppl4_seq_open(struct inode *inode, struct file *file) {
	struct seq_file *seq;
	int rc = -EAGAIN;
	struct generic_pp_table *g = kmalloc(sizeof(struct generic_pp_table), GFP_KERNEL);

	if (!g)
		goto out;
	rc = seq_open(file, &ppl4_seq_ops);
	if (rc)
		goto out_kfree;


	g->sram_idx = ALE_L4TABLE_SRAM_IDX;
	g->sram_way = ALE_L4TABLE_SRAM_WAY;

	g->data_size = sizeof(rtl8672_tblAsicDrv_l4Param_t);
	g->func_hashIdexBits = rtl8672_getAsicL4HashIdxBits;
	g->func_getWay 		 = rtl8672_getAsicL4Way;
	g->func_sram_get	 = (int(*)(u32,u32,void *))rtl8672_getAsicL4Table_Sram;
	g->func_get			 = (int(*)(u32,u32,void *))rtl8672_getAsicL4Table;

	seq = file->private_data;
	seq->private = (void *)g;
out:
	return rc;

out_kfree:
	kfree(g);
	goto out;
	goto out;
}

static struct file_operations ppl4_seq_fops = {
	.owner		= THIS_MODULE,
	.open       = ppl4_seq_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release	= seq_release_private,
};


__init int pp_debug_proc_init(void) {
	struct proc_dir_entry *p;

#if 0
        p = create_proc_entry("ppcounter", S_IRUGO, proc_net);
        if (p)  p->read_proc = pp_dumpCounter;

        p = create_proc_entry("ptmprxring2", S_IRUGO, proc_net);
        if (p)  p->read_proc = pp_dumpPTMPRXRing2;

        p = create_proc_entry("ptmvrxring2", S_IRUGO, proc_net);
        if (p)  p->read_proc = pp_dumpPTMVRXRing2;

        p = create_proc_entry("prxring8", S_IRUGO, proc_net);
        if (p)  p->read_proc = pp_dumpPRXRing8;

        p = create_proc_entry("vrxring8", S_IRUGO, proc_net);
        if (p)  p->read_proc = pp_dumpVRXRing8;

        p = create_proc_entry("prxring9", S_IRUGO, proc_net);
        if (p)  p->read_proc = pp_dumpPRXRing9;

        p = create_proc_entry("vrxring9", S_IRUGO, proc_net);
        if (p)  p->read_proc = pp_dumpVRXRing9;
 
	p = create_proc_entry(PPL2_PROC_NAME, S_IRUGO, proc_net);
	if (p)	p->proc_fops = &ppl2_seq_fops;

	p = create_proc_entry(PPL4_PROC_NAME, S_IRUGO, proc_net);
	if (p)	p->proc_fops = &ppl4_seq_fops;

	p = create_proc_entry(PPINTF_PROC_NAME, S_IRUGO, proc_net);
	if (p)	p->proc_fops = &ppintf_seq_fops;

	p = create_proc_entry(PPHSA_PROC_NAME, S_IRUGO, proc_net);
	if (p)	p->proc_fops = &pphsa_seq_fops;

	p = create_proc_entry(PPHSB_PROC_NAME, S_IRUGO, proc_net);
	if (p)	p->proc_fops = &pphsb_seq_fops;
#else
	p = create_proc_entry("pkt2cpu", S_IRUGO, init_net.proc_net);
	if (p){
		p->read_proc = dump_pp_packet2CPU_read;
		p->write_proc = dump_pp_packet2CPU_write;
	}

	p = create_proc_entry("HSAB", S_IRUGO, init_net.proc_net);
	if (p){
		p->read_proc = dump_pp_hsab_read;
		p->write_proc = dump_pp_hsab_write;
	}

        p = create_proc_entry("ppcounter", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpCounter;

        p = create_proc_entry("ptmprxring2", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpPTMPRXRing2;

        p = create_proc_entry("ptmvrxring2", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpPTMVRXRing2;

        p = create_proc_entry("ptmvtxring2", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpPTMVTXRing2;

        p = create_proc_entry("ptmptxring2", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpPTMPTXRing2;

        p = create_proc_entry("ptmspring2", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpPTMSPRing2;

        p = create_proc_entry("prxring8", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpPRXRing8;

        p = create_proc_entry("vrxring8", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpVRXRing8;

        p = create_proc_entry("prxring9", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpPRXRing9;

        p = create_proc_entry("vrxring9", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpVRXRing9;
 
        p = create_proc_entry("vtxring9", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpVTXRing9;

        p = create_proc_entry("ptxring9", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpPTXRing9;

        p = create_proc_entry("spring9", S_IRUGO, init_net.proc_net);
        if (p)  p->read_proc = pp_dumpSPRing9;

	p = create_proc_entry(PPL2_PROC_NAME, S_IRUGO, init_net.proc_net);
	if (p)	p->proc_fops = &ppl2_seq_fops;

	p = create_proc_entry(PPL4_PROC_NAME, S_IRUGO, init_net.proc_net);
	if (p)	p->proc_fops = &ppl4_seq_fops;

	p = create_proc_entry(PPINTF_PROC_NAME, S_IRUGO, init_net.proc_net);
	if (p)	p->proc_fops = &ppintf_seq_fops;

	p = create_proc_entry(PPHSA_PROC_NAME, S_IRUGO, init_net.proc_net);
	if (p)	p->proc_fops = &pphsa_seq_fops;

	p = create_proc_entry(PPHSB_PROC_NAME, S_IRUGO, init_net.proc_net);
	if (p)	p->proc_fops = &pphsb_seq_fops;
#endif

	return 0;
}

module_init(pp_debug_proc_init);

