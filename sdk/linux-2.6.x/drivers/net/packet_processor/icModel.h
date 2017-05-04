/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for RTL8672 IC-specific Function
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icModel.h,v 1.1.1.1 2010/05/05 09:00:49 jiunming Exp $
*/

#ifndef __IC_MODEL_H__
#define __IC_MODEL_H__

#include "hsModel.h"

/* Port number for 'spa' */
enum PORT_NUM
{
	PN_PORT0 = 0,
	PN_PORT1 = 1,
	PN_PORT_NOTPHY = 2,
};

enum EXTPORT_NUM
{
	PN_PORT_EXT0 = 0,
	PN_PORT_EXT1 = 1,
	PN_PORT_EXT2 = 2,
	PN_PORT_CPU = 3,
};


/* Port number for 'dp' & 'dpext'*/
enum PORT_MASK
{
	PM_PORT_0 = (1<<PN_PORT0),
	PM_PORT_1 = (1<<PN_PORT1),
	PM_PORT_NOTPHY = (1<<PN_PORT_NOTPHY),
	PM_PORT_ALL = (1<<PN_PORT0)|(1<<PN_PORT1)|(1<<PN_PORT_NOTPHY),
};

enum EXTPORT_MASK
{
	PM_PORT_EXT0 = (1<<PN_PORT_EXT0), /* PN_PORT_EXT0 is 0. For uniform port mask, use 'PM_PORT_EXT0<<RTL8651_PORT_NUMBER' */
	PM_PORT_EXT1 = (1<<PN_PORT_EXT1), /* PN_PORT_EXT1 is 1. For uniform port mask, use 'PM_PORT_EXT1<<RTL8651_PORT_NUMBER' */
	PM_PORT_EXT2 = (1<<PN_PORT_EXT2), /* PN_PORT_EXT2 is 2. For uniform port mask, use 'PM_PORT_EXT2<<RTL8651_PORT_NUMBER' */
	PM_PORT_CPU = (1<<PN_PORT_CPU),
	PM_EXTPORT = (1<<PN_PORT_CPU)|(1<<PN_PORT_EXT0)|(1<<PN_PORT_EXT1)|(1<<PN_PORT_EXT2),
};


/*
    Functions
*/
void modelIcInit( void );
void modelIcExit( void );
int32 modelIcReinit( void );
void modelIcSetDefaultValue( void );
void modelStart( void );



#endif	/* __IC_MODEL_H__ */
