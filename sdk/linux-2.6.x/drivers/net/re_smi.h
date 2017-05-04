/*	
 *	re_smi.h
*/
#ifndef _RE_SMI_H_
#define _RE_SMI_H_

void internal_select_page(unsigned char page);
void internal_miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value);
void miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value);
void miiar_read(unsigned char phyaddr,unsigned char regaddr,unsigned short *value);
void miiar_WriteBit(unsigned char phyad, unsigned char regad, unsigned char bit, unsigned char value);
void internal_miiar_WriteBit(unsigned char phyad, unsigned char regad, unsigned char bit, unsigned char value);
void internal_miiar_read(unsigned char phyaddr,unsigned char regaddr,unsigned short *value);


#endif /*_RE_SMI_H_*/
