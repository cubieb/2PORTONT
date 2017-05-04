/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for RTL8672 ASIC Registers
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: rtl8672_asicregs.h,v 1.4 2012/07/12 08:01:01 czyao Exp $
*/

#include "types.h"
#ifndef __RTL8672_ASICREGS_H__
#define __RTL8672_ASICREGS_H__


#define UNCACHED_ADDRESS(x) ((void *)(0x20000000 | (uint32)x ))
#define CACHED_ADDRESS(x) ((void*)(~0x20000000 & (uint32)x ))
#define PHYSICAL_ADDRESS(x) (((uint32)x) & 0x1fffffff)
#define KSEG0_ADDRESS(x) ((void*)(PHYSICAL_ADDRESS(x) | 0x80000000))
#define KSEG1_ADDRESS(x) ((void*)(PHYSICAL_ADDRESS(x) | 0xA0000000))


/* Register access macro (REG*()). */
#define REG32(reg) 			(*((volatile uint32 *)(reg)))
#define REG16(reg) 			(*((volatile uint16 *)(reg)))
#define REG8(reg) 				(*((volatile uint8 *)(reg)))

/* Register access macro (READ_MEM32() and WRITE_MEM32()). */

#if defined(RTL867X_MODEL_USER)
#define big_endian32(x) ((((x)&0xff000000)>>24)|(((x)&0x00ff0000)>>8)|(((x)&0x0000ff00)<<8)|(((x)&0x000000ff)<<24))
#define big_endian16(x) (((x)>>8)|(((x)&0x000000ff)<<8))
#define big_endian(x) big_endian32(x) /* backward-compatible */
#else
#define big_endian32(x) (x)
#define big_endian16(x) (x)
#define big_endian(x) big_endian32(x) /* backward-compatible */
#endif


#if 0

#define WRITE_VIR32(reg,val) REG32(reg)=big_endian(val)
//#define WRITE_VIR16(reg,val) REG16(reg)=big_endian(val)
//#define WRITE_VIR8(reg,val) REG8(reg)=big_endian(val)
#define READ_VIR32(reg) big_endian(REG32(reg))
//#define READ_VIR16(reg) big_endian(REG16(reg))
//#define READ_VIR8(reg) big_endian(REG8(reg))
#define WRITE_MEM32(reg,val) REG32(reg)=big_endian(val)
//#define WRITE_MEM16(reg,val) REG16(reg)=big_endian16(val)
//#define WRITE_MEM8(reg,val) REG8(reg)=(val)
#define READ_MEM32(reg) big_endian(REG32(reg))
//#define READ_MEM16(reg) big_endian16(REG16(reg))
//#define READ_MEM8(reg) (REG8(reg))
#else

void WRITE_MEM32(uint32 reg, uint32 val);
//void WRITE_MEM16(uint32 reg, uint32 val);
//void WRITE_MEM8(uint32 reg, uint32 val);
uint32 READ_MEM32(uint32 reg);
//uint32 READ_MEM16(uint32 reg);
//uint32 READ_MEM8(uint32 reg);
void WRITE_VIR32(uint32 reg, uint32 val);
//void WRITE_VIR16(uint32 reg, uint32 val);
//void WRITE_VIR8(uint32 reg, uint32 val);
uint32 READ_VIR32(uint32 reg);
//uint32 READ_VIR16(uint32 reg);
//uint32 READ_VIR8(uint32 reg);
#endif 

/* MACRO to update HW configuration */
#define UPDATE_MEM32(reg, val, mask, offset)	\
	do{ \
		WRITE_MEM32(	(reg),	\
						((READ_MEM32(reg) & ~((mask) << (offset))) | (((val) & (mask)) << (offset))));\
	} while (0)

/* MACRO to get value of HW configuration */
#define GET_MEM32_VAL(reg, mask, offset)	((READ_MEM32(reg) >> (offset)) & (mask))


//#define VIRTUAL_SWCORE_REG_SIZE	(1*64*1024) /* said, BB80_xxxx */
//#define VIRTUAL_SYSTEM_REG_SIZE	(2*64*1024) /* said, B800_xxxx */
//#define VIRTUAL_SWCORE_TBL_SIZE	(1*1024*1024+256*1024) /* said, BB00_xxxx */
//extern int8						*pVirtualSWReg;
//extern int8						*pVirtualSysReg;
extern int8						*pVirtualPPReg;
extern int8						*pVirtualAle;
extern int8						*pVirtualHsb;
extern int8						*pVirtualHsa;
extern int8						*pVirtualIfTable;
extern int8						*pVirtualL2;
extern int8						*pVirtualL4;
extern int8						*pVirtualDbg1;
extern int8						*pVirtualDbg2;
extern int8						*pVirtualHsbp;
extern int8						*pVirtualXlatorDbg;



#define REAL_SWCORE_BASE		0xBB800000
#define REAL_SYSTEM_BASE		0xB8000000
#define REAL_SWCORE_BASE		0xBB800000

//#define REAL_SWTBL_BASE			0xBB000000


#define REAL_PP_BASE			0xB8620000
#define REAL_ALE_BASE			0xB8625000

#define REAL_HSB_BASE			0xB8625100
#define REAL_HSA_BASE			0xB8625140
#define REAL_IFTABLE_BASE		0xB8625200

#define REAL_L2TABLE_BASE		0xB8626000
#define REAL_L4TABLE_BASE		0xB8627000

#define REAL_DBG1_BASE			0xb862513c
#define REAL_DBG2_BASE			0xb8625180
#define REAL_HSBP_BASE			0xB8625190
#define REAL_XLATOR_DBG_BASE	0xB86251A0



