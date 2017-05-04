/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : Header File of ALE(Address Lookup Engine) for RTL8672
* Abstract :
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icModel_ALE.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef __ICMODEL_ALE_H__
#define __ICMODEL_ALE_H__

/* Reduce the interface operation */
//#define	IFTBL(index, field)	(((struct interface_table *)(intf_tbl + index))->field)

/* Reduce the MAC operation */
#define	MAC_EQUAL(mac1, mac2)	((mac1[0]==mac2[0])&&(mac1[1]==mac2[1])&&(mac1[2]==mac2[2])&&(mac1[3]==mac2[3])&&(mac1[4]==mac2[4])&&(mac1[5]==mac2[5]))
#define	MAC_COPY(mac1, mac2)	{ mac1[0]=mac2[0]; mac1[1]=mac2[1]; mac1[2]=mac2[2]; mac1[3]=mac2[3]; mac1[4]=mac2[4]; mac1[5]=mac2[5]; }


/* ALE Process Status */
enum ALE_STATUS
{
	AS_NONE = 0,
	AS_INIT,
	AS_FROMCPUCHK,
	AS_DIRECTTX,
	AS_REASONCHK,
	AS_TAGACCEPTCHK,
	AS_PIDTAGCHK,
	AS_PIDPNPCHK,
	AS_VIDTAGCHK,
	AS_VIDPNPCHK,
	AS_MACCHK,
	AS_L2VALIDCHK,
	AS_L2SMACCHK,
	AS_L2GMACCHK,
	AS_L2BRIDGECHK,
	AS_L2DMACCHK,
	AS_L3HEADERCHK,
	AS_L3CHKSUMCHK,
	AS_L3TTLCHK,
	AS_L3IPECNCHK,
	AS_L3IPFRAGCHK,
	AS_L3GIPCHK,
	AS_L4VALIDCHK,
	AS_L4CHKSUMCHK,
	AS_L4PROTOCHK,
	AS_L4TCPFLAGCHK,
	AS_L4TCPECNCHK,
	AS_L4FLOWCHK,
	AS_L3TTLDECHK,
	AS_L2MTUCHK,
	AS_TOCPUCHK,
	AS_FORWARD,
	AS_EXIT
};


extern void *ale_control_registers_base;
extern void *ale_l2table_realbase;
extern void *ale_l4table_realbase;


/* Function Prototype */
void model_ale_init(void);
void model_ale_exit(void);

void rtl8672_ALE(hsb_param_t *hsb, hsa_param_t *hsa);

void model_pktale(void);




#endif	/* __ICMODEL_ALE_H__ */

