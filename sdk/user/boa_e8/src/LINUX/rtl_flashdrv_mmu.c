/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/user/boa/src/LINUX/rtl_flashdrv_mmu.c,v 1.24 2012/09/19 06:18:16 kaohj Exp $
*
* Abstract: Flash driver source code.
*
* $Author: kaohj $
*
*
* ---------------------------------------------------------------
*/

#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../include/linux/autoconf.h"
#endif
#include <linux/version.h>
#include <dirent.h>

//andrew, maybe we can merge into one, by replace MEMWRITE/MEMREAD with write/read
#if (LINUX_VERSION_CODE < 0x00020600)  // linux 2.4?

#define KERNEL_VERSION_24 1

#ifdef EMBED
#include <linux/mtd/mtd.h>
#endif

#else // linux 2.6

#ifdef EMBED
#include <mtd/mtd-user.h>
#endif
#include "mib.h"
#endif

#include "rtl_types.h"
#include "rtl_board.h"
//#include "rtl_flashdrv.h"

#if 0
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
#endif

struct flash_mtd_info_t {
	uint32	start;
	uint32	end;
	uint32	erasesize;
	char	name[16];
};

//see drivers/mtd/maps/rtl86xx_flash.c for partition details
static struct flash_mtd_info_t flash_mtd_info[8];
int flash_mtd_num = 0;

int get_flash_index(void *src_addr)
{
    int idx=0;
    
    do {
    	if ((uint32)src_addr<flash_mtd_info[idx].end)
    		return idx;
    	idx++;
    } while (idx<(sizeof(flash_mtd_info)/sizeof(struct flash_mtd_info_t)));
    return -1;
}

/***
 * get_mtd_fd - get the file descriptor of the named MTD device
 * @name: the name of the MTD device
 *
 * returns fd if success. Otherwise, returns -1.
 */
int get_mtd_fd(const char *name)
{
	FILE *fp_mtd;
	char buf[128], flashdev[32];
	int index = -1, fd = -1;

	fp_mtd = fopen("/proc/mtd", "r");
	if (fp_mtd) {
		while (fgets(buf, sizeof(buf), fp_mtd)) {
			if (strstr(buf, name)) {
				sscanf(buf, "mtd%u", &index);
				break;
			}
		}
		fclose(fp_mtd);

		if (index == -1) {
			fprintf(stderr, "can't find partition %s in /proc/mtd\n", name);
			goto fail;
		}
	} else {
		fprintf(stderr, "open /proc/mtd fail!\n");
		goto fail;
	}

	sprintf(flashdev, "/dev/mtd%d", index);
	printf("%s: %s\n", name, flashdev);
	fd = open(flashdev, O_RDWR);

fail:
	return fd;
}

/*
 *	Get the start address offset of mtd named 'part_name'.
 *	Return address on successful, or -1 on error.
*/
int get_mtd_start(const char *part_name)
{
    int idx=0;
    int num;
    
    do {
	if (!strcmp(flash_mtd_info[idx].name, part_name))
		return flash_mtd_info[idx].start;
	idx++;
    } while (idx < flash_mtd_num);
    
    return -1;
}

/*
 *	Get the flash CURRENT_SETTING_OFFSET.
 *	Return offset address on successful, or -1 on error.
*/
int get_cs_offset()
{
	int ret;
#ifdef CONFIG_MTD_NAND
	ret = CURRENT_SETTING_OFFSET;
#else
	ret = get_mtd_start("CS");
#endif
	return ret;
}
/*
static void dump_mtd_info(mtd_info_t *mtd) {
	printf("\nMTD Info\n");
	printf("\tType: %d\tFlags: %d\tSize: 0x%x\n", mtd->type, mtd->flags, mtd->size);
	printf("\tErasesize: 0x%x\tWritesize: 0x%x\tOOBSize: 0x%x\n", mtd->erasesize, mtd->writesize, mtd->oobsize);
}
*/

void dump_flash_mtd_info()
{
	int i=0;
    
	printf("Flash MTD INFO:\n");
	printf("Index\tStart\t\tEnd\t\tErase_size\tName\n");
	
	do {
		printf("%d\t0x%08x\t0x%08x\t0x%08x\t%s\n", i, flash_mtd_info[i].start
			, flash_mtd_info[i].end, flash_mtd_info[i].erasesize
			, flash_mtd_info[i].name);
		i++;
	} while (i<flash_mtd_num);
}

