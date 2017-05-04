/*
* Copyright (c) Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Source file of Test case for RTL8672 ALE
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icTest_ALE.c,v 1.4 2012/07/12 07:55:45 czyao Exp $
*/

#include "rtl_types.h"
#include "rtl_glue.h"
#include "cle_struct.h"
#include "types.h"
#include "rtl8672_asicregs.h"
#include "rtl8672_Proto.h"
#include "hsModel.h"
#include "icModel.h"
#include "icTest.h"
#include "modelRandom.h"
#include "virtualMac.h"
#include "icModel_Packet.h"
#include "icModel_ALE.h"
#include "icModel_ringController.h"
#include "rtl_utils.h"
#include "icTest_ALE.h"
#include "icTest_ringController.h"
#include "rtl8672_tblAsicDrv.h"
#include "rtl8672_tblDrv.h"
#include "modelTrace.h"
#include "icModel_pktParser.h"
#include "icTest_PktParser.h"
#include "icTest_PktTranslator.h"

/*****************************
 **   testModel functions   **
 *****************************/
void *ale_l2table_realbase = NULL;
void *ale_l4table_realbase = NULL;
int32 testALE_Init(void)
{
	uint32 ale_l2teb;
	uint32 ale_l2hib;
	uint32 ale_l2way;
	uint32 ale_l2table_idx;
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	uint32 ale_l4teb;
	uint32 ale_l4hib;
	uint32 ale_l4way;
	uint32 ale_l4table_idx;
#endif
	
	/* Alloc L2 Table */
	rtl8672_getAsicL2Ability(&ale_l2teb);
	if (ale_l2teb && (ale_l2table_realbase == NULL))
	{
		rtl8672_getAsicL2HashIdxBits(&ale_l2hib);
		rtl8672_getAsicL2Way(&ale_l2way);
		ale_l2table_idx = (0x1 << ale_l2hib);	/* hash index total size */
		ale_l2table_realbase = rtlglue_malloc( (ale_l2table_idx * ale_l2way * ALE_L2TABLE_WIDTH) + 3 );	/* +3: 4Bytes-Align */
		if (ale_l2table_realbase == NULL)
		{
			rtlglue_printf("%s():%d alloc memory failed.\n", __FUNCTION__, __LINE__);
			return FAILED;
		}
		memset(ale_l2table_realbase, 0, (ale_l2table_idx * ale_l2way * ALE_L2TABLE_WIDTH) + 3 );	/* Clear Memory */
		rtl8672_setAsicL2BaseAddress(((uint32)ale_l2table_realbase + 4) & 0xfffffffc);
	}

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	/* Alloc L4 Table */
	rtl8672_getAsicL4Ability(&ale_l4teb);
	if (ale_l4teb && (ale_l4table_realbase == NULL))
	{
		rtl8672_getAsicL4HashIdxBits(&ale_l4hib);
		rtl8672_getAsicL4Way(&ale_l4way);
		ale_l4table_idx = (0x1 << ale_l4hib);	/* hash index total size */
		ale_l4table_realbase = rtlglue_malloc( (ale_l4table_idx * ale_l4way * ALE_L4TABLE_WIDTH) + 3 );	/* +3: 4Bytes-Align */
		if (ale_l4table_realbase == NULL)
		{
			rtlglue_free(ale_l2table_realbase);	/* have to free memory of l2 table */
			ale_l2table_realbase = NULL;
			rtlglue_printf("%s():%d alloc memory failed.\n", __FUNCTION__, __LINE__);
			return FAILED;
		}
		memset(ale_l4table_realbase, 0, (ale_l4table_idx * ale_l4way * ALE_L4TABLE_WIDTH) + 3 );	/* Clear Memory */
		rtl8672_setAsicL4BaseAddress(((uint32)ale_l4table_realbase + 4) & 0xfffffffc);
	}
#endif

	return SUCCESS;
}

int32 testALE_Exit(void)
{
	if (ale_l2table_realbase) {
		rtlglue_free(ale_l2table_realbase);
		ale_l2table_realbase = NULL;
	}

	if (ale_l4table_realbase) {
		rtlglue_free(ale_l4table_realbase);
		ale_l4table_realbase = NULL;
	}
	
	rtl8672_setAsicL2Ability(0);	/* Disable L2 Table(SDRam) */
	rtl8672_setAsicL4Ability(0);	/* Disable L4 Table(SDRam) */
	
	return SUCCESS;
}

int32 testALE_InitPNP(void)
{

	rtl8672_tblAsicDrv_pnpParam_t pnpp;

	/* entry 0 */
	pnpp.EtherType = (0x0800);
	pnpp.SrcPortMember = 0xff;
	pnpp.VlanID = 111;
	pnpp.PriorityDisable = 0;
	ASSERT( SUCCESS == rtl8672_setAsicProtocolBasedVLAN(0,&pnpp));

	/* entry 1 */
	pnpp.EtherType = (0x0800);
	pnpp.SrcPortMember = 0x1f00;
	pnpp.VlanID = 222;
	pnpp.PriorityDisable = 0;
	ASSERT( SUCCESS == rtl8672_setAsicProtocolBasedVLAN(1,&pnpp));

	/* entry 2 */
	pnpp.EtherType = (0x0800);
	pnpp.SrcPortMember = 0xe000;
	pnpp.VlanID = 333;
	pnpp.PriorityDisable = 0;
	ASSERT( SUCCESS == rtl8672_setAsicProtocolBasedVLAN(2,&pnpp));
	
	/* entry 3*/
	pnpp.EtherType = (0x8864);
	pnpp.SrcPortMember = 0xff;
	pnpp.VlanID = 444;
	pnpp.PriorityDisable = 0;
	ASSERT( SUCCESS == rtl8672_setAsicProtocolBasedVLAN(3,&pnpp));

	/* entry 4 */
	pnpp.EtherType = (0x8864);
	pnpp.SrcPortMember = 0x1f00;
	pnpp.VlanID = 555;
	pnpp.PriorityDisable = 0;
	ASSERT( SUCCESS == rtl8672_setAsicProtocolBasedVLAN(4,&pnpp));

	/* entry 5 */
	pnpp.EtherType = (0x8864);
	pnpp.SrcPortMember = 0xe000;
	pnpp.VlanID = 666;
	pnpp.PriorityDisable = 0;
	ASSERT( SUCCESS == rtl8672_setAsicProtocolBasedVLAN(5,&pnpp));

	/* entry 6 */
	pnpp.EtherType = (0x8863);
	pnpp.SrcPortMember = 0xffff;
	pnpp.VlanID = 777;
	pnpp.PriorityDisable = 0;
	ASSERT( SUCCESS == rtl8672_setAsicProtocolBasedVLAN(6,&pnpp));

	/* entry 7 */
	pnpp.EtherType = (0x86dd);
	pnpp.SrcPortMember = 0xffff;
	pnpp.VlanID = 888;
	pnpp.PriorityDisable = 0;
	ASSERT( SUCCESS == rtl8672_setAsicProtocolBasedVLAN(7,&pnpp));
	
	return SUCCESS;
}

int32 testALE_Reset(void)
{
	/* Clear Interface Tables */
	MT_ALE_DEBUG(" ALE reset all tables! %s[%d]\n",__FUNCTION__,__LINE__);
	do {
		int32 retval;
		uint32 idx;
		rtl8672_tblAsicDrv_intfParam_t intfp;
		
		bzero(&intfp, sizeof(rtl8672_tblAsicDrv_intfParam_t));	/* pnp empty entry */
		for (idx = 0; idx < (MAC_TX_INTFS+SAR_INTFS+EXT_INTFS); idx++) {
			retval = rtl8672_setAsicNetInterface(idx,&intfp);
			ASSERT(retval == SUCCESS);
		}
	} while(0);
	
	/* Clear Port and Protocol based Table */
	do {
		int32 retval;
		uint32 idx;
		rtl8672_tblAsicDrv_pnpParam_t pnpp;
		
		bzero(&pnpp, sizeof(rtl8672_tblAsicDrv_pnpParam_t));	/* pnp empty entry */
		for (idx = 0; idx < ALE_PNPTBL_SIZE; idx++) {
			retval = rtl8672_setAsicProtocolBasedVLAN(idx,&pnpp);
			ASSERT(retval == SUCCESS);
		}
	} while(0);

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#ifdef SRAM_MAPPING_ENABLED
	/* Clear Layer2 Table (SRAM) */
	do {
		int32 retval;
		uint32 idx, way;
		rtl8672_tblAsicDrv_l2Param_t l2p;
		
		bzero(&l2p, sizeof(rtl8672_tblAsicDrv_l2Param_t));	/* l2 empty entry */
		for (idx = 0; idx < ALE_L2TABLE_SRAM_IDX; idx++) {
			for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++) {
				retval = rtl8672_setAsicL2Table_Sram(idx, way, &l2p);
				ASSERT(retval == SUCCESS);
			}
		}
	} while(0);
#endif
#endif

	/* Clear Layer2 Table (SDRAM */
	if (ale_l2table_realbase) {
		uint32 ale_l2hib;
		uint32 ale_l2way;
		uint32 ale_l2table_idx;

		rtl8672_getAsicL2HashIdxBits(&ale_l2hib);
		rtl8672_getAsicL2Way(&ale_l2way);
		ale_l2table_idx = (0x1 << ale_l2hib);	/* hash index total size */
		memset(ale_l2table_realbase, 0, (ale_l2table_idx * ale_l2way * ALE_L2TABLE_WIDTH) + 3 );	/* Clear Memory */
	}

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#ifdef SRAM_MAPPING_ENABLED
	/* Clear Layer4 Table (SRAM) */
	do {
		int32 retval;
		uint32 idx, way;
		rtl8672_tblAsicDrv_l4Param_t l4p;
		
		bzero(&l4p, sizeof(rtl8672_tblAsicDrv_l4Param_t));	/* l4 empty entry */
		for (idx = 0; idx < ALE_L4TABLE_SRAM_IDX; idx++) {
			for (way = 0; way < ALE_L4TABLE_SRAM_WAY; way++) {
				retval = rtl8672_setAsicL4Table_Sram(idx, way,&l4p);
				ASSERT(retval == SUCCESS);
			}
		}
	} while(0);
#endif
#endif

	/* Clear Layer4 Table (SDRAM) */
	if (ale_l4table_realbase) {
		uint32 ale_l4hib;
		uint32 ale_l4way;
		uint32 ale_l4table_idx;
		
		rtl8672_getAsicL4HashIdxBits(&ale_l4hib);
		rtl8672_getAsicL4Way(&ale_l4way);
		ale_l4table_idx = (0x1 << ale_l4hib);	/* hash index total size */
		memset(ale_l4table_realbase, 0, (ale_l4table_idx * ale_l4way * ALE_L4TABLE_WIDTH) + 3 );	/* Clear Memory */
	}
	
	return SUCCESS;
}

uint8 empty_mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8 l4_nhmac[6] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };


