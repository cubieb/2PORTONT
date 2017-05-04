/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8814AEGen.c
	
Abstract:
	Defined RTL8814AE HAL Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2013-05-28 Filen            Create.	
--*/

#ifndef __ECOS
#include "HalPrecomp.h"
#else
#include "../../../HalPrecomp.h"
#endif

RT_STATUS
InitPON8814AE(
    IN  HAL_PADAPTER Adapter,
    IN  u4Byte     	ClkSel        
)
{
    u32     bytetmp;
    u32     retry;
    
    RT_TRACE_F( COMP_INIT, DBG_LOUD, ("\n"));

    // TODO: Filen, first write IO will fail, don't know the root cause
    printk("0: Reg0x0: 0x%x, Reg0x4: 0x%x, Reg0x1C: 0x%x\n", HAL_RTL_R32(0x0), HAL_RTL_R32(0x4), HAL_RTL_R32(0x1C));
	HAL_RTL_W8(REG_RSV_CTRL, 0x00);
    printk("1: Reg0x0: 0x%x, Reg0x4: 0x%x, Reg0x1C: 0x%x\n", HAL_RTL_R32(0x0), HAL_RTL_R32(0x4), HAL_RTL_R32(0x1C));
	HAL_RTL_W8(REG_RSV_CTRL, 0x00);
    printk("2: Reg0x0: 0x%x, Reg0x4: 0x%x, Reg0x1C: 0x%x\n", HAL_RTL_R32(0x0), HAL_RTL_R32(0x4), HAL_RTL_R32(0x1C));

    // TODO: Filen, check 8814A setting
	if(ClkSel == XTAL_CLK_SEL_25M) {
	} else if (ClkSel == XTAL_CLK_SEL_40M){
	}	

	if (!HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
			PWR_INTF_PCI_MSK, rtl8814A_card_enable_flow))
    {
        RT_TRACE( COMP_INIT, DBG_SERIOUS, ("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__));
        return RT_STATUS_FAILURE;
    }

    printk("3: Reg0x0: 0x%x, Reg0x4: 0x%x, Reg0x1C: 0x%x\n", HAL_RTL_R32(0x0), HAL_RTL_R32(0x4), HAL_RTL_R32(0x1C));

#ifdef RTL_8814A_MP_TEMP

    HAL_RTL_W32(REG_BD_RWPTR_CLR,0xffffffff);

    HAL_RTL_W32(0x1000, HAL_RTL_R32(0x1000)|BIT16|BIT17);
    printk("%s(%d): 0x1000:0x%x \n", __FUNCTION__, __LINE__, HAL_RTL_R32(0x1000));
#endif

    return  RT_STATUS_SUCCESS;
}


RT_STATUS
StopHW8814AE(
    IN  HAL_PADAPTER Adapter
)
{
    // TODO:

    return RT_STATUS_SUCCESS;
}


RT_STATUS
ResetHWForSurprise8814AE(
    IN  HAL_PADAPTER Adapter
)
{
    // TODO: Filen, necessary to be added code here

    return RT_STATUS_SUCCESS;
}


RT_STATUS	
hal_Associate_8814AE(
	 HAL_PADAPTER        Adapter,
    BOOLEAN             IsDefaultAdapter
)
{
    PHAL_INTERFACE              pHalFunc = GET_HAL_INTERFACE(Adapter);
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);

    //
    //Initialization Related
    //
    pHalData->AccessSwapCtrl        = HAL_ACCESS_SWAP_MEM;
    pHalFunc->InitPONHandler        = InitPON8814AE;
    pHalFunc->InitMACHandler        = InitMAC88XX;
    pHalFunc->InitFirmwareHandler   = InitMIPSFirmware88XX;
    pHalFunc->InitHCIDMAMemHandler  = InitHCIDMAMem88XX;
    pHalFunc->InitHCIDMARegHandler  = InitHCIDMAReg88XX;    
#if CFG_HAL_SUPPORT_MBSSID    
    pHalFunc->InitMBSSIDHandler     = InitMBSSID88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->InitVAPIMRHandler     = InitVAPIMR88XX;
    pHalFunc->InitLLT_TableHandler  = InitLLT_Table88XX_V1;
#if CFG_HAL_HW_FILL_MACID
    pHalFunc->InitMACIDSearchHandler    = InitMACIDSearch88XX;            
    pHalFunc->CheckHWMACIDResultHandler = CheckHWMACIDResult88XX;            
#endif //CFG_HAL_HW_FILL_MACID

    //
    //Stop Related
    //
#if CFG_HAL_SUPPORT_MBSSID        
    pHalFunc->StopMBSSIDHandler     = StopMBSSID88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->StopHWHandler         = StopHW88XX;
    pHalFunc->StopSWHandler         = StopSW88XX;
    pHalFunc->DisableVXDAPHandler   = DisableVXDAP88XX;
    pHalFunc->ResetHWForSurpriseHandler     = ResetHWForSurprise8814AE;


    //
    //ISR Related
    //
    pHalFunc->InitIMRHandler                    = InitIMR88XX;
    pHalFunc->EnableIMRHandler                  = EnableIMR88XX;
    pHalFunc->InterruptRecognizedHandler        = InterruptRecognized88XX;
    pHalFunc->GetInterruptHandler               = GetInterrupt88XX;
    pHalFunc->AddInterruptMaskHandler           = AddInterruptMask88XX;
    pHalFunc->RemoveInterruptMaskHandler        = RemoveInterruptMask88XX;
    pHalFunc->DisableRxRelatedInterruptHandler  = DisableRxRelatedInterrupt88XX;
    pHalFunc->EnableRxRelatedInterruptHandler   = EnableRxRelatedInterrupt88XX;


    //
    //Tx Related
    //
    pHalFunc->PrepareTXBDHandler            = PrepareTXBD88XX;    
    pHalFunc->FillTxHwCtrlHandler           = FillTxHwCtrl88XX;
    pHalFunc->SyncSWTXBDHostIdxToHWHandler  = SyncSWTXBDHostIdxToHW88XX;
    pHalFunc->TxPollingHandler              = TxPolling88XX;
    pHalFunc->SigninBeaconTXBDHandler       = SigninBeaconTXBD88XX;
    pHalFunc->SetBeaconDownloadHandler      = SetBeaconDownload88XX;
    pHalFunc->FillBeaconDescHandler         = FillBeaconDesc88XX;
    pHalFunc->GetTxQueueHWIdxHandler        = GetTxQueueHWIdx88XX;
    pHalFunc->MappingTxQueueHandler         = MappingTxQueue88XX;
    pHalFunc->QueryTxConditionMatchHandler  = QueryTxConditionMatch88XX;
#if CFG_HAL_TX_SHORTCUT
//    pHalFunc->GetShortCutTxDescHandler      = GetShortCutTxDesc88XX;
//    pHalFunc->ReleaseShortCutTxDescHandler  = ReleaseShortCutTxDesc88XX;
    pHalFunc->GetShortCutTxBuffSizeHandler  = GetShortCutTxBuffSize88XX;
    pHalFunc->SetShortCutTxBuffSizeHandler  = SetShortCutTxBuffSize88XX;
    pHalFunc->CopyShortCutTxDescHandler     = CopyShortCutTxDesc88XX;
    pHalFunc->FillShortCutTxHwCtrlHandler   = FillShortCutTxHwCtrl88XX;    
    pHalFunc->ReleaseOnePacketHandler       = ReleaseOnePacket88XX;                  
#endif // CFG_HAL_TX_SHORTCUT

    //
    //Rx Related
    //
    pHalFunc->PrepareRXBDHandler            = PrepareRXBD88XX;
    pHalFunc->QueryRxDescHandler            = QueryRxDesc88XX;
    pHalFunc->UpdateRXBDInfoHandler         = UpdateRXBDInfo88XX;
    pHalFunc->UpdateRXBDHWIdxHandler        = UpdateRXBDHWIdx88XX;
    pHalFunc->UpdateRXBDHostIdxHandler      = UpdateRXBDHostIdx88XX;    

    //
    // General operation
    //
    pHalFunc->GetChipIDMIMOHandler          =   GetChipIDMIMO88XX;
    pHalFunc->SetHwRegHandler               =   SetHwReg88XX;
    pHalFunc->GetHwRegHandler               =   GetHwReg88XX;
    pHalFunc->SetMACIDSleepHandler          =   SetMACIDSleep88XX;
	pHalFunc->CheckHangHandler              =   CheckHang88XX;
    pHalFunc->GetMACIDQueueInTXPKTBUFHandler=   GetMACIDQueueInTXPKTBUF88XX;

    //
    // Timer Related
    //
    pHalFunc->Timer1SecHandler              =   Timer1Sec88XX;


    //
    // Security Related     
    //
    pHalFunc->CAMReadMACConfigHandler       =   CAMReadMACConfig88XX;
    pHalFunc->CAMEmptyEntryHandler          =   CAMEmptyEntry88XX;
    pHalFunc->CAMFindUsableHandler          =   CAMFindUsable88XX;
    pHalFunc->CAMProgramEntryHandler        =   CAMProgramEntry88XX;


    //
    // PHY/RF Related
    //
    pHalFunc->PHYSetCCKTxPowerHandler       = PHYSetCCKTxPower88XX_AC;
    pHalFunc->PHYSetOFDMTxPowerHandler      = PHYSetOFDMTxPower88XX_AC;
    pHalFunc->PHYSwBWModeHandler            = SwBWMode88XX_AC;
    pHalFunc->PHYUpdateBBRFValHandler       = UpdateBBRFVal88XX_AC;
    // TODO: 8814A Power Tracking should be done
    pHalFunc->TXPowerTrackingHandler        = TXPowerTracking_ThermalMeter_Tmp8814A;
    pHalFunc->PHYSSetRFRegHandler           = PHY_SetRFReg_88XX_AC;
    pHalFunc->PHYQueryRFRegHandler          = PHY_QueryRFReg_8814;
    pHalFunc->IsBBRegRangeHandler           = IsBBRegRange88XX_V1;


    //
    // Firmware CMD IO related
    //
    pHalData->H2CBufPtr88XX     = 0;
    pHalData->bFWReady          = _FALSE;
    // TODO: code below should be sync with new 3081 FW
    pHalFunc->FillH2CCmdHandler             = FillH2CCmd88XX;
    pHalFunc->UpdateHalRAMaskHandler        = UpdateHalRAMask8814A;
    pHalFunc->UpdateHalMSRRPTHandler        = UpdateHalMSRRPT88XX;
    pHalFunc->SetAPOffloadHandler           = HalGeneralDummy;
    pHalFunc->SetRsvdPageHandler	        = HalGeneralDummy;
    pHalFunc->GetRsvdPageLocHandler	        = HalGeneralDummy;
    pHalFunc->DownloadRsvdPageHandler	    = HalGeneralDummy;
    pHalFunc->C2HHandler                    = HalGeneralDummy;
    pHalFunc->C2HPacketHandler              = C2HPacket88XX;    
    pHalFunc->GetTxRPTHandler               = GetTxRPTBuf88XX;
    pHalFunc->SetTxRPTHandler               = SetTxRPTBuf88XX;    
#if CFG_HAL_HW_FILL_MACID
    pHalFunc->SetCRC5ToRPTBufferHandler     = SetCRC5ToRPTBuffer88XX;        
#endif //#if CFG_HAL_HW_FILL_MACID
    pHalFunc->DumpRxBDescTestHandler        = DumpRxBDesc88XX;
    pHalFunc->DumpTxBDescTestHandler        = DumpTxBDesc88XX;
    
    return  RT_STATUS_SUCCESS;    
}


