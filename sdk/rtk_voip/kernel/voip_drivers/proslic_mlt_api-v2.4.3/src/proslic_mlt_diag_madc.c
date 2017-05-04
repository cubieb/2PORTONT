/*
** Copyright (c) 2010-2011 by Silicon Laboratories
**
** $Id: proslic_mlt_diag_madc.c 4714 2015-02-05 18:16:07Z nizajerk $
**
*/
/*! \file proslic_mlt_diag_madc.c
**  \brief ProSLIC MLT diag and madc utility functions implementation file
**
**  This is the implementation file for the ProSLIC MLT diag and madc utility functions
**
** \author Silicon Laboratories, Inc (cdp)
**
** \attention
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "proslic.h"
#define SI_USE_IFACE_MACROS
#include "proslic_mlt.h"
#include "proslic_mlt_diag_madc.h"

#include "mlt_comm_regs.h"
#include "mlt17x_b_regs.h"
#include "mlt17x_c_regs.h"
#include "mlt26x_c_regs.h"



/*
** Function:   getRAMScale 
*/
int32 getRAMScale(uInt16 addr,uInt8 ext_v_opt)
{
int32 scale;

    switch(addr)
    {
        case MLT_COMM_RAM_MADC_ILOOP:
        case MLT_COMM_RAM_MADC_ITIP:
        case MLT_COMM_RAM_MADC_IRING:
        case MLT_COMM_RAM_MADC_ILONG:
            scale = MLT_CONST_MADC_I;        
        break;

        case MLT_COMM_RAM_MADC_VTIPC:
        case MLT_COMM_RAM_MADC_VRINGC:
        case MLT_COMM_RAM_VDIFF_COARSE:
            if(ext_v_opt)
            {
                scale = MLT_CONST_MADC_VCOARSE_EV;
            }
            else
            {
                scale = MLT_CONST_MADC_VCOARSE_SV;
            }
        break;

        case MLT_COMM_RAM_MADC_VBAT:
        case MLT_COMM_RAM_MADC_VLONG:
        case MLT_COMM_RAM_VDIFF_SENSE:
        case MLT_COMM_RAM_VDIFF_FILT:
        case MLT_COMM_RAM_VTIP:
        case MLT_COMM_RAM_VRING:
            scale = MLT_CONST_MADC_V;
        break;

        case MLT_COMM_RAM_MADC_DIAG_RAW:
        case MLT_COMM_RAM_MADC_VTIPC_DIAG_OS:
        case MLT_COMM_RAM_MADC_VRINGC_DIAG_OS:
            if(ext_v_opt)
            {
                scale = MLT_CONST_MADC_HIRES_V_EV;
            }
            else
            {
                scale = MLT_CONST_MADC_HIRES_V_SV;
            }
        break;

        case MLT_COMM_RAM_RINGOF:
            scale = MLT_CONST_AUTO_V_RINGOF;
        break;

        case MLT_COMM_RAM_V_FEED_IN:
            scale = MLT_CONST_AUTO_V_VFEED;
        break;

        default:
            scale = 1;
        break;
    }

    return scale;
}

/*
** Function:   ReadMADCScaled 
*/
int32 ReadMADCScaled(ProSLICMLTType *pProSLICMLT, uInt16 addr, int32 scale)
{
int32 data;
uInt8 ext_v_opt = 0;

    /*
    ** Read 29-bit RAM and sign extend to 32-bit
    */
    data = pProSLICMLT->ReadRAM(pMLT_HW,pMLT_CHAN,addr);
	if (data & 0x10000000L)
		data |= 0xF0000000L;

    /* Determine if extened voltage option applies */
#ifdef MLT_RUNTIME_CONFIG
    if(pProSLICMLT->mlt_config.mlt_ext_v_flag)
    {
        ext_v_opt = 1;
    }
#else
#ifdef MLT_USE_EXTENDED_V_RANGE
    ext_v_opt = 1;
#endif
#endif


    /*
    ** Scale to provided value, or use defualts if scale = 0 
    */
    if(scale == 0) 
        scale = getRAMScale(addr,ext_v_opt);

    data /= scale;

    return data;
}


/*
** Function:   setupCoarseSensors
*/
int setupCoarseSensors(ProSLICMLTType *pProSLICMLT,int32*tipOffs,int32*ringOffs)
{
	uInt8 reg;
	
    /*
    ** Powerup MADC
    */
	reg = pProSLICMLT->ReadReg(pMLT_HW,pMLT_CHAN,MLT_COMM_REG_PDN);
	reg |= 0x80;
	pProSLICMLT->WriteReg(pMLT_HW, pMLT_CHAN,MLT_COMM_REG_PDN,reg);

    /*
    ** Powerup coarse sensors
    */
	pProSLICMLT->WriteRAM(pMLT_HW,pMLT_CHAN,MLT_COMM_RAM_PD_DC_COARSE_SNS,0x200000L);
	pProSLICMLT->WriteReg(pMLT_HW,pMLT_CHAN,MLT_COMM_REG_GPIO,0);

    /* 
    ** Read offsets (stored during madc offset calibration)
    */
    *tipOffs = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_VTIPC_DIAG_OS,0);
    *ringOffs = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_VRINGC_DIAG_OS,0);

#ifdef ENABLE_DEBUG
    if(pProSLICMLT->pProslic->debugMode)
    {
        LOGPRINT("proslic_mlt_diag_madc:  setupCoarseSensors  :  tipOffs  = %d\n",*tipOffs);
        LOGPRINT("proslic_mlt_diag_madc:  setupCoarseSensors  :  ringOffs = %d\n",*ringOffs);
    }
