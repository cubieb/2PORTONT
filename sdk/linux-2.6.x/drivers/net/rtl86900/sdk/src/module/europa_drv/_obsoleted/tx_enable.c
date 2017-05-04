#ifdef CONFIG_EUROPA_MODULE
#include <linux/kernel.h>
#include <module/europa/europa.h>
#include <module/europa/common.h>
#include <module/europa/utility.h>
#include <module/europa/tx_enable.h>
#include <module/europa/laser_drv.h>

#include <module/europa/rtl8290_api.h>


extern uint16 laser_lut_addr;
extern uint8 loop_mode;
extern uint16 apc_dcl_P0, apc_dcl_P1, apc_dcl_Pavg;

void loop_mode_set(apc_loop_mode_t mode)
{
    //uint32 regData1, regData2;

#if 0
    if(mode >=APC_SCL_BIAS_MODE)
    {
        drv_setAsicRegBits(EUROPA_REG_W61_ADDR, EUROPA_REG_W61_APCDIG_CODE_DACPAVG_MASK, 50);
    }
    else 
    {
        //APC_DCL_PAVG_target=((APC_DCL_P0_target*8)+(APC_DCL_P1_target*25))/32 // not used, reference only
        drv_getAsicRegBits(EUROPA_REG_W58_ADDR, EUROPA_REG_W58_APCDIG_P0_1_MASK, &regData1); 
        drv_getAsicRegBits(EUROPA_REG_W59_ADDR, EUROPA_REG_W59_APCDIG_P1_1_MASK, &regData2);         
        drv_setAsicRegBits(EUROPA_REG_W61_ADDR, EUROPA_REG_W61_APCDIG_CODE_DACPAVG_MASK, ((regData1*8)+(regData1*25))/32);
    }
	
    drv_getAsicRegBits(EUROPA_REG_W61_ADDR, EUROPA_REG_W61_APCDIG_CODE_DACPAVG_MASK, &regData1);	
#ifdef UART_DBG		
    printf(" Pavg target value= 0x%X   \n",regData1);
#endif 
    //----set dual settle time cnt0
    drv_setAsicRegBits(EUROPA_REG_W71_ADDR, EUROPA_REG_W71_APCDIG_DUAL_SETTLE_CNT0_MASK, 14);
    //----set dual settle time cnt1
    drv_setAsicRegBits(EUROPA_REG_W72_ADDR, EUROPA_REG_W72_APCDIG_DUAL_SETTLE_CNT1_MASK, 14);
#endif

    // set W74 for different APC mode
    if(mode >=APC_SCL_BIAS_MODE)
    {
        drv_setAsicRegBits(EUROPA_REG_W74_ADDR, 0xFF, 0x09);
	 drv_setAsicRegBits(EUROPA_REG_W57_ADDR, 0xFF, 0xF8);			
    }
    else 
    {
         drv_setAsicRegBits(EUROPA_REG_W74_ADDR, 0xFF, 0x19);
    }

    busy_loop(0x550);
	
    //---- set SCL/DCL mode 
    drv_setAsicRegBits(EUROPA_REG_W69_ADDR, EUROPA_REG_W69_APCDIG_LOOPMODE_MASK, mode);
	
     // set final W77 value  
    //drv_setAsicRegBits(EUROPA_REG_W77_ADDR, EUROPA_REG_W77_APCDIG_BACKUP_MASK, 0x1);

    //busy_loop(0x550);

    /*if(mode >=APC_SCL_BIAS_MODE)
    {
        drv_setAsicRegBits(EUROPA_REG_W74_ADDR, 0xFF, 0x09);
    }
    else 
    {
         drv_setAsicRegBits(EUROPA_REG_W74_ADDR, 0xFF, 0x19);
    }*/

}

