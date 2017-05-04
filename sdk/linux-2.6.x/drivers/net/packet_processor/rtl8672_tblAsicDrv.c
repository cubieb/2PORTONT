/*
 * Copyright (c) 2007 Realtek Semiconductor Corporation.
 *
 * Program : Source File of RTL8672 Asic Driver
 * Abstract :
 * Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
 * $Id: rtl8672_tblAsicDrv.c,v 1.3 2012/07/12 08:30:15 czyao Exp $
 */

#include "rtl_types.h"
#include "rtl_glue.h"
#include "rtl_utils.h"
#include "rtl8672_asicregs.h"
#include "rtl8672_tblAsicDrv.h"
#include "icTest_ringController.h"


uint32 rtl8672_L2Hash(uint8 *mac, uint32 vid)
{
	uint32 hashidx = 0;

	hashidx = ((mac[0] << 4) | (mac[1] >> 4)) ^
		  ((mac[1] << 8) | (mac[2] >> 0)) ^
		  ((mac[3] << 4) | (mac[4] >> 4)) ^
		  ((mac[4] << 8) | (mac[5] >> 0)) ^ vid;

	hashidx = ((hashidx>>11)&(0x1<<0)) |
			((hashidx>>9)&(0x1<<1)) |
			((hashidx>>7)&(0x1<<2)) |
			((hashidx>>5)&(0x1<<3)) |
			((hashidx>>3)&(0x1<<4)) |
			((hashidx>>1)&(0x1<<5)) |
			((hashidx<<1)&(0x1<<6)) |
			((hashidx<<3)&(0x1<<7)) |
			((hashidx<<5)&(0x1<<8)) |
			((hashidx<<7)&(0x1<<9)) |
			((hashidx<<9)&(0x1<<10)) |
			((hashidx<<11)&(0x1<<11));

	switch (READ_MEM32(ALECR_L2HIB) & L2HIB_MASK >> L2HIB_OFFSET) {
		case 0x0:	/* 7 bits */
			hashidx >>= 5;
			//hashidx &= 0x7f;
			break;
		case 0x1:	/* 8 bit */
			hashidx >>=4;
			//hashidx &= 0xff;
			break;
		case 0x2:	/* 9 bits */
			hashidx >>=3;
			//hashidx &= 0x1ff;
			break;
		case 0x3:	/* 10 bits */
			hashidx >>=2;
			//hashidx &= 0x3ff;
			break;
		case 0x4:	/* 11 bits */
			hashidx >>=1;
			//hashidx &= 0x7ff;
			break;
		case 0x5:	/* 12 bits */
			hashidx &= 0xfff;
			break;
		default:	/* 7 bits */
			hashidx >>=5;
			//hashidx &= 0x7f;
			break;
	}
//rtlglue_printf("------------- vid:%x hashidx:%d ------------------\n",vid,hashidx);	
	return hashidx;
}

uint32 rtl8672_L4Hash(uint8 proto, uint32 sip, uint32 dip, uint16 sport, uint16 dport)
{
	uint32 hashidx = 0;
#if 0
	uint32 reg_l4hib = 0;
	hashidx = ((dip & 0xfff00000) >> 20) ^
		  ((dip & 0x000fff00) >> 8) ^
		  ((dip & 0x000000ff) << 4) ^
		  ((sip & 0xf0000000) >> 28) ^
		  ((sip & 0x0fff0000) >> 16) ^
		  ((sip & 0x0000fff0) >> 4) ^
		  ((sip & 0x0000000f) << 8) ^
		  ((dport & 0xff00) >> 8) ^
		  ((dport & 0x00ff) << 4) ^
		  ((sport & 0xf000) >> 12) ^
		  ((sport & 0x0fff) >> 0) ^ proto;
	rtl8672_getAsicL4HashIdxBits(&reg_l4hib);
	hashidx &= 0x0fff >> (12 - reg_l4hib);
#endif

	hashidx = ((dip & 0xfff00000) >> 20) ^
		  ((dip & 0x000fff00) >> 8) ^
		  (((dip & 0x000000ff) << 4) | ((sip & 0xf0000000) >> 28) ) ^
		  ((sip & 0x0fff0000) >> 16) ^
		  ((sip & 0x0000fff0) >> 4) ^
		  (((sip & 0x0000000f) << 8) |  ((dport & 0xff00) >> 8)) ^
		  (((dport & 0x00ff) << 4) |  ((sport & 0xf000) >> 12) ) ^
		  ((sport & 0x0fff) >> 0) ^ proto;

	switch (READ_MEM32(ALECR_L4HIB) & L4HIB_MASK >> L4HIB_OFFSET){
			case 0x1:
				hashidx &= 0xff;
				break;
			case 0x2:
				hashidx &= 0x1ff;
				break;
			case 0x3:
				hashidx &= 0x3ff;
				break;
			case 0x4:
				hashidx &= 0x7ff;
				break;
			case 0x5:
				hashidx &= 0xfff;
				break;
			default:
				hashidx &= 0x7f;
				break;
		}
	
	return hashidx;
}

uint32 rtl8672_L2Hash_Sram(uint8 *mac, uint32 vid)
{
	uint32 hashidx = 0;
	hashidx = ((mac[0] << 4) | (mac[1] >> 4)) ^
		  ((mac[1] << 8) | (mac[2] >> 0)) ^
		  ((mac[3] << 4) | (mac[4] >> 4)) ^
		  ((mac[4] << 8) | (mac[5] >> 0)) ^ vid;
	hashidx &= ALE_L2TABLE_SRAM_IDX_MASK;

	hashidx = ((hashidx>>4)&1) |
			((hashidx>>2)&2) |
			(hashidx&4) |
			((hashidx<<2)&8) |
			((hashidx<<4)&16);
	return hashidx;
}

uint32 rtl8672_L4Hash_Sram(uint8 proto, uint32 sip, uint32 dip, uint16 sport, uint16 dport)
{
	uint32 hashidx = 0;
#if 0
	hashidx = ((dip & 0xfff00000) >> 20) ^
		  ((dip & 0x000fff00) >> 8) ^
		  ((dip & 0x000000ff) << 4) ^
		  ((sip & 0xf0000000) >> 28) ^
		  ((sip & 0x0fff0000) >> 16) ^
		  ((sip & 0x0000fff0) >> 4) ^
		  ((sip & 0x0000000f) << 8) ^
		  ((dport & 0xff00) >> 8) ^
		  ((dport & 0x00ff) << 4) ^
		  ((sport & 0xf000) >> 12) ^
		  ((sport & 0x0fff) >> 0) ^ proto;

	hashidx &= ALE_L4TABLE_SRAM_IDX_MASK;	
#endif

	hashidx = ((dip & 0xfff00000) >> 20) ^
		  ((dip & 0x000fff00) >> 8) ^
		  (((dip & 0x000000ff) << 4) | ((sip & 0xf0000000) >> 28) ) ^
		  ((sip & 0x0fff0000) >> 16) ^
		  ((sip & 0x0000fff0) >> 4) ^
		  (((sip & 0x0000000f) << 8) |  ((dport & 0xff00) >> 8)) ^
		  (((dport & 0x00ff) << 4) |  ((sport & 0xf000) >> 12) ) ^
		  ((sport & 0x0fff) >> 0) ^ proto;

	switch (READ_MEM32(ALECR_L4HIB) & L4HIB_MASK >> L4HIB_OFFSET){
			case 0x1:
				hashidx = (hashidx & 0xfc) >> 2;
				break;
			case 0x2:
				hashidx = (hashidx & 0x1f8) >> 3;
				break;
			case 0x3:
				hashidx = (hashidx & 0x3f0) >> 4;
				break;
			case 0x4:
				hashidx = (hashidx & 0x7e0) >> 5;
				break;
			case 0x5:
				hashidx = (hashidx & 0xfc0) >> 6;
				break;
			default:
				hashidx = (hashidx & 0x7e) >> 1;
				break;
		}
	
	return hashidx;
}

/* RTL8672 Asic Driver API Functions */
int32 rtl8672_setAsicL2HashIdxBits(uint32 l2hib)
{
	if (l2hib < 7 || l2hib > 12)
		return FAILED;
	
	WRITE_MEM32(ALECR_L2HIB, (l2hib - 7));
	
	return SUCCESS;
}

int32 rtl8672_getAsicL2HashIdxBits(uint32 *l2hib)
{
	uint32 reg_l2hib = 0;
	reg_l2hib = (READ_MEM32(ALECR_L2HIB) & L2HIB_MASK) >> L2HIB_OFFSET;

	*l2hib = (reg_l2hib < 6)? (reg_l2hib + 7) : 7;
	
	return SUCCESS;
}

int32 rtl8672_setAsicL2Way(uint32 l2way)
{
	if (l2way < 1 || l2way > 8 || l2way == 3 || l2way == 5 || l2way == 6 || l2way == 7)
		return FAILED;
	
	//WRITE_MEM32(ALECR_L2WAY, l2way);
	switch(l2way)
	{
		case 1:
			WRITE_MEM32(ALECR_L2WAY, 0 << L2WAY_OFFSET); break;
		case 2:
			WRITE_MEM32(ALECR_L2WAY, 1 << L2WAY_OFFSET); break;
		case 4:
			WRITE_MEM32(ALECR_L2WAY, 2 << L2WAY_OFFSET); break;
		case 8:
			WRITE_MEM32(ALECR_L2WAY, 3 << L2WAY_OFFSET); break;
		default:
			WRITE_MEM32(ALECR_L2WAY, 0 << L2WAY_OFFSET); break;
	}
	
	return SUCCESS;
}

