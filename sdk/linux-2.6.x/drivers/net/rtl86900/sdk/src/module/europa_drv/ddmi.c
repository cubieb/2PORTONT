#ifdef CONFIG_EUROPA_MODULE
//#include <stdio.h>
#include <linux/kernel.h>
#include <module/europa/europa.h>
#include <module/europa/common.h>
#include <module/europa/utility.h>
#include <module/europa/init.h>
#include <linux/delay.h>
#include <asm-generic/div64.h>

#include <module/europa/rtl8290_api.h>


#define INTERNAL_DDMI_A0_REG_START_ADDRESS 0
#define INTERNAL_DDMI_A2_REG_START_ADDRESS 256

#define EUROPA_DDMI_MAXIMUM              0x10000
#define EUROPA_R_REGISTER_MAXIMUM              0x1000000

#define PARAM_RX_A_SHIFT 30
#define PARAM_RX_B_SHIFT 13
#define PARAM_RX_C_SHIFT 12

// shift bits for TX power coefficients
#define PARAM_TX_A_SHIFT 8
#define PARAM_TX_B_SHIFT 5

extern uint8 internal;
//extern uint32 global_mpd0;
extern uint16 loopcnt;
//extern uint32 voltage_v0;

extern rtl8290_param_t europa_param;

#if 0 //Window Mechanism
uint32 window[16];
#endif

void get_internal(void) 
{
    uint16 status;
    status = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 92);
    if(status & 0x020)
        internal = 1;
    else
        internal = 0;
}

#if 0
void set_temperature_threshold(void) 
{
    uint16  regData1, regData2;
    int16  temperature;
	
    /* 1. REG_TEMP_TH   -- Get from DDMI A2 byte 0/1  alarm high */
    /*DDMI temperature is 128~-128 C, uint is 1/256*/
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 0);
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 1);
    temperature = (regData1 << 8) | regData2;
    //temperature = (temperature/128) - 273;
    temperature = temperature/128;	//To fit EUROPA_REG_R2_ADDR value
    drv_setAsicRegBits(EUROPA_REG_TEMP_TH_ADDR, EUROPA_REG_TEMP_TH_TEMP_THRESHOLD_MASK, (uint16)temperature);
}
#endif

int32 ddmi_temp_threshold_set(void) 
{
    uint16  regData1, regData2;
    int16  temperature;
	
    /* 1. REG_TEMP_TH   -- Get from DDMI A2 byte 0/1  alarm high */
    /*DDMI temperature is 128~-128 C, uint is 1/256*/
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 0);
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 1);
    temperature = (regData1 << 8) | regData2;
    //temperature = (temperature/128) - 273;
    temperature = temperature/128;	//To fit EUROPA_REG_R2_ADDR value
    drv_setAsicRegBits(EUROPA_REG_TEMP_TH_ADDR, EUROPA_REG_TEMP_TH_TEMP_THRESHOLD_MASK, (uint16)temperature);
    return SUCCESS;
}

