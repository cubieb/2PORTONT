/*
 * ----------------------------------------------------------------
 * Copyright c                  Realtek Semiconductor Corporation, 2002  
 * All rights reserved.
 * 
 *
 * Abstract: Switch core driver source code.
 *
 * $Author: yachang $
 *
 * ---------------------------------------------------------------
 */
#include <common.h>
#include "swCore.h"

#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *) (addr)) = (val)
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *) (addr))

#define MACReg(offset, val)		(WRITE_MEM32(SWITCH_BASE + offset, val))

int swCore_init(void)
{
	//printf("%s %d\n", __func__, __LINE__);
        printf("\n\r swCore_init 6266\n\r");
        //patch phy done
        MACReg(0x88, 1);
        //SVLAN uplink port
        MACReg(0x23110, 0);
        //port isolation
        MACReg(0x27000, 0x3e0ff);
        //cpu port force mode
        MACReg(0x120, 0x40);
        //cpu port ability
        MACReg(0xa8, 0x196);
        //meter set tick-token tick-period 53 token 58
        MACReg(0x25000, 0x1353a);
        //meter set pon-tick-token tick-period 53 token 58
        MACReg(0x25108, 0x1353a);
        //vlan set state disable
        MACReg(0x13108, 0x00000000);
        //vlan set tag-mode port 0-6 keep-format
        MACReg(0x20030, 0x00000001);
        MACReg(0x20430, 0x00000001);
        MACReg(0x20830, 0x00000001);
        MACReg(0x20c30, 0x00000001);
        MACReg(0x21030, 0x00000001);
        MACReg(0x21430, 0x00000001);
        MACReg(0x21830, 0x00000001);
        //cpu set trap-insert-tag state disable
        MACReg(0x23064, 0x00000000);
        //MOCIR_FRC_MD, 0711
        MACReg(0x2D8F8, 0xffffffff);
        //#MOCIR_FRC_VAL, 0711
        MACReg(0x2D8FC, 0xffffffff);

        *((volatile unsigned int *)0xb8000044) = *((volatile unsigned int *)0xb8000044) | 1;


	return 0;
}



