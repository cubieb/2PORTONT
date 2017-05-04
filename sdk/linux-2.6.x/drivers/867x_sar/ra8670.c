 /* ra8670.c - RealTek rtl8670 sar interface header */

#define DRV_NAME		"RTL8670 SAR"
#define DRV_VERSION		"v0.0.2"
#define DRV_RELDATE		"Jun 17, 2003"
#define AUTO_PVC_SEARCH	 
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/atmdev.h>
#include <linux/atm.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/crc32.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
// Kaohj -- for /proc/sys/sar
#include <linux/sysctl.h>
//ql_xu -- code optimize for route mode
//#include "../../../config/autoconf.h"
#include "../net/re830x.h"
#include "ra8670.h"
#include "ra867x_pp.h"
#include "ra_debug.h"
#ifdef CONFIG_SAR_FASTSKB
#include "fastskb.h"
#endif

#ifdef CONFIG_RTL8671
  #ifdef CONFIG_UCLINUX
    #include "../../arch/mipsnommu/rtl8670/lx5280.h"
  #else
    #include "../../arch/mips/realtek/rtl8670/lx5280.h"
  #endif
#endif

#ifdef CONFIG_RTL8681
#include <gpio.h>
#endif

//ql--upstream traffic control
//tylo, temp. marked for linux2.6
//#include "../../../user/boa/src/LINUX/options.h"
#define __SWAP  
extern int enable_ipqos;
/*linux-2.6.19*/
//int sar_irq=SAR_IRQ;
int sar_irq=BSP_SAR_IRQ;
//ql_xu: qos
#define QOS_IPTV_TR69			2
#define QOS_RULE_EXIST		1
int qosRuleExist=0;
int qosIPtv=0;

int 	sar_send (struct atm_vcc *,struct sk_buff *);

#ifdef CONFIG_DSL_CODESWAP
int	DSPInShowtime=0;
#else 
#define DSPInShowtime 1
#endif
#define IC8672 0
#define IC8671B 1
#define IC8671B_costdown 2

#ifdef UPSTREAM_TRAFFIC_CTL
#define BANDWIDT_1M	10*1024/8	//KBps

unsigned char ucTcEbl=0;
unsigned int uMaxTraffic=0;
unsigned int uTc1Data;
unsigned int LB_MIN=15000;
unsigned int LB_THD=20000;
unsigned int LB_SEC=30000;
unsigned int LB_MAX=95000;
unsigned int LB_len=100000;
#endif

//#define SAR_RX_FORMATION // adaptive RX descriptor sharing among PVCs
#define ZTE_531B_TEST	1

#ifndef AUTO_PVC_SERCH
	//unsigned int latestTraffic=0,AtmTraffic=0;	
	//unsigned int boa_pid=0;	//Records startup process ID
	//unsigned int sntp_pid=0;	//Records sntp process ID

#endif
#ifdef AUTO_PVC_SEARCH
#include <linux/proc_fs.h>
#define PROCFS_MAX_SIZE		32
#define PROCFS_NAME 		"AUTO_PVC_SEARCH"
#define MAX_PVC_SEARCH_PAIRS 16	//supports most 16 pairs 

static struct proc_dir_entry *APS_Proc_File;
static char procfs_buffer[PROCFS_MAX_SIZE] = {'0', ',', '0'};
int auto_search_start=0;
int found_pvc=0;	//default is set to 1 as disable auto-pvc-search
uint16 detect_vpi; 
uint16 detect_vci;
uint16 detect_framing;

// Added by Mason Yu for autohunt
// ARP request(192.168.8.1)
char test_packet[]={
	0xaa, 0xaa, 0x03, 0x00, 0x80, 0xc2, 0x00, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0xe0, 0x4c, 0xdd, 0x23, 0x15, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
	0x00, 0xe0, 0x4c, 0xdd, 0x23, 0x15, 0xc0, 0xa8, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xc0, 0xa8, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

// DHCP Discovery
char test_packet2[]={
	0xaa, 0xaa, 0x03, 0x00, 0x80, 0xc2, 0x00, 0x07, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0, 0x4c, 0xdd, 0x23, 0x15, 0x08, 0x00, 0x45, 0x00,
	0x01, 0x48, 0xf8, 0xbe, 0x00, 0x00, 0x80, 0x11, 0x40, 0xe7, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
	0xff, 0xff, 0x00, 0x44, 0x00, 0x43, 0x01, 0x34, 0x78, 0xb3, 0x01, 0x01, 0x06, 0x00, 0x43, 0x20,
	0xf7, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x4c, 0xdd, 0x23, 0x15, 0x00, 0x00, 0x00, 0x00,
	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82, 0x53, 0x63, 0x35, 0x01, 0x01, 0xfb, 0x01, 0x01,
	0x3d, 0x07, 0x01, 0x00, 0xe0, 0x4c, 0xdd, 0x23, 0x15, 0x32, 0x04, 0xc0, 0xa8, 0x01, 0x05, 0x0c,
	0x09, 0x72, 0x74, 0x70, 0x64, 0x49, 0x33, 0x33, 0x36, 0x35, 0x3c, 0x08, 0x4d, 0x53, 0x46, 0x54,
	0x20, 0x35, 0x2e, 0x30, 0x37, 0x0a, 0x01, 0x0f, 0x03, 0x06, 0x2c, 0x2e, 0x2f, 0x1f, 0x21, 0x2b,
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00	
};

struct VPIVCI{
	int vpi;
	int vci;
};
//PVC table
struct VPIVCI Possible_PVC[MAX_PVC_SEARCH_PAIRS];
// PVC table counter
int tbl_counter=0;

#ifdef CONFIG_RTL8681
/************************************ 
	enable_mode :		0 => ATM mode 
			          	1 => PTM mode
     	enable_phy_role: 	0 => master
     				  	1 => slave 
     				  	2 => slave
     				  	3 => slave     		 
    ************************************/
extern unsigned int enable_mode, enable_phy_role;
#endif
/*
 * Detect traffic for LINK(ACT) LED blinking (gpio.c)
 */
unsigned int latestTraffic=0,AtmTraffic=0;	

#ifdef CONFIG_PORT_MIRROR        
int sar_mirror_flag = 0;
#endif

/*
 * Get VPI from PVC table
 */
int getActiveVPINum(int count){
	return Possible_PVC[count].vpi;
}

/*
 * Get VCI from PVC table
 */
int getActiveVCINum(int count){
	return Possible_PVC[count].vci;
}

/*
 * Check if PVC exists in flash PVC table
 */
BOOL isPVCActiveExist(uint8 vpi, uint16 vci)
{
	int i;
	
	for(i=0;i< tbl_counter;i++)
	{
		if( (Possible_PVC[i].vpi == vpi) && (Possible_PVC[i].vci == vci) )
			return TRUE;
	}
	return FALSE;
}

// Added by Mason Yu for autohunt
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
int autohunt(struct atm_vcc *vcc, char *packet, int len)
{
	//sar_private *cp = sar_dev;
	int count2=0;	

#if 0
	// Send to default vc0
	for(count2=0;count2<tbl_counter;count2++){
						
		struct sk_buff test_skb, *testskb;		
		int vci, vpi, loop;					
		sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);					
		
		vpi = getActiveVPINum(count2);
		vci = getActiveVCINum(count2);
		if ((vpi==-1)&&(vci==-1)) continue;		
					
		SetVpiVci(cp,vpi,vci,vcc_dev->ch_no);	
		
		testskb = dev_alloc_skb( SAR_TX_Buffer_Size );
		memset(testskb, 0, sizeof(struct sk_buff));
		testskb->data=packet;
		testskb->len=len;
		
		mdelay(20);					
		sar_send(vcc,testskb);
		
		mdelay(100);
		//for (loop = 0; loop <= (150000000/(1000*3)); loop++) {
				// delay 1ms    
		//		;
		//}								
	}
#else
	// Send to outband channel
	for(count2=0;count2<tbl_counter;count2++){
						
		struct sk_buff *testskb;		
		int vci, vpi;		
		unsigned char *tail;			
		//sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);					
		
		vpi = getActiveVPINum(count2);
		vci = getActiveVCINum(count2);
		if ((vpi==-1)&&(vci==-1)) continue;		
		
		testskb = dev_alloc_skb( SAR_TX_Buffer_Size );
        	tail=testskb->data;
        	skb_put(testskb, len);
    		memcpy(tail, packet, len);
    		mdelay(20);
    		//printk("Send to outband\n");
		OutBandAAL5Tx(vpi, vci, testskb);
		mdelay(100);								
	}
#endif	

	return 0;
}

// called then the /proc/AUTO_PVC_SEARCH is read	
int procfile_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{

	int procfs_buffer_size;
	
	if (offset > 0) {
		procfs_buffer_size  = 0;
	} else {
		procfs_buffer_size = strlen(procfs_buffer);	
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_buffer, procfs_buffer_size);
	}
	printk(KERN_INFO "%s\n", procfs_buffer);
	return procfs_buffer_size;
}

void startPVCSearch(void);
/*
 * called then the /proc/AUTO_PVC_SEARCH is wrote
 *	1 : enable SAR driver auto-pvc-search and attach PVC table
 *	0 : disable auto-pvc-search
 */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
int procfile_write(struct file *file, const char *buffer, unsigned long count,
		   void *data)
{
	sar_private *cp = sar_dev;
	struct atm_vcc *vcc;
	UCHAR proc_buffer[MAX_PVC_SEARCH_PAIRS*12], tmp[12];
	int ok,vpi,vci, i,j;
	//int count2;
	
    	vcc = cp->vcc[0].dev_data;
	
	/* write data to the buffer */
	memset(proc_buffer, 0, sizeof(proc_buffer));
	if ( copy_from_user(proc_buffer, buffer, count) ) {
		return -EFAULT;
	}

//	printk(KERN_INFO "\nprocfile_write received string %s(count %d) \n", proc_buffer, count);

	/*
	 * Disable SAR driver auto-pvc-search if startup write '0' to /proc/auto_pvc_search
	 * 	Parsing the PVC table pair received from user space
	 */
	if(proc_buffer[0] == '1') {
		for(i=1;i<count;i++) {
			memset(tmp,0,sizeof(tmp));
			if(proc_buffer[i] == '(') {
				for(j=0;j<5;j++){
					tmp[j] = proc_buffer[i+1+j];
					if(proc_buffer[i+j+2] == ')')
						break;
				}
				sscanf(tmp, "%d %d ", &vpi, &vci);
			//	printk("procfile_write : received  pair %d (%d %d)\n", tbl_counter,vpi, vci);
				Possible_PVC[tbl_counter].vpi = vpi;
				Possible_PVC[tbl_counter++].vci = vci;
			}
		}			

		auto_search_start=1;				
		// Start SAR driver PVC search
		startPVCSearch();				
		found_pvc=0;		
		
		// Added by Mason Yu. for autoHunt
		autohunt(vcc, test_packet2, sizeof(test_packet2));

	} else {
//		printk("procfile_write: disable SAR driver auto-pvc-search (%d,%d) auto_search_start %d found_pvc %d\n", vcc->vpi,vcc->vci, auto_search_start, found_pvc);
		if(found_pvc == 0 ) {
			found_pvc = 1;		
		}
		 if( auto_search_start&& !found_pvc){
			sscanf(proc_buffer, "%d %d,%d", &ok,&vpi,&vci);
	//		printk("**********  procfile_write: setting sar_dev (%d, %d) **********\n", vpi,vci);
		 	
		 	if(vcc->vpi == 0 && vcc->vci == 0) {
				sar_dev->vcc[0].framing=0;		
				SetVpiVci(sar_dev,vpi,vci,0);
	 		}				
			Enable_rx_ch(sar_dev,0);	
#ifdef CONFIG_RTL8672	 		
			Enable_Drop_NonOAM(sar_dev,OAM_CH_NO+1);
#else
			Enable_Drop_NonOAM(sar_dev,OAM_CH_NO);
#endif
			
			// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
#ifdef CONFIG_RTL8672	 		
			Disable_rx_ch(cp, OAM_CH_NO+1);
#endif
			detect_vpi = vpi;
			detect_vci = vci;
//			detect_framing =0;	//fixme
			found_pvc=1;
			auto_search_start=0;
		
		}
	}
	return count;
}

#endif	

#define DEV_LABEL	"ratm"

//#ifdef CONFIG_EXT_SWITCH
extern int enable_ipqos;

sar_private	*sar_dev;
struct tasklet_struct *sar_rx_tasklets=NULL;
extern struct net_device *eth_net_dev;
#ifdef FAST_ROUTE_Test
//tylo, for linux 2.6 porting
extern int fast_route_mode;
int fast_frame_up=1;

typedef struct _ch_val
{
	unsigned long counter;		//counts the tiny (<256bytes) pkt of this CH
	unsigned long start_stamp;	//records the first tiny pkt of this CH arrivaled time
	int	 tx_inuse_counter;	//for store the vcc->tx_inuse value before change to fast_route_mode
} ch_val;
ch_val ch_counter[Enable_VC_CNT];

#define MAX_MAC_ADDR	8

typedef struct _mac_table_items
{
	unsigned int index;
	unsigned int enable;
	uint8 mac[MAX_MAC_ADDR][6];
} mac_table_items;

mac_table_items  fast_route_ch_no[Enable_VC_CNT];

#endif
//for Auto PVC-Search

unsigned int tr069_pid = 0;

unsigned int boa_pid=0;	//Records startup process ID
#ifdef AUTO_PVC_SEARCH

unsigned int sntp_pid=0;	//Records sntp process ID

/*
 * Found PVC 
 * 	Sets the correct PVC and enables CH 0 rx and dropping nonoam cell.
 *	Notify startup process we found PVC and recreate the connection.
 */
 extern long sys_kill(int pid, int sig);
//1__SWAP
void foundVC(uint8 vpi,uint16 vci,int mode)
{
	
	found_pvc = 1;
	//Enable_rx_ch(sar_dev,0);
#ifdef CONFIG_RTL8672	 		
	Enable_Drop_NonOAM(sar_dev,OAM_CH_NO+1);
#else
	Enable_Drop_NonOAM(sar_dev,OAM_CH_NO);
#endif
	
	if(mode==0) 
		printk("get vpi %d   vci %d  LLC mode\n",vpi,vci);
	else
		printk("get vpi %d   vci %d  VC-MUX mode\n",vpi,vci);
    
	detect_vpi = vpi;
	detect_vci = vci;
	detect_framing = mode;

	memset(procfs_buffer, 0, PROCFS_MAX_SIZE);
	sprintf(procfs_buffer, "%d,%d", vpi,vci);

	if(boa_pid) {
		printk("foundVC: kill SIGUSR2 signal to process boa (%d)\n", boa_pid);
		sys_kill(boa_pid, SIGUSR2);	//raise sigusr2 to process boa
	} else {
		printk("foundVC: startup process is gone??\n");
	}
}

/*
 * Received OAM cell check
 *	Check the PVC of OAM cell and protocol
 *	If PVC didn't exist and protocol is br1483, we found it!
 */
__SWAP
void searchOamAutoPvc(sar_private *cp, uint8 ch_no)
{
	int8    j=0;
	RV_STS_DESC   *RVDesc=NULL;
	unsigned char *outband_cell;
	uint8 rx_vpi; uint16 rx_vci;

	j=cp->vcc[ch_no].RV.desc_pr;
	// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
#ifdef CONFIG_RTL8672	
	if(!(cp->RODesc[j+ SAR_RX_RING_SIZE].STS&OWN))
#else
	if(!(cp->RODesc[j].STS&OWN))
#endif
	{	
		// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
#ifdef CONFIG_RTL8672
		RVDesc=(RV_STS_DESC   *)&cp->RODesc[j+ SAR_RX_RING_SIZE];
#else
		RVDesc=(RV_STS_DESC   *)&cp->RODesc[j];
#endif
		outband_cell=(unsigned char *)RVDesc->START_ADDR;
			
		rx_vpi=(uint8)(outband_cell[0]<<4|outband_cell[1]>>4);
		rx_vci=(uint16)(outband_cell[1]<<12|outband_cell[2]<<4|outband_cell[3]>>4);
		printk("searchOamAutoPvc: receiving %d/%d\n", rx_vpi, rx_vci);	
		if (isPVCActiveExist(rx_vpi, rx_vci)) {  //pvc match, check packet type	
#if 0		
		for(i=0;i<0x30;i++){
			if(i%0x10==0)
				printk("\n");
			printk("%x  ",outband_cell[i]);
		}
		printk("\n\n");
#endif		
		// routed protocol
		/*	support bridged mode only 
	    		if( (outband_cell[4]==0xaa) && (outband_cell[5]==0xaa) && 
				(outband_cell[6]==0x03) && (outband_cell[7]==0x00) && 
				(outband_cell[8]==0x00) && (outband_cell[9]==0x00) )
			{// compare outband cell
				//printk("RX  %x  %x  %x  %x  %x\n",outband_cell[0],outband_cell[1],
				//outband_cell[2],outband_cell[3],outband_cell[4],outband_cell[5]);
				//xprintfk("get vpi %d   vci %d (%d)\n",rx_vpi,rx_vci,ch_no);
				foundVC(rx_vpi, rx_vci,0);//LLCHdr802_3
			}// end of compare outband cell
		*/
		// bridged protocol
	    		if( (outband_cell[4]==0xaa) && (outband_cell[5]==0xaa) && 
				(outband_cell[6]==0x03) && (outband_cell[7]==0x00) && 
				(outband_cell[8]==0x80) && (outband_cell[9]==0xc2) && 
				(outband_cell[10]==0x00) && 
				(outband_cell[11]==0x01 || outband_cell[11]==0x07))
			{// compare outband cell
				//printk("RX  %x  %x  %x  %x  %x\n",outband_cell[0],outband_cell[1],
				//outband_cell[2],outband_cell[3],outband_cell[4],outband_cell[5]);
				//xprintfk("get vpi %d   vci %d (%d)\n",rx_vpi,rx_vci,ch_no);
				foundVC(rx_vpi, rx_vci,0);//LLCHdr802_3				
			}// end of compare outband cell
			if((outband_cell[18]==0x08 && 
				(outband_cell[19]==0x00 || outband_cell[19]==0x06))||
				(outband_cell[18]==0x80 && outband_cell[19]==0x35))
			{
				foundVC(rx_vpi, rx_vci,1);//VCMuxHdr802_3
			}
		}// is pvc active exist 	
		RVDesc=NULL;
	} //->RODesc[j].STS&OWN)
}
#endif

//UCHAR SarMacAddr[6]={0x00, 0x00, 0x00, 0x04, 0x05, 0x06};

UCHAR LLCHdr802_3[10]		= {0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00};
UCHAR VCMuxHdr802_3[2]		= {0x00, 0x00};
UCHAR LLCHdrRoutedIP[8]		= {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00};
UCHAR VCMuxHdrRoutedIP[1]	= {0x00}; 
UCHAR ARP_1577[8]			= {0xAA,0xAA,0x03,0x00,0x00,0x00,0x08,0x06};
UCHAR DATA_1577[8]			= {0xAA,0xAA,0x03,0x00,0x00,0x00,0x08,0x00};

UCHAR *FrameHeader_1483[4]={
			LLCHdr802_3, 		/* RFC1483_BR_LLC, LLC/SNAP for 802.3 */
			VCMuxHdr802_3, 	/* RFC1483_BR_VCMux, VC Mux for 802.3 */
			LLCHdrRoutedIP,		/* RFC1483_RT_LLC, LLC/SNAP for routed ip*/
			VCMuxHdrRoutedIP	/* RFC1483_RT_VCMux, VC Mux for routed ip should be NULL encap */
		};
		
UCHAR *FrameHeader_1577[2]={
			ARP_1577, 		/* for Arp Packet */
			DATA_1577		/* for Data Packet */
			};	

int8	FrameHeaderSize_1483[4]={0x0A, 0x02, 0x08, 0x00};
int8	FrameHeaderSize_1577[2]={0x08, 0x08};

#ifdef CONFIG_PORT_MIRROR        
int8	FrameHeaderSize_2364[2]={4, 0}; // {LLC, VC-MUX}
UCHAR Fake_MAC_addr_header[14] = {0x00,0x06,0x68,0x89,0x90,0x06,0x00,0x66,0x88,0x99,0x00,0x66,0xFF,0xFF};
UCHAR Fake_PPPoE_header[6]     = {0x11,0x00,0x56,0x78,0xFF,0xFF};
#define MAC_PROTOCOL_POS       12
#define PPPOE_LENGTH_POS       4
#define FILL_MAC_ADDR          0x01
#define FILL_PPPOE_HEADER      0x02
#endif

//1/1/06' hrchen, desc # of a PVC is depended on total PVC numbers
int total_pvc_number=0, per_vc_desc_number=0;
//1/13/06' hrchen, for desc num re-assign
int current_desc_number[Enable_VC_CNT];  //record current desc # of a PVC
#ifdef SAR_RX_FORMATION
struct sk_buff 	*free_rx_skb_list[SAR_MAX_Process_DESC];  //free skb list
int free_rx_skb_producer=0;  //location to put free skb
int free_rx_skb_consumer=SAR_MAX_Process_DESC-1;  //location to get free skb
int busy_channel[Enable_VC_CNT];
int do_formation;
#endif // of SAR_RX_FORMATION

// for debug
int debug_obaal5=0;
int debug_obcell=0;
int debug_num2print=100;

/* These identify the driver base version and may not be removed. */
//static char version[] __devinitdata =
//    KERN_INFO DRV_NAME " sar driver v" DRV_VERSION " (" DRV_RELDATE ")\n";

MODULE_AUTHOR("Jonah Chen <jonah@realtek.com.tw>");
MODULE_DESCRIPTION("RealTek RTL-8670 series SAR driver");
MODULE_LICENSE("GPL");
/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
static int multicast_filter_limit = 32;

/* bitmapped message enable number */
static int debug = -1;
module_param(debug, int, 0);
MODULE_PARM_DESC (debug, "sar bitmapped message enable number");

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
module_param(multicast_filter_limit, int, 0);
MODULE_PARM_DESC (multicast_filter_limit, "8139cp maximum number of filtered multicast addresses");

#define PFX			DRV_NAME ": "


extern void enable_lx4180_irq(int irq);
extern void disable_lx4180_irq(int irq);
//static void sar_tx (sar_private *cp);
//static void sar_clean_rings (sar_private *cp);
static void sar_close (struct atm_vcc *vcc);
extern void FreeVcBuff(sar_private *cp, int8 ch_no);
void set_atm_data_mode(int mode);
// Kaohj -- map vc index to channel number
int index_vc2ch(int vc_idx);


#ifdef ATM_OAM
void InitOAM(unsigned short Vpi, unsigned short Vci);
#endif

#ifdef CONFIG_PORT_MIRROR        
extern void nic_tx_mirror(struct sk_buff *skb);
#endif

static int sar_to_qos(int type)
{
	switch(type) {
	case ATM_NONE:
	case ATM_UBR:
		return QoS_UBR;
	case ATM_CBR:
		return QoS_CBR;
	case ATM_VBR:
		return QoS_nrtVBR;
	default:
		return QoS_rtVBR;
	}
}

