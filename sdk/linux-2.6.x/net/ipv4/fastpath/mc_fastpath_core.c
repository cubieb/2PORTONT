/*
	Linux Kernel Hacking:
	net/atm/br2684.c	// MC_FastPath_Enter()
	net/ipv4/ipmr.c		// ipmr_cache_find() and mcast forward
*/

#include <linux/ip.h>
#include <linux/mroute.h>
#include <linux/proc_fs.h>
#include "mc_fastpath_core.h"

#define	MODULE_NAME	"Realtek MCast FastPath"
struct mfc_cache *ipmr_cache_find(struct net *net, __be32 origin, __be32 mcastgrp);
int ip_mr_forward_fast(struct sk_buff *skb, struct mfc_cache *cache);
static int mc_fp_on=1;

int MC_FastPath_Enter(struct sk_buff *skb)	/* Ethertype = 0x0800 (IP Packet) */
{
	struct iphdr *iph;
	struct mfc_cache *mcache;
	
	if (!mc_fp_on)
		return 0;
	iph = (struct iphdr *)skb_network_header(skb);
	if(iph->frag_off & htons(0x3fff)){ // fragmentted packets go normal path
		return 0;
	}
	
	mcache = ipmr_cache_find(&init_net, 0, iph->daddr);
	if (mcache) {
		//printk("find 0x%x\n", iph->daddr);
		return ip_mr_forward_fast(skb, mcache);
	}
	return 0;
}

static struct proc_dir_entry *FP_Proc_File;
#define PROCFS_NAME 		"mc_FastPath"
static int fp_proc_read(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
	if(mc_fp_on==1)
		printk("mcast fastpath ON!\n");
	if(mc_fp_on==0)
		printk("mcast fastpath OFF!\n");
	return 0;
}

static int fp_proc_write(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
	char proc_buffer[count];
	
	/* write data to the buffer */
	memset(proc_buffer, 0, sizeof(proc_buffer));
	if ( copy_from_user(proc_buffer, buffer, count) ) {
		return -EFAULT;
	}
	switch(proc_buffer[0]) {
	case '0':
		mc_fp_on = 0;
		break;
	case '1':
		mc_fp_on = 1;
		break;
	default:
		printk("Error setting!\n");
	}
	return -1;
}

static int __init mc_fastpath_init(void)
{
	printk("%s\n",MODULE_NAME);
	//create proc
	FP_Proc_File= create_proc_entry(PROCFS_NAME, 0644, NULL);
	if (FP_Proc_File == NULL) {
		remove_proc_entry(PROCFS_NAME, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME);
		return -ENOMEM;
	}
	
	FP_Proc_File->read_proc  = (read_proc_t *)fp_proc_read;
	FP_Proc_File->write_proc  = (write_proc_t *)fp_proc_write;
	FP_Proc_File->mode 	  = S_IFREG | S_IRUGO;
	FP_Proc_File->uid 	  = 0;
	FP_Proc_File->gid 	  = 0;
	FP_Proc_File->size 	  = 37;
	
	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	return 0;
}

static void __exit mc_fastpath_exit(void)
{
	printk("%s removed!\n", MODULE_NAME);
}

module_init(mc_fastpath_init);
module_exit(mc_fastpath_exit);
