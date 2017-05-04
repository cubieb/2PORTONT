/*
 * Command for updating vm.img format image (containing uImage and rootfs) into corresponding partition on luna platform.
 * Author: bohungwu@realtek.com
 */

#include <common.h>
#include <malloc.h>

#include <asm/io.h>

//#define VMIMG_DEBUG

struct vmimg_hdr {
	u32 key;
	u32 load_addr;
	u32 img_sz;
	u32 entry_addr;
	u32 padding[11];
	u32 flash_base;
};

#ifdef VMIMG_DEBUG
static void dump_vmimg_hdr(struct vmimg_hdr *vmimg_hdr_p) {

	printf("key=0x%x\n", vmimg_hdr_p->key);
	printf("load_addr=0x%x\n", vmimg_hdr_p->load_addr);
	printf("img_sz=0x%x(%u)\n", vmimg_hdr_p->img_sz, vmimg_hdr_p->img_sz);
	printf("entry_addr=0x%x\n", vmimg_hdr_p->entry_addr);
	printf("flash_base=0x%x\n", vmimg_hdr_p->flash_base);
}
#else
#define dump_vmimg_hdr(x)
#endif

#define VMIMG_KERNEL_KEY (0xa0000203)
#define VMIMG_ROOTFS_KEY (0xa0000403)
#define CMD_BUF_SZ (256)
static int do_upvmimg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	char *endp;
	u8 *img;
	char cmd_buf[CMD_BUF_SZ] = {0};
	struct vmimg_hdr hdr;
	u32 part_base, part_sz;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	/* Retrieve vmimg location */
	img = (u8 *)simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;
	printf("vmimg is located at %p\n", img);

	/* Update kernel image */
	memcpy((void *)&hdr, (void *)img, sizeof(hdr));
	dump_vmimg_hdr(&hdr);
	if(hdr.key == VMIMG_KERNEL_KEY) {
		printf("\nKernel image key consistent, start updating...\n");
	} else {
		printf("\nError: kernel key mismatch, key found in image is 0x%08x, but should be 0x%08x\n", hdr.key, VMIMG_KERNEL_KEY);
		printf("Operation aborted\n");
		goto done;
	}
	part_base = simple_strtoul(getenv("fl_kernel1"), &endp, 16);
	part_sz = simple_strtoul(getenv("fl_kernel1_sz"), &endp, 16);
	printf("kernel partition at 0x%08x, size=0x%08x\n", part_base, part_sz);
	printf("kernel image size=0x%08x(%u)\n", hdr.img_sz, hdr.img_sz);
	sprintf(cmd_buf, "sf erase %x +%x;sf write %x %x %x", part_base, part_sz, (unsigned int)(img + sizeof(hdr)), part_base, hdr.img_sz);
	if(hdr.img_sz > part_sz) {
		printf("Error: image size is larger than the partition, operation aborted\n");
		goto done;
	}
	printf("CMD=%s\n", cmd_buf);
	run_command(cmd_buf, 0);

#ifdef CONFIG_LUNA_MULTI_BOOT
	/* Update 2nd kernel image */
	memcpy((void *)&hdr, (void *)img, sizeof(hdr));
	dump_vmimg_hdr(&hdr);
	if(hdr.key == VMIMG_KERNEL_KEY) {
		printf("\nKernel image key consistent, start updating...\n");
	} else {
		printf("\nError: kernel key mismatch, key found in image is 0x%08x, but should be 0x%08x\n", hdr.key, VMIMG_KERNEL_KEY);
		printf("Operation aborted\n");
		goto done;
	}
	part_base = simple_strtoul(getenv("fl_kernel2"), &endp, 16);
	part_sz = simple_strtoul(getenv("fl_kernel2_sz"), &endp, 16);
	printf("kernel partition at 0x%08x, size=0x%08x\n", part_base, part_sz);
	printf("kernel image size=0x%08x(%u)\n", hdr.img_sz, hdr.img_sz);
	sprintf(cmd_buf, "sf erase %x +%x;sf write %x %x %x", part_base, part_sz, (unsigned int)(img + sizeof(hdr)), part_base, hdr.img_sz);
	if(hdr.img_sz > part_sz) {
		printf("Error: image size is larger than the partition, operation aborted\n");
		goto done;
	}
	printf("CMD=%s\n", cmd_buf);
	run_command(cmd_buf, 0);

#endif


	/* Rootfs image */
	img = (u8 *)(img + hdr.img_sz + sizeof(struct vmimg_hdr));
	memcpy((void *)&hdr, (void *)img, sizeof(hdr));
	dump_vmimg_hdr(&hdr);
	if(hdr.key == VMIMG_ROOTFS_KEY) {
		printf("\nrootfs image key consistent, start updating...\n");
	} else {
		printf("\nError: rootfs key mismatch, key found in image is 0x%08x, but should be 0x%08x\n", hdr.key, VMIMG_ROOTFS_KEY);
		printf("Operation aborted\n");
		goto done;
	}
	part_base = simple_strtoul(getenv("fl_rootfs1"), &endp, 16);
	part_sz = simple_strtoul(getenv("fl_rootfs1_sz"), &endp, 16);
	printf("rootfs partition at 0x%08x, size=0x%08x\n", part_base, part_sz);
	printf("rootfs image size=0x%08x(%u)\n", hdr.img_sz, hdr.img_sz);
	sprintf(cmd_buf, "sf erase %x +%x;sf write %x %x %x", part_base, part_sz, (u32)(img + sizeof(hdr)), part_base, hdr.img_sz);
	if(hdr.img_sz > part_sz) {
		printf("Error: image size is larger than the partition, operation aborted\n");
		goto done;
	}
	printf("CMD=%s\n", cmd_buf);
	run_command(cmd_buf, 0);

#ifdef CONFIG_LUNA_MULTI_BOOT
	/* 2nd Rootfs image */

	memcpy((void *)&hdr, (void *)img, sizeof(hdr));
	dump_vmimg_hdr(&hdr);
	if(hdr.key == VMIMG_ROOTFS_KEY) {
		printf("\nrootfs image key consistent, start updating...\n");
	} else {
		printf("\nError: rootfs key mismatch, key found in image is 0x%08x, but should be 0x%08x\n", hdr.key, VMIMG_ROOTFS_KEY);
		printf("Operation aborted\n");
		goto done;
	}
	part_base = simple_strtoul(getenv("fl_rootfs2"), &endp, 16);
	part_sz = simple_strtoul(getenv("fl_rootfs2_sz"), &endp, 16);
	printf("rootfs partition at 0x%08x, size=0x%08x\n", part_base, part_sz);
	printf("rootfs image size=0x%08x(%u)\n", hdr.img_sz, hdr.img_sz);
	sprintf(cmd_buf, "sf erase %x +%x;sf write %x %x %x", part_base, part_sz, (u32)(img + sizeof(hdr)), part_base, hdr.img_sz);
	if(hdr.img_sz > part_sz) {
		printf("Error: image size is larger than the partition, operation aborted\n");
		goto done;
	}
	printf("CMD=%s\n", cmd_buf);
	run_command(cmd_buf, 0);

#endif


done:
	if (ret != -1)
		return ret;

usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	upvmimg,	2,	1,	do_upvmimg,
	"update kernel and rootfs by vmimg format on luna platform",
	"ADDRESS\n"
	"\n"
	"Description:\n"
	"Extract kernel and rootfs in vmimg image located at ADDRESS and write them into corresponding partitions.\n"
);
