/*
 * FILE NAME rtl_gpio.c
 *
 * BRIEF MODULE DESCRIPTION
 *  GPIO For Flash Reload Default
 *
 *  Author: jimmylin@realtek.com.tw
 *
 * Copyright 2005 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE	LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */



#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/reboot.h>
#include <linux/kmod.h>
#include <linux/proc_fs.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <linux/mtd/mtd.h>

// Mason Yu
#if defined(CONFIG_RTL8670)
#include "lx4180.h"
#elif defined(CONFIG_RTL8671)// 8671
#include "lx5280.h"
#else
//#include "platform.h"
/*linux-2.6.19*/
#include <bspchip.h> 
#endif
#include "gpio.h"
//#include "../../../../../config/autoconf.h"

//#define CONFIG_WIFI_SIMPLE_CONFIG

//#ifdef TR068_POWER_LED

#define PROBE_NULL	0
#define PROBE_ACTIVE	1
#define PROBE_RESET	2
#define PROBE_RELOAD	3


#ifdef CONFIG_WIRELESS_EXT
#ifndef CONFIG_WIFI_SIMPLE_CONFIG
void wps_led_set(int isOn) 
{
	return;
}
#else

#if (BOARD_TYPE == ALPHA_MODEL_WPS)
static void rtl8185_wps_led(int isOn) 
{
	unsigned char val;

	struct net_device *dev = dev_get_by_name(WLAN_NAME);
	if (0 == dev) {
		printk("dev %s not found\n", WLAN_NAME);
		return;
	}

	rtk8185_reg_write(dev, 2, 0x80, rtk8185_reg_read(dev, 2, 0x80) | (1 << 10));
	rtk8185_reg_write(dev, 2, 0x82, rtk8185_reg_read(dev, 2, 0x82) & (~(1 << 10)));
	rtk8185_reg_write(dev, 2, 0x84, rtk8185_reg_read(dev, 2, 0x84) | (1 << 10));

	
	val = rtk8185_reg_read(dev, 1, 0x91);
	if (isOn) {
		val = val | 0x02;
	} else {
		val = val & (~0x02);
	}
	rtk8185_reg_write(dev, 1, 0x91, val);
	
}
#endif  //ALPHA_MODEL_WPS
#endif // CONFIG_WIFI_SIMPLE_CONFIG

/****** <=====  HARDWARE DEPENDENT SECTION ******/


#endif
//#if defined(CONFIG_GPIO_LED_CHT_E8B)
#ifdef CONFIG_GPIO_LED_TR068

#include <linux/notifier.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include "led-generic.h"
#include "pushbutton.h"

// static int wan_ipif_counter = 0;
// Mason Yu. Add proc file for push button.
static char default_flag='0';
static char reboot_flag='0';

#ifdef CONFIG_WLAN_ON_OFF_BUTTON
//cathy
static char wlan_onoff_flag='0';
#endif

/*linux-2.6.19*/ 
void tr068_internet_led_start(void);
void tr068_internet_led_stop(void);

void tr068_internet_led(char state);
extern char adsl_showtime;

void internet_led_check(void) {
	struct net_device *dev;
	struct in_device *in_dev;

	read_lock(&dev_base_lock);
	/*linux-2.6.19*/
	//for (dev = dev_base; dev; dev = dev->next)
	for_each_netdev(&init_net, dev)
	{
		//printk("check(1) %s, %x, %x\n", dev->name, dev->flags, dev->priv_flags);
		if (!(dev->flags & IFF_UP))
			continue;
			
		if ((!(dev->priv_flags & IFF_DOMAIN_WAN)) && (!(dev->flags & IFF_POINTOPOINT)))
			continue;
			
		in_dev = __in_dev_get_rcu(dev);
		//printk("check(2) %s, %p\n", dev->name, in_dev);
		if (!in_dev)
			continue;

		for_ifa(in_dev) {
			// at least one WAN IP exist.
			if (adsl_showtime)
				tr068_internet_led('1');
			read_unlock(&dev_base_lock);
			return;
		} endfor_ifa(i_dev);
	}
	read_unlock(&dev_base_lock);
		tr068_internet_led('0');
}
	

static int if_inetaddr_event(struct notifier_block *this, unsigned long event, void *ifa)
{
	//printk("%s(%p, %d, %p)\n", __FUNCTION__, this, event, ifa);
	if (((struct net_device*)ifa)->priv_flags != IFF_DOMAIN_WLAN)
		internet_led_check();
	return NOTIFY_DONE;
}


