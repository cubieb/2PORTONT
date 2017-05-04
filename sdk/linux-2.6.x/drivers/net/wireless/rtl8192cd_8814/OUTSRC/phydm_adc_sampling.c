#include "Mp_Precomp.h"
#include "phydm_precomp.h"
#include "Hal8814AReg.h"
#include "Hal8822BReg.h"
#include "HalJaguarPhyReg.h"
#if 0
BOOLEAN
ADCSmp_BufferAllocate(
	IN	PADAPTER			Adapter,
	IN 	PRT_ADCSMP			AdcSmp
	)
{
	PRT_ADCSMP_STRING	ADCSmpBuf = &(AdcSmp->ADCSmpBuf);

	if(ADCSmpBuf->Length == 0)
	{
		if(PlatformAllocateMemoryWithZero(Adapter, (void**)&(ADCSmpBuf->Octet), 0x10000) == RT_STATUS_SUCCESS)
			ADCSmpBuf->Length = 0x10000;
		else
			return FALSE;
	}

	return TRUE;
}
#endif

VOID
ADCSmp_GetTxPktBuf(
	IN		PVOID			pDM_VOID,
	IN 	PRT_ADCSMP_STRING		ADCSmpBuf
	)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte				i = 0, value32, DataL=0, DataH=0;
	u4Byte				Addr, Finish_Addr;//End_Addr = 0x3ffff;
	BOOLEAN				bRoundUp;
	static u4Byte			page=0xFF;

	memset(ADCSmpBuf->Octet, 0, ADCSmpBuf->Length);

	ODM_Write1Byte(pDM_Odm, REG_PKT_BUFF_ACCESS_CTRL_8814A, 0x69);
	//PlatformEFIOWrite1Byte(Adapter, REG_PKT_BUFF_ACCESS_CTRL_8814A, 0x69);
	//0x106[7:0]=0x69: access TXPKT Buffer
	//			0xA5: access RXPKT Buffer
	//			0x7F: access TXREPORT buffer

	DbgPrint("%s\n", __FUNCTION__);

	value32 = ODM_Read4Byte(pDM_Odm, REG_IQ_DUMP_8814A);
	bRoundUp = (BOOLEAN)((value32 & BIT31) >> 31);
	Finish_Addr = (value32 & 0x7FFF0000) >> 16; //Reg7C0[30:16]: finish addr (unit: 8byte)

	if(bRoundUp)
		Addr = (Finish_Addr+1)<<3;
	else	
		Addr = 0x30000;

	DbgPrint("bRoundUp = %d, Finish_Addr=0x%x, value32=0x%x \n", bRoundUp, Finish_Addr, value32);

	for(i = 0; i < (ADCSmpBuf->Length>>2); i+=2) //each ADC sample is 64bits
	{
		if (page != (Addr >> 12))
		{
			//Reg140=0x780+(Addr>>12), Addr=0x30~0x3F, total 16 pages
			page = (Addr >> 12);
			ODM_Write2Byte(pDM_Odm, REG_PKTBUF_DBG_CTRL_8814A, 0x780+page);
		}
		//pDataL = 0x8000+(Addr&0xfff);
		DataL = ODM_Read4Byte(pDM_Odm, 0x8000+(Addr&0xfff));
		DataH = ODM_Read4Byte(pDM_Odm, 0x8000+(Addr&0xfff)+4);
		//DataL = PlatformEFIORead4Byte(Adapter, pDataL);
		//DataH = PlatformEFIORead4Byte(Adapter, pDataL+4);
		//DbgPrint("page = 0x%x, Addr = 0x%x\n", page, Addr);

		ADCSmpBuf->Octet[i] = DataH;
		ADCSmpBuf->Octet[i+1] = DataL;

		//DbgPrint("%x%x\n", DataH, DataL);
		if((Addr+8) >= 0x40000)
			Addr = 0x30000;
		else
			Addr = Addr + 8;
	}

}	