void 
InitMAC8814AE(
    IN  HAL_PADAPTER Adapter
)
{


    
}

#ifdef BEAMFORMING_SUPPORT
VOID
SetBeamformRfMode8814A(
	struct rtl8192cd_priv *priv,
	PRT_BEAMFORMING_INFO 	pBeamformingInfo,
	u1Byte					idx
	)
{
	u1Byte					i, Nr_index = 0;
	BOOLEAN					bSelfBeamformer = FALSE;
	RT_BEAMFORMING_ENTRY	BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[idx];
	BEAMFORMING_CAP		BeamformCap = BEAMFORMING_CAP_NONE;
	
	BeamformCap = Beamforming_GetBeamCap(pBeamformingInfo);

	Nr_index = TxBF_Nr(N_TX, BeamformeeEntry.CompSteeringNumofBFer);
	
	pBeamformingInfo->BeamformCap = BeamformCap;

	if(get_rf_mimo_mode(priv) == MIMO_1T1R)
		return;

	bSelfBeamformer = BeamformeeEntry.BeamformEntryCap & (BEAMFORMEE_CAP_HT_EXPLICIT|BEAMFORMEE_CAP_VHT_SU);

	for(i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
	{
		PHY_SetRFReg(priv, i, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable
	}
	
	if(bSelfBeamformer)
	{	
		for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
		{
			PHY_SetRFReg(priv, i, RF_RCK_OS, 0xfffff,0x18000); // Select RX mode
			PHY_SetRFReg(priv, i, RF_TXPA_G1, 0xfffff,0xBE77F); // Set Table data
			PHY_SetRFReg(priv, i, RF_TXPA_G2, 0xfffff,0x226BF); // Enable TXIQGEN in RX mode
		}
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff,0xE26BF); // Enable TXIQGEN in RX mode
	}

	for (i= ODM_RF_PATH_A; i < MAX_RF_PATH; i++)
	{
		PHY_SetRFReg(priv, i, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable
	}
	
	
	if(bSelfBeamformer)			// for 8814 19ac(idx 1), 19b4(idx 0), different Tx ant setting
	{
		PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT28|BIT29, 0x2);// enable BB TxBF ant mapping register
		
		if(idx == 0)
		{
			switch(Nr_index)
			{			
				case 0:	
				break;

				case 1:		// Nsts = 2	BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT3|BIT2|BIT1|BIT0, 0x6);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT7|BIT6|BIT5|BIT4, 0x6);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0x0000ff00, 0x10);			//BC
				break;

				case 2:		//Nsts = 3	BCD
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT3|BIT2|BIT1|BIT0, 0xe);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT7|BIT6|BIT5|BIT4, 0xe);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0x0000ff00, 0x90);			//BCD
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT19|BIT18|BIT17|BIT16, 0xe);	//3ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0xff00000, 0x90);			//bcd
				break;
			
				default:		//Nr>3, same as Case 3
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT3|BIT2|BIT1|BIT0, 0xf);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT7|BIT6|BIT5|BIT4, 0xf);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0x0000ff00, 0xe4);			//BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, BIT19|BIT18|BIT17|BIT16, 0xf);	//3ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF0, 0xff00000, 0xe4);			//bcd
				break;
			}
		}
		else			// IDX =1
		{
			switch(Nr_index)
			{
				case 0:	
				break;

				case 1:		// Nsts = 2	BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT3|BIT2|BIT1|BIT0, 0x6);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT7|BIT6|BIT5|BIT4, 0x6);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0x0000ff00, 0x10);			//BC
				break;

				case 2:		//Nsts = 3	BCD
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT3|BIT2|BIT1|BIT0, 0xe);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT7|BIT6|BIT5|BIT4, 0xe);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0x0000ff00, 0x90);			//BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT19|BIT18|BIT17|BIT16, 0xe);	//3ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0xff00000, 0x90);			//bcd
				break;
			
				default:		//Nr>3, same as Case 3
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT3|BIT2|BIT1|BIT0, 0xf);	//1ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT7|BIT6|BIT5|BIT4, 0xf);	//2ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0x0000ff00, 0xe4);			//BC
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, BIT19|BIT18|BIT17|BIT16, 0xf);	//3ss
				PHY_SetBBReg(priv, REG_BB_TXBF_ANT_SET_BF1, 0xff00000, 0xe4);			//bcd
				break;
	
			}
		}
		
	}


}
VOID
SetBeamformEnter8814A(
	struct rtl8192cd_priv *priv,
	u1Byte				BFerBFeeIdx
	)
{
	u1Byte					i = 0;
	u1Byte					BFerIdx = (BFerBFeeIdx & 0xF0)>>4;
	u1Byte					BFeeIdx = (BFerBFeeIdx & 0xF);
	u2Byte					CSI_Param = 0;
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[BFeeIdx];
	RT_BEAMFORMER_ENTRY	BeamformerEntry = pBeamformingInfo->BeamformerEntry[BFerIdx];
	u2Byte					STAid = 0;
	u1Byte					Nc_index = 0,Nr_index = 0, grouping = 0, codebookinfo = 0, coefficientsize = 0;

	SetBeamformRfMode8814A(priv, pBeamformingInfo, BFeeIdx);	

	//I am Bfee
	if((pBeamformingInfo->BeamformCap & BEAMFORMEE_CAP) && (BFerIdx < BEAMFORMER_ENTRY_NUM))
	{
	
		RTL_W8( REG_SOUNDING_PTCL_CTRL, 0x9B);	

		// MAC addresss/Partial AID of Beamformer
		if(BFerIdx == 0)
		{
			for(i = 0; i < 6 ; i++)
				RTL_W8( (REG_ASSOCIATED_BFMER0_INFO+i), BeamformerEntry.MacAddr[i]);
		}
		else
		{
			for(i = 0; i < 6 ; i++)
				RTL_W8( (REG_ASSOCIATED_BFMER1_INFO+i), BeamformerEntry.MacAddr[i]);
		}

		// CSI report parameters of Beamformer

		Nc_index = TxBF_Nc(N_RX, BeamformerEntry.NumofSoundingDim);	// for 8814A Nrx = 3(4 Ant), min=0(1 Ant)
		Nr_index = BeamformerEntry.NumofSoundingDim;	//0x718[7] = 1 use Nsts, 0x718[7] = 0 use reg setting. as Bfee, we use Nsts, so Nr_index don't care
		grouping = 0;

		//// for ac = 1, for n = 3
		if(BeamformerEntry.BeamformEntryCap & BEAMFORMEE_CAP_VHT_SU)
			codebookinfo = 1;	
		else if(BeamformerEntry.BeamformEntryCap & BEAMFORMEE_CAP_HT_EXPLICIT)
			codebookinfo = 3;	

		coefficientsize = 3;

		CSI_Param =(u2Byte)((coefficientsize<<10)|(codebookinfo<<8)|(grouping<<6)|(Nr_index<<3)|(Nc_index));

		if(BFerIdx == 0)
		{
			RTL_W16(REG_TX_CSI_RPT_PARAM_BW20, CSI_Param);
		}
		else
		{
			RTL_W16(REG_TX_CSI_RPT_PARAM_BW20+2, CSI_Param);
		}

		// Timeout value for MAC to leave NDP_RX_standby_state 60 us
		RTL_W8( REG_SOUNDING_PTCL_CTRL+3, 0x60);				// // ndp_rx_standby_timer, 8814 need > 0x56, suggest from Dvaid
	}

	//I am Bfer
	if((pBeamformingInfo->BeamformCap & BEAMFORMER_CAP) && (BFeeIdx < BEAMFORMEE_ENTRY_NUM))
	{	
	
		if(OPMODE & WIFI_ADHOC_STATE)
			STAid = BeamformeeEntry.AID;
		else 
			STAid = BeamformeeEntry.P_AID;

		// P_AID of Beamformee & enable NDPA transmission
		if(BFeeIdx == 0)
		{	
			RTL_W16( REG_TXBF_CTRL, STAid);	
			RTL_W8( REG_TXBF_CTRL+3, RTL_R8( REG_TXBF_CTRL+3)|BIT6|BIT7|BIT4);
		}	
		else
		{
			RTL_W16( REG_TXBF_CTRL+2, STAid |BIT14| BIT15|BIT12);
		}	

		// CSI report parameters of Beamformee
		if(BFeeIdx == 0)	
		{
			// Get BIT24 & BIT25
			u1Byte	tmp = RTL_R8( REG_BEAMFORMEE_SEL+3) & 0x3;	
			RTL_W8( REG_BEAMFORMEE_SEL+3, tmp | 0x60);
			RTL_W16( REG_BEAMFORMEE_SEL, STAid | BIT9);
			
		}	
		else
		{
			// Set BIT25
			RTL_W16( REG_BEAMFORMEE_SEL+2, STAid | 0xE200);
		}

		RTL_W16( REG_TXBF_CTRL, RTL_R16(REG_TXBF_CTRL)|BIT15);	// disable NDP/NDPA packet use beamforming 

		Beamforming_Notify(priv);
	}
	
}


