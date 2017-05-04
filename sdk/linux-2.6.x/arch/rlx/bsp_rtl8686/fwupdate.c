#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/slab.h> 
#include <linux/unistd.h> 
#include <linux/sched.h> 
#include <linux/fs.h> 
#include <linux/file.h> 
#include <linux/mm.h> 
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/mtd/mtd.h>
#include <asm/uaccess.h> 

#define ST_IDLE		0
#define ST_WRITING	1
#define ST_UNLOCKED	2
#define MAGIC_KEY	"1321"

extern struct proc_dir_entry *realtek_proc;

typedef struct {
    unsigned int    key;            /* magic key */
#define BOOT_IMAGE             0xB0010001
#define CONFIG_IMAGE           0xCF010002
#define APPLICATION_IMAGE      0xA0000003
#define APPLICATION_UBOOT       0xA0000103      /*uboot only*/
#define APPLICATION_UIMAGE      0xA0000203      /*uimage only*/
#define APPLICATION_ROOTFS      0xA0000403      /*rootfs only*/
#define BOOTPTABLE             0xB0AB0004

    unsigned int    address;        /* image loading DRAM address */
    unsigned int    length;         /* image length */
    unsigned int    entry;          /* starting point of program */
    unsigned short  chksum;         /* chksum of */

    unsigned char   type;
#define KEEPHEADER    0x01   /* set save header to flash */
#define FLASHIMAGE    0x02   /* flash image */
#define COMPRESSHEADER    0x04       /* compress header */
#define MULTIHEADER       0x08       /* multiple image header */
#define IMAGEMATCH        0x10       /* match image name before upgrade */

    unsigned char      date[25];  /* sting format include 24 + null */
    unsigned char      version[16];
    unsigned int  *flashp;  /* pointer to flash address */

} IMGHDR;



static int state;
static int file_read(struct file *fp,char *buf,int len) {
	if (fp->f_op && fp->f_op->read)
		return fp->f_op->read(fp, buf, len, &fp->f_pos);
	else
		return -1;
}