int32 rtl8672_getAsicL2Way(uint32 *l2way)
{
	uint32 reg_l2way = 0;
	reg_l2way = (READ_MEM32(ALECR_L2WAY) & L2WAY_MASK) >> L2WAY_OFFSET;
	
	*l2way = 0x1 << (reg_l2way);
	
	return SUCCESS;
}

int32 rtl8672_setAsicL2BaseAddress(uint32 l2ba)
{
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	WRITE_MEM32(ALECR_L2TSA, (l2ba & L2TSA_MASK));
#else
	WRITE_MEM32(ALECR_L2TSA, PHYSICAL_ADDRESS(l2ba & L2TSA_MASK));
#endif

	return SUCCESS;
}

int32 rtl8672_getAsicL2BaseAddress(uint32 *l2ba)
{
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	*l2ba = (READ_MEM32(ALECR_L2TSA) & L2TSA_MASK);
#else
	*l2ba = (uint32)(KSEG1_ADDRESS(READ_MEM32(ALECR_L2TSA) & L2TSA_MASK));
#endif

	return SUCCESS;
}

int32 rtl8672_setAsicL2Ability(uint32 isEnable)
{
	/* Invalid Input Parameter */
	if ((isEnable != 0) && (isEnable != 1))
		return FAILED;

	if (isEnable == TRUE)
	{
		uint32 reg_l2teb;
		
		reg_l2teb = READ_MEM32(ALECR_L2TEB);
		WRITE_MEM32(ALECR_L2TEB, reg_l2teb | L2TEB_EN_MASK);
	} else {
		uint32 reg_l2teb;
		
		reg_l2teb = READ_MEM32(ALECR_L2TEB);
		WRITE_MEM32(ALECR_L2TEB, reg_l2teb & ~(L2TEB_EN_MASK));
	}

	return SUCCESS;
}

int32 rtl8672_getAsicL2Ability(uint32 *isEnable)
{
	/* Invalid Input Parameter */
	if (isEnable == NULL)
		return FAILED;
	
	*isEnable = (READ_MEM32(ALECR_L2TEB) & L2TEB_EN_MASK) >> L2TEB_EN_OFFSET;
	
	return SUCCESS;
}

int32 rtl8672_setAsicL4HashIdxBits(uint32 bits)
{
	if (bits < 7 || bits > 12)
		return FAILED;
	
	WRITE_MEM32(ALECR_L4HIB, (bits - 7));
	
	return SUCCESS;
}

int32 rtl8672_getAsicL4HashIdxBits(uint32 *bits)
{
	uint32 reg_l4hib = 0;
	reg_l4hib = (READ_MEM32(ALECR_L4HIB) & L4HIB_MASK) >> L4HIB_OFFSET;

	*bits = (reg_l4hib < 6)? (reg_l4hib + 7) : 7;
	
	return SUCCESS;
}

int32 rtl8672_setAsicL4Way(uint32 l4way)
{
	if (l4way < 1 || l4way > 8 || l4way == 3 || l4way == 5 || l4way == 6 || l4way == 7)
		return FAILED;
	
	//WRITE_MEM32(ALECR_L4WAY, l4way);
	switch(l4way)
	{
		case 1:
			WRITE_MEM32(ALECR_L4WAY, 0 << L4WAY_OFFSET); break;
		case 2:
			WRITE_MEM32(ALECR_L4WAY, 1 << L4WAY_OFFSET); break;
		case 4:
			WRITE_MEM32(ALECR_L4WAY, 2 << L4WAY_OFFSET); break;
		case 8:
			WRITE_MEM32(ALECR_L4WAY, 3 << L4WAY_OFFSET); break;
		default:
			WRITE_MEM32(ALECR_L4WAY, 0 << L4WAY_OFFSET); break;
	}
	
	return SUCCESS;
}

int32 rtl8672_getAsicL4Way(uint32 *l4way)
{
	uint32 reg_l4way = 0;
	reg_l4way = (READ_MEM32(ALECR_L4WAY) & L4WAY_MASK) >> L4WAY_OFFSET;
	
	*l4way = 0x1 << (reg_l4way);
	
	return SUCCESS;
}

int32 rtl8672_setAsicL4BaseAddress(uint32 l4ba)
{
	WRITE_MEM32(ALECR_L4TSA, (l4ba & L4TSA_MASK));
	
	return SUCCESS;
}

int32 rtl8672_getAsicL4BaseAddress(uint32 *l4ba)
{
	*l4ba = (READ_MEM32(ALECR_L4TSA) & L4TSA_MASK);
	
	return SUCCESS;
}

int32 rtl8672_setAsicL4Ability(uint32 isEnable)
{
	/* Invalid Input Parameter */
	if ((isEnable != 0) && (isEnable != 1))
		return FAILED;

	if (isEnable == TRUE)
	{
		uint32 reg_l4teb;
		
		reg_l4teb = READ_MEM32(ALECR_L4TEB);
		WRITE_MEM32(ALECR_L4TEB, reg_l4teb | L4TEB_EN_MASK);
	} else {
		uint32 reg_l4teb;
		
		reg_l4teb = READ_MEM32(ALECR_L4TEB);
		WRITE_MEM32(ALECR_L4TEB, reg_l4teb & ~(L4TEB_EN_MASK));
	}

	return SUCCESS;
}

int32 rtl8672_getAsicL4Ability(uint32 *isEnable)
{
	/* Invalid Input Parameter */
	if (isEnable == NULL)
		return FAILED;
	
	*isEnable = (READ_MEM32(ALECR_L4TEB) & L4TEB_EN_MASK) >> L4TEB_EN_OFFSET;
	
	return SUCCESS;
}

int32 rtl8672_setAsicNetInterface(uint32 idx, rtl8672_tblAsicDrv_intfParam_t *intfp)
{
	rtl8672_tblAsic_intfTable_t entry;
	
	if (idx >= (MAC_TX_INTFS + SAR_INTFS + EXT_INTFS) || intfp == NULL)
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.ATMPORT		= intfp->ATMPORT;
	entry.TRLREN		= intfp->TRLREN;
	entry.CLP		= intfp->CLP;
	entry.PTI		= intfp->PTI;
	entry.TRLR		= intfp->TRLR;
	entry.GMAC47_32		= (intfp->GMAC[0] << 8) | (intfp->GMAC[1] << 0);
	entry.GMAC31_16		= (intfp->GMAC[2] << 8) | (intfp->GMAC[3] << 0);
	entry.GMAC15_0		= (intfp->GMAC[4] << 8) | (intfp->GMAC[5] << 0);
	entry.L2Encap		= intfp->L2Encap;
	entry.LanFCS		= intfp->LanFCS;
	entry.CompPPP		= intfp->CompPPP;
	entry.IfType		= intfp->IfType;
	entry.SARhdr		= intfp->SARhdr;
	entry.RXshift		= intfp->RXshift;
	entry.GIP		= intfp->GIP;
	entry.AcceptTagged	= intfp->AcceptTagged;
	entry.AcceptUntagged	= intfp->AcceptUntagged;
	entry.PPID		= intfp->PortPriorityID;
	entry.SrcPortFilter	= intfp->SrcPortFilter;
	entry.L2BRIDGE		= intfp->L2BridgeEnable;
	entry.Dot1QREMR		= intfp->Dot1QRemr;
	entry.LogicalID		= intfp->LogicalID;
	entry.PVID		= intfp->PortVlanID;
	entry.MTU		= intfp->MTU;
	entry.VIDR		= intfp->VlanIDRemr;

	WRITE_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x00, *((uint32 *)&entry + 0));
	WRITE_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x04, *((uint32 *)&entry + 1));
	WRITE_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x08, *((uint32 *)&entry + 2));
	WRITE_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x0c, *((uint32 *)&entry + 3));
	WRITE_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x10, *((uint32 *)&entry + 4));
	WRITE_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x14, *((uint32 *)&entry + 5));
	
	return SUCCESS;
}

int32 rtl8672_getAsicNetInterface(uint32 idx, rtl8672_tblAsicDrv_intfParam_t *intfp)
{
	rtl8672_tblAsic_intfTable_t entry;
	
	if (idx >= (MAC_TX_INTFS + SAR_INTFS + EXT_INTFS) || intfp == NULL)
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	*((uint32 *)&entry + 0) = READ_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x00);
	*((uint32 *)&entry + 1) = READ_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x04);
	*((uint32 *)&entry + 2) = READ_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x08);
	*((uint32 *)&entry + 3) = READ_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x0c);
	*((uint32 *)&entry + 4) = READ_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x10);
	*((uint32 *)&entry + 5) = READ_MEM32(IFTABLE_BASE + (idx * IFTABLE_WIDTH) + 0x14);

	intfp->ATMPORT		= entry.ATMPORT;
	intfp->TRLREN		= entry.TRLREN;
	intfp->CLP		= entry.CLP;
	intfp->PTI		= entry.PTI;
	intfp->TRLR		= entry.TRLR;
	intfp->GMAC[0]		= (entry.GMAC47_32 & 0xff00) >> 8;
	intfp->GMAC[1]		= (entry.GMAC47_32 & 0x00ff) >> 0;
	intfp->GMAC[2]		= (entry.GMAC31_16 & 0xff00) >> 8;
	intfp->GMAC[3]		= (entry.GMAC31_16 & 0x00ff) >> 0;
	intfp->GMAC[4]		= (entry.GMAC15_0 & 0xff00) >> 8;
	intfp->GMAC[5]		= (entry.GMAC15_0 & 0x00ff) >> 0;
	intfp->L2Encap		= entry.L2Encap;
	intfp->LanFCS		= entry.LanFCS;
	intfp->CompPPP		= entry.CompPPP;
	intfp->IfType		= entry.IfType;
	intfp->SARhdr		= entry.SARhdr;
	intfp->RXshift		= entry.RXshift;
	intfp->GIP		= entry.GIP;
	intfp->AcceptTagged	= entry.AcceptTagged;
	intfp->AcceptUntagged	= entry.AcceptUntagged;
	intfp->PortPriorityID	= entry.PPID;
	intfp->SrcPortFilter	= entry.SrcPortFilter;
	intfp->L2BridgeEnable	= entry.L2BRIDGE;
	intfp->Dot1QRemr	= entry.Dot1QREMR;
	intfp->LogicalID	= entry.LogicalID;
	intfp->PortVlanID	= entry.PVID;
	intfp->MTU		= entry.MTU;
	intfp->VlanIDRemr	= entry.VIDR;
	
	return SUCCESS;
}