VOID
SetBeamformLeave8814A(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);

	/*	Clear P_AID of Beamformee
	* 	Clear MAC addresss of Beamformer
	*	Clear Associated Bfmee Sel
	*/

	if(Idx == 0)
	{	
		RTL_W16( REG_TXBF_CTRL, 0);	

		RTL_W32( REG_ASSOCIATED_BFMER0_INFO, 0);
		RTL_W16( REG_ASSOCIATED_BFMER0_INFO+4, 0);

		RTL_W16( REG_BEAMFORMEE_SEL, 0);
	}	
	else
	{
		RTL_W16(REG_TXBF_CTRL+2, RTL_R16( REG_TXBF_CTRL+2) & 0xF000);

		RTL_W32( REG_ASSOCIATED_BFMER1_INFO, 0);
		RTL_W16( REG_ASSOCIATED_BFMER1_INFO+4, 0);

		RTL_W16(REG_BEAMFORMEE_SEL+2,	RTL_R16( REG_BEAMFORMEE_SEL+2) & 0x60);
	}	
}
VOID
SetBeamformStatus8814A(
	struct rtl8192cd_priv *priv,
	 u1Byte				Idx
	)
{
	u2Byte					BeamCtrlVal;
	u4Byte					BeamCtrlReg;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformEntry = pBeamformingInfo->BeamformeeEntry[Idx];
	if(OPMODE & WIFI_ADHOC_STATE)
		BeamCtrlVal = BeamformEntry.MacId;
	else 
		BeamCtrlVal = BeamformEntry.P_AID;
	if(Idx == 0)
		BeamCtrlReg = REG_TXBF_CTRL;
	else
	{
		BeamCtrlReg = REG_TXBF_CTRL+2;
		BeamCtrlVal |= BIT12|BIT14|BIT15;
	}
	if(BeamformEntry.BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSED)
	{
		if(BeamformEntry.BW == HT_CHANNEL_WIDTH_20)
			BeamCtrlVal |= BIT9;
		else if(BeamformEntry.BW == HT_CHANNEL_WIDTH_20_40)
			BeamCtrlVal |= (BIT9 | BIT10);
		else if(BeamformEntry.BW == HT_CHANNEL_WIDTH_80)
			BeamCtrlVal |= (BIT9 | BIT10 | BIT11);
	}
	else
	{
		BeamCtrlVal &= ~(BIT9 | BIT10 | BIT11);
	}
	RTL_W16(BeamCtrlReg, BeamCtrlVal);
	RTL_W16( REG_TXBF_CTRL, RTL_R16(REG_TXBF_CTRL)|BIT15);	// disable NDP/NDPA packet use beamforming 

}
//2REG_C2HEVT_CLEAR
#define		C2H_EVT_HOST_CLOSE			0x00	// Set by driver and notify FW that the driver has read the C2H command message
#define		C2H_EVT_FW_CLOSE			0xFF	// Set by FW indicating that FW had set the C2H command message and it's not yet read by driver.