static struct notifier_block if_addr_notifier = {
	.notifier_call = if_inetaddr_event,
};

static struct notifier_block if_dev_notifier = {
	.notifier_call = if_inetaddr_event,
};



static char power_flag='2';
static char internet_flag='0';

static int power_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", power_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}
static int power_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{	
	if (buffer && !copy_from_user(&power_flag, buffer, sizeof(power_flag))) {  
		switch(power_flag) {
		case '0':
			led_off(LED_POWER_RED);
			led_on(LED_POWER_GREEN);
			//led_power_red(0);
			//led_power_green(1);
			break;
		case '2':
			led_off(LED_POWER_GREEN);
			led_on(LED_POWER_RED);
			//led_power_green(0);
			//led_power_red(1);
			break;
		}
		return count;
	}
	return -EFAULT;
}

/*
extern int traffic_count;
void tr068_internet_traffic(void) {
	if (traffic_count < 10)
		traffic_count++;
}
*/

void tr068_internet_led(char state) {
	//if (internet_flag==state)
	//	return;	
	
	switch(state) {
	case '0':
		/*
		led_internet_red(0);
		led_internet_green(0);
		tr068_internet_led_stop();
		*/
		led_off(LED_INTERNET_GREEN);
		led_off(LED_INTERNET_RED);
		tr068_internet_led_stop();
		break;
	case '1':
		/*
		led_internet_green(1);
		led_internet_red(0);
		tr068_internet_led_start();
		*/
		led_on(LED_INTERNET_GREEN);
		led_off(LED_INTERNET_RED);
		tr068_internet_led_start();
		break;
	case '2':
		led_off(LED_INTERNET_GREEN);
		led_on(LED_INTERNET_RED);
		tr068_internet_led_stop();
		/*
		led_internet_green(0);
		led_internet_red(1);
		tr068_internet_led_stop();
		*/
		break;
	// test only
	/*
	case '3': 
		tr068_internet_traffic();
		printk("%u\n", traffic_count);
	*/

	default:
		return;
	}

	internet_flag = state;
}



static int internet_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", internet_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}
static int internet_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{	
	char input;
	if (buffer && !copy_from_user(&input, buffer, sizeof(input))) {  
		tr068_internet_led(input);
		return count;
	}
	return -EFAULT;
}

// Mason Yu. Add proc file for push button. Start.
static int default_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", default_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}

static int default_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
      if (count < 2)
         return -EFAULT;
      if (buffer && !copy_from_user(&default_flag, buffer, 1)) {
         return count;
         }
      return -EFAULT;
}
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
extern int test_drv_reset(void);
#endif
static int reboot_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", reboot_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}

static int reboot_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
      if (count < 2)
         return -EFAULT;
      if (buffer && !copy_from_user(&reboot_flag, buffer, 1)) {
         return count;
         }
      return -EFAULT;
}
// Mason Yu. Add proc file for push button. End.

#ifdef CONFIG_WLAN_ON_OFF_BUTTON
//cathy, for WIFISW button
static int wlan_onoff_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
	if (test_drv_reset()) {
		wlan_onoff_flag = '2';
	}
#endif      
      len = sprintf(page, "%c\n", wlan_onoff_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;

          return len;

}

static int wlan_onoff_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
      if (buffer && !copy_from_user(&wlan_onoff_flag, buffer, 1)) {
         return count;
      }
      return -EFAULT;
}
#endif

// Mason Yu. Add proc file for push button.
static void pb_reset_event(int event) {
	static unsigned int pb_counter = 0;
	static int pb_state = PROBE_NULL;

	switch(pb_state) {
	case PROBE_NULL:
		if (event) {
			pb_state = PROBE_ACTIVE;
			pb_counter ++;
		}
		break;
	case PROBE_ACTIVE:
		if (event) {
			pb_counter++;
			// Mason Yu
			/*
			if (pb_counter >= 5) {				
				kernel_restart(NULL);
			}
			*/			
		} else {
			// Mason Yu
			pb_state = PROBE_NULL;			
			if (pb_counter < 2)
			{				
				printk("Push Button do nothing.\n");			
			}
			else if (pb_counter >= 5)	
			{
				//reload default
			        default_flag='1';	
			        printk("Set default_flag = '1'\n");
				

			}			
			else//2<=probe_counter<5
			{				
				reboot_flag='1';
				printk("Set reboot_flag = '1'\n");				
			}
			pb_counter = 0;
		}
		break;
	}
}

#ifdef CONFIG_WLAN_ON_OFF_BUTTON
//cathy, if pressed less than 4 seconds, do on-off switch
static void pb_wlan_event(int event) {
	static unsigned int pb_counter = 0;
	static int pb_state = PROBE_NULL;

	switch(pb_state) {
	case PROBE_NULL:
		if (event) {
			pb_state = PROBE_ACTIVE;
			pb_counter ++;
		}
		break;
	case PROBE_ACTIVE:
		if (event) {
			pb_counter++;
		} else {
			pb_state = PROBE_NULL;			
			if (pb_counter < 4)
			{
				wlan_onoff_flag='1';
				printk("wifi on-off switch\n");			
			}
			pb_counter = 0;
		}
		break;
	}
}
#endif

static void rtl_gpio_timer(unsigned long data) {
	struct timer_list *timer = (struct timer_list *)data;
	pb_reset_event(  pb_is_pushed(PB_RESET) );
#ifdef CONFIG_WLAN_ON_OFF_BUTTON	
	pb_wlan_event(  pb_is_pushed(PB_WIFISW) );
#endif
	mod_timer(timer, jiffies + 100);
}

static struct timer_list probe_timer;

static int __init rtl_gpio_init(void)
{
	int ret = 0;
	struct proc_dir_entry *entry=NULL;
	
	printk("Realtek GPIO Driver for Flash Reload Default\n");

	register_inetaddr_notifier(&if_addr_notifier);
	register_netdevice_notifier(&if_dev_notifier);

	// diagnostic..
	//led_test();	

	entry = create_proc_entry("power_flag", 0644, NULL);
	if (entry) {
		entry->read_proc=power_read_proc;
		entry->write_proc=power_write_proc;
		/*linux-2.6.19*/ 
		//entry->owner 	  = NULL;
		entry->mode 	  = S_IFREG | S_IRUGO;
		entry->uid 	  = 0;
		entry->gid 	  = 0;
		entry->size 	  = 37;
	}	

	entry = create_proc_entry("internet_flag", 0644, NULL);
	if (entry) {
		entry->read_proc=internet_read_proc;
		entry->write_proc=internet_write_proc;
		/*linux-2.6.19*/ 
		//entry->owner 	  = NULL;
		entry->mode 	  = S_IFREG | S_IRUGO;
		entry->uid 	  = 0;
		entry->gid 	  = 0;
		entry->size 	  = 37;
	}

	/*
	entry = create_proc_entry("led", 0644, NULL);
		if (entry) {
			entry->read_proc=led_read_proc;
			entry->write_proc=led_write_proc;
			entry->owner	  = NULL;
			entry->mode 	  = S_IFREG | S_IRUGO;
			entry->uid	  = 0;
			entry->gid	  = 0;
			entry->size 	  = 37;
		}	
	*/
	
	// Mason Yu. Add proc file for push button. Start.
	entry = create_proc_entry("load_default", 0, NULL);
        if (entry) {
                entry->read_proc=default_read_proc;
	        entry->write_proc=default_write_proc;
	}

	entry = create_proc_entry("load_reboot", 0, NULL);
        if (entry) {
                entry->read_proc=reboot_read_proc;
	        entry->write_proc=reboot_write_proc;
	}
	// Mason Yu. Add proc file for push button. End.
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
	//cathy, add for WIFISW button
	entry = create_proc_entry("wlan_onoff", 0, NULL);
        if (entry) {
                entry->read_proc=wlan_onoff_read_proc;
	        entry->write_proc=wlan_onoff_write_proc;
	}
#endif	
	//led_tr068_init();
	init_timer (&probe_timer);
	probe_timer.expires = jiffies + 100;
	probe_timer.data = (unsigned long)&probe_timer;
	probe_timer.function = &rtl_gpio_timer;
	mod_timer(&probe_timer, jiffies + 100);

cleanup:

	return ret;
}