int32 rtl8672_setAsicProtocolBasedVLAN(uint32 idx, rtl8672_tblAsicDrv_pnpParam_t *pnpp)
{
	rtl8672_tblAsic_pnpTable_t entry;
	
	if (idx >= ALE_PNPTBL_SIZE || pnpp == NULL)
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.ETH = pnpp->EtherType;
	entry.SPM = pnpp->SrcPortMember;
	entry.PDS = pnpp->PriorityDisable;
	entry.PID = pnpp->PriorityID;
	entry.VID = pnpp->VlanID;

	WRITE_MEM32(ALECR_PNPCHK0 + (idx * ALE_PNPTBL_WIDTH), (entry.ETH << PNPCHK_ETH_OFFSET) | (entry.SPM << PNPCHK_SPM_OFFSET));
	WRITE_MEM32(ALECR_PNPACT0 + (idx * ALE_PNPTBL_WIDTH), (entry.PDS << PNPACT_PDS_OFFSET) | (entry.PID << PNPACT_PID_OFFSET) | (entry.VID << PNPACT_VID_OFFSET));
	
	return SUCCESS;
}

int32 rtl8672_getAsicProtocolBasedVLAN(uint32 idx, rtl8672_tblAsicDrv_pnpParam_t *pnpp)
{
	rtl8672_tblAsic_pnpTable_t entry;
	
	if (idx >= ALE_PNPTBL_SIZE || pnpp == NULL)
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.ETH = (READ_MEM32(ALECR_PNPCHK0 + (idx * ALE_PNPTBL_WIDTH)) & PNPCHK_ETH_MASK) >> PNPCHK_ETH_OFFSET;
	entry.SPM = (READ_MEM32(ALECR_PNPCHK0 + (idx * ALE_PNPTBL_WIDTH)) & PNPCHK_SPM_MASK) >> PNPCHK_SPM_OFFSET;
	entry.PDS = (READ_MEM32(ALECR_PNPACT0 + (idx * ALE_PNPTBL_WIDTH)) & PNPACT_PDS_MASK) >> PNPACT_PDS_OFFSET;
	entry.PID = (READ_MEM32(ALECR_PNPACT0 + (idx * ALE_PNPTBL_WIDTH)) & PNPACT_PID_MASK) >> PNPACT_PID_OFFSET;
	entry.VID = (READ_MEM32(ALECR_PNPACT0 + (idx * ALE_PNPTBL_WIDTH)) & PNPACT_VID_MASK) >> PNPACT_VID_OFFSET;

	pnpp->EtherType = entry.ETH;
	pnpp->SrcPortMember = entry.SPM;
	pnpp->PriorityDisable = entry.PDS;
	pnpp->PriorityID = entry.PID;
	pnpp->VlanID = entry.VID;
	
	return SUCCESS;
}

int32 rtl8672_setAsicL2Table(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l2Param_t *l2p)
{
	uint32 ale_l2table_idx;
	uint32 ale_l2table_way;
	uint32 ale_l2table_base;
	rtl8672_tblAsic_l2Table_t entry;
	
	if (!((READ_MEM32(ALECR_L2TEB) & L2TEB_EN_MASK) >> L2TEB_EN_OFFSET))
		return FAILED;
	
	rtl8672_getAsicL2HashIdxBits(&ale_l2table_idx);
	ale_l2table_idx = (0x1 << ale_l2table_idx);
	rtl8672_getAsicL2Way(&ale_l2table_way);
	rtl8672_getAsicL2BaseAddress(&ale_l2table_base);
	
	if (idx >= ale_l2table_idx || way >= ale_l2table_way || l2p == NULL)
		return FAILED;
	
	memset(&entry, 0,sizeof(entry));
	entry.MAC47_32 = (l2p->MAC[0] << 8) | (l2p->MAC[1] << 0);
	entry.MAC31_16 = (l2p->MAC[2] << 8) | (l2p->MAC[3] << 0);
	entry.MAC15_0 = (l2p->MAC[4] << 8) | (l2p->MAC[5] << 0);
	entry.VID = l2p->VlanID;
	entry.PORT = l2p->Port;
	entry.AGE = l2p->Age;
	entry.PIDR = l2p->PriorityIDRemr;
	entry.OTAGIF = l2p->OTagIf;
	entry.SADROP = l2p->SADrop;
	entry.Dot1PREMR = l2p->Dot1PRemr;
	entry.VALID = l2p->Valid;
	entry.APMAC47_32 = (l2p->APMAC[0] << 8) | (l2p->APMAC[1] << 0);
	entry.APMAC31_16 = (l2p->APMAC[2] << 8) | (l2p->APMAC[3] << 0);
	entry.APMAC15_0 = (l2p->APMAC[4] << 8) | (l2p->APMAC[5] << 0);
#if 0
rtlglue_printf("rtl8672_setAsicL2Table(): ale_l2table_base = 0x%08x \n", ale_l2table_base);
rtlglue_printf("rtl8672_setAsicL2Table(): ale_l2table_way = %d \n", ale_l2table_way);
rtlglue_printf("rtl8672_setAsicL2Table(): idx = %d, way = %d \n", idx, way);
#endif

/*	[FIXME] please implement the indirect access to write
	WRITE_MEM32(ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x0, *((uint32 *)&entry + 0));
	WRITE_MEM32(ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x4, *((uint32 *)&entry + 1));
	WRITE_MEM32(ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x8, *((uint32 *)&entry + 2));
	WRITE_MEM32(ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0xc, *((uint32 *)&entry + 3));
*/
	REG32( ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x0 ) = big_endian( *((uint32 *)&entry + 0) );
	REG32( ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x4 ) = big_endian( *((uint32 *)&entry + 1) );
	REG32( ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x8 ) = big_endian( *((uint32 *)&entry + 2) );
	REG32( ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0xc ) = big_endian( *((uint32 *)&entry + 3) );
	
	return SUCCESS;
}

int32 rtl8672_getAsicL2Table(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l2Param_t *l2p)
{
	uint32 ale_l2table_idx;
	uint32 ale_l2table_way;
	uint32 ale_l2table_base;
	rtl8672_tblAsic_l2Table_t entry;
	
	if (!((READ_MEM32(ALECR_L2TEB) & L2TEB_EN_MASK) >> L2TEB_EN_OFFSET))
		return FAILED;
	
	rtl8672_getAsicL2HashIdxBits(&ale_l2table_idx);
	ale_l2table_idx = (0x1 << ale_l2table_idx);
	rtl8672_getAsicL2Way(&ale_l2table_way);
	rtl8672_getAsicL2BaseAddress(&ale_l2table_base);
	if(ale_l2table_base==0) return FAILED;
	
	if (idx >= ale_l2table_idx || way >= ale_l2table_way || l2p == NULL)
		return FAILED;

	memset(&entry, 0,sizeof(entry));
/*	[FIXME] please implement the indirect access to read
	*((uint32 *)&entry + 0) = READ_MEM32(ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x0);
	*((uint32 *)&entry + 1) = READ_MEM32(ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x4);
	*((uint32 *)&entry + 2) = READ_MEM32(ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x8);
	*((uint32 *)&entry + 3) = READ_MEM32(ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0xc);
*/
	*((uint32 *)&entry + 0) = big_endian( REG32( ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x0 ) );
	*((uint32 *)&entry + 1) = big_endian( REG32( ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x4 ) );
	*((uint32 *)&entry + 2) = big_endian( REG32( ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0x8 ) );
	*((uint32 *)&entry + 3) = big_endian( REG32( ale_l2table_base + ((idx * ale_l2table_way + way) * ALE_L2TABLE_WIDTH) + 0xc ) );

	l2p->MAC[0] = (entry.MAC47_32 & 0xff00) >> 8;
	l2p->MAC[1] = (entry.MAC47_32 & 0x00ff) >> 0;
	l2p->MAC[2] = (entry.MAC31_16 & 0xff00) >> 8;
	l2p->MAC[3] = (entry.MAC31_16 & 0x00ff) >> 0;
	l2p->MAC[4] = (entry.MAC15_0 & 0xff00) >> 8;
	l2p->MAC[5] = (entry.MAC15_0 & 0x00ff) >> 0;
	l2p->VlanID = entry.VID;
	l2p->Port = entry.PORT;
	l2p->Age = entry.AGE;
	l2p->PriorityIDRemr = entry.PIDR;
	l2p->OTagIf = entry.OTAGIF;
	l2p->SADrop = entry.SADROP;
	l2p->Dot1PRemr = entry.Dot1PREMR;
	l2p->Valid = entry.VALID;
	l2p->APMAC[0] = (entry.APMAC47_32 & 0xff00) >> 8;
	l2p->APMAC[1] = (entry.APMAC47_32 & 0x00ff) >> 0;
	l2p->APMAC[2] = (entry.APMAC31_16 & 0xff00) >> 8;
	l2p->APMAC[3] = (entry.APMAC31_16 & 0x00ff) >> 0;
	l2p->APMAC[4] = (entry.APMAC15_0 & 0xff00) >> 8;
	l2p->APMAC[5] = (entry.APMAC15_0 & 0x00ff) >> 0;
	
	return SUCCESS;
}

