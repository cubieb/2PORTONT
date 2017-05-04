#ifdef CONFIG_EUROPA_MODULE
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

#include <module/europa/europa.h>
#include <module/europa/common.h>
#include <module/europa/utility.h>
#include <module/europa/apc_enable.h>
#include <module/europa/tx_enable.h>
#include <module/europa/init.h>
#include <module/europa/ddmi.h>
#include <module/europa/interrupt.h>

#include <module/europa/rtl8290_api.h>


#define INTERNAL_DDMI_A0_REG_START_ADDRESS 0
#define INTERNAL_DDMI_A2_REG_START_ADDRESS 256

#define EUROPA_REGISTER_PATCH_ADDR              (0xC000+0x0202)

extern uint8 flash_data[];
extern uint8 loopmode;
//extern uint16 loopcnt;
extern uint8  internal;
//extern uint8 loop_mode;
extern uint8 flash_update;

//uint32 voltage_v0;
//uint32 global_mpd0;

extern rtl8290_param_t europa_param;

#if 0
uint8 apd_lut_table[] = {
0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94,
0x94, 0x94, 0x94, 0x94, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x96, 0x98, 0x98,
0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA2, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
0xA4, 0xA4, 0xA4, 0xA4, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6,
0xA6, 0xA6, 0xA6, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0
};
#endif

uint8 local_apd_lut_table[] = {
0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B,
0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D,
0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23,
0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24
};

uint8 local_laser_lut_table[][2] = {
{0x0A, 0x20}, {0x0A, 0x20}, {0x0A, 0x20}, {0x0A, 0x20}, {0x0A, 0x20}, {0x0A, 0x20}, {0x0A, 0x20}, {0x0A, 0x20},
{0x0A, 0x20}, {0x0A, 0x20}, {0x0C, 0x20}, {0x0C, 0x20}, {0x0C, 0x20}, {0x0C, 0x20}, {0x0C, 0x20}, {0x0C, 0x20},
{0x0C, 0x20}, {0x0C, 0x20}, {0x0C, 0x20}, {0x0C, 0x20}, {0x0E, 0x20}, {0x0E, 0x20}, {0x0E, 0x20}, {0x0E, 0x20},
{0x0E, 0x20}, {0x0E, 0x20}, {0x0E, 0x20}, {0x0E, 0x20}, {0x0E, 0x20}, {0x0E, 0x20}, {0x10, 0x20}, {0x10, 0x20},
{0x10, 0x20}, {0x10, 0x20}, {0x10, 0x20}, {0x10, 0x20}, {0x10, 0x20}, {0x10, 0x20}, {0x10, 0x20}, {0x10, 0x20},
{0x12, 0x20}, {0x12, 0x20}, {0x12, 0x20}, {0x12, 0x20}, {0x12, 0x20}, {0x12, 0x20}, {0x12, 0x20}, {0x12, 0x20},
{0x12, 0x20}, {0x12, 0x20}, {0x14, 0x32}, {0x14, 0x32}, {0x14, 0x32}, {0x14, 0x32}, {0x14, 0x32}, {0x14, 0x32},
{0x14, 0x32}, {0x14, 0x32}, {0x14, 0x32}, {0x14, 0x32}, {0x18, 0x34}, {0x18, 0x34}, {0x18, 0x34}, {0x18, 0x34},
{0x18, 0x34}, {0x18, 0x34}, {0x18, 0x34}, {0x18, 0x34}, {0x18, 0x34}, {0x18, 0x34}, {0x1C, 0x38}, {0x1C, 0x38},
{0x1C, 0x38}, {0x1C, 0x38}, {0x1C, 0x38}, {0x1C, 0x38}, {0x1C, 0x38}, {0x1C, 0x38}, {0x1C, 0x38}, {0x1C, 0x38},
{0x24, 0x40}, {0x24, 0x40}, {0x24, 0x40}, {0x24, 0x40}, {0x24, 0x40}, {0x24, 0x40}, {0x24, 0x40}, {0x24, 0x40},
{0x24, 0x40}, {0x24, 0x40}, {0x2C, 0x4C}, {0x2C, 0x4C}, {0x2C, 0x4C}, {0x2C, 0x4C}, {0x2C, 0x4C}, {0x2C, 0x4C},
{0x2C, 0x4C}, {0x2C, 0x4C}, {0x2C, 0x4C}, {0x2C, 0x4C}, {0x30, 0x54}, {0x30, 0x54}, {0x30, 0x54}, {0x30, 0x54},
{0x30, 0x54}, {0x30, 0x54}, {0x30, 0x54}, {0x30, 0x54}, {0x30, 0x54}, {0x30, 0x54}, {0x40, 0x5C}, {0x40, 0x5C},
{0x40, 0x5C}, {0x40, 0x5C}, {0x40, 0x5C}, {0x40, 0x5C}, {0x40, 0x5C}, {0x40, 0x5C}, {0x40, 0x5C}, {0x40, 0x5C},
{0x50, 0x60}, {0x50, 0x60}, {0x50, 0x60}, {0x50, 0x60}, {0x50, 0x60}, {0x70, 0x68}, {0x70, 0x68}, {0x70, 0x68},
{0x70, 0x68}, {0x70, 0x68}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C},
{0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C},
{0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}, {0x88, 0x6C}
};

