#ifndef _CRC32_H
#define _CRC32_H

/*
 * This code implements the AUTODIN II polynomial used by Ethernet,
 * and can be used to calculate multicast address hash indices.
 * It assumes that the low order bits will be transmitted first,
 * and consequently the low byte should be sent first when
 * the crc computation is finished.  The crc should be complemented
 * before transmission.
 * The variable corresponding to the macro argument "crc" should
 * be an unsigned long and should be preset to all ones for Ethernet
 * use.  An error-free packet will leave 0xDEBB20E3 in the crc.
 *			Spencer Garrett <srg@quick.com>
 */

#include <net/rtl/rtl_types.h>


void	pktForm_crc32(char *buf, unsigned int len, unsigned int *cval );
//the result must put higher bytes at after lower bytes

#endif

