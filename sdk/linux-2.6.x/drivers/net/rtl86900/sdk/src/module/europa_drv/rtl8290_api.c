/*
 * Include Files
 */
#include <linux/kernel.h>  /* Needed for KERN_ALERT */
#include <module/europa/common.h>
#include <module/europa/init.h>
#include <rtk/i2c.h>
#include <rtk/gpio.h>
#include <module/europa/europa_reg_definition.h>
#include <module/europa/rtl8290_api.h>
#include <linux/delay.h>
#include <asm-generic/div64.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

extern rtl8290_param_t europa_param; 
extern uint8 flash_data[];

extern uint8 I2C_PORT;
extern uint8 INTR_PIN;
extern uint8 TXDIS_PIN;
extern uint8 TXPWR_PIN;
extern uint8 PON_MODE;


/*
 * Macro Declaration
 */
#define DELAY_TIME                                             2000
#define TEMPERATURE_NUM                                  14


void busy_loop(uint16 loopCnt)
{
    // FIXME  busy wait is not reliable on faster cpu (eg. 9602b)
    // FIXME  which is much faster than 9601b
#if 0
    uint16 i;
    for(i=0;i<loopCnt;i++);
#else
    udelay( loopCnt>>4 );   // asume original 0x50 = 80 means 5us
#endif

}

void setReg(uint16 address, uint16 value)
{
    uint16 eeprom_addr=0, i2c_addr = 0;
    uint8 devId=0;
    uint32 status;
    int32   ret;

    if(address >= 0xc000) //EEPROM WRITE
    {
        eeprom_addr = address - 0xc000;
        devId = 0x55;
        printk(KERN_EMERG "1 setReg eeprom_addr=%d  devId=%d\n",eeprom_addr, devId);
        rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_ADDR_LB_ADDR - 0x300, eeprom_addr & 0xff); //ADDR_LB
        rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_ADDR_HB_ADDR - 0x300, 0xc0+((eeprom_addr>>8) & 0xff)); //ADDR_HB
        rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_WDATA_0_ADDR - 0x300, value & 0xff); //WDATA_0
        while(1) //check EEPROM BUSY
        {
            rtk_i2c_read(I2C_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x1) == 0)
                break;
        }
       ret =  rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_CONTROL_ADDR - 0x300, 0x13); //CONTROL(Write one byte)
    }
    else if(address >= 0x6000) //iROM
    {
	/* single byte mode */
    
	uint16 irom_addr=0;

        irom_addr = address - 0x6000;

        devId = 0x55;

        printk(KERN_EMERG "setReg irom_addr=%d\n", irom_addr);
        rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_ADDR_HB_ADDR - 0x300, 0x80+((irom_addr>>8) & 0xff)); //ADDR_HB

        rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_ADDR_LB_ADDR - 0x300, irom_addr & 0xff); //ADDR_LB

        rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_WDATA_0_ADDR - 0x300, value & 0xff); //WDATA_0
        while(1) //check EEPROM BUSY
        {
            rtk_i2c_read(I2C_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x2) == 0)
                break;
        }
       ret =  rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_CONTROL_ADDR - 0x300, 0x13); //CONTROL(Write one byte)

        while(1) //check EEPROM BUSY
        {
            rtk_i2c_read(I2C_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x1) == 0)
                break;
        }
    }
    else
    {
        if(address < 0x100) //0xa0
        {
            devId = 0x50;
            i2c_addr = address;
        }
        else if((address >= 0x100) && (address < 0x200)) //0xa1
        {
            devId = 0x51;
            i2c_addr = address - 0x100;
        }
        else if((address >= 0x200) && (address < 0x300)) //0xa4
        {
            devId = 0x54;
            i2c_addr = address - 0x200;
        }
        else if(address >= 0x300) //0xa5
        {
            devId = 0x55;
            i2c_addr = address - 0x300;
        }

        ret = rtk_i2c_write(I2C_PORT, devId, i2c_addr, value & 0xff);

    }

    if (ret !=0)
    {
       printk(KERN_EMERG "setReg(0x%x, 0x%x) FAULT: 0x%x!!!!!\n", address, value, ret);
    }
}

uint16 getReg(uint16 address)
{
    uint16 eeprom_addr=0, i2c_addr = 0;
    uint16 devId=0;
    uint32 status, data;
    int32 ret;

    if(address >= 0xc000) //EEPROM READ
    {
        eeprom_addr = address - 0xc000;
        devId = 0x55;
        printk(KERN_EMERG "1 getReg eeprom_addr=%d  devId=%d\n",eeprom_addr, devId);
        ret =  rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_ADDR_LB_ADDR - 0x300, eeprom_addr & 0xff); //ADDR_LB
        ret =  rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_ADDR_HB_ADDR - 0x300, 0xc0+((eeprom_addr>>8) & 0xff)); //ADDR_HB
        ret = rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_CONTROL_ADDR - 0x300, 0x11); //CONTROL(Read one byte)
        while(1) //check EEPROM BUSY
        {
            ret = rtk_i2c_read(I2C_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x1) == 0)
                break;
        }
        busy_loop(0xffff);
        ret = rtk_i2c_read(I2C_PORT, devId, EUROPA_REG_INDACC_RDATA_0_ADDR - 0x300, &data); //Get data
    }
    else if(address >= 0x6000) //iROM
    {
	/* single byte mode */
    
	uint16 irom_addr=0;

        irom_addr = address - 0x6000;

        devId = 0x55;

        printk(KERN_EMERG "getReg irom_addr=%d\n", irom_addr);
        rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_ADDR_HB_ADDR - 0x300, 0x80+((irom_addr>>8) & 0xff)); //ADDR_HB

        rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_ADDR_LB_ADDR - 0x300, irom_addr & 0xff); //ADDR_LB

        while(1) //check EEPROM BUSY
        {
            rtk_i2c_read(I2C_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x2) == 0)
                break;
        }

        ret = rtk_i2c_write(I2C_PORT, devId, EUROPA_REG_INDACC_CONTROL_ADDR - 0x300, 0x11); //CONTROL(Read one byte)

        while(1) //check EEPROM BUSY
        {
            rtk_i2c_read(I2C_PORT, devId, EUROPA_REG_INDACC_STATUS_ADDR - 0x300, &status); //STATUS
            if((status & 0x2) == 0)
                break;
        }

        ret = rtk_i2c_read(I2C_PORT, devId, EUROPA_REG_INDACC_RDATA_0_ADDR - 0x300, &data); //Get data
    }
    else
    {
        if(address < 0x100) //0xa0
        {
            devId = 0x50;
            i2c_addr = address;
        }
        else if((address >= 0x100) && (address < 0x200)) //0xa1
        {
            devId = 0x51;
            i2c_addr = address - 0x100;
        }
        else if((address >= 0x200) && (address < 0x300)) //0xa4
        {
            devId = 0x54;
            i2c_addr = address - 0x200;
        }
        else if(address >= 0x300) //0xa5
        {
            devId = 0x55;
            i2c_addr = address - 0x300;
        }
        ret = rtk_i2c_read(I2C_PORT, devId, i2c_addr, &data); //Get data
    }

    if (ret !=0)
    {
       printk(KERN_EMERG "getReg(%x) FAULT: 0x%x!!!!!\n", address, ret);
    }
    return data & 0xff;
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

void _bubble_sort(uint16 iarr[], uint8 num)
{
    uint8  i, j;
    //uint8 k;
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

   // printk("\nAfter pass %d : ", i);
    //for (k = 0; k < num; k++)
    //{
         //printk(" %d ", iarr[k]);
    //}

}


/*  cheeck is tx in disabled 
 *  TX_DIS source could be TX_DIS pin, Soft TX_DIS or by software */