//#define SWCORE_BASE			((uint32)pVirtualSWReg)
//#define SWCORE_SIZE			VIRTUAL_SWCORE_REG_SIZE
///#define SYSTEM_BASE			((uint32)pVirtualSysReg)
//#define SYSTEM_SIZE			(VIRTUAL_SYSTEM_REG_SIZE)

#define PP_SIZE				0x5000
#define ALE_SIZE				0x100
#define HSB_SIZE				0x40
#define HSA_SIZE			0x40

#define	IFTABLE_WIDTH		(24)
#define IFTABLE_SIZE		0x200
#define L2TABLE_SIZE		0x800
#define L4TABLE_SIZE		0x2800
#define DBG1_SIZE			0x4
#define DBG2_SIZE			0x40
#define HSBP_SIZE			0xc
#define XLATOR_DBG_SIZE	0x4

#define PP_BASE				REAL_PP_BASE
#define ALE_BASE			REAL_ALE_BASE
#define HSB_BASE			REAL_HSB_BASE
#define HSA_BASE			REAL_HSA_BASE
#define IFTABLE_BASE		REAL_IFTABLE_BASE
#define L2TABLE_BASE		REAL_L2TABLE_BASE
#define L4TABLE_BASE		REAL_L4TABLE_BASE
#define DBG1_BASE			REAL_DBG1_BASE
#define DBG2_BASE			REAL_DBG2_BASE
#define HSBP_BASE			REAL_HSBP_BASE
#define XLATOR_DBG_BASE	REAL_XLATOR_DBG_BASE


#define SWCORE_BASE		REAL_SWCORE_BASE
#define SYSTEM_BASE			REAL_SYSTEM_BASE

/* Packet Processor Register Set - for Control */
#define PPCR						PP_BASE
#define SRAMMAPADDR			(0x004 + PP_BASE)
#define PENDINGRING				(0x008 + PP_BASE)
#define DESCWT0					(0x00c + PP_BASE)
#define DESCWT1					(0x010 + PP_BASE)
#define DESCWT2					(0x014 + PP_BASE)
#define DESCWT3					(0x018 + PP_BASE)
#define DESCWT4					(0x01c + PP_BASE)
#define DESCWT5					(0x020 + PP_BASE)
#define DESCWT6					(0x024 + PP_BASE)
#define DESCWT7					(0x028 + PP_BASE)
#define BUFFERSIZE				(0x02c + PP_BASE)
#define DEBUG_CTL				(0x030 + PP_BASE)

#define PENDINGRING_SAR_VTX0	(1)
#define PENDINGRING_SAR_VTX1	(1<<1)
#define PENDINGRING_SAR_VTX2	(1<<2)
#define PENDINGRING_SAR_VTX3	(1<<3)
#define PENDINGRING_SAR_VTX4	(1<<4)
#define PENDINGRING_SAR_VTX5	(1<<5)
#define PENDINGRING_SAR_VTX6	(1<<6)
#define PENDINGRING_SAR_VTX7	(1<<7)
#define PENDINGRING_MAC_VTX0	(1<<8)
#define PENDINGRING_MAC_VTX1	(1<<9)
#define PENDINGRING_MAC_VTX2	(1<<10)
#define PENDINGRING_MAC_VTX3	(1<<11)
#define PENDINGRING_MAC_VTX4	(1<<12)
#define PENDINGRING_EXT_VTX0	(1<<13)
#define PENDINGRING_EXT_VTX1	(1<<14)
#define PENDINGRING_EXT_VTX2	(1<<15)
#define PENDINGRING_EXT_PRX0	(1<<16)
#define PENDINGRING_EXT_PRX1	(1<<17)
#define PENDINGRING_EXT_PRX2	(1<<18)


/* Packet Processor Register Set - for SAR interface  */
#define SVRXDESC0				(0x1000 + PP_BASE)
#define SVRXDESC1				(0x1004 + PP_BASE)
#define SVRXDESC2				(0x1008 + PP_BASE)
#define SVRXDESC3				(0x100c + PP_BASE)
#define SVRXDESC4				(0x1010 + PP_BASE)
#define SVRXDESC5				(0x1014 + PP_BASE)
#define SVRXDESC6				(0x1018 + PP_BASE)
#define SVRXDESC7				(0x101c + PP_BASE)

#define SVTXDESC0				(0x1100 + PP_BASE)
#define SVTXDESC1				(0x1104 + PP_BASE)
#define SVTXDESC2				(0x1108 + PP_BASE)
#define SVTXDESC3				(0x110c + PP_BASE)
#define SVTXDESC4				(0x1110 + PP_BASE)
#define SVTXDESC5				(0x1114 + PP_BASE)
#define SVTXDESC6				(0x1118 + PP_BASE)
#define SVTXDESC7				(0x111c + PP_BASE)

#define SPRXDESC0				(0x1200 + PP_BASE)
#define SPRXDESC1				(0x1204 + PP_BASE)
#define SPRXDESC2				(0x1208 + PP_BASE)
#define SPRXDESC3				(0x120c + PP_BASE)
#define SPRXDESC4				(0x1210 + PP_BASE)
#define SPRXDESC5				(0x1214 + PP_BASE)
#define SPRXDESC6				(0x1218 + PP_BASE)
#define SPRXDESC7				(0x121c + PP_BASE)

