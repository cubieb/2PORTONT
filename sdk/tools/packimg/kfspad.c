/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /usr/local/dslrepos/uClinux-dist/tools/packimg/kfspad.c,v 1.1 2005/12/07 11:43:06 kaohj Exp $
 *
 * $Author: kaohj $
 *
 * Abstract:
 *
 *   Append image header, calculate checksum and padding.
 *
 * $Log: kfspad.c,v $
 * Revision 1.1  2005/12/07 11:43:06  kaohj
 * mmu support
 *
 * Revision 1.2  2005/03/11 04:25:10  rupert
 * *:  give filename to mkstemp
 *
 * Revision 1.1  2004/12/01 07:34:21  yjlou
 * *** empty log message ***
 *
 * Revision 1.1  2002/07/19 05:50:00  danwu
 * Create file.
 *
 *
 * 
 */

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <time.h>
/* integration with Loader. */
#include    "rtl_types.h"
#include    "rtl_image.h"

const char logo_msg[] = {
	"(c)Copyright Realtek, Inc. 2002\n" 
	"Project BERLIN\n\n"
};
#define ENDIAN_SWITCH32(x) (((x) >> 24) | (((x) >> 8) & 0xFF00) | \
                            (((x) << 8) & 0xFF0000) | (((x) << 24) &0xFF000000))
#define ENDIAN_SWITCH16(x) ((((x) >> 8) & 0xFF) | (((x) << 8) & 0xFF00))

void
main(int argc, char *argv[])
{
    FILE *              fp1;
    FILE *              fp2;
    char                tmpFilename[50] = "packbin.XXXXXX";
    unsigned long       imgSize;
    unsigned long       padTo;
    struct stat         fileStat;
    int       i;
    unsigned char       ch;
    unsigned char *     pch;
    unsigned char       chksum;
    unsigned int tmp;

for (i=0;i<argc;i++)
  printf("arg[%d]:%s\n",argc,argv[i]);

    /* Check arguments */
    if (argc != 4)
    {
        printf("Usage: kfspad rootfs kernel_file output_file\n");
        printf("    rootfs        Name of the root file system binary image file\n");
        printf("    kernel_file   Name of the kernel binary image file.\n");
        printf("    output_file   Name of the output binary image file.\n");
        
        return;
    }
    
    /* Open file */
     if ((fp1=fopen(argv[1], "r+b")) == NULL)
    {
        printf("Cannot open %s !!\n", argv[1]);
        return;
    }
    
    /* Get file size */
    if (stat(argv[1], &fileStat) != 0)
    {
        printf("Cannot get file statistics !!\n");
        fclose(fp1);
        exit(-1);
    }
    imgSize = fileStat.st_size;
    printf("Root File System Image Size = 0x%lx %d\n", imgSize,imgSize);
    
    /* Temparay file */
    //tmpnam(tmpFilename);
    mkstemp(tmpFilename);
    if ((fp2=fopen(tmpFilename, "w+b")) == NULL)
    {
        printf("Cannot open temprary file !!\n");
        return;
    }
    
    /* Copy image */
    fseek(fp1, 0L, SEEK_SET);
    chksum = 0;
    for (i=0; i<imgSize; i++)
    {
        ch = fgetc(fp1);
        fputc(ch, fp2);
    }
    close(fp1);
    
    //16 byte aligned between kernel & fs
    padTo = (16-(imgSize&0x0F))&0x0F;
    i=0;
    while (i++ < padTo)
            fputc(0, fp2);

     if ((fp1=fopen(argv[2], "r+b")) == NULL)
    {
        printf("Cannot open %s !!\n", argv[2]);
        return;
    }
    
    /* Get file size */
    if (stat(argv[2], &fileStat) != 0)
    {
        printf("Cannot get file statistics !!\n");
        fclose(fp1);
        exit(-1);
    }
    imgSize = fileStat.st_size;
    printf("Kernel Image Size = 0x%lx %d\n", imgSize,imgSize);
    for (i=0; i<imgSize; i++)
    {
        ch = fgetc(fp1);
        fputc(ch, fp2);
    }

    /* Close file and exit */
    fclose(fp1);
    fclose(fp2);
    
    /*Add offset information at file start*/
    
    printf("Binary image %s generated!\n", argv[3]);
    remove(argv[3]);
    rename(tmpFilename, argv[3]);
}

