//#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/delay.h>  	// udelay
#include "rtk_voip.h"
//#include "Daa_api.h"
#include "snd_proslic_daa.h"
#include "snd_proslic_type.h"
#include "spi.h"
#include "voip_timer.h"


//extern void writeDAAReg(rtl_spi_dev_t *pDev, int chid, unsigned int address, unsigned int data);
//extern unsigned char readDAAReg(int chid, unsigned int address);
//extern void cyg_thread_delay(int delay);

//---static function prototype-----------------------------//
//static void Resistor_ADC_calibration(int chid);
//static void device_revision(int chid);
//static void tran_rec_full_scale(vdaaChanType *daas, unsigned char scale);
static unsigned char system_line_connect(vdaaChanType *daas);
//static void ring_validation_DAA(vdaaChanType *daas, unsigned char reg22, unsigned char reg23, unsigned char reg24);
//---------------------------------------------------------//

/*********** Select the Si3018 or Si3019 Country Specific register 16,26,30 and 31 *******/
#if 0
	#define _Si3018_CountryReg_
#else
	#define _Si3019_CountryReg_
#endif
/*****************************************************************/

/***************  PULSE DIALING ************************/
//For pulse dialing used, the value of FOH bit of reg31 must be set to mini vlaue.
//The resistor calibration (RCALD bit of reg25) and auto-calibration (CALD bit of reg17)
//must be disabled.
#define pulse_dialing 1 //0: pulse_dialing ,1: tone(DTMF)_dialing
/*********************************************/

/********************************************************************************/
//for caller ID pass through.It must be set in on-hook transmission state on slic.
#ifdef FXO_CALLER_ID_DET
#define CALLER_ID 1	//0: disable caller ID. 1: enable caller ID
#else
#define CALLER_ID 0
#endif
/********************************************************************************/

/******only for Si3019 used**********/
#define _Si3019_used_ 0
/************************************/

static void Resistor_ADC_calibration(vdaaChanType *daas)
{
#if 0
	const int chid = daas ->channel;
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	unsigned int reg17,i; 
	
	//disable resistor calibration
	//writeDAAReg(25,	readDAAReg(25)|0xa0);
	while((readDAAReg(pdev, chid, 25)&0x80)); //auto resistor calibration.
	//reg17 = readDAAReg(17) | 0x20;//disable auto ADC_calibration.CALD
	//writeDAAReg(17,reg17);
	reg17 = readDAAReg(pdev, chid, 17) | 0x40;//enable manual ADC_calibration.MCAL	
	writeDAAReg(pdev, chid, 17,reg17);
	//The duration of the calibration needs 256ms, then toggles 1 to 0.
	for (i=0;i<23000000;i++);
	reg17 = readDAAReg(pdev, chid, 17) & 0xbf;//No ADC_calibration.MCAL
	writeDAAReg(pdev, chid, 17,reg17);
#else
	Vdaa_ADCCal(daas, 0);
#endif
	return;
	
}


static unsigned char system_line_connect(vdaaChanType *daas)
{
#if 0
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	
	if ((readDAAReg(pdev, chid, 12)>>6))
		return 1; //DAA system-side and line-side connect ok;
	else
		return 0; //DAA system-side and line-side connect fail;
#else
	if (Vdaa_ReadFDTStatus(daas))
		return 1; //DAA system-side and line-side connect ok;
	else
		return 0; //DAA system-side and line-side connect fail;
#endif
}


int daa_line_check(vdaaChanType *daas)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	char line_voltage;
	short loop_c;
	const int daa_chid = daas ->channel;
	
	if (!(system_line_connect(daas))) 
	{
		PRINT_R("daa system-side and line-side not connect\n");
		//daa_init_all(SLIC_CH_NUM);
		return 0xff;
	}
	
	/* Daa must going off-hook, and then we can judge line is connected or not. */
#if 0
	off_hook_daa(daas);
#else
	Vdaa_SetHookStatus(daas, VDAA_OFFHOOK);
#endif
	udelay(5000);//spec: delay > 250us
	
	Vdaa_ReadLinefeedStatus(daas, &line_voltage, &loop_c);
	
	if ((!loop_c) && (ReadReg(daas, daas->channel, INTL_CTRL4)&0x02)) 
	{
#if 0
		on_hook_daa(daas);
#else
		Vdaa_SetHookStatus(daas, VDAA_ONHOOK);
#endif
		PRINT_MSG("phone line not connect\n");
		return 1;	
	}