#ifdef CONFIG_PORT_MIRROR
/* non-reentrant */
static void sar_port_mirror(sar_private *cp, int ch_no, struct sk_buff *skb, int dir)
{
	struct atm_vcc *atm_vcc = cp->vcc[ch_no].dev_data;
	struct sk_buff *skb_mirror;
	struct net_device *net_dev = (struct net_device *)atm_vcc->net_dev;
	int cut_len = 0, fill_flag = 0, fill_len = 0, encap_mode;
	unsigned char *mac_header = NULL;
	
	if ((sar_mirror_flag & dir) || (atm_vcc->mirror_flag & dir))
	{
		switch (cp->vcc[ch_no].rfc)
		{
			case RFC1483_BRIDGED:
				/* with mac */
				encap_mode = cp->vcc[ch_no].rfc * 2 + cp->vcc[ch_no].framing;
				cut_len = FrameHeaderSize_1483[encap_mode];
				mac_header = &(skb->data[cut_len]);
				if (NULL != net_dev->br_port)
				{
					/* Bridged interface; only frames from or to this device will be mirrored */
					if (dir & PORT_MIRROR_OUT)
					{
						/* check source mac */
						if (0 != memcmp(mac_header + ETH_ALEN, net_dev->dev_addr, ETH_ALEN))
						{
							/* not sent by modem */
							return;
						}
					}
					else
					{
						/* check dst mac */
						if (0 != memcmp(mac_header, net_dev->dev_addr, ETH_ALEN))
						{
							/* not sent to modem */
							return;
						}
					}
				}
				break;
			
			case RFC1483_ROUTED: /* fall through */
				encap_mode = cp->vcc[ch_no].rfc * 2 + cp->vcc[ch_no].framing;
				cut_len = FrameHeaderSize_1483[encap_mode];
				fill_flag = FILL_MAC_ADDR;
				if (VC_MUX == cp->vcc[ch_no].framing)
				{
					/* IPoA */
					*(unsigned short *)(&Fake_MAC_addr_header[MAC_PROTOCOL_POS]) = (unsigned short)(0x0800);
				}
				else
				{
					/* aligned */
					memcpy(&Fake_MAC_addr_header[MAC_PROTOCOL_POS], skb->data + cut_len - 2, sizeof(unsigned short));
				}
				break;
			
			case RFC1577:
				cut_len = FrameHeaderSize_1577[0];
				fill_flag = FILL_MAC_ADDR;
				memcpy(&Fake_MAC_addr_header[MAC_PROTOCOL_POS], skb->data + cut_len - 2, sizeof(unsigned short));
				break;
			
			case RFC2364:
				cut_len = FrameHeaderSize_2364[cp->vcc[ch_no].framing];
				fill_flag = FILL_PPPOE_HEADER | FILL_MAC_ADDR;
				*(unsigned short *)(&Fake_MAC_addr_header[MAC_PROTOCOL_POS]) = (unsigned short)(0x8864);
				break;
			
			default:
				return;
		}
		
		if (fill_flag & FILL_PPPOE_HEADER)
		{
			fill_len += sizeof(Fake_PPPoE_header);
		}
		
		if (fill_flag & FILL_MAC_ADDR)
		{
			fill_len += sizeof(Fake_MAC_addr_header);
		}
		
		/* reserve enough room */
		if (NULL == (skb_mirror = skb_copy_expand(skb, fill_len + skb_headroom(skb), skb_tailroom(skb), GFP_ATOMIC)))
		{
			return;
		}
		
		skb_pull(skb_mirror, cut_len);
		
		if (NULL != mac_header)
		{
			/* when mirroring unicast frames, we should modify the dst mac, or they may
			 * not be transmitted by 8305, bridge things.
			 */
			if (!(skb_mirror->data[0] & 0x1))
			{
				skb_mirror->data[2] += 1;
				skb_mirror->data[3] += 1;
			}
		}
		
		/* Fill header */
		if (fill_flag & FILL_PPPOE_HEADER)
		{
			*(unsigned short *)(&Fake_PPPoE_header[PPPOE_LENGTH_POS]) = (unsigned short)(skb_mirror->len);
			skb_push(skb_mirror, sizeof(Fake_PPPoE_header));
			memcpy(skb_mirror->data, Fake_PPPoE_header, sizeof(Fake_PPPoE_header));
		}
		
		if (fill_flag & FILL_MAC_ADDR)
		{
			skb_push(skb_mirror, sizeof(Fake_MAC_addr_header));
			memcpy(skb_mirror->data, Fake_MAC_addr_header, sizeof(Fake_MAC_addr_header));
		}
		
		/* mirror packets can be received on switch ports which are not bound. */
		skb_mirror->vlan_member = 0;
		nic_tx_mirror(skb_mirror);
	}
}
#endif

#ifdef SAR_RX_FORMATION
//1/14/06' hrchen, reduce rx desc num for a PVC
/*
  If we can't free enough desc, it means we get "Busy case!". There are pkts burst into
  this pvc. This pvc will extend desc in next run. Therefore, return reduce status is not
  necessary.
*/
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
int reduce_rx_desc(sar_private *cp, int ch_no, int sub_num)
{
  int i, j, jm2;
  RV_STS_DESC	*RVDesc, *jm2RVDesc;

	local_irq_disable();	//disable CPU local interrupts
    //move to last desc, free backward from last available desc
    j=(cp->vcc[ch_no].RV.desc_pr+current_desc_number[ch_no]-1)&(SAR_RX_DESC_NUM-1);
    jm2=(cp->vcc[ch_no].RV.desc_pr+current_desc_number[ch_no]-2)&(SAR_RX_DESC_NUM-1);
    for (i=0;i<sub_num;i++) {
		jm2RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+jm2];
		if (!(((int32)jm2RVDesc->STS)&OWN)) {  //skb already receive a pkt, no more to free
			//printk("Busy case! %d\n", i);
			break;
		}
		//move this rx skb to free list
		RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j];
    	if (cp->vcc[ch_no].rx_buf[j]==(unsigned int)NULL)
    	    printk("error");
		RVDesc->STS = 0;  //set owner to cpu
    	free_rx_skb_list[free_rx_skb_producer] = (struct sk_buff *)cp->vcc[ch_no].rx_buf[j];
    	cp->vcc[ch_no].rx_buf[j] = (UINT32)NULL;
    	free_rx_skb_producer++;
    	if (free_rx_skb_producer==SAR_MAX_Process_DESC) 
    	    free_rx_skb_producer=0;  //touch end, move to head
    	current_desc_number[ch_no]--;
    	if (current_desc_number[ch_no]<=SAR_RX_DESC_LOW_LIMIT) 
    	    break;  //reach low limit, stop reduce
		j = (j-1)&(SAR_RX_DESC_NUM-1);  //1 desc ahead
		jm2 = (jm2-1)&(SAR_RX_DESC_NUM-1);  //1 desc ahead
    }
	local_irq_enable();	  
	return 0;
}

//1/14/06' hrchen, extend rx desc num for a PVC
//rtn: 1 ok, 0 can not extend all desc at this run
int extend_rx_desc(sar_private *cp, int ch_no, int add_num)
{
  int i, j, rtn=1;
  RV_STS_DESC	*RVDesc;
  int tmp_consumer;
  struct sk_buff 	*skb;

    //move to last desc, allocate foreward from last free desc
    j=(cp->vcc[ch_no].RV.desc_pr+current_desc_number[ch_no])&(SAR_RX_DESC_NUM-1);
    for (i=0;i<add_num;i++) {
    	tmp_consumer=free_rx_skb_consumer+1;
    	//get a skb from free list
    	if (tmp_consumer==SAR_MAX_Process_DESC) 
    	    tmp_consumer=0;  //touch end, move to head
    	if (tmp_consumer==free_rx_skb_producer) {
    		rtn=0;
    	    break;  //no free skb
    	};
        
    	if (current_desc_number[ch_no]>=SAR_RX_DESC_HI_LIMIT) 
    	    break;  //reach hi limit, stop extend
    	    
        //get a free skb, update counter
    	current_desc_number[ch_no]++;
        free_rx_skb_consumer = tmp_consumer;
        
		//set & activate this rx desc
		RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j];
		skb = free_rx_skb_list[tmp_consumer];
		if (skb==NULL)
		    printk("error\n");
		RVDesc->START_ADDR=(UINT32)skb->data | Uncache_Mask;
		RVDesc->LEN=SAR_RX_Buffer_Size;
    	cp->vcc[ch_no].rx_buf[j]= (UINT32)skb;
		RVDesc->STS= ((j!=(SAR_RX_DESC_NUM-1))?OWN:(OWN|EOR)); //set owner to device
        
		j = (1 + j)&(SAR_RX_DESC_NUM-1);  //1 desc ahead   
    };
    return rtn;
}

//rtn:1 ok, 0 can't extend all desc for a pvc at this run
int change_formation(sar_private *cp, int ch_no, int ch_rx_num, int avg_desc_num)
{
  int rtn=1;
    if (ch_rx_num==0) {  //reduce desc
    	int diff_num=current_desc_number[ch_no]-SAR_RX_DESC_LOW_LIMIT;
    	if (diff_num==0) goto ret_change_formation;
    	else {
    		diff_num=(diff_num>8)?8:diff_num;  //slow down reduce process
   	        reduce_rx_desc(cp, ch_no, diff_num);
   	        //printk("CFR%d %d\n", ch_no, diff_num);
   	    };
    } else {  //extend desc
    	int diff_num=avg_desc_num - current_desc_number[ch_no];
    	if (diff_num==0) goto ret_change_formation;
    	else if (diff_num<0) {
    		diff_num=-diff_num;
    		diff_num=(diff_num>8)?8:diff_num;  //slow down reduce process
   	        reduce_rx_desc(cp, ch_no, diff_num);
   	        //printk("CFR%d %d\n", ch_no, diff_num);
   	    } else {
      		rtn=extend_rx_desc(cp, ch_no, diff_num);
   	        //printk("CFE%d %d %d\n", ch_no, diff_num, rtn);
   	    };
    };
    
ret_change_formation:
    return rtn;
}
#endif // of SAR_RX_FORMATION

//2__SWAP
void sar_rx_error_desc(sar_private *cp, uint8 ch_no, RV_STS_DESC *RVDesc, struct sk_buff *skb, int8 *j,RV_STS_DESC *enable_RVDesc,int8 *enable_j)
{
//Error_Desc:
		cp->vcc[ch_no].stat.rx_desc_fail++;
		cp->vcc[ch_no].stat.rx_pkt_fail++;
		//RVDesc->STS &= 0x4FFF;
#if 0
		if(ch_no==OAM_CH_NO)
			RVDesc->LEN=SAR_OAM_Buffer_Size;
		else
			RVDesc->LEN=SAR_RX_Buffer_Size;
		/* Fill Descriptor with old skb */
		RVDesc->STS= ((*j!=(SAR_RX_DESC_NUM-1))?OWN:(OWN|EOR));
		//cp->vcc[ch_no].rx_buf[*j]=(UINT32)skb;
		*j = (1 + *j)&(SAR_RX_DESC_NUM-1);
	    RVDesc = (*j==0)?(&cp->RVDesc[ch_no*SAR_RX_RING_SIZE]):(RVDesc+1);
		//k= GetRxCDOI(cp, ch_no);
#else
		if(ch_no==OAM_CH_NO)
			enable_RVDesc->LEN=SAR_OAM_Buffer_Size;
		else
			enable_RVDesc->LEN=SAR_RX_Buffer_Size;
		enable_RVDesc->START_ADDR=(UINT32)skb->data | Uncache_Mask;
		enable_RVDesc->STS= ((*enable_j!=(SAR_RX_DESC_NUM-1))?OWN:(OWN|EOR));
		cp->vcc[ch_no].rx_buf[*enable_j]=(UINT32)skb;
		cp->vcc[ch_no].rx_buf[*j]=(UINT32)NULL;

		*j = (1 + *j)&(SAR_RX_DESC_NUM-1);
		//RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+*j];

		//printk("next j:%d\n",*j);
		*enable_j=(1 + *enable_j)&(SAR_RX_DESC_NUM-1);
		//enable_RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+*enable_j];
		//RVDesc = (*j==0)?(&cp->RVDesc[ch_no*SAR_RX_RING_SIZE]):(RVDesc+1);
		//enable_RVDesc= (*enable_j==0)?(&cp->RVDesc[ch_no*SAR_RX_RING_SIZE]):(enable_RVDesc+1);

#endif

}

#ifdef ZTE_531B_TEST
//3__SWAP
void sar_rx_err_process(sar_private *cp, uint8 ch_no, RV_STS_DESC **RVDesc, struct sk_buff *skb, int8 *j,RV_STS_DESC **enable_RVDesc,int8 *enable_j)
{
	cp->vcc[ch_no].stat.rx_desc_fail++;
	cp->vcc[ch_no].stat.rx_pkt_fail++;
	//RVDesc->STS &= 0x4FFF;
#if 0
	if(ch_no==OAM_CH_NO)
		RVDesc->LEN=SAR_OAM_Buffer_Size;
	else
		RVDesc->LEN=SAR_RX_Buffer_Size;
	/* Fill Descriptor with old skb */
	RVDesc->STS= ((*j!=(SAR_RX_DESC_NUM-1))?OWN:(OWN|EOR));
	//cp->vcc[ch_no].rx_buf[*j]=(UINT32)skb;
	*j = (1 + *j)&(SAR_RX_DESC_NUM-1);
	RVDesc = (*j==0)?(&cp->RVDesc[ch_no*SAR_RX_RING_SIZE]):(RVDesc+1);
	//k= GetRxCDOI(cp, ch_no);
#else
	if(ch_no==OAM_CH_NO)
		(*enable_RVDesc)->LEN=SAR_OAM_Buffer_Size;
	else
		(*enable_RVDesc)->LEN=SAR_RX_Buffer_Size;
	(*enable_RVDesc)->START_ADDR=(UINT32)skb->data | Uncache_Mask;
	(*enable_RVDesc)->STS= ((*enable_j!=(SAR_RX_DESC_NUM-1))?OWN:(OWN|EOR));
	cp->vcc[ch_no].rx_buf[*enable_j]=(UINT32)skb;
	cp->vcc[ch_no].rx_buf[*j]=(UINT32)NULL;

	*j = (1 + *j)&(SAR_RX_DESC_NUM-1);
	(*RVDesc)=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+*j];

	//printk("next j:%d\n",*j);
	*enable_j=(1 + *enable_j)&(SAR_RX_DESC_NUM-1);
	(*enable_RVDesc)=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+*enable_j];
	//RVDesc = (*j==0)?(&cp->RVDesc[ch_no*SAR_RX_RING_SIZE]):(RVDesc+1);
	//enable_RVDesc= (*enable_j==0)?(&cp->RVDesc[ch_no*SAR_RX_RING_SIZE]):(enable_RVDesc+1);
#endif
	//(*RVDesc)=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+*j];
	//(*enable_RVDesc)=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+*enable_j];
}
#endif

int pvc_exist(sar_private *cp, int vpi, int vci)
{
  int i;
  
	for (i=0;i<Enable_VC_CNT;i++) {
	    if ((cp->vcc[i].vpi==vpi)&&(cp->vcc[i].vci==vci)){
	    	return 1;
	    };
	};
	return 0;
}


// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
//#ifdef CONFIG_RTL8672
// handle RO0 and RO1 CH Cell receiving
BOOL sar_rx_oam2 (sar_private *cp, uint8 ch_no)
{
	DRV_ENTER

	int8 		j;
	RV_STS_DESC	*RVDesc;
#ifdef AUTO_PVC_SEARCH	
	unsigned char * oamData;  //Check the indication field of receiving OAM cell for auto_pvc search
	// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
	if(!found_pvc && auto_search_start)	
		searchOamAutoPvc(cp,ch_no);	
#endif

	j=cp->vcc[ch_no].RV.desc_pr;

	cp->vcc[ch_no].stat.rcv_cnt++;
	
	/* working on descriptors */
	while(!(cp->RODesc[j+SAR_RX_RING_SIZE*(ch_no-OAM_CH_NO) ].STS&OWN)){
		unsigned int *ptr;
		int vpi, vci;
		
		/* Drop OAM cell now */		
			cp->vcc[ch_no].stat.rx_oam_count++;

			RVDesc = (RV_STS_DESC*)&cp->RODesc[j+SAR_RX_RING_SIZE*(ch_no-OAM_CH_NO)];
/*			
		printk("%s: dealing RODesc[%d] (0x%08x)now!\n", __func__, j+SAR_RX_RING_SIZE*(ch_no-OAM_CH_NO),(unsigned int )RVDesc);
		printk("%s: \nRVDesc->STS %x LEN %x\n", __func__, RVDesc->STS, RVDesc->LEN);
		printk("RVDesc->START_ADDR %x\n", RVDesc->START_ADDR);
*/		
#if 1	
//			ptr = (unsigned int *)RVDesc->START_ADDR;
                        ptr = (unsigned int *)(RVDesc->START_ADDR | Uncache_Mask);	

			vpi = (*ptr>>20)&0x000000FF;
			vci = (*ptr>>4)&0x0000FFFF;
			//cp->vcc[ch_no].stat.rx_FS_cnt++;
			//cp->vcc[ch_no].stat.rx_LS_cnt++;
			//cp->vcc[ch_no].stat.rx_desc_cnt++;

			#ifdef ATM_OAM
				{
			int oam_type;
			//status = RVDesc->STS;
			if(RVDesc->STS&0x0100) {
				//only responce what PVC you have
#ifdef AUTO_PVC_SEARCH	
				oamData = (unsigned char *)RVDesc->START_ADDR;

				oam_type=(RVDesc->STS&0x00C0)>>6;
				if((oam_type == 0)||(oam_type == 1))
				{	// OAM F4 cell
					// only reply F4 OAM LB request and PVC had been existed
				        if(oamData[OAM_FORMAT_LB_INDICATION] & 1) {
						if(pvc_exist(cp, vpi,vci))
							OAMRxF4Cell((unsigned char *)(RVDesc->START_ADDR| Uncache_Mask), &OAMF4_info);
					} else {
						OAMRxF4Cell((unsigned char *)(RVDesc->START_ADDR| Uncache_Mask), &OAMF4_info);
					}
				}
				else if((oam_type == 2)||(oam_type == 3))
				{	// OAM F5 cell
				//  Only reply F5 OAM LB request and PVC had been existed

					if(oamData[OAM_FORMAT_LB_INDICATION] & 1) {
						if(pvc_exist(cp, vpi,vci))
							OAMRxF5Cell((unsigned char *)(RVDesc->START_ADDR| Uncache_Mask), &OAMF5_info);
					} else {
						OAMRxF5Cell((unsigned char *)(RVDesc->START_ADDR| Uncache_Mask), &OAMF5_info);
					}
				}
#else	//AUTO_PVC_SEARCH
				if (pvc_exist(cp, vpi, vci)) {

					
					#if 1
					oam_type=(RVDesc->STS&0x00C0)>>6;
					if((oam_type == 0)||(oam_type == 1))
					{	// OAM F4 cell
						OAMRxF4Cell((unsigned char *)(RVDesc->START_ADDR| Uncache_Mask), &OAMF4_info);
					}
					else if((oam_type == 2)||(oam_type == 3))
					{	// OAM F5 cell
						OAMRxF5Cell((unsigned char *)(RVDesc->START_ADDR| Uncache_Mask), &OAMF5_info);
					}
					#else
						process_aal0(cp, (unsigned char *)RVDesc->START_ADDR);
					#endif
				};
#endif	//AUTO_PVC_SEARCH				
			}
				}
			#endif
#ifdef REMOTE_MANAGEMENT_ENABLE
			//for Remote Management channel 0/16
#ifndef RM_TEST
			if ((vpi==0)&&(vci==16)) {
#else			
			if ((vpi==1)&&(vci==39)) {
#endif				
			    struct atm_vcc * atm_vcc = cp->vcc[ch_no-1].dev_data;
				OutBandAAL5Rx(atm_vcc, (unsigned char *)(RVDesc->START_ADDR| Uncache_Mask));
			};
#endif					
					
			

		/* Restore LEN and CMD field */
			RVDesc->LEN=SAR_OAM_Buffer_Size;	
			RVDesc->STS= ((j!=(SAR_RX_DESC_NUM-1))?OWN:(OWN|EOR));
#endif	// #if 0
		/* Next Descriptor */
		//cp->vcc[ch_no].RV.desc_pr = j = (1 + j)%SAR_RX_DESC_NUM;		
		j = (1 + j)&(SAR_RX_DESC_NUM-1);

		
		//k= GetRxCDOI(cp, ch_no);
	}	/* end of while */
	
	/* Restore Descriptor Index */
	cp->vcc[ch_no].RV.desc_pr = j;
	//cp->ProcessRcv = 0;
	cp->vcc[ch_no].stat.rcv_ok++;

	return SUCCESS;	

}
//#endif  // Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt

#ifdef FAST_ROUTE_Test

void learn_mac (uint8 *buf, int ch_no) {
	int i;

/*	
	printk("learn_mac got called MAC=%02x%02x%02x %02x%02x%02x  buf[0] & 0x01 = %02x\n", 
	buf[0],buf[1],buf[2],buf[3],buf[4],buf[5], buf[0] & 0x01);
*/	
	// check if pkt is boardcast or multicast
	if( (buf[0] & 0x01) == 0x00 )	// No, it's not board/multi-cast pkt
	{	
		//mac table full! restart record the mac address from index 0
		if(fast_route_ch_no[ch_no].index == MAX_MAC_ADDR-1 )
			fast_route_ch_no[ch_no].index = 0;

		// search mac table from 0 to 7 
		for(i=0; i < MAX_MAC_ADDR; i++) {
			if(!memcmp(buf, fast_route_ch_no[ch_no].mac[i], 6)) {
//				printk("Learning a MAC address on CH %d\n", ch_no);
				break;
			}
		}
		// add a new one in mac table[index]
		if(i == MAX_MAC_ADDR ) {
			memcpy(fast_route_ch_no[ch_no].mac[ fast_route_ch_no[ch_no].index++], buf, 6);
/*			
			printk("Added one MAC address %02x %02x %02x %02x %02x %02x CH %d index %d\n",
				buf[0],buf[1],buf[2],buf[3],buf[4],buf[5], ch_no, fast_route_ch_no[ch_no].index-1);
*/			
		}

	}
}

unsigned int found_counter=9;
unsigned int find_mac( uint8 *buf)
{
	unsigned int i,j, ch_no=-1;
	
/*	
	printk("Finding MAC address %02x %02x %02x %02x %02x %02x \n",
		buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
*/	
	if( (buf[0] & 0x01) == 0x00 ) {
		for(i=0; i< Enable_VC_CNT; i++) {
			for(j=0; j< MAX_MAC_ADDR; j++) {
				if(!memcmp(buf, fast_route_ch_no[i].mac[j], 6)	) {
					ch_no = i;
					if(found_counter  != ch_no) {						
						/*
						printk("Found MAC address %02x %02x %02x %02x %02x %02x on CH %d\n",
						buf[0],buf[1],buf[2],buf[3],buf[4],buf[5], ch_no);
						*/
						found_counter = ch_no;
					}
					break;
				}
			}
		}	
	}
	if(ch_no != -1) 		
		return ch_no;
/*	
	printk("Cannot found MAC address %02x %02x %02x %02x %02x %02x, using default CH 0\n",
		buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
*/
	return -1;	//cannot find it
}

void print_mac_tbl(void) {
	int i,j;
	//char buf[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
	printk("FAST_ROUTE_Test mode %d\n", fast_route_mode);
		for(i=0; i< Enable_VC_CNT; i++) 
		{
			printk("CH %d\nindex: %d enable %d\n", i, fast_route_ch_no[i].index, fast_route_ch_no[i].enable);
			for(j=0; j< MAX_MAC_ADDR; j++) 
			{
						printk("%02x %02x %02x %02x %02x %02x\n",
							 fast_route_ch_no[i].mac[j][0],
							 fast_route_ch_no[i].mac[j][1],
							 fast_route_ch_no[i].mac[j][2],
							 fast_route_ch_no[i].mac[j][3],
							 fast_route_ch_no[i].mac[j][4],							 
							 fast_route_ch_no[i].mac[j][5] );
			}
		}
}

extern void nic_tx(struct sk_buff *skb);

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
void sar_tx(struct sk_buff *skb, unsigned int ch_no)
{

	struct atm_vcc * atm_vcc =  sar_dev->vcc[ch_no].dev_data;

	// Add LLCHdr802_3 header before transmitting
	memcpy(skb_push(skb, 0x0a), LLCHdr802_3, 0x0a);
local_irq_disable();	
	sar_send(atm_vcc, skb);
local_irq_enable();	
}
#endif

#ifdef ZTE_531B_TEST
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
void sar_tasklet_schedule(sar_private *cp)
{
	tasklet_hi_schedule(&cp->tasklets);
}

//1__SWAP
// Kaohj
/*
 * Process received packets; Function relative to external switch.
 */
int sar_rx_sw(struct atm_vcc *vcc, struct sk_buff *skb)
{
	sar_private *cp = RTATM_DEV(vcc->dev);
	sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);
	int ch_no;
	int encap_mode;
	
	ch_no = vcc_dev->ch_no;
	encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;
	// Kaohj -- for testing; open this to add vlan tag on rx
	#if 0
	{
		int i, copy_len;
		unsigned char tag_info[4]={0x81, 0x00, 0x00, 0x64};
		
		copy_len = FrameHeaderSize_1483[encap_mode]+12;
		skb_push(skb, 4);
		for (i=0; i<copy_len; i++)
			skb->data[i] = skb->data[i+4];
		memcpy(&skb->data[copy_len], tag_info, 4);
	}
	#endif
	// Port-Mapping: set the membership that this packet belongs to
	// This information should be passed to Ethernet
	skb->vlan_member = vcc->ifgrp.member;

	/*wt-146, ethprio*/
	if(vcc->pvcvlan.vlan) 
	{
		// 1483_br_llc && vlan tag
 		if (cp->vcc[ch_no].rfc == RFC1483_BRIDGED && *(unsigned short *)(skb->data+FrameHeaderSize_1483[encap_mode]+12) == 0x8100) {
			unsigned short vtag;
			vtag = *(unsigned short *)(skb->data+FrameHeaderSize_1483[encap_mode]+14) ;
			/*linux-2.6.19*/
			//skb->nfmark=vtag>>13;
			skb->mark=vtag>>13;
 		}
	}

	// 1483_br_llc && vlan tag
	if (enable_vlan_grouping && cp->vcc[ch_no].rfc == RFC1483_BRIDGED && *(unsigned short *)(skb->data+FrameHeaderSize_1483[encap_mode]+12) == 0x8100) {
		// Kaohj --- find vlan (pvid)
		unsigned short vtag;
		int vid;
		
		// find associated VLAN
		vtag = *(unsigned short *)(skb->data+FrameHeaderSize_1483[encap_mode]+14) ;
		vid = vtag & 0x0fff;
		skb->pvid = get_pvid(vid);
	}
	
	return 1;
}

