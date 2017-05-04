/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for RTL8672 Header Stamp Model
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: hsModel.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef __HS_MODEL_H__
#define __HS_MODEL_H__

#include "rtl_types.h"

#define _MTU 1518

/* parametets for packet generator */
struct pktGen_Param_Table
{
  uint8 IfType;
  uint8 L2Encap;
  uint8 SARhdr;
  uint8 ip_proto;	/* 6:TCP 17:UDP else w/o l4 layer packet */
  uint8 l2encap_flagType;
  uint8 compPPP;
  uint8 FCSSupp;
  uint8 ppp_proto;	
  uint8 frmctltype;  /* FRAMECTLTYPE1: IBSS, FRAMECTLTYPE2: from AP, FRAMECTLTYPE3: to AP, FRAMECTLTYPE4: WDS */
  uint8 AAL5Supp;
  uint8 CRCSupp;
  uint8 l3err;		/* 1 for ip checksum error */
  uint8 l4err;		/* 1 for tcp/udp checksum error */
  uint8 casttype;	/* o:unicast 1:multicast	2:broadcast */
};


#if 0
/* Interface table */
struct interface_table
{
	uint32 RSVD7:3;
	uint32 ATMPORT:1; /* only for SAR interface */
	uint32 RSVD8:3;	
	uint32 TRLREN:1;	/* only for SAR interface */
	uint32 RSVD9:3;
	uint32 CLP:1;	/* only for SAR interface */
	uint32 RSVD10:2;	
	uint32 PTI:2;	/* only for SAR interface */
	uint32 TRLR:16; /* only for SAR interface */
	
	char GMAC[6]; 	/* Gateway MAC Address */
	uint16 L2Encap:1;			/* only for SAR interface */
	uint16 LanFCS:1;			/* only for SAR interface */ /* 0:PID=00-07, 1:PID=00-01 */
	uint16 CompPPP:1;		/* only for SAR interface */ /* 0:the Protocol Field in PPP have 2 bytes*/
	uint16 IfType:2;
	uint16 SARhdr:3;			/* only for SAR interface */
	uint16 RXshift:8;
	uint32 GIP;		/* Gateway IP Address */
	uint32 RSVD1:2;
	uint32 AcceptTagged:1;	/* Accept VLAN Tagged: VID = 1~4095 */
	uint32 AcceptUntagged:1;	/* Accept VLAN Untagged or 802.1p Tagged */
	uint32 RSVD2:1;
	uint32 PPID:3;			/* Default Port Priority */
	uint32 RSVD3:1;
	uint32 SrcPortFilter:1;
	uint32 L2BRIDGE:1;
	uint32 Dot1QREMR:1;
	uint32 RSVD4:8;
	uint32 PVID:12;
	uint32 RSVD5:4;
	uint32 MTU:12;		/* Maxinum Transfer Unit */
	uint32 RSVD6:4;  
	uint32 VIDR:12;
};
#endif
	
enum IfType 
{
  		IF_ETHERNET = 0,
		IF_SAR,
		IF_WIRELESS,
		IFType_MAX
};
enum L2Encap
{
  VC_MULTIPLEXING = 0,
  LLC_ENCAPSULATION,
  L2Encap_MAX
};
enum SARhdr
{
  ROUTED = 0,
  BRIDGED,
  MER,
  PPPOE,
  PPPOA,
  SARhdr_MAX
};
enum LanFCS
{
  PID7 = 0,
  PID1,
  LanFCS_MAX
};

#if 0
/* HSB (Header Stamp Before):
 * Software-friendly structure definition */