int32 ddmi_volt_threshold_set(uint16 internal) 
{
    uint16 regData1, regData2, regData3;
    uint32 volt_ddmi, cx, c0, ldo_volt;
    //uint32 temp32;
    uint64 temp64;	
    
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 8);
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 9);
    volt_ddmi = ((regData1 & 0xff) << 8) | (regData2 & 0xff); //unit is 100uV
    cx = volt_ddmi; 
    
    if(internal)
    {
        drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x62);
        udelay(DELAY_TIME);
        drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x6A);
        udelay(DELAY_TIME);	
        drv_getAsicRegBits(EUROPA_REG_R14_ADDR, EUROPA_REG_R14_REG_R14_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R15_ADDR, EUROPA_REG_R15_REG_R15_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R16_ADDR, EUROPA_REG_R16_REG_R16_MASK, &regData3);
        ldo_volt =((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
	
        drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
        c0 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
		
        /*Disable Hold action*/
        drv_setAsicRegBit(EUROPA_REG_W18_ADDR, 3, 0);
		
        //cx = (volt_ddmi - 12000)*(ldo_volt - c0)/12000 + ldo_volt;		
        temp64 = ldo_volt - c0;
        temp64 = temp64*(volt_ddmi - 12000);
        do_div(temp64, 12000);
        cx = (uint32)temp64 + ldo_volt;

        //printk("volt_trans_func_1: DDMI_VOL = 0x%x, LDO  = 0x%x, GND = 0x%x, CX = 0x%x", volt_ddmi,ldo_volt, c0, cx);		
        if (EUROPA_R_REGISTER_MAXIMUM>cx)
        {	
            //use 24bits's first 8 bits
            cx = cx & 0xFF0000;
            cx = (cx >> 16) & 0xff;		
        }
        else
        {
            cx = 0xF0;
            printk("volt_trans_func_1: CX overflow!!! Set EUROPA_REG_VOL_TH = 0x%x", cx);	
        }
    }
    else//external mode
    {
        cx = (cx >> 8) & 0xff;
    }
    //set cx to REG_VOL_TH(use MSB 8 bits)
    drv_setAsicRegBits(EUROPA_REG_VOL_TH_ADDR, EUROPA_REG_VOL_TH_VOL_THRESHOLD_MASK, cx);

    return SUCCESS;
}

void ddmi_reg_move(uint16 i2cAddr, uint16 addrStart, uint16 addrEnd)
{
    uint16 addr;    
    uint8 ddmiVal;    
    uint16 testStart,testEnd;    
    
    
    if(i2cAddr == 2)
    {
        testStart = addrStart + 256;
        testEnd = addrEnd + 256;
    }
    else
    {
        testStart = addrStart;
        testEnd = addrEnd;
    }
    
    for(addr=testStart; addr <= testEnd ; addr++)
    {
        
        /*read from internal reg*/
        ddmiVal = getReg(addr); 
        
        /*write to external EEPROM*/    
        setReg(0xC000+addr+REG_DDMI_EXTERNAL_EEPROM_BASE_ADDR, ddmiVal); 
        busy_loop(0x800);
    }
}

void compare_imod_threshold(void) 
{
    uint16  regData1, regData2, imod_th;
    uint32  tx_mod;
	
    drv_getAsicRegBits(EUROPA_REG_R32_ADDR, EUROPA_REG_R32_APCDIG_RO_MOD_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R34_ADDR, EUROPA_REG_R34_APCDIG_RO_MOD_1_MASK, &regData2);

    tx_mod = (regData1<<4)|(regData2&0xF);

    drv_getAsicRegBits(EUROPA_REG_IMOD_TH_ADDR, EUROPA_REG_IMOD_TH_IMOD_THRESHOLD_MASK, &imod_th);

    if ((tx_mod>>4)>imod_th)
    {
        drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS2_ADDR, EUROPA_REG_SW_FAULT_STATUS2_OVER_MOD_FAULT_OFFSET, 1);			
    }	

}

int16 update_ddmi_temperature(uint16 internal) 
{
    uint16 temp1, temp2;
    int16 temp_sys;

    if (internal)
    {
        //temp1 = get_temperature();
        rtl8290_temperature_get(&temp1);    

	// offset value
	temp1 = temp1 - europa_param.temp_offset;

        temp_sys = (temp1 - 273)<<8; //ddmi unit is 1/256
    }
    else
    {
        drv_getAsicRegBits(EUROPA_REG_R2_ADDR, EUROPA_REG_R2_REG_R2_MASK, &temp1);
        drv_getAsicRegBits(EUROPA_REG_R3_ADDR, EUROPA_REG_R3_REG_R3_MASK, &temp2);    
        temp_sys = ((temp1 & 0xff) << 8) | (temp2);
    }
	
    return temp_sys;
}

