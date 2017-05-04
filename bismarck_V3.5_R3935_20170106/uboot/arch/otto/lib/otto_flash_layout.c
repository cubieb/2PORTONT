/*
 * flash layout module
 * Author: bohungwu@realtek.com
 * Last modified: May 15, 2014
 */

#include <common.h>
#include <soc.h>

//#define FL_DEBUG
#ifdef FL_DEBUG
#define DEBUG(format, args...) printf(format, ##args)
#else
#define DEBUG(args...)
#endif

#define CL_SIZE (256)

#ifdef CONFIG_CMD_SF
	#define RTK_MTD_DEV_NAME "rtk_spi_nor_mtd"
	#include <spi_flash.h>
	#include "nor_spi_gen3/nor_spif_core.h"
#elif defined CONFIG_CMD_SPI_NAND
	#define RTK_MTD_DEV_NAME "spinand"
#elif defined CONFIG_CMD_ONFI
        #define RTK_MTD_DEV_NAME "onfi"
#else
	#error "Error!!! One of CONFIG_CMD_SF or CONFIG_CMD_SPI_NAND or CONFIG_CMD_ONFI must be defined"
#endif

#define GB_BITS (30)
#define MB_BITS (20)
#define KB_BITS (10)
#define GB_MASK ((0x1<<GB_BITS)-1)
#define MB_MASK ((0x1<<MB_BITS)-1)
#define KB_MASK ((0x1<<KB_BITS)-1)

typedef struct {
	char* name;
	u32 base;
	u32 size;
} part_info_t;

#define BOOT_KERNEL "linux"
#define BOOT_ROOTFS "rootfs"
/* Move the following to different setting */
static part_info_t part_info[] = {
	{"config",    0                       , 0}, /* CFGFS    */
	{"env2",      CONFIG_ENV_OFFSET_REDUND, 0}, /* ENV2    */
	{"opt3",      0                       , 0}, /* opt3    */
	{"opt4",      0                       , 0}, /* opt4    */
	{"k0",        0                       , 0}, /* kernel1 */
	{"k1",        0                       , 0}, /* kernel2    */
	{"r0",        0                       , 0}, /* rootfs1 */
	{"r1",        0                       , 0}, /* rootfs2 */
	{"END",       0                       , 0}, /* end     */
	{"env",       CONFIG_ENV_OFFSET       , 0}, /* U-Boot environment partition */
	{"boot",      0                       , 0}, /* BOOTLOADER partition, must be the last entity in this array */
};
/* Should be consistent with part_info defined above */
#define FL_CFGFS 0
#define FL_ENV2 1
#define FL_OPT3 2
#define FL_OPT4 3
#define FL_KERNEL1 4
#define FL_KERNEL2 5
#define FL_ROOTFS1 6
#define FL_ROOTFS2 7
#define FL_END 8
#define FL_ENV 9
#define FL_BOOT 10

static part_info_t *pi_ptr[ARRAY_SIZE(part_info)];

#ifdef CONFIG_OTTO_FL_TO_MTDPARTS
char * to_KMGB(u32 val, char *output_p, u32 output_size) {
	if((val & GB_MASK) == 0) { /* To GB */
		sprintf(output_p, "%uG", (val >> GB_BITS));
	} else if((val & MB_MASK) == 0) {/* To MB */
		sprintf(output_p, "%uM", (val >> MB_BITS));
	} else if((val & KB_MASK) == 0) { /* To KB */
		sprintf(output_p, "%uK", (val >> KB_BITS));
	} else {
		sprintf(output_p, "%u", (val));
	}
	return output_p;

}

