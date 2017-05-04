/*
 * luna kernel soc parameters & mtd partition preparation
 * Author: bohungwu@realtek.com.tw
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <asm/bootinfo.h>

#include "prom.h"
#include <soc.h>


extern void prom_printf(char *fmt, ...);

#define DEBUG_PRINTK 0
 
parameter_to_bootloader_t kernel_soc_parameters;
#define plr_param_soc (kernel_soc_parameters.soc)

extern char arcs_cmdline[CL_SIZE];

#define GB_BITS (30)
#define MB_BITS (20)
#define KB_BITS (10)
#define GB_MASK ((0x1<<GB_BITS)-1)
#define MB_MASK ((0x1<<MB_BITS)-1)
#define KB_MASK ((0x1<<KB_BITS)-1)
__init char * to_KMGB(u32 val, char *output_p, u32 output_size) {
	if((val & GB_MASK) == 0) { /* To GB */
		snprintf(output_p, output_size, "%uG", (val >> GB_BITS));
	} else if((val & MB_MASK) == 0) {/* To MB */
		snprintf(output_p, output_size, "%uM", (val >> MB_BITS));
	} else if((val & KB_MASK) == 0) { /* To KB */
		snprintf(output_p, output_size, "%uK", (val >> KB_BITS));
	} else {
		snprintf(output_p, output_size, "%u", (val));
	}
	return output_p;
	
}

typedef struct {
	char* name;
	u32 base;
	u32 size;
} part_info_t;

__initdata static part_info_t part_info[] = {
	{"CONFIG",  0, 0}, /* opt1    */
	{"CS",      0, 0}, /* opt2    */
	{"TR069",   0, 0}, /* opt3    */
	{"UNUSED0", 0, 0}, /* opt4    */
	{"LINUX",   0, 0}, /* kernel1 */
	{"UNUSED1", 0, 0}, /* kernel2 */
	{"ROOTFS",  0, 0}, /* rootfs1 */
	{"UNUSED2", 0, 0}, /* rootfs2 */
	{"END",     0, 0}, /* end     */ 
	{"BOOT",    0, 0}, /* BOOTLOADER partition */
};

__initdata static part_info_t *pi_ptr[ARRAY_SIZE(part_info)];

#define TS_SIZE (64) /* Magic number */
__init void flash_layout_to_mtdparts(char *mtdparts_output) {
	char tmp_str[TS_SIZE] = {0};
	char tmp_str0[TS_SIZE] = {0};
	char tmp_str1[TS_SIZE] = {0};
	u32 i;

	snprintf(mtdparts_output, CL_SIZE, " mtdparts=%s:", RTK_MTD_DEV_NAME);
#if 0
	for(i = 0; i < ARRAY_SIZE(part_info); i++) {
		if(part_info[i].size != 0) {
			if(first_part == 0) {
				strlcat(mtdparts_output, ",", (CL_SIZE - strlen(mtdparts_output)));
			}
			snprintf(tmp_str, CL_SIZE, "%s@%s(%s)", to_KMGB(part_info[i].size, tmp_str0, TS_SIZE), to_KMGB(part_info[i].base, tmp_str1, TS_SIZE), part_info[i].name);
			strlcat(mtdparts_output, tmp_str, (CL_SIZE - strlen(mtdparts_output)));
			first_part = 0;
		}
	}
#endif 
	for(i = 0; i < ARRAY_SIZE(part_info); i++) {
		if(pi_ptr[i]->size != 0) {
			snprintf(tmp_str, CL_SIZE, "%s@%s(%s),", to_KMGB(pi_ptr[i]->size, tmp_str0, TS_SIZE), to_KMGB(pi_ptr[i]->base, tmp_str1, TS_SIZE), pi_ptr[i]->name);
			strlcat(mtdparts_output, tmp_str, (CL_SIZE - strlen(mtdparts_output)));
		}
	}

	/* Replace the last ',' (comma sign) by a null character '\0' */
	if(mtdparts_output[strlen(mtdparts_output) - 1] == ',') { 
		mtdparts_output[strlen(mtdparts_output) - 1] = '\0';
	}

}

