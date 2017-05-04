/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* Program : utility
*
* Original Creator: Edward Jin-Ru Chen 2002/4/29
* $Author $
*
* $Revision: 1.1 $
* $Id: utility.c,v 1.1 2012/10/24 04:18:57 ikevin362 Exp $
* $Log: utility.c,v $
* Revision 1.1  2012/10/24 04:18:57  ikevin362
* initial version
*
* Revision 1.4  2006/01/10 05:53:05  rupert
* * fix inet_addr
*
* Revision 1.3  2005/07/20 15:29:38  yjlou
* +: porting Model Code to Linux Kernel: check RTL865X_MODEL_KERNEL.
*
* Revision 1.2  2004/03/03 10:40:38  yjlou
* *: commit for mergence the difference in rtl86xx_tbl/ since 2004/02/26.
*
* Revision 1.1  2004/02/25 14:27:24  chhuang
* *** empty log message ***
*
* Revision 1.4  2004/02/18 13:12:21  chenyl
* *** empty log message ***
*
* Revision 1.2  2004/02/17 13:01:47  chenyl
* *** empty log message ***
*
* Revision 1.1  2004/02/13 05:18:47  chenyl
* + Pkt generating functions
*
* Revision 1.7  2002/09/27 15:45:49  waynelee
* -> hex2bin
*
* Revision 1.6  2002/09/21 09:54:24  waynelee
* add pkt_aton()
*
* Revision 1.5  2002/09/16 07:31:15  waynelee
* support IPX (experimental)
*
*/

#include "utility.h"
#include <net/rtl/rtl_types.h>


#if defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER) || defined(RTL865X_MODEL_KERNEL)
/* memDump() has defined in rtl_utils.c. */
#else
void memDump(void *start, uint32 size, int8 * strHeader)
{

	int32     row, column, index, index2, max;
	uint8    *buf, ascii[17];
	char      empty = ' ';

	assert(start && (size > 0));
	buf = (uint8 *) start;

	/*
	   16 bytes per line
	 */
	if (strHeader)
		printf("%s", strHeader);
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, buf += 16)
	{
		memset(ascii, 0, 17);
		printf("\n%08x ", (memaddr) buf);

		max = (index == row - 1) ? column : 16;

		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
				printf("  ");
			printf("%02x ", (uint8) buf[index2]);
			ascii[index2] = ((uint8) buf[index2] < 32) ? empty : buf[index2];
		}

		if (max != 16)
		{
			if (max < 8)
				printf("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				printf("   ");
		}

		//ASCII
		printf("  %s", ascii);
	}
	printf("\nThe total length of this packet is: %i bytes.\n",size);
	return;
}
#endif /*0*/

uint16 ipcsum(uint16 *ptr, uint32 len, uint16 resid) {
	uint32 csum = resid;
	uint32 odd = 0;
//	rtlglue_printf("***** the ptr is [%d]*****\n", *ptr);
//	rtlglue_printf("****  the len is [%d]****\n", len);
  //      rtlglue_printf("***** the resid is [%d]****\n", resid);
	if(len & 1) odd = 1;
	len = len >> 1;
//	rtlglue_printf("**** the len is [%d]***\n", len);	
	for(;len > 0 ; len--,ptr++){ 
		csum += ntohs(*ptr);
//		rtlglue_printf("the *ptr is [%d]\n", *ptr);
//		rtlglue_printf("***** the csum is [%d]****\n", csum);
	}

	if(odd)
		csum += (*((uint8 *)ptr) <<8) & 0xff00;

	/* take care of 1's complement */
	while(csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);

	if(csum == 0xffff)	csum = 0;

	return((uint16)csum);
}

uint16 ipadjcsum(uint16 old, uint32 diff) {
	uint32 csum = old + diff;

	csum = (csum & 0xffff) + (csum >> 16);
	if(!csum)	csum = 0xffff;
	return((uint16)csum);
}

#if 0
/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 * returns in network order
 */
int32 inet_aton(const int8 *cp, struct in_addr *addr){
	uint32 val;
	int32 base, n;
	int8 c;
	uint32 parts[4];
	uint32 *pp = parts;

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;

		//determine the base
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}

		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}

		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if ((val > 0xffffff) || (parts[0] > 0xff))
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}
#endif /*0*/

