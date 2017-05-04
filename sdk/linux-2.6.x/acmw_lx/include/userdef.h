#ifndef USERDEF_H
#define USERDEF_H

#include "../rtk_voip/include/rtk_voip.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC
#define ACMW_MAX_CHANNELS	4
#elif defined (CONFIG_RTK_VOIP_DRIVERS_IP_PHONE)
#define ACMW_MAX_CHANNELS	2
#else
#error "ACMW_MAX_CHANNELS is not defined" 
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#define ACMW_MAX_AEC_NUM_CH	1
#endif
/***********************************************************************
    Application Specific Constants Definitions:
     ( To be set by user according to user's hardware/application )
************************************************************************/

#if (ACMW_MAX_CHANNELS > MAX_VOIP_CH_NUM)
	#ERROR ACMW_MAX_CHANNELS > MAX_VOIP_CH_NUM)
#endif

/***********************************************************************
    General Macros:
************************************************************************/

#undef IP2LONG
#define IP2LONG(a,b,c,d) ((unsigned int)a<<24 | (unsigned int)b<<16 | (unsigned int)c<<8 | (unsigned int)d)


/***********************************************************************
    RTP TRAP Def.
************************************************************************/
#define UDP_PROTOCOL	  0x11	
#define TCP_PROTOCOL	  0x10


#endif  /* end of #ifndef USERDEF_H */
