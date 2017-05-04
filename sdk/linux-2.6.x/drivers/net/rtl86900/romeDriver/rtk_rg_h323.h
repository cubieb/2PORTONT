#ifndef __RTK_RG_H323_H__
#define __RTK_RG_H323_H__

#define H323_TPKT_STR			0x03
#define H323_TPKT_RESERVE	    0x00

#define H323_SUCCESS 0	    
#define H323_STOP 1	/* just stop process, not  an fail */
#define H323_FAIL -1   
#define H323_FORMAT_ERROR -2  /* the packet data is not transmitted correctly*/

#include <rtk_rg_struct.h>

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_TCP_h323(int direct, int after, unsigned char *pSkb,unsigned char * pPktInfo);

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_UDP_ras(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);


#endif