#ifdef CONFIG_HTTP_FILE
#define debug(fmt,...)  //printk(fmt, ## __VA_ARGS__)
#define ASSERT(s) if (!(s)) printk("%s(%d): ASSERT failed\n",__FUNCTION__,__LINE__)
#define MIN(a, b)  ((a) > (b)) ? (b) : (a)
#define FW_BLK_SIZE 4096
#define FW_TMP_SIZE 0x100000
#define FW_IMG_SIZE 0x400000

struct fw_mem_block {
	u32		blk_max_num;	/* num of elements allocated */
	u32		blk_size;	/* current data size */
	u32		blk_truesize; /* actual allocated size */
	u8** 	blocks;		/* element array */
};

struct fw_mtd_block {
	int		mtd_fd;
	u32		mtd_size;	/* current data size */
	u32		mtd_truesize; /* actual avail size */
	u32		mtd_offset;
	const char * mtd_file;
};

#define DESC_TYPE_NONE 0
#define DESC_TYPE_MEM 1
#define DESC_TYPE_MTD 2


struct block_desc {
	u8	type;

	union {
		struct fw_mem_block mem;
		struct fw_mtd_block mtd;
	} u;
};


#define BUF1_SIZE (512 * 1024 / FW_BLK_SIZE)
#define BUF1_SIZE (512 * 1024 / FW_BLK_SIZE)
#define NUM_OF_DESC 3

struct fw_sb {
	u32	fw_idx;
	u32 fw_size;
	struct proc_dir_entry *fw_file;
	struct block_desc descs[NUM_OF_DESC];
};

static struct fw_sb fw_super;

static inline void mem_block_reset(struct fw_mem_block *fw) {
	size_t size;
	
	size = fw->blk_max_num * sizeof(u8 *);
	fw->blk_size    = 0;
	fw->blk_truesize = 0;
	memset(fw->blocks, 0, size);
}

static int fw_block_init(struct fw_mem_block *fw, u32 max_block) {
	size_t size;
	
	size = max_block * sizeof(u8 *);

	fw->blocks = kmalloc(size, GFP_KERNEL);
	if (fw->blocks) {
		fw->blk_max_num = max_block;
		mem_block_reset(fw);				
	} else {
		printk("%s(%d):not enough memory\n", __FUNCTION__,__LINE__);
		return -1;
	}

	return 0;
};


static unsigned int fw_block_release(struct fw_mem_block * fw) {
	u32 idx;

	for (idx=0;idx < fw->blk_truesize/FW_BLK_SIZE;idx++) {
		debug("%s(%d): free %p\n",__FUNCTION__,__LINE__,fw->blocks[idx]);
		kfree(fw->blocks[idx]);		
	}
	
	mem_block_reset(fw);
	
	debug("%s(%d): %d blks freed\n",__FUNCTION__,__LINE__,idx);
	return idx;
}


/* copy <len> bytes to userspace <buffer> from <offset> location in <fw>

	return number of bytes read.
*/
static ssize_t
fw_block_read(struct fw_mem_block * fw, const char __user *buffer, size_t len, loff_t offset) 
{
	u32 num_to_copy, num_copied = 0;

	num_to_copy = MIN(fw->blk_size-offset, len);

	while (num_to_copy) {
		u32 blk_id, blk_off, n;
		int ret; 
		u8 *where_to_read;

		blk_id = offset / FW_BLK_SIZE;
		blk_off = offset % FW_BLK_SIZE;

		n = MIN(num_to_copy, FW_BLK_SIZE - blk_off);

		ASSERT(fw->blocks[blk_id]!= 0);
		ASSERT(blk_id < fw->blk_max_num);
		where_to_read = fw->blocks[blk_id] + blk_off;
		debug("%s(%d): read %x bytes from %p to %p\n",__FUNCTION__,__LINE__,n,where_to_read,&buffer[num_copied]);
		ret = copy_to_user((void __user *)&buffer[num_copied], (void *)where_to_read, n);
		if (ret) {
			printk("%s(%d): read fail %d\n", __FUNCTION__,__LINE__,ret);
			return 0;
		}

		num_to_copy -= n;
		offset += n;
		num_copied += n;
	}	

	return num_copied;
}


