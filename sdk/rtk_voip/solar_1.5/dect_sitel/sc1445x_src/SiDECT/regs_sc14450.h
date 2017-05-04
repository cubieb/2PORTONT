
/*
 *  sc14450-regs.c
 *
 *  Helper macros providing easy acesss to the peripherial control
 *  registers on the SiTel SC14450
 *
 *  Author:     Bennie Affleck
 *  Created:    11th October 2007
 *  Tabs:       4 spaces
 *  Copyright:  MPC Data LTD
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */


#ifndef __SC14450_REGS_H
#define __SC14450_REGS_H


#ifndef __ASSEMBLY__
/* Give us access to memory mapped control registers */
#define __REG(x)	(*((volatile u16 *)x))
#define __REG32(x)	(*((volatile u32 *)x))

/* Register access at BASE + Offset */
#define __REG2(x,y)	(*(volatile u16 *)((u16)&__REG(x) + (y)))
#endif


#define SHIF(a) ((a)&0x0001?0: (a)&0x0002?1: (a)&0x0004?2: (a)&0x0008?3:\
		 (a)&0x0010?4: (a)&0x0020?5: (a)&0x0040?6: (a)&0x0080?7:\
		 (a)&0x0100?8: (a)&0x0200?9: (a)&0x0400?10:(a)&0x0800?11:\
		 (a)&0x1000?12:(a)&0x2000?13:(a)&0x4000?14: 15)

#define SetBits(a,f,d)	(a) = ( (a) & (u16)~(f) ) | ( (d) << SHIF(f) )
#define SetPort(PORT, IO_CONFIG, PID)	(PORT) = (IO_CONFIG) | (PID)



/*
 * Access Bus (I2C under another name!)
 */
#define ACCESS1_IN_OUT_REG		__REG(0xFF4920)	/* ACCESS bus 1 receive/transmit register */
#define ACCESS1_CTRL_REG		__REG(0xFF4922)	/* ACCESS bus 1 Control register */
#define ACCESS1_CLEAR_INT_REG	        __REG(0xFF4924)	/* Clear ACCESS bus 1 interrupt */
#define ACCESS2_IN_OUT_REG		__REG(0xFF4930)	/* ACCESS bus 2 receive/transmit register */
#define ACCESS2_CTRL_REG		__REG(0xFF4932)	/* ACCESS bus 2 Control register */
#define ACCESS2_CLEAR_INT_REG	        __REG(0xFF4934)	/* Clear ACCESS bus 2 interrupt */


/*
 * Analogue to Digital Converters
 */
#define AD_CTRL_REG			__REG(0xFF4960)	/* ADC control register */
#define AD_CTRL1_REG		        __REG(0xFF4962)	/* ADC control register */
#define AD_CLEAR_INT_REG	        __REG(0xFF4964)	/* Clears ADC interrupt */
#define ADC0_REG			__REG(0xFF4966)	/* ADC0 value */
#define ADC1_REG			__REG(0xFF4968)	/* ADC1 value */


/*
 * Battery 
 */
#define BANDGAP_REG			__REG(0xFF4810)	/* Bandgap register */
#define BAT_CTRL_REG		        __REG(0xFF4812)	/* Power management control register */
#define BAT_CTRL2_REG		        __REG(0xFF4814)	/* Power charge control register */
#define BAT_STATUS_REG		        __REG(0xFF4816)	/* Power status register */
#define BAT_SOC_HIGH_REG	        __REG(0xFF4818)	/* Power state of charge counter (high) */
#define BAT_SOC_LOW_REG		        __REG(0xFF481A)	/* Power state of charge counter (low) */


/*
 * Cache
 */
#define CACHE_CTRL_REG		        __REG(0xFF5006)	/* Cache control register */
#define CACHE_LEN0_REG		        __REG(0xFF5008)	/* Cache length register 0 */
#define CACHE_START0_REG	        __REG(0xFF500A)	/* Cache start register 0, can be used for iCache and dCache */
#define CACHE_LEN1_REG		        __REG(0xFF500C)	/* Cache length register 1 */
#define CACHE_START1_REG	        __REG(0xFF500E)	/* Cache start register 1, can be used for iCache anddCache */
#define CACHE_STATUS_REG	        __REG(0xFF5010)	/* Cache status register */


/*
 * CRC Generator
 */
#define CCU_CRC_LOW_REG		        __REG(0xFF6C00)	/* CCU MSB result */
#define CCU_CRC_HIGH_REG	        __REG(0xFF6C02)	/* CCU LSB result */
#define CCU_IN_REG			__REG(0xFF6C04)	/* CCU Input */
#define CCU_MODE_REG		        __REG(0xFF6C06)	/* CCU mode */
#define INT_ACK_CR16_START	        __REG(0xFFFC00)	/* CR16 interrupt acknowledge */
#define INT_ACK_CR16_END	        __REG(0xFFFFFF)	/* End Address?? */


/*
 * Chip Information
 */
#define CHIP_ID1_REG		        __REG(0xFFFBF8)	/* Chip identification register 1 */
#define CHIP_ID2_REG		        __REG(0xFFFBF9)	/* Chip identification register 2 */
#define CHIP_ID3_REG		        __REG(0xFFFBFA)	/* Chip identification register 3 */
#define CHIP_MEM_SIZE_REG	        __REG(0xFFFBFB)	/* Chip memory size register */
#define CHIP_REVISION_REG	        __REG(0xFFFBFC)	/* Chip revision registers (Corresponds to Chip Marking) */
#define CHIP_TEST1_REG		        __REG(0xFFFBFD)	/* Chip test register */
#define CHIP_TEST2_REG		        __REG(0xFFFBFE)	/* Chip test register */


/*
 * Class D Audio Amplifier
 */
#define CLASSD_CTRL_REG			__REG(0xFF5C00)	/* Class D control register */
#define CLASSD_CLEAR_INT_REG	        __REG(0xFF5C02)	/* Class D Clear interrupt register */
#define CLASSD_BUZZER_REG		__REG(0xFF5C04)	/* Class D buzzer register */
#define CLASSD_TEST_REG			__REG(0xFF5C06)	/* Class D test register */
#define CLASSD_NR_REG			__REG(0xFF5C08)	/* Class D noise reduction register */


/*
 * Clock Generator
 */
#define CLK_AMBA_REG			__REG(0xFF4000)	/* HCLK, PCLK, divider and clock gates */
#define CLK_CODEC_DIV_REG		__REG(0xFF4002)	/* Codec divider register */
#define CLK_CODEC_REG			__REG(0xFF4004)	/* Codec clock selection register */
#define CLK_DSP_REG			__REG(0xFF4006)	/* DSP clock selection register */
#define CLK_FREQ_TRIM_REG		__REG(0xFF400A)	/* Xtal frequency trimming register. */
#define CLK_PER_DIV_REG			__REG(0xFF400C)	/* Peripheral divider register for 2,1, 1/2, 1/4 MHz clocks */
#define CLK_PER10_DIV_REG		__REG(0xFF400E)	/* Peripheral divider register for 20, 10 MHz clocks */
#define CLK_PLL1_CTRL_REG		__REG(0xFF4010)	/* PLL1 control register */
#define CLK_PLL1_DIV_REG		__REG(0xFF4012)	/* PLL1 divider register */
#define CLK_PLL2_CTRL_REG		__REG(0xFF4014)	/* PLL2 control register */
#define CLK_PLL2_DIV_REG		__REG(0xFF4016)	/* PLL2 divider register */
#define CLK_XTAL_CTRL_REG		__REG(0xFF4018)	/* Crystal control register */
#define CLK_AUX_REG		        __REG(0xFF401A)	/* Auxiliary clock control register */


