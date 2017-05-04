#ifndef _NFBI_GPIOSMI_H_
#define _NFBI_GPIOSMI_H_

#include <linux/autoconf.h>

#ifdef CONFIG_RTL8676
#define  NFBI_GPIO_SIMULATE

void NFBI_smiRead(unsigned char phyad, unsigned char regad, unsigned short *data);
void NFBI_smiWrite(unsigned char phyad, unsigned char regad, unsigned short data);
#endif /*CONFIG_RTL8676*/

#endif /*_NFBI_GPIOSMI_H_*/