#define TS_SIZE (64) /* Magic number temp string size */
#define VIRTUAL_MTD_START 12
const u32 FL_KERNEL_ID[] = {FL_KERNEL1, FL_KERNEL2};
const u32 FL_ROOTFS_ID[] = {FL_ROOTFS1, FL_ROOTFS2};
//Generate mtdparts (root=31:? is also included)
static void flash_layout_to_mtdparts(char *mtdparts_output, u32 img_id) {
	char tmp_str0[TS_SIZE] = {0};
	char tmp_str1[TS_SIZE] = {0};
	u32 i, j, rootfs_partition;
	char *p;

	p = mtdparts_output;
	rootfs_partition = ARRAY_SIZE(part_info);

	p += sprintf(p, "mtdparts=%s:", RTK_MTD_DEV_NAME);
	for(i = 0, j = 0; i < ARRAY_SIZE(part_info); i++) {
		if(pi_ptr[i]->size != 0) {
			p += sprintf(p, "%s(%s),", to_KMGB(pi_ptr[i]->size, tmp_str0, TS_SIZE), pi_ptr[i]->name);
			//p += sprintf(p, "%s@%s(%s),", to_KMGB(pi_ptr[i]->size, tmp_str0, TS_SIZE), to_KMGB(pi_ptr[i]->base, tmp_str1, TS_SIZE), pi_ptr[i]->name);

			/* Record rootfs partition */
			if(pi_ptr[i]->name == part_info[FL_ROOTFS_ID[img_id]].name) {
				DEBUG("pi_ptr[i]->name=%s, part_info[FL_ROOTFS_ID[img_id]].name=%s,i=%d,j=%d\n", pi_ptr[i]->name, part_info[FL_ROOTFS_ID[img_id]].name, i, j);
				rootfs_partition = j;
#if (OTTO_NAND_FLASH==1)
				rootfs_partition--; /* Magic number handling for NAND platform */
				DEBUG("rootfs_partition-- for NAND platform\n");
#endif /* (OTTO_NAND_FLASH==1) */
			}
			j++;
		}
	}
#ifdef CONFIG_LUNA_MULTI_BOOT
	/* Fill padding mtd partition */
	for(;j<VIRTUAL_MTD_START;j++) {
		p += sprintf(p, "4K@0ro,");
		//p += sprintf(p, "4K@%s(m)ro,", to_KMGB(part_info[FL_END].size, tmp_str0, TS_SIZE));
	}
	/* Create alias partition for booting linux & kernel */
	p += sprintf(p, "%s@%s(%s),", to_KMGB(part_info[FL_KERNEL_ID[img_id]].size, tmp_str0, TS_SIZE), to_KMGB(part_info[FL_KERNEL_ID[img_id]].base, tmp_str1, TS_SIZE), BOOT_KERNEL);
	p += sprintf(p, "%s@%s(%s),", to_KMGB(part_info[FL_ROOTFS_ID[img_id]].size, tmp_str0, TS_SIZE), to_KMGB(part_info[FL_ROOTFS_ID[img_id]].base, tmp_str1, TS_SIZE), BOOT_ROOTFS);
#endif

	/* Replace the last ',' (comma sign) by a null character '\0' */
	if(mtdparts_output[strlen(mtdparts_output) - 1] == ',') {
		mtdparts_output[strlen(mtdparts_output) - 1] = '\0';
	}

	if(rootfs_partition == ARRAY_SIZE(part_info)) {
		printf("Error: Cannot find rootfs partition\n");
	} else {
		DEBUG("rootfs_partition=%d\n", rootfs_partition);
		p = mtdparts_output + strlen(mtdparts_output);
		p += sprintf(p, " root=31:%d", rootfs_partition);
	}

}

#endif /* #ifdef CONFIG_OTTO_FL_TO_MTDPARTS */

void sort_part_info(part_info_t *input_pi_ptr[], u32 num) {
	u32 i, j;
	part_info_t *item_p;

	/* Use insertion sort as the number of elements in the array is not huge */
	for(i=1; i<num; i++) {
		j = i;
		item_p = input_pi_ptr[j];
		while( (j > 0) && (item_p->base < input_pi_ptr[j-1]->base) ) {
			input_pi_ptr[j] = input_pi_ptr[j - 1];
			j--;
		}
		input_pi_ptr[j]= item_p;
	}
}