void sar2nic(sar_private *cp, struct sk_buff *skb, int ch_no, int16 total_len)
{
	struct atm_vcc * atm_vcc = cp->vcc[ch_no].dev_data;

#ifdef FAST_ROUTE_Test
	// Kaohj
	// in normal path; go fast-bridge next time if in fast-bridge
	fast_frame_up = 0;
#endif
	skb->dev = cp->dev;
	cp->vcc[ch_no].stat.rx_byte_cnt += (int)total_len;
	//skb->tstamp = xtime;
	sar_rx_sw(atm_vcc, skb);
	
	skb_pvc_debug(skb, ch_no, 0);
	
	// end of our responsability
	atm_vcc->push (atm_vcc, skb);
}
#endif

//for small pkt
int rx_small_pkt=0;
//static int maxRdaCount=15;
__IRAM_SAR static int sar_rx (sar_private *cp, int ch_no)
{
	DRV_ENTER

	int8 		j,enable_j;
	int16	total_len;
	//BOOL	rc= TRUE;
	int32		status;
	RV_STS_DESC	*RVDesc,*enable_RVDesc;
	struct sk_buff 	*skb, *tmpskb;
	int rx_num=0;
	//static int rdaCount=0;
//#ifdef CONFIG_EXT_SWITCH
	//int encap_mode;
//#endif
#ifdef FAST_ROUTE_Test
static unsigned int fastBridgeEnable =0, fastBridgeStartStamp=0, fastBridgeSmallPktCount=0;
#endif  //FAST_ROUTE_Test
	j=cp->vcc[ch_no].RV.desc_pr;
	enable_j=(j+current_desc_number[ch_no])&(SAR_RX_RING_SIZE-1);
	RVDesc = &cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j];
	enable_RVDesc= &cp->RVDesc[ch_no*SAR_RX_RING_SIZE+enable_j];

//jim add to avoid BUG() called when skb==NULL, caused by sar_closed and interrupted to process rx action...
	if(cp->vcc[ch_no].created == VC_NOT_CREATED)
		return 0;
	//cp->vcc[ch_no].stat.rcv_cnt++;

	/* working on descriptors */
	//while(((j!=k)||(!(cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j].STS&OWN)))&&(i<SAR_MAX_Process_DESC)){
	while(!((status=(int32)RVDesc->STS)&OWN)){
		int len_err;
		//if(j==63) break; //for debug
		/* this descriptor is still own by hardware, exit this loop */
		// JONAH_DEBUG open
		//if(cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j].STS&OWN)
		//	break;

		////patch to increase interrupt times.
		//if (rdaCount++ > maxRdaCount) {
		//	Enable_IERDA(sar_dev);
		//	rdaCount = 0;
		//}

		/* Drop OAM cell now */
			/* Handle this Descriptor, get recv skb */
			skb=(struct sk_buff *)cp->vcc[ch_no].rx_buf[j];
			if (!skb)
				BUG();
		
#if 0			
			if(skb==(struct sk_buff *)NULL) {
				/* it mustn't happen, maybe we should reboot */
				goto Next_Desc;
			}
#endif			
	
		if((status&(FS|LS))==(FS|LS)){	/* One descriptor contains one complete packet */

			//cp->vcc[ch_no].stat.rx_FS_cnt++;
			//cp->vcc[ch_no].stat.rx_LS_cnt++;
			//cp->vcc[ch_no].stat.rx_desc_cnt++;
			
			if(status&LENErr) {
				len_err = 1;
				total_len = RVDesc->BPC_LENGTH;
			}
			else  {		
				len_err = 0;
				total_len = (RVDesc->LEN&LEN_Mask);
			}
			
			//for small pkt
			if(total_len<256)
				rx_small_pkt++;
			//if (total_len > 1518){
			//	cp->vcc[ch_no].stat.rx_lenb_error;
    		//		goto Error_Desc;
    		//      sar_rx_error_desc(cp, ch_no, RVDesc, skb, &j);
    		//      continue;
    		//	}

			if(total_len <= 0){
				//goto Error_Desc;
#ifndef ZTE_531B_TEST
    		    sar_rx_error_desc(cp, ch_no, RVDesc, skb, &j,enable_RVDesc,&enable_j);
				RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j];
				enable_RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+enable_j];
#else
				sar_rx_err_process(cp, ch_no, &RVDesc, skb, &j, &enable_RVDesc,&enable_j);
#endif
    		    continue;
			}

			if(status&CRC32Err){
				cp->vcc[ch_no].stat.rx_crc_error++;
				//goto Error_Desc;
#ifndef ZTE_531B_TEST
    		    sar_rx_error_desc(cp, ch_no, RVDesc, skb, &j,enable_RVDesc,&enable_j);
				//printk("after err:%d          %x\n",j,&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j]);
				RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j];
				enable_RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+enable_j];
#else
				sar_rx_err_process(cp, ch_no, &RVDesc, skb, &j, &enable_RVDesc,&enable_j);
#endif
    		    continue;
			}
#ifdef CONFIG_SAR_SHARE_PRIV_SKB_WITH_ETH
			tmpskb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
#else
			tmpskb = dev_alloc_skb(SAR_RX_Buffer_Size);
#endif //CONFIG_SAR_SHARE_PRIV_SKB_WITH_ETH
			//tmpskb = rtl8671_getPreAlloc();
			if (unlikely(tmpskb==NULL)){
				//cp->vcc[ch_no].stat.rx_buf_lack++;
				//mark_bh(SAR_BH);
				//queue_task (&cp->bh, &tq_immediate);
				//mark_bh (IMMEDIATE_BH);
				//goto Error_Desc;
    		    //sar_rx_error_desc(cp, ch_no, RVDesc, skb, &j);
				cp->RDAI_Reg |= 1<<ch_no;
#ifndef ZTE_531B_TEST
				tasklet_hi_schedule(&cp->tasklets);
#else
				sar_tasklet_schedule(cp);
#endif
				break;
			}

			/* Handle Word Insertion */
			if(status&WII){
				skb_reserve(skb, 2);
				if (!len_err)
				total_len-=2;
			}
           
           //1/5/05' tylo, pid=0x0001, discard CRC
           if(cp->vcc[ch_no].rfc==RFC1483_BRIDGED &&
           	cp->vcc[ch_no].framing==LLC_SNAP &&
           	skb->data[7]==0x01)
           	total_len-=4;

			//skb_reserve(skb, FrameHeaderSize_1483[encap_mode]);
#if 0
		#ifdef LoopBack_Test
			total_len=Lying_Engine(skb->data, total_len);
			/*exchange_mac(skb->data);*/
		#endif	
#endif
			skb_put(skb, total_len);

#ifdef CONFIG_PORT_MIRROR        
		sar_port_mirror(cp, ch_no, skb, PORT_MIRROR_IN);
#endif

#ifdef FAST_ROUTE_Test
			
			if (fast_route_mode&&cp->vcc[ch_no].br ) {				

				// deals with the tiny pkt (specially 64 and 128 bytes)
				if(skb->len < 300) {				
					
					//Not specific pkt type, this is a test pkt and just bridge it					
					if((*(unsigned short *)(skb->data+0x16))==0x0000) {
						fastBridgeEnable = 1;
						fastBridgeSmallPktCount = 0;
					} else
 
					{
						if(fastBridgeStartStamp==0) 
							fastBridgeStartStamp = jiffies;

						if(fastBridgeSmallPktCount++ > 20) {
							if(jiffies - fastBridgeStartStamp <= 1) {
								fastBridgeEnable = 1;
								fastBridgeSmallPktCount = 0;	//shlee
							} else {
								fastBridgeStartStamp = jiffies;
								fastBridgeSmallPktCount = 0;
								fastBridgeEnable =0;
							}
						}
					}

				} else {
					fastBridgeEnable = 0;
					fastBridgeStartStamp = 0;
					fastBridgeSmallPktCount = 0;					
				}
			}
			//if(fastBridgeEnable) 
			if(fastBridgeEnable && !fast_frame_up) 
			{
				#ifdef CONFIG_ETHWAN
				extern void nic_tx2(struct sk_buff *skb, struct net_device *tdev);
				#else
				extern void nic_tx2(struct sk_buff *skb);
				#endif
				//skb->dev = eth_net_dev;
				//skb->pkt_type = PACKET_FASTROUTE;
				if (skb->data[0]==0xaa)
					skb_pull(skb, 10);
				//printk("fix me nic_tx2()!");
				skb->fastbr =1;
				#ifdef CONFIG_ETHWAN
				nic_tx2(skb, NULL);
				#else
				nic_tx2(skb);
				#endif
			}else {
#endif	//FAST_ROUTE_Test
			sar2nic(cp, skb, ch_no, total_len);
#ifdef FAST_ROUTE_Test
			}
#endif
			
		}else {
			//goto Error_Desc;
#ifndef ZTE_531B_TEST
    		sar_rx_error_desc(cp, ch_no, RVDesc, skb, &j,enable_RVDesc,&enable_j);
			RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j];
			enable_RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+enable_j];
#else
			sar_rx_err_process(cp, ch_no, &RVDesc, skb, &j, &enable_RVDesc,&enable_j);
#endif
    		continue;
		}

		//cp->vcc[ch_no].stat.rx_desc_ok_cnt++;
		rx_num++;

		/* mask EOR and STS, clear OWN, FS, LS */
		//RVDesc->STS &= 0x4FFF;
		//goto Next_Desc;			
//Error_Desc:

//Next_Desc:	


//jim: we should invalidate cache to sure accessing DMAed data through cache.
		dma_cache_wback_inv((unsigned long)tmpskb->data, SAR_RX_Buffer_Size);
#if 0
		/* Fill Descriptor with new skb */
		RVDesc->START_ADDR=(UINT32)tmpskb->data|Uncache_Mask;
		/* Restore LEN and CMD field */
		RVDesc->LEN=SAR_RX_Buffer_Size;
		RVDesc->STS= ((j!=(SAR_RX_DESC_NUM-1))?OWN:(OWN|EOR));
		cp->vcc[ch_no].rx_buf[j]=(UINT32)tmpskb;
#else
		enable_RVDesc->START_ADDR=(UINT32)tmpskb->data | Uncache_Mask;
		enable_RVDesc->LEN=SAR_RX_Buffer_Size;
		cp->vcc[ch_no].rx_buf[enable_j]=(UINT32)tmpskb;
		enable_RVDesc->STS= ((enable_j!=(SAR_RX_DESC_NUM-1))?OWN:(OWN|EOR));
		cp->vcc[ch_no].rx_buf[j]=(UINT32)NULL;
		//printk("enable_j:%d    %x       %x\n",enable_j,enable_RVDesc->STS,tmpskb);
#endif
		/* Next Descriptor */
		//cp->vcc[ch_no].RV.desc_pr = j = (1 + j)%SAR_RX_DESC_NUM;		
		j = (1 + j)&(SAR_RX_DESC_NUM-1);
		//printk("next j:%d\n",j);
		enable_j=(1 + enable_j)&(SAR_RX_DESC_NUM-1);
		RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j];
		enable_RVDesc=&cp->RVDesc[ch_no*SAR_RX_RING_SIZE+enable_j];
	    //RVDesc = (j==0)?(&cp->RVDesc[ch_no*SAR_RX_RING_SIZE]):(RVDesc+1);
	    //enable_RVDesc= (enable_j==0)?(&cp->RVDesc[ch_no*SAR_RX_RING_SIZE]):(enable_RVDesc+1);
		//k= GetRxCDOI(cp, ch_no);
	}	/* end of while */
	
	/* Restore Descriptor Index */
	cp->vcc[ch_no].RV.desc_pr = j;
	//cp->ProcessRcv = 0;
	//cp->vcc[ch_no].stat.rcv_ok++;
	cp->vcc[ch_no].stat.rx_pkt_cnt+=rx_num;
	AtmTraffic += rx_num;	// Rx traffic tracks
	return rx_num;	

}
int sar_rbf=0;
//1__SWAP
__IRAM_SYS_MIDDLE void sar_rx_bh(sar_private *cp){
	int	ch_no;
	unsigned int RDAI_Reg, RBFI_Reg;
	unsigned long flags;

	spin_lock_irqsave(&cp->lock, flags);
	RDAI_Reg = cp->RDAI_Reg;
	RBFI_Reg = cp->RBFI_Reg;
	cp->RDAI_Reg = 0;
	cp->RBFI_Reg = 0;
	spin_unlock_irqrestore (&cp->lock, flags);
	if (RDAI_Reg&0x10000) {  //Out band channel
		sar_rx_oam2(cp, OAM_CH_NO);
		RDAI_Reg&=~0x10000;
	};
	
	// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
#ifdef CONFIG_RTL8672	
	if (RDAI_Reg&0x20000) {  //Out band channel 1
		printk("Recieve NonOAM packet from OB ch1\n");
		sar_rx_oam2(cp, OAM_CH_NO+1);
		RDAI_Reg&=~0x20000;
	};
#endif

	if(RDAI_Reg) {
		int rx_num;
#ifdef SAR_RX_FORMATION
		int pvc_rx_busy=0;
		int reformat_shape=FORMATION_NO_CHANGE; //do not change formation in default
#endif
		for(ch_no=0;ch_no<total_pvc_number;ch_no++){
			if(RDAI_Reg&((UINT32)0x00000001) &&cp->vcc[ch_no].created == VC_CREATED) {	// avoid receiveing NULL skb after delete VC
				rx_num=sar_rx(cp, ch_no);
#ifdef SAR_RX_FORMATION
				if (do_formation) {
					if (rx_num>1) {  //rx pkt number is large enough, candidate for desc increase
						if (busy_channel[ch_no]==0) {  //this channel is not busy last time, we have to increase desc for it
							reformat_shape = FORMATION_CHANGE;
							//printk("BE %d %d\n",ch_no,rx_num);
						} else {
							//this channel is really busy and we have increase desc for it last time, leave it alone
							//printk("BN %d %d\n",ch_no,rx_num);
						};
						busy_channel[ch_no] = rx_num;
						pvc_rx_busy++;
					} else {  //rx pkt number is not large enough, candidate for desc reduce
						if (busy_channel[ch_no]!=0) {  //this channel is not busy, decrease desc for it
							if (rx_num>0) {  //if no rx pkt, it is a dummy alert and do no change formation
								reformat_shape = FORMATION_CHANGE;
								//printk("LR0 %d %d\n",ch_no,rx_num);
							};
						} else {
							//this channel is not busy, decrease desc for it
							//printk("LR1 %d %d\n",ch_no,rx_num);
						};
						busy_channel[ch_no] = 0;
					};
				} // of do_formation
#endif // of SAR_RX_FORMATION
				cp->vcc[ch_no].RDA_cnt++;
			};
			RDAI_Reg>>=1;
		};
#ifdef SAR_RX_FORMATION
		if (do_formation) {
			if ( (FORMATION_CHANGE==reformat_shape)&&(0!=pvc_rx_busy) ) {
				//when 0==pvc_rx_busy, it means current formation can handle slow rx traffic. no need
				//formation change.
				int busy_ch_avg_desc_num;
				busy_ch_avg_desc_num=(SAR_MAX_Process_DESC-SAR_RX_DESC_LOW_LIMIT*total_pvc_number)/pvc_rx_busy+SAR_RX_DESC_LOW_LIMIT;
				for(ch_no=0;ch_no<total_pvc_number;ch_no++){
					if (0==change_formation(cp, ch_no, busy_channel[ch_no], busy_ch_avg_desc_num)) {
						//can't allocate enough desc, leave it to next run
						busy_channel[ch_no] = 0;
					};
				};
			};
		} // of do_formation
#endif // of SAR_RX_FORMATION
	}
	/* handle RBF */
	if(RBFI_Reg){
#if 0			
		ch_no=0;
		while(!(RBFI_Reg&(0x00000001<<ch_no))){
			ch_no++;
		}
		if (ch_no!=HW_OAM_CH_NO &&cp->vcc[ch_no].created == VC_CREATED) sar_rx(cp, ch_no);		// avoid rx NULL skb after delete vc
		else {  //ch_no=16			
			sar_rx_oam2(cp, OAM_CH_NO);
			ch_no=OAM_CH_NO;
		};
		Clear_RBF(cp, ch_no, GetRxCDOI(cp, ch_no));		

#else
		/* Check each CH */
		if (RBFI_Reg&0x10000) {  //Out band channel
       			sar_rx_oam2(cp, OAM_CH_NO);
       			Clear_RBF(cp, OAM_CH_NO, 0); 	
    		}
		
		for(ch_no=0;ch_no<total_pvc_number;ch_no++){
		    if(RBFI_Reg&((UINT32)0x00000001) &&cp->vcc[ch_no].created == VC_CREATED) {	// avoid receiveing NULL skb after delete VC
			sar_rx(cp, ch_no);		
			Clear_RBF(cp, ch_no, 0); 				
	    	    }
		    RBFI_Reg>>=1;
		}

		//continue to handle RBF, avoid sar rx stopping!!
		if((cp->RBFI_Reg|=reg(SAR_RBFI_Addr))!=0)
			tasklet_hi_schedule(&cp->tasklets);
		//Re-enable RBF int 
		Enable_IERBF(sar_dev);	
#endif
	}

	//7/25/05' hrchen, SAR_IRQ will be disabled if RDU happen. Open SAR_IRQ here.
	//enable_lx4180_irq(cp->irq);
//scout(']');		
	//tylo, fpga        
	//shlee 2.6 REG32(GIMR) |= SAR_IE ;
	//enable_irq(sar_irq);	//shlee 2.6
	//Enable_IERBF(sar_dev);	

	if(sar_rbf==1){
		for(ch_no=0;ch_no<total_pvc_number;ch_no++){
			REG32(RV_Ctrl_Addr+0x10*ch_no) &= (~CFD);	
		}
		sar_rbf=0;	
	}
	
	//Sachem disable cell discard
//	REG16(0xb8000c00) = 0x5023;

	//Enable_IERDA(sar_dev);
	return;     
}

