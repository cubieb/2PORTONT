/*
 * Copyright (c) 2007 Realtek Semiconductor Corporation.
 *
 * Program : Header File of RTL8672 Asic Driver
 * Abstract :
 * Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
 * $Id: rtl8672_tblAsicDrv.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
 */

#ifndef	__RTL8672_TBLASICDRV_H__
#define	__RTL8672_TBLASICDRV_H__


#define	RTL8672_NETIFTBL_SIZE	16
#define	RTL8672_PNPTBL_SIZE	8
#define	RTL8672_L2TBL_SIZE	128
#define	RTL8672_L4TBL_SIZE	256


typedef struct rtl8672_tblAsic_intfTable_s {
#ifndef	_LITTLE_ENDIAN
	/* word [0] */
	uint32 RSVD7:3;
	uint32 ATMPORT:1; /* only for SAR interface */
	uint32 RSVD8:3;	
	uint32 TRLREN:1;	/* only for SAR interface */
	uint32 RSVD9:3;
	uint32 CLP:1;	/* only for SAR interface */
	uint32 RSVD10:2;	
	uint32 PTI:2;	/* only for SAR interface */
	uint32 TRLR:16; /* only for SAR interface */

	/* word [1] */
	uint16 GMAC47_32;	/* Gateway MAC Address */
	uint16 GMAC31_16;

	/* word [2] */
	uint16 GMAC15_0;
	uint16 L2Encap:1;			/* only for SAR interface: 00:VC Multiplexing, 01:L2 Encapsulation  */
	uint16 LanFCS:1;			/* only for SAR interface */ /* 0:PID=00-07, 1:PID=00-01 */
	uint16 CompPPP:1;		/* only for SAR interface */ /* 0:the Protocol Field in PPP have 2 bytes*/
	uint16 IfType:2;
	uint16 SARhdr:3;			/* only for SAR interface:000:Routed, 001:Bridged, 010:MER, 011:PPPoE, 100:PPPoA */
	uint16 RXshift:8;

	/* word [3] */
	uint32 GIP;		/* Gateway IP Address */
	
	/* word [4] */
	uint32 RSVD1:2;
	uint32 AcceptTagged:1;	/* Accept VLAN Tagged: VID = 1~4095 */
	uint32 AcceptUntagged:1;	/* Accept VLAN Untagged or 802.1p Tagged */
	uint32 RSVD2:1;
	uint32 PPID:3;			/* Default Port Priority */
	uint32 RSVD3:9;
	uint32 SrcPortFilter:1;
	uint32 L2BRIDGE:1;
	uint32 Dot1QREMR:1;
	uint32 PVID:12;	

	/* word [5] */
	uint32 LogicalID:4;
	uint32 MTU:12;		/* Maxinum Transfer Unit */
	uint32 RSVD6:4;  
	uint32 VIDR:12;
#else	/* _LITTLE_ENDIAN */
	/* word [0] */
	uint32 TRLR:16; /* only for SAR interface */
	uint32 PTI:2;	/* only for SAR interface */
	uint32 RSVD10:2;
	uint32 CLP:1;	/* only for SAR interface */
	uint32 RSVD9:3;
	uint32 TRLREN:1;	/* only for SAR interface */
	uint32 RSVD8:3;	
	uint32 ATMPORT:1; /* only for SAR interface */
	uint32 RSVD7:3;

	/* word [1] */
	uint16 GMAC31_16;
	uint16 GMAC47_32;	/* Gateway MAC Address */

	/* word [2] */
	uint16 RXshift:8;
	uint16 SARhdr:3;			/* only for SAR interface: :Routed, 001:Bridged, 010:MER, 011:PPPoE, 100:PPPoA */
	uint16 IfType:2;
	uint16 CompPPP:1;		/* only for SAR interface */ /* 0:the Protocol Field in PPP have 2 bytes*/
	uint16 LanFCS:1;			/* only for SAR interface */ /* 0:PID=00-07, 1:PID=00-01 */
	uint16 L2Encap:1;			/* only for SAR interface */
	uint16 GMAC15_0;

	/* word [3] */
	uint32 GIP;		/* Gateway IP Address */

	/* word [4] */
	uint32 PVID:12;	
	uint32 Dot1QREMR:1;
	uint32 L2BRIDGE:1;
	uint32 SrcPortFilter:1;
	uint32 RSVD3:9;
	uint32 PPID:3;			/* Default Port Priority */
	uint32 RSVD2:1;
	uint32 AcceptUntagged:1;	/* Accept VLAN Untagged or 802.1p Tagged */
	uint32 AcceptTagged:1;	/* Accept VLAN Tagged: VID = 1~4095 */
	uint32 RSVD1:2;

	/* word [5] */
	uint32 VIDR:12;
	uint32 RSVD6:4;
	uint32 MTU:12;		/* Maxinum Transfer Unit */
	uint32 LogicalID:4;	

#endif
} rtl8672_tblAsic_intfTable_t;