static int32 _rtl8290_is_tx_disabled(void)
{
    uint16 tx_dis_status, tx_dis_polarity;
    uint16 tx_dis_ctl;
    uint16 ddmi_tx_dis;
    uint16 ddmi_tx_dis_hw_enable, ddmi_tx_dis_ignore;
    uint16 enld_l, tx_pow;

    /* check A5/131 REG_STATUS_2 [3] TX_DIS_STATUS */
    drv_getAsicRegBits(EUROPA_REG_STATUS_2_ADDR, EUROPA_REG_STATUS_2_TX_DIS_STATUS_MASK, &tx_dis_status);

    /* check A4/94 REG_POLARITY [2] TX_DIS_PLOARITY_CTL */
    /*  0 for active high, 1 for active low */
    drv_getAsicRegBits(EUROPA_REG_POLARITY_ADDR, EUROPA_REG_POLARITY_TX_DIS_POLARITY_CTL_MASK, &tx_dis_polarity);

    /* check A4/84[1:0] for tx_dis_ctl */
    drv_getAsicRegBits(EUROPA_REG_CONTROL2_ADDR, EUROPA_REG_CONTROL2_TX_DIS_CTL_MASK, &tx_dis_ctl);

    if ( (tx_dis_status ^ tx_dis_polarity) && (0x03 != tx_dis_ctl) )
    {
	/* TX_DIS assert */
        return 1;
    }


    /* check A4/85[3] for ddmi_tx_dis */
    drv_getAsicRegBits(EUROPA_REG_CONTROL3_ADDR, EUROPA_REG_CONTROL3_DDMI_TX_DIS_MASK, &ddmi_tx_dis);

    /* check A4/85[4] for ddmi_tx_dis_hw_enable */
    drv_getAsicRegBits(EUROPA_REG_CONTROL3_ADDR, EUROPA_REG_CONTROL3_DDMI_TX_DIS_HW_ENABLE_MASK, &ddmi_tx_dis_hw_enable);

    /* check A5/155[1] for ignore_ddmi_tx_dis_set */
    drv_getAsicRegBits(EUROPA_REG_IIC_S_ADDR, EUROPA_REG_IIC_S_IGNORE_DDMI_TX_DIS_SET_MASK, &ddmi_tx_dis_ignore);

    if ( (ddmi_tx_dis && (ddmi_tx_dis_hw_enable && !ddmi_tx_dis_ignore)) && (0x03 != tx_dis_ctl) )
    {
	/* Soft TX_DIS assert */
        return 1;
    }

    /* check A4/84[3] and [2] for VDD_LDX and TX_POW */
    drv_getAsicRegBits(EUROPA_REG_CONTROL2_ADDR, EUROPA_REG_CONTROL2_ENLD_L_MASK, &enld_l);
    drv_getAsicRegBits(EUROPA_REG_CONTROL2_ADDR, EUROPA_REG_CONTROL2_TX_POW_CTL_MASK, &tx_pow);

    if ( (0==enld_l) || (0==tx_pow) )
    {
	/* TX_DIS by software assert */
        return 1;
    }

    return 0;
}




int32  rtl8290_reset( rtk_ldd_reset_mode_t mode)
{
    switch(mode)
    {
        case LDD_RESET_CHIP:
            drv_setAsicRegBit(EUROPA_REG_RESET_ADDR, EUROPA_REG_RESET_TOTAL_CHIP_RESET_OFFSET, 1);
            break;
        case LDD_RESET_DIGITAL:
            drv_setAsicRegBit(EUROPA_REG_RESET_ADDR, EUROPA_REG_RESET_ALL_DIGITAL_RESET_OFFSET, 1);
            break;
        case LDD_RESET_8051:
            drv_setAsicRegBit(EUROPA_REG_RESET_ADDR, EUROPA_REG_RESET_DW8051_RESET_OFFSET, 1);
            break;
        case LDD_RESET_ANALOG:
            drv_setAsicRegBit(EUROPA_REG_RESET_ADDR, EUROPA_REG_RESET_ALL_ANALOG_RESET_OFFSET, 1);
            break;
        default:
            break;
    }

    return SUCCESS;
}

int32  rtl8290_powerOnStatus_get(uint32 *result)
{

   uint16 status;
   uint16 cnt=0;

   *result = 0xFFFF;

    //printk(KERN_EMERG " rtl8290_powerOnStatus_get 1 result =0x %x\n", *result);

    /*A5/131[7:5]=110"  for BOSA on board*/
    while(1)
    {
        drv_getAsicRegBits(EUROPA_REG_STATUS_2_ADDR,0xFF,&status);
        if(((EUROPA_REG_STATUS_2_TEMP_VALID_MASK&status)!=0) && ((EUROPA_REG_STATUS_2_LVCMP_TX_VALID_MASK&status)!=0) &&
		((EUROPA_REG_STATUS_2_DEBUG_MODE_STATUS_MASK&status)==0) )
        {
            break;
        }
        cnt++;
        if (cnt>=20000)
        {
             printk(KERN_EMERG "EUROPA_REG_STATUS_2_ADDR check over 20000 times!!!!!\n");
           *result = 0;
             printk(KERN_EMERG " rtl8290_powerOnStatus_get 2 result =0x %x!!!!!\n", *result);
             return SUCCESS;
         }
    }

    cnt = 0;
    /*R calibration ready(A5_R1[8]=1)*/
    while(1)
    {
        drv_getAsicRegBits(EUROPA_REG_R1_ADDR, 0xff, &status);
        if((status & 0x80) == 0x80)
        {
           *result = 0;
            //printk(KERN_EMERG " rtl8290_powerOnStatus_get 3 result =0x %x!!!!\n", *result);
            return SUCCESS;
        }
        cnt++;
        if (cnt>=20000)
        {
             //SW user define fault!! useEUROPA_REG_SW_FAULT_STATUS1_ADDR bit 2
             printk(KERN_EMERG "R calibration over 20000 times!!!!!\n");
             drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, 2,1);
           *result = 0;
             printk(KERN_EMERG " rtl8290_powerOnStatus_get 4 result =0x %x!!!!!\n", *result);
             return SUCCESS;
         }

    }
    printk(KERN_EMERG " rtl8290_powerOnStatus_get 5 result =0x %x!!!!!\n", *result);
    *result = 0;
    return SUCCESS;
}

/* export version which save and restore W48 value */
int32  rtl8290_mpd0_get(uint16 count, uint32 *value)
{
    uint16 w48_orig;

    /* save W48 value */
    drv_getAsicRegBits(EUROPA_REG_W48_ADDR, 0xFF, &w48_orig);

    rtl8290_mpd0_get_NoBackUpW48(count, value);

    /* restore W48 value */
    drv_setAsicRegBits(EUROPA_REG_W48_ADDR, 0xFF, w48_orig);

    return SUCCESS;
}

/* orig version which had side-effect on clear W48 to 0 */
int32  rtl8290_mpd0_get_NoBackUpW48(uint16 count, uint32 *value)
{
    uint16 regData1, regData2, regData3, i;
    uint32 mpd;
    uint32 temp32;
    uint64 temp64;

    //drv_setAsicRegBits(EUROPA_REG_W48_ADDR, 0xff, 0);

    /** for debug ...
    drv_getAsicRegBits(EUROPA_REG_W48_ADDR, 0xff, &regData1);
    printk(" W48 = 0x%x\n", regData1);
    drv_getAsicRegBits(EUROPA_REG_W80_ADDR, 0xff, &regData1);
    printk(" W80 = 0x%x\n", regData1);
    drv_getAsicRegBits(EUROPA_REG_CONTROL1_ADDR, 0xff, &regData1);
    printk(" REG_CONTROL1 = 0x%x\n", regData1);
    ***/

    /* measure MPD0 in continuous mode */
    drv_setAsicRegBits(EUROPA_REG_W48_ADDR, 0xff, 0x80);

    temp64 = 0;
    for (i=0;i<count;i++)
    {
        //printk("loop %d: ", i);
        drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x02);
        //drv_getAsicRegBits(EUROPA_REG_W18_ADDR, 0xFF, &regData1);
        //printk("W18= 0x%x, ", regData1);
        udelay(DELAY_TIME);
        drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x0A);
        //drv_getAsicRegBits(EUROPA_REG_W18_ADDR, 0xFF, &regData1);
        //printk("W18= 0x%x, ", regData1);
        udelay(DELAY_TIME);
        drv_getAsicRegBits(EUROPA_REG_R11_ADDR, EUROPA_REG_R11_REG_R11_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R12_ADDR, EUROPA_REG_R12_REG_R12_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R13_ADDR, EUROPA_REG_R13_REG_R13_MASK, &regData3);
        temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
        //printk("R11= 0x%x, R12= 0x%x, R13= 0x%x,  ", regData1, regData2, regData3);
        /*Disable Hold action*/
        drv_setAsicRegBit(EUROPA_REG_W18_ADDR, 3, 0);
        //drv_getAsicRegBits(EUROPA_REG_W18_ADDR, 0xFF, &regData1);
        //printk("W18= 0x%x, ", regData1);
        printk(" MPD0[%d] = 0x%x\n", i, temp32);
        udelay(DELAY_TIME);
        temp64 +=temp32;
    }
    
    /* clear W48 value */
    drv_setAsicRegBits(EUROPA_REG_W48_ADDR, 0xff, 0x00);

    do_div(temp64, count);
    mpd = (uint32)temp64;
    printk(" MPD0 = 0x%x\n", mpd);
    *value = mpd;

    return SUCCESS;
}