VOID
ADCSmp_Start(
	IN		PVOID			pDM_VOID,
	IN 	PRT_ADCSMP			AdcSmp
	)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u1Byte					tmpU1b;
	PRT_ADCSMP_STRING		Buffer = &(AdcSmp->ADCSmpBuf);
	RT_ADCSMP_TRIG_SIG_SEL	TrigSigSel = AdcSmp->ADCSmpTrigSigSel;

	DbgPrint("%s \n", __FUNCTION__);

	ODM_SetBBReg(pDM_Odm , rDMA_trigger_Jaguar2, 0xf00, AdcSmp->ADCSmpDmaDataSigSel);	// 0x95C[11:8]	
	ODM_Write1Byte(pDM_Odm, REG_IQ_DUMP_8822B+1, AdcSmp->ADCSmpTriggerTime);

	if(AdcSmp->ADCSmpTrigSel == ADCSMP_MAC_TRIG) //trigger by MAC
	{
		if(TrigSigSel == ADCSMP_TRIG_REG) //manual trigger 0x7C0[5] = 0 -> 1
		{
			ODM_Write1Byte(pDM_Odm, REG_IQ_DUMP_8822B, 0xCB);		// 0x7C0[7:0]=8'b1100_1011
			ODM_Write1Byte(pDM_Odm, REG_IQ_DUMP_8822B, 0xEB);		// 0x7C0[7:0]=8'b1110_1011
		}	
		else if(TrigSigSel == ADCSMP_TRIG_CCA)
			ODM_Write1Byte(pDM_Odm, REG_IQ_DUMP_8822B, 0x8B);		// 0x7C0[7:0]=8'b1000_1011
		else if(TrigSigSel == ADCSMP_TRIG_CRCFAIL)
			ODM_Write1Byte(pDM_Odm, REG_IQ_DUMP_8822B, 0x4B);		// 0x7C0[7:0]=8'b0100_1011
		else if(TrigSigSel == ADCSMP_TRIG_CRCOK)
			ODM_Write1Byte(pDM_Odm, REG_IQ_DUMP_8822B, 0x0B);		// 0x7C0[7:0]=8'b0000_1011
	}
	else //trigger by BB
	{
		ODM_SetBBReg(pDM_Odm , rDMA_trigger_Jaguar2, 0x1f, TrigSigSel);	// 0x95C[4:0], 0x1F: trigger by CCA
		ODM_Write1Byte(pDM_Odm, REG_IQ_DUMP_8822B, 0x03);	// 0x7C0[7:0]=8'b0000_0011
	}

	// Polling
	do{
		tmpU1b = ODM_Read1Byte(pDM_Odm, REG_IQ_DUMP_8822B);

		if(AdcSmp->ADCSmpState != ADCSMP_STATE_SET)
			break;
		else if(tmpU1b & BIT1)
		{
			delay_us(AdcSmp->ADCSmpPollingTime);
			continue;
		}
		else
		{
			DbgPrint("%s Query OK\n", __FUNCTION__);
			break;
		}	
	}while(1);

	if(AdcSmp->ADCSmpState == ADCSMP_STATE_SET)
		ADCSmp_GetTxPktBuf(pDM_Odm, Buffer);

	if(AdcSmp->ADCSmpState == ADCSMP_STATE_SET)
		AdcSmp->ADCSmpState = ADCSMP_STATE_QUERY;

	AdcSmp->ADCSmpState = ADCSMP_STATE_IDLE;

	DbgPrint("%s Status %d \n", __FUNCTION__, AdcSmp->ADCSmpState);
}

#if 0
VOID
ADCSmpWorkItemCallback(
	IN	PVOID	pContext
	)
{
	PADAPTER			Adapter = (PADAPTER)pContext;
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(Adapter);
	PRT_ADCSMP			AdcSmp = &(pHalData->ADCSmp);

	ADCSmp_Start(Adapter, AdcSmp); 
}
#endif

VOID
ADCSmp_Set(
	IN		PVOID			pDM_VOID,
	IN	RT_ADCSMP_TRIG_SEL		TrigSel,
	IN	RT_ADCSMP_TRIG_SIG_SEL	TrigSigSel,
	IN	u1Byte					DmaDataSigSel,
	IN	u1Byte					TriggerTime,
	IN	u2Byte					PollingTime
	)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;
	BOOLEAN				retValue = TRUE;

	PRT_ADCSMP			AdcSmp = &(pDM_Odm->adcsmp);
	
	//DbgPrint("%s \n ADCSmpState = %d\n ADCSmpTrigSig = %d\n  ADCSmpTrigSigSel = %d\n ", 
	//		__FUNCTION__, AdcSmp->ADCSmpState, TrigSel, TrigSigSel);

	//DbgPrint("ADCSmpDmaDataSigSel = %d\n ADCSmpTriggerTime = %d\n ADCSmpPollingTime = %d\n ", 
	//		DmaDataSigSel, TriggerTime, PollingTime);

	AdcSmp->ADCSmpTrigSel = TrigSel;
	AdcSmp->ADCSmpTrigSigSel = TrigSigSel;
	AdcSmp->ADCSmpDmaDataSigSel = DmaDataSigSel;
	AdcSmp->ADCSmpTriggerTime= TriggerTime;
	AdcSmp->ADCSmpPollingTime = PollingTime;

	if(AdcSmp->ADCSmpState != ADCSMP_STATE_IDLE)
		retValue = FALSE;
	//else if(AdcSmp->ADCSmpBuf.Length == 0)
	//	retValue = ADCSmp_BufferAllocate(pDM_Odm, AdcSmp);

	if(retValue)
	{
		AdcSmp->ADCSmpState = ADCSMP_STATE_SET;
		ADCSmp_Start(pDM_Odm, AdcSmp); 
	}	

	DbgPrint("ADCSmpState %d Return Status %d\n", AdcSmp->ADCSmpState, retValue);
}