/* Notes: testALE_Example is just a example for reference, do not make sense. */
#if 0
int32 testALE_Example(uint32 caseNo)
{
	uint8 dmac[6] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	uint8 smac[6] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
	uint8 src_gmac[6] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	uint8 dst_gmac[6] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
	hsb_param_t hsb;
	hsa_param_t hsa, expect_hsa;
	rtl8672_tblAsicDrv_intfParam_t intfp;
	rtl8672_tblAsicDrv_pnpParam_t pnpp;
	rtl8672_tblAsicDrv_l2Param_t l2p;
	rtl8672_tblAsicDrv_l4Param_t l4p;
	
		
		/* Initialize HSB, HSA*/
		bzero(&hsb, sizeof(hsb_param_t));
		bzero(&hsa, sizeof(hsa_param_t));
		bzero(&expect_hsa, sizeof(hsa_param_t));
		
		/* Prepare HSB */
		hsb.srcportidx = 1;
		hsb.fromcpu = 0;
		hsb.hasmac = 1;
		MAC_COPY(hsb.dmac, dmac);
		MAC_COPY(hsb.smac, smac);
			
		hsb.tif = 0;
		hsb.pif = 0;
		hsb.linkid = 0;
		hsb.pid = 0;
		hsb.sid = 0;
		hsb.cfi = 0;
		hsb.vid = 0;
		hsb.framectrl = 0;
		hsb.ethtp = 0x0800;
		hsb.l3type = 1;
		hsb.l3hwfwdip = 1;
		hsb.l3offset = 24;
		hsb.l3len = 64;
		hsb.sip = 0xc0a80101;	/* 192.168.1.1 */
		hsb.dip = 0xc0a801fe;	/* 192.168.1.254 */
		hsb.sport = 0x1234;
		hsb.dport = 0x0050;
		hsb.l3csok = 1;
		hsb.l3cs = 0xffff;
		hsb.ipfrag = 0;
		hsb.tos = 0x3c;
		hsb.ttl = 20;
		hsb.l4proto = IPPROTO_TCP;
		hsb.l4csok = 1;
		hsb.l4cs = 0xffff;
		hsb.udpnocs = 0;
		hsb.tcpflag = 0x50;
		hsb.reason = 0;
		
		/* Expect HSA */
		expect_hsa.srcportidx = hsb.srcportidx;
		expect_hsa.dstportidx = 5;
		
		/* ALE Control Registers */
		rtl8672_setAsicL2Ability(TRUE); /* Layer2 Table Enable */
		rtl8672_setAsicL4Ability(TRUE); /* Layer4 Table Enable */
		
		/* icTest Initialize */
		rtl8672_setAsicL2HashIdxBits(7);
		rtl8672_setAsicL2Way(4);
		rtl8672_setAsicL4HashIdxBits(7);
		rtl8672_setAsicL4Way(4);
		testALE_Init();
		
		/* Interface Table (Source) */
		bzero(&intfp, sizeof(rtl8672_tblAsicDrv_intfParam_t));
		MAC_COPY(intfp.GMAC, src_gmac);
		intfp.IfType = IF_SAR;
		intfp.GIP = 0xaabbccdd;
		intfp.AcceptTagged = 1;
		intfp.AcceptUntagged = 1;
		intfp.PortPriorityID = 3;
		intfp.PortVlanID = 11;
		intfp.MTU = 1500;
		intfp.SrcPortFilter = 1;
		intfp.L2BridgeEnable = 1;
		intfp.Dot1QRemr = 0;
		intfp.VlanIDRemr = 12;
		intfp.LogicalID = expect_hsa.srcportidx;
		rtl8672_setAsicNetInterface(expect_hsa.srcportidx, &intfp);
		
		/* Interface Table (Destination) */
		bzero(&intfp, sizeof(rtl8672_tblAsicDrv_intfParam_t));
		MAC_COPY(intfp.GMAC, dst_gmac);
		intfp.IfType = IF_ETHERNET;
		intfp.GIP = 0xaabbccee;
		intfp.AcceptTagged = 1;
		intfp.AcceptUntagged = 1;
		intfp.PortPriorityID = 2;
		intfp.PortVlanID = 11;
		intfp.MTU = 1500;
		intfp.SrcPortFilter = 1;
		intfp.L2BridgeEnable = 1;
		intfp.Dot1QRemr = 0;
		intfp.VlanIDRemr = 12;
		intfp.LogicalID = expect_hsa.dstportidx;
		rtl8672_setAsicNetInterface(expect_hsa.dstportidx, &intfp);
		
		/* Port and Protocol base VLAN and Priority */
		bzero(&pnpp, sizeof(rtl8672_tblAsicDrv_pnpParam_t));
		pnpp.EtherType = 0x8864;
		pnpp.SrcPortMember = 0xffff;
		rtl8672_setAsicProtocolBasedVLAN(7, &pnpp);
		
		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC, smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = expect_hsa.srcportidx;
		l2p.VlanID = 11;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		//rtl8672_setAsicL2Table_Sram(rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID), 0, &l2p);
		rtlglue_printf("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);
		
		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC, dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = expect_hsa.dstportidx;
		l2p.VlanID = 11;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		//rtl8672_setAsicL2Table_Sram(rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID), 0, &l2p);
		rtlglue_printf("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);
		
		/* Layer4 Entry */
		bzero(&l4p, sizeof(rtl8672_tblAsic_l2Table_t));
		l4p.Proto = hsb.l4proto;
		l4p.SrcIP = hsb.sip;
		l4p.DstIP = hsb.dip;
		l4p.SrcPort = hsb.sport;
		l4p.DstPort = hsb.dport;
		l4p.DstPortIdx = expect_hsa.dstportidx;
		MAC_COPY(l4p.NHMAC, l4_nhmac);
		l4p.NewSrcIP = 0x12345678;
		l4p.NewDstIP = 0x9abcdef0;
		l4p.NewSrcPort = 0x1234;
		l4p.NewDstPort = 0x5678;
		l4p.IVlanIDChk = 0;
		l4p.IVlanID = 11;
		l4p.IPppoeIf = 0;
		l4p.SessionID = 0;
		l4p.OTagIf = 0;
		l4p.OVlanID = 11;
		l4p.OPppoeIf = 0;
		l4p.PrecedRemr = 0;
		l4p.TosRemr = 0;
		l4p.Tos = 0;
		l4p.TtlDe = 1;
		l4p.Valid = 1;
		//rtl8672_setAsicL4Table_Sram(rtl8672_L4Hash_Sram(l4p.Proto, l4p.SrcIP, l4p.DstIP, l4p.SrcPort, l4p.DstPort), 0, &l4p);
		rtl8672_addAsicL4Entry(&l4p, OPT_AUTO);
		
		dumpHSB(&hsb);
		rtl8672_ALE(&hsb, &hsa);
		dumpHSA(&hsa);

		/* Reset (Clear) */
		testALE_Reset();
	
		/* Release Memory */
		testALE_Exit();

	
	return SUCCESS;
}

/*
@func int32	| testALE_PNP	| Test case for ALE pnp table.
@parm uint32	| test case numberic code.
@rvalue SUCCESS	| Test case passes. 
@rvalue FAILED 	| Test case fails.
@comm 
We add pnp table specific entry and verify the Packet Processor ALE behavior by checking hsa pid,vid and reason fields.
@devnote
 */
int32 testALE_PNP(uint32 caseNo)
{
	hsb_param_t hsb;
	hsa_param_t hsa, expect_hsa;
	rtl8672_tblAsicDrv_pnpParam_t pnpp;
	struct rtl8672_tblAsicDrv_intfParam_s *src_iftbl = NULL, *dst_iftbl = NULL;
	int pnptblIdx = 0, i = 0, pid = 0;
	rtl8672_tblAsicDrv_l2Param_t l2p;	

	/* Initialize HSB, HSA, interface table*/
	bzero(&hsb, sizeof(hsb_param_t));
	bzero(&hsa, sizeof(hsa_param_t));
	bzero(&expect_hsa, sizeof(hsa_param_t));
	src_iftbl = rtlglue_malloc(sizeof(struct rtl8672_tblAsicDrv_intfParam_s));
	ASSERT(src_iftbl != NULL);
	dst_iftbl = rtlglue_malloc(sizeof(struct rtl8672_tblAsicDrv_intfParam_s));
	ASSERT(dst_iftbl != NULL);

	/* initialization */
	{
		/* ALE Control Registers */
		rtl8672_setAsicL2Ability(TRUE); /* Layer2 Table Enable */
		rtl8672_setAsicL4Ability(TRUE); /* Layer4 Table Enable */
		/* icTest Initialize */
		rtl8672_setAsicL2HashIdxBits(7);
		rtl8672_setAsicL2Way(4);
		rtl8672_setAsicL4HashIdxBits(7);
		rtl8672_setAsicL4Way(4);
		testALE_Init();
		init_ring();
	}

	/****************************************************
	**** Priority tagged packet , pnp table not matched for vid ****
	****************************************************/
	MT_ALE_DEBUG("Priority tagged packet , pnp table not matched for vid. [%s(%d)]\n",__FUNCTION__,__LINE__);
	testALE_Reset();
	_ale_reset_hsb(&hsb);
	/* priority tag */
	hsb.srcportidx = rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS);
	hsb.tif = 1;
	hsb.cfi = 0;
	hsb.vid = 0;
	hsb.l3len = 64;
	hsb.pid = rtlglue_random()%0x7;
	rtl8672_setAsicHSB(&hsb);
	/* write altering entry(not match packet) to pnp table */
	pnpp.EtherType= hsb.ethtp;
	pnpp.SrcPortMember = ((rtlglue_random()%0xffff) & ~(1<<hsb.srcportidx));
	pnpp.PriorityDisable = 0;
	pnpp.PriorityID = rtlglue_random()%0x7;
	pnpp.VlanID = 0x123;
	rtl8672_setAsicProtocolBasedVLAN(0, &pnpp);
	/* source port interface table setting */
	rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);
	ASSERT(src_iftbl != NULL);
	if(hsb.srcportidx < SAR_INTFS){
		src_iftbl->SARhdr = BRIDGED;
	}
	src_iftbl->PortVlanID= 0x345;
	src_iftbl->AcceptTagged = 1;
	src_iftbl->AcceptUntagged = 1;
	src_iftbl->L2BridgeEnable = 1;
	src_iftbl->SrcPortFilter = 0;
	src_iftbl->MTU = 1518;
	rtl8672_setAsicNetInterface(hsb.srcportidx, src_iftbl);
	/* Layer2 Entry (SMAC) */
	bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
	MAC_COPY(l2p.MAC,hsb.smac);
	MAC_COPY(l2p.APMAC, empty_mac);
	l2p.Port = hsb.srcportidx;
	l2p.VlanID = 0x345;
	l2p.OTagIf = 0;
	l2p.Dot1PRemr = 0;
	l2p.PriorityIDRemr = 0;
	l2p.SADrop = 0;
	l2p.Valid = 1;
	MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
	rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);
	/* Layer2 Entry (DMAC) */
	bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
	MAC_COPY(l2p.MAC,hsb.dmac);
	MAC_COPY(l2p.APMAC, empty_mac);
	if(hsb.srcportidx < SAR_INTFS){
		l2p.Port = (hsb.srcportidx + 1)%(MAC_TX_INTFS + EXT_INTFS);
	}else{
		l2p.Port = (hsb.srcportidx + 1)%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS);
	}
	l2p.VlanID = 0x345;
	l2p.OTagIf = 0;
	l2p.Dot1PRemr = 0;
	l2p.PriorityIDRemr = 0;
	l2p.SADrop = 0;
	l2p.Valid = 1;
	MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
	rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);	
	/* dest. port interface table setting */
	rtl8672_getAsicNetInterface(l2p.Port, dst_iftbl);
	ASSERT(dst_iftbl != NULL);
	if(l2p.Port < SAR_INTFS){
		dst_iftbl->SARhdr = BRIDGED;
	}
	dst_iftbl->PortVlanID= 0x345;
	dst_iftbl->AcceptTagged = 1;
	dst_iftbl->AcceptUntagged = 1;
	dst_iftbl->L2BridgeEnable = 1;
	dst_iftbl->SrcPortFilter = 0;
	dst_iftbl->MTU = 1518;
	rtl8672_setAsicNetInterface(l2p.Port, dst_iftbl);	
	/* run ALE engine */
	WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
	WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
	do{
		/* polling trigger bit */
	}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
	rtl8672_getAsicHSA(&hsa);
	/* check whether the result match our exception */
	rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);
	ASSERT(src_iftbl != NULL);
	ASSERT(hsa.reason == 0);
	ASSERT(hsa.pid == hsb.pid);
	ASSERT(hsa.vid == src_iftbl->PortVlanID);

	/*************************************************
	**** Priority tagged packet , pnp table matched for vid ****
	*************************************************/
	MT_ALE_DEBUG("\n\n**************************************************************************** \n");
	MT_ALE_DEBUG("Priority tagged packet , pnp table matched for vid. [%s(%d)]\n",__FUNCTION__,__LINE__);
	_ale_reset_hsb(&hsb);
	/* priority tag */
	hsb.srcportidx = rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS);
	hsb.tif = 1;
	hsb.cfi = 0;
	hsb.vid = 0;
	hsb.pid = rtlglue_random()%0x7;
	rtl8672_setAsicHSB(&hsb);
	for(pnptblIdx = 0; pnptblIdx < ALE_PNPTBL_SIZE; pnptblIdx++)
	{
		/* reset ALE table */
		testALE_Reset();
		/* write altering entry(match packet) to pnp table */
		pnpp.EtherType= hsb.ethtp;
		pnpp.SrcPortMember = ((rtlglue_random()%0xffff) | (1<<hsb.srcportidx));
		pnpp.PriorityDisable = 0;
		pid = rtlglue_random()%0x7;
		pnpp.PriorityID = pid;
		pnpp.VlanID = 0x123;
		rtl8672_setAsicProtocolBasedVLAN(pnptblIdx, &pnpp);
		/* write altering entry(not match packet) to pnp table */
		pnpp.EtherType= hsb.ethtp;
		pnpp.SrcPortMember = ((rtlglue_random()%0xffff) & ~(1<<hsb.srcportidx));
		pnpp.PriorityDisable = 0;
		do{
			pnpp.PriorityID = rtlglue_random()%0x7;
		}while(pnpp.PriorityID == pid);
		pnpp.VlanID = 0x234;
		i = (pnptblIdx == 0)?((rtlglue_random()%(ALE_PNPTBL_SIZE - 1)) + 1):(pnptblIdx - 1);
		rtl8672_setAsicProtocolBasedVLAN(i, &pnpp);
		/* source port interface table setting */
		rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);
		ASSERT(src_iftbl != NULL);
		if(hsb.srcportidx < SAR_INTFS){
			src_iftbl->SARhdr = BRIDGED;
		}		
		src_iftbl->PortVlanID= 0x345;
		src_iftbl->AcceptTagged = 1;
		src_iftbl->AcceptUntagged = 1;
		src_iftbl->L2BridgeEnable = 1;
		src_iftbl->SrcPortFilter = 0;	
		src_iftbl->MTU = 1518;
		rtl8672_setAsicNetInterface(hsb.srcportidx, src_iftbl);
		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb.smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb.srcportidx;
		l2p.VlanID = 0x123;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);
		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb.dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		if(hsb.srcportidx < SAR_INTFS){
			l2p.Port = (hsb.srcportidx + 1)%(MAC_TX_INTFS + EXT_INTFS);
		}else{
			l2p.Port = (hsb.srcportidx + 1)%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS);
		}
		l2p.VlanID = 0x123;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);			
		/* dest. port interface table setting */
		rtl8672_getAsicNetInterface(l2p.Port, dst_iftbl);
		ASSERT(dst_iftbl != NULL);
		if(l2p.Port < SAR_INTFS){
			dst_iftbl->SARhdr = BRIDGED;
		}
		dst_iftbl->PortVlanID= 0x345;
		dst_iftbl->AcceptTagged = 1;
		dst_iftbl->AcceptUntagged = 1;
		dst_iftbl->L2BridgeEnable = 1;
		dst_iftbl->SrcPortFilter = 0;
		dst_iftbl->MTU = 1518;
		rtl8672_setAsicNetInterface(l2p.Port, dst_iftbl);	
		/* run ALE engine */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(&hsa);
		/* check whether the result match our exception */
		rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);
		ASSERT(src_iftbl != NULL);
		ASSERT(hsa.reason == 0);
		ASSERT(hsa.pid == hsb.pid);
		ASSERT(hsa.vid == 0x123);
		MT_ALE_DEBUG("===========================================================\n");		
	}

	/************************************************
	**** Untagged packet , pnp table not matched for pid ****
	************************************************/
	MT_ALE_DEBUG("Untagged packet , pnp table not matched for pid or vid. [%s(%d)]\n",__FUNCTION__,__LINE__);
	testALE_Reset();
	_ale_reset_hsb(&hsb);
	/* Untagged */
	hsb.srcportidx = rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS);
	hsb.tif = 0;
	rtl8672_setAsicHSB(&hsb);
	/* write altering entry(not match packet) to pnp table */
	pnpp.EtherType= hsb.ethtp;
	pnpp.SrcPortMember = ((rtlglue_random()%0xffff) & ~(1<<hsb.srcportidx));
	pnpp.PriorityDisable = 0;
	pnpp.PriorityID = rtlglue_random()%0x7;
	pnpp.VlanID = 0x123;
	rtl8672_setAsicProtocolBasedVLAN(0, &pnpp);
	/* source port interface table setting */
	rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);
	ASSERT(src_iftbl != NULL);
	if(hsb.srcportidx < SAR_INTFS){
		src_iftbl->SARhdr = BRIDGED;
	}
	src_iftbl->PortVlanID= 0x345;
	src_iftbl->PortPriorityID = pnpp.PriorityID + 1;
	src_iftbl->AcceptTagged = 1;
	src_iftbl->AcceptUntagged = 1;
	src_iftbl->L2BridgeEnable = 1;
	src_iftbl->SrcPortFilter = 0;
	src_iftbl->MTU = 1518;
	rtl8672_setAsicNetInterface(hsb.srcportidx, src_iftbl);
	/* Layer2 Entry (SMAC) */
	bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
	MAC_COPY(l2p.MAC,hsb.smac);
	MAC_COPY(l2p.APMAC, empty_mac);
	l2p.Port = hsb.srcportidx;
	l2p.VlanID = 0x345;
	l2p.OTagIf = 0;
	l2p.Dot1PRemr = 0;
	l2p.PriorityIDRemr = 0;
	l2p.SADrop = 0;
	l2p.Valid = 1;
	MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
	rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);
	/* Layer2 Entry (DMAC) */
	bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
	MAC_COPY(l2p.MAC,hsb.dmac);
	MAC_COPY(l2p.APMAC, empty_mac);
	if(hsb.srcportidx < SAR_INTFS){
		l2p.Port = (hsb.srcportidx + 1)%(MAC_TX_INTFS + EXT_INTFS);
	}else{
		l2p.Port = (hsb.srcportidx + 1)%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS);
	}
	l2p.VlanID = 0x345;
	l2p.OTagIf = 0;
	l2p.Dot1PRemr = 0;
	l2p.PriorityIDRemr = 0;
	l2p.SADrop = 0;
	l2p.Valid = 1;
	MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
	rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);	
	/* dest. port interface table setting */
	rtl8672_getAsicNetInterface(l2p.Port, dst_iftbl);
	ASSERT(dst_iftbl != NULL);
	if(l2p.Port < SAR_INTFS){
		dst_iftbl->SARhdr = BRIDGED;
	}
	dst_iftbl->PortVlanID= 0x345;
	dst_iftbl->AcceptTagged = 1;
	dst_iftbl->AcceptUntagged = 1;
	dst_iftbl->L2BridgeEnable = 1;
	dst_iftbl->SrcPortFilter = 0;
	dst_iftbl->MTU = 1518;
	rtl8672_setAsicNetInterface(l2p.Port, dst_iftbl);
	/* run ALE engine */
	WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
	WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
	do{
		/* polling trigger bit */
	}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
	rtl8672_getAsicHSA(&hsa);
	/* check whether the result match our exception */
	rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);
	ASSERT(src_iftbl != NULL);
	ASSERT(hsa.reason == 0);
	ASSERT(hsa.pid == src_iftbl->PortPriorityID);
	ASSERT(hsa.vid == src_iftbl->PortVlanID);

	/******************************************
	**** Untagged packet, pnp table match for pid ****
	******************************************/
	MT_ALE_DEBUG("\n\n**************************************************************************** \n");
	MT_ALE_DEBUG("Untagged packet, pnp table match for pid & vid. [%s(%d)]\n",__FUNCTION__,__LINE__);
	_ale_reset_hsb(&hsb);
	/* priority tag */
	hsb.srcportidx = rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS);
	hsb.tif = 0;
	rtl8672_setAsicHSB(&hsb);
	for(pnptblIdx = 0; pnptblIdx < ALE_PNPTBL_SIZE; pnptblIdx++)
	{
		/* reset ALE table */
		testALE_Reset();
		/* write altering entry(match packet) to pnp table */
		pnpp.EtherType= hsb.ethtp;
		pnpp.SrcPortMember = ((rtlglue_random()%0xffff) | (1<<hsb.srcportidx));
		pnpp.PriorityDisable = 0;
		pid = rtlglue_random()%0x7;
		pnpp.PriorityID = pid;
		pnpp.VlanID = 0x123;
		rtl8672_setAsicProtocolBasedVLAN(pnptblIdx, &pnpp);
		/* write altering entry(match packet but priority disabled bit high) to pnp table */
		pnpp.EtherType= hsb.ethtp;
		pnpp.SrcPortMember = ((rtlglue_random()%0xffff) | (1<<hsb.srcportidx));
		pnpp.PriorityDisable = 1;
		do{
			pnpp.PriorityID = rtlglue_random()%0x7;
		}while(pnpp.PriorityID == pid);
		pnpp.VlanID = 0x123;
		i = (pnptblIdx == 0)?((rtlglue_random()%(ALE_PNPTBL_SIZE - 1)) + 1):(pnptblIdx - 1);
		rtl8672_setAsicProtocolBasedVLAN(i, &pnpp);
		/* source port interface table setting */
		rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);
		/* write altering entry(not match packet) to pnp table */
		pnpp.EtherType= hsb.ethtp;
		pnpp.SrcPortMember = ((rtlglue_random()%0xffff) & ~(1<<hsb.srcportidx));
		pnpp.PriorityDisable = 1;
		do{
			pnpp.PriorityID = rtlglue_random()%0x7;
		}while(pnpp.PriorityID == pid);
		pnpp.VlanID = 0x234;
		i = (pnptblIdx == 0)?((rtlglue_random()%(ALE_PNPTBL_SIZE - 1)) + 1):(pnptblIdx - 1);
		rtl8672_setAsicProtocolBasedVLAN(i, &pnpp);
		/* source port interface table setting */
		rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);		
		ASSERT(src_iftbl != NULL);
		if(hsb.srcportidx < SAR_INTFS){
			src_iftbl->SARhdr = BRIDGED;
		}		
		src_iftbl->PortVlanID= 0x345;
		src_iftbl->AcceptTagged = 1;
		src_iftbl->AcceptUntagged = 1;
		src_iftbl->L2BridgeEnable = 1;
		src_iftbl->SrcPortFilter = 0;	
		src_iftbl->MTU = 1518;
		rtl8672_setAsicNetInterface(hsb.srcportidx, src_iftbl);
		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb.smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb.srcportidx;
		l2p.VlanID = 0x123;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);
		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb.dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		if(hsb.srcportidx < SAR_INTFS){
			l2p.Port = (hsb.srcportidx + 1)%(MAC_TX_INTFS + EXT_INTFS);
		}else{
			l2p.Port = (hsb.srcportidx + 1)%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS);
		}
		l2p.VlanID = 0x123;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);			
		/* dest. port interface table setting */
		rtl8672_getAsicNetInterface(l2p.Port, dst_iftbl);
		ASSERT(dst_iftbl != NULL);
		if(l2p.Port < SAR_INTFS){
			dst_iftbl->SARhdr = BRIDGED;
		}
		dst_iftbl->PortVlanID= 0x345;
		dst_iftbl->AcceptTagged = 1;
		dst_iftbl->AcceptUntagged = 1;
		dst_iftbl->L2BridgeEnable = 1;
		dst_iftbl->SrcPortFilter = 0;
		dst_iftbl->MTU = 1518;
		rtl8672_setAsicNetInterface(l2p.Port, dst_iftbl);	
		/* run ALE engine */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(&hsa);
		/* check whether the result match our exception */
		rtl8672_getAsicNetInterface(hsb.srcportidx, src_iftbl);
		ASSERT(src_iftbl != NULL);
		ASSERT(hsa.reason == 0);
		ASSERT(hsa.pid == pid);
		ASSERT(hsa.vid == 0x123);
		MT_ALE_DEBUG("===========================================================\n");		
	}

	testALE_Reset();
	testALE_Exit();
	return SUCCESS;
}


int32 testALE_L2Bridge(uint32 caseNo)
{
	uint32 dmac_idx, smac_idx;
	rtl8672_tblAsicDrv_l2Param_t l2p,l2r;

	//memDump((void *)L2TABLE_BASE, ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY, "L2 Table 0");
	
	bzero(&l2p, sizeof(rtl8672_tblAsicDrv_l2Param_t));
	l2p.MAC[0] = 0xd0;
	l2p.MAC[1] = 0xd1;
	l2p.MAC[2] = 0xd2;
	l2p.MAC[3] = 0xd3;
	l2p.MAC[4] = 0xd4;
	l2p.MAC[5] = 0xd5;
	l2p.APMAC[0] = 0xd6;
	l2p.APMAC[1] = 0xd7;
	l2p.APMAC[2] = 0xd8;
	l2p.APMAC[3] = 0xd9;
	l2p.APMAC[4] = 0xda;
	l2p.APMAC[5] = 0xdb;
	dmac_idx = rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID);
	rtl8672_setAsicL2Table_Sram(dmac_idx, 0, &l2p);
	memDump((void *)L2TABLE_BASE + (dmac_idx * ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY), ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY, "L2 Table 1");

	rtl8672_getAsicL2Table_Sram(dmac_idx, 0, &l2r);
	memDump((void *)&l2r,sizeof(rtl8672_tblAsicDrv_l2Param_t),"L2 entry");

return SUCCESS;
	bzero(&l2p, sizeof(rtl8672_tblAsicDrv_l2Param_t));
	l2p.MAC[0] = 0x56;
	l2p.MAC[1] = 0x57;
	l2p.MAC[2] = 0x58;
	l2p.MAC[3] = 0x59;
	l2p.MAC[4] = 0x5a;
	l2p.MAC[5] = 0x5b;
	l2p.APMAC[0] = 0x50;
	l2p.APMAC[1] = 0x51;
	l2p.APMAC[2] = 0x52;
	l2p.APMAC[3] = 0x53;
	l2p.APMAC[4] = 0x54;
	l2p.APMAC[5] = 0x55;
	smac_idx = rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID);
	rtl8672_setAsicL2Table_Sram(smac_idx, 2, &l2p);

	memDump((void *)L2TABLE_BASE + (smac_idx * ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY), ALE_L2TABLE_WIDTH * ALE_L2TABLE_SRAM_WAY, "L2 Table 2");

	bzero(&l2p, sizeof(rtl8672_tblAsicDrv_l2Param_t));
	rtl8672_getAsicL2Table_Sram(dmac_idx, 0, &l2p);
	rtlglue_printf("mac0 = %02x \n", l2p.MAC[0]);
	rtlglue_printf("mac1 = %02x \n", l2p.MAC[1]);
	rtlglue_printf("mac2 = %02x \n", l2p.MAC[2]);
	rtlglue_printf("mac3 = %02x \n", l2p.MAC[3]);
	rtlglue_printf("mac4 = %02x \n", l2p.MAC[4]);
	rtlglue_printf("mac5 = %02x \n", l2p.MAC[5]);

	rtlglue_printf("apmac0 = %02x \n", l2p.APMAC[0]);
	rtlglue_printf("apmac1 = %02x \n", l2p.APMAC[1]);
	rtlglue_printf("apmac2 = %02x \n", l2p.APMAC[2]);
	rtlglue_printf("apmac3 = %02x \n", l2p.APMAC[3]);
	rtlglue_printf("apmac4 = %02x \n", l2p.APMAC[4]);
	rtlglue_printf("apmac5 = %02x \n", l2p.APMAC[5]);
	
	return SUCCESS;
}