int32 rtl8672_setAsicL2Table_Sram(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l2Param_t *l2p)
{
	rtl8672_tblAsic_l2Table_t entry;

	if (idx >= ALE_L2TABLE_SRAM_IDX || way >= ALE_L2TABLE_SRAM_WAY || l2p == NULL)
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.MAC47_32 = (l2p->MAC[0] << 8) | (l2p->MAC[1] << 0);
	entry.MAC31_16 = (l2p->MAC[2] << 8) | (l2p->MAC[3] << 0);
	entry.MAC15_0 = (l2p->MAC[4] << 8) | (l2p->MAC[5] << 0);
	entry.VID = l2p->VlanID;
	entry.PORT = l2p->Port;
	entry.AGE = l2p->Age;
	entry.PIDR = l2p->PriorityIDRemr;
	entry.OTAGIF = l2p->OTagIf;
	entry.SADROP = l2p->SADrop;
	entry.Dot1PREMR = l2p->Dot1PRemr;
	entry.VALID = l2p->Valid;
	entry.APMAC47_32 = (l2p->APMAC[0] << 8) | (l2p->APMAC[1] << 0);
	entry.APMAC31_16 = (l2p->APMAC[2] << 8) | (l2p->APMAC[3] << 0);
	entry.APMAC15_0 = (l2p->APMAC[4] << 8) | (l2p->APMAC[5] << 0);
	WRITE_MEM32(ALECR_DEBUG_W0 , *((uint32 *)&entry + 0));
	WRITE_MEM32(ALECR_DEBUG_W1 , *((uint32 *)&entry + 1));
	WRITE_MEM32(ALECR_DEBUG_W2 , *((uint32 *)&entry + 2));
	WRITE_MEM32(ALECR_DEBUG_W3 , *((uint32 *)&entry + 3));
	WRITE_MEM32(ALECR_ICR , ALECR_ICR_CMD_WRITE|ALECR_ICR_TYPE_SRAM|ALECR_ICR_TABLE_LAYER2|(idx * ALE_L2TABLE_SRAM_WAY + way) );
	
	while(READ_MEM32(ALECR_ISR)!=0)
	{
		rtlglue_printf("indirect access running...\n");
	}
	
	
	return SUCCESS;
}

int32 rtl8672_getAsicL2Table_Sram(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l2Param_t *l2p)
{
	rtl8672_tblAsic_l2Table_t entry;

	if (idx >= ALE_L2TABLE_SRAM_IDX || way >= ALE_L2TABLE_SRAM_WAY || l2p == NULL)
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	WRITE_MEM32(ALECR_ICR , ALECR_ICR_CMD_READ|ALECR_ICR_TYPE_SRAM|ALECR_ICR_TABLE_LAYER2|(idx * ALE_L2TABLE_SRAM_WAY + way) );

	while(READ_MEM32(ALECR_ISR)!=0)
	{
		rtlglue_printf("indirect access running...\n");
	}	
	
	*((uint32 *)&entry + 0) = READ_MEM32(ALECR_DEBUG_W0);
	*((uint32 *)&entry + 1) = READ_MEM32(ALECR_DEBUG_W1);
	*((uint32 *)&entry + 2) = READ_MEM32(ALECR_DEBUG_W2);
	*((uint32 *)&entry + 3) = READ_MEM32(ALECR_DEBUG_W3);

	l2p->MAC[0] = (entry.MAC47_32 & 0xff00) >> 8;
	l2p->MAC[1] = (entry.MAC47_32 & 0x00ff) >> 0;
	l2p->MAC[2] = (entry.MAC31_16 & 0xff00) >> 8;
	l2p->MAC[3] = (entry.MAC31_16 & 0x00ff) >> 0;
	l2p->MAC[4] = (entry.MAC15_0 & 0xff00) >> 8;
	l2p->MAC[5] = (entry.MAC15_0 & 0x00ff) >> 0;
	l2p->VlanID = entry.VID;
	l2p->Port = entry.PORT;
	l2p->Age = entry.AGE;
	l2p->PriorityIDRemr = entry.PIDR;
	l2p->OTagIf = entry.OTAGIF;
	l2p->SADrop = entry.SADROP;
	l2p->Dot1PRemr = entry.Dot1PREMR;
	l2p->Valid = entry.VALID;
	l2p->APMAC[0] = (entry.APMAC47_32 & 0xff00) >> 8;
	l2p->APMAC[1] = (entry.APMAC47_32 & 0x00ff) >> 0;
	l2p->APMAC[2] = (entry.APMAC31_16 & 0xff00) >> 8;
	l2p->APMAC[3] = (entry.APMAC31_16 & 0x00ff) >> 0;
	l2p->APMAC[4] = (entry.APMAC15_0 & 0xff00) >> 8;
	l2p->APMAC[5] = (entry.APMAC15_0 & 0x00ff) >> 0;
	
	return SUCCESS;
}

int32 rtl8672_setAsicL4Table(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l4Param_t *l4p)
{
	uint32 ale_l4table_idx;
	uint32 ale_l4table_way;
	uint32 ale_l4table_base;
	rtl8672_tblAsic_l4Table_t entry;
	
	if (!((READ_MEM32(ALECR_L4TEB) & L4TEB_EN_MASK) >> L4TEB_EN_OFFSET))
		return FAILED;
	
	rtl8672_getAsicL4HashIdxBits(&ale_l4table_idx);
	ale_l4table_idx = (0x1 << ale_l4table_idx);
	rtl8672_getAsicL4Way(&ale_l4table_way);
	rtl8672_getAsicL4BaseAddress(&ale_l4table_base);
	
	if (idx >= ale_l4table_idx || way >= ale_l4table_way || l4p == NULL)
		return FAILED;
	
	memset(&entry, 0,sizeof(entry));
	entry.PID		= l4p->PriorityID;
	entry.VALID		= l4p->Valid;
	entry.DstPortIdx	= l4p->DstPortIdx;
	entry.TOS		= l4p->Tos;
	entry.TOSREMR	= l4p->TosRemr;
	entry.AGE		= l4p->Age;
	entry.PROTO		= l4p->Proto;
	entry.SIP		= l4p->SrcIP;
	entry.DIP		= l4p->DstIP;
	entry.SPORT		= l4p->SrcPort;
	entry.DPORT		= l4p->DstPort;
	entry.NSIP		= l4p->NewSrcIP;
	entry.NDIP		= l4p->NewDstIP;
	entry.NSPORT		= l4p->NewSrcPort;
	entry.NDPORT		= l4p->NewDstPort;
	entry.NHMAC47_32	= (l4p->NHMAC[0] << 8) | (l4p->NHMAC[1] << 0);
	entry.NHMAC31_16	= (l4p->NHMAC[2] << 8) | (l4p->NHMAC[3] << 0);
	entry.NHMAC15_0		= (l4p->NHMAC[4] << 8) | (l4p->NHMAC[5] << 0);
	entry.SID		= l4p->SessionID;
	entry.TTLDE		= l4p->TtlDe;
	entry.PRECEDREMR	= l4p->PrecedRemr;
	entry.Dot1PREMR		= l4p->Dot1PRemr;
	entry.OTAGIF		= l4p->OTagIf;
	entry.IPPPOEIF		= l4p->IPppoeIf;
	entry.OPPPOEIF		= l4p->OPppoeIf;
	entry.IVIDCHK		= l4p->IVlanIDChk;
	entry.IVID		= l4p->IVlanID;
	entry.OVID		= l4p->OVlanID;
	
/*	[FIXME] please implement the indirect access to write
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x00, *((uint32 *)&entry + 0));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x04, *((uint32 *)&entry + 1));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x08, *((uint32 *)&entry + 2));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x0c, *((uint32 *)&entry + 3));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x10, *((uint32 *)&entry + 4));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x14, *((uint32 *)&entry + 5));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x18, *((uint32 *)&entry + 6));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x1c, *((uint32 *)&entry + 7));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x20, *((uint32 *)&entry + 8));
	WRITE_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x24, *((uint32 *)&entry + 9));
*/

/*
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x00 ) = big_endian( *((uint32 *)&entry + 0) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x04 ) = big_endian( *((uint32 *)&entry + 1) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x08 ) = big_endian( *((uint32 *)&entry + 2) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x0c ) = big_endian( *((uint32 *)&entry + 3) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x10 ) = big_endian( *((uint32 *)&entry + 4) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x14 ) = big_endian( *((uint32 *)&entry + 5) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x18 ) = big_endian( *((uint32 *)&entry + 6) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x1c ) = big_endian( *((uint32 *)&entry + 7) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x20 ) = big_endian( *((uint32 *)&entry + 8) );
	REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x24 ) = big_endian( *((uint32 *)&entry + 9) );
*/
	WRITE_MEM32(ALECR_DEBUG_W0, *((uint32 *)&entry + 0));
	WRITE_MEM32(ALECR_DEBUG_W1, *((uint32 *)&entry + 1));
	WRITE_MEM32(ALECR_DEBUG_W2, *((uint32 *)&entry + 2));
	WRITE_MEM32(ALECR_DEBUG_W3, *((uint32 *)&entry + 3));
	WRITE_MEM32(ALECR_DEBUG_W4, *((uint32 *)&entry + 4));
	WRITE_MEM32(ALECR_DEBUG_W5, *((uint32 *)&entry + 5));
	WRITE_MEM32(ALECR_DEBUG_W6, *((uint32 *)&entry + 6));
	WRITE_MEM32(ALECR_DEBUG_W7, *((uint32 *)&entry + 7));
	WRITE_MEM32(ALECR_DEBUG_W8, *((uint32 *)&entry + 8));
	WRITE_MEM32(ALECR_DEBUG_W9, *((uint32 *)&entry + 9));
	WRITE_MEM32(ALECR_ICR , ALECR_ICR_CMD_WRITE|ALECR_ICR_TYPE_SDRAM|ALECR_ICR_TABLE_LAYER4|(idx * ALE_L4TABLE_SRAM_WAY + way) );
	
	while(READ_MEM32(ALECR_ISR)!=0)
	{
		rtlglue_printf("indirect access running...\n");
	}
	
	return SUCCESS;
}