void ddmi_diagnostic_monitor_temperature(int16 temp_sys) 
{
    uint16 status, status2;
    int16 temp_ddmi;

    //set temperature to DDMI
    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 96, (temp_sys >> 8)&0xff);
    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 97, temp_sys & 0xFF);

    status = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 0); //temperature high alarm
    status2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 1);
    temp_ddmi = (status << 8) + status2;
    if(temp_sys > temp_ddmi)
    { //A2[112] bit7
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 112, 7, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, EUROPA_REG_SW_FAULT_STATUS1_OVER_HTEMP_FAULT_OFFSET, 1);		
    }

    status = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 2); //temperature low alarm
    status2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 3);
    temp_ddmi = (status << 8) + status2;
    if(temp_sys < temp_ddmi)
    { //A2[112] bit6
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 112, 6, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, EUROPA_REG_SW_FAULT_STATUS1_OVER_LTEMP_FAULT_OFFSET, 1);			
    }

    status= getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 4); //temperature high warning
    status2= getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 5);
    temp_ddmi = (status << 8) + status2;
    if(temp_sys > temp_ddmi)
    { //A2[116] bit7
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 116, 7, 1);
    }
	
    status = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 6); //temperature low warning
    status2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 7);
    temp_ddmi = (status << 8) + status2;	
    if(temp_sys < temp_ddmi)
    { //A2[116] bit6
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 116, 6, 1);
    }


}

void ddmi_temperature(void) 
{
    int16 temp_sys;

    temp_sys = update_ddmi_temperature(internal);
	
    ddmi_diagnostic_monitor_temperature(temp_sys);
}

//Voltage translation function 2
uint32 update_ddmi_vcc(uint16 internal) 
{
    uint16 regData1, regData2, regData3;
    uint32 sdadc_vdd, volt_sys;
    uint8 mode;

    mode = SDADC_TX_VDD;
    if (SDADC_TX_VDD == mode)
        drv_setAsicRegBits(EUROPA_REG_W37_ADDR, EUROPA_REG_W37_REG_W37_MASK, 0x1e);
    else
        drv_setAsicRegBits(EUROPA_REG_W37_ADDR, EUROPA_REG_W37_REG_W37_MASK, 0x1f);

    //voltage translate function 2
    if(internal)
    {        
        rtl8290_vdd_get(&sdadc_vdd);

        //Change to 100uV unit for DDMI A2	
        volt_sys =  sdadc_vdd/100;		

       if (EUROPA_DDMI_MAXIMUM<=volt_sys)
        {	
            printk("update_ddmi_vcc: volt_sys overflow!!! volt_sys = 0x%x", volt_sys);	
        }
        ///printk("VCC = 0x%x\n", volt_sys);	
  
    }
    else //external
    {   
        drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x62);
        udelay(DELAY_TIME);
        drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x6A);
        udelay(DELAY_TIME);	
    
        drv_getAsicRegBits(EUROPA_REG_R8_ADDR, EUROPA_REG_R8_REG_R8_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R9_ADDR, EUROPA_REG_R9_REG_R9_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R10_ADDR, 0xFF, &regData3);
        sdadc_vdd = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);    
        volt_sys = sdadc_vdd>>8;
        /*Disable Hold action*/
        drv_setAsicRegBit(EUROPA_REG_W18_ADDR, 3, 0);
    }
	
    return volt_sys;
}