extern char use_low_rate;
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
irqreturn_t sar_interrupt (int irq, void *dev_instance)
{
	DRV_ENTER

	//struct atm_dev *dev = dev_instance;
	sar_private *cp = RTATM_DEV((struct atm_dev *)dev_instance);
	INT32		ch_no;

//scout('3');
	
//shlee 1116 mask all interrupts about SAR
/*
cp->CNFG_Reg = reg(SAR_CNFG_Addr);
reg(SAR_CNFG_Addr)&=~(IERBF|IERDA|IERTO|IETBE|IETDF);
*/
//shlee 1116

	cp->STS_Reg=reg(SAR_STS_Addr);
	//spin_lock(&cp->lock);

	/* TDF */

	/* TBE */	
	if((cp->TBEI_Reg=reg(SAR_TBEI_Addr)!=0)){
//		printk("Deal with TBEI\n");
		for(ch_no=0;ch_no<(Enable_VC_CNT+1);ch_no++){
			if(!(cp->TBEI_Reg&((UINT32)0x00000001<<ch_no)))	continue;
			/*
			cp->vcc[ch_no].TBE_cnt++;
			cp->vcc[ch_no].TBE_Flag=TRUE;
			*/
			Clear_TBE(ch_no, 0);
		}
	}


	/* RBF */
	if(cp->STS_Reg&RBF){

		#if 0 //8672 
		//Sachem cell discard enable
		REG16(0xb8000c00) = 0x50a3;
		if(sar_rbf==0){
			//enable CFD
			for(ch_no=0;ch_no<total_pvc_number;ch_no++){
				REG32(RV_Ctrl_Addr+0x10*ch_no) |= CFD;	
			}
			
			sar_rbf=1;
		}
		
        #endif 
	//	printk("RBF!!\n");
            // Disable RBF int (sar_rx will re-enable it)
	    Disable_IERBF(sar_dev);	
	    if((cp->RBFI_Reg|=reg(SAR_RBFI_Addr))){

		//7/25/05' hrchen, stop SAR IRQ, re-enabled in sar_rx_bh()
	    //disable_lx4180_irq(cp->irq);
	    //tylo, fpga
            //kernel irq_dispatch will ack this irq as disabled	    
            //if(REG32(GIMR)&SAR_IE)
            //	REG32(GIMR) &= ~SAR_IE ;
            /*linux-2.6.19*/
            //if(REG32(BSP_GIMR)&BSP_SAR_IE)
            //	REG32(BSP_GIMR) &= ~BSP_SAR_IE ;

		//mark_bh(SAR_BH);
		//queue_task (&cp->bh, &tq_immediate);
		//mark_bh (IMMEDIATE_BH);
//scout('f');				
		tasklet_hi_schedule(&cp->tasklets);
		//sar_rx_bh(cp);
				
		/* handle RX buffer first */
		//rc = sar_rx(cp, ch_no);

		//cdoi= GetRxCDOI(cp, ch_no);
		/* Then clear RBF */
		//if(rc==TRUE)	Clear_RBF(ch_no, cdoi);
		//else			Cell_Forced_Dropped(ch_no);
	    };
	}

	/* RDA */
	if(cp->STS_Reg&RDA) {
		unsigned int RDAI_Reg;
	    if((RDAI_Reg=reg(SAR_RDAI_Addr))){
	    	reg(SAR_RDAI_Addr) = RDAI_Reg;	//write to clear RDAI reg
			cp->RDAI_Reg |= RDAI_Reg;
		//7/25/05' hrchen, stop SAR IRQ, re-enabled in sar_rx_bh()
	    //disable_lx4180_irq(cp->irq);
	    //tylo, fpga
            //kernel irq_dispatch will ack this irq as disabled
            //if(REG32(GIMR)&SAR_IE)
            // 	REG32(GIMR) &= ~SAR_IE ;
            /*linux-2.6.19*/
            //if(REG32(BSP_GIMR)&BSP_SAR_IE)
            //	REG32(BSP_GIMR) &= ~BSP_SAR_IE ;


		//mark_bh(SAR_BH);
		//queue_task (&cp->bh, &tq_immediate);
		//mark_bh (IMMEDIATE_BH);
//scout('r');		
		tasklet_hi_schedule(&cp->tasklets);
		//sar_rx_bh(cp);
	    };
	}
      	reg(SAR_STS_Addr) = cp->STS_Reg;		//shlee 1115 clear SAR_STS
        //shlee 1116 resume enabled interrupts about SAR
/*        
        reg(SAR_CNFG_Addr) = cp->CNFG_Reg;
*/        
        //shlee 1116

	//spin_unlock(&cp->lock);
	return IRQ_RETVAL(1);
}

//#ifdef CONFIG_EXT_SWITCH
//return 0: drop pkt
//2__SWAP
struct sk_buff *sar_send_vlan(struct atm_vcc *vcc,sar_private *cp,int ch_no, struct sk_buff *skb)
{
	INT8		encap_mode ;
	char *vlan_p, *ether_type;
	struct sk_buff *skb_copy;
	
	// Kaohj, copy-on-write this skb
	if ((skb_copy=dev_alloc_skb(SAR_TX_Buffer_Size))==NULL)
		return 0;
	//vlan tagging
	encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;
	ether_type = skb->data + FrameHeaderSize_1483[encap_mode]+12;
	
	if(*(unsigned short *)ether_type == 0x8100){
		memcpy(skb_put(skb_copy, skb->len), skb->data, skb->len);
		vlan_p = skb_copy->data + FrameHeaderSize_1483[encap_mode]+12;
		vlan_p+=2;
		*(unsigned short *)vlan_p = 0;
		// 3 bits priority
		#ifdef CONFIG_NETFILTER
		//*(unsigned short *)vlan_p |= ((skb->nfmark&0x7)<<13);
		/*linux-2.6.19*/ 
		if((skb->mark&0xffff)>>8>=1){
			*(unsigned short *)vlan_p |= ((skb->mark&0x7)<<13);
		}
		else
		#endif
		{
			if(vcc->pvcvlan.vlan_prio!=0)
				*(unsigned short *)vlan_p |= ((vcc->pvcvlan.vlan_prio-1) &0x7);
		}
		// 12 bits vid
		*(unsigned short *)vlan_p |= (vcc->pvcvlan.vid & 0x0fff);
	}
	else{	
		int copy_len;
		//printk("hsize=%d\n", FrameHeaderSize_1483[encap_mode]);
		// add fram_header(rfc1483)+ethernet_header and reserve vlan 4-byte
		copy_len = FrameHeaderSize_1483[encap_mode]+12;
		memcpy(skb_put(skb_copy, copy_len+4), skb->data, copy_len);
		// add ether_type(2 bytes)+the reset of packet data(ip packet)
		copy_len = skb->len-FrameHeaderSize_1483[encap_mode]-12;
		memcpy(skb_put(skb_copy, copy_len), ether_type, copy_len);
		vlan_p = skb_copy->data + FrameHeaderSize_1483[encap_mode]+12;
		*(unsigned short *)vlan_p = 0x8100;
		vlan_p+=2;
		*(unsigned short *)vlan_p = 0;
// Kaohj -- Add E8B support
#ifdef CONFIG_E8B
		// 3 bits priority: if bit-16 is set on skb->mark, use value in skb->mark;
		// otherwise use value of per pvc's if applicable.
		#ifdef CONFIG_NETFILTER
		if (skb->mark & (1<<16)) { // marked by IPQoS
			*(unsigned short *)vlan_p |= ((skb->mark&0x7)<<13);
		} else {
			if (vcc->pvcvlan.vlan_prio)
				*(unsigned short *)vlan_p |= (((vcc->pvcvlan.vlan_prio-1) &0x7)<<13);
		}
		#endif
		// 12 bits vid
		*(unsigned short *)vlan_p |= (vcc->pvcvlan.vid & 0x0fff);
		//printk("vtag=%x\n", *(unsigned short *)vlan_p);
#else
		// 3 bits priority
		#ifdef CONFIG_NETFILTER
		//*(unsigned short *)vlan_p |= ((skb->nfmark&0x7)<<13);
		/*linux-2.6.19*/
		if((skb->mark&0xffff)>>8>=1){
			*(unsigned short *)vlan_p |= ((skb->mark&0x7)<<13);
		}
		else	
		#endif
		{
			if(vcc->pvcvlan.vlan_prio!=0)
				*(unsigned short *)vlan_p |= (((vcc->pvcvlan.vlan_prio-1) &0x7)<<13);
		}
		// 12 bits vid
		*(unsigned short *)vlan_p |= (vcc->pvcvlan.vid & 0x0fff);
		//printk("vtag=%x\n", *(unsigned short *)vlan_p);
#endif
	}
	return skb_copy;
}

// Kaohj -- for vlan-grouping
int get_vid(int pvid);

/*
 *	Return value:
 *	0: drop it
 *	1: no changes, keep going
 *	other: new skb
 */
struct sk_buff *sar_send_vlan2(struct atm_vcc *vcc,sar_private *cp,int ch_no, struct sk_buff *skb)
{
	INT8		encap_mode ;
	char *vlan_p, *ether_type;
	struct sk_buff *skb_copy;
	int vid;
	
	if (skb->pvid == 5) // default group
		return (struct sk_buff *)1;
	vid = get_vid(skb->pvid);
	//printk("sar_send: pvid=%d, vid=%d\n", skb->pvid, vid);
	if (vid<=0)
		return (struct sk_buff *)1;
	encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;
	// Kaohj -- at least we have Ethernet header
	if (skb->len <= (FrameHeaderSize_1483[encap_mode]+14))
		return (struct sk_buff *)0;
	// Kaohj, copy-on-write this skb
	if ((skb_copy=dev_alloc_skb(SAR_TX_Buffer_Size))==NULL)
		return (struct sk_buff *)0;
	
	//vlan tagging
	//encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;
	ether_type = skb->data + FrameHeaderSize_1483[encap_mode]+12;
	
	if(*(unsigned short *)ether_type == 0x8100){
		memcpy(skb_put(skb_copy, skb->len), skb->data, skb->len);
		vlan_p = skb_copy->data + FrameHeaderSize_1483[encap_mode]+12;
		vlan_p+=2;
		*(unsigned short *)vlan_p = 0;
		// 3 bits priority
		//*(unsigned short *)vlan_p |= ((skb->nfmark&0x7)<<13);
		// 12 bits vid
		*(unsigned short *)vlan_p |= (vid & 0x0fff);
	}
	else{	
		int copy_len;
		//printk("hsize=%d\n", FrameHeaderSize_1483[encap_mode]);
		// add fram_header(rfc1483)+ethernet_header and reserve vlan 4-byte
		copy_len = FrameHeaderSize_1483[encap_mode]+12;
		memcpy(skb_put(skb_copy, copy_len+4), skb->data, copy_len);
		// add ether_type(2 bytes)+the reset of packet data(ip packet)
		copy_len = skb->len-FrameHeaderSize_1483[encap_mode]-12;
		memcpy(skb_put(skb_copy, copy_len), ether_type, copy_len);
		vlan_p = skb_copy->data + FrameHeaderSize_1483[encap_mode]+12;
		*(unsigned short *)vlan_p = 0x8100;
		vlan_p+=2;
		*(unsigned short *)vlan_p = 0;
		// 3 bits priority
		//*(unsigned short *)vlan_p |= ((skb->nfmark&0x7)<<13);
		// 12 bits vid
		*(unsigned short *)vlan_p |= (vid & 0x0fff);
		//printk("vtag=%x\n", *(unsigned short *)vlan_p);
	}
	
	return skb_copy;
}


struct sk_buff *pp_sar_send_vlan(int ch_no, struct sk_buff *skb){
	sar_private *cp = sar_dev;
	struct atm_vcc *vcc;
    	vcc = cp->vcc[ch_no].dev_data;

    	return sar_send_vlan(vcc,cp,ch_no,skb);
}
//#endif

#ifdef AUTO_PVC_SEARCH
/*
 * sar_send_auto
 *	Clone the incoming pkt and send them to the every possible PVC 
 */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
int 	sar_send_auto (struct atm_vcc *vcc,struct sk_buff *skb){
	int count, ch_no;
	volatile int loop;
	sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);
	sar_private *cp = RTATM_DEV(vcc->dev);
	struct sk_buff *tmp_skb;
	ch_no = vcc_dev->ch_no;  	
	loop = 0;

	if ((found_pvc==0) && (ch_no < 6) && auto_search_start) { // auto search on.

		for(count=0;count<tbl_counter;count++){   		    
			int vci, vpi;

			vpi = getActiveVPINum(count);
			vci = getActiveVCINum(count);
			if ((vpi==-1)&&(vci==-1)) continue;
						
			SetVpiVci(cp,vpi, vci,ch_no);

			tmp_skb = skb_clone(skb,GFP_ATOMIC);
			if(count==0)
				tmp_skb->truesize=skb->truesize;
			else
				tmp_skb->truesize=0;
			mdelay(2);
			sar_send(vcc, tmp_skb);

         
			for (loop = 0; loop <= (150000000/(1000*3)); loop++) {
				// delay 1ms    
				;
			}
		}
		dev_kfree_skb_irq(skb);
		return 0;
	} else {
		return sar_send(vcc, skb);
	}
}
#endif
extern int sendToPort0;
#ifdef CONFIG_FLOW_CTRL
unsigned int FCFlag;
unsigned int FCEnable=0;
unsigned int FCCtrl=1;
unsigned int FC_low=2;
unsigned int FC_high=60;
#endif	//CONFIG_FLOW_CTRL
// Kaohj
/* Packets prepared to be sent; Function relative to external switch.
 * Return value:
 * 0: Drop
 * 1: Continue
 */
int sar_send_sw(struct atm_vcc *vcc, struct sk_buff **skb)
{
	sar_private *cp = RTATM_DEV(vcc->dev);
	sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);
	int ch_no;
	struct sk_buff 	*skb_to_send;
	struct sk_buff 	*tmpskb;
	
	ch_no = vcc_dev->ch_no;
	skb_to_send = *skb;
//#ifdef CONFIG_EXT_SWITCH
	/*
	* The bitmap for interface should follow this convention commonly shared
	* by user-space program (say, startup.c: setupEth2pvc())
	* Bit-map:  bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
	*           wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
	*/
#ifndef NEW_PORTMAPPING
	if (enable_port_mapping && skb_to_send->vlan_member != 0) { // not from device
		if(vcc->ifgrp.flag && (vcc->ifgrp.member != skb_to_send->vlan_member)){
//			printk("Drop pkt!!vcc->ifgrp.member %x skb vlan member %x\n",vcc->ifgrp.member,skb_to_send->vlan_member);
			return 0;;
		}
	}
#endif
	if (enable_vlan_grouping && skb_to_send->pvid != 0) { // not from device
		// check vlan membership
		if( !(rtl8305_info.vlan[skb_to_send->pvid].member & (0x10000 << ch_no)) )
		{
			return 0;
		}
	}
	//printk("vlan=%d, vid=%d, vlan_prio=%d, vpass=%d\n", vcc->pvcvlan.vlan, vcc->pvcvlan.vid, vcc->pvcvlan.vlan_prio, vcc->pvcvlan.vlan_pass);
	if (vcc->pvcvlan.vlan) {
		//alex check tiny packet
		
		if(skb_to_send->len > 22){
			// Kaohj, we need to modify this packet, so copy-on-write this skb
			if ((tmpskb=sar_send_vlan(vcc, cp, ch_no, skb_to_send))==0)
				return 0;
			dev_kfree_skb(skb_to_send);
			*skb = tmpskb;
		}
	}
	// Kaohj -- for vlan-grouping
	else
	if (enable_vlan_grouping && cp->vcc[ch_no].rfc == RFC1483_BRIDGED) {
		if ((tmpskb=sar_send_vlan2(vcc, cp, ch_no, skb_to_send))==0)
				return 0;
		if (tmpskb != (struct sk_buff *)1) {
			dev_kfree_skb(skb_to_send);
			*skb = tmpskb;
		}
	}
//#endif
	return 1;
}
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
__SWAP
int _sar_send (struct atm_vcc *vcc,struct sk_buff *skb)
{
	DRV_ENTER

	sar_private *cp = RTATM_DEV(vcc->dev);
	sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);
	INT8				ch_no, CRC_Gen_offset ;
	INT16 			j, avail_desc;
	int			currCDOI;
	UINT16			CMD;
	struct sk_buff 	*skb_to_send;		
	unsigned long flags;

	TV_CMD_DESC	*TVDesc;
	//BOOL			insert_desc = FALSE;
#ifdef UPSTREAM_TRAFFIC_CTL
	//ql-- upstream traffic control
	static unsigned int uTcStamp1=0,uTcStamp2=0;
	static unsigned int uTcStamp=0;
	static unsigned int LB=0;
#define TIMER_HZ		(35328/19)
#define LEAKY_BUCKET	100000
#endif
	int tx_cell=0;
	struct net_device_stats *netstats;
	skb_to_send = skb;

//scout('(');
	// Casey, 2004/02/06, I'm not sure, if it should be added here
#ifdef FAST_ROUTE_Test
//	if (!fast_route_mode || !fast_route_ch_no[vcc_dev->ch_no].enable)
#endif
	//atomic_sub(skb_to_send->truesize, &vcc->tx_inuse);
	atomic_sub(skb_to_send->truesize, &sk_atm(vcc)->sk_wmem_alloc);	//maintain sending counter

	ch_no = vcc_dev->ch_no;

	spin_lock_irqsave(&cp->lock, flags);

#ifdef REMOTE_MANAGEMENT_ENABLE
	if (ch_no==OAM_CH_NO) {
		// Mason Yu abc
		//printk("vcc_dev->vpi=%d vcc_dev->vci=%d\n", vcc_dev->vpi, vcc_dev->vci);
		vcc_dev->vpi = 0;
		vcc_dev->vci = 16;

		//for Remote Management
		OutBandAAL5Tx(vcc_dev->vpi, vcc_dev->vci, skb_to_send);
		//rtl8671_putPreAlloc(skb_to_send);  //free skb
		spin_unlock_irqrestore (&cp->lock, flags);
		 DRV_LEAVE;
		return 0;
	};
#endif
#ifndef CONFIG_RTL8672_ATM_QoS

	if ((cp->QoS_Tx_Credit)&&(!vcc_dev->creditQoSTx)) {
		//if (vcc_dev->QoS.Type&2) {
		//	printk("VBR %d drop pkt\n", ch_no);
		//};
	    goto DropPacket;
	}
#endif

//#ifdef CONFIG_EXT_SWITCH
	if (!sar_send_sw(vcc, &skb_to_send))
		goto DropPacket;
//#endif
#if 0
#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
	if (enable_port_mapping && skb_to_send->vlan_member != 0) { // not from device
		if(vcc->ifgrp.flag && (vcc->ifgrp.member != skb_to_send->vlan_member)){
			//printk("Drop pkt!!vcc->ifgrp.member %x skb vlan member %x\n",vcc->ifgrp.member,skb_to_send->vlan_member);
			goto DropPacket;
		}
	}
#endif
#endif


	/* Clear transmitted descriptors */
	//ClearTxBuffer(cp, ch_no);

	/* Now For 1483 only */
	//encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;
#if 0
	/*if no descriptor available, clear TBE if needed */
	if(((((cp->vcc[ch_no].TV.desc_pw+1)%SAR_TX_DESC_NUM)==cp->vcc[ch_no].TV.desc_pf)&&
		(cp->TVDesc[ch_no*SAR_TX_RING_SIZE+cp->vcc[ch_no].TV.desc_pw].CMD&OWN)!=0)){

		cp->vcc[ch_no].stat.send_desc_full++;	/* descriptors full*/
		/* get current CDOI */
		currCDOI=GetTxCDOI(cp, ch_no);

		/* if TBE occurs and current descriptor is own by hardware (that means last time TBE occurs,
			and the desriptors has been refilled), clear TBE and make this channel be scheduled again */
		if ((reg(SAR_TBEI_Addr)&(0x00000001<<ch_no))&&((cp->TVDesc[ch_no*SAR_TX_RING_SIZE+currCDOI].CMD)&OWN))
			Clear_TBE(ch_no, currCDOI);
		//printk(KERN_ERR" no descriptor available\n");

		goto DropPacket;
	}
#endif
		//ql-- limit upstream traffic 
#ifdef UPSTREAM_TRAFFIC_CTL
		if (ucTcEbl == 1) {//enable upstream traffic control
			unsigned int uRate;
			unsigned int uCompTime;	//compensate time
			unsigned int uLeakyBytes;
			
			uTcStamp1 = uTcStamp2;
			uTcStamp2 = (*(volatile u32*)0xb9c0102c);
			uTcStamp2 = (uTcStamp2 >> 8) & 0xffffff;
			
			if (uTcStamp == 0)
				uTcStamp = jiffies;
			total_len = skb_to_send->len;
			
			uCompTime = jiffies - uTcStamp;
			uTcStamp = jiffies;
			
			if (uTcStamp1 >= uTcStamp2) {
				if (uCompTime >= 800)
					uCompTime = 800;
				else
					uCompTime = 0;
				uLeakyBytes = (uTcStamp1 - uTcStamp2)*uMaxTraffic/TIMER_HZ + 
							uCompTime*uMaxTraffic*10;
			} else {
				if (uCompTime >= 800)
					uCompTime = 801;
				else
					uCompTime = 1;
				uLeakyBytes = (uTc1Data - uTcStamp2 + uTcStamp1)*uMaxTraffic/TIMER_HZ +
							(uCompTime-1)*uMaxTraffic*10;
			}
			LB = (LB <= uLeakyBytes)?0:(LB-uLeakyBytes);
			
			//printk("uCompTime:%d uLeakyBytes:%d uTcStamp2:%d uTcStamp1:%d uTc0Data:%d\n", uCompTime, uLeakyBytes, 
			//	uTcStamp2,uTcStamp1, uTc1Data);
			
			if (total_len + LB > LB_len/*LEAKY_BUCKET*/) {
				goto DropPacket;
			} else if (total_len + LB > LB_MAX) {
				if (uTcStamp2 & 0x01 == 0x01)//drop packet according to 1/2 probability
					goto DropPacket;
			}  else if (total_len + LB > LB_SEC) {
				if (uTcStamp2 & 0x03 == 0x01)//drop packet according to 1/2 probability
					goto DropPacket;
			} else if (total_len + LB > LB_THD) {
				if (uTcStamp2 & 0x07 == 0x01)//drop packet according to 1/2 probability
					goto DropPacket;
			} else if (total_len + LB > LB_MIN) {//drop packet according to 1/8 probability
				if (uTcStamp2 & 0x0f == 0x01)
					goto DropPacket;
			}
			
			LB += total_len;
		}
#endif

	/* get fragment number */
	//if(skb_shinfo(skb_to_send)->nr_frags)
	//	printk("sar_send: skb need frag!\n");
	//avail_desc = (cp->vcc[ch_no].TV.desc_pf- 1 - cp->vcc[ch_no].TV.desc_pw)&(SAR_TX_DESC_NUM-1);
	//avail_desc = cp->vcc[ch_no].TV.desc_pf - cp->vcc[ch_no].TV.desc_pw;
#ifndef	CONFIG_FLOW_CTRL
	//ClearTxBuffer(cp, ch_no);
	avail_desc = (cp->vcc[ch_no].TV.desc_pf- 1 - cp->vcc[ch_no].TV.desc_pw + SAR_TX_DESC_NUM)%SAR_TX_DESC_NUM;
	/* if not enough descriptors, drop this packet */
	if (avail_desc == 0)
	{
		/*printk(KERN_ERR " SAR Tx Fail need:%d, availble: %d,desc_pf %d,desc_pw %d\n",
			fragNum+1, avail_desc, cp->vcc[ch_no].TV.desc_pf, cp->vcc[ch_no].TV.desc_pw);*/
		if ((cp->TVDesc[ch_no*SAR_TX_RING_SIZE+cp->vcc[ch_no].TV.desc_pw].CMD&OWN)!=0) {
		cp->vcc[ch_no].stat.send_desc_lack++;
		//printk(KERN_ERR" avail_desc %d <= fragNum %d\n", avail_desc, fragNum);
		goto DropPacket;
		};
		//__cli();
	    ClearTxBuffer(cp, ch_no);
	    //__sti();
		//free tx desc num = SAR_TX_RING_SIZE
	}