#if 0
	on_hook_daa(daas);
#else
	Vdaa_SetHookStatus(daas, VDAA_ONHOOK);
#endif
	udelay(5000);
	
	/* line busy ~ 12V(0x0c) , otherwise >= 48V(0x30) */
	//printk("--->%dV\n", readDAAReg(29));
#if 0
	line_voltage = daa_get_line_voltage(daas);
#else
	Vdaa_ReadLinefeedStatus(daas, &line_voltage, &loop_c);
#endif
	PRINT_MSG("DAA line voltage = %d, ch=%d\n", line_voltage, daas->channel);

	if ( ((line_voltage >= 0) && (line_voltage < 26)) ||
 		((line_voltage < 0) && (line_voltage > -26)) )  /* 0x1a ~ 26V */
	{
		PRINT_MSG("phone line busy\n");
		return 2; 
	}

	return 0;
}


unsigned char daa_polarity_reversal_det(vdaaChanType *daas, daa_det_t *daa_det)
{
	unsigned char ret;
	//static timetick_t timeOut[VOIP_CH_NUM];
	//static int need_to_clear[VOIP_CH_NUM];
	
	if ( 0 == (ReadReg(daas, daas ->channel, CTRL2)&0x80))
		WriteReg(daas, daas ->channel, CTRL2, ReadReg(daas, daas ->channel, CTRL2)|0x80);	//enable INT pin

	if ( 0 == (ReadReg(daas, daas ->channel, INTE_MSK)&0x01))
		WriteReg(daas, daas ->channel, INTE_MSK, ReadReg(daas, daas ->channel, INTE_MSK)|0x01);	//enable Polarity Reversal Detect Mask

	ret = ReadReg(daas, daas ->channel, INTE_SRC)&0x01;	// 0: has not changed states.
										// 1: has transitioned from 0 to 1, or from 1 to 0, indicating the polarity of  TIP and RING is switched.
	
	if (ret && (daa_det ->polrev_need_to_clear==0))
	{
		//daa_det ->polrev_timeOut = jiffies + HZ*(1000/1000);
		daa_det ->polrev_timeOut = timetick + 1000;	// 1000ms;
		daa_det ->polrev_need_to_clear = 1;

		return 1;
	}

    if ( ret && (timetick_after(timetick, daa_det ->polrev_timeOut)) ) // wait 0.5s, then clean 
	{
		WriteReg(daas, daas ->channel, INTE_SRC,  ReadReg(daas, daas ->channel, INTE_SRC)&0xFE);	//clear the interrup
		daa_det ->polrev_need_to_clear = 0;

	}

	return 0;
}

void daa_disable_polarity_reversal_det(vdaaChanType *daas)
{
#if 0
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	writeDAAReg(pdev, chid, 3, readDAAReg(pdev, chid, 3)&0xFE);	//Disable Polarity Reversal Detect Mask
#else
	unsigned char reg;
	reg = ReadReg(daas, daas ->channel, INTE_MSK);
	Vdaa_SetInterruptMask(daas, reg&0xFE); // Disable POLM interrupt mask
#endif
}

void daa_enable_polarity_reversal_det(vdaaChanType *daas)
{
#if 0
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	writeDAAReg(pdev, chid, 3, readDAAReg(pdev, chid, 3)|0x01);	//enable Polarity Reversal Detect Mask
#else
	unsigned char reg;
	reg = ReadReg(daas, daas ->channel, INTE_MSK);
	Vdaa_SetInterruptMask(daas, reg|POLM); // Enable POLM interrupt mask
#endif
}

unsigned int daa_get_polarity(vdaaChanType *daas)
{
	return (unsigned int )((ReadReg(daas, daas ->channel, LINE_VOLT_STAT) & 0x80) >>7);
}

