/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXGen.c
	
Abstract:
	Defined RTL8192EE HAL Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#include "../../../HalPrecomp.h"
#include "../../../../8192cd.h"

typedef enum _RTL8881A_C2H_EVT
{
	C2H_8881A_DBG = 0,
	C2H_8881A_LB = 1,
	C2H_8881A_TXBF = 2,
	C2H_8881A_TX_REPORT = 3,
	C2H_8881A_TX_RATE =4,
	C2H_8881A_BT_INFO = 9,
	C2H_8881A_BT_MP = 11,
	C2H_8881A_RA_RPT = 12,
#ifdef TXRETRY_CNT
	C2H_8881A_TX_RETRY = 13, //0x0D
#endif
	C2H_8881A_RA_PARA_RPT = 14,
	C2H_8881A_EXTEND_IND = 0xFF,
	MAX_8881A_C2HEVENT
}RTL8881A_C2H_EVT;

typedef enum _RTL8881A_EXTEND_C2H_EVT
{
	EXTEND_C2H_8881A_DBG_PRINT = 0

}RTL8881A_EXTEND_C2H_EVT;

VOID
C2HTxTxReportHandler_8881A(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	struct tx_rpt rpt1;
	int k=0, j=0;
	for(j=0; j<2; j++) {
		rpt1.macid= CmdBuf[k];
		rpt1.txok = CmdBuf[k+1] | ((short)CmdBuf[k+2]<<8);
		rpt1.txfail = CmdBuf[k+3] | ((short)CmdBuf[k+4]<<8);
		rpt1.initil_tx_rate = CmdBuf[k+5];
		if(rpt1.macid != 0xff)
			txrpt_handler(priv, &rpt1);
		k+=6;
	}
}


VOID
_C2HContentParsing8881A(
	struct rtl8192cd_priv *priv,
		u1Byte			c2hCmdId, 
		u1Byte			c2hCmdLen,
		pu1Byte 			tmpBuf
)
{
	u1Byte	Extend_c2hSubID = 0;

	switch(c2hCmdId)
	{
		case C2H_8881A_TX_RATE:
#ifdef TXREPORT	
			C2HTxTxReportHandler_8881A(priv, tmpBuf, c2hCmdLen);

#ifdef TXRETRY_CNT
			requestTxRetry88XX(priv);
#else
			requestTxReport88XX(priv);
#endif			
#endif		
			break;
		
#ifdef TXRETRY_CNT
		case C2H_8881A_TX_RETRY:
			C2HTxTxRetryHandler(priv, tmpBuf);
			requestTxReport88XX(priv);	
			break;
#endif
		case C2H_8881A_RA_PARA_RPT:
			ODM_C2HRaParaReportHandler(ODMPTR, tmpBuf, c2hCmdLen);
			break;
			
		case C2H_8881A_RA_RPT:
			phydm_c2h_ra_report_handler(ODMPTR, tmpBuf, c2hCmdLen);
			break;
			
		case C2H_8881A_EXTEND_IND:	
			Extend_c2hSubID= tmpBuf[0];
			if(Extend_c2hSubID == EXTEND_C2H_8881A_DBG_PRINT)
			{
				phydm_fw_trace_handler_8051(ODMPTR, tmpBuf, c2hCmdLen);
			}
			break;
			
		default:
			break;
	}
}



VOID
C2HPacketHandler_8881A(
	struct rtl8192cd_priv *priv,
		pu1Byte			Buffer,
		u1Byte			Length
	)
{
	u1Byte	c2hCmdId=0, c2hCmdSeq=0, c2hCmdLen=0;
	pu1Byte tmpBuf=NULL;
	c2hCmdId = Buffer[0];
	c2hCmdSeq = Buffer[1];
	c2hCmdLen = Length -2;
	tmpBuf = Buffer+2;
	_C2HContentParsing8881A(priv, c2hCmdId, c2hCmdLen, tmpBuf);
}


