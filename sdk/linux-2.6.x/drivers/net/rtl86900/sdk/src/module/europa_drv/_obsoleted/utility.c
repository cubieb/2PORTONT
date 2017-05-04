#ifdef CONFIG_EUROPA_MODULE
#include <linux/module.h>  /* Needed by all modules */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <asm-generic/div64.h>
#include <module/europa/europa.h>
#include <module/europa/common.h>
#include <module/europa/utility.h>
#include <module/europa/europa.h>

#include <module/europa/rtl8290_api.h>


#define INTERNAL_DDMI_A0_REG_START_ADDRESS 0
#define INTERNAL_DDMI_A2_REG_START_ADDRESS 256

#define TEMPERATURE_NUM                                  14
#define EUROPA_R_REGISTER_MAXIMUM              0x1000000

#if 0
void busy_loop(uint16 loopCnt)
{
    uint16 i;
    for(i=0;i<loopCnt;i++);
    
}

void drv_setAsicRegBit(uint16 reg, uint16 rBit, uint16 value) 
{
    uint16 regData;
    regData = getReg(reg); 

	if(value)
		regData = regData | (1 << rBit);
	else
		regData = regData & (~(1 << rBit));

	setReg(reg,regData);
}

void drv_getAsicRegBit(uint16 reg, uint16 rBit, uint16 *pValue)
{
    uint16 regData;
    regData = getReg(reg); 

    *pValue = (regData & (0x1 << rBit)) >> rBit;
}

void drv_setAsicRegBits(uint16 reg, uint16 rBits, uint16 value)
{
	uint16 regData;
	uint16 bitsShift;
	uint16 valueShifted;

	bitsShift = 0;
	while(!(rBits & (1 << bitsShift)))
	{
		bitsShift++;
	}

	valueShifted = value << bitsShift;

	regData = getReg(reg);

	regData = regData & (~rBits);
	regData = regData | (valueShifted & rBits);

	setReg(reg,regData);
}


void drv_getAsicRegBits(uint16 reg, uint16 rBits, uint16 *pValue) 
{
	uint16 regData;
	uint16 bitsShift;

	bitsShift = 0;
	while(!(rBits & (1 << bitsShift)))
	{
		bitsShift++;
	}

	regData= getReg(reg);

	*pValue = (regData & rBits) >> bitsShift;
}
#endif

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

#if 0
uint8 check_all_one(uint32 reg, uint8 start, uint8 length)
{
    uint8 i;
    uint32 regData;
    
    regData = getReg(reg);
    for(i = start; i< start + length; i++)
    {
        if(!((regData >> i) & 1))
            return FALSE;
    }
    return TRUE;

    
}
#endif




uint32 volt_trans_func_1(uint16 internal) 
{
    uint16 regData1, regData2, regData3;
    uint32 volt_ddmi, cx, c0, ldo_volt;
    uint32 temp32;
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

    return cx;
}



uint32 impd_trans_func_1(uint16 internal, uint32 power_ddmi) 
{
    uint16 regData1, regData2, regData3;
    uint32 cx, c0, c1, threshold;

#if 0	
    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 24);
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 25);
    power_ddmi = ((regData1 & 0xff) << 8) | (regData2 & 0xff);
    cx = volt_ddmi;
#endif

    if(internal)
    {  
        drv_getAsicRegBits(EUROPA_REG_R5_ADDR, EUROPA_REG_R5_REG_R5_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R6_ADDR, EUROPA_REG_R6_REG_R6_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R7_ADDR, 0xFF, &regData3);
        c1 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);

        drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
        c0 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);

        //V=3.3/(c1-c0) x (Cx - 0x800000) + 1.65
        //Cx = (IMPD *(R/Gain/1000)-1650)*(C1-C0)/3300 + 0x800000 
        //power_ddmi = (IMPD *(R/Gain/1000)
        cx = (power_ddmi - 1650)*(c1 - c0)/3300 +0x800000;
        //use 24bits's first 8 bits
        threshold = ((cx & 0xFF0000) >> 16) & 0xff;		
    }
    else//external mode
    {
        threshold = (cx >> 8) & 0xff;
    }
    //set cx to REG_VOL_TH(use MSB 8 bits)
    drv_setAsicRegBits(EUROPA_REG_IMPD_TH_ADDR, EUROPA_REG_IMPD_TH_IMPD_THRESHOLD_MASK, threshold);

    return cx;
}

void set_impd_threshold(uint16 internal, uint32 Resister, uint32 Gain) 
{
    uint16 regData1, regData2;
    uint32 power_ddmi;

    regData1 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 24);
    regData2 = getReg(INTERNAL_DDMI_A2_REG_START_ADDRESS + 25);
    power_ddmi = ((regData1 & 0xff) << 8) | (regData2 & 0xff);
    if(internal)
    { 
        power_ddmi = power_ddmi *Resister/Gain;
    }
	
    impd_trans_func_1(internal, power_ddmi);
}