#define SPTXDESC0				(0x1300 + PP_BASE)
#define SPTXDESC1				(0x1304 + PP_BASE)
#define SPTXDESC2				(0x1308 + PP_BASE)
#define SPTXDESC3				(0x130c + PP_BASE)
#define SPTXDESC4				(0x1310 + PP_BASE)
#define SPTXDESC5				(0x1314 + PP_BASE)
#define SPTXDESC6				(0x1318 + PP_BASE)
#define SPTXDESC7				(0x131c + PP_BASE)

#define SVRXRECVCNT0			(0x1400 + PP_BASE)
#define SVRXRECVCNT1			(0x1404 + PP_BASE)
#define SVRXRECVCNT2			(0x1408 + PP_BASE)
#define SVRXRECVCNT3			(0x140C + PP_BASE)
#define SVRXRECVCNT4			(0x1410 + PP_BASE)
#define SVRXRECVCNT5			(0x1414 + PP_BASE)
#define SVRXRECVCNT6			(0x1418 + PP_BASE)
#define SVRXRECVCNT7			(0x141C + PP_BASE)

#define SVTXFWDCNT0			(0x1500 + PP_BASE)
#define SVTXFWDCNT1			(0x1504 + PP_BASE)
#define SVTXFWDCNT2			(0x1508 + PP_BASE)
#define SVTXFWDCNT3			(0x150C + PP_BASE)
#define SVTXFWDCNT4			(0x1510 + PP_BASE)
#define SVTXFWDCNT5			(0x1514 + PP_BASE)
#define SVTXFWDCNT6			(0x1518 + PP_BASE)
#define SVTXFWDCNT7			(0x151C + PP_BASE)

#define SPRXDROPCNT0			(0x1600 + PP_BASE)
#define SPRXDROPCNT1			(0x1604 + PP_BASE)
#define SPRXDROPCNT2			(0x1608 + PP_BASE)
#define SPRXDROPCNT3			(0x1610 + PP_BASE)
#define SPRXDROPCNT4			(0x1614 + PP_BASE)
#define SPRXDROPCNT5			(0x1618 + PP_BASE)
#define SPRXDROPCNT6			(0x161C + PP_BASE)
#define SPRXDROPCNT7			(0x1620 + PP_BASE)

#define SARIMR					(0x1700 + PP_BASE)
#define SARISR					(0x1704 + PP_BASE)

#define SAR_VTDF				(1<<1)
#define SAR_RDA					(1<<2)
#define SAR_RBF					(1<<3)
#define SAR_PTDF				(1<<4)

#define SARVTDFI				(0x1708 + PP_BASE)
#define SARRDAI					(0x170C + PP_BASE)
#define SARRBFI					(0x1710 + PP_BASE)
#define SARPTDFI				(0x1714 + PP_BASE)

#define SAR_VC0					(1<<0)
#define SAR_VC1					(1<<1)
#define SAR_VC2					(1<<2)
#define SAR_VC3					(1<<3)
#define SAR_VC4					(1<<4)
#define SAR_VC5					(1<<5)
#define SAR_VC6					(1<<6)
#define SAR_VC7					(1<<7)


/* Packet Processor Register Set - for MAC interface  */
#define MVRXDESC0				(0x2000 + PP_BASE)	
#define MVRXDESC1				(0x2004 + PP_BASE)

#define MVTXDESC0				(0x2100 + PP_BASE)
#define MVTXDESC1				(0x2104 + PP_BASE)
#define MVTXDESC2				(0x2108 + PP_BASE)
#define MVTXDESC3				(0x210C + PP_BASE)
#define MVTXDESC4				(0x2110 + PP_BASE)

#define MPRXDESC0				(0x2200 + PP_BASE)
#define MPRXDESC1				(0x2204 + PP_BASE)

#define MPTXDESC0				(0x2300 + PP_BASE)
#define MPTXDESC1				(0x2304 + PP_BASE)
#define MPTXDESC2				(0x2308 + PP_BASE)
#define MPTXDESC3				(0x230C + PP_BASE)
#define MPTXDESC4				(0x2310 + PP_BASE)

#define MVRXRECVCNT0			(0x2400 + PP_BASE)
#define MVRXRECVCNT1			(0x2404 + PP_BASE)

#define MVTXFWDCNT0			(0x2500 + PP_BASE)
#define MVTXFWDCNT1			(0x2504 + PP_BASE)
#define MVTXFWDCNT2			(0x2508 + PP_BASE)
#define MVTXFWDCNT3			(0x250C + PP_BASE)
#define MVTXFWDCNT4			(0x2510 + PP_BASE)

#define MPRXDROPCNT0			(0x2600 + PP_BASE)
#define MPRXDROPCNT1			(0x2604 + PP_BASE)

#define MAC0IMR					(0x2700 + PP_BASE)
#define MAC0ISR					(0x2704 + PP_BASE)
#define MAC0SWINT				(0x2708 + PP_BASE)

#define MAC1IMR					(0x2800 + PP_BASE)
#define MAC1ISR					(0x2804 + PP_BASE)
#define MAC1SWINT				(0x2808 + PP_BASE)

#define MAC_ROK					(1<<0)
#define MAC_RER_OVF			(1<<4)
#define MAC_VTOK				(1<<6)
#define MAC_TDU					(1<<9)
#define MAC_SWINT				(1<<10)
#define MAC_PTOK0				(1<<12)
#define MAC_PTOK1				(1<<13)
#define MAC_PTOK2				(1<<14)
#define MAC_PTOK3				(1<<15)

/* Packet Processor Register Set - for EXT interface  */
#define EVRXDESC0				(0x3000 + PP_BASE)
#define EVRXDESC1				(0x3004 + PP_BASE)
#define EVRXDESC2				(0x3008 + PP_BASE)