struct rtl8672_hsb_param_s
{
/* word 0 */
#ifdef __LITTLE_ENDIAN
	uint32 vid:12;
	uint32 cfi:1;
	uint32 pid:3;
	uint32 linkid:7;
	uint32 l3hwfwdip:1;
	uint32 srcportidx:4;
	uint32 pif:1;
	uint32 tif:1;
	uint32 hasmac:1;
	uint32 fromcpu:1;
#else
	uint32 fromcpu:1;
	uint32 hasmac:1;
	uint32 tif:1;
	uint32 pif:1;
	uint32 srcportidx:4;
	uint32 l3hwfwdip:1;
	uint32 linkid:7;
	uint32 pid:3;
	uint32 cfi:1;
	uint32 vid:12;
#endif	

/* word 1 */
#ifdef __LITTLE_ENDIAN
	uint32 framectrl:16;
	uint32 rsvd_2:16;
#else
	uint32 rsvd_2:16;
	uint32 framectrl:16;
#endif

/* word 2 */
#ifdef __LITTLE_ENDIAN
	uint8 dmac4;
	uint8 dmac5;
	uint32 ethtp:16;
#else
	uint32 ethtp:16;
	uint8 dmac5;
	uint8 dmac4;
#endif

/* word 3 */
#ifdef __LITTLE_ENDIAN
	uint8 dmac0;
	uint8 dmac1;
	uint8 dmac2;
	uint8 dmac3;
#else
	uint8 dmac3;
	uint8 dmac2;
	uint8 dmac1;
	uint8 dmac0;
#endif

/*word 4 */
#ifdef __LITTLE_ENDIAN
	uint8 smac2;
	uint8 smac3;
	uint8 smac4;
	uint8 smac5;
#else
	uint8 smac5;
	uint8 smac4;
	uint8 smac3;
	uint8 smac2;
#endif

/* word 5 */
#ifdef __LITTLE_ENDIAN
	uint8 mac3_4;
	uint8 mac3_5;
	uint8 smac0;
	uint8 smac1;
#else
	uint8 smac1;
	uint8 smac0;
	uint8 mac3_5;
	uint8 mac3_4;
#endif

/*word 6 */
#ifdef __LITTLE_ENDIAN
	uint8 mac3_0;
	uint8 mac3_1;
	uint8 mac3_2;
	uint8 mac3_3;
#else
	uint8 mac3_3;
	uint8 mac3_2;
	uint8 mac3_1;
	uint8 mac3_0;
#endif

/* word 7 */
#ifdef __LITTLE_ENDIAN
	uint8 mac4_4;
	uint8 mac4_5;
	uint32 sid:16;
#else
	uint32 sid:16;
	uint8 mac4_5;
	uint8 mac4_4;
#endif

/* word 8 */
#ifdef __LITTLE_ENDIAN
	uint8 mac4_0;
	uint8 mac4_1;
	uint8 mac4_2;
	uint8 mac4_3;
#else
	uint8 mac4_3;
	uint8 mac4_2;
	uint8 mac4_1;
	uint8 mac4_0;
#endif

/*word 9 */
#ifdef __LITTLE_ENDIAN
	uint32 l3len:16;
	uint32 l3cs:16;	
#else
	uint32 l3cs:16;	
	uint32 l3len:16;
#endif

/* word 10 */
	uint32 sip;
/* word 11 */
	uint32 dip;

/* word 12 */
#ifdef __LITTLE_ENDIAN
	uint32 reason:3;
	uint32 l4csok:1;
	uint32 udpnocs:1;
	uint32 l3csok:1;
	uint32 ipfrag:1;
	uint32 l3type:1;
	uint32 ttl:8;
	uint32 l3offset:8;
	uint32 tos:8;
#else
	uint32 tos:8;
	uint32 l3offset:8;
	uint32 ttl:8;
	uint32 l3type:1;
	uint32 ipfrag:1;
	uint32 l3csok:1;
	uint32 udpnocs:1;
	uint32 l4csok:1;
	uint32 reason:3;
#endif

/* word 13 */
#ifdef __LITTLE_ENDIAN
	uint32 l4cs:16;
	uint32 tcpflag:8;
	uint32 l4proto:8;
#else
	uint32 l4proto:8;
	uint32 tcpflag:8;
	uint32 l4cs:16;
#endif

/* word 14 */
#ifdef __LITTLE_ENDIAN
	uint32 dport:16;
	uint32 sport:16;
#else
	uint32 sport:16;
	uint32 dport:16;
#endif	
};
typedef struct rtl8672_hsb_param_s hsb_param_t;

/* HSA (Header Stamp After):
 * Software-friendly structure definition */