#if 0 //temp remove
void apc_performanceVlueDCL_set(void)
{
    
}

void apc_performanceVlueSCL_set(void)
{   //RLE0482_interface_0315_TXAPC_only.xls    Europa_2012_0315_Richard_TXPAC_Startup.pdf

    //drv_setAsicRegBits(EUROPA_REG_W61_ADDR, EUROPA_REG_W61_APCDIG_CODE_DACPAVG_MASK, tmp); //APCDIG_CODE_DACPAVG[7:0], related to MPD component ??????????
    drv_setAsicRegBit(EUROPA_REG_W57_ADDR, EUROPA_REG_W57_APCDIG_CODEOFFSET_DACPAVG_OFFSET, 0);//APCDIG_CODEOFFSET_DACPAVG
    //APCDIG_DAC_BIASHALF
    drv_setAsicRegBit(EUROPA_REG_W68_ADDR, EUROPA_REG_W68_APCDIG_SING_HFCUT_OFFSET, 1);//APCDIG_SING_HFCUT
    //APCDIG_SING_GAIN[2:0]  wait experiment result
    drv_setAsicRegBits(EUROPA_REG_W69_ADDR, EUROPA_REG_W69_APCDIG_SING_ANAGAIN_MASK, 0);//APCDIG_SING_ANAGAIN
    //APCDIG_SING_SETTLE wait experiment result
    drv_setAsicRegBits(EUROPA_REG_W69_ADDR, EUROPA_REG_W69_APCDIG_SING_LPF_BW_MASK, 0xf);//APCDIG_SING_LPF_BW
    
}


void tx_performanceVlue_set(void)
{
    uint32 tmp;
    /* set performace related register, REG_W50~REG_W52 use RLE0482_interface_0315_TXAPC_only.xls preferred column */
    //REG_W50
    drv_setAsicRegBits(EUROPA_REG_W50_ADDR,EUROPA_REG_W50_TX_CSSEL_MASK,0x1);
    drv_getAsicRegBits(EUROPA_REG_R31_ADDR, EUROPA_REG_R31_TX_RPD_RCODE_OUT_MASK, &tmp);
    { //W50[7:6] and W50[5:4] set to same value as R31[1:0]
        drv_setAsicRegBits(EUROPA_REG_W50_ADDR,EUROPA_REG_W50_TX_RLSEL_MASK,tmp);
        drv_setAsicRegBits(EUROPA_REG_W50_ADDR,EUROPA_REG_W50_TX_RSSEL_MASK,tmp);
    }

    //REG_W51
#if 1
        drv_setAsicRegBits(EUROPA_REG_W51_ADDR,0xff,0x0c); //W51 = 0x0c
#else
        drv_setAsicRegBits(EUROPA_REG_W51_ADDR,0x3,0); //W51[1:0] = 0
        drv_setAsicRegBit(EUROPA_REG_W51_ADDR,2,1); //W51[2] = 1
        drv_setAsicRegBit(EUROPA_REG_W51_ADDR,3,1); //W51[3] = 1
        drv_setAsicRegBit(EUROPA_REG_W51_ADDR,4,0); //W51[4] = 0
        drv_setAsicRegBit(EUROPA_REG_W51_ADDR,5,0); //W51[5] = 0
        drv_setAsicRegBit(EUROPA_REG_W51_ADDR,6,0); //W51[6] = 0, need to test before
        drv_setAsicRegBit(EUROPA_REG_W51_ADDR,7,0); //W51[7] = 0
#endif

    //REG_W52
#if 1
        drv_setAsicRegBits(EUROPA_REG_W52_ADDR,0x018,0xa0); //W52 = 0xa0
#else
        drv_setAsicRegBit(EUROPA_REG_W52_ADDR,0,0); //W52[0] = 0
        drv_setAsicRegBit(EUROPA_REG_W52_ADDR,1,0); //W52[1] = 0
        drv_setAsicRegBit(EUROPA_REG_W52_ADDR,2,0); //W52[2] = 0
        drv_setAsicRegBits(EUROPA_REG_W52_ADDR,0x018,0); //W52[4:3] = 2'b00
        drv_setAsicRegBit(EUROPA_REG_W52_ADDR,5,1); //W52[5] = 1
        drv_setAsicRegBit(EUROPA_REG_W52_ADDR,6,0); //W52[6] = 0, need to test before
        drv_setAsicRegBit(EUROPA_REG_W52_ADDR,7,1); //W52[7] = 1
#endif

}
#endif


 
void bubble_sort(uint16 iarr[], uint8 num) 
{
    uint8  i, j, k;
    uint16 temp;
	
    //printk(KERN_EMERG "\nUnsorted Data:");
    //for (k = 0; k < num; k++) {
    //  printk(KERN_EMERG " %d ", iarr[k]);
   //}
 
    for (i = 1; i < num; i++) 
    {
        for (j = 0; j < num - 1; j++)
        {
            if (iarr[j] > iarr[j + 1]) 
            {
                temp = iarr[j];
                iarr[j] = iarr[j + 1];
                iarr[j + 1] = temp;
            }
        }
    }

    //printk(KERN_EMERG"\nAfter pass %d : ", i);
    //for (k = 0; k < num; k++) 
    //{
    //     printk(KERN_EMERG" %d ", iarr[k]);
    //}
	
}

