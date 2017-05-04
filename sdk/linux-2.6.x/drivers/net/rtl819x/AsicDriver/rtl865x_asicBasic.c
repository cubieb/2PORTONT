/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table basic operation driver
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#include <net/rtl/rtl_types.h>
//#include "types.h"
#include "asicRegs.h"
#include "asicTabs.h"
//#include "rtl_glue.h"
#include "rtl865x_hwPatch.h"
#include "rtl865x_asicBasic.h"

/*
 *  According to ghhuang's suggest,
 *    we DO NOT need to access 8 ASIC entries at once.
 *  We just need to access several entries as we need.
 */

#define RTL865X_FAST_ASIC_ACCESS

/*
 *  For RTL865xC Access protection mechanism
 *
 *	We define 2 different MACROs for ASIC table
 *	read/write protection correspondingly
 *
 */
//#define		RTL865XC_ASIC_READ_PROTECTION				/* Enable/Disable ASIC read protection */
//#define		RTL865XC_ASIC_WRITE_PROTECTION				/* Enable/Disable ASIC write protection */

//#undef		RTL865X_READ_MULTIPLECHECK
#define 		RTL865X_READ_MULTIPLECHECK
#define		RTL865X_READ_MULTIPLECHECK_CNT		2
#define		RTL865X_READ_MULTIPLECHECK_MAX_RETRY	10


int8 RtkHomeGatewayChipName[16];
int32 RtkHomeGatewayChipRevisionID;
int32 RtkHomeGatewayChipNameID;

#ifdef RTL865X_FAST_ASIC_ACCESS
static uint32 _rtl8651_asicTableSize[] =
{
        2 /*TYPE_L2_SWITCH_TABLE*/,
        1 /*TYPE_ARP_TABLE*/,
	 2 /*TYPE_L3_ROUTING_TABLE*/,
        3 /*TYPE_MULTICAST_TABLE*/,
        5 /*TYPE_NETIF_TABLE*/,
        3 /*TYPE_EXT_INT_IP_TABLE*/,
        3 /*TYPE_VLAN_TABLE*/,
        3 /*TYPE_VLAN1_TABLE*/,          
    4 /*TYPE_SERVER_PORT_TABLE*/,
    3 /*TYPE_L4_TCP_UDP_TABLE*/,
    3 /*TYPE_L4_ICMP_TABLE*/,
    1 /*TYPE_PPPOE_TABLE*/,
    8 /*TYPE_ACL_RULE_TABLE*/,
    1 /*TYPE_NEXT_HOP_TABLE*/,
    3 /*TYPE_RATE_LIMIT_TABLE*/,
    1 /*TYPE_ALG_TABLE*/,

};
#endif
static void _rtl8651_asicTableAccessForward(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	ASSERT_CSP(entryContent_P);

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

#ifdef RTL865X_FAST_ASIC_ACCESS

	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			WRITE_MEM32(TCR0+(index<<2), *((uint32 *)entryContent_P + index));
		}

	}
#else
	WRITE_MEM32(TCR0, *((uint32 *)entryContent_P + 0));
	WRITE_MEM32(TCR1, *((uint32 *)entryContent_P + 1));
	WRITE_MEM32(TCR2, *((uint32 *)entryContent_P + 2));
	WRITE_MEM32(TCR3, *((uint32 *)entryContent_P + 3));
	WRITE_MEM32(TCR4, *((uint32 *)entryContent_P + 4));
	WRITE_MEM32(TCR5, *((uint32 *)entryContent_P + 5));
	WRITE_MEM32(TCR6, *((uint32 *)entryContent_P + 6));
	WRITE_MEM32(TCR7, *((uint32 *)entryContent_P + 7));
#endif	
	WRITE_MEM32(SWTAA, ((uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH));//Fill address
}

int32 _rtl8651_addAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);

	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{
      		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	#endif

	WRITE_MEM32(SWTACR, ACTION_START | CMD_ADD );//Activate add command

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
    
	if ( (READ_MEM32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
	{
		#ifdef RTL865XC_ASIC_WRITE_PROTECTION
		if(RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
		{
			WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
		}
		#endif

		return FAILED;
	}

	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}
	#endif

	return SUCCESS;
}
#ifdef CONFIG_PROC_FS
static unsigned int mcastForceAddOpCnt=0;
unsigned int _rtl865x_getForceAddMcastOpCnt(void)
{
	return mcastForceAddOpCnt;
}
#endif
int32 _rtl8651_forceAddAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
#ifdef CONFIG_PROC_FS
	if(tableType==TYPE_MULTICAST_TABLE)
	{
		mcastForceAddOpCnt++;
	}
#endif	
	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	#endif

	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);

 	WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}
	#endif

	return SUCCESS;
}

int32 _rtl8651_readAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	uint32 *entryAddr;
	uint32 tmp;/* dummy variable, don't remove it */

	#ifdef RTL865X_READ_MULTIPLECHECK
	uint32 entryContent[RTL865X_READ_MULTIPLECHECK_CNT][8];
	uint32 entryContentIdx;
	uint32 entryContent_new = RTL865X_READ_MULTIPLECHECK_CNT;/* to indicate which content is newer */
	uint32 entryCompare_max_count = RTL865X_READ_MULTIPLECHECK_MAX_RETRY;
	uint32 needRetry;
	#endif
	
	#ifdef RTL865XC_ASIC_READ_PROTECTION
	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	#endif

	ASSERT_CSP(entryContent_P);
	entryAddr = (uint32 *) (
		(uint32) rtl8651_asicTableAccessAddrBase(tableType) + (eidx<<5 /*RTL8651_ASICTABLE_ENTRY_LENGTH*/) ) ;
		/*(uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);*/

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command ready
    