int32 rtl8672_getAsicL4Table(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l4Param_t *l4p)
{
	uint32 ale_l4table_idx;
	uint32 ale_l4table_way;
	uint32 ale_l4table_base;
	rtl8672_tblAsic_l4Table_t entry;
	
	if (!((READ_MEM32(ALECR_L4TEB) & L4TEB_EN_MASK) >> L4TEB_EN_OFFSET))
		return FAILED;
	
	rtl8672_getAsicL4HashIdxBits(&ale_l4table_idx);
	ale_l4table_idx = (0x1 << ale_l4table_idx);
	rtl8672_getAsicL4Way(&ale_l4table_way);
	rtl8672_getAsicL4BaseAddress(&ale_l4table_base);
	if(ale_l4table_base==0) return FAILED;	
	
	if (idx >= ale_l4table_idx || way >= ale_l4table_way || l4p == NULL)
		return FAILED;
	
	memset(&entry, 0,sizeof(entry));
/*
	*((uint32 *)&entry + 0) = READ_MEM32(ALECR_DEBUG_W0);
	*((uint32 *)&entry + 1) = READ_MEM32(ALECR_DEBUG_W1);
	*((uint32 *)&entry + 2) = READ_MEM32(ALECR_DEBUG_W2);
	*((uint32 *)&entry + 3) = READ_MEM32(ALECR_DEBUG_W3);
	*((uint32 *)&entry + 4) = READ_MEM32(ALECR_DEBUG_W4);
	*((uint32 *)&entry + 5) = READ_MEM32(ALECR_DEBUG_W5);
	*((uint32 *)&entry + 6) = READ_MEM32(ALECR_DEBUG_W6);
	*((uint32 *)&entry + 7) = READ_MEM32(ALECR_DEBUG_W7);
	*((uint32 *)&entry + 8) = READ_MEM32(ALECR_DEBUG_W8);
	*((uint32 *)&entry + 9) = READ_MEM32(ALECR_DEBUG_W9);
*/
/*	[FIXME] please implement the indirect access to read
	*((uint32 *)&entry + 0) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x00);
	*((uint32 *)&entry + 1) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x04);
	*((uint32 *)&entry + 2) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x08);
	*((uint32 *)&entry + 3) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x0c);
	*((uint32 *)&entry + 4) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x10);
	*((uint32 *)&entry + 5) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x14);
	*((uint32 *)&entry + 6) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x18);
	*((uint32 *)&entry + 7) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x1c);
	*((uint32 *)&entry + 8) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x20);
	*((uint32 *)&entry + 9) = READ_MEM32(ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x24);
*/
	*((uint32 *)&entry + 0) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x00 ) );
	*((uint32 *)&entry + 1) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x04 ) );
	*((uint32 *)&entry + 2) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x08 ) );
	*((uint32 *)&entry + 3) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x0c ) );
	*((uint32 *)&entry + 4) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x10 ) );
	*((uint32 *)&entry + 5) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x14 ) );
	*((uint32 *)&entry + 6) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x18 ) );
	*((uint32 *)&entry + 7) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x1c ) );
	*((uint32 *)&entry + 8) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x20 ) );
	*((uint32 *)&entry + 9) = big_endian( REG32( ale_l4table_base + ((idx * ale_l4table_way + way) * ALE_L4TABLE_WIDTH) + 0x24 ) );
	
	l4p->PriorityID		= entry.PID;
	l4p->Valid		= entry.VALID;
	l4p->DstPortIdx		= entry.DstPortIdx;
	l4p->Tos		= entry.TOS;
	l4p->TosRemr	= entry.TOSREMR;
	l4p->Age		= entry.AGE;
	l4p->Proto		= entry.PROTO;
	l4p->SrcIP		= entry.SIP;
	l4p->DstIP		= entry.DIP;
	l4p->SrcPort		= entry.SPORT;
	l4p->DstPort		= entry.DPORT;
	l4p->NewSrcIP		= entry.NSIP;
	l4p->NewDstIP		= entry.NDIP;
	l4p->NewSrcPort		= entry.NSPORT;
	l4p->NewDstPort		= entry.NDPORT;
	l4p->NHMAC[0]		= (entry.NHMAC47_32 & 0xff00) >> 8;
	l4p->NHMAC[1]		= (entry.NHMAC47_32 & 0x00ff) >> 0;
	l4p->NHMAC[2]		= (entry.NHMAC31_16 & 0xff00) >> 8;
	l4p->NHMAC[3]		= (entry.NHMAC31_16 & 0x00ff) >> 0;
	l4p->NHMAC[4]		= (entry.NHMAC15_0 & 0xff00) >> 8;
	l4p->NHMAC[5]		= (entry.NHMAC15_0 & 0x00ff) >> 0;
	l4p->SessionID		= entry.SID;
	l4p->TtlDe		= entry.TTLDE;
	l4p->PrecedRemr		= entry.PRECEDREMR;
	l4p->Dot1PRemr		= entry.Dot1PREMR;
	l4p->OTagIf		= entry.OTAGIF;
	l4p->IPppoeIf		= entry.IPPPOEIF;
	l4p->OPppoeIf		= entry.OPPPOEIF;
	l4p->IVlanIDChk		= entry.IVIDCHK;
	l4p->IVlanID		= entry.IVID;
	l4p->OVlanID		= entry.OVID;
	
	return SUCCESS;
}

int32 rtl8672_setAsicL4Table_Sram(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l4Param_t *l4p)
{
	rtl8672_tblAsic_l4Table_t entry;

	if (idx >= ALE_L4TABLE_SRAM_IDX || way >= ALE_L4TABLE_SRAM_WAY || l4p == NULL)
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.PID		= l4p->PriorityID;
	entry.VALID		= l4p->Valid;
	entry.DstPortIdx	= l4p->DstPortIdx;
	entry.TOS		= l4p->Tos;
	entry.TOSREMR	= l4p->TosRemr;
	entry.AGE		= l4p->Age;
	entry.PROTO		= l4p->Proto;
	entry.SIP		= l4p->SrcIP;
	entry.DIP		= l4p->DstIP;
	entry.SPORT		= l4p->SrcPort;
	entry.DPORT		= l4p->DstPort;
	entry.NSIP		= l4p->NewSrcIP;
	entry.NDIP		= l4p->NewDstIP;
	entry.NSPORT		= l4p->NewSrcPort;
	entry.NDPORT		= l4p->NewDstPort;
	entry.NHMAC47_32	= (l4p->NHMAC[0] << 8) | (l4p->NHMAC[1] << 0);
	entry.NHMAC31_16	= (l4p->NHMAC[2] << 8) | (l4p->NHMAC[3] << 0);
	entry.NHMAC15_0		= (l4p->NHMAC[4] << 8) | (l4p->NHMAC[5] << 0);
	entry.SID		= l4p->SessionID;
	entry.TTLDE		= l4p->TtlDe;
	entry.PRECEDREMR	= l4p->PrecedRemr;
	entry.Dot1PREMR		= l4p->Dot1PRemr;
	entry.OTAGIF		= l4p->OTagIf;
	entry.IPPPOEIF		= l4p->IPppoeIf;
	entry.OPPPOEIF		= l4p->OPppoeIf;
	entry.IVIDCHK		= l4p->IVlanIDChk;
	entry.IVID		= l4p->IVlanID;
	entry.OVID		= l4p->OVlanID;

	WRITE_MEM32(ALECR_DEBUG_W0, *((uint32 *)&entry + 0));
	WRITE_MEM32(ALECR_DEBUG_W1, *((uint32 *)&entry + 1));
	WRITE_MEM32(ALECR_DEBUG_W2, *((uint32 *)&entry + 2));
	WRITE_MEM32(ALECR_DEBUG_W3, *((uint32 *)&entry + 3));
	WRITE_MEM32(ALECR_DEBUG_W4, *((uint32 *)&entry + 4));
	WRITE_MEM32(ALECR_DEBUG_W5, *((uint32 *)&entry + 5));
	WRITE_MEM32(ALECR_DEBUG_W6, *((uint32 *)&entry + 6));
	WRITE_MEM32(ALECR_DEBUG_W7, *((uint32 *)&entry + 7));
	WRITE_MEM32(ALECR_DEBUG_W8, *((uint32 *)&entry + 8));
	WRITE_MEM32(ALECR_DEBUG_W9, *((uint32 *)&entry + 9));
	WRITE_MEM32(ALECR_ICR , ALECR_ICR_CMD_WRITE|ALECR_ICR_TYPE_SRAM|ALECR_ICR_TABLE_LAYER4|(idx * ALE_L4TABLE_SRAM_WAY + way) );
	
	while(READ_MEM32(ALECR_ISR)!=0)
	{
		rtlglue_printf("indirect access running...\n");
	}

	
	return SUCCESS;
}

