/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : Header File of Packet Model for RTL8672
* Abstract :
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icModel_Packet.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef __ICMODEL_PACKET_H__
#define __ICMODEL_PACKET_H__


struct rtl_pkt
{
	uint8*	content;		/* point to addr of content of the packet */
	int32	length;			/* indicate the length of packet (Bytes) */
};


#endif	/* __ICMODEL_PACKET_H__ */