/**
 * Store MTD name to flash_mtd_info from proc fs.
 * Retrun 0 if success, return else if fail.
*/
static int get_mtd_name_from_proc()
{
	FILE	*fp=NULL;
	char buff[128], tmp[20];
	int i=0;
	int ret=0;
	
	if (!(fp=fopen("/proc/mtd", "r")))
		printf("/proc/mtd not exists.\n");
	else {
		fgets(buff, sizeof(buff), fp);
		i = 0;
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			if (sscanf(buff, "%*s%*s%*s%s", tmp) != 1) {
				printf("Unsuported MTD partition format\n");
				ret = 1;
				break;
			}
			// strip enclosed "
			tmp[strlen(tmp)-1] = '\0';
			strcpy(flash_mtd_info[i].name, tmp+1);
			//printf("i=%d name=%s start=0x%x\n", i, flash_mtd_info[i].name, flash_mtd_info[i].start);
			i++;
		}
		fclose(fp);
	}

	return 0;
}


#define SYSFS_MTD_PATH "/sys/class/mtd"
#define SYSFS_MTD_PATT "mtd%d"
#define SYSFS_MTD_NAME_FMT "/sys/class/mtd/mtd%d/name"
/**
 * See if the MTD driver of this system supports sysfs or not.
 * Return 1 if yes, return 0 if no, return -1 if error.
 *
 * The concept is got from mtd-utils.
*/
static int is_sysfs_supported()
{
	DIR *sysfs_mtd = NULL;
	char filename[128]={0};
	int num = -1;
	int fd = 0;

	sysfs_mtd = opendir(SYSFS_MTD_PATH);
	if (!sysfs_mtd)
		return 0;

	/*
	 * mtd-utils:
	 * First of all find an "mtdX" directory. This is needed because there
	 * may be, for example, mtd1 but no mtd0.
	 * 
	 * W.H.: Although we start with mtd0, I still add these codes to make 
	 * this function still workable if we decide to hide some blocks in future.
	 */
	while (1) {
		int ret, mtd_num;
		char tmp_buf[256];
		struct dirent *dirent;

		dirent = readdir(sysfs_mtd);
		if (!dirent)
			break;

		if (strlen(dirent->d_name) >= 255) {
			printf("invalid entry in %s: \"%s\"",
			       SYSFS_MTD_PATH, dirent->d_name);
			errno = EINVAL;
			closedir(sysfs_mtd);
			return -1;
		}

		ret = sscanf(dirent->d_name, SYSFS_MTD_PATT"%s",
			     &mtd_num, tmp_buf);
		if (ret == 1) {
			num = mtd_num;
			break;
		}
	}

	if (closedir(sysfs_mtd))
		return printf("closedir failed on \"%s\"", SYSFS_MTD_PATH);

	if (num == -1)
		/* No mtd device, treat this as pre-sysfs system */
		return 0;

	// Check name node is existed or not
	sprintf(filename, SYSFS_MTD_NAME_FMT, num);
	fd = open(filename, O_RDONLY);
	if (fd == -1)
		return 0;

	if (close(fd))
	{
		printf("close failed on \"%s\"", filename);
		return -1;
	}
	
	return 1;
}

/**
 * Store MTD name to flash_mtd_info from proc fs.
 */
static int get_mtd_name_from_sysfs()
{
	FILE	*fp=NULL;
	char filename[256]={0};
	int i=0, len=0;

	for(i=0 ; i < flash_mtd_num ; i++)
	{
		sprintf(filename, SYSFS_MTD_NAME_FMT, i);
		if (!(fp=fopen(filename, "r")))
		{
			printf("Cannot open %s to read, skip this node", filename);
			continue;
		}

		// get name and strip '\n'
		fgets(flash_mtd_info[i].name, sizeof(flash_mtd_info[i].name), fp);
		len = strlen(flash_mtd_info[i].name);
		flash_mtd_info[i].name[len-1] = '\0';
		
		fclose(fp);
	}

	return 0;
}


