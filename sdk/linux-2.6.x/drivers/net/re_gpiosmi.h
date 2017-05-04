/*	
 *	re_gpiosmi.h
*/
#ifndef _RE_GPIOSMI_H_
#define _RE_GPIOSMI_H_

#ifdef CONFIG_EXT_SWITCH
// IAD board doesn't uses GPIO as mdio/mdc
#ifndef CONFIG_RTK_VOIP
#define  GPIO_SIMULATE
#endif
#if defined(CONFIG_6166_IAD_SILAB3217X) || defined(CONFIG_6166_IAD_ZARLINK)
#define  GPIO_SIMULATE
#endif
#endif

void rtl8305s_smiRead(unsigned char phyad, unsigned char regad, unsigned short * data);
void rtl8305s_smiWrite(unsigned char phyad, unsigned char regad, unsigned short data);

#endif /*_RE_GPIOSMI_H_*/
