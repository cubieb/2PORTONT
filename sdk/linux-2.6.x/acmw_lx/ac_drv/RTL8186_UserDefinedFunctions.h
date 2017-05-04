	
#ifndef	_RTL8186_USER_DEFINED_FUNCTIONS_
#define	_RTL8186_USER_DEFINED_FUNCTIONS_

#include "AC49xDrv_Drv.h"
#include "dsp_drv.h"
#include "Ac49xDrv_Ethernet_Definitions.h"
#include "userdef.h"

#define ALEXHOSTOFFSET 24;

typedef volatile struct {

	long ActiveMedia;
	Tacmw_MediaRoutingConfigurationPayload MediaRouting;
	
} Trtl8186MediaRoute;

int 	Rtl8186_SetMediaSession(Tac49xTxPacketParams	*pTxPacketParams);
int 	Rtl8186_UnSetMediaSession(Tac49xTxPacketParams	*pTxPacketParams);
int 	Rtl8186_PcmReset(int device, int channel);/*(Tac49xTxPacketParams 			*pTxPacketParams);*/
int 	Rtl8186_PcmEnable(int device, int channel);/*(Tac49xTxPacketParams 		*pTxPacketParams);*/
int 	Rtl8186_PcmDisable(int device, int channel);/*(Tac49xTxPacketParams 		*pTxPacketParams);*/
int 	Rtl8186_Ring(int device, int channel);/*(Tac49xTxPacketParams				*pTxPacketParams);*/
int 	Rtl8186_HookStatus(int device, int channel);/*(Tac49xTxPacketParams	*pTxPacketParams);*/



extern Trtl8186MediaRoute *pRtl8186MediaRouteTbl[PCM_CH_NUM];

#endif