VOID Beamforming_NDPARate_8814A(
	struct rtl8192cd_priv *priv,
	BOOLEAN		Mode,
	u1Byte		BW,
	u1Byte		Rate)
{

	u2Byte	NDPARate = Rate;

	if(NDPARate == 0)
	{
		if(priv->pshare->rssi_min > 30) // link RSSI > 30%
			NDPARate = 0x8;				//MGN_24M, TxDesc = 0x08
		else
			NDPARate = 0x4;				//MGN_6M, TxDesc = 0x4
	}

	if(NDPARate < MGN_MCS0)
		BW = HT_CHANNEL_WIDTH_20;	

	RTL_W8(REG_NDPA_OPT_CTRL, BW & 0x3);
	RTL_W8(REG_NDPA_RATE, NDPARate);

}

VOID
C2HTxBeamformingHandler_8814A(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	u1Byte 	status = CmdBuf[0] & BIT0;
	Beamforming_End(priv, status);
}

VOID HW_VAR_HW_REG_TIMER_START_8814A(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x15F, 0x0);
	RTL_W8(0x15F, 0x5);

}

VOID HW_VAR_HW_REG_TIMER_INIT_8814A(struct rtl8192cd_priv *priv, int t)
{
	RTL_W8(0x164, 1);
	RTL_W16(0x15C, t);

}