#define FLASHI                   (norsf_info)
u32 otto_get_flash_size(void) {
	u32 ret_val;
#if defined (CONFIG_CMD_SPI_NAND)
	extern uint32_t spi_nand_chip_size(uint32_t idx);
	ret_val=spi_nand_chip_size(0);
	/* ToDo , multiple chip to be considered */
#elif defined (CONFIG_CMD_ONFI)
        extern uint32_t onfi_chip_size(uint32_t idx);
        ret_val=onfi_chip_size(0);
        /* ToDo , multiple chip to be considered */
#else
	ret_val = FLASHI.num_chips * FLASHI.size_per_chip_b;
#endif
	return ret_val;
}

void part_info_init(void) {
	u32 i, j;

	i =  otto_get_flash_size();
#ifdef CONFIG_CMD_SF
	j =  getenv_ulong("fl_size", 10, 0);
	if( (j != 0) && (j != i) ){
		if( (j < i) && ((j == FL_8MB) || (j == FL_16MB))){
			printf("INFO: flash size=%dMB, but %dMB layout is used.\n", (i >> 20), (j >> 20));
			i = j;
		} else {
			printf("ERROR: flash size=%dMB, %dMB layout unsupported.\n", (i >> 20), (j >> 20));
		}
	}
#elif defined (CONFIG_CMD_SPI_NAND)
	j =  getenv_ulong("fl_size", 10, 0);
	if( (j != 0) && (j != i) ){
		if( (j < i) && (j == FL_64MB)){
			printf("INFO: flash size=%dMB, but %dMB layout is used.\n", (i >> 20), (j >> 20));
			i = j;
		} else {
			printf("ERROR: flash size=%dMB, %dMB layout unsupported.\n", (i >> 20), (j >> 20));
		}
	}
#elif defined (CONFIG_CMD_ONFI)
        j =  getenv_ulong("fl_size", 10, 0);
        if( (j != 0) && (j != i) ){
            if( (j < i) && (j == FL_64MB)){
                printf("INFO: flash size=%dMB, but %dMB layout is used.\n", (i >> 20), (j >> 20));
                i = j;
            } else {
                printf("ERROR: flash size=%dMB, %dMB layout unsupported.\n", (i >> 20), (j >> 20));
            }
        }
#endif
	switch(i) {
#ifdef CONFIG_CMD_SF
	case FL_32MB:
		part_info[FL_CFGFS].base   = FL_32MB_CFGFS_BASE;
		part_info[FL_KERNEL1].base = FL_32MB_K0_BASE;
		part_info[FL_ROOTFS1].base = FL_32MB_R0_BASE;
		part_info[FL_KERNEL2].base = FL_32MB_K1_BASE;
		part_info[FL_ROOTFS2].base = FL_32MB_R1_BASE;
		part_info[FL_END].base = FL_32MB;
	break;

	case FL_16MB:
		part_info[FL_CFGFS].base   = FL_16MB_CFGFS_BASE;
		part_info[FL_KERNEL1].base = FL_16MB_K0_BASE;
		part_info[FL_ROOTFS1].base = FL_16MB_R0_BASE;
		part_info[FL_KERNEL2].base = FL_16MB_K1_BASE;
		part_info[FL_ROOTFS2].base = FL_16MB_R1_BASE;
		part_info[FL_END].base = FL_16MB;
	break;

	case FL_8MB:
		part_info[FL_CFGFS].base   = FL_8MB_CFGFS_BASE;
		part_info[FL_KERNEL1].base = FL_8MB_K0_BASE;
		part_info[FL_ROOTFS1].base = FL_8MB_R0_BASE;
		part_info[FL_KERNEL2].base = FL_8MB_K1_BASE;
		part_info[FL_ROOTFS2].base = FL_8MB_R1_BASE;
		part_info[FL_END].base = FL_8MB;
	break;

	default:
		printf("INFO: layout for flash size=%dMB is not defined, %dMB layout is used.\n", (i >> 20), (FL_8MB >> 20));
		part_info[FL_CFGFS].base   = FL_8MB_CFGFS_BASE;
		part_info[FL_KERNEL1].base = FL_8MB_K0_BASE;
		part_info[FL_ROOTFS1].base = FL_8MB_R0_BASE;
		part_info[FL_KERNEL2].base = FL_8MB_K1_BASE;
		part_info[FL_ROOTFS2].base = FL_8MB_R1_BASE;
		part_info[FL_END].base = FL_8MB;
	break;
#elif defined (CONFIG_CMD_SPI_NAND)
	case FL_64MB:
		part_info[FL_CFGFS].base   = FL_64MB_CFGFS_BASE;
		part_info[FL_KERNEL1].base = FL_64MB_K0_BASE;
		part_info[FL_ROOTFS1].base = FL_64MB_R0_BASE;
		part_info[FL_KERNEL2].base = FL_64MB_K1_BASE;
		part_info[FL_ROOTFS2].base = FL_64MB_R1_BASE;
		part_info[FL_END].base = FL_MAX_USE_64MB;
	break;

	case FL_128MB:
		part_info[FL_CFGFS].base   = FL_128MB_CFGFS_BASE;
		part_info[FL_KERNEL1].base = FL_128MB_K0_BASE;
		part_info[FL_ROOTFS1].base = FL_128MB_R0_BASE;
		part_info[FL_KERNEL2].base = FL_128MB_K1_BASE;
		part_info[FL_ROOTFS2].base = FL_128MB_R1_BASE;
		part_info[FL_END].base = FL_MAX_USE_128MB;
	break;

	default:
		printf("INFO: layout for flash size=%dMB is not defined, %dMB layout is used.\n", (i >> 20), (FL_128MB >> 20));
		part_info[FL_CFGFS].base   = FL_64MB_CFGFS_BASE;
		part_info[FL_KERNEL1].base = FL_64MB_K0_BASE;
		part_info[FL_ROOTFS1].base = FL_64MB_R0_BASE;
		part_info[FL_KERNEL2].base = FL_64MB_K1_BASE;
		part_info[FL_ROOTFS2].base = FL_64MB_R1_BASE;
		part_info[FL_END].base = FL_MAX_USE_64MB;
	break;
#elif defined (CONFIG_CMD_ONFI)
        case FL_64MB:
            part_info[FL_CFGFS].base   = FL_64MB_CFGFS_BASE;
            part_info[FL_KERNEL1].base = FL_64MB_K0_BASE;
            part_info[FL_ROOTFS1].base = FL_64MB_R0_BASE;
            part_info[FL_KERNEL2].base = FL_64MB_K1_BASE;
            part_info[FL_ROOTFS2].base = FL_64MB_R1_BASE;
            part_info[FL_END].base = FL_MAX_USE_64MB;
        break;

        case FL_128MB:
            part_info[FL_CFGFS].base   = FL_128MB_CFGFS_BASE;
            part_info[FL_KERNEL1].base = FL_128MB_K0_BASE;
            part_info[FL_ROOTFS1].base = FL_128MB_R0_BASE;
            part_info[FL_KERNEL2].base = FL_128MB_K1_BASE;
            part_info[FL_ROOTFS2].base = FL_128MB_R1_BASE;
            part_info[FL_END].base = FL_MAX_USE_128MB;
        break;

        default:
            printf("INFO: layout for flash size=%dMB is not defined, %dMB layout is used.\n", (i >> 20), (FL_128MB >> 20));
            part_info[FL_CFGFS].base   = FL_64MB_CFGFS_BASE;
            part_info[FL_KERNEL1].base = FL_64MB_K0_BASE;
            part_info[FL_ROOTFS1].base = FL_64MB_R0_BASE;
            part_info[FL_KERNEL2].base = FL_64MB_K1_BASE;
            part_info[FL_ROOTFS2].base = FL_64MB_R1_BASE;
            part_info[FL_END].base = FL_MAX_USE_64MB;
        break;
#endif
	}

	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		pi_ptr[i] = &part_info[i];
	}
	DEBUG("Original setting\n");
	DEBUG("pi_ptr: name\tbase\tsize\n");
	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		DEBUG("[%d]: %s\t0x%08x\t0x%08x\n", i, pi_ptr[i]->name, pi_ptr[i]->base, pi_ptr[i]->size);
	}

	sort_part_info(pi_ptr, ARRAY_SIZE(part_info));
	DEBUG("After sorting\n");
	DEBUG("pi_ptr: name\tbase\tsize\n");
	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		DEBUG("[%d]: %s\t0x%08x\t0x%08x\n", i, pi_ptr[i]->name, pi_ptr[i]->base, pi_ptr[i]->size);
	}

	/* Fill size */
	for(i=0; i<(ARRAY_SIZE(part_info)-1); i++) {
		pi_ptr[i]->size = pi_ptr[i+1]->base - pi_ptr[i]->base;
	}
	DEBUG("After filling size\n");
	DEBUG("pi_ptr: name\tbase\tsize\n");
	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		DEBUG("[%d]: %s\t0x%08x\t0x%08x\n", i, pi_ptr[i]->name, pi_ptr[i]->base, pi_ptr[i]->size);
	}

	DEBUG("Valid partitions\n");
	DEBUG("part_info: name\tbase\tsize\n");
	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		if(pi_ptr[i]->size !=0) {
			DEBUG("[%d]: %s\t0x%08x\t0x%08x\n", i, pi_ptr[i]->name, pi_ptr[i]->base, pi_ptr[i]->size);
		}
	}
}

