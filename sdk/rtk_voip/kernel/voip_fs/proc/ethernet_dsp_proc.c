#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_proc.h"

#ifdef IPC_ARCH_DEBUG_HOST
extern unsigned int host_ctrl_tx_cnt[];
extern unsigned int host_rtp_rtcp_tx_cnt[];
extern unsigned int host_t38_tx_cnt[];
extern unsigned int host_ack_tx_cnt[];

extern unsigned int host_resp_rx_cnt[];
extern unsigned int host_rtp_rtcp_rx_cnt[];
extern unsigned int host_t38_rx_cnt[];
extern unsigned int host_event_rx_cnt[];
#endif

#ifdef IPC_ARCH_DEBUG_DSP
extern unsigned int dsp_ctrl_rx_cnt;
extern unsigned int dsp_rtp_rtcp_rx_cnt;
extern unsigned int dsp_t38_rx_cnt;
extern unsigned int dsp_ack_rx_cnt;

extern unsigned int dsp_resp_tx_cnt;
extern unsigned int dsp_rtp_rtcp_tx_cnt;
extern unsigned int dsp_event_tx_cnt;
extern unsigned int dsp_t38_tx_cnt;
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
int get_ethernet_dsp_info(struct seq_file *f, void *v)
{
#ifdef IPC_ARCH_DEBUG_HOST
	int i;
#endif
	
#ifdef IPC_ARCH_DEBUG_HOST
	seq_printf( f, "\n* Host Pakcet Tx Info:\n");
	for (i=0; i<CONFIG_RTK_VOIP_DSP_DEVICE_NR; i++)
	{
		seq_printf( f, "** To DSP#%d:\n", i);
		seq_printf( f, "  - Ctrl tx cnt: %d\n", host_ctrl_tx_cnt[i]);
		seq_printf( f, "  - Rtp/Rtcp tx cnt: %d\n", host_rtp_rtcp_tx_cnt[i]);
		seq_printf( f, "  - T38 tx cnt: %d\n", host_t38_tx_cnt[i]);
		seq_printf( f, "  - Ack Tx cnt: %d\n", host_ack_tx_cnt[i]);
	}
	
	seq_printf( f, "\n* Host Pakcet Rx Info:\n");
	for (i=0; i<CONFIG_RTK_VOIP_DSP_DEVICE_NR; i++)
	{
		seq_printf( f, "** From DSP#%d:\n", i);
		seq_printf( f, "  - Resp rx cnt: %d\n", host_resp_rx_cnt[i]);
		seq_printf( f, "  - Rtp/Rtcp rx cnt: %d\n", host_rtp_rtcp_rx_cnt[i]);
		seq_printf( f, "  - T38 rx cnt: %d\n", host_t38_rx_cnt[i]);
		seq_printf( f, "  - Event rx cnt: %d\n", host_event_rx_cnt[i]);
	}
#endif

#ifdef IPC_ARCH_DEBUG_DSP
	seq_printf( f, "\n* DSP Packet Rx Info:\n");
	seq_printf( f, " - Ctrl rx cnt: %d\n", dsp_ctrl_rx_cnt);
	seq_printf( f, " - Rtp/Rtcp  rx cnt: %d\n", dsp_rtp_rtcp_rx_cnt);
	seq_printf( f, " - T38	rx cnt: %d\n", dsp_t38_rx_cnt);
	seq_printf( f, " - Ack rx cnt: %d\n", dsp_ack_rx_cnt);
	
	seq_printf( f, "\n* DSP Packet Tx Info:\n");
	seq_printf( f, " - Resp tx cnt: %d\n", dsp_resp_tx_cnt);
	seq_printf( f, " - Rtp/Rtcp  tx cnt: %d\n", dsp_rtp_rtcp_tx_cnt);
	seq_printf( f, " - T38	tx cnt: %d\n", dsp_t38_tx_cnt);
	seq_printf( f, " - Event tx cnt: %d\n", dsp_event_tx_cnt);
#endif
	
	return -1;
}

#else
int get_ethernet_dsp_info( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
#ifdef IPC_ARCH_DEBUG_HOST
	int i;
#endif

#ifdef IPC_ARCH_DEBUG_HOST
	printk("\n* Host Pakcet Tx Info:\n");
	for (i=0; i<CONFIG_RTK_VOIP_DSP_DEVICE_NR; i++)
	{
		printk("** To DSP#%d:\n", i);
		printk("  - Ctrl tx cnt: %d\n", host_ctrl_tx_cnt[i]);
		printk("  - Rtp/Rtcp tx cnt: %d\n", host_rtp_rtcp_tx_cnt[i]);
		printk("  - T38 tx cnt: %d\n", host_t38_tx_cnt[i]);
		printk("  - Ack Tx cnt: %d\n", host_ack_tx_cnt[i]);
	}
	
	printk("\n* Host Pakcet Rx Info:\n");
	for (i=0; i<CONFIG_RTK_VOIP_DSP_DEVICE_NR; i++)
	{
		printk("** From DSP#%d:\n", i);
		printk("  - Resp rx cnt: %d\n", host_resp_rx_cnt[i]);
		printk("  - Rtp/Rtcp rx cnt: %d\n", host_rtp_rtcp_rx_cnt[i]);
		printk("  - T38 rx cnt: %d\n", host_t38_rx_cnt[i]);
		printk("  - Event rx cnt: %d\n", host_event_rx_cnt[i]);
	}
#endif

#ifdef IPC_ARCH_DEBUG_DSP
	printk("\n* DSP Packet Rx Info:\n");
	printk(" - Ctrl rx cnt: %d\n", dsp_ctrl_rx_cnt);
	printk(" - Rtp/Rtcp  rx cnt: %d\n", dsp_rtp_rtcp_rx_cnt);
	printk(" - T38  rx cnt: %d\n", dsp_t38_rx_cnt);
	printk(" - Ack rx cnt: %d\n", dsp_ack_rx_cnt);
	
	printk("\n* DSP Packet Tx Info:\n");
	printk(" - Resp tx cnt: %d\n", dsp_resp_tx_cnt);
	printk(" - Rtp/Rtcp  tx cnt: %d\n", dsp_rtp_rtcp_tx_cnt);
	printk(" - T38  tx cnt: %d\n", dsp_t38_tx_cnt);
	printk(" - Event tx cnt: %d\n", dsp_event_tx_cnt);
#endif
	
	return -1;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int get_ether_dsp_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, get_ethernet_dsp_info, NULL);
}

struct file_operations proc_get_ether_dsp_info_fops = {
	.owner	= THIS_MODULE,
	.open	= get_ether_dsp_info_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
//read:   get_ethernet_dsp_info
};
#endif

int __init ethernet_dsp_info_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	proc_create_data( "ethernet_dsp_info",0, NULL, &proc_get_ether_dsp_info_fops, NULL );
#else
	create_proc_read_entry( "ethernet_dsp_info",0, NULL, get_ethernet_dsp_info, NULL );
#endif
	return  0;
}

void __exit ethernet_dsp_info_exit( void )
{
	remove_proc_entry( "ethernet_dsp_info", NULL );
}

#ifndef CONFIG_RTK_VOIP_MODULE
voip_initcall(ethernet_dsp_info_init);
voip_exitcall(ethernet_dsp_info_exit);
#endif