#if 0
uint16 prom_code_addr;
uint16 ap_addr;
uint16 laser_lut_addr;
uint16 apd_lut_addr;

void get_lut_addr(void)
{
    uint16 regData1, regData2, len;

    regData2 = getReg(EUROPA_REGISTER_PATCH_ADDR);
    busy_loop(0xfff);
    regData1 = getReg(EUROPA_REGISTER_PATCH_ADDR + 1);
    len = (regData1 << 8) | regData2; //Reg Patch length
    if((len %16) != 0) //alignment to 16 bytes
    {
        len = len + (16 - (len %16));
    }
    prom_code_addr = EUROPA_REGISTER_PATCH_ADDR + len;

    regData2 = getReg(prom_code_addr);
    busy_loop(0xfff);
    regData1 = getReg(prom_code_addr + 1);
    len = (regData1 << 8) | regData2; //PROM code length
    ap_addr = prom_code_addr + len;

    laser_lut_addr = ap_addr + 16;

    regData2 = getReg(ap_addr);
    regData1 = getReg(ap_addr + 1);
    len = (regData1 << 8) | regData2; //laser lut length
    ap_addr = ap_addr + len;

    apd_lut_addr = ap_addr + 16;

#if 0
//#ifdef DBG_DDMIA0
    setReg(0xA4, regData2);
    setReg(0xA5, regData1);

    regData2 = apd_lut_addr >>8;
    regData1 = apd_lut_addr & 0xFF;
    setReg(0xB4, regData2);
    setReg(0xB5, regData1);
#endif

}
#endif

#if 0
void update_laser_lut(apc_loop_mode_t mode)
{
    uint16 tmp1,tmp2;
    uint16 temp_sys, bias, modulation;
    uint8 *laser_lut_table;
    uint16 tmp;

    if (flash_update == 1)
        laser_lut_table = &flash_data[1024];
    else
        laser_lut_table = &local_laser_lut_table[0];

    /* ADD APCDIG_BIAS/MOD by 16LSB */
#if 0
    //get temperature
    drv_getAsicRegBits(EUROPA_REG_R2_ADDR, EUROPA_REG_R2_REG_R2_MASK, &tmp1);
    drv_getAsicRegBit(EUROPA_REG_R3_ADDR, 7, &tmp2);
    temp_sys = tmp1 << 1 | tmp2; //0k
    if (temp_sys<233)
    {
        temp_sys = 233;
    }
    if (temp_sys>383)
    {
        temp_sys = 383;
    }
#endif

    temp_sys = get_temperature();

#ifdef DBG_EUROPA
    printk("%s(): temperature = %d",__FUNCTION__, temp_sys);
#endif

    //tmp1 = getReg(laser_lut_addr + (temp_sys - 233)*2);
    //tmp2 = getReg(laser_lut_addr + (temp_sys - 233)*2 + 1);
    //bias = tmp1;
    //modulation = tmp2;
    //bias = laser_lut_table[temp_sys - 233][0];
    //modulation = laser_lut_table[temp_sys - 233][1];

    tmp = (temp_sys - 233)*2;
    bias = laser_lut_table[tmp];
    modulation = laser_lut_table[tmp+1];

    if (APC_DCL_MODE != mode)
    {
        if (APC_DOL_MODE == mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, bias);
            drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modulation);
        }
        else if (APC_SCL_BIAS_MODE == mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modulation);
        }
        if (APC_SCL_MOD_MODE == mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, bias);
        }
        /* Load BIAS/MOD initial code, Write REG_W70[7] to 1 and than write to 0 */
        drv_setAsicRegBit(EUROPA_REG_W70_ADDR, 0xFF, 0xBF);
        busy_loop(0x500);
        drv_setAsicRegBit(EUROPA_REG_W70_ADDR, 0xFF, 0x3F);
        busy_loop(0x500);
     }

    printk(KERN_EMERG "update_laser_lut: temperature = %d, Bias = 0x%x, Modulation = 0x%x\n", temp_sys, bias, modulation);

}