#define CLK_AMBA_HCLK_PRE		(1 << 9)	/* HCLK prescaler P3 */
#define CLK_AMBA_MCRAM2_EN		(1 << 8)	/* Micro Code RAM2 Interface */
#define CLK_AMBA_MCRAM1_EN		(1 << 7)	/* Micro Code RAM1 Interface */
#define CLK_AMBA_SRAM2_EN		(1 << 6)	/* Shared RAM2, ROM2 and Gen2DSP2 Interface */
#define CLK_AMBA_SRAM1_EN		(1 << 5)	/* Shared RAM1, ROM1 and Gen2DSP1 Interface */
#define CLK_AMBA_PCLK_MASK		(0x3 << 3)	/* APB interface clock divider mask */
#define CLK_AMBA_PCLK_DIV_4		(0 << 3)	/* APB clock = HCLK / 4 */
#define CLK_AMBA_PCLK_DIV_2		(2 << 3)	/* APB clock = HCLK / 2 */
#define CLK_AMBA_PCLK_DIV_1		(1 << 3)	/* APB clock = HCLK / 1 */
#define CLK_AMBA_HCLK_MASK		(0x07)		/* AHB interface and CR16C clock mask */
#define CLK_AMBA_HCLK_DIV(x)	        (x & 0x7)	/* CR16C and AHB clock = XTAL / x (x=0 => /8) */

#define CLK_AUX_USB_TST_CLK		(1 << 9)	/* USB clock src: 0=PLL 1=HCLK (test only) */
#define CLK_AUX_XDIV			(1 << 8)	/* 1=divide clocks by 16 0=divide by 1 */
#define CLK_AUX_OWI_SEL			(1 << 7)	/* Select XTAL(0) or PLL(1) for JTAG OWI clock */
#define CLK_AUX_OWI_DIV_MASK	        (0x3 << 5)	/* JTAG One wire interface clock divider (mask) */
#define CLK_AUX_OWI_DIV_8		(0 << 5)	/* clock = source / 8 */
#define CLK_AUX_OWI_DIV_1		(1 << 5)	/* clock = source / 1 */
#define CLK_AUX_OWI_DIV_2		(2 << 5)	/* clock = source / 2 */
#define CLK_AUX_OWI_DIV_4		(3 << 5)	/* clock = source / 4 */
#define CLK_AUX_BXTAL_EN_MASK	        (0x3 << 3)	/* Select BXTAL clock (mask) */
#define CLK_AUX_BXTAL_EN_OFF	        (0x0 << 3)	/* BXTAL clock disabled */
#define CLK_AUX_BXTAL_EN_XTAL	        (0x2 << 3)	/* BXTAL clock is XTAL / BXTAL_DIV [2-0] */
#define CLK_AUX_BXTAL_EN_PLL	        (0x3 << 3)	/* BXTAL clock is (PLL1/2) / BXTAL_DIV[2-0] */
#define CLK_AUX_BXTAL_DIV_MASK	        (0x7)		/* BXTAL divider (mask) */
#define CLK_AUX_BXTAL_DIV(x)	        (x & 0x7)	/* BXTAL / x (x=0 => /8) */

#define CLK_CODEC_DIV_SEL_MASK		(0x3 << 7)	/* CODEC divider clock input select (mask) */
#define CLK_CODEC_DIV_SEL_PLL_N		(0 << 7)	/* use ratio from [CLK_CODEC_DIV_DIVIDER] */
#define CLK_CODEC_DIV_SEL_PLL_2		(1 << 7)	/* divide by 2 */
#define CLK_CODEC_DIV_SEL_PLL_1		(2 << 7)	/* no divider */
#define CLK_CODEC_DIV_DIVIDER_MASK	(0x7f)		/* mask out divider */
#define CLK_CODEC_DIV_DIVIDER(x)	(x & 0x7f)	/* divide by N ratio */

#define CLK_CODEC_PCM_SEL_MASK		(0x3 << 8)	/* PCM_CLK clock */
#define CLK_CODEC_PCM_SEL_OFF		(0 << 8)	/* clock off */
#define CLK_CODEC_PCM_SEL_X1		(1 << 8)	/* 1.152 MHz */
#define CLK_CODEC_PCM_SEL_X2		(2 << 8)	/* 2 x 1.152 MHz */
#define CLK_CODEC_PCM_SEL_X4		(3 << 8)	/* 4 x 1.152 MHz */
#define CLK_CODEC_DA_LSR_SEL_MASK	(0x3 << 6)	/* Codec DA clock to loudspeaker */
#define CLK_CODEC_DA_LSR_SEL_OFF	(0 << 6)	/* clock off */
#define CLK_CODEC_DA_LSR_SEL_X1		(1 << 6)	/* 1.152 MHz */
#define CLK_CODEC_DA_LSR_SEL_X2		(2 << 6)	/* 2 x 1.152 MHz */
#define CLK_CODEC_DA_LSR_SEL_X4		(3 << 6)	/* 4 x 1.152 MHz */
#define CLK_CODEC_DA_CLASSD_SEL_MASK	(0x3 << 4)	/* Codec DA clock to CLASSD amplifier */
#define CLK_CODEC_DA_CLASSD_SEL_OFF	(0 << 4)	/* clock off */
#define CLK_CODEC_DA_CLASSD_SEL_X1	(1 << 4)	/* 1.152 MHz */
#define CLK_CODEC_DA_CLASSD_SEL_X2	(2 << 4)	/* 2 x 1.152 MHz */
#define CLK_CODEC_DA_CLASSD_SEL_X4	(3 << 4)	/* 4 x 1.152 MHz */
#define CLK_CODEC_AD_SEL_MASK		(0x3 << 2)	/* Codec AD clock from microphone */
#define CLK_CODEC_AD_SEL_OFF		(0 << 2)	/* clock off */
#define CLK_CODEC_AD_SEL_X1		(1 << 2)	/* 1.152 MHz */
#define CLK_CODEC_AD_SEL_X2		(2 << 2)	/* 2 x 1.152 MHz */
#define CLK_CODEC_AD_SEL_X4		(3 << 2)	/* 4 x 1.152 MHz */
#define CLK_CODEC_MAIN_SEL_MASK		(0x3)		/* DSP main counter clock */
#define CLK_CODEC_MAIN_SEL_OFF		(0)			/* clock off */
#define CLK_CODEC_MAIN_SEL_X1		(1)			/* 1.152 MHz <- base frequency for 8, 16, 32 kHz !! */
#define CLK_CODEC_MAIN_SEL_X2		(2)			/* 2 x 1.152 MHz */
#define CLK_CODEC_MAIN_SEL_X4		(3)			/* 4 x 1.152 MHz */

#define CLK_DSP_DSP2_EN			(1 << 7)		/* Gen2DSP2 clock divider */
#define CLK_DSP_DSP2_DIV_MASK		(0x7 << 4)		/* Gen2DSP2 clock divider (mask) */
#define CLK_DSP_DSP2_DIV(x)		((x & 0x7) << 4)	/* divide by N ratio */
#define CLK_DSP_DSP1_EN			(1 << 3)		/* Gen2DSP2 clock divider */
#define CLK_DSP_DSP1_DIV_MASK		(0x7)			/* Gen2DSP2 clock divider (mask) */
#define CLK_DSP_DSP1_DIV(x)		(x & 0x7)		/* divide by N ratio */

#define CLK_FREQ_TRIM_OSC_OK		(1 << 9)	/* 1= when oscillator is running OK */
#define CLK_FREQ_TRIM_CL_SEL		(1 << 8)	/* Set XTAL load capacitence 0: >=20pF  1: <20pF */
#define CLK_FREQ_TRIM_COARSE_ADJ_MASK	(0x7 << 5)	/* mask MSB adjustment bits */
#define CLK_FREQ_TRIM_COARSE_ADJ(x)	((x & 0x7) << 5)	/* note: increment or decrement by 1 */
#define CLK_FREQ_TRIM_FINE_ADJ_MASK	(0x1f)		/* mask LSB adjustment bits */
#define CLK_FREQ_TRIM_FINE_ADJ(x)	(x & 0x1f)	/* note: change 1 bit at a time */

#define CLK_PER_DIV_SEL_MASK		(0x3 << 7)	/* Divider clock input selection (mask) */
#define CLK_PER_DIV_SEL_PLL_N		(0x0 << 7)	/* use ratio from [CLK_PER_DIV_DIVIDER] */
#define CLK_PER_DIV_SEL_PLL_2		(0x1 << 7)	/* divide by 2 */
#define CLK_PER_DIV_MASK		(0x7f)		/* Peripheral clock divider (mask) */
#define CLK_PER_DIV_DIVIDER(x)		(x & 0x7f)	/* Divider to use */