typedef struct rtl8672_tblAsic_pnpTable_s {
#ifndef	_LITTLE_ENDIAN
	/* word [0] */
	uint32 ETH:16;
	uint32 SPM:16;
	/* word [1] */
	uint32 RSVD1:16;
	uint32 PDS:1;
	uint32 PID:3;
	uint32 VID:12;
#else	/* _LITTLE_ENDIAN */
	/* word [0] */
	uint32 SPM:16;
	uint32 ETH:16;
	/* word [1] */
	uint32 VID:12;
	uint32 PID:3;
	uint32 PDS:1;
	uint32 RSVD1:16;
#endif
} rtl8672_tblAsic_pnpTable_t;

typedef struct rtl8672_tblAsic_l2Table_s {
#ifndef	_LITTLE_ENDIAN
	/* word [0] */
	uint16 MAC47_32;
	uint16 MAC31_16;
	/* word [1] */
	uint16 MAC15_0;
	uint32 VID:12;
	uint32 PORT:4;
	/* word [2] */
	uint32 RSVD1:7;
	uint32 AGE:1;
	uint32 PIDR:3;
	uint32 OTAGIF:1;
	uint32 RSVD2:1;
	uint32 SADROP:1;
	uint32 Dot1PREMR:1;
	uint32 VALID:1;
	uint16 APMAC47_32;
	/* word [3] */
	uint16 APMAC31_16;
	uint16 APMAC15_0;
#else	/* _BIG_ENDIAN */
	/* word [0] */
	uint16 MAC31_16;
	uint16 MAC47_32;
	/* word [1] */
	uint32 PORT:4;
	uint32 VID:12;
	uint16 MAC15_0;
	/* word [2] */
	uint16 APMAC47_32;
	uint32 VALID:1;
	uint32 Dot1PREMR:1;
	uint32 SADROP:1;
	uint32 RSVD2:1;
	uint32 OTAGIF:1;
	uint32 PIDR:3;
	uint32 AGE:1;
	uint32 RSVD1:7;
	/* word [3] */
	uint16 APMAC15_0;
	uint16 APMAC31_16;
#endif
} rtl8672_tblAsic_l2Table_t;

