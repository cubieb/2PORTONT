/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

#ifndef	__PHYDMANTDIV_H__
#define    __PHYDMANTDIV_H__

#define ANTDIV_VERSION	"1.0"

//1 ============================================================
//1  Definition 
//1 ============================================================


#define	MAIN_ANT	1		//Ant A or Ant Main
#define	AUX_ANT		2		//AntB or Ant Aux
#define	MAX_ANT		3		// 3 for AP using

#define ANT1_2G 0 // = ANT2_5G
#define ANT2_2G 1 // = ANT1_5G

//Antenna Diversty Control Type
#define	ODM_AUTO_ANT	0
#define	ODM_FIX_MAIN_ANT	1
#define	ODM_FIX_AUX_ANT	2

#define ODM_ANTDIV_SUPPORT		(ODM_RTL8188E|ODM_RTL8192E|ODM_RTL8723B|ODM_RTL8821|ODM_RTL8881A|ODM_RTL8812)
#define ODM_N_ANTDIV_SUPPORT		(ODM_RTL8188E|ODM_RTL8192E|ODM_RTL8723B)
#define ODM_AC_ANTDIV_SUPPORT		(ODM_RTL8821|ODM_RTL8881A|ODM_RTL8812)
#define ODM_SMART_ANT_SUPPORT		(ODM_RTL8188E|ODM_RTL8192E)

#define ODM_OLD_IC_ANTDIV_SUPPORT		(ODM_RTL8723A|ODM_RTL8192C|ODM_RTL8192D)

#define ODM_ANTDIV_2G_SUPPORT_IC			(ODM_RTL8188E|ODM_RTL8192E|ODM_RTL8723B|ODM_RTL8881A)
#define ODM_ANTDIV_5G_SUPPORT_IC			(ODM_RTL8821|ODM_RTL8881A|ODM_RTL8812)

#define ODM_EVM_ENHANCE_ANTDIV_SUPPORT_IC	(ODM_RTL8192E)

#define ODM_ANTDIV_2G	BIT0
#define ODM_ANTDIV_5G	BIT1

#define ANTDIV_ON 1
#define ANTDIV_OFF 0

#define FAT_ON 1
#define FAT_OFF 0

#define TX_BY_DESC 1
#define REG 0

#define RSSI_METHOD 0
#define EVM_METHOD 1
#define CRC32_METHOD 2

#define INIT_ANTDIV_TIMMER 0
#define CANCEL_ANTDIV_TIMMER 1
#define RELEASE_ANTDIV_TIMMER 2

#define CRC32_FAIL 1
#define CRC32_OK 0

#define Evm_RSSI_TH_High 25
#define Evm_RSSI_TH_Low 20

#define NORMAL_STATE_MIAN 1
#define NORMAL_STATE_AUX 2
#define TRAINING_STATE 3

#define FORCE_RSSI_DIFF 10

#define CSI_ON 1
#define CSI_OFF 0

#define DIVON_CSIOFF 1
#define DIVOFF_CSION 2

#define BDC_DIV_TRAIN_STATE 0
#define BDC_BFer_TRAIN_STATE 1
#define BDC_DECISION_STATE 2
#define BDC_BF_HOLD_STATE 3
#define BDC_DIV_HOLD_STATE 4

#define BDC_MODE_1 1
#define BDC_MODE_2 2
#define BDC_MODE_3 3
#define BDC_MODE_4 4
#define BDC_MODE_NULL 0xff

#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1

//1 ============================================================
//1  structure
//1 ============================================================



//1 ============================================================
//1  enumeration
//1 ============================================================



typedef enum _FAT_STATE
{
	FAT_NORMAL_STATE			= 0,
	FAT_TRAINING_STATE 		= 1,
}FAT_STATE_E, *PFAT_STATE_E;


typedef enum _ANT_DIV_TYPE
{
	NO_ANTDIV			= 0xFF,	
	CG_TRX_HW_ANTDIV		= 0x01,
	CGCS_RX_HW_ANTDIV 	= 0x02,
	FIXED_HW_ANTDIV		= 0x03,
	CG_TRX_SMART_ANTDIV	= 0x04,
	CGCS_RX_SW_ANTDIV	= 0x05,
	S0S1_SW_ANTDIV          = 0x06 //8723B intrnal switch S0 S1
}ANT_DIV_TYPE_E, *PANT_DIV_TYPE_E;


//1 ============================================================
//1  function prototype
//1 ============================================================


VOID
ODM_StopAntennaSwitchDm(
	IN	PDM_ODM_T	pDM_Odm
	);
