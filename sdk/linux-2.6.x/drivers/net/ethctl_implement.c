#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/netdevice.h>
#include "ethctl_implement.h"
#include <bspchip.h>

#ifdef CONFIG_RTL8672
#include "../../arch/rlx/bsp/gpio.h"
#endif


#ifdef CONFIG_EXT_SWITCH
#include "./rtl8306/Rtl8306_types.h"
#ifdef CONFIG_RE8306_API
#ifdef CONFIG_ETHWAN
#include "./rtl8306/rtl8306e_asicdrv.h"
#include "./rtl8306/rtk_api.h"
#endif //CONFIG_ETHWAN
#endif // of CONFIG_RE8306_API
#endif // of CONFIG_EXT_SWITCH

#ifdef CONFIG_RTL8676_Dynamic_ACL
extern int rtl8676_clean_L34Unicast_hwacc(void);
#endif



#ifdef CONFIG_RTL8672NIC
#include "re830x.h"
#include "re867x.h"
#include "re_smi.h"
#include "re_igmp.h"
#define test_len 500
extern int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev);
extern unsigned int eth_rx_count;
extern unsigned int iocmd_reg;
extern struct net_device *eth_net_dev;
struct timer_list rxtest_timer;
unsigned char rxtest_running=0;
struct sk_buff testskb;
unsigned char testskbdata[test_len];
void show_rx(void){
	//printk("ethernet rx count:%d\n",eth_rx_count);
	//rxtest_running=0;
	if(rxtest_running==0)
		return;
	printk("time out!\n");
	rxtest_timer.expires=jiffies+HZ;
	rxtest_timer.function=(void (*)(unsigned long))show_rx;
	eth_rx_count=0;
	add_timer(&rxtest_timer);
}
#ifdef CONFIG_EXT_SWITCH
void dump_vlan_info(void) // Kaohj --- dump VLAN info (rtl8305_info)
{
	int i;
	
	printk("phy num=%d; vlan num=%d, enabled=%d\n\n", MAX_SWITCH_PORT+4, VLAN_NUM, rtl8305_info.vlan_en);
	printk("phy info:\n");
	printk("phy     vlanIndex\n");
	for (i=0; i<MAX_SWITCH_PORT+4; i++)
		printk("%d      %d\n", i, rtl8305_info.phy[i].vlanIndex);
	printk("\nvlan info\n");
	printk("index    vid    member\n");
	for (i=0; i<VLAN_NUM; i++)
		printk("%d       %d     0x%x\n", i, rtl8305_info.vlan[i].vid, rtl8305_info.vlan[i].member);
	printk("\n");
}
extern void dump_igmp_info(void);
#endif
#ifdef CONFIG_ETHWAN
extern unsigned int debug_enable;
#endif
#endif
int enable_ipqos=0;

