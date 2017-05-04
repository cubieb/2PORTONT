
#ifndef _MDCMDIO_H_
#define _MDCMDIO_H_
#define 	SMI_HIGHSPEED   0

//extern uint32 smi_MDC;		/* GPIO used for SMI Clock Generation */
//extern uint32 smi_MDIO;		/* GPIO used for SMI Data signal */

#if (!SMI_HIGHSPEED)
int32 smiInit(uint32 port, uint32 pinMDC, uint32 pinMDIO);
#endif /*!SMI_HIGHSPEED*/

int32 smiRead(uint32 phyad, uint32 regad, uint32 * data);
int32 smiWrite(uint32 phyad, uint32 regad, uint32 data);
int32 smiReadBit(uint32 phyad, uint32 regad, uint32 bit, uint32 * pdata);
int32 smiWriteBit(uint32 phyad, uint32 regad, uint32 bit, uint32 data);


#endif /*_MDCMDIO_H_*/