#define CLK_PER10_DIV_PER20_MASK	(0x7 << 5)	/* Peripheral clock divider factor for SPI1 and SPI2 */
#define CLK_PER10_DIV_PER20_DIVIDER(x)	((x  & 0x7) << 5)
#define CLK_PER10_DIV_SEL_MASK		(0x3 << 3)	/* divider clock input selection */
#define CLK_PER10_DIV_SEL_PLL_N		(0 << 3)	/* use ratio from [CLK_PER10_DIV_DIVIDER] */
#define CLK_PER10_DIV_SEL_PLL_2		(1 << 3)	/* divide by 2 */
#define CLK_PER10_DIV_MASK		(0x7)		/* Peripheral clock divider factor for SPI1 and SPI2 */
#define CLK_PER10_DIV_DIVIDER(x)	(x  & 0x7)	/* divide by N ratio */

#define CLK_PLL1_CTRL_DIV2_CLK_SEL	(1 << 12)	/* must be set 0! */
#define CLK_PLL1_CTRL_DYN_SW		(1 << 11)	/* 1=dynamic switching, 0=fixed */
#define CLK_PLL1_CTRL_PLL_DIP_DIV_MASK	(0xf << 7)	/* DIP clock divider (mask) */
#define CLK_PLL1_CTRL_PLL_DIP_DIV(x)	((x & 0xf) << 7)	/* DIP clock divider */
#define CLK_PLL1_CTRL_HF_SEL		(1 << 6)	/* High frequency mode selection */
#define CLK_PLL1_CTRL_VCO_ON		(1 << 4)	/* control PLL1 VCO */
#define CLK_PLL1_CTRL_PLL_CLK_SEL	(1 << 3)	/* set system main clock: 0=XTAL, 1=PLL */
#define CLK_PLL1_CTRL_PLL_OUT_DIV	(1 << 2)	/* 1=divide PLL1 o/p by 2 */
#define CLK_PLL1_CTRL_CP_ON		(1 << 1)	/* 0=disable PLL1 VCO charge pump */
#define CLK_PLL1_CTRL_TESTMODE_SEL	(1 << 11)	/* 0=VCO test mode, 1=PLL test mode */

#define CLK_PLL2_CTRL_HF_SEL		(1 << 6)	/* High frequency mode selection */
#define CLK_PLL2_CTRL_VCO_ON		(1 << 4)	/* control PLL2 VCO */
#define CLK_PLL2_CTRL_PLL_CLK_SEL	(1 << 3)	/* set system main clock: 0=XTAL, 1=PLL */
#define CLK_PLL2_CTRL_PLL_OUT_DIV	(1 << 2)	/* 1=divide PLL2 o/p by 2 */
#define CLK_PLL2_CTRL_CP_ON		(1 << 1)	/* 0=disable PLL2 VCO charge pump */
#define CLK_PLL2_CTRL_TESTMODE_SEL	(1 << 11)	/* 0=VCO test mode, 1=PLL test mode */

#define CLK_PLL1_DIV_VD1_MASK		(0x7 << 2)	/* PLL1 VCO divider */
#define CLK_PLL1_DIV_VD1_1X4		(0 << 2)	/* divide by 1x4 */
#define CLK_PLL1_DIV_VD1_2X4		(1 << 2)	/* divide by 2x4 */
#define CLK_PLL1_DIV_VD1_3X4		(2 << 2)	/* divide by 3x4 */
#define CLK_PLL1_DIV_VD1_4X4		(7 << 2)	/* divide by 4x4 */
#define CLK_PLL1_DIV_VD1_1X9		(4 << 2)	/* divide by 1x9 */
#define CLK_PLL1_DIV_VD1_2X9		(5 << 2)	/* divide by 2x9 */
#define CLK_PLL1_DIV_VD1_3X9		(6 << 2)	/* divide by 3x9 */
#define CLK_PLL1_DIV_XD1_MASK		(0x3)		/* PLL1 xtal divider */
#define CLK_PLL1_DIV_XD1_DIV_1		(1)			/* Divide by 1 */
#define CLK_PLL1_DIV_XD1_DIV_2		(2)			/* Divide by 2 */

#define CLK_PLL2_DIV_DIV5		(1 << 15)	/* VCO divisor: 0=DIV3, DIV4, VD, 1=use 13x23 */
#define CLK_PLL2_DIV_DIV3		(1 << 14)
#define CLK_PLL2_DIV_DIV4		(1 << 13)
#define CLK_PLL2_DIV_VD_MASK		(0xf << 9)	/* PLL VCO divider */
#define CLK_PLL2_DIV_VD(x)		((x & 0xf) << 13)
#define CLK_PLL2_DIV_DIV1		(1 << 8)
#define CLK_PLL2_DIV_DIV2		(1 << 7)
#define CLK_PLL2_DIV_XD_MASK		(0x7f)		/* PLL xtal divider */
#define CLK_PLL2_DIV_XD(x)		((x & 0x7f))	

#define CLK_XTAL_CTRL_XTAL_EXTRA_CV	(1 << 7)	/* 0=disable extra 25pf XTAL caps */
#define CLK_XTAL_CTRL_LDO_RFCLK_OK	(1 << 6)	/* 1=LDO_RF_CLK voltage OK */
#define CLK_XTAL_CTRL_RFCLK_SUPPLY	(1 << 5)	/* set RFCLK source: 0=LDO_RFCLK 1=VDDIO_RFDIG */
#define CLK_XTAL_CTRL_LDO_RFCLK_MASK	(0x3 << 3)	/* set voltage for RFCK */
#define CLK_XTAL_CTRL_LDO_RFCLK_OFF	(0x0 << 3)	/* LDO_RFCLK is off */
#define CLK_XTAL_CTRL_LDO_RFCLK_1V6	(0x1 << 3)	/* VDDIO_RFCLK = 1.6V, VDDIO_RFDIG >1.8V */
#define CLK_XTAL_CTRL_LDO_RFCLK_2V3	(0x2 << 3)	/* VDDIO_RFCLK = 2.3V, VDDIO_RFDIG >2.5V */
#define CLK_XTAL_CTRL_LDO_RFCLK_3V1	(0x3 << 3)	/* VDDIO_RFCLK = 3.1V, VDDIO_RFDIG >3.3V */
#define CLK_XTAL_CTRL_AVD_XTAL_OK	(1 << 2)	/* 1=AVD_XTAL voltage (1v6) is OK */
#define CLK_XTAL_CTRL_XTAL_SUPPLY	(1 << 1)	/* set XTAL supply: 0=LDO_XTAL 1=VDDREF */
#define CLK_XTAL_CTRL_LDO_XTAL_ON	(1)		/* 1=enable LDO_XTAL supply */



/*
 * Audio CODEC
 */
#define CODEC_MIC_REG			__REG(0xFF5800)	/* Codec microphone control register */
#define CODEC_LSR_REG			__REG(0xFF5802)	/* Conopoudspeaker control register */
#define CODEC_VREF_REG			__REG(0xFF5804)	/* Codec vref control register */
#define CODEC_TONE_REG			__REG(0xFF5806)	/* Codec CID control register */
#define CODEC_ADDA_REG			__REG(0xFF5808)	/* Codec ad/da control register */
#define CODEC_OFFSET1_REG		__REG(0xFF580A)	/* Codec offset error and compensation register */
#define CODEC_TEST_CTRL_REG		__REG(0xFF580C)	/* Codec test control register codec */
#define CODEC_OFFSET2_REG		__REG(0xFF580E)	/* Codec offset compensation register */


/*
 * BMC is part of the DIP.  It manages to the RF module data link.
 */
#define BMC_CTRL_REG			__REG(0xFF6400)	/* BMC control register */
#define BMC_CTRL2_REG			__REG(0xFF6402)	/* BMC control register 2 */
#define BMC_TX_SFIELDL_REG		__REG(0xFF6404)	/* BMC Tx S field register Low */
#define BMC_TX_SFIELDH_REG		__REG(0xFF6406)	/* BMC Tx S field register High */
#define BMC_RX_SFIELDL_REG		__REG(0xFF6408)	/* BMC Rx S field register Low */
#define BMC_RX_SFIELDH_REG		__REG(0xFF640A)	/* BMC Rx S field register High */