uint16 get_temperature(void)
{
    uint16 array[TEMPERATURE_NUM], tmp1, tmp2, tmp_sys;
    uint8 i;

    for (i=0;i<TEMPERATURE_NUM;i++)
    {
        drv_getAsicRegBits(EUROPA_REG_R2_ADDR, EUROPA_REG_R2_REG_R2_MASK, &tmp1);
        drv_getAsicRegBit(EUROPA_REG_R3_ADDR, 7, &tmp2);
        tmp_sys = tmp1 << 1 | tmp2; //0k
        if (tmp_sys<233)
        {
            tmp_sys = 233;
        }
        if (tmp_sys>383)
        {
            tmp_sys = 383;
        }
	  array[i] = tmp_sys;
    }

    bubble_sort(array, TEMPERATURE_NUM);

    tmp_sys = 0;
    for(i=0;i<(TEMPERATURE_NUM-4);i++)
    {
        tmp_sys +=array[i+2];
    }

    return (tmp_sys/(TEMPERATURE_NUM-4));	
}

#if 0
uint32 volt_trans_tx_power(uint32 ref_volt)
{
    uint16 regData1, regData2, regData3;
    uint32 mpd, volt_mpd, c0;

    drv_getAsicRegBits(EUROPA_REG_R11_ADDR, EUROPA_REG_R11_REG_R11_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R12_ADDR, EUROPA_REG_R12_REG_R12_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R13_ADDR, EUROPA_REG_R13_REG_R13_MASK, &regData3);
    mpd = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    //Read GND		
    drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
    c0 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);

     volt_mpd =  (1000000*(12*(mpd - ref_volt))/(10*(ref_volt - c0)) + 1200000); //unit is uV	
		
    return volt_mpd;
}

