/*
 *  Routines to access hardware
 *
 *  $Id: 8190n_hw.c,v 1.1.1.1 2010/05/05 09:00:44 jiunming Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8190N_HW_C_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/unistd.h>

#include "./8190n_cfg.h"
#include "./8190n.h"
#include "./8190n_hw.h"
#include "./8190n_headers.h"
#include "./8190n_debug.h"

#include <linux/syscalls.h>

#define MAX_CONFIG_FILE_SIZE (20*1024) // for 8192, added to 20k

//int rtl8190n_fileopen(const char *filename, int flags, int mode);

struct file* file_open(const char* path, int flags, int rights);
void file_close(struct file* file);
int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size);




void PHY_RF6052SetOFDMTxPower(struct mac80211_shared_priv *priv, unsigned char powerlevel_1ss,
	unsigned char powerlevel_2ss);
static int LoadFirmware(struct mac80211_shared_priv *priv);


unsigned int TxPwrTrk_OFDM_SwingTbl[TxPwrTrk_OFDM_SwingTbl_Len] = {
	/*  +6.0dB */ 0x7f8001fe,
	/*  +5.5dB */ 0x788001e2,
	/*  +5.0dB */ 0x71c001c7,
	/*  +4.5dB */ 0x6b8001ae,
	/*  +4.0dB */ 0x65400195,
	/*  +3.5dB */ 0x5fc0017f,
	/*  +3.0dB */ 0x5a400169,
	/*  +2.5dB */ 0x55400155,
	/*  +2.0dB */ 0x50800142,
	/*  +1.5dB */ 0x4c000130,
	/*  +1.0dB */ 0x47c0011f,
	/*  +0.5dB */ 0x43c0010f,
	/*   0.0dB */ 0x40000100,
	/*  -0.5dB */ 0x3c8000f2,
	/*  -1.0dB */ 0x390000e4,
	/*  -1.5dB */ 0x35c000d7,
	/*  -2.0dB */ 0x32c000cb,
	/*  -2.5dB */ 0x300000c0,
	/*  -3.0dB */ 0x2d4000b5,
	/*  -3.5dB */ 0x2ac000ab,
	/*  -4.0dB */ 0x288000a2,
	/*  -4.5dB */ 0x26000098,
	/*  -5.0dB */ 0x24000090,
	/*  -5.5dB */ 0x22000088,
	/*  -6.0dB */ 0x20000080,
	/*  -6.5dB */ 0x1a00006c,
	/*  -7.0dB */ 0x1c800072,
	/*  -7.5dB */ 0x18000060,
	/*  -8.0dB */ 0x19800066,
	/*  -8.5dB */ 0x15800056,
	/*  -9.0dB */ 0x26c0005b,
	/*  -9.5dB */ 0x14400051,
	/* -10.0dB */ 0x24400051,
	/* -10.5dB */ 0x1300004c,
	/* -11.0dB */ 0x12000048,
	/* -11.5dB */ 0x11000044,
	/* -12.0dB */ 0x10000040
};

unsigned char TxPwrTrk_CCK_SwingTbl[TxPwrTrk_CCK_SwingTbl_Len][8] = {
	/*   0.0dB */ {0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},
	/*   0.5dB */ {0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},
	/*   1.0dB */ {0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},
	/*   1.5dB */ {0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},
	/*   2.0dB */ {0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},
	/*   2.5dB */ {0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},
	/*   3.0dB */ {0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},
	/*   3.5dB */ {0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},
	/*   4.0dB */ {0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},
	/*   4.5dB */ {0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},
	/*   5.0dB */ {0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},
	/*   5.5dB */ {0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},
	/*   6.0dB */ {0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},
	/*   6.5dB */ {0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},
	/*   7.0dB */ {0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},
	/*   7.5dB */ {0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},
	/*   8.0dB */ {0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},
	/*   8.5dB */ {0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},
	/*   9.0dB */ {0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},
	/*   9.5dB */ {0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},
	/*  10.0dB */ {0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},
	/*  10.5dB */ {0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},
	/*  11.0dB */ {0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01}
};

unsigned char TxPwrTrk_CCK_SwingTbl_CH14[TxPwrTrk_CCK_SwingTbl_Len][8] = {
	/*   0.0dB */ {0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},
	/*   0.5dB */ {0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},
	/*   1.0dB */ {0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},
	/*   1.5dB */ {0x2d, 0x2d, 0x27, 0x17, 0x00, 0x00, 0x00, 0x00},
	/*   2.0dB */ {0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},
	/*   2.5dB */ {0x28, 0x28, 0x22, 0x14, 0x00, 0x00, 0x00, 0x00},
	/*   3.0dB */ {0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},
	/*   3.5dB */ {0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},
	/*   4.0dB */ {0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},
	/*   4.5dB */ {0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},
	/*   5.0dB */ {0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},
	/*   5.5dB */ {0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},
	/*   6.0dB */ {0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},
	/*   6.5dB */ {0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},
	/*   7.0dB */ {0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},
	/*   7.5dB */ {0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},
	/*   8.0dB */ {0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},
	/*   8.5dB */ {0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},
	/*   9.0dB */ {0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},
	/*   9.5dB */ {0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},
	/*  10.0dB */ {0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},
	/*  10.5dB */ {0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},
	/*  11.0dB */ {0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00}
};

#ifdef ADD_TX_POWER_BY_CMD
#define ASSIGN_TX_POWER_OFFSET(offset, setting) { \
	if (setting != 0xff) \
		offset = setting; \
}
#endif


#ifdef MERGE_FW
#include "data_rtl8192sfw.c"
#include "data_MACPHY_REG.c"
#include "data_AGC_TAB.c"
#include "data_phy_reg.c"
#include "data_PHY_REG_PG.c"
#include "data_radio_a.c"
#include "data_radio_b.c"
#include "data_PHY_to1T1R.c"
#include "data_PHY_to1T1R_b.c"
#include "data_PHY_to1T2R.c"
#include "data_PHY_to1T2R_b.c"
#include "data_phy_reg_MP.c"
#include "data_radio_a_hp.c"


#define VAR_MAPPING(dst,src) \
	unsigned char *data_##dst##_start = &data_##src[0]; \
	unsigned char *data_##dst##_end   = &data_##src[sizeof(data_##src)]; \

VAR_MAPPING(rtl8192sfw, rtl8192sfw);
VAR_MAPPING(MACPHY_REG, MACPHY_REG);
VAR_MAPPING(AGC_TAB, AGC_TAB);
VAR_MAPPING(phy_reg, phy_reg);
VAR_MAPPING(phy_reg_MP, phy_reg_MP);
VAR_MAPPING(PHY_to1T1R, PHY_to1T1R);
VAR_MAPPING(PHY_to1T1R_b, PHY_to1T1R_b);
VAR_MAPPING(PHY_to1T2R, PHY_to1T2R);
VAR_MAPPING(PHY_to1T2R_b, PHY_to1T2R_b);
VAR_MAPPING(PHY_REG_PG, PHY_REG_PG);
VAR_MAPPING(radio_a, radio_a);
VAR_MAPPING(radio_a_hp, radio_a_hp);
VAR_MAPPING(radio_b, radio_b);
#endif


/**
* Function:	phy_CalculateBitShift
*
* OverView:	Get shifted position of the BitMask
*
* Input:
*			u4Byte		BitMask,
*
* Output:	none
* Return:		u4Byte		Return the shift bit bit position of the mask
*/
unsigned int phy_CalculateBitShift(unsigned int BitMask)
{
	unsigned int i;

	for (i=0; i<=31; i++) {
		if (((BitMask>>i) & 0x1) == 1)
			break;
	}

	return (i);
}


/**
* Function:	PHY_QueryBBReg
*
* OverView:	Read "sepcific bits" from BB register
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be readback
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be readback
* Output:	None
* Return:		u4Byte			Data			//The readback register value
* Note:		This function is equal to "GetRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryBBReg(struct mac80211_shared_priv *priv, unsigned int RegAddr, unsigned int BitMask)
{
  	unsigned int ReturnValue = 0, OriginalValue, BitShift;
	GET_PRIV_IOADDR;

	OriginalValue = RTL_READ_BB(RegAddr);
	BitShift = phy_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	return (ReturnValue);
}


/**
* Function:	PHY_SetBBReg
*
* OverView:	Write "Specific bits" to BB register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register value in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRegSetting" in PHY programming guide
*/
void PHY_SetBBReg(struct mac80211_shared_priv *priv, unsigned int RegAddr, unsigned int BitMask, unsigned int Data)
{
	unsigned int OriginalValue, BitShift, NewValue;
	GET_PRIV_IOADDR;

	if (BitMask != bMaskDWord)
	{//if not "double word" write
		OriginalValue = RTL_READ_BB(RegAddr);
		BitShift = phy_CalculateBitShift(BitMask);
		NewValue = ((OriginalValue & (~BitMask)) | (Data << BitShift));
		RTL_WRITE_BB(RegAddr, NewValue);
	}
	else
		RTL_WRITE_BB(RegAddr, Data);


	return;
}


/**
* Function:	phy_RFSerialWrite
*
* OverView:	Write data to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF90_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target to be read
*
* Output:	None
* Return:		None
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
 *
 * Note: 		  For RF8256 only
 *			 The total count of RTL8256(Zebra4) register is around 36 bit it only employs
 *			 4-bit RF address. RTL8256 uses "register mode control bit" (Reg00[12], Reg00[10])
 *			 to access register address bigger than 0xf. See "Appendix-4 in PHY Configuration
 *			 programming guide" for more details.
 *			 Thus, we define a sub-finction for RTL8526 register address conversion
 *		       ===========================================================
 *			 Register Mode		RegCTL[1]		RegCTL[0]		Note
 *								(Reg00[12])		(Reg00[10])
 *		       ===========================================================
 *			 Reg_Mode0				0				x			Reg 0 ~15(0x0 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode1				1				0			Reg 16 ~30(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode2				1				1			Reg 31 ~ 45(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
*/
void phy_RFSerialWrite(struct mac80211_shared_priv *priv, RF90_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int Data)
{
	struct rtl8190_hw			*phw = GET_HW(priv);
	unsigned int				DataAndAddr = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];

	// Joseph test
	unsigned int				NewOffset;

	Offset &= 0x3f;

	//
	// Switch page for 8256 RF IC
	//
		NewOffset = Offset;

	//
	// Put write addr in [5:0]  and write data in [31:16]
	//
	//DataAndAddr = (Data<<16) | (NewOffset&0x3f);
	DataAndAddr = ((NewOffset<<20) | (Data&0x000fffff)) & 0x0fffffff;	// T65 RF

	//
	// Write Operation
	//
	PHY_SetBBReg(priv, pPhyReg->rf3wireOffset, bMaskDWord, DataAndAddr);


}


/**
* Function:	phy_RFSerialRead
*
* OverView:	Read regster from RF chips
*
* Input:
*			PADAPTER		Adapter,
*			RF90_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			dbg_avoid,	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			reback value
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
*/
unsigned int phy_RFSerialRead(struct mac80211_shared_priv *priv, RF90_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int dbg_avoid)
{
	struct rtl8190_hw			*phw = GET_HW(priv);
	unsigned int 				tmplong, tmplong2;
	unsigned int				retValue = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned long ioaddr = priv->pshare_hw->ioaddr;

	// Joseph test
	unsigned int				NewOffset;

	//
	// Make sure RF register offset is correct
	//
	Offset &= 0x3f;

	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;


	// For 92S LSSI Read RFLSSIRead
	// For RF A/B write 0x824/82c(does not work in the future)
	// We must use 0x824 for RF A and B to execute read trigger
	tmplong = RTL_R32(rFPGA0_XA_HSSIParameter2);
	tmplong2 = RTL_R32(pPhyReg->rfHSSIPara2);
	tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | ((NewOffset<<23) | bLSSIReadEdge);	//T65 RF

	RTL_W32(rFPGA0_XA_HSSIParameter2,tmplong&(~bLSSIReadEdge));
	mdelay(1);
	RTL_W32(pPhyReg->rfHSSIPara2,tmplong2);
	mdelay(1);
	RTL_W32(rFPGA0_XA_HSSIParameter2,tmplong|bLSSIReadEdge);
	mdelay(1);

	//Read from BBreg8a0, 12 bits for 8190, 20 bits for T65 RF
	if ((RTL_R32(0x820)&BIT(8)) && (RTL_R32(0x828)&BIT(8)))
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBack_92S_PI, bLSSIReadBackData);
	else
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBack, bLSSIReadBackData);


	return retValue;
}


/**
* Function:	PHY_QueryRFReg
*
* OverView:	Query "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF90_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be read
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be read
*			u4Byte			dbg_avoid	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			Readback value
* Note:		This function is equal to "GetRFRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryRFReg(struct mac80211_shared_priv *priv, RF90_RADIO_PATH_E eRFPath,
				unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
	unsigned int	Original_Value, Readback_Value, BitShift;

#ifdef RTL8192SU
	Original_Value = phy_QueryUsbRFReg(priv, eRFPath, RegAddr);	
#else
	Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, dbg_avoid);
#endif
   	BitShift =  phy_CalculateBitShift(BitMask);
   	Readback_Value = (Original_Value & BitMask) >> BitShift;

	return (Readback_Value);
}


/**
* Function:	PHY_SetRFReg
*
* OverView:	Write "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF90_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRFRegSetting" in PHY programming guide
*/
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void PHY_SetRFReg(struct mac80211_shared_priv *priv, RF90_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				unsigned int BitMask, unsigned int Data)
{
	unsigned int Original_Value, BitShift, New_Value;
	

#ifdef RTL8192SU

	if (BitMask != bMask20Bits) // RF data is 12 bits only
	{
		Original_Value = phy_QueryUsbRFReg(priv, eRFPath, RegAddr);
		BitShift =  phy_CalculateBitShift(BitMask);
		New_Value = (((Original_Value)&(~BitMask))|(Data<< BitShift));
		phy_SetUsbRFReg(priv, eRFPath, RegAddr, New_Value);
	}
	else
	{
		phy_SetUsbRFReg(priv, eRFPath, RegAddr, Data);
	}

#else

	unsigned long flags;

	SAVE_INT_AND_CLI(flags);


   	if (BitMask != bMask20Bits) // RF data is 12 bits only
   	{
		Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, 1);
		BitShift = phy_CalculateBitShift(BitMask);
		New_Value = ((Original_Value & (~BitMask)) | (Data<< BitShift));

		phy_RFSerialWrite(priv, eRFPath, RegAddr, New_Value);
	}
	else
		phy_RFSerialWrite(priv, eRFPath, RegAddr, Data);

	RESTORE_INT(flags);
	
#endif

}


static int is_hex(char s)
{
	if (( s >= '0' && s <= '9') || ( s >= 'a' && s <= 'f') || (s >= 'A' && s <= 'F') || (s == 'x' || s == 'X'))
		return 1;
	else
		return 0;
}


static unsigned char *get_digit(unsigned char **data)
{
	unsigned char *buf=*data;
	int i=0;

	while(buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;
	*data = &buf[i];

	while(buf[i]) {
		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		if (!is_hex(buf[i]))
			return NULL;
		i++;
	}
	if (i == 0)
		return NULL;
	else
		return &buf[i+1];
}


static int get_offset_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bRegValue)
{
	unsigned char *next;
	int base, idx;
	int num=0;
	extern int _atoi(char *s, int base);

	*u4bRegOffset = *u4bRegValue = '\0';

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	}
	else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi(&line_head[idx], base);

	if (next) {
		if (!get_digit(&next))
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		}
		else {
			base = 10;
			idx = 0;
		}
		*u4bRegValue = _atoi(&next[idx], base);
	}
	else
		*u4bRegValue = 0;

	return num;
}


static int get_offset_mask_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	unsigned char *next, *n1;
	int base, idx;
	extern int _atoi(char *s, int base);
	int num=0;

	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	}
	else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi(&line_head[idx], base);

	if (next) {
		n1 = get_digit(&next);
		if (n1 == NULL)
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		}
		else {
			base = 10;
			idx = 0;
		}
		*u4bMask = _atoi(&next[idx], base);

		if (n1) {
			if (!get_digit(&n1))
				return num;
			num++;
			if ((!memcmp(n1, "0x", 2)) || (!memcmp(n1, "0X", 2))) {
				base = 16;
				idx = 2;
			}
			else {
				base = 10;
				idx = 0;
			}
			*u4bRegValue = _atoi(&n1[idx], base);
		}
		else
			*u4bRegValue = 0;
	}
	else
		*u4bMask = 0;

	return num;
}


unsigned char *get_line(unsigned char **line)
{
	unsigned char *p=*line;

	while (*p && ((*p == '\n') || (*p == '\r')))
		p++;

	if (*p == '\0') {
		*line = NULL;
		return NULL;
	}
	*line = p;

	while (*p && *p != '\n' && *p != '\r' )
			p++;

	*p = '\0';
	return p+1;
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigBBWithParaFile()
 *
 * Overview:    This function read BB parameters from general file format, and do register
 *			  Read/Write
 *
 * Input:      	PADAPTER		Adapter
 *			ps1Byte 			pFileName
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 *---------------------------------------------------------------------------*/
int PHY_ConfigBBWithParaFile(struct mac80211_shared_priv *priv, int reg_file)
{
	int           read_bytes=0, num, len=0;
#ifndef MERGE_FW
	//int           fd=0;
	//mm_segment_t  old_fs;
	struct file* fp;
	unsigned char *pFileName=NULL;
#endif
	unsigned int  u4bRegOffset, u4bRegValue, u4bRegMask;
	unsigned char *mem_ptr, *line_head, *next_head=NULL;
	struct PhyRegTable *reg_table=NULL;
	struct MacRegTable *pg_reg_table = NULL;
	unsigned short max_len=0;
	int file_format = TWO_COLUMN;
#if !defined(CONFIG_X86) && !defined(MERGE_FW)
	extern long sys_read(unsigned int fd, char * buf, size_t count);
#endif

#ifdef MERGE_FW
	if (reg_file == AGCTAB) {
		reg_table = (struct PhyRegTable *)priv->pshare_hw->agc_tab_buf;
		next_head = data_AGC_TAB_start;
		read_bytes = (int)( data_AGC_TAB_end - data_AGC_TAB_start);
		max_len = AGC_TAB_SIZE;
	}
	else if (reg_file == PHYREG_PG){
		pg_reg_table = (struct MacRegTable *)priv->pshare_hw->phy_reg_pg_buf;
		next_head = data_PHY_REG_PG_start;
		read_bytes = (int)( data_PHY_REG_PG_end - data_PHY_REG_PG_start);
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
	}
	else if (reg_file == PHYREG_1T2R){
		if(priv->pshare_hw->rf_ft_var.pathB_1T == 0){//PATH A
			pg_reg_table = (struct MacRegTable *)priv->pshare_hw->phy_reg_2to1;
			next_head = data_PHY_to1T2R_start;
			read_bytes = (int)( data_PHY_to1T2R_end - data_PHY_to1T2R_start);
			max_len = PHY_REG_1T2R;
			file_format = THREE_COLUMN;
		}else{//PATH B

			pg_reg_table = (struct MacRegTable *)priv->pshare_hw->phy_reg_2to1;
			next_head = data_PHY_to1T2R_b_start;
			read_bytes = (int)( data_PHY_to1T2R_b_end - data_PHY_to1T2R_b_start);
			max_len = PHY_REG_1T2R;
			file_format = THREE_COLUMN;
			printk("PHY_to1T2R_b\n");
		}
	}else if(reg_file == PHYREG_1T1R){
		if(priv->pshare_hw->rf_ft_var.pathB_1T == 0){
			pg_reg_table = (struct MacRegTable *)priv->pshare_hw->phy_reg_2to1;
			next_head = data_PHY_to1T1R_start;
			read_bytes = (int)( data_PHY_to1T1R_end - data_PHY_to1T1R_start);
			max_len = PHY_REG_1T1R;
			file_format = THREE_COLUMN;
			printk("PHY_to1T1R\n");
		}else{//PATH B
			pg_reg_table = (struct MacRegTable *)priv->pshare_hw->phy_reg_2to1;
			next_head = data_PHY_to1T1R_b_start;
			read_bytes = (int)( data_PHY_to1T1R_b_end - data_PHY_to1T1R_b_start);
			max_len = PHY_REG_1T1R;
			file_format = THREE_COLUMN;
			printk("PHY_to1T1R_b\n");
		}
	}
	else {
#if defined(RTL8192SE) && defined(MP_TEST)
		if (priv->pshare_hw->rf_ft_var.mp_specific) {
			reg_table = (struct PhyRegTable *)priv->pshare_hw->phy_reg_mp_buf;
			next_head = data_phy_reg_MP_start;
			read_bytes = (int)( data_phy_reg_MP_end - data_phy_reg_MP_start);
		}
		else
#endif
		{
		reg_table = (struct PhyRegTable *)priv->pshare_hw->phy_reg_buf;
		next_head = data_phy_reg_start;
		read_bytes = (int)( data_phy_reg_end - data_phy_reg_start);
		}
		max_len = PHY_REG_SIZE;
	}
#else

	switch (reg_file) {
	case AGCTAB:

#if defined(RTL8192SE) || defined(RTL8192SU)
		pFileName = "/usr/rtl8192Pci/AGC_TAB.txt";
#endif
		reg_table = (struct PhyRegTable *)priv->pshare_hw->agc_tab_buf;
		max_len = AGC_TAB_SIZE;
		break;
	case PHYREG_1T2R:

#if defined(RTL8192SE) || defined(RTL8192SU)
		if(priv->pshare_hw->rf_ft_var.pathB_1T == 0)
			pFileName = "/usr/rtl8192Pci/PHY_to1T2R.txt";
		else
			pFileName = "/usr/rtl8192Pci/PHY_to1T2R_b.txt";
		pg_reg_table = (struct MacRegTable *)priv->pshare_hw->phy_reg_pg_buf;
		file_format = THREE_COLUMN;
#endif
		max_len = PHY_REG_SIZE;
		break;
	case PHYREG_2T4R:
#if defined(RTL8192SE) || defined(RTL8192SU)
		pFileName = "/usr/rtl8192Pci/phy_reg.txt";
#endif
		reg_table = (struct PhyRegTable *)priv->pshare_hw->phy_reg_buf;
		max_len = PHY_REG_SIZE;
		break;
	case PHYREG_2T2R:
#if defined(RTL8192SE) || defined(RTL8192SU)
		pFileName = "/usr/rtl8192Pci/phy_reg.txt";
#endif
		reg_table = (struct PhyRegTable *)priv->pshare_hw->phy_reg_buf;
		max_len = PHY_REG_SIZE;
		break;
	case PHYREG_1T1R:

#if defined(RTL8192SE) || defined(RTL8192SU)
		if(priv->pshare_hw->rf_ft_var.pathB_1T == 0)
			pFileName = "/usr/rtl8192Pci/PHY_to1T1R.txt";
		else
			pFileName = "/usr/rtl8192Pci/PHY_to1T1R_b.txt";
		pg_reg_table = (struct MacRegTable *)priv->pshare_hw->phy_reg_pg_buf;
		file_format = THREE_COLUMN;
#endif
		max_len = PHY_REG_SIZE;
		break;
#if defined(RTL8192SE) || defined(RTL8192SU)
	case PHYREG_PG:
		pFileName = "/usr/rtl8192Pci/PHY_REG_PG.txt";
		pg_reg_table = (struct MacRegTable *)priv->pshare_hw->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
		break;
#endif
	}
#endif // MERGE_FW

//#ifdef CHECK_HANGUP
	//if (!priv->reset_hangup)
//#endif
	{
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

#ifdef MERGE_FW
		memcpy(mem_ptr, next_head, read_bytes);
#else
/*
		old_fs = get_fs();
		set_fs(KERNEL_DS);

#if	defined(CONFIG_X86)
		if ((fd = rtl8190n_fileopen(pFileName, O_RDONLY, 0)) < 0)
#else
		if ((fd = sys_open(pFileName, O_RDONLY, 0)) < 0)
#endif
		
*/

		if ((fp=file_open(pFileName, O_RDONLY, 0))==NULL)
		{
			printk("PHY_ConfigBBWithParaFile(): cannot open %s\n", pFileName);
			//set_fs(old_fs);
			kfree(mem_ptr);
			return -1;
		}
			
		//read_bytes = sys_read(fd, mem_ptr, MAX_CONFIG_FILE_SIZE);

		read_bytes = file_read(fp, 0, mem_ptr, MAX_CONFIG_FILE_SIZE);
		
#endif // MERGE_FW

		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					reg_table[len].offset = u4bRegOffset;
					reg_table[len].value = u4bRegValue;
					len++;
					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			}
			else {
				num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask ,&u4bRegValue);
				if (num > 0) {
					pg_reg_table[len].offset = u4bRegOffset;
					pg_reg_table[len].mask = u4bRegMask;
					pg_reg_table[len].value = u4bRegValue;
					len++;
					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;
				}
			}
		}

