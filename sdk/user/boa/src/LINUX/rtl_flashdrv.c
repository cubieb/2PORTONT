/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/user/boa/src/LINUX/rtl_flashdrv.c,v 1.3 2012/09/11 08:46:54 tsaitc Exp $
*
* Abstract: Flash driver source code.
*
* $Author: tsaitc $
*
*
* ---------------------------------------------------------------
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "rtl_types.h"
#include "rtl_board.h"
#include "rtl_flashdrv.h"

/* Manufacturers */
#define MANUFACTURER_MXIC	0x00C2

/* MXIC deivce id */
#define MX29LV800B		0x225B
#define MX29LV160AB		0x2249
#define MX29LV320AB		0x22A8

struct flash_desc {
	unsigned short mfr_id;
	unsigned short dev_id;
	uint32 size;
	uint32 block_num;
	const uint32 *blockOffset;
};

static struct flash_desc table[]= {
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV800B,
		size: 0x00100000,
		block_num: BLOCK_NUM_1M,
		blockOffset: blockOffset_1M
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV160AB,
		size: 0x00200000,
		block_num: BLOCK_NUM_2M,
		blockOffset: blockOffset_2M
	},
	{
		mfr_id: MANUFACTURER_MXIC,
		dev_id: MX29LV320AB,
		size: 0x00400000,
		block_num: BLOCK_NUM_4M,
		blockOffset: blockOffset_4M
	}
};

static struct flash_desc *map;

#if 0
uint32 flashdrv_init(flashdriver_obj_t * const drvObj_P)
{
    ASSERT_CSP( drvObj_P );
    if ( drvObj_P->blockBaseArrayCapacity < FLASH_NUM_OF_BLOCKS )
        return 1;
    
	drvObj_P->flashSize = FLASH_TOTAL_SIZE;
	drvObj_P->flashBaseAddress = FLASH_BASE;
	drvObj_P->blockNumber = FLASH_NUM_OF_BLOCKS;
	memcpy(drvObj_P->blockBaseArray_P, flashBlockOffset,
	             FLASH_NUM_OF_BLOCKS * sizeof(uint32));
    
    return 0;
}
#endif

uint32 flashdrv_init()
{
	int i;
	unsigned short mfid, devid;
	
	// issue reset and auto-selection command
	*((volatile uint16 *)(FLASH_BASE)) = 0xf0;
	
	*AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_COMMAND1;
	*AM29LVXXX_COMMAND_ADDR2 = AM29LVXXX_COMMAND2;
	*AM29LVXXX_COMMAND_ADDR1 = 0x90;
	
	mfid = (*(volatile unsigned short *)(FLASH_BASE));
	devid = (*(volatile unsigned short *)(FLASH_BASE+1*2));
	//printf("mfid = %x, devid = %x\n", mfid, devid);
	
	*((volatile uint16 *)(FLASH_BASE)) = 0xf0;
	
	for (i=0; i< sizeof(table)/sizeof(table[0]); i++) {
//		if ( mfid==table[i].mfr_id && devid==table[i].dev_id)
		if ( devid==table[i].dev_id)
			break;
	}
	
	if ( i == sizeof(table)/sizeof(table[0]) ) {
		printf("Probe flash memory failed!\n");
		return 0;
	}
	
	map = &table[i];
}