void update_APD_lut(void)
{
    uint16 status, status2, temperature;
    uint16 regData1;
    uint8 *apd_lut_table;

    if (flash_update == 1)
        apd_lut_table = &flash_data[768];
    else
        apd_lut_table = &local_apd_lut_table[0];

#if 0
    /* get current temperature */
    drv_getAsicRegBits(EUROPA_REG_R2_ADDR, EUROPA_REG_R2_REG_R2_MASK, &status);
    drv_getAsicRegBit(EUROPA_REG_R3_ADDR, 7, &status2);
    temperature = status << 1 | status2;

    if (temperature<233)
    {
        temperature = 233;
    }
    if (temperature>383)
    {
        temperature = 383;
    }
#endif

    temperature = get_temperature();

#ifdef DBG_EUROPA
    printk("%s(): temperature = %d",__FUNCTION__, temperature);
#endif

    //New Format only one byte.
    //regData1 = getReg(apd_lut_addr +  (temperature - 233)); //high byte(8bits) DAC[8:1]
    regData1 = apd_lut_table[temperature-233];

    drv_setAsicRegBits(EUROPA_REG_DAC_HB_ADDR, EUROPA_REG_DAC_HB_DIN_L_8_1_MASK, regData1);
    //For Archy's Test
    //drv_setAsicRegBits(EUROPA_REG_DAC_HB_ADDR, EUROPA_REG_DAC_HB_DIN_L_8_1_MASK, 0xA4);

    printk(KERN_EMERG "update_APD_lut: temperature = %d, REG_DAC_HB = 0x%x\n", temperature, regData1);

}

void update_lookup_table(void)
{
    uint16 temperature, bias, modulation, apd;
    uint8 *apd_lut_table;
    uint8 *laser_lut_table;
    uint16 tmp;

    if (flash_update == 1)
    {
        apd_lut_table = &flash_data[768];
        laser_lut_table = &flash_data[1024];
    }
    else
    {
        apd_lut_table = &local_apd_lut_table[0];
        laser_lut_table = &local_laser_lut_table[0];
    }

    temperature = get_temperature();

    // bias = laser_lut_table[temperature - 233][0];
    // modulation = laser_lut_table[temperature - 233][1];
    tmp=(temperature - 233)*2;
    bias = laser_lut_table[tmp];
    modulation = laser_lut_table[tmp+1];

    if (APC_DCL_MODE != loop_mode)
    {
        if (APC_DOL_MODE == loop_mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, bias);
            drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modulation);
        }
        else if (APC_SCL_BIAS_MODE == loop_mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modulation);
        }
        if (APC_SCL_MOD_MODE == loop_mode)
        {
            drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, bias);
        }
        /* Load BIAS/MOD initial code, Write REG_W70[7] to 1 and than write to 0 */
        drv_setAsicRegBit(EUROPA_REG_W70_ADDR, 0xFF, 0xBF);
        busy_loop(0x500);
        drv_setAsicRegBit(EUROPA_REG_W70_ADDR, 0xFF, 0x3F);
        busy_loop(0x500);
     }

    apd = apd_lut_table[temperature-233];

    drv_setAsicRegBits(EUROPA_REG_DAC_HB_ADDR, EUROPA_REG_DAC_HB_DIN_L_8_1_MASK, apd);
    //For Archy's Test
    //drv_setAsicRegBits(EUROPA_REG_DAC_HB_ADDR, EUROPA_REG_DAC_HB_DIN_L_8_1_MASK, 0xA4);

    printk(KERN_EMERG "update_APD_lut: temperature = %d,  Bias = 0x%x, Modulation = 0x%x, REG_DAC_HB = 0x%x\n", temperature, bias, modulation, apd);

}