#ifndef MERGE_FW
		//sys_close(fd);
		//set_fs(old_fs);
		file_close(fp);
#endif

		kfree(mem_ptr);

		if ((len * sizeof(struct PhyRegTable)) > max_len) {
#ifdef MERGE_FW
			printk("PHY REG table buffer not large enough!\n");
#else
			printk("PHY REG table buffer not large enough! (%s)\n", pFileName);
#endif
			return -1;
		}
	}

	num = 0;
	while (1) {
		if (file_format == THREE_COLUMN) {
			u4bRegOffset = pg_reg_table[num].offset;
			u4bRegValue = pg_reg_table[num].value;
			u4bRegMask = pg_reg_table[num].mask;
		}
		else {
			u4bRegOffset = reg_table[num].offset;
			u4bRegValue = reg_table[num].value;
		}

		if (u4bRegOffset == 0xff)
			break;
		else if (file_format == THREE_COLUMN)
			PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
		else
			PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
		num++;
	}

	return 0;
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigRFWithParaFile()
 *
 * Overview:    This function read RF parameters from general file format, and do RF 3-wire
 *
 * Input:      	PADAPTER			Adapter
 *			ps1Byte 				pFileName
 *			RF90_RADIO_PATH_E	eRFPath
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 * Note:		Delay may be required for RF configuration
 *---------------------------------------------------------------------------*/
#ifdef MERGE_FW
int PHY_ConfigRFWithParaFile(struct mac80211_shared_priv *priv, unsigned char *start, int read_bytes, RF90_RADIO_PATH_E eRFPath)
#else
int PHY_ConfigRFWithParaFile(struct mac80211_shared_priv *priv, unsigned char *pFileName, RF90_RADIO_PATH_E eRFPath)
#endif
{
	int           num;
#ifndef MERGE_FW
	//int           fd=0, read_bytes;
	int		read_bytes;
	struct file* fp;

	//mm_segment_t  old_fs;
#endif
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;
#if !defined(CONFIG_X86) && !defined(MERGE_FW)
	extern long sys_read(unsigned int fd, char * buf, size_t count);
#endif

	if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {

		printk("PHY_ConfigRFWithParaFile(): not enough memory\n");
		return -1;
	}

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

#ifdef MERGE_FW
	memcpy(mem_ptr, start, read_bytes);
#else
/*
	old_fs = get_fs();
	set_fs(KERNEL_DS);
#if	defined(CONFIG_X86)
	if ((fd = rtl8190n_fileopen(pFileName, O_RDONLY, 0)) < 0)
#else
	if ((fd = sys_open(pFileName, O_RDONLY, 0)) < 0)
#endif*/
	if ((fp=file_open(pFileName, O_RDONLY, 0))==NULL)
	{
		printk("PHY_ConfigRFWithParaFile(): cannot open %s\n", pFileName);
		//set_fs(old_fs);
		kfree(mem_ptr);
		return -1;
	}

	//read_bytes = sys_read(fd, mem_ptr, MAX_CONFIG_FILE_SIZE);
	read_bytes = file_read(fp, 0, mem_ptr, MAX_CONFIG_FILE_SIZE);
#endif // MERGE_FW

	next_head = mem_ptr;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);
		if (line_head == NULL)
			break;

		if (line_head[0] == '/')
			continue;

		num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
		if (num > 0) {
			if (u4bRegOffset == 0xff)
				break;
			else if (u4bRegOffset == 0xfe)
				mdelay(50);	// Delay 50 ms. Only RF configuration require delay
			else if (num == 2) {
#if	defined(RTL8190) || defined(RTL8192E)
				PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask12Bits, u4bRegValue);
#elif defined(RTL8192SE) || defined(RTL8192SU)
				PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask20Bits, u4bRegValue);
#endif
				mdelay(1);
			}
		}
	}

#ifndef MERGE_FW
	//sys_close(fd);
	//set_fs(old_fs);
	file_close(fp);
#endif

	kfree(mem_ptr);

	return 0;
}


int PHY_ConfigMACWithParaFile(struct mac80211_shared_priv *priv)
{
	int  read_bytes, num, len=0;
	unsigned int  u4bRegOffset, u4bRegMask, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;

	unsigned long ioaddr = priv->pshare_hw->ioaddr;

#ifdef MERGE_FW
	
#else
	//int fd=0;
	//mm_segment_t  old_fs;
	struct file* fp;


	unsigned char *pFileName = "/usr/rtl8192Pci/MACPHY_REG.txt";


#ifndef CONFIG_X86
	extern long sys_read(unsigned int fd, char * buf, size_t count);
#endif
#endif

	struct MacRegTable *reg_table = (struct MacRegTable *)priv->pshare_hw->mac_reg_buf;

	{
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

#ifdef MERGE_FW
		read_bytes = (int)(data_MACPHY_REG_end - data_MACPHY_REG_start);
		memcpy(mem_ptr, data_MACPHY_REG_start, read_bytes);
#else
/*
		old_fs = get_fs();
		set_fs(KERNEL_DS);

#if	defined(CONFIG_X86)
		if ((fd = rtl8190n_fileopen(pFileName, O_RDONLY, 0)) < 0)
#else
		if ((fd = sys_open(pFileName, O_RDONLY, 0)) < 0)
#endif*/
		if ((fp=file_open(pFileName, O_RDONLY, 0))==NULL)
		{
			printk("PHY_ConfigMACWithParaFile(): cannot open %s\n", pFileName);
			//set_fs(old_fs);
			kfree(mem_ptr);
			return -1;
		}

		//read_bytes = sys_read(fd, mem_ptr, MAX_CONFIG_FILE_SIZE);
		
		read_bytes = file_read(fp, 0, mem_ptr, MAX_CONFIG_FILE_SIZE);
		
#endif // MERGE_FW

		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask, &u4bRegValue);
			if (num > 0) {
				reg_table[len].offset = u4bRegOffset;
				reg_table[len].mask = u4bRegMask;
				reg_table[len].value = u4bRegValue;
				len++;
				if (u4bRegOffset == 0xff)
					break;
				if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE)
					break;
			}
		}

#ifndef MERGE_FW
		//sys_close(fd);
		//set_fs(old_fs);
		file_close(fp);
#endif

		kfree(mem_ptr);

		if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE) {
			printk("MAC REG table buffer not large enough!\n");
			return -1;
		}
	}

	num = 0;
	while (1) {
		u4bRegOffset = reg_table[num].offset;
		u4bRegMask = reg_table[num].mask;
		u4bRegValue = reg_table[num].value;

		if (u4bRegOffset == 0xff)
			break;
		else
			RTL_W8(u4bRegOffset, u4bRegValue);

		num++;
	}

	return 0;
}


#ifdef UNIVERSAL_REPEATER
static struct rtl8190_priv *get_another_interface_priv(struct rtl8190_priv *priv)
{
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		return GET_VXD_PRIV(priv);
	else if (IS_DRV_OPEN(GET_ROOT_PRIV(priv)))
		return GET_ROOT_PRIV(priv);
	else
		return NULL;
}


static int get_shortslot_for_another_interface(struct rtl8190_priv *priv)
{
	struct rtl8190_priv *p_priv;

	p_priv = get_another_interface_priv(priv);
	if (p_priv) {
		if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE)
			return (p_priv->pmib->dot11ErpInfo.shortSlot);
		else {
			if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)
				return (p_priv->pmib->dot11ErpInfo.shortSlot);
		}
	}
	return -1;
}
#endif // UNIVERSAL_REPEATER


void set_slot_time(struct rtl8190_priv *priv, int use_short)
{
	ULONG ioaddr = priv->pshare->ioaddr;

#ifdef UNIVERSAL_REPEATER
	int is_short;
	is_short = get_shortslot_for_another_interface(priv);
	if (is_short != -1) { // not abtained
		use_short &= is_short;
	}
#endif

	if (use_short)
		RTL_W8(_SLOT_, 0x09);
	else
		RTL_W8(_SLOT_, 0x14);
}



void SwChnl(struct mac80211_shared_priv *priv, unsigned char channel, int offset)
{

	static unsigned int val;


	val = channel;

	if (channel > 14)
		priv->pshare_hw->curr_band = BAND_5G;
	else
		priv->pshare_hw->curr_band = BAND_2G;

	if (priv->pshare_hw->CurrentChannelBW) {
		if (offset == 1)
			val -= 2;
		else
			val += 2;
	}

	if (priv->pshare_hw->rf_ft_var.use_frq_2_3G)
		val += 14;

	{
//		return ; // for 40Mhz
		unsigned int val_read;
		val_read = PHY_QueryRFReg(priv, 0, 0x18, bMask12Bits, 1);
		val_read &= 0xfffffff0;
		PHY_SetRFReg(priv, 0, 0x18, bMask12Bits, val_read | val);
		PHY_SetRFReg(priv, 1, 0x18, bMask12Bits, val_read | val);

	}

	if (priv->pshare_hw->rf_ft_var.use_frq_2_3G)
		val -= 14;

	priv->pshare_hw->working_channel = val;
	//HY_RF6052SetOFDMTxPower(priv, priv->pmib->dot11RFEntry.pwrlevelOFDM_1SS[priv->pshare_hw->working_channel-1],
	//riv->pmib->dot11RFEntry.pwrlevelOFDM_2SS[priv->pshare_hw->working_channel-1]);

	PHY_RF6052SetOFDMTxPower(priv, 0x28, 0x28 );	
/*
	if (!priv->pshare_hw->use_default_para) {
#ifdef ADD_TX_POWER_BY_CMD
		if (priv->pshare_hw->rf_ft_var.txPowerPlus_cck != 0xff)
			PHY_SetBBReg(priv, rTxAGC_CCK_Mcs32, bTxAGCRateCCK, priv->pmib->dot11RFEntry.pwrlevelCCK[priv->pshare->working_channel-1]+priv->pshare->rf_ft_var.txPowerPlus_cck);
		else
#endif
			PHY_SetBBReg(priv, rTxAGC_CCK_Mcs32, bTxAGCRateCCK, priv->pmib->dot11RFEntry.pwrlevelCCK[priv->pshare->working_channel-1]+priv->pshare->phw->CCKTxAgc);
	}
*/
	return;
}


void SwitchAntenna(struct rtl8190_priv *priv)
{
}

#if defined(RTL8192SE) || defined(RTL8192SU)


#ifdef EXT_ANT_DVRY
// SwitchExtAnt(), EXT_ANT_PATH specifies which external antennna is been used.
// EXT_ANT_PATH: 0 -- default, 1 -- extra ant
void SwitchExtAnt(struct rtl8190_priv *priv, unsigned char EXT_ANT_PATH){

	ULONG ioaddr = priv->pshare->ioaddr;
//	panic_printk("EXT_ANT_PATH = %d\n", EXT_ANT_PATH);

	if(EXT_ANT_PATH == 0){
		RTL_W32(0x2ec, 0xff0400);
		priv->pshare->EXT_AD_selection = 0;
	}
	else{
		RTL_W32(0x2ec, 0xff0800);
		priv->pshare->EXT_AD_selection = 1;
	}
}
#endif

// switch 1 spatial stream path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_1SS_Antenna(struct mac80211_shared_priv *priv, unsigned int antPath )
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	unsigned int dword = 0;
	if(get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	switch(antPath){
	case 1:
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x01100000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x01100000; // Path A
		RTL_W32(0x90C, dword);
		break;
	case 2:
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x02200000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x02200000;	// Path B
		RTL_W32(0x90C, dword);
		break;

	case 3:
		if(priv->pshare_hw->rf_ft_var.ofdm_1ss_oneAnt == 1)// use one ANT for 1ss
			goto switch_1ss_end;// do nothing
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x03300000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x03300000; // Path A,B
		RTL_W32(0x90C, dword);
		break;

	default:// do nothing
		break;
	}
switch_1ss_end:
	return;

}

// switch OFDM path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_OFDM_Antenna(struct mac80211_shared_priv *priv, unsigned int antPath )
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	unsigned int dword = 0;
	if(get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	switch(antPath){
	case 1:
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000010)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000010; // Path A
		RTL_W32(0x90C, dword);
		break;
	case 2:
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000020)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000020;	// Path B
		RTL_W32(0x90C, dword);
		break;

	case 3:
		if(priv->pshare_hw->rf_ft_var.ofdm_1ss_oneAnt == 1)// use one ANT for 1ss
			goto switch_OFDM_end;// do nothing
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000030)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000030; // Path A,B
		RTL_W32(0x90C, dword);
		break;

	default:// do nothing
		break;
	}
switch_OFDM_end:
	return;

}


#endif

void enable_hw_LED(struct rtl8190_priv *priv, unsigned int led_type)
{
	ULONG ioaddr = priv->pshare->ioaddr;

#if defined(RTL8192SE)
	switch (led_type) {
	case LEDTYPE_HW_TX_RX:
		RTL_W8(LEDCFG, 0x64);
		break;
	case LEDTYPE_HW_LINKACT_INFRA:
		RTL_W8(LEDCFG, 0x02);
		if ((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_STATION_STATE))
			RTL_W8(LEDCFG, RTL_R8(LEDCFG) & 0x0f);
		else
			RTL_W8(LEDCFG, RTL_R8(LEDCFG) | 0x80);
		break;
	default:
		break;
	}
#endif
}


/**
* Function:	phy_InitBBRFRegisterDefinition
*
* OverView:	Initialize Register definition offset for Radio Path A/B/C/D
*
* Input:
*			PADAPTER		Adapter,
*
* Output:	None
* Return:		None
* Note:		The initialization value is constant and it should never be changes
*/
void phy_InitBBRFRegisterDefinition(struct mac80211_shared_priv *priv)
{
	struct rtl8190_hw *phw = GET_HW(priv);

	// RF Interface Sowrtware Control
	phw->PHYRegDef[RF90_PATH_A].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 LSBs if read 32-bit from 0x870
	phw->PHYRegDef[RF90_PATH_B].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 MSBs if read 32-bit from 0x870 (16-bit for 0x872)
	phw->PHYRegDef[RF90_PATH_C].rfintfs = rFPGA0_XCD_RFInterfaceSW;// 16 LSBs if read 32-bit from 0x874
	phw->PHYRegDef[RF90_PATH_D].rfintfs = rFPGA0_XCD_RFInterfaceSW;// 16 MSBs if read 32-bit from 0x874 (16-bit for 0x876)

	// RF Interface Readback Value
	phw->PHYRegDef[RF90_PATH_A].rfintfi = rFPGA0_XAB_RFInterfaceRB; // 16 LSBs if read 32-bit from 0x8E0
	phw->PHYRegDef[RF90_PATH_B].rfintfi = rFPGA0_XAB_RFInterfaceRB;// 16 MSBs if read 32-bit from 0x8E0 (16-bit for 0x8E2)
	phw->PHYRegDef[RF90_PATH_C].rfintfi = rFPGA0_XCD_RFInterfaceRB;// 16 LSBs if read 32-bit from 0x8E4
	phw->PHYRegDef[RF90_PATH_D].rfintfi = rFPGA0_XCD_RFInterfaceRB;// 16 MSBs if read 32-bit from 0x8E4 (16-bit for 0x8E6)

	// RF Interface Output (and Enable)
	phw->PHYRegDef[RF90_PATH_A].rfintfo = rFPGA0_XA_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x860
	phw->PHYRegDef[RF90_PATH_B].rfintfo = rFPGA0_XB_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x864
	phw->PHYRegDef[RF90_PATH_C].rfintfo = rFPGA0_XC_RFInterfaceOE;// 16 LSBs if read 32-bit from 0x868
	phw->PHYRegDef[RF90_PATH_D].rfintfo = rFPGA0_XD_RFInterfaceOE;// 16 LSBs if read 32-bit from 0x86C

	// RF Interface (Output and)  Enable
	phw->PHYRegDef[RF90_PATH_A].rfintfe = rFPGA0_XA_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x860 (16-bit for 0x862)
	phw->PHYRegDef[RF90_PATH_B].rfintfe = rFPGA0_XB_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x864 (16-bit for 0x866)
	phw->PHYRegDef[RF90_PATH_C].rfintfe = rFPGA0_XC_RFInterfaceOE;// 16 MSBs if read 32-bit from 0x86A (16-bit for 0x86A)
	phw->PHYRegDef[RF90_PATH_D].rfintfe = rFPGA0_XD_RFInterfaceOE;// 16 MSBs if read 32-bit from 0x86C (16-bit for 0x86E)

	//Addr of LSSI. Wirte RF register by driver
	phw->PHYRegDef[RF90_PATH_A].rf3wireOffset = rFPGA0_XA_LSSIParameter; //LSSI Parameter
	phw->PHYRegDef[RF90_PATH_B].rf3wireOffset = rFPGA0_XB_LSSIParameter;
	phw->PHYRegDef[RF90_PATH_C].rf3wireOffset = rFPGA0_XC_LSSIParameter;
	phw->PHYRegDef[RF90_PATH_D].rf3wireOffset = rFPGA0_XD_LSSIParameter;

	// RF parameter
	phw->PHYRegDef[RF90_PATH_A].rfLSSI_Select = rFPGA0_XAB_RFParameter;  //BB Band Select
	phw->PHYRegDef[RF90_PATH_B].rfLSSI_Select = rFPGA0_XAB_RFParameter;
	phw->PHYRegDef[RF90_PATH_C].rfLSSI_Select = rFPGA0_XCD_RFParameter;
	phw->PHYRegDef[RF90_PATH_D].rfLSSI_Select = rFPGA0_XCD_RFParameter;

	// Tx AGC Gain Stage (same for all path. Should we remove this?)
	phw->PHYRegDef[RF90_PATH_A].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage
	phw->PHYRegDef[RF90_PATH_B].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage
	phw->PHYRegDef[RF90_PATH_C].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage
	phw->PHYRegDef[RF90_PATH_D].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage

	// Tranceiver A~D HSSI Parameter-1
	phw->PHYRegDef[RF90_PATH_A].rfHSSIPara1 = rFPGA0_XA_HSSIParameter1;  //wire control parameter1
	phw->PHYRegDef[RF90_PATH_B].rfHSSIPara1 = rFPGA0_XB_HSSIParameter1;  //wire control parameter1
	phw->PHYRegDef[RF90_PATH_C].rfHSSIPara1 = rFPGA0_XC_HSSIParameter1;  //wire control parameter1
	phw->PHYRegDef[RF90_PATH_D].rfHSSIPara1 = rFPGA0_XD_HSSIParameter1;  //wire control parameter1

	// Tranceiver A~D HSSI Parameter-2
	phw->PHYRegDef[RF90_PATH_A].rfHSSIPara2 = rFPGA0_XA_HSSIParameter2;  //wire control parameter2
	phw->PHYRegDef[RF90_PATH_B].rfHSSIPara2 = rFPGA0_XB_HSSIParameter2;  //wire control parameter2
	phw->PHYRegDef[RF90_PATH_C].rfHSSIPara2 = rFPGA0_XC_HSSIParameter2;  //wire control parameter2
	phw->PHYRegDef[RF90_PATH_D].rfHSSIPara2 = rFPGA0_XD_HSSIParameter2;  //wire control parameter1

	// RF switch Control
	phw->PHYRegDef[RF90_PATH_A].rfSwitchControl = rFPGA0_XAB_SwitchControl; //TR/Ant switch control
	phw->PHYRegDef[RF90_PATH_B].rfSwitchControl = rFPGA0_XAB_SwitchControl;
	phw->PHYRegDef[RF90_PATH_C].rfSwitchControl = rFPGA0_XCD_SwitchControl;
	phw->PHYRegDef[RF90_PATH_D].rfSwitchControl = rFPGA0_XCD_SwitchControl;

	// AGC control 1
	phw->PHYRegDef[RF90_PATH_A].rfAGCControl1 = rOFDM0_XAAGCCore1;
	phw->PHYRegDef[RF90_PATH_B].rfAGCControl1 = rOFDM0_XBAGCCore1;
	phw->PHYRegDef[RF90_PATH_C].rfAGCControl1 = rOFDM0_XCAGCCore1;
	phw->PHYRegDef[RF90_PATH_D].rfAGCControl1 = rOFDM0_XDAGCCore1;

	// AGC control 2
	phw->PHYRegDef[RF90_PATH_A].rfAGCControl2 = rOFDM0_XAAGCCore2;
	phw->PHYRegDef[RF90_PATH_B].rfAGCControl2 = rOFDM0_XBAGCCore2;
	phw->PHYRegDef[RF90_PATH_C].rfAGCControl2 = rOFDM0_XCAGCCore2;
	phw->PHYRegDef[RF90_PATH_D].rfAGCControl2 = rOFDM0_XDAGCCore2;

	// RX AFE control 1
	phw->PHYRegDef[RF90_PATH_A].rfRxIQImbalance = rOFDM0_XARxIQImbalance;
	phw->PHYRegDef[RF90_PATH_B].rfRxIQImbalance = rOFDM0_XBRxIQImbalance;
	phw->PHYRegDef[RF90_PATH_C].rfRxIQImbalance = rOFDM0_XCRxIQImbalance;
	phw->PHYRegDef[RF90_PATH_D].rfRxIQImbalance = rOFDM0_XDRxIQImbalance;

	// RX AFE control 1
	phw->PHYRegDef[RF90_PATH_A].rfRxAFE = rOFDM0_XARxAFE;
	phw->PHYRegDef[RF90_PATH_B].rfRxAFE = rOFDM0_XBRxAFE;
	phw->PHYRegDef[RF90_PATH_C].rfRxAFE = rOFDM0_XCRxAFE;
	phw->PHYRegDef[RF90_PATH_D].rfRxAFE = rOFDM0_XDRxAFE;

	// Tx AFE control 1
	phw->PHYRegDef[RF90_PATH_A].rfTxIQImbalance = rOFDM0_XATxIQImbalance;
	phw->PHYRegDef[RF90_PATH_B].rfTxIQImbalance = rOFDM0_XBTxIQImbalance;
	phw->PHYRegDef[RF90_PATH_C].rfTxIQImbalance = rOFDM0_XCTxIQImbalance;
	phw->PHYRegDef[RF90_PATH_D].rfTxIQImbalance = rOFDM0_XDTxIQImbalance;

	// Tx AFE control 2
	phw->PHYRegDef[RF90_PATH_A].rfTxAFE = rOFDM0_XATxAFE;
	phw->PHYRegDef[RF90_PATH_B].rfTxAFE = rOFDM0_XBTxAFE;
	phw->PHYRegDef[RF90_PATH_C].rfTxAFE = rOFDM0_XCTxAFE;
	phw->PHYRegDef[RF90_PATH_D].rfTxAFE = rOFDM0_XDTxAFE;

	// Tranceiver LSSI Readback
	phw->PHYRegDef[RF90_PATH_A].rfLSSIReadBack = rFPGA0_XA_LSSIReadBack;
	phw->PHYRegDef[RF90_PATH_B].rfLSSIReadBack = rFPGA0_XB_LSSIReadBack;
	phw->PHYRegDef[RF90_PATH_C].rfLSSIReadBack = rFPGA0_XC_LSSIReadBack;
	phw->PHYRegDef[RF90_PATH_D].rfLSSIReadBack = rFPGA0_XD_LSSIReadBack;
#if defined(RTL8192SE) || defined(RTL8192SU)
	phw->PHYRegDef[RF90_PATH_A].rfLSSIReadBack_92S_PI = 0x8b8;
	phw->PHYRegDef[RF90_PATH_B].rfLSSIReadBack_92S_PI = 0x8bc;
#endif
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_CheckBBAndRFOK()
 *
 * Overview:    This function is write register and then readback to make sure whether
 *			  BB[PHY0, PHY1], RF[Patha, path b, path c, path d] is Ok
 *
 * Input:      	PADAPTER			Adapter
 *			HW90_BLOCK_E		CheckBlock
 *			RF90_RADIO_PATH_E	eRFPath		// it is used only when CheckBlock is HW90_BLOCK_RF
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: PHY is OK
 *
 * Note:		This function may be removed in the ASIC
 *---------------------------------------------------------------------------*/
int PHY_CheckBBAndRFOK(struct rtl8190_priv *priv, HW90_BLOCK_E CheckBlock, RF90_RADIO_PATH_E eRFPath)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	int rtStatus = 0;
	unsigned int i, CheckTimes = 4, ulRegRead = 0;
	unsigned int WriteAddr[4];
	unsigned int WriteData[] = {0xfffff027, 0xaa55a02f, 0x00000027, 0x55aa502f};

	// Initialize register address offset to be checked
	WriteAddr[HW90_BLOCK_MAC] = 0x100;
	WriteAddr[HW90_BLOCK_PHY0] = 0x900;
	WriteAddr[HW90_BLOCK_PHY1] = 0x800;
	WriteAddr[HW90_BLOCK_RF] = 0x3;

	for(i=0; i<CheckTimes; i++)
	{
		//
		// Write Data to register and readback
		//
		switch(CheckBlock)
		{
		case HW90_BLOCK_MAC:
			break;

		case HW90_BLOCK_PHY0:
		case HW90_BLOCK_PHY1:
			RTL_W32(WriteAddr[CheckBlock], WriteData[i]);
			ulRegRead = RTL_R32(WriteAddr[CheckBlock]);
			break;

		case HW90_BLOCK_RF:
			WriteData[i] &= 0xfff;
			PHY_SetRFReg(priv->share_priv, eRFPath, WriteAddr[HW90_BLOCK_RF], bMaskDWord, WriteData[i]);
			// TODO: we should not delay for such a long time. Ask SD3
			mdelay(10);
			ulRegRead = PHY_QueryRFReg(priv->share_priv, eRFPath, WriteAddr[HW90_BLOCK_RF], bMaskDWord, 1);
			mdelay(10);
			break;

		default:
			rtStatus = -1;
			break;
		}

		//
		// Check whether readback data is correct
		//
		if(ulRegRead != WriteData[i])
		{
			printk("ulRegRead: %x, WriteData: %x \n", ulRegRead, WriteData[i]);
			rtStatus = -1;
			break;
		}
	}

	return rtStatus;
}