int32 testALE_L4Flow(uint32 caseNo)
{
	uint32 hash_idx1 = 0;
	uint32 hash_idx2 = 0;
	rtl8672_tblAsicDrv_l4Param_t l4p;

	bzero(&l4p, sizeof(rtl8672_tblAsicDrv_l4Param_t));
	l4p.PriorityID = 7;
	l4p.Valid = 1;
	l4p.DstPortIdx = 1;
	l4p.Tos = 0x22;
	l4p.Proto = 0x55;
	l4p.SrcIP = 0x11223344;
	l4p.DstIP = 0x55667788;
	l4p.SrcPort = 0xaaaa;
	l4p.DstPort = 0xbbbb;
	hash_idx1 = rtl8672_L4Hash_Sram(l4p.Proto, l4p.SrcIP, l4p.DstIP, l4p.SrcPort, l4p.DstPort);
	memDump((void *)L4TABLE_BASE + (hash_idx1 * ALE_L4TABLE_WIDTH * ALE_L4TABLE_SRAM_WAY), ALE_L4TABLE_WIDTH * ALE_L4TABLE_SRAM_WAY, "L4 Table 1b");
	rtl8672_setAsicL4Table_Sram(hash_idx1, 0, &l4p);
	memDump((void *)L4TABLE_BASE + (hash_idx1 * ALE_L4TABLE_WIDTH * ALE_L4TABLE_SRAM_WAY), ALE_L4TABLE_WIDTH * ALE_L4TABLE_SRAM_WAY, "L4 Table 1a");

	bzero(&l4p, sizeof(rtl8672_tblAsicDrv_l4Param_t));
	l4p.PriorityID = 1;
	l4p.Valid = 1;
	l4p.DstPortIdx = 1;
	l4p.Tos = 0x11;
	l4p.Proto = 0x11;
	l4p.SrcIP = 0xaaaaaaaa;
	l4p.DstIP = 0xbbbbbbbb;
	l4p.SrcPort = 0xcccc;
	l4p.DstPort = 0xdddd;
	hash_idx2 = rtl8672_L4Hash_Sram(l4p.Proto, l4p.SrcIP, l4p.DstIP, l4p.SrcPort, l4p.DstPort);
	memDump((void *)L4TABLE_BASE + (hash_idx2 * ALE_L4TABLE_WIDTH * ALE_L4TABLE_SRAM_WAY), ALE_L4TABLE_WIDTH * ALE_L4TABLE_SRAM_WAY, "L4 Table 2b");
	rtl8672_setAsicL4Table_Sram(hash_idx2, 0, &l4p);
	memDump((void *)L4TABLE_BASE + (hash_idx2 * ALE_L4TABLE_WIDTH * ALE_L4TABLE_SRAM_WAY), ALE_L4TABLE_WIDTH * ALE_L4TABLE_SRAM_WAY, "L4 Table 2b");

	bzero(&l4p, sizeof(rtl8672_tblAsicDrv_l4Param_t));
	rtl8672_getAsicL4Table_Sram(hash_idx1, 0, &l4p);

	rtlglue_printf("testALE_L4Flow(): Proto = 0x%02x \n", l4p.Proto);
	rtlglue_printf("testALE_L4Flow(): SrcIP = 0x%08x \n", l4p.SrcIP);
	rtlglue_printf("testALE_L4Flow(): DstIP = 0x%08x \n", l4p.DstIP);
	rtlglue_printf("testALE_L4Flow(): SrcPort = 0x%04x \n", l4p.SrcPort);
	rtlglue_printf("testALE_L4Flow(): DstPort = 0x%04x \n", l4p.DstPort);
	
	return SUCCESS;
}


int32 testALE_L2Table(uint32 caseNo)
{
#if 1
	uint8 mac1[6] = {0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6 };
	uint8 mac2[6] = {0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6 };
	
	uint32 ale_l2hib = 13;
	uint32 ale_l2way = 8;
	uint32 ale_l4hib = 13;
	uint32 ale_l4way = 8;
	
	uint32 ale_l2table_idx;
	uint32 ale_l2table_base = 0;
	uint32 ale_l4table_idx;
	uint32 ale_l4table_base = 0;
	
	uint32 idx, way;
	
	rtl8672_tblAsicDrv_l2Param_t l2p;
	
	/* ALE Control Registers */
	rtl8672_setAsicL2Ability(TRUE);	/* Layer2 Table Enable */
	rtl8672_setAsicL4Ability(TRUE);	/* Layer4 Table Enable */
	
	/* icTest Initialize */
	rtl8672_setAsicL2HashIdxBits(ale_l2hib);
	rtl8672_setAsicL2Way(ale_l2way);
	rtl8672_setAsicL4HashIdxBits(ale_l4hib);
	rtl8672_setAsicL4Way(ale_l4way);
	testALE_Init();
	
	rtlglue_printf("ale_l2table_realbase = 0x%p \n", ale_l2table_realbase);
	rtlglue_printf("ale_l4table_realbase = 0x%p \n", ale_l4table_realbase);
	
	rtl8672_getAsicL2BaseAddress(&ale_l2table_base);
	rtl8672_getAsicL4BaseAddress(&ale_l4table_base);
	
	rtlglue_printf("ale_l2table_base = 0x%08x \n", ale_l2table_base);
	rtlglue_printf("ale_l4table_base = 0x%08x \n", ale_l4table_base);
	
	ale_l2table_idx = (0x1 << 7);
	ale_l4table_idx = (0x1 << 7);
	
	idx = 1;
	way = 3;
	MAC_COPY(l2p.MAC, mac1);
	l2p.VlanID = 0x123;
	l2p.Valid = 1;
	
	memDump((void *)(ale_l2table_base + (ALE_L2TABLE_WIDTH * idx * ale_l2way)), (ALE_L2TABLE_WIDTH * ale_l2way), "L2 Table Before");
	if (rtl8672_setAsicL2Table(idx, way, &l2p) == SUCCESS)
		rtlglue_printf("rtl8672_setAsicL2Table() = SUCCESS\n");
	else
		rtlglue_printf("rtl8672_setAsicL2Table() = FAILED\n");
	memDump((void *)(ale_l2table_base + (ALE_L2TABLE_WIDTH * idx * ale_l2way)), (ALE_L2TABLE_WIDTH * ale_l2way), "L2 Table After");
	
	idx = 1;
	way = 0;
	MAC_COPY(l2p.MAC, mac2);
	l2p.VlanID = 0x456;
	l2p.Valid = 0;
	
	memDump((void *)(ale_l2table_base + (ALE_L2TABLE_WIDTH * idx * ale_l2way)), (ALE_L2TABLE_WIDTH * ale_l2way), "L2 Table Before");
	if (rtl8672_setAsicL2Table(idx, way, &l2p) == SUCCESS)
		rtlglue_printf("rtl8672_setAsicL2Table() = SUCCESS\n");
	else
		rtlglue_printf("rtl8672_setAsicL2Table() = FAILED\n");
	memDump((void *)(ale_l2table_base + (ALE_L2TABLE_WIDTH * idx * ale_l2way)), (ALE_L2TABLE_WIDTH * ale_l2way), "L2 Table After");

	bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
	
	if (rtl8672_getAsicL2Table(1, 3, &l2p) == SUCCESS)
		rtlglue_printf("rtl8672_getAsicL2Table() = SUCCESS\n");
	else
		rtlglue_printf("rtl8672_getAsicL2Table() = FAILED\n");
	rtlglue_printf("l2p.MAC = %02x:%02x:%02x:%02x:%02x:%02x \n", l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5]);
	rtlglue_printf("l2p.vid = %03x \n", l2p.VlanID);
	
	testALE_Exit();
#endif
	return SUCCESS;
}


int32 testALE_L4Table(uint32 caseNo)
{
#if 0
	uint8 mac1[6] = {0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6 };
	uint8 mac2[6] = {0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6 };
	
	uint32 ale_l2hib = 7;
	uint32 ale_l2way = 2;
	uint32 ale_l4hib = 7;
	uint32 ale_l4way = 2;
	
	uint32 ale_l2table_idx;
	uint32 ale_l2table_base = 0;
	uint32 ale_l4table_idx;
	uint32 ale_l4table_base = 0;
	
	uint32 idx, way;
	
	rtl8672_tblAsicDrv_l4Param_t l4p;
	
	/* ALE Control Registers */
	rtl8672_setAsicL2Ability(TRUE);	/* Layer2 Table Enable */
	rtl8672_setAsicL4Ability(TRUE);	/* Layer4 Table Enable */
	
	/* icTest Initialize */
	rtl8672_setAsicL2HashIdxBits(ale_l2hib);
	rtl8672_setAsicL2Way(ale_l2way);
	rtl8672_setAsicL4HashIdxBits(ale_l4hib);
	rtl8672_setAsicL4Way(ale_l4way);
	testALE_Init();
	
	rtlglue_printf("ale_l2table_realbase = 0x%p \n", ale_l2table_realbase);
	rtlglue_printf("ale_l4table_realbase = 0x%p \n", ale_l4table_realbase);
	
	rtl8672_getAsicL2BaseAddress(&ale_l2table_base);
	rtl8672_getAsicL4BaseAddress(&ale_l4table_base);
	
	rtlglue_printf("ale_l2table_base = 0x%08x \n", ale_l2table_base);
	rtlglue_printf("ale_l4table_base = 0x%08x \n", ale_l4table_base);
	
	ale_l2table_idx = (0x1 << 7);
	ale_l4table_idx = (0x1 << 7);
	
	idx = 1;
	way = 1;
	MAC_COPY(l4p.NHMAC, mac1);
	l4p.Proto = IPPROTO_TCP;
	l4p.SrcIP = 0x11111111;
	l4p.DstIP = 0x22222222;
	l4p.SrcPort = 0x3333;
	l4p.DstPort = 0x4444;
	l4p.Valid = 1;
	
	memDump((void *)(ale_l4table_base + (ALE_L4TABLE_WIDTH * idx * ale_l4way)), (ALE_L4TABLE_WIDTH * ale_l4way), "L4 Table Before");
	if (rtl8672_setAsicL4Table(idx, way, &l4p) == SUCCESS)
		rtlglue_printf("rtl8672_setAsicL4Table() = SUCCESS\n");
	else
		rtlglue_printf("rtl8672_setAsicL4Table() = FAILED\n");
	memDump((void *)(ale_l4table_base + (ALE_L4TABLE_WIDTH * idx * ale_l4way)), (ALE_L4TABLE_WIDTH * ale_l4way), "L4 Table After");
	
	idx = 1;
	way = 0;
	MAC_COPY(l4p.NHMAC, mac2);
	l4p.Proto = IPPROTO_UDP;
	l4p.SrcIP = 0x55555555;
	l4p.DstIP = 0x66666666;
	l4p.SrcPort = 0x7777;
	l4p.DstPort = 0x8888;
	l4p.Valid = 1;
	
	memDump((void *)(ale_l4table_base + (ALE_L4TABLE_WIDTH * idx * ale_l4way)), (ALE_L4TABLE_WIDTH * ale_l4way), "L4 Table Before");
	if (rtl8672_setAsicL4Table(idx, way, &l4p) == SUCCESS)
		rtlglue_printf("rtl8672_setAsicL4Table() = SUCCESS\n");
	else
		rtlglue_printf("rtl8672_setAsicL4Table() = FAILED\n");
	memDump((void *)(ale_l4table_base + (ALE_L4TABLE_WIDTH * idx * ale_l4way)), (ALE_L4TABLE_WIDTH * ale_l4way), "L4 Table After");

	bzero(&l4p, sizeof(rtl8672_tblAsic_l2Table_t));
	
	if (rtl8672_getAsicL4Table(1, 1, &l4p) == SUCCESS)
		rtlglue_printf("rtl8672_getAsicL4Table() = SUCCESS\n");
	else
		rtlglue_printf("rtl8672_getAsicL4Table() = FAILED\n");
	rtlglue_printf("l4p.NHMAC = %02x:%02x:%02x:%02x:%02x:%02x \n", l4p.NHMAC[0], l4p.NHMAC[1], l4p.NHMAC[2], l4p.NHMAC[3], l4p.NHMAC[4], l4p.NHMAC[5]);
	rtlglue_printf("l4p.SrcIP = %08x \n", l4p.SrcIP);
	rtlglue_printf("l4p.DstIP = %08x \n", l4p.DstIP);
	rtlglue_printf("l4p.SrcPort = %04x \n", l4p.SrcPort);
	rtlglue_printf("l4p.DstPort = %04x \n", l4p.DstPort);
	
	testALE_Exit();
#endif
	return SUCCESS;
}


int32 testALE_Generic(uint32 caseNo)
{
	uint8 i[50];
	uint32 counter = 0;
	hsb_param_t hsb;
	uint8 srcportidx[4] = { 0, 8, 11 , 15 };
	uint8 reason[2] = { 0, 7 };
	uint8 linkid[2] = { 0, 7 };
	uint8 pid[4] = { 0, 2, 5, 7 };
	uint16 vid[4] = { 0, 8, 9, 4095 };
	uint16 ethtp[4] = { 0x0800, 0x86dd, 0x8863, 0x8864 };
	uint16 sid[2] = { 0x0000, 0xffff };
	uint8 l3offset[2] = { 48, 64 };
	uint16 l3len[2] = { 0, 48 };
	uint16 l3cs[2] = { 0x1234, 0 };
	uint32 sip[4] = { 0x00000000, 0xc0a80101, 0xc0a80101, 0xd2f1ee11 };
	uint32 dip[4] = { 0x00000000, 0xd2f1ee11, 0xc0a801fe, 0xc0a801fe };
	uint8 ttl[2] = { 1, 2 };
	uint8 tos[2] = { 0xaa, 0x54 };
	uint16 l4cs[2] = { 0x5678, 0 };
	uint8 l4proto[4] = { 0, 1, 6, 17 };
	uint8 tcpflag[4] = { 0x00, 0x00, 0xaa, 0xf8 };
	uint16 sport[4] = { 0, 1234, 80, 0 };
	uint16 dport[4] = { 0, 80, 1234, 0 };
	uint8 dmac[4][6] = {
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x01 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x01 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x01 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x01 } };
	uint8 smac[4][6] = {
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x02 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x02 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x02 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x02 } };
	uint8 mac3[4][6] = {
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x03 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x03 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x03 } };
	uint8 mac4[4][6] = {
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0xab, 0xcd, 0x04 } };
	
	bzero(&hsb, sizeof(hsb_param_t));
	for (i[0]=0; i[0]<3; i[0]++) {
		/* srcportidx */
		hsb.srcportidx = srcportidx[i[0]];
		
		for (i[1]=0; i[1]<2; i[1]++) {
			/* fromcpu */
			hsb.fromcpu = i[1];
			
			if (hsb.fromcpu == 1) continue;	/* for reduce case */
			
			for (i[2]=0; i[2]<2; i[2]++) {
				/* reason */
				hsb.reason = reason[i[2]];

				if (hsb.reason != 0) continue;	/* for reduce case */
				if (hsb.fromcpu == 1 && hsb.reason != 0) continue;
				
				for (i[3]=0; i[3]<2; i[3]++) {
					/* linkid */
					hsb.linkid = linkid[i[3]];
					
					if (hsb.linkid != 0 && (hsb.srcportidx < 13)) continue;	/* Non-Extension Port */
					
					for (i[4]=0; i[4]<(1<<6); i[4]++) {
						/* tif, pid, cfi, vid */
						hsb.tif = (i[4] & 0x20) >> 5;
						hsb.pid = pid[((i[4] & 0x18) >> 3)];
						hsb.cfi = (i[4] & 0x04) >> 2;
						hsb.vid = vid[((i[4] & 0x03) >> 0)];
						
						if ((hsb.tif == 0) && (hsb.pid != 0 || hsb.cfi != 0 || hsb.vid != 0)) continue;
						
						for (i[5]=0; i[5]<(1<<7); i[5]++) {
							/* hasmac: ethtp, pif, sid, framectrl */
							hsb.hasmac = (i[5] & 0x40) >> 6;
							hsb.ethtp = (hsb.hasmac == 1)? ethtp[((i[5] & 0x30) >> 4)] : 0;
							hsb.pif = (i[5] & 0x08) >> 3;
							hsb.sid = sid[((i[5] & 0x04) >> 2)];
							hsb.framectrl = ((i[5] & 0x03) >> 0) << 6;
							
							if (hsb.hasmac == 0 && ethtp != 0) continue;
							if (hsb.pif == 0 && hsb.sid != 0) continue;
							if (hsb.pif == 1 && (hsb.ethtp != 0x8863 && hsb.ethtp != 0x8864)) continue;
							
							for (i[6]=0; i[6]<1; i[6]++) {
								/* mac: dmac, smac, mac3, mac4 */
								hsb.dmac[0] = dmac[(hsb.framectrl >> 6)][0];
								hsb.dmac[1] = dmac[(hsb.framectrl >> 6)][1];
								hsb.dmac[2] = dmac[(hsb.framectrl >> 6)][2];
								hsb.dmac[3] = dmac[(hsb.framectrl >> 6)][3];
								hsb.dmac[4] = dmac[(hsb.framectrl >> 6)][4];
								hsb.dmac[5] = dmac[(hsb.framectrl >> 6)][5];
								hsb.smac[0] = smac[(hsb.framectrl >> 6)][0];
								hsb.smac[1] = smac[(hsb.framectrl >> 6)][1];
								hsb.smac[2] = smac[(hsb.framectrl >> 6)][2];
								hsb.smac[3] = smac[(hsb.framectrl >> 6)][3];
								hsb.smac[4] = smac[(hsb.framectrl >> 6)][4];
								hsb.smac[5] = smac[(hsb.framectrl >> 6)][5];
								hsb.mac3[0] = mac3[(hsb.framectrl >> 6)][0];
								hsb.mac3[1] = mac3[(hsb.framectrl >> 6)][1];
								hsb.mac3[2] = mac3[(hsb.framectrl >> 6)][2];
								hsb.mac3[3] = mac3[(hsb.framectrl >> 6)][3];
								hsb.mac3[4] = mac3[(hsb.framectrl >> 6)][4];
								hsb.mac3[5] = mac3[(hsb.framectrl >> 6)][5];
								hsb.mac4[0] = mac4[(hsb.framectrl >> 6)][0];
								hsb.mac4[1] = mac4[(hsb.framectrl >> 6)][1];
								hsb.mac4[2] = mac4[(hsb.framectrl >> 6)][2];
								hsb.mac4[3] = mac4[(hsb.framectrl >> 6)][3];
								hsb.mac4[4] = mac4[(hsb.framectrl >> 6)][4];
								hsb.mac4[5] = mac4[(hsb.framectrl >> 6)][5];
								
								for (i[7]=0; i[7]<2; i[7]++) {
									/* l3offset, l3len, l3cs */
									hsb.l3offset = l3offset[i[7]];
									hsb.l3len = l3len[i[7]];
									hsb.l3cs = l3cs[((i[7] & 0x2) >> 1)];
									hsb.l3csok = ((i[7] & 0x1) >> 0);
									
									for (i[8]=0; i[8]<2; i[8]++) {
										/* l3type */
										hsb.l3type = i[8];
										
										for (i[9]=0; i[9]<(1<<6); i[9]++) {
											/* l3hwfwdip, sip, dip, ttl, tos, ipfrag */
											if ((hsb.l3type == 0) && (i[9] != 0)) continue;
											
											hsb.l3hwfwdip = (i[9] & 0x20) >> 5;
											hsb.sip = sip[((i[9] & 0x18) >> 3)];
											hsb.dip = dip[((i[9] & 0x18) >> 3)];
											hsb.ttl = ttl[((i[9] & 0x04) >> 2)];
											hsb.tos = tos[((i[9] & 0x02) >> 1)];
											hsb.ipfrag = ((i[9] & 0x01) >> 0);
											
											if (hsb.l3hwfwdip == 0 && (i[9] != 0)) continue;	/* for reduce case */
											
											for (i[10]=0; i[10]<2; i[10]++) {
												/* l4cs, l4csok */
												hsb.l4cs = l4cs[((i[10] & 0x02) >> 1)];
												hsb.l4csok = ((i[10] & 0x01) >> 0);
												
												for (i[11]=0; i[11]<4; i[11]++) {
													/* l4proto */
													hsb.l4proto = l4proto[i[11]];
													
													for (i[12]=0; i[12]<(1<<4); i[12]++) {
														/* tcpflag, udpnocs, sport, dport */
														hsb.tcpflag = tcpflag[((i[12] & 0x0c)>> 2)];
														hsb.udpnocs = (hsb.tcpflag == 0x0 && hsb.l4proto == 17)? 1: 0;
														hsb.sport = sport[((i[12] & 0x03)>> 0)];
														hsb.sport = dport[((i[12] & 0x03)>> 0)];
														
														if (((i[12] & 0x03)>> 0) == 4) continue;
														if ((hsb.l4proto != 6) && (hsb.tcpflag != 0)) continue;
														if ((hsb.l4proto != 17) && (hsb.udpnocs != 0)) continue;
														if ((hsb.l4proto == 1) && (hsb.sport != 0 || hsb.dport !=0)) continue;
														if ((hsb.l4proto == 6 || hsb.l4proto == 17) && (hsb.sport == 0 && hsb.dport == 0)) continue;
														
														counter++;
													}	/* tcpflag, udpnocs, sport, dport */
												}	/* l4proto */
											}	/* l4cs, l4csok */
										}	/* l3hwfwdip, sip, dip, ttl, tos, ipfrag */
									}	/* l3type */
								}	/* l3offset, l3len */
							}	/* mac */
						}	/* hasmac: ethtp, pif, sid, framectrl */
					}	/* tif, pid, cfi, vid */
				}	/* linkid */
			}	/* reason */
		}	/* fromcpu */
	}	/* srcportidx */

	//counter++; //dumpHSB(&hsb);
	rtlglue_printf("counter = %u\n", counter);
	
	return SUCCESS;
}