/* copy <len> bytes from userspace <buffer> to <offset> location in <fw>

	return number of bytes written.
*/
static ssize_t
fw_block_write(struct fw_mem_block * fw, const char __user *buffer, size_t len, loff_t off) 
{
	u32 num_to_copy, num_copied = 0;
	loff_t offset = off;

	debug("%s(%d): len=%x off=%llx\n",__FUNCTION__,__LINE__,len,off);
	/* determine if we expand */	
	if ((off + len ) > fw->blk_truesize) {
		int num_to_expand, idx;
		num_to_expand = ((offset + len ) - fw->blk_truesize + (FW_BLK_SIZE-1)) / FW_BLK_SIZE;
		idx = fw->blk_truesize / FW_BLK_SIZE;		
		while(num_to_expand && (idx < fw->blk_max_num)) {
			if (0 == (fw->blocks[idx] = kmalloc(FW_BLK_SIZE, GFP_KERNEL))) {
				printk("%s(%d): no more mem\n", __FUNCTION__, __LINE__);
				return 0;
			}
			debug("%s(%d): idx=%x mem=%p\n",__FUNCTION__,__LINE__,idx,fw->blocks[idx]);
			num_to_expand--;
			idx++;
			fw->blk_truesize += FW_BLK_SIZE;
		}
		debug("%s(%d): now %d block, truesize=%x\n",__FUNCTION__,__LINE__,num_to_expand,fw->blk_truesize);
	}

	num_to_copy = MIN(fw->blk_truesize-off, len);

	while (num_to_copy) {
		u32 blk_id, blk_off, n;
		int ret; 
		u8 *where_to_write;

		blk_id = offset / FW_BLK_SIZE;
		blk_off = offset % FW_BLK_SIZE;

		n = MIN(num_to_copy, FW_BLK_SIZE - blk_off);

		where_to_write = fw->blocks[blk_id] + blk_off;

		ASSERT(fw->blocks[blk_id]!= 0);
		ASSERT(blk_id < fw->blk_max_num);
		
		debug("%s(%d): write %x bytes from %p to %p\n",__FUNCTION__,__LINE__,n,&buffer[num_copied],where_to_write);		
		ret = copy_from_user((void *)where_to_write, (void __user *)&buffer[num_copied], n);
		if (ret) {
			printk("%s(%d): write fail %d, size=%x (id=%x,off=%x)\n", __FUNCTION__,__LINE__,
				ret,n,blk_id,blk_off);
			return 0;
		}
		/* for debug
		if (blk_id==0) {
			u8 tmpbuf[256];
			printk("%s(%d): \n", __FUNCTION__,__LINE__);
			memDump(fw->blocks[blk_id],256,"mem");					

			copy_from_user(tmpbuf, buffer, sizeof(tmpbuf));
			memDump(tmpbuf, sizeof(tmpbuf), "tmpbuf");
		} */

		num_to_copy -= n;
		offset += n;
		num_copied += n;
	}	

	if ((len+off) > fw->blk_size) {
		fw->blk_size = len+off;
	}
	debug("%s(%d): len=%x wrote\n",__FUNCTION__,__LINE__,num_copied);
	return num_copied;
}

static inline size_t get_size(struct block_desc *desc) {
	switch (desc->type) {
	case DESC_TYPE_MEM:
		return desc->u.mem.blk_size;

	case DESC_TYPE_MTD:
		return desc->u.mtd.mtd_size;

	default:
		return 0;
	}
};


static u32 fw_size(struct fw_sb *sb) {
	u32 size=0;
	int idx;
	for (idx = 0; idx < NUM_OF_DESC; idx++) {
		size += get_size(&sb->descs[idx]);
	}
	return size;	
}


static int fw_open (struct inode *node, struct file *file) {	
	struct fw_sb *sb = &fw_super;
	u32 size;
	
	printk("%s(%d): priv=%p\n",__FUNCTION__,__LINE__, file->private_data);

	size = fw_size(sb);
	
	return generic_file_open(node, file);
}


static int fw_release(struct inode *inode, struct file *file) {	
	return 0;
}



static inline size_t get_maxsize(struct block_desc *desc) {
	size_t size;
	switch (desc->type) {
	case DESC_TYPE_MEM:		
		size = desc->u.mem.blk_max_num * FW_BLK_SIZE;
		//printk("%s(%d): size=(%d)%x\n",__FUNCTION__,__LINE__,desc->u.mem.blk_max_num,size);
		return size;

	case DESC_TYPE_MTD:
		return desc->u.mtd.mtd_truesize;

	default:
		return 0;
	}
};