typedef struct rtl8672_tblAsic_l4Table_s {
#ifndef	_LITTLE_ENDIAN
	/* word [0] */
	uint32 PID:3;
	uint32 VALID:1;
	uint32 DstPortIdx:4;
	uint32 TOS:8;
	uint32 RSVD1:7;
	uint32 AGE:1;
	uint32 PROTO:8;
	/* word [1] */
	uint32 SIP;
	/* word [2] */
	uint32 DIP;
	/* word [3] */
	uint32 SPORT:16;
	uint32 DPORT:16;
	/* word [4] */
	uint32 NSIP;
	/* word [5] */
	uint32 NDIP;
	/* word [6] */
	uint32 NSPORT:16;
	uint32 NDPORT:16;
	/* word [7] */
	uint16 NHMAC47_32;
	uint16 NHMAC31_16;
	/* word [8] */
	uint16 NHMAC15_0;
	uint32 SID:16;
	/* word [9] */
	uint32 TTLDE:1;
	uint32 PRECEDREMR:1;
	uint32 TOSREMR:1;
	uint32 Dot1PREMR:1;
	uint32 OTAGIF:1;
	uint32 IPPPOEIF:1;
	uint32 OPPPOEIF:1;
	uint32 IVIDCHK:1;
	uint32 IVID:12;
	uint32 OVID:12;	
#else	/* _LITTLE_ENDIAN */
	/* word [0] */
	uint32 PROTO:8;
	uint32 AGE:1;
	uint32 RSVD1:7;
	uint32 TOS:8;
	uint32 DstPortIdx:4;
	uint32 VALID:1;
	uint32 PID:3;
	/* word [1] */
	uint32 SIP;
	/* word [2] */
	uint32 DIP;
	/* word [3] */
	uint32 DPORT:16;
	uint32 SPORT:16;
	/* word [4] */
	uint32 NSIP;
	/* word [5] */
	uint32 NDIP;
	/* word [6] */
	uint32 NDPORT:16;
	uint32 NSPORT:16;
	/* word [7] */
	uint16 NHMAC31_16;
	uint16 NHMAC47_32;
	/* word [8] */
	uint32 SID:16;
	uint16 NHMAC15_0;
	/* word [9] */
	uint32 OVID:12;	
	uint32 IVID:12;
	uint32 IVIDCHK:1;
	uint32 OPPPOEIF:1;
	uint32 IPPPOEIF:1;
	uint32 OTAGIF:1;
	uint32 Dot1PREMR:1;
	uint32 TOSREMR:1;
	uint32 PRECEDREMR:1;
	uint32 TTLDE:1;
#endif
} rtl8672_tblAsic_l4Table_t;

typedef struct rtl8672_tblAsic_hsbTable_s {
#ifndef	_LITTLE_ENDIAN
	/* word [0] */
	uint32 FROMCPU:1;
	uint32 HASMAC:1;
	uint32 TIF:1;
	uint32 PIF:1;
	uint32 SRCPORTIDX:4;
	uint32 L3HWFWDIP:1;
	uint32 LINKID:7;
	uint32 PID:3;
	uint32 CFI:1;
	uint32 VID:12;
	/* word [1] */
	uint32 RSVD_2:15;
	uint32 PPPCOMPABLE:1;
	uint32 FRAMECTRL:16;
	/* word [2] */
	uint32 ETHTP:16;
	uint16 DMAC47_32;
	/* word [3] */
	uint16 DMAC31_16;
	uint16 DMAC15_0;
	/* word [4] */
	uint16 SMAC47_32;
	uint16 SMAC31_16;
	/* word [5] */
	uint16 SMAC15_0;
	uint16 MAC3_47_32;
	/* word [6] */
	uint16 MAC3_31_16;
	uint16 MAC3_15_0;
	/* word [7] */
	uint32 SID:16;
	uint16 MAC4_47_32;
	/* word [8] */
	uint16 MAC4_31_16;
	uint16 MAC4_15_0;
	/* word [9] */
	uint32 L3CS:16;	
	uint32 L3LEN:16;
	/* word [10] */
	uint32 SIP;
	/* word [11] */
	uint32 DIP;
	/* word [12] */
	uint32 TOS:8;
	uint32 L3OFFSET:8;
	uint32 TTL:8;
	uint32 L3TYPE:1;
	uint32 IPFRAG:1;
	uint32 L3CSOK:1;
	uint32 UDPNOCS:1;
	uint32 L4CSOK:1;
	uint32 REASON:3;
	/* word [13] */
	uint32 L4PROTO:8;
	uint32 TCPFLAG:8;
	uint32 L4CS:16;
	/* word [14] */
	uint32 SPORT:16;
	uint32 DPORT:16;
#else	/* _LITTLE_ENDIAN */
	/* word [0] */
	uint32 VID:12;
	uint32 CFI:1;
	uint32 PID:3;
	uint32 LINKID:7;
	uint32 L3HWFWDIP:1;
	uint32 SRCPORTIDX:4;
	uint32 PIF:1;
	uint32 TIF:1;
	uint32 HASMAC:1;
	uint32 FROMCPU:1;
	/* word [1] */
	uint32 FRAMECTRL:16;
	uint32 PPPCOMPABLE:1;
	uint32 RSVD_2:15;
	/* word [2] */
	uint16 DMAC47_32;
	uint32 ETHTP:16;
	/* word [3] */
	uint16 DMAC15_0;
	uint16 DMAC31_16;
	/* word [4] */
	uint16 SMAC31_16;
	uint16 SMAC47_32;
	/* word [5] */
	uint16 MAC3_47_32;
	uint16 SMAC15_0;
	/* word [6] */
	uint16 MAC3_15_0;
	uint16 MAC3_31_16;
	/* word [7] */
	uint16 MAC4_47_32;
	uint32 SID:16;
	/* word [8] */
	uint16 MAC4_15_0;
	uint16 MAC4_31_16;
	/* word [9] */
	uint32 L3LEN:16;
	uint32 L3CS:16;	
	/* word [10] */
	uint32 SIP;
	/* word [11] */
	uint32 DIP;
	/* word [12] */
	uint32 REASON:3;
	uint32 L4CSOK:1;
	uint32 UDPNOCS:1;
	uint32 L3CSOK:1;
	uint32 IPFRAG:1;
	uint32 L3TYPE:1;
	uint32 TTL:8;
	uint32 L3OFFSET:8;
	uint32 TOS:8;
	/* word [13] */
	uint32 L4PROTO:8;
	uint32 TCPFLAG:8;
	uint32 L4CS:16;
	/* word [14] */
	uint32 SPORT:16;
	uint32 DPORT:16;
#endif
} rtl8672_tblAsic_hsbTable_t;