/*
@func int32	| testALE_SMACCheck	| Test case for ALE SMAC checking behavior.
@parm uint32	| test case numberic code.
@rvalue SUCCESS	| Test case passes. 
@rvalue FAILED 	| Test case fails.
@comm 
This case verify the Packet Processor ALE source MAC check. We first set the interface 9~13 with Logical ID 9 ~13 and add layer 2 table entrys with
srcPortIdx 9~13. And then we pass hsb  to the Packet Processor with hsb SrcPortIdx 9 and specific hsb SMAC. Only the case layer 2 table entry 
PortIdx 9 (which index to interface table 9 with Logical ID 9) can pass the source MAC check.
If we set the interface table 9~13 with the same Logical ID(7). Then all cases can pass the source MAC check.
This cases mainly make sure the ALE source MAC check issure the interface table Logical ID the L2 table entry PortIdx index to instead of the PortIdx.
@devnote
 */
int32 testALE_SMACCheck(uint32 caseNo)
{
	int ncase = 0, dstportidx = 0, portIdx = 0;
	hsb_param_t *hsb = NULL;
	hsa_param_t *hsa = NULL;
	struct rtl8672_tblAsicDrv_intfParam_s *piftbl = NULL;
	rtl8672_tblAsicDrv_l2Param_t l2p;

	hsb = rtlglue_malloc(sizeof(hsb_param_t));
	ASSERT(hsb != NULL);
	hsa = rtlglue_malloc(sizeof(hsa_param_t));
	ASSERT(hsa != NULL);
	piftbl = rtlglue_malloc(sizeof(struct rtl8672_tblAsicDrv_intfParam_s));
	ASSERT(piftbl != NULL);


		/* initialization */
		ncase = 0;
		{
			init_ring();
			/* ALE Control Registers */
			rtl8672_setAsicL2Ability(TRUE); /* Layer2 Table Enable */
			rtl8672_setAsicL4Ability(TRUE); /* Layer4 Table Enable */
			/* icTest Initialize */
			rtl8672_setAsicL2HashIdxBits(7);
			rtl8672_setAsicL2Way(4);
			rtl8672_setAsicL4HashIdxBits(7);
			rtl8672_setAsicL4Way(4);
			testALE_Init();
		}

		MT_ALE_DEBUG("SMAC check -- Interface table idex 9~12 with the different logical ID. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		for(portIdx = 9;portIdx < 13; portIdx++){
			l2p.MAC[0] = 0x0;l2p.MAC[1] = 0x0;l2p.MAC[2] = 0x0;
			l2p.MAC[3] = 0x0;l2p.MAC[4] = 0x0;l2p.MAC[5] = portIdx;
			MAC_COPY(l2p.APMAC, empty_mac);
			l2p.Port = portIdx;
			l2p.VlanID = 11;
			l2p.OTagIf = 0;
			l2p.Dot1PRemr = 0;
			l2p.PriorityIDRemr = 0;
			l2p.SADrop = 0;
			l2p.Valid = 1;
			MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
			rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

			/* get & set interface table */
			rtl8672_getAsicNetInterface(portIdx, piftbl);
			ASSERT(piftbl != NULL);
			piftbl->AcceptTagged = 1;		/* accept_tag enable */
			piftbl->AcceptUntagged = 1;
			piftbl->L2BridgeEnable = 1;		/* L2 bridge disable */
			piftbl->MTU = 456;
			piftbl->SrcPortFilter = 0;		/* source port filter enable */
			piftbl->LogicalID = portIdx;			/* identical logical ID */
			rtl8672_setAsicNetInterface(portIdx, piftbl);
		}

		for(portIdx = 9; portIdx < 13; portIdx++){
			/* pkt incoming source port index */
			hsb->srcportidx = 9;
			hsb->smac[0] = 0x0;hsb->smac[1] = 0x0;hsb->smac[2] = 0x0;
			hsb->smac[3] = 0x0;hsb->smac[4] = 0x0;hsb->smac[5] = portIdx;
			hsb->tif = 1;
			hsb->vid = 11;
			hsb->l3len = 234;
			rtl8672_setAsicHSB(hsb);
			/* pkt outgoing dest. port index */
			dstportidx = hsb->srcportidx;
			rtl8672_getAsicNetInterface(dstportidx, piftbl);
			ASSERT(piftbl != NULL);
			piftbl->MTU = 456;
			rtl8672_setAsicNetInterface(dstportidx, piftbl);			
			
			/* Layer2 Entry (DMAC) */
			bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
			MAC_COPY(l2p.MAC,hsb->dmac);
			MAC_COPY(l2p.APMAC, empty_mac);
			l2p.Port = dstportidx;
			l2p.VlanID = hsb->vid;
			l2p.OTagIf = 0;
			l2p.Dot1PRemr = 0;
			l2p.PriorityIDRemr = 0;
			l2p.SADrop = 0;
			l2p.Valid = 1;
			MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
			rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

 			/* run ALE */
			WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
			WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
			do{
				/* polling trigger bit */
			}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
			rtl8672_getAsicHSA(hsa);
			ASSERT(hsa->droppacket == 0);
			if(portIdx == 9){
				ASSERT(hsa->reason == 0);
			}else{
				ASSERT(hsa->reason == 0xa);
			}
 		}

		MT_ALE_DEBUG("SMAC check -- Interface table idex 9~12 with the same logical ID. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		for(portIdx = 9;portIdx < 13; portIdx++){
			l2p.MAC[0] = 0x0;l2p.MAC[1] = 0x0;l2p.MAC[2] = 0x0;
			l2p.MAC[3] = 0x0;l2p.MAC[4] = 0x0;l2p.MAC[5] = portIdx;
			MAC_COPY(l2p.APMAC, empty_mac);
			l2p.Port = portIdx;
			l2p.VlanID = 11;
			l2p.OTagIf = 0;
			l2p.Dot1PRemr = 0;
			l2p.PriorityIDRemr = 0;
			l2p.SADrop = 0;
			l2p.Valid = 1;
			MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
			rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

			/* get & set interface table */
			rtl8672_getAsicNetInterface(portIdx, piftbl);
			ASSERT(piftbl != NULL);
			piftbl->AcceptTagged = 1;		/* accept_tag enable */
			piftbl->AcceptUntagged = 1;
			piftbl->L2BridgeEnable = 1;		/* L2 bridge disable */
			piftbl->MTU = 456;
			piftbl->SrcPortFilter = 0;		/* source port filter enable */
			piftbl->LogicalID = 7;			/* identical logical ID */
			rtl8672_setAsicNetInterface(portIdx, piftbl);
		}

		for(portIdx = 9; portIdx < 13; portIdx++){
			/* pkt incoming source port index */
			hsb->srcportidx = 9;
			hsb->smac[0] = 0x0;hsb->smac[1] = 0x0;hsb->smac[2] = 0x0;
			hsb->smac[3] = 0x0;hsb->smac[4] = 0x0;hsb->smac[5] = portIdx;
			hsb->tif = 1;
			hsb->vid = 11;
			hsb->l3len = 234;
			rtl8672_setAsicHSB(hsb);
			/* pkt outgoing dest. port index */
			dstportidx = hsb->srcportidx;
			rtl8672_getAsicNetInterface(dstportidx, piftbl);
			ASSERT(piftbl != NULL);
			piftbl->MTU = 456;
			rtl8672_setAsicNetInterface(dstportidx, piftbl);			
			
			/* Layer2 Entry (DMAC) */
			bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
			MAC_COPY(l2p.MAC,hsb->dmac);
			MAC_COPY(l2p.APMAC, empty_mac);
			l2p.Port = dstportidx;
			l2p.VlanID = hsb->vid;
			l2p.OTagIf = 0;
			l2p.Dot1PRemr = 0;
			l2p.PriorityIDRemr = 0;
			l2p.SADrop = 0;
			l2p.Valid = 1;
			MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
			rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

 			/* run ALE */
			WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
			WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
			do{
				/* polling trigger bit */
			}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
			rtl8672_getAsicHSA(hsa);

			MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
			ASSERT(hsa->droppacket == 0);
			ASSERT(hsa->reason == 0);
 		}

		/* Release Memory */
		testALE_Reset();
		testALE_Exit();



	rtlglue_free(hsb);
	hsb = NULL;
	rtlglue_free(hsa);
	hsa = NULL;
	rtlglue_free(piftbl);
	piftbl = NULL;

	return SUCCESS;
}

/*
@func int32	| testALE_ReasonPriority	| Test case for ALE with packets should trap to CPU or dropping.
@parm uint32	| test case numberic code.
@rvalue SUCCESS	| Test case passes. 
@rvalue FAILED 	| Test case fails.
@comm 
This case verify the Packet Processor ALE behavior for packets should be trap to CPU or dopping.
We design diversiform header format packets to see whether hsa reason field result from ALE match our expected value.
There is priority issue with the hsa reasons. So we check the ALE behavior case by case.
@devnote
 */
int32 testALE_ReasonPriority(uint32 caseNo)
{
	int ncase = 0, dstportidx = 0;
	hsb_param_t *hsb = NULL;
	hsa_param_t *hsa = NULL;
	struct rtl8672_tblAsicDrv_intfParam_s *piftbl = NULL;
	rtl8672_tblAsicDrv_l2Param_t l2p;
	rtl8672_tblAsicDrv_l4Param_t l4p;
	int aledropcnt = 0;

	hsb = rtlglue_malloc(sizeof(hsb_param_t));
	ASSERT(hsb != NULL);
	hsa = rtlglue_malloc(sizeof(hsa_param_t));
	ASSERT(hsa != NULL);
	piftbl = rtlglue_malloc(sizeof(struct rtl8672_tblAsicDrv_intfParam_s));
	ASSERT(piftbl != NULL);


		/* initialization */
		ncase = 0;
		{
			init_ring();
			/* ALE Control Registers */
			rtl8672_setAsicL2Ability(TRUE); /* Layer2 Table Enable */
			rtl8672_setAsicL4Ability(TRUE); /* Layer4 Table Enable */
			/* icTest Initialize */
			rtl8672_setAsicL2HashIdxBits(7);
			rtl8672_setAsicL2Way(4);
			rtl8672_setAsicL4HashIdxBits(7);
			rtl8672_setAsicL4Way(4);
			testALE_Init();
		}

		/* ==================== reason 1~6 & 8  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("HSB reason in 1~6 or packt with tag but not accepted. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->reason = (rtlglue_random()%6) + 1;	/* Parser hsb with reason in 1~6 */
		hsb->tif = 1;				/* packet with vlan tag */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 0;		/* accept_tag disable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<8) | 0x7e));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);
		
		MT_ALE_DEBUG("=========>HSB reason = %d HSA reason = %x, Trap to CPU!  [%s(%d)]\n\n",hsb->reason,hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == hsb->reason);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<8) | 0x7e));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSB reason = %d HSA reason = %x, Drop packet!  [%s(%d)]\n\n",hsb->reason,hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == hsb->reason);

		/* ==================== reason 1~6 & 8  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("HSB reason in 1~6 or untagged packt not accepted. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->reason = (rtlglue_random()%6) + 1;	/* Parser hsb with reason in 1~6 */
		hsb->tif = 0;			/* packet without vlan tag */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptUntagged = 0;		/* accept_untag disable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<8) | 0x7e));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == hsb->reason);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<8) | 0x7e));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == hsb->reason);

		/* ==================== reason 8 & 9  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("Tagged Packt not accepted or vid = 4095. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;				/* packet with vlan tag */
		hsb->vid = 4095;			/* pid of vlan tag = 4095 */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 0;		/* accept_tag disable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<8) | (0x1<<9)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>Hsa reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 8);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<8) | (0x1<<9)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>Hsa reason = %d, Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 8);

		/* ==================== reason 9 & 10  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("Tagged Packt with vid = 4095 and SMAC not found. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;				/* packet with vlan tag */
		hsb->vid = 4095;			/* pid of vlan tag = 4095 */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<9) | (0x1<<10)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 9);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<9) | (0x1<<10)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 9);	

		/* ==================== reason 10 & 11  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("SMAC not found and L2 Bridge disable. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;				/* packet with vlan tag */
		hsb->vid = 11;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 0;		/* L2 bridge disable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<10) | (0x1<<11)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 10);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<10) | (0x1<<11)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 10);	

		/* ==================== reason 10 & 12  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("SMAC not found and L3 type error. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;				/* packet with vlan tag */
		hsb->vid = 11;
		hsb->l3type = 0x1;		/***********************/
		hsb->l3hwfwdip = 0;		/* Not normal IPv4 Packet */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<10) | (0x1<<12)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 10);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<10) | (0x1<<12)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 10);	

		/* ==================== reason 10 & 25  ========================*/		
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("SMAC not found and L2 bridge disabled. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;				/* packet with vlan tag */
		hsb->vid = 11;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 0;		/* L2 bridge disable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<10) | (0x1<<25)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 10);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<10) | (0x1<<25)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 10);	

		/* ==================== reason 11 & 12  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("SMAC drop and L3 type error. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 0;
		hsb->tif = 1;
		hsb->vid = 11;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 1;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<11) | (0x1<<12)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 11);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<11) | (0x1<<12)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 11);	

		/* ==================== reason 11 & 25  ========================*/		
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("SMAC drop and L2 bridge disable. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;
		hsb->vid = 11;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 0;		/* L2 bridge disable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 1;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<11) | (0x1<<25)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 11);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<11) | (0x1<<25)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 11);	

		/* ==================== reason 9 & 12  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("Vid == 4095 and L3 type error(hasmac == 0). [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;				/* packet with vlan tag */
		hsb->vid = 4095;
		hsb->l3type = 0x0;
		hsb->l3hwfwdip = 0;
		hsb->hasmac = 0;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<9) | (0x1<<12)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 9);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<9) | (0x1<<12)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 9);	

		/* ==================== reason 25 & 26  ========================*/		
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L2 bridge disable and DMAC not found. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;
		hsb->vid = 11;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 0;		/* L2 bridge disable */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<25) | (0x1<<26)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 25);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<25) | (0x1<<26)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 25);	

		/* ==================== reason 26 & 28  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("DMAC not found and source port == dest. port. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->l3len = 234;
		rtl8672_setAsicHSB(hsb);
		/* pkt outgoing dest. port index */
		dstportidx = hsb->srcportidx;

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge disable */
		piftbl->MTU = 456;
		piftbl->LogicalID = hsb->srcportidx;
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);
		rtl8672_getAsicNetInterface(dstportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->MTU = 123;
		piftbl->LogicalID = dstportidx;
		rtl8672_setAsicNetInterface(dstportidx, piftbl);		

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<26) | (0x1<<27)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 26);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<26) | (0x1<<27)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 26);	

		/* ==================== reason 28 & 29  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L3 length > MTU and spa == dpa . [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->l3len = 234;
		rtl8672_setAsicHSB(hsb);
		/* pkt outgoing dest. port index */
		dstportidx = hsb->srcportidx;
		
		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge disable */
		piftbl->MTU = 456;
		piftbl->SrcPortFilter = 1;		/* source port filter enable */
		piftbl->LogicalID = hsb->srcportidx;
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);
		rtl8672_getAsicNetInterface(dstportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->MTU = 123;
		piftbl->LogicalID = dstportidx;
		rtl8672_setAsicNetInterface(dstportidx, piftbl);
		
		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = dstportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<28) | (0x1<<29)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 28);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<28) | (0x1<<29)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 28);	

		/* ==================== reason 12 & 13  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L3 type error and l3 checksum error. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 0;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->l3csok = 0;		/* l3 checksum error */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<12) | (0x1<<13)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 12);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<12) | (0x1<<13)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 12);	

		/* ==================== reason 13 & 14  ========================*/		
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L3 checksum error and l3 ttl = 0. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->l3csok = 0;	/* l3 checksum error */
		hsb->ttl = 0;		/* ttl = 0 */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<13) | (0x1<<14)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 13);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<13) | (0x1<<14)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 13);	

		/* ==================== reason 14 & 15  ========================*/		
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L3 ttl = 0 and layer 3 congestion. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->ttl = 0;			/* ttl = 0 */
		hsb->tos |= 0x3;		/* layer 3 congestion */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (dMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<14) | (0x1<<15)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 14);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<14) | (0x1<<15)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 14);			

		/* ==================== reason 15 & 16  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("Layer 3 congestion and packet is IP fragment. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos |= 0x3;		/* layer 3 congestion */
		hsb->ipfrag = 1;		/* IP fragment */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<15) | (0x1<<16)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 15);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<15) | (0x1<<16)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 15);

		/* ==================== reason 16 & 17  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("Packet is IP fragment and GIP == DIP. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->ipfrag = 1;		/* IP fragment */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		piftbl->GIP = (hsb->dip);	/* GIP == DIP */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<16) | (0x1<<17)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 16);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<16) | (0x1<<17)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 16);

		/* ==================== reason 17 & 18  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("GIP == DIP and L4 table not exist. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		piftbl->GIP = (hsb->dip);	/* GIP == DIP */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);

		/* Disable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) & ~(L4TEB_EN_MASK));
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<17) | (0x1<<18)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 17);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<17) | (0x1<<18)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 17);

		/* ==================== reason 18 & 19  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L4 table not exist and L4 checksum error. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->l4csok = 0;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);

		/* Disable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) & ~(L4TEB_EN_MASK));
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<18) | (0x1<<19)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 18);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<18) | (0x1<<19)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 18);

		/* ==================== reason 19 & 20  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L4 checksum error and it's a SYN/FIN/RST packet. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->l4csok = 0;
		hsb->tcpflag |= 0x7;			/* TCP FLAG: SYN/FIN/RST packet!*/
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);

		/* Enable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) | L4TEB_EN_MASK);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<19) | (0x1<<20)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 19);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<19) | (0x1<<20)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 19);

		/* ==================== reason 20 & 21  ========================*/		
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("SYN/FIN/RST packet and L4 congestion. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->tcpflag |= 0x7;			/* TCP FLAG: SYN/FIN/RST packet!*/
		hsb->tcpflag |= 0xc0;			/* TCP ecn bits high */
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);
		/* set TCP congestion to CPU */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_TCPECN2CPU_MASK));

		/* Enable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) | L4TEB_EN_MASK);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<20) | (0x1<<21)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 20);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<20) | (0x1<<21)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 20);

		/* ==================== reason 19 & 22  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L4 checksum error and L4 flow not match. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->l4csok = 0;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);

		/* Enable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) | L4TEB_EN_MASK);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<19) | (0x1<<22)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 19);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<19) | (0x1<<22)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 19);
		
		/* ==================== reason 22 & 27  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L4 flow not match and ttl equal to 1. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->ttl = 1;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);

		/* Enable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) | L4TEB_EN_MASK);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<22) | (0x1<<27)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 22);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<22) | (0x1<<27)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 22);
		
		/* ==================== reason 23 & 27  ========================*/	
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("Empty SIP & DIP and ttl equal to 1. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->ttl = 1;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);

		/* Enable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) | L4TEB_EN_MASK);
		/* Layer4 Entry */
		bzero(&l4p, sizeof(rtl8672_tblAsic_l2Table_t));
		l4p.Proto = hsb->l4proto;
		l4p.SrcIP = hsb->sip;
		l4p.DstIP = hsb->dip;
		l4p.SrcPort = hsb->sport;
		l4p.DstPort = hsb->dport;
		do{
			l4p.DstPortIdx = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l4p.DstPortIdx == hsb->srcportidx);
		MAC_COPY(l4p.NHMAC, l4_nhmac);
		l4p.NewSrcIP = 0x0;
		l4p.NewDstIP = 0x0;
		l4p.NewSrcPort = 0x1234;
		l4p.NewDstPort = 0x5678;
		l4p.IVlanIDChk = 0;
		l4p.IVlanID = 11;
		l4p.IPppoeIf = 0;
		l4p.SessionID = 0;
		l4p.OTagIf = 0;
		l4p.OVlanID = 11;
		l4p.OPppoeIf = 0;
		l4p.PrecedRemr = 0;
		l4p.TosRemr = 0;
		l4p.Tos = 0;
		l4p.TtlDe = 1;
		l4p.Valid = 1;
		MT_ALE_DEBUG("add L4 entry hashidx = %d \n", rtl8672_L4Hash_Sram(l4p.Proto, l4p.SrcIP, l4p.DstIP, l4p.SrcPort, l4p.DstPort));
		rtl8672_addAsicL4Entry(&l4p, OPT_AUTO);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<23) | (0x1<<27)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 23);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<23) | (0x1<<27)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 23);
		
		/* ==================== reason 24 & 27  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("WLAN APMAC miss and ttl equal to 1. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->ttl = 1;
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);

		/* Enable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) | L4TEB_EN_MASK);
		/* Layer4 Entry */
		bzero(&l4p, sizeof(rtl8672_tblAsic_l2Table_t));
		l4p.Proto = hsb->l4proto;
		l4p.SrcIP = hsb->sip;
		l4p.DstIP = hsb->dip;
		l4p.SrcPort = hsb->sport;
		l4p.DstPort = hsb->dport;
		do{
			l4p.DstPortIdx = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l4p.DstPortIdx == hsb->srcportidx);
		MAC_COPY(l4p.NHMAC, l4_nhmac);
		l4p.NewSrcIP = 0x12345678;
		l4p.NewDstIP = 0x9abcdef0;
		l4p.NewSrcPort = 0x1234;
		l4p.NewDstPort = 0x5678;
		l4p.IVlanIDChk = 0;
		l4p.IVlanID = 11;
		l4p.IPppoeIf = 0;
		l4p.SessionID = 0;
		l4p.OTagIf = 0;
		l4p.OVlanID = 11;
		l4p.OPppoeIf = 0;
		l4p.PrecedRemr = 0;
		l4p.TosRemr = 0;
		l4p.Tos = 0;
		l4p.TtlDe = 1;
		l4p.Valid = 1;
		MT_ALE_DEBUG("add L4 entry hashidx = %d \n", rtl8672_L4Hash_Sram(l4p.Proto, l4p.SrcIP, l4p.DstIP, l4p.SrcPort, l4p.DstPort));
		rtl8672_addAsicL4Entry(&l4p, OPT_AUTO);

		/* get & set dst. interface table */
		rtl8672_getAsicNetInterface(l4p.DstPortIdx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->IfType = IF_WIRELESS;
		rtl8672_setAsicNetInterface(l4p.DstPortIdx, piftbl);
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<24) | (0x1<<27)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 24);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<24) | (0x1<<27)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 24);
#if 0		
		/* ==================== reason 27 & 28  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L3 ttl equal to 1 and Source port filter and L3 length > MTU. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->ttl = 1;
		hsb->l3len = 1500;
		rtl8672_setAsicHSB(hsb);
		dstportidx = hsb->srcportidx;

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		piftbl->SrcPortFilter = 1;
		piftbl->LogicalID = hsb->srcportidx;
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		piftbl->MTU = 64;		/* l3 length > MTU */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->MTU = 123;
		piftbl->LogicalID = dstportidx;
		rtl8672_setAsicNetInterface(dstportidx, piftbl);		

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* Layer2 Entry (NHMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,l4_nhmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		do{
			l2p.Port = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l2p.Port == hsb->srcportidx);
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);
		/* set TCP congestion to CPU */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_TCPECN2CPU_MASK));

		/* Enable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) | L4TEB_EN_MASK);
		/* Layer4 Entry */
		bzero(&l4p, sizeof(rtl8672_tblAsic_l2Table_t));
		l4p.Proto = hsb->l4proto;
		l4p.SrcIP = hsb->sip;
		l4p.DstIP = hsb->dip;
		l4p.SrcPort = hsb->sport;
		l4p.DstPort = hsb->dport;
		do{
			l4p.DstPortIdx = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(l4p.DstPortIdx == hsb->srcportidx);
		MAC_COPY(l4p.NHMAC, l4_nhmac);
		l4p.NewSrcIP = 0x12345678;
		l4p.NewDstIP = 0x9abcdef0;
		l4p.NewSrcPort = 0x1234;
		l4p.NewDstPort = 0x5678;
		l4p.IVlanIDChk = 0;
		l4p.IVlanID = 11;
		l4p.IPppoeIf = 0;
		l4p.SessionID = 0;
		l4p.OTagIf = 0;
		l4p.OVlanID = 11;
		l4p.OPppoeIf = 0;
		l4p.PrecedRemr = 0;
		l4p.TosRemr = 0;
		l4p.Tos = 0;
		l4p.TtlDe = 1;
		l4p.Valid = 1;
		rtlglue_printf("add L4 entry hashidx = %d \n", rtl8672_L4Hash_Sram(l4p.Proto, l4p.SrcIP, l4p.DstIP, l4p.SrcPort, l4p.DstPort));
		rtl8672_addAsicL4Entry(&l4p, OPT_AUTO);		

		/* get & set dst. interface table */
		rtl8672_getAsicNetInterface(l4p.DstPortIdx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->IfType = IF_WIRELESS;
		rtl8672_setAsicNetInterface(l4p.DstPortIdx, piftbl);	
		
		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<27) | (0x1<<28)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 27);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<27) | (0x1<<28)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		ASSERT(hsa->reason == 27);