void check_MIMO_TR_status(struct mac80211_shared_priv *priv)
{

	unsigned long ioaddr = priv->pshare_hw->ioaddr;

	if (RTL_R32(PMC_FSM) & BIT(24))
		GET_HW(priv)->MIMO_TR_hw_support = MIMO_2T2R;
	else
		GET_HW(priv)->MIMO_TR_hw_support = MIMO_1T2R;
	return;

}


#if	defined(RTL8192SE) || defined(RTL8192SU)

//------------------------------------------------------------------------------
static int efuse_one_byte_read(struct mac80211_shared_priv *priv, u16 addr,u8 *data)
{
	u8 tmpidx = 0;
	int bResult;
	GET_PRIV_IOADDR;
	
	// -----------------e-fuse reg ctrl ---------------------------------				
	RTL_W8(EFUSE_CTRL+1, (u8)(addr&0xFF));//address
	RTL_W8(EFUSE_CTRL+2, ((u8)((addr>>8) &0x03) ) | (RTL_R8(EFUSE_CTRL+2)&0xFC ));	

	RTL_W8(EFUSE_CTRL+3,  0x72);//read cmd	

	while(!(0x80 & RTL_R8(EFUSE_CTRL+3)) && (tmpidx<100) ){
		tmpidx++;
	}
	if(tmpidx<100){			
		*data=RTL_R8(EFUSE_CTRL);		
		bResult = TRUE;
	}
	else{
		*data = 0xff;		
		bResult = FALSE;
	}
	
	return bResult;
	
}
//------------------------------------------------------------------------------
static void efuse_reg_ctrl(struct mac80211_shared_priv *priv, u8 bPowerOn)
{		
	u8 tmpu8 = 0;
	GET_PRIV_IOADDR;

#define _PRE_EXECUTE_READ_CMD_

	
	if(bPowerOn)
	{
		// -----------------SYS_FUNC_EN Digital Core Vdd enable ---------------------------------		

		tmpu8 = RTL_R8(SYS_FUNC_EN+1) ;
		
		if(!(tmpu8 & 0x20)){
			RTL_W8(SYS_FUNC_EN+1, ( tmpu8|0x20) );
			msleep(10);
		}	
	
		//EE Loader to Retention path1: attach 0x1[0]=0
		RTL_W8(SYS_ISO_CTRL+1, (RTL_R8(SYS_ISO_CTRL+1) & 0xFE));	        

		// -----------------e-fuse pwr & clk reg ctrl ---------------------------------
		RTL_W8(EFUSE_TEST+3, (RTL_R8( EFUSE_TEST+3)|0x80));// 2.5v LDO
		
		// Change Efuse Clock for write action to 40MHZ
		//write8(padapter, EFUSE_CLK_CTRL, (read8(padapter, EFUSE_CLK_CTRL)|0x03));
		RTL_W8(0x02F8, 0x03);
		
#ifdef _PRE_EXECUTE_READ_CMD_
		{
			unsigned char tmpdata;
			efuse_one_byte_read(priv, 0,&tmpdata);
		}		
#endif

	}
	else
	{
		// -----------------e-fuse pwr & clk reg ctrl ---------------------------------
		RTL_W8(EFUSE_TEST+3, RTL_R8(EFUSE_TEST+3)&0x7F);
		
		// Change Efuse Clock for write action to 500K
		//write8(padapter, EFUSE_CLK_CTRL, read8(padapter, EFUSE_CLK_CTRL)&0xFE);
		RTL_W8(0x02F8, 0x02);
		
#if 0
		// -----------------SYS_FUNC_EN Digital Core Vdd disable ---------------------------------
		if(check_fwstate(pmlmepriv, WIFI_MP_STATE) == _FALSE){ 
			write8(padapter, SYS_FUNC_EN+1,  read8(padapter,SYS_FUNC_EN+1)&0xDF);
		}
#endif

	}	
	
}
//------------------------------------------------------------------------------
static void pgpacket_copy_data(const u8 word_en,const u8 *sourdata,u8 *targetdata)
{	
	u8 tmpindex = 0;
	u8 word_idx,byte_idx;
	
	for(word_idx = 0;word_idx<4;word_idx++){
		if(!(word_en&BIT(word_idx))){			
			byte_idx = word_idx*2 ;
			targetdata[byte_idx] 	  = sourdata[tmpindex++];
			targetdata[byte_idx+1] = sourdata[tmpindex++];			
		}
	}
}
//------------------------------------------------------------------------------
static u8 calculate_word_cnts(const u8 word_en)
{
	u8 word_cnts = 0;
	u8 word_idx;
	
#define PGPKG_MAX_WORDS	4
	
	for(word_idx = 0;word_idx<PGPKG_MAX_WORDS;word_idx++){
		if(!(word_en & BIT(word_idx)))	word_cnts++; // 0 : write enable		
	}
	
	return word_cnts;
}
//------------------------------------------------------------------------------
int efuse_pg_packet_read(struct mac80211_shared_priv *priv, u8 offset,u8 *data)
{	

#define PG_STATE_HEADER 	0x01
#define PG_STATE_DATA		0x20
#define PGPKG_MAX_WORDS	4

	u8 ReadState = PG_STATE_HEADER;	
	
	int bContinual = TRUE;	

	u8 efuse_data,word_cnts=0;
	u16 efuse_addr = 0;
	u8 hoffset=0,hworden=0;	
	u8 tmpidx=0;
	u8 tmpdata[8];

	//const u8 MAX_PGPKT_SIZE = 9; //header+ 2* 4 words (BYTES)
	const u8 PGPKT_DATA_SIZE = 8; //BYTES sizeof(u8)*8
	static int EFUSE_MAX_SIZE = 0x1FD; //reserve 3 bytes for HW stop read

	
	if(data==NULL)	return FALSE;
	if(offset>0x0f)	return FALSE;	
	
	memset(data, 0xFF, sizeof(u8)*PGPKT_DATA_SIZE);	
	memset(tmpdata, 0xFF, sizeof(u8)*PGPKT_DATA_SIZE);		
	
	while(bContinual && (efuse_addr  < EFUSE_MAX_SIZE) ){			
		//-------  Header Read -------------
		if(ReadState & PG_STATE_HEADER){
			if(efuse_one_byte_read(priv, efuse_addr ,&efuse_data)&&(efuse_data!=0xFF)){				
				hoffset = (efuse_data>>4) & 0x0F;
				hworden =  efuse_data & 0x0F;									
				word_cnts = calculate_word_cnts(hworden);
				
				if(hoffset==offset){
					
					for(tmpidx = 0;tmpidx< word_cnts*2 ;tmpidx++){
						if(efuse_one_byte_read(priv, efuse_addr+1+tmpidx ,&efuse_data) ){
							tmpdata[tmpidx] = efuse_data;						
						}					
					}
					ReadState = PG_STATE_DATA;	
					
				}
				else{//read next header	
					efuse_addr = efuse_addr + (word_cnts*2)+1;
					ReadState = PG_STATE_HEADER; 
				}
				
			}
			else{
				bContinual = FALSE ;
			}
		}		
		//-------  Data section Read -------------
		else if(ReadState & PG_STATE_DATA){
			pgpacket_copy_data(hworden,tmpdata,data);
			efuse_addr = efuse_addr + (word_cnts*2)+1;
			ReadState = PG_STATE_HEADER; 
		}		
	}
	
	
	if(	(data[0]==0xff) &&(data[1]==0xff) && (data[2]==0xff)  && (data[3]==0xff) &&
		(data[4]==0xff) &&(data[5]==0xff) && (data[6]==0xff)  && (data[7]==0xff))
		return FALSE;
	else
		return TRUE;

}

int read_efuse_content(struct mac80211_shared_priv *priv)
{
	int i, offset;
	struct priv_shared_hw	*pshare_hw = priv->pshare_hw;

	unsigned char *pdata =  &pshare_hw->eFuseData[0];
	
	efuse_reg_ctrl(priv, 1);
	
	for(i=0, offset=0 ; i<128; i+=8, offset++)
	{
		efuse_pg_packet_read(priv, offset, pdata+i);			
	}
	
	efuse_reg_ctrl(priv, 0);
	

	return 0;
}


/*-----------------------------------------------------------------------------
 * Function:	PHY_IQCalibrateBcut()
 *
 * Overview:	After all MAC/PHY/RF is configued. We must execute IQ calibration
 *			to improve RF EVM!!?
 *
 * Input:		IN	PADAPTER	pAdapter
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	11/18/2008	MHC		Create. Document from SD3 RFSI Jenyu.
 *						92S B-cut QFN 68 pin IQ calibration procedure.doc
 *
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Function:	PHY_RF6052SetOFDMTxPower
 *
 * Overview:	For legacy and HY OFDM, we must read EEPROM TX power index for
 *			different channel and read original value in TX power register area from
 *			0xe00. We increase offset and original value to be correct tx pwr.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/05/2008 	MHC		Simulate 8192 series method.
 *
 *---------------------------------------------------------------------------*/

void PHY_RF6052SetOFDMTxPower(struct mac80211_shared_priv *priv, unsigned char powerlevel_1ss, unsigned char powerlevel_2ss)
{
//	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	unsigned int writeVal, powerBase0, powerBase1;
	unsigned char index = 0;
	unsigned short RegOffset1[2] = {rTxAGC_Rate18_06, rTxAGC_Rate54_24};
	unsigned short RegOffset2[4] = {rTxAGC_Mcs03_Mcs00, rTxAGC_Mcs07_Mcs04, rTxAGC_Mcs11_Mcs08, rTxAGC_Mcs15_Mcs12};
	unsigned char  byte0, byte1, byte2, byte3;
	unsigned char RF6052_MAX_TX_PWR = 0x3F;


	if(priv->pshare_hw->use_default_para){
		RTL_W32(0xe00, 0x28282828);
		RTL_W32(0xe04, 0x28282828);
#ifdef HIGH_POWER_EXT_PA
	if(priv->pshare_hw->rf_ft_var.use_ext_pa)
		RTL_W16(0xe08, 0x1515);
	else
#endif
		RTL_W16(0xe08, 0x2424);
		RTL_W32(0xe10, 0x28282828);
		RTL_W32(0xe14, 0x28282828);
		RTL_W32(0xe18, 0x28282828);
		RTL_W32(0xe1c, 0x28282828);

		return; // use default
	}

	if (priv->pshare_hw->legacyOFDM_pwrdiff_A & 0x80)
		powerBase0 = powerlevel_1ss - (priv->pshare_hw->legacyOFDM_pwrdiff_A & 0x0f);
	else
		powerBase0 = powerlevel_1ss + (priv->pshare_hw->legacyOFDM_pwrdiff_A & 0x0f);

	for(index=0; index<2; index++)
	{
		byte0 = powerBase0 + priv->pshare_hw->phw->OFDMTxAgcOffset[0+index*4];
		byte1 = powerBase0 + priv->pshare_hw->phw->OFDMTxAgcOffset[1+index*4];
		byte2 = powerBase0 + priv->pshare_hw->phw->OFDMTxAgcOffset[2+index*4];
		byte3 = powerBase0 + priv->pshare_hw->phw->OFDMTxAgcOffset[3+index*4];

		// Max power index = 0x3F Range = 0-0x3F
		if (byte0 > RF6052_MAX_TX_PWR)
			byte0 = RF6052_MAX_TX_PWR;
		if (byte1 > RF6052_MAX_TX_PWR)
			byte1 = RF6052_MAX_TX_PWR;
		if (byte2 > RF6052_MAX_TX_PWR)
			byte2 = RF6052_MAX_TX_PWR;
		if (byte3 > RF6052_MAX_TX_PWR)
			byte3 = RF6052_MAX_TX_PWR;
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		PHY_SetBBReg(priv, RegOffset1[index], 0x7f7f7f7f, writeVal);
	}

	for(index=0; index<4; index++)
	{
		if (index <= 1)
			powerBase1 = powerlevel_1ss;
		else
			powerBase1 = powerlevel_2ss;

		byte0 = powerBase1 + priv->pshare_hw->phw->MCSTxAgcOffset[0+index*4];
		byte1 = powerBase1 + priv->pshare_hw->phw->MCSTxAgcOffset[1+index*4];
		byte2 = powerBase1 + priv->pshare_hw->phw->MCSTxAgcOffset[2+index*4];
		byte3 = powerBase1 + priv->pshare_hw->phw->MCSTxAgcOffset[3+index*4];

		// Max power index = 0x3F Range = 0-0x3F
		if (byte0 > RF6052_MAX_TX_PWR)
			byte0 = RF6052_MAX_TX_PWR;
		if (byte1 > RF6052_MAX_TX_PWR)
			byte1 = RF6052_MAX_TX_PWR;
		if (byte2 > RF6052_MAX_TX_PWR)
			byte2 = RF6052_MAX_TX_PWR;
		if (byte3 > RF6052_MAX_TX_PWR)
			byte3 = RF6052_MAX_TX_PWR;
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		PHY_SetBBReg(priv, RegOffset2[index], 0x7f7f7f7f, writeVal);
	}
/*
	if (priv->pmib->dot11RFEntry.pwrlevelOFDM_2SS[priv->pshare_hw->working_channel-1+14] != 0)
		priv->pshare->channelAB_pwrdiff =
		        (int)priv->pmib->dot11RFEntry.pwrlevelOFDM_2SS[priv->pshare_hw->working_channel-1+14] -
		        (int)priv->pmib->dot11RFEntry.pwrlevelOFDM_2SS[priv->pshare_hw->working_channel-1];
	else
		*/
		priv->pshare_hw->channelAB_pwrdiff = 0;
	PHY_SetBBReg(priv, rFPGA0_TxGainStage, bXBTxAGC, (priv->pshare_hw->channelAB_pwrdiff & 0x0f));

}	/* PHY_RF6052SetOFDMTxPower */


int phy_BB8192SE_Config_ParaFile(struct mac80211_shared_priv *priv)
{

	int rtStatus;

	phy_InitBBRFRegisterDefinition(priv);


	/*----Check MIMO TR hw setting----*/
	check_MIMO_TR_status(priv);


	/*----BB Register Initilazation----*/
	rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_2T2R);
	if (rtStatus) {
		printk("phy_BB8192SE_Config_ParaFile(): Write BB Reg Fail!!\n");
		return rtStatus;
	}

	/*----If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt----*/

	if (!priv->EE_AutoloadFail)
	{
//		printk("need PG setup\n");
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_PG);
	}
	if(rtStatus){
		printk("phy_BB8192SE_Config_ParaFile():BB_PG Reg Fail!!\n");
		return rtStatus;
	}

	/*----BB AGC table Initialization----*/
	rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);
	if (rtStatus) {
		printk("phy_BB8192SE_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		return rtStatus;
	}

	/*----For 1T2R Config----*/
	if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T2R);
		if (rtStatus) {
			printk("phy_BB8192SE_Config_ParaFile(): Write BB Reg for 1T2R Fail!!\n");
			return rtStatus;
		}
	}else if (get_rf_mimo_mode(priv) == MIMO_1T1R){
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
		if (rtStatus) {
			printk("phy_BB8192SE_Config_ParaFile(): Write BB Reg for 1T1R Fail!!\n");
			return rtStatus;
		}
	}

	DEBUG_INFO("PHY-BB Initialization Success\n");
	return 0;
}


// 8192SE use RF8225 as single chip phy
// So far, since only 8192SE use RF8225, we just write the funciton for 8192SE only
int phy_RF8225_Config_ParaFile(struct mac80211_shared_priv *priv)
{
	int rtStatus=0;
	RF90_RADIO_PATH_E eRFPath;
	BB_REGISTER_DEFINITION_T *pPhyReg;
//	unsigned int  RF0_Final_Value;
	unsigned int  u4RegValue = 0;
//	unsigned char RetryTimes;


	priv->pshare_hw->phw->NumTotalRFPath = 2; // 8192SE only 1T2R or 2T2R, joshua

	for (eRFPath = RF90_PATH_A; eRFPath<priv->pshare_hw->phw->NumTotalRFPath; eRFPath++)
	{
//		if (!PHYCheckIsLegalRfPath8190Pci(priv, eRFPath))
//			continue;
/*
  		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF90_PATH_A) || (eRFPath == RF90_PATH_B))
				continue;
  		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF90_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/

		pPhyReg = &priv->pshare_hw->phw->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch(eRFPath)
		{
		case RF90_PATH_A:
		case RF90_PATH_C:
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		case RF90_PATH_B :
		case RF90_PATH_D:
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV<<16);
			break;
		case RF90_PATH_MAX:
			break;
		}

		/*----Set RF_ENV enable----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfe, bRFSI_RFENV<<16, 0x1);

		/*----Set RF_ENV output high----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);

		/* Set bit number of Address and Data for RF register */
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0); 	// Set 0 to 4 bits for Z-serial and set 1 to 6 bits for 8258
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);	// Set 0 to 12 bits for Z-serial and 8258, and set 1 to 14 bits for ???

#if 0
		rtStatus  = PHY_CheckBBAndRFOK(priv, HW90_BLOCK_RF, eRFPath);
		if(rtStatus!= 0)
		{
			DEBUG_INFO("PHY_RF8256_Config():Check Radio[%d] Fail!!\n", eRFPath);
			goto phy_RF8256_Config_ParaFile_Fail;
		}
#endif
//		RF0_Final_Value = 0;
//		RetryTimes = 5;

		/*----Initialize RF fom connfiguration file----*/
		switch (eRFPath)
		{
#ifdef MERGE_FW
			case RF90_PATH_A:
				rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_start,
												(int)(data_radio_a_end -data_radio_a_start),  eRFPath);
#ifdef HIGH_POWER_EXT_PA
				if(priv->pshare_hw->rf_ft_var.use_ext_pa)
					PHY_ConfigRFWithParaFile(priv, data_radio_a_hp_start,	(int)(data_radio_a_hp_end -data_radio_a_hp_start),  RF90_PATH_A);
#endif
				break;
			case RF90_PATH_B:
				rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_start,
												(int)(data_radio_b_end -data_radio_b_start),  eRFPath);
				break;
#else
			case RF90_PATH_A:
				rtStatus = PHY_ConfigRFWithParaFile(priv, "/usr/rtl8192Pci/radio_a.txt", eRFPath);
				break;
			case RF90_PATH_B:
				rtStatus = PHY_ConfigRFWithParaFile(priv, "/usr/rtl8192Pci/radio_b.txt", eRFPath);
				break;
#endif
			case RF90_PATH_C:
			case RF90_PATH_D:
			default:
				break;
		}

		/*----Restore RFENV control type----*/;
		switch(eRFPath)
		{
		case RF90_PATH_A:
		case RF90_PATH_C:
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
				break;
		case RF90_PATH_B :
		case RF90_PATH_D:
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV<<16, u4RegValue);
			break;
		case RF90_PATH_MAX:
			break;
		}
	}

	DEBUG_INFO("PHY-RF Initialization Success\n");
	printk("PHY-RF Initialization Success\n");

//phy_RF8256_Config_ParaFile_Fail:

	return rtStatus;
}


void check_and_set_ampdu_spacing(struct rtl8190_priv *priv, struct stat_info *pstat)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned char ampdu_des;

	// set MCS15-Short GI spacing (8192SE hardware restriction),
	// which is used while spacing is 16us and rate is 300M

	if (priv->pshare->min_ampdu_spacing < _HTCAP_AMPDU_SPC_16_US_) {
		if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != 0) {
			priv->pshare->min_ampdu_spacing = _HTCAP_AMPDU_SPC_16_US_;
			RTL_W8(AMPDU_MIN_SPACE, _HTCAP_AMPDU_SPC_16_US_ | (0x13<<3));
		}
		else if (priv->pmib->dot11WdsInfo.wdsEnabled && (priv->pmib->dot11WdsInfo.wdsNum > 0)) {
			if (priv->pmib->dot11WdsInfo.wdsPrivacy != 0) {
				priv->pshare->min_ampdu_spacing = _HTCAP_AMPDU_SPC_16_US_;
				RTL_W8(AMPDU_MIN_SPACE, _HTCAP_AMPDU_SPC_16_US_ | (0x13<<3));
			}
		}
		else if (pstat) {
			ampdu_des = (pstat->ht_cap_buf.ampdu_para & _HTCAP_AMPDU_SPC_MASK_) >> _HTCAP_AMPDU_SPC_SHIFT_;
			if (ampdu_des > priv->pshare->min_ampdu_spacing) {
				priv->pshare->min_ampdu_spacing = ampdu_des;
				RTL_W8(AMPDU_MIN_SPACE, ampdu_des | (0x13<<3));
			}
		}
	}
}


static void rtl8192SE_FW_hdr_swap(void *data)
{
#ifdef _BIG_ENDIAN_
	struct _RT_8192S_FIRMWARE_HDR *pFwHdr = (struct _RT_8192S_FIRMWARE_HDR *)data;

	pFwHdr->Signature        = le16_to_cpu(pFwHdr->Signature);
	pFwHdr->Version          = le16_to_cpu(pFwHdr->Version);
	pFwHdr->DMEMSize         = le32_to_cpu(pFwHdr->DMEMSize);
	pFwHdr->IMG_IMEM_SIZE    = le32_to_cpu(pFwHdr->IMG_IMEM_SIZE);
	pFwHdr->IMG_SRAM_SIZE    = le32_to_cpu(pFwHdr->IMG_SRAM_SIZE);
	pFwHdr->FW_PRIV_SIZE     = le32_to_cpu(pFwHdr->FW_PRIV_SIZE);
	pFwHdr->efuse_addr       = le16_to_cpu(pFwHdr->efuse_addr);
	pFwHdr->h2ccmd_resp_addr = le16_to_cpu(pFwHdr->h2ccmd_resp_addr);
	pFwHdr->source_version   = le16_to_cpu(pFwHdr->source_version);
	pFwHdr->release_time     = le32_to_cpu(pFwHdr->release_time);
#endif
}

