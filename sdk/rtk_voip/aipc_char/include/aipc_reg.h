#ifndef __AIPC_REG_H__
#define __AIPC_REG_H__

//#include "aipc_global.h"

/*
*	Macros
*/
#ifndef BIT
#define BIT(x)	(1 << x)
#endif

/*
*	SRAM DRAM control registers
*/
//	CPU0
//	Unmaped Memory Segment Address Register
#define R_C0UMSAR0_BASE		(0xB8001300)
#define R_C0UMSAR0 			(R_C0UMSAR0_BASE + 0x00)
#define R_C0UMSAR1 			(R_C0UMSAR0_BASE + 0x10)
#define R_C0UMSAR2 			(R_C0UMSAR0_BASE + 0x20)
#define R_C0UMSAR3 			(R_C0UMSAR0_BASE + 0x30)

//	Unmaped Memory Segment Size Register
#define R_C0UMSSR0_BASE		(0xB8001304)
#define R_C0UMSSR0 			(R_C0UMSSR0_BASE + 0x00)
#define R_C0UMSSR1 			(R_C0UMSSR0_BASE + 0x10)
#define R_C0UMSSR2 			(R_C0UMSSR0_BASE + 0x20)
#define R_C0UMSSR3 			(R_C0UMSSR0_BASE + 0x30)

//	SRAM Segment Address Register
#define R_C0SRAMSAR0_BASE	(0xB8004000)
#define R_C0SRAMSAR0		(R_C0SRAMSAR0_BASE + 0x00)
#define R_C0SRAMSAR1		(R_C0SRAMSAR0_BASE + 0x10)
#define R_C0SRAMSAR2		(R_C0SRAMSAR0_BASE + 0x20)
#define R_C0SRAMSAR3		(R_C0SRAMSAR0_BASE + 0x30)

//	SRAM Segment Size Register
#define R_C0SRAMSSR0_BASE	(0xB8004004)
#define R_C0SRAMSSR0		(R_C0SRAMSSR0_BASE + 0x00)
#define R_C0SRAMSSR1		(R_C0SRAMSSR0_BASE + 0x10)
#define R_C0SRAMSSR2		(R_C0SRAMSSR0_BASE + 0x20)
#define R_C0SRAMSSR3		(R_C0SRAMSSR0_BASE + 0x30)

//	SRAM Segment Base Register
#define R_C0SRAMSBR0_BASE	(0xB8004008)
#define R_C0SRAMSBR0		(R_C0SRAMSBR0_BASE + 0x00)
#define R_C0SRAMSBR1		(R_C0SRAMSBR0_BASE + 0x10)
#define R_C0SRAMSBR2		(R_C0SRAMSBR0_BASE + 0x20)
#define R_C0SRAMSBR3		(R_C0SRAMSBR0_BASE + 0x30)


//	CPU1
//	Unmaped Memory Segment Address Register
#define R_C1UMSAR0_BASE		(0xB8001340)
#define R_C1UMSAR0 			(R_C1UMSAR0_BASE + 0x00)
#define R_C1UMSAR1 			(R_C1UMSAR0_BASE + 0x10)
#define R_C1UMSAR2 			(R_C1UMSAR0_BASE + 0x20)
#define R_C1UMSAR3 			(R_C1UMSAR0_BASE + 0x30)

//	Unmaped Memory Segment Size Register
#define R_C1UMSSR0_BASE		(0xB8001344)
#define R_C1UMSSR0 			(R_C1UMSSR0_BASE + 0x00)
#define R_C1UMSSR1 			(R_C1UMSSR0_BASE + 0x10)
#define R_C1UMSSR2 			(R_C1UMSSR0_BASE + 0x20)
#define R_C1UMSSR3 			(R_C1UMSSR0_BASE + 0x30)

//	SRAM Segment Address Register
#define R_C1SRAMSAR0_BASE	(0xB8004040)
#define R_C1SRAMSAR0		(R_C1SRAMSAR0_BASE + 0x00)
#define R_C1SRAMSAR1		(R_C1SRAMSAR0_BASE + 0x10)
#define R_C1SRAMSAR2		(R_C1SRAMSAR0_BASE + 0x20)
#define R_C1SRAMSAR3		(R_C1SRAMSAR0_BASE + 0x30)

//	SRAM Segment Size Register
#define R_C1SRAMSSR0_BASE	(0xB8004044)
#define R_C1SRAMSSR0		(R_C1SRAMSSR0_BASE + 0x00)
#define R_C1SRAMSSR1		(R_C1SRAMSSR0_BASE + 0x10)
#define R_C1SRAMSSR2		(R_C1SRAMSSR0_BASE + 0x20)
#define R_C1SRAMSSR3		(R_C1SRAMSSR0_BASE + 0x30)

