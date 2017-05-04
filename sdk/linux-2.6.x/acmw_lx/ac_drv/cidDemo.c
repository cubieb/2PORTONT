/****************************************************************************
*
*	Company:			Audiocodes Ltd.
*
*   Project:			VoPP SoC
*
*   Hardware Module: 	AC494-EVM
*
*   File Name: 			cid_demo.c
*
*
******************************************************************************
*
* 	DESCRIPTION:		PSEDO CODE DEMO FOR CID_TX API's
*
******************************************************************************/

#include "AC49xDrv_Drv.h"
#include "CidSampleTbl.h"

// CID Message Def see CidSampleTbl.h
//#define CID_MDMF_LEN 41
//extern char CID_MDMF1[];		//CID Message TABLE
//extern char CID_MDMF2[];		//CID Message TABLE
// SEND CID API
//extern int Ac49xSendCallerIdMessage(int Device, int Channel, Tac49xCallerIdAttr *pAttr, U8 *pMessage);


#define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF 			0x0	/* FOR DTMF BASED CID */
#define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF 	0x1	/* FOR FSK BASED CID*/



Tac49xCallerIdAttr 		callerIdAttr;
Tac49xCallerIdAsciMessage 	callerIdAsciMessage;
Tac49xCallerIdAttr 		*pCallerIdAttr;
Tac49xCallerIdAsciMessage 	*pCallerIdAsciMessage;


void SEND_CALLERID_DEMO(void)
{
	long device = 0;	//always 0
	long channel = 0;
	///////////	SET CALLER ID ETSI FSK TYPE 1 ATTRIBUTES //////////		
	
	pCallerIdAttr = &callerIdAttr;
	//CID STANDARD
	//CALLER_ID_STANDARD__TELCORDIA_BELLCORE, CALLER_ID_STANDARD__ETSI, CALLER_ID_STANDARD__NTT
	pCallerIdAttr->Standard 			= CALLER_ID_STANDARD__ETSI;
	//CID SERVICE TYPE
	//CALLER_ID_SERVICE_TYPE__1_ON_HOOK, CALLER_ID_SERVICE_TYPE__2_OFF_HOOK
	pCallerIdAttr->ServiceType 		= CALLER_ID_SERVICE_TYPE__1_ON_HOOK;
	//for ETSI TYPE1 
	//ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL, ETSI_ON_HOOK_METHOD__WITH_PRECEDING_DUAL_TONE_ALERT_SIGNAL
	pCallerIdAttr->EtsiOnhookMethod 	= ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL;
	
	pCallerIdAttr->ChecksumByteAbsent = CONTROL__DISABLE;	//defualt Value for command
	pCallerIdAttr->Event 	= CALLER_ID_EVENT__NONE_EVENT;		//defualt Value for command

	///////////	SET CALLER ID MESSAGE //////////		
	pCallerIdAsciMessage = &callerIdAsciMessage;
	//DTMF FSK based CID
	//CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF, CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF
	pCallerIdAsciMessage->Title.Type.Value = (char)CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF;
	//CALLER ID MESSAGE LENGTH (in BYTE)
	pCallerIdAsciMessage->Title.Length.Value = (char)CID_MDMF_LEN;
	memcpy(&pCallerIdAsciMessage->Data[0], (char *)&CID_MDMF1[0], pCallerIdAsciMessage->Title.Length.Value);

	//SEND CID API
	Ac49xSendCallerIdMessage(device, channel, pCallerIdAttr, pCallerIdAsciMessage);


	///////////	SET CALLER ID ETSI DTMF TYPE 1 ATTRIBUTES //////////

	pCallerIdAttr = &callerIdAttr;
	pCallerIdAttr->Standard 			= CALLER_ID_STANDARD__DTMF_CLIP_ETSI;
	pCallerIdAttr->ServiceType 		= CALLER_ID_SERVICE_TYPE__1_ON_HOOK;
	pCallerIdAttr->EtsiOnhookMethod 	= ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL;	
	pCallerIdAttr->ChecksumByteAbsent = CONTROL__DISABLE;
	pCallerIdAttr->Event 				= CALLER_ID_EVENT__NONE_EVENT;	

	///////////	SET CALLER ID MESSAGE //////////		
	pCallerIdAsciMessage = &callerIdAsciMessage;
	//DTMF FSK based CID
	//CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF, CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF
	pCallerIdAsciMessage->Title.Type.Value = (char)CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF;
	//CALLER ID MESSAGE LENGTH (in BYTE)
	pCallerIdAsciMessage->Title.Length.Value = 0x7;
	pCallerIdAsciMessage->Data[0] = 'A';	//start preambel ('A' or 'D')
	pCallerIdAsciMessage->Data[1] = '1';
	pCallerIdAsciMessage->Data[2] = '1';
	pCallerIdAsciMessage->Data[3] = '1';
	pCallerIdAsciMessage->Data[4] = '1';
	pCallerIdAsciMessage->Data[5] = '1';
	pCallerIdAsciMessage->Data[6] = 'C';	//stop preambel ('C')

	//SEND CID API
	Ac49xSendCallerIdMessage(device, channel, pCallerIdAttr, pCallerIdAsciMessage);

 }