void set_loop_mode(uint8 mode)
{
    loop_mode = mode;
}

void get_loop_mode(uint8 *pMode)
{
   *pMode = loop_mode;
}


void reset_8051(void)
{
    drv_setAsicRegBit(EUROPA_REG_RESET_ADDR, EUROPA_REG_RESET_DW8051_RESET_OFFSET,1);
}

void powerOnStatusCheck(void)
{

   uint16 status;
   uint16 cnt=0;

#if 0 //Load parameter from Apollo
    /*A5/131[1:0]=би11"  and  A5/131[7:5]=110"  */
    while(1)
    {
        drv_getAsicRegBits(EUROPA_REG_STATUS_2_ADDR,0xFF,&status);
        if(((EUROPA_REG_STATUS_2_PATCH_REG_STATUS_MASK&status)!=0)&&((EUROPA_REG_STATUS_2_IROM_DOWNLOAD_STATUS_MASK&status)!=0)&&
		((EUROPA_REG_STATUS_2_TEMP_VALID_MASK&status)!=0) && ((EUROPA_REG_STATUS_2_LVCMP_TX_VALID_MASK&status)!=0) &&
		((EUROPA_REG_STATUS_2_DEBUG_MODE_STATUS_MASK&status)==0) )
            break;
    }
#endif

    /*A5/131[7:5]=110"  for BOSA on board*/
    while(1)
    {
        drv_getAsicRegBits(EUROPA_REG_STATUS_2_ADDR,0xFF,&status);
        if(((EUROPA_REG_STATUS_2_TEMP_VALID_MASK&status)!=0) && ((EUROPA_REG_STATUS_2_LVCMP_TX_VALID_MASK&status)!=0) &&
		((EUROPA_REG_STATUS_2_DEBUG_MODE_STATUS_MASK&status)==0) )
            break;
    }


    /*R calibration ready(A5_R1[8]=1)*/
    while(1)
    {
        drv_getAsicRegBits(EUROPA_REG_R1_ADDR, 0xff, &status);
        if((status & 0x80) == 0x80)
        {
            break;
        }
        cnt++;
        if (cnt>=20000)
        {
             //SW user define fault!! useEUROPA_REG_SW_FAULT_STATUS1_ADDR bit 2
             printk(KERN_EMERG "R calibration over 20000 times!!!!\n");
             drv_setAsicRegBit(EUROPA_REG_SW_FAULT_STATUS1_ADDR, 2,1);
            break;
         }

    }

}
#endif

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

#if 0
uint32 get_voltage_v0(void)
{
    uint32 volt;
    laser_drv_functionCtrl(EUROPA_LASER_FUN_BOOSTER,DISABLE);
    mdelay(2);
    volt =(uint32) cal_rssi_voltage(loopcnt);
    return volt;
}

void rx_enable(rx_enable_fun_t func)
{
    switch(func)
    {
        case RX_ENABLE_BOOSTER:
            /* Enable Booster: set REG_W4<4> = 1 */
            drv_setAsicRegBit(EUROPA_REG_W4_ADDR, EUROPA_REG_W4_EN_L_OFFSET, 1);
            break;
        case RX_ENABLE_RX:
            /* Enable RX: set REG_W41<4> = 0 */
            drv_setAsicRegBits(EUROPA_REG_W41_ADDR, 0xFF, 0x08);
            udelay(2);
            drv_setAsicRegBits(EUROPA_REG_W41_ADDR, 0xFF, 0x28);
            break;
        case RX_ENABLE_RSSI:
            /*Enable RSSI*/
            drv_setAsicRegBits(EUROPA_REG_W40_ADDR, 0xFF, 0xFB);
            drv_setAsicRegBits(EUROPA_REG_W39_ADDR, EUROPA_REG_W39_REG_W39_MASK, 0);
            drv_setAsicRegBits(EUROPA_REG_W49_ADDR, 0xFF, 0xAC);
            drv_setAsicRegBits(EUROPA_REG_W38_ADDR, 0xFF, 0x88);
            drv_setAsicRegBits(EUROPA_REG_W75_ADDR, 0xFF, 0x88);
            break;
        case RX_ENABLE_DAC:
            /*Enable DAC*/
            drv_setAsicRegBits(EUROPA_REG_W36_ADDR, 0xFF, 0xBC);
            drv_setAsicRegBits(EUROPA_REG_W35_ADDR, EUROPA_REG_W35_REG_W35_MASK, 0x8);
        default:
            break;
    }

}