void tx_enable_flow(tx_enable_fun_t func)
{
    //uint32 tmp1,tmp2;
    //uint32 temp_sys, bias, modulation;
    //uint16 regData;
    //uint16 apc_dcl_P0, apc_dcl_P1, apc_dcl_Pavg;
	

    switch(func)
    {
        case TX_EN_BIAS_MOD_PROTECT:
            //----Bias/Mod DAC  max protect
            drv_setAsicRegBits(EUROPA_REG_W77_ADDR, 0xFF, 0xa5);	  
            break;    
			
        case TX_EN_ENABLE_TX:
            //drv_setAsicRegBits(EUROPA_REG_CONTROL2_ADDR, 0xFF, 0x4D);
            drv_setAsicRegBits(EUROPA_REG_CONTROL2_ADDR, 0xFF, 0x8D);
            break;    
			
        case TX_EN_BIAS_MOD_LUT:
            /* ADD APCDIG_BIAS/MOD by 16LSB */
           update_laser_lut(APC_DOL_MODE);			
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

#ifdef DBG_DDMIA0	
    setReg(0x72,(temp_sys&0xFF00)>>8);
    setReg(0x73,(temp_sys&0xFF));
#endif
			
            tmp1 = getReg(laser_lut_addr + (temp_sys - 233)*2);
            tmp2 = getReg(laser_lut_addr + (temp_sys - 233)*2 + 1);
            bias = tmp1;
            modulation = tmp2; 
            drv_setAsicRegBits(EUROPA_REG_W54_ADDR, EUROPA_REG_W54_APCDIG_BIAS_1_MASK, bias);
            drv_setAsicRegBits(EUROPA_REG_W55_ADDR, EUROPA_REG_W55_APCDIG_MOD_1_MASK, modulation);
            /* Load BIAS/MOD initial code, Write REG_W70[7] to 1 and than write to 0 */
            drv_setAsicRegBit(EUROPA_REG_W70_ADDR, EUROPA_REG_W70_APCDIG_CODE_LOADIN_OFFSET, 1);
            busy_loop(0x500);
            drv_setAsicRegBit(EUROPA_REG_W70_ADDR, EUROPA_REG_W70_APCDIG_CODE_LOADIN_OFFSET, 0);
            busy_loop(0x500);	
#endif			
            break;   
			
        case TX_EN_P0_P1_PAV:
            //apc_dcl_P0 =  getReg(laser_lut_addr - 16 + 4);
            drv_setAsicRegBits(EUROPA_REG_W58_ADDR, EUROPA_REG_W58_APCDIG_P0_1_MASK, apc_dcl_P0); 
            //apc_dcl_P1 = getReg(laser_lut_addr - 16 + 5);
            drv_setAsicRegBits(EUROPA_REG_W59_ADDR, EUROPA_REG_W59_APCDIG_P1_1_MASK, apc_dcl_P1);  
            //apc_dcl_Pavg= getReg(laser_lut_addr - 16 + 6);
            drv_setAsicRegBits(EUROPA_REG_W61_ADDR, EUROPA_REG_W61_APCDIG_CODE_DACPAVG_MASK, apc_dcl_Pavg);     
            break;
			
        case TX_EN_LOOP_MODE:
            loop_mode_set(loop_mode);        
            break;   
			
        case TX_EN_BURST_MODE_CTRL:
            /* Burst control change to be controlled by BENP/BENN --> SW should write REG_W48[6] = 0 */
            drv_setAsicRegBits(EUROPA_REG_W48_ADDR, 0xFF, 0);    
            drv_setAsicRegBits(EUROPA_REG_W80_ADDR, EUROPA_REG_W80_APCDIG_BACKUP2_MASK, 0x7);				
            break;
			
        case TX_EN_WAIT_500MS:		
            // set final W77 value  
            busy_loop(0x2000);			    
            drv_setAsicRegBits(EUROPA_REG_W77_ADDR, EUROPA_REG_W77_APCDIG_BACKUP_MASK, 0x1);	
            break;

        case TX_EN_SET_W74:
            if(loop_mode >=APC_SCL_BIAS_MODE)
            {
                drv_setAsicRegBits(EUROPA_REG_W74_ADDR, 0xFF, 0x09);
            }
            else 
            {
                 drv_setAsicRegBits(EUROPA_REG_W74_ADDR, 0xFF, 0x19);
            }
            break;			
        case TX_EN_FAULT_ENABLE:		
            /* Fault enable, turn on all bits in REG_W53 */
            drv_setAsicRegBits(EUROPA_REG_W53_ADDR, 0xff, 0xff);
            drv_setAsicRegBits(EUROPA_REG_FAULT_CTL_ADDR, 0xff, 0x2);
            drv_setAsicRegBits(EUROPA_REG_FAULT_INHIBIT_ADDR, 0xff, 0);
            break;
	 
        default:
            break;        
    }
		
}
#endif