/* Only when daa off-hook, it can detect drou out */
unsigned char daa_drop_out_det(vdaaChanType *daas, daa_det_t *daa_det)
{
	unsigned char ret;
	//static timetick_t timeOut[VOIP_CH_NUM];
	//static int need_to_clear[VOIP_CH_NUM];

	if ( 0 == (ReadReg(daas, daas ->channel, CTRL2)&0x80))
		WriteReg(daas, daas ->channel, CTRL2, ReadReg(daas, daas ->channel, CTRL2)|0x80);	//enable INT pin

	if ( 0 == (ReadReg(daas, daas ->channel, INTE_MSK)&0x08))
		WriteReg(daas, daas ->channel, INTE_MSK, ReadReg(daas, daas ->channel, INTE_MSK)|0x08);	//enable Drop Out Detect Mask

	ret = ReadReg(daas, daas ->channel, INTE_SRC)&0x08;	// 0: has not changed states.
						// 1: battery drop out
	
	if (ret && (daa_det ->dropout_need_to_clear==0))
	{
		//daa_det ->dropout_timeOut = jiffies + HZ*(500/1000);
		daa_det ->dropout_timeOut = timetick + 500;	// 500ms
		daa_det ->dropout_need_to_clear = 1;

		return 1;
	}

	if ( ret && (timetick_after(timetick, daa_det ->dropout_timeOut)) ) // wait 0.5s, then clean 
	{
		WriteReg(daas, daas ->channel, INTE_SRC,  ReadReg(daas, daas ->channel, INTE_SRC)&0xF7);	//clear the interrup
		daa_det ->dropout_need_to_clear = 0;

	}

	return 0;
}

void daa_disable_drop_out_det(vdaaChanType *daas)
{
#if 0
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	writeDAAReg(pdev, chid, 3, readDAAReg(pdev, chid, 3)&0xF7);	//Disable Drop Out Detect Mask
#else
	unsigned char reg;
	reg = ReadReg(daas, daas ->channel, INTE_MSK);
	Vdaa_SetInterruptMask(daas, reg&0xF7); // Disable DODM interrupt mask
#endif
}

void daa_enable_drop_out_det(vdaaChanType *daas)
{
#if 0
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	writeDAAReg(pdev, chid, 3, readDAAReg(pdev, chid, 3)|0x08);	//enable Drop Out Detect Mask
#else
	unsigned char reg;
	reg = ReadReg(daas, daas ->channel, INTE_MSK);
	Vdaa_SetInterruptMask(daas, reg|DODM); // Enable DODM interrupt mask
#endif
}

/* caller id detect need this. */
unsigned int daa_line_stat(vdaaChanType *daas)
{
#if 0
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	unsigned int temp;
	/* for SI3050 register 5 */
	/* bit0; 0= ON-HOOK, 1= OFF-HOOK */
	/* bit2; 0= ringing off will delayed, 1= ringing occurring */
	/* bit5; 0= no positive ring occuring, 1= positive ring occuring (realtime)*/
	/* bit6; 0= no negative ring occuring, 1= negative ring occuring (realtime)*/
	/* other bit don't care */
	temp = readDAAReg(pdev, chid, 5);
#else
	unsigned int temp;
	temp = ReadReg(daas, daas ->channel, DAA_CTRL1);
#endif

	return temp;
}

#if 0	// obsolete api
static void device_revision(int chid)
{
	unsigned int reg11,reg13;
	unsigned char temp;

	reg11 = readDAAReg(pdev, chid, 11);	
	reg13 = readDAAReg(pdev, chid, 13);
	
	//Verify the revision of System-side
	temp = reg11 & 0x0f;
	switch (temp)
	{
		case 0:
			printk("System-side Si3050A ");
			break;	
		case 2:
			printk("System-side Si3050B ");
			break;
		case 3:
			printk("System-side Si3050C ");
			break;
		case 4:
			printk("System-side Si3050D ");
			break;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x
		case 8:
			printk("System-side Si32177/8 ");
			break;
#endif
		default:
			printk("System-side ? ");
	}
	
	//Verify line-side device
	temp = (reg11 & 0xf0)>>4;
	switch (temp)
	{
		case 1:
			printk(",Line-side Si3018");
			break;
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x
		case 3:
			printk(",Line-side Si3019");
			break;
#else
		case 3:
			printk(",Line-side Si32919");
			break;
		case 4:
			printk(",Line-side Si32911");
			break;
#endif
		default:
			printk(",Line-side ?");
			break;
	}
		
	//Verify the line-side device revision
	temp = (reg13>>2) & 0x0f;
	
	switch (temp)
	{
		case 1:
			printk("A\n");
			break;	
		case 2:
			printk("B\n");
			break;
		case 3:
			printk("C\n");
			break;
		case 4:
			printk("D\n");
			break;
		case 5:
			printk("E\n");
			break;
		case 6:
			printk("F\n");
			break;
		default:
			printk("unknow version");
	}
	printk("\n");
	return;
}

