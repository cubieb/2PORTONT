/*
 *      Headler file of Realtek STP
 *
 *      $Id: rtk_stp.h,v 1.1 2011/04/12 02:26:00 tylo Exp $
 */

#ifndef _RTK_STP_H
#define _RTK_STP_H
#include "rtl_types.h"

#define MAX_RTL_STP_PORT_WH	5

#define RTL8651_PORTSTA_DISABLED		0x00
#define RTL8651_PORTSTA_BLOCKING		0x01
#define RTL8651_PORTSTA_LISTENING		0x02
#define RTL8651_PORTSTA_LEARNING		0x03
#define RTL8651_PORTSTA_FORWARDING	0x04

int32 rtl865x_setSpanningEnable(int8 spanningTreeEnabled);
int32 rtl865x_setMulticastSpanningTreePortState(uint32 port, uint32 portState);
int32 rtl865x_setSpanningTreePortState(uint32 port, uint32 portState);

#endif // _RTK_STP_H