__init void sort_part_info(part_info_t *input_pi_ptr[], u32 num) {
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


__init u32 otto_get_flash_size(void) {
	u8 nc, spc;
	u32 ret_val;
#ifdef CONFIG_MTD_NAND_RTK

	ret_val=plr_param_soc.flash_info.page_size * plr_param_soc.flash_info.num_page_per_block*plr_param_soc.flash_info.num_block;

#else
	nc  = plr_param_soc.flash_info.num_chips;
	spc = plr_param_soc.flash_info.size_per_chip;
	//printk("nc=%u, spc=%u\n", nc, spc);
	ret_val = nc * (0x1 << spc);
#endif
	return ret_val;
}

__init void part_info_init(void) {
	u32 i;

	part_info[0].base = plr_param_soc.layout.opt1_addr;
	part_info[1].base = plr_param_soc.layout.opt2_addr;
	part_info[2].base = plr_param_soc.layout.opt3_addr;
	part_info[3].base = plr_param_soc.layout.opt4_addr;
	part_info[4].base = plr_param_soc.layout.kernel1_addr;
	part_info[5].base = plr_param_soc.layout.kernel2_addr;
	part_info[6].base = plr_param_soc.layout.rootfs1_addr;
	part_info[7].base = plr_param_soc.layout.rootfs2_addr;
	if(plr_param_soc.layout.end_addr == 0) {
		//plr_param_soc.layout.end_addr = otto_get_spi_flash_size(); //This failed as soc_t is declarded as const
		printk("INFO: layout.end_addr is 0, fill flash size to the base of end partition\n");
		part_info[8].base = otto_get_flash_size();
	} else {
		part_info[8].base = plr_param_soc.layout.end_addr;
	}

	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		pi_ptr[i] = &part_info[i];
	}
	printk("Original setting\n");
	printk("pi_ptr: name\tbase\tsize\n");
	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		printk("[%d]: %s\t0x%08x\t0x%08x\n", i, pi_ptr[i]->name, pi_ptr[i]->base, pi_ptr[i]->size);
	}

	sort_part_info(pi_ptr, ARRAY_SIZE(part_info));
	printk("After sorting\n");
	printk("pi_ptr: name\tbase\tsize\n");
	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		printk("[%d]: %s\t0x%08x\t0x%08x\n", i, pi_ptr[i]->name, pi_ptr[i]->base, pi_ptr[i]->size);
	}

	/* Fill size */
	for(i=0; i<(ARRAY_SIZE(part_info)-1); i++) {
		pi_ptr[i]->size = pi_ptr[i+1]->base - pi_ptr[i]->base;
	}
	printk("After filling size\n");
	printk("pi_ptr: name\tbase\tsize\n");
	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		printk("[%d]: %s\t0x%08x\t0x%08x\n", i, pi_ptr[i]->name, pi_ptr[i]->base, pi_ptr[i]->size);
	}

	printk("Valid partitions\n");
	printk("part_info: name\tbase\tsize\n");
	for(i=0; i<ARRAY_SIZE(part_info); i++) {
		if(part_info[i].size !=0) {
			printk("[%d]: %s\t0x%08x\t0x%08x\n", i, part_info[i].name, part_info[i].base, part_info[i].size);
		}
	}
}

__init void preloader_parameters_init(void) {
	char str_tmp[CL_SIZE] = {0};

	printk("Compiled soc.h version=0x%08x\n", SOC_HEADER_VERSION);
	printk("sram_parameters.soc.header_ver=0x%08x\n", sram_parameters.soc.header_ver);
	if(sram_parameters.soc.header_ver != SOC_HEADER_VERSION) {
		prom_printf("==================================================================================\n");
		prom_printf("FATAL ERROR: soc.h version mismatch!\n");
		prom_printf("Kernel uses 0x%08x but Preloader adopts 0x%08x\n", SOC_HEADER_VERSION, sram_parameters.soc.header_ver);
		prom_printf("Please align soc.h version between Kernel and Preloader.\n");
		prom_printf("Or disable CONFIG_USE_PRELOADER_PARAMETERS in menuconfig under root directory.\n");
		prom_printf("The system is blocked at file %s:line %u!\n", __FILE__, __LINE__);
		prom_printf("==================================================================================\n");
		while(1);
	}

	memcpy((void *)&kernel_soc_parameters, (void *)&sram_parameters, sizeof(kernel_soc_parameters));

	/* Clear parameters/function ptrs which need to re-filled insdie kernel */
	memset(((void *)&(kernel_soc_parameters.dram_init_result)), 0x0, sizeof(kernel_soc_parameters) - ((void *)&(kernel_soc_parameters.dram_init_result) - (void *)&kernel_soc_parameters)); 

	/* Clear SRAM to avoid deceptive execution results */
	memset( (void *)SRAM_BASE, 0xFF, SRAM_SIZE);

	if(strstr(arcs_cmdline, "mtdparts") == NULL) {
		part_info_init();
		flash_layout_to_mtdparts(str_tmp);
		printk("Append %s to kernel command line\n", str_tmp);
		strlcat(arcs_cmdline, str_tmp, CL_SIZE);
	} else {
		printk("Kernel command line has mtdparts, ignoring appending %s from soc parameters\n", str_tmp);
	}
	printk("Kernel arcs_cmdline=%s\n", arcs_cmdline);

	return;
}