typedef struct rtl8672_tblAsic_hsaTable_s {
#ifndef	_LITTLE_ENDIAN
	/* word [0] */
	uint32 OUTIFTYPE:2;
	uint32 OUTL2ENCAP:1;
	uint32 OUTLANFCS:1;
	uint32 PPPPROTOLEN:1;
	uint32 OUTSARHDR:3;
	uint32 DROPPACKET:1;
	uint32 PIF:1;
	uint32 L3CHANGE:1;
	uint32 L4CHANGE:1;
	uint32 FROMCPU:1;
	uint32 TOCPU:1;
	uint32 INIFTYPE:2;
	uint32 TIF:1;
	uint32 LINKID:7;
	uint32 DSTPORTIDX:4;
	uint32 SRCPORTIDX:4;
	/* word [1] */
	uint32 FRAMECTRL:16;
	uint32 ETHTP:16;
	/* word [2] */
	uint16 DMAC47_32;
	uint16 DMAC31_16;
	/* word [3] */
	uint16 DMAC15_0;
	uint16 SMAC47_32;
	/* word [4] */
	uint16 SMAC31_16;
	uint16 SMAC15_0;
	/* word [5] */
	uint16 MAC3_47_32;
	uint16 MAC3_31_16;
	/* word [6] */
	uint16 MAC3_15_0;
	uint16 SID;
	/* word [7] */
	uint16 MAC4_47_32;
	uint16 MAC4_31_16;
	/* word [8] */
	uint16 MAC4_15_0;
	uint32 PID:3;
	uint32 CFI:1;
	uint32 VID:12;
	/* word [9] */
	uint32 VIDREMARK:1;
	uint32 RSVD_1:5;
	uint32 L3TYPE:1;
	uint32 TOSREMR:1;
	uint32 TOS:8;
	uint32 TTL:8;
	uint32 INL3OFFSET:8;
	/* word [10] */
	uint32 INL3LEN:16;
	uint32 L3CS:16;
	/* word [11] */
	uint32 SIP;
	/* word [12] */
	uint32 DIP;
	/* word [13] */
	uint32 SPORT:16;
	uint32 DPORT:16;
	/* word [14] */
	uint32 L4CS:16;
	uint32 L4PROTO:8;
	uint32 RSVD_2:3;
	uint32 REASON:5;
#else	/* _LITTLE_ENDIAN */
	/* word [0] */
	uint32 SRCPORTIDX:4;
	uint32 DSTPORTIDX:4;
	uint32 LINKID:7;
	uint32 TIF:1;
	uint32 INIFTYPE:2;
	uint32 TOCPU:1;
	uint32 FROMCPU:1;
	uint32 L4CHANGE:1;
	uint32 L3CHANGE:1;
	uint32 PIF:1;
	uint32 DROPPACKET:1;
	uint32 OUTSARHDR:3;
	uint32 PPPPROTOLEN:1;
	uint32 OUTLANFCS:1;
	uint32 OUTL2ENCAP:1;
	uint32 OUTIFTYPE:2;
	/* word [1] */
	uint32 ETHTP:16;
	uint32 FRAMECTRL:16;
	/* word [2] */
	uint16 DMAC31_16;
	uint16 DMAC47_32;
	/* word [3] */
	uint16 SMAC47_32;
	uint16 DMAC15_0;
	/* word [4] */
	uint16 SMAC15_0;
	uint16 SMAC31_16;
	/* word [5] */
	uint16 MAC3_31_16;
	uint16 MAC3_47_32;
	/* word [6] */
	uint16 SID;
	uint16 MAC3_15_0;
	/* word [7] */
	uint16 MAC4_31_16;
	uint16 MAC4_47_32;
	/* word [8] */
	uint32 VID:12;
	uint32 CFI:1;
	uint32 PID:3;
	uint16 MAC4_15_0;
	/* word [9] */
	uint32 INL3OFFSET:8;
	uint32 TTL:8;
	uint32 TOS:8;
	uint32 TOSREMR:1;
	uint32 L3TYPE:1;
	uint32 RSVD_1:5;
	uint32 VIDREMARK:1;
	/* word [10] */
	uint32 L3CS:16;
	uint32 INL3LEN:16;
	/* word [11] */
	uint32 SIP;
	/* word [12] */
	uint32 DIP;
	/* word [13] */
	uint32 DPORT:16;
	uint32 SPORT:16;
	/* word [14] */
	uint32 REASON:5;
	uint32 RSVD_2:3;
	uint32 L4PROTO:8;
	uint32 L4CS:16;
#endif
} rtl8672_tblAsic_hsaTable_t;


