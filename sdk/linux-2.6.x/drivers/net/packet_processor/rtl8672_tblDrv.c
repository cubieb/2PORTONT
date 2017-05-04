/*
 * Copyright (c) 2007 Realtek Semiconductor Corporation.
 *
 * Program : Source File of RTL8672 Table Driver
 * Abstract :
 * Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
 * $Id: rtl8672_tblDrv.c,v 1.3 2012/07/12 08:03:56 czyao Exp $
 */

#include "rtl_types.h"
#include "rtl_glue.h"
#include "rtl_utils.h"
#include "rtl8672_asicregs.h"
#include "rtl8672_tblDrv.h"
#include "modelTrace.h"

int rtl8672_L2EntryLookup(unsigned char *mac,int vlanid,rtl8672_tblAsicDrv_l2Param_t *l2pt)
{
	uint32 hashidx;
	uint32 way, ale_l2way;

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#ifdef SRAM_MAPPING_ENABLED	
	/* SRAM */
	hashidx = rtl8672_L2Hash_Sram(mac, vlanid);
	
	for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)
	{
		if(rtl8672_getAsicL2Table_Sram(hashidx, way, l2pt)==FAILED) return FAILED;
		if ((l2pt->Valid == 1) && (l2pt->VlanID==vlanid))
		{
			if(memcmp(l2pt->MAC,mac,6)==0)
			{
				return SUCCESS;
			}
		}
	}
#endif
#endif
	
	/* SDRAM */
	hashidx = rtl8672_L2Hash(mac, vlanid);
	rtl8672_getAsicL2Way(&ale_l2way);
	for (way = 0; way < ale_l2way; way++)
	{
		if(rtl8672_getAsicL2Table(hashidx, way, l2pt)==FAILED) return FAILED;

		if ((l2pt->Valid == 1) && (l2pt->VlanID==vlanid))
		{
			if(memcmp(l2pt->MAC,mac,6)==0)
			{
				return SUCCESS;
			}
		}
	}

	return FAILED;

}


 int rtl8672_L2EntrySearch(int (*match_func)(void *, void *), void *user_data)
 {
	 uint32 hashidx, l2hib;
	 uint32 way, ale_l2way;
	 rtl8672_tblAsicDrv_l2Param_t l2;

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#ifdef SRAM_MAPPING_ENABLED
	 /* SRAM */
	 for (hashidx = 0; hashidx < ALE_L2TABLE_SRAM_IDX; hashidx++) {
		 for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)
		 {
			 if(rtl8672_getAsicL2Table_Sram(hashidx, way, &l2)==FAILED) return FAILED;

			 if (!l2.Valid)
			 	continue;
			 
			 if (match_func(&l2, user_data))
			 {
				 return SUCCESS;
			 }
			 
	 	}
	 }
#endif
#endif	 
	 /* SDRAM */
	 rtl8672_getAsicL2HashIdxBits(&l2hib);
	 rtl8672_getAsicL2Way(&ale_l2way);
	 for (hashidx = 0; hashidx < (0x1 << l2hib); hashidx++) {
		 for (way = 0; way < ale_l2way; way++)
		 {
			 if(rtl8672_getAsicL2Table(hashidx, way, &l2)==FAILED) return FAILED;

			 if (!l2.Valid)
			 	continue;
 
			 if (match_func(&l2, user_data))
			 {
				 return SUCCESS;
			 }
		 	
	 	}
	 }
	 return FAILED;
 
 }


 int32 rtl8672_addAsicL2Entry(rtl8672_tblAsicDrv_l2Param_t *l2p, uint32 option)
{
	uint32 hashidx;
	uint32 way, ale_l2way;
	rtl8672_tblAsicDrv_l2Param_t l2pt;

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#ifdef SRAM_MAPPING_ENABLED	
	/* SRAM */
	hashidx = rtl8672_L2Hash_Sram(&l2p->MAC[0], l2p->VlanID);
	if (option == OPT_AUTO || option == OPT_SRAMONLY)
	{
		for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)
		{
			rtl8672_getAsicL2Table_Sram(hashidx, way, &l2pt);
			MT_ALE_DEBUG("[SRAM]Add L2 entry hashidx = %d valid = %d vlanid = %d mac=0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\n",hashidx,l2pt.Valid,l2p->VlanID,l2p->MAC[0],l2p->MAC[1],l2p->MAC[2],l2p->MAC[3],l2p->MAC[4],l2p->MAC[5]);
			if (l2pt.Valid == 0)
			{
				return rtl8672_setAsicL2Table_Sram(hashidx, way, l2p);
			}
		}
	}