/*
 * DIP (Dedicated Instruction Processor) (RF Front End Interface)
 */
#define DIP_STACK_REG			__REG(0xFF6000)	/* DIP Stack pointer. (read only). The DIP stack is 4 deep */
#define DIP_PC_REG			__REG(0xFF6002)	/* DIP program counter */
#define DIP_STATUS_REG			__REG(0xFF6004)	/* DIP Status register, */
#define DIP_CTRL_REG			__REG(0xFF6006)	/* DIP Control register1 */
#define DIP_STATUS1_REG			__REG(0xFF6008)	/* DIP Status register1, */
#define DIP_CTRL1_REG			__REG(0xFF600A)	/* DIP Control register, clears DIP_INT if read */
#define DIP_SLOT_NUMBER_REG		__REG(0xFF600C)	/* DIP slot number register, returns the current slot number */
#define DIP_CTRL2_REG			__REG(0xFF600E)	/* DIP Control register 2 (debug status information) */
#define DIP_USB_PHASE_REG		__REG(0xFF6010)	/* Phase between SLOTZERO and USB SOF */
#define DIP_MOD_SEL_REG			__REG(0xFF6012)	/* DIP Modulo counters selection */
#define DIP_MOD_VAL_REG			__REG(0xFF6014)	/* DIP Modulo values selection */

#define DIP_RAM_START			__REG(0x1000000)	/* DIP Sequencer RAM (256 words) */
#define DIP_RAM_END			__REG(0x10001FF)	/* End of Sequencer RAM */
#define DIP_RAM_2_START			__REG(0x1000200)	/* Sequencer RAM (256 words) to be addressed with new */
#define DIP_RAM_2_END			__REG(0x10003FF)	/* <JMPF>,<BR_B1> DiP commands */


/*
 * DMA 
 */
#define DMA0_A_STARTL_REG	        __REG(0xFF4400)	/* Start address Low A of DMA channel 0 */
#define DMA0_A_STARTH_REG	        __REG(0xFF4402)	/* Start address Low A of DMA channel 0 */
#define DMA0_B_STARTL_REG	        __REG(0xFF4404)	/* Start address Low B of DMA channel 0 */
#define DMA0_B_STARTH_REG	        __REG(0xFF4406)	/* Start address High B of DMA channel 0 */
#define DMA0_INT_REG		        __REG(0xFF4408)	/* DMA receive interrupt register channel 0 */
#define DMA0_LEN_REG		        __REG(0xFF440A)	/* DMA receive length register channel 0 */
#define DMA0_CTRL_REG		        __REG(0xFF440C)	/* Control register for the DMA channel 0 */
#define DMA0_IDX_REG		        __REG(0xFF440E)	/* Internal Index register for the DMA channel 0 */

#define DMA1_A_STARTL_REG	        __REG(0xFF4410)	/* Start address Low A of DMA channel 1 */
#define DMA1_A_STARTH_REG	        __REG(0xFF4412)	/* Start address High A of DMA channel 1 */
#define DMA1_B_STARTL_REG	        __REG(0xFF4414)	/* Start address Low B of DMA channel 1 */
#define DMA1_B_STARTH_REG	        __REG(0xFF4416)	/* Start address High B of DMA channel 1 */
#define DMA1_INT_REG		        __REG(0xFF4418)	/* DMA receive interrupt register channel 1 */
#define DMA1_LEN_REG		        __REG(0xFF441A)	/* DMA receive length register channel 1 */
#define DMA1_CTRL_REG		        __REG(0xFF441C)	/* Control register for the DMA channel 1 */
#define DMA1_IDX_REG		        __REG(0xFF441E)	/* Internal Index register for the DMA channel 1 */

#define DMA2_A_STARTL_REG	        __REG(0xFF4420)	/* Start address Low A of DMA channel 2 */
#define DMA2_A_STARTH_REG	        __REG(0xFF4422)	/* Start address High A of DMA channel 2 */
#define DMA2_B_STARTL_REG	        __REG(0xFF4424)	/* Start address Low B of DMA channel 2 */
#define DMA2_B_STARTH_REG	        __REG(0xFF4426)	/* Start address High B of DMA channel 2 */
#define DMA2_INT_REG		        __REG(0xFF4428)	/* DMA receive interrupt register channel 2 */
#define DMA2_LEN_REG		        __REG(0xFF442A)	/* DMA receive length register channel 2 */
#define DMA2_CTRL_REG	       	        __REG(0xFF442C)	/* Control register for the DMA channel 2 */
#define DMA2_IDX_REG		        __REG(0xFF442E)	/* Internal Index register for the DMA channel 2 */

#define DMA3_A_STARTL_REG	        __REG(0xFF4430)	/* Start address Low A of DMA channel 3 */
#define DMA3_A_STARTH_REG	        __REG(0xFF4432)	/* Start address High A of DMA channel 3 */
#define DMA3_B_STARTL_REG	        __REG(0xFF4434)	/* Start address Low B of DMA channel 3 */
#define DMA3_B_STARTH_REG	        __REG(0xFF4436)	/* Start address High B of DMA channel 3 */
#define DMA3_INT_REG		        __REG(0xFF4438)	/* DMA receive interrupt register channel 3 */
#define DMA3_LEN_REG		        __REG(0xFF443A)	/* DMA receive length register channel 3 */
#define DMA3_CTRL_REG		        __REG(0xFF443C)	/* Control register for the DMA channel 3 */
#define DMA3_IDX_REG		        __REG(0xFF443E)	/* Internal Index register for the DMA channel 3 */


/*
 * DSP (2 processors)
 */

/*
Gen2DSP1 and Gen2DSP2 Shared ROM
0x1020000 SHARED_ROM1_START Shared ROM (16k bytes)
0x10207FF SHARED_ROM1_END
0x1020800 - Reserved 16k- X
0x1027F7F -
*/


/* Gen2DSP1 and Gen2DSP2 common registers */
#define DSP_MAIN_SYNC0_REG		__REG(0x1027F80)	/* DSP main counter outputs selection register 0 */
#define DSP_MAIN_SYNC1_REG		__REG(0x1027F82)	/* DSP main counter outputs selection register 1 */
#define DSP_MAIN_CNT_REG		__REG(0x1027F84)	/* DSP main counter and reload register */
#define DSP_ADC0S_REG			__REG(0x1027F86)	/* ADC0 Input 2s Complement register */
#define DSP_ADC1S_REG			__REG(0x1027F88)	/* ADC1 Input 2s Complement register */
#define DSP_CLASSD_REG			__REG(0x1027F8A)	/* CLASSD Output output data register */
#define DSP_CODEC_MIC_GAIN_REG	        __REG(0x1027F8C)	/* CODEC MIC GAIN register */
#define DSP_CODEC_OUT_REG		__REG(0x1027F8E)	/* CODEC DATA register */
#define DSP_CODEC_IN_REG		__REG(0x1027F90)	/* CODEC DATA register */
#define DSP_RAM_OUT0_REG		__REG(0x1027F92)	/* Shared RAM 1 or 2 output register 0 */
#define DSP_RAM_OUT1_REG		__REG(0x1027F94)	/* Shared RAM 1 or 2 output register 1 */
#define DSP_RAM_OUT2_REG		__REG(0x1027F96)	/* Shared RAM 1 or 2 output register 2 */
#define DSP_RAM_OUT3_REG		__REG(0x1027F98)	/* Shared RAM 1 or 2 output register 3 */
#define DSP_RAM_IN0_REG			__REG(0x1027F9A)	/* Shared RAM 1 or 2 input register 0 */
#define DSP_RAM_IN1_REG			__REG(0x1027F9C)	/* Shared RAM 1 or 2 input register 1 */
#define DSP_RAM_IN2_REG			__REG(0x1027F9E)	/* Shared RAM 1 or 2 input register 2 */
#define DSP_RAM_IN3_REG			__REG(0x1027FA0)	/* Shared RAM 1 or 2 input register 3 */
#define DSP_ZCROSS1_OUT_REG		__REG(0x1027FA2)	/* ZERO CROSSING 1 output register */
#define DSP_ZCROSS2_OUT_REG		__REG(0x1027FA4)	/* ZERO CROSSING 2 output register */
#define DSP_PCM_OUT0_REG		__REG(0x1027FA6)	/* PCM channel 0 output register */
#define DSP_PCM_OUT1_REG		__REG(0x1027FA8)	/* PCM channel 1 output register */
#define DSP_PCM_OUT2_REG		__REG(0x1027FAA)	/* PCM channel 2 output register */
#define DSP_PCM_OUT3_REG		__REG(0x1027FAC)	/* PCM channel 3 output register */
#define DSP_PCM_IN0_REG			__REG(0x1027FAE)	/* PCM channel 0 input register */
#define DSP_PCM_IN1_REG			__REG(0x1027FB0)	/* PCM channel 1 input register */
#define DSP_PCM_IN2_REG			__REG(0x1027FB2)	/* PCM channel 2 input register */
#define DSP_PCM_IN3_REG			__REG(0x1027FB4)	/* PCM channel 3 input register */
#define DSP_PCM_CTRL_REG		__REG(0x1027FB6)	/* PCM Control register */
#define DSP_PHASE_INFO_REG		__REG(0x1027FB8)	/* Phase information between PCM FSC 8/16/32 and maincounter 8/16/32 kHz */
#define DSP_VQI_REG			__REG(0x1027FBA)	/* BMC VQI register */
#define DSP_MAIN_CTRL_REG		__REG(0x1027FBC)	/* DSP Main counter control and preset value */
#define DSP_CLASSD_BUZZOFF_REG	        __REG(0x1027FBE)	/* CLASSD Buzzer on/off controller, bit 15 */