/* ASIC Driver - API Function Parameter Struct */
typedef struct rtl8672_tblAsicDrv_intfParam_s {
	uint32 Reserved7:3;
	uint32 ATMPORT:1; 		/* only for SAR interface */
	uint32 Reserved8:3;	
	uint32 TRLREN:1;		/* only for SAR interface */
	uint32 Reserved9:3;
	uint32 CLP:1;			/* only for SAR interface */
	uint32 Reserved10:2;	
	uint32 PTI:2;			/* only for SAR interface */	
	uint32 TRLR:16; 		/* only for SAR interface */
	
	uint8 GMAC[6];			/* Gateway MAC Address */
	uint16 L2Encap:1;		/* only for SAR interface */
	uint16 LanFCS:1;		/* only for SAR interface */ /* 0:PID=00-07, 1:PID=00-01 */
	uint16 CompPPP:1;		/* only for SAR interface */ /* 0:the Protocol Field in PPP have 2 bytes*/
	uint16 IfType:2;		/* IF_ETHERNET, IF_SAR, IF_WIRELESS */
	uint16 SARhdr:3;		/* only for SAR interface: :Routed, 001:Bridged, 010:MER, 011:PPPoE, 100:PPPoA */
	uint16 RXshift:8;

	uint32 GIP;			/* Gateway IP Address */

	uint32 Reserved1:2;
	uint32 AcceptTagged:1;		/* Accept VLAN Tagged: VID = 1~4095 */
	uint32 AcceptUntagged:1;	/* Accept VLAN Untagged or 802.1p Tagged */
	uint32 Reserved2:1;
	uint32 PortPriorityID:3;	/* Default Port Priority */
	uint32 Reserved3:9;
	uint32 SrcPortFilter:1;
	uint32 L2BridgeEnable:1;
	uint32 Dot1QRemr:1;
	uint32 PortVlanID:12;
	
  	uint32 LogicalID:4;
	uint32 MTU:12;			/* Maxinum Transfer Unit */
	uint32 Reserved6:4;  
	uint32 VlanIDRemr:12;
} rtl8672_tblAsicDrv_intfParam_t;


typedef struct rtl8672_tblAsicDrv_pnpParam_s {
	uint32 EtherType:16;
	uint32 SrcPortMember:16;
	uint32 Reserved1:16;
	uint32 PriorityDisable:1;
	uint32 PriorityID:3;
	uint32 VlanID:12;
} rtl8672_tblAsicDrv_pnpParam_t;

