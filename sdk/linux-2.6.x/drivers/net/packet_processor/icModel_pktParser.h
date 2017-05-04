/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : Header File of PktParser Model Code for RTL8672
* Abstract :
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icModel_pktParser.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef __ICMODEL_PKTPARSER_H__
#define __ICMODEL_PKTPARSER_H__

#include "rtl8672_tblAsicDrv.h"
#include "rtl8672_Proto.h"

/* Reduce the MAC operation */
#define	MAC_EQUAL(mac1, mac2)	((mac1[0]==mac2[0])&&(mac1[1]==mac2[1])&&(mac1[2]==mac2[2])&&(mac1[3]==mac2[3])&&(mac1[4]==mac2[4])&&(mac1[5]==mac2[5]))
#define	MAC_COPY(mac1, mac2)	{ mac1[0]=mac2[0]; mac1[1]=mac2[1]; mac1[2]=mac2[2]; mac1[3]=mac2[3]; mac1[4]=mac2[4]; mac1[5]=mac2[5]; }

/* Function Prototype */
//void rtl8672_pktParser(struct rtl_pkt *pPkt, hsb_param_t *hsb);
//void rtl8672_pktParser(struct rtl_pkt *pPkt, hsb_param_t *hsb, struct interface_table *iftbl, struct parser_param_table *ppt);
int rtl8672_pktParser(int srcPortIdx,void* pDesc,hsb_param_t *hsb, struct rtl8672_tblAsicDrv_intfParam_s *iftbl);
int _EthernetpktGen(uint8 *pPkt, struct pktGen_Param_Table *ptbl);
int _EthernetLongpktGen(uint8 *pPkt, struct pktGen_Param_Table *ptbl, uint32 lenpayload);
int _SARpktGen(uint8 *pPkt, struct pktGen_Param_Table *ptbl);
int _SARLongpktGen(uint8 *pPkt, struct pktGen_Param_Table *ptbl, uint32 lenpayload);
int _WLANpktGen(uint8 *pPkt, struct pktGen_Param_Table *ptbl);
int _WLANLongpktGen(uint8 *pPkt, struct pktGen_Param_Table *ptbl, uint32 lenpayload);
int _rtl8672_PktGen(uint8 *pPkt, struct pktGen_Param_Table *ptbl,uint32 lenpayload);

uint16 model_ipChecksum(struct ip *pip);
uint16 model_tcpChecksum(struct ip *pip);

void model_pktparser(void);

#endif	/* __ICMODEL_PKTPARSER_H__ */