#if 0 //Old Version
//Voltage translation function 2
uint32 update_ddmi_vcc(uint16 internal) 
{
    uint16 regData1, regData2, regData3;
    uint32 c0, c1, cx, volt_sys;

    //voltage translate function 2
    drv_getAsicRegBits(EUROPA_REG_R8_ADDR, EUROPA_REG_R8_REG_R8_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R9_ADDR, EUROPA_REG_R9_REG_R9_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R10_ADDR, 0xFF, &regData3);
    cx = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    if(internal)
    {
        drv_getAsicRegBits(EUROPA_REG_R5_ADDR, EUROPA_REG_R5_REG_R5_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R6_ADDR, EUROPA_REG_R6_REG_R6_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R7_ADDR, 0xFF, &regData3);
        c1 =((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
        
        drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
        c0 =((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
        volt_sys =  (33 * (cx - 0x800000))/(10 * (c1 - c0))*10000 +16500;		
    }
    else //external
    {
        volt_sys = cx>>8;
    }
    return volt_sys;
}
#endif

void ddmi_diagnostic_monitor_vcc(uint32 volt_sys) 
{
    uint16 regData1, regData2;
    uint32 volt_ddmi;

    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 98, (volt_sys >> 8)& 0xff);
    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 99, (volt_sys) & 0xff);
    
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 8); //voltage high alarm
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 9);
	
    volt_ddmi = (regData1 << 8) + regData2;
    if(volt_sys > volt_ddmi)
    { //A2[112] bit5
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 112, 5, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, EUROPA_REG_SW_FAULT_STATUS1_OVER_HVOL_FAULT_OFFSET, 1);				
    }

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 10); //voltage low alarm
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 11);
    volt_ddmi = (regData1 << 8) + regData2;
    if(volt_sys < volt_ddmi)
    { //A2[112] bit4
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 112, 4, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, EUROPA_REG_SW_FAULT_STATUS1_OVER_LVOL_FAULT_OFFSET, 1);				
		
    }

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 12); //voltage high warning
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 13);
    volt_ddmi = (regData1 << 8) + regData2;
    if(volt_sys > volt_ddmi)
    { //A2[116] bit5
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 116, 5, 1);
    }
    
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 14); //voltage low warning
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 15);
    volt_ddmi = (regData1 << 8) + regData2;
    if(volt_sys < volt_ddmi)
    { //A2[116] bit4
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 116, 4, 1);
    }
}

void ddmi_vcc(void) 
{
    //uint16 regData1, regData2, regData3;
    uint32 volt;

    volt = update_ddmi_vcc(internal);
    ddmi_diagnostic_monitor_vcc(volt);
}

uint32 update_ddmi_tx_bias(uint16 internal) 
{
    uint16 regData1, regData2;
    uint32 bias_sys;

    if(internal)
    {
        rtl8290_tx_bias_get(&bias_sys);
    }
    else
    {
    //tx bias translation function
    drv_getAsicRegBits(EUROPA_REG_R33_ADDR, EUROPA_REG_R33_APCDIG_RO_BIAS_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R34_ADDR, EUROPA_REG_R34_APCDIG_RO_BIAS_1_MASK, &regData2);
    bias_sys = (regData1 << 4) | regData2;
    }
	
    return bias_sys;
}

void ddmi_diagnostic_monitor_tx_bias(uint32 bias_sys) 
{
    uint16 regData1, regData2;
    uint32 bias_ddmi;

    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 100, (bias_sys >> 8)& 0xff);
    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 101, (bias_sys) & 0xff);	
	
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 16); //tx bias high alarm
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 17);
	
    bias_ddmi = (regData1 << 8) + regData2;
    if(bias_sys > bias_ddmi)
    { //A2[112] bit3
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 112, 3, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, EUROPA_REG_SW_FAULT_STATUS1_OVER_HBIAS_FAULT_OFFSET, 1);						
    }

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 18); //tx bias low alarm
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 19);
    bias_ddmi = (regData1 << 8) + regData2;
    if(bias_sys < bias_ddmi)
    { //A2[112] bit2
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 112, 2, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, EUROPA_REG_SW_FAULT_STATUS1_OVER_LBIAS_FAULT_OFFSET, 1);			
    }    

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 20); //tx bias high warning
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 21);
    bias_ddmi = (regData1 << 8) + regData2;
    if(bias_sys > bias_ddmi)
    { //A2[116] bit3
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 116, 3, 1);
    }

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 22); //tx bias low warning
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 23);
    bias_ddmi = (regData1 << 8) + regData2;
    if(bias_sys < bias_ddmi)
    { //A2[116] bit2
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 116, 2, 1);
    }    

}

void ddmi_tx_bias(void) 
{
    uint32 bias_sys;

    //tx bias translation function
    bias_sys = update_ddmi_tx_bias(internal);
    ddmi_diagnostic_monitor_tx_bias(bias_sys);
}