#endif
		/* ==================== reason 28 & 29 (L4) ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("Source port filter and L3 length > MTU. [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->l3type = 1;
		hsb->l3hwfwdip = 1;
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->tos &= (uint8)(~0x3);		/* layer 3 no congestion */
		hsb->l3len = 1500;
		rtl8672_setAsicHSB(hsb);
		dstportidx = hsb->srcportidx;

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge enable */
		piftbl->SrcPortFilter = 1;		/* source port filter enable */
		/* DMAC == GMAC */
		MAC_COPY(piftbl->GMAC,hsb->dmac);
		piftbl->MTU = 64;		/* l3 length > MTU */
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);
		rtl8672_getAsicNetInterface(dstportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->MTU = 123;
		piftbl->LogicalID = dstportidx;
		rtl8672_setAsicNetInterface(dstportidx, piftbl);
		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&l2p, OPT_AUTO));

		/* set DSCP mode and enabe congestion to cpu */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_IPECN2CPU_MASK) & ~QOSECN_TOSMODE_MASK);
		/* set TCP congestion to CPU */
		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_TCPECN2CPU_MASK));

		/* Enable L4 table */
		WRITE_MEM32(ALECR_L4TEB,READ_MEM32(ALECR_L4TEB) | L4TEB_EN_MASK);
		/* Layer4 Entry */
		bzero(&l4p, sizeof(rtl8672_tblAsic_l2Table_t));
		l4p.Proto = hsb->l4proto;
		l4p.SrcIP = hsb->sip;
		l4p.DstIP = hsb->dip;
		l4p.SrcPort = hsb->sport;
		l4p.DstPort = hsb->dport;
		l4p.DstPortIdx = hsb->srcportidx;
		MAC_COPY(l4p.NHMAC, l4_nhmac);
		l4p.NewSrcIP = 0x12345678;
		l4p.NewDstIP = 0x9abcdef0;
		l4p.NewSrcPort = 0x1234;
		l4p.NewDstPort = 0x5678;
		l4p.IVlanIDChk = 0;
		l4p.IVlanID = 11;
		l4p.IPppoeIf = 0;
		l4p.SessionID = 0;
		l4p.OTagIf = 0;
		l4p.OVlanID = 11;
		l4p.OPppoeIf = 0;
		l4p.PrecedRemr = 0;
		l4p.TosRemr = 0;
		l4p.Tos = 0;
		l4p.TtlDe = 1;
		l4p.Valid = 1;
		rtlglue_printf("add L4 entry hashidx = %d \n", rtl8672_L4Hash_Sram(l4p.Proto, l4p.SrcIP, l4p.DstIP, l4p.SrcPort, l4p.DstPort));
		rtl8672_addAsicL4Entry(&l4p, OPT_AUTO);		

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<28) | (0x1<<29)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 29);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<28) | (0x1<<29)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 29);
		/* ==================== reason 29 & 30  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("L3 length > MTU and other error case(reason = 30). [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->l3len = 456;
		hsb->ethtp = (0x0800);
		do{
			dstportidx = (rtlglue_random()%(SAR_INTFS + MAC_TX_INTFS + EXT_INTFS));
		}while(dstportidx == hsb->srcportidx);
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge disable */
		piftbl->MTU = 123;
		piftbl->SrcPortFilter = 1;		/* source port filter enable */
		piftbl->LogicalID = hsb->srcportidx;
		piftbl->IfType = IF_SAR;
		piftbl->SARhdr = ROUTED;
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);
		rtl8672_getAsicNetInterface(dstportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->MTU = 123;
		piftbl->LogicalID = dstportidx;
		rtl8672_setAsicNetInterface(dstportidx, piftbl);
		
		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = dstportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~((0x1<<29) | (0x1<<30)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 29);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | ((0x1<<29) | (0x1<<30)));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 29);	

		/* ==================== reason 30  ========================*/
		MT_ALE_DEBUG("\n\n************************* ALE Drop/Trap_to_CPU test case :: %d***********************\n",ncase++);
		MT_ALE_DEBUG("Error case(reason = 30). [%s(%d)]\n",__FUNCTION__,__LINE__);

		/* reset hsb */
		_ale_reset_hsb(hsb);
		/* reset ALE tables */
		testALE_Reset();
		/* pkt incoming source port index */
		hsb->srcportidx = (rtlglue_random()%(SAR_INTFS + MAC_RX_INTFS + EXT_INTFS));
		hsb->tif = 1;
		hsb->vid = 11;
		hsb->l3len = 123;
		hsb->ethtp = (0x0800);
		rtl8672_setAsicHSB(hsb);

		/* get & set interface table */
		rtl8672_getAsicNetInterface(hsb->srcportidx, piftbl);
		ASSERT(piftbl != NULL);
		piftbl->AcceptTagged = 1;		/* accept_tag enable */
		piftbl->L2BridgeEnable = 1;		/* L2 bridge disable */
		piftbl->MTU = 456;
		piftbl->SrcPortFilter = 0;		/* source port filter disable */
		piftbl->IfType = IF_SAR;
		piftbl->SARhdr = ROUTED;
		rtl8672_setAsicNetInterface(hsb->srcportidx, piftbl);

		/* Layer2 Entry (SMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->smac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (SMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Layer2 Entry (DMAC) */
		bzero(&l2p, sizeof(rtl8672_tblAsic_l2Table_t));
		MAC_COPY(l2p.MAC,hsb->dmac);
		MAC_COPY(l2p.APMAC, empty_mac);
		l2p.Port = hsb->srcportidx;
		l2p.VlanID = hsb->vid;
		l2p.OTagIf = 0;
		l2p.Dot1PRemr = 0;
		l2p.PriorityIDRemr = 0;
		l2p.SADrop = 0;
		l2p.Valid = 1;
		MT_ALE_DEBUG("add L2 entry (DMAC) hashidx = %d \n", rtl8672_L2Hash_Sram(&l2p.MAC[0], l2p.VlanID));
		rtl8672_addAsicL2Entry(&l2p, OPT_AUTO);

		/* Trap to CPU while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) & ~(0x1<<30));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d, Trap to CPU!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 0);
		ASSERT(hsa->reason == 30);
		
		/* Drop packet while reason not equal to zero */
		WRITE_MEM32(ALECR_REASONACT,READ_MEM32(ALECR_REASONACT) | (0x1<<30));
		/* run ALE */
		WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
		WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
		do{
			/* polling trigger bit */
		}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
		rtl8672_getAsicHSA(hsa);

		MT_ALE_DEBUG("=========>HSA reason = %d Drop packet!  [%s(%d)]\n\n",hsa->reason,__FUNCTION__,__LINE__);
		ASSERT(hsa->droppacket == 1);
		aledropcnt++;
#ifdef	RTL867X_MODEL_USER
#else
		ASSERT(aledropcnt == READ_MEM32(ALEDROPCNT));
#endif
		ASSERT(hsa->reason == 30);	
		
		/* Release Memory */
		testALE_Reset();
		testALE_Exit();



	rtlglue_free(hsb);
	hsb = NULL;
	rtlglue_free(hsa);
	hsa = NULL;
	rtlglue_free(piftbl);
	piftbl = NULL;

	return SUCCESS;
}

/*
@func int32	| testALE_L2ModelFlow	| Test case for ALE Layer 2 flow behavior.
@parm uint32	| test case numberic code.
@rvalue SUCCESS	| Test case passes. 
@rvalue FAILED 	| Test case fails.
@comm 
This case verify the Packet Processor ALE layer 2 flow behavior. We assume any hsb filed valid value add the layer 2 table specific entrys
to make the Packet Processor process packets with layer 2 flow. And then we get the hsa results from Packet Processor to compare our expected
hsa. This case runs in Packet Processor Test mode.
@devnote
 */
