#include <linux/module.h> 
#include <linux/init.h> 
//#include <linux/slab.h> 
#include <linux/unistd.h> 
//#include <linux/sched.h> 
#include <linux/fs.h> 
#include <linux/file.h> 
#include <linux/proc_fs.h>
//#include <asm/uaccess.h> 
#include <linux/netdevice.h>


static int netdev_flag_proc_read( char *buf, char **start, off_t offset, int count, int *eof ) 
{
	return 0;
}

static int netdev_flag_proc_write( struct file *file, const char __user *buffer, unsigned long count, void *data )
{
	char buf[128];
	char *dev, *flag, *cmd;
	char *strptr;
	unsigned int flag_no;
	struct net_device* netdevice;
	
	if (copy_from_user(buf, buffer, sizeof(buf))) {
		goto err_out;
	}
	
	strptr = buf;
	printk("%s(%d): buf = %s\n", __func__,__LINE__, buf);

	dev = strsep(&strptr," ");
	if(dev == NULL)
		goto err_out;

	cmd = strsep(&strptr," ");
	if(!memcmp(cmd, "read", 4)){
		netdevice = __dev_get_by_name(&init_net,dev);
		if(netdevice == NULL){
			printk("find no device!\n");
			goto err_out;
		}
		printk("%s priv_flags 0x%x\n", dev, netdevice->priv_flags);
		return count;
	}
	else if(!memcmp(cmd, "write", 5)){
		flag = strsep(&strptr," ");
		if (flag==NULL)
			goto err_out;

		flag_no=simple_strtol(flag, NULL, 0);

		netdevice = __dev_get_by_name(&init_net,dev);
		if(netdevice == NULL){
			printk("find no device!\n");
			goto err_out;
		}
		//printk("netdevice->priv_flags %x\n", netdevice->priv_flags);
		
		switch(flag_no)
		{
			case IFF_DOMAIN_WAN: //0x2000
				if(netdevice->priv_flags & IFF_DOMAIN_WLAN)
					netdevice->priv_flags &= (~IFF_DOMAIN_WLAN);
				netdevice->priv_flags |= IFF_DOMAIN_WAN;
				break;
			case IFF_DOMAIN_WLAN: //0x4000
				if(netdevice->priv_flags & IFF_DOMAIN_WAN)
					netdevice->priv_flags &= (~IFF_DOMAIN_WAN);
				netdevice->priv_flags |= IFF_DOMAIN_WLAN;
				break;
			case IFF_DOMAIN_ELAN:
				break;
			default:
				goto err_out;
		}
		printk("%s priv_flags change to 0x%x\n", dev, netdevice->priv_flags);
		
		return count;
	}
err_out:
	printk("invalid command!\n");
	return -EFAULT;
}

static int __init netdev_flag_init(void) {
	struct proc_dir_entry *pe;
	
	pe = create_proc_entry("netdev_flag", S_IRUSR |S_IWUSR | S_IRGRP | S_IROTH, NULL);
	if (!pe) {
		return -EINVAL;
	}
	
	pe->read_proc  = netdev_flag_proc_read;
	pe->write_proc = netdev_flag_proc_write;
	return 0;
}

static void __exit netdev_flag_exit(void) {
}

module_init(netdev_flag_init); 
module_exit(netdev_flag_exit); 

MODULE_DESCRIPTION("Net Device Flags"); 
MODULE_AUTHOR("Paula Tseng<pntseng@realtek.com>"); 
MODULE_LICENSE("GPL"); 