int32 rtl8672_getAsicL4Table_Sram(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l4Param_t *l4p)
{
	rtl8672_tblAsic_l4Table_t entry;

	if (idx >= ALE_L4TABLE_SRAM_IDX || way >= ALE_L4TABLE_SRAM_WAY || l4p == NULL)
		return FAILED;

	WRITE_MEM32(ALECR_ICR , ALECR_ICR_CMD_READ|ALECR_ICR_TYPE_SRAM|ALECR_ICR_TABLE_LAYER4|(idx * ALE_L4TABLE_SRAM_WAY + way) );

	while(READ_MEM32(ALECR_ISR)!=0)
	{
		rtlglue_printf("indirect access running...[ALECR_ISR:%x]\n",READ_MEM32(ALECR_ISR));
	}	

	memset(&entry, 0,sizeof(entry));

	*((uint32 *)&entry + 0) = READ_MEM32(ALECR_DEBUG_W0);
	*((uint32 *)&entry + 1) = READ_MEM32(ALECR_DEBUG_W1);
	*((uint32 *)&entry + 2) = READ_MEM32(ALECR_DEBUG_W2);
	*((uint32 *)&entry + 3) = READ_MEM32(ALECR_DEBUG_W3);
	*((uint32 *)&entry + 4) = READ_MEM32(ALECR_DEBUG_W4);
	*((uint32 *)&entry + 5) = READ_MEM32(ALECR_DEBUG_W5);
	*((uint32 *)&entry + 6) = READ_MEM32(ALECR_DEBUG_W6);
	*((uint32 *)&entry + 7) = READ_MEM32(ALECR_DEBUG_W7);
	*((uint32 *)&entry + 8) = READ_MEM32(ALECR_DEBUG_W8);
	*((uint32 *)&entry + 9) = READ_MEM32(ALECR_DEBUG_W9);

	l4p->PriorityID		= entry.PID;
	l4p->Valid		= entry.VALID;
	l4p->DstPortIdx		= entry.DstPortIdx;
	l4p->Tos		= entry.TOS;
	l4p->TosRemr	= entry.TOSREMR;
	l4p->Age		= entry.AGE;
	l4p->Proto		= entry.PROTO;
	l4p->SrcIP		= entry.SIP;
	l4p->DstIP		= entry.DIP;
	l4p->SrcPort		= entry.SPORT;
	l4p->DstPort		= entry.DPORT;
	l4p->NewSrcIP		= entry.NSIP;
	l4p->NewDstIP		= entry.NDIP;
	l4p->NewSrcPort		= entry.NSPORT;
	l4p->NewDstPort		= entry.NDPORT;
	l4p->NHMAC[0]		= (entry.NHMAC47_32 & 0xff00) >> 8;
	l4p->NHMAC[1]		= (entry.NHMAC47_32 & 0x00ff) >> 0;
	l4p->NHMAC[2]		= (entry.NHMAC31_16 & 0xff00) >> 8;
	l4p->NHMAC[3]		= (entry.NHMAC31_16 & 0x00ff) >> 0;
	l4p->NHMAC[4]		= (entry.NHMAC15_0 & 0xff00) >> 8;
	l4p->NHMAC[5]		= (entry.NHMAC15_0 & 0x00ff) >> 0;
	l4p->SessionID		= entry.SID;
	l4p->TtlDe		= entry.TTLDE;
	l4p->PrecedRemr		= entry.PRECEDREMR;
	l4p->Dot1PRemr		= entry.Dot1PREMR;
	l4p->OTagIf		= entry.OTAGIF;
	l4p->IPppoeIf		= entry.IPPPOEIF;
	l4p->OPppoeIf		= entry.OPPPOEIF;
	l4p->IVlanIDChk		= entry.IVIDCHK;
	l4p->IVlanID		= entry.IVID;
	l4p->OVlanID		= entry.OVID;
	
	return SUCCESS;
}


int32 rtl8672_setAsicHSB(rtl8672_tblAsicDrv_hsbParam_t *hsbp)
{
	rtl8672_tblAsic_hsbTable_t hsbt;
	
	memset(&hsbt, 0,sizeof(hsbt));
	hsbt.FROMCPU		= hsbp->fromcpu;
	hsbt.HASMAC			= hsbp->hasmac;
	hsbt.TIF			= hsbp->tif;
	hsbt.PIF			= hsbp->pif;
	hsbt.SRCPORTIDX		= hsbp->srcportidx;
	hsbt.L3HWFWDIP		= hsbp->l3hwfwdip;
	hsbt.LINKID			= hsbp->linkid;
	hsbt.PID			= hsbp->pid;
	hsbt.CFI			= hsbp->cfi;
	hsbt.VID			= hsbp->vid;
	hsbt.RSVD_2			= hsbp->rsvd_2;
	hsbt.PPPCOMPABLE	= hsbp->pppcompable;
	hsbt.FRAMECTRL		= hsbp->framectrl;
	hsbt.ETHTP			= hsbp->ethtp;
	hsbt.DMAC47_32		= (hsbp->dmac[0] << 8) | (hsbp->dmac[1] << 0);
	hsbt.DMAC31_16		= (hsbp->dmac[2] << 8) | (hsbp->dmac[3] << 0);
	hsbt.DMAC15_0		= (hsbp->dmac[4] << 8) | (hsbp->dmac[5] << 0);
	hsbt.SMAC47_32		= (hsbp->smac[0] << 8) | (hsbp->smac[1] << 0);
	hsbt.SMAC31_16		= (hsbp->smac[2] << 8) | (hsbp->smac[3] << 0);
	hsbt.SMAC15_0		= (hsbp->smac[4] << 8) | (hsbp->smac[5] << 0);
	hsbt.MAC3_47_32		= (hsbp->mac3[0] << 8) | (hsbp->mac3[1] << 0);
	hsbt.MAC3_31_16		= (hsbp->mac3[2] << 8) | (hsbp->mac3[3] << 0);
	hsbt.MAC3_15_0		= (hsbp->mac3[4] << 8) | (hsbp->mac3[5] << 0);
	hsbt.SID			= hsbp->sid;
	hsbt.MAC4_47_32		= (hsbp->mac4[0] << 8) | (hsbp->mac4[1] << 0);
	hsbt.MAC4_31_16		= (hsbp->mac4[2] << 8) | (hsbp->mac4[3] << 0);
	hsbt.MAC4_15_0		= (hsbp->mac4[4] << 8) | (hsbp->mac4[5] << 0);
	hsbt.L3CS			= hsbp->l3cs;
	hsbt.L3LEN			= hsbp->l3len;
	hsbt.SIP			= hsbp->sip;
	hsbt.DIP			= hsbp->dip;
	hsbt.TOS			= hsbp->tos;
	hsbt.L3OFFSET		= hsbp->l3offset;
	hsbt.TTL			= hsbp->ttl;
	hsbt.L3TYPE			= hsbp->l3type;
	hsbt.IPFRAG			= hsbp->ipfrag;
	hsbt.L3CSOK			= hsbp->l3csok;
	hsbt.UDPNOCS		= hsbp->udpnocs;
	hsbt.L4CSOK			= hsbp->l4csok;
	hsbt.REASON			= hsbp->reason;
	hsbt.L4PROTO		= hsbp->l4proto;
	hsbt.TCPFLAG		= hsbp->tcpflag;
	hsbt.L4CS			= hsbp->l4cs;
	hsbt.SPORT			= hsbp->sport;
	hsbt.DPORT			= hsbp->dport;
	
	WRITE_MEM32(HSB_BASE + 0x00, *((uint32 *)&hsbt + 0));
	WRITE_MEM32(HSB_BASE + 0x04, *((uint32 *)&hsbt + 1));
	WRITE_MEM32(HSB_BASE + 0x08, *((uint32 *)&hsbt + 2));
	WRITE_MEM32(HSB_BASE + 0x0c, *((uint32 *)&hsbt + 3));
	WRITE_MEM32(HSB_BASE + 0x10, *((uint32 *)&hsbt + 4));
	WRITE_MEM32(HSB_BASE + 0x14, *((uint32 *)&hsbt + 5));
	WRITE_MEM32(HSB_BASE + 0x18, *((uint32 *)&hsbt + 6));
	WRITE_MEM32(HSB_BASE + 0x1c, *((uint32 *)&hsbt + 7));
	WRITE_MEM32(HSB_BASE + 0x20, *((uint32 *)&hsbt + 8));
	WRITE_MEM32(HSB_BASE + 0x24, *((uint32 *)&hsbt + 9));
	WRITE_MEM32(HSB_BASE + 0x28, *((uint32 *)&hsbt + 10));
	WRITE_MEM32(HSB_BASE + 0x2c, *((uint32 *)&hsbt + 11));
	WRITE_MEM32(HSB_BASE + 0x30, *((uint32 *)&hsbt + 12));
	WRITE_MEM32(HSB_BASE + 0x34, *((uint32 *)&hsbt + 13));
	WRITE_MEM32(HSB_BASE + 0x38, *((uint32 *)&hsbt + 14));
	
	return SUCCESS;
}