static void tran_rec_full_scale(vdaaChanType *daas, unsigned char scale)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	
	if (scale == 1)
	{
		writeDAAReg(pdev, chid, 31,(readDAAReg(pdev, chid, 31) | 0x80)); //Transmit/receive full scale x1
		writeDAAReg(pdev, chid, 26,(readDAAReg(pdev, chid, 26) | 0xc0)); //set DCV[1:0]=3,MINI[1:0]=0
	}
	else if (scale == 2)
	{
		writeDAAReg(pdev, chid, 30,(readDAAReg(pdev, chid, 30) | 0x10)); //Transmit/receive full scale x2
		writeDAAReg(pdev, chid, 26,(readDAAReg(pdev, chid, 26) | 0xc0)); //set DCV[1:0]=3,MINI[1:0]=0    
	}
	else
		printk("Error full_scale ratio.\n");
	
	return;
	
}

void country_specific_termination(vdaaChanType *daas, unsigned char ohs,unsigned char ohs2,unsigned char rz,
		unsigned char rt,unsigned char ilim,unsigned char dcv,unsigned char mini,unsigned char acim)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	unsigned char reg16,reg26,reg30,reg31;	
	
	reg16 = readDAAReg(pdev, chid, 16) | (ohs<<6) | (rz<<1) | rt;
	writeDAAReg(pdev, chid, 16,reg16); //setup OHS,RZ and RT of register 16.
	
	reg26 = readDAAReg(pdev, chid, 26) | (ilim<<1) | (dcv<<6) | (mini<<4);	
	writeDAAReg(pdev, chid, 26,reg26); //setup LILM,DCV,and MINI of register 26.
	
	reg30 = readDAAReg(pdev, chid, 30) | acim;
	writeDAAReg(pdev, chid, 30,reg30); //setup ACIM of register 30.
	
	reg31 = readDAAReg(pdev, chid, 31) | (ohs2<<3);
	writeDAAReg(pdev, chid, 31,reg31); //setup OHS2 of register 31.
	BOOT_MSG("The setting of country specific registers is ok.\n");
	return;		
}

void DAA_hybrid_coeff(vdaaChanType *daas, unsigned char acim,unsigned char coeff1,unsigned char coeff2,unsigned char coeff3,
		unsigned char coeff4,unsigned char coeff5,unsigned char coeff6,unsigned char coeff7,unsigned char coeff8)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	unsigned char reg30;
	
	reg30 = readDAAReg(pdev, chid, 30) | acim;
	writeDAAReg(pdev, chid, 30,reg30); //setup ACIM of register 30.
	
	writeDAAReg(pdev, chid, 45,coeff1);//setup Hybrid coeff1 of register 45.	
	writeDAAReg(pdev, chid, 46,coeff2);//setup Hybrid coeff2 of register 46.
	writeDAAReg(pdev, chid, 47,coeff3);//setup Hybrid coeff3 of register 47.
	writeDAAReg(pdev, chid, 48,coeff4);//setup Hybrid coeff4 of register 48.
	writeDAAReg(pdev, chid, 49,coeff5);//setup Hybrid coeff5 of register 49.
	writeDAAReg(pdev, chid, 50,coeff6);//setup Hybrid coeff6 of register 50.
	writeDAAReg(pdev, chid, 51,coeff7);//setup Hybrid coeff7 of register 51.
	writeDAAReg(pdev, chid, 52,coeff8);//setup Hybrid coeff8 of register 52.
	return;			
} 

void DAA_Tx_Gain_ctrl(vdaaChanType *daas, unsigned char tga2,unsigned char txg2,unsigned char tga3,unsigned char txg3) 
{
	unsigned char reg38,reg40;
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	
	reg38 = (tga2<<4) | txg2; //tga2 = 0 means gaining up, tga2 = 1 means attenuating
#if 0
	writeDAAReg(pdev, chid, 38,reg38);			   //txg2 = 0 means 0dB, txg2 = 1 means 1dB,...txg2 = 15 means 15dB
#else
	WriteReg(daas, daas ->channel, 38, reg38);
#endif
	
	reg40 = (tga3<<4) | txg3; //tga3 = 0 means gaining up, tga3 = 1 means attenuating
#if 0
	writeDAAReg(pdev, chid, 40,reg40);			   //txg3 = 0 means 0dB, txg3 = 1 means 0.1dB,...txg3 = 15 means 0.15dB
#else
	WriteReg(daas, daas ->channel, 40, reg40);
#endif
	
	return;
}