struct rtl8672_hsa_param_s
{
#ifdef __LITTLE_ENDIAN
	uint32 srcportidx:4;
	uint32 dstportidx:4;
	uint32 linkid:7;
	uint32 tif:1;
	uint32 iniftype:2;
	uint32 tocpu:1;
	uint32 fromcpu:1;
	uint32 l4change:1;
	uint32 l3change:1;
	uint32 pif:1;
	uint32 droppacket:1;
	uint32 outsarhdr:3;
	uint32 outcompppp:1;
	uint32 outlanfcs:1;
	uint32 outl2encap:1;
	uint32 outiftype:2;
#else
	uint32 outiftype:2;
	uint32 outl2encap:1;
	uint32 outlanfcs:1;
	uint32 outcompppp:1;
	uint32 outsarhdr:3;
	uint32 droppacket:1;
	uint32 pif:1;
	uint32 l3change:1;
	uint32 l4change:1;
	uint32 fromcpu:1;
	uint32 tocpu:1;
	uint32 iniftype:2;
	uint32 tif:1;
	uint32 linkid:7;
	uint32 dstportidx:4;
	uint32 srcportidx:4;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 framectrl:16;
	uint32 ethtp:16;
#else
	uint32 ethtp:16;
	uint32 framectrl:16;
#endif

#ifdef __LITTLE_ENDIAN
	uint8 dmac2;
	uint8 dmac3;
	uint8 dmac4;
	uint8 dmac5;
#else
	uint8 dmac5;
	uint8 dmac4;
	uint8 dmac3;
	uint8 dmac2;
#endif

#ifdef __LITTLE_ENDIAN
	uint8 smac4;
	uint8 smac5;
	uint8 dmac0;	
	uint8 dmac1;
#else
	uint8 dmac1;
	uint8 dmac0;
	uint8 smac5;
	uint8 smac4;
#endif

#ifdef __LITTLE_ENDIAN
	uint8 smac0;
	uint8 smac1;
	uint8 smac2;
	uint8 smac3;
#else
	uint8 smac3;
	uint8 smac2;
	uint8 smac1;
	uint8 smac0;
#endif

#ifdef __LITTLE_ENDIAN
	uint8 mac3_2;
	uint8 mac3_3;
	uint8 mac3_4;
	uint8 mac3_5;
#else
	uint8 mac3_5;
	uint8 mac3_4;
	uint8 mac3_3;
	uint8 mac3_2;
#endif

#ifdef __LITTLE_ENDIAN
	uint16 sid;
	uint8 mac3_0;
	uint8 mac3_1;
#else
	uint8 mac3_1;
	uint8 mac3_0;
	uint16 sid;
#endif

#ifdef __LITTLE_ENDIAN
	uint8 mac4_2;
	uint8 mac4_3;
	uint8 mac4_4;
	uint8 mac4_5;
#else
	uint8 mac4_5;
	uint8 mac4_4;
	uint8 mac4_3;
	uint8 mac4_2;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 vid:12;
	uint32 cfi:1;
	uint32 pid:3;
	uint8 mac4_0;
	uint8 mac4_1;
#else
	uint8 mac4_1;
	uint8 mac4_0;
	uint32 pid:3;
	uint32 cfi:1;
	uint32 vid:12;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 inl3offset:8;
	uint32 ttl:8;
	uint32 tos:8;
	uint32 tosremr:1;
	uint32 l3type:1;
	uint32 rsvd_1:6;
#else
	uint32 rsvd_1:6;
	uint32 l3type:1;
	uint32 tosremr:1;
	uint32 tos:8;
	uint32 ttl:8;
	uint32 inl3offset:8;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 l3cs:16;
	uint32 inl3len:16;
#else
	uint32 inl3len:16;
	uint32 l3cs:16;
#endif	

	uint32 sip;
	uint32 dip;

#ifdef __LITTLE_ENDIAN	
	uint32 dport:16;
	uint32 sport:16;
#else
	uint32 sport:16;
	uint32 dport:16;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 reason:5;
	uint32 rsvd_2:3;
	uint32 l4proto:8;
	uint32 l4cs:16;
#else
	uint32 l4cs:16;
	uint32 l4proto:8;
	uint32 rsvd_2:3;
	uint32 reason:5;
#endif
};
typedef struct rtl8672_hsa_param_s hsa_param_t;
#endif

typedef struct rtl8672_tblAsicDrv_hsbParam_s hsb_param_t;
typedef struct rtl8672_tblAsicDrv_hsaParam_s hsa_param_t;


/* To directly access ASIC-mapped header stamp, we must set pack pragma. */
#pragma pack(push,1)

/* RAW HSB: Raw structure to access ASIC.
 * The structure is directly mapped to ASIC, however, it is not friendly for software. */
struct hsb_s
{
	/* [FIXME] */
};
typedef struct hsb_s hsb_t;


/* RAW HSA: Raw structure to access ASIC.
 * The structure is directly mapped to ASIC, however, it is not friendly for software. */
struct hsa_s
{
	/* [FIXME] */
};
typedef struct hsa_s hsa_t;

#pragma pack(pop)
/* Recover pragma setting */

enum HSB_TYPE 
{
	HSB_TYPE_ETHERNET = 0,
	HSB_TYPE_IPV4 = 1,
	HSB_TYPE_IPV6 = 2,
};

enum HSA_TYPE 
{
	HSA_TYPE_ETHERNET = 0,
	HSA_TYPE_IPV4 = 1,
	HSA_TYPE_IPV6 = 2,
};


#endif /* __HS_MODEL_H__ */