uint32 flashdrv_init()
{
#ifdef EMBED
	int	fd, i, ret;
	char flashdev[32];
	mtd_info_t	mtd_info;

	flash_mtd_num = 0;
	memset(&flash_mtd_info[0], 0, sizeof(flash_mtd_info));
	for (i=0;i<sizeof(flash_mtd_info)/sizeof(struct flash_mtd_info_t);i++) {
		sprintf(flashdev, "/dev/mtd%d", i);
		if ((fd = open(flashdev, O_RDONLY)) < 0) {
			//printf("ERROR: failed to open(%s), errno=%d\n",
			//    flashdev, errno);
			goto flashdrv_finish;
		}
		if (ioctl(fd, MEMGETINFO, &mtd_info) < 0) {
			printf("ERROR: ioctl(MEMGETINFO) failed, errno=%d\n",
			    errno);
			close(fd);
			return 1;
		}

		flash_mtd_num++;
		flash_mtd_info[i].erasesize = mtd_info.erasesize;
		flash_mtd_info[i].end = (uint32)mtd_info.size;
		if (i>0) flash_mtd_info[i].end += flash_mtd_info[i-1].end;
		
		// Added by Mason Yu
		if (i>0)
			flash_mtd_info[i].start = flash_mtd_info[i-1].end;
		else
			flash_mtd_info[i].start = 0x0;
		
		//printf("flash end %d: %08x\n", i, mtd_end[i]);
		//printf("flash start %d: %08x\n", i, mtd_start[i]);
		close(fd);
	}
flashdrv_finish:
	// mtd name
	if(is_sysfs_supported() > 0)
	{
		// get mtd name from /sys/class/mtd/mtdX/name
		ret = get_mtd_name_from_sysfs();
	}
	else
	{
		// original code flow, get from /proc/mtd
		ret = get_mtd_name_from_proc();
	}

#endif

	return ret;
}

#if 0
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
#endif

uint32 flashdrv_read (void *dstAddr_P, void *srcAddr_P, uint32 size)
{
#ifdef EMBED
	int idx;
	unsigned long thislen, thissrc, ofs;
	unsigned char *thisdst;
	//ASSERT_CSP( srcAddr_P );
	//ASSERT_CSP( dstAddr_P );
	
	if ( (idx=get_flash_index(srcAddr_P)) >=0 ) {
		int	fd;
		char flashdev[32];
		#if KERNEL_VERSION_24
		struct mtd_oob_buf arg;
		#endif
		
		sprintf(flashdev, "/dev/mtd%d", idx);
		if ((fd = open(flashdev, O_RDWR)) < 0) {
			printf("ERROR: failed to open(%s), errno=%d\n",
			    flashdev, errno);
			return 1;
		}
		
		thissrc = (uint32)srcAddr_P;
			thisdst = (unsigned char *)dstAddr_P;
			ofs = 0;
		while (size) {
			thissrc = (uint32)srcAddr_P+ofs;
			thisdst = (unsigned char *)dstAddr_P+ofs;
			// Kaohj
			// Note: the size (4096) depends on the mtd read function.
			//	drivers/mtd/maps/rtl865x_flash.c: rtl865x_map_copy_from()
			//	would check to fit the filesystem block size
			if  (size <= 4096) {
				thislen = size;
			}
			else {
				thislen = 4096;
				ofs += 4096;
			}
			
			#if KERNEL_VERSION_24
			arg.start=thissrc;
			arg.length=thislen;
			arg.ptr=thisdst;
			if (ioctl(fd, MEMREADDATA, &arg) < 0) {
				printf("ERROR: ioctl(MEMREADDATA) failed, errno=%d\n",
				    errno);
				close(fd);
				return 1;
			}
			#else
			{
			/*int rv; 
			fprintf(stderr, "lseek(%d, %x) = ", fd, thissrc);
			rv = lseek(fd, thissrc, SEEK_SET);			
			fprintf(stderr, "%d\n", rv);
			fprintf(stderr, "read(%d, %x, %d) = ", fd, thisdst, thislen);
			rv = read(fd, thisdst, thislen);
			fprintf(stderr, "%d\n", rv);
			if (rv < 0) {
				printf("ERROR: read %u bytes from %x to %x failed, errno=%d\n",
					thislen, thissrc, thisdst, errno);
				return 1;
			}*/
			  thissrc = thissrc - flash_mtd_info[idx].start;
			if ((lseek(fd, thissrc, SEEK_SET) < 0) ||(read(fd, thisdst, thislen) < 0)) {
				printf("ERROR: read %u bytes from %x to %x failed, errno=%d\n",
					thislen, thissrc, thisdst, errno);
				close(fd);
			    return 1;
		    }
			}
			#endif
			size-=thislen;
	    }

		close(fd);
		return 0;
	} else
		return 1;
#else
	return 0;
#endif
}
/*
 *	Erase block with this dstAddr_P.
 *	dstAddr_P: offset to flash
 */