VOID HW_VAR_HW_REG_TIMER_STOP_8814A(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x15F, 0);
}


VOID _Beamforming_CLK_8814A(struct rtl8192cd_priv *priv)
{
	u2Byte	u2btmp;
	u1Byte	QueueID, Count = 0, u1btmp;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if(priv->pshare->bScanInProcess==TRUE)
		return;

	SAVE_INT_AND_CLI(flags);
	
	// Stop Usb TxDMA
	RTL_W16(REG_PCIE_CTRL, 0xff00 );

	// Wait TXFF empty
	for(Count = 0; Count < 100; Count++) {
		u2btmp = RTL_R16(REG_TXPKT_EMPTY);
		u2btmp = u2btmp & 0xfff;
		if(u2btmp != 0xfff) {
			delay_ms(10);
			continue;
		} else {
			break;
		}
	}

	// TX pause
	RTL_W8(REG_TXPAUSE, 0xFF);

	// Wait TX State Machine OK
	for(Count = 0; Count < 100; Count++) {
		if(RTL_R32(REG_SCH_TXCMD) != 0)
			continue;
		else 
			break;
	}
	
	// Stop RX DMA path
	u1btmp = RTL_R8(REG_RXDMA_CONTROL);
	RTL_W8(REG_RXDMA_CONTROL, u1btmp| BIT2);

	for(Count = 0; Count < 100; Count++) {
		u1btmp = RTL_R8( REG_RXDMA_CONTROL);
		if(u1btmp & BIT1)
			break;
		else
			delay_ms(10);
	}

	// Disable clock
	RTL_W8(REG_SYS_CLK_CTRL+1, 0xf0);
	// Disable 320M
	RTL_W8(REG_AFE_CTRL2+3, 0x8);
	// Enable 320M
	RTL_W8(REG_AFE_CTRL2+3, 0xa);
	// Enable clock
	RTL_W8( REG_SYS_CLK_CTRL+1, 0xfc);

	// Release Tx pause
	RTL_W8(REG_TXPAUSE, 0);

	// Enable RX DMA path
	u1btmp = RTL_R8(REG_RXDMA_CONTROL);
	RTL_W8(REG_RXDMA_CONTROL, u1btmp & (~ BIT2));

	// Start Usb TxDMA
	RTL_W16(REG_PCIE_CTRL, 0x00 );
	RESTORE_INT(flags);
//	panic_printk("FBEAM_ERROR <==%s\n", __FUNCTION__);
}


#endif 