void laser_drv_kickWatchdog(void)
{
    getReg(EUROPA_REG_WDOG_VALUE_ADDR);
}
#endif

void laser_drv_powerOnSequence_1(void)
{
    uint8 *ptr_data;
    uint32 status;

    //powerOnStatusCheck();

    rtl8290_powerOnStatus_get(&status);
    if (status !=0)
    {
        printk(KERN_EMERG "rtk_europa_powerOnStatus_get error!!!!!!!status = 0x%x \n", status);
        return;
    }

    //update_APD_lut();
    //20150527: move to the place after TX/RX init.
    //voltage_v0 = get_voltage_v0();
    rtl8290_rssiV0_get(100, &europa_param.rssi_v0);

    printk(KERN_EMERG "SDADC RSSI V0 = %d!!!!\n", europa_param.rssi_v0);

    /* APC Enable flow*/
    //apc_enable_flow(APC_EN_MCU_READY);
    //apc_enable_flow(APC_EN_CHECK_READY);
    //apc_enable_flow(APC_EN_TX_SD_THRESHOLD);
    //apc_enable_flow(APC_EN_APC_BIAS_POWER_ON);
   // apc_enable_flow(APC_EN_APC_POWER_SET);
    //apc_enable_flow(APC_EN_APC_DIGITAL_POWER_ON);
    //apc_enable_flow(APC_EN_R_PROCESS_DETECT);
    //apc_enable_flow(APC_EN_TX_RPD_RCODE_OUT);
    //apc_enable_flow(APC_EN_OFFSET_CALIBR);
    //apc_enable_flow(APC_EN_MEASURE_MPD0);
    //apc_enable_flow(APC_EN_WAIT_CALIBR);
    //apc_enable_flow(APC_EN_MOVE_CALIBR_RESULT);

     rtl8290_apcEnableFlow_set(LDD_APC_MCU_READY, loopmode);
     rtl8290_apcEnableFlow_set(LDD_APC_CHECK_READY, loopmode);
     rtl8290_apcEnableFlow_set(LDD_APC_TX_SD_THRESHOLD, loopmode);
     rtl8290_apcEnableFlow_set(LDD_APC_BIAS_POWER_ON, loopmode);
     rtl8290_apcEnableFlow_set(LDD_APC_POWER_SET, loopmode);
    // rtl8290_apcEnableFlow_set(LDD_APC_DIGITAL_POWER_ON, loop_mode);
     rtl8290_apcEnableFlow_set(LDD_APC_R_PROCESS_DETECT, loopmode);
    /*Richard suggest to move after R Process Detect*/
     rtl8290_apcEnableFlow_set(LDD_APC_DIGITAL_POWER_ON, loopmode);
     rtl8290_apcEnableFlow_set(LDD_APC_TX_RPD_RCODE_OUT, loopmode);
     rtl8290_apcEnableFlow_set(LDD_APC_OFFSET_CALIBR, loopmode);

     europa_param.mpd0 = 0;
     rtl8290_mpd0_get(10, &europa_param.mpd0);

    if (europa_param.mpd0 == 0)
    {
        printk(KERN_EMERG "\nMPD0 Error in APC Enable flow!!!!!!!\n");
    }

    // rtl8290_apcEnableFlow_set(LDD_APC_MEASURE_MPD0, loop_mode);
     rtl8290_apcEnableFlow_set(LDD_APC_WAIT_CALIBR, loopmode);
     rtl8290_apcEnableFlow_set(LDD_APC_MOVE_CALIBR_RESULT, loopmode);


    /* Tx Enable flow */
    //tx_enable_flow(TX_EN_BIAS_MOD_PROTECT);
    //tx_enable_flow(TX_EN_ENABLE_TX);
    //tx_enable_flow(TX_EN_BIAS_MOD_LUT);
    //tx_enable_flow(TX_EN_P0_P1_PAV);
    //tx_enable_flow(TX_EN_LOOP_MODE);
    //tx_enable_flow(TX_EN_BURST_MODE_CTRL);
    //tx_enable_flow(TX_EN_WAIT_500MS);
    //tx_enable_flow(TX_EN_SET_W74);

    /* Tx Enable flow */
    //udelay(100);
    //ptr_data = init_data + EUROPA_LASER_LUT_ADDR;
    ptr_data = &flash_data[1024];
    rtl8290_txEnableFlow_set(LDD_TX_BIAS_MOD_PROTECT, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_ENABLE_TX, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_BIAS_MOD_LUT, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_LOOP_MODE, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_BURST_MODE_CTRL, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_WAIT_500MS, loopmode, ptr_data);

    /* Rx Enable flow */
    //diag_util_printf("Rx Enable flow !!!!!!!\n");
    //osal_time_mdelay(100);
    rtl8290_rxEnableFlow_set(LDD_RX_THRESHOLD);
    /* Enable Booster: set REG_W4<4> = 1 */
    rtl8290_rxEnableFlow_set(LDD_RX_BOOSTER);
    /* Enable RX: set REG_W41<4> = 0 */
    rtl8290_rxEnableFlow_set(LDD_RX_RX);
    /*Enable RSSI*/
    rtl8290_rxEnableFlow_set(LDD_RX_RSSI);
    /*Enable DAC*/
    rtl8290_rxEnableFlow_set(LDD_RX_DAC);

    /* Update APD lookup table */
    //diag_util_printf("Update APD lookup table!!!!!!!\n");
   // osal_time_mdelay(100);
    //ptr_data = init_data + EUROPA_APD_LUT_ADDR;
    ptr_data = &flash_data[768];
    rtl8290_apdLut_set(ptr_data);

    //diag_util_printf("Enable TX SD!!!!!!!\n");
    //osal_time_mdelay(100);
    //ptr_data = init_data + EUROPA_LASER_LUT_ADDR;
    ptr_data = &flash_data[1024];
    rtl8290_txEnableFlow_set(LDD_TX_ENABLE_TX_SD, loopmode, ptr_data);
    rtl8290_txEnableFlow_set(LDD_TX_FAULT_ENABLE, loopmode, ptr_data);

}