#endif
#endif	
	/* SDRAM */
	hashidx = rtl8672_L2Hash(&l2p->MAC[0], l2p->VlanID);
	rtl8672_getAsicL2Way(&ale_l2way);
	if (option == OPT_AUTO || option == OPT_SDRAMONLY)
	{
		for (way = 0; way < ale_l2way; way++)
		{
			rtl8672_getAsicL2Table(hashidx, way, &l2pt);
			MT_ALE_DEBUG("[SDRAM]Add L2 entry hashidx = %d valid = %d vlanid = %d mac=0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\n",hashidx,l2pt.Valid,l2p->VlanID,l2p->MAC[0],l2p->MAC[1],l2p->MAC[2],l2p->MAC[3],l2p->MAC[4],l2p->MAC[5]);
			if (l2pt.Valid == 0)
			{
				return rtl8672_setAsicL2Table(hashidx, way, l2p);
			}
		}
	}
	
	return FAILED;
}

int32 rtl8672_delAsicL2Entry(rtl8672_tblAsicDrv_l2Param_t *l2p, uint32 option)
{
	uint32 hashidx;
	uint32 way, ale_l2way;
	rtl8672_tblAsicDrv_l2Param_t l2pt;

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#ifdef SRAM_MAPPING_ENABLED
	/* SRAM */
	hashidx = rtl8672_L2Hash_Sram(&l2p->MAC[0], l2p->VlanID);
	if (option == OPT_AUTO || option == OPT_SRAMONLY)
	{
		for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)
		{
			rtl8672_getAsicL2Table_Sram(hashidx, way, &l2pt);
			if ((l2pt.MAC[0] == l2p->MAC[0]) &&
				(l2pt.MAC[1] == l2p->MAC[1]) &&
				(l2pt.MAC[2] == l2p->MAC[2]) &&
				(l2pt.MAC[3] == l2p->MAC[3]) &&
				(l2pt.MAC[4] == l2p->MAC[4]) &&
				(l2pt.MAC[5] == l2p->MAC[5]) &&
				(l2pt.VlanID == l2p->VlanID) &&
				(l2pt.Valid == 1))
			{
				memset((void *)&l2pt, 0, sizeof(rtl8672_tblAsicDrv_l2Param_t));
				MT_ALE_DEBUG("[SRAM]Del L2 entry hashidx = %d way = %d\n", hashidx,way);
				return rtl8672_setAsicL2Table_Sram(hashidx, way, &l2pt);
			}
		}
	}
#endif
#endif	
	/* SDRAM */
	hashidx = rtl8672_L2Hash(&l2p->MAC[0], l2p->VlanID);
	rtl8672_getAsicL2Way(&ale_l2way);
	if (option == OPT_AUTO || option == OPT_SDRAMONLY)
	{
		for (way = 0; way < ale_l2way; way++)
		{
			rtl8672_getAsicL2Table(hashidx, way, &l2pt);
			if ((l2pt.MAC[0] == l2p->MAC[0]) &&
				(l2pt.MAC[1] == l2p->MAC[1]) &&
				(l2pt.MAC[2] == l2p->MAC[2]) &&
				(l2pt.MAC[3] == l2p->MAC[3]) &&
				(l2pt.MAC[4] == l2p->MAC[4]) &&
				(l2pt.MAC[5] == l2p->MAC[5]) &&
				(l2pt.VlanID == l2p->VlanID) &&
				(l2pt.Valid == 1))
			{
				memset((void *)&l2pt, 0, sizeof(rtl8672_tblAsicDrv_l2Param_t));
				MT_ALE_DEBUG("[SDRAM]Del L2 entry hashidx = %d \n", hashidx);
				return rtl8672_setAsicL2Table(hashidx, way, &l2pt);
			}
		}
	}
	
	return FAILED;
}

int32 rtl8672_addAsicL4Entry(rtl8672_tblAsicDrv_l4Param_t *l4p, uint32 option)
{
	uint32 hashidx;
	uint32 way, ale_l4way;
	rtl8672_tblAsicDrv_l4Param_t l4pt;

#ifdef SRAM_MAPPING_ENABLED	
	/* SRAM */
	hashidx = rtl8672_L4Hash_Sram(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);
//rtlglue_printf("------ hashidx=%d\n",hashidx);
	if (option == OPT_AUTO || option == OPT_SRAMONLY)
	{
		for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)
		{
			rtl8672_getAsicL4Table_Sram(hashidx, way, &l4pt);
			if (l4pt.Valid == 0)
			{
				MT_ALE_DEBUG("[SRAM]add L4 entry hashidx = %d \n", hashidx);			
				return rtl8672_setAsicL4Table_Sram(hashidx, way, l4p);
			}
		}
	}