void DAA_Rx_Gain_ctrl(vdaaChanType *daas, unsigned char rga2,unsigned char rxg2,unsigned char rga3,unsigned char rxg3) 
{
	unsigned char reg39,reg41;
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	
	reg39 = (rga2<<4) | rxg2; //rga2 = 0 means gaining up, rga2 = 1 means attenuating
#if 0
	writeDAAReg(pdev, chid, 39,reg39);			   //rxg2 = 0 means 0dB, rxg2 = 1 means 1dB,...rxg2 = 15 means 15dB
#else
	WriteReg(daas, daas ->channel, 39, reg39);
#endif
	
	reg41 = (rga3<<4) | rxg3; //rga3 = 0 means gaining up, rga3 = 1 means attenuating
#if 0
	writeDAAReg(pdev, chid, 41,reg41);			   //rxg3 = 0 means 0dB, rxg3 = 1 means 0.1dB,...rxg3 = 15 means 0.15dB
#else
	WriteReg(daas, daas ->channel, 41, reg41);
#endif
	
	return;
}

void DAA_Tx_Gain_Web(vdaaChanType *daas, unsigned char gain) 
{
	static unsigned char gain_before = 10;
	
	if ( gain >= 7) 
		DAA_Tx_Gain_ctrl(daas, 0,(gain-7),0,0);
	else 
		DAA_Tx_Gain_ctrl(daas, 1,(7-gain),0,0);	
	//printk("preset gain 3");
	gain_before = gain;
	return;	
}	

void DAA_Rx_Gain_Web(vdaaChanType *daas, unsigned char gain) 
{
	static unsigned char gain_before = 10;
	
	if ( gain >= 7) 
		DAA_Rx_Gain_ctrl(daas, 0,(gain-7),0,0);
	else 
		DAA_Rx_Gain_ctrl(daas, 1,(7-gain),0,0);		
	gain_before = gain;
	return;	
}

unsigned short daa_get_line_voltage(vdaaChanType *daas)
{
#if 0
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int daa_chid = daas ->channel;
	unsigned short reg29, reg_29=0;
	
	reg29 = readDAAReg(pdev, daa_chid, 29); // 2's complement
		
	if ((reg29>>7) == 1)// polarity reversal of the TIP/RING
	{
		int i;
		for (i=0; i<8; i++)
		{
			reg_29 += (((reg29>>i)&0x01)^1)<<i;
		}
		reg_29 += 1;
	}
	else
		reg_29 = reg29;

	//PRINT_MSG("DAA line voltage = %d, ch=%d\n", reg_29, daa_chid);
	
	return reg_29;
#else
	char volt;
	short loop_c;
	int ret;
	ret = Vdaa_ReadLinefeedStatus(daas, &volt, &loop_c);

	if (ret != RC_NONE )
	{
		PRINT_R("Vdaa_ReadLinefeedStatus return %d, ch%d\n", ret, daas ->channel);
	}

	PRINT_MSG("DAA line voltage = %d, ch=%d\n", volt, daas ->channel);
	PRINT_MSG("DAA line loop_current = %d, ch=%d\n", loop_c, daas ->channel);
	
	if (volt < 0)
		volt = -volt;

	return volt;
#endif

}

unsigned char going_off_hook(int chid) 
{
	unsigned int i;
	unsigned char reg31;
	
	off_hook_daa(chid);
	
	while(!(readDAAReg(pdev, chid, 5)&0x01)); //Ensure line-side is going off-hook.

#if 0	
	//The following is transients to settle line-side device.Default delay 128ms.
	//0:512ms 1:128ms 2:64ms 3:8ms.
	if (1 == DAA_Get_Dial_Mode(chid))		// Pulse dial
		reg31 = 1;
	else if (0 == DAA_Get_Dial_Mode(chid))	// DTMF dial
		reg31 = 3;

	reg31 = readDAAReg(pdev, chid, 31) | (reg31<<5);
	writeDAAReg(chid, 31,reg31);
	
	for (i=0;i<11500000;i++); //delay 128ms
#endif
	
	if (0 == DAA_Get_Dial_Mode(chid))	// DTMF dial
		Resistor_ADC_calibration(chid); //calibration
	
	return 1; //going off-hook ok
	 	
}