int32  rtl8290_loopMode_set( rtk_ldd_loop_mode_t mode)
{
    // set W74 for different APC mode
    if(mode >=LDD_LOOP_SCL_BIAS)
    {
        drv_setAsicRegBits(EUROPA_REG_W74_ADDR, 0xFF, 0x09);
	 drv_setAsicRegBits(EUROPA_REG_W57_ADDR, 0xFF, 0xF8);
    }
    else
    {
         drv_setAsicRegBits(EUROPA_REG_W74_ADDR, 0xFF, 0x19);
    }

    udelay(500);

    //---- set SCL/DCL mode
    drv_setAsicRegBits(EUROPA_REG_W69_ADDR, EUROPA_REG_W69_APCDIG_LOOPMODE_MASK, mode);

    return SUCCESS;
}

int32  rtl8290_temperature_get(uint16 *temp)
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

    _bubble_sort(array, TEMPERATURE_NUM);

    tmp_sys = 0;
    for(i=0;i<(TEMPERATURE_NUM-4);i++)
    {
        tmp_sys +=array[i+2];
    }

    *temp = (tmp_sys/(TEMPERATURE_NUM-4));

    //printk("\nTenperature = %d , tmp_sys = %d ", *temp, tmp_sys);

    return SUCCESS;
}

int32  rtl8290_laserLut_set( rtk_ldd_loop_mode_t mode, uint8 *lut_data)
{
    //uint16 tmp1,tmp2;
    uint16 temp_sys, bias, modulation;
    uint8 *laser_lut_table;
    uint16 tmp;

    laser_lut_table = lut_data;

     rtl8290_temperature_get(&temp_sys);

    tmp = (temp_sys - 233)*2;
    bias = laser_lut_table[tmp];
    modulation = laser_lut_table[tmp+1];

    if (APC_DCL_MODE != mode)
    {
        if (APC_DOL_MODE == mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, bias);
            drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modulation);
	
	    printk(KERN_EMERG " rtl8290_laserLut_set: temperature = %d, Bias = 0x%x, Modulation = 0x%x\n", temp_sys, bias, modulation);
        }
        else if (APC_SCL_BIAS_MODE == mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modulation);
	
	    printk(KERN_EMERG " rtl8290_laserLut_set: temperature = %d, Modulation = 0x%x\n", temp_sys, modulation);
        }
	else if (APC_SCL_MOD_MODE == mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, bias);
	
	    printk(KERN_EMERG " rtl8290_laserLut_set: temperature = %d, Bias = 0x%x\n", temp_sys, bias);
        }

        /* Load BIAS/MOD code, Write REG_W70[7] to 1 and than write to 0 */
        drv_setAsicRegBits(EUROPA_REG_W70_ADDR, 0xFF, 0xBF);
        udelay(200);
        drv_setAsicRegBits(EUROPA_REG_W70_ADDR, 0xFF, 0x3F);
        udelay(200);
     }
	
    //printk(KERN_EMERG " rtl8290_laserLut_set: temperature = %d, Bias = 0x%x, Modulation = 0x%x\n", temp_sys, bias, modulation);

    return SUCCESS;
}

int32  rtl8290_apdLut_set(uint8 *lut_data)
{
    uint16 /*status, status2,*/ temperature;
    uint16 regData1;
    uint8 *apd_lut_table;

    apd_lut_table = lut_data;

     rtl8290_temperature_get(&temperature);

    //New Format only one byte.
    //regData1 = getReg(apd_lut_addr +  (temperature - 233)); //high byte(8bits) DAC[8:1]
    regData1 = apd_lut_table[temperature-233];

    drv_setAsicRegBits(EUROPA_REG_DAC_HB_ADDR, EUROPA_REG_DAC_HB_DIN_L_8_1_MASK, regData1);

    printk(KERN_EMERG " rtl8290_apdLut_set: temperature = %d, REG_DAC_HB = 0x%x\n", temperature, regData1);

    return SUCCESS;
}