void laser_drv_powerOnSequence_2(void)
{
    uint16 status, regData1, regData2, regData3;

    /* Change SCL or DCL */
    /* Archy will set a register for SW to read and set to EUROPA_REG_W69_APCDIG_LOOPMODE_MASK */
    //drv_setAsicRegBits(EUROPA_REG_W69_ADDR, EUROPA_REG_W69_APCDIG_LOOPMODE_MASK, 1);//00: Dual Open Loop; 01: Dual Closed Loop; 10: Single Closed Loop for BIASDAC; 11: Single Closed Loop for MODDAC

    /* set threshold value */
    /* 1. REG_TEMP_TH   -- Get from DDMI A2 byte 0/1  alarm high */
    set_temperature_threshold();
    /* 2. REG_IMPD_TH   -- Read address (IMPD translate function 1 ) */
    //????????????????

    /* 3. REG_VOL_TH     -- Get from DDMI A2 byte 8/9 (voltage translate function 1) */
    //Use reference LDO now, next version will change to reference bandgap voltage

    volt_trans_func_1(internal);

    //tx_enable_flow(TX_EN_FAULT_ENABLE);

    /* Fault enable, turn on all bits in REG_W53 */
    //drv_setAsicRegBits(EUROPA_REG_W53_ADDR, 0xff, 0xff);
    //drv_setAsicRegBits(EUROPA_REG_FAULT_CTL_ADDR, 0xff, 0x2);
    //drv_setAsicRegBits(EUROPA_REG_FAULT_INHIBIT_ADDR, 0xff, 0);

    /* Turn on Interrupt mask */
    //laser_drv_intrEnable(0x20, 0x5) ;

    /* wait no fault condition */
    drv_getAsicRegBits(EUROPA_REG_FAULT_STATUS_ADDR, 0xff, &status);
    if(status != 0)
    {
        printk(KERN_EMERG "FAULT STATUS in Startup Procedure!!!!\n");

        laser_drv_fault_status();

        drv_getAsicRegBits(EUROPA_REG_STATUS_2_ADDR,0xFF,&status);
        if((EUROPA_REG_STATUS_2_FAULT_STATUS_MASK&status)!=0 )
        {
             /* disable TX(REG_CONTROL2.TX_POW_CTL =0) */
            laser_drv_functionCtrl(EUROPA_LASER_FUN_TX,DISABLE);
            laser_drv_functionCtrl(EUROPA_LASER_FUN_VDD_LDX,DISABLE);
            laser_drv_functionCtrl(EUROPA_LASER_FUN_BOOSTER,DISABLE);
        }

    //laser_drv_intrDisable();

#if 0
        while(1)
        {
            busy_loop(0x800);
            status = getReg(EUROPA_REG_INT_STATUS1_ADDR);
            if(status & EUROPA_REG_INT_STATUS1_FAULT_RELEASE_INT_STATUS_MASK)
            {
                /*handle fault release interrupt*/
                laser_drv_faultReleaseIntrHandle();

            }
        }
#endif
    }
    Check_C0_C1();

}