// on success, this function will reboot system.
#define _NAME_ "FWUPDATE:"
static int fw_write(char *fwfile) {
	mm_segment_t oldfs;
	struct mtd_info *mtd;
	struct file *fp; 	
	struct kstat stat;
	struct erase_info ei;	
	int offset, part, fsize, imgFileSize;
	int nRead=0, nWritten, bChanged=0;
	loff_t pos;	
	u8 *buf;
	unsigned long flags;
	char *strtmp, *strmtd=0;	
	IMGHDR imghdr;
	
	extern void (*_machine_restart)(char *command);
	
	int rv = -EINVAL;
		
	oldfs = get_fs(); 
	set_fs(KERNEL_DS); 
	
	/*formt should be "offset;filename;part;imgFileSize" */
	do {
		char *pc, *s_offset;
		
		pc = strchr(fwfile, ';');
		if (!pc) goto ERROR;
		s_offset = fwfile;
		fwfile = &pc[1];
		*pc = '\0';
		offset = simple_strtol(s_offset, NULL, 0);
		
		// Mason Yu
		pc = strchr(fwfile, ';');
		if (!pc) goto ERROR;		
		s_offset = &pc[1];
		strtmp = &pc[1];
		*pc = '\0';
		part = simple_strtol(s_offset, NULL, 0);
		
		// Mason Yu
		pc = strchr(strtmp, ';');
		if (!pc) goto ERROR;
		s_offset = &pc[1];
		strtmp = &pc[1];
		*pc = '\0';
		imgFileSize = simple_strtol(s_offset, NULL, 0);
		
	} while (0);
	printk(_NAME_ "fwfile=%s, offset=%d, part=%d, imgFileSize=%d\n", fwfile, offset, part, imgFileSize);
	
	fsize = imgFileSize;
	
	fp = filp_open(fwfile, O_RDONLY, 0); 	
	//printk("%s(%d): fp=%p\n",__func__,__LINE__,fp);
	if(IS_ERR(fp))
		goto ERROR1;
	
	fp->f_pos = offset;
	
	local_irq_save(flags);
Stage_Read_Header:		
	nWritten = 0;
	nRead = file_read(fp, (char *)&imghdr, sizeof(imghdr));
	if (nRead != sizeof(imghdr)) {
		printk(_NAME_ "ERROR, cannot read header: %d\n", nRead);
		goto ERROR2;
	}
	
	switch (imghdr.key) {
	case APPLICATION_UBOOT:
		strmtd = "boot";
		fsize -= sizeof(IMGHDR);
		break;
	case APPLICATION_UIMAGE:
		strmtd = "linux";
		fsize -= sizeof(IMGHDR);
		break;
	case APPLICATION_ROOTFS:
		strmtd = "rootfs";
		fsize -= sizeof(IMGHDR);
		break;
	default:
		printk(_NAME_ "ERROR, unknown header %x\n", imghdr.key);
		if (bChanged)
			goto RESTART;
		else
			goto ERROR2;
	}
	
	if ((strmtd == 0) || (NULL == (mtd = get_mtd_device_nm(strmtd)))) {
		goto ERROR2;
	}
	printk(_NAME_" mtd=%s imglen=%u mtdsize=%llu fpos=%llu\n", strmtd, imghdr.length, mtd->size, fp->f_pos);
	
	if (imghdr.length > mtd->size) {
		printk(_NAME_ "ERROR, fw is larger than mtd(%s)\n", mtd->name);
		goto ERROR2;
	}
	
	buf = vmalloc(65536);
	if (!buf)
		goto ERROR2;
	
	printk(_NAME_ "fw ready to write, pos=%lld\n", fp->f_pos);
	/* start to write */	
	
	memset(&ei, 0, sizeof(ei));
	ei.mtd = mtd;
	ei.addr = 0;
	ei.len  = (imghdr.length + mtd->erasesize - 1) & ~(mtd->erasesize - 1);
	printk(_NAME_ "fw erase, addr=%llx, len=%llx , stat.size=%lld, mtd->erasesize=%x\n", ei.addr, ei.len, stat.size, mtd->erasesize);
	rv = mtd->erase(mtd, &ei);
	//printk("fw erase = %d\n", rv);
	if (rv) 
		goto ERROR3;
		
	pos = 0;	
	while (nWritten < imghdr.length) {
		size_t retlen;
		size_t rsize;
		
		nRead = (65536 >
			 (imghdr.length - nWritten)) ? (imghdr.length -
					       nWritten) : 65536;
		
		rsize = file_read(fp, buf, nRead);		
		
		printk(_NAME_ "writing, pos %llxh, size %xh\n", pos, rsize);
		rv = mtd->write(mtd, pos, rsize, &retlen, buf);
		if (rv || (rsize != retlen)) {
			printk(_NAME_ "rv=%d, rsize=%d, retlen=%d\n",rv,rsize,retlen);
		}
		pos += rsize;
		nWritten += rsize;
	} 
	bChanged = 1;
	fsize -= nWritten;
	if (fsize > sizeof(IMGHDR)) {
		put_mtd_device(mtd);
		printk(_NAME_ "Parsing next header. %d\n", fsize);
		goto Stage_Read_Header;
	}
	
	
RESTART:	
	printk("restart\n");
	if (_machine_restart)
		_machine_restart(NULL);
	
	rv = 0;
ERROR3:
	vfree(buf);
ERROR2:	
	filp_close(fp, NULL);	
	if (mtd)
		put_mtd_device(mtd);
ERROR1:		
ERROR:	
	set_fs(oldfs);
	local_irq_restore(flags);
	return rv;
}

static int fw_proc_read( char *buf, char **start, off_t offset, int count, int *eof ) {	
	switch(state) {
	case ST_IDLE: 	 return sprintf(buf, "idle"); 
	case ST_WRITING: return sprintf(buf, "writing"); 
	case ST_UNLOCKED:return sprintf(buf, "unlocked"); 
	}
	return 0;
}

static int fw_proc_write( struct file *file, const char __user *buffer, unsigned long count, void *data ) {
	char buf[128];
	if (copy_from_user(buf, buffer, sizeof(buf))) {
		return -EFAULT;
	}
	printk("%s(%d): buf = %s\n", __func__,__LINE__,buf);
	switch (state) {
	case ST_IDLE: 	 		
		if (!strncmp(buf, MAGIC_KEY, sizeof(MAGIC_KEY))) 
			state = ST_UNLOCKED;
		break;
	case ST_WRITING: 
		return -EFAULT;
	case ST_UNLOCKED:
		if (fw_write(buf))
			state = ST_IDLE;
		break;
	}
	
	return count;
}

static int __init fwupdate_init(void) {
	struct proc_dir_entry *pe;
	
	pe = create_proc_entry("fwupdate", S_IRUSR |S_IWUSR | S_IRGRP | S_IROTH, realtek_proc);
	if (!pe) {
		return -EINVAL;
	}
	
	pe->read_proc  = fw_proc_read;
	pe->write_proc = fw_proc_write;
	return 0;
}

static void __exit fwupdate_exit(void) {
}

module_init(fwupdate_init); 
module_exit(fwupdate_exit); 

MODULE_DESCRIPTION("FwUpdate"); 
MODULE_AUTHOR("Andrew Chang<yachang@realtek.com>"); 
MODULE_LICENSE("GPL"); 