int32 testALE_L2ModelFlow(uint32 caseNo)
{
	uint32 cnd_iftype = 0, cnd_tif = 0, cnd_vid = 0, cnd_hasmac = 0, cnd_l3type = 0, cnd_pif = 0, cnd_ethtp = 0, cnd_framtctrl = 0, cnd_toap = 0,
#if 0	/* Layer 2 only */
		cnd_l4proto = 0, cnd_udpnocs = 0, cnd_l3ttlde = 0, cnd_l3tosremr = 0, cnd_l4ipchange = 0, cnd_l4portchange = 0,cnd_gmac = 0, cnd_l2invalid = 0,
#endif		
		cnd_srcif_sarhdr = 0, cnd_dstif_sarhdr = 0, cnd_dstif_1premr = 0, cnd_dstif_1qremr = 0;
	uint32 ncase = 0, errcnt = 0;;
	uint32 srcportidx = 0, dstportidx = 0, isToAP = 0;
	hsb_param_t hsb;
	hsa_param_t expect_hsa, hsa;
	struct rtl8672_tblAsicDrv_intfParam_s src_iftbl,dst_iftbl;
	rtl8672_tblAsicDrv_l2Param_t src_l2p, dst_l2p;
//	rtl8672_tblAsicDrv_l4Param_t l4p;
	rtl8672_tblAsicDrv_pnpParam_t pnpp;
	uint8 dmac[6], smac[6], mac3[6], mac4[6], apmac[6], gmac[6];
	dmac[0] = 0x0; dmac[1] = 0xd1; dmac[2] = 0xd2; dmac[3] = 0xd3; dmac[4] = 0xd4; dmac[5] = 0xd5;
	smac[0] = 0x0; smac[1] = 0xe1; smac[2] = 0xe2; smac[3] = 0xe3; smac[4] = 0xe4; smac[5] = 0xe5;
	mac3[0] = 0x0; mac3[1] = 0x31; mac3[2] = 0x32; mac3[3] = 0x33; mac3[4] = 0x34; mac3[5] = 0x35;
	mac4[0] = 0x0; mac4[1] = 0x41; mac4[2] = 0x42; mac4[3] = 0x43; mac4[4] = 0x44; mac4[5] = 0x45;
	apmac[0] = 0x0; apmac[1] = 0xa1; apmac[2] = 0xa2; apmac[3] = 0xa3; apmac[4] = 0xa4; apmac[5] = 0xa5;	
	gmac[0] = 0x0; gmac[1] = 0xb1; gmac[2] = 0xb2; gmac[3] = 0xb3; gmac[4] = 0xb4; gmac[5] = 0xb5;	

	testALE_Reset();
	testALE_InitPNP();
	init_ring();

	for(cnd_iftype = 9; cnd_iftype--; ){
		for(cnd_tif = 4; cnd_tif--; ){
			for(cnd_vid = 2; cnd_vid--; ){
				for(cnd_hasmac = 2; cnd_hasmac--; ){
					for(cnd_l3type = 2; cnd_l3type--; ){
						for(cnd_pif = 4; cnd_pif--; ){
							for(cnd_ethtp = 4; cnd_ethtp--; ){
								for(cnd_framtctrl = 4; cnd_framtctrl--; ){
									for(cnd_toap = 2; cnd_toap--; ){
/* Layer 2 only */
#if 0
										for(cnd_gmac = 2; cnd_gmac--; ){
											for(cnd_l2invalid = 2; cnd_l2invalid--; ){
												for(cnd_l4proto = 2; cnd_l4proto--; ){
													for(cnd_udpnocs = 2; cnd_udpnocs--; ){
														for(cnd_l3ttlde = 2; cnd_l3ttlde--; ){
															for(cnd_l3tosremr = 2; cnd_l3tosremr--; ){
																for(cnd_l4ipchange = 2; cnd_l4ipchange--; ){
																	for(cnd_l4portchange = 2; cnd_l4portchange--; ){
#endif
																		for(cnd_srcif_sarhdr = 5; cnd_srcif_sarhdr--; ){
																			for(cnd_dstif_sarhdr = 5; cnd_dstif_sarhdr--; ){
																				for(cnd_dstif_1premr = 2; cnd_dstif_1premr--; ){
																					for(cnd_dstif_1qremr = 2; cnd_dstif_1qremr--; ){
																						if((ncase%10) == 0)
																							rtlglue_printf("#");

																						/* initial */
																						memset(&hsb,0,sizeof(hsb_param_t));
																						memset(&src_iftbl,0,sizeof(struct rtl8672_tblAsicDrv_intfParam_s));
																						memset(&dst_iftbl,0,sizeof(struct rtl8672_tblAsicDrv_intfParam_s));
																						memset(&src_l2p,0,sizeof(rtl8672_tblAsicDrv_l2Param_t));
																						memset(&dst_l2p,0,sizeof(rtl8672_tblAsicDrv_l2Param_t));
																						/* mac */
																						if(cnd_hasmac == 0){
																							hsb.hasmac = 0;
																							continue;	/* L2 flow only */
																						}else{
																							hsb.hasmac = 1;
																						}
																						/* If. Type */
																						switch(cnd_iftype){
																							case 0:
																								/* Incoming SAR interface, outgoing SAR interface */
																								src_iftbl.IfType = IF_SAR;
																								dst_iftbl.IfType = IF_SAR;
																								continue;
																							case 1:
																								/* Incoming SAR interface, outgoing MAC interface */
																								src_iftbl.IfType = IF_SAR;
																								dst_iftbl.IfType = IF_ETHERNET;
																								break;
																							case 2:
																								/* Incoming SAR interface, outgoing ext. port interface */
																								src_iftbl.IfType = IF_SAR;
																								dst_iftbl.IfType = IF_WIRELESS;
																								break;
																							case  3:
																								/* Incoming MAC interface, outgoing SAR interface */
																								src_iftbl.IfType = IF_ETHERNET;
																								dst_iftbl.IfType = IF_SAR;
																								break;
																							case 4:
																								/* Incoming MAC interface, outgoing MAC interface */
																								src_iftbl.IfType = IF_ETHERNET;
																								dst_iftbl.IfType = IF_ETHERNET;
																								break;
																							case 5:
																								/* Incoming MAC interface, outgoing ext. port interface */
																								src_iftbl.IfType = IF_ETHERNET;
																								dst_iftbl.IfType = IF_WIRELESS;
																								break;
																							case 6:
																								/* Incoming ext. interface, outgoing SAR interface */
																								src_iftbl.IfType = IF_WIRELESS;
																								dst_iftbl.IfType = IF_SAR;
																								break;
																							case 7:
																								/* Incoming ext. interface, outgoing MAC interface */
																								src_iftbl.IfType = IF_WIRELESS;
																								dst_iftbl.IfType = IF_ETHERNET;
																								break;
																							default:
																								/* Incoming ext. interface, outgoing ext. port interface */
																								src_iftbl.IfType = IF_WIRELESS;
																								dst_iftbl.IfType = IF_WIRELESS;
																								break;
																						}
																						/* condition hsb & interface table */
																						if(src_iftbl.IfType == IF_SAR){
																							srcportidx = rtlglue_random()%(SAR_INTFS);
																						}else if(src_iftbl.IfType == IF_ETHERNET){
																							srcportidx = rtlglue_random()%(MAC_RX_INTFS) + SAR_INTFS;
																						}else{
																							srcportidx = rtlglue_random()%(EXT_INTFS) + SAR_INTFS + MAC_TX_INTFS;
																						}
																						if(dst_iftbl.IfType == IF_SAR){
																							do{
																								dstportidx = rtlglue_random()%(SAR_INTFS);
																							}while(dstportidx == srcportidx);
																						}else if(dst_iftbl.IfType == IF_ETHERNET){
																							do{
																								dstportidx = rtlglue_random()%(MAC_TX_INTFS) + SAR_INTFS;
																							}while(dstportidx == srcportidx);
																						}else{
																							do{
																								dstportidx = rtlglue_random()%(EXT_INTFS) + SAR_INTFS + MAC_TX_INTFS;
																							}while(dstportidx == srcportidx);
																						}

																						hsb.srcportidx = srcportidx;
																						src_iftbl.AcceptTagged = 1;
																						src_iftbl.AcceptUntagged = 1;
																						
																						if(src_iftbl.IfType == IF_SAR){
																							src_iftbl.L2Encap = rtlglue_random()%2;
																							src_iftbl.LanFCS = rtlglue_random()%2;
																							switch(cnd_srcif_sarhdr){
																								case 0:
																									src_iftbl.SARhdr = ROUTED;
																									continue;
																								case 1:
																									src_iftbl.SARhdr = BRIDGED;
																									break;
																								case 2:
																									src_iftbl.SARhdr = MER;
																									continue;
																								case 3:
																									src_iftbl.SARhdr = PPPOE;
																									continue;
																								default:
																									src_iftbl.SARhdr = PPPOA;
																									continue;
																							}

																						}
																						else
																						{
																							if(cnd_srcif_sarhdr != 0) continue;
																						}
																						if(dst_iftbl.IfType == IF_SAR){
																							dst_iftbl.L2Encap = rtlglue_random()%2;
																							dst_iftbl.LanFCS = rtlglue_random()%2;
																							switch(cnd_dstif_sarhdr){
																								case 0:
																									dst_iftbl.SARhdr = ROUTED;
																									continue;
																								case 1:
																									dst_iftbl.SARhdr = BRIDGED;
																									break;
																								case 2:
																									dst_iftbl.SARhdr = MER;
																									continue;
																								case 3:
																									dst_iftbl.SARhdr = PPPOE;
																									continue;
																								default:
																									dst_iftbl.SARhdr = PPPOA;
																									continue;
																							}

																						}	
																						else
																						{
																							if(cnd_dstif_sarhdr != 0) continue;
																						}
																						/* valn tag */
																						switch(cnd_tif){
																							case 0:
																								/* incoming packet untagged, outgoing packet untagged either */
																								if(cnd_vid != 0) continue;
																								hsb.tif = 0;
																								hsb.vid = 0;
																								hsb.cfi = 0;
																								hsb.pid = 0;
																								dst_l2p.OTagIf = 0;
																								break;
																							case 1:
																								/* incoming packet tagged, outgoing packet untagged */
																								hsb.tif = 1;
																								if(cnd_vid == 0){
																									hsb.vid = 0;		/* priority tag */
																									hsb.pid = rtlglue_random()%0x7;
																								}else{
																									hsb.vid = (rtlglue_random()%4094) + 1;		/* vid = 1~4094 */
																									hsb.pid = rtlglue_random()%0x7;
																								}
																								dst_l2p.OTagIf = 0;
																								break;
																							case 2:
																								/* incoming packet untagged, outgoing packet tagged */
																								if(cnd_vid != 0) continue;
																								hsb.tif = 0;
																								hsb.vid = 0;
																								hsb.cfi = 0;
																								hsb.pid = 0;
																								dst_l2p.OTagIf = 1;
																								break;
																							default:
																								/* incoming packet tagged, outgoing packet tagged */
																								hsb.tif = 1;
																								if(cnd_vid == 0){
																									hsb.vid = 0;
																									hsb.pid = rtlglue_random()%0x7;
																								}else{
																									hsb.vid = (rtlglue_random()%4094) + 1;		/* vid = 1~4094 */
																									hsb.pid = rtlglue_random()%0x7;
																								}
																								dst_l2p.OTagIf = 1;
																								break;
																						}
																						if(dst_l2p.OTagIf == 1){
																							/* 802.1P remarking */
																							if(cnd_dstif_1premr == 0){
																								dst_l2p.Dot1PRemr = 0;
																								dst_l2p.PriorityIDRemr = 0;
																							}else{
																								dst_l2p.Dot1PRemr = 1;
																								do{
																									dst_l2p.PriorityIDRemr = rtlglue_random()%0x7;
																								}while(dst_l2p.PriorityIDRemr == hsb.pid);
																							}
																							/* 802.1Q remarking */
																							if(cnd_dstif_1qremr != 0){
																								dst_iftbl.Dot1QRemr = 1;
																								do{
																									dst_iftbl.VlanIDRemr = (rtlglue_random()%4094) + 1;
																								}while(dst_iftbl.VlanIDRemr == hsb.vid);
																							}else{
																								dst_iftbl.Dot1QRemr = 0;
																								dst_iftbl.PortVlanID = 0;
																							}
																						}else{
																							if(cnd_dstif_1premr != 0) continue;
																							if(cnd_dstif_1qremr != 0) continue;
																						}
																						if((dst_l2p.Dot1PRemr == 1) || (dst_iftbl.Dot1QRemr == 1)){
																							expect_hsa.vidremark = 1;
																						}else{
																							expect_hsa.vidremark = 0;
																						}
																						/* pppoe tag */
																						switch(cnd_pif){
																							case 0:
																								/* Incoming packet without pppoe tag, outgoing without pppoe tag */
																								hsb.pif = 0;
																								hsb.sid = 0;
																								break;
																							case 1:
																								/* Incoming packet without pppoe tag, outgoing with pppoe tag */
																								continue;
																							case 2:
																								/* Incoming packet with pppoe tag, outgoing without pppoe tag */
																								continue;
																							default:
																								/* Incoming packet with pppoe tag, outgoing with pppoe tag */
																								hsb.pif = 1;
																								hsb.sid = 0x1234;
																								hsb.pppcompable = rtlglue_random()%2;
																								break;
																						}
																						/* Layer 3 type */
																						if(cnd_l3type == 0){
																							hsb.l3type = 0x0;
																						}else{
																							hsb.l3type = 0x1;
																						}
																						/* ether type */
																						switch(cnd_ethtp){
																							case 0:
																								/* 0x0800 */
																								if((hsb.hasmac == 1) && (hsb.l3type == 1) && (hsb.pif != 1)){
																									hsb.ethtp = (0x0800);
																								}else continue;
																								break;
																							case 1:
																								/* 0x8864 */
																								if((hsb.hasmac == 1) && (hsb.l3type == 1) && (hsb.pif == 1)){
																									hsb.ethtp = (0x8864);
																								}else continue;
																								break;
																							case 2:
																								/* 0x8863 */
																								if((hsb.hasmac == 1) && (hsb.l3type != 1) && (hsb.pif != 1)){
																									hsb.ethtp = (0x8863);
																								}else continue;
																								break;
																							default:
																								/* other */
																								if((hsb.hasmac == 1) && (hsb.l3type != 1) && (hsb.pif != 1)){
																									hsb.ethtp = (0x86dd);
																								}else continue;
																								break;
																						}
																						if(src_iftbl.IfType == IF_WIRELESS){
																							/* 802.11 frame control type */
																							switch(cnd_framtctrl){
																								case 0:
																									/* ToDS = 0, FromDS = 0 */
																									hsb.framectrl = (0x3800);
																									MAC_COPY(src_l2p.MAC,smac);
																									MAC_COPY(dst_l2p.MAC,dmac);
																									break;
																								case 1:
																									/* ToDS = 0, FromDS = 1 */
																									hsb.framectrl = (0x3802);
																									MAC_COPY(src_l2p.MAC,mac3);
																									MAC_COPY(dst_l2p.MAC,dmac);
																									break;
																								case 2:
																									/* ToDS = 1, FromDS = 0 */
																									hsb.framectrl = (0x3801);
																									MAC_COPY(src_l2p.MAC,smac);
																									MAC_COPY(dst_l2p.MAC,mac3);
																									break;
																								default:
																									/* ToDS = 1, FromDS = 1 */
																									hsb.framectrl = (0x3803);
																									MAC_COPY(src_l2p.MAC,mac4);
																									MAC_COPY(dst_l2p.MAC,mac3);
																									break;
																							}
																						}
																						else
																						{
																							if(cnd_framtctrl != 0) continue;
																							hsb.framectrl = 0;
																							MAC_COPY(src_l2p.MAC,smac);
																							MAC_COPY(dst_l2p.MAC,dmac);
																						}
																						if(dst_iftbl.IfType == IF_WIRELESS){
																							if(cnd_toap == 1){
																								isToAP = 1;
																							}else{
																								isToAP = 0;
																							}
																						}

																						/* error case ignore */
																						if((src_iftbl.IfType == IF_SAR) && (dst_iftbl.IfType == IF_SAR)) continue;
																						if((src_iftbl.IfType == IF_SAR) && (src_iftbl.SARhdr != BRIDGED)) continue;
																						if((dst_iftbl.IfType == IF_SAR) && (dst_iftbl.SARhdr != BRIDGED)) continue;
																						if((hsb.ethtp == (0x8864)) && (hsb.pif != 1)) continue;
																						if((hsb.ethtp != (0x8864)) && (hsb.pif == 1)) continue;
																						
																					#if 1
																						/* other hsb filed value */
																						hsb.dip = 0xc0a89afc;
																						hsb.sip = 0xc0a89a3c;
																						hsb.dport = 0x88;
																						hsb.sport = 0x99;
																						hsb.ipfrag = 0;
																						hsb.l3cs = 0x3333;
																						hsb.l3csok = 1;
																						hsb.l3len = rtlglue_random()%1500;
																						hsb.l3offset = 24;		/* altering value, it's not the issue */
																						hsb.l4cs = 0x4444;
																						hsb.l4csok = 1;
																						hsb.l4proto = ((rtlglue_random()%2) == 0)?IPPROTO_TCP:IPPROTO_UDP;
																						MAC_COPY(hsb.dmac,dmac);
																						MAC_COPY(hsb.smac,smac);
																						MAC_COPY(hsb.mac3,mac3);
																						MAC_COPY(hsb.mac4,mac4);
																						hsb.tcpflag = ((rtlglue_random()%0xff) & 0xf8);
																						rtl8672_setAsicHSB(&hsb);

																						/* source port interface table */
																						src_iftbl.AcceptTagged = 1;
																						src_iftbl.AcceptUntagged = 1;
																						src_iftbl.L2BridgeEnable = 1;
																						src_iftbl.MTU = 1518;
																						do{
																							src_iftbl.PortPriorityID= rtlglue_random()%0x7;
																						}while((src_iftbl.PortPriorityID == hsb.pid) || (src_iftbl.PortPriorityID == src_l2p.PriorityIDRemr));
																						do{
																							src_iftbl.PortVlanID = ((rtlglue_random()%4094) + 1);
																						}while((src_iftbl.PortVlanID == hsb.vid) || (src_iftbl.PortVlanID == dst_iftbl.VlanIDRemr));
																						src_iftbl.SrcPortFilter = 1;
																						src_iftbl.LogicalID = srcportidx;
																						src_iftbl.L2Encap = rtlglue_random()%2;
																						src_iftbl.LanFCS = rtlglue_random()%2;
																						src_iftbl.CompPPP = rtlglue_random()%2;
																						rtl8672_setAsicNetInterface(srcportidx,&src_iftbl);

																						/* dest. port interface table */
																						dst_iftbl.AcceptTagged = 1;
																						dst_iftbl.AcceptUntagged = 1;
																						dst_iftbl.L2BridgeEnable = 1;
																						dst_iftbl.MTU = 1518;
																						dst_iftbl.PortPriorityID= rtlglue_random()%0x7;
																						dst_iftbl.PortVlanID = ((rtlglue_random()%4094) + 1);
																						dst_iftbl.SrcPortFilter = 1;
																						dst_iftbl.LogicalID = dstportidx;
																						if(dst_iftbl.IfType == IF_SAR){
																							dst_iftbl.L2Encap = rtlglue_random()%2;
																							dst_iftbl.LanFCS = rtlglue_random()%2;
																						}else{
																							dst_iftbl.L2Encap = 0;
																							dst_iftbl.LanFCS = 0;
																						}
																						if(hsb.pif == 1){
																							dst_iftbl.CompPPP = rtlglue_random()%2;
																						}else{
																							dst_iftbl.CompPPP = 0;
																						}
																						MAC_COPY(dst_iftbl.GMAC,gmac);
																						rtl8672_setAsicNetInterface(dstportidx,&dst_iftbl);

																						/* pnpp table selection */
																						if(hsb.ethtp == (0x0800)){
																							if(src_iftbl.IfType == IF_SAR){
																								rtl8672_getAsicProtocolBasedVLAN(0, &pnpp);
																							}else if(src_iftbl.IfType == IF_ETHERNET){
																								rtl8672_getAsicProtocolBasedVLAN(1, &pnpp);
																							}else{
																								rtl8672_getAsicProtocolBasedVLAN(2, &pnpp);
																							}
																						}else if(hsb.ethtp == (0x8864)){
																							if(src_iftbl.IfType == IF_SAR){
																								rtl8672_getAsicProtocolBasedVLAN(3, &pnpp);
																							}else if(src_iftbl.IfType == IF_ETHERNET){
																								rtl8672_getAsicProtocolBasedVLAN(4, &pnpp);
																							}else{
																								rtl8672_getAsicProtocolBasedVLAN(5, &pnpp);
																							}
																						}else if(hsb.ethtp == (0x8863)){
																							rtl8672_getAsicProtocolBasedVLAN(6, &pnpp);
																						}else{
																							rtl8672_getAsicProtocolBasedVLAN(7, &pnpp);
																						}

																						/* Layer 2 entery for smac*/
																						src_l2p.Age = rtlglue_random()%2;
																						MAC_COPY(src_l2p.APMAC,empty_mac);
																						src_l2p.Port = hsb.srcportidx;
																						src_l2p.SADrop = 0;
																						src_l2p.Valid = 1;
																						if((hsb.tif == 1) && (hsb.vid != 0)){
																							src_l2p.VlanID = hsb.vid;
																						}else{
																							src_l2p.VlanID = pnpp.VlanID;
																						}
																						src_l2p.PriorityIDRemr = rtlglue_random()%0x7;
																						src_l2p.Dot1PRemr = 1;
																						ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&src_l2p, OPT_AUTO));

																						/* Layer 2 entry for dmac */
																						dst_l2p.Age = rtlglue_random()%2;
																						if(isToAP == 1){
																							MAC_COPY(dst_l2p.APMAC,apmac);
																						}else{
																							MAC_COPY(dst_l2p.APMAC,empty_mac);
																						}
																						dst_l2p.Port = dstportidx;
																						dst_l2p.SADrop = 0;
																						dst_l2p.Valid = 1;
																						dst_l2p.VlanID = src_l2p.VlanID;
																						ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&dst_l2p, OPT_AUTO));
																						
																					#endif

																						/* excepted hsa */
																						expect_hsa.outiftype = dst_iftbl.IfType;
																						expect_hsa.outl2encap = dst_iftbl.L2Encap;
																						expect_hsa.outlanfcs = dst_iftbl.LanFCS;
																						expect_hsa.pppprotolen= hsb.pppcompable;
																						expect_hsa.outsarhdr = dst_iftbl.SARhdr;
																						expect_hsa.droppacket = 0;
																						expect_hsa.pif = hsb.pif;
																						expect_hsa.l3change = 0;
																						expect_hsa.l4change = 0;
																						expect_hsa.fromcpu = 0;
																						expect_hsa.tocpu = 0;
																						expect_hsa.iniftype = src_iftbl.IfType;
																						expect_hsa.tif = dst_l2p.OTagIf;
																						expect_hsa.linkid = 0;
																						expect_hsa.dstportidx = dstportidx;
																						expect_hsa.srcportidx = srcportidx;
																						expect_hsa.ethtp = hsb.ethtp;

																						if(expect_hsa.outiftype == IF_WIRELESS){
																							/* 802.11 */
																							expect_hsa.framectrl = hsb.framectrl | 0x2 ;
																							if (isToAP == 1){
																								/* to AP */
																								expect_hsa.framectrl |=  0x1 ;
																								MAC_COPY(expect_hsa.dmac,apmac);
																								MAC_COPY(expect_hsa.smac,dst_iftbl.GMAC);
																								MAC_COPY(expect_hsa.mac3,dst_l2p.MAC);
																								MAC_COPY(expect_hsa.mac4,src_l2p.MAC);
																							}else{
																								/* to station */
																								expect_hsa.framectrl &= ~0x1 ;
																								MAC_COPY(expect_hsa.dmac,dst_l2p.MAC);
																								MAC_COPY(expect_hsa.smac,dst_iftbl.GMAC);
																								MAC_COPY(expect_hsa.mac3,src_l2p.MAC);
																								MAC_COPY(expect_hsa.mac4,empty_mac);
																							}
																						}
																						else
																						{
																							expect_hsa.framectrl = hsb.framectrl & ~0x3;		/* clear toDS & fromDS */
																							MAC_COPY(expect_hsa.dmac,dst_l2p.MAC);
																							MAC_COPY(expect_hsa.smac,src_l2p.MAC);
																							MAC_COPY(expect_hsa.mac3,hsb.mac3);
																							MAC_COPY(expect_hsa.mac4,hsb.mac4);
																						}
																						
																						expect_hsa.sid = hsb.sid;
																						/* priority id decision */
																						if(hsb.tif == 1){
																							expect_hsa.pid = hsb.pid;
																							expect_hsa.cfi = hsb.cfi;
																						}else{
																							expect_hsa.pid = pnpp.PriorityID;
																							expect_hsa.cfi = 0;
																						}
																						if(dst_l2p.OTagIf == 0){
																							expect_hsa.tif = 0;
																						}else{
																							expect_hsa.tif = 1;
																							if(dst_l2p.Dot1PRemr == 1){
																								expect_hsa.pid = dst_l2p.PriorityIDRemr;
																							}
																						}
																						/* vlan id decision */
																						if((hsb.tif == 1) && (hsb.vid != 0)){
																							expect_hsa.vid = hsb.vid;
																						}else{
																							expect_hsa.vid = pnpp.VlanID;
																						}
																						if(dst_l2p.OTagIf == 0){
																							expect_hsa.tif = 0;
																						}else{
																							expect_hsa.tif = 1;
																							if(dst_iftbl.Dot1QRemr == 1){
																								expect_hsa.vid = dst_iftbl.VlanIDRemr;
																							}
																						}
																						
																						expect_hsa.l3type = hsb.l3type;
																						expect_hsa.tosremr = 0;
																						expect_hsa.tos = hsb.tos;
																						expect_hsa.ttl = hsb.ttl;
																						expect_hsa.inl3offset = hsb.l3offset;
																						expect_hsa.inl3len = hsb.l3len;
																						expect_hsa.l3cs = hsb.l3cs;
																						expect_hsa.sip = hsb.sip;
																						expect_hsa.dip = hsb.dip;
																						expect_hsa.sport = hsb.sport;
																						expect_hsa.dport = hsb.dport;
																						expect_hsa.l4cs = hsb.l4cs;
																						expect_hsa.l4proto = hsb.l4proto;
																						expect_hsa.reason = 0;

																						/* ALE working */
																						MT_ALE_DEBUG("srcportidx=%d dstportidx=%d\n",srcportidx,dstportidx);
																						
																						WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
																						WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
																						do{
																							/* polling trigger bit */
																						}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
																						rtl8672_getAsicHSA(&hsa);

																						if(compHsa(&hsa,&expect_hsa)==FAILED) 
																						{
																							errcnt++;
																							rtlglue_printf("Error Case : %d!!\n",ncase);
																							return FAILED; 
																						}

																						ASSERT(SUCCESS == rtl8672_delAsicL2Entry(&dst_l2p, OPT_AUTO));
																						ASSERT(SUCCESS == rtl8672_delAsicL2Entry(&src_l2p, OPT_AUTO));
																						
																						ncase++;

																					}
																				}
																			}
																		}
#if 0																	
																	}
																}
															}
														}
													}
												}
											}
										}
#endif									
									}
								}
							}
						}
					}
				}
			}
		}
	}
	rtlglue_printf("************** Total case:%d\n",ncase);	
	testALE_Reset();
	testALE_Exit();
	return SUCCESS;
}