void otto_flash_layout_init(void) {
#ifdef CONFIG_OTTO_FL_TO_MTDPARTS
	char str_tmp[CL_SIZE] = {0};
#endif /* #ifdef CONFIG_OTTO_FL_TO_MTDPARTS */
#ifdef CONFIG_CMD_SPI_NAND
    u32 freeAddr;
    u32 decSize = 0xc00000;
#endif

	part_info_init();

	setenv_addr("fl_boot_sz",    (void *)part_info[FL_BOOT].size);
	setenv_addr("fl_env",        (void *)part_info[FL_ENV].base);
	setenv_addr("fl_env_sz",     (void *)part_info[FL_ENV].size);
	setenv_addr("fl_env2",       (void *)part_info[FL_ENV2].base);
	setenv_addr("fl_cfgfs",      (void *)part_info[FL_CFGFS].base);
	setenv_addr("fl_cfgfs_sz",   (void *)part_info[FL_CFGFS].size);
	setenv_addr("fl_kernel1",    (void *)part_info[FL_KERNEL1].base);
	setenv_addr("fl_kernel1_sz", (void *)part_info[FL_KERNEL1].size);
	setenv_addr("fl_rootfs1",    (void *)part_info[FL_ROOTFS1].base);
	setenv_addr("fl_rootfs1_sz", (void *)part_info[FL_ROOTFS1].size);
#ifdef CONFIG_LUNA_MULTI_BOOT
	setenv_addr("fl_kernel2",    (void *)part_info[FL_KERNEL2].base);
	setenv_addr("fl_kernel2_sz", (void *)part_info[FL_KERNEL2].size);
	setenv_addr("fl_rootfs2",    (void *)part_info[FL_ROOTFS2].base);
	setenv_addr("fl_rootfs2_sz", (void *)part_info[FL_ROOTFS2].size);
#endif /* #ifdef CONFIG_LUNA_MULTI_BOOT */
#ifdef CONFIG_CMD_SF
	setenv_addr("img0_kernel",    (void *)(NORSF_CFLASH_BASE + part_info[FL_KERNEL1].base));

#ifdef CONFIG_LUNA_MULTI_BOOT
	setenv_addr("img1_kernel",    (void *)(NORSF_CFLASH_BASE + part_info[FL_KERNEL2].base));
#endif /* #ifdef CONFIG_LUNA_MULTI_BOOT */
#endif /* #ifdef CONFIG_SPINOR_FLASH */

#ifdef CONFIG_CMD_SPI_NAND
    /* caculate kernel boot addr according to textbase */
    freeAddr = CONFIG_SYS_TEXT_BASE - decSize;
    if (freeAddr < 0x80000000)
    {
        printf("Error!!! bootm addr is %d. Set it to 0x80000000!!!\n",freeAddr);
        freeAddr = 0x80000000;
    }
    setenv_addr("freeAddr", (void *)freeAddr);
#endif /* #if defined (CONFIG_CMD_SPI_NAND) */

#ifdef CONFIG_OTTO_FL_TO_MTDPARTS
	flash_layout_to_mtdparts(str_tmp, 0);
	printf("mtdparts0=%s\n", str_tmp);
	setenv("mtdparts0", str_tmp);
#ifdef CONFIG_LUNA_MULTI_BOOT
	flash_layout_to_mtdparts(str_tmp, 1);
	printf("mtdparts1=%s\n", str_tmp);
	setenv("mtdparts1", str_tmp);
#endif /* #ifdef CONFIG_LUNA_MULTI_BOOT */
#endif /* #ifdef CONFIG_OTTO_FL_TO_MTDPARTS */
	/**** Run-time set commands instead of keeping a default configuration ***/
#ifdef CONFIG_CMD_SF
	setenv("erase_cfgfs", "sf erase ${fl_cfgfs} +${fl_cfgfs_sz}");
	setenv("erase_env", "sf erase ${fl_env} +${fl_env_sz};sf erase ${fl_env2} +${fl_env_sz}");
	setenv("upb", "tftp 80000000 plr.img && crc32 ${fileaddr} ${filesize} && sf erase 0 +${fl_boot_sz} && sf write ${fileaddr} 0 ${filesize}");
	setenv("upk", "tftp 80000000 uImage && crc32 ${fileaddr} ${filesize} && sf erase ${fl_kernel1} +${fl_kernel1_sz} && sf write ${fileaddr} ${fl_kernel1} ${filesize}");
	setenv("upr", "tftp 80000000 rootfs && crc32 ${fileaddr} ${filesize} && sf erase ${fl_rootfs1} +${fl_rootfs1_sz} && sf write ${fileaddr} ${fl_rootfs1} ${filesize}");
	setenv("yu", "loady 80000000 && cp.b 80000000 81000000 ${filesize} && cmp.b 80000000 81000000 ${filesize} && sf erase 0 ${filesize} && sf write 80000000 0 ${filesize}");
#ifdef CONFIG_LUNA_MULTI_BOOT
	setenv("upk1", "tftp 80000000 uImage && crc32 ${fileaddr} ${filesize} && sf erase ${fl_kernel2} +${fl_kernel2_sz} && sf write ${fileaddr} ${fl_kernel2} ${filesize}");
	setenv("upr1", "tftp 80000000 rootfs && crc32 ${fileaddr} ${filesize} && sf erase ${fl_rootfs2} +${fl_rootfs2_sz} && sf write ${fileaddr} ${fl_rootfs2} ${filesize}");
	setenv("bootargs_base", "console=ttyS0,115200");
	setenv("b0", "setenv bootargs ${bootargs_base} ${mtdparts0} ${rst2dfl_flg}; bootm ${img0_kernel}");
	setenv("b1", "setenv bootargs ${bootargs_base} ${mtdparts1} ${rst2dfl_flg}; bootm ${img1_kernel}");
#endif /* #ifdef CONFIG_LUNA_MULTI_BOOT */
#elif defined (CONFIG_CMD_SPI_NAND)
	setenv("erase_cfgfs", "spi_nand erase ${fl_cfgfs} ${fl_cfgfs_sz}");
	setenv("erase_env", "spi_nand erase ${fl_env} ${fl_env_sz} && spi_nand erase ${fl_env2} ${fl_env_sz}");
	setenv("upb", "tftp ${freeAddr} encode_uboot.img && crc32 ${fileaddr} ${filesize} && spi_nand erase 0x0 ${fl_boot_sz} && spi_nand write.raw ${fileaddr} 0x0 ${filesize}");
	setenv("upk", "tftp ${freeAddr} uImage && crc32 ${fileaddr} ${filesize} && spi_nand erase ${fl_kernel1} ${fl_kernel1_sz} && spi_nand write ${fileaddr} ${fl_kernel1} ${filesize}");
	setenv("upr", "tftp ${freeAddr} rootfs && crc32 ${fileaddr} ${filesize} && spi_nand erase ${fl_rootfs1} ${fl_rootfs1_sz} && spi_nand write ${fileaddr} ${fl_rootfs1} ${filesize}");
	setenv("yu", "loady 80000000 && cp.b 80000000 81000000 ${filesize} && cmp.b 80000000 81000000 ${filesize} && spi_nand erase 0 ${filesize} && spi_nand write.raw 80000000 0 ${filesize}");
#ifdef CONFIG_LUNA_MULTI_BOOT
	setenv("upk1", "tftp ${freeAddr} uImage && crc32 ${fileaddr} ${filesize} && spi_nand erase ${fl_kernel2} ${fl_kernel2_sz} && spi_nand write ${fileaddr} ${fl_kernel2} ${filesize}");
	setenv("upr1", "tftp ${freeAddr} rootfs && crc32 ${fileaddr} ${filesize} && spi_nand erase ${fl_rootfs2} ${fl_rootfs2_sz} && spi_nand write ${fileaddr} ${fl_rootfs2} ${filesize}");
	setenv("bootargs_base", "console=ttyS0,115200");
	setenv("b0", "setenv bootargs ${bootargs_base} ${mtdparts0};spi_nand read ${freeAddr} ${fl_kernel1} ${fl_kernel1_sz};bootm ${freeAddr}");
	setenv("b1", "setenv bootargs ${bootargs_base} ${mtdparts1};spi_nand read ${freeAddr} ${fl_kernel2} ${fl_kernel2_sz};bootm ${freeAddr}");
#endif /* #ifdef CONFIG_LUNA_MULTI_BOOT */
#elif defined (CONFIG_CMD_ONFI)
	setenv("erase_cfgfs", "onfi erase ${fl_cfgfs} ${fl_cfgfs_sz}");
	setenv("erase_env", "onfi erase ${fl_env} ${fl_env_sz};onfi erase ${fl_env2} ${fl_env_sz}");
	setenv("upb", "tftp 0x81ff0000 encode_uboot.img;crc32 ${fileaddr} ${filesize};spi_nand erase 0x0 ${fl_boot_sz};onfi write.raw ${fileaddr} 0x0 ${filesize}");
	setenv("upk", "tftp 0x81ff0000 uImage;crc32 ${fileaddr} ${filesize};onfi erase ${fl_kernel1} ${fl_kernel1_sz};onfi write ${fileaddr} ${fl_kernel1} ${filesize}");
	setenv("upr", "tftp 0x81ff0000 rootfs;crc32 ${fileaddr} ${filesize};onfi erase ${fl_rootfs1} ${fl_rootfs1_sz};onfi write ${fileaddr} ${fl_rootfs1} ${filesize}");
	setenv("yu", "loady 80000000; cp.b 80000000 81000000 ${filesize}; cmp.b 80000000 81000000 ${filesize}; onfi erase 0 ${filesize}; onfi write.raw 80000000 0 ${filesize}");
#ifdef CONFIG_LUNA_MULTI_BOOT
	setenv("upk1", "tftp 0x81ff0000 uImage;crc32 ${fileaddr} ${filesize};onfi erase ${fl_kernel2} ${fl_kernel2_sz};onfi write ${fileaddr} ${fl_kernel2} ${filesize}");
	setenv("upr1", "tftp 0x81ff0000 rootfs;crc32 ${fileaddr} ${filesize};onfi erase ${fl_rootfs2} ${fl_rootfs2_sz};onfi write ${fileaddr} ${fl_rootfs2} ${filesize}");
	setenv("bootargs_base", "console=ttyS0,115200");
	setenv("b0", "setenv bootargs ${bootargs_base} ${mtdparts0};onfi read 0x82000000 ${fl_kernel1} ${fl_kernel1_sz};bootm 0x82000000");
	setenv("b1", "setenv bootargs ${bootargs_base} ${mtdparts1};onfi read 0x82000000 ${fl_kernel2} ${fl_kernel2_sz};bootm 0x82000000");
#endif /* #ifdef CONFIG_LUNA_MULTI_BOOT */
#endif
	return;
}