uint32 flashdrv_updateImg(void *srcAddr_P, void *dstAddr_P, uint32 size)
{
    uint32  blockNum;
    uint32  startBlkNum;
    
    ASSERT_CSP( srcAddr_P );
    ASSERT_CSP( dstAddr_P );
    ASSERT_CSP( map );
    
    //if( ((uint32) dstAddr_P + size) > (FLASH_TOTAL_SIZE + FLASH_BASE) )
    if( ((uint32) dstAddr_P + size) > (map->size + FLASH_BASE) )
        return 1;
    
    /* Search the starting block */
    //for (blockNum = 0; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
    for (blockNum = 0; blockNum < (map->block_num - 1); blockNum++)
        //if ( flashBlockOffset[blockNum + 1] > ((uint32) dstAddr_P - FLASH_BASE) )
        if ( map->blockOffset[blockNum + 1] > ((uint32) dstAddr_P - FLASH_BASE) )
            break;
    startBlkNum = blockNum;

    /* Erase blocks */
    //for (blockNum = startBlkNum; blockNum < FLASH_NUM_OF_BLOCKS; blockNum++)
    for (blockNum = startBlkNum; blockNum < map->block_num; blockNum++)
        //if ( flashBlockOffset[blockNum] < ((uint32) dstAddr_P - FLASH_BASE + size) )
        if ( map->blockOffset[blockNum] < ((uint32) dstAddr_P - FLASH_BASE + size) )
        {
            /* Erase the block */
            if ( flashdrv_eraseBlock(
                    //(void *) (flashBlockOffset[blockNum] + FLASH_BASE) ) )
                    (void *) (map->blockOffset[blockNum] + FLASH_BASE) ) )
            /* Retry */
            /*if ( flashdrv_eraseBlock(
                    (void *) (flashBlockOffset[blockNum] + FLASH_BASE) ) )*/
                return 1;
        }
        else
            break;

    /* Write blocks */
    return flashdrv_write(dstAddr_P, srcAddr_P, size);
}

uint32 flashdrv_eraseBlock(void *startAddr_P)
{
    uint32                  blockNum;
    uint32                  count = 0;
    volatile uint16         *flash_cui;
    volatile uint32         *test_cui;
    
    ASSERT_CSP( startAddr_P );
    ASSERT_CSP( map );
    
    flash_cui = (volatile uint16 *) ( (uint32)startAddr_P - FLASH_BASE );
    
    /* Check if legal starting address */
    //for (blockNum = 0; blockNum < FLASH_NUM_OF_BLOCKS; blockNum++)
    for (blockNum = 0; blockNum < map->block_num; blockNum++)
        //if (flashBlockOffset[blockNum] == (uint32) flash_cui)
        if (map->blockOffset[blockNum] == (uint32) flash_cui)
            break;
    //if (blockNum >= FLASH_NUM_OF_BLOCKS)
    if (blockNum >= map->block_num)
        return 1;

    /* Start erase sequence */
    flash_cui = (volatile uint16 *) startAddr_P;
    
    *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_COMMAND1;
    *AM29LVXXX_COMMAND_ADDR2 = AM29LVXXX_COMMAND2;
    *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_SECTOR_ERASE_CMD1;
    *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_COMMAND1;
    *AM29LVXXX_COMMAND_ADDR2 = AM29LVXXX_COMMAND2;
    *flash_cui = AM29LVXXX_SECTOR_ERASE_CMD2;
    
    /* Check if completed */
    //if ( blockNum < FLASH_NUM_OF_BLOCKS - 1 )
    if ( blockNum < map->block_num - 1 )
        //test_cui = (volatile uint32 *) (flashBlockOffset[blockNum + 1] 
        test_cui = (volatile uint32 *) (map->blockOffset[blockNum + 1] 
                            + FLASH_BASE - 4);
    else
        test_cui = (volatile uint32 *) (FLASH_BASE - 4);
    
    while ((uint32)test_cui >= (uint32)startAddr_P)
    {
        if (*test_cui == 0xFFFFFFFF)
            test_cui--;
        else
        {
            count++;
            if (count > 0x3fFFFF)
            {
              printf("! Block number %d erase \n", blockNum);
            	return 1;
            }
        }
    }
    
    return 0;
}

uint32 flashdrv_read (void *dstAddr_P, void *srcAddr_P, uint32 size)
{
    ASSERT_CSP( srcAddr_P );
    ASSERT_CSP( dstAddr_P );
    
    memcpy(dstAddr_P, srcAddr_P, size);
    
    return 0;
}