/* return the idx of desc, -1 if not in range */
static inline int get_idx(struct fw_sb *sb, loff_t off) {
	int idx;
	loff_t head = 0;
	debug("%s(%d): off=%llx\n", __FUNCTION__,__LINE__,off);
	for (idx = 0; idx < NUM_OF_DESC; idx++) {
		struct block_desc *desc =&sb->descs[idx];
		size_t /*datasize,*/ max_size;

		
		max_size = get_maxsize(desc);
		debug("%s(%d): idx=%d max_size=%u\n", __FUNCTION__,__LINE__,idx,max_size);
		if ((off >= head) && (off < (head+max_size))) {
			return idx;
		}

		head += max_size; 
	}
	return -1;
}


static ssize_t 
fw_read(struct file *file, char *buffer,size_t len,loff_t * off)
{
	struct fw_sb *sb = &fw_super;// (struct fw_sb *)file->private_data;
	int idx, s_idx;
	ssize_t num_to_read;
	u32 head = 0;
	ssize_t num_read;	
	struct block_desc *desc;
	size_t size;
	mm_segment_t  old_fs;
	loff_t where_to_read;
	long result1,result4;
	off_t result2;
	
	debug("%s(%d): buffer=%p len=%x,off=%llx\n", __FUNCTION__,__LINE__,buffer,len,(*off));

	idx = s_idx = get_idx(sb, *off);

	debug("%s(%d): s_idx=%d\n", __FUNCTION__,__LINE__,s_idx);
	
	for(idx=0;idx<s_idx;idx++) {
		struct block_desc *desc =&sb->descs[idx];
		head += get_maxsize(desc);
	}	
	
	{
		desc =&sb->descs[idx];			
		size     = get_size(desc);
		num_to_read = MIN(len, size - (*off - head));
		where_to_read = *off - head;

		debug("%s(%d): wheretor=%llx,numtor=%x,size=%x\n", __FUNCTION__,__LINE__,
			where_to_read,num_to_read,size);
				
		switch (desc->type) {
		case DESC_TYPE_MEM:
			num_read = fw_block_read(&desc->u.mem, buffer, num_to_read, where_to_read);			
			break;
			
		case DESC_TYPE_MTD:
			old_fs = get_fs();
			set_fs(KERNEL_DS);

			if (
				((result1 = desc->u.mtd.mtd_fd = sys_open(desc->u.mtd.mtd_file, O_RDWR, 0)) > 0) &&
				((result2 = sys_lseek(desc->u.mtd.mtd_fd, where_to_read + desc->u.mtd.mtd_offset, SEEK_SET)) == (where_to_read + desc->u.mtd.mtd_offset)) &&
				((num_read = sys_read(desc->u.mtd.mtd_fd, buffer,num_to_read)) >= 0) &&
				((result4 = sys_close(desc->u.mtd.mtd_fd)) == 0)
			) {
				debug("%s(%d): %s flash seek %lx read %x bytes\n",__FUNCTION__,__LINE__,
					desc->u.mtd.mtd_file,result2,num_read);
			} else {
				printk("%s(%d): error(%ld,%llx,%ld,%ld)\n", __FUNCTION__,__LINE__,result1,result2,num_read,result4);
			}			
			
			set_fs(old_fs);
			break;
		default:
			num_read = -EINVAL;
			break;
			
		}
			
		
	}

	debug("%s(%d):%x read\n", __FUNCTION__,__LINE__,num_read);
	if (num_read > 0) {
		*off += num_read;
	}
	return num_read;


}

static inline int erase_file(struct fw_mtd_block *mtd, int fd) {
	struct erase_info_user arge;
	if (0 == mtd->mtd_size) {
		arge.start = mtd->mtd_offset;
		arge.length = mtd->mtd_truesize;
		return sys_ioctl(fd,MEMERASE,&arge);
	}
	return 0;
}