//hmchung: The function to be define
void update_ddmi_tx_power(void ) 
{
    uint32 i_mpd, mpd1;
    int64 sum, sum_mpd;
    int32 i;
	
    sum = 0;
    sum_mpd = 0;
    for (i=0;i<10;i++)
    {
	europa_debug_level_set(0);
        rtl8290_tx_power_get(europa_param.mpd0, &mpd1, &i_mpd);
	europa_debug_level_set(1);
        sum += i_mpd;
        sum_mpd += mpd1;
    }
    //i_mpd = sum/10;
    do_div(sum, 10); 
    i_mpd = (uint32)sum;

    //mpd1 = sum_mpd/10;
    do_div(sum_mpd, 10); 
    mpd1 = (uint32)sum_mpd;
 
    //temp_f = (double)i_mpd*(double)ldd_param.tx_a/(1<<TX_A_SHIFT) + 
    //              (double)(ldd_param.tx_b)/(1<<TX_B_SHIFT);
    //tx_power = (0.0<temp_f) ? (uint32)(temp_f*1000) : 0; /* nW */

    sum =  ((i_mpd *europa_param.tx_a*10)>>PARAM_TX_A_SHIFT)+((europa_param.tx_b*10)>>PARAM_TX_B_SHIFT);;
    if (sum>0)
    {
        europa_param.tx_power = (uint32)sum;
    }
    else
    {
        europa_param.tx_power = 0;
    }
    ///printk("TX power: MPD0 = %d, MPD1 = %d, I_MPD = %d uA, TX Power = %d (0.1uW)\n", europa_param.mpd0, mpd1, i_mpd, europa_param.tx_power);

    return;
}

void ddmi_diagnostic_monitor_tx_power(uint32 power_sys) 
{
    uint16 regData1, regData2;
    uint32 power_ddmi;
	
    //Is here OK??? Before translation or After translation?
    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 102, (power_sys >>8)& 0xff);
    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 103, (power_sys) & 0xff);
	
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 24); //tx power high alarm
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 25);
    power_ddmi = (regData1 << 8) + regData2;
    if(power_sys > power_ddmi)
    { //A2[112] bit1
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 112, 0x1, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, EUROPA_REG_SW_FAULT_STATUS1_OVER_HTXPWR_FAULT_OFFSET, 1);			
    }

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 26); //tx power low alarm
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 27);
    power_ddmi = (regData1 << 8) + regData2;
    if(power_sys < power_ddmi)
    { //A2[112] bit0
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 112, 0, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, EUROPA_REG_SW_FAULT_STATUS1_OVER_LTXPWR_FAULT_OFFSET, 1);			
    }

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 28); //tx power high warning
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 29);
    power_ddmi = (regData1 << 8) + regData2;
    if(power_sys > power_ddmi)
    { //A2[116] bit1
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 116, 0x1, 1);
    }
    
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 30); //tx power low warning
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 31);
    power_ddmi = (regData1 << 8) + regData2;
    if(power_sys < power_ddmi)
    { //A2[116] bit0
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 116, 0, 1);
    }

}



void ddmi_tx_power(void) 
{
    update_ddmi_tx_power();
    ddmi_diagnostic_monitor_tx_power(europa_param.tx_power);

}