int32  rtl8290_apcEnableFlow_set( rtk_ldd_apc_func_t func,  rtk_ldd_loop_mode_t mode)
{
    uint16 tmp, tmp2;
    uint16 tx_sd_threshold;
    uint16 cnt=0;
    uint32 mpd0;

    switch(func)
    {
        case LDD_APC_MCU_READY:
            //set MCU_ready
           drv_setAsicRegBit(EUROPA_REG_M0_ADDR, EUROPA_REG_M0_MCU_READY_STATUS_OFFSET, 0x1);
            break;

        case LDD_APC_CHECK_READY:
            //---- check ready_Bit status---- (bit 2 == 1)
            while(1)
            {
                drv_getAsicRegBit(EUROPA_REG_STATUS_1_ADDR, EUROPA_REG_STATUS_1_READY_STATUS_OFFSET, &tmp);
                if(tmp  == 0x1)
                    break;
                mdelay(1);
            }
            break;

        case LDD_APC_TX_SD_THRESHOLD:
            drv_setAsicRegBits(EUROPA_REG_W69_ADDR, 0xFF, 0x10);
            setReg(EUROPA_REG_W88_ADDR, 0x01);
            setReg(EUROPA_REG_W80_ADDR, 0x06); /* TX_SD fault when BEN 1-> 0 */
            setReg(EUROPA_REG_W71_ADDR, 0x05);
            setReg(EUROPA_REG_W72_ADDR, 0x86);
            setReg(EUROPA_REG_W57_ADDR, 0xFC);
            drv_setAsicRegBit(EUROPA_REG_W74_ADDR, 3, 1);
            setReg(EUROPA_REG_W73_ADDR, 0x06);
            //setReg(EUROPA_REG_W73_ADDR, 0x04); //For Test
            setReg(EUROPA_REG_W76_ADDR, 0x71);
            if (mode == LDD_LOOP_DOL)
            {
                drv_getAsicRegBit(EUROPA_REG_W57_ADDR, EUROPA_REG_W57_APCDIG_DAC_BIAS_HALF_OFFSET, &tmp);
                if(tmp  == 0x1)
                    tx_sd_threshold = 0x8;
                else
                    tx_sd_threshold = 0x10;
            }
            else if (mode == LDD_LOOP_DCL)
            {
                tx_sd_threshold = getReg(EUROPA_REG_W58_ADDR);
                tx_sd_threshold = tx_sd_threshold/2;
            }
            else
            {
                tx_sd_threshold = getReg(EUROPA_REG_W61_ADDR);
                tx_sd_threshold = tx_sd_threshold/10;
            }
            drv_setAsicRegBits(EUROPA_REG_W62_ADDR, EUROPA_REG_W62_APCDIG_CODE_DACTXSD_MASK, tx_sd_threshold);
            // Rogue ONU Issue
            //setReg(EUROPA_REG_W50_ADDR, 0x0f);
            setReg(EUROPA_REG_W50_ADDR, 0x07);
            setReg(EUROPA_REG_W68_ADDR, 0xf8);
            break;

        case LDD_APC_BIAS_POWER_ON:
            //Turn on APC  BIAS Power ;
            drv_setAsicRegBits(EUROPA_REG_W57_ADDR, 0xff, 0xFC);
            break;

        case LDD_APC_POWER_SET:
            //Set  APC power and config :set REG_W60=0xFC
            drv_setAsicRegBits(EUROPA_REG_W60_ADDR, 0xff, 0xFD);
            break;

        case LDD_APC_DIGITAL_POWER_ON:
            //power on APC_Digital
            drv_setAsicRegBit(EUROPA_REG_APCDIG_RSTN_ADDR, EUROPA_REG_APCDIG_RSTN_APCDIG_RSTN_OFFSET, 1);
            mdelay(100);;
            break;

        case LDD_APC_R_PROCESS_DETECT:
             /* Read REG_R31 TX_RPD_RCODE_OUT[1:0] and Write to REG_W50[7:6] / REG_W50[5:4] / REG_W73[4:3] */
            drv_getAsicRegBits(EUROPA_REG_R31_ADDR, EUROPA_REG_R31_TX_RPD_RCODE_OUT_MASK, &tmp);
            drv_setAsicRegBits(EUROPA_REG_W50_ADDR, EUROPA_REG_W50_TX_RLSEL_MASK, tmp);
            drv_setAsicRegBits(EUROPA_REG_W73_ADDR, EUROPA_REG_W73_APCDIG_DUAL_RLSEL_MASK, tmp);
            break;

        case LDD_APC_TX_RPD_RCODE_OUT:
            drv_setAsicRegBit(EUROPA_REG_W60_ADDR,EUROPA_REG_W60_APCDIG_TIA_RLSEL_OFFSET,1);
            break;

        case LDD_APC_OFFSET_CALIBR:
            //Offset calibration make sure REG_W63/64/65/66/67's[7] and W76[6]/W76[5]=1 ,
            //Set REG_W76[7](APCDIG_OFFK_EN) = 0==> 1
            drv_setAsicRegBit(EUROPA_REG_W63_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W64_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W65_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W66_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W67_ADDR,7,1);
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,5,1);
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,6,1);

            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,7,0);
            udelay(500);
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,7,1);
            break;

        case LDD_APC_MEASURE_MPD0:
             //rtl8290_mpd0_get(20, &mpd0) ;
	     rtl8290_mpd0_get_NoBackUpW48(20, &mpd0) ;
             printk("LDD_APC_MEASURE_MPD0: MPD0 = 0x%x\n", mpd0);
            break;

        case LDD_APC_WAIT_CALIBR:
            udelay(500);
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,7,1);
             //----wate 120us ,polling  REG_R30[7]= 1
            udelay(500);
            while(1)
            {
                drv_getAsicRegBit(EUROPA_REG_R30_ADDR, EUROPA_REG_R30_APCDIG_RO_OFFK_DONE_OFFSET, &tmp);
                if((tmp  == 1)||cnt>=2000)
                {
                    if (cnt>=2000)
                    {
                        //SW user define fault!! useEUROPA_REG_SW_FAULT_STATUS1_ADDR bit 1
                        printk("LDD_APC_WAIT_CALIBR over 2000 times!!!!!");
                        drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, 1,1);
			    return FAILED;
                    }
                    break;
                }
                cnt++;
                udelay(100); ;

            }
            drv_setAsicRegBit(EUROPA_REG_W76_ADDR,7,0);
            break;

        case LDD_APC_MOVE_CALIBR_RESULT:
            //set REG_W74[7:4](=4~8) , readREG_R29[7:0]
             //move offset result to 36bits,REG_W63[6:0]~REG_W67[6:0]----
             drv_getAsicRegBits(EUROPA_REG_W73_ADDR, 0xff, &tmp2);
             printk(" CALIBR_RESULT: W73 = 0x%x\n", tmp2);

             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x3);
             drv_getAsicRegBits(EUROPA_REG_W74_ADDR, 0xff, &tmp2);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             printk(" CALIBR_RESULT: W74 = 0x%x   R29 = 0x%x\n", tmp2, tmp);

             //write W63
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x4);
             drv_getAsicRegBits(EUROPA_REG_W74_ADDR, 0xff, &tmp2);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             printk(" CALIBR_RESULT: W74 = 0x%x   R29 = 0x%x\n", tmp2, tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W63_ADDR, 0x7F, tmp);
             //write W64
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x5);
             drv_getAsicRegBits(EUROPA_REG_W74_ADDR, 0xff, &tmp2);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             printk(" CALIBR_RESULT: W74 = 0x%x   R29 = 0x%x\n", tmp2, tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W64_ADDR, 0x7F, tmp);
            //write W65
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x6);
             drv_getAsicRegBits(EUROPA_REG_W74_ADDR, 0xff, &tmp2);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             printk(" CALIBR_RESULT: W74 = 0x%x   R29 = 0x%x\n", tmp2, tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W65_ADDR, 0x7F, tmp);
             //write W66
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x7);
             drv_getAsicRegBits(EUROPA_REG_W74_ADDR, 0xff, &tmp2);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             printk(" CALIBR_RESULT: W74 = 0x%x   R29 = 0x%x\n", tmp2, tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W66_ADDR, 0x7F, tmp);
             //write W67
             drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x8);
             drv_getAsicRegBits(EUROPA_REG_W74_ADDR, 0xff, &tmp2);
             drv_getAsicRegBits(EUROPA_REG_R29_ADDR, EUROPA_REG_R29_REG_R29_MASK, &tmp);
             printk(" CALIBR_RESULT: W74 = 0x%x   R29 = 0x%x\n", tmp2, tmp);
             tmp = tmp & 0x7F;
             drv_setAsicRegBits(EUROPA_REG_W67_ADDR, 0x7F, tmp);
            break;

        default:
            break;
    }

    return SUCCESS;
}

int32  rtl8290_txEnableFlow_set( rtk_ldd_tx_func_t func,  rtk_ldd_loop_mode_t mode, uint8 *lut_data)
{

    switch(func)
    {
        case LDD_TX_BIAS_MOD_PROTECT:
            drv_setAsicRegBits(EUROPA_REG_W77_ADDR, 0xFF, 0xa5);
            break;

        case LDD_TX_ENABLE_TX:
            drv_setAsicRegBits(EUROPA_REG_CONTROL2_ADDR, 0xFF, 0x8D);
            break;

        case LDD_TX_BIAS_MOD_LUT:
            /* ADD APCDIG_BIAS/MOD by 16LSB */
             rtl8290_laserLut_set(LDD_LOOP_DOL, lut_data);
            break;

#if 0
        case LDD_TX_P0_P1_PAV:
            //apc_dcl_P0 =  getReg(laser_lut_addr - 16 + 4);
            drv_setAsicRegBits(EUROPA_REG_W58_ADDR, EUROPA_REG_W58_APCDIG_P0_1_MASK, apc_dcl_P0);
            //apc_dcl_P1 = getReg(laser_lut_addr - 16 + 5);
            drv_setAsicRegBits(EUROPA_REG_W59_ADDR, EUROPA_REG_W59_APCDIG_P1_1_MASK, apc_dcl_P1);
            //apc_dcl_Pavg= getReg(laser_lut_addr - 16 + 6);
            drv_setAsicRegBits(EUROPA_REG_W61_ADDR, EUROPA_REG_W61_APCDIG_CODE_DACPAVG_MASK, apc_dcl_Pavg);
            break;
#endif

        case LDD_TX_LOOP_MODE:
             rtl8290_loopMode_set(mode);
            break;

        case LDD_TX_BURST_MODE_CTRL:
            /* Burst control change to be controlled by BENP/BENN --> SW should write REG_W48[6] = 0 */
            drv_setAsicRegBits(EUROPA_REG_W48_ADDR, 0xFF, 0);
            drv_setAsicRegBits(EUROPA_REG_W80_ADDR, EUROPA_REG_W80_APCDIG_BACKUP2_MASK, 0x7);
            break;

        case LDD_TX_WAIT_500MS:
            // set final W77 value
            //mdelay(500);
            mdelay(50);
            drv_setAsicRegBits(EUROPA_REG_W77_ADDR, EUROPA_REG_W77_APCDIG_BACKUP_MASK, 0x1);
            break;

        case LDD_TX_FAULT_ENABLE:
            /* Fault enable, turn on all bits in REG_W53 */
            drv_setAsicRegBits(EUROPA_REG_W53_ADDR, 0xff, 0xff);
            drv_setAsicRegBits(EUROPA_REG_FAULT_CTL_ADDR, 0xff, 0x2);
            drv_setAsicRegBits(EUROPA_REG_FAULT_INHIBIT_ADDR, 0xff, 0);
            break;

        case LDD_TX_ENABLE_TX_SD:
            //drv_setAsicRegBits(EUROPA_REG_CONTROL2_ADDR, 0xFF, 0x4D);
            drv_setAsicRegBits(EUROPA_REG_CONTROL2_ADDR, EUROPA_REG_CONTROL2_TX_SD_PIN_TRI_MASK, 0x0);
            break;

        default:
            break;
    }

    return SUCCESS;
}