void laser_drv_powerOnSequence_3(void)
{

    //Check_C0_C1();

    /*20150312 */
    //setReg(EUROPA_REG_RX_TH_ADDR, 0x10);
    //setReg(EUROPA_REG_RX_DE_TH_ADDR, 0x40);

    /* Enable Booster: set REG_W4<4> = 1 */
    //rx_enable(RX_ENABLE_BOOSTER);
    /* Enable RX: set REG_W41<4> = 0 */
    //rx_enable(RX_ENABLE_RX);
    /*Enable RSSI*/
    //rx_enable(RX_ENABLE_RSSI);
    /*Enable DAC*/
    //rx_enable(RX_ENABLE_DAC);

    /*20150128 Change to Here*/
    //update_APD_lut();

#if 1 //temp remove
    /* Update DDMI */
    update_ddmi();
#endif

    /* Set DDMI ready bit */
    drv_setAsicRegBit(EUROPA_REG_M0_ADDR, EUROPA_REG_M0_MCU_DDMI_READY_STATUS_OFFSET, 1);
    drv_setAsicRegBit(EUROPA_REG_M0_ADDR, EUROPA_REG_M0_MCU_READY_STATUS_OFFSET, 1);

    /*enable TS_SD/SD*/
    //drv_setAsicRegBits(EUROPA_REG_CONTROL2_ADDR, 0xFF, 0x4D);

    /*enable watch dog*/
    //drv_setAsicRegBit(EUROPA_REG_CONTROL1_ADDR,EUROPA_REG_CONTROL1_WDOG_ENABLE_OFFSET,1);
    /*disable watch dog for BOSA on board*/
    drv_setAsicRegBit(EUROPA_REG_CONTROL1_ADDR,EUROPA_REG_CONTROL1_WDOG_ENABLE_OFFSET,0);

}


void europa_init(void)
{
    uint32 rssi_voltage;
    uint32 start;
    uint32 total_time;

    printk("\r\nReset IC\n");
    drv_setAsicRegBit(EUROPA_REG_RESET_ADDR, EUROPA_REG_RESET_TOTAL_CHIP_RESET_OFFSET, 1);

    busy_loop(0x500);

    parameter_init(loopmode);

    /* power*/
    printk(KERN_EMERG "laser_drv_powerOnSequence_1 \n");
    laser_drv_powerOnSequence_1();
    printk(KERN_EMERG "laser_drv_powerOnSequence_2 \n");
    laser_drv_powerOnSequence_2();
    printk(KERN_EMERG "laser_drv_powerOnSequence_3 \n");
    laser_drv_powerOnSequence_3();

   //start = jiffies;
   //rssi_voltage = (uint32) cal_rssi_voltage(loopcnt);
   //total_time = jiffies - start ;
   //printk("\r\nSDADC RSSI Voltage Calculation loop = %d, time duration = %u, USER_HZ = %d, RSSI Voltage = %d!!!!!\n", loopcnt, total_time, USER_HZ, rssi_voltage);

}


#endif