#else
		
	 /* Clear transmitted descriptors */
	//ClearTxBuffer(cp, ch_no);
	
	avail_desc = (cp->vcc[ch_no].TV.desc_pf- 1 - cp->vcc[ch_no].TV.desc_pw + SAR_TX_DESC_NUM)&(SAR_TX_DESC_NUM-1);
	/* if not enough descriptors, enable flow control only if IPQoS not enabled */
//#ifdef CONFIG_EXT_SWITCH
	if (enable_ipqos==0) {
//#endif
		//tylo, FCEnable:check if sar is ready
		if(FCCtrl && FCEnable){
			if (avail_desc <= FC_low)
			{
				//save_flags(flags);
				disable_irq(sar_irq);
//				printk("%s: avail_desc %d  enable flow control(ori RCR %x)\n", __func__, avail_desc, REG32(0xb9800044));
				FCFlag = 1;
				//reject packets with physical address not to me
				//REG32(0xb9800044)=0x0000000a;
				REG32(0xb9800044)=0x0000000e;
				//restore_flags(flags);
			}
		}
//#ifdef CONFIG_EXT_SWITCH
	}
//#endif
	
#endif	//CONFIG_FLOW_CTRL

	if (avail_desc <= 1 && cp->vcc[ch_no].dev!=NULL) {
		//printk("stop queue\n");
//#ifdef CONFIG_EXT_SWITCH
		if (enable_ipqos)
//#endif
			netif_stop_queue(cp->vcc[ch_no].dev); 
	}
	//tylo
	if(avail_desc==0)
		goto DropPacket;
	
	CRC_Gen_offset=4;
	//total_len = skb_to_send->len;
	
	{
		//i = 0;
		{
			/* Get Current Descriptor Index */
			//j= (cp->vcc[ch_no].TV.desc_pw+i)%SAR_TX_DESC_NUM;
			j= cp->vcc[ch_no].TV.desc_pw;

			/* Handle this descriptor */
			TVDesc=&cp->TVDesc[ch_no*SAR_TX_RING_SIZE+j];
			//CMD=0;

			/* if this Descriptor is OWN by SAR, skip it. */
			//if(TVDesc->CMD&OWN){
			//	goto DropPacket;
			//}

			{/* first of the fragment */
				TVDesc->START_ADDR=(u32)skb_to_send->data|Uncache_Mask;
				cp->vcc[ch_no].tx_buf[j]=(UINT32)skb_to_send;

				TVDesc->LEN=skb_to_send->len&LEN_Mask;
			}
			#if 0
			{
				/* get next frag */
				skb_frag_t *this_frag = &skb_shinfo(skb_to_send)->frags[i-1];
				TVDesc->START_ADDR=(u32)this_frag->page_offset|Uncache_Mask;
				TVDesc->LEN=this_frag->size;
				cp->vcc[ch_no].tx_skb[j]=(UINT32)skb_to_send;
			}
			#endif

			// JONAH_DEBUG
			//printk(KERN_ERR"sar tx 2 %d\n", TVDesc->LEN);

			/* Set FS LS */
			//CMD |= FS;
			//CMD |= LS;

			CMD = FS|LS
					|TRLREN /* Enable AAL5 Trailer*/
// #ifdef CONFIG_RTL8671
//#if 1  //temp. fix to port 1
//					|0x0800  /* fix to ATM port 1*/
//#else
//					|cp->atmport/*use ATMPORT 1 */
//#endif
					//|CLP      //6/9/04' hrchen, disable for Nokia D500 bug /* Set CLP */
					|OWN	/* set owner */
					|(((uint16)cp->vcc[ch_no].TV.Ether_Offset_Value<<ETHNT_OFFSET)
						);//&ETHNT_OFFSET_MSK); /* enable Ethernet CRC32 generation and append*/

	     if(sendToPort0) {
			CMD |= 0x0800;
			}


			if(j==(SAR_TX_DESC_NUM-1))
				CMD |= EOR;

			//tylo, fpga
			dma_cache_wback_inv((unsigned long)skb_to_send->data, skb_to_send->len);
			
			/* save the CMD to Descriptor */
			TVDesc->CMD=CMD;//|((j==(SAR_TX_DESC_NUM-1))?EOR:0);

		}

		//cp->vcc[ch_no].TV.desc_pw = (cp->vcc[ch_no].TV.desc_pw + 1)%SAR_TX_DESC_NUM;
		cp->vcc[ch_no].TV.desc_pw = (j + 1)&(SAR_TX_DESC_NUM-1);

	}


	skb_pvc_debug(skb_to_send, ch_no, 1);
#ifdef CONFIG_PORT_MIRROR        
	sar_port_mirror(cp, ch_no, skb_to_send, PORT_MIRROR_OUT);
#endif
	/* if TBE occurs and the current descriptor is owned by hardware, re-schedule this channel */
#ifdef CONFIG_CPU_RLX4181
	//dcache write back, to make memory consist with cache before DMA
	dma_cache_wback_inv((unsigned long)skb_to_send->data, skb_to_send->len);
#endif
	if (reg(SAR_TBEI_Addr)&(0x00000001<<ch_no)) {
	    //if ((cp->TVDesc[ch_no*SAR_TX_RING_SIZE+currCDOI].CMD)&OWN)
	    currCDOI = GetTxCDOI(cp, ch_no);
		Clear_TBE(ch_no, currCDOI);
	};
#ifdef AUTO_PVC_SEARCH
	if(found_pvc==0){
		int delay=0;
		while(TVDesc->CMD & OWN){
			if(delay>0x2000){
		//		printk("Drop pkt!\n");
				break;
			}

			delay++;
		}
	}
#endif

	tx_cell=skb_to_send->len/48;
	if(skb_to_send->len > (tx_cell*48))
		tx_cell++;
	cp->vcc[ch_no].hwTxCellCount+=(int)(tx_cell*512);

	AtmTraffic += tx_cell;	//Tx traffic tracks
	/* Clear transmitted descriptors AGAIN */
	//ClearTxBuffer(cp, ch_no);

	//cp->vcc[ch_no].stat.send_cnt++;
	//cp->vcc[ch_no].stat.send_ok++;
//scout(')');		

    //2/4/06' hrchen, for VBR credit adjust
    //adjust_ATM_QoS(vcc_dev);
    
    //decrease credit
    	vcc_dev->creditQoSTx--;
    
	spin_unlock_irqrestore (&cp->lock, flags);
	DRV_LEAVE;
	return 1;

DropPacket:
	//printk(KERN_ERR"tx packet dropped, desc_num = %d\n",avail_desc);
	//printk("sar_send> drop %p\n", skb_to_send);
	dev_kfree_skb(skb_to_send);
	//dev_kfree_skb_irq(skb_to_send);
	//rtl8671_putPreAlloc(skb_to_send);
	cp->vcc[ch_no].stat.send_fail++;
	cp->vcc[ch_no].stat.tx_pkt_fail_cnt++;
	//printk("WARNING: should never come here!!\n");

	//netif_stop_queue(dev);
//scout('_');			
	spin_unlock_irqrestore (&cp->lock, flags);
	DRV_LEAVE;
	// Kaohj --- count for net_device tx_dropped
	if (vcc_dev->dev && vcc_dev->dev->get_stats) {
		netstats = vcc_dev->dev->get_stats(vcc_dev->dev);
		netstats->tx_dropped++;
	}
	return 0;

}

/*jiunming*/
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
extern int SARvtx(struct atm_vcc *vcc, struct sk_buff* skb, uint32 len, int port);
int 	sar_send (struct atm_vcc *vcc,struct sk_buff *skb)
{	
	sar_private *cp = RTATM_DEV(vcc->dev);
	sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);
	INT8	ch_no;
	int ret;
	unsigned long flags;


	DRV_ENTER

	if( DSPInShowtime )
	{
		if (!pp_enabled)
			ret = _sar_send( vcc, skb );
		else
			ret = pp_sar_send(vcc, skb,skb->len,vcc_dev->ch_no);
		return ret;
	}
	
	//atomic_sub(skb->truesize, &vcc->tx_inuse);
	ch_no = vcc_dev->ch_no;
	spin_lock_irqsave(&cp->lock, flags);
	//printk(KERN_ERR"tx packet dropped, desc_num = %d\n",avail_desc);
	//printk("sar_send> drop %p\n", skb_to_send);
	dev_kfree_skb(skb);
	//dev_kfree_skb_irq(skb_to_send);
	//rtl8671_putPreAlloc(skb_to_send);
	cp->vcc[ch_no].stat.send_fail++;
	cp->vcc[ch_no].stat.tx_pkt_fail_cnt++;
	//printk("WARNING: should never come here!!\n");
	//netif_stop_queue(dev);
	spin_unlock_irqrestore (&cp->lock, flags);
	DRV_LEAVE;
	return 0;	
}

//tylo, removed for linux2.6
#if 0
static void sar_set_rx_mode (struct net_device *dev)
{
	DRV_ENTER;

	printk(KERN_DEBUG "sar_set_rx_mode: called.\n");

	if (dev==NULL) { printk(KERN_ERR "sar_set_rx_mode: dev is NULL!\n"); return; }
	if (dev->flags & IFF_PROMISC)
	{
		printk(KERN_DEBUG "%s: Setting promiscuous mode.\n", dev->name);
	}
	else if ((dev->mc_list) || (dev->flags & IFF_ALLMULTI))
	{
		printk(KERN_DEBUG "%s: All multicast list.\n", dev->name);
	}

	DRV_LEAVE;

}
#endif



#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
static void sar_stop_hw (sar_private *cp)
{
	DRV_ENTER


	int i;
	struct atm_vcc *vcc;

	/* clear Config Register */
	Disable_SAR(cp);

	//synchronize_irq();
	/*linux-2.6.19*/
	synchronize_irq(cp->irq);
	udelay(10);
	
	Reset_Sar();

	/* set all desc point to 0, include OB channel*/
	for(i=0;i<(Enable_VC_CNT+NUMBER_OF_OAM_CH);i++){
	        if(cp->vcc[i].created==VC_CREATED){
		    vcc = cp->vcc[i].dev_data;
		    clear_bit(ATM_VF_READY,&vcc->flags);		
		};
		cp->vcc[i].TV.desc_pf = 0;
		cp->vcc[i].TV.desc_pc = 0;
		cp->vcc[i].TV.desc_pw = 0;
		cp->vcc[i].RV.desc_pr = 0;
		cp->vcc[i].RV.desc_pc = 0;
		cp->vcc[i].RV.desc_pa = 0;
	}

	//remove_bh(SAR_BH);  //5/22/04' hrchen, SAR_BH has no use
	/* ??? !!! free all memory */

	DRV_LEAVE;
}

extern void Enable_IS8672_10(sar_private *cp);
extern void Enable_IS8671G_1(sar_private *cp);
extern void Enable_IS8671G_0(sar_private *cp);



static void sar_init_hw (sar_private *cp)
{
	DRV_ENTER
#if 0
	struct SAR_IOCTL_CFG	cfg;
#endif	
	//ql
#ifdef UPSTREAM_TRAFFIC_CTL
	uTc1Data = (*(volatile u32*)0xb9c01024);
	uTc1Data = (uTc1Data >> 8) & 0xffffff;
#endif
	/* ---> SAR Entire Module Related <--- */
	Reset_Sar();
	Init_reg(cp);	/* set corresponding register address */
	//SetCRCTbl();
	//GenCRC10Table();

	//Enable_SAR(cp);

	/* Enable/Disable Loopback test mode */
#ifdef LoopBack_Test
	/*Enable_LoopBack(); */
	Disable_LoopBack(cp); 
	Enable_Sachem_Loopback();
#else
	Disable_LoopBack(cp);
	//REG16(0xb8000c20)=0x0600;
	//Enable_Sachem_Utopia();
	//Enable_Sachem_Loopback();
#endif
//	Enable_LoopBack(cp);	

	/* Select Qos Clock from External clock */
//QCLKSEL has been removed 	Set_QoS_Ext(cp, 0);		/* external */

	
	cp->QoS_Test= FALSE;

#if 0
	/* ---> Individual Chanel Related <--- */
	/* Create one VC first */
	cfg.ch_no		= 0;
	cfg.vpi 		= 5;
	cfg.vci		= 35;
	cfg.rfc		= RFC1483_BRIDGED;
	cfg.framing	= LLC_SNAP;
	//cfg.loopback	= FALSE;
	cfg.QoS.Type	= QoS_UBR;
	cfg.QoS.PCR	= 0x1DFF;
	cfg.QoS.SCR	= 0;
	cfg.QoS.MBS	= 128;
	cfg.QoS.CRD	= 0;
	cfg.QoS.CDVT	= 0;
	CreateVC(cp, &cfg, 1);
#endif


#ifdef ENA_OAM_CH
	AllocVcBuff(cp, OAM_CH_NO);	
	if (cp->vcc[OAM_CH_NO].dev_data)  //Remote Management Channel is created
	    Disable_Drop_NonOAM(cp, OAM_CH_NO);
	else 
	    Enable_Drop_NonOAM(cp, OAM_CH_NO);
	Enable_rx_ch(cp, OAM_CH_NO);
	Enable_tx_ch(cp, OAM_CH_NO);
	
#endif

#ifdef ATM_OAM
	//remember to change the VPI/VCI setting
	OAMF5Init(0, 0, &OAMF5_info);
	OAMF4Init(0, 0, &OAMF4_info);
#endif

	/* ---> Interrupt Related <--- */
	Enable_IERDA(cp);
	Set_RDA(cp, RDATHR, RDATimer);
	/* polling TBE */
	Disable_IETBE(cp);
	//Disable_IETDF(cp);
	//Enable_IETBE(cp);
	//Enable_IETDF(cp);
	Enable_IERBF(cp);

#ifdef CONFIG_RTL8672_ATM_QoS //enable 8672 new function
	Enable_IS8671G_1(cp);	
	Enable_IS8671G_0(cp);	
	Enable_IS8672_10(cp);	
#endif
	//init_bh(SAR_BH, sar_rx_bh);
	// initialise bottom half
	//INIT_LIST_HEAD(&cp->bh.list);
	//cp->bh.sync = 0;
	//cp->bh.routine = (void (*)(void *)) sar_rx_bh;
	//cp->bh.data = cp;
	Enable_QoS_Priority(cp, 0);
	Enable_HP_CHTx(cp, 0);

#ifdef CONFIG_RTL8681
	//printk("[%s, line %d] REG32(0xb8302100) = 0x%08X\n",__func__,__LINE__,REG32(0xb8302100));

	reg(SAR_LINK_STS) = 0x303;
	reg(SAR_BOND_CFG) = 0x2000;
	reg(SAR_DUAL_MAP) = 0x0;	
	
	reg(SAR_BOND_WFQ0) = 0x01010101;
	reg(SAR_BOND_WFQ1) = 0x01010101;
	reg(SAR_BOND_CFG) = 0x20002000; //enable in ADSL lib
	reg(SAR_DUMMY_CFG) = 0x16a;

	//printk("[%s, line %d] REG32(0xb8302100) = 0x%08X\n",__func__,__LINE__,REG32(0xb8302100));
#endif

	memset(&cp->tasklets, 0, sizeof(struct tasklet_struct));
	cp->tasklets.func=(void (*)(unsigned long))sar_rx_bh;
	cp->tasklets.data=(unsigned long)cp;
	sar_rx_tasklets = &cp->tasklets;

	DRV_LEAVE;

}

#if 0
static int sar_refill_rx (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	unsigned i;

	for (i = 0; i < SAR_RX_RING_SIZE; i++) {
		struct sk_buff *skb;

		skb = dev_alloc_skb(cp->rx_buf_sz + RX_OFFSET);
		if (!skb)
			goto err_out;

		skb->dev = cp->dev;
#if 0
		cp->rx_skb[i].mapping = pci_map_single(cp->pdev,
			skb->tail, cp->rx_buf_sz, PCI_DMA_FROMDEVICE);
#endif
		cp->rx_skb[i].skb = skb;
		cp->rx_skb[i].frag = 0;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);
		
		cp->rx_ring[i].addr = (u32)skb->data|Uncache_Mask;
		if (i == (SAR_RX_RING_SIZE - 1))
			cp->rx_ring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_ring[i].opts1 =(DescOwn | cp->rx_buf_sz);
		cp->rx_ring[i].opts2 = 0;
	}

	return 0;

err_out:
	sar_clean_rings(cp);
	#endif
	DRV_LEAVE;
	return -ENOMEM;
}


static int sar_init_rings (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	memset(cp->tx_hqring, 0, sizeof(DMA_DESC) * SAR_TX_RING_SIZE);
	memset(cp->rx_ring, 0, sizeof(DMA_DESC) * SAR_RX_RING_SIZE);
	cp->rx_tail = 0;
	cp->tx_hqhead = cp->tx_hqtail = 0;

	#endif
	DRV_LEAVE;
	return sar_refill_rx (cp);
}