#ifdef RTL8192SE
static void rtl8192SE_ReadIMG(struct mac80211_shared_priv *priv)
{
	unsigned char *swap_arr;
	struct _RT_8192S_FIRMWARE_HDR *pFwHdr = NULL;
	struct _RT_8192S_FIRMWARE_PRIV *pFwPriv = NULL;
#ifdef MERGE_FW
	unsigned long fw_ptr = (unsigned long)data_rtl8192sfw_start;
#else
		//mm_segment_t old_fs;
		//int fd=0;
	struct file* fp;
	int read_bytes = 0;
#ifndef	CONFIG_X86
		extern long sys_read(unsigned int fd, char * buf, size_t count);
#endif
#endif

	swap_arr = kmalloc(RT_8192S_FIRMWARE_HDR_SIZE, GFP_ATOMIC);
	if (swap_arr == NULL)
		return;

#ifdef MERGE_FW
	memcpy(swap_arr, (unsigned char *)fw_ptr, RT_8192S_FIRMWARE_HDR_SIZE);
	fw_ptr += RT_8192S_FIRMWARE_HDR_SIZE;
	priv->pshare_hw->fw_DMEM_buf = (unsigned char *)fw_ptr;
	fw_ptr += FW_DMEM_SIZE;
#else
	/*
		old_fs = get_fs();
		set_fs(KERNEL_DS);
	
		//read from fw img and setup parameters
	
#ifdef CONFIG_X86
		if ((fd = rtl8190n_fileopen("/usr/rtl8192Pci/rtl8192sfw.bin", O_RDONLY, 0)) < 0)
#else
		if ((fd = sys_open("/usr/rtl8192Pci/rtl8192sfw.bin", O_RDONLY, 0)) < 0)
#endif*/
		
		if ((fp=file_open("/usr/rtl8192Pci/rtl8192sfw.bin", O_RDONLY, 0))==NULL)
		{
			printk("/usr/rtl8192Pci/rtl8192sfw.bin cannot be opened\n");
			BUG();
		}
	
		// read from bin file
		/*
		sys_read(fd, swap_arr, RT_8192S_FIRMWARE_HDR_SIZE);
		sys_read(fd, priv->pshare_hw->fw_DMEM_buf, FW_DMEM_SIZE);
		*/
		
		read_bytes += file_read(fp, 0, swap_arr, RT_8192S_FIRMWARE_HDR_SIZE);
		read_bytes += file_read(fp, read_bytes, priv->pshare_hw->fw_DMEM_buf, FW_DMEM_SIZE);
		
		rtl_cache_sync_wback(priv, (unsigned int)priv->pshare_hw->fw_DMEM_buf, RT_8192S_FIRMWARE_HDR_SIZE, PCI_DMA_TODEVICE);
#endif // MERGE_FW

	rtl8192SE_FW_hdr_swap(swap_arr);
	pFwHdr = (struct _RT_8192S_FIRMWARE_HDR *)swap_arr;

	//set parameters
	priv->pshare_hw->fw_EMEM_len = pFwHdr->IMG_SRAM_SIZE;
	priv->pshare_hw->fw_IMEM_len = pFwHdr->IMG_IMEM_SIZE;
	priv->pshare_hw->fw_DMEM_len = pFwHdr->DMEMSize;
	priv->pshare_hw->fw_version     = pFwHdr->Version;
	priv->pshare_hw->fw_src_version = pFwHdr->source_version;
	priv->pshare_hw->fw_sub_version = pFwHdr->subversion;

	printk("fw_version: %04x(%d.%d), fw_EMEM_len: %d, fw_IMEM_len: %d, fw_DMEM_len: %d\n",
		priv->pshare_hw->fw_version, priv->pshare_hw->fw_src_version, priv->pshare_hw->fw_sub_version,
		priv->pshare_hw->fw_EMEM_len, priv->pshare_hw->fw_IMEM_len, priv->pshare_hw->fw_DMEM_len);

	pFwPriv = (struct _RT_8192S_FIRMWARE_PRIV *)priv->pshare_hw->fw_DMEM_buf;
	// for rf_config filed
	if (get_rf_mimo_mode(priv) == MIMO_1T2R ||get_rf_mimo_mode(priv) == MIMO_1T1R )
		pFwPriv->rf_config = 0x12;
	else // 2T2R
		pFwPriv->rf_config = 0x22;
	// root Fw beacon
	if (priv->pshare_hw->rf_ft_var.rootFwBeacon && (!priv->pshare_hw->rf_ft_var.mp_specific))
		pFwPriv->beacon_offload = 3;
	else
		pFwPriv->beacon_offload = 0;

	//read IMEM
#ifdef MERGE_FW
	priv->pshare_hw->fw_IMEM_buf = (unsigned char *)fw_ptr;
	fw_ptr += priv->pshare_hw->fw_IMEM_len;
#else
		//sys_read(fd, priv->pshare_hw->fw_IMEM_buf, priv->pshare_hw->fw_IMEM_len);
		read_bytes += file_read(fp, read_bytes, priv->pshare_hw->fw_IMEM_buf, priv->pshare_hw->fw_IMEM_len);
		rtl_cache_sync_wback(priv, (unsigned int)priv->pshare_hw->fw_IMEM_buf,  priv->pshare_hw->fw_IMEM_len, PCI_DMA_TODEVICE);
#endif

	//read EMEM
#ifdef MERGE_FW
	priv->pshare_hw->fw_EMEM_buf = (unsigned char *)fw_ptr;
	fw_ptr += priv->pshare_hw->fw_EMEM_len;
#else
		//sys_read(fd, priv->pshare_hw->fw_EMEM_buf, priv->pshare_hw->fw_EMEM_len);
		read_bytes += file_read(fp, read_bytes, priv->pshare_hw->fw_EMEM_buf, priv->pshare_hw->fw_EMEM_len);
		rtl_cache_sync_wback(priv, (unsigned int)priv->pshare_hw->fw_EMEM_buf,  priv->pshare_hw->fw_EMEM_len, PCI_DMA_TODEVICE);
#endif

#ifndef MERGE_FW
		//sys_close(fd);
		//set_fs(old_fs);
		file_close(fp);
		
#endif
	kfree(swap_arr);

}
#endif
#ifdef RTL8192SU
static void rtl8192SU_ReadIMG(struct mac80211_shared_priv *priv)
{
	struct fw_hdr  fwhdr;
	size_t ulfilelength;
	u8 *pmappedfw = NULL, *ptr;	
	void	*phfwfile_hdl = NULL;


	ulfilelength = rtl8192s_open_fw(priv, &phfwfile_hdl, &pmappedfw);

	if((!pmappedfw) || (ulfilelength<=0))
		return;

		
	update_fwhdr(&fwhdr, pmappedfw);

	if(chk_fwhdr(&fwhdr, ulfilelength))
	{
		printk("CHK FWHDR fail!\n");
		goto exit;
	}
				
	//firmware check ok		
	printk("Downloading RTL8192SU firmware major(%d)/minor(%d) version...\n", fwhdr.version >>8, fwhdr.version & 0xff);

	ptr = pmappedfw + FIELD_OFFSET(struct fw_hdr, fwpriv) + fwhdr.fw_priv_sz;

	//.1 load imem
	priv->pshare_hw->fw_IMEM_buf = ptr;
	priv->pshare_hw->fw_IMEM_len = (u16)fwhdr.img_IMEM_size;
		
	//.2 load emem
	ptr += fwhdr.img_IMEM_size;
	priv->pshare_hw->fw_EMEM_buf = ptr;
	priv->pshare_hw->fw_EMEM_len = (u16)fwhdr.img_SRAM_size;

	//.3 load dmem
	//ptr += fwhdr.img_SRAM_size;
	priv->pshare_hw->fw_DMEM_buf = NULL;
	priv->pshare_hw->fw_DMEM_len = (u16)fwhdr.dmem_size;


exit:

	rtl8192s_close_fw(priv, phfwfile_hdl);

}
#endif
#if 0
/*-----------------------------------------------------------------------------
 * Function:	fw_SetRQPN()
 *
 * Overview:
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	04/30/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static	void	fw_SetRQPN(struct rtl8190_priv *priv)
{
	// Only for 92SE HW bug, we have to set RAPN before every FW download
	// We can remove the code later.
	unsigned long ioaddr = priv->pshare->ioaddr;

	RTL_W32(RQPN, 0xffffffff);
	RTL_W32(RQPN+4, 0xffffffff);
	RTL_W8(RQPN+8, 0xff);
	RTL_W8(RQPN+0xB, 0x80);

}	/* fw_SetRQPN */
#endif


static void LoadIMG(struct mac80211_shared_priv *priv, int fw_file)
{
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	int bResult=0;
	unsigned char *mem_ptr=NULL;
	unsigned int buf_len=0;
	int i=0;

//	fw_SetRQPN(priv);	// For 92SE only

	switch(fw_file) {
	case LOAD_IMEM:
		mem_ptr = priv->pshare_hw->fw_IMEM_buf;
		buf_len = priv->pshare_hw->fw_IMEM_len;
		break;
	case LOAD_EMEM:
		mem_ptr = priv->pshare_hw->fw_EMEM_buf;
		buf_len = priv->pshare_hw->fw_EMEM_len;
		break;
	case LOAD_DMEM:
		mem_ptr = priv->pshare_hw->fw_DMEM_buf;
		buf_len = priv->pshare_hw->fw_DMEM_len;
		break;
	default:
		printk("ERROR!, not such loading option\n");
		break;
	}


#ifdef RTL8192SU
	bResult = rtl8192SU_Write_FWIMG(priv, mem_ptr, buf_len, fw_file);
#else
	while(1)
	{
		if ((i+LoadPktSize) < buf_len) {
			//for test
			bResult = rtl8192SE_SetupOneCmdPacket(priv, mem_ptr, LoadPktSize, 0);
			RTL_W16(TPPoll, TPPoll_CQ);
//			mdelay(100);
//			bResult = rtl8192SE_SetupOneCmdPacket(priv, (mem_ptr+buf_len/2), buf_len/2, 1);
//			RTL_W16(TPPoll, TPPoll_CQ);

			if(bResult)
				mdelay(1);

			mem_ptr += LoadPktSize;
			i += LoadPktSize;
		}
		else {
			bResult = rtl8192SE_SetupOneCmdPacket(priv, mem_ptr, (buf_len-i), 1);
			if(bResult)
				RTL_W8(TPPoll, POLL_CMD);
			break;
		}
	}
#endif	

	if (bResult == 0)
		printk("desc not available, firmware cannot be loaded \n");
}


//
// Description:   CPU register locates in different page against general register.
//			    Switch to CPU register in the begin and switch back before return
//
// Arguments:   The pointer of the adapter
// Created by Roger, 2008.04.10.
//
unsigned int FirmwareEnableCPU(struct mac80211_shared_priv *priv)
{

	unsigned int	rtStatus = SUCCESS;
	unsigned char	tmpU1b, CPUStatus = 0;
	unsigned short	tmpU2b;
	unsigned int iCheckTime = 200;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;

//	RT_TRACE(COMP_INIT, DBG_LOUD, ("-->FirmwareEnableCPU()\n") );

//	fw_SetRQPN(priv);	// For 92SE only

	// Enable CPU.
	tmpU1b = RTL_R8(SYS_CLKR);
	mdelay(2);
	RTL_W8(SYS_CLKR, (tmpU1b|SYS_CPU_CLKSEL)); //AFE source
	mdelay(2);

	tmpU2b = RTL_R16(SYS_FUNC_EN);
	mdelay(2);

	RTL_W16(SYS_FUNC_EN, (tmpU2b|FEN_CPUEN));
	mdelay(2);


	//Polling IMEM Ready after CPU has refilled.
	do
	{
		CPUStatus = RTL_R8(TCR);
		if(CPUStatus & IMEM_RDY)
		{
//			RT_TRACE(COMP_INIT, DBG_LOUD, ("IMEM Ready after CPU has refilled.\n"));
			printk("IMEM Ready after CPU has refilled.\n");
			break;
		}

		mdelay(10);
	}while(iCheckTime--);

	if(!(CPUStatus & IMEM_RDY))
		return FAIL;

//	RT_TRACE(COMP_INIT, DBG_LOUD, ("<--FirmwareEnableCPU(): rtStatus(%#x)\n", rtStatus));
	printk("<--FirmwareEnableCPU(): rtStatus(%#x)\n", rtStatus);
	return rtStatus;
}


static int FirmwareCheckReady(struct mac80211_shared_priv *priv,int fw_file)
{
	unsigned int rtStatus = SUCCESS;
	int			PollingCnt = 100;
	unsigned char	 	/*tmpU1b, */ CPUStatus = 0;
	unsigned int		tmpU4b;
//	unsigned int		bOrgIMREnable;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;

//	DEBUG_INFO(("--->FirmwareCheckReady(): LoadStaus(%d)\n", LoadFWStatus));

	if( fw_file == LOAD_IMEM)
	{
		do
		{//Polling IMEM code done.
			CPUStatus = RTL_R8(TCR);
			if(CPUStatus& IMEM_CODE_DONE)
				break;

			mdelay(10);
		}while(PollingCnt--);
		if(!(CPUStatus & IMEM_CHK_RPT) || PollingCnt <= 0)
		{
//			RT_TRACE(COMP_INIT, DBG_LOUD, ("FW_STATUS_LOAD_IMEM FAIL CPUStatus=%x\r\n", CPUStatus));
			printk("Check IMEM fail!, PollingCnt: %x, CPU stats: %x\n",PollingCnt,CPUStatus);
			return FAIL;
		}
		DEBUG_INFO("Load IMEM ok\n");
	}
	else if( fw_file == LOAD_EMEM)
	{//Check Put Code OK and Turn On CPU

		do
		{//Polling EMEM code done.
			CPUStatus = RTL_R8(TCR);
			if(CPUStatus & EMEM_CODE_DONE)
				break;

			mdelay(10);
		}while(PollingCnt--);
		if(!(CPUStatus & EMEM_CHK_RPT))
		{
//			RT_TRACE(COMP_INIT, DBG_LOUD, ("FW_STATUS_LOAD_EMEM FAIL CPUStatus=%x\r\n", CPUStatus));
			printk("Check EMEM fail!, CPU Stats = %x, PollingCnt = %x\n", CPUStatus, PollingCnt);
			return FAIL;
		}

		// Turn On CPU
		rtStatus = FirmwareEnableCPU(priv);
		if(rtStatus != SUCCESS)
		{
//			RT_TRACE(COMP_INIT, DBG_LOUD, ("Enable CPU fail ! \n") );
			printk("CPU Enable Fail\n");
			return FAIL;
		}
		DEBUG_INFO("Load EMEM ok, CPU Enabled\n");
	}
	else if( fw_file == LOAD_DMEM)
	{
		do
		{//Polling DMEM code done
			CPUStatus = RTL_R8(TCR);
			if(CPUStatus& DMEM_CODE_DONE)
				break;

			mdelay(10);
		}while(PollingCnt--);
		if(!(CPUStatus & DMEM_CODE_DONE)){
			printk("Check DMEM fail!\n");
			return FAIL;
		}

		do
		{//Polling Load Firmware ready
			CPUStatus = RTL_R8(TCR);
			if(CPUStatus & LOAD_FW_READY)
				break;

			mdelay(10);
		}while(PollingCnt--);

//		RTL_W32(RQPN, 0x10101010);
//		RTL_W8(LD_RQPN, 0x80);

		tmpU4b = RTL_R32(TCR);
		RTL_W32(TCR, (tmpU4b&(~TCR_ICV)));

		tmpU4b = RTL_R32(RCR);
		RTL_W32(RCR,
			(tmpU4b|RCR_APPFCS|RCR_APP_ICV|RCR_APP_MIC));

		// Set to normal mode
		// 2008/05/26 MH For firmware operation, we must wait BIT 7 check is OK
		// We can set Loopback  mode.

		PollingCnt = 0;
		tmpU4b = RTL_R32(TCR);
		while (!(tmpU4b & FWRDY) && PollingCnt++ < 10)
		{
			tmpU4b = RTL_R32(TCR);
			mdelay(1000);
		}
		if (PollingCnt > 10)
		{
			printk("polling fw ready fail!\n");
			return FAIL;
		}	

		// for test
		RTL_W8(LBKMD_SEL, LBK_NORMAL);
		//DEBUG_INFO("Load DMEM ok. FW is ready and RCR, TCR initialized\n");
		printk("Load DMEM ok. FW is ready and RCR, TCR initialized\n");
	}

//	RT_TRACE(COMP_INIT, DBG_LOUD, ("<---FirmwareCheckReady(): rtStatus(%#x)\n", rtStatus));
	printk("<---FirmwareCheckReady(): rtStatus(%#x)\n", rtStatus);
	return rtStatus;
}


int LoadIMEMIMG(struct mac80211_shared_priv *priv)
{
	LoadIMG(priv, LOAD_IMEM);
	return FirmwareCheckReady(priv, LOAD_IMEM);
}


int LoadDMEMIMG(struct mac80211_shared_priv *priv)
{
	LoadIMG(priv, LOAD_DMEM);
	return FirmwareCheckReady(priv, LOAD_DMEM);
}


int LoadEMEMIMG(struct mac80211_shared_priv *priv)
{
	LoadIMG(priv, LOAD_EMEM);
	return FirmwareCheckReady(priv, LOAD_EMEM);
}



//
// Description:
//	Set the MAC offset [0x09] and prevent all I/O for a while (about 20us~200us, suggested from SD4 Scott).
//	If the protection is not performed well or the value is not set complete, the next I/O will cause the system hang.
// Note:
//	This procudure is designed specifically for 8192S and references the platform based variables
//	which violates the stucture of multi-platform.
//	Thus, we shall not extend this procedure to common handler.
// By Bruce, 2009-01-08.
//
unsigned char
HalSetSysClk8192SE(	struct mac80211_shared_priv *priv,	unsigned char Data)
{
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	RTL_W8((SYS_CLKR + 1), Data);
	udelay(200);
	return TRUE;
}

// Chip version
/*-----------------------------------------------------------------------------
 * Function:	MacConfigBeforeFwDownload()
 *
 * Overview:	Copy from WMAc for init setting. MAC config before FW download
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	04/29/2008	MHC		The same as 92SE MAC initialization.
 *	07/11/2008	MHC		MAC config before FW download
 *	09/02/2008	MHC		The initialize sequence can preven NIC reload fail
 *						NIC will not disappear when power domain init twice.
 *
 *---------------------------------------------------------------------------*/
static void  MacConfigBeforeFwDownload(struct mac80211_shared_priv *priv)
{
	unsigned char		i;
	unsigned char		tmpU1b;
	unsigned short		tmpU2b;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	unsigned char PollingCnt = 20;
	struct rtl8190_hw *phw = GET_HW(priv);

//	RT_TRACE(COMP_INIT, DBG_LOUD, ("--->MacConfig8192SE()\n"));

	// 2008/05/14 MH For 92SE rest before init MAC data sheet not defined now.
	// HW provide the method to prevent press reset bbutton every time.
//	RT_TRACE(COMP_INIT, DBG_LOUD, ("Set some register before enable NIC\r\n"));

	// 2009/03/24 MH Reset PCIE Digital
	tmpU1b = RTL_R8(SYS_FUNC_EN+1);
	tmpU1b &= 0xFE;
	RTL_W8(SYS_FUNC_EN+1, tmpU1b);
	mdelay(1);
	RTL_W8(SYS_FUNC_EN+1, tmpU1b|BIT(0));

	// Switch to SW IO control
	tmpU1b = RTL_R8((SYS_CLKR + 1));
	if(tmpU1b & BIT(7))
	{
		tmpU1b &= ~(BIT(6)|BIT(7));
		if(!HalSetSysClk8192SE(priv, tmpU1b))
			return; // Set failed, return to prevent hang.
	}

	// Clear FW RPWM for FW control LPS. by tynli. 2009.02.23
	RTL_W8(RPWM, 0x0);

	tmpU1b = RTL_R8(SYS_FUNC_EN+1);

//	tmpU1b &= 0xfb;
	tmpU1b &= 0x73;

	RTL_W8(SYS_FUNC_EN+1, tmpU1b);
	// wait for BIT 10/11/15 to pull high automatically!!
	mdelay(1000);
	RTL_W16( CMDR, 0);
	RTL_W32( TCR, 0);
/*
	tmpU1b = RTL_R8(SPS1_CTRL);
	tmpU1b &= 0xfc;
	RTL_W8(SPS1_CTRL, tmpU1b);
*/

	tmpU1b = RTL_R8(0x562);
	tmpU1b |= 0x08;
	RTL_W8(0x562, tmpU1b);
	tmpU1b &= ~(BIT(3));
	RTL_W8(0x562, tmpU1b);

	//tmpU1b = PlatformEFIORead1Byte(Adapter, SYS_FUNC_EN+1);
	//tmpU1b &= 0x73;
	//PlatformEFIOWrite1Byte(Adapter, SYS_FUNC_EN+1, tmpU1b);

	//tmpU1b = PlatformEFIORead1Byte(Adapter, SYS_CLKR);
	//tmpU1b &= 0xfa;
	//PlatformEFIOWrite1Byte(Adapter, SYS_CLKR, tmpU1b);

	// Switch to 80M clock
	//PlatformEFIOWrite1Byte(Adapter, SYS_CLKR, SYS_CLKSEL_80M);

	 // Enable VSPS12 LDO Macro block
	//tmpU1b = PlatformEFIORead1Byte(Adapter, SPS1_CTRL);
	//PlatformEFIOWrite1Byte(Adapter, SPS1_CTRL, (tmpU1b|SPS1_LDEN));

	//Enable AFE clock source
//	RT_TRACE(COMP_INIT, DBG_LOUD, 	("Enable AFE clock source\r\n"));
	tmpU1b = RTL_R8(AFE_XTAL_CTRL);
	RTL_W8(AFE_XTAL_CTRL, (tmpU1b|0x01));
	// Delay 1.5ms
	mdelay(2);
	tmpU1b = RTL_R8(AFE_XTAL_CTRL+1);
	RTL_W8(AFE_XTAL_CTRL+1, (tmpU1b&0xfb));



	//Enable AFE Macro Block's Bandgap
//	RT_TRACE(COMP_INIT, DBG_LOUD, 	("Enable AFE Macro Block's Bandgap\r\n"));
	tmpU1b = RTL_R8(AFE_MISC);
	RTL_W8(AFE_MISC, (tmpU1b|BIT(0)));
	mdelay(2);

	//Enable AFE Mbias
//	RT_TRACE(COMP_INIT, DBG_LOUD, 	("Enable AFE Mbias\r\n"));
	tmpU1b = RTL_R8(AFE_MISC);
	RTL_W8(AFE_MISC, (tmpU1b|0x02));
	mdelay(2);

	//Enable LDOA15 block
	tmpU1b = RTL_R8(LDOA15_CTRL);
	RTL_W8(LDOA15_CTRL, (tmpU1b|BIT(0)));


	 //Enable VSPS12_SW Macro Block
	//tmpU1b = PlatformEFIORead1Byte(Adapter, SPS1_CTRL);
	//PlatformEFIOWrite1Byte(Adapter, SPS1_CTRL, (tmpU1b|SPS1_SWEN));

	//Enable AFE Macro Block's Mbias
	//tmpU1b = PlatformEFIORead1Byte(Adapter, AFE_MISC);
	//PlatformEFIOWrite1Byte(Adapter, AFE_MISC, (tmpU1b|AFE_MBEN));

	// Set Digital Vdd to Retention isolation Path.
	tmpU2b =RTL_R16(SYS_ISO_CTRL);
	printk("sysiso = %x\n", ioread32((void *)priv->pshare_hw->ioaddr));
	RTL_W16(SYS_ISO_CTRL, (tmpU2b|BIT(11)));
	printk("sysiso = %x\n", RTL_R16(SYS_ISO_CTRL));

	// 2008/09/25 MH From SD1 Jong, For warm reboot NIC disappera bug.
	tmpU2b = RTL_R16(SYS_FUNC_EN);
	RTL_W16( SYS_FUNC_EN, (tmpU2b |= BIT(13)));

	RTL_W8(SYS_ISO_CTRL+1, 0x68);

	tmpU1b = RTL_R8(AFE_PLL_CTRL);
	RTL_W8(AFE_PLL_CTRL, (tmpU1b|BIT(0)|BIT(4)));
	// Enable MAC 80MHZ clock
	tmpU1b = RTL_R8(AFE_PLL_CTRL+1);
	RTL_W8( AFE_PLL_CTRL+1, (tmpU1b|BIT(0)));
	mdelay(2);
	// Attatch AFE PLL to MACTOP/BB/PCIe Digital
//	tmpU2b = RTL_R16(SYS_ISO_CTRL);
//	RTL_W16(SYS_ISO_CTRL, (tmpU2b &(~ BIT(12))));

	//Enable AFE clock
	//tmpU2b = PlatformEFIORead2Byte(Adapter, AFE_XTAL_CTRL);
	//PlatformEFIOWrite2Byte(Adapter, AFE_XTAL_CTRL, (tmpU2b &(~XTAL_GATE_AFE)));

	// Release isolation AFE PLL & MD
	RTL_W8(SYS_ISO_CTRL, 0xA6);



	//Enable MAC clock
	tmpU2b = RTL_R16( SYS_CLKR);
	//PlatformEFIOWrite2Byte(Adapter, SYS_CLKR, (tmpU2b|SYS_MAC_CLK_EN));
	RTL_W16(SYS_CLKR, (tmpU2b|BIT(12)|BIT(11)));


	//Enable Core digital and enable IOREG R/W
	tmpU2b = RTL_R16(SYS_FUNC_EN);
	RTL_W16(SYS_FUNC_EN, (tmpU2b|BIT(11)));

	tmpU1b = RTL_R8(SYS_FUNC_EN+1);
	RTL_W8(SYS_FUNC_EN+1, tmpU1b&~(BIT(7)));

	// 2008/09/25 MH From SD1 Jong, For warm reboot NIC disappera bug.
// 	tmpU2b = RTL_R16(SYS_FUNC_EN);
//	RTL_W16(SYS_FUNC_EN, tmpU2b | BIT(13));

	//enable REG_EN
	RTL_W16(SYS_FUNC_EN, (tmpU2b|BIT(11)|BIT(15)));

	 //Switch the control path.
	 tmpU2b = RTL_R16(SYS_CLKR);
	RTL_W16(SYS_CLKR, (tmpU2b&(~BIT(2))));

	tmpU1b = RTL_R8((SYS_CLKR + 1));
	tmpU1b = ((tmpU1b | BIT(7)) & (~BIT(6)));
	if(!HalSetSysClk8192SE(priv, tmpU1b))
		return; // Set failed, return to prevent hang.

	RTL_W16(CMDR, 0x07FC);

	// 2008/09/02 MH We must enable the section of code to prevent load IMEM fail.
	// Load MAC register from WMAc temporarily We simulate macreg.txt HW will provide
	// MAC txt later
	RTL_W8(0x6, 0x30);
//	RTL_W8(0x48, 0x3f);
	RTL_W8(0x49, 0xf0);

	RTL_W8(0x4b, 0x81);

	RTL_W8(0xb5, 0x21);

	RTL_W8(0xdc, 0xff);
	RTL_W8(0xdd, 0xff);
	RTL_W8(0xde, 0xff);
	RTL_W8(0xdf, 0xff);

	RTL_W8(0x11a, 0x00);
	RTL_W8(0x11b, 0x00);

	for (i = 0; i < 32; i++)
		RTL_W8(INIMCS_SEL+i, 0x1b); // MCS15

	RTL_W8(CFEND_TH, 0xff);

	RTL_W8( 0x503, 0x22);
//	RTL_W8( 0x560, 0x09);
	RTL_W8( 0x560, 0x40);

	RTL_W8( DBG_PORT, 0x91);

	RTL_W32(RDSA, phw->ring_dma_addr);
	RTL_W32(TMDA, phw->tx_ring0_addr);
	RTL_W32(TBKDA, phw->tx_ring1_addr);
	RTL_W32(TBEDA, phw->tx_ring2_addr);
	RTL_W32(TVIDA, phw->tx_ring3_addr);
	RTL_W32(TVODA, phw->tx_ring4_addr);
	RTL_W32(THPDA, phw->tx_ring5_addr);
	RTL_W32(TBDA, phw->tx_ringB_addr);
	RTL_W32(RCDA, phw->rxcmd_ring_addr);
	RTL_W32(TCDA, phw->txcmd_ring_addr);
//	RTL_W32(TCDA, phw->tx_ring5_addr);

	// 2009/03/13 MH Prevent incorrect DMA write after accident reset !!!
	RTL_W16(CMDR, 0x37FC);

	//
	// <Roger_EXP> To make sure that TxDMA can ready to download FW.
	// We should reset TxDMA if IMEM RPT was not ready.
	// Suggested by SD1 Alex. 2008.10.23.
	//
	do
	{
		tmpU1b = RTL_R8( TCR);
		if((tmpU1b & (IMEM_CHK_RPT|EXT_IMEM_CHK_RPT)) == (IMEM_CHK_RPT|EXT_IMEM_CHK_RPT))
			break;

		mdelay(5);
	}while(PollingCnt--);	// Delay 1ms

	if(PollingCnt <= 0 )
	{
		printk("MacConfigBeforeFwDownloadASIC(): Polling TXDMA_INIT_VALUE timeout!! Current TCR(%x)\n", tmpU1b);
		tmpU1b = RTL_R8(CMDR);
		RTL_W8(CMDR, tmpU1b&(~TXDMA_EN));
		mdelay(10);
		RTL_W8(CMDR, tmpU1b|TXDMA_EN);// Reset TxDMA
	}


//	RT_TRACE(COMP_INIT, DBG_LOUD, ("<---MacConfig8192SE()\n"));

	RTL_W16(BCN_DRV_EARLY_INT, (10<<4)); // 2
}	/* MacConfigBeforeFwDownload */