#endif
	
	/* SDRAM */
	hashidx = rtl8672_L4Hash(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);
	rtl8672_getAsicL4Way(&ale_l4way);
	if (option == OPT_AUTO || option == OPT_SDRAMONLY)
	{
		for (way = 0; way < ale_l4way; way++)
		{
			rtl8672_getAsicL4Table(hashidx, way, &l4pt);
			if (l4pt.Valid == 0)
			{
				MT_ALE_DEBUG("[SDRAM]add L4 entry hashidx = %d \n", hashidx);
				return rtl8672_setAsicL4Table(hashidx, way, l4p);
			}
		}
	}
	
	return FAILED;
}

int32 rtl8672_delAsicL4Entry(rtl8672_tblAsicDrv_l4Param_t *l4p, uint32 option)
{
	uint32 hashidx;
	uint32 way, ale_l4way;
	rtl8672_tblAsicDrv_l4Param_t l4pt;

#ifdef SRAM_MAPPING_ENABLED	
	/* SRAM */
	hashidx = rtl8672_L4Hash_Sram(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);
	if (option == OPT_AUTO || option == OPT_SRAMONLY)
	{
		for (way = 0; way < ALE_L4TABLE_SRAM_WAY; way++)
		{
			rtl8672_getAsicL4Table_Sram(hashidx, way, &l4pt);
			if ((l4pt.Proto == l4p->Proto) &&
				(l4pt.SrcIP == l4p->SrcIP) &&
				(l4pt.DstIP == l4p->DstIP) &&
				(l4pt.SrcPort == l4p->SrcPort) &&
				(l4pt.DstPort == l4p->DstPort) &&
				(l4pt.Valid == 1))
			{
				memset((void *)&l4pt, 0, sizeof(rtl8672_tblAsicDrv_l4Param_t));
				MT_ALE_DEBUG("[SRAM]Del L4 entry hashidx = %d \n", hashidx);
				return rtl8672_setAsicL4Table_Sram(hashidx, way, &l4pt);
			}
		}
	}
#endif
	
	/* SDRAM */
	hashidx = rtl8672_L4Hash(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);
	rtl8672_getAsicL4Way(&ale_l4way);
	if (option == OPT_AUTO || option == OPT_SDRAMONLY)
	{
		for (way = 0; way < ale_l4way; way++)
		{
			rtl8672_getAsicL4Table(hashidx, way, &l4pt);
			if ((l4pt.Proto == l4p->Proto) &&
				(l4pt.SrcIP == l4p->SrcIP) &&
				(l4pt.DstIP == l4p->DstIP) &&
				(l4pt.SrcPort == l4p->SrcPort) &&
				(l4pt.DstPort == l4p->DstPort) &&
				(l4pt.Valid == 1))
			{
				memset((void *)&l4pt, 0, sizeof(rtl8672_tblAsicDrv_l4Param_t));
				MT_ALE_DEBUG("[SDRAM]Del L4 entry hashidx = %d \n", hashidx);
				return rtl8672_setAsicL4Table(hashidx, way, &l4pt);
			}
		}
	}
	
	return FAILED;
}


int32 rtl8672_flushAsicL4Entry(void){	
	uint32 hashidx;	
	uint32 way, ale_l4way;	
	rtl8672_tblAsicDrv_l4Param_t l4pt;		/* SRAM */	
	//hashidx = rtl8672_L4Hash_Sram(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);	
#ifdef SRAM_MAPPING_ENABLED
	for(hashidx=0 ; hashidx<255; hashidx++){		
		//rtlglue_printf("------ hashidx=%d\n",hashidx);		
		for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)		
		{			
			rtl8672_getAsicL4Table_Sram(hashidx, way, &l4pt);			
			l4pt.Valid = 0;			
			rtl8672_setAsicL4Table_Sram(hashidx, way, &l4pt);		
		}	
	}	
#endif
	/* SDRAM */	
	//hashidx = rtl8672_L4Hash(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);	
	for(hashidx=0 ; hashidx>255; hashidx++){		
		rtl8672_getAsicL4Way(&ale_l4way);		
		for (way = 0; way < ale_l4way; way++)		
		{			
			rtl8672_getAsicL4Table(hashidx, way, &l4pt);			
			l4pt.Valid = 0;			
			rtl8672_setAsicL4Table(hashidx, way, &l4pt);		
		}	
	}	
	return SUCCESS;
}


