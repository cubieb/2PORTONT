/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : Header File of PktTranslator Model Code for RTL8672
* Abstract :
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icModel_pktTranslator.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef __ICMODEL_PKTTRANLATOR_H__
#define __ICMODEL_PKTTRANLATOR_H__

#include "hsModel.h"
//#include "rtl8672_Proto.h"
#include "icModel_Packet.h"
#include "rtl8672_asicregs.h"

/* parent prototype of all status descriptions */
struct sts_desc{
	uint32 f1:20;
	uint32 data_length:12;
	uint32 rx_buffer_addr;
	uint32 rx_shift:8;
	uint32 f2:24;
	uint32 f3;
};

struct translator_param_table {
	int dest_port_number;	/* destination port number */
};

/* Function Prototype */
//void rtl8672_pktTranslator(struct rtl_pkt *pPkt, hsa_param_t *hsa, struct translator_param_table *ppt);
int rtl8672_pktTranslator(void *pDesc, hsa_param_t *hsa);

void model_pkttranslator(void);


#endif	/* __ICMODEL_PKTTRANLATOR_H__ */