int32 rtl8672_getAsicHSB(rtl8672_tblAsicDrv_hsbParam_t *hsbp)
{
	rtl8672_tblAsic_hsbTable_t hsbt;
	
	memset(&hsbt, 0,sizeof(hsbt));
	*((uint32 *)&hsbt + 0) = READ_MEM32(HSB_BASE + 0x00);
	*((uint32 *)&hsbt + 1) = READ_MEM32(HSB_BASE + 0x04);
	*((uint32 *)&hsbt + 2) = READ_MEM32(HSB_BASE + 0x08);
	*((uint32 *)&hsbt + 3) = READ_MEM32(HSB_BASE + 0x0c);
	*((uint32 *)&hsbt + 4) = READ_MEM32(HSB_BASE + 0x10);
	*((uint32 *)&hsbt + 5) = READ_MEM32(HSB_BASE + 0x14);
	*((uint32 *)&hsbt + 6) = READ_MEM32(HSB_BASE + 0x18);
	*((uint32 *)&hsbt + 7) = READ_MEM32(HSB_BASE + 0x1c);
	*((uint32 *)&hsbt + 8) = READ_MEM32(HSB_BASE + 0x20);
	*((uint32 *)&hsbt + 9) = READ_MEM32(HSB_BASE + 0x24);
	*((uint32 *)&hsbt + 10) = READ_MEM32(HSB_BASE + 0x28);
	*((uint32 *)&hsbt + 11) = READ_MEM32(HSB_BASE + 0x2c);
	*((uint32 *)&hsbt + 12) = READ_MEM32(HSB_BASE + 0x30);
	*((uint32 *)&hsbt + 13) = READ_MEM32(HSB_BASE + 0x34);
	*((uint32 *)&hsbt + 14) = READ_MEM32(HSB_BASE + 0x38);
	
	hsbp->fromcpu		= hsbt.FROMCPU;
	hsbp->hasmac		= hsbt.HASMAC;
	hsbp->tif			= hsbt.TIF;
	hsbp->pif			= hsbt.PIF;
	hsbp->srcportidx	= hsbt.SRCPORTIDX;
	hsbp->l3hwfwdip		= hsbt.L3HWFWDIP;
	hsbp->linkid		= hsbt.LINKID;
	hsbp->pid			= hsbt.PID;
	hsbp->cfi			= hsbt.CFI;
	hsbp->vid			= hsbt.VID;
	hsbp->rsvd_2		= hsbt.RSVD_2;
	hsbp->pppcompable	= hsbt.PPPCOMPABLE;
	hsbp->framectrl		= hsbt.FRAMECTRL;
	hsbp->ethtp			= hsbt.ETHTP;
	hsbp->dmac[0]		= (hsbt.DMAC47_32 & 0xff00) >> 8;
	hsbp->dmac[1]		= (hsbt.DMAC47_32 & 0x00ff) >> 0;
	hsbp->dmac[2]		= (hsbt.DMAC31_16 & 0xff00) >> 8;
	hsbp->dmac[3]		= (hsbt.DMAC31_16 & 0x00ff) >> 0;
	hsbp->dmac[4]		= (hsbt.DMAC15_0 & 0xff00) >> 8;
	hsbp->dmac[5]		= (hsbt.DMAC15_0 & 0x00ff) >> 0;
	hsbp->smac[0]		= (hsbt.SMAC47_32 & 0xff00) >> 8;
	hsbp->smac[1]		= (hsbt.SMAC47_32 & 0x00ff) >> 0;
	hsbp->smac[2]		= (hsbt.SMAC31_16 & 0xff00) >> 8;
	hsbp->smac[3]		= (hsbt.SMAC31_16 & 0x00ff) >> 0;
	hsbp->smac[4]		= (hsbt.SMAC15_0 & 0xff00) >> 8;
	hsbp->smac[5]		= (hsbt.SMAC15_0 & 0x00ff) >> 0;
	hsbp->mac3[0]		= (hsbt.MAC3_47_32 & 0xff00) >> 8;
	hsbp->mac3[1]		= (hsbt.MAC3_47_32 & 0x00ff) >> 0;
	hsbp->mac3[2]		= (hsbt.MAC3_31_16 & 0xff00) >> 8;
	hsbp->mac3[3]		= (hsbt.MAC3_31_16 & 0x00ff) >> 0;
	hsbp->mac3[4]		= (hsbt.MAC3_15_0 & 0xff00) >> 8;
	hsbp->mac3[5]		= (hsbt.MAC3_15_0 & 0x00ff) >> 0;
	hsbp->sid 			= hsbt.SID;
	hsbp->mac4[0]		= (hsbt.MAC4_47_32 & 0xff00) >> 8;
	hsbp->mac4[1] 		= (hsbt.MAC4_47_32 & 0x00ff) >> 0;
	hsbp->mac4[2]		= (hsbt.MAC4_31_16 & 0xff00) >> 8;
	hsbp->mac4[3]		= (hsbt.MAC4_31_16 & 0x00ff) >> 0;
	hsbp->mac4[4]		= (hsbt.MAC4_15_0 & 0xff00) >> 8;
	hsbp->mac4[5]		= (hsbt.MAC4_15_0 & 0x00ff) >> 0;
	hsbp->l3cs 			= hsbt.L3CS;
	hsbp->l3len			= hsbt.L3LEN;
	hsbp->sip			= hsbt.SIP;
	hsbp->dip			= hsbt.DIP;
	hsbp->tos			= hsbt.TOS;
	hsbp->l3offset		= hsbt.L3OFFSET;
	hsbp->ttl			= hsbt.TTL;
	hsbp->l3type		= hsbt.L3TYPE;
	hsbp->ipfrag		= hsbt.IPFRAG;
	hsbp->l3csok		= hsbt.L3CSOK;
	hsbp->udpnocs		= hsbt.UDPNOCS;
	hsbp->l4csok		= hsbt.L4CSOK;
	hsbp->reason		= hsbt.REASON;
	hsbp->l4proto		= hsbt.L4PROTO;
	hsbp->tcpflag		= hsbt.TCPFLAG;
	hsbp->l4cs			= hsbt.L4CS;
	hsbp->sport			= hsbt.SPORT;
	hsbp->dport			= hsbt.DPORT;
	
	return SUCCESS;
}

int32 rtl8672_setAsicHSA(rtl8672_tblAsicDrv_hsaParam_t *hsap)
{
	rtl8672_tblAsic_hsaTable_t hsat;

	memset(&hsat, 0,sizeof(hsat));
	hsat.OUTIFTYPE 		= hsap->outiftype;
	hsat.OUTL2ENCAP 	= hsap->outl2encap;
	hsat.OUTLANFCS 		= hsap->outlanfcs;
	hsat.PPPPROTOLEN 	= hsap->pppprotolen;
	hsat.OUTSARHDR 		= hsap->outsarhdr;
	hsat.DROPPACKET		= hsap->droppacket;
	hsat.PIF			= hsap->pif;
	hsat.L3CHANGE		= hsap->l3change;
	hsat.L4CHANGE		= hsap->l4change;
	hsat.FROMCPU		= hsap->fromcpu;
	hsat.TOCPU			= hsap->tocpu;
	hsat.INIFTYPE		= hsap->iniftype;
	hsat.TIF			= hsap->tif;
	hsat.LINKID			= hsap->linkid;
	hsat.DSTPORTIDX		= hsap->dstportidx;
	hsat.SRCPORTIDX		= hsap->srcportidx;
	hsat.ETHTP			= hsap->ethtp;
	hsat.FRAMECTRL		= hsap->framectrl;
	hsat.DMAC47_32		= (hsap->dmac[0] << 8) | (hsap->dmac[1] << 0);
	hsat.DMAC31_16		= (hsap->dmac[2] << 8) | (hsap->dmac[3] << 0);
	hsat.DMAC15_0		= (hsap->dmac[4] << 8) | (hsap->dmac[5] << 0);
	hsat.SMAC47_32		= (hsap->smac[0] << 8) | (hsap->smac[1] << 0);
	hsat.SMAC31_16		= (hsap->smac[2] << 8) | (hsap->smac[3] << 0);
	hsat.SMAC15_0		= (hsap->smac[4] << 8) | (hsap->smac[5] << 0);
	hsat.MAC3_47_32		= (hsap->mac3[0] << 8) | (hsap->mac3[1] << 0);
	hsat.MAC3_31_16		= (hsap->mac3[2] << 8) | (hsap->mac3[3] << 0);
	hsat.MAC3_15_0		= (hsap->mac3[4] << 8) | (hsap->mac3[5] << 0);
	hsat.SID			= hsap->sid;
	hsat.MAC4_47_32		= (hsap->mac4[0] << 8) | (hsap->mac4[1] << 0);
	hsat.MAC4_31_16		= (hsap->mac4[2] << 8) | (hsap->mac4[3] << 0);
	hsat.MAC4_15_0		= (hsap->mac4[4] << 8) | (hsap->mac4[5] << 0);
	hsat.PID			= hsap->pid;
	hsat.CFI			= hsap->cfi;
	hsat.VID			= hsap->vid;
	hsat.VIDREMARK	= hsap->vidremark;
	hsat.RSVD_1			= hsap->rsvd_1;
	hsat.L3TYPE			= hsap->l3type;
	hsat.TOSREMR		= hsap->tosremr;
	hsat.TOS			= hsap->tos;
	hsat.TTL			= hsap->ttl;
	hsat.INL3OFFSET		= hsap->inl3offset;
	hsat.INL3LEN		= hsap->inl3len;
	hsat.L3CS			= hsap->l3cs;
	hsat.SIP			= hsap->sip;
	hsat.DIP			= hsap->dip;
	hsat.SPORT			= hsap->sport;
	hsat.DPORT			= hsap->dport;
	hsat.L4CS			= hsap->l4cs;
	hsat.L4PROTO		= hsap->l4proto;
	hsat.RSVD_2			= hsap->rsvd_2;
	hsat.REASON			= hsap->reason;
	
	WRITE_MEM32(HSA_BASE + 0x00, *((uint32 *)&hsat + 0));
	WRITE_MEM32(HSA_BASE + 0x04, *((uint32 *)&hsat + 1));
	WRITE_MEM32(HSA_BASE + 0x08, *((uint32 *)&hsat + 2));
	WRITE_MEM32(HSA_BASE + 0x0c, *((uint32 *)&hsat + 3));
	WRITE_MEM32(HSA_BASE + 0x10, *((uint32 *)&hsat + 4));
	WRITE_MEM32(HSA_BASE + 0x14, *((uint32 *)&hsat + 5));
	WRITE_MEM32(HSA_BASE + 0x18, *((uint32 *)&hsat + 6));
	WRITE_MEM32(HSA_BASE + 0x1c, *((uint32 *)&hsat + 7));
	WRITE_MEM32(HSA_BASE + 0x20, *((uint32 *)&hsat + 8));
	WRITE_MEM32(HSA_BASE + 0x24, *((uint32 *)&hsat + 9));
	WRITE_MEM32(HSA_BASE + 0x28, *((uint32 *)&hsat + 10));
	WRITE_MEM32(HSA_BASE + 0x2c, *((uint32 *)&hsat + 11));
	WRITE_MEM32(HSA_BASE + 0x30, *((uint32 *)&hsat + 12));
	WRITE_MEM32(HSA_BASE + 0x34, *((uint32 *)&hsat + 13));
	WRITE_MEM32(HSA_BASE + 0x38, *((uint32 *)&hsat + 14));
	
	return SUCCESS;
}

