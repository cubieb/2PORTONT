#ifndef __RTK_RG_ALG_PPTP_H__
#define __RTK_RG_ALG_PPTP_H__

#include <rtk_rg_alg_tool.h>
#include <rtk_rg_struct.h>

rtk_rg_fwdEngineReturn_t _rtk_rg_PPTP_GREModify(rtk_rg_naptDirection_t direct, struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_pptp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);

#endif