#define EVTXDESC0				(0x3100 + PP_BASE)
#define EVTXDESC1				(0x3104 + PP_BASE)
#define EVTXDESC2				(0x3108 + PP_BASE)

#define EPRXDESC0				(0x3200 + PP_BASE)
#define EPRXDESC1				(0x3204 + PP_BASE)
#define EPRXDESC2				(0x3208 + PP_BASE)

#define EPTXDESC0				(0x3300 + PP_BASE)
#define EPTXDESC1				(0x3304 + PP_BASE)
#define EPTXDESC2				(0x3308 + PP_BASE)

#define EVRXRECVCNT0			(0x3400 + PP_BASE)
#define EVRXRECVCNT1			(0x3404 + PP_BASE)
#define EVRXRECVCNT2			(0x3408 + PP_BASE)

#define EVTXFWDCNT0			(0x3500 + PP_BASE)
#define EVTXFWDCNT1			(0x3504 + PP_BASE)
#define EVTXFWDCNT2			(0x3508 + PP_BASE)

#define EPRXDROPCNT0			(0x3600 + PP_BASE)
#define EPRXDROPCNT1			(0x3604 + PP_BASE)
#define EPRXDROPCNT2			(0x3608 + PP_BASE)

#define EPIMR					(0x3700 + PP_BASE)
#define EPISR					(0x3704 + PP_BASE)

#define EXT0_VRX_OK				(1<<0)
#define EXT1_VRX_OK				(1<<1)
#define EXT2_VRX_OK				(1<<2)
#define EXT0_PTX_OK				(1<<3)
#define EXT1_PTX_OK				(1<<4)
#define EXT2_PTX_OK				(1<<5)
#define EXT0_VRX_FULL			(1<<6)
#define EXT1_VRX_FULL			(1<<7)
#define EXT2_VRX_FULL			(1<<8)
#define EXT0_VTX_EMPTY			(1<<9)
#define EXT1_VTX_EMPTY			(1<<10)
#define EXT2_VTX_EMPTY			(1<<11)
#define EXT0_PTX_FULL			(1<<12)
#define EXT1_PTX_FULL			(1<<13)
#define EXT2_PTX_FULL			(1<<14)


/* Packet Processor Register Set - for Share Pool interface  */
#define SPSAR0DESC				(0x4000 + PP_BASE)
#define SPSAR1DESC				(0x4004 + PP_BASE)
#define SPSAR2DESC				(0x4008 + PP_BASE)
#define SPSAR3DESC				(0x400c + PP_BASE)
#define SPSAR4DESC				(0x4010 + PP_BASE)
#define SPSAR5DESC				(0x4014 + PP_BASE)
#define SPSAR6DESC				(0x4018 + PP_BASE)
#define SPSAR7DESC				(0x401c + PP_BASE)

#define SPMAC0DESC				(0x4100 + PP_BASE)
#define SPMAC1DESC				(0x4104 + PP_BASE)
#define SPMAC2DESC				(0x4108 + PP_BASE)
#define SPMAC3DESC				(0x410c + PP_BASE)
#define SPMAC4DESC				(0x4110 + PP_BASE)

#define SPEXT0DESC				(0x4200 + PP_BASE)
#define SPEXT1DESC				(0x4204 + PP_BASE)
#define SPEXT2DESC				(0x4208 + PP_BASE)

#define SPSAR0FWDCNT			(0x4300 + PP_BASE)
#define SPSAR1FWDCNT			(0x4304 + PP_BASE)
#define SPSAR2FWDCNT			(0x4308 + PP_BASE)
#define SPSAR3FWDCNT			(0x430c + PP_BASE)
#define SPSAR4FWDCNT			(0x4310 + PP_BASE)
#define SPSAR5FWDCNT			(0x4314 + PP_BASE)
#define SPSAR6FWDCNT			(0x4318 + PP_BASE)
#define SPSAR7FWDCNT			(0x431c + PP_BASE)

#define SPMAC0FWDCNT			(0x4400 + PP_BASE)
#define SPMAC1FWDCNT			(0x4404 + PP_BASE)
#define SPMAC2FWDCNT			(0x4408 + PP_BASE)
#define SPMAC3FWDCNT			(0x440c + PP_BASE)
#define SPMAC4FWDCNT			(0x4410 + PP_BASE)

#define SPEXT0FWDCNT			(0x4500 + PP_BASE)
#define SPEXT1FWDCNT			(0x4504 + PP_BASE)
#define SPEXT2FWDCNT			(0x4508 + PP_BASE)



/* PPCR  */
#define PPCR_ENABLE				(1<<31)
#define PPCR_RESET				(1<<30)
#define PPCR_START				(1<<29)
#define PPCR_MAXSRAMID_MASK	(0x1ff)

/* ALE Relation Definition */
#define	ALE_CONTROL_REGISTERS_SIZE	80	/* Offset: 0x5000 ~ 0x5080 */

#define	ALE_PNPTBL_WIDTH		8
#define	ALE_PNPTBL_SIZE			8

#define	ALE_L2TABLE_WIDTH		16
#define	ALE_L2TABLE_SRAM_IDX		32
#define	ALE_L2TABLE_SRAM_IDX_MASK	(0x1f)
#define	ALE_L2TABLE_SRAM_WAY		4
#define	ALE_L2TABLE_SRAM_SIZE 		(ALE_L2TABLE_SRAM_IDX * ALE_L2TABLE_SRAM_WAY)