int32  rtl8290_rxEnableFlow_set( rtk_ldd_rx_func_t func)
{
    switch(func)
    {
        case LDD_RX_BOOSTER:
            /* Enable Booster: set REG_W4<4> = 1 */
            drv_setAsicRegBit(EUROPA_REG_W4_ADDR, EUROPA_REG_W4_EN_L_OFFSET, 1);
            break;
        case LDD_RX_RX:
            /* Enable RX: set REG_W41<4> = 0 */
            drv_setAsicRegBits(EUROPA_REG_W41_ADDR, 0xFF, 0x08);
            udelay(2);
            drv_setAsicRegBits(EUROPA_REG_W41_ADDR, 0xFF, 0x28);
            break;
        case LDD_RX_RSSI:
            /*Enable RSSI*/
            drv_setAsicRegBits(EUROPA_REG_W40_ADDR, 0xFF, 0xFB);
            drv_setAsicRegBits(EUROPA_REG_W39_ADDR, EUROPA_REG_W39_REG_W39_MASK, 0);
            drv_setAsicRegBits(EUROPA_REG_W49_ADDR, 0xFF, 0xAC);
            drv_setAsicRegBits(EUROPA_REG_W38_ADDR, 0xFF, 0x88);
            drv_setAsicRegBits(EUROPA_REG_W75_ADDR, 0xFF, 0x88);
            break;
        case LDD_RX_DAC:
            /*Enable DAC*/
            drv_setAsicRegBits(EUROPA_REG_W36_ADDR, 0xFF, 0xBC);
            drv_setAsicRegBits(EUROPA_REG_W35_ADDR, EUROPA_REG_W35_REG_W35_MASK, 0x8);
            break;
        case LDD_RX_THRESHOLD:
            /*20150312 */
            setReg(EUROPA_REG_RX_TH_ADDR, 0x10);
            setReg(EUROPA_REG_RX_DE_TH_ADDR, 0x40);
            break;

        case LDD_RX_ENABLE_RX_SD:
	    mdelay(200);
            drv_setAsicRegBits(EUROPA_REG_CONTROL2_ADDR, EUROPA_REG_CONTROL2_LOS_PIN_TRI_MASK, 0x1);
            break;

        default:
            break;
    }

    return SUCCESS;
}


int32  rtl8290_rssiVoltage_get(uint32 *rssi_voltage)
{
    uint16 regData1, regData2, regData3;
    uint32 temp32;
    uint64 temp64;
    uint32 rssi_code, vdd_code, gnd_code, ldo_code, half_vdd_code;
    uint32 volt_half_vdd, volt_ldo, volt_rssi;//unit is uV

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
    rssi_code = temp32;

    //Read GND code
    temp32 = 0;
    drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
    temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    gnd_code = temp32;

    //Read VDD code
    temp32 = 0;
    drv_getAsicRegBits(EUROPA_REG_R5_ADDR, EUROPA_REG_R5_REG_R5_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R6_ADDR, EUROPA_REG_R6_REG_R6_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R7_ADDR, 0xFF, &regData3);
    temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    vdd_code = temp32;

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
    ldo_code = temp32;

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
    half_vdd_code = temp32;

    /*Disable Hold action*/
    drv_setAsicRegBit(EUROPA_REG_W18_ADDR, 3, 0);

    temp64 = half_vdd_code - gnd_code;
    temp64 = temp64*3300000;
    temp32 = vdd_code - gnd_code;
    do_div(temp64, temp32);
    volt_half_vdd = (uint32)temp64;
    //printk("volt_half_vdd = 0x%llx,  0x%x", temp64, rssi_para->volt_half_vdd);

    temp64 = ldo_code - gnd_code;
    temp64 = temp64*3300000;
    temp32 = vdd_code - gnd_code;
    do_div(temp64, temp32);
    volt_ldo =(uint32)temp64;
    //printk("volt_ldo = 0x%llx,  0x%x", temp64, rssi_para->volt_ldo);

#if 0
    temp64 = volt_ldo-volt_half_vdd;
    temp64 = temp64*(rssi_code - half_vdd_code);
    temp32 = ldo_code - half_vdd_code;
    do_div(temp64, temp32);
    volt_rssi = volt_half_vdd + (uint32)temp64;
#endif

    if (rssi_code>=half_vdd_code)
    {
        temp64 = abs(volt_ldo-volt_half_vdd);
        temp64 = temp64*(rssi_code - half_vdd_code);
        temp32 = abs(ldo_code - half_vdd_code);
        do_div(temp64, temp32);
        volt_rssi = volt_half_vdd + (uint32)temp64;
    }
    else
    {
        temp64 = abs(volt_ldo-volt_half_vdd);
        temp64 = temp64*(half_vdd_code - rssi_code);
        temp32 = abs(ldo_code - half_vdd_code);
        do_div(temp64, temp32);
        volt_rssi = volt_half_vdd - (uint32)temp64;
    }
    //printk(" rtl8290_rssiVoltage_get: rssi_code = 0x%x, gnd_code = 0x%x, vdd_code = 0x%x, ldo_code = 0x%x, half_vdd_code = 0x%x,  volt_half_vdd = 0x%x, volt_ldo = 0x%x, volt_rssi = 0x%x\n", rssi_code, gnd_code, vdd_code, ldo_code, half_vdd_code, volt_half_vdd, volt_ldo, volt_rssi);
    //printk("rssi_code = %d, gnd_code = %d, vdd_code = %d, ldo_code = %d, half_vdd_code = %d,  volt_half_vdd = %d, volt_ldo = %d, volt_rssi = %d\n", rssi_code, gnd_code, vdd_code, ldo_code, half_vdd_code, volt_half_vdd, volt_ldo, volt_rssi);

    *rssi_voltage = volt_rssi;

    return SUCCESS;
}

int32  rtl8290_rssiV0_get(uint16 loopcnt, uint32 *rssi_v0)
{
    uint32 temp;
    uint16 i;
    uint32 sum;
    int32 ret;

    sum = 0;
    for (i=0;i<loopcnt;i++)
    {
        ret = rtl8290_rssiVoltage_get(&temp);
        if(ret)
        {
            printk("Europa RSSI V0 fail!!! (%d)\n", ret);
            return FAILED;
        }
        sum += temp;
    }

    //do_div(sum, loopcnt);
    //*rssi_v0 = sum;

    *rssi_v0 = sum/loopcnt;

    return SUCCESS;
}

int32  rtl8290_rx_power_get(uint32 rssi_v0, uint32 *v_rssi,uint32 *i_rssi)
{
    //uint16 regData1, regData2, regData3;
    uint32 temp32;
    uint64 temp64;
    //uint32 rssi_code, vdd_code, gnd_code, ldo_code, half_vdd_code;
    uint32 /*volt_half_vdd, volt_ldo,*/ volt_rssi;//unit is uV
    uint16 ra, rb;

    //Enable Booster
    //drv_setAsicRegBit(EUROPA_REG_W4_ADDR,4,1);
    //mdelay(2);

     rtl8290_rssiVoltage_get(&volt_rssi);

    *v_rssi = volt_rssi;

    if (volt_rssi<rssi_v0)
    {
        *i_rssi = 0;
    }	
    else
    {
        //The unit is nA
        ra = 1000;
        rb = 1000;
        //*i_rssi = 1000*(volt_rssi- rssi_v0)*(ra+rb)/(ra*rb);
    
        temp64 = abs(volt_rssi- rssi_v0);
        temp64 = temp64*(ra+rb)*1000;
        temp32 = ra*rb;
        do_div(temp64, temp32);
        *i_rssi  = (uint32)temp64;
    }
    //printk(" rtl8290_rx_power_get: RSSI_voltage = %d, RSSI V0 = %d, I_RSSI = %d, ra = %d, rb = %d\n",volt_rssi, rssi_v0, *i_rssi, ra, rb);
    //printk(" rtl8290_rx_power_get: rssi_code = %d, gnd_code = %d, vdd_code = %d, ldo_code = %d, half_vdd_code = %d,  volt_half_vdd = %d, volt_ldo = %d, volt_rssi = %d RSSI_voltage = %d, RSSI V0 = %d, I_RSSI = %d, ra = %d, rb = %d\n", rssi_code, gnd_code, vdd_code, ldo_code, half_vdd_code, volt_half_vdd, volt_ldo, volt_rssi, volt_rssi, rssi_v0, *i_rssi, ra, rb);

    return SUCCESS;
}