int rtl8672_searchAsicL4Entry(int (*match_func)(void *, void *), void *user_data)
{
	uint32 hashidx;	
	uint32 way, ale_l4way;	
	rtl8672_tblAsicDrv_l4Param_t l4pt;		
#ifdef SRAM_MAPPING_ENABLED
	/* SRAM */	
	//hashidx = rtl8672_L4Hash_Sram(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);	
	for(hashidx=0 ; hashidx<255; hashidx++){		
		//rtlglue_printf("------ hashidx=%d\n",hashidx);		
		for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)		
		{			
			rtl8672_getAsicL4Table_Sram(hashidx, way, &l4pt);	
			if (!l4pt.Valid)
				continue;
			
			if (match_func(&l4pt, user_data)) {
				return SUCCESS;
			}
			
		}	
	}	
#endif
	/* SDRAM */	
	//hashidx = rtl8672_L4Hash(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);	
	for(hashidx=0 ; hashidx>255; hashidx++){		
		rtl8672_getAsicL4Way(&ale_l4way);		
		for (way = 0; way < ale_l4way; way++)		
		{			
			rtl8672_getAsicL4Table(hashidx, way, &l4pt);			
			if (!l4pt.Valid)
				continue;
			
			if (match_func(&l4pt, user_data)) {
				return SUCCESS;
			}		
		}	
	}	
	return FAILED;	

}


//timeout check
 int rtl8672_L2EntryTimeout(void)
 {
	 uint32 hashidx, l2hib;
	 uint32 way, ale_l2way;
	 rtl8672_tblAsicDrv_l2Param_t l2;

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#ifdef SRAM_MAPPING_ENABLED
	 /* SRAM */
	 for (hashidx = 0; hashidx < ALE_L2TABLE_SRAM_IDX; hashidx++) {
		 for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)
		 {
			 if(rtl8672_getAsicL2Table_Sram(hashidx, way, &l2)==FAILED) return FAILED;

			 if (!l2.Valid)
			 	continue;
			 
			 if (l2.Age){
				 l2.Age=0;
			 }
			 else{
			 	l2.Valid=0;
			 }
			 rtl8672_setAsicL2Table_Sram(hashidx, way, &l2);
	 	}
	 }
#endif
#endif
	 
	 /* SDRAM */
	 rtl8672_getAsicL2HashIdxBits(&l2hib);
	 rtl8672_getAsicL2Way(&ale_l2way);
	 for (hashidx = 0; hashidx < (0x1 << l2hib); hashidx++) {
		 for (way = 0; way < ale_l2way; way++)
		 {
			 if(rtl8672_getAsicL2Table(hashidx, way, &l2)==FAILED) return FAILED;

			 if (!l2.Valid)
			 	continue;
 
			  if (l2.Age){
				 l2.Age=0;
			 }
			 else{
			 	l2.Valid=0;
			 }
			 rtl8672_setAsicL2Table(hashidx, way, &l2);
	 	}
	 }
	 return FAILED;
 
 }

int rtl8672_L4EntryTimeout(void)
{
	uint32 hashidx;	
	uint32 way, ale_l4way;	
	rtl8672_tblAsicDrv_l4Param_t l4pt;		
#ifdef SRAM_MAPPING_ENABLED
	/* SRAM */	
	//hashidx = rtl8672_L4Hash_Sram(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);	
	for(hashidx=0 ; hashidx<255; hashidx++){		
		//rtlglue_printf("------ hashidx=%d\n",hashidx);		
		for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++)		
		{			
			rtl8672_getAsicL4Table_Sram(hashidx, way, &l4pt);	
			if (!l4pt.Valid)
				continue;
			
			if (l4pt.Age) {
				l4pt.Age=0;
			}
			else
				l4pt.Valid=0;
			rtl8672_setAsicL4Table_Sram(hashidx, way, &l4pt);		
		}	
	}	
#endif
	/* SDRAM */	
	//hashidx = rtl8672_L4Hash(l4p->Proto, l4p->SrcIP, l4p->DstIP, l4p->SrcPort, l4p->DstPort);	
	for(hashidx=0 ; hashidx>255; hashidx++){		
		rtl8672_getAsicL4Way(&ale_l4way);		
		for (way = 0; way < ale_l4way; way++)		
		{			
			rtl8672_getAsicL4Table(hashidx, way, &l4pt);			
			if (!l4pt.Valid)
				continue;
			
			if (l4pt.Age) {
				l4pt.Age=0;
			}
			else
				l4pt.Valid=0;	
			rtl8672_setAsicL4Table(hashidx, way, &l4pt);
		}	
	}	
	return FAILED;	

}