#define	ALE_L4TABLE_WIDTH		40
#define	ALE_L4TABLE_SRAM_IDX		64
#define	ALE_L4TABLE_SRAM_IDX_MASK	(0x3f)
#define	ALE_L4TABLE_SRAM_WAY		4
#define	ALE_L4TABLE_SRAM_SIZE 		(ALE_L4TABLE_SRAM_IDX * ALE_L4TABLE_SRAM_WAY)

/* ALE Control Registers */
#define	ALECR_L2TEB				(0x0000 + ALE_BASE)
#define	ALECR_L2TSA				(0x0004 + ALE_BASE)
#define	ALECR_L2WAY				(0x0008 + ALE_BASE)
#define	ALECR_L2HIB				(0x000C + ALE_BASE)
#define	ALECR_L4TEB				(0x0010 + ALE_BASE)
#define	ALECR_L4TSA				(0x0014 + ALE_BASE)
#define	ALECR_L4WAY				(0x0018 + ALE_BASE)
#define	ALECR_L4HIB				(0x001C + ALE_BASE)
#define	ALECR_QOSECN				(0x0020 + ALE_BASE)
#define	ALECR_RSVD1				(0x0024 + ALE_BASE)
#define	ALECR_MAP1PREMR			(0x0028 + ALE_BASE)
#define	ALECR_RSVD2				(0x002C + ALE_BASE)
#define	ALECR_PNPCHK0				(0x0030 + ALE_BASE)
#define	ALECR_PNPACT0				(0x0034 + ALE_BASE)
#define	ALECR_PNPCHK1				(0x0038 + ALE_BASE)
#define	ALECR_PNPACT1				(0x003C + ALE_BASE)
#define	ALECR_PNPCHK2				(0x0040 + ALE_BASE)
#define	ALECR_PNPACT2				(0x0044 + ALE_BASE)
#define	ALECR_PNPCHK3				(0x0048 + ALE_BASE)
#define	ALECR_PNPACT3				(0x004C + ALE_BASE)
#define	ALECR_PNPCHK4				(0x0050 + ALE_BASE)
#define	ALECR_PNPACT4				(0x0054 + ALE_BASE)
#define	ALECR_PNPCHK5				(0x0058 + ALE_BASE)
#define	ALECR_PNPACT5				(0x005C + ALE_BASE)
#define	ALECR_PNPCHK6				(0x0060 + ALE_BASE)
#define	ALECR_PNPACT6				(0x0064 + ALE_BASE)
#define	ALECR_PNPCHK7				(0x0068 + ALE_BASE)
#define	ALECR_PNPACT7				(0x006C + ALE_BASE)
#define	ALECR_REASONACT			(0x0070 + ALE_BASE)
#define	ALECR_RSVD3				(0x0074 + ALE_BASE)
#define	ALECR_RSVD4				(0x0078 + ALE_BASE)
#define	ALECR_RSVD5				(0x007C + ALE_BASE)
#define	ALECR_DEBUG_W0			(0x0080 + ALE_BASE)
#define	ALECR_DEBUG_W1			(0x0084 + ALE_BASE)
#define	ALECR_DEBUG_W2			(0x0088 + ALE_BASE)
#define	ALECR_DEBUG_W3			(0x008C + ALE_BASE)
#define	ALECR_DEBUG_W4			(0x0090 + ALE_BASE)
#define	ALECR_DEBUG_W5			(0x0094 + ALE_BASE)
#define	ALECR_DEBUG_W6			(0x0098 + ALE_BASE)
#define	ALECR_DEBUG_W7			(0x009C + ALE_BASE)
#define	ALECR_DEBUG_W8			(0x00A0 + ALE_BASE)
#define	ALECR_DEBUG_W9			(0x00A4 + ALE_BASE)

#define	ALECR_ICR					(0x00B0 + ALE_BASE)
#define	ALECR_ISR					(0x00B4 + ALE_BASE)

#define 	ALECR_ICR_CMD_MASK		0x3
#define 	ALECR_ICR_CMD_OFFSET		24
#define	ALECR_ICR_CMD_READ		(0<<24)
#define	ALECR_ICR_CMD_WRITE		(1<<24)
#define	ALECR_ICR_CMD_SETAGE		(2<<24)
#define	ALECR_ICR_CMD_CLEARAGE	(3<<24)

#define	ALECR_ICR_TYPE_MASK		0x1
#define 	ALECR_ICR_TYPE_OFFSET		17
#define	ALECR_ICR_TYPE_SRAM		(0<<17)
#define	ALECR_ICR_TYPE_SDRAM		(1<<17)
#define	ALECR_ICR_TABLE_MASK		0x1
#define	ALECR_ICR_TABLE_OFFSET	16
#define	ALECR_ICR_TABLE_LAYER2	(0<<16)
#define	ALECR_ICR_TABLE_LAYER4	(1<<16)
#define 	ALECR_ICR_ENTRY_MASK		0x7fff
#define 	ALECR_ICR_ENTRY_OFFSET	0

#define  	ALEDROPCNT		(0x5024 + ALE_BASE)