/* caller id detect need this. */
void on_hook_daa(vdaaChanType *daas) 
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	writeDAAReg(pdev, chid, 5,readDAAReg(pdev, chid, 5)&0xfe); //set OH bit of register 5 to on-hook.
}

/* caller id detect need this. */
void off_hook_daa(vdaaChanType *daas)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	writeDAAReg(pdev, chid, 5,readDAAReg(pdev, chid, 5)|0x01); //set OH bit of register 5 to off-hook.
}

unsigned char daa_hook_state(vdaaChanType *daas)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int daa_ch = daas ->channel;
	return readDAAReg(pdev, daa_ch, 5)&0x01; // 0: on-hook, 1: off-hook
}

void caller_ID_pass(vdaaChanType *daas)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	writeDAAReg(pdev, chid, 5,readDAAReg(pdev, chid, 5)|0x08); //set ONHM bit of register 5 to enable caller ID detection.
	return;
}

//using ring_detection_daa() by while() polling. 
unsigned char ring_detection_DAA(vdaaChanType *daas)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int daa_chid = daas ->channel;
	unsigned short reg5_val = 0;
	
	#if _Si3019_used_
		if (readDAAReg(pdev, daa_chid, 29) == 0) {
			printk("phone line not connect\n");
			return 2;//Line not connect.	
	}
	#endif 
	
	reg5_val = readDAAReg(pdev, daa_chid, 5);
	
	if (reg5_val == 0xff)
	{
		PRINT_R("ring_detection_DAA error occur!\n");
		//daa_init_all(SLIC_CH_NUM);
	}
		
	#if 1
	#if CALLER_ID
	if (reg5_val&0x04)
	//if ((reg5_val&0x04) || (reg5_val&0x20) || (reg5_val&0x40))//ring signal postive //ring signal negative 
		return 1; //Ringing now.
	else
		return 0; //Not ring state.
	#else
	if (reg5_val&0x04)
	//if ((reg5_val&0x04) || (reg5_val&0x20) || (reg5_val&0x40))//ring signal postive //ring signal negative 
		return 1; //Ringing now.
	else
		return 0; //Not ring state.
	#endif
	#else
	unsigned char temp=1;
	while (temp) {
		if ((reg5_val&0x04)) {
			printk("Ringing now\n");
			temp = 0;
		}	
		else	{
			//printk("NO Ringing \n");
			temp = 1;
		}
	}
	return 1; //Ringing now.
	#endif
}

static void ring_validation_DAA(vdaaChanType *daas, unsigned char reg22,unsigned char reg23,unsigned char reg24)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	
	writeDAAReg(pdev, chid, 22,reg22); //setup RDLY and RMX of register 22.
	if (readDAAReg(pdev, chid, 22) != reg22)
		PRINT_R("The reg22 of DAA isn't correct.\n");
	writeDAAReg(pdev, chid, 23,reg23); //setup RDLY,RTO and RCC of register 23.
	if (readDAAReg(pdev, chid, 23) != reg23)
		PRINT_R("The reg23 of DAA isn't correct.\n");
	writeDAAReg(pdev, chid, 24,reg24); //setup RAS of register 24.
	if (readDAAReg(pdev, chid, 24) != reg24)
		PRINT_R("The reg24 of DAA isn't correct.\n");
	writeDAAReg(pdev, chid, 18,INIT_R18); //set ring detector full-wave rectifier enable.RFWE of register 18.
	if (readDAAReg(pdev, chid, 18) != INIT_R18)
		PRINT_R("The reg18 of DAA isn't correct.\n");
	writeDAAReg(pdev, chid, 24,(readDAAReg(pdev, chid, 24)|0x80));	//enable RNGV of register 24.
	BOOT_MSG("The validation of incoming ring is ok.\n");
	return;
}

void daa_timeslot(vdaaChanType *daas, int timeslot, int bitOfTimeSlot)
{
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;

	if (timeslot < (256/bitOfTimeSlot))// timeslot 0~31 if bitOfTimeSlot=8
	{
		writeDAAReg(pdev, chid, 34, 1 + 8*timeslot);//Tx low byte
		writeDAAReg(pdev, chid, 36, 1 + 8*timeslot);//Rx low byte	
		writeDAAReg(pdev, chid, 35, 0);		//Tx high byte	
		writeDAAReg(pdev, chid, 37, 0);		//Rx high byte	
	}
	else
	{
		printk("Error! Not Suppport TimeSlot Setting for DAA. You must modify the driver to support it!\n");
	}

}