void update_ddmi_rx_power(void) 
{
    uint16 i;
    uint32 rssi_v0, v_rssi, i_rssi;
    int64 sum_i, temp64;
    int32 temp_a, temp_b, temp_c, sum;	
#if 0  //Window Mechanism
    uint32 i_rssi_avg;
#endif

    rssi_v0 = europa_param.rssi_v0;
    sum_i = 0;

    //printk("Europa RX power for %d times, RSSI V0 = %d\n", loopcnt, europa_param.rssi_v0);
    //printk("RX A = %d, RX B = %d, RX C = %d\n", europa_param.rx_a, europa_param.rx_b, europa_param.rx_c);
    
    //printk("Loop    10times    Window    \n");	

    sum_i = 0;
    for (i=0;i<10;i++)
    {
        rtl8290_rx_power_get(rssi_v0, &v_rssi, &i_rssi);
        sum_i += i_rssi;		
    }       
    do_div(sum_i, 10);	
    i_rssi  = (uint32)sum_i;		

    //temp_f = ((double)ldd_param.rx_a/(1<<RX_A_SHIFT))*((double)i_rssi*i_rssi/1000000) + \
    //                ((double)ldd_param.rx_b/(1<<RX_B_SHIFT))*((double)i_rssi/1000) + \
    //                ((double)ldd_param.rx_c/(1<<RX_C_SHIFT));
    //rx_power = (0.0<temp_f) ? (uint32)(temp_f*1000) : 0; /* nW */	    

    temp64 = i_rssi*i_rssi*europa_param.rx_a;
    do_div( temp64, 100000);	
    do_div( temp64, (1<<PARAM_RX_A_SHIFT));	
    temp_a =(int32)( temp64);

    temp64 = i_rssi*europa_param.rx_b;
    do_div( temp64, 100);	
    do_div( temp64,  (1<<PARAM_RX_B_SHIFT));	
    temp_b =(int32)( temp64);

    temp_c = (europa_param.rx_c*10)/(1<<PARAM_RX_C_SHIFT);
    /*Unit is 0.1uW = 100nW*/
    sum = temp_a+temp_b+temp_c;
    if (sum>0)
    {
        europa_param.rx_power = sum;
    }
    else
    {
        europa_param.rx_power = 0;
    }
    //printk("RX Power: I RSSI = %d  I  a = 0x%x  b = 0x%x  c = 0x%x   power = %d(0.1uW)\n", i_rssi, temp_a, temp_b, temp_c, europa_param.rx_power);    		
#if 0 //Window Mechanism
    if (europa_param.rx_windowcnt>0)
    {
        window[europa_param.rx_loopcnt] = i_rssi;   
        europa_param.rx_sum+= i_rssi;	
        sum_i = europa_param.rx_sum;
        do_div( sum_i, europa_param.rx_loopcnt+1);	
        i_rssi_avg  = (uint32) sum_i;	
        europa_param.rx_windowcnt--;		
    }
    else
    {
         europa_param.rx_sum =  europa_param.rx_sum - window[europa_param.rx_loopcnt%16] + i_rssi;
        window[europa_param.rx_loopcnt%16] = i_rssi;            
        i_rssi_avg =  europa_param.rx_sum>>4;
    }
    europa_param.rx_loopcnt++;

    // note:
    //temp_f = ((double)ldd_param.rx_a/(1<<RX_A_SHIFT))*((double)i_rssi*i_rssi/1000000) +       ((double)ldd_param.rx_b/(1<<RX_B_SHIFT))*((double)i_rssi/1000) + ((double)ldd_param.rx_c/(1<<RX_C_SHIFT));
    //rx_power = (0.0<temp_f) ? (uint32)(temp_f*1000) : 0; /* nW */	    

    temp64 = i_rssi_avg*i_rssi_avg*europa_param.rx_a;
    do_div( temp64, 100000);	
    do_div( temp64, (1<<PARAM_RX_A_SHIFT));	
    temp_a =(int32)( temp64);

    temp64 = i_rssi_avg*europa_param.rx_b;
    do_div( temp64, 100);	
    do_div( temp64,  (1<<PARAM_RX_B_SHIFT));	
    temp_b =(int32)( temp64);

    temp_c = (europa_param.rx_c*10)/(1<<PARAM_RX_C_SHIFT);
    /*Unit is 0.1uW = 100nW*/
    sum = temp_a+temp_b+temp_c;
    if (sum>0)
    {
        europa_param.rx_power = sum;
    }
    else
    {
        europa_param.rx_power = 0;
    }
    printk("RX Power: loop = %d I RSSI = %d  I RSSI Avg = %d a = 0x%x  b = 0x%x  c = 0x%x   power = %d(0.1uW)\n", europa_param.rx_loopcnt, i_rssi, i_rssi_avg, temp_a, temp_b, temp_c, europa_param.rx_power);    	
#endif

}