#if 0
RT_STATUS
ADCSmp_Query(
	IN	PADAPTER			Adapter,
	IN 	ULONG				InformationBufferLength, 
	OUT	PVOID				InformationBuffer, 
	OUT	PULONG				BytesWritten
	)
{
	RT_STATUS			retStatus = RT_STATUS_SUCCESS;
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(Adapter);
	PRT_ADCSMP			AdcSmp = &(pHalData->ADCSmp);
	PRT_ADCSMP_STRING	ADCSmpBuf = &(AdcSmp->ADCSmpBuf);

	DbgPrint("%s ADCSmpState %d ", __FUNCTION__, AdcSmp->ADCSmpState);

	if(InformationBufferLength != 0x10000)
	{
		*BytesWritten = 0;
		retStatus = RT_STATUS_RESOURCE;
	}	
	else if(ADCSmpBuf->Length != 0x10000)
	{
		*BytesWritten = 0;
		retStatus = RT_STATUS_RESOURCE;
	}	
	else if(AdcSmp->ADCSmpState != ADCSMP_STATE_QUERY)
	{
		*BytesWritten = 0;
		retStatus = RT_STATUS_PENDING;
	}
	else
	{
		PlatformMoveMemory(InformationBuffer, ADCSmpBuf->Octet, 0x10000);
		//move ADCSmpBuf->Octet to InformationBuffer
		*BytesWritten = 0x10000;

		AdcSmp->ADCSmpState = ADCSMP_STATE_IDLE;
	}

	DbgPrint("Return Status %d\n", retStatus);

	return retStatus;
}


VOID
ADCSmp_Stop(
	IN	PADAPTER			Adapter
	)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(Adapter);
	PRT_ADCSMP			AdcSmp = &(pHalData->ADCSmp);

	AdcSmp->ADCSmpState = ADCSMP_STATE_IDLE;

	//DbgPrint("%s status %d\n", __FUNCTION__, AdcSmp->ADCSmpState);
}


#endif

u1Byte ADC_buffer[0x10000];

VOID
ADCSmp_Init(
	IN		PVOID			pDM_VOID
	)
{
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;	
	PRT_ADCSMP			AdcSmp = &(pDM_Odm->adcsmp);
	PRT_ADCSMP_STRING	ADCSmpBuf = &(AdcSmp->ADCSmpBuf);

	AdcSmp->ADCSmpState = ADCSMP_STATE_IDLE;
	
	ADCSmpBuf->Octet = (pu4Byte) ADC_buffer;
	ADCSmpBuf->Length = 0x10000;
	
#if 0	
	PlatformInitializeWorkItem(
		Adapter,
		&(pHalData->ADCSmpWorkItem), 
		(RT_WORKITEM_CALL_BACK)ADCSmpWorkItemCallback, 
		(PVOID)Adapter,
		"ADCSmpWorkItem");
#endif
}

#if 0
VOID
ADCSmp_DeInit(
	PADAPTER		Adapter
	)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(Adapter);
	PRT_ADCSMP			AdcSmp = &(pHalData->ADCSmp);
	PRT_ADCSMP_STRING	ADCSmpBuf = &(AdcSmp->ADCSmpBuf);

	ADCSmp_Stop(Adapter);

	PlatformFreeWorkItem(&(pHalData->ADCSmpWorkItem));

	if(ADCSmpBuf->Length != 0x0)
	{
		PlatformFreeMemory(ADCSmpBuf->Octet, ADCSmpBuf->Length);
		ADCSmpBuf->Length = 0x0;
	}	
}	


VOID
Dump_MAC(
	PADAPTER		Adapter
	)
{

	u4Byte	Addr = 0;
	for(Addr = 0; Addr < 0x1A3D; Addr++)
	{
		DbgPrint("%04x %04x\n", Addr, PlatformEFIORead4Byte(Adapter, Addr));
	}
}


VOID
Dump_BB(
	PADAPTER		Adapter
	)
{
	u4Byte	Addr = 0;
	for(Addr = 0; Addr < 0x1AFD; Addr++)
	{
		DbgPrint("%04x %04x\n", Addr, PHY_QueryBBReg(Adapter, Addr, bMaskDWord));
	}
}


VOID
Dump_RF(
	PADAPTER		Adapter
	)
{
	u1Byte	Addr = 0, Path = 0;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	
	for(Path = ODM_RF_PATH_A; Path < pHalData->NumTotalRFPath; Path++)
	{
		for(Addr = 0; Addr < 0xF6; Addr++)
		{
			DbgPrint("%04x %04x\n", Addr, PHY_QueryRFReg(Adapter, Path, Addr, bRFRegOffsetMask));
		}
	}
}
#endif


