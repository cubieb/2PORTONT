#ifndef __RTK_RG_ALG_SIP_H__
#define __RTK_RG_ALG_SIP_H__

#include <rtk_rg_alg_tool.h>

#define _RTL8651_L4_MAX_SIP_ALIAS_BUF_SIZE 1600

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_sip(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);


#endif