/* ALE Control Register Flags */
#define	L2TEB_EN_MASK				(0x1 << 0)
#define	L2TEB_EN_OFFSET				(0)
#define	L2TSA_MASK				(0x3fffffff << 2)
#define	L2TSA_OFFSET				(2)
#define	L4TEB_EN_MASK				(0x1 << 0)
#define	L4TEB_EN_OFFSET				(0)
#define	L4TSA_MASK				(0x3fffffff << 2)
#define	L4TSA_OFFSET				(2)
#define	L2HIB_MASK				(0x7 << 0)
#define	L2HIB_OFFSET				(0)
#define	L2WAY_MASK				(0x3 << 0)
#define	L2WAY_OFFSET				(0)
#define	L4HIB_MASK				(0x7 << 0)
#define	L4HIB_OFFSET				(0)
#define	L4WAY_MASK				(0x3 << 0)
#define	L4WAY_OFFSET				(0)
#define	PNPCHK_ETH_MASK				(0xffff << 16)
#define	PNPCHK_ETH_OFFSET			(16)
#define	PNPCHK_SPM_MASK				(0xffff)
#define	PNPCHK_SPM_OFFSET			(0)
#define	PNPACT_PDS_MASK				(0x1 << 15)
#define	PNPACT_PDS_OFFSET			(15)
#define	PNPACT_PID_MASK				(0x7 << 12)
#define	PNPACT_PID_OFFSET			(12)
#define	PNPACT_VID_MASK				(0xfff << 0)
#define	PNPACT_VID_OFFSET			(0)
#define	QOSECN_ACPTCRCERRPKT_MASK		(0x1 << 24)
#define	QOSECN_ACPTCRCERRPKT_OFFSET		(24)
#define	QOSECN_TOSMODE_MASK			(0x1 << 16)
#define	QOSECN_TOSMODE_OFFSET			(16)
#define	QOSECN_TCPECN2CPU_MASK			(0x1 << 8)
#define	QOSECN_TCPECN2CPU_OFFSET		(8)
#define	QOSECN_IPECN2CPU_MASK			(0x1 << 0)
#define	QOSECN_IPECN2CPU_OFFSET			(0)


/* HSB */
#define	ALE_DEBUG					(0x003C + HSB_BASE)
#define   ALE_DEBUG_ACTION			(0x1 << 0)	/* 0:HSA is ready, 1: Trigger ALE */
#define   ALE_DEBUG_MODE			(0x1 << 8)	/* 0:Normal,	1:Test */
#define   ALE_DEBUG_STATUS			(0x1 << 16)	/* 0:IDLE, 1:process a normal packet */



/* FIXME : the following regs is copy from 865XC */
/* Global interrupt control registers */
#define GICR_BASE                           (SYSTEM_BASE+0x3000) /* 0xB8003000 */
#define GIMR                                (0x000 + GICR_BASE)       /* Global interrupt mask */
#define GISR                                (0x004 + GICR_BASE)       /* Global interrupt status */
#define IRR                                 (0x008 + GICR_BASE)       /* Interrupt routing */
#define IRR0                                (0x008 + GICR_BASE)       /* Interrupt routing */
#define IRR1                                (0x00C + GICR_BASE)       /* Interrupt routing */
#define IRR2                                (0x010 + GICR_BASE)       /* Interrupt routing */
#define IRR3                                (0x014 + GICR_BASE)       /* Interrupt routing */
/* Timer control registers 
*/
#define TIMER_BASE                          (SYSTEM_BASE+0x3100)		/* 0xB8003100 */
#define TC0DATA                             (0x000 + TIMER_BASE)       /* Timer/Counter 0 data */
#define TC1DATA                             (0x004 + TIMER_BASE)       /* Timer/Counter 1 data */
#define TC0CNT                              (0x008 + TIMER_BASE)       /* Timer/Counter 0 count */
#define TC1CNT                              (0x00C + TIMER_BASE)       /* Timer/Counter 1 count */
#define TCCNR                               (0x010 + TIMER_BASE)       /* Timer/Counter control */
#define TCIR                                (0x014 + TIMER_BASE)       /* Timer/Counter intertupt */
#define CDBR                                (0x018 + TIMER_BASE)       /* Clock division base */
#define WDTCNR                              (0x01C + TIMER_BASE)       /* Watchdog timer control */
/* Timer/Counter data register field definitions 
*/
#define TCD_OFFSET                          8
/* Timer/Counter control register field defintions 
*/
#define TC0EN                               (1 << 31)       /* Timer/Counter 0 enable */
#define TC0MODE_COUNTER                     0               /* Timer/Counter 0 counter mode */
#define TC0MODE_TIMER                       (1 << 30)       /* Timer/Counter 0 timer mode */
#define TC1EN                               (1 << 29)       /* Timer/Counter 1 enable */
#define TC1MODE_COUNTER                     0               /* Timer/Counter 1 counter mode */
#define TC1MODE_TIMER                       (1 << 28)       /* Timer/Counter 1 timer mode */
/* Timer/Counter interrupt register field definitions 
*/
#define TC0IE                               ((uint32)1 << 31)       /* Timer/Counter 0 interrupt enable */
#define TC1IE                               (1 << 30)       /* Timer/Counter 1 interrupt enable */
#define TC0IP                               (1 << 29)       /* Timer/Counter 0 interrupt pending */
#define TC1IP                               (1 << 28)       /* Timer/Counter 1 interrupt pending */
/* Clock division base register field definitions 
*/
#define DIVF_OFFSET                         16
/* Watchdog control register field definitions 
*/
#define WDTE_OFFSET                         24              /* Watchdog enable */
#define WDSTOP_PATTERN                      0xA5            /* Watchdog stop pattern */
#define WDTCLR                              (1 << 23)       /* Watchdog timer clear */
#define OVSEL_15                            0               /* Overflow select count 2^15 */
#define OVSEL_16                            (1 << 21)       /* Overflow select count 2^16 */
#define OVSEL_17                            (2 << 21)       /* Overflow select count 2^17 */
#define OVSEL_18                            (3 << 21)       /* Overflow select count 2^18 */
#define WDTIND                              (1 << 20)       /* Indicate whether watchdog ever occurs */
/* RTC control register field definitions */
#define WRITE_CDLW_OK                       (1 << 31)       /* Write CDLW OK */
#define RTCSEL                              (1 << 30)       /* Fase mode */
#define RTCEN                               (1 << 29)       /* Enable */