typedef struct rtl8672_tblAsicDrv_l2Param_s {
	uint8 MAC[6];
	uint32 VlanID:12;
	uint32 Port:4;
	uint32 Reserved1:7;
	uint32 Age:1;
	uint32 PriorityIDRemr:3;
	uint32 OTagIf:1;
	uint32 Reserved2:1;
	uint32 SADrop:1;
	uint32 Dot1PRemr:1;
	uint32 Valid:1;
	uint8 APMAC[6];
} rtl8672_tblAsicDrv_l2Param_t;

typedef struct rtl8672_tblAsicDrv_l4Param_s {
	uint32 PriorityID:3;
	uint32 Valid:1;
	uint32 DstPortIdx:4;
	uint32 Tos:8;
	uint32 Reserved1:7;
	uint32 Age:1;
	uint32 Proto:8;
	uint32 SrcIP;
	uint32 DstIP;
	uint32 SrcPort:16;
	uint32 DstPort:16;
	uint32 NewSrcIP;
	uint32 NewDstIP;
	uint32 NewSrcPort:16;
	uint32 NewDstPort:16;
	uint8 NHMAC[6];
	uint32 SessionID:16;
	uint32 TtlDe:1;
	uint32 PrecedRemr:1;
	uint32 TosRemr:1;
	uint32 Dot1PRemr:1;
	uint32 OTagIf:1;
	uint32 IPppoeIf:1;
	uint32 OPppoeIf:1;
	uint32 IVlanIDChk:1;
	uint32 IVlanID:12;
	uint32 OVlanID:12;
} rtl8672_tblAsicDrv_l4Param_t;

/* Header Stamp */
typedef struct rtl8672_tblAsicDrv_hsbParam_s
{
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
		uint32 rsvd_2:15;
		uint32 pppcompable:1;
		uint32 framectrl:16;
		uint32 ethtp:16;
		uint8 dmac[6];
		uint8 smac[6];
		uint8 mac3[6];
		uint32 sid:16;
		uint8 mac4[6];
		uint32 l3cs:16; 
		uint32 l3len:16;
		uint32 sip;
		uint32 dip;
		uint32 tos:8;
		uint32 l3offset:8;
		uint32 ttl:8;
		uint32 l3type:1;
		uint32 ipfrag:1;
		uint32 l3csok:1;
		uint32 udpnocs:1;
		uint32 l4csok:1;
		uint32 reason:3;
		uint32 l4proto:8;
		uint32 tcpflag:8;
		uint32 l4cs:16;
		uint32 sport:16;
		uint32 dport:16;
} rtl8672_tblAsicDrv_hsbParam_t;

typedef struct rtl8672_tblAsicDrv_hsaParam_s
{
		uint32 outiftype:2;
		uint32 outl2encap:1;
		uint32 outlanfcs:1;
		uint32 pppprotolen:1;
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
		uint32 framectrl:16;
		uint32 ethtp:16;
		uint8 dmac[6];
		uint8 smac[6];
		uint8 mac3[6];
		uint16 sid;
		uint8 mac4[6];
		uint32 pid:3;
		uint32 cfi:1;
		uint32 vid:12;
		uint32 vidremark:1;
		uint32 rsvd_1:5;
		uint32 l3type:1;
		uint32 tosremr:1;
		uint32 tos:8;
		uint32 ttl:8;
		uint32 inl3offset:8;
		uint32 inl3len:16;
		uint32 l3cs:16;
		uint32 sip;
		uint32 dip;
		uint32 sport:16;
		uint32 dport:16;
		uint32 l4cs:16;
		uint32 l4proto:8;
		uint32 rsvd_2:3;
		uint32 reason:5;
} rtl8672_tblAsicDrv_hsaParam_t;

typedef struct rtl8672_tblAsic_hsbpTable_s {
#ifdef	_LITTLE_ENDIAN
	/* word [0] */
	uint32 pppproto:16;
	uint32 rxshift:8;
	uint32 rsvd_0:3;
	uint32 compppp:1;
	uint32 rsvd_1:1;
	uint32 tif:1;
	uint32 pif:1;
	uint32 RxHdrInSram:1;
	/* word [1] */
	uint32 sramaddr;
	/* word [2] */
	uint32 dramaddr;
#else	/* _BIG_ENDIAN */
	/* word [0] */
	uint32 RxHdrInSram:1;
	uint32 pif:1;
	uint32 tif:1;
	uint32 rsvd_1:1;
	uint32 compppp:1;
	uint32 rsvd_0:3;
	uint32 rxshift:8;
	uint32 pppproto:16;
	/* word [1] */
	uint32 sramaddr;
	/* word [2] */
	uint32 dramaddr;
#endif
} rtl8672_tblAsic_hsbpTable_t;