/*-----------------------------------------------------------------------------
 * Function:	MacConfigAfterFwDownload()
 *
 * Overview:	After download Firmware, we must set some MAC register for some dedicaed
 *			purpose.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	07/02/2008	MHC		Create.
 *
 *---------------------------------------------------------------------------*/
static void MacConfigAfterFwDownload(struct mac80211_shared_priv *priv)
{
	GET_PRIV_IOADDR;

	//
	// 1. System Configure Register (Offset: 0x0000 - 0x003F)
	//

	//
	// 2. Command Control Register (Offset: 0x0040 - 0x004F)
	//
	// Turn on 0x40 Command register
	// Original is RTL_W8 ... but should be RTL_W16?
	RTL_W16(CMDR, BBRSTn|BB_GLB_RSTn|SCHEDULE_EN|MACRXEN|MACTXEN|DDMA_EN|FW2HW_EN|	RXDMA_EN|TXDMA_EN|HCI_RXDMA_EN|HCI_TXDMA_EN);

	//RTL_W16(CMDR, 0);

	printk("CMDR = 0x%x 0x%x -- 0x%x \n", (RTL_R8(CMDR)), (RTL_R8(CMDR+1)), (RTL_R16(CMDR)));

	// for sending beacon in current stage, disable RX
//	RTL_W16(CMDR, BBRSTn|BB_GLB_RSTn|SCHEDULE_EN|MACRXEN|MACTXEN|DDMA_EN|FW2HW_EN|	RXDMA_EN|TXDMA_EN|HCI_RXDMA_EN|HCI_TXDMA_EN);

	// Set TCR TX DMA pre 2 FULL enable bit
	RTL_W32(TCR, RTL_R32(TCR)|TXDMAPRE2FULL);
	// Set RCR ... disable RX now, will enable later

	RTL_W32(RCR, RCR_APPFCS |RCR_APP_PHYST_RXFF | RCR_APP_PHYST_RXFF | RCR_APWRMGT /*| RCR_ADD3*/ |	RCR_AMF	| RCR_ADF | RCR_APP_MIC | RCR_APP_ICV |       RCR_AICV	| /*RCR_ACRC32	|*/				// Accept ICV error, CRC32 Error
	RCR_AB 		| RCR_AM		|				// Accept Broadcast, Multicast
    	RCR_APM 	|  								// Accept Physical match
     	/*RCR_AAP		|    */ 						// Accept Destination Address packets
     	RCR_APP_PHYST_STAFF | RCR_APP_PHYST_RXFF |	// Accept PHY status
	(7/*pHalData->EarlyRxThreshold*/<<RCR_FIFO_OFFSET));

	//
	// 3. MACID Setting Register (Offset: 0x0050 - 0x007F)
	//


	//
	// 4. Timing Control Register  (Offset: 0x0080 - 0x009F)
	//
	// Set CCK/OFDM SIFS

	RTL_W16(SIFS_CCK, 0x0a0a);//0x1010
	RTL_W16(SIFS_OFDM, 0x0a0a);// 0x1010
	//3
	//Set AckTimeout
	RTL_W8(ACK_TIMEOUT, 0x40);

	//Beacon related
	set_fw_reg(priv, 0xf1006400, 0, 0);
//	RTL_W32(0x2A4, 0x00006300);
//	RTL_W32(0x2A0, 0xb026007C);
//	mdelay(1);
//	while(RTL_R32(0x2A0) != 0){};
	RTL_W16(ATIMWND, 2);
	//PlatformEFIOWrite2Byte(Adapter, BCN_DRV_EARLY_INT, 0x0022);
	//PlatformEFIOWrite2Byte(Adapter, BCN_DMATIME, 0xC8);
	//PlatformEFIOWrite2Byte(Adapter, BCN_ERR_THRESH, 0xFF);
	//PlatformEFIOWrite1Byte(Adapter, MLT, 8);

	//
	// 5. FIFO Control Register (Offset: 0x00A0 - 0x015F)
	//
	// Initialize Number of Reserved Pages in Firmware Queue
#if 0
	PlatformEFIOWrite4Byte(Adapter, RQPN1,
	NUM_OF_PAGE_IN_FW_QUEUE_BK<<0 | NUM_OF_PAGE_IN_FW_QUEUE_BE<<8 |\
	NUM_OF_PAGE_IN_FW_QUEUE_VI<<16 | NUM_OF_PAGE_IN_FW_QUEUE_VO<<24);
	PlatformEFIOWrite4Byte(Adapter, RQPN2,
	NUM_OF_PAGE_IN_FW_QUEUE_HCCA << 0 | NUM_OF_PAGE_IN_FW_QUEUE_CMD << 8|\
	NUM_OF_PAGE_IN_FW_QUEUE_MGNT << 16 |NUM_OF_PAGE_IN_FW_QUEUE_HIGH << 24);
	PlatformEFIOWrite4Byte(Adapter, RQPN3,
	NUM_OF_PAGE_IN_FW_QUEUE_BCN<<0 | NUM_OF_PAGE_IN_FW_QUEUE_PUB<<8);
	// Active FW/MAC to load the new RQPN setting
	PlatformEFIOWrite1Byte(Adapter, LD_RQPN, BIT7);
#endif
	// Setting TX/RX page size 0/1/2/3/4=64/128/256/512/1024
	//PlatformEFIOWrite1Byte(Adapter, PBP, 0x22);


	// Set RXDMA arbitration to control RXDMA/MAC/FW R/W for RXFIFO
	// Set RXDMA arbitration to control RXDMA/MAC/FW R/W for RXFIFO
	RTL_W8( RXDMA_, RTL_R8(RXDMA_)|BIT(6));

	//
	// 6. Adaptive Control Register  (Offset: 0x0160 - 0x01CF)
	//
	// Set RRSR to all legacy rate and HT rate
	// CCK rate is supported by default.
	// CCK rate will be filtered out only when associated AP does not support it.
	// Only enable ACK rate to OFDM 24M
#ifdef RTL8192SE_ACUT
	RTL_W8(RRSR, 0xf0);
#else
	RTL_W8(RRSR, 0x5f);
#endif
	RTL_W8(RRSR+1, 0x01);
//	RTL_W8(RRSR+1, 0xff);
	RTL_W8(RRSR+2, 0x0);
	//
	// 6. Adaptive Control Register  (Offset: 0x0160 - 0x01CF)
	//
	// Set RRSR to all legacy rate and HT rate
	// CCK rate is supported by default.
	// CCK rate will be filtered out only when associated AP does not support it.
	// Only enable ACK rate to OFDM 24M

	// ARFR table set by f/w, 20081130
#if 0
	RTL_W32(ARFR0, 0x1f0ff0f0);
	RTL_W32(ARFR1, 0x1f0ff0f0);
	RTL_W32(ARFR2, 0x1f0ff0f0);
	RTL_W32(ARFR3, 0x1f0ff0f0);
	RTL_W32(ARFR4, 0x1f0ff0f0);
	RTL_W32(ARFR5, 0x1f0ff0f0);
	RTL_W32(ARFR6, 0x1f0ff0f0);
	RTL_W32(ARFR7, 0x1f0ff0f0);
#endif
	// Different rate use different AMPDU size
	// MCS32/ MCS15_SG use max AMPDU size 15*2=30K
	RTL_W8(AGGLEN_LMT_H, 0x0f);
	// MCS0/1/2/3 use max AMPDU size 4*2=8K
	RTL_W16(AGGLEN_LMT_L, 0x5221);
	// MCS4/5 use max AMPDU size 8*2=16K 6/7 use 10*2=20K
	RTL_W16(AGGLEN_LMT_L+2, 0xBBB5);
	// MCS8/9 use max AMPDU size 8*2=16K 10/11 use 10*2=20K
	RTL_W16(AGGLEN_LMT_L+4, 0xB551);
	// MCS12/13/14/15 use max AMPDU size 15*2=30K
	RTL_W16(AGGLEN_LMT_L+6, 0xFFFB);

	// WHChang's suggestion
	RTL_W32(DARFRC, 0x01000000);
	RTL_W32(DARFRC+4, 0x07060504);
	RTL_W32(RARFRC, 0x01000000);
	RTL_W32(RARFRC+4, 0x07060504);

	set_fw_reg(priv, 0xfd0000af, 0 ,0);
	set_fw_reg(priv, 0xfd0000a6, 0 ,0);
	set_fw_reg(priv, 0xfd0000a0, 0 ,0);

//	RTL_W32(0x2c0, 0xd0000000);
	RTL_W32(0x1d8, 0xa44f);


	// Set Data / Response auto rate fallack retry count
	//PlatformEFIOWrite4Byte(Adapter, DARFRC, 0x04030201);
	//PlatformEFIOWrite4Byte(Adapter, DARFRC+4, 0x08070605);
	//PlatformEFIOWrite4Byte(Adapter, RARFRC, 0x04030201);
	//PlatformEFIOWrite4Byte(Adapter, RARFRC+4, 0x08070605);

	// MCS/CCK TX Auto Gain Control Register
	//PlatformEFIOWrite4Byte(Adapter, MCS_TXAGC, 0x0D0C0C0C);
	//PlatformEFIOWrite4Byte(Adapter, MCS_TXAGC+4, 0x0F0E0E0D);
	//PlatformEFIOWrite2Byte(Adapter, CCK_TXAGC, 0x0000);


	//
	// 7. EDCA Setting Register (Offset: 0x01D0 - 0x01FF)
	//
	// BCNTCFG
	//PlatformEFIOWrite2Byte(Adapter, BCNTCFG, 0x0000);
	// Set all rate to support SG
//	RTL_W16(SG_RATE, 0xFFFF);	// set by fw, 20090708


	//
	// 8. WMAC, BA, and CCX related Register (Offset: 0x0200 - 0x023F)
	//
#if 0
	// Set NAV protection length
	RTL_W16(NAV_PROT_LEN, 0x01C0);
	// CF-END Threshold
	RTL_W8(CFEND_TH, 0xFF);
#endif

	{
		// Set AMPSU minimum space
		priv->pshare_hw->min_ampdu_spacing = 0;

		// for RF reset
		RTL_W8(0x1F, 0x0);
		mdelay(1);
		RTL_W8(0x1F, 0x07);
	}

	RTL_W8(AMPDU_MIN_SPACE, priv->pshare_hw->min_ampdu_spacing);
	// Set TXOP stall control for several queue/HI/BCN/MGT/
	RTL_W8(TXOP_STALL_CTRL, 0x00);

    //Set Driver INFO size
	RTL_W8(RXDRVINFO_SZ, 0x4);

	//Set RQPN
#if 0
	RTL_W32(RQPN1,0x15150707);
	RTL_W32(RQPN2,0x03030000);
	RTL_W32(RQPN3,0x8000bc00);
#endif
	if (!RTL_R8(RQPN3) && (RTL_R8(RQPN3+1)>1)) {
		//default assign bcn q 2 page when pub q page >= 2 and bcn q page == 0
		RTL_W8(RQPN3+1, RTL_R8(RQPN3+1)-2);
		RTL_W8(RQPN3, 2);
	}

	// for warm reboot.
        RTL_W8(0x03, RTL_R8(0x03) | BIT(4));
        RTL_W8(0x01, RTL_R8(0x01) & 0xFE);

	//Make sure DIG is done by false alarm
	set_fw_reg(priv, 0xfd00ff00, 0 ,0);

	// Disable DIG as default
//	set_fw_reg(priv, 0xfd000001, 0, 0);
	RTL_W8(0x364, RTL_R8(0x364) &  ~FW_REG364_DIG);

#if defined(EXT_ANT_DVRY) && defined(RTL8192SE)
//for giga 2T3R
	RTL_W8(0x2f1, RTL_R8(0x2f1)|BIT(3));
	RTL_W32(0x2ec, 0xff0400);
#endif

	// for test, set fall back related regs
//	RTL_W32(0x1b0, 0x04030201);
//	RTL_W32(0x1b4, 0x08070605);
//	RTL_W32(0x1b8, 0x04030201);
//	RTL_W32(0x1bc, 0x08070605);

	// 9. Security Control Register (Offset: 0x0240 - 0x025F)
	// 10. Power Save Control Register (Offset: 0x0260 - 0x02DF)
	// 11. General Purpose Register (Offset: 0x02E0 - 0x02FF)
	// 12. Host Interrupt Status Register (Offset: 0x0300 - 0x030F)
	// 13. Test Mode and Debug Control Register (Offset: 0x0310 - 0x034F)

	//
	// 13. HOST/MAC PCIE Interface Setting
	//
	// For 92Se if we can not assign as 0x77 in init step~~~~ other wise TP is bad!!!!???,
	// Set Tx dma burst
	/*PlatformEFIOWrite1Byte(Adapter, PCIF, ((MXDMA2_NoLimit<<MXDMA2_RX_SHIFT) | \
											(MXDMA2_NoLimit<<MXDMA2_TX_SHIFT) | \
											(1<<MULRW_SHIFT)));*/
	//3 Set Tx/Rx Desc Address
	/*PlatformEFIOWrite4Byte(Adapter, RDQDA, pHalData->RxDescMemory[RX_MPDU_QUEUE].PhysicalAddressLow);
	PlatformEFIOWrite4Byte(Adapter, RCQDA, pHalData->RxDescMemory[RX_CMD_QUEUE].PhysicalAddressLow);

	PlatformEFIOWrite4Byte(Adapter, TCDA	, pHalData->TxDescMemory[TXCMD_QUEUE].PhysicalAddressLow);  // Command queue address borrowed VI descriptors
	PlatformEFIOWrite4Byte(Adapter, TMDA, 	pHalData->TxDescMemory[MGNT_QUEUE].PhysicalAddressLow);
	PlatformEFIOWrite4Byte(Adapter, TVODA, pHalData->TxDescMemory[VO_QUEUE].PhysicalAddressLow);
	PlatformEFIOWrite4Byte(Adapter, TVIDA, 	pHalData->TxDescMemory[VI_QUEUE].PhysicalAddressLow);
	PlatformEFIOWrite4Byte(Adapter, TBEDA, pHalData->TxDescMemory[BE_QUEUE].PhysicalAddressLow);
	PlatformEFIOWrite4Byte(Adapter, TBKDA, pHalData->TxDescMemory[BK_QUEUE].PhysicalAddressLow);
	PlatformEFIOWrite4Byte(Adapter, TBDA, pHalData->TxDescMemory[BEACON_QUEUE].PhysicalAddressLow);
	*/

}	/* MacConfigAfterFwDownload */

#ifdef RTL8192SE
int rtl819x_init_hw_PCI(struct mac80211_shared_priv *priv)
{
	static unsigned long ioaddr;
	static unsigned long val32;
	static unsigned short val16;
	//static int i;
	//static unsigned short fixed_rate;
	//static unsigned int ii;
	//static unsigned char calc_rate;
	int max_aggre_time;

	unsigned char net_work_type = WIRELESS_11N | WIRELESS_11G | WIRELESS_11B;

	ioaddr  = priv->pshare_hw->ioaddr;

	DBFENTER;


//1 For Test, Firmware Downloading

	MacConfigBeforeFwDownload(priv);


	// ===========================================================================================
	// Download Firmware
	// allocate memory for tx cmd packet
	if((priv->pshare_hw->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
		printk("not enough memory for txcmd_buf\n");
		return -1;
	}

	priv->pshare_hw->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare_hw->txcmd_buf,
			LoadPktSize, PCI_DMA_TODEVICE);

	if(LoadFirmware(priv) == FALSE){
//		panic_printk("Load Firmware Fail!\n");
#ifdef CONFIG_PRINTK_DISABLED
		panic_printk
#else
		printk
#endif
                ("Load Firmware check!\n");
		return -1;
	}else {
//		mdelay(20);
		PRINT_INFO("Load firmware successful! \n");
	}

	MacConfigAfterFwDownload(priv);



	//
	// 2. Initialize MAC/PHY Config by MACPHY_reg.txt
	//

	PHY_ConfigMACWithParaFile(priv);// this should be opened in the future


	//
	// 3. Initialize BB After MAC Config PHY_reg.txt, AGC_Tab.txt
	//
	{
		val32 = phy_BB8192SE_Config_ParaFile(priv);
		if (val32)
			return -1;
	}



	// support up to MCS7 for 1T1R, modify rx capability here
	//if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		//pmib->dot11nConfigEntry.dot11nSupportedMCS &= 0x00ff;

	// Set NAV protection length
	// CF-END Threshold
/*
	if (priv->pmib->dot11OperationEntry.wifi_specific) {
		RTL_W16(NAV_PROT_LEN, 0x80);
		RTL_W8(CFEND_TH, 0x2);
	}
	else 
*/	
	{
		RTL_W16(NAV_PROT_LEN, 0x01C0);
		RTL_W8(CFEND_TH, 0xFF);
	}

	//
	// 4. Initiailze RF RAIO_A.txt RF RAIO_B.txt
	//
	// 2007/11/02 MH Before initalizing RF. We can not use FW to do RF-R/W.
	// close loopback, normal mode

	// For RF test only from Scott's suggestion
	RTL_W8(0x27, 0xDB);
//	RTL_W8(0x1B, 0x07); // ACUT


	// set RCR: RX_SHIFT and disable ACF
	RTL_W8(0x48, 0x3e);
	RTL_W32(0x48, RTL_R32(0x48) & ~ RCR_ACF  & ~RCR_ACRC32);
	// for debug by victoryman, 20081119
	RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYST_RXFF);

/*
#ifdef DRVMAC_LB
	if (priv->pmib->miscEntry.drvmac_lb) {
		RTL_W8(MSR, MSR_NOLINK);
		drvmac_loopback(priv);
	}
	else
#endif
	{
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup
#ifdef CLIENT_MODE
				|| (!(OPMODE & WIFI_AP_STATE) &&
						(priv->join_res != STATE_Sta_Bss) && (priv->join_res != STATE_Sta_Ibss_Active))
#endif
			)
#endif
*/
		{
			// so far, 8192SE does not need to initialize BB or RF
			val32 = phy_RF8225_Config_ParaFile(priv);
			if (val32)
				return -1;
		}
//	}



#ifdef DW_FW_BY_MALLOC_BUF
	kfree(priv->pshare->fw_IMEM_buf);
	kfree(priv->pshare->fw_EMEM_buf);
	kfree(priv->pshare->fw_DMEM_buf);
#endif

/*
	{
		// for test, switch to 40Mhz mode
		unsigned int val_read;
		val_read = PHY_QueryRFReg(priv, 0, 18, bMask20Bits, 1);
		val_read &= ~(BIT(10)|BIT(11));
		PHY_SetRFReg(priv, 0, 18, bMask20Bits, val_read);
		val_read = PHY_QueryRFReg(priv, 1, 18, bMask20Bits, 1);
		val_read &= ~(BIT(10)|BIT(11));
		PHY_SetRFReg(priv, 1, 18, bMask20Bits, val_read);

		RTL_W8(0x800,RTL_R8(0x800)|0x1);
		RTL_W8(0x800,RTL_R8(0x900)|0x1);

		RTL_W8(0xc04 , 0x33);
		RTL_W8(0xd04, 0x33);

	}
*/

	/*---- Set CCK and OFDM Block "ON"----*/
//	PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
//	PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);


//	RTL_W8(BW_OPMODE, BIT(2)); // 40Mhz:0 20Mhz:1
//	RTL_W32(MACIDR,0x0);

	// under loopback mode
//	RTL_W32(MACIDR,0xffffffff);
/*
#ifdef CONFIG_NET_PCI
	if (IS_PCIBIOS_TYPE)
		pci_unmap_single(priv->pshare->pdev, priv->pshare->cmdbuf_phyaddr,
			(LoadPktSize), PCI_DMA_TODEVICE);
#endif
*/
// clear TPPoll
//	RTL_W16(TPPoll, 0x0);
// Should 8192SE do this initialize? I don't know yet, 080812. Joshua
	// PJ 1-5-2007 Reset PHY parameter counters
//	RTL_W32(0xD00, RTL_R32(0xD00)|BIT(27));
//	RTL_W32(0xD00, RTL_R32(0xD00)&(~(BIT(27))));

	// configure timing parameter
	RTL_W8(ACK_TIMEOUT, 0x30);
	RTL_W8(PIFS_TIME,0x13);
//	RTL_W16(LBDLY, 0x060F);
//	RTL_W16(SIFS_OFDM, 0x0e0e);
//	RTL_W8(SLOT_TIME, 0x10);