void Check_C0_C1(void)
{
    uint16 regData1, regData2, regData3;
    uint32 c0, c1, c0_1, c1_1;

    /* wait both C0 and C1 stable, Set C0C1_VALID = 1 */
    drv_getAsicRegBits(EUROPA_REG_R5_ADDR, EUROPA_REG_R5_REG_R5_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R6_ADDR, EUROPA_REG_R6_REG_R6_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R7_ADDR, EUROPA_REG_R7_REG_R7_7_TO_2_MASK, &regData3);
    c1 = ((regData1 & 0xff) << 14) | ((regData2 & 0xff) << 6) | (regData3 & 0x3f);
    while(1)
    {
        busy_loop(0x800);
        drv_getAsicRegBits(EUROPA_REG_R5_ADDR, EUROPA_REG_R5_REG_R5_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R6_ADDR, EUROPA_REG_R6_REG_R6_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R7_ADDR, EUROPA_REG_R7_REG_R7_7_TO_2_MASK, &regData3);
        c1_1 = ((regData1 & 0xff) << 14) | ((regData2 & 0xff) << 6) | (regData3 & 0x3f);
        if(c1_1 > c1)
        {
            if(c1_1 - c1 < 5)
                break;
        }
        else
        {
            if(c1 - c1_1 < 5)
                break;
        }
        c1 = c1_1;
    }

    drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R22_ADDR, EUROPA_REG_R22_REG_R22_7_TO_2_MASK, &regData3);
    c0 = ((regData1 & 0xff) << 14) | ((regData2 & 0xff) << 6) | (regData3 & 0x3f);
    while(1)
    {
        busy_loop(0x800);
        drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R22_ADDR, EUROPA_REG_R22_REG_R22_7_TO_2_MASK, &regData3);
        c0_1 = ((regData1 & 0xff) << 14) | ((regData2 & 0xff) << 6) | (regData3 & 0x3f);
        if(c0_1 > c0)
        {
            if(c0_1 - c0 < 5)
                break;
        }
        else
        {
            if(c0 - c0_1 < 5)
                break;
        }
        c0 = c0_1;
    }

    drv_setAsicRegBit(EUROPA_REG_M0_ADDR, EUROPA_REG_M0_C0C1_VALID_OFFSET, 1);

}

int32  rtl8290_i2c_init(rtk_i2c_port_t i2cPort)
{
    //uint32 data;
    //uint32 value;
    //uint32 array_idx;
    rtk_enable_t enable;
    rtk_i2c_width_t width;
    int32 retval;
    uint32 clock;

    retval = rtk_i2c_init(i2cPort);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_i2c_init fail  retval=%d \n",retval);
        return FAILED;
    }
    retval = rtk_i2c_enable_set(i2cPort,ENABLED);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_i2c_enable_set fail  retval=%d \n",retval);
        return FAILED;
    }
    retval = rtk_i2c_width_set(i2cPort,I2C_WIDTH_8bit);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_i2c_width_set fail  retval=%d \n",retval);
        return FAILED;
    }
    //Set to 1MHz in diagshell mode
    retval = rtk_i2c_clock_set(i2cPort,1000);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_i2c_clock_set fail  retval=%d \n",retval);
        return FAILED;
     }
    retval = rtk_i2c_enable_get(i2cPort,&enable);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_i2c_enable_get fail  retval=%d \n",retval);
        return FAILED;
    }
    retval = rtk_i2c_width_get(i2cPort,&width);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_i2c_width_get fail  retval=%d \n",retval);
        return FAILED;
    }
    retval = rtk_i2c_clock_get(i2cPort,&clock);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_i2c_width_get fail  retval=%d \n",retval);
        return FAILED;
    }

    printk(KERN_EMERG "rtl8290_i2c_init  enable = %d width=%d  clock = %d \n",enable, width, clock);

    return SUCCESS;
}   /* end of dal_apollomp_i2c_europa_init */

int32  rtl8290_gpio_init(uint32 pin)
{
    int32 retval;

    retval = rtk_gpio_mode_set(pin,GPIO_OUTPUT);	;
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_gpio_mode_set fail  retval=%d \n",retval);
        return FAILED;
    }

    retval = rtk_gpio_state_set(pin, ENABLED);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_gpio_state_set fail  retval=%d \n",retval);
        return FAILED;
    }

    retval = rtk_gpio_databit_set(pin, 0);
    if(retval != 0)
    {
        printk(KERN_EMERG "rtk_gpio_databit_set fail  retval=%d \n",retval);
        return FAILED;
    }

    printk(KERN_EMERG "Config GPIO %d to OUTPUT mode, value = 0\n", pin);

    return SUCCESS;
}

