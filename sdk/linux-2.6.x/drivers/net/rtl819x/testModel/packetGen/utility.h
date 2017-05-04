/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* Program : utility Header
*
* $Author $
*
* $Revision: 1.1 $
* $Id: utility.h,v 1.1 2012/10/24 04:18:57 ikevin362 Exp $
* $Log: utility.h,v $
* Revision 1.1  2012/10/24 04:18:57  ikevin362
* initial version
*
* Revision 1.3  2005/09/09 15:04:14  yjlou
* *: Fixed for Kernel Model Code. Including:
*    change printf() to rtlglue_printf()
*    unused variables
*    uninitialized variables
*    blah blah
*
* Revision 1.2  2005/06/20 13:58:47  yjlou
* *: merge PktProc
* +: add rough packet parser model code and test bench.
*
* Revision 1.1  2004/02/25 14:27:24  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/18 13:12:21  chenyl
* *** empty log message ***
*
* Revision 1.2  2004/02/17 13:01:47  chenyl
* *** empty log message ***
*
* Revision 1.1  2004/02/13 05:18:47  chenyl
* + Pkt generating functions
*
* Revision 1.4  2002/09/27 15:45:49  waynelee
* -> hex2bin
*
*/

#ifndef UTILITY_H
#define UTILITY_H

#include <net/rtl/rtl_types.h>



void memDump(void *start, uint32 size, int8 * strHeader);
uint16 ipcsum(uint16 *ptr, uint32 len, uint16 resid);
uint16 ipadjcsum(uint16 old, uint32 diff);

void set_mac (int8 *a, int8 h1, int8 h2, int8 h3, int8 h4, int8 h5, int8 h6);
void set_v6addr(int8 *a,int8 h1,int8 h2,int8 h3,int8 h4,int8 h5,int8 h6,int8 h7,int8 h8,int8 h9,int8 h10,int8 h11,int8 h12,int8 h13,int8 h14,int8 h15,int8 h16);

#ifndef RTL8651_TBLDRV_PROTO_H /* rtl8651_tbldrv_proto.h also defined this structure. */
struct in_addr
{
	uint32    s_addr;
};
#endif


#define	INADDR_NONE	0xffffffff	/* -1 return */

//Network order part
int32 inet_aton(const int8 *cp, struct in_addr *addr);
/*deprecated.*/
uint32 inet_addr(const int8 *cp);
int8 *inet_ntoa(struct in_addr ina);


#define ETHER_ADDR_LEN                          6

#ifndef RTL8651_TBLDRV_PROTO_H /* rtl8651_tbldrv_proto.h also defined this structure. */
struct  ether_addr {
        uint8 octet[ETHER_ADDR_LEN];
};
#endif

/*
 * Convert an ASCII representation of an ethernet address to
 * binary form.
 */
struct ether_addr * ether_aton (const int8 *a);

/*
 * Convert a binary representation of an ethernet address to
 * an ASCII string.
 */
int8 * ether_ntoa (const struct ether_addr *n);

int8 * hex2bin (const int8 *a);

#ifdef RTL865X_MODEL_USER
/* The following information should be sync to man page. */
#ifndef htonl
uint32 htonl(uint32 hostlong);
#endif
#ifndef htons
uint16 htons(uint16 hostshort);
#endif
#ifndef ntohl
uint32 ntohl(uint32 netlong);
#endif
#ifndef ntohs
uint16 ntohs(uint16 netshort);
#endif
#endif

#endif