uint32 flashdrv_erase(void *dstAddr_P)
{
#ifdef EMBED
	int idx;
	uint32	offset;
	
	if ((idx=get_flash_index(dstAddr_P))>=0) {
		int	fd;
		char flashdev[32];
		struct erase_info_user erase_info;
		struct mtd_oob_buf argw;
		int writebyte;
		
		sprintf(flashdev, "/dev/mtd%d", idx);
		//printf("Erase %s\n", flashdev);
		if ((fd = open(flashdev, O_RDWR)) < 0) {
			printf("ERROR: failed to open(%s), errno=%d\n",
				flashdev, errno);
			return 1;
		}
		
		erase_info.length = flash_mtd_info[idx].erasesize;
		// mtd offset
		offset=(uint32)dstAddr_P - flash_mtd_info[idx].start;
		// aligned with erasesize
		offset = (offset & ~(flash_mtd_info[idx].erasesize-1));
		erase_info.start = offset;
		//printf("erase: offset=0x%x, erasesize=0x%x\n", offset, flash_mtd_info[idx].erasesize);
		if (ioctl(fd, MEMERASE, &erase_info) < 0) {
			printf("ERROR: ioctl(MEMERASE) failed, errno=%d\n",
				errno);
			close(fd);
			return 1;
		}
		return 1;
	} else
		return 1;
	
#else
	return 0;
#endif
}

// Copy -> Erase -> Write new copy
uint32 flashdrv_write2(void *dstAddr_P, void *srcAddr_P, uint32 size)
{
#ifdef EMBED
	int idx;
	char *buff = 0;
	uint32 write_offset, block_offset, offset, erasesize;
	
	if ((idx=get_flash_index(dstAddr_P))>=0) {
		int	fd;
		char flashdev[32];
		int writebyte;
		
		// get write offset against mtd
		write_offset=(uint32)dstAddr_P - flash_mtd_info[idx].start;
		// aligned with erasesize to get the block start
		block_offset = (write_offset & ~(flash_mtd_info[idx].erasesize-1));
		erasesize = flash_mtd_info[idx].erasesize;
		// check if we will write over the boundary
		if ((write_offset+size) > (block_offset+erasesize)) {
			printf("Write over the erase boundary !\n");
			return 1;
		}
		//printf("write_offset=0x%x, block_offset=0x%x, erasesize=0x%x\n",
		//	write_offset, block_offset, erasesize);
		buff = malloc(erasesize);
		if (!buff) {
			printf("%s: allocate buffer fail!!\n", __FUNCTION__);
			return 1;
		}
		// Read this block
		offset = ((uint32)dstAddr_P & ~(flash_mtd_info[idx].erasesize-1));
		if (flashdrv_read(buff, (void *)offset, erasesize) !=0 )
		{	
			printf("%s: read fail!!", __FUNCTION__);
			goto error;
		}
		// new copy with write data
		memcpy(buff + (write_offset-block_offset) , srcAddr_P, size);
		// Erase block
		flashdrv_erase(dstAddr_P);
		
		sprintf(flashdev, "/dev/mtd%d", idx);
		printf("write %s\n", flashdev);
		if ((fd = open(flashdev, O_RDWR)) < 0) {
			printf("ERROR: failed to open(%s), errno=%d\n",
				flashdev, errno);
			goto error;
		}
		
		if ((lseek(fd, block_offset, SEEK_SET) < 0)) {
			printf("ERROR: write block failed, errno=%d\n", errno);
			close(fd);
			goto error;
		}
		// Write new copy to block
		writebyte = write(fd, buff, erasesize);
		
		close(fd);
		free(buff);
		return writebyte;
	} else
		return 1;
error:
	if (buff)	free(buff);
	return 1;

#else
	return 0;
#endif
}