//hmchung: The function to be define
int32 rtl8290_tx_power_get(uint32 mpd0, uint32 *v_mpd, uint32 *i_mpd)
{
    uint16 regData1, regData2, regData3;
    uint32 real_mpd, c0, mpd, sdadc_ldo, mpd1;
    uint16 mpd_gain;
    uint64 temp64;

    /* use MPD0 value measured at APC init flow */
    mpd0 = europa_param.mpd0;

    /*** new SDADC workaround ***/
    // if (LOS) and NOT(TX_SD) return 0
    {
	uint16 rx_los, los_polarity;
	uint16 tx_sd;

	/* check A5/131 REG_STATUS_2 [2] RX_LOS_STATUS */
	/*  0 for deassert, 1 for assert */
        drv_getAsicRegBits(EUROPA_REG_STATUS_2_ADDR, EUROPA_REG_STATUS_2_RX_LOS_STATUS_MASK, &rx_los);

	/* check A4/94 REG_POLARITY [0] LOS_SD_PLOARITY_CTL */
	/*  0 for SD, 1 for LOS */
        drv_getAsicRegBits(EUROPA_REG_POLARITY_ADDR, EUROPA_REG_POLARITY_LOS_SD_POLARITY_CTL_MASK, &los_polarity);

	/* if los_polarity==rx_los means there is no valid rx signal */

	/* check A5/194 EXTRA_STATUS[4] for tx_sd */
        drv_getAsicRegBits(EUROPA_REG_EXTRA_STATUS1_ADDR, EUROPA_REG_EXTRA_STATUS1_TX_SD_MASK, &tx_sd);

	if ( (rx_los==los_polarity) && (0==tx_sd))
	{
	    *v_mpd = 0;
	    *i_mpd = 0;
    
	    return SUCCESS;
	}
    }


    if (_rtl8290_is_tx_disabled())
    {
	    *v_mpd = 0;
	    *i_mpd = 0;
    
	    return SUCCESS;
    }

    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x62);
    udelay(DELAY_TIME);
    drv_setAsicRegBits(EUROPA_REG_W18_ADDR, EUROPA_REG_W18_REG_W18_MASK, 0x6A);
    udelay(DELAY_TIME);
    //Read MPD code
    drv_getAsicRegBits(EUROPA_REG_R11_ADDR, EUROPA_REG_R11_REG_R11_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R12_ADDR, EUROPA_REG_R12_REG_R12_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R13_ADDR, EUROPA_REG_R13_REG_R13_MASK, &regData3);
    mpd = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);

    //if (mpd0>mpd)
    //{
    //     *v_mpd = 0;
    //     *i_mpd = 0;
    //}
    //else
    //{
        //Read LDO code
        drv_getAsicRegBits(EUROPA_REG_R14_ADDR, EUROPA_REG_R14_REG_R14_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R15_ADDR, EUROPA_REG_R15_REG_R15_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R16_ADDR, EUROPA_REG_R16_REG_R16_MASK, &regData3);
        sdadc_ldo =((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    
        //Read CAL_GND code
        drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
        drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
        drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
        c0 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    
        /*Disable Hold action*/
       drv_setAsicRegBit(EUROPA_REG_W18_ADDR, 3, 0);
    
        /*Get MPD Gain*/
        drv_getAsicRegBit(EUROPA_REG_W60_ADDR,EUROPA_REG_W60_APCDIG_TIA_RLSEL_OFFSET,&mpd_gain);
    
    if (mpd0>mpd)
    {
         *v_mpd = 0;
         *i_mpd = 0;

         real_mpd = 0;
    }
    else
    {
        //for APC_low Gain, set SDADC_MPD [expr ( 3.3 * ($MPD - 0x800000) / ($CAL_VCC - $CAL_GND)  * 0.0019 )*1000000 ]
        temp64 = mpd - mpd0;
        temp64 = temp64*1200000;
        do_div(temp64, (sdadc_ldo - c0));
        mpd1 = (uint32)temp64;
    
        *v_mpd = mpd1;
    
        if (mpd_gain == 1)
        {
            mpd1 = mpd1/500;
        }
        else
        {
            mpd1 = mpd1/250;
        }
    
        real_mpd = mpd1/2;
    
        *i_mpd = real_mpd;
    }

    if (europa_debug_level_get())
    {
        printk(" rtl8290_tx_power_get: MPD = 0x%x, LDO  = 0x%x, GND = 0x%x, MPD0 = 0x%x, MPD_GAIN = %d, MPD1 = 0x%x, Real MPD = 0x%x\n", mpd, sdadc_ldo, c0, mpd0, mpd_gain, *v_mpd, real_mpd);
    }

    return SUCCESS;
}

int32  rtl8290_vdd_get(uint32 *vdd)
{
    uint16 regData1, regData2, regData3;
    uint32 temp32;
    uint64 temp64;
    uint32 vdd_code, gnd_code, ldo_code;
    uint32 volt_vdd;//unit is uV

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
    ldo_code = temp32;

    //Read GND code
    temp32 = 0;
    drv_getAsicRegBits(EUROPA_REG_R20_ADDR, EUROPA_REG_R20_REG_R20_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R21_ADDR, EUROPA_REG_R21_REG_R21_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R22_ADDR, 0xFF, &regData3);
    temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    gnd_code = temp32;

    //Read External VDD code
    temp32 = 0;
    drv_getAsicRegBits(EUROPA_REG_R8_ADDR, EUROPA_REG_R8_REG_R8_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R9_ADDR, EUROPA_REG_R9_REG_R9_MASK, &regData2);
    drv_getAsicRegBits(EUROPA_REG_R10_ADDR, 0xFF, &regData3);
    temp32 = ((regData1 & 0xff) << 16) | ((regData2 & 0xff) << 8) | (regData3);
    vdd_code = temp32;

    /*Disable Hold action*/
    drv_setAsicRegBit(EUROPA_REG_W18_ADDR, 3, 0);

    //volt_sys =  (10000*(12*(sdadc_vdd - sdadc_ldo))/(10*(sdadc_ldo - c0)) + 12000);

    temp64 = vdd_code - ldo_code;
    temp64 = temp64*1200000;
    temp32 = ldo_code - gnd_code;
    do_div(temp64, temp32);
    volt_vdd = (uint32)(temp64+1200000);
    //printk("volt_half_vdd = 0x%llx,  0x%x", temp64, rssi_para->volt_half_vdd);

    //printk(" rtl8290_vdd_get: gnd_code = 0x%x, vdd_code = 0x%x, ldo_code = 0x%x, volt_vdd = 0x%x\n", gnd_code, vdd_code, ldo_code, volt_vdd);
    //printk(" rtl8290_rssiVoltage_get: rssi_code = %d, gnd_code = %d, vdd_code = %d, ldo_code = %d, half_vdd_code = %d,  volt_half_vdd = %d, volt_ldo = %d, volt_rssi = %d\n", rssi_code, gnd_code, vdd_code, ldo_code, half_vdd_code, volt_half_vdd, volt_ldo, volt_rssi);

    *vdd = volt_vdd;

    return SUCCESS;
}

int32  rtl8290_parameter_set(uint32 length, uint32 offset, uint8 *flash_data)
{
    uint16 addr, data;
    uint8 *data_ptr;

    if(flash_data == NULL)
    {
        printk(KERN_EMERG " rtl8290_parameter_set flash_data NULL!!! \n");
        return FAILED;
    }

    if(length >0xFF)
    {
        printk(KERN_EMERG " rtl8290_parameter_set length =%d !!! \n", length);
        return FAILED;
    }

    data_ptr = flash_data;

    //printk(KERN_EMERG " rtl8290_parameter_set: length = %d, offset = 0x%x,  flash_data = 0x%x!!! \n", length, offset, flash_data);

    for(addr=0; addr < length ; addr++)
    {
        data = *data_ptr;
        setReg(addr+offset, data);
#if 0 //Close debug message
        if (addr%16==0)
            printk(" addr = 0x%3X  ", addr+offset);

        printk("%02X ", data);

        if(addr%8 == 7)
            printk("   ");
        if(addr%16 == 15)
            printk( "\n");
#endif
        data_ptr++;
    }

    return SUCCESS;

}

int32  rtl8290_parameter_get(uint32 length, uint32 offset, uint8 *flash_data)
{
    uint16 addr, data;
    uint8 *data_ptr;

    if(flash_data == NULL)
    {
        printk(KERN_EMERG " rtl8290_parameter_get flash_data NULL!!! \n");
        return FAILED;
    }

    if(length >0xFF)
    {
        printk(KERN_EMERG " rtl8290_parameter_get length =%d !!! \n", length);
        return FAILED;
    }

    data_ptr = flash_data;

    //printk(KERN_EMERG " rtl8290_parameter_get: length = %d, offset = 0x%x,  flash_data = 0x%x!!! \n", length, offset, flash_data);

    for(addr=0; addr < length ; addr++)
    {
        data = getReg(addr+offset);
        *data_ptr = (uint8)data;

        if (addr%16==0)
            printk(" addr = 0x%3X  ", addr+offset);

        printk("%02X ", *data_ptr);

        if(addr%8 == 7)
            printk("   ");
        if(addr%16 == 15)
            printk( "\n");

        data_ptr++;
    }

    return SUCCESS;

}

/*
 * BiasCurrent: 80mA max for 10bits DAC, unit for return value is 2uA
 */
int32  rtl8290_tx_bias_get(uint32 *bias)
{
    uint16 regData1, regData2;
    uint32 bias_sys;

    //tx bias translation function
    drv_getAsicRegBits(EUROPA_REG_R33_ADDR, EUROPA_REG_R33_APCDIG_RO_BIAS_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R34_ADDR, EUROPA_REG_R34_APCDIG_RO_BIAS_1_MASK, &regData2);
    bias_sys = (regData1 << 4) | regData2;
    *bias = bias_sys *((80*1000)/(4096*2));

    //printk(KERN_EMERG " rtl8290_tx_bias_get: ASIC Internal Value = 0x%x\n", bias_sys);

    return SUCCESS;
}

/*
 * ModCurrent: 100mA max for 10bits DAC, unit for return value is 2uA
 */
int32  rtl8290_tx_mod_get(uint32 *mod)
{
    uint16 regData1, regData2;
    uint32 mod_sys;

    //tx mod translation function
    drv_getAsicRegBits(EUROPA_REG_R32_ADDR, EUROPA_REG_R32_APCDIG_RO_MOD_MASK, &regData1);
    drv_getAsicRegBits(EUROPA_REG_R34_ADDR, EUROPA_REG_R34_APCDIG_RO_MOD_1_MASK, &regData2);
    mod_sys = (regData1 << 4) | regData2;
    *mod = mod_sys *((100*1000)/(4096*2));

    //printk(KERN_EMERG " rtl8290_tx_mod_get: ASIC Internal Value = 0x%x\n", mod_sys);

    return SUCCESS;
}

/*
 * BiasCurrent: 80mA max for 10bits DAC, unit for return value is 2uA
 */
int32  rtl8290_tx_bias_set(uint32 bias)
{
    uint16 w54, w56;
    uint32 bias_sys;

    if (80<bias)
    {
	    printk(KERN_EMERG " rtl8290_tx_bias_set: Invalid bias current.\n");
	    return FAILED;
    }

    bias_sys = bias * 4096 / 80;

    w54 = (bias_sys & 0xff0)>>4;
    w56 = (bias_sys & 0x00f);

    drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, w54);
    drv_setAsicRegBits(EUROPA_REG_W56_ADDR, EUROPA_REG_W56_APCDIG_BIAS_2_MASK, w56);

    /* Load BIAS/MOD initial code, Write REG_W70[7] to 1 and than write to 0 */
    drv_setAsicRegBits(EUROPA_REG_W70_ADDR, EUROPA_REG_W70_APCDIG_CODE_LOADIN_MASK, 1);
    udelay(200);
    drv_setAsicRegBits(EUROPA_REG_W70_ADDR, EUROPA_REG_W70_APCDIG_CODE_LOADIN_MASK, 0);
    udelay(200);
 
    printk(KERN_EMERG " rtl8290_tx_bias_set: done. %d: 0x%02x 0x%02x\n",
		    bias, w54, w56);
    return SUCCESS;
}

/*
 * ModCurrent: 100mA max for 10bits DAC, unit for return value is 2uA
 */
int32  rtl8290_tx_mod_set(uint32 mod)
{
    uint16 w55, w56;
    uint32 mod_sys;

    if (100<mod)
    {
	    printk(KERN_EMERG " rtl8290_tx_mod_set: Invalid mod current.\n");
	    return FAILED;
    }

    mod_sys = mod * 4096 / 100;

    w55 = (mod_sys & 0xff0)>>4;
    w56 = (mod_sys & 0x00f);

    drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, w55);
    drv_setAsicRegBits(EUROPA_REG_W56_ADDR, EUROPA_REG_W56_APCDIG_MOD_2_MASK, w56);

    /* Load BIAS/MOD initial code, Write REG_W70[7] to 1 and than write to 0 */
    drv_setAsicRegBits(EUROPA_REG_W70_ADDR, EUROPA_REG_W70_APCDIG_CODE_LOADIN_MASK, 1);
    udelay(200);
    drv_setAsicRegBits(EUROPA_REG_W70_ADDR, EUROPA_REG_W70_APCDIG_CODE_LOADIN_MASK, 0);
    udelay(200);
 
    printk(KERN_EMERG " rtl8290_tx_mod_set: done. %d: 0x%02x 0x%02x\n",
		    mod, w55, w56);
 

    return SUCCESS;
}