/* Memory Control Register registers 
*/
#define MCR_BASE				(SYSTEM_BASE+0x1000)     /* 0xB8001000 */
#define MCR						(0x000 + MCR_BASE)       /* Memory configuration register */
#define MTCR0					(0x004 + MCR_BASE)       /* Memory timing configuration register */
#define MTCR1					(0x008 + MCR_BASE)       /* Memory timing configuration register */
#define PFCR					(0x10 + MCR_BASE)	/* Pre-Fetch Control Register */

/* Debug Mode Control Registers */
#define DBG_CTRL			(DBG1_BASE)		/* debug model control registers */
#define DBG_SRCPORTIDX		(DBG2_BASE)		/* Source port index */
#define DBG_PRX_ADDR		(0x04 + DBG2_BASE)		/* Physical Rx Descriptor Address */
#define DBG_TRIGGER				0x1		/* Action for Test mode, Write 1 to start. HW clear the bit when job is done. */
#define DBG_TRIGGER_OFFSET		0
#define DBG_ITEM				0x3		/* 0: Parser, 1: ALE, 2: Translator */
#define DBG_PARSER				0x0
#define DBG_ALE					0x1
#define DBG_TRANSLATOR			0x2
#define DBG_ITEM_OFFSET		1
#define DBG_MODE				0x1		/* 0=Normal mode, 1=Test mode */
#define DBG_NORMAL_MODE		0x0
#define DBG_TEST_MODE			0x1
#define DBG_MODE_OFFSET		8

#define DBG_SIGNAL		(DBG2_BASE+0x10)	/* Header Stamp by PASS (HSBP) Format(Register Base = B862-5190~B862-519B). */
#define HSBP_SramAddr	(DBG2_BASE+0x14)
#define HSBP_DramAddr	(DBG2_BASE+0x18)
#define XLATOR_DEBUG	(DBG2_BASE+0x20)	/*  Translator Debug Signal (Register Base = B862-51A0~B862-51A3). */
#define LLIP_DEBUG		(DBG2_BASE+0x30)	/*  low-level IP simulation control register (Register Base = B862-51B0~B862-51BB). */
#define LLIP_SIM_EN		(DBG2_BASE+0x34)
#define LLIP_SIM_RX		(DBG2_BASE+0x38)
#define LLIP_SIM_TX		(DBG2_BASE+0x3C)
#define PP_SINGLE_CYCLE	 (DBG2_BASE+0x40)
#define PP_TOTAL_CYCLE	 (DBG2_BASE+0x44)
#define PP_TOTAL_CNT	 (DBG2_BASE+0x48)


/* Translator Debug Singal */
#define XLATOR_RXSHIFT_MASK		0xff0000
#define XLATOR_RXSHIFT_OFFSET		16
#define XLATOR_DATALEN_MASK		0xfff
#define XLATOR_DATALEN_OFFSET		0

#define	DIVISOR	1000         /* cfliu: change DIVISOR from 0xe to 4096 for watchdog */
                             /* yjlou: change DIVISOR from 4096 to 100 for better resolution. */
                             /* yjlou: change DIVISOR from 100 to 1000 for longer watchdog trigger. */
                             /* cfliu: change DIVISOR back to 4000 to let printk happy. */


/* for re865x directory */
//#define SWMISC_BASE                         (0x4200+SWCORE_BASE)
//#define CRMR                                (0x3C+SWMISC_BASE)     /* Chip Revision Management Register */  

//tylo, SAR related registers
#define 	SAR_ADDR		0xB8300000
#define	Uncache_Mask	0xA0000000
/* Channel */
#define	TV_Ctrl_Addr	(SAR_ADDR+0x00000000)
#define	TV_FDP_Addr	(SAR_ADDR+0x00000004)
#define	TV_SCR_Addr	(SAR_ADDR+0x00000008)
#define	TV_HDR_Addr	(SAR_ADDR+0x0000000C)

#define	TO_Ctrl_Addr	(SAR_ADDR+0x00000100)
#define	TO_FDP_Addr	(SAR_ADDR+0x00000104)

#define	RV_Ctrl_Addr	(SAR_ADDR+0x00001000)
#define	RV_FDP_Addr	(SAR_ADDR+0x00001004)
#define	RV_CKS_Addr	(SAR_ADDR+0x00001008)
#define	RV_HDR_Addr	(SAR_ADDR+0x0000100C)

#define	RO_Ctrl_Addr	(SAR_ADDR+0x00001100)
#define	RO_FDP_Addr	(SAR_ADDR+0x00001104)

/* Control */
#define	SAR_CNFG_Addr	 (SAR_ADDR + 0x00002000)
#define	SAR_STS_Addr	 (SAR_ADDR + 0x00002004)
#define	SAR_TBEI_Addr	 (SAR_ADDR + 0x00002010)
#define	SAR_TDFI_Addr	 (SAR_ADDR + 0x00002014)
#define	SAR_RDAI_Addr	 (SAR_ADDR + 0x00002018)
#define	SAR_RBFI_Addr	 (SAR_ADDR + 0x0000201C)
#define	SAR_RTOI_Addr	 (SAR_ADDR + 0x0000200C)

/*PP*/
#define 	SAR_SBA		(SAR_ADDR + 0x0000202c)