static int sar_alloc_rings (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	/*cp->rx_ring = pci_alloc_consistent(cp->pdev, CP_RING_BYTES, &cp->ring_dma);*/
	void*	pBuf;
	
	
	
	pBuf = kmalloc(SAR_RXRING_BYTES,GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	cp->rxdesc_buf = pBuf;
	memset(pBuf, 0, SAR_RXRING_BYTES);
	
	pBuf = (void*)( (u32)(pBuf + Desc_Align-1) &  ~(Desc_Align -1) ) ;
	cp->rx_ring = (DMA_DESC*)((u32)(pBuf) | Uncache_Mask);


	pBuf= kmalloc(SAR_TXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	cp->txdesc_buf = pBuf;
	memset(pBuf, 0, SAR_TXRING_BYTES);
	pBuf = (void*)( (u32)(pBuf + Desc_Align-1) &  ~(Desc_Align -1) ) ;
	cp->tx_hqring = (DMA_DESC*)((u32)(pBuf) | Uncache_Mask);

	return sar_init_rings(cp);

ErrMem:
	if (cp->rxdesc_buf)
		kfree(cp->rxdesc_buf);
	if (cp->txdesc_buf)
		kfree(cp->txdesc_buf);
	#endif
	DRV_LEAVE;
	return -ENOMEM;
	
}

static void sar_clean_rings (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	unsigned i;


	for (i = 0; i < SAR_RX_RING_SIZE; i++) {
		if (cp->rx_skb[i].skb) {
			dev_kfree_skb(cp->rx_skb[i].skb);
		}
	}

	for (i = 0; i < SAR_TX_RING_SIZE; i++) {
		if (cp->tx_skb[i].skb) {
			struct sk_buff *skb = cp->tx_skb[i].skb;
			dev_kfree_skb(skb);
		}
	}

	memset(&cp->rx_skb, 0, sizeof(struct ring_info) * SAR_RX_RING_SIZE);
	memset(&cp->tx_skb, 0, sizeof(struct ring_info) * SAR_TX_RING_SIZE);
	#endif
	DRV_LEAVE;
}

static void sar_free_rings (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	sar_clean_rings(cp);
	/*pci_free_consistent(cp->pdev, CP_RING_BYTES, cp->rx_ring, cp->ring_dma);*/
	kfree(cp->rxdesc_buf);
	kfree(cp->txdesc_buf);
	
	cp->rx_ring = NULL;
	cp->tx_hqring = NULL;
	#endif
	DRV_LEAVE;
}
#endif

static int sar_init (struct atm_dev *dev)
{
	DRV_ENTER

	sar_private *cp = RTATM_DEV(dev);
	int rc;
	int i;

	Alloc_desc(cp);	/* alloocate memory for descriptors */
	
	//rtl8671_initPreAlloc();
	
	cp->atmport = ATMPORT_SLOW;  //5/23/04' hrchen, default to fast path, for auto path switch
	sar_init_hw(cp);

#ifdef FAST_ROUTE_Test	// FAST_ROUTE_Test variables initialization
	for(i=0;i<Enable_VC_CNT; i++) {
		ch_counter[i].counter = 0;
		ch_counter[i].start_stamp = 0;
		
		fast_route_ch_no[i].index = 0;
		fast_route_ch_no[i].enable = 0;
		memset(fast_route_ch_no[i].mac[i], 0, 6);
	}
#endif	
	
	/* !!! irq 3 is used at this point due to rupert's ISR dispatch method */
	cp->irq = sar_irq;
	//rc = request_irq(cp->irq, sar_interrupt, SA_INTERRUPT, dev->type, dev);
	/*linux-2.6.19*/
	rc = request_irq(cp->irq, sar_interrupt, IRQF_DISABLED, dev->type, dev);
	if (rc)
		goto err_out_hw;

	dev->ci_range.vpi_bits = 8;
	dev->ci_range.vci_bits = 16;

        //request_irq will enable requested irq  	 
        //REG32(GIMR) |= SAR_IE ;

	DRV_LEAVE;
	return 0;

err_out_hw:
	sar_stop_hw(cp);
	/* free all buffers */
	//5/22/04' hrchen
	//free pkt bfr
	for (i=0;i<Enable_VC_CNT;i++) {
	    sar_close(cp->vcc[i].dev_data);
	};
	FreeVcBuff(cp, OAM_CH_NO);
	Free_desc(cp);
	DRV_LEAVE;
	return rc;

}
#if 0	
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
static int sar_open (struct atm_vcc *vcc, short vpi, int vci)
{
	sar_private *cp = RTATM_DEV(vcc->dev);
	struct SAR_IOCTL_CFG	cfg;
	int error=0;
	
	if (vci == ATM_VPI_UNSPEC || vpi == ATM_VCI_UNSPEC)
		return -EINVAL;
	if (!test_bit(ATM_VF_PARTIAL,&vcc->flags))
		RTATM_VCC(vcc) = NULL;
	printk("fixme atm_find_ci in sar_open!\n");
	//error = atm_find_ci(vcc,&vpi,&vci);
	if (error)
		return error;
	vcc->vpi = vpi;
	vcc->vci = vci;
	if (vci != ATM_VPI_UNSPEC && vpi != ATM_VCI_UNSPEC)
		set_bit(ATM_VF_ADDR,&vcc->flags);
	if (vcc->qos.aal != ATM_AAL5)
		return -EINVAL; /* @@@ AAL0 */
	printk(DEV_LABEL "(itf %d): open %d.%d\n",vcc->dev->number,vcc->vpi,
	    vcc->vci);
	if (!test_bit(ATM_VF_PARTIAL,&vcc->flags)) {
#ifdef REMOTE_MANAGEMENT_ENABLE		
        if ((vpi==0)&&(vci==16)) {    //Remote Management Channel
            vcc->dev_data = &cp->vcc[OAM_CH_NO];
            cp->vcc[OAM_CH_NO].dev_data = vcc;
            cp->vcc[OAM_CH_NO].ch_no = OAM_CH_NO;
            cp->vcc[OAM_CH_NO].vpi = vpi;
            cp->vcc[OAM_CH_NO].vci = vci;
        } else {    //normal PVC channel
#endif
		    int i;
            /* find a free channel */
            for(i=0; i<(Enable_VC_CNT+1); i++)
            	if(cp->vcc[i].created == VC_NOT_CREATED)
            		break;
            if(i!=(Enable_VC_CNT+1))
            	cfg.ch_no = i;
            else
            	return -1;
                  
            vcc->dev_data = &cp->vcc[i];
            cp->vcc[i].dev_data = vcc;
            /* Create one VC first */
            cfg.vpi 	= vpi;
            cfg.vci		= vci;
            /* default setting */
            cfg.rfc		= RFC1483_BRIDGED;
            cfg.framing	= LLC_SNAP;
            //cfg.loopback	= FALSE;
                  
            cp->vcc[i].br=0;	//default direct bridge disabled
                  
            cfg.QoS.Type	= sar_to_qos(vcc->qos.txtp.traffic_class);
            if(vcc->qos.txtp.pcr)
            	cfg.QoS.PCR = vcc->qos.txtp.pcr;
            else
            	cfg.QoS.PCR	= 0x1DFF;
            if(vcc->qos.txtp.scr)
            	cfg.QoS.SCR = vcc->qos.txtp.scr;
            else			
            	cfg.QoS.SCR	= 0;
            if(vcc->qos.txtp.mbs)
            	cfg.QoS.MBS = vcc->qos.txtp.mbs;
            else
            	cfg.QoS.MBS	= 128;
            cfg.QoS.CRD	= 0;
            cfg.QoS.CDVT	= 0;
            CreateVC(cp, &cfg, 1);
            set_atm_data_mode(1);
#ifdef REMOTE_MANAGEMENT_ENABLE            
        };
#endif  
    }
	set_bit(ATM_VF_READY,&vcc->flags);
	return 0;
}
#else
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
static int sar_open (struct atm_vcc *vcc)
{
	sar_private *cp = RTATM_DEV(vcc->dev);
	struct SAR_IOCTL_CFG	cfg;
	short vpi; int vci;
	int error=0;

	vpi = vcc->vpi; vci = vcc->vci;
	if (vci == ATM_VPI_UNSPEC || vpi == ATM_VCI_UNSPEC)
		return -EINVAL;
	if (!test_bit(ATM_VF_PARTIAL,&vcc->flags))
		//RTATM_VCC(vcc) = NULL;
		(vcc)->dev_data = NULL;
	printk("fixme atm_find_ci in sar_open!\n");
	//error = atm_find_ci(vcc,&vpi,&vci);
	if (error)
		return error;
	vcc->vpi = vpi;
	vcc->vci = vci;
	if (vci != ATM_VPI_UNSPEC && vpi != ATM_VCI_UNSPEC)
		set_bit(ATM_VF_ADDR,&vcc->flags);
	if (vcc->qos.aal != ATM_AAL5)
		return -EINVAL; /* @@@ AAL0 */
	printk(DEV_LABEL "(itf %d): open %d.%d\n",vcc->dev->number,vcc->vpi,
	    vcc->vci);
	if (!test_bit(ATM_VF_PARTIAL,&vcc->flags)) {
#ifdef REMOTE_MANAGEMENT_ENABLE		
        if ((vpi==0)&&(vci==16)) {    //Remote Management Channel
            vcc->dev_data = &cp->vcc[OAM_CH_NO];
            cp->vcc[OAM_CH_NO].dev_data = vcc;
            cp->vcc[OAM_CH_NO].ch_no = OAM_CH_NO;
            cp->vcc[OAM_CH_NO].vpi = vpi;
            cp->vcc[OAM_CH_NO].vci = vci;
        } else {    //normal PVC channel
#endif
		    int i;
            /* find a free channel */
            for(i=0; i<(Enable_VC_CNT+1); i++)
            	if(cp->vcc[i].created == VC_NOT_CREATED)
            		break;
            if(i!=(Enable_VC_CNT+1))
            	cfg.ch_no = i;
            else
            	return -1;
                  
            vcc->dev_data = &cp->vcc[i];
            cp->vcc[i].dev_data = vcc;
            /* Create one VC first */
            cfg.vpi 	= vpi;
            cfg.vci		= vci;
            /* default setting */
            cfg.rfc		= RFC1483_BRIDGED;
            cfg.framing	= LLC_SNAP;
            //cfg.loopback	= FALSE;
                  
            cp->vcc[i].br=0;	//default direct bridge disabled
                  
            cfg.QoS.Type	= sar_to_qos(vcc->qos.txtp.traffic_class);
            if(vcc->qos.txtp.pcr)
            	cfg.QoS.PCR = vcc->qos.txtp.pcr;
            else
            	cfg.QoS.PCR	= 0x1DFF;
            if(vcc->qos.txtp.scr)
            	cfg.QoS.SCR = vcc->qos.txtp.scr;
            else			
            	cfg.QoS.SCR	= 0;
            if(vcc->qos.txtp.mbs)
            	cfg.QoS.MBS = vcc->qos.txtp.mbs;
            else
            	cfg.QoS.MBS	= 128;
            cfg.QoS.CRD	= 0;
            cfg.QoS.CDVT	= 0;
            CreateVC(cp, &cfg, 1);
            set_atm_data_mode(1);
#ifdef REMOTE_MANAGEMENT_ENABLE            
        };
#endif  
    }
	set_bit(ATM_VF_READY,&vcc->flags);
	return 0;
}

#endif //#if 0 
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
static void sar_close (struct atm_vcc *vcc)
{
	sar_private *cp = RTATM_DEV(vcc->dev);
	sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);
	struct SAR_IOCTL_CFG	cfg;
	
	DRV_ENTER;
	if (!vcc_dev)
		return;
	clear_bit(ATM_VF_READY,&vcc->flags);
	//disable_lx4180_irq(3); //disable SAR IRQ before free buffer
	
	cfg.ch_no	= vcc_dev->ch_no;
	cfg.vpi 	= vcc->vpi;
	cfg.vci		= vcc->vci;
	DeleteVC(cp, &cfg);
	
	clear_bit(ATM_VF_ADDR,&vcc->flags);
	DRV_LEAVE;
	printk("sar_close\n");
	return;
}


extern int StartAtmOAMLoopBack(ATMOAMLBReq *pATMOAMLBReq);
extern int SetAtmOAMCpid(ATMOAMLBID *pATMOAMLBID);
extern int StopAtmOAMLoopBack(ATMOAMLBReq *pATMOAMLBReq);
extern int GetAtmOAMLoopBackStatus(ATMOAMLBState *pATMOAMLBState);
extern int GetAtmOAMLoopBackStatusFE(ATMOAMLBRXState *pATMOAMLBRXState);
extern void OAMF5ContiLbTest(ATMOAMLBReq *pucPtr);
extern void OAMF5StopLbTest(void);

extern int test_memory_allocate;
char pvc_mode[16];
char pvc_encap[16];
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
int sar_ioctl (struct atm_dev *dev,unsigned int cmd,void *arg)
{
	DRV_ENTER
	sar_private *cp = RTATM_DEV(dev);
	char 		*data = (char *)arg;
	int8		ch_no;
//	BOOL		rc = FALSE;
//	int			i;
	int		iVal;
	
	
	if (dev==NULL) { printk("sar_ioctl: dev is NULL!\n"); return -1;}
		
	if (cp==NULL) { printk("sar_ioctl: cp is NULL!\n"); return -2;}

//	printk(KERN_DEBUG "sar_ioctl: called. cmd=0x%x, arg=%s\n", cmd, (char*)arg);

	switch (cmd) {

		case ATM_SAR_GETSTAT:
			{
				//printk(KERN_DEBUG "sar_ioctl: SAR_GET_STATS called.\n");
				struct SAR_IOCTL_CFG	*get_cfg = (struct SAR_IOCTL_CFG	*)data;
				ch_no = get_cfg->ch_no;
				if(ch_no < Enable_VC_CNT && ch_no >= 0 && cp->vcc[ch_no].created == VC_CREATED) {
					//printk("\n\nRV.desc_pr=%02x, RV.desc_pc=%02x, RV.desc_pa=%02x\n", cp->vcc[ch_no].RV.desc_pr, cp->vcc[ch_no].RV.desc_pc, cp->vcc[ch_no].RV.desc_pa);
					//printk("TV.desc_pf=%02x, TV.desc_pc=%02x, TV.desc_pw=%02x\n\n", cp->vcc[ch_no].TV.desc_pf, cp->vcc[ch_no].TV.desc_pc, cp->vcc[ch_no].TV.desc_pw);
				}
				if (copy_to_user(&(get_cfg->vpi), &(cp->vcc[ch_no].vpi), sizeof(char)))
					return -EFAULT;
				if (copy_to_user(&(get_cfg->vci), &(cp->vcc[ch_no].vci), sizeof(short)))
					return -EFAULT;
				if (copy_to_user(&(get_cfg->rfc), &(cp->vcc[ch_no].rfc), sizeof(int)))
					return -EFAULT;
				if (copy_to_user(&(get_cfg->created), &(cp->vcc[ch_no].created), sizeof(int)))
					return -EFAULT;
				if (copy_to_user(&(get_cfg->stat),	&(cp->vcc[ch_no].stat), sizeof(ch_stat)))
					return -EFAULT;
		
				//printk(KERN_DEBUG "sar_ioctl: SAR_GET_STATS done.\n");
				break;
			}
#ifdef UPSTREAM_TRAFFIC_CTL
		//ql
		case SAR_SET_TRAFFIC_CTL:
			{
				if (copy_from_user(&uMaxTraffic, data, sizeof(int)))
					return -EFAULT;/*//for test
				if (copy_from_user(&LB_len, data+4, sizeof(int)))
					return -EFAULT;
				if (copy_from_user(&LB_MAX, data+4, sizeof(int)))
					return -EFAULT;
				if (copy_from_user(&LB_SEC, data+4, sizeof(int)))
					return -EFAULT;
				if (copy_from_user(&LB_THD, data+4, sizeof(int)))
					return -EFAULT;
				if (copy_from_user(&LB_MIN, data+4, sizeof(int)))
					return -EFAULT;*/
				
				if (uMaxTraffic > BANDWIDT_1M) {
			    	printk("sar_ioctl: SAR_SET_TRAFFIC_CTL not allowed (0x%x)\n", uMaxTraffic);
			    	return -EFAULT;
		    	};
				if (uMaxTraffic > 0)
					ucTcEbl = 1;
				else	//uMaxTraffic==0
					ucTcEbl = 0;

				uMaxTraffic = uMaxTraffic*1024/1000;

				break;
			}
#endif
#ifdef CONFIG_RTL8672
		case SAR_SET_SARHDR:
			{
				uint32 mode,vpi,vci,i, ch_no=0xff;
				printk(KERN_DEBUG "sar_ioctl: SAR_SET_SARHDR called.\n");
				if (copy_from_user(&mode, data, sizeof(int)))
					return -EFAULT;
				if(mode>0) {
					if (copy_from_user(&vpi, data+4, sizeof(int)))
						return -EFAULT;
					if (copy_from_user(&vci, data+8, sizeof(int)))
						return -EFAULT;

					printk(KERN_DEBUG "vpi=%d, vci = %d\n", vpi,vci);
					for (i=0; i<Enable_VC_CNT; i++) {
						if (cp->vcc[i].vpi ==(short) vpi && cp->vcc[i].vci == vci) 
							ch_no = (uint32)cp->vcc[i].ch_no;
													
					}
					if(ch_no==0xff){
						printk(KERN_DEBUG "Cannot find PVC (%d, %d) corresponding channle number!\n", vpi,vci);
						break;
					}
				
				
					switch(mode){
					case 1:	//MER
						Set_SARhdr(cp, ch_no, MER_mode);	//MER mode 				
						printk(KERN_DEBUG "Set CH %d MER mode on CKS register\n", ch_no);
						pvc_mode[ch_no]=MER_mode;
						break;
					case 2:	//PPPoE
						Set_SARhdr(cp, ch_no, PPPoE_mode);	//PPPoE mode 				
						printk(KERN_DEBUG "Set CH %d PPPoE mode on CKS register\n", ch_no);
						pvc_mode[ch_no]=PPPoE_mode;
						break;
					case 3:	//PPPoA						
						Set_SARhdr(cp, ch_no, PPPoA_mode);	//PPPoA mode 				
						printk(KERN_DEBUG "Set CH %d PPPoA mode on CKS register\n", ch_no);
						pvc_mode[ch_no]=PPPoA_mode;
						break;
					case 4: //1483 Routed
						Set_SARhdr(cp, ch_no, Routed_mode);	//Routed mode 				
						printk(KERN_DEBUG "Set CH %d Routed mode on CKS register\n", ch_no);
						pvc_mode[ch_no]=Routed_mode;
						break;
					default:
						printk(KERN_DEBUG "sar_iotcl: Unknown or bridged encapsulate mode\n");
					}
				}
				printk(KERN_DEBUG "sar_ioctl: SAR_SET_SARHDR done.\n");
				break;
			}
		case SAR_SET_PKTA:
			{
				pp_set_vc_type(total_pvc_number);
				break;
			}
#endif
#if 1
		case SAR_SET_BRIDGE_MODE:
			{
                                //Merge Kao's fix from 2.4.x 
				int ch_no;
				printk(KERN_DEBUG "sar_ioctl: SAR_SET_BRIDGE_MODE called.\n");
				if (copy_from_user(&ch_no, data, sizeof(int)))
					return -EFAULT;

				ch_no = index_vc2ch(ch_no);
				if (ch_no >= 0) {
					cp->vcc[ch_no].br=1;
#ifdef CONFIG_RTL8672
               				Set_SARhdr(cp, ch_no, Bridged_mode);	
						pvc_mode[ch_no]=Bridged_mode;

#endif //CONFIG_RTL8672
				}
				else
					printk("sar_ioctl(SAR_SET_BRIDGE_MODE): channel not found!\n");
//				printk("Enable CH %d direct bridge mode\n", ch_no);						

				printk(KERN_DEBUG "sar_ioctl: SAR_SET_BRIDGE_MODE done.\n");
				break;
			}
#endif

		case QOS_ENABLE_IMQ:
			{
				int tmp;
				if (copy_from_user(&tmp, data, sizeof(int)))
					return -EFAULT;

				printk("set qos state:%d\n", tmp);
				if (tmp & QOS_IPTV_TR69)
					qosIPtv = 1;
				else
					qosIPtv = 0;

				if (tmp & QOS_RULE_EXIST)
					qosRuleExist = 1;
				else
					qosRuleExist = 0;
				
				break;
			}
		case PVC_QOS_TYPE:
			{				
				struct SAR_IOCTL_CFG cfg;
				int idx;
				char ifname[6];
				printk(KERN_DEBUG "sar_ioctl: PVC_QOS_TYPE called.\n");

				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;

				ch_no = cfg.ch_no;
				snprintf(ifname, 6, "vc%d", ch_no);
				
				ch_no = 0;
				for (idx = 0; idx < Enable_VC_CNT; idx++) {
					if (cp->vcc[idx].dev && !strcmp(ifname, cp->vcc[idx].dev->name)) {
						ch_no = idx;
						break;
					}
				}
				////////
				printk("set channel %d qos to %d\n", ch_no, cfg.QoS.Type);
				SetQoS(cp, ch_no, cfg.QoS.Type);
				
				if (cfg.QoS.Type==QoS_nrtVBR || cfg.QoS.Type==QoS_rtVBR) {
					SetPCR(cp, ch_no, cfg.QoS.PCR);
					SetSCR(cp, ch_no, cfg.QoS.SCR);
					SetMBS(cp, ch_no, cfg.QoS.MBS);
				}
				else if (cfg.QoS.Type==QoS_CBR) {
					SetPCR(cp, ch_no, cfg.QoS.PCR);
				} else {
					SetPCR(cp, ch_no, cfg.QoS.PCR);
				}
				break;
			}
		case SAR_ENABLE:
			{
				printk(KERN_DEBUG "sar_ioctl: SAR_ENABLE called.\n");
				//set_atm_data_mode(1);
				Enable_SAR(cp);
				printk(KERN_DEBUG "sar_ioctl: SAR_ENABLE done.\n");
				break;
			}
	

		case SAR_DISABLE:
			{
				
				printk(KERN_DEBUG "sar_ioctl: SAR_DISABLE called.\n");
				Disable_SAR(cp);
				printk(KERN_DEBUG "sar_ioctl: SAR_DISABLE done.\n");
				
				break;				
			}

	
		case ATM_SAR_GETCONFIG:
		//case SAR_GET_CONFIG:
			{
				struct SAR_IOCTL_CFG	*get_cfg = (struct SAR_IOCTL_CFG	*)data;
				struct atm_vcc *vcc;
				
				printk(KERN_DEBUG "sar_ioctl: SAR_GET_CONFIG called.\n");

				ch_no = get_cfg->ch_no;
				get_cfg->ch_no=ch_no;
				get_cfg->vpi = cp->vcc[ch_no].vpi;
				get_cfg->vci = cp->vcc[ch_no].vci;
				get_cfg->rfc = cp->vcc[ch_no].rfc;
				get_cfg->framing = cp->vcc[ch_no].framing;
				get_cfg->created = cp->vcc[ch_no].created;
				//get_cfg->loopback = cp->vcc[ch_no].loopback;

				vcc = cp->vcc[ch_no].dev_data;
				get_cfg->QoS.Type = sar_to_qos(vcc->qos.txtp.traffic_class);
				if(vcc->qos.txtp.pcr)
					get_cfg->QoS.PCR = vcc->qos.txtp.pcr;
				else
					get_cfg->QoS.PCR	= 0x1DFF;
				if(vcc->qos.txtp.scr)
					get_cfg->QoS.SCR = vcc->qos.txtp.scr;
				else			
					get_cfg->QoS.SCR	= 0;
				if(vcc->qos.txtp.mbs)
					get_cfg->QoS.MBS = vcc->qos.txtp.mbs;
				else
					get_cfg->QoS.MBS	= 128;
				get_cfg->QoS.CRD = 0;
				get_cfg->QoS.CDVT = 0;
				//memcpy(&get_cfg->QoS, &cp->vcc[ch_no].QoS, sizeof(Traffic_Manage));
				memcpy(&get_cfg->stat, &cp->vcc[ch_no].stat, sizeof(ch_stat));
	
				printk(KERN_DEBUG "sar_ioctl: RLCM_GET_CONFIG done.\n");
				break;
			}
	
		case SAR_SET_CONFIG:
			{
				struct SAR_IOCTL_CFG 	cfg;
				printk(KERN_DEBUG "sar_ioctl: SAR_SET_CONFIG called.\n");

				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;

				ch_no = cfg.ch_no;

				if(cp->vcc[ch_no].created !=FALSE)	return -EFAULT;

				CreateVC(cp, &cfg, 0);
	
				printk(KERN_DEBUG "sar_ioctl: SAR_SET_CONFIG done.\n");
				break;
			}

		case SAR_SET_ENCAPS:
			{
				struct SAR_IOCTL_CFG 	*cfg;
				int i;
				
				//if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
				//	return -EFAULT;
				cfg = (struct SAR_IOCTL_CFG *)data;
				if (!cfg)
					return -EFAULT;
				
				// find the channel number
				for (i=0; i<Enable_VC_CNT; i++) {
					if (cp->vcc[i].vpi == cfg->vpi && cp->vcc[i].vci == cfg->vci)
						break;
				}
				
				if (i == Enable_VC_CNT) {
					printk(KERN_DEBUG "sar_ioctl: set encapsulation failed!\n");
					return -EFAULT;
				}
				ch_no = i;
				cp->vcc[ch_no].rfc = cfg->rfc;
				cp->vcc[ch_no].framing = cfg->framing;

				// andrew, VC is created before encap is set? need to review!! 2007/11/08
				if(RFC2364 == cfg->rfc) {
					if (!pp_enabled) {
						Enable_Word_Insert(cp, ch_no);
						Write_IP_Parser(cp, ch_no, 1, 2);  // enable eth parse, 2 offset
					}
				}

				
				/* Set L2Encap in RV[x]_CKS for checking RX packet correctness */
				if(cp->vcc[ch_no].framing == LLC_SNAP)
					Set_L2Encap(cp, ch_no, L2Encap_LLC);
				else
					Set_L2Encap(cp, ch_no, L2Encap_VCM);

//#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
				//set SAR interface encap. here
				if(cp->vcc[ch_no].framing == LLC_SNAP)
					pvc_encap[ch_no]=L2Encap_LLC;
				else
					pvc_encap[ch_no]=L2Encap_VCM;
//#endif

				break;
			}

	
#ifdef ATM_OAM
		case ATM_OAM_SET_ID:
			{
				ATMOAMLBID pATMOAMLBID;
				int i;
	
				/*jiunming*/
				if( !DSPInShowtime )
				{
					printk( "Not Enter Showtime!(line:%d)\n", __LINE__ );
					return -EFAULT; 
				}
				
				if (copy_from_user(&pATMOAMLBID, (ATMOAMLBID *)data, sizeof(ATMOAMLBID)))
					return -EFAULT;
				printk("SAR_ATM_OAM_SET_ID: Vpi = %d, Vci = %d, ", (int)pATMOAMLBID.vpi, (int)pATMOAMLBID.vci);
				printk("ID = ");
				for (i = 0; i < OAM_LB_SRCID_SIZE; i++)
					printk("%.02x", pATMOAMLBID.LocID[i]);
				printk("\n");

					if(SetAtmOAMCpid(&pATMOAMLBID) == 0)
				{
					return -EFAULT; 
				}
				break;
			}	
		case ATM_OAM_LB_START:
			{
				ATMOAMLBReq pucPtr;
				//unsigned char i;
	
				/*jiunming*/
				if( !DSPInShowtime )
				{
					printk( "Not Enter Showtime!(line:%d)\n", __LINE__ );
					return -EFAULT; 
				}
	
				if (copy_from_user(&pucPtr, (ATMOAMLBReq *)data, sizeof(ATMOAMLBReq)))
					return -EFAULT;
	
				printk("SAR_ATM_OAM_START: Vpi = %d, Vci = %d\n", (int)pucPtr.vpi, (int)pucPtr.vci);

				if(StartAtmOAMLoopBack(&pucPtr) == 0)
				{
					return -EFAULT; 
				}
				else
				{
					put_user(pucPtr.Tag, &((ATMOAMLBReq *)data)->Tag);
					printk("SAR_ATM_OAM_START: Tag is %d\n", (int)pucPtr.Tag);
				}
				break;
			}

		case ATM_OAM_LB_STOP:
			{
				ATMOAMLBReq pucPtr;
		
				/*jiunming*/
				if( !DSPInShowtime )
				{
					printk( "Not Enter Showtime!(line:%d)\n", __LINE__ );
					return -EFAULT; 
				}
		
				if (copy_from_user(&pucPtr, (ATMOAMLBReq *)data, sizeof(ATMOAMLBReq)))
					return -EFAULT;
				if(StopAtmOAMLoopBack(&pucPtr) == 0)
				{
					return -EFAULT; 
				}
				else
				{
					printk("SAR_ATM_OAM_STOP: Tag is %d\n", (int)pucPtr.Tag);
				}
				break;
			}
					
		case ATM_OAM_LB_STATUS:
			{
				//OAMF5_LB_INFO *pLBInfo;
				//OAMF5_LBRX_INFO *pLBRXInfo;
				ATMOAMLBState pucPtr;
				//unsigned char i, j;
	
				/*jiunming*/
				if( !DSPInShowtime )
				{
					printk( "Not Enter Showtime!(line:%d)\n", __LINE__ );
					return -EFAULT; 
				}
	
				if (copy_from_user(&pucPtr, (ATMOAMLBState *)data, sizeof(ATMOAMLBState)))
					return -EFAULT;
		
				//printk("SAR_ATM_OAM_STATUS: Vpi = %d, Vci = %d\n",(int)pucPtr.vpi, (int)pucPtr.vci);
				if(GetAtmOAMLoopBackStatus(&pucPtr) == 0)
				{
					printk("SAR_ATM_OAM_STATUS: Tag %d NOT found!\n", (int)pucPtr.Tag);
					return -EFAULT; 
				}
				else
				{
					//int i, j;
				/*
					printk("SAR_ATM_OAM_STATUS: Tag %d\n", (int)pucPtr.Tag);

					for(i=0;i<6;i++)
					{
						if(pucPtr.status[i] == FAULT_LB_WAITING)
						{
							printk("status Waiting\n");
						}
						else if(pucPtr.status[i] == FAULT_LB_STOP)
						{
							printk("status Stop, count %u, rtt %u, LLID ", (int)pucPtr.count[i], (int)pucPtr.rtt[i]);
							for (j = 0; j < OAM_LB_LLID_SIZE; j++)
								printk("%.02x", pucPtr.LocID[i][j]);
							printk("\n");
						}
					}
				*/
					
					copy_to_user(data, &pucPtr, sizeof(ATMOAMLBState));
				}
				break;
			}
		case ATM_OAM_STATUS_FE:
			{
				ATMOAMLBRXState pucPtr;

				/*jiunming*/
				if( !DSPInShowtime )
				{
					printk( "Not Enter Showtime!(line:%d)\n", __LINE__ );
					return -EFAULT; 
				}

				if (copy_from_user(&pucPtr, (ATMOAMLBRXState *)data, sizeof(ATMOAMLBRXState)))
					return -EFAULT;

				if(GetAtmOAMLoopBackStatusFE(&pucPtr) == 0)
				{
					printk("SAR_ATM_OAM_STATUS_FE: NOT found!\n");
					return -EFAULT; 
				}
				else
				{
					//int i, j;
					printk("SAR_ATM_OAM_STATUS_FE: \n");
					copy_to_user(data, &pucPtr, sizeof(ATMOAMLBRXState));
				}
				break;
			}
		case SAR_ATM_OAM_RPT_LB:
			{
				ATMOAMLBReq pucPtr;
				//unsigned char i;
	
				/*jiunming*/
				if( !DSPInShowtime )
				{
					printk( "Not Enter Showtime!(line:%d)\n", __LINE__ );
					return -EFAULT; 
				}
	
				if (copy_from_user(&pucPtr, (ATMOAMLBReq *)data, sizeof(ATMOAMLBReq)))
					return -EFAULT;
	
				printk("SAR_ATM_OAM_START: Vpi = %d, Vci = %d\n", (int)pucPtr.vpi, (int)pucPtr.vci);

				OAMF5ContiLbTest(&pucPtr);
				break;
			}
		case SAR_ATM_OAM_STOP_LB:
			{
				/*jiunming*/
				if( !DSPInShowtime )
				{
					printk( "Not Enter Showtime!(line:%d)\n", __LINE__ );
					return -EFAULT; 
				}

				OAMF5StopLbTest();
				break;
			}
#endif

		case SAR_CREATE_VC:
			{
				struct SAR_IOCTL_CFG 	cfg;
	
				printk(KERN_DEBUG "sar_ioctl: SAR_CREATE_VC called.\n");

				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;
				
				CreateVC(cp, &cfg, 1);
				printk(KERN_DEBUG "sar_ioctl: SAR_CREATE_VC done.\n");
				break;
			}

		case SAR_DELETE_VC:
			{
	
				struct SAR_IOCTL_CFG	cfg;	
				printk(KERN_DEBUG "sar_ioctl: SAR_DELETE_VC called.\n");

				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;

				ch_no = cfg.ch_no;

				if(cp->vcc[ch_no].created !=VC_CREATED)	return -EFAULT;
				
				DeleteVC(cp, &cfg);
	
				//cfg.created = FALSE;
				
				printk(KERN_DEBUG "sar_ioctl: SAR_DELETE_VC done.\n");
				break;
			}
		case SAR_SETMAC:
			{	
				struct SAR_IOCTL_CFG cfg;
				printk(KERN_DEBUG "sar_ioctl: SAR_SETMAC called.\n");
				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;
				ch_no=cfg.ch_no;
				memcpy(cp->vcc[ch_no].MAC, cfg.MAC, 6);
				//for(i=0;i<6;i++)
				//		"%02X ", cp->vcc[ch_no].MAC[i]);
					
				cp->QoS_Test=TRUE;
				printk(KERN_DEBUG "sar_ioctl: SAR_SETMAC done.\n");
				
				break;

			}
			
		case SAR_ENABLE_UTOPIA:
			{
	
				printk(KERN_DEBUG "sar_ioctl: SAR_ENABLE_UTOPIA called.\n");

//				REG16(0xB8000c44)= (uint16)0xBE03;
				
				printk(KERN_DEBUG "sar_ioctl: SAR_ENABLE_UTOPIA done.\n");
				break;
			}		

		case SAR_UTOPIA_FAST:
			{
	
				printk(KERN_DEBUG "sar_ioctl: SAR_UTOPIA_FAST called.\n");

				cp->atmport = ATMPORT_FAST;
				//flush_tx_desc(cp);
				
				printk(KERN_DEBUG "sar_ioctl: SAR_UTOPIA_FAST done.\n");
				break;
			}		

		case SAR_UTOPIA_SLOW:
			{
	
				printk(KERN_DEBUG "sar_ioctl: SAR_UTOPIA_SLOW called.\n");

				cp->atmport = ATMPORT_SLOW;
				//flush_tx_desc(cp);
				
				printk(KERN_DEBUG "sar_ioctl: SAR_UTOPIA_SLOW done.\n");
				break;
			}		
		case SAR_EnableLOOPBACK:
		{
			printk("Enable SAR Loopback\n");
			Enable_LoopBack(cp);
			break;
		}
		case SAR_DisableLOOPBACK:
		{
			printk("Disable SAR Loopback\n");
			Disable_LoopBack(cp);
			break;
		}
        //12/30/05' hrchen, for PVC desc number setting
		case SAR_SET_PVC_NUMBER:
		{
#if 1
			int i;
			if (copy_from_user(&total_pvc_number, data, sizeof(int)))
				return -EFAULT;
			/*if ((total_pvc_number>=1)&&(total_pvc_number<=8)) {
			    per_vc_desc_number = ((SAR_RX_NUM / total_pvc_number )+3)&0xFFFFFFFC;
			    if (per_vc_desc_number>SAR_RX_DESC_HI_LIMIT) per_vc_desc_number=SAR_RX_DESC_HI_LIMIT;
		    } else 
		        per_vc_desc_number = 8;
		    */
		    if ((total_pvc_number<1)||(total_pvc_number>Enable_VC_CNT)) {
			    printk(KERN_DEBUG "sar_ioctl: SAR_SET_PVC_NUMBER not allowed (0x%x)\n", total_pvc_number);
			    return -EFAULT;
		    };
		    per_vc_desc_number = SAR_RX_NUM/total_pvc_number;
		    if (per_vc_desc_number>SAR_RX_DESC_HI_LIMIT) per_vc_desc_number = SAR_RX_DESC_HI_LIMIT;
		    for (i=0;i<total_pvc_number;i++)
		        current_desc_number[i] = per_vc_desc_number;
			printk("PVC Number = %d. Set Desc number per VC = %d\n", total_pvc_number, per_vc_desc_number);
#ifdef SAR_RX_FORMATION
			if (total_pvc_number <= 2) // do rx formation if pvc_number > 2
				do_formation = 0;
			else
				do_formation = 1;
#endif // of SAR_RX_FORMATION
#endif		
			break;
		}
        //1/13/06' hrchen, for memory read
		case SAR_READ_MEM:
		{
			unsigned int *address, len;
			if (copy_from_user(&address, data, sizeof(int)))
				return -EFAULT;
			if (copy_from_user(&len, data+4, sizeof(int)))
				return -EFAULT;
			data += 8;  //skip 8 bytes address/len
			//tylo, for packet processor debug
			printk("Read Address 0x%08x len 0x%08x\n", (unsigned int)address, len);
			
			if (copy_to_user(data, address, len))
				return -EFAULT;			
			break;
		}
        //1/13/06' hrchen, for memory write 4 bytes
		case SAR_WRITE_MEM:
		{
			unsigned int *address, value;
			if (copy_from_user(&address, data, sizeof(int)))
				return -EFAULT;
			if (copy_from_user(&value, data+4, sizeof(int)))
				return -EFAULT;
			
			printk("Write Address 0x%08x Value 0x%08x\n", (unsigned int)address, value);
			*((unsigned int*)address) = value;
			break;
		}
//#ifdef AUTO_PVC_SEARCH	
		case SENT_BOA_PID:
		{			
			if(copy_from_user(&boa_pid, data, sizeof(int)))
				return -EFAULT;
			break;
		}
//#endif
		case SENT_TR069_PID:
		{
			if(copy_from_user(&tr069_pid, data, sizeof(int)))
				return -EFAULT;
			printk("Got PID %d from TR069 \n", tr069_pid);
			break;
		}
			
		case SENT_SNTP_PID:
		{			
			if(copy_from_user(&sntp_pid, data, sizeof(int)))
				return -EFAULT;
			break;
		}

		case SENT_TERM_REQ:
		{
//			printk("send PPP term-req\n");
		//	sendTERM();
			break;
		}

		// for debug
		case SAR_DEBUG_PVC_ENABLE:
		{
			if (copy_from_user(&dbg_cfg, (struct SAR_IOCTL_DEBUG *)data, sizeof(struct SAR_IOCTL_DEBUG)))
				return -EFAULT;
			break;
		}
		case SAR_DEBUG_OBAAL5_ENABLE:
		{
			if (copy_from_user(&debug_obaal5, data, sizeof(int)))
				return -EFAULT;
			break;
		}
		case SAR_DEBUG_OBCELL_ENABLE:
		{
			if (copy_from_user(&debug_obcell, data, sizeof(int)))
				return -EFAULT;
			break;
		}
		case SAR_DEBUG_NUM2PRINT:
		{
			if (copy_from_user(&debug_num2print, data, sizeof(int)))
				return -EFAULT;
			break;
		}
		case SAR_DEBUG_STATUS:
		{
			int _avail_desc, k;
			TV_CMD_DESC	*TVDesc;
			
			printk("size = %d\npvc = %d\nobaal5 = %d\nobcell = %d\n",
				debug_num2print, dbg_cfg.enable, debug_obaal5, debug_obcell);
#ifdef CONFIG_FLOW_CTRL
			printk("RCR = 0x%08x, FCFlag = %d\n", *(int *)(0xb9800044), FCFlag);
			printk("FCCtrl=%d, FC_low=%d, FC_high=%d\n", FCCtrl, FC_low, FC_high);
#endif
			for(ch_no=0;ch_no<Enable_VC_CNT ;ch_no++){
				if(cp->vcc[ch_no].created!=VC_CREATED)
					continue;
				_avail_desc = (cp->vcc[ch_no].TV.desc_pf- 1 - cp->vcc[ch_no].TV.desc_pw + SAR_TX_DESC_NUM)%SAR_TX_DESC_NUM;
				printk("[%02d] avail_desc = %d\n\t", ch_no, _avail_desc);
				TVDesc=&cp->TVDesc[ch_no*SAR_TX_RING_SIZE];
				for (k=0; k<SAR_TX_DESC_NUM; k++) {
					printk("%1d", (TVDesc->CMD&OWN)? 1:0);
					TVDesc++;
				}
				printk("\n");
			}
			break;
		}
#ifdef CONFIG_PORT_MIRROR        
		case SAR_PORT_MIRROR:
		{
			int vpi, vci, flag;
			struct atm_vcc * atm_vcc;
			
			if (copy_from_user(&vpi, data, sizeof(int)))
				return -EFAULT;
			if (copy_from_user(&vci, data+4, sizeof(int)))
				return -EFAULT;
			if (copy_from_user(&flag, data+8, sizeof(int)))
				return -EFAULT;
			
			ch_no = 0;
			if ((0 == vpi) && (0 == vci))
			{
				sar_mirror_flag = flag;
			}
			else
			{
				/* search vpi vci */
				for (; ch_no < Enable_VC_CNT; ch_no++)
				{
					if (VC_CREATED != cp->vcc[ch_no].created)
					{
						continue;
					}
					
					atm_vcc = cp->vcc[ch_no].dev_data;
					if (NULL == atm_vcc)
					{
						continue;
					}
					
					if ((atm_vcc->vpi == vpi) && (atm_vcc->vci == vci))
					{
						atm_vcc->mirror_flag = flag;
						break;
					}
				}
			}
			
			if (Enable_VC_CNT == ch_no)
			{
				return -EFAULT;
			}
			break;
		}
#endif
		case SAR_UPGRADE_FW:
		{
			#ifdef CONFIG_SAR_FASTSKB
			enable_sar_fastskb = 0; // disable fastskb
			flush_skb_data_bfr();
			#endif
			break;
		}
		case SAR_RESET_STATS:
		{
			if(copy_from_user(&iVal, arg, sizeof(int)))
				return -EFAULT;
			if (iVal == -1) { // all
				for (iVal=0; iVal<Enable_VC_CNT; iVal++) {
					memset(&cp->vcc[iVal].stat, 0, sizeof(ch_stat));
				}
			}
			else if (iVal>=0 && iVal <= Enable_VC_CNT) {
				memset(&cp->vcc[iVal].stat, 0, sizeof(ch_stat));
			}
			else
				return -EOPNOTSUPP;
			break;
		}
		default:
			printk("Command error or not supported\n");
			//printk(KERN_DEBUG "sar_ioctl: not support ioctl command (0x%x)\n", cmd);
			return -EOPNOTSUPP;

	}

	DRV_LEAVE;
	return 0;
}

/*
for auto switch data mode,
FAST mode is 1, INTERLEAVED mode is 2
hrchen 5/22/04'
*/
static void sar_restart_hw (sar_private *cp)
{

	/* ---> SAR Entire Module Related <--- */
	Reset_Sar();
	Init_reg(cp);	/* set corresponding register address */
	//SetCRCTbl();
	//GenCRC10Table();

	//Enable_SAR(cp);

	/* Enable/Disable Loopback test mode */
#ifdef LoopBack_Test
	/*Enable_LoopBack(); */
	Disable_LoopBack(cp); 
	Enable_Sachem_Loopback();
#else
	Disable_LoopBack(cp);
	//REG16(0xb8000c20)=0x0600;
	//Enable_Sachem_Utopia();
	//Enable_Sachem_Loopback();
#endif
//	Enable_LoopBack(cp);	

	/* Select Qos Clock from External clock */
// QCLKSEL has been removed 	Set_QoS_Ext(cp, 0);		/* external */

	
	cp->QoS_Test= FALSE;
	
#ifdef ENA_OAM_CH
	//AllocVcBuff(cp, OAM_CH_NO);
	//if (cp->vcc[OAM_CH_NO].dev_data)  //Remote Management Channel is created
	//    Disable_Drop_NonOAM(cp, OAM_CH_NO);
	//else 
	//    Enable_Drop_NonOAM(cp, OAM_CH_NO);
	//Enable_rx_ch(cp, OAM_CH_NO);
	//Enable_tx_ch(cp, OAM_CH_NO);
#endif

#ifdef ATM_OAM
	//remember to change the VPI/VCI setting
	OAMF5Init(0, 0, &OAMF5_info);
	OAMF4Init(0, 0, &OAMF4_info);
#endif

	/* ---> Interrupt Related <--- */
	Enable_IERDA(cp);
	Set_RDA(cp, RDATHR, RDATimer);
	/* polling TBE */
	Disable_IETBE(cp);
	//Disable_IETDF(cp);
	//Enable_IETBE(cp);
	//Enable_IETDF(cp);
	Enable_IERBF(cp);
#ifdef CONFIG_RTL8672_ATM_QoS	//enable 8672 new function
	Enable_IS8671G_1(cp);	
	Enable_IS8671G_0(cp);	
	Enable_IS8672_10(cp);	
	Enable_QoS_Starvation(cp);
#endif	
	Enable_QoS_Priority(cp, 0);
	Enable_HP_CHTx(cp, 0);

#ifdef CONFIG_RTL8681
	//printk("[%s, line %d] REG32(0xb8302100) = 0x%08X\n",__func__,__LINE__,reg(0xb8302100));
	reg(SAR_LINK_STS) = 0x303;
	reg(SAR_BOND_CFG) = 0x2000;
	reg(SAR_DUAL_MAP) = 0x0;	
	
	reg(SAR_BOND_WFQ0) = 0x01010101;
	reg(SAR_BOND_WFQ1) = 0x01010101;
	reg(SAR_BOND_CFG) = 0x20002000;
	reg(SAR_DUMMY_CFG) = 0x16a;
	//Disable_SAR_Bonding(cp);
	//Enable_SAR_Dummy(cp);
	//printk("[%s, line %d] REG32(0xb8302100) = 0x%08X\n",__func__,__LINE__,reg(0xb8302100));
#endif



}

#ifdef SAR_RX_FORMATION
//free skb buffer in free list
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
void free_rx_free_list(void)
{
	struct sk_buff *skb;
	
free_next_rx_free_list:
	free_rx_skb_consumer++;
	if (free_rx_skb_consumer==SAR_MAX_Process_DESC)
	  free_rx_skb_consumer=0;  //touch end, move to head
	
	if (free_rx_skb_producer==free_rx_skb_consumer)
	    goto stop_free;  //no mroe skb in free list
	    
	skb = free_rx_skb_list[free_rx_skb_consumer];
	if(skb!=(struct sk_buff *)NULL)
		dev_kfree_skb(skb);
	goto free_next_rx_free_list;
	
stop_free:	
	free_rx_skb_producer=0;
	free_rx_skb_consumer=SAR_MAX_Process_DESC-1;	
}
#endif // of SAR_RX_FORMATION

#ifdef AUTO_PVC_SEARCH
void searchPVC(void){
	int i;
	
	//set temp vpi/vci
	SetVpiVci(sar_dev,0,0,0); 
	//Disable_rx_ch(sar_dev,0);	
	for (i = 0; i<Enable_VC_CNT; i++)	
		Disable_rx_ch(sar_dev,i);
}
void startPVCSearch(void)
{
#ifdef CONFIG_RTL8672
	// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
	 sar_private *cp = sar_dev;
#endif	 
	//search possible VC
	//REG16(GIMR)&=(~SAR_IM);
//	search_flag=0;
#ifdef CONFIG_RTL8672
	Disable_Drop_NonOAM(sar_dev,OAM_CH_NO+1);
#else
	Disable_Drop_NonOAM(sar_dev,OAM_CH_NO);
#endif
	//REG16(GIMR)|=SAR_IM;
	
	// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
#ifdef CONFIG_RTL8672
	Enable_rx_ch(cp, OAM_CH_NO+1);
#endif
}
#endif

//#include "../../net/atm/br2684.c"
extern char PrioChannelNum[5];
//tylo, temp. marked for linux2.6 porting
//extern BOOL GetLinkSpeed(char *Rate);
T_LinkSpeed LINE_rate;
char checkRateEnable=0;

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
void set_atm_data_mode(int mode)
{
#if 1
  int io_cmd=0;
  int i;
  sar_private *cp = sar_dev;
  sar_atm_vcc *bakup_vcc[Enable_VC_CNT];
  struct SAR_IOCTL_CFG	cfg;
  struct atm_vcc *vcc;
  struct atm_vcc * tmp_atm_vcc;

	checkRateEnable=1;
    if (mode==1) io_cmd = SAR_UTOPIA_FAST;
    else if (mode==2) io_cmd = SAR_UTOPIA_SLOW;
    
    for(i=0;i<5;i++){
	PrioChannelNum[i]=0;
    }
	
    if (io_cmd!=0) {
		//stop & reset VC structure
		//disable_lx4180_irq(cp->irq);
		//tasklet_disable(&cp->tasklets);
		sar_stop_hw(cp);
	
	    //close VC
		for (i=0;i<Enable_VC_CNT;i++) {
		    if(cp->vcc[i].created==VC_CREATED){
	        	//backup vcc
		        bakup_vcc[i] = kmalloc(sizeof(sar_atm_vcc), GFP_KERNEL);
	        	memcpy(bakup_vcc[i], &cp->vcc[i], sizeof(sar_atm_vcc));        
		    	vcc = cp->vcc[i].dev_data;
		    	sar_close(cp->vcc[i].dev_data);
		    } else {
		    	bakup_vcc[i]=NULL;
		    };
		};
		Disable_rx_ch(cp, OAM_CH_NO);
		Disable_tx_ch(cp, OAM_CH_NO);
		FreeVcBuff(cp, OAM_CH_NO);
		
#ifdef SAR_RX_FORMATION
        //for rx desc re-assign reset
        free_rx_free_list();
#endif // of SAR_RX_FORMATION
        
		//reset current_desc_number, per_vc_desc_number is set in ioctl(SAR_SET_PVC_NUMBER)
		for (i=0;i<total_pvc_number;i++)
		    current_desc_number[i] = per_vc_desc_number;
		
	        
        //set data mode
        if (mode==1)
            cp->atmport = ATMPORT_FAST;
        else
            cp->atmport = ATMPORT_SLOW;

        //restart SAR
        sar_restart_hw(cp);
	
	        //open PVC
		for (i=0;i<Enable_VC_CNT;i++) {
		    if (bakup_vcc[i]==NULL) continue;
		    if (bakup_vcc[i]->created==VC_CREATED){
	        	//restore vcc
	        	memcpy(&cp->vcc[i], bakup_vcc[i], sizeof(sar_atm_vcc));
		    	vcc = cp->vcc[i].dev_data;
				cfg.ch_no = i;
				/* Create one VC first */
				cfg.vpi 	= cp->vcc[i].vpi;
				cfg.vci		= cp->vcc[i].vci;
				/* default setting */
				cfg.rfc		= cp->vcc[i].rfc;
				cfg.framing	= cp->vcc[i].framing;
				cfg.QoS.Type	= sar_to_qos(vcc->qos.txtp.traffic_class);
				if(vcc->qos.txtp.pcr)
					cfg.QoS.PCR = vcc->qos.txtp.pcr;
				else
					cfg.QoS.PCR	= 0x1DFF;
				if(vcc->qos.txtp.scr)
					cfg.QoS.SCR = vcc->qos.txtp.scr;
				else			
					cfg.QoS.SCR	= 0;
				if(vcc->qos.txtp.mbs)
					cfg.QoS.MBS = vcc->qos.txtp.mbs;
				else
					cfg.QoS.MBS	= 128;
				cfg.QoS.CRD	= 0;
				cfg.QoS.CDVT	= 0;
				cp->vcc[i].created = VC_NOT_CREATED;
				CreateVC(cp, &cfg, 0);
			//get net_device
			tmp_atm_vcc=(struct atm_vcc *)cp->vcc[i].dev_data;
			cp->vcc[i].dev = (struct net_device *)tmp_atm_vcc->net_dev;
			//check if this interface is bridged
#if 0			
			if (cp->vcc[i].dev) {
				if(cp->vcc[i].dev->br_port)
					cp->vcc[i].br=1;
				else{
					cp->vcc[i].br=0;
				}
			}
#endif
			
			//printk("net_device: %x\n", (unsigned long)cp->vcc[i].dev);
			if (cp->vcc[i].dev)
				netif_wake_queue(cp->vcc[i].dev);
		    	set_bit(ATM_VF_READY, &vcc->flags);
		    };
	       	kfree(bakup_vcc[i]);
		};
		AllocVcBuff(cp, OAM_CH_NO);
	    if (cp->vcc[OAM_CH_NO].dev_data)  //Remote Management Channel is created
	        Disable_Drop_NonOAM(cp, OAM_CH_NO);
	    else 
	        Enable_Drop_NonOAM(cp, OAM_CH_NO);
		Enable_rx_ch(cp, OAM_CH_NO);
		Enable_tx_ch(cp, OAM_CH_NO);
		//tasklet_enable(&cp->tasklets);
		//enable_lx4180_irq(cp->irq);
		//REG16(GIMR) |= SAR_IM ;
	    }
		
		pp_init_sar_desc(cp, total_pvc_number);
	    Enable_SAR(cp);
	    //GetLinkSpeed(&LINE_rate);
#ifdef AUTO_PVC_SEARCH
		found_pvc=1;	//set as found already, wait for startup to inform driver pvc-search
#endif //AUTO_PVC_SEARCH
	if(sntp_pid) {
		//printk("signal SIGUSR1 to process vsntp (%d)\n", sntp_pid);
		extern long sys_kill(int pid, int sig);
		sys_kill(sntp_pid, SIGUSR1);	//raise sigusr1 to process vsntp
	}
#endif    


	#if 1 // workaround
	printk("applying workaround...");
	REG16(0xb8600c42) = 0x0003;
	REG16(0xb8600c42) = 0x0000;
	printk("done\n");
	#endif
}

int sar_getsockopt(struct atm_vcc *vcc, int level, int optname, void *optval, int optlen)
{
	return -EINVAL;
}

int sar_setsockopt(struct atm_vcc *vcc, int level, int optname, void *optval, int optlen)
{
	return -EINVAL;
}

void sar_phy_put(struct atm_dev *dev,unsigned char value, unsigned long addr)
{
}

unsigned char sar_phy_get(struct atm_dev *dev,unsigned long addr)
{
	return -EINVAL;
}

void sar_feedback(struct atm_vcc *vcc, struct sk_buff *skb, unsigned long start, unsigned long dest, int len)
{
}

int  sar_change_qos(struct atm_vcc *vcc,struct atm_qos *qos,int flags)
{
	sar_private *cp;
	sar_atm_vcc *vcc_dev;
	int ch_no;

	/* if it set before vcc open, it's ok to return no error */
	cp = RTATM_DEV(vcc->dev);
	if(!cp)
		return 0;
	vcc_dev = RTATM_VCC(vcc);
	if(!vcc_dev)
		return 0;

	ch_no = vcc_dev->ch_no;
	SetQoS(cp, ch_no, sar_to_qos(vcc->qos.txtp.traffic_class));
	SetPCR(cp, ch_no, vcc->qos.txtp.pcr);
	SetSCR(cp, ch_no, vcc->qos.txtp.scr);
	SetMBS(cp, ch_no, vcc->qos.txtp.mbs);
	return 0;
}



///////////////////////////////OAM related
#ifdef ATM_OAM
__SWAP
int OAMFSendACell(unsigned char *pCell, int flagOAM){

	sar_private *cp = sar_dev;
	INT16 			j=0, fragNum=1;
	int			currCDOI;
	UINT16			CMD=0;
	TV_CMD_DESC	*TVDesc;
		
	//END_TX_SEM_TAKE (&sar_end->end, WAIT_FOREVER);

	cp->vcc[OAM_CH_NO].stat.send_cnt++;

	/* Clear transmitted descriptors */
	ClearTxBuffer(cp, OAM_CH_NO);


	{
		static int oam_tx_counter = 0;
		oam_tx_counter++;
		if(oam_tx_counter%10000 == 1)
			printk("OAM tx %d\n",oam_tx_counter);
	}

#ifdef Do_Make
	/*if no descriptor available, clear TBE if needed */
	if(((((cp->vcc[OAM_CH_NO].TV.desc_pw+1)%SAR_TX_DESC_NUM)==cp->vcc[ch_no].TV.desc_pf)&&	
		(cp->TODesc[cp->vcc[OAM_CH_NO].TV.desc_pw].CMD&OWN)!=0)){

		cp->vcc[OAM_CH_NO].stat.send_desc_full++;	/* descriptors full*/
		
		/* get current CDOI */
		currCDOI=GetTxCDOI(cp,OAM_CH_NO);

		/* if TBE occurs and current descriptor is own by hardware (that means last time TBE occurs, 
			and the desriptors has been refilled), clear TBE and make this channel be scheduled again */
		if ((reg(SAR_TBEI_Addr)&(0x00000001<<OAM_CH_NO))&&((cp->TODesc[currCDOI].CMD)&OWN))
			Clear_TBE(OAM_CH_NO, currCDOI);

		goto DropPacket;
	}


	/* if not enough descriptors, drop this packet */
	avail_desc = (cp->vcc[OAM_CH_NO].TV.desc_pf- 1 - cp->vcc[OAM_CH_NO].TV.desc_pw + SAR_TX_DESC_NUM)%SAR_TX_DESC_NUM;
	if (avail_desc <= fragNum) 
	{		
		cp->vcc[OAM_CH_NO].stat.send_desc_lack++;		
		goto DropPacket;
	}    
#endif

        /* Fix the unstable OAM ping -shlee */
        dma_cache_wback_inv((unsigned long)pCell, 52);
	/* Get Current Descriptor Index */
	j= cp->vcc[OAM_CH_NO].TV.desc_pw;

	/* if this Descriptor is OWN by SAR*/
	if(cp->TODesc[j].CMD&OWN){
		goto Drop_OAM_Cell;
	} 	

	/* Handle this descriptor */
	TVDesc=(TV_CMD_DESC *)(&cp->TODesc[j]);
	CMD=0;
		
	cell_debug(pCell, debug_obcell, 1);
	/* fill 1483 header if first descriptor*/
	TVDesc->START_ADDR = (uint32)pCell|Uncache_Mask;
	TVDesc->LEN = 52;		
	cp->vcc[OAM_CH_NO].tx_buf[j]=(UINT32)pCell;		
		
	/* Set FS LS */
	CMD |= (FS|LS);
		
	CMD = CMD 
			|(flagOAM?CRC10EN:0)	/* Enable CRC10 Generate if OAM cell*/
#ifdef CONFIG_RTL8672
//			|0x0800  /* fix to ATM port 1*/
#else
			|cp->atmport/*use ATMPORT 1 */
#endif					
			|OWN;	/* set owner */

			if(sendToPort0){
				//atmport 1
				CMD |= 0x0800;
			}

	if(j==(SAR_TX_DESC_NUM-1))		
		CMD |= EOR;
	
	/* save the CMD to Descriptor */
	TVDesc->CMD=CMD;

	cp->vcc[OAM_CH_NO].TV.desc_pw = (cp->vcc[OAM_CH_NO].TV.desc_pw + 1)%SAR_TX_DESC_NUM;

	currCDOI = GetTxCDOI(cp, OAM_CH_NO);
	
	/* if TBE occurs and the current descriptor is owned by hardware, re-schedule this channel */
	if (reg(SAR_TBEI_Addr)&(0x00000001<<HW_OAM_CH_NO)) {
	//if ((reg(SAR_TBEI_Addr)&(0x00000001<<HW_OAM_CH_NO))&&((cp->TVDesc[OAM_CH_NO*SAR_TX_RING_SIZE+currCDOI].CMD)&OWN)) {
		Clear_TBE(OAM_CH_NO, currCDOI);
	};
		

	/* Clear transmitted descriptors AGAIN */
	//ClearTxBuffer(cp, OAM_CH_NO);
	cp->vcc[OAM_CH_NO].stat.send_ok++;
	cp->vcc[OAM_CH_NO].stat.tx_byte_cnt += 53;
//	END_TX_SEM_GIVE (&sar_end->end);
	return 1;

Drop_OAM_Cell:
	//printk("  tx OAM Cell dropped, available descriptors = %d\n",avail_desc);
	cp->vcc[OAM_CH_NO].stat.send_fail++;
	cp->vcc[OAM_CH_NO].stat.tx_pkt_fail_cnt+=fragNum;
	kfree(pCell);  //5/25/04' hrchen, fix pCell not free bug
	//END_TX_SEM_GIVE (&sar_end->end);
	return 0;

}


/*void  OAMFReceiveCell(unsigned int type, unsigned char *pCell){

	unsigned char *pcell2;

	pcell2 = (unsigned char *)malloc(SAR_OAM_Buffer_Size);
	memcpy(pcell2, pCell, SAR_OAM_Buffer_Size);
	OAMFSendACell(pcell2, 1);
	return;

}*/

#if 0
typedef struct
{
  unsigned gfc:4; 	/* Generic Flow Control;      */ 
  unsigned vpi:8;	/* Virtual Path Identifier    */ 
  unsigned vci:16;	/* Virtual Channel Identifier */ 
  unsigned pti:3; 	/* Payload type Identifier    */
  unsigned clp:1; 	/* Cell Loss Priority         */
  unsigned oam:4;
  unsigned fun:4;
} Cell_header;  // AAL0 channel used 

typedef struct
{
  unsigned long hdr0;
  unsigned char	hdr1;	
  unsigned char	loopbk;
  unsigned long	ctag;
  unsigned char	lbk_loc_id[16];
  unsigned char	lbk_src_id[16];
  unsigned char	unused[8];
} lbk_spec_field;

#define get_gfc(x)	(x->hdr0>>28)
#define get_vpi(x)	(x->hdr0>>20)&0xFF
#define get_vci(x)	(x->hdr0>>4)&0xFFFF
#define get_pti(x)	(x->hdr0>>1)&0x07
#define get_clp(x)	(x->hdr0&0x01)
#define get_oam(x)	(x->hdr1>>4)
#define get_fun(x)	(x->hdr1&0x0f)

void process_aal0(sar_private *cp, unsigned char *mp)
{
   	lbk_spec_field	*pCell;
	int	i,vc;

	pCell = (lbk_spec_field*)mp;

    if ( get_vci(pCell) == 3 || get_vci(pCell) == 4) {
    	/* F4 end-to-end and s-to-s cell */
    	if( (get_oam(pCell)==1) && (get_fun(pCell)==8) ) {
			/* loopback type */
           	if (get_vci(pCell) == 3)
          		printk("Rx F4 SEG LB cell\n");
           	else
           		printk("Rx F4 ETE LB cell\n");

			/* check if VPI is exist */
            
            /* process loopback function field */
    	    if(pCell->loopbk) {
        		/* the cell should be loopback */
        		/* LoopBack identifier decrement 1 */
	           	pCell->loopbk = 0;   
    	    }
            else {
            	/* the cell is a loopback response */
				/* try to pass to ATM socket */
            	return;
            }
        } 
        else if((get_oam(pCell)==1) && (get_fun(pCell)==0)) {
	        /* AIS received */
			if (get_vci(pCell) == 3)
				printk("Rx F4 SEG AIS cell\n");
			else
               	printk("Rx F4 ETE AIS cell\n");

			/* Func type set RDI */
        	get_fun(pCell)=0x1;
			/* generate RDI payload */
        	for(i=5;i<(5+17);i++)
        		mp[i]=0x6A;     // RDI default value 
        	mp[i]=0x00;
        }	    
        else {
        	printk("Unprocessed raw cell, discard\n"); 
        	return;
        }
    }
         
	else if( get_pti(pCell) == 4 || get_pti(pCell) == 5) {
		/* F5 end-to-end and s-to-s cell */
		if( (get_oam(pCell)==1) && (get_fun(pCell)==8) ) {
			/* LoopBack type */
			if (get_pti(pCell) == 4)
				printk("Rx F5 SEG LB cell\n");
			else
				printk("Rx F5 ETE LB cell\n");
            /* search active vccs */
            	
	        /* process loopback function field */
    	    if(pCell->loopbk) {
        		/* the cell should be loopback */
				/* LoopBack identifier decrement 1 */
	           	pCell->loopbk = 0;
    	    }
        	else {
            	/* the cell is a loopback response */
				/* try to pass to ATM socket */
	           	return;
			}
		}
        
        else if((get_oem(pCell)==1) && (get_fun(pCell)==0))  {
			/* AIS received */
			if (get_pti(pCell) == 4)
				printf("Rx F5 SEG AIS cell\n");
			else
				printf("Rx F5 ETE AIS cell\n");

			/* search active vccs */

			/* Func type set RDI */
        	get_fun(pCell) = 0x1;
			/* generate RDI payload */
        	for(i=5;i<(5+17);i++)
        		mp[i]=0x6A;     // RDI default value 
        	mp[i]=0x00;
      	}
      	else { 
      		printk("Unprocessed raw cell, discard\n"); 
      		return;
      	}
	}
    else {
    	printk("Unprocessed raw cell, discard\n"); 
    	return;
    }

	/* response OAM cell */
	{
	unsigned char *pRCell = (unsigned char *) kmalloc(OAM_CELL_SIZE);
		if(pRCell == NULL)
			return;
		memcpy(pRCell, mp, OAM_CELL_SIZE);
		OAMFSendACell(pRCell, 1);
		kfree(pRCell);
	}
}
#endif

#endif


static const struct atmdev_ops ops = {
	.owner = THIS_MODULE,
	.open = 		sar_open,
	.close =		sar_close,
	.ioctl =		sar_ioctl,
	.getsockopt=	sar_getsockopt,
	.setsockopt=	sar_setsockopt,
#if 0
	.send =		sar_send_auto,//sar_send,
#else	
	.send =		sar_send,
#endif	
	/*sar_sg_send*/
	.phy_put =	sar_phy_put,
	.phy_get =	sar_phy_get,
	//feedback:	sar_feedback,
	.change_qos =	sar_change_qos,
};



static void __exit sar_exit (void)
{
}

#ifdef REMOTE_MANAGEMENT_ENABLE
extern void init_crc32_table(void);
#endif

// Kaohj -- /proc/sys/sar
static struct ctl_table_header *sar_table_header;
extern int netdev_backlog_drop;	// net/core/dev.c
static ctl_table sar_table[]= {
#ifdef 	CONFIG_FLOW_CTRL  
	{.ctl_name	= 3,
	 .procname	= "flow_ctrl",
	 .data		= &FCCtrl,
	 .maxlen	= sizeof(FCCtrl),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
	{.ctl_name	= 4,
	 .procname	= "fc_low",
	 .data		= &FC_low,
	 .maxlen	= sizeof(FC_low),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
	{.ctl_name	= 5,
	 .procname	= "fc_high",
	 .data		= &FC_high,
	 .maxlen	= sizeof(FC_high),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
#endif	 
#ifdef FAST_ROUTE_Test
	{.ctl_name	= 3,
	 .procname	= "direct_bridge",
	 .data		= &fast_route_mode,
	 .maxlen	= sizeof(fast_route_mode),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
#endif
#if 0
	{.ctl_name	= 3,
	 .procname	= "rx0",
	 .data		= &current_desc_number[0],
	 .maxlen	= sizeof(current_desc_number[0]),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
	{.ctl_name	= 3,
	 .procname	= "rx1",
	 .data		= &current_desc_number[1],
	 .maxlen	= sizeof(current_desc_number[1]),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
#endif	 
#ifdef SAR_RX_FORMATION
	{.ctl_name	= 3,
	 .procname	= "rx_formation",
	 .data		= &do_formation,
	 .maxlen	= sizeof(do_formation),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
#endif // of SAR_RX_FORMATION
	{.ctl_name	= 3,
	 .procname	= "netdev_backlog_drop",
	 .data		= &netdev_backlog_drop,
	 .maxlen	= sizeof(netdev_backlog_drop),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
#ifdef CONFIG_SAR_FASTSKB
	{.ctl_name	= 3,
	 .procname	= "sar_fastskb",
	 .data		= &enable_sar_fastskb,
	 .maxlen	= sizeof(enable_sar_fastskb),
	 .mode		= 0644,
	 .proc_handler	= &proc_dointvec },
#endif
	{}
};

static ctl_table sar_root_table[]= {
	{.ctl_name	= 2,
	 .procname	= "sar",
	 .mode		= 0644,
	 .child		= sar_table },
	{}
};


int __init rtl8670_atm_detect (void)
{
	struct atm_dev *dev;
	sar_private	*cp;
	int devs = 1;

//	cp = (sar_private *) kmalloc(sizeof(sar_private), GFP_KERNEL);
	cp = (sar_private *) kzalloc(sizeof(sar_private), GFP_KERNEL|__GFP_NOFAIL);

	if (!cp)
		return -ENOMEM;
    
// replaced by kzalloc	memset(cp, 0, sizeof(sar_private));
	sar_dev = cp;
	dev = atm_dev_register(DEV_LABEL, &ops, -1, NULL);
	
	//RTATM_DEV(dev) = cp;
	 (dev)->dev_data = cp;

	if(sar_init(dev)) {
		atm_dev_deregister(dev);
		return 0;
	}
	cp->dev = dev;

	printk (KERN_INFO "%s: %s %s (%s)\n",
		dev->type, DRV_NAME, DRV_VERSION, DRV_RELDATE);

#ifdef CONFIG_RTL8681
	// DSP lib select the ptm/atm mode, and don't selet mode here
	REG32(BSP_IP_SEL) |= (BSP_EN_SAR | BSP_EN_SACHEM);

	gpioClear(GPIO_A_2);
	gpioConfig(GPIO_A_2,GPIO_FUNC_OUTPUT);
	gpioClear(GPIO_A_3);
	gpioConfig(GPIO_A_3,GPIO_FUNC_OUTPUT);
	gpioClear(GPIO_A_4);
	gpioConfig(GPIO_A_4,GPIO_FUNC_OUTPUT);
	gpioClear(GPIO_A_5);
	gpioConfig(GPIO_A_5,GPIO_FUNC_OUTPUT);
	gpioClear(GPIO_A_6);
	gpioConfig(GPIO_A_6,GPIO_FUNC_OUTPUT);

	gpioClear(GPIO_B_1);
	gpioConfig(GPIO_B_1,GPIO_FUNC_OUTPUT);
#endif	

#ifdef AUTO_PVC_SEARCH
	APS_Proc_File= create_proc_entry(PROCFS_NAME, 0644, NULL);
	if (APS_Proc_File == NULL) {
		/*linux-2.6.19*/
		//remove_proc_entry(PROCFS_NAME, &proc_root);
		remove_proc_entry(PROCFS_NAME, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME);
		return -ENOMEM;
	}

	APS_Proc_File->read_proc  = procfile_read;
	APS_Proc_File->write_proc  = procfile_write;
	/*linux-2.6.19*/
	//APS_Proc_File->owner 	  = THIS_MODULE;
	APS_Proc_File->mode 	  = S_IFREG | S_IRUGO;
	APS_Proc_File->uid 	  = 0;
	APS_Proc_File->gid 	  = 0;
	APS_Proc_File->size 	  = 37;

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);	
#endif
	// Kaohj -- /proc/sys/sar
	//sar_table_header = register_sysctl_table(sar_root_table, 0);
	/*linux-2.6.19*/
	sar_table_header = register_sysctl_table(sar_root_table);
	/*
	 * Looks like this is necessary to deal with on all architectures,
	 * even this %$#%$# N440BX Intel based thing doesn't get it right.
	 * Ie. having two NICs in the machine, one will have the cache
	 * line set at boot time, the other will not.
	 */
	return devs;

}

/*
 * Traffic tracker called by gpio.c for Link LED blinking
 */
int IsTrafficOnAtm(void)
{
	if(latestTraffic != AtmTraffic){
		latestTraffic = AtmTraffic;
		return 1;
	} else {
		latestTraffic = AtmTraffic = 0;
		return 0;
	}
}

// Kaohj
/*
 *	Return value:
 *	-1: fail
 *	0 ~ Enable_VC_CNT-1 : ok
 */
int index_vc2ch(int vc_idx)
{
	sar_private *cp = sar_dev;
	int ch_no;
	int idx;
	char ifname[] = "vc0";
	
	ifname[2] += vc_idx;
	ch_no = -1;
	for (idx = 0; idx < Enable_VC_CNT; idx++) {
		if (cp->vcc[idx].dev && !strcmp(ifname, cp->vcc[idx].dev->name)) {
			ch_no = idx;
			break;
		}
	}
	
	return ch_no;
}

/*
 *	Return value:
 *	-1: fail
 *	0 ~ Enable_VC_CNT-1 : ok
 */
int index_dev2ch(struct net_device* dev)
{
	sar_private *cp = sar_dev;
	int ch_no;
	int idx;
	
	ch_no = -1;
	for (idx = 0; idx < Enable_VC_CNT; idx++) {
		if (cp->vcc[idx].dev == dev) {
			ch_no = idx;
			break;
		}
	}
	
	return ch_no;
}

void vc_ch_remapping(unsigned *member)
{
	int ch_no;
	int j;
	unsigned tmp_mbr = 0, msk = 0x10000, mbr = *member;
	// | resvd | vc7 vc6 vc5 vc4 | vc3 vc2 vc1 vc0 | vap3 | vap2 vap1 vap0 wlan0 | resvd | LAN4 LAN3 LAN2 LAN1

	if( (mbr & 0xFF0000) == 0 )  // Skip it if this vlan group doesn't contain any vc channels.
		return;

	tmp_mbr = mbr & 0xff00ffff; // mask vc bits

	// remap vc bits
	for( j = 0; j < 8; j++ )
	{
		if( mbr & (msk << j) )
		{
			// vcj
			ch_no = index_vc2ch(j);
			if( ch_no >= 0 )
				tmp_mbr |= msk << ch_no;
		}
	}

	*member = tmp_mbr;
	//printk("set %s ch_no %d to br; member = 0x%.08x; tmp_mbr = 0x%.08x\n", cp->vcc[ch_no].dev->name, ch_no, mbr, tmp_mbr);
}