/* Gen2DSP1 registers (shared by Gen2DSP2) */
#define DSP1_CTRL_REG			__REG(0x1027FD0)	/* DSP1 control register */
#define DSP1_PC_REG			__REG(0x1027FD2)	/* DSP1 Programma counter */
#define DSP1_PC_START_REG		__REG(0x1027FD4)	/* DSP1 Programma counter start */
#define DSP1_IRQ_START_REG		__REG(0x1027FD6)	/* DSP1 Interrupt vector start */
#define DSP1_INT_REG			__REG(0x1027FD8)	/* DSP1 to CR16 interrupt vector */
#define DSP1_INT_MASK_REG		__REG(0x1027FDA)	/* DSP1 to CR16 interrupt vector maks */
#define DSP1_INT_PRIO1_REG		__REG(0x1027FDC)	/* DSP1 Interrupt source mux 1 register */
#define DSP1_INT_PRIO2_REG		__REG(0x1027FDE)	/* DSP1 Interrupt source mux 2 register */
#define DSP1_OVERFLOW_REG		__REG(0x1027FE0)	/* DSP1 to CR16 overflow register */
#define DBG1_IREG			__REG(0x1027FF0)	/* DSP1 JTAG instruction register */
#define DBG1_INOUT_REG_LSW		__REG(0x1027FF4)	/* DSP1 DEBUG data register (32 bits) */
#define DBG1_INOUT_REG_MSW		__REG(0x1027FF6)	/* (Most Significant Word) */
#define SHARED_ROM2_START		__REG(0x1028000)	/* Shared RAM2 (16k bytes) */
#define SHARED_ROM2_END			__REG(0x102BFFF)	/* (Most Significant Word) */

/* Gen2DSP2 registers (shared by Gen2DSP1) */
#define DSP2_CTRL_REG			__REG(0x102FFD0)	/* DSP2 control register */
#define DSP2_PC_REG			__REG(0x102FFD2)	/* DSP2 Programma counter */
#define DSP2_PC_START_REG		__REG(0x102FFD4)	/* DSP2 Programma counter start */
#define DSP2_IRQ_START_REG		__REG(0x102FFD6)	/* DSP2 Interrupt vector start */
#define DSP2_INT_REG			__REG(0x102FFD8)	/* DSP2 to CR16 interrupt vector */
#define DSP2_INT_MASK_REG		__REG(0x102FFDA)	/* DSP2 to CR16 interrupt vector maks */
#define DSP2_INT_PRIO1_REG		__REG(0x102FFDC)	/* DSP2 Interrupt source mux 1 register */
#define DSP2_INT_PRIO2_REG		__REG(0x102FFDE)	/* DSP2 Interrupt source mux 2 register */
#define DSP2_OVERFLOW_REG		__REG(0x102FFE0)	/* DSP2 to CR16 overflow register */
#define DBG2_IREG			__REG(0x102FFF0)	/* DSP2 JTAG instruction register */
#define DBG2_INOUT_REG_LSW		__REG(0x102FFF4)	/* DSP2 DEBUG data register (32 bits) */
#define DBG2_INOUT_REG_MSW		__REG(0x102FFF6)	/* (Most Significant Word) */
#define DSP_MC_RAM1_START		__REG(0x1030000)	/* Gen2DSP MicroCode RAM 1 */
#define DSP_MC_RAM1_END			__REG(0x1033FFF)	/* 16kbyte */
#define DSP_MC_RAM2_START		__REG(0x1034000)	/* Gen2DSP MicroCode RAM 2 */
#define DSP_MC_RAM2_END			__REG(0x1037FFF)	/* 16kbyte */
#define DSP_MC_ROM1_START		__REG(0x1040000)	/* Gen2DSP MicroCode ROM1 */
#define DSP_MC_ROM1_END			__REG(0x1047FFF)	/* 32kbyte */
#define DSP_MC_ROM2_START		__REG(0x1060000)	/* Gen2DSP MicroCode ROM 2 */
#define DSP_MC_ROM2_END			__REG(0x107FFFF)	/* 96kbyte */


/*
 * External Bus memory controller
 */
#define EBI_SDCONR_REG			__REG32(0xFF0000)	/* SDRAM Configuration register */
#define EBI_SDTMG0R_REG			__REG32(0xFF0004)	/* SDRAM Timing register 0 */
#define EBI_SDTMG1R_REG			__REG32(0xFF0008)	/* SDRAM Timing register 1 */
#define EBI_SDCTLR_REG			__REG32(0xFF000C)	/* SDRAM Control register */
#define EBI_SDREFR_REG			__REG32(0xFF0010)	/* SDRAM Refresh register */
#define EBI_ACS0_LOW_REG		__REG32(0xFF0014)	/* Chip select 0 base address register */
#define EBI_ACS1_LOW_REG		__REG32(0xFF0018)	/* Chip select 1 base address register */
#define EBI_ACS2_LOW_REG		__REG32(0xFF001C)	/* Chip select 2 base address register */
#define EBI_ACS3_LOW_REG		__REG32(0xFF0020)	/* Chip select 3 base address register */
#define EBI_ACS4_LOW_REG		__REG32(0xFF0024)	/* Chip select 4 base address register */
#define EBI_ACS0_CTRL_REG		__REG32(0xFF0054)	/* Chip select 0 control register */
#define EBI_ACS1_CTRL_REG		__REG32(0xFF0058)	/* Chip select 1 control register */
#define EBI_ACS2_CTRL_REG		__REG32(0xFF005C)	/* Chip select 2 control register */
#define EBI_ACS3_CTRL_REG		__REG32(0xFF0060)	/* Chip select 3 control register */
#define EBI_ACS4_CTRL_REG		__REG32(0xFF0064)	/* Chip select 4 control register */
#define EBI_SMTMGR_SET0_REG		__REG32(0xFF0094)	/* Static memory Timing register Set 0 */
#define EBI_SMTMGR_SET1_REG		__REG32(0xFF0098)	/* Static memory Timing register Set 1 */
#define EBI_SMTMGR_SET2_REG		__REG32(0xFF009C)	/* Static memory Timing register Set 2 */
#define EBI_FLASH_TRPDR_REG		__REG32(0xFF00A0)	/* FLASH Timing Register */
#define EBI_SMCTLR_REG			__REG32(0xFF00A4)	/* Static memory Control register */
#define EBI_SDEXN_MODE_REG		__REG32(0xFF00AC)	/* Mobile SDRAM Extende mode register */