//	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
	if (net_work_type & WIRELESS_11N) {
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
//	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
	else if (net_work_type & WIRELESS_11A) {		
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
//	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
	else if (net_work_type & WIRELESS_11G) {		
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else { // WIRELESS_11B
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x14);
	}

//	init_EDCA_para(priv, pmib->dot11BssType.net_work_type);
	init_EDCA_para(priv, net_work_type);


	// we don't have EEPROM yet, Mark this for FPGA Platform
//	RTL_W8(_9346CR_, CR9346_CFGRW);

//	92SE Windows driver does not set the PCIF as 0x77, seems HW bug?
	// Set Tx and Rx DMA burst size
//	RTL_W8(PCIF, 0x77);
	// Enable byte shift
//	RTL_W8(_PCIF_+2, 0x01);


	// Retry Limit
/*	
	if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
		val16 = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
	else {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			val16 = 0x30;
		else
			val16 = 0x06;
	}
	if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
		val16 |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
	else {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			val16 |= (0x30 << 8);
		else
			val16 |= (0x06 << 8);
	}
*/
	val16 = 0x30 |  (0x30 << 8);
	RTL_W16(RETRY_LIMIT, val16);

/*	This should be done later, but Windows Driver not done yet.
	// Response Rate Set
	// let ACK sent by highest of 24Mbps
	val32 = 0x1ff;
	if (pmib->dot11RFEntry.shortpreamble)
		val32 |= BIT(23);
	RTL_W32(_RRSR_, val32);
*/

	// Adaptive Rate Table for Basic Rate

/*
	val32 = 0;
	for (i=0; i<32; i++) {
		if (AP_BSSRATE[i]) {
			if (AP_BSSRATE[i] & 0x80)
				val32 |= get_bit_value_from_ieee_value(AP_BSSRATE[i] & 0x7f);
		}
	}
	val32 |= (priv->pmib->dot11nConfigEntry.dot11nBasicMCS << 12);
*/
	val32= 0xfffffff;
	set_fw_reg(priv, (0xfd00002a | (1<<9 | ARFR_BMC)<<8), val32 ,1);

//	panic_printk("0x2c4 = bitmap = 0x%08x\n", (unsigned int)val32);
//	panic_printk("0x2c0 = cmd | macid | band = 0x%08x\n", 0xfd0000a2 | (1<<9 | (sta_band & 0xf))<<8);
//	panic_printk("Add id %d val %08x to ratr\n", 0, (unsigned int)val32);

/*	for (i = 0; i < 8; i++)
#ifdef RTL8192SE_ACUT
		RTL_W32(ARFR0+i*4, val32 & 0x1f0ff0f0);
#else
		RTL_W32(ARFR0+i*4, val32 & 0x1f0ff0f0);
#endif
*/

	//settting initial rate for control frame to 24M
	RTL_W8(INIRTSMCS_SEL, 8);

	//setting MAR
	RTL_W32(MAR, 0xffffffff);
	RTL_W32((MAR+4), 0xffffffff);

	//setting BSSID, not matter AH/AP/station
//	memcpy((void *)&val32, (pmib->dot11OperationEntry.hwaddr), 4);
//	memcpy((void *)&val16, (pmib->dot11OperationEntry.hwaddr + 4), 2);

	val32 = 0x00e04c81;
	val16= 0x96bc;

	RTL_W32(BSSIDR, cpu_to_le32(val32));
	RTL_W16((BSSIDR + 4), cpu_to_le16(val16));

	
//	RTL_W32(BSSIDR, 0x814ce000);
//	RTL_W16((BSSIDR + 4), 0xee92);

	//setting TCR
	//TCR, use default value

	//setting RCR // set in MacConfigAfterFwDownload
//	RTL_W32(_RCR_, _APWRMGT_ | _AMF_ | _ADF_ | _AICV_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
//	if (pmib->dot11OperationEntry.crc_log)
//		RTL_W32(_RCR_, RTL_R32(_RCR_) | _ACRC32_);

	//setting MSR
//	if (opmode & WIFI_AP_STATE)
#if 0	//brian move to 
	if( priv->opmode == NL80211_IFTYPE_AP)
	{
		DEBUG_INFO("AP-mode enabled...\n");
/*
#if defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
		if (priv->pmib->dot11WdsInfo.wdsPure || priv->pmib->dot1180211sInfo.meshSilence )
#else
		if (priv->pmib->dot11WdsInfo.wdsPure)
#endif
			RTL_W8(MSR, MSR_NOLINK);
		else
*/			
			RTL_W8(MSR, MSR_AP);
// Move init beacon after f/w download
#if 0
		if (priv->auto_channel == 0) {
			DEBUG_INFO("going to init beacon\n");
			init_beacon(priv);
		}
#endif
	}
#ifdef CLIENT_MODE
	else if (priv->opmode == NL80211_IFTYPE_STATION)
	{
		DEBUG_INFO("Station-mode enabled...\n");
		RTL_W8(MSR, MSR_INFRA);
	}
	else if (priv->opmode == NL80211_IFTYPE_ADHOC)
	{
		DEBUG_INFO("Adhoc-mode enabled...\n");
		RTL_W8(MSR, MSR_ADHOC);
	}
#endif
	else if (priv->opmode == NL80211_IFTYPE_MONITOR)
	{
		DEBUG_INFO("Monitor-mode enabled...\n");
		RTL_W8(MSR, MSR_NOLINK);
	}
	else
	{
		printk("Operation mode error!\n");
		return 2;
	}
#endif

	CamResetAllEntry(priv);
	RTL_W16(SECR, 0x0000);

/*	
	if ((OPMODE & (WIFI_AP_STATE|WIFI_STATION_STATE|WIFI_ADHOC_STATE)) &&
		!priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) {
		pmib->dot11GroupKeysTable.dot11Privacy = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
			i = 5;
		else
			
			i = 13;
		

#ifdef CONFIG_RTL8196B_KLD
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
#else
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], i);
#endif
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;

	}
*/
/*
	//here add if legacy WEP
	// if 1x is enabled, do not set default key, david
//#if 0	// marked by victoryman, use pairwise key at present, 20070627
//#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD)
#ifdef USE_WEP_DEFAULT_KEY
#ifdef MBSSID
	if (!(OPMODE & WIFI_AP_STATE) || !priv->pmib->miscEntry.vap_enable)
#endif
	{
		if(!SWCRYPTO && !IEEE8021X_FUN &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_))
			init_DefaultKey_Enc(priv, NULL, 0);
	}
#endif
*/

	//Setup Beacon Interval/interrupt interval, ATIM-WIND ATIM-Interrupt
//	RTL_W16(BCN_INTERVAL, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	//Setting BCNITV is done by firmware now
//	set_fw_reg(priv, (0xF1000000 | (pmib->dot11StationConfigEntry.dot11BeaconPeriod << 8)), 0, 0);
	set_fw_reg(priv, (0xF1000000 | (DEFAULT_BINTVAL << 8)), 0, 0);

	// Set max AMPDU aggregation time
	max_aggre_time = 0xc0; // in 4us
	set_fw_reg(priv, (0xFD0000B1|((max_aggre_time << 8) & 0xff00)), 0 ,0);

//	RTL_W32(0x2A4, 0x00006300);
//	RTL_W32(0x2A0, 0xb026007C);
//	mdelay(1);
//	while(RTL_R32(0x2A0) != 0){};
//	RTL_W16(ATIMWND, 0); //0

	RTL_W16(BCN_DRV_EARLY_INT, (10<<4)); // 2


/*	if (!((OPMODE & WIFI_AP_STATE)
#if defined(WDS) && defined(CONFIG_RTK_MESH)
		&& ((priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure) || priv->pmib->dot1180211sInfo.meshSilence))
#elif defined(WDS)
		&& priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure )
#elif defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
		&& priv->pmib->dot1180211sInfo.meshSilence )
#else
		)
#endif
	)

		RTL_W16(BCN_DRV_EARLY_INT, RTL_R16(BCN_DRV_EARLY_INT)|BIT(15)); // sw beacon

*/
#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable && RTL8190_NUM_VWLAN == 1 &&
					priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < 30)
		RTL_W16(BCN_DRV_EARLY_INT, (RTL_R16(BCN_DRV_EARLY_INT)&0xf00f) | ((6<<4)&0xff0));
#endif

	RTL_W16(BCN_DMATIME, 0x400); // 1ms

// for debug
/*#ifdef CLIENT_MODE
	if (OPMODE & WIFI_ADHOC_STATE)
		RTL_W8(BCN_ERR_THRESH, 100);
#endif*/
//--------------

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
/*	if (OPMODE & WIFI_AP_STATE)
		RTL_W16(BCNTCFG, 0x000a);
	else 
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
	RTL_W16(BCNTCFG, 0x0204);
	*/


	// Ack ISR, and then unmask IMR
#if 0
	RTL_W32(ISR, RTL_R32(ISR));
	RTL_W32(ISR+4, RTL_R16(ISR+4));
	RTL_W32(IMR, 0x0);
	RTL_W32(IMR+4, 0x0);
	priv->pshare->InterruptMask = _ROK_ | _BCNDMAINT_ | _RDU_ | _RXFOVW_ | _RXCMDOK_;
	priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
	IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
	IMR_BDOK | IMR_RXCMDOK | /*IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
	IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/);// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;
#endif
	priv->pshare_hw->InterruptMask = IMR_ROK | IMR_BcnInt |IMR_RDU | IMR_RXFOVW ;
#if 0
	priv->pshare_hw->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
	IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
	IMR_BDOK | IMR_RXCMDOK | /*IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|		\
	 IMR_BcnInt/*| IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/);
#endif
#ifdef MP_TEST
	priv->pshare_hw->InterruptMask	|= IMR_BEDOK;
#endif
	priv->pshare_hw->InterruptMaskExt = 0;

//	if (opmode & WIFI_AP_STATE)
		priv->pshare_hw->InterruptMask |= IMR_BDOK;

//#ifdef CLIENT_MODE
	//else if (opmode & WIFI_ADHOC_STATE)
		//priv->pshare_hw->InterruptMaskExt |= (IMR_TBDER | IMR_TBDOK);
//#endif


//	if (priv->pmib->miscEntry.ack_timeout && (priv->pmib->miscEntry.ack_timeout < 0xff))
//		RTL_W8(ACK_TIMEOUT, priv->pmib->miscEntry.ack_timeout);


	// FGPA does not have eeprom now
//	RTL_W8(_9346CR_, 0);
/*
	// ===========================================================================================
	// Download Firmware
	// allocate memory for tx cmd packet
	if((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
		printk("not enough memory for txcmd_buf\n");
		return -1;
	}

	priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
			LoadPktSize, PCI_DMA_TODEVICE);

	if(LoadFirmware(priv) == FALSE){
		printk("Load Firmware Fail!\n");
		return -1;
	}else {
		mdelay(20);
		PRINT_INFO("Load firmware successful! \n");
	}

	MacConfigAfterFwDownload(priv);
*/

	kfree(priv->pshare_hw->txcmd_buf);

//	if (opmode & WIFI_AP_STATE)
	//
//		if (priv->auto_channel == 0) 
			//
			//BUG_INFO("going to init beacon\n");
			//chris : init_beacon when hostapd call bss_info_change
#ifndef NEW_MAC80211_DRV
			init_beacon(priv);
#endif
		//
	//

	//enable interrupt
	RTL_W32(IMR, priv->pshare_hw->InterruptMask);
	RTL_W32(IMR+4, priv->pshare_hw->InterruptMaskExt);
//	RTL_W32(IMR, 0xffffffff);
//	RTL_W8(IMR+4, 0x3f);

	// ===========================================================================================


		// for test, loopback
//		RTL_W8(MSR, MSR_NOLINK);
//		RTL_W8(LBKMD_SEL, BIT(0)| BIT(1) | BIT(3));


/*
#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
	else
#endif
	{
		if (opmode & WIFI_AP_STATE)
			priv->pshare_hw->CurrentChannelBW = priv->pshare->is_40m_bw;
		else
			priv->pshare_hw->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
	}
*/

/*
	if (get_rf_mimo_mode(priv) == MIMO_2T2R)
	{
		if ((priv->pmib->dot11RFEntry.LOFDM_pwd_A == 0) ||
			(priv->pmib->dot11RFEntry.LOFDM_pwd_B == 0)) {
			priv->pshare->use_default_para = 1;
			priv->pshare->legacyOFDM_pwrdiff_A = 0;
			priv->pshare->legacyOFDM_pwrdiff_B = 0;
		}
		else
		{
			priv->pshare_hw->use_default_para = 0;

			if (priv->pmib->dot11RFEntry.LOFDM_pwd_A >= 32)
				priv->pshare_hw->legacyOFDM_pwrdiff_A = (priv->pmib->dot11RFEntry.LOFDM_pwd_A - 32) & 0x0f;
			else {
				priv->pshare_hw->legacyOFDM_pwrdiff_A = (32 - priv->pmib->dot11RFEntry.LOFDM_pwd_A) & 0x0f;
				priv->pshare_hw->legacyOFDM_pwrdiff_A |= 0x80;
			}

			if (priv->pmib->dot11RFEntry.LOFDM_pwd_B >= 32)
				priv->pshare->legacyOFDM_pwrdiff_B = (priv->pmib->dot11RFEntry.LOFDM_pwd_B - 32) & 0x0f;
			else {
				priv->pshare_hw->legacyOFDM_pwrdiff_B = (32 - priv->pmib->dot11RFEntry.LOFDM_pwd_B) & 0x0f;
				priv->pshare_hw->legacyOFDM_pwrdiff_B |= 0x80;
			}
		}
	}
*/
	
/*	
	else
	{
		if (priv->pmib->dot11RFEntry.LOFDM_pwd_A == 0) {
			priv->pshare->use_default_para = 1;
			priv->pshare->legacyOFDM_pwrdiff_A = 0;
			priv->pshare->legacyOFDM_pwrdiff_B = 0;
		}
		else
		{
			priv->pshare->use_default_para = 0;

			if (priv->pmib->dot11RFEntry.LOFDM_pwd_A >= 32)
				priv->pshare->legacyOFDM_pwrdiff_A = (priv->pmib->dot11RFEntry.LOFDM_pwd_A - 32) & 0x0f;
			else {
				priv->pshare->legacyOFDM_pwrdiff_A = (32 - priv->pmib->dot11RFEntry.LOFDM_pwd_A) & 0x0f;
				priv->pshare->legacyOFDM_pwrdiff_A |= 0x80;
			}

			if (priv->pmib->dot11RFEntry.LOFDM_pwd_B != 0) {
				if (priv->pmib->dot11RFEntry.LOFDM_pwd_B >= 32)
					priv->pshare->legacyOFDM_pwrdiff_B = (priv->pmib->dot11RFEntry.LOFDM_pwd_B - 32) & 0x0f;
				else {
					priv->pshare->legacyOFDM_pwrdiff_B = (32 - priv->pmib->dot11RFEntry.LOFDM_pwd_B) & 0x0f;
					priv->pshare->legacyOFDM_pwrdiff_B |= 0x80;
				}
			}
			else
				priv->pshare->legacyOFDM_pwrdiff_B = priv->pshare->legacyOFDM_pwrdiff_A;
		}
	}
*/

	// get default Tx AGC offset
	*(unsigned int *)(&priv->pshare_hw->phw->MCSTxAgcOffset[0])  = RTL_R32(rTxAGC_Mcs03_Mcs00);
	*(unsigned int *)(&priv->pshare_hw->phw->MCSTxAgcOffset[4])  = RTL_R32(rTxAGC_Mcs07_Mcs04);
	*(unsigned int *)(&priv->pshare_hw->phw->MCSTxAgcOffset[8])  = RTL_R32(rTxAGC_Mcs11_Mcs08);
	*(unsigned int *)(&priv->pshare_hw->phw->MCSTxAgcOffset[12]) = RTL_R32(rTxAGC_Mcs15_Mcs12);
	*(unsigned int *)(&priv->pshare_hw->phw->OFDMTxAgcOffset[0]) = RTL_R32(rTxAGC_Rate18_06);
	*(unsigned int *)(&priv->pshare_hw->phw->OFDMTxAgcOffset[4]) = RTL_R32(rTxAGC_Rate54_24);
	*(unsigned int *)(&priv->pshare_hw->phw->CCKTxAgc) = RTL_R8(rTxAGC_CCK_Mcs32 + 1);

 

//	if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x1d)) {
//		DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", priv->pmib->dot11RFEntry.ther);
//		priv->pmib->dot11RFEntry.ther = 0;
//	}

/*
	if (opmode & WIFI_AP_STATE)
	{
		if (priv->auto_channel == 0) {
			DEBUG_INFO("going to init beacon\n");
			init_beacon(priv);
		}
	}
*/
	/*---- Set CCK and OFDM Block "ON"----*/
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);
	mdelay(2);

#if	defined(RTL8192)
	//Turn on the RF8256, joshua
	PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 1<<4, 0x1);		// 0x860[4]
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x300, 0x3);		// 0x88c[4]
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, 0x60, 0x3); 		// 0x880[6:5]
	PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0xf, 0x3);			// 0xc04[3:0]
	PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, 0xf, 0x3);			// 0xd04[3:0]
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, 0x7000, 0x3);

	//Enable LED
	RTL_W32(0x87, 0x0);
#endif

	//SwBWMode(priv, priv->pshare_hw->CurrentChannelBW, priv->pshare_hw->offset_2nd_chan);
	//SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare_hw->offset_2nd_chan);
	if(priv->pshare_hw->rf_ft_var.ofdm_1ss_oneAnt == 1){// use one PATH for ofdm and 1SS
		Switch_1SS_Antenna(priv, 2);
		Switch_OFDM_Antenna(priv, 2);
	}
/*
	// set Max Short GI Rate
	if (priv->pmib->dot11StationConfigEntry.autoRate == 1) {
		// set by fw, 20090707
#if 0
		if (get_rf_mimo_mode(priv) == MIMO_2T2R)
			RTL_W16(SG_RATE, 0xffff);
		else // 1T2R, 1T1R?
			RTL_W16(SG_RATE, 0x7777);
#endif
	}
	else { // fixed rate
		fixed_rate = (priv->pmib->dot11StationConfigEntry.fixedTxRate >> 12) & 0xffff;
		calc_rate = 0;
		for (ii = 0; ii < 16; ii++) {
			if ((fixed_rate & 0x1) == 0)
				calc_rate += 1;
			else
				break;
			fixed_rate >>= 1;
		}
		RTL_W16(SG_RATE, (calc_rate)|(calc_rate << 4) | (calc_rate << 8) | (calc_rate<<12));
	}
*/

/*
	// disable tx brust for wifi
	if (priv->pmib->dot11OperationEntry.wifi_specific)
		set_fw_reg(priv, 0xfd0001b0, 0, 0);
*/

/*
	// enable RIFS function for wifi
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
		priv->pmib->dot11OperationEntry.wifi_specific)
		PHY_SetBBReg(priv, 0x818, 0xffff, 0x03c5);
*/

	// IQCal
	set_fw_reg(priv, 0xf0000020, 0, 0);
#ifdef _DEBUG_RTL8190_
	{
		unsigned int reply;
		mdelay(150);
		reply = RTL_R32(0x2c4) & 0xffff;
		if (reply == 0xdddd) {
			DEBUG_INFO("IQK: Cal success!\n");
		}
		else if (reply == 0xffff) {
			DEBUG_WARN("IQK: Cal fail!\n");
		}
		else {
			DEBUG_ERR("IQK: returned unknown 0x%04x\n", reply);
		}
	}
#endif

	mdelay(100);

	//RTL_W32(0x100, RTL_R32(0x100) | BIT(14)); //for 8190 fw debug

	DBFEXIT;

	return 0;

}


// finetune PCIe analog PHY characteristic, by Pei-Si Wu, 20090227
void rtl8192se_ePhyInit(struct mac80211_shared_priv *priv)
{
	unsigned long ioaddr = priv->pshare_hw->ioaddr;

	RTL_W16(MDIO_DATA, 0x1000);
	RTL_W8(MDIO_CTRL, 0x20);
	mdelay(1);
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W16(MDIO_DATA, 0xc49a);
	RTL_W8(MDIO_CTRL, 0x23);
	mdelay(1);
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W16(MDIO_DATA, 0xa5bc);
	RTL_W8(MDIO_CTRL, 0x26);
	mdelay(1);
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W16(MDIO_DATA, 0x1a80);
	RTL_W8(MDIO_CTRL, 0x27);
	mdelay(1);
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W16(MDIO_DATA, 0xff80);
	RTL_W8(MDIO_CTRL, 0x39);
	mdelay(1);
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W16(MDIO_DATA, 0xa0eb);
	RTL_W8(MDIO_CTRL, 0x3e);
	mdelay(1);
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	// debug print
#if 0
	RTL_W8(MDIO_CTRL, 0x40);
	mdelay(1);
	printk("ePhyReg0x%02x=0x%04x\n", 0x00, RTL_R16(MDIO_DATA+2));
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W8(MDIO_CTRL, 0x43);
	mdelay(1);
	printk("ePhyReg0x%02x=0x%04x\n", 0x03, RTL_R16(MDIO_DATA+2));
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W8(MDIO_CTRL, 0x46);
	mdelay(1);
	printk("ePhyReg0x%02x=0x%04x\n", 0x06, RTL_R16(MDIO_DATA+2));
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W8(MDIO_CTRL, 0x47);
	mdelay(1);
	printk("ePhyReg0x%02x=0x%04x\n", 0x07, RTL_R16(MDIO_DATA+2));
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W8(MDIO_CTRL, 0x59);
	mdelay(1);
	printk("ePhyReg0x%02x=0x%04x\n", 0x19, RTL_R16(MDIO_DATA+2));
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);

	RTL_W8(MDIO_CTRL, 0x5e);
	mdelay(1);
	printk("ePhyReg0x%02x=0x%04x\n", 0x1e, RTL_R16(MDIO_DATA+2));
	RTL_W8(MDIO_CTRL, 0);
	mdelay(10);
#endif
}
#endif

