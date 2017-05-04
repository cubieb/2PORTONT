/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /usr/local/dslrepos/uClinux-dist/tools/packimg/rtl_image.h,v 1.1 2005/12/07 11:43:06 kaohj Exp $
 *
 * $Author: kaohj $
 *
 * Abstract:
 *
 * rtl_image.h
 *
 *   Structure definitions of root directory and image header.
 *
 * $Log: rtl_image.h,v $
 * Revision 1.1  2005/12/07 11:43:06  kaohj
 * mmu support
 *
 * Revision 1.2  2005/05/27 11:40:58  yjlou
 * *: merge the 'rtl_image.h' file of loader and boa.
 * *: fixed the bug of Web page that cannot update CRAMFS and SQUASHFS.
 *
 * Revision 1.1  2005/04/19 04:58:15  tony
 * +: BOA web server initial version.
 *
 * Revision 1.1.1.1  2003/08/27 06:20:15  rupert
 * uclinux initial
 *
 * Revision 1.1.1.1  2003/08/27 03:08:53  rupert
 *  initial version 
 *
 * Revision 1.2  2003/06/11 07:35:23  orlando
 * comment out <core/types.h>.
 *
 * Revision 1.1  2003/06/09 05:15:03  orlando
 * initial check-in of this file to vxworks development platform.
 *
 * Revision 1.1  2002/11/29 05:44:26  orlando
 * Moved due to include/export/mgmt -> include/export/management.
 *
 * Revision 1.3  2002/07/19 05:47:08  danwu
 * Add product magic number.
 * Add image types for root directory and boot image.
 * Change data type of checksums to uint8.
 *
 * Revision 1.1  2002/07/11 13:59:18  orlando
 * Structure definitions of root directory and file image header.
 * This header file will be referenced by boot code and filesystem
 * module.
 *
 * 
 */

#ifndef _RTL_IMAGE_H_
#define _RTL_IMAGE_H_

//#include <core/types.h>

/* "productMagic" field of fileImageHeader_t for ROME
 */
#define RTL_PRODUCT_MAGIC     0x59a0e842

/* "imageType" field of rootDirEntry_t and
 * fileImageHeader_t
 */
#define RTL_IMAGE_TYPE_RDIR   0xb162
#define RTL_IMAGE_TYPE_BOOT   0xea43
#define RTL_IMAGE_TYPE_RUN    0x8dc9
#define RTL_IMAGE_TYPE_KFS    0xd92f
#define RTL_IMAGE_TYPE_CCFG   0x2a05
#define RTL_IMAGE_TYPE_DCFG   0x6ce8
#define RTL_IMAGE_TYPE_LOG    0xc371

#define RTL_IMAGE_ROOT_DIR_ENTRY_NAME_MAX_LEN   15
typedef struct rootDirEntry_s {
	char name[RTL_IMAGE_ROOT_DIR_ENTRY_NAME_MAX_LEN+1];
	     /* entry name.
	      * example value:
	      *   "run"
	      *   "web"
	      *   "ccfg"
	      *   "dcfg"
	      *   "log"
	      */
	uint32 startOffset;  /* starting offset
	                      * offset relative to memory map origin
                          * of this file device
	                      * !! not the absolute address in memory map !!
	                      */
	uint32 maxSize;      /* this entry has been allocated an area of size
	                      * 'maxSize' on the flash memory device.
	                      * The size of image header is counted in this maxSize.
	                      * That is, the effective size is actually
	                      * ( maxSize - sizeof(fileImageHeader_t) )
	                      */
	uint32 reserved1;  /* used to store the offset when this file image is loaded
	                    * into this module's local buffer
	                    */
	uint8  filePerm;  /* rtl_fcmn.h: FCMN_FP_R | FCMN_FP_W | FCMN_FP_RW */
	uint8  fileType;  /* rtl_fcmn.h: FCMN_FT_REG_FILE | FCMN_FT_DIR */
	uint16 imageType; /* RTL_IMAGE_TYPE_XXX */
} rootDirEntry_t;


#define RTL_IMAGE_HDR_VER_1_0    1
typedef struct fileImageHeader_s {
    uint32 productMagic; /* Product Magic Number */
	uint16 imageType;   /* RTL_IMAGE_TYPE_XXX */
	uint8  imageHdrVer; /* image header format version */
	uint8  reserved1; /* for 32-bit alignment */
	uint32 date;      /* Image Creation Date (in Network Order)
                       * B1B2:year(0..65535) (BigEndian)
                       * B3:month(1..12)
                       * B4:day(1..31)
                       */
	uint32 time;     /* Image Creation Time (in Network Order)
                      * B1:hour(0..23)
                      * B2:minute(0..59)
                      * B3:second(0..59)
                      */
	uint32 imageLen;      /* image header length not counted in */
	uint16 reserved2;
	uint8 imageBdyCksm;  /* cheacksum cover range: image body */
	uint8 imageHdrCksm;  /* cheacksum cover range: image header */	
} fileImageHeader_t;


#endif /* _RTL_IMAGE_H_ */
