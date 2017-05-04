#include <linux/init.h>
#include <linux/proc_fs.h>

struct proc_dir_entry *realtek_proc=NULL;

EXPORT_SYMBOL(realtek_proc);

static int __init rtl_proc_init(void)
{
	int ret = 0;
	
	realtek_proc = proc_mkdir("realtek", NULL);
	if (!realtek_proc) {
		printk("realtek proc directory create fail\n");
	}
	return ret;
}

fs_initcall(rtl_proc_init); 