#ifdef RTL8192SU
int rtl819x_init_hw_USB(struct mac80211_shared_priv *priv)
{
	u16 val16;
	u32 val32;
	int max_aggre_time, ret=0;
	unsigned char net_work_type = WIRELESS_11N | WIRELESS_11G | WIRELESS_11B;


	//.0 check chip id - read efuse data
	ret = read_efuse_content(priv);



	//.1 pos init	- Power on init before FW Downloading.
	ret = rtl819x_pos_init(priv);

	
	//.2 download f/w
#if 0	
	ret = rtl819x_fw_init(priv);	
	if(ret)
	{
		printk("rtl819x_init_hw: download f/w fail!\n");
		goto hw_init_fail;
	}
#else	
	if(LoadFirmware(priv) == FALSE)
	{
		printk("rtl819x_init_hw: download f/w fail!\n");
		ret = -1;
		goto hw_init_fail;
	}	
#endif

		PRINT_INFO("Load firmware successful! \n");

#if 0
	//.3 mac init	
	ret = rtl819x_mac_init(priv);	

#else	
	//.3 mac init
	MacConfigAfterFwDownload(priv);
	ret = rtl819x_mac_init(priv);
	PHY_ConfigMACWithParaFile(priv);//Initialize MAC/PHY Config by MACPHY_reg.txt




	//.4 bb init
	ret = rtl819x_bb_init(priv);
	if(phy_BB8192SE_Config_ParaFile(priv)) //Initialize BB After MAC Config PHY_reg.txt, AGC_Tab.txt
	{
		printk("phy_BB8192SE_Config_ParaFile init fail!\n");
		ret = -1;
		goto hw_init_fail;
	}	


	// Set NAV protection length
	// CF-END Threshold
	RTL_W16(NAV_PROT_LEN, 0x01C0);
	RTL_W8(CFEND_TH, 0xFF);
	
	// For RF test only from Scott's suggestion
	RTL_W8(0x27, 0xDB);
	//RTL_W8(0x1B, 0x07); // ACUT

	// set RCR: RX_SHIFT and disable ACF
	RTL_W8(0x48, 0x3e);
	RTL_W32(0x48, RTL_R32(0x48) & ~ RCR_ACF  & ~RCR_ACRC32);
	// for debug by victoryman, 20081119
	RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYST_RXFF);


	
	//.5 rf init
	ret = rtl819x_rf_init(priv);
	if(phy_RF8225_Config_ParaFile(priv)) //Initialize RF
	{
		printk("phy_RF8225_Config_ParaFile init fail!\n");
		ret = -1;
		goto hw_init_fail;
	}
	
	

	//.6 misc init
	ret = rtl819x_misc_init(priv);


	//clear TPPoll
	//RTL_W16(TPPoll, 0x0);

	//configure timing parameter
	RTL_W8(ACK_TIMEOUT, 0x30);
	RTL_W8(PIFS_TIME,0x13);
	//RTL_W16(LBDLY, 0x060F);
	//RTL_W16(SIFS_OFDM, 0x0e0e);
	//RTL_W8(SLOT_TIME, 0x10);


	if (net_work_type & WIRELESS_11N) {

		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else if (net_work_type & WIRELESS_11A) {		
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else if (net_work_type & WIRELESS_11G) {		
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else { // WIRELESS_11B
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x14);
	}

	init_EDCA_para(priv, net_work_type);


	//we don't have EEPROM yet, Mark this for FPGA Platform
	//RTL_W8(_9346CR_, CR9346_CFGRW);

	// Retry Limit
	/*	
	if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
		val16 = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
	else {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			val16 = 0x30;
		else
			val16 = 0x06;
	}
	if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
		val16 |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
	else {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			val16 |= (0x30 << 8);
		else
			val16 |= (0x06 << 8);
	}
	*/
	
	val16 = 0x30 |  (0x30 << 8);
	RTL_W16(RETRY_LIMIT, val16);

	/*This should be done later, but Windows Driver not done yet.
	// Response Rate Set
	// let ACK sent by highest of 24Mbps
	val32 = 0x1ff;
	if (pmib->dot11RFEntry.shortpreamble)
		val32 |= BIT(23);
	RTL_W32(_RRSR_, val32);
	*/

	// Adaptive Rate Table for Basic Rate
	/*
	val32 = 0;
	for (i=0; i<32; i++) {
		if (AP_BSSRATE[i]) {
			if (AP_BSSRATE[i] & 0x80)
				val32 |= get_bit_value_from_ieee_value(AP_BSSRATE[i] & 0x7f);
		}
	}
	val32 |= (priv->pmib->dot11nConfigEntry.dot11nBasicMCS << 12);
	*/
	val32= 0xfffffff;
	set_fw_reg(priv, (0xfd00002a | (1<<9 | ARFR_BMC)<<8), val32 ,1);

	//panic_printk("0x2c4 = bitmap = 0x%08x\n", (unsigned int)val32);
	//panic_printk("0x2c0 = cmd | macid | band = 0x%08x\n", 0xfd0000a2 | (1<<9 | (sta_band & 0xf))<<8);
	//panic_printk("Add id %d val %08x to ratr\n", 0, (unsigned int)val32);

	/*for (i = 0; i < 8; i++)
#ifdef RTL8192SE_ACUT
		RTL_W32(ARFR0+i*4, val32 & 0x1f0ff0f0);
#else
		RTL_W32(ARFR0+i*4, val32 & 0x1f0ff0f0);
#endif
	*/

	//settting initial rate for control frame to 24M
	RTL_W8(INIRTSMCS_SEL, 8);

	//setting MAR
	RTL_W32(MAR, 0xffffffff);
	RTL_W32((MAR+4), 0xffffffff);

	//setting BSSID, not matter AH/AP/station
	//memcpy((void *)&val32, (pmib->dot11OperationEntry.hwaddr), 4);
	//memcpy((void *)&val16, (pmib->dot11OperationEntry.hwaddr + 4), 2);

	val32 = 0x00e04c81;
	val16= 0x96bc;

	RTL_W32(BSSIDR, cpu_to_le32(val32));
	RTL_W16((BSSIDR + 4), cpu_to_le16(val16));


	//setting TCR
	//TCR, use default value

	//setting RCR // set in MacConfigAfterFwDownload
	//RTL_W32(_RCR_, _APWRMGT_ | _AMF_ | _ADF_ | _AICV_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
	//if (pmib->dot11OperationEntry.crc_log)
	//		RTL_W32(_RCR_, RTL_R32(_RCR_) | _ACRC32_);

	//setting MSR
	//if (opmode & WIFI_AP_STATE)
	if( priv->opmode == NL80211_IFTYPE_AP)
	{
		DEBUG_INFO("AP-mode enabled...\n");
/*
#if defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
		if (priv->pmib->dot11WdsInfo.wdsPure || priv->pmib->dot1180211sInfo.meshSilence)
#else
		if (priv->pmib->dot11WdsInfo.wdsPure)
#endif
			RTL_W8(MSR, MSR_NOLINK);
		else
*/			
			RTL_W8(MSR, MSR_AP);
// Move init beacon after f/w download
#if 0
		if (priv->auto_channel == 0) {
			DEBUG_INFO("going to init beacon\n");
			init_beacon(priv);
		}
#endif
	}
#ifdef CLIENT_MODE
	else if (priv->opmode == NL80211_IFTYPE_STATION)
	{
		DEBUG_INFO("Station-mode enabled...\n");
		RTL_W8(MSR, MSR_INFRA);
	}
	else if (priv->opmode == NL80211_IFTYPE_ADHOC)
	{
		DEBUG_INFO("Adhoc-mode enabled...\n");
		RTL_W8(MSR, MSR_ADHOC);
	}
#endif
	else if (priv->opmode == NL80211_IFTYPE_MONITOR)
	{
		DEBUG_INFO("Monitor-mode enabled...\n");
		RTL_W8(MSR, MSR_NOLINK);
	}
	else
	{
		printk("Operation mode error!\n");
		ret = -1;
		goto hw_init_fail;
	}

	CamResetAllEntry(priv);
	RTL_W16(SECR, 0x0000);

/*	
	if ((OPMODE & (WIFI_AP_STATE|WIFI_STATION_STATE|WIFI_ADHOC_STATE)) &&
		!priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) {
		pmib->dot11GroupKeysTable.dot11Privacy = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
			i = 5;
		else
			
			i = 13;
		

#ifdef CONFIG_RTL8196B_KLD
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
#else
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], i);
#endif
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;

	}
*/
/*
	//here add if legacy WEP
	// if 1x is enabled, do not set default key, david
//#if 0	// marked by victoryman, use pairwise key at present, 20070627
//#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD)
#ifdef USE_WEP_DEFAULT_KEY
#ifdef MBSSID
	if (!(OPMODE & WIFI_AP_STATE) || !priv->pmib->miscEntry.vap_enable)
#endif
	{
		if(!SWCRYPTO && !IEEE8021X_FUN &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_))
			init_DefaultKey_Enc(priv, NULL, 0);
	}
#endif
*/

	//Setup Beacon Interval/interrupt interval, ATIM-WIND ATIM-Interrupt
//	RTL_W16(BCN_INTERVAL, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	//Setting BCNITV is done by firmware now
//	set_fw_reg(priv, (0xF1000000 | (pmib->dot11StationConfigEntry.dot11BeaconPeriod << 8)), 0, 0);
	set_fw_reg(priv, (0xF1000000 | (DEFAULT_BINTVAL << 8)), 0, 0);

	// Set max AMPDU aggregation time
	max_aggre_time = 0xc0; // in 4us
	set_fw_reg(priv, (0xFD0000B1|((max_aggre_time << 8) & 0xff00)), 0 ,0);

//	RTL_W32(0x2A4, 0x00006300);
//	RTL_W32(0x2A0, 0xb026007C);
//	mdelay(1);
//	while(RTL_R32(0x2A0) != 0){};
//	RTL_W16(ATIMWND, 0); //0

	RTL_W16(BCN_DRV_EARLY_INT, (10<<4)); // 2


/*	if (!((OPMODE & WIFI_AP_STATE)
#if defined(WDS) && defined(CONFIG_RTK_MESH)
		&& ((priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure) || priv->pmib->dot1180211sInfo.meshSilence))
#elif defined(WDS)
		&& priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure )
#elif defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
		&& priv->pmib->dot1180211sInfo.meshSilence )
#else
		)
#endif
	)

		RTL_W16(BCN_DRV_EARLY_INT, RTL_R16(BCN_DRV_EARLY_INT)|BIT(15)); // sw beacon

*/

#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable && RTL8190_NUM_VWLAN == 1 &&
		priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < 30)
		
	RTL_W16(BCN_DRV_EARLY_INT, (RTL_R16(BCN_DRV_EARLY_INT)&0xf00f) | ((6<<4)&0xff0));
#endif

	RTL_W16(BCN_DMATIME, 0x400); // 1ms

// for debug
/*#ifdef CLIENT_MODE
	if (OPMODE & WIFI_ADHOC_STATE)
		RTL_W8(BCN_ERR_THRESH, 100);
#endif*/
//--------------

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
/*	if (OPMODE & WIFI_AP_STATE)
		RTL_W16(BCNTCFG, 0x000a);
	else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
		RTL_W16(BCNTCFG, 0x0204);
*/


	// Ack ISR, and then unmask IMR
#if 0
	RTL_W32(ISR, RTL_R32(ISR));
	RTL_W32(ISR+4, RTL_R16(ISR+4));
	RTL_W32(IMR, 0x0);
	RTL_W32(IMR+4, 0x0);
	priv->pshare->InterruptMask = _ROK_ | _BCNDMAINT_ | _RDU_ | _RXFOVW_ | _RXCMDOK_;
	priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
	IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
	IMR_BDOK | IMR_RXCMDOK | /*IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
	IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/);// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;
#endif

	priv->pshare_hw->InterruptMask = IMR_ROK | IMR_BcnInt |IMR_RDU | IMR_RXFOVW ;

#if 0
	priv->pshare_hw->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
	IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
	IMR_BDOK | IMR_RXCMDOK | /*IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|		\
	 IMR_BcnInt/*| IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/);
#endif
	
#ifdef MP_TEST
	priv->pshare_hw->InterruptMask	|= IMR_BEDOK;
#endif
	priv->pshare_hw->InterruptMaskExt = 0;

//	if (opmode & WIFI_AP_STATE)
	priv->pshare_hw->InterruptMask |= IMR_BDOK;

//#ifdef CLIENT_MODE
	//else if (opmode & WIFI_ADHOC_STATE)
		//priv->pshare_hw->InterruptMaskExt |= (IMR_TBDER | IMR_TBDOK);
//#endif


//	if (priv->pmib->miscEntry.ack_timeout && (priv->pmib->miscEntry.ack_timeout < 0xff))
//		RTL_W8(ACK_TIMEOUT, priv->pmib->miscEntry.ack_timeout);


	// FGPA does not have eeprom now
	//RTL_W8(_9346CR_, 0);


	//enable interrupt
	RTL_W32(IMR, priv->pshare_hw->InterruptMask);
	RTL_W32(IMR+4, priv->pshare_hw->InterruptMaskExt);
	//RTL_W32(IMR, 0xffffffff);
	//RTL_W8(IMR+4, 0x3f);


	// get default Tx AGC offset
	*(unsigned int *)(&priv->pshare_hw->phw->MCSTxAgcOffset[0])  = RTL_R32(rTxAGC_Mcs03_Mcs00);
	*(unsigned int *)(&priv->pshare_hw->phw->MCSTxAgcOffset[4])  = RTL_R32(rTxAGC_Mcs07_Mcs04);
	*(unsigned int *)(&priv->pshare_hw->phw->MCSTxAgcOffset[8])  = RTL_R32(rTxAGC_Mcs11_Mcs08);
	*(unsigned int *)(&priv->pshare_hw->phw->MCSTxAgcOffset[12]) = RTL_R32(rTxAGC_Mcs15_Mcs12);
	*(unsigned int *)(&priv->pshare_hw->phw->OFDMTxAgcOffset[0]) = RTL_R32(rTxAGC_Rate18_06);
	*(unsigned int *)(&priv->pshare_hw->phw->OFDMTxAgcOffset[4]) = RTL_R32(rTxAGC_Rate54_24);
	*(unsigned int *)(&priv->pshare_hw->phw->CCKTxAgc) = RTL_R8(rTxAGC_CCK_Mcs32 + 1);


	/*---- Set CCK and OFDM Block "ON"----*/
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);
	mdelay(2);

#if	defined(RTL8192)
	//Turn on the RF8256, joshua
	PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 1<<4, 0x1);		// 0x860[4]
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x300, 0x3);		// 0x88c[4]
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, 0x60, 0x3); 		// 0x880[6:5]
	PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0xf, 0x3);			// 0xc04[3:0]
	PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, 0xf, 0x3);			// 0xd04[3:0]
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, 0x7000, 0x3);

	//Enable LED
	RTL_W32(0x87, 0x0);
#endif

	//SwBWMode(priv, priv->pshare_hw->CurrentChannelBW, priv->pshare_hw->offset_2nd_chan);
	//SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare_hw->offset_2nd_chan);
	if(priv->pshare_hw->rf_ft_var.ofdm_1ss_oneAnt == 1)
	{
		// use one PATH for ofdm and 1SS
		Switch_1SS_Antenna(priv, 2);
		Switch_OFDM_Antenna(priv, 2);
	}

/*
	// set Max Short GI Rate
	if (priv->pmib->dot11StationConfigEntry.autoRate == 1) {
		// set by fw, 20090707
#if 0
		if (get_rf_mimo_mode(priv) == MIMO_2T2R)
			RTL_W16(SG_RATE, 0xffff);
		else // 1T2R, 1T1R?
			RTL_W16(SG_RATE, 0x7777);
#endif
	}
	else { // fixed rate
		fixed_rate = (priv->pmib->dot11StationConfigEntry.fixedTxRate >> 12) & 0xffff;
		calc_rate = 0;
		for (ii = 0; ii < 16; ii++) {
			if ((fixed_rate & 0x1) == 0)
				calc_rate += 1;
			else
				break;
			fixed_rate >>= 1;
		}
		RTL_W16(SG_RATE, (calc_rate)|(calc_rate << 4) | (calc_rate << 8) | (calc_rate<<12));
	}
*/

/*
	// disable tx brust for wifi
	if (priv->pmib->dot11OperationEntry.wifi_specific)
		set_fw_reg(priv, 0xfd0001b0, 0, 0);
*/

/*
	// enable RIFS function for wifi
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
		priv->pmib->dot11OperationEntry.wifi_specific)
		PHY_SetBBReg(priv, 0x818, 0xffff, 0x03c5);
*/

	// IQCal
	set_fw_reg(priv, 0xf0000020, 0, 0);

#ifdef _DEBUG_RTL8190_
	{
		unsigned int reply;
		mdelay(150);
		reply = RTL_R32(0x2c4) & 0xffff;
		if (reply == 0xdddd) {
			DEBUG_INFO("IQK: Cal success!\n");
		}
		else if (reply == 0xffff) {
			DEBUG_WARN("IQK: Cal fail!\n");
		}
		else {
			DEBUG_ERR("IQK: returned unknown 0x%04x\n", reply);
		}
	}
#endif

#endif

	//keep RCR Values
	priv->pshare_hw->rcr_reg = RTL_R32(RCR);

	mdelay(100);

	return 0;
	
hw_init_fail:

	return ret;

}
#endif


#endif




static int LoadFirmware(struct mac80211_shared_priv *priv)
{

	//we should read header first
#ifdef RTL8192SU
	rtl8192SU_ReadIMG(priv);
#else	
	rtl8192SE_ReadIMG(priv);
#endif

	if(LoadIMEMIMG(priv) == FALSE)
		return FALSE;

	if(LoadEMEMIMG(priv) == FALSE)
		return FALSE;

	if(LoadDMEMIMG(priv) == FALSE)
		return FALSE;


	PRINT_INFO("RTL8192 Firmware version: %04x(%d.%d)\n",
		priv->pshare_hw->fw_version, priv->pshare_hw->fw_src_version, priv->pshare_hw->fw_sub_version);

	return TRUE;
}


#define	SET_RTL8192SE_RF_HALT(priv)						\
{ 														\
	unsigned char u1bTmp;								\
	unsigned long ioaddr=priv->pshare_hw->ioaddr;			\
														\
	do													\
	{													\
		u1bTmp = RTL_R8(LDOV12D_CTRL);					\
		u1bTmp |= BIT(0); 								\
		RTL_W8(LDOV12D_CTRL, u1bTmp);					\
		RTL_W8(SPS1_CTRL, 0x0);							\
		RTL_W8(TXPAUSE, 0xFF);							\
		RTL_W16(CMDR, 0x57FC);							\
		udelay(100);									\
		RTL_W16(CMDR, 0x77FC);							\
		RTL_W8(PHY_CCA, 0x0);							\
		udelay(10);									\
		RTL_W16(CMDR, 0x37FC);							\
		udelay(10);									\
		RTL_W16(CMDR, 0x77FC);							\
		udelay(10);									\
		RTL_W16(CMDR, 0x57FC);							\
		RTL_W16(CMDR, 0x0000);							\
		u1bTmp = RTL_R8((SYS_CLKR + 1));				\
		if (u1bTmp & BIT(7))							\
		{												\
			u1bTmp &= ~(BIT(6) | BIT(7));				\
			if (!HalSetSysClk8192SE(priv, u1bTmp))		\
			break;										\
		}												\
		RTL_W8(0x03, 0x71);								\
		RTL_W8(0x09, 0x70);								\
		RTL_W8(0x29, 0x68);								\
		RTL_W8(0x28, 0x00);								\
		RTL_W8(0x20, 0x50);								\
		RTL_W8(0x26, 0x0E);								\
	} while (FALSE);									\
}


int rtl819x_stop_hw(struct mac80211_shared_priv *priv, int reset_bb)
{
	static RF90_RADIO_PATH_E eRFPath;
	static BB_REGISTER_DEFINITION_T *pPhyReg;
	static int do_mac_reset;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;

	do_mac_reset = 1;
	

	if (reset_bb) {
		for (eRFPath = RF90_PATH_A; eRFPath< priv->pshare_hw->phw->NumTotalRFPath; eRFPath++) {
			pPhyReg = &priv->pshare_hw->phw->PHYRegDef[eRFPath];
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV, bRFSI_RFENV);
			PHY_SetBBReg(priv, pPhyReg->rfintfo, bRFSI_RFENV, 0);
		}
		
		RTL_W32(IMR, 0x0);
		RTL_W32(IMR+4, 0x0);
		RTL_W8(MSR, MSR_NOLINK);

		SET_RTL8192SE_RF_HALT(priv);
		udelay(120);
		RTL_W16(CMDR, 0);
		mdelay(1);

	}
	if (reset_bb)
		do_mac_reset = 0;

	if (do_mac_reset) {
		udelay(800); // it is critial!
	}
	return SUCCESS;
}


void SwBWMode(struct mac80211_shared_priv *priv, unsigned int bandwidth, int offset)
{
	static unsigned long ioaddr;
	static unsigned char regBwOpMode, nCur40MhzPrimeSC;

	ioaddr = priv->pshare_hw->ioaddr;

	DEBUG_INFO("SwBWMode(): Switch to %s bandwidth\n", bandwidth?"40MHz":"20MHz");

	//3 <1> Set MAC register
	regBwOpMode = RTL_R8(_BWOPMODE_);

	switch (bandwidth)
	{
		case HT_CHANNEL_WIDTH_20:
			regBwOpMode |= BW_OPMODE_20MHZ;
			RTL_W8(_BWOPMODE_, regBwOpMode);
			break;
		case HT_CHANNEL_WIDTH_20_40:
			regBwOpMode &= ~BW_OPMODE_20MHZ;
			RTL_W8(_BWOPMODE_, regBwOpMode);
			break;
		default:
			DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
			return;
			break;
	}

	//3 <2> Set PHY related register
	if (offset == 1)
		nCur40MhzPrimeSC = 2;
	else
		nCur40MhzPrimeSC = 1;
	switch (bandwidth)
	{
		case HT_CHANNEL_WIDTH_20:
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x0);
			PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x0);

#if defined(RTL8192SE) || defined(RTL8192SU)
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, 0x00100000, 1);
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, 0x000000ff, 0x58);
						// From SD3 WHChang
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, 0x00300000, 3);
#endif
			PHY_SetBBReg(priv, rCCK0_TxFilter1, bMaskDWord, 0x1a1b0000);	//suggest by YN
			PHY_SetBBReg(priv, rCCK0_TxFilter2, bMaskDWord, 0x090e1317);	//suggest by YN
			PHY_SetBBReg(priv, rCCK0_DebugPort, bMaskDWord, 0x00000204);	//suggest by YN
			break;
		case HT_CHANNEL_WIDTH_20_40:
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x1);
			PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x1);
			PHY_SetBBReg(priv, rCCK0_System, bCCKSideBand, (nCur40MhzPrimeSC>>1));
#if defined(RTL8192SE) || defined(RTL8192SU)
			PHY_SetBBReg(priv, rOFDM1_LSTF, 0xC00, nCur40MhzPrimeSC);
#endif

#if defined(RTL8192SE) || defined(RTL8192SU)
			// From SD3 WHChang
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, 0x00300000, 3);
			// Set Control channel to upper or lower. These settings are required only for 40MHz
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, 0x00100000, 1);
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, 0x000000ff, 0x18);
//			PHY_SetBBReg(priv, rCCK0_System, bCCKSideBand, (nCur40MhzPrimeSC>>1));
//			PHY_SetBBReg(priv, rOFDM1_LSTF, 0xC00, nCur40MhzPrimeSC);
#endif
                        PHY_SetBBReg(priv, rCCK0_TxFilter1, bMaskDWord, 0x1a1b0000);    //suggest by YN
                        PHY_SetBBReg(priv, rCCK0_TxFilter2, bMaskDWord, 0x090e1317);    //suggest by YN
                        PHY_SetBBReg(priv, rCCK0_DebugPort, bMaskDWord, 0x00000204);    //suggest by YN
			break;
		default:
			DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
			return;
			break;
	}

#if	defined(RTL8192SE) || defined(RTL8192SU)

/*
	udelay(100);

	{
		unsigned int val_read;
		val_read = PHY_QueryRFReg(priv, 0, 0x18, bMask12Bits, 1);
		//switch to channel 3
		PHY_SetRFReg(priv, 0, 0x18, bMask12Bits, val_read|0x3);
	}
*/
	if(bandwidth == HT_CHANNEL_WIDTH_20_40)// switch rf to 40Mhz
	{
		unsigned int val_read;

		val_read = PHY_QueryRFReg(priv, 0, 0x18, bMask20Bits, 1);
		val_read &= ~(BIT(10)|BIT(11));
		PHY_SetRFReg(priv, 0, 0x18, bMask20Bits, val_read);
		mdelay(10);

		val_read = PHY_QueryRFReg(priv, 1, 0x18, bMask20Bits, 1);
		val_read &= ~(BIT(10)|BIT(11));
		PHY_SetRFReg(priv, 1, 0x18, bMask20Bits, val_read);
		mdelay(10);

	}else{ // 20Mhz mode
		unsigned int val_read;
		val_read = PHY_QueryRFReg(priv, 0, 0x18, bMask20Bits, 1);
		val_read |= (BIT(10)|BIT(11));
		PHY_SetRFReg(priv, 0, 0x18, bMask20Bits, val_read);

		val_read = PHY_QueryRFReg(priv, 1, 0x18, bMask20Bits, 1);
		val_read |= (BIT(10)|BIT(11));
		PHY_SetRFReg(priv, 1, 0x18, bMask20Bits, val_read);

		PHY_SetBBReg(priv, 0x840,0x0000ffff, 0x7406);
	}
#endif
	if (priv->pshare_hw->rf_ft_var.use_frq_2_3G)
		PHY_SetRFReg(priv, RF90_PATH_C, 0x2c, 0x60, 0);
}


void GetHardwareVersion(struct rtl8190_priv *priv)
{
	ULONG ioaddr = priv->pshare->ioaddr;

	if (RTL_R8(0x301) == 0x02)
		priv->pshare->VersionID = VERSION_8190_C;
	else
		priv->pshare->VersionID = VERSION_8190_B;
}


void init_EDCA_para(struct mac80211_shared_priv *priv, int mode)
{
	static ULONG ioaddr;
	static unsigned int slot_time, VO_TXOP, VI_TXOP, sifs_time;

	ioaddr = priv->pshare_hw->ioaddr;
	slot_time = 20;
	VO_TXOP = 47;
	VI_TXOP = 94;
	sifs_time = 10;

	if (mode & WIRELESS_11N)
		sifs_time = 16;

	if ((mode & WIRELESS_11N) ||
		(mode & WIRELESS_11G)) {
		slot_time = 9;
	}
	else {
		VO_TXOP = 102;
		VI_TXOP = 188;
	}

	//RTL_W32(_ACVO_PARM_, (VO_TXOP << 16) | (3 << 12) | (2 << 8) | (sifs_time + ((OPMODE & WIFI_AP_STATE)?1:2) * slot_time));
	RTL_W32(_ACVO_PARM_, (VO_TXOP << 16) | (3 << 12) | (2 << 8) | (sifs_time + slot_time));
/*#ifdef SEMI_QOS
	if (QOS_ENABLE)
		//RTL_W32(_ACVI_PARM_, (VI_TXOP << 16) | (4 << 12) | (3 << 8) | (sifs_time + ((OPMODE & WIFI_AP_STATE)?1:2) * slot_time));
		RTL_W32(_ACVI_PARM_, (VI_TXOP << 16) | (4 << 12) | (3 << 8) | (sifs_time + slot_time));
	else
#endif*/
		RTL_W32(_ACVI_PARM_, (10 << 12) | (4 << 8) | (sifs_time + 2 * slot_time));

	//RTL_W32(_ACBE_PARM_, (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
	RTL_W32(_ACBE_PARM_, (6 << 12) | (4 << 8) | (sifs_time + 3 * slot_time));

	RTL_W32(_ACBK_PARM_, (10 << 12) | (4 << 8) | (sifs_time + 7 * slot_time));

	RTL_W8(_ACM_CTRL_, 0x00);
}