#if defined(CONFIG_RTL_819X_SWCORE) && defined(CONFIG_RTL_8367B)
#include <net/rtl/rtl_types.h>
#include "rtl819x/AsicDriver/rtl865x_asicCom.h"
#include "rtl819x/AsicDriver/rtl865x_asicL2.h"
#include "rtl8367b/port.h"
extern int global_probe_extPhy;
#endif
#ifdef CONFIG_NF_CONNTRACK
extern void nf_conntrack_flush(struct net *net, u32 pid, int report);
#endif
void eth_ctl(struct eth_arg * arg){
	unsigned char cmd1;
	unsigned int cmd2,cmd3;
	unsigned short cmd4;
	unsigned int cmd5;
	struct net_device *dev;	

	#ifdef CONFIG_RTL8672NIC
	unsigned int i;
	struct skb_shared_info skbshare;
	#endif

	#if defined(CONFIG_RTL_819X_SWCORE) && defined(CONFIG_RTL_8367B)
	unsigned int data;
	#endif

	
	cmd1=arg->cmd;
	cmd2=arg->cmd2;
	cmd3=arg->cmd3;
	cmd4=(unsigned short)arg->cmd4;
	cmd5 = arg->cmd5;
	


	switch(cmd1){
		#ifdef CONFIG_RTL8672NIC
		case 1: //tx
			for(i=0;i<test_len;i++){
				testskbdata[i]=i%0xff;
			}
			skbshare.nr_frags=0;
			testskb.end=(unsigned char*)&skbshare;

			testskb.data=testskbdata;
			testskb.len=test_len;
			
			
			re8670_start_xmit(&testskb,  eth_net_dev);
			
			mdelay(1);
			break;
		case 2://rx
			if(rxtest_running==1){
				rxtest_running=0;
				break;
			}
			rxtest_timer.expires=jiffies+cmd2*HZ;
			rxtest_timer.function=(void (*)(unsigned long))show_rx;
			eth_rx_count=0;
			rxtest_running=1;
			add_timer(&rxtest_timer);
			break;
		#endif
		#if defined(CONFIG_EXT_SWITCH)
		case 3://phy on
			rtl8305sc_setPower(cmd2, 1);
			break;

		case 4://phy off
			rtl8305sc_setPower(cmd2, 0);
			break;

		case 5://read phy
			// Kaohj -- Select PHY Register page through configuring PHY 0 Register 16 [bit1 bit15]
			select_page(cmd4);
			miiar_read(cmd2,cmd3,&cmd4);
			printk("PHY%d.REG%d: 0x%04X\n", cmd2, cmd3, cmd4);
			select_page(0);
			break;
			
		case 6://write phy
			// Kaohj -- Select PHY Register page through configuring PHY 0 Register 16 [bit1 bit15]
			select_page(cmd5);
			miiar_write(cmd2,cmd3,cmd4);
			printk("wPHY%d.REG%d: 0x%04X\n", cmd2, cmd3, cmd4);
			miiar_read(cmd2,cmd3,&cmd4);
			printk("rPHY%d.REG%d: 0x%04X\n", cmd2, cmd3, cmd4);
			select_page(0);
			break;
			
		case 7://LED all on
			break;
		#elif defined(CONFIG_RTL_819X_SWCORE) && defined(CONFIG_RTL_8367B)
		//for 0412 hw nat
		case 5://read phy
			if (RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B == global_probe_extPhy) {
				RL6000_get_phy(cmd2, cmd3, &data, cmd4);
				printk("rtl8367b ");
			}
			else {
				if (!global_probe_extPhy && (7 == cmd2))	//phyid=7 is only for ext. phy: rtl8211E
					break;
				//select page
				rtl8651_setAsicEthernetPHYReg(cmd2, 31, cmd4);
				//read phy reg
				rtl8651_getAsicEthernetPHYReg(cmd2, cmd3, &data);
				//select page 0
				rtl8651_setAsicEthernetPHYReg(cmd2, 31, 0);
				printk("%s ", ((RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8211E == global_probe_extPhy) && (7 == cmd2)) ? 
					"rtl8211e":"rtl8676");
			}
			printk("PHY%d.REG%d: 0x%04X\n", cmd2, cmd3, data);
			break;
			
		case 6://write phy
			data = cmd4;
			if (RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B == global_probe_extPhy) {
				RL6000_set_phy(cmd2, cmd3, data, cmd5);
				printk("rtl8367b ");
			}
			else {
				if (!global_probe_extPhy && (7 == cmd2))	//phyid=7 is only for ext. phy: rtl8211E
					break;
				//select page
				rtl8651_setAsicEthernetPHYReg(cmd2, 31, cmd5);
				//write phy reg
				rtl8651_setAsicEthernetPHYReg(cmd2, cmd3, data);
				printk("%s ", ((RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8211E == global_probe_extPhy) && (7 == cmd2)) ? 
					"rtl8211e":"rtl8676");
			}
			printk("wPHY%d.REG%d: 0x%04X\n", cmd2, cmd3, data);

			if (RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B == global_probe_extPhy) {
				RL6000_get_phy(cmd2, cmd3, &data, cmd5);
				printk("rtl8367b ");
			}
			else {
				//read phy reg
				rtl8651_getAsicEthernetPHYReg(cmd2, cmd3, &data);
				//select page 0
				rtl8651_setAsicEthernetPHYReg(cmd2, 31, 0);
				printk("%s ", ((RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8211E == global_probe_extPhy) && (7 == cmd2)) ? 
					"rtl8211e":"rtl8676");
			}
			printk("rPHY%d.REG%d: 0x%04X\n", cmd2, cmd3, data);
			break;
		#endif
		#ifdef CONFIG_RTL8672NIC
		case 8:
			//for wifi test
			//modify  RX_MIT
			if(cmd2==1) //wifi test
				iocmd_reg = 0x3c | 1 << 8 | RX_FIFO << 11 |  RX_TIMER << 13 | TX_MIT << 16 | TX_FIFO<<19;
			else
				iocmd_reg=CMD_CONFIG;
			
			printk("(cmd2 = %d) set iocmd_reg:%x\n",cmd2,iocmd_reg);
			
			break;
		#endif
		case 9:
			break;	
		case 10:  //GPIO test
		    	gpioConfig(cmd2, GPIO_FUNC_OUTPUT);
		    	if (cmd3 == 0)
		    		gpioClear(cmd2);
		    	else
		    		gpioSet(cmd2);
		    	break;
		// Added by Mason Yu for PPP LED		
		case 12:  //PPP on	
		#ifdef CONFIG_GPIO_LED_CHT_E8B		
		    g_ppp_up++;
		#endif
		#if defined(CONFIG_STD_LED)
			gpio_LED_PPP(1);
		#elif defined(CONFIG_GPIO_LED_CHT_E8B)           
            gpio_LED_PPP(1);
		#endif
			break;
		
		// Added by Mason Yu for PPP LED
		case 13:  //PPP off	
		#ifdef CONFIG_GPIO_LED_CHT_E8B			
            g_ppp_up--;
			if (g_ppp_up<0)
				g_ppp_up = 0;
		#endif
		#if defined(CONFIG_STD_LED)				
			gpio_LED_PPP(0);
		#elif defined(CONFIG_GPIO_LED_CHT_E8B)
                    if (g_ppp_up == 0)
                       gpio_LED_PPP(0);
		#endif 	
			break;
			
		//7/13/06' hrchen, watchdog command
		case 14: {
		#if 1	//shlee 8672	
            extern int update_watchdog_timer(void);
            extern void start_watchdog(int);
            extern void stop_watchdog(void);
            extern void get_watchdog_status(void);
            extern void set_watchdog_kick_time(int kick_sec);
            extern void set_watchdog_timeout_time(int kick_sec);
            switch (cmd2) {
            	case 1:  //update watchdog
            	  arg->cmd3=update_watchdog_timer();//for kick cycle time update in watchdog task
            	  break;
            	case 2:  //enable hw watchdog
            	  start_watchdog(cmd3);
            	  break;
            	case 3:  //disable hw watchdog
            	  stop_watchdog();
            	  break;
            	case 4:  //set watchdog kick time
            	  set_watchdog_kick_time(cmd3);
            	  break;
            	case 5:  //get watchdog status
            	  get_watchdog_status();
            	  break;
            	case 6:  //set watchdog timeout time
            	  set_watchdog_timeout_time(cmd3);

            	  break;

            }
		#endif            	              	              
            break;
        };
		#ifdef CONFIG_GPIO_LED_CHT_E8B			
	   case 15:
	   	
			g_internet_up = 1;
			break;

	   case 16:
			g_internet_up = 0;
			break;
		#endif	
		case 21: // conntrack killall
			//drop_all_conntrack();
			/*linux-2.6.19*/
			//ip_conntrack_flush();
			#ifdef CONFIG_NF_CONNTRACK
			nf_conntrack_flush(&init_net,0,0);
			#endif /*CONFIG_NETFILTER*/

			#ifdef CONFIG_RTL8676_Dynamic_ACL
			rtl8676_clean_L34Unicast_hwacc();
			#endif
			break;

		// Kaohj
		#ifdef CONFIG_EXT_SWITCH
		#ifdef CONFIG_RE8306_API
		case 26: // dump 8306 qos
			dump_8306_qos();
			break;
		case 27: // reset 8306 mib counters
			reset_8306_counter();
			break;
		case 28: // dump 8306 mib counters
			dump_8306_counter();
			break;
		#endif
		case 29: // dump vlan info
			dump_vlan_info();
			break;
		case 30: // dump igmp info
			dump_igmp_info();
			break;
		#endif // of CONFIG_EXT_SWITCH
		#ifdef CONFIG_RTL8672NIC
		case 31:    //polling switch's port link status at user space for e8b case.
			miiar_read(arg->cmd2, 1, &cmd4);
			if(cmd4&4)
				arg->cmd=1;
			else
				arg->cmd=0;
			break;		
		case 40:
			if (cmd2 == 1) { // igmp
				if (cmd3 > 0)
					debug_igmp = 1;
				else
					debug_igmp = 0;
			}
			break;
		#endif 
		case 41: // nic phy power up
			//printk("Enable NIC phy\n");
			if(!IS_RTL8676) {
				cmd2 = *(volatile u32*)(BSP_IP_SEL);
				cmd2 &= 0xfbffffff; // bit-26: Power down IntPhy PCS
				*(volatile u32*)(BSP_IP_SEL) = cmd2;
			}
			break;
		case 42: // nic phy power down
			//printk("Disable NIC phy\n");
			if(!IS_RTL8676) {
				cmd2 = *(volatile u32*)(BSP_IP_SEL);
				cmd2 |= 0x04000000; // bit-26: Power down IntPhy PCS
				*(volatile u32*)(BSP_IP_SEL) = cmd2;
			}
			break;
		#if defined(CONFIG_EXT_SWITCH) && defined(CONFIG_RE8306_API) && defined(CONFIG_ETHWAN)
		case 43:
			printk("--- switch8306info ---\n");
			switch8306info();
			break;
		case 44:
			printk("--- switch8306alive ---\n");
			switch8306alive();
			break;
		case 45:
			//0:disable, 1:all, 2:tx, 3:rx
			debug_enable = cmd2;
			printk("set debug_enable to %d\n", debug_enable);
			break;
		case 46:	
			if(cmd2 == 0){//reset
				reset_rtk8306_igrAcl_rule_by_user();
				printk("reset sip port\n");
			}
			else if(cmd2 == 1){//set port
				add_rtk8306_igrAcl_rule_by_user(arg->cmd3);
				printk("set sip/rtp port %d \n",arg->cmd3);
			}
			break;
		#endif

		case 50:
			if(cmd2==1){
				printk("enable ipqos!!!\n");
				enable_ipqos=1;
			}
			else{
				printk("disable ipqos!!!\n");
				enable_ipqos=0;
			}
			break;

#if defined(CONFIG_RTL_819X_SWCORE) && defined(CONFIG_RTL_8367B)
		case 51:	//read mac
			if (RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B == global_probe_extPhy) {
				RL6000_read(cmd2, &data);
				printk("rtl8367b read REG0x%x: 0x%04X\n", cmd2, data);
			}
			else
				printk("rtl8367b is not detected!\n");
			break;
		case 52:	//write mac
			if (RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B == global_probe_extPhy) {
				data = (cmd3&0xffff);
				RL6000_write(cmd2, data);
				
				printk("rtl8367b write REG0x%x: 0x%04X\n", cmd2, data);
				RL6000_read(cmd2, &data);
				printk("rtl8367b read REG0x%x: 0x%04X\n", cmd2, data);
			}
			else
				printk("rtl8367b is not detected!\n");
			break;
#endif
		case 53: //enable_usb0_wan
			dev = dev_get_by_name(&init_net,"usb0");
			if(dev==NULL)
			{
				printk("Error ! can't find usb0 device\n");
				return;
			}

			if(cmd2==1){		
				printk("Enable Wan flag for usb0\n");
				dev->priv_flags |= IFF_DOMAIN_WAN;
			}
			else{
				printk("Disable Wan flag for usb0\n");
				dev->priv_flags &= ~IFF_DOMAIN_WAN;
			}
			dev_put(dev);		
			break;
		
		default:
			printk("error cmd\n");
	}
}