/* Debug */
#define	Test_Reg_0		0xB8303000
#define	Test_Reg_1		0xB8303010

/*--------------------------------
	SAR Control Definition
---------------------------------*/
#define	SAREN				(1<<31)
#define	UFLB				(1<<30)
#define	RDATHR_Offset		24
#define	RDATIMER_Offset	16
#define	TBEOMSK			(1<<14)
#define	TDFOMSK			(1<<13)
#define	QCLKSEL				(1<<12)
#define	QCLKOFFSET_Offset	8
#define	IERBF				(1<<6)
#define	IERDA				(1<<5)
#define	IERTO				(1<<4)
#define	IETBE				(1<<3)
#define	IETDF				(1<<2)

#define	TBEC			(1<<25)
#define	AAL5			(1<<30)
#define	ATRLREN		(1<<29)
#define	CFD				(1<<25)
#define	RBFC			(1<<24)
#define	CHEN			(1<<31)

//SAR CKS register
#define	FFCN_OFFSET			16
#define	POFF_MD_OFFSET		14
#define	L2Encap_OFFSET			13
#define	SARhdr_OFFSET			10
#define	WIEN_OFFSET			8
#define	IEPEN_OFFSET			6
#define	POFF_OFFSET			0

#define	FFCN					31
#define	POFF_MD				0
#define	L2Encp					1
#define 	SARhdr_Routed			0
#define 	SARhdr_Bridged			1
#define 	SARhdr_MER				2
#define 	SARhdr_PPPOE			3
#define 	SARhdr_PPPOA			4
#define 	WIEN					0
#define	IEPEN					1

#define	RDATHR		1
#define	RDATimer	20		//delay = RDATimer*500us



/* NIC IP registers */
#define NIC100MII_BASE	0xb8010000
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#define NIC100_BASE		0xb8018000
#else //czyao 0437FPGA
#define NIC100_BASE		0xb8012000
#endif

#define IDR0TO3			0x0000
#define IDR4TO5			0x0004
#define MAR0TO3			0x0008
#define MAR4TO7			0x000c
#define CR				0x003b
#define TCR				0x0040
#define RCR				0x0044
#define TXFDP1			0x1300
#define TXCDO1			0x1304
#define TXFDP2			0x1310
#define TXCDO2			0x1314
#define TXFDP3			0x1320
#define TXCDO3			0x1324
#define TXFDP4			0x1330
#define TXCDO4			0x1334
#define TXFDP5			0x1340
#define TXCDO5			0x1344
#define RXFDP			0x13f0
#define RXCDO			0x13f4
#define RX_RING_SIZE	0x13f6
#define SMSA				0x13fc
#define Rx_Pse_Des_Thres_h	0x142c
#define PROBE_SELEXCT	0x1403
#define ETHRNT_RX_CPU_DES_NUM	0x1430
#define RX_ETHRNT_DES_NUM			0x1431
#define RX_PSE_DES_THRES			0x1432
#define ETHER_IO_CMD				0x1434
#define ETHER_IO_CMD1				0x1438

// PTM registers
#define PTM_BASE			0xb8400000
#define PTM_CR			0xb7
#define PTM_ISR0			0xbc
#define PTM_ISR1			0xc4
#define PTM_RCR			0xc6

#define PTM_QoS_MIS			0x101c

#define PTM_BOND_FRAG_LF 	 0x1060
#define PTM_BOND_FRAG_HF 	 0x1064
#define PTM_BOND_FRAG_LS 	 0x1068
#define PTM_BOND_FRAG_HS      0x106c
#define PTM_BD_SLV_NUM           0x10d0
#define PTM_BD_TIMEOUT 		 0x10d4

//PTM Tx ring
#define PTM_TxFDP0_F  		0x1300
#define PTM_TxFDP1_F 	        0x1310
#define PTM_TxFDP2_F 		0x1320
#define PTM_TxFDP3_F 		0x1330
#define PTM_TxFDP0_S 		0x1340
#define PTM_TxFDP1_S 		0x1350
#define PTM_TxFDP2_S 		0x1360
#define PTM_TxFDP3_S 		0x1370
//PTM Rx ring
#define PTM_RxFDP_HMQ_L_F	0x13c0
#define PTM_RxFDP_HMQ_H_F	0x13c8
#define PTM_RxFDP_HMQ_L_S	0x13d0
#define PTM_RxFDP_HMQ_H_S	0x13d8
#define PTM_RxFDP_L_F		0x13e0
#define PTM_RxFDP_H_F 		 0x13e8
#define PTM_RxFDP_L_S 		 0x13f0
#define PTM_RxFDP_H_S 		 0x13f8

#define PTM_SMSA			0x1400
#define PTM_IOCMD			0x1434


//GPIO registers.
#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#define GPIO_BASE	0xB8003500
#else
#define GPIO_BASE	0xB8003200
#endif
#define PABCD_PTYPE	GPIO_BASE+0x4
#define PABCD_DIR	GPIO_BASE+0x8
#define PABCD_DAT	GPIO_BASE+0xc
#define PEFGH_DIR	GPIO_BASE+0x24
#define PEFGH_DAT	GPIO_BASE+0x28

#if !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
#define PKT_IRQ	28
#define PKT_NIC100_IRQ         23
#define PKT_NIC100MII_IRQ         22
#define PKT_SAR_IRQ		18
#else
#define PKT_NIC100_IRQ         14
#define PKT_SAR_IRQ		13
#define PKT_IRQ         12
#endif

#endif	/*__RTL8672_ASICREGS_H__ */
