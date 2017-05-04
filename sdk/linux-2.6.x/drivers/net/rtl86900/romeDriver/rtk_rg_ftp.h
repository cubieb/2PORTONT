#ifndef __RTK_RG_ALG_FTP_H__
#define __RTK_RG_ALG_FTP_H__

#include <rtk_rg_alg_tool.h>

#define ALG_FTP_PORT_STR			0x504f5254
#define ALG_FTP_PORT_LENGTH			5
#define ALG_FTP_PASV_RESP_STR		0x32323720

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_ftp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);


#endif
