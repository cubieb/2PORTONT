#ifndef __RTK_RG_L2TP_H__
#define __RTK_RG_L2TP_H__

#include <rtk_rg_alg_tool.h>

typedef struct rtk_rg_l2tp_header_s
{
	uint8 code;
	uint8 version;
	uint16 length;
	uint16 tunnelId;
	uint16 sessionId;
	uint16 Ns;
	uint16 Nr;
	uint16 offsetSize;
	unsigned int dataOff;
	uint16 controlType;
}rtk_rg_l2tp_header_t;

typedef struct rtk_rg_l2tp_avp_s
{
	uint8 mask;
	uint8 length; /* the length field is actually 10 bits */
	uint16 vendorId;
	uint16 attrType;
	uint8 attrValue[1];
}rtk_rg_l2tp_avp_t;

int rtk_rg_algRegFunc_l2tp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);


#endif