/*
 * Interrupt 
 */
#define SET_INT_PENDING_REG		__REG(0xFF5400)	/* Set interrupt pending register */
#define RESET_INT_PENDING_REG   __REG(0xFF5402)	/* Reset interrupt pending register */
#define INT0_PRIORITY_REG		__REG(0xFF5404)	/* Interrupt priority register 0 */
#define INT1_PRIORITY_REG		__REG(0xFF5406)	/* Interrupt priority register 1 */
#define INT2_PRIORITY_REG		__REG(0xFF5408)	/* Interrupt priority register 2 */
#define INT3_PRIORITY_REG		__REG(0xFF540A)	/* Interrupt priority register 2 */


/*
 * Keypad 
 */
#define KEY_GP_INT_REG		        __REG(0xFF49B0)	/* General purpose interrupts for KEYB_ */
#define KEY_BOARD_INT_REG	        __REG(0xFF49B2)	/* Keyboard interrupt enable register */
#define KEY_DEBOUNCE_REG	        __REG(0xFF49B4)	/* Keyboard debounce and auto key */
#define KEY_STATUS_REG		        __REG(0xFF49B6)	/* Keyboard interrupt status. */


/*
 * GPIO 
 */
#define P0_DATA_REG		        __REG(0xFF4830)	/* P0 Data input /out register */
#define P0_SET_DATA_REG		        __REG(0xFF4832)	/* P0 Set port pins register */
#define P0_RESET_DATA_REG	        __REG(0xFF4834)	/* P0 Reset port pins register */
#define P0_00_MODE_REG		        __REG(0xFF4840)	/* P0y Mode Register */
#define P0_01_MODE_REG		        __REG(0xFF4842)	/* P0y Mode Register */
#define P0_02_MODE_REG		        __REG(0xFF4844)	/* P0y Mode Register */
#define P0_03_MODE_REG		        __REG(0xFF4846)	/* P0y Mode Register */
#define P0_04_MODE_REG		        __REG(0xFF4848)	/* P0y Mode Register */
#define P0_05_MODE_REG		        __REG(0xFF484A)	/* P0y Mode Register */
#define P0_06_MODE_REG		        __REG(0xFF484C)	/* P0y Mode Register */
#define P0_07_MODE_REG		        __REG(0xFF484E)	/* P0y Mode Register */
#define P0_08_MODE_REG		        __REG(0xFF4850)	/* P0y Mode Register */
#define P0_09_MODE_REG		        __REG(0xFF4852)	/* P0y Mode Register */
#define P0_10_MODE_REG		        __REG(0xFF4854)	/* P0y Mode Register */
#define P0_11_MODE_REG		        __REG(0xFF4856)	/* P0y Mode Register */
#define P0_12_MODE_REG		        __REG(0xFF4858)	/* P0y Mode Register */
#define P0_13_MODE_REG		        __REG(0xFF485A)	/* P0y Mode Register */
#define P0_14_MODE_REG		        __REG(0xFF485C)	/* P0y Mode Register */
#define P0_15_MODE_REG		        __REG(0xFF485E)	/* P0y Mode Register */

#define P1_DATA_REG			__REG(0xFF4860)	/* P1 Data input /out register */
#define P1_SET_DATA_REG		        __REG(0xFF4862)	/* P1 Set port pins register */
#define P1_RESET_DATA_REG	        __REG(0xFF4864)	/* P1 Reset port pins register */
#define P1_00_MODE_REG		        __REG(0xFF4870)	/* P1y Mode Register */
#define P1_01_MODE_REG		        __REG(0xFF4872)	/* P1y Mode Register */
#define P1_02_MODE_REG		        __REG(0xFF4874)	/* P1y Mode Register */
#define P1_03_MODE_REG		        __REG(0xFF4876)	/* P1y Mode Register */
#define P1_04_MODE_REG		        __REG(0xFF4878)	/* P1y Mode Register */
#define P1_05_MODE_REG		        __REG(0xFF487A)	/* P1y Mode Register */
#define P1_06_MODE_REG		        __REG(0xFF487C)	/* P1y Mode Register */
#define P1_07_MODE_REG		        __REG(0xFF487E)	/* P1y Mode Register */
#define P1_08_MODE_REG		        __REG(0xFF4880)	/* P1y Mode Register */
#define P1_09_MODE_REG		        __REG(0xFF4882)	/* P1y Mode Register */
#define P1_10_MODE_REG		        __REG(0xFF4884)	/* P1y Mode Register */
#define P1_11_MODE_REG	       	        __REG(0xFF4886)	/* P1y Mode Register */
#define P1_12_MODE_REG		        __REG(0xFF4888)	/* P1y Mode Register */
#define P1_13_MODE_REG		        __REG(0xFF488A)	/* P1y Mode Register */
#define P1_14_MODE_REG		        __REG(0xFF488C)	/* P1y Mode Register */
#define P1_15_MODE_REG		        __REG(0xFF488E)	/* P1y Mode Register */

#define P2_DATA_REG		        __REG(0xFF4890)	/* P2 Data input /out register */
#define P2_SET_DATA_REG		        __REG(0xFF4892)	/* P2 Set port pins register */
#define P2_RESET_DATA_REG	        __REG(0xFF4894)	/* P2 Reset port pins register */
#define P2_00_MODE_REG		        __REG(0xFF48A0)	/* P2y Mode Register */
#define P2_01_MODE_REG		        __REG(0xFF48A2)	/* P2y Mode Register */
/* P2_02 is not configurable */
/* P2_03 is not configurable */
#define P2_04_MODE_REG		        __REG(0xFF48A8)	/* P2y Mode Register */
#define P2_05_MODE_REG		        __REG(0xFF48AA)	/* P2y Mode Register */
#define P2_06_MODE_REG		        __REG(0xFF48AC)	/* P2y Mode Register */
#define P2_07_MODE_REG		        __REG(0xFF48AE)	/* P2y Mode Register */
#define P2_08_MODE_REG		        __REG(0xFF48B0)	/* P2y Mode Register */
#define P2_09_MODE_REG		        __REG(0xFF48B2)	/* P2y Mode Register */
#define P2_10_MODE_REG		        __REG(0xFF48B4)	/* P2y Mode Register */
/* P2_11 is not configurable */
/* P2_12 is not configurable */
/* P2_13 is not configurable */
/* P2_14 is not configurable */
/* P2_15 is not configurable */


/* Pull-up and Pull-down select for ports P0, P1, P2: bits 9, 8
   (In analog mode, these bits are have no effect) */
#define GPIO_PUPD_IN_NONE	       (0x0 << 8)	/* Input, no resistors selected */
#define GPIO_PUPD_IN_PULLUP	       (0x1 << 8)	/* Input, pull-up selected */
#define GPIO_PUPD_IN_PULLDOWN	       (0x2 << 8)	/* Input, Pull-down selected */
#define GPIO_PUPD_OUT_NONE	       (0x3 << 8)	/* Output, no resistors selected*/

/* GPIO Peripherial IDs for ports P0, P1, P2
   There are limits for which peripherials can be connected to which GPIOs.
   Please refer to Table 4 on page 15 of the SC14450 datasheet */