VOID
ODM_SetAntConfig(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u1Byte		antSetting	// 0=A, 1=B, 2=C, ....
	);


#define SwAntDivRestAfterLink	ODM_SwAntDivRestAfterLink
VOID ODM_SwAntDivRestAfterLink(	IN	PDM_ODM_T	pDM_Odm);

#if (defined(CONFIG_HW_ANTENNA_DIVERSITY))

VOID
ODM_UpdateRxIdleAnt(
	IN	 	PDM_ODM_T 		pDM_Odm, 
	IN		 u1Byte		Ant
);

#if (RTL8723B_SUPPORT == 1)||(RTL8821A_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
VOID
ODM_SW_AntDiv_Callback(
	IN 	PRT_TIMER		pTimer
	);

VOID
ODM_SW_AntDiv_WorkitemCallback(
	IN 		PVOID            pContext
	);


#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)

VOID
ODM_SW_AntDiv_Callback(void *FunctionContext);

#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
VOID
odm_S0S1_SwAntDivByCtrlFrame(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			Step
);

VOID
odm_AntselStatisticsOfCtrlFrame(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			antsel_tr_mux,
	IN		u4Byte			RxPWDBAll
);

VOID
odm_S0S1_SwAntDivByCtrlFrame_ProcessRSSI(
	IN		PDM_ODM_T				pDM_Odm,
	IN		PODM_PHY_INFO_T		pPhyInfo,
	IN		PODM_PACKET_INFO_T		pPktinfo
);

#endif 
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV
VOID
odm_EVM_FastAntTrainingCallback(
	IN		PDM_ODM_T		pDM_Odm
);
#endif

VOID
odm_HW_AntDiv(
	IN		PDM_ODM_T		pDM_Odm
);

#if( defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY) ) ||( defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY) )
VOID
odm_FastAntTraining(
	IN		PDM_ODM_T		pDM_Odm
);

VOID
odm_FastAntTrainingCallback(
	IN		PDM_ODM_T		pDM_Odm
);

VOID
odm_FastAntTrainingWorkItemCallback(
	IN		PDM_ODM_T		pDM_Odm
);
#endif


VOID
ODM_AntDivInit(
	IN		 PDM_ODM_T		pDM_Odm 
);

VOID
ODM_AntDiv(
	IN		PDM_ODM_T		pDM_Odm
);

VOID
odm_AntselStatistics(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			antsel_tr_mux,
	IN		u4Byte			MacId,
	IN		u4Byte			utility,
	IN            u1Byte			method
);

VOID
ODM_Process_RSSIForAntDiv(	
	IN OUT	PDM_ODM_T					pDM_Odm,
	IN		PODM_PHY_INFO_T				pPhyInfo,
	IN		PODM_PACKET_INFO_T			pPktinfo
);

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
VOID
ODM_SetTxAntByTxInfo(
	IN		PDM_ODM_T		pDM_Odm,
	IN		pu1Byte			pDesc,
	IN		u1Byte			macId	
);

#elif(DM_ODM_SUPPORT_TYPE == ODM_AP)
VOID
ODM_SetTxAntByTxInfo(
	//IN		PDM_ODM_T		pDM_Odm,
	struct	rtl8192cd_priv		*priv,
	struct 	tx_desc			*pdesc,
	struct	tx_insn			*txcfg,
	unsigned short			aid	
);
#ifdef  CONFIG_WLAN_HAL
VOID
ODM_SetTxAntByTxInfo_HAL(
	//IN		PDM_ODM_T		pDM_Odm,
	struct	rtl8192cd_priv		*priv,
	PVOID					pdesc_data,
	struct	tx_insn			*txcfg,
	unsigned short			aid	
);
#endif	//#ifdef  CONFIG_WLAN_HAL
#endif


VOID
ODM_AntDiv_Config(
	IN		PDM_ODM_T		pDM_Odm
);


VOID
ODM_UpdateRxIdleAnt_8723B(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			Ant,
	IN		u4Byte			DefaultAnt, 
	IN		u4Byte			OptionalAnt
);

VOID
ODM_AntDivTimers(
	IN PDM_ODM_T	pDM_Odm,
	IN 		u1Byte		state
);

#endif //#if (defined(CONFIG_HW_ANTENNA_DIVERSITY))

VOID
ODM_AntDivReset(
	IN		PDM_ODM_T		pDM_Odm 
);

VOID
odm_AntennaDiversityInit(
	IN 		PDM_ODM_T		pDM_Odm 
);

VOID
odm_AntennaDiversity(
	IN 		PDM_ODM_T		pDM_Odm 
);


#endif //#ifndef	__ODMANTDIV_H__