#ifdef RTL865X_READ_MULTIPLECHECK
		do
		{
			for (	entryContentIdx = 0 ;
				entryContentIdx < RTL865X_READ_MULTIPLECHECK_CNT ;
			   	entryContentIdx ++ )
			{
				if ( entryContent_new != entryContentIdx )
				{
					entryContent[entryContentIdx][0] = *(entryAddr + 0);
					entryContent[entryContentIdx][1] = *(entryAddr + 1);
					entryContent[entryContentIdx][2] = *(entryAddr + 2);
					entryContent[entryContentIdx][3] = *(entryAddr + 3);
					entryContent[entryContentIdx][4] = *(entryAddr + 4);
					entryContent[entryContentIdx][5] = *(entryAddr + 5);
					entryContent[entryContentIdx][6] = *(entryAddr + 6);
					entryContent[entryContentIdx][7] = *(entryAddr + 7);

					entryContent_new = entryContentIdx;
				}
			}

			needRetry = FALSE;

			for (	entryContentIdx = 0 ;
				entryContentIdx < RTL865X_READ_MULTIPLECHECK_CNT ;
			   	entryContentIdx ++ )
			{
				uint32 entryCompareContentIdx;

				for (	entryCompareContentIdx = (entryContentIdx) ;
				   	entryCompareContentIdx < RTL865X_READ_MULTIPLECHECK_CNT ;
				   	entryCompareContentIdx ++ )
				{
					int32 idx;

					for ( idx = 0 ; idx < 8 ; idx ++ )
					{
						if (	entryContent[entryContentIdx][idx] !=
							entryContent[entryCompareContentIdx][idx]	)
						{
							needRetry = TRUE;
							goto retry;
						}
					}
				}
			}
retry:
		entryCompare_max_count --;
		} while (	( needRetry == FALSE ) &&
		      		( entryCompare_max_count > 0 ) );
	
	/* Update entryAddr for newer one */
	entryAddr = &( entryContent[ entryContent_new ][0] );
#endif


#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
		}
	}
#else
	*((uint32 *)entryContent_P + 0) = *(entryAddr + 0);
	*((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
	*((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
	*((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
	*((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
	*((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
	*((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
	*((uint32 *)entryContent_P + 7) = *(entryAddr + 7);
#endif

	/* Dummy read. Must read an un-used table entry to refresh asic latch */
	tmp = *(uint32 *)((uint32) rtl8651_asicTableAccessAddrBase(TYPE_ACL_RULE_TABLE) + 1024 * RTL8651_ASICTABLE_ENTRY_LENGTH);
	#ifdef RTL865XC_ASIC_READ_PROTECTION
	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}
	#endif

	return 0;
}
#if 0  //No need to Stop_Table_Lookup process 
int32 _rtl8651_readAsicEntryStopTLU(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	uint32 *entryAddr;
	uint32 tmp;/* dummy variable, don't remove it */
	

	ASSERT_CSP(entryContent_P);
	entryAddr = (uint32 *) (
		(uint32) rtl8651_asicTableAccessAddrBase(tableType) + (eidx<<5 /*RTL8651_ASICTABLE_ENTRY_LENGTH*/) ) ;
		/*(uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);*/
#if 0
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command ready
#endif	

	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{	/* No need to stop HW table lookup process */
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		#if 0
		//while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
		#endif	
	}

#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
		}
	}
#else
	*((uint32 *)entryContent_P + 0) = *(entryAddr + 0);
	*((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
	*((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
	*((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
	*((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
	*((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
	*((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
	*((uint32 *)entryContent_P + 7) = *(entryAddr + 7);
#endif


	if (RTL865X_TLU_BUG_FIXED)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}

	/* Dummy read. Must read an un-used table entry to refresh asic latch */
	tmp = *(uint32 *)((uint32) rtl8651_asicTableAccessAddrBase(TYPE_ACL_RULE_TABLE) + 1024 * RTL8651_ASICTABLE_ENTRY_LENGTH);

	return 0;
}
#endif
int32 _rtl8651_delAsicEntry(uint32 tableType, uint32 startEidx, uint32 endEidx) 
{
	uint32 eidx = startEidx;

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			WRITE_MEM32(TCR0+(index<<2), 0);
		}
	}
#else
	WRITE_MEM32(TCR0, 0);
	WRITE_MEM32(TCR1, 0);
	WRITE_MEM32(TCR2, 0);
	WRITE_MEM32(TCR3, 0);
	WRITE_MEM32(TCR4, 0);
	WRITE_MEM32(TCR5, 0);
	WRITE_MEM32(TCR6, 0);
	WRITE_MEM32(TCR7, 0);
#endif	
	
	while (eidx <= endEidx) {
		WRITE_MEM32(SWTAA, (uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);//Fill address
        
		WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command

		while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
    
		if ( (READ_MEM32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
			return FAILED;
		
		++eidx;
	}
	return SUCCESS;
}




