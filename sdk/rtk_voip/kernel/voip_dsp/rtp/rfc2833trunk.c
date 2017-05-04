/*
	RFC 2833 trunk event
*/
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#ifdef CONFIG_ARCH_LUNA_SLAVE
#include "bspchip.h"
#endif

#include "dsp_main.h"
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_dev.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "snd_mux_slic.h"
#include "../voip_manager/voip_mgr_events.h"

// trunk event id
#define EVENT_RingOn	144
#define EVENT_ReverseBattery 148
#define EVENT_OnHook	149
#define EVENT_RingOff	149
#define EVENT_OffHook	159

// telephone hook status
#define PHONE_ONHOOK	0
#define PHONE_OFFHOOK	1

#define NAME_SIZE 10

#ifndef CONFIG_ARCH_LUNA_SLAVE
#define REG32(reg)      (*(volatile unsigned int   *)((unsigned int)reg))
#define BSP_TC5_IRQ 68
#define BSP_TC_BASE         0xB8003200
#define BSP_TC5CNT          (BSP_TC_BASE + 0x54)
#define BSP_TC5INT          (BSP_TC_BASE + 0x5C)
#define BSP_TCIP             (1 << 16)
#endif

extern unsigned char RtpOpen[];
extern uint32 rtpConfigOK[];
extern int32 isTranMode(uint32 sid);
extern int RtpTx_transmitEvent_ISR( uint32 chid, uint32 sid, int event);
extern void trunkevent_send(void);

void RFC2833TrunkEventEnable(int chid, unsigned char bEnable);
int RFC2833GenerateTrunkEvent(int chid, unsigned char signal);
int RFC2833TrunkEventOut(int chid, int event);
int RFC2833TrunkEventIn(int chid, int event);

struct timer2_priv{
	char name[NAME_SIZE];
};
static struct timer2_priv timer2ctrl_devices[ 1 ];
static int eventEnable[2] = {0, 0};


void RFC2833TrunkEventEnable(int chid, unsigned char bEnable)
{	
	if ( (chid > 1) || (chid < 0) )
	{
		printk("[%s] chid out of range\n", __FUNCTION__);
		return;
	}
	
	eventEnable[chid] = bEnable;
#ifdef CONFIG_ARCH_LUNA_SLAVE
	if ( bEnable == 0 )
		disable_irq(BSP_TC5_IRQ);
	else if ( bEnable == 1 )
		enable_irq(BSP_TC5_IRQ);
#endif		
}

int RFC2833GenerateTrunkEvent(int chid, unsigned char signal)
{
	extern unsigned char rfc2833_dtmf_pt_local[];
	extern unsigned char rfc2833_dtmf_pt_remote[];
	extern int32 chanInfo_GetRegSessionRank(uint32 chid, uint32 rank[]);
	/**********************************************                 
	 *                               	      *                 
	 * 	    Send RFC2833 Trunk	Event	      *                 
	 *                                  	  *                 
	 ****************************************************/	
		               
	if ( (chid > 1) || (chid < 0) )
	{
		printk("[%s] chid out of range\n", __FUNCTION__);
		return 0;
	}					   
					   
	if ( !eventEnable[chid] )
		return 0;
	
	int i, j, SessNum;
	int event = (int)signal;
	uint32 pSessRank[ 2 ];
		 
	/* 
	 * send_2833_flag_ss is used to decide to send RFC2833 packets or not.
	 * According to payload type of local and remote UA, it will change the send_2833_flag_ss result.
	 * If payload type of local and remote UA is equal to ZERO, it means DTMF Inband.
	 */
	PRINT_MSG("[%s] channel id = [%d], signal = 0x[%x]\n", __FUNCTION__, chid, signal);
	chanInfo_GetRegSessionRank(chid, pSessRank);

	SessNum = chanInfo_GetRegSessionNum(chid);

	for (j=0; j < SessNum; j++)
	{

		if( pSessRank[j] == SESSION_NULL )
			continue;
			
//		printk("[%s] rfc2833_dtmf_pt_local[pSessRank[%d]] = [%d], rfc2833_dtmf_pt_remote[pSessRank[%d]], = [%d]\n", 
//			__FUNCTION__, j, rfc2833_dtmf_pt_local[pSessRank[j]], j, rfc2833_dtmf_pt_remote[pSessRank[j]]);
		if (rfc2833_dtmf_pt_local[pSessRank[j]]!=0 && rfc2833_dtmf_pt_remote[pSessRank[j]]!=0)//Not DTMF Inband
		{	
			if (RtpOpen[pSessRank[j]] == 0) 		// Check RTP session opened or not 
			{
				continue;
			}

			if ((rtpConfigOK[pSessRank[j]]) == 0)	// Check RTP session enable and hold
			{
				continue;
			}

			if (isTranMode(pSessRank[j]) == 0)	// Check RTP session state if TX available
			{
				continue;
			}
				
			RtpTx_transmitEvent_ISR(chid, pSessRank[j], event);
#ifdef CONFIG_ARCH_LUNA_SLAVE				
			enable_irq(BSP_TC5_IRQ);
#endif
		}
		else
		{
			// if local, or remote is Inband DTMF, then not send 2833 pkts.
			PRINT_MSG("[%s] not RFC2833 mode\n", __FUNCTION__);
			continue;
			//Must not set send_2833_flag_ch = 0 here, send_2833_flag_ch is for this channel DTMF removal
		}
	} /* SessNum */

	return 1;
}