// Write directly (no Erase)
uint32 flashdrv_write(void *dstAddr_P, void *srcAddr_P, uint32 size)
{
#ifdef EMBED
  int idx;
    //ASSERT_CSP( srcAddr_P );
    //ASSERT_CSP( dstAddr_P );
    
    if ((idx=get_flash_index(dstAddr_P))>=0) {
        int	fd;
        char flashdev[32];
        struct erase_info_user arge;
        struct mtd_oob_buf argw;
	int writebyte;

		sprintf(flashdev, "/dev/mtd%d", idx);
		printf("write %s\n", flashdev);
	    if ((fd = open(flashdev, O_RDWR)) < 0) {
		    printf("ERROR: failed to open(%s), errno=%d\n",
			    flashdev, errno);
		    return 1;
	    }
	    
	    // Added by Mason Yu	   
	    dstAddr_P = dstAddr_P - flash_mtd_info[idx].start;
	    	

#if 0	
	    argw.start=(uint32)dstAddr_P;
	    argw.length=(uint32)size;
	    argw.ptr=(unsigned char *)srcAddr_P;
	    if (ioctl(fd, MEMWRITEDATA, &argw) < 0) {
		    printf("ERROR: ioctl(MEMREADDATA) failed, errno=%d\n",
			    errno);
		    close(fd);    
		    return 1;
	    }
#else
	    //if ((lseek(fd, dstAddr_P, SEEK_SET) < 0) ||(write(fd, srcAddr_P, size) < 0)) {
	    if ((lseek(fd, (uint32_t)dstAddr_P, SEEK_SET) < 0)) {
		   printf("ERROR: write %u bytes from %x to %x failed, errno=%d\n",
			size, srcAddr_P, dstAddr_P, errno);
		   close(fd);
		   return 1;
	    }
	    writebyte = write(fd, srcAddr_P, size);
#endif
	    
	    close(fd);    
	    return writebyte;
//       return 0;
    } else
        return 1;

#if 0	
    volatile uint16         *dstAddr;
    volatile uint16         *srcAddr;
    uint32                  len;
    uint32                  count = 0;

    //ASSERT_CSP( srcAddr_P );
    //ASSERT_CSP( dstAddr_P );
    //ASSERT_CSP( map );

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
#endif    
#else
	return 0;
#endif
}

uint32 flashdrv_updateImg(void *srcAddr_P, void *dstAddr_P, uint32 size)
{
#ifdef EMBED
  int idx;
    //ASSERT_CSP( srcAddr_P );
    //ASSERT_CSP( dstAddr_P );

    if ((idx=get_flash_index(dstAddr_P))>=0) {
        int	fd;
        char flashdev[32];
	int ret;
        struct erase_info_user arge;
        struct mtd_oob_buf argw;

		sprintf(flashdev, "/dev/mtd%d", idx);
	    if ((fd = open(flashdev, O_RDWR)) < 0) {
		    printf("ERROR: failed to open(%s), errno=%d\n",
			    flashdev, errno);
		    return 1;
	    }
	    
	    // Added by Mason Yu	    
	    dstAddr_P = dstAddr_P - flash_mtd_info[idx].start;
	    //printf("flashdrv_updateImg(1): dstAddr_P=0x%x mtd_start[%d]=0x%x\n", dstAddr_P, idx, mtd_start[idx] );
	    
	    arge.start=(uint32)dstAddr_P;
	    arge.length=(uint32)size;
	    if (ioctl(fd, MEMERASE, &arge) < 0) {
		    printf("ERROR: ioctl(MEMERASE) failed, errno=%d\n",
			    errno);
			close(fd);
		    return 1;
	    }
	    close(fd);    
        /* Write blocks */
        // Added by Mason Yu        
	   dstAddr_P = dstAddr_P + flash_mtd_info[idx].start;        
           //printf("flashdrv_updateImg(2):dstAddr_P=0x%x  mtd_start[%d]=0x%x\n", dstAddr_P, idx, mtd_start[idx] );
        
	    ret = flashdrv_write(dstAddr_P, srcAddr_P, size);
	   if (ret == size)
	        return 0;
	   else
	   	return 1;
        //return flashdrv_write(dstAddr_P, srcAddr_P, size);
    } else
        return 1;
#else
	return 0;
#endif
}

#if 0
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
#endif
