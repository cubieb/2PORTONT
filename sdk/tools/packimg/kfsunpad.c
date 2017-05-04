/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * Abstract:
 *
 *   Seperate rootfs(squash fs) and kernel image.
 *
 */

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <time.h>

const char logo_msg[] = {
	"(c)Copyright Realtek, Inc. 2002\n" 
	"Project BERLIN\n\n"
};

#define ENDIAN_SWITCH32(x) (((x) >> 24) | (((x) >> 8) & 0xFF00) | \
                            (((x) << 8) & 0xFF0000) | (((x) << 24) &0xFF000000))
#define ENDIAN_SWITCH16(x) ((((x) >> 8) & 0xFF) | (((x) << 8) & 0xFF00))
// Kaohj
char squashfs_magic[4] = {0x73, 0x71, 0x73, 0x68};
char df_rootfs[16] = "myrootfs";
char df_klzma[16] = "myk.lzma";

int
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
    // Kaohj
    char buf[16], *pchar;
    int *ptr, *pint;
    int sqfsSize;
    char *rootfs, *klzma;

    for (i=0;i<argc;i++)
        printf("arg[%d]:%s\n",i,argv[i]);

    /* Check arguments */
    if (argc < 2)
    {
        printf("Usage: kfsunpad kfs_file [out-rootfs] [out-kernel_file]\n");
        printf("    kfs_file          Name of the rootfs+kernel image file.\n");
        printf("    out-rootfs        Name of the output root file system image file, default to 'myrootfs'.\n");
        printf("    out-kernel_file   Name of the output kernel image file, default to 'myk.lzma'.\n");
        
        return 0;
    }
    
    /* Open file */
     if ((fp1=fopen(argv[1], "r+b")) == NULL)
    {
        printf("Cannot open %s !!\n", argv[1]);
        return 0;
    }
    
    // Kaohj --- check squashfs
    fread(buf, 16, 1, fp1);
    ptr = (int *)&buf[0];
    //printf("head=0x%x\n", *ptr);
    if ( memcmp(squashfs_magic, buf, 4) != 0 ) {
    	printf("not Squashfs!\n");
    	fclose(fp1);
    	return 0;
    }
    /* Get file size */
    if (stat(argv[1], &fileStat) != 0)
    {
        printf("Cannot get file statistics !!\n");
        fclose(fp1);
        exit(-1);
    }
    imgSize = fileStat.st_size;
    printf("Source Image Size = 0x%lx %d\n", imgSize,imgSize);
    // Endian transition: big->little
    pchar = (char *)(ptr+2);
    buf[0] = pchar[3];
    buf[1] = pchar[2];
    buf[2] = pchar[1];
    buf[3] = pchar[0];
    pint = (int *)&buf[0];
    sqfsSize = (*pint+0x0FFF)&(~0x0FFF);
    printf("sqfs size = %d\n", sqfsSize);
    // Copy Squash rootfs image
    if (argc >= 3)
    	rootfs = argv[2];
    else
    	rootfs = df_rootfs;
    if ((fp2=fopen(rootfs, "w+b")) == NULL)
    {
        printf("Cannot create rootfs file !!\n");
        fclose(fp1);
        return 0;
    }
    fseek(fp1, 0L, SEEK_SET);
    for (i=0; i<sqfsSize; i++)
    {
        ch = fgetc(fp1);
        fputc(ch, fp2);
    }
    fclose(fp2);
    // Copy kernel lzma image
    if (argc >= 3)
    	klzma = argv[3];
    else
    	klzma = df_klzma;
    if ((fp2=fopen(klzma, "w+b")) == NULL)
    {
        printf("Cannot create rootfs file !!\n");
        fclose(fp1);
        return 0;
    }
    imgSize = imgSize - sqfsSize;
    printf("lzma kernel size = %d\n", imgSize);
    fseek(fp1, sqfsSize, SEEK_SET);
    for (i=0; i<imgSize; i++)
    {
        ch = fgetc(fp1);
        fputc(ch, fp2);
    }
    fclose(fp1);
    fclose(fp2);
    return 0;
}