/*
@func int32	| testALE_L34ModelFlow	| Test case for ALE Layer 4 flow behavior.
@parm uint32	| test case numberic code.
@rvalue SUCCESS	| Test case passes. 
@rvalue FAILED 	| Test case fails.
@comm 
This case verify the Packet Processor ALE layer 4 flow behavior. We assume any hsb filed valid value add the layer 2 and layer 3/4 table specific entrys
to make the Packet Processor process packets with layer 4 flow. And then we get the hsa results from Packet Processor to compare our expected
hsa. This case runs in Packet Processor Test mode.
@devnote
 */
int32 testALE_L34ModelFlow(uint32 caseNo)
{
	uint32 cnd_iftype = 0, cnd_tif = 0, cnd_vid = 0, cnd_hasmac = 0, cnd_pif = 0, cnd_ethtp = 0, cnd_dscpremr = 0, cnd_toap = 0,
		cnd_l4proto = 0, cnd_l3ttlde = 0, cnd_l3tosremr = 0, cnd_l2invalid = 0, cnd_srcif_sarhdr = 0, cnd_dstif_sarhdr = 0, 
		cnd_dstif_1premr = 0, cnd_tosmode = 0;
	uint32 ncase = 0, errcnt = 0;;
	uint32 srcportidx = 0, dstportidx = 0, isToAP = 0;
	hsb_param_t hsb;
	hsa_param_t expect_hsa, hsa;
	struct rtl8672_tblAsicDrv_intfParam_s src_iftbl,dst_iftbl;
	rtl8672_tblAsicDrv_l2Param_t dst_l2p, src_l2p;
	rtl8672_tblAsicDrv_l4Param_t l4p;
	rtl8672_tblAsicDrv_pnpParam_t pnpp;
	uint8 dmac[6], smac[6], mac3[6], mac4[6], apmac[6], gmac[6];
	dmac[0] = 0x0; dmac[1] = 0xd1; dmac[2] = 0xd2; dmac[3] = 0xd3; dmac[4] = 0xd4; dmac[5] = 0xd5;
	smac[0] = 0x0; smac[1] = 0xe1; smac[2] = 0xe2; smac[3] = 0xe3; smac[4] = 0xe4; smac[5] = 0xe5;
	mac3[0] = 0x0; mac3[1] = 0x31; mac3[2] = 0x32; mac3[3] = 0x33; mac3[4] = 0x34; mac3[5] = 0x35;
	mac4[0] = 0x0; mac4[1] = 0x41; mac4[2] = 0x42; mac4[3] = 0x43; mac4[4] = 0x44; mac4[5] = 0x45;
	apmac[0] = 0x0; apmac[1] = 0xa1; apmac[2] = 0xa2; apmac[3] = 0xa3; apmac[4] = 0xa4; apmac[5] = 0xa5;	
	gmac[0] = 0x0; gmac[1] = 0xb1; gmac[2] = 0xb2; gmac[3] = 0xb3; gmac[4] = 0xb4; gmac[5] = 0xb5;	

	testALE_Init();
	testALE_Reset();
	testALE_InitPNP();
	init_ring();

	for(cnd_iftype = 9; cnd_iftype--; ){
		for(cnd_tif = 4; cnd_tif--; ){
			for(cnd_vid = 2; cnd_vid--; ){
				for(cnd_hasmac = 2; cnd_hasmac--; ){
					for(cnd_pif = 4; cnd_pif--; ){
						for(cnd_ethtp = 2; cnd_ethtp--; ){
							for(cnd_dscpremr = 2; cnd_dscpremr--; ){
								for(cnd_toap = 2; cnd_toap--; ){
									for(cnd_l2invalid = 2; cnd_l2invalid--; ){
										for(cnd_l4proto = 3; cnd_l4proto--; ){
											for(cnd_l3ttlde = 2; cnd_l3ttlde--; ){
												for(cnd_l3tosremr = 2; cnd_l3tosremr--; ){
													for(cnd_srcif_sarhdr = 5; cnd_srcif_sarhdr--; ){
														for(cnd_dstif_sarhdr = 5; cnd_dstif_sarhdr--; ){
															for(cnd_dstif_1premr = 2; cnd_dstif_1premr--; ){
																for(cnd_tosmode = 2; cnd_tosmode--; ){
																	/* initial */
																	if((ncase%50) == 0)
																		rtlglue_printf("#");
																	
																	memset(&hsb,0,sizeof(hsb_param_t));
																	memset(&src_iftbl,0,sizeof(struct rtl8672_tblAsicDrv_intfParam_s));
																	memset(&dst_iftbl,0,sizeof(struct rtl8672_tblAsicDrv_intfParam_s));
																	memset(&l4p,0,sizeof(rtl8672_tblAsicDrv_l4Param_t));

																	/* mac */
																	if(cnd_hasmac == 0){
																		hsb.hasmac = 0;
																	}else{
																		hsb.hasmac = 1;
																	}
																	/* If. Type */
																	switch(cnd_iftype){
																		case 0:
																			/* Incoming SAR interface, outgoing SAR interface */
																			src_iftbl.IfType = IF_SAR;
																			dst_iftbl.IfType = IF_SAR;
																			continue;
																		case 1:
																			/* Incoming SAR interface, outgoing MAC interface */
																			src_iftbl.IfType = IF_SAR;
																			dst_iftbl.IfType = IF_ETHERNET;
																			break;
																		case 2:
																			/* Incoming SAR interface, outgoing ext. port interface */
																			src_iftbl.IfType = IF_SAR;
																			dst_iftbl.IfType = IF_WIRELESS;
																			break;
																		case  3:
																			/* Incoming MAC interface, outgoing SAR interface */
																			src_iftbl.IfType = IF_ETHERNET;
																			dst_iftbl.IfType = IF_SAR;
																			break;
																		case 4:
																			/* Incoming MAC interface, outgoing MAC interface */
																			src_iftbl.IfType = IF_ETHERNET;
																			dst_iftbl.IfType = IF_ETHERNET;
																			break;
																		case 5:
																			/* Incoming MAC interface, outgoing ext. port interface */
																			src_iftbl.IfType = IF_ETHERNET;
																			dst_iftbl.IfType = IF_WIRELESS;
																			break;
																		case 6:
																			/* Incoming ext. interface, outgoing SAR interface */
																			src_iftbl.IfType = IF_WIRELESS;
																			dst_iftbl.IfType = IF_SAR;
																			break;
																		case 7:
																			/* Incoming ext. interface, outgoing MAC interface */
																			src_iftbl.IfType = IF_WIRELESS;
																			dst_iftbl.IfType = IF_ETHERNET;
																			break;
																		default:
																			/* Incoming ext. interface, outgoing ext. port interface */
																			src_iftbl.IfType = IF_WIRELESS;
																			dst_iftbl.IfType = IF_WIRELESS;
																			break;
																	}
																	/* condition hsb & interface table */
																	if(src_iftbl.IfType == IF_SAR){
																		srcportidx = rtlglue_random()%(SAR_INTFS);
																	}else if(src_iftbl.IfType == IF_ETHERNET){
																		srcportidx = rtlglue_random()%(MAC_RX_INTFS) + SAR_INTFS;
																	}else{
																		srcportidx = rtlglue_random()%(EXT_INTFS) + SAR_INTFS + MAC_TX_INTFS;
																	}
																	if(dst_iftbl.IfType == IF_SAR){
																		do{
																			dstportidx = rtlglue_random()%(SAR_INTFS);
																		}while(dstportidx == srcportidx);
																	}else if(dst_iftbl.IfType == IF_ETHERNET){
																		do{
																			dstportidx = rtlglue_random()%(MAC_TX_INTFS) + SAR_INTFS;
																		}while(dstportidx == srcportidx);
																	}else{
																		do{
																			dstportidx = rtlglue_random()%(EXT_INTFS) + SAR_INTFS + MAC_TX_INTFS;
																		}while(dstportidx == srcportidx);
																	}
																	hsb.srcportidx = srcportidx;
																						
																	if(src_iftbl.IfType == IF_SAR){
																		src_iftbl.L2Encap = rtlglue_random()%2;
																		src_iftbl.LanFCS = rtlglue_random()%2;
																		switch(cnd_srcif_sarhdr){
																			case 0:
																				src_iftbl.SARhdr = ROUTED;
																				if((src_iftbl.L2Encap == 0) && (hsb.hasmac == 1)) continue;
																				break;
																			case 1:
																				src_iftbl.SARhdr = BRIDGED;
																				break;
																			case 2:
																				src_iftbl.SARhdr = MER;
																				break;
																			case 3:
																				src_iftbl.SARhdr = PPPOE;
																				break;
																			default:
																				src_iftbl.SARhdr = PPPOA;
																				if(hsb.hasmac == 1) continue;
																				break;
																		}
																	}
																	else
																	{
																		if(cnd_srcif_sarhdr != 0) continue;
																	}
																	if(dst_iftbl.IfType == IF_SAR){
																		dst_iftbl.L2Encap = rtlglue_random()%2;
																		dst_iftbl.LanFCS = rtlglue_random()%2;
																		switch(cnd_dstif_sarhdr){
																			case 0:
																				dst_iftbl.SARhdr = ROUTED;
																				break;
																			case 1:
																				dst_iftbl.SARhdr = BRIDGED;
																				break;
																			case 2:
																				dst_iftbl.SARhdr = MER;
																				break;
																			case 3:
																				dst_iftbl.SARhdr = PPPOE;
																				break;
																			default:
																				dst_iftbl.SARhdr = PPPOA;
																				break;
																		}
																	}	
																	else
																	{
																		if(cnd_dstif_sarhdr != 0) continue;
																	}
																	/* valn tag */
																	switch(cnd_tif){
																		case 0:
																		/* incoming packet untagged, outgoing packet untagged either */
																			if(cnd_vid != 0) continue;
																			hsb.tif = 0;
																			hsb.vid = 0;
																			hsb.cfi = 0;
																			hsb.pid = 0;
																			l4p.OTagIf = 0;
																			break;
																		case 1:
																			/* incoming packet tagged, outgoing packet untagged */
																			if(hsb.hasmac != 1) continue;
																			hsb.tif = 1;
																			if(cnd_vid == 0){
																				hsb.vid = 0;		/* priority tag */
																				hsb.pid = rtlglue_random()%0x7;
																			}else{
																				hsb.vid = (rtlglue_random()%4094) + 1;		/* vid = 1~4094 */
																				hsb.pid = rtlglue_random()%0x7;
																			}
																			l4p.OTagIf = 0;
																			break;
																		case 2:
																			/* incoming packet untagged, outgoing packet tagged */
																			if(cnd_vid != 0) continue;
																			hsb.tif = 0;
																			hsb.vid = 0;
																			hsb.cfi = 0;
																			hsb.pid = 0;
																			l4p.OTagIf = 1;
																			break;
																		default:
																			/* incoming packet tagged, outgoing packet tagged */
																			if(hsb.hasmac != 1) continue;
																			hsb.tif = 1;
																			if(cnd_vid == 0){
																				hsb.vid = 0;
																				hsb.pid = rtlglue_random()%0x7;
																			}else{
																				hsb.vid = (rtlglue_random()%4094) + 1;		/* vid = 1~4094 */
																				hsb.pid = rtlglue_random()%0x7;
																			}
																			l4p.OTagIf = 1;
																			break;
																	}
																	if(l4p.OTagIf == 1){
																		/* 802.1P remarking */
																		if(cnd_dstif_1premr == 0){
																			l4p.Dot1PRemr = 0;
																			l4p.PriorityID= 0;
																		}else{
																			l4p.Dot1PRemr = 1;
																			do{
																				l4p.PriorityID= rtlglue_random()%0x7;
																			}while(l4p.PriorityID== hsb.pid);
																		}
																		l4p.OVlanID = (rtlglue_random()%4094) + 1;
																	}else{
																		if(cnd_dstif_1premr != 0) continue;
																	}
																	/* pppoe tag */
																	switch(cnd_pif){
																		case 0:
																			/* Incoming packet without pppoe tag, outgoing without pppoe tag */
																			hsb.pif = 0;
																			hsb.sid = 0;
																			l4p.OPppoeIf = 0;
																			l4p.SessionID = 0;
																			l4p.IPppoeIf = 0;
																			break;
																		case 1:
																			/* Incoming packet without pppoe tag, outgoing with pppoe tag */
																			if((dst_iftbl.IfType == IF_SAR) && (dst_iftbl.SARhdr != PPPOE) && (dst_iftbl.SARhdr != BRIDGED)) continue;
																			hsb.pif = 0;
																			hsb.sid = 0;
																			l4p.OPppoeIf = 1;
																			l4p.SessionID = rtlglue_random()%0xffff;
																			l4p.IPppoeIf = 0;
																			break;
																		case 2:
																			/* Incoming packet with pppoe tag, outgoing without pppoe tag */
																			if((src_iftbl.IfType == IF_SAR) && (src_iftbl.SARhdr != PPPOE) && (src_iftbl.SARhdr != BRIDGED)) continue;
																			hsb.pif = 1;
																			hsb.sid = 0x1234;
																			l4p.OPppoeIf = 0;
																			l4p.SessionID = 0;
																			l4p.IPppoeIf = 1;
																			break;
																		default:
																			/* Incoming packet with pppoe tag, outgoing with pppoe tag */
																			if((src_iftbl.IfType == IF_SAR) && (src_iftbl.SARhdr != PPPOE) && (src_iftbl.SARhdr != BRIDGED)) continue;
																			if((dst_iftbl.IfType == IF_SAR) && (dst_iftbl.SARhdr != PPPOE) && (dst_iftbl.SARhdr != BRIDGED)) continue;
																			hsb.pif = 1;
																			hsb.sid = 0x1234;
																			l4p.OPppoeIf = 1;
																			l4p.SessionID = rtlglue_random()%0xffff;
																			l4p.IPppoeIf = 1;
																			break;
																	}
																	/* ether type */
																	hsb.l3type = 1;
																	hsb.l3hwfwdip = 1;
																	if(hsb.hasmac == 1){
																		switch(cnd_ethtp){
																			case 0:
																				/* 0x0800 */
																				if(hsb.pif == 1) continue;
																				hsb.ethtp = (0x0800);
																				break;
																			case 1:
																				/* 0x8864 */
																				if(hsb.pif == 0) continue;
																				hsb.ethtp = (0x8864);
																				break;
																		}
																	}
																	else
																	{
																		hsb.ethtp = 0;
																	}
																			
																	if(dst_iftbl.IfType == IF_WIRELESS){
																		if(cnd_toap == 1){
																			isToAP = 1;
																			MAC_COPY(dst_l2p.MAC,dmac);
																			MAC_COPY(dst_l2p.APMAC,apmac);
																		}else{
																			isToAP = 0;
																			MAC_COPY(dst_l2p.MAC,dmac);
																			MAC_COPY(dst_l2p.APMAC,empty_mac);
																		}
																	}else{
																		if(cnd_toap != 0) continue;
																		MAC_COPY(dst_l2p.MAC,dmac);
																		MAC_COPY(dst_l2p.APMAC,empty_mac);
																	}

																	if(hsb.hasmac == 1){
																		if(cnd_l2invalid == 0){
																			/* L2 table invalid */
																			WRITE_MEM32(ALECR_L2TEB,(READ_MEM32(ALECR_L2TEB) & ~L2TEB_EN_MASK));
																		}else{
																			WRITE_MEM32(ALECR_L2TEB,(READ_MEM32(ALECR_L2TEB) | L2TEB_EN_MASK));
																			/* DMAC == GMAC */
																			MAC_COPY(src_iftbl.GMAC,dmac);
																		}
																	}else{
																		if(cnd_l2invalid != 0) continue;
																	}

																	if(cnd_l4proto == 0){
																		/* TCP */
																		l4p.Proto = IPPROTO_TCP;
																		expect_hsa.l4proto = IPPROTO_TCP;
																		expect_hsa.l4cs = 0x4444;
																		hsb.udpnocs = 0;
																	}else if(cnd_l4proto == 1){
																		/* UDP */
																		l4p.Proto = IPPROTO_UDP;
																		expect_hsa.l4proto = IPPROTO_UDP;
																		expect_hsa.l4cs = 0x4444;
																		hsb.udpnocs = 0;
																	}else if(cnd_l4proto == 2){
																		/* UDP no cs */
																		l4p.Proto = IPPROTO_UDP;
																		expect_hsa.l4proto = IPPROTO_UDP;
																		expect_hsa.l4cs = 0;
																		hsb.l4cs = 0;
																		hsb.udpnocs = 1;
																	}

																	/* ttl decrsase */
																	hsb.ttl = rtlglue_random()%10 + 2;
																	if(cnd_l3ttlde == 0){
																		l4p.TtlDe = 0;
																		expect_hsa.ttl= hsb.ttl;
																	}else{
																		l4p.TtlDe = 1;
																		expect_hsa.ttl = hsb.ttl - 1;
																	}
																	expect_hsa.tosremr = 0;
																	if(cnd_tosmode == 1){
																		hsb.tos = rtlglue_random()%0xff;
																		expect_hsa.tos = hsb.tos;
																		l4p.Tos = rtlglue_random()%0xff;
																		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) | QOSECN_TOSMODE_MASK));
																		/* tosmode : 3 + 5 */
																		if(cnd_l3tosremr == 1){
																			l4p.TosRemr = 1;
																			expect_hsa.tos = (hsb.tos & 0xe0) | (l4p.Tos & 0x1f);
																			expect_hsa.tosremr = 1;
																		}else{
																			l4p.TosRemr = 0;
																		}
																		if(cnd_dscpremr == 1){
																			l4p.PrecedRemr = 1;
																			expect_hsa.tos = (expect_hsa.tos & 0x1f) | (l4p.Tos & 0xe0);
																			expect_hsa.tosremr = 1;
																		}else{
																			l4p.PrecedRemr = 0;
																		}
																	}else{
																		hsb.tos = rtlglue_random()%0xff & ~0x3;
																		expect_hsa.tos = hsb.tos;
																		l4p.Tos = rtlglue_random()%0xff;
																		WRITE_MEM32(ALECR_QOSECN,(READ_MEM32(ALECR_QOSECN) & ~QOSECN_TOSMODE_MASK));
																		/* dscp : 3 + 3 + 2(ECN) */
																		if(cnd_l3tosremr == 1){
																			expect_hsa.tos = (hsb.tos & 0xe3) | (l4p.Tos & 0x1c);
																			expect_hsa.tosremr = 1;
																			l4p.TosRemr = 1;
																		}else{
																			l4p.TosRemr = 0;
																		}
																		if(cnd_dscpremr == 1){
																			expect_hsa.tos = (expect_hsa.tos & 0x1f) | (l4p.Tos & 0xe0);
																			expect_hsa.tosremr = 1;
																			l4p.PrecedRemr = 1;
																		}else{
																			l4p.PrecedRemr = 0;
																		}
																	}

																	/* error case ignore */
																	if((src_iftbl.IfType == IF_SAR) && (dst_iftbl.IfType == IF_SAR)) continue;
																	if((hsb.ethtp == (0x8864)) && (hsb.pif != 1)) continue;
																	if((hsb.ethtp != (0x8864)) && (hsb.pif == 1)) continue;
																	if((hsb.ethtp != (0x8864)) && (hsb.ethtp != (0x0800)) && (src_iftbl.IfType != IF_SAR) && (dst_iftbl.IfType == IF_SAR)) continue;
																	if((hsb.ethtp != (0x8864)) && (hsb.ethtp != (0x0800)) && (src_iftbl.IfType == IF_SAR) && (dst_iftbl.IfType != IF_SAR)) continue;
																	if((hsb.pif != 1) && (expect_hsa.pif == 1) && (hsa.l3type != 1)) continue;
																	if((hsb.pif == 1) && (expect_hsa.pif != 1) && (hsa.l3type != 1)) continue;
																	if((dst_iftbl.IfType == IF_SAR) && (l4p.OPppoeIf != 0) && (dst_iftbl.SARhdr != BRIDGED) && (dst_iftbl.SARhdr != PPPOE)) continue;
																	if((dst_iftbl.IfType == IF_SAR) && (l4p.OPppoeIf != 1) && (dst_iftbl.SARhdr == PPPOE)) continue;
																	if((l4p.OTagIf == 1) && ((dst_iftbl.IfType == IF_SAR) && ((dst_iftbl.SARhdr == ROUTED) && (dst_iftbl.L2Encap == VC_MULTIPLEXING)))) continue;
																	if((l4p.OTagIf == 1) && ((dst_iftbl.IfType == IF_SAR) && (dst_iftbl.SARhdr == PPPOA) )) continue;
																	if(((src_iftbl.IfType == IF_SAR) && ((src_iftbl.SARhdr == BRIDGED) || (src_iftbl.SARhdr == MER) || (src_iftbl.SARhdr == PPPOE) )) && (hsb.hasmac != 1)) continue;
																	if((src_iftbl.IfType != IF_SAR) && (hsb.hasmac == 0)) continue;
																					
																	/* other hsb filed value */
																	hsb.dip = 0xc0a89afc;
																	hsb.sip = 0xc0a89a3c;
																	hsb.dport = 0x88;
																	hsb.sport = 0x99;
																	hsb.ipfrag = 0;
																	hsb.l3cs = 0x3333;
																	hsb.l3csok = 1;
																	hsb.l3len = rtlglue_random()%1500;
																	hsb.l3offset = 24;		/* altering value, it's not the issue */
																	hsb.l4cs = 0x4444;
																	hsb.l4csok = 1;
																	hsb.l4proto = ((rtlglue_random()%2) == 0)?IPPROTO_TCP:IPPROTO_UDP;
																	MAC_COPY(hsb.dmac,dmac);
																	MAC_COPY(hsb.smac,smac);
																	MAC_COPY(hsb.mac3,mac3);
																	MAC_COPY(hsb.mac4,mac4);
																	hsb.tcpflag = ((rtlglue_random()%0xff) & ~0xc7);
																	rtl8672_setAsicHSB(&hsb);
																	
																	/* source port interface table */
																	MAC_COPY(src_l2p.MAC,smac);
																	src_iftbl.L2BridgeEnable = 1;
																	src_iftbl.MTU = 1518;
																	do{
																		src_iftbl.PortPriorityID= rtlglue_random()%0x7;
																	}while((src_iftbl.PortPriorityID == hsb.pid) || (src_iftbl.PortPriorityID == src_l2p.PriorityIDRemr));
																	do{
																		src_iftbl.PortVlanID = ((rtlglue_random()%4094) + 1);
																	}while((src_iftbl.PortVlanID == hsb.vid) || (src_iftbl.PortVlanID == l4p.OVlanID));
																	src_iftbl.SrcPortFilter = 1;
																	src_iftbl.LogicalID = srcportidx;
																	src_iftbl.L2Encap = rtlglue_random()%2;
																	src_iftbl.LanFCS = rtlglue_random()%2;
																	src_iftbl.CompPPP = rtlglue_random()%2;
																	src_iftbl.AcceptTagged = 1;
																	src_iftbl.AcceptUntagged = 1;
																	rtl8672_setAsicNetInterface(srcportidx,&src_iftbl);

																	/* dest. port interface table */
																	dst_iftbl.AcceptTagged = 1;
																	dst_iftbl.AcceptUntagged = 1;
																	dst_iftbl.L2BridgeEnable = 1;
																	dst_iftbl.MTU = 1518;
																	dst_iftbl.PortPriorityID= rtlglue_random()%0x7;
																	dst_iftbl.PortVlanID = ((rtlglue_random()%4094) + 1);
																	dst_iftbl.SrcPortFilter = 1;
																	dst_iftbl.LogicalID = dstportidx;
																	if(dst_iftbl.IfType == IF_SAR){
																		dst_iftbl.LanFCS = rtlglue_random()%2;
																	}else{
																		dst_iftbl.L2Encap = 0;
																		dst_iftbl.LanFCS = 0;
																	}
																	if(hsb.pif == 1){
																		dst_iftbl.CompPPP = rtlglue_random()%2;
																	}else{
																		dst_iftbl.CompPPP = 0;
																	}
																	MAC_COPY(dst_iftbl.GMAC,gmac);
																	rtl8672_setAsicNetInterface(dstportidx,&dst_iftbl);

																	/* pnpp table selection */
																	if(hsb.ethtp == (0x0800)){
																		if(src_iftbl.IfType == IF_SAR){
																			rtl8672_getAsicProtocolBasedVLAN(0, &pnpp);
																		}else if(src_iftbl.IfType == IF_ETHERNET){
																			rtl8672_getAsicProtocolBasedVLAN(1, &pnpp);
																		}else{
																			rtl8672_getAsicProtocolBasedVLAN(2, &pnpp);
																		}
																	}else if(hsb.ethtp == (0x8864)){
																		if(src_iftbl.IfType == IF_SAR){
																			rtl8672_getAsicProtocolBasedVLAN(3, &pnpp);
																		}else if(src_iftbl.IfType == IF_ETHERNET){
																			rtl8672_getAsicProtocolBasedVLAN(4, &pnpp);
																		}else{
																			rtl8672_getAsicProtocolBasedVLAN(5, &pnpp);
																		}
																	}else if(hsb.ethtp == (0x8863)){
																		rtl8672_getAsicProtocolBasedVLAN(6, &pnpp);
																	}else{
																		rtl8672_getAsicProtocolBasedVLAN(7, &pnpp);
																	}

																	/* Layer 2 entery for smac */
																	src_l2p.Age = rtlglue_random()%2;
																	MAC_COPY(src_l2p.APMAC,empty_mac);
																	src_l2p.Port = hsb.srcportidx;
																	src_l2p.SADrop = 0;
																	src_l2p.Valid = 1;
																	if((hsb.tif == 1) && (hsb.vid != 0)){
																		src_l2p.VlanID = hsb.vid;
																	}else{
																		if((hsb.ethtp == 0x0800) || (hsb.ethtp == 0x8864) || (hsb.ethtp == 0x8863) || (hsb.ethtp == 0x86dd)){
																			src_l2p.VlanID = pnpp.VlanID;
																		}else{
																			src_l2p.VlanID = src_iftbl.PortVlanID;
																		}
																	}
																	src_l2p.PriorityIDRemr = rtlglue_random()%0x7;
																	src_l2p.Dot1PRemr = 1;
																	ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&src_l2p, OPT_AUTO));

																	/* Layer 2 entry for dmac */
																	dst_l2p.Age = rtlglue_random()%2;
																	dst_l2p.Port = dstportidx;
																	dst_l2p.SADrop = 0;
																	dst_l2p.Valid = 1;
																	dst_l2p.VlanID = l4p.OVlanID;
																	ASSERT(SUCCESS == rtl8672_addAsicL2Entry(&dst_l2p, OPT_AUTO));

																	/* Layer 4 table */
																	l4p.Age = rtlglue_random()%2;
																	MAC_COPY(l4p.NHMAC,dmac);
																	l4p.DstIP = hsb.dip;
																	l4p.SrcIP = hsb.sip;
																	l4p.DstPort = hsb.dport;
																	l4p.SrcPort = hsb.sport;
																	l4p.DstPortIdx = dstportidx;
																	l4p.IPppoeIf = hsb.pif;
																	l4p.SessionID = hsb.sid;
																	if((hsb.ethtp != 0x0800) && (hsb.ethtp != 0x8864) && (hsb.ethtp != 0x8863) && (hsb.ethtp != 0x86dd)){
																		l4p.IVlanID = src_iftbl.PortVlanID;
																	}
																	else
																	{
																		/* pnp table include those pre-condition entry*/
																		if((hsb.tif == 1) && (hsb.vid != 0)){
																			l4p.IVlanID = hsb.vid;
																		}else{
																			l4p.IVlanID = pnpp.VlanID;
																		}
																	}
																	l4p.IVlanIDChk = rtlglue_random()%2;
																	l4p.NewDstIP = 0xc0a89af3;
																	l4p.NewSrcIP = 0xc0a89a33;
																	l4p.NewDstPort = 0xdd;
																	l4p.NewSrcPort = 0xee;
																	l4p.Proto = hsb.l4proto;
																	l4p.Valid = 1;
																	ASSERT(SUCCESS == rtl8672_addAsicL4Entry(&l4p, OPT_AUTO));

																	/* excepted hsa */
																	expect_hsa.outiftype = dst_iftbl.IfType;
																	expect_hsa.outl2encap = dst_iftbl.L2Encap;
																	expect_hsa.outlanfcs = dst_iftbl.LanFCS;
																	expect_hsa.pppprotolen = dst_iftbl.CompPPP;
																	expect_hsa.outsarhdr = dst_iftbl.SARhdr;
																	expect_hsa.droppacket = 0;
																	if(expect_hsa.outiftype == IF_SAR){
																		if(expect_hsa.outsarhdr == PPPOE){
																			expect_hsa.pif = 1;
																		}else if(expect_hsa.outsarhdr == BRIDGED){
																			expect_hsa.pif = l4p.OPppoeIf;
																		}else{
																			expect_hsa.pif = 0;
																		}
																	}else{
																		expect_hsa.pif = l4p.OPppoeIf;
																	}
																	expect_hsa.l3change = 1;
																	expect_hsa.l4change = 1;
																	expect_hsa.fromcpu = 0;
																	expect_hsa.tocpu = 0;
																	expect_hsa.iniftype = src_iftbl.IfType;
																	expect_hsa.tif = l4p.OTagIf;
																	expect_hsa.linkid = 0;
																	expect_hsa.dstportidx = dstportidx;
																	expect_hsa.srcportidx = srcportidx;
																	expect_hsa.ethtp = hsb.ethtp;
																	if((hsb.hasmac == 0) && ((dst_iftbl.IfType == IF_WIRELESS) ||(dst_iftbl.IfType == IF_ETHERNET)))
																		expect_hsa.ethtp = 0x0800;
																	if((hsb.pif == 1) && (expect_hsa.pif == 0)){
																		expect_hsa.ethtp = 0x0800;
																	}else if((hsb.pif == 0) && (expect_hsa.pif == 1)){
																		expect_hsa.ethtp = 0x8864;
																	}

																	if(expect_hsa.outiftype == IF_WIRELESS){
																		/* 802.11 */
																		expect_hsa.framectrl = hsb.framectrl | 0x2 ;
																		if (isToAP == 1){
																			/* to AP */
																			expect_hsa.framectrl |=  0x1 ;
																			MAC_COPY(expect_hsa.dmac,apmac);
																			MAC_COPY(expect_hsa.smac,dst_iftbl.GMAC);
																			MAC_COPY(expect_hsa.mac3,dst_l2p.MAC);
																			MAC_COPY(expect_hsa.mac4,dst_iftbl.GMAC);
																		}else{
																			/* to station */
																			expect_hsa.framectrl &= ~0x1 ;
																			MAC_COPY(expect_hsa.dmac,dst_l2p.MAC);
																			MAC_COPY(expect_hsa.smac,dst_iftbl.GMAC);
																			MAC_COPY(expect_hsa.mac3,dst_iftbl.GMAC);
																			MAC_COPY(expect_hsa.mac4,empty_mac);
																		}
																	}
																	else
																	{
																		expect_hsa.framectrl = hsb.framectrl & ~0x3;		/* clear toDS & fromDS */
																		MAC_COPY(expect_hsa.dmac,dst_l2p.MAC);
																		MAC_COPY(expect_hsa.smac,dst_iftbl.GMAC);
																		MAC_COPY(expect_hsa.mac3,hsb.mac3);
																		MAC_COPY(expect_hsa.mac4,hsb.mac4);
																	}
																	
																	expect_hsa.sid = hsb.sid;
																	/* priority id decision */
																	if(hsb.tif == 1){
																		expect_hsa.pid = hsb.pid;
																		expect_hsa.cfi = hsb.cfi;
																	}else{
																		if((hsb.ethtp == 0x0800) || (hsb.ethtp == 0x8864) || (hsb.ethtp == 0x8863) || (hsb.ethtp == 0x86dd)){																		
																			expect_hsa.pid = pnpp.PriorityID;
																		}else{
																			expect_hsa.pid = src_iftbl.PortPriorityID;
																		}
																		expect_hsa.cfi = 0;
																	}
																	if(l4p.OTagIf == 0){
																		expect_hsa.tif = 0;
																	}else{
																		expect_hsa.tif = 1;
																		if(l4p.Dot1PRemr == 1){
																			expect_hsa.pid = l4p.PriorityID;
																		}
																	}
																	/* vlan id decision */
																	expect_hsa.vid = l4p.OVlanID;
																	if(l4p.OTagIf == 0){
																		expect_hsa.tif = 0;
																	}else{
																		expect_hsa.tif = 1;
																	}
																	expect_hsa.vidremark = 1;
																	expect_hsa.l3type = hsb.l3type;
																	expect_hsa.inl3offset = hsb.l3offset;
																	expect_hsa.inl3len = hsb.l3len;
																	expect_hsa.l3cs = hsb.l3cs;
																	expect_hsa.sip = l4p.NewSrcIP;
																	expect_hsa.dip = l4p.NewDstIP;
																	expect_hsa.sport = l4p.NewSrcPort;
																	expect_hsa.dport = l4p.NewDstPort;
																	expect_hsa.l4cs = hsb.l4cs;
																	expect_hsa.l4proto = hsb.l4proto;
																	expect_hsa.reason = 0;
																	
																	/* Layer34 Checksum Adjust */
																	int32 l3cs_dt = 0;

																	l3cs_dt += (hsb.ttl - expect_hsa.ttl) << 8;
																	l3cs_dt += (hsb.tos - expect_hsa.tos);
																	l3cs_dt += (hsb.sip & 0x0000ffff) - (expect_hsa.sip & 0x0000ffff);
																	l3cs_dt += (hsb.sip >> 16) - (expect_hsa.sip >> 16);
																	l3cs_dt += (hsb.dip & 0x0000ffff) - (expect_hsa.dip & 0x0000ffff);
																	l3cs_dt += (hsb.dip >> 16) - (expect_hsa.dip >> 16);
																	
																	l3cs_dt += hsb.l3cs;
																	if (l3cs_dt < 0) {
																		l3cs_dt = -l3cs_dt;
																		l3cs_dt = (l3cs_dt >> 16) + (l3cs_dt & 0x0000ffff);
																		l3cs_dt += (l3cs_dt >> 16);
																		hsa.l3cs = ~l3cs_dt;
																	} else {
																		l3cs_dt = (l3cs_dt >> 16) + (l3cs_dt & 0x0000ffff);
																		l3cs_dt += (l3cs_dt >> 16);
																		expect_hsa.l3cs = l3cs_dt;
																	}
																	if (expect_hsa.l4change == 1 && hsb.udpnocs != 1) {
																		int32 l4cs_dt = 0;

																		l4cs_dt += (hsb.sip & 0x0000ffff) - (expect_hsa.sip & 0x0000ffff);
																		l4cs_dt += (hsb.sip >> 16) - (expect_hsa.sip >> 16);
																		l4cs_dt += (hsb.dip & 0x0000ffff) - (expect_hsa.dip & 0x0000ffff);
																		l4cs_dt += (hsb.dip >> 16) - (expect_hsa.dip >> 16);
																		l4cs_dt += (hsb.sport - expect_hsa.sport);
																		l4cs_dt += (hsb.dport - expect_hsa.dport);

																		l4cs_dt += hsb.l4cs;
																		if (l4cs_dt < 0) {
																			l4cs_dt = -l4cs_dt;
																			l4cs_dt = (l4cs_dt >> 16) + (l4cs_dt & 0x0000ffff);
																			l4cs_dt += (l4cs_dt >> 16);
																			expect_hsa.l4cs = ~l4cs_dt;
																		} else {
																			l4cs_dt = (l4cs_dt >> 16) + (l4cs_dt & 0x0000ffff);
																			l4cs_dt += (l4cs_dt >> 16);
																			expect_hsa.l4cs = l4cs_dt;
																		}

																	}

																	/* ALE working */
																	MT_ALE_DEBUG("srcportidx=%d dstportidx=%d\n",srcportidx,dstportidx);
																	
																	WRITE_MEM32(DBG_CTRL, (DBG_TEST_MODE<<DBG_MODE_OFFSET) |(DBG_ALE<<DBG_ITEM_OFFSET));			/*set test mode and select parser item */
																	WRITE_MEM32(DBG_CTRL, READ_MEM32(DBG_CTRL) | (DBG_TRIGGER<<DBG_TRIGGER_OFFSET));	/* trigger test mode*/
																	do{
																		/* polling trigger bit */
																	}while((READ_MEM32(DBG_CTRL) & (DBG_TRIGGER<<DBG_TRIGGER_OFFSET)) != 0);
																	rtl8672_getAsicHSA(&hsa);

																	if(compHsa(&hsa,&expect_hsa)==FAILED) 
																	{
																		errcnt++;
																		rtlglue_printf("Error Case : %d!!\n",ncase);
																		return FAILED; 
																	}

																	ASSERT(SUCCESS == rtl8672_delAsicL2Entry(&dst_l2p, OPT_AUTO));
																	ASSERT(SUCCESS == rtl8672_delAsicL2Entry(&src_l2p, OPT_AUTO));
																	ASSERT(SUCCESS == rtl8672_delAsicL4Entry(&l4p,OPT_AUTO));
																		
																	ncase++;
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	rtlglue_printf("************** Total case:%d\n",ncase);	
	testALE_Reset();
	testALE_Exit();
	return SUCCESS;
}
#endif
/* hsb reset function for test case */
int32 _ale_reset_hsb(hsb_param_t *hsb)
{
	memset(hsb,0,sizeof(hsb_param_t));

	hsb->fromcpu = 0;
	hsb->hasmac = 1;
	hsb->tif = 0;
	hsb->pif = 0;
	hsb->srcportidx = 0;
	hsb->l3hwfwdip = 1;
	hsb->linkid = 0;
	hsb->pid = 0;
	hsb->cfi = 0;
	hsb->vid = 0;
	hsb->framectrl = 0;
	hsb->ethtp = (0x0800);
	hsb->dmac[0] = 0x0; hsb->dmac[1] = 0xd1; hsb->dmac[2] = 0xd2;
	hsb->dmac[3] = 0xd3; hsb->dmac[4] = 0xd4; hsb->dmac[5] = 0xd5;

	hsb->smac[0] = 0x0; hsb->smac[1] = 0xa1; hsb->smac[2] = 0xa2;
	hsb->smac[3] = 0xa3; hsb->smac[4] = 0xa4; hsb->smac[5] = 0xa5;

	hsb->mac3[0] = 0x0; hsb->mac3[1] = 0x31; hsb->mac3[2] = 0x32;
	hsb->mac3[3] = 0x33; hsb->mac3[4] = 0x34; hsb->mac3[5] = 0x35;

	hsb->sid = 0;
	hsb->mac4[0] = 0x0; hsb->mac4[1] = 0x41; hsb->mac4[2] = 0x42;
	hsb->mac4[3] = 0x43; hsb->mac4[4] = 0x44; hsb->mac4[5] = 0x45;
	hsb->l3cs = 0x1234;	
	hsb->l3len = 111;
	hsb->sip = htonl(0xc0a89a7b);		/* 192.168.154.123 */
	hsb->dip = htonl(0xc0a89aea);		/* 192.168.154.234 */
	hsb->tos = 0;
	hsb->l3offset = 0;
	hsb->ttl = 2;
	hsb->l3type = 0x1;
	hsb->ipfrag = 0;
	hsb->l3csok = 1;
	hsb->udpnocs = 0;
	hsb->l4csok = 1;
	hsb->reason = 0;
	hsb->l4proto = IPPROTO_TCP;
	hsb->tcpflag = 0;
	hsb->l4cs = 0x5678;
	hsb->sport = 1111;
	hsb->dport = 3333;

	return SUCCESS;
}