uint32 flashdrv_write(void *dstAddr_P, void *srcAddr_P, uint32 size)
{
    volatile uint16         *dstAddr;
    volatile uint16         *srcAddr;
    uint32                  len;
    uint32                  count = 0;

    ASSERT_CSP( srcAddr_P );
    ASSERT_CSP( dstAddr_P );
    ASSERT_CSP( map );

    /* Check if legal range */
    if ( ((uint32) dstAddr_P < FLASH_BASE) || 
            //((uint32) ( (uint32)dstAddr_P + size ) > (FLASH_BASE + FLASH_TOTAL_SIZE)) )
            ((uint32) ( (uint32)dstAddr_P + size ) > (FLASH_BASE + map->size)) )
    {
        printf("flashdrv_write legal range fail! %08x %d\n ", (uint32)dstAddr_P, size);
        return 1;
    }
    
    dstAddr = (volatile uint16 *) dstAddr_P;
    srcAddr = (volatile uint16 *) srcAddr_P;
    len = size / 2 + size % 2;
    
    while (len)
    {
        /* Start program sequence */
        *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_COMMAND1;
        *AM29LVXXX_COMMAND_ADDR2 = AM29LVXXX_COMMAND2;
        *AM29LVXXX_COMMAND_ADDR1 = AM29LVXXX_PROGRAM_CMD;
        *dstAddr = *srcAddr;
        
	    /* Check if completed */
	    count = 0;
	    while (1)
	    {
	        if ((uint16)*dstAddr == (uint16)*srcAddr)
	        {
	            dstAddr++;
	            srcAddr++;
	            break;
	        }
	        else
	        {
	            count++;
	            if (count > 0xFF)
                {
                    		printf("! Address %x write \n", (uint32) dstAddr);
	         	   	return 1;
	         	}
	        }
	    }
	    
        len--;
    }    
    
    return 0;
}

void flashdrv_test(uint32 channel, int32 argc, int8 ** argv)
{
    void *  pStartAddr;
    uint32  count = 0;
    uint32  blockNum;
    uint32  startBlkNum;
    
    pStartAddr = (void *) 0x80000;
    
    /* Search the starting block */
    //for (blockNum = 0; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
    for (blockNum = 0; blockNum < (map->block_num - 1); blockNum++)
        //if ( flashBlockOffset[blockNum + 1] > (uint32) pStartAddr )
        if ( map->blockOffset[blockNum + 1] > (uint32) pStartAddr )
            break;
    startBlkNum = blockNum;
    
    while (count++ < 1000)
    {
        /* Erase the blocks */
        //for (blockNum = startBlkNum; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
        for (blockNum = startBlkNum; blockNum < (map->block_num - 1); blockNum++)
        {
            //if ( flashdrv_eraseBlock((void *) (flashBlockOffset[blockNum] + FLASH_BASE) ) )
            if ( flashdrv_eraseBlock((void *) (map->blockOffset[blockNum] + FLASH_BASE) ) )
            {
                printf("Erase block #%d fail!\n", blockNum);
                return;
            }
        }
            
        /* Write blocks */
        //for (blockNum = startBlkNum; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
        for (blockNum = startBlkNum; blockNum < (map->block_num - 1); blockNum++)
        {
            //if ( flashdrv_write((void *) (flashBlockOffset[blockNum] + FLASH_BASE), 
            if ( flashdrv_write((void *) (map->blockOffset[blockNum] + FLASH_BASE), 
                                (void *) (0x80600000), 
                                0x10000) )
            {
                printf("Write block #%d fail!\n", blockNum);
                return;
            }
        }
        
        printf(".");
    }
    
    /* Clear */
    //for (blockNum = startBlkNum; blockNum < (FLASH_NUM_OF_BLOCKS - 1); blockNum++)
    for (blockNum = startBlkNum; blockNum < (map->block_num - 1); blockNum++)
    {
        /* Erase the block */
        //if ( flashdrv_eraseBlock((void *) (flashBlockOffset[blockNum] + FLASH_BASE) ) )
        if ( flashdrv_eraseBlock((void *) (map->blockOffset[blockNum] + FLASH_BASE) ) )
        {
            printf("Erase block #%d fail!\n", blockNum);
            return;
        }
    }
    
    printf("SUCCESS!\n");
}