int32 rtl8290_calibration_state_set(rtk_enable_t state)
{
    if (state)
    {
        // disable interrupt
        europa_intrDeInit();
        // mark calibration state enabled
        calibration_state_set(state);
    }
    else
    {
        // mark calibration state disabled
        calibration_state_set(state);
        // enable interrupt
        europa_intrInit();
    }

    return SUCCESS;
}

int32 rtl8290_driver_version_get(uint32 *ver)
{
    *ver = EUROPA_DRIVER_VERSION;
    return SUCCESS;
}


//--------------------------------------------------------------------
// steering mode & integration mode API 
//--------------------------------------------------------------------

void _rtl8290_loadin(void)
{
    drv_setAsicRegBits(EUROPA_REG_W70_ADDR, EUROPA_REG_W70_APCDIG_CODE_LOADIN_MASK, 1);
    udelay(200);
    drv_setAsicRegBits(EUROPA_REG_W70_ADDR, EUROPA_REG_W70_APCDIG_CODE_LOADIN_MASK, 0);
}
 

/* Steering Mode:
 * 
 *   This mode is used before O5 for fast loop convergence during 
 *   short burst length Tx data.
 * 
 *   SCL/M, with fast loop configuration (W68=0x81, W69=0x13)
 */
int32  rtl8290_steering_mode_set(void)
{
    uint16 P0,P1,Pavg;
    uint16 biasDAC, modDAC;

    /* set Pavg = (P0*8 + P1*25)/32 */
    drv_getAsicRegBits(EUROPA_REG_W58_ADDR, 0xff, &P0);
    drv_getAsicRegBits(EUROPA_REG_W59_ADDR, 0xff, &P1);
    Pavg = (P0*8 + P1*25)/32;
    drv_setAsicRegBits(EUROPA_REG_W61_ADDR, 0xff, Pavg);

    /* set faster SCL */
    drv_setAsicRegBits(EUROPA_REG_W68_ADDR, 0xff, 0x81);
    drv_setAsicRegBits(EUROPA_REG_W69_ADDR, 0xff, 0x13);
    rtl8290_loopMode_set(APC_SCL_MOD_MODE);

    rtl8290_laserLut_set(APC_SCL_MOD_MODE, &flash_data[EUROPA_LASER_LUT_ADDR]);

    /* BIAS/MOD validation */
    drv_getAsicRegBits(EUROPA_REG_R33_ADDR, EUROPA_REG_R33_APCDIG_RO_BIAS_MASK, &biasDAC);
    drv_getAsicRegBits(EUROPA_REG_R32_ADDR, EUROPA_REG_R32_APCDIG_RO_MOD_MASK, &modDAC);
    if (0x10>biasDAC)
    {
        biasDAC = 0x10;
        drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, biasDAC);
	_rtl8290_loadin();
    }
    if (0x15>modDAC)
    {
        modDAC = 0x15;
        drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modDAC);

	rtl8290_loopMode_set(APC_DOL_MODE);
	_rtl8290_loadin();
	rtl8290_loopMode_set(APC_SCL_MOD_MODE);
    }
 
    europa_param.steering_mode = 1;

    printk(KERN_EMERG " rtl8290_steering_mode_set: done.\n");
 
    return SUCCESS;
}

/* Integration Mode:
 * 
 *   This mode is used after O5 for stable convergence.
 * 
 *   DCL, restore SCL/M original settings (W68=0xF8, W69=0x10)
 */
int32  rtl8290_integration_mode_set(void)
{
    if ( 1 == europa_param.steering_mode )
    {
       drv_setAsicRegBits(EUROPA_REG_W68_ADDR, 0xff, 0xf8);
       drv_setAsicRegBits(EUROPA_REG_W69_ADDR, 0xff, 0x10);

       rtl8290_loopMode_set(APC_DCL_MODE);

       europa_param.steering_mode = 0;

       printk(KERN_EMERG " rtl8290_integration_mode_set: done.\n");
    }

    return SUCCESS;
}


/* Steering Mode fixup
 * 
 *   Since SCL/M rely on proper BIAS LUT, this fixup can be called
 *   during O3->O2 state, to compensate in case BIAS LUT is not good.
 */
int32  rtl8290_steering_mode_fixup(void)
{
    uint16 Pavg;
    uint16 biasDAC, modDAC;
    uint32 Vmpd, Impd;
    static uint8 cnt = 0;

    uint16 biasStep, modStep;


    if (_rtl8290_is_tx_disabled())
        return SUCCESS;


    if ( 1 == europa_param.steering_mode )
    {
       /* get real_pavg */
       drv_setAsicRegBits(EUROPA_REG_W74_ADDR, EUROPA_REG_W74_APCDIG_RO_DATA_SEL_MASK, 0x0);
       drv_getAsicRegBits(EUROPA_REG_R29_ADDR, 0xff, &Pavg);

       /* get mpd current */
       europa_debug_level_set(0);
       rtl8290_tx_power_get(europa_param.mpd0, &Vmpd, &Impd);
       europa_debug_level_set(1);

       /* get BIAS/MOD DAC */
       drv_getAsicRegBits(EUROPA_REG_R33_ADDR, EUROPA_REG_R33_APCDIG_RO_BIAS_MASK, &biasDAC);
       drv_getAsicRegBits(EUROPA_REG_R32_ADDR, EUROPA_REG_R32_APCDIG_RO_MOD_MASK, &modDAC);

       //printk(KERN_EMERG "\n--> Pavg=0x%x I_MPD=%duA BIAS=0x%x MOD=0x%x\n", Pavg, Impd, biasDAC, modDAC);
    
       /* BIAS/MOD DAC compensation */
       biasStep=2;
       modStep=5;
       if (Impd < ((Pavg*4)/2))  /* 1/2 target */
       {
          if (0 == Impd)
          {
	      if (5 > cnt++)
	      {
	         biasDAC = biasDAC + biasDAC/4;
                 //printk(KERN_EMERG "  > correct BIAS to 0x%x\n", biasDAC);
	      }
	  }
 
	  if (Impd < ((Pavg*4)/4)) /* 1/4 target */
	  {
	      biasStep=3;
	      modStep=10;
	  }

	  biasDAC += biasStep;
          //printk(KERN_EMERG "  > compensate BIAS to 0x%x\n", biasDAC);
	  modDAC += modStep;
          //printk(KERN_EMERG "  > compensate MOD to 0x%x\n", modDAC);

          drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, biasDAC);
          drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modDAC);

	  rtl8290_loopMode_set(APC_DOL_MODE);
	  _rtl8290_loadin();
	  rtl8290_loopMode_set(APC_SCL_MOD_MODE);
       }
       else if (Impd > (Pavg*4))
       {
	  if ((0 == modDAC) && (0 != biasDAC))
	  {
	      biasDAC = biasDAC - biasDAC/4;
	  }

	  biasDAC -= 4;
          //printk(KERN_EMERG "  > decrease BIAS to 0x%x\n", biasDAC);

          drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, biasDAC);
	  _rtl8290_loadin();
       }

       //printk(KERN_EMERG " rtl8290_steering_mode_fixup: done.\n");
    }

    return SUCCESS;
}


/* Integration Mode fixup
 * 
 *   DCL mode stay in ignore-validation mode for a while, then
 *   switch back to validation mode.
 *   This can be done after O5, to be "trained" with longer burst.
 */
int32  rtl8290_integration_mode_fixup(void)
{
#if 0
    /* stay in "ignore validation" for a while */
    drv_setAsicRegBits(EUROPA_REG_W77_ADDR, EUROPA_REG_W77_APCDIG_BACKUP_MASK, 0x05);

    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(200*HZ/100); /* delay 200 ms */

    drv_setAsicRegBits(EUROPA_REG_W77_ADDR, EUROPA_REG_W77_APCDIG_BACKUP_MASK, 0x01);
#endif

    //printk(KERN_EMERG " rtl8290_integration_mode_fixup: done.\n");
 
    return SUCCESS;
}


