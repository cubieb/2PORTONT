/*
** Copyright (c) 2010 by Silicon Laboratories
**
** $Id: proslic_mlt_dcfeed.c 4550 2014-10-27 20:57:00Z nizajerk $
**
*/
/*! \file proslic_mlt_dcfeed.c
**  \brief DC feed setup during MLT tests
**
** This is the implementation file for the ProSLIC DC feed setup during MLT tests
**
** \author Silicon Laboratories, Inc (cdp)
**
** \attention
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
** This code may produce different results on different host processors 
** 
*/
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "proslic.h"
#define SI_USE_IFACE_MACROS
#include "proslic_mlt.h"

#include "mlt_comm_regs.h"


/*
**
*/
void setupDcFeedCloseToZero(ProSLICMLTType *pProSLICMLT){
	/*sets dc feed close to zero vcm, and vdiff*/
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1C206275L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1A10433FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x3E0BD90L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1E0FA137L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x150D28L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0xC0784L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x603C2L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0xDF58CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x94E5DL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0xAA10L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x10624EL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	if (pProSLICMLT->pProslic->deviceId->chipRev == 0)
		pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_AUTO,0x1e);
	else
		pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_AUTO,0x1f);
}



/*
**
*/
void setupDcFeed10VLONG (ProSLICMLTType *pProSLICMLT){
	/*set up dc feed for longitudinal resistance measurement V1*/
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x18842BD7L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1F389145L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x1ECE8A98L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1EF744EAL  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x2657F6FL  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0x21D5253L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x168E18CL  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0x8B9779L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x5D0FA6L );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x135C2CL  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x116872BL  );

	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	if (pProSLICMLT->pProslic->deviceId->chipRev == 0)
		pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_AUTO,0x1e);
	else
		pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_AUTO,0x1f);
}


/*
**
*/
void setupDcFeed50VLONG (ProSLICMLTType *pProSLICMLT){
	/*set up dc feed for longitudinal resistance measurement V2*/
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1D28720FL  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1F5E4A5BL  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x1C617135L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1D6A2C4AL  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x476CA3EL  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0x3C25977L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x2EFD5E5L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0xB026C3L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x5D0FA6L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x241016L  );
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x20C49BAL  );

	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	if (pProSLICMLT->pProslic->deviceId->chipRev == 0)
		pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_AUTO,0x1e);
	else
    {
		pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_AUTO,0x1f);
    }
}



/*
**
*/
void setupDcFeed35VCM (ProSLICMLTType *pProSLICMLT){
	/*
    ** V_VLIM = 2v    V_RFEED = 0.8v    V_ILIM = 0.2V   VCM_OH = 30v    VOV_BAT = 0v    VOV_GND = 0v
    ** I_VLIM = 0mA   I_RFEED = 5mA     I_ILIM = 8mA
    */
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1B1852DAL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1A10433FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x7C17B20L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1E72E75FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x3C2597L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0x180F09L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x603C2L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0x29E0A4L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x253975L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x1E5E6L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x1EB851FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	if (pProSLICMLT->pProslic->deviceId->chipRev == 0)
		pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_AUTO,0x1e);
	else
		pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_AUTO,0x1f);
}



/*
**
*/
void setupDcFeed35V (ProSLICMLTType *pProSLICMLT){
	/*set up dcfeed for 35V, 0V VTIP - no overhead*/
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1C8A024CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1F03C159L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x1CC4B75DL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1DD87A3EL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x41C91DBL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0x38633E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x2EFD5E5L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0xD16335L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x5D0FA6L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x21373DL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x11EB852L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	
}


/*
**
*/
void setupDcFeedV1SmallR (ProSLICMLTType *pProSLICMLT){
	/*set up dcfeed for 40V 14mA so that we are in constant current region for our small R measurement < 2.5k*/
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1A10433FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1DC6E1D3L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x1DD87A3EL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1EB51625L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x4B2EFD5L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0x476CA3EL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x43AA4A6L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0x19029B0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x41248DL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x25F5FDL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x147AE14L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	
}


/*
**
*/
void setupDcFeedV2SmallR (ProSLICMLTType *pProSLICMLT){
	/*set up dcfeed for 35V, 7ma so that we are in constant current region for our small R measurement < 2.5k*/
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1E3081AAL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1F03C159L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x164E2617L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1C1F426FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x50D2839L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0x4B2EFD5L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x476CA3EL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0xD16335L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x209246L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x28CED6L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x1604189L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	
}


/*
**
*/
void setupDcFeed45V (ProSLICMLTType *pProSLICMLT){
	/*set up dcfeed for 45V, 0V VTIP - no overhead*/
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1C8A024CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1F03C159L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x1CC4B75DL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1DD87A3EL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x5494DD0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0x4B2EFD5BL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x41C91DBL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0xFFEB09L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x5D0FA6L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x2AB4BCL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x170A3D7L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	
}


/*
**
*/
void setupDcFeedForRENTest (ProSLICMLTType *pProSLICMLT){
	
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1A10433FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1D28720FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0x1D6A2C4AL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1EB51625L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x12CBBF5L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0xD28392L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x965DFAL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0x97396EL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x5D0FA6L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x97D7FL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x23D70A4L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0x500000L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0x500000L);

}


/*
**
*/
void setupDcFeedAutoV (ProSLICMLTType *pProSLICMLT){
	/*set up dcfeed for 45V, 0V VTIP - no overhead*/
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_VLIM,0x1E655196);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RFEED,0x1F389145);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_ILIM,0x40A0E0L);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1CFCA14CL);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA1,0xF64E2617);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_SLOPE_DELTA2,0x1BB0F47C);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_VLIM,0x5A38633);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_RFEED,0x476CA3E);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_V_ILIM,0x3C25977);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_RFEED,0xD92483);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_CONST_ILIM,0x5D0FA6);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_I_VLIM,0x2D8D96);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VCM_OH,0x189374B);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_BAT,0);
	pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_VOV_GND,0);
	
}