#ifdef SEMI_QOS
void BE_switch_to_VI(struct rtl8190_priv *priv, int mode, char enable)
{
	ULONG ioaddr = priv->pshare->ioaddr;
	unsigned int slot_time = 20, TXOP = 47, sifs_time = 10, cw_max = 6;

	if ((mode & WIRELESS_11N) && (priv->pshare->ht_sta_num
#ifdef WDS
		|| ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum)
#endif
		))
		sifs_time = 16;

	if ((mode & WIRELESS_11N) ||
		(mode & WIRELESS_11G)) {
		slot_time = 9;
	}
	else {
		TXOP = 94;
	}

	if (priv->BE_cwmax_enhance)
		cw_max = 10;

	if (!enable) {
#ifdef RTL8192SE
		/*if (priv->pmib->dot11OperationEntry.wifi_specific == 2) {
			RTL_W16(NAV_PROT_LEN, 0x80);
			RTL_W8(CFEND_TH, 0x2);
			set_fw_reg(priv, 0xfd0001b0, 0, 0);// turn off tx burst
			if (priv->BE_wifi_EDCA_enhance)
				RTL_W32(_ACBE_PARM_, (cw_max << 12) | (3 << 8) | (sifs_time + 3 * slot_time));
			else
				RTL_W32(_ACBE_PARM_, (cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
		}
		else*/
#endif
			RTL_W32(_ACBE_PARM_, (cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
	}
	else {
		if (priv->pshare->ht_sta_num
#ifdef WDS
			|| ((OPMODE & WIFI_AP_STATE) && (mode & WIRELESS_11N) &&
			priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum)
#endif
			) {
#ifdef RTL8192SE
			if (priv->pshare->txop_enlarge == 0xf) {
				// is 8192S client
				RTL_W32(_ACBE_PARM_, ((TXOP*2) << 16) |
							(6 << 12) | (4 << 8) | (sifs_time + slot_time+ 0xf)); // 0xf is 92s circuit delay
				priv->pshare->txop_enlarge = 2;
			}
			else if (priv->pshare->txop_enlarge == 0xe) {
				// is intel client, use a different edca value
				RTL_W32(_ACBE_PARM_, (TXOP << 16) | (cw_max << 12) | (4 << 8) | 0x2b);
				priv->pshare->txop_enlarge = 1;
			}
			else
#endif
			RTL_W32(_ACBE_PARM_, ((TXOP*priv->pshare->txop_enlarge) << 16) |
				(cw_max << 12) | (4 << 8) | (sifs_time + slot_time));
		}
		else {
#if defined(RTL8192SE)

			if (OPMODE & WIFI_AP_STATE)
				RTL_W32(_ACBE_PARM_, (cw_max << 12) | (4 << 8) | 0x19);
			else
				RTL_W32(_ACBE_PARM_, (TXOP << 16) | (6 << 12) | (4 << 8) | 0x19);
#endif
		}

#ifdef RTL8192SE
		if (priv->pmib->dot11OperationEntry.wifi_specific == 2) {
			RTL_W16(NAV_PROT_LEN, 0x01C0);
			RTL_W8(CFEND_TH, 0xFF);
			set_fw_reg(priv->share_priv, 0xfd000ab0, 0, 0);
		}
#endif
	}
}
#endif

#if defined(NEW_MAC80211_DRV)
void setup_timer1(struct mac80211_shared_priv *priv, int timeout)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;

	RTL_W32(_TIMER1_, timeout);
	RTL_W32(_IMR_, RTL_R32(_IMR_) | _TIMEOUT1_);
}

void cancel_timer1(struct mac80211_shared_priv *priv)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;

	RTL_W32(_IMR_, RTL_R32(_IMR_) & ~_TIMEOUT1_);
}

void setup_timer2(struct mac80211_shared_priv *priv, unsigned int timeout)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	unsigned int current_value=RTL_R32(_TSFTR_L_);

	if (TSF_LESS(timeout, current_value))
		timeout = current_value+20;

	RTL_W32(_TIMER2_, timeout);
	RTL_W32(_IMR_, RTL_R32(_IMR_) | _TIMEOUT2_);
}


void cancel_timer2(struct mac80211_shared_priv *priv)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;

	RTL_W32(_IMR_, RTL_R32(_IMR_) & ~_TIMEOUT2_);
}
#else
void setup_timer1(struct rtl8190_priv *priv, int timeout)
{
	ULONG ioaddr = priv->pshare->ioaddr;

	RTL_W32(_TIMER1_, timeout);
	RTL_W32(_IMR_, RTL_R32(_IMR_) | _TIMEOUT1_);
}

void cancel_timer1(struct rtl8190_priv *priv)
{
	ULONG ioaddr = priv->pshare->ioaddr;

	RTL_W32(_IMR_, RTL_R32(_IMR_) & ~_TIMEOUT1_);
}

void setup_timer2(struct rtl8190_priv *priv, unsigned int timeout)
{
	ULONG ioaddr = priv->pshare->ioaddr;
	unsigned int current_value=RTL_R32(_TSFTR_L_);

	if (TSF_LESS(timeout, current_value))
		timeout = current_value+20;

	RTL_W32(_TIMER2_, timeout);
	RTL_W32(_IMR_, RTL_R32(_IMR_) | _TIMEOUT2_);
}


void cancel_timer2(struct rtl8190_priv *priv)
{
	ULONG ioaddr = priv->pshare->ioaddr;

	RTL_W32(_IMR_, RTL_R32(_IMR_) & ~_TIMEOUT2_);
}
#endif

// dynamic DC_TH of Fsync in regC38 for non-BCM solution
void check_DC_TH_by_rssi(struct rtl8190_priv *priv, unsigned char rssi_strength)
{
	ULONG ioaddr = priv->pshare->ioaddr;

#if defined(RTL8190) || defined(RTL8192E)
	if (!priv->fsync_monitor_pstat)
#endif
	{
		if ((priv->dc_th_current_state != DC_TH_USE_UPPER) &&
			(rssi_strength >= priv->pshare->rf_ft_var.dcThUpper)) {
#if	defined(RTL8192SE)
			RTL_W8(0xc38, 0x94);
#else
			RTL_W8(0xc38, 0x14);
#endif
			priv->dc_th_current_state = DC_TH_USE_UPPER;
		}
		else if ((priv->dc_th_current_state != DC_TH_USE_LOWER) &&
			(rssi_strength <= priv->pshare->rf_ft_var.dcThLower)) {
#if	defined(RTL8192SE)
			RTL_W8(0xc38, 0x90);
#else
			RTL_W8(0xc38, 0x10);
#endif
			priv->dc_th_current_state = DC_TH_USE_LOWER;
		}
		else if (priv->dc_th_current_state == DC_TH_USE_NONE) {
#if	defined(RTL8192SE)
			RTL_W8(0xc38, 0x94);
#else
			RTL_W8(0xc38, 0x14);
#endif
			priv->dc_th_current_state = DC_TH_USE_UPPER;
		}
	}
}


void check_DIG_by_rssi(struct rtl8190_priv *priv, unsigned char rssi_strength)
{
	ULONG ioaddr = priv->pshare->ioaddr;
	unsigned int dig_on = 0;

	if (OPMODE & WIFI_SITE_MONITOR)
		return;

	if ((rssi_strength > priv->pshare->rf_ft_var.digGoUpperLevel)
		&& (rssi_strength < 71) && (priv->pshare->phw->signal_strength != 2)) {
		if (priv->pshare->is_40m_bw)
			RTL_W8(0xc87, 0x20);
		else
			RTL_W8(0xc30, 0x44);

		if (priv->pshare->phw->signal_strength != 3)
			dig_on++;

		priv->pshare->phw->signal_strength = 2;
	}
	else if ((rssi_strength > 75) && (priv->pshare->phw->signal_strength != 3)) {
		if (priv->pshare->is_40m_bw)
			RTL_W8(0xc87, 0x10);
		else
			RTL_W8(0xc30, 0x43);

		if (priv->pshare->phw->signal_strength != 2)
			dig_on++;

		priv->pshare->phw->signal_strength = 3;
	}
	else if (((rssi_strength < priv->pshare->rf_ft_var.digGoLowerLevel)
		&& (priv->pshare->phw->signal_strength != 1)) || !priv->pshare->phw->signal_strength) {
		// DIG off
//		set_fw_reg(priv, 0xfd000001, 0, 0); //old form of fw
		RTL_W8(0x364, RTL_R8(0x364) & ~FW_REG364_DIG);

		if (priv->pshare->is_40m_bw)
			RTL_W8(0xc87, 0);
		else
			RTL_W8(0xc30, 0x42);

		priv->pshare->phw->signal_strength = 1;
	}

	if (dig_on) {
		// DIG on
//		set_fw_reg(priv, 0xfd000002, 0, 0);
		RTL_W8(0x364, RTL_R8(0x364) | FW_REG364_DIG);
	}


	check_DC_TH_by_rssi(priv, rssi_strength);
}


void DIG_for_site_survey(struct rtl8190_priv *priv, int do_ss)
{
	ULONG ioaddr = priv->pshare->ioaddr;


	if (do_ss) {
		// DIG off
		//set_fw_reg(priv, 0xfd000001, 0, 0);
		RTL_W8(0x364, RTL_R8(0x364) & ~FW_REG364_DIG);
	}
	else {
		// DIG on
		if (priv->pshare->phw->signal_strength > 1)
//		set_fw_reg(priv, 0xfd000002, 0, 0);
			RTL_W8(0x364, RTL_R8(0x364) | FW_REG364_DIG);
	}

}



#ifdef RTL8192SE
// dynamic CCK CCA enhance by rssi
void CCK_CCA_dynamic_enhance(struct rtl8190_priv *priv, unsigned char rssi_strength)
{
	unsigned long ioaddr = priv->pshare->ioaddr;

	if (!priv->pshare->phw->CCK_CCA_enhanced && (rssi_strength < 30)) {
		priv->pshare->phw->CCK_CCA_enhanced = TRUE;
		RTL_W8(0xa0a, 0x83);
	}
	else if (priv->pshare->phw->CCK_CCA_enhanced && (rssi_strength > 35)) {
		priv->pshare->phw->CCK_CCA_enhanced = FALSE;
		RTL_W8(0xa0a, 0xcd);
	}
}
#endif


#ifdef RTL8192SE
void tx_path_by_rssi(struct rtl8190_priv *priv, struct stat_info *pstat, unsigned char enable){

	if((get_rf_mimo_mode(priv->share_priv) != MIMO_2T2R))
		return; // 1T2R, 1T1R; do nothing

	if(pstat == NULL)
		return;

#ifdef	STA_EXT
	if ((pstat->remapped_aid == FW_NUM_STAT-1) ||
		(priv->pshare->has_2r_sta & BIT(pstat->remapped_aid)))// 2r STA
#else
	if (priv->pshare->has_2r_sta & BIT(pstat->aid))// 2r STA
#endif
		return; // do nothing

	// for debug, by victoryman 20090623
	if (pstat->tx_ra_bitmap & 0xff00000) {
		// this should be a 2r station!!!
		return;
	}

	if (pstat->tx_ra_bitmap & 0xffff000){// 11n 1R client
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv->share_priv, 1);
			else
				Switch_1SS_Antenna(priv->share_priv, 2);
		}
		else
			Switch_1SS_Antenna(priv->share_priv, 3);
  }
	else if (pstat->tx_ra_bitmap & 0xff0){// 11bg client
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv->share_priv, 1);
			else
				Switch_OFDM_Antenna(priv->share_priv, 2);
		}
		else
			Switch_OFDM_Antenna(priv->share_priv, 3);
  }



}
#endif


/*
 *
 * Move from 8190n_cam.c to here for open source consideration
 *
 */

#define  WritePortUlong  RTL_W32
#define  WritePortUshort  RTL_W16
#define  WritePortUchar  RTL_W8

#define ReadPortUchar(offset,value)	do{*value=RTL_R8(offset);}while(0)
#define ReadPortUshort(offset,value)	do{*value=RTL_R16(offset);}while(0)
#define ReadPortUlong(offset,value)	do{*value=RTL_R32(offset);}while(0)

/*******************************************************/
/*CAM related utility                                  */
/*CamAddOneEntry                                       */
/*CamDeleteOneEntry                                    */
/*CamResetAllEntry                                     */
/*******************************************************/
#define TOTAL_CAM_ENTRY 32

#define CAM_CONTENT_COUNT 8
#define CAM_CONTENT_USABLE_COUNT 6

#define CFG_VALID        BIT(15)



//return first not invalid entry back.
static UCHAR CAM_find_usable(struct mac80211_shared_priv *priv)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	ULONG ulCommand=0;
	ULONG ulContent=0;
	UCHAR ucIndex;
	int for_begin = 4;

	for(ucIndex=for_begin; ucIndex<TOTAL_CAM_ENTRY; ucIndex++) {
		// polling bit, and No Write enable, and address
		ulCommand= CAM_CONTENT_COUNT*ucIndex;
		WritePortUlong(_CAMCMD_, (_CAM_POLL_| ulCommand));

	   	//Check polling bit is clear
		while(1) {
			ReadPortUlong(_CAMCMD_, &ulCommand);
			if(ulCommand & _CAM_POLL_)
				continue;
			else
				break;
		}
		ReadPortUlong(_CAM_R_, &ulContent);

		//check valid bit. if not valid,
		if((ulContent & CFG_VALID)==0) {
			return ucIndex;
		}
	}
	return TOTAL_CAM_ENTRY;
}


static void CAM_program_entry(struct mac80211_shared_priv *priv,UCHAR index, UCHAR* macad,UCHAR* key128, USHORT config)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	ULONG target_command=0;
	ULONG target_content=0;
	UCHAR entry_i=0;
//	struct stat_info	*pstat;

	for(entry_i=0; entry_i<CAM_CONTENT_USABLE_COUNT; entry_i++)
	{
		// polling bit, and write enable, and address
		target_command= entry_i+CAM_CONTENT_COUNT*index;
		target_command= target_command |_CAM_POLL_ | _CAM_WE_;
		if(entry_i == 0) {
		    //first 32-bit is MAC address and CFG field
		    target_content= (ULONG)(*(macad+0))<<16
							|(ULONG)(*(macad+1))<<24
							|(ULONG)config;
		    target_content=target_content|config;
	    }
		else if(entry_i == 1) {
			//second 32-bit is MAC address
			target_content= (ULONG)(*(macad+5))<<24
							|(ULONG)(*(macad+4))<<16
							|(ULONG)(*(macad+3))<<8
							|(ULONG)(*(macad+2));
		}
		else {
			target_content= (ULONG)(*(key128+(entry_i*4-8)+3))<<24
							|(ULONG)(*(key128+(entry_i*4-8)+2))<<16
							|(ULONG)(*(key128+(entry_i*4-8)+1))<<8
							|(ULONG)(*(key128+(entry_i*4-8)+0));
		}

		WritePortUlong(_CAM_W_, target_content);
		WritePortUlong(_CAMCMD_, target_command);
	}
/*
	pstat = get_stainfo(priv, macad);
	if (pstat) {
		pstat->cam_id = index;
	}
*/	
}

int CamAddOneEntry(struct mac80211_shared_priv *priv,UCHAR *pucMacAddr, ULONG ulKeyId, ULONG ulEncAlg, ULONG ulUseDK, UCHAR *pucKey)
{
	UCHAR retVal = 0;
    UCHAR ucCamIndex = 0;
    USHORT usConfig = 0;
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	UCHAR wpaContent = 0;

    //use Hardware Polling to check the valid bit.
    //in reality it should be done by software link-list
	if ((!memcmp(pucMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (ulUseDK))
		ucCamIndex = ulKeyId;
	else
		ucCamIndex = CAM_find_usable(priv);

    if(ucCamIndex==TOTAL_CAM_ENTRY)
    	return retVal;

	usConfig=usConfig|CFG_VALID|((USHORT)(ulEncAlg))|(UCHAR)ulKeyId;

    CAM_program_entry(priv,ucCamIndex,pucMacAddr,pucKey,usConfig);

	if (priv->pshare_hw->CamEntryOccupied == 0) {
		if (ulUseDK == 1)
			wpaContent =  _RX_USE_DK_ | _TX_USE_DK_;
		RTL_W8(_WPACFG_, RTL_R8(_WPACFG_) | _RX_DEC_ | _TX_ENC_ | wpaContent);
	}

    return 1;
}



void CAM_read_mac_config(struct mac80211_shared_priv *priv,UCHAR ucIndex, UCHAR* pucMacad, USHORT* pusTempConfig)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	ULONG ulCommand=0;
	ULONG ulContent=0;

	// polling bit, and No Write enable, and address
	// cam address...
	// first 32-bit
	ulCommand= CAM_CONTENT_COUNT*ucIndex+0;
	ulCommand= ulCommand | _CAM_POLL_;
	WritePortUlong(_CAMCMD_, ulCommand);

   	//Check polling bit is clear
	while(1) {
		ReadPortUlong(_CAMCMD_, &ulCommand);
		if(ulCommand & _CAM_POLL_)
			continue;
		else
			break;
	}
	ReadPortUlong(_CAM_R_, &ulContent);

	//first 32-bit is MAC address and CFG field
	*(pucMacad+0)= (UCHAR)((ulContent>>16)&0x000000FF);
	*(pucMacad+1)= (UCHAR)((ulContent>>24)&0x000000FF);
	*pusTempConfig  = (USHORT)(ulContent&0x0000FFFF);

	ulCommand= CAM_CONTENT_COUNT*ucIndex+1;
	ulCommand= ulCommand | _CAM_POLL_;
	WritePortUlong(_CAMCMD_, ulCommand);

   	//Check polling bit is clear
	while(1) {
		ReadPortUlong(_CAMCMD_, &ulCommand);
		if(ulCommand & _CAM_POLL_)
			continue;
		else
			break;
	}
	ReadPortUlong(_CAM_R_, &ulContent);

	*(pucMacad+5)= (UCHAR)((ulContent>>24)&0x000000FF);
	*(pucMacad+4)= (UCHAR)((ulContent>>16)&0x000000FF);
	*(pucMacad+3)= (UCHAR)((ulContent>>8)&0x000000FF);
	*(pucMacad+2)= (UCHAR)((ulContent)&0x000000FF);
}





void CAM_empty_entry(struct mac80211_shared_priv *priv,UCHAR ucIndex)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	ULONG ulCommand=0;
	ULONG ulContent=0;
	int i;

	for(i=0;i<CAM_CONTENT_COUNT;i++) {
		// polling bit, and No Write enable, and address
		ulCommand= CAM_CONTENT_COUNT*ucIndex+i;
		ulCommand= ulCommand | _CAM_POLL_ |_CAM_WE_;
		// write content 0 is equal to mark invalid
		WritePortUlong(_CAM_W_, ulContent);
		WritePortUlong(_CAMCMD_, ulCommand);
	}
}


int CamDeleteOneEntry(struct mac80211_shared_priv *priv,UCHAR *pucMacAddr, ULONG ulKeyId, unsigned int useDK)
{
	UCHAR ucIndex;
	UCHAR ucTempMAC[6];
	USHORT usTempConfig=0;
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	int for_begin=0;

	// group key processing for RTL8190
	if ((!memcmp(pucMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK)) {
		CAM_read_mac_config(priv,ulKeyId,ucTempMAC,&usTempConfig);
		if (usTempConfig&CFG_VALID) {
			CAM_empty_entry(priv, ulKeyId);
			if (priv->pshare_hw->CamEntryOccupied == 1)
				RTL_W8(_WPACFG_, 0);
			return 1;
		}
		else
			return 0;
	}

	for_begin = 4;

	// unicast key processing for RTL8190
	// key processing for RTL818X(B) series
	for(ucIndex = for_begin; ucIndex < TOTAL_CAM_ENTRY; ucIndex++) {
		CAM_read_mac_config(priv,ucIndex,ucTempMAC,&usTempConfig);
		if(!memcmp(pucMacAddr,ucTempMAC,6)) {

			CAM_empty_entry(priv, ucIndex);	// reset MAC address, david+2007-1-15

			if (priv->pshare_hw->CamEntryOccupied == 1)
				RTL_W8(_WPACFG_, 0);

			return 1;
		}
	}
	return 0;
}


/*now use empty to fill in the first 4 entries*/
void CamResetAllEntry(struct mac80211_shared_priv *priv)
{
	UCHAR ucIndex;

	ULONG ioaddr = priv->pshare_hw->ioaddr;
	WritePortUlong(_CAMCMD_, _CAM_CLR_);

	for(ucIndex=0;ucIndex<TOTAL_CAM_ENTRY;ucIndex++) {
		CAM_empty_entry(priv,ucIndex);
	}

	priv->pshare_hw->CamEntryOccupied = 0;
	//priv->pmib->dot11GroupKeysTable.keyInCam = 0;
}


void CAM_read_entry(struct mac80211_shared_priv *priv,UCHAR index, UCHAR* macad,UCHAR* key128, USHORT* config)
{
	ULONG ioaddr = priv->pshare_hw->ioaddr;
	ULONG target_command=0;
	ULONG target_content=0;
	unsigned char entry_i=0;
	ULONG ulStatus;

	for(entry_i=0; entry_i<CAM_CONTENT_USABLE_COUNT; entry_i++)
	{
		// polling bit, and No Write enable, and address
		target_command= (ULONG)(entry_i+CAM_CONTENT_COUNT*index);
		target_command= target_command | _CAM_POLL_;

		WritePortUlong(_CAMCMD_, target_command);
	   	//Check polling bit is clear
		while(1) {
			ReadPortUlong(_CAMCMD_, &ulStatus);
			if(ulStatus & _CAM_POLL_)
				continue;
			else
				break;
		}
		ReadPortUlong(_CAM_R_, &target_content);

		if(entry_i==0) {
			//first 32-bit is MAC address and CFG field
		    *(config)= (USHORT)((target_content)&0x0000FFFF);
		    *(macad+0)= (UCHAR)((target_content>>16)&0x000000FF);
		    *(macad+1)= (UCHAR)((target_content>>24)&0x000000FF);
		}
		else if(entry_i==1) {
			*(macad+5)= (unsigned char)((target_content>>24)&0x000000FF);
		    *(macad+4)= (unsigned char)((target_content>>16)&0x000000FF);
	    	*(macad+3)= (unsigned char)((target_content>>8)&0x000000FF);
	    	*(macad+2)= (unsigned char)((target_content)&0x000000FF);
    		}
		else {
	    	*(key128+(entry_i*4-8)+3)= (unsigned char)((target_content>>24)&0x000000FF);
	    	*(key128+(entry_i*4-8)+2)= (unsigned char)((target_content>>16)&0x000000FF);
	    	*(key128+(entry_i*4-8)+1)= (unsigned char)((target_content>>8)&0x000000FF);
	    	*(key128+(entry_i*4-8)+0)= (unsigned char)(target_content&0x000000FF);
		}

		target_content = 0;
	}
}


void debug_cam(UCHAR*TempOutputMac,UCHAR*TempOutputKey,USHORT TempOutputCfg)
{
	printk("MAC Address\n");
	printk(" %X %X %X %X %X %X\n",*TempOutputMac
					    ,*(TempOutputMac+1)
					    ,*(TempOutputMac+2)
					    ,*(TempOutputMac+3)
					    ,*(TempOutputMac+4)
					    ,*(TempOutputMac+5));
	printk("Config:\n");
	printk(" %X\n",TempOutputCfg);

	printk("Key:\n");
	printk("%X %X %X %X,%X %X %X %X,\n%X %X %X %X,%X %X %X %X\n"
	      ,*TempOutputKey,*(TempOutputKey+1),*(TempOutputKey+2)
	      ,*(TempOutputKey+3),*(TempOutputKey+4),*(TempOutputKey+5)
	      ,*(TempOutputKey+6),*(TempOutputKey+7),*(TempOutputKey+8)
	      ,*(TempOutputKey+9),*(TempOutputKey+10),*(TempOutputKey+11)
	      ,*(TempOutputKey+12),*(TempOutputKey+13),*(TempOutputKey+14)
	      ,*(TempOutputKey+15));
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void CamDumpAll(struct mac80211_shared_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg=0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	
	for(i=0;i<TOTAL_CAM_ENTRY;i++)
	{
		printk("%X-",i);
		CAM_read_entry(priv,i,TempOutputMac,TempOutputKey,&TempOutputCfg);
		debug_cam(TempOutputMac,TempOutputKey,TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);

}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void CamDump4(struct mac80211_shared_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg=0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);

	for(i=0;i<4;i++)
	{
		printk("%X",i);
		CAM_read_entry(priv,i,TempOutputMac,TempOutputKey,&TempOutputCfg);
		debug_cam(TempOutputMac,TempOutputKey,TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);

}