#endif

    return 0; 

}

/*
** Function:  readCoarseSensor
*/
int32 readCoarseSensor(ProSLICMLTType *pProSLICMLT,int32 offset)
{
    int32 data;

    
    data = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_DIAG_RAW, 0);
    data -= MLT_CONST_COARSE_SENSE_VOFFS;
    if(data&0x10000000L)
        data |= 0xF0000000L;
    data -= offset;
    return data;
}

/*
** Function:   diagSet
*/
void diagSet(proslicChanType_ptr pProslic, uInt8 setting)
{
    WriteReg(pProHW,pProslic->channel,MLT_COMM_REG_DIAG1, setting);
}


/*
** Function:   diagSelect
*/
void diagSelect(proslicChanType_ptr pProslic, uInt8 select, int hiresFlag)
{
    if(hiresFlag)
        select |= DIAG_HIRES_EN;

    WriteReg(pProHW,pProslic->channel,MLT_COMM_REG_DIAG2, select);
}

/*
** Funtion:  setupDiagCurrentSource
*/
void setupDiagCurrentSource (proslicChanType_ptr pProslic, int32 iForce)
{
int32 val;
	WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_RINGAMP,0L);
	WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_RINGFR,0L);
	WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_SLOPE_RING,0x1F000000L);
    /* iForce in uA */
    val = iForce * MLT_CONST_AUTO_V_RINGOF;
    val &= 0x1FFFFFFFL;
    WriteRAM(pProHW,pProslic->channel,MLT_COMM_RAM_RINGOF,val);
}



/*
**
*/
void readTerminalValues(ProSLICMLTType *pProSLICMLT,ProSLIC_term_meas_t *term, int index)
{        
    if(index >= MLT_MAX_IV_SAMPLES) 
        return;
    term->term.itip[index] = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_ITIP,0);
    term->term.vtip[index] = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_VTIP,0);
    term->term.iring[index] = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_IRING,0);
    term->term.vring[index] = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_VRING,0);
    term->term.iloop[index] = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_ILOOP,0);
    term->term.vloop[index] = term->term.vring[index] - term->term.vtip[index];
    term->term.ilong[index] = ReadMADCScaled(pProSLICMLT,MLT_COMM_RAM_MADC_ILONG,0);
    term->term.vlong[index] = ReadMADCScaled(pProSLICMLT, MLT_COMM_RAM_MADC_VLONG,0);
}


void printTerminalValues(ProSLIC_iv_t *term, int index)
{
    #ifdef ENABLE_DEBUG
    LOGPRINT ("VTIP %d     =  %d\n", index,term->vtip[index]);
    LOGPRINT ("VRING %d    =  %d\n", index,term->vring[index]);
    LOGPRINT ("VLOOP %d    =  %d\n", index,term->vloop[index]);
    LOGPRINT ("VLONG %d    =  %d\n\n", index,term->vlong[index]);
    LOGPRINT ("ITIP %d     =  %d\n", index,term->itip[index]);
    LOGPRINT ("IRING %d    =  %d\n", index,term->iring[index]);
    LOGPRINT ("ILOOP %d    =  %d\n", index,term->iloop[index]);
    LOGPRINT ("ILONG %d    =  %d\n\n", index,term->ilong[index]);
    #endif
}


void setup300HzBandpass(ProSLICMLTType * pProSLICMLT)
{
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB0_1, 0x10B2A0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB1_1, 0x0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB2_1, 0x1FEF4D60L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA1_1, 0xF3F2BB0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA2_1, 0x185119D0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB0_2, 0x2863A0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB1_2, 0x0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB2_2, 0x1FD79C60L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA1_2, 0xF3F2BB0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA2_2, 0x185119D0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB0_3, 0x2899D0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB1_3, 0x0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB2_3, 0x1FD76630L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA1_3, 0xF3F2BB0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA2_3, 0x185119D0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTAVBW, 0x40000L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTWLN, 0x7D00000L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTAVTH, 0x0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTPKTH, 0x0L);

}
void setup3014HzBandpass(ProSLICMLTType * pProSLICMLT)
{
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB0_1, 0x16CFF0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB1_1, 0x0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB2_1, 0x1FE93010L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA1_1, 0x14CA9450L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA2_1, 0x185119D0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB0_2, 0x2845C0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB1_2, 0x0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB2_2, 0x1FD7BA40L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA1_2, 0x14CA9450L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA2_2, 0x185119D0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB0_3, 0x28A850L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB1_3, 0x0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTB2_3, 0x1FD757B0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA1_3, 0x14ca9450L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTA2_3, 0x185119D0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTAVBW, 0x40000L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTWLN, 0x7D00000L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTAVTH, 0x0L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_TESTPKTH, 0x0L);

}
void setup300HzTestTone(ProSLICMLTType * pProSLICMLT)
{
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_OSC1FREQ, 0x7c70000L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_OSC1AMP, 0x2a000L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_OSC1PHAS, 0x0L);
    pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_OMODE, 2);
    pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_OCON, 1);	/*enable osc1 */
}
void setup3014HzTestTone(ProSLICMLTType * pProSLICMLT)
{
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_OSC1FREQ, 0x1a480000L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_OSC1AMP, 0x376000L);
    pProSLICMLT->WriteRAM(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_RAM_OSC1PHAS, 0x0L);
    pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_OMODE, 2);
    pProSLICMLT->WriteReg(pProSLICMLT->pProHW, pProSLICMLT->pProslic->channel,MLT_COMM_REG_OCON, 1);	/*enable osc1 */
}