int32 rtl8672_getAsicHSA(rtl8672_tblAsicDrv_hsaParam_t *hsap)
{
	rtl8672_tblAsic_hsaTable_t hsat;

	memset(&hsat, 0,sizeof(hsat));
	*((uint32 *)&hsat + 0) = READ_MEM32(HSA_BASE + 0x00);
	*((uint32 *)&hsat + 1) = READ_MEM32(HSA_BASE + 0x04);
	*((uint32 *)&hsat + 2) = READ_MEM32(HSA_BASE + 0x08);
	*((uint32 *)&hsat + 3) = READ_MEM32(HSA_BASE + 0x0c);
	*((uint32 *)&hsat + 4) = READ_MEM32(HSA_BASE + 0x10);
	*((uint32 *)&hsat + 5) = READ_MEM32(HSA_BASE + 0x14);
	*((uint32 *)&hsat + 6) = READ_MEM32(HSA_BASE + 0x18);
	*((uint32 *)&hsat + 7) = READ_MEM32(HSA_BASE + 0x1c);
	*((uint32 *)&hsat + 8) = READ_MEM32(HSA_BASE + 0x20);
	*((uint32 *)&hsat + 9) = READ_MEM32(HSA_BASE + 0x24);
	*((uint32 *)&hsat + 10) = READ_MEM32(HSA_BASE + 0x28);
	*((uint32 *)&hsat + 11) = READ_MEM32(HSA_BASE + 0x2c);
	*((uint32 *)&hsat + 12) = READ_MEM32(HSA_BASE + 0x30);
	*((uint32 *)&hsat + 13) = READ_MEM32(HSA_BASE + 0x34);
	*((uint32 *)&hsat + 14) = READ_MEM32(HSA_BASE + 0x38);
	
	hsap->outiftype		= hsat.OUTIFTYPE;
	hsap->outl2encap 	= hsat.OUTL2ENCAP;
	hsap->outlanfcs		= hsat.OUTLANFCS;
	hsap->pppprotolen	= hsat.PPPPROTOLEN;
	hsap->outsarhdr 	= hsat.OUTSARHDR;
	hsap->droppacket	= hsat.DROPPACKET;
	hsap->pif			= hsat.PIF;
	hsap->l3change		= hsat.L3CHANGE;
	hsap->l4change		= hsat.L4CHANGE;
	hsap->fromcpu		= hsat.FROMCPU;
	hsap->tocpu			= hsat.TOCPU;
	hsap->iniftype		= hsat.INIFTYPE;
	hsap->tif			= hsat.TIF;
	hsap->linkid		= hsat.LINKID;
	hsap->dstportidx	= hsat.DSTPORTIDX;
	hsap->srcportidx	= hsat.SRCPORTIDX;
	hsap->ethtp			= hsat.ETHTP;
	hsap->framectrl		= hsat.FRAMECTRL;
	hsap->dmac[0]		= (hsat.DMAC47_32 & 0xff00) >> 8;
	hsap->dmac[1]		= (hsat.DMAC47_32 & 0x00ff) >> 0;
	hsap->dmac[2]		= (hsat.DMAC31_16 & 0xff00) >> 8;
	hsap->dmac[3]		= (hsat.DMAC31_16 & 0x00ff) >> 0;
	hsap->dmac[4]		= (hsat.DMAC15_0 & 0xff00) >> 8;
	hsap->dmac[5]		= (hsat.DMAC15_0 & 0x00ff) >> 0;
	hsap->smac[0]		= (hsat.SMAC47_32 & 0xff00) >> 8;
	hsap->smac[1]		= (hsat.SMAC47_32 & 0x00ff) >> 0;
	hsap->smac[2]		= (hsat.SMAC31_16 & 0xff00) >> 8;
	hsap->smac[3]		= (hsat.SMAC31_16 & 0x00ff) >> 0;
	hsap->smac[4]		= (hsat.SMAC15_0 & 0xff00) >> 8;
	hsap->smac[5]		= (hsat.SMAC15_0 & 0x00ff) >> 0;
	hsap->mac3[0]		= (hsat.MAC3_47_32 & 0xff00) >> 8;
	hsap->mac3[1]		= (hsat.MAC3_47_32 & 0x00ff) >> 0;
	hsap->mac3[2]		= (hsat.MAC3_31_16 & 0xff00) >> 8;
	hsap->mac3[3]		= (hsat.MAC3_31_16 & 0x00ff) >> 0;
	hsap->mac3[4]		= (hsat.MAC3_15_0 & 0xff00) >> 8;
	hsap->mac3[5]		= (hsat.MAC3_15_0 & 0x00ff) >> 0;
	hsap->sid			= hsat.SID;
	hsap->mac4[0]		= (hsat.MAC4_47_32 & 0xff00) >> 8;
	hsap->mac4[1]		= (hsat.MAC4_47_32 & 0x00ff) >> 0;
	hsap->mac4[2]		= (hsat.MAC4_31_16 & 0xff00) >> 8;
	hsap->mac4[3]		= (hsat.MAC4_31_16 & 0x00ff) >> 0;
	hsap->mac4[4]		= (hsat.MAC4_15_0 & 0xff00) >> 8;
	hsap->mac4[5]		= (hsat.MAC4_15_0 & 0x00ff) >> 0;
	hsap->pid			= hsat.PID;
	hsap->cfi			= hsat.CFI;
	hsap->vid			= hsat.VID;
	hsap->vidremark		= hsat.VIDREMARK;
	hsap->rsvd_1		= hsat.RSVD_1;
	hsap->l3type		= hsat.L3TYPE;
	hsap->tosremr		= hsat.TOSREMR;
	hsap->tos			= hsat.TOS;
	hsap->ttl			= hsat.TTL;
	hsap->inl3offset	= hsat.INL3OFFSET;
	hsap->inl3len		= hsat.INL3LEN;
	hsap->l3cs			= hsat.L3CS;
	hsap->sip			= hsat.SIP;
	hsap->dip			= hsat.DIP;
	hsap->sport			= hsat.SPORT;
	hsap->dport			= hsat.DPORT;
	hsap->l4cs			= hsat.L4CS;
	hsap->l4proto		= hsat.L4PROTO;
	hsap->rsvd_2		= hsat.RSVD_2;
	hsap->reason		= hsat.REASON;
	
	return SUCCESS;
}

int32 rtl8672_setAsicHSBP(rtl8672_tblAsic_hsbpTable_t *hsbp)
{
	rtl8672_tblAsic_hsbpTable_t thishsbp;
	memcpy(&thishsbp,hsbp,sizeof(rtl8672_tblAsic_hsbpTable_t));
	WRITE_MEM32(HSBP_BASE + 0x00, *((uint32 *)&thishsbp + 0));
	WRITE_MEM32(HSBP_BASE + 0x04, *((uint32 *)&thishsbp + 1));
	WRITE_MEM32(HSBP_BASE + 0x08, *((uint32 *)&thishsbp + 2));
	
	return SUCCESS;
}

int32 rtl8672_getAsicHSBP(rtl8672_tblAsic_hsbpTable_t *hsbp)
{
	memset(hsbp, 0,sizeof(rtl8672_tblAsic_hsbpTable_t));
	*((uint32 *)hsbp + 0) = READ_MEM32(HSBP_BASE + 0x00);
	*((uint32 *)hsbp + 1) = READ_MEM32(HSBP_BASE + 0x04);
	*((uint32 *)hsbp + 2) = READ_MEM32(HSBP_BASE + 0x08);
	
	return SUCCESS;
}

