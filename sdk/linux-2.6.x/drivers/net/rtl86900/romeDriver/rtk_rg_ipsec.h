#ifndef __RTK_RG_IPSEC_H__
#define __RTK_RG_IPSEC_H__

#include <rtk_rg_alg_tool.h>

#define IPSEC_Max_ISAKMP 16 
#define IPSEC_AGING_TIME 30

//check the old rule(500-500) if any work
//#define IPSEC_OLD_RULE	1
#define IPSEC_HEADER_LEN	28
#define IPSEC_NATT_MD5_LEN	16

typedef struct rtk_rg_isakmp_s {
	int valid;
	uint64	icookie;	/* initiator cookie */
	uint64	rcookie;	/* responder cookie */
	
	uint32	peer_ip;
	uint32  local_ip;
	uint32  alias_ip;
	
	uint32	ospi;
	uint32	ispi;
	unsigned long  expire;
}rtk_rg_isakmp_t;

rtk_rg_fwdEngineReturn_t rtk_rg_alg_ESP(int direct, unsigned char *pSkb,unsigned char *pPktInfo, rtk_rg_isakmp_t * pIsakmp);

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_IPsec(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);


#endif