static ssize_t
fw_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
	struct fw_sb *sb = &fw_super;//(struct fw_sb *)file->private_data;
	int idx, s_idx;
	u32 num_to_write;
	u32 head = 0;
	mm_segment_t  old_fs;
	ssize_t num_write;
	long where_to_write;
	u32 max_size, size;
	struct block_desc *desc;
	long result1,result2,result4;	

	debug("%s(%d): buffer=%p len=%x,off=%llx\n", __FUNCTION__,__LINE__,buffer,len,(*off));
	
	s_idx = get_idx(sb, *off);

	debug("%s(%d): s_idx=%d\n", __FUNCTION__,__LINE__,s_idx);
	
	for(idx=0;idx<s_idx;idx++) {
		struct block_desc *desc =&sb->descs[idx];
		head += get_maxsize(desc);
	}

	
	{
		desc =&sb->descs[idx];
			
		max_size = get_maxsize(desc); 
		size     = get_size(desc);		
		num_to_write = MIN(len, max_size - size);		
		where_to_write = *off - head;

		debug("%s(%d): wheretow=%lx,numtow=%x max=%x,size=%x\n", __FUNCTION__,__LINE__,
			where_to_write,num_to_write,max_size,size);
		
		switch (desc->type) {
		case DESC_TYPE_MEM:
			num_write = fw_block_write(&desc->u.mem, buffer, num_to_write, where_to_write);						
			break;
			
		case DESC_TYPE_MTD:			

			where_to_write += desc->u.mtd.mtd_offset;
			old_fs = get_fs();
			set_fs(KERNEL_DS);

			if (
				(result1 = (desc->u.mtd.mtd_fd = sys_open(desc->u.mtd.mtd_file, O_RDWR, 0)) > 0) &&				
				(erase_file(&desc->u.mtd, desc->u.mtd.mtd_fd) == 0) &&
				((result2 = sys_lseek(desc->u.mtd.mtd_fd, where_to_write, SEEK_SET)) == where_to_write) &&
				((num_write = sys_write(desc->u.mtd.mtd_fd,(void __user *)buffer,num_to_write)) >= 0) &&
				((result4 = sys_close(desc->u.mtd.mtd_fd)) == 0)
			
			) {												
				desc->u.mtd.mtd_size += num_write;					
			} else {
				printk("%s(%d): error(%ld,%ld,%ld,%ld)\n", __FUNCTION__,__LINE__,result1,result2,num_write,result4);
			}
						
			set_fs(old_fs);			
			break;
		default:
			num_write = -EINVAL;
			break;
			
		}

		debug("%s(%d): head=%x, to_write=%x,this_write=%x\n", __FUNCTION__,__LINE__,
			head,num_to_write,num_write);
		

	}

	
	//printk("%s(%d): buffer=%p len=%x,off=%llx,num_write=%dd\n", __FUNCTION__,__LINE__,buffer,len,(*off),num_write);
	if (num_write > 0) {
		*off += num_write;
	}
	
	//printk("%s(%d): %p\n",__FUNCTION__,__LINE__,sb->fw_file);
	//sb->fw_file->size = fw_size(sb);
	return num_write;	
}

static loff_t fw_llseek (struct file *f, loff_t offset, int origin) {
	loff_t retval;
	struct fw_sb *sb = &fw_super;
	//loff_t _offset = offset;		
	switch(origin) {
	case SEEK_END:
		offset += fw_size(sb);
		break;
	case SEEK_CUR:
		offset += f->f_pos;
		break;
	}

	retval = -EINVAL;
	if ((offset >=0) && (offset <= fw_size(sb))) {
		if (offset != f->f_pos) {
			f->f_pos = offset;
			f->f_version = 0;
		}
		
		retval = offset;		
	}
	debug("%s(%d): orig=%d, off(%lld) ret:%lld pos=%lld\n",__FUNCTION__,__LINE__,origin,offset,retval,f->f_pos);
	return retval;
}


static int 
fw_permission (struct inode *node, int op, struct nameidata *ni) {
	
	printk("%s(%d): %x  uid=%d\n",__FUNCTION__,__LINE__, op, current->euid);	
	if (op == 4 || (op == 2 && current->euid == 0))
		return 0;

	/* 
	 * If it's anything else, access is denied 
	 */
	return -EACCES;
}

static int fw_getattr (struct vfsmount *mnt, struct dentry *dentry, struct kstat *stat) {
	struct inode *inode = dentry->d_inode;
	loff_t size = 0;
	int idx;
	struct fw_sb *sb = &fw_super;//(struct fw_sb *)file->private_data;
		
	generic_fillattr(inode, stat);

	for (idx = 0; idx < NUM_OF_DESC; idx++) {
		struct block_desc *desc = &sb->descs[idx];	
		size += get_size(desc);
		debug("%s(%d):idx%d size=%lx\n",__FUNCTION__,__LINE__, idx, get_size(desc));
	}
	stat->size = size; 	
	return 0;
}