#define GPIO_PID_NONE		       (0)		/* Plain GPIO - Not attached to a peripherial */
#define GPIO_PID_CLK100		       (1)
#define GPIO_PID_TDOD		       (2)
#define GPIO_PID_BXTAL		       (3)
#define GPIO_PID_PWM0		       (4)		/* T0 */
#define GPIO_PID_PWM1		       (5)		/* T0 */
#define GPIO_PID_ECZ1		       (6)
#define GPIO_PID_ECZ2		       (7)
#define GPIO_PID_PLL2_CLK	       (8)
#define GPIO_PID_WTF_IN1	       (9)
#define GPIO_PID_WTF_IN2	       (10)
#define GPIO_PID_UTX		       (11)
#define GPIO_PID_URX		       (12)
#define GPIO_PID_SDA1		       (13)
#define GPIO_PID_SCL1	      	       (14)
#define GPIO_PID_SDA2		       (15)
#define GPIO_PID_SCL2		       (16)
#define GPIO_PID_SPI1_DOUT	       (17)
#define GPIO_PID_SPI1_DIN	       (18)
#define GPIO_PID_SPI1_CLK	       (19)
#define GPIO_PID_SPI1_EN	       (20)
#define GPIO_PID_SPI2_DOUT	       (21)
#define GPIO_PID_SPI2_DIN	       (22)
#define GPIO_PID_SPI2_CLK	       (23)
#define GPIO_PID_PCM_DO		       (24)
#define GPIO_PID_PCM_CLK	       (25)
#define GPIO_PID_PCM_FSC	       (26)
#define GPIO_PID_PCM_DI		       (27)
#define GPIO_PID_AD13		       (28)
#define GPIO_PID_AD14		       (28)
#define GPIO_PID_AD15		       (28)
#define GPIO_PID_AD16		       (28)
#define GPIO_PID_AD17		       (28)
#define GPIO_PID_AD18		       (28)
#define GPIO_PID_AD19		       (28)
#define GPIO_PID_AD20		       (28)
#define GPIO_PID_AD21		       (28)
#define GPIO_PID_AD22		       (28)
#define GPIO_PID_AD23		       (28)
#define GPIO_PID_SDA10		       (29)
#define GPIO_PID_SDWEN		       (29)
#define GPIO_PID_SDBA0		       (29)
#define GPIO_PID_SDBA1		       (29)
#define GPIO_PID_SDLDQM		       (29)
#define GPIO_PID_SDUDQM		       (29)
#define GPIO_PID_SDRASn		       (29)
#define GPIO_PID_SDCASn		       (29)
#define GPIO_PID_SDCKE		       (29)
#define GPIO_PID_SDCLK		       (30)
#define GPIO_PID_SF_ADV		       (31)
#define GPIO_PID_BE0n		       (32)
#define GPIO_PID_BE1n		       (33)
#define GPIO_PID_READY		       (34)
#define GPIO_PID_INT0n		       (35)
#define GPIO_PID_INT1n		       (36)
#define GPIO_PID_INT2n		       (37)
#define GPIO_PID_INT3n		       (38)
#define GPIO_PID_INT4n		       (39)
#define GPIO_PID_INT5n		       (40)
#define GPIO_PID_INT6n		       (41)
#define GPIO_PID_INT7n		       (42)
#define GPIO_PID_INT8n		       (43)
#define GPIO_PID_ACS0 		       (44)	/* default = FLASH */
#define GPIO_PID_ACS1		       (45)
#define GPIO_PID_ACS2		       (46)
#define GPIO_PID_ACS3 		       (47)	/* default = PSRAM */
#define GPIO_PID_ACS4 		       (48)	/* default = SDRAM */
#define GPIO_PID_PD1		       (49)
#define GPIO_PID_PD2		       (50)
#define GPIO_PID_PD3		       (51)
#define GPIO_PID_PD4		       (51)
#define GPIO_PID_PD5		       (53)
#define GPIO_PID_PAOUTp		       (63)
#define GPIO_PID_PAOUTn		       (63)
#define GPIO_PID_ADC0		       (63)
#define GPIO_PID_ADC1		       (63)
#define GPIO_PID_ADC2_RSSI	       (63)



/*
 * Misc
 */
#define DEBUG_REG		       __REG(0xFF5004)	/* DEBUG_REG for boot program control and debug control */
#define PC_START_REG		       __REG(0xFF540C)	/* CR16C+ startup address */
#define CP_CTRL_REG		       __REG(0xFF481C)	/* Charge pump for LEDs control register. */


/*
 * SPI 
 */
#define SPI1_CTRL_REG		       __REG(0xFF4940)	/* SPI 1 control register */
#define SPI1_RX_TX_REG0		       __REG(0xFF4942)	/* SPI 1 RX/TX register0 */
#define SPI1_RX_TX_REG1		       __REG(0xFF4944)	/* SPI 1 RX/TX register1 */
#define SPI1_CLEAR_INT_REG	       __REG(0xFF4946)	/* SPI 1 clear interrupt register */
#define SPI2_CTRL_REG		       __REG(0xFF4950)	/* SPI 2 control register */
#define SPI2_RX_TX_REG0		       __REG(0xFF4952)	/* SPI 2 RX/TX register0 */
#define SPI2_RX_TX_REG1		       __REG(0xFF4954)	/* SPI 2 RX/TX register1 */
#define SPI2_CLEAR_INT_REG	       __REG(0xFF4956)	/* SPI 2 clear interrupt register */

/* Common to SPI1_CTRL_REG, SPI2_CTRL_REG*/
#define SPI_CTRL_ON                (0x0001)
#define SPI_CTRL_PHA               (0x0002)
#define SPI_CTRL_POL               (0x0004)
#define SPI_CTRL_CLK               (0x0018)
#define SPI_CTRL_DO                (0x0020)
#define SPI_CTRL_SMN               (0x0040)
#define SPI_CTRL_WORD              (0x0180)
#define SPI_CTRL_RST               (0x0200)
#define SPI_CTRL_FORCE_DO          (0x0400)
#define SPI_CTRL_TXH               (0x0800)
#define SPI_CTRL_DI                (0x1000)
#define SPI_CTRL_INT_BIT           (0x2000)
#define SPI_CTRL_MINT              (0x4000)
#define SPI_CTRL_EN_CTRL           (0x8000)

/*
 * Test registers (DO NOT USE!)
 */
#define TEST_ENV_REG		       __REG(0xFF4800)	/* CR16C boot mode control register */
#define TEST_CTRL_REG		       __REG(0xFF4802)	/* Test control register */
#define TEST_CTRL2_REG		       __REG(0xFF4804)	/* Test control register 2 */


/*
 * Timers (for OS and PWM)
 */
#define TIMER_CTRL_REG		       __REG(0xFF4970)	/* Timers control registers */
#define TIMER0_ON_REG		       __REG(0xFF4972)	/* Timers 0 on control registers */
#define TIMER0_RELOAD_M_REG	       __REG(0xFF4974)	/* 2 x 16 bits reload value for Timer0 */
#define TIMER0_RELOAD_N_REG	       __REG(0xFF4976)	/* (seconds part of above) */
#define TIMER1_RELOAD_M_REG	       __REG(0xFF4978)	/* 2 x 16 bits reload value for Timer1 */
#define TIMER1_RELOAD_N_REG	       __REG(0xFF497A)	/* (second part of above) */
#define TIMER2_DUTY_REG		       __REG(0xFF497C)	/* Timer2 duty cycle */
#define TIMER2_PRSC_REG		       __REG(0xFF497E)	/* Timer2 frequency prescaler */


/*
 * Tone Generator
 */
#define TONE_CTRL1_REG		       __REG(0xFF4990)	/* Capture timer control register 1 */
#define TONE_COUNTER1_REG	       __REG(0xFF4992)	/* Capture timer Counter 1 */
#define TONE_LATCH1_REG		       __REG(0xFF4994)	/* Capture timer Latch 1 */
#define TONE_CLEAR_INT1_REG	       __REG(0xFF4996)	/* Clears CT1 interrupt and TONE_LATCH1_REG */
#define TONE_CTRL2_REG		       __REG(0xFF4998)	/* Capture timer control register 2 */
#define TONE_COUNTER2_REG	       __REG(0xFF499A)	/* Capture timer Counter 2 */
#define TONE_LATCH2_REG		       __REG(0xFF499C)	/* Capture timer Latch 2 */
#define TONE_CLEAR_INT2_REG	       __REG(0xFF499E)	/* Clears CT2 interrupt and TONE_LATCH2_REG */


/*
 * UART
 */
#define UART_CTRL_REG		       __REG(0xFF4900)	/* Control register */
#define UART_RX_TX_REG		       __REG(0xFF4902)	/* Data transmit/receive register */
#define UART_CLEAR_TX_INT_REG	       __REG(0xFF4904)	/* Clear transmit interrupt */
#define UART_CLEAR_RX_INT_REG	       __REG(0xFF4906)	/* Clear receive interrupt */
#define UART_ERROR_REG		       __REG(0xFF4908)	/* Parity error register */