void ddmi_diagnostic_monitor_rx_power(uint32 power_sys) 
{
    uint16 regData1, regData2;
    uint32 power_ddmi;

    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 104, (power_sys >> 8)& 0xff);
    setReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 105, (power_sys) & 0xff);
    
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 24); //rx power high alarm
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 25);
    power_ddmi = (regData1 << 8) + regData2;
    if(power_sys > power_ddmi)
    { //A2[113] bit7
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 113, 7, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS2_ADDR, EUROPA_REG_SW_FAULT_STATUS2_OVER_HRXPWR_FAULT_OFFSET, 1);			
    }

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 26); //rx power low alarm
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 27);
    power_ddmi = (regData1 << 8) + regData2;
    if(power_sys < power_ddmi)
    { //A2[113] bit6
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 113, 6, 1);
        //drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS2_ADDR, EUROPA_REG_SW_FAULT_STATUS2_OVER_LRXPWR_FAULT_OFFSET, 1);			
    }

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 28); //rx power high warning
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 29);
    power_ddmi = (regData1 << 8) + regData2;
    if(power_sys > power_ddmi)
    { //A2[117] bit7
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 117, 7, 1);
    }
    
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 30); //rx power low warning
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 31);
    power_ddmi = (regData1 << 8) + regData2;
    if(power_sys < power_ddmi)
    { //A2[117] bit6
        drv_setAsicRegBit(INTERNAL_DDMI_A2_REG_START_ADDRESS + 117, 6, 1);
    }
}


//hmchung: The function to be define
void ddmi_rx_power(void) 
{
    update_ddmi_rx_power();	
    ddmi_diagnostic_monitor_rx_power(europa_param.rx_power);
}

void update_ddmi(void) 
{
    uint16 freeze;

    /* check A4/83 REG_CONTROL1[4]: FREEZE_DDMI_UPDATE */
    drv_getAsicRegBits(EUROPA_REG_CONTROL1_ADDR, EUROPA_REG_CONTROL1_FREEZE_DDMI_UPDATE_MASK, &freeze);

    if (freeze)
    {
        return;   /* do not update DDMI */
    }

    /*Imod threshold*/
    //compare_imod_threshold();

    /* Update DDMI temperature */
    ddmi_temperature();

    /* Update DDMI Vcc */
    ddmi_vcc();

    /* Update DDMI TX Bias */
    ddmi_tx_bias();

    /* Update DDMI RX Power */
    ddmi_rx_power();

    /* Update DDMI TX Power */
    ddmi_tx_power();
    
}

void ddmi_intr_handle(uint16 status) 
{
    //uint8 status;
    
    //status = getReg(EUROPA_REG_DDMI_INT_STATUS_ADDR); 
    /*debug only*/
    //setReg(0xC000+0x400+2, status); 

    if(status & EUROPA_REG_DDMI_INT_STATUS_DDMI_P0_INT_MASK)
        ddmi_reg_move(0,0,63);
    else if(status & EUROPA_REG_DDMI_INT_STATUS_DDMI_P1_INT_MASK)
        ddmi_reg_move(0,64,127);
    else if(status & EUROPA_REG_DDMI_INT_STATUS_DDMI_P2_INT_MASK)
        ddmi_reg_move(0,128,191);
    else if(status & EUROPA_REG_DDMI_INT_STATUS_DDMI_P3_INT_MASK)
        ddmi_reg_move(0,192,255);
    else if(status & EUROPA_REG_DDMI_INT_STATUS_DDMI_P4_INT_MASK)
        ddmi_reg_move(2,0,63);
    else if(status & EUROPA_REG_DDMI_INT_STATUS_DDMI_P5_INT_MASK)
        ddmi_reg_move(2,64,127);
    else if(status & EUROPA_REG_DDMI_INT_STATUS_DDMI_P6_INT_MASK)
        ddmi_reg_move(2,128,191);
    else if(status & EUROPA_REG_DDMI_INT_STATUS_DDMI_P7_INT_MASK)
        ddmi_reg_move(2,192,255);

}

#endif