#ifdef __KERNEL__
typedef uint32 in_addr_t;
 int inet_aton(const char *cp, struct in_addr *addrptr)
{
	in_addr_t addr;
	int value;
	int part;

	addr = 0;
	for (part = 1; part <= 4; part++) {

		if (!isdigit(*cp))
			return 0;

		value = 0;
		while (isdigit(*cp)) {
			value *= 10;
			value += *cp++ - '0';
			if (value > 255)
				return 0;
		}

		if (part < 4) {
			if (*cp++ != '.')
				return 0;
		} else {
			char c = *cp++;
			if (c != '\0' && !isspace(c))
			return 0;
		}

		addr <<= 8;
		addr |= value;
	}

	/*  W. Richard Stevens in his book UNIX Network Programming,
	 *  Volume 1, second edition, on page 71 says:
	 *
	 *  An undocumented feature of inet_aton is that if addrptr is
	 *  a null pointer, the function still performs it validation
	 *  of the input string, but does not store the result.
	 */
	if (addrptr) {
	    addrptr->s_addr = htonl(addr);
	}

	return 1;
}
#endif

/*deprecated.*/
uint32 inet_addr(const int8 *cp){

	struct in_addr a;

	if (!inet_aton(cp, &a))
		return -1;
	else
		return a.s_addr;
}
static int8 *ui8tod( uint8 n, int8 *p )
{
	if( n > 99 ) *p++ = (n/100) + '0';
	if( n >  9 ) *p++ = ((n/10)%10) + '0';
	*p++ = (n%10) + '0';
	return p;
}

int8 *inet_ntoa(struct in_addr ina)
{
	static int8 buf[4*sizeof "123"];
	int8 *p = buf;
	uint8 *ucp = (unsigned char *)&ina;

	p = ui8tod( ucp[0] & 0xFF, p);
	*p++ = '.';
	p = ui8tod( ucp[1] & 0xFF, p);
	*p++ = '.';
	p = ui8tod( ucp[2] & 0xFF, p);
	*p++ = '.';
	p = ui8tod( ucp[3] & 0xFF, p);
	*p++ = '\0';

	return (buf);
}



/*
 * Convert an ASCII representation of an ethernet address to
 * binary form.
 * Original Author: hiwu
 */
struct ether_addr * ether_aton (const int8 *a)
{
  int32 i;
  static struct ether_addr o;
  int32 o0, o1, o2, o3, o4, o5;

  i = sscanf (a, "%02x-%02x-%02x-%02x-%02x-%02x", &o0, &o1, &o2, &o3, &o4, &o5);

  if (i != 6)
    return (NULL);

  o.octet[0] = o0;
  o.octet[1] = o1;
  o.octet[2] = o2;
  o.octet[3] = o3;
  o.octet[4] = o4;
  o.octet[5] = o5;

  return ((struct ether_addr *) &o);
}

/*
 * Convert a binary representation of an ethernet address to
 * an ASCII string.
 * Original Author: hiwu
 */
int8 * ether_ntoa (const struct ether_addr *n)
{
  int32 i;
  static int8 a[18];

  i = sprintf (a, "%02x-%02x-%02x-%02x-%02x-%02x", n->octet[0], n->octet[1], n->octet[2],
	       n->octet[3], n->octet[4], n->octet[5]);
  if (i < 11)
    return (NULL);
  return ((int8 *) & a);
}

/*
 * Convert HEX representation of a string to a binary form
 */

int8 * hex2bin (const int8 *a)
{
	static int8	buffer[2048]={0};
	uint32	i;
	int32	o;

	for(i=0;i<strlen(a);i+=2)
	{
		sscanf(a+i,"%02x",&o);
		buffer[i/2] = o;
	}

	return (buffer);
}

void set_mac (int8 *a, int8 h1, int8 h2, int8 h3, int8 h4, int8 h5, int8 h6)
{
	int8 *b = a;

	*b++ = h1;
	*b++ = h2;
	*b++ = h3;
	*b++ = h4;
	*b++ = h5;
	*b = h6;
}

void set_v6addr (int8 *a, int8 h1, int8 h2, int8 h3, int8 h4, int8 h5, int8 h6, int8 h7, int8 h8, int8 h9, int8 h10,int8 h11,int8 h12, int8 h13,int8 h14,int8 h15, int8 h16)
{
	int8 *b = a;
	*b++ = h1;
	*b++ = h2;
	*b++ = h3;
	*b++ = h4;
	*b++ = h5;
	*b++ = h6;
	*b++ = h7;
	*b++ = h8;
	*b++ = h9;
	*b++ = h10;
	*b++ = h11;
	*b++ = h12;
	*b++ = h13;
	*b++ = h14;
	*b++ = h15;
	*b = h16;
}