/* ASIC Build-In SRam Functions */
int32 rtl8672_setAsicL2HashIdxBits(uint32 l2hib);
int32 rtl8672_getAsicL2HashIdxBits(uint32 *l2bib);

int32 rtl8672_setAsicL2Way(uint32 l2way);
int32 rtl8672_getAsicL2Way(uint32 *l2way);

int32 rtl8672_setAsicL2BaseAddress(uint32 l2ba);
int32 rtl8672_getAsicL2BaseAddress(uint32 *l2ba);

int32 rtl8672_setAsicL2Ability(uint32 isEnable);
int32 rtl8672_getAsicL2Ability(uint32 *isEnable);

int32 rtl8672_setAsicL4HashIdxBits(uint32 l4hib);
int32 rtl8672_getAsicL4HashIdxBits(uint32 *l4hib);

int32 rtl8672_setAsicL4Way(uint32 l4way);
int32 rtl8672_getAsicL4Way(uint32 *l4way);

int32 rtl8672_setAsicL4BaseAddress(uint32 l4ba);
int32 rtl8672_getAsicL4BaseAddress(uint32 *l4ba);

int32 rtl8672_setAsicL4Ability(uint32 isEnable);
int32 rtl8672_getAsicL4Ability(uint32 *isEnable);

uint32 rtl8672_L2Hash(uint8 *mac, uint32 vid);
uint32 rtl8672_L4Hash(uint8 proto, uint32 sip, uint32 dip, uint16 sport, uint16 dport);
uint32 rtl8672_L2Hash_Sram(uint8 *mac, uint32 vid);
uint32 rtl8672_L4Hash_Sram(uint8 proto, uint32 sip, uint32 dip, uint16 sport, uint16 dport);

int32 rtl8672_setAsicNetInterface(uint32 idx, rtl8672_tblAsicDrv_intfParam_t *intfp);
int32 rtl8672_getAsicNetInterface(uint32 idx, rtl8672_tblAsicDrv_intfParam_t *intfp);

int32 rtl8672_setAsicProtocolBasedVLAN(uint32 idx, rtl8672_tblAsicDrv_pnpParam_t *l2p);
int32 rtl8672_getAsicProtocolBasedVLAN(uint32 idx, rtl8672_tblAsicDrv_pnpParam_t *l2p);

int32 rtl8672_setAsicL2Table(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l2Param_t *l2p);
int32 rtl8672_getAsicL2Table(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l2Param_t *l2p);

int32 rtl8672_setAsicL2Table_Sram(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l2Param_t *l2p);
int32 rtl8672_getAsicL2Table_Sram(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l2Param_t *l2p);

int32 rtl8672_setAsicL4Table(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l4Param_t *l4p);
int32 rtl8672_getAsicL4Table(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l4Param_t *l4p);

int32 rtl8672_setAsicL4Table_Sram(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l4Param_t *l4p);
int32 rtl8672_getAsicL4Table_Sram(uint32 idx, uint32 way, rtl8672_tblAsicDrv_l4Param_t *l4p);

int32 rtl8672_setAsicHSB(rtl8672_tblAsicDrv_hsbParam_t *hsbp);
int32 rtl8672_getAsicHSB(rtl8672_tblAsicDrv_hsbParam_t *hsbp);
int32 rtl8672_setAsicHSA(rtl8672_tblAsicDrv_hsaParam_t *hsap);
int32 rtl8672_getAsicHSA(rtl8672_tblAsicDrv_hsaParam_t *hsap);

int32 rtl8672_setAsicHSBP(rtl8672_tblAsic_hsbpTable_t *hsbp);
int32 rtl8672_getAsicHSBP(rtl8672_tblAsic_hsbpTable_t *hsbp);


#endif	/* __RTL8672_TBLASICDRV_H__ */