/*
	when detect the telephone event change, send out the related rfc 2833 trunk event
*/
int RFC2833TrunkEventOut(int chid, int status)
{
	if ( (chid > 1) || (chid < 0) )
	{
		printk("[%s] chid out of range\n", __FUNCTION__);
		return 0;
	}

	if ( !eventEnable[chid] )
		return 0;
		
	PRINT_MSG("[%s] chid = [%d], status = [%d]\n", __FUNCTION__, chid, status);
	switch (status)
	{
		case PHONE_ONHOOK :
			RFC2833GenerateTrunkEvent( chid, EVENT_OnHook );
			break;
		case PHONE_OFFHOOK :
			RFC2833GenerateTrunkEvent( chid, EVENT_OffHook );
			break;
		default:
			break;
	}
	return 1;
}

/*
	Receive RFC2833 event to do actions
*/
int RFC2833TrunkEventIn(int chid, int event)
{
	if ( (chid > 1) || (chid < 0) )
	{
		printk("[%s] chid out of range\n", __FUNCTION__);
		return 0;
	}
		
	if ( !eventEnable[chid] )
		return 0;
		
	PRINT_MSG("[%s] chid = [%d], event = [%d]\n", __FUNCTION__, chid, event);
	switch (event)
	{
		case EVENT_RingOn :
			FXS_Ring(chid, 1);
			break;
		case EVENT_ReverseBattery :
			SLIC_Set_FXS_Line_State(chid, 6); // 6 is for VP_LINE_TALK_POLREV
			break;
		case EVENT_RingOff :
			FXS_Ring( chid, 0);
			SLIC_Set_FXS_Line_State(chid, 2); // 2 is for VP_LINE_STANDBY
			break;
		default:
			break;
	}
	return 1;
}

#if defined(CONFIG_DEFAULTS_KERNEL_2_6) || defined(CONFIG_DEFAULTS_KERNEL_3_4) || defined(CONFIG_DEFAULTS_KERNEL_3_10)
static irqreturn_t timer2_interrupt(int irq, void *dev_instance)
#else
static void timer2_interrupt(int32 irq, void *dev_instance, struct pt_regs *regs)
#endif
{

	trunkevent_send();

	REG32(BSP_TC5INT) |= (BSP_TCIP);
#if defined(CONFIG_DEFAULTS_KERNEL_2_6) || defined(CONFIG_DEFAULTS_KERNEL_3_4) || defined(CONFIG_DEFAULTS_KERNEL_3_10)
	return IRQ_HANDLED;
#endif
}

static int __init timer2init(void)
{
	struct timer2_priv *timer2dev;
	char *timer2name = "timer2";
	char strbuf[NAME_SIZE];
	int result;
	
	sprintf(strbuf, timer2name, NAME_SIZE);
	memcpy(timer2ctrl_devices[0].name, strbuf, NAME_SIZE);
	
	timer2dev = &timer2ctrl_devices[0];
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	result = request_irq(BSP_TC5_IRQ, timer2_interrupt, IRQF_DISABLED | IRQF_PERCPU | IRQF_TIMER,
			timer2dev->name, timer2dev);
#else
	result = request_irq(BSP_TC5_IRQ, timer2_interrupt, SA_INTERRUPT,
			timer2dev->name, timer2dev);
#endif

	if(result)
	{
		printk("Can't request IRQ for timer2\n");
		return 0;
	}
	printk("Request IRQ for timer2 OK!.\n");
#ifdef CONFIG_ARCH_LUNA_SLAVE	
	disable_irq(BSP_TC5_IRQ);
#endif	
	return 1;
}
static void __exit timer2exit( void )
{
	disable_irq(BSP_TC5_IRQ);
}

voip_initcall_bus( timer2init );
voip_exitcall( timer2exit );