//	SRAM Segment Base Register
#define R_C1SRAMSBR0_BASE	(0xB8004048)
#define R_C1SRAMSBR0		(R_C1SRAMSBR0_BASE + 0x00)
#define R_C1SRAMSBR1		(R_C1SRAMSBR0_BASE + 0x10)
#define R_C1SRAMSBR2		(R_C1SRAMSBR0_BASE + 0x20)
#define R_C1SRAMSBR3		(R_C1SRAMSBR0_BASE + 0x30)


/*
*	ROM control registers
*/
#define R_ROMSAR_BASE		(0xB8004080)
//	ROM Segment Address Register
#define R_ROMSAR			(R_ROMSAR_BASE + 0x00)
//	ROM Segment Size Register
#define R_ROMSSR			(R_ROMSAR_BASE + 0x04)


/*
*	IPC related registers
*/
#define R_GIMR0_0_BASE		(0xb8003000)
#define R_GIMR0_0			(R_GIMR0_0_BASE + 0x00)
#define R_GIMR1_0			(R_GIMR0_0_BASE + 0x04)
#define R_GISR0_0			(R_GIMR0_0_BASE + 0x08)
#define R_GISR1_0			(R_GIMR0_0_BASE + 0x0c)

#define R_GIMR0_1_BASE		(0xb8003100)
#define R_GIMR0_1			(R_GIMR0_1_BASE + 0x00)
#define R_GIMR1_1			(R_GIMR0_1_BASE + 0x04)
#define R_GISR0_1			(R_GIMR0_1_BASE + 0x08)
#define R_GISR1_1			(R_GIMR0_1_BASE + 0x0c)

#define R_AIPC_BASE 		(0xb8141040)
#define R_AIPC_CPU_MUTEX	(R_AIPC_BASE + 0x00)
#define R_AIPC_DSP_MUTEX	(R_AIPC_BASE + 0x04)
#define R_AIPC_OWN_MUTEX	(R_AIPC_BASE + 0x08)
#define R_AIPC_EVT_T_DSP	(R_AIPC_BASE + 0x0c)
#define R_AIPC_EVT_T_CPU	(R_AIPC_BASE + 0x10)

#ifdef CONFIG_RTL8686_FPGA
//Boot DSP register. It could be changed in ASIC
#define R_AIPC_BOOT_DSP		0xB8000308
#define BOOT_DSP_BIT		BIT(5)
#endif

#ifdef CONFIG_RTL8686_ASIC
//Boot DSP register. ASIC Register
#define R_AIPC_ASIC_ENABLE_DSP_CLK 		0xB8000044
#define R_AIPC_ASIC_KICK_DSP 			0xB800408c
#define BIT_ENABLE_DSP_TAP				BIT(6)
#define BIT_ENABLE_DSP_CLOCK			BIT(5)
#define BIT_KICK_DSP					BIT(0)

//Reset CPU register. 
#define R_AIPC_ASIC_RESET_CPU			0xBB00018C
#define BIT_RESET_CPU					BIT(2)
#endif


#define CPU_T_DSP_IE	 	BIT(1)		//CPU Trigger DSP Interrupt Enable 
#define DSP_T_CPU_IE 		BIT(0)		//DSP Trigger CPU Interrupt Enable
#define CPU_T_DSP_IP	 	BIT(1)		//CPU Trigger DSP Interrupt Pending
#define DSP_T_CPU_IP 		BIT(0)		//DSP Trigger CPU Interrupt Pending

#define ASRT_DSP_IPC		BIT(0)
#define ASRT_CPU_IPC		BIT(0)
#define DASRT_DSP_IPC		BIT(0)
#define DASRT_CPU_IPC		BIT(0)

/*
*	HW IP enable control register
*/
#define R_AIPC_IP_ENABLE_CTRL          (0xb8000600)
#define BIT_IP_ENABLE_VOIPFFT          BIT(12)
#define BIT_IP_ENABLE_VOIPACC          BIT(11)
#define BIT_IP_ENABLE_GDMA1            BIT(9)
#define BIT_IP_ENABLE_GDMA0            BIT(8)
#define BIT_IP_ENABLE_PCM              BIT(2)
#define BIT_IP_ENABLE_GMAC             BIT(1)
#define BIT_IP_ENABLE_PREI_VOIP        BIT(0)

#endif