static int 
fw_unlink(struct inode *node,struct dentry *entry) 
{
	struct fw_sb *sb = &fw_super;//(struct fw_sb *)file->private_data;
	int idx;
	debug("%s(%d):\n",__FUNCTION__,__LINE__);

	for (idx = 0; idx < NUM_OF_DESC; idx++) {
		struct block_desc *desc = &sb->descs[idx];	
		if (DESC_TYPE_MEM==desc->type) {

			fw_block_release(&desc->u.mem);
		} else if (DESC_TYPE_MTD==desc->type) {
			desc->u.mtd.mtd_size = 0;
		}
	}
	//sb->fw_file->size = 0;
	//printk("%s(%d): %d freed\n", __FUNCTION__,__LINE__,idx);
	return 0;
}

static struct inode_operations fw_inode_ops,fw_file_inode_ops;
static struct file_operations fw_file_ops;


void setup_http_file_flash(u32 flashsize) {
	printk("HTTP_FILE: flash size 0x%x\n", flashsize);
	fw_super.descs[1].type = DESC_TYPE_MTD;
	fw_super.descs[1].u.mtd.mtd_file = "/dev/mtd2";
	fw_super.descs[1].u.mtd.mtd_offset  = flashsize - FW_TMP_SIZE;
	fw_super.descs[1].u.mtd.mtd_truesize= FW_TMP_SIZE;

}

static int __init rtk_fw_init(void)
{
	struct proc_dir_entry *entry=NULL;
	struct proc_dir_entry *priv=NULL;

	priv = proc_mkdir("rtk", NULL);
	if (priv) {
		fw_inode_ops = *(priv->proc_iops);
		fw_inode_ops.unlink = fw_unlink;
		//fw_inode_ops.getattr = fw_getattr;
		priv->proc_iops = &fw_inode_ops;
		priv->mode	|= S_IRWXUGO;
	} else {
		goto ERR1;
	}
		
	entry = create_proc_entry("http_buf", 0644, priv);
	if (entry) {
	
		fw_file_ops = *(entry->proc_fops);
		fw_file_inode_ops = *(priv->proc_iops);
		//fw_file_ops.open = fw_open;
		//fw_file_ops.release = fw_release;
		fw_file_ops.read = fw_read;
		fw_file_ops.write = fw_write;
		fw_file_ops.llseek = fw_llseek;

		fw_file_inode_ops.getattr = fw_getattr;
		entry->proc_fops = &fw_file_ops;	
		entry->proc_iops = &fw_file_inode_ops;
		//entry->proc_iops = &fw_inode_ops;
		//entry->owner  = THIS_MODULE;
		//entry->mode   = S_IFREG | S_IRUGO | S_IWUSR;
		entry->mode   |= S_IRWXUGO;
		//entry->uid 	  = 0;
		//entry->gid 	  = 0;
		entry->size   = 0;
		
	} else {
		goto ERR2;
	}

	memset(&fw_super, 0, sizeof(fw_super));
	fw_super.fw_file = entry;
	fw_super.descs[0].type = DESC_TYPE_MEM;
	fw_block_init(&fw_super.descs[0].u.mem, 512 * 1024 / FW_BLK_SIZE);

	//fw_super.descs[1].type = DESC_TYPE_MTD;
	//fw_super.descs[1].u.mtd.mtd_file = "/dev/mtd2";
	//fw_super.descs[1].u.mtd.mtd_offset  = 0x280000;
	//fw_super.descs[1].u.mtd.mtd_truesize= 0x3e0000 - fw_super.descs[1].u.mtd.mtd_offset;

	fw_super.descs[2].type = DESC_TYPE_MEM;
	fw_block_init(&fw_super.descs[2].u.mem, 
		(FW_IMG_SIZE - fw_super.descs[0].u.mem.blk_truesize - fw_super.descs[1].u.mtd.mtd_truesize) / FW_BLK_SIZE);
	return 0;
ERR2:
	
ERR1:
	return -EINVAL;
}

module_init(rtk_fw_init);

#endif

static void __exit rtl_gpio_exit(void)
{
	printk("Unload Realtek GPIO Driver \n");
	//del_timer_sync(&probe_timer);
}


module_init(rtl_gpio_init);
module_exit(rtl_gpio_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");

#endif