void daa_init_all(int pcm_channel, int pcm_mode)
{
	extern int slic_ch_num;
	//Call init_spi() to reset DAA before call this.
	printk("----- DAA Port%d Init -----\n", pcm_channel-slic_ch_num); 
	writeDAAReg(pcm_channel, 3,INIT_R3);//disable all interrupt.Default should be disable. 
	writeDAAReg(pcm_channel, 7,INIT_R7);//setup sample rate 8kHz.
	writeDAAReg(pcm_channel, 17,(readDAAReg(pdev, pcm_channel, 17)|0x20));//disable auto ADC_calibration.CALD
	
	if (1 == DAA_Get_Dial_Mode(pcm_channel))//Pulse dial
		writeDAAReg(pcm_channel, 25,(readDAAReg(pdev, pcm_channel, 25)|0x20));//disable resistor calibration.RCALD

	writeDAAReg(pcm_channel, 25,(readDAAReg(pdev, pcm_channel, 25)&0xdf));//setup resistor_calibration. reg25.It may not be written.
	
	if (pcm_mode == 2)
		writeDAAReg(pcm_channel, 33, 0x28);//enable PCM,use u-law.
	else if (pcm_mode == 1)
		writeDAAReg(pcm_channel, 33, 0x20);//enable PCM,use A-law.
	else if (pcm_mode == 0)
		writeDAAReg(pcm_channel, 33, 0x38);//enable PCM,use 16bit linear

	daa_timeslot(pcm_channel, pcm_channel, 8);
	writeDAAReg(pcm_channel, 42,INIT_R42);//disable GCI interface.
	
	country_specific_termination(pcm_channel, 0,0,0,0,0,3,0,0);//setup DC and AC Termination.Default Taiwan.
	
	ring_validation_DAA(pcm_channel, INIT_R22,INIT_R23,INIT_R24);//setup ring detection threshold.
	
	//DAA_hybrid_coeff(0,255,247,255,4,253,2,0,0);//set hybrid coefficient from reg45 to reg52.
	
	writeDAAReg(pcm_channel, 6,0x00);//powerup line-side device.
	writeDAAReg(pcm_channel, 31,(readDAAReg(pdev, pcm_channel, 31)|2));//200hz high pass 
	#if CALLER_ID
	caller_ID_pass(pcm_channel);
	#endif
	mdelay(1); // add delay for revision check
	device_revision(pcm_channel);//display DAA revision.
	daa_enable_polarity_reversal_det(pcm_channel);
	//daa_enable_drop_out_det(pcm_channel);
	return; 
		
}

//dial_number count
unsigned char pulse_dialing_number(unsigned int chid)
{
	unsigned char count,dial_number=0;
	unsigned int break_for_number=0,hold_time_long=0,hold_time_long_one=0;	
	#if 1
	while ((readDirectReg(chid, 68)&0x01));//when bit0 of reg68 equals to 0,break while loop.
	#else
	for (;;) {
		if (!(readDirectReg(chid, 68)&0x01)) {//when bit0 of reg68 equals to 0,break for loop.
				break;
			} else	
				hold_time_long++;
		if (hold_time_long >= 1500)
			return 11;
	}
	#endif	
	for (count=0;count<10;count++) {
		
		for (;;) {
			if (!(readDirectReg(chid, 68)&0x01)) {//when bit0 of reg68 equals to 0,break for loop.
				break;
			} else	
				break_for_number++;
			//printk("break_for_number=%d\n",break_for_number);
			if (break_for_number >= 220)
				break;	
		}
		//printk("break_for_number=%d\n",break_for_number);
		if (break_for_number >= 220)
			break;
		//printk("first\n");
		#if 0
		while (!(readDirectReg(chid, 68)&0x01));//when bit0 of reg68 equals to 1,break while loop.	
		#else
		for (;;) {
			if ((readDirectReg(chid, 68)&0x01)) {//when bit0 of reg68 equals to 1,break for loop.
				break;
			} else	
				hold_time_long_one++;
			//printk("break_for_number=%d\n",break_for_number);
			if (hold_time_long_one >= 550)
				return 11;	
		}
		#endif
		dial_number++;
		
		//printk("second\n");
		//break_for_number=0;	
	}	
		 
	return dial_number;
}

#endif