#define UART_CTRL_INV_UTX	       (1 << 10)	/* R/W - UTX inversion (1) */
#define UART_CTRL_INV_URX	       (1 << 9)	/* R/W - URX inversion (1) */
#define UART_CTRL_IRDA_EN	       (1 << 8)	/* R/W - NRZ mode (1) or normal UART mode */
#define UART_CTRL_UART_MODE	       (1 << 7)	/* R/W - Force (1) to use 10.125kbaud even parity */
#define UART_CTRL_RI		       (1 << 6)	/* R - Set if RX IRQ received */
#define UART_CTRL_TI		       (1 << 5)	/* R - Set if TX IRQ received */
#define UART_CTRL_BAUDRATE_MASK	       (7 << 2)	/* Mask all the baud-rate config bits */
#define UART_CTRL_BAUDRATE_9K6	       (0 << 2)	/* R/W - 0 = 9.6 kBaud */
#define UART_CTRL_BAUDRATE_19K2	       (1 << 2)	/* R/W - 1 = 19.2 kBaud */
#define UART_CTRL_BAUDRATE_57K6	       (2 << 2)	/* R/W - 2 = 57.6 kBaud */
#define UART_CTRL_BAUDRATE_115K2       (3 << 2)	/* R/W - 3 = 115.2 kBaud */
#define UART_CTRL_BAUDRATE_FSYS	       (4 << 2)	/* R/W - 4 = Fsys/(X*45) (230.4 kBaud @10.368 MHz) */		
#define UART_CTRL_TEN		       (1 << 1)	/* R/W - enable (1) UART transmitter */
#define UART_CTRL_REN		       (1 << 0)	/* R/W - enable (1) UART receiver */

#define UART_RX_TX_MASK		       (0xff)		/* only the lower 8 bits contain data */

#define UART_ERROR_DMA_PARITY	       (1 << 1)	/* R/W Set when parity error duing DMA. clear by writing. */
#define UART_ERROR_PAR_STATUS	       (1)			/* R - 1= parity error, updated for every new byte */

/*
 * USB Controller 
 */
#define USB_MCTRL_REG		       __REG(0xFF6800)	/* Main Control Register) */
#define USB_XCVDIAG_REG		       __REG(0xFF6802)	/* Transceiver diagnostic Register (for test purpose only) */
#define USB_TCR_REG	       	       __REG(0xFF6804)	/* Transceiver configuration Register */
#define USB_UTR_REG	       	       __REG(0xFF6806)	/* USB test Register (for test purpose only) */
#define USB_FAR_REG		       __REG(0xFF6808)	/* Function Address Register */
#define USB_NFSR_REG		       __REG(0xFF680A)	/* Node Functional State Register */
#define USB_MAEV_REG		       __REG(0xFF680C)	/* Main Event Register */
#define USB_MAMSK_REG		       __REG(0xFF680E)	/* Main Mask Register */
#define USB_ALTEV_REG		       __REG(0xFF6810)	/* Alternate Event Register */
#define USB_ALTMSK_REG		       __REG(0xFF6812)	/* Alternate Mask Register */
#define USB_TXEV_REG		       __REG(0xFF6814)	/* Transmit Event Register */
#define USB_TXMSK_REG		       __REG(0xFF6816)	/* Transmit Mask Register */
#define USB_RXEV_REG		       __REG(0xFF6818)	/* Receive Event Register */
#define USB_RXMSK_REG		       __REG(0xFF681A)	/* Receive Mask Register */
#define USB_NAKEV_REG		       __REG(0xFF681C)	/* NAK Event Register */
#define USB_NAKMSK_REG		       __REG(0xFF681E)	/* NAK Mask Register */
#define USB_FWEV_REG		       __REG(0xFF6820)	/* FIFO Warning Event Register */
#define USB_FWMSK_REG		       __REG(0xFF6822)	/* FIFO Warning Mask Register */
#define USB_FNH_REG		       __REG(0xFF6824)	/* Frame Number High Byte Register */
#define USB_FNL_REG		       __REG(0xFF6826)	/* Frame Number Low Byte Register */
#define USB_UX20CDR_REG		       __REG(0xFF683E)	/* Transceiver 2.0 Configuration and Diagnostics Register(for test purpose only) */

#define USB_EPC0_REG	      	       __REG(0xFF6840)	/* Endpoint Control 0 Register */
#define USB_TXD0_REG		       __REG(0xFF6842)	/* Transmit Data 0 Register */
#define USB_TXS0_REG		       __REG(0xFF6844)	/* Transmit Status 0 Register */
#define USB_TXC0_REG	      	       __REG(0xFF6846)	/* Transmit command 0 Register */
#define USB_EP0_NAK_REG		       __REG(0xFF6848)	/* EP0 INNAK and OUTNAK Register */
#define USB_RXD0_REG		       __REG(0xFF684A)	/* Receive Data 0 Register */
#define USB_RXS0_REG		       __REG(0xFF684C)	/* Receive Status 0 Register */
#define USB_RXC0_REG		       __REG(0xFF684E)	/* Receive Command 0 Register */

#define USB_EPC1_REG		       __REG(0xFF6850)	/* Endpoint Control Register 1 */
#define USB_TXD1_REG		       __REG(0xFF6852)	/* Transmit Data Register 1 */
#define USB_TXS1_REG		       __REG(0xFF6854)	/* Transmit Status Register 1 */
#define USB_TXC1_REG		       __REG(0xFF6856)	/* Transmit Command Register 1 */
#define USB_EPC2_REG		       __REG(0xFF6858)	/* Endpoint Control Register 2 */
#define USB_RXD1_REG		       __REG(0xFF685A)	/* Receive Data Register,1 */
#define USB_RXS1_REG		       __REG(0xFF685C)	/* Receive Status Register 1 */
#define USB_RXC1_REG		       __REG(0xFF685E)	/* Receive Command Register 1 */

#define USB_EPC3_REG		       __REG(0xFF6860)	/* Endpoint Control Register 3 */
#define USB_TXD2_REG		       __REG(0xFF6862)	/* Transmit Data Register 2 */
#define USB_TXS2_REG		       __REG(0xFF6864)	/* Transmit Status Register 2 */
#define USB_TXC2_REG		       __REG(0xFF6866)	/* Transmit Command Register 2 */
#define USB_EPC4_REG		       __REG(0xFF6868)	/* Endpoint Control Register 4 */
#define USB_RXD2_REG		       __REG(0xFF686A)	/* Receive Data Register 2 */
#define USB_RXS2_REG		       __REG(0xFF686C)	/* Receive Status Register 2 */
#define USB_RXC2_REG		       __REG(0xFF686E)	/* Receive Command Register 2 */

#define USB_EPC5_REG		       __REG(0xFF6870)	/* Endpoint Control Register 5 */
#define USB_TXD3_REG		       __REG(0xFF6872)	/* Transmit Data Register 3 */
#define USB_TXS3_REG		       __REG(0xFF6874)	/* Transmit Status Register 3 */
#define USB_TXC3_REG		       __REG(0xFF6876)	/* Transmit Command Register 3 */
#define USB_EPC6_REG		       __REG(0xFF6878)	/* Endpoint Control Register 6 */
#define USB_RXD3_REG		       __REG(0xFF687A)	/* Receive Data Register 3 */
#define USB_RXS3_REG		       __REG(0xFF687C)	/* Receive Status Register 3 */
#define USB_RXC3_REG		       __REG(0xFF687E)	/* Receive Command Register 3 */


/*
 * Watchdog Timer
 */
#define WATCHDOG_REG		       __REG(0xFF4C00)	/* Watchdog preset value. */
#define SET_FREEZE_REG		       __REG(0xFF5000)	/* Freeze watchdog, timer1 and DIP during debugging */
#define RESET_FREEZE_REG	       __REG(0xFF5002)	/* Release watchdog, timer1 and DIP during debugging after setting in freeze mode */



#endif

/* end of file */