uint32 volt_trans_rx_power(uint32 ref_volt)
{
    uint16 regData1, regData2, regData3;
    uint32 rssi, volt_rssi, c0;

    //Set data hold
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x6a);
    //Read Vrssi
    drv_getAsicRegBits(EUROPA_REG_R17_ADDR, EUROPA_REG_R17_REG_R17_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R18_ADDR, EUROPA_REG_R18_REG_R18_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R19_ADDR, 0xFF, &regData3);
    rssi = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);	
    //Read GND		
    drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
    c0 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);

    //Set data un-hold
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x62);

    volt_rssi =  (1000000*(12*(rssi - ref_volt))/(10*(ref_volt - c0)) + 1200000); //unit is uV	
		
    return volt_rssi;
}
#endif
void get_rssi_voltage(europa_volt_para_t *rssi_para)
{
    uint16 regData1, regData2, regData3;
    uint32 temp32;
    uint64 temp64;
    //uint32 rssi_code, vdd_code, gnd_code, ldo_code, half_vdd_code;
    //uint64_t volt_half_vdd, volt_ldo, volt_rssi;//unit is uV

    //Read RSSI code
    temp32 = 0;
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0xC2);
    udelay(DELAY_TIME);
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0xCA);
    udelay(DELAY_TIME);	
    drv_getAsicRegBits(EUROPA_REG_R14_ADDR, EUROPA_REG_R14_REG_R14_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R15_ADDR, EUROPA_REG_R15_REG_R15_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R16_ADDR, EUROPA_REG_R16_REG_R16_MASK, &regData3);
    temp32 =((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    rssi_para->rssi_code = temp32;

    //Read GND code
    temp32 = 0;
    drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
    temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    rssi_para->gnd_code = temp32;

    //Read VDD code	
    temp32 = 0;
    drv_getAsicRegBits(EUROPA_REG_R5_ADDR, EUROPA_REG_R5_REG_R5_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R6_ADDR, EUROPA_REG_R6_REG_R6_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R7_ADDR, 0xFF, &regData3);
    temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    rssi_para->vdd_code = temp32;

    //Read LDO code
    temp32 = 0;
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x62);
    udelay(DELAY_TIME);
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x6A);
    udelay(DELAY_TIME);	
    drv_getAsicRegBits(EUROPA_REG_R14_ADDR, EUROPA_REG_R14_REG_R14_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R15_ADDR, EUROPA_REG_R15_REG_R15_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R16_ADDR, EUROPA_REG_R16_REG_R16_MASK, &regData3);
    temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    rssi_para->ldo_code = temp32;

    //Read VDD/2 code
    temp32 = 0;
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x82);
    udelay(DELAY_TIME);
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x8A);
    udelay(DELAY_TIME);	
    drv_getAsicRegBits(EUROPA_REG_R14_ADDR, EUROPA_REG_R14_REG_R14_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R15_ADDR, EUROPA_REG_R15_REG_R15_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R16_ADDR, EUROPA_REG_R16_REG_R16_MASK, &regData3);
    temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);	
    rssi_para->half_vdd_code = temp32;	

    /*Disable Hold action*/
    drv_setAsicRegBit(EUROPA_REG_W18_ADDR, 3, 0);

    temp64 = rssi_para->half_vdd_code - rssi_para->gnd_code;
    temp64 = temp64*3300000;
    temp32 = rssi_para->vdd_code - rssi_para->gnd_code;
    do_div(temp64, temp32);	
    rssi_para->volt_half_vdd = (uint32)temp64;
    //printk("volt_half_vdd = 0x%llx,  0x%x", temp64, rssi_para->volt_half_vdd);	

    temp64 = rssi_para->ldo_code - rssi_para->gnd_code;
    temp64 = temp64*3300000;
    temp32 = rssi_para->vdd_code - rssi_para->gnd_code;
    do_div(temp64, temp32);	
    rssi_para->volt_ldo =(uint32)temp64;
    //printk("volt_ldo = 0x%llx,  0x%x", temp64, rssi_para->volt_ldo);	
	
    if (rssi_para->rssi_code>=rssi_para->half_vdd_code)
    { 
        temp64 = abs(rssi_para->volt_ldo-rssi_para->volt_half_vdd);
        temp64 = temp64*(rssi_para->rssi_code - rssi_para->half_vdd_code);
        temp32 = abs(rssi_para->ldo_code - rssi_para->half_vdd_code);
        do_div(temp64, temp32);		
        rssi_para->volt_rssi = rssi_para->volt_half_vdd + (uint32)temp64;
    }
    else
    {
        temp64 = abs(rssi_para->volt_ldo-rssi_para->volt_half_vdd);
        temp64 = temp64*(rssi_para->half_vdd_code - rssi_para->rssi_code);
        temp32 = abs(rssi_para->ldo_code - rssi_para->half_vdd_code);		
        do_div(temp64, temp32);	
        rssi_para->volt_rssi = rssi_para->volt_half_vdd - (uint32)temp64;			
    }
    //printk("volt_rssi = 0x%llx,  0x%x", temp64, rssi_para->volt_rssi);	

}


uint64 cal_rssi_voltage(uint16 count)
{
    uint16 loop;  
    uint32 rssi_avg;
    europa_volt_para_t rssi_para;

    rssi_avg = 0;
    for(loop=0;loop<count; loop++)
    {
        memset(&rssi_para, 0, sizeof(europa_volt_para_t));
        get_rssi_voltage(&rssi_para);
        //printk("loop= %d, rssi_code = %d, gnd_code = %d, vdd_code = %d, ldo_code = %d, half_vdd_code = %d volt_half_vdd = %llu, volt_ldo = %llu, volt_rssi = %llu\n", loop, rssi_para.rssi_code, rssi_para.gnd_code, rssi_para.vdd_code, rssi_para.ldo_code, rssi_para.half_vdd_code, rssi_para.volt_half_vdd, rssi_para.volt_ldo, rssi_para.volt_rssi);
        printk("loop= %d, rssi_code = %d, gnd_code = %d, vdd_code = %d, ldo_code = %d, half_vdd_code = %d,  volt_half_vdd = %d, volt_ldo = %d, volt_rssi = %d\n", loop, rssi_para.rssi_code, rssi_para.gnd_code, rssi_para.vdd_code, rssi_para.ldo_code, rssi_para.half_vdd_code, rssi_para.volt_half_vdd, rssi_para.volt_ldo, rssi_para.volt_rssi);	
        rssi_avg+=rssi_para.volt_rssi;
    }
	
    printk(" rssi_total = 0x%x\n",rssi_avg);

    rssi_avg = rssi_avg/count;	
	
    //do_div(rssi_avg, count);

    return rssi_avg;
}

#endif


