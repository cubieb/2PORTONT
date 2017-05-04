/** @file
 *  @brief This file contains 'User-defined Function Prototypes'.
 *  @n Users must rewrite these functions according to their application and hardware environment.
 */

#include "AC49xDrv_Drv.h"

#if	((AC49X_DEVICE_TYPE == AC490_DEVICE) || (AC49X_DEVICE_TYPE == AC491_DEVICE))
	#include "stdafx.h"
#ifdef HOST_45X
	#include "45xxxEBDoc.h"
#else
	#include "49xxxaEB.h"
#endif
	#include "Pcif.h"
#ifndef HOST_45X
	#include "JtagUtilities.h"
#endif

#elif	(  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC495_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC496_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC497_DEVICE)\
		)
	#if (AC49X_OS_CONFIG == AC49X_OS__WINDOWS)
	#include "stdafx.h"
	#include "49xxxaEB.h"
    #include "windows.h"
	#include "Pcif.h"
	#include "JtagUtilities.h"

	#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
	#include "vxWorks.h"
	#include "taskLib.h"

	#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
	#include <asm/addrspace.h>
	#include <linux/delay.h>
	#include <linux/kernel.h>
	#include <asm/system.h>
	#include <linux/module.h>
	#include <linux/types.h>
	#include <linux/byteorder/little_endian.h>
	#include <asm/avalanche/titan/titan.h>

	#endif
#elif		(AC49X_DEVICE_TYPE == ACLX5280_DEVICE)
	#if (AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
	#include "AC49xDrv_Drv.h"
#ifndef ACL_SIMPLE_KERNEL
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/delay.h>
#include <linux/init.h>
//#include <linux/devfs_fs_kernel.h>
#include <linux/spinlock.h>
#include <linux/version.h>
	#endif
	#endif

#endif

// copy from rtk_voip.h 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#define save_flags(x) local_irq_save(x)
#define cli(format, ...)
#define restore_flags(x) local_irq_restore(x)
#endif

#if __cplusplus
extern "C"	{
#endif
#if		(AC49X_DEVICE_TYPE != ACLX5280_DEVICE)
Tac49xHpiPort *ac49xUserDef_HpiPhysicalAddress[AC49X_NUMBER_OF_DEVICES];
#endif //		(AC49X_DEVICE_TYPE == ACLX5280_DEVICE)

#if	((AC49X_DEVICE_TYPE == AC490_DEVICE) || (AC49X_DEVICE_TYPE == AC491_DEVICE))

void Ac49xUserDef_DelayForHostResponse(void){Sleep(10);}
/*************************************************************************************/

/*************************************************************************************/
void Ac49xUserDef_DelayForDeviceResponse(void)
{
#ifndef HOST_45X
    theApp.MyWait(DELAY_FOR_DEVICE_RESPONSE*10);
#else
    Sleep(2);
#endif
}
/*************************************************************************************/
void Ac49xUserDef_VoppReset(int Device, int ResetOnOff)
{
#if	(AC49X_DEVICE_TYPE == AC490_DEVICE)
    AC490DeviceReset(Device, ResetOnOff);
#elif (AC49X_DEVICE_TYPE == AC491_DEVICE)
#ifndef HOST_45X
    AC491DeviceReset(Device, ResetOnOff);
#else
    AC491DeviceResetOnMeb(Device, ResetOnOff);
//    AC450DeviceResetOnMeb(Device, ResetOnOff);
#endif
#endif
}
/*************************************************************************************/
int Ac49xUserDef_GetGpio0(int Device)
{
#ifdef HOST_45X
    return (GetMeb450StatusRegister(Device/NUMBER_OF_DEVICES_PER_DSP)&0x40) ? 1 : 0;
#else
    return (AEBGetStatusRegister(Device/NUMBER_OF_DEVICES_PER_DSP)&0x80) ? 1 : 0;
#endif
}
/*************************************************************************************/
void Ac49xUserDef_RaffleAndTrashPacketBits(int Device, int Channel, char *_pPacket, int _PacketSize, Tac49xTransferMedium TransferMedium)
{
#ifndef HOST_45X
	theApp.m_CrashTestApp.RaffleAndTrashPacketBits(Device, Channel, _pPacket, _PacketSize, TransferMedium);
#endif
}

#elif	(  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC495_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC496_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC497_DEVICE)\
		)

#if(AC49X_NUMBER_OF_DEVICES>1)
/*********************************************************************************/
int	Ac49xUserDef_498ConfigDevicePll(int Device)/* Brings the Clk to 150 MHz  write the value at the beginning to 0x1DC*/ 
			    	                    /* Wait bit 0(status) is 0*/
				                        /* Write the value at to 0x57FE*/
			    	                    /* Wait bit 0(status) is 1*/
{
	volatile U32 *pPllReg;
    int MaxNumberOfIterations = 3000;

    if(Device==0)
        return 0;

	pPllReg   = (volatile U32*)((U32)ac49xUserDef_HpiPhysicalAddress[Device]
                                  + offsetof(Tac498MemoryMap, ClockControlRegisters)
                                  - offsetof(Tac498MemoryMap, McBSP0[0])
                                  + offsetof(Tac498ClockControlRegisters, ClockControlRegister)
                                 );

	*pPllReg  = (volatile U32)0x802;/*1DC*/;
	while((*pPllReg)&0x0001)
         {
          MaxNumberOfIterations--;
          if(MaxNumberOfIterations==0)
              return 1;
         }
	*pPllReg  = (volatile U32)0x57FE;

    MaxNumberOfIterations = 3000;
	while(!((*pPllReg)&0x0001))
       {
       MaxNumberOfIterations--;
       if(MaxNumberOfIterations==0)
           return 1;
       }
    return 0;
}

/*********************************************************************************/
void Ac49xUserDef_498SetPeripheralResetRegister(int Device, int Value)
{
	volatile U32 *pReg;
    volatile U32 RegVal;
    int MaxNumberOfIterations = 3000;

	pReg = (volatile U32*)((U32)ac49xUserDef_HpiPhysicalAddress[Device]
                           + offsetof(Tac498MemoryMap, ResetControlRegisters)
                           - offsetof(Tac498MemoryMap, McBSP0[0])
                          );
    RegVal = *pReg;
    RegVal &= ~(0x3E7);
    RegVal |=  Value;
    *pReg = RegVal;
}


/*********************************************************************************/
int	Ac49xUserDef_498ConfigAllDevicesPll()
{
    int Device;
    for(Device=AC49X_NUMBER_OF_DEVICES-1; Device>=1; Device--)
        {
        if(Ac49xUserDef_498ConfigDevicePll(Device))
            return 1;
        }
    return 0;
}

/*********************************************************************************
void Ac49xUserDef_498SetMif(int Device, int Value)/* Bit 8=1 brings MIF out of reset and gives ctrl to the MIPS
		      	                        /* Bit 8=0 gives MIF ctrl to the VoPP/
{
	volatile U32 *pReg;
    volatile U32 RegVal;
    int MaxNumberOfIterations = 3000;

	pReg = (volatile U32*)((U32)ac49xUserDef_HpiPhysicalAddress[Device]
                           + offsetof(Tac498MemoryMap, ResetControlRegisters)
                           - offsetof(Tac498MemoryMap, McBSP0[0])
                          );
    RegVal = *pReg;
    RegVal &= ~(1<<AC498_PRCR_DSP_MIF_BIT);
    RegVal |=  ((Value&0x1)<<AC498_PRCR_DSP_MIF_BIT);
    *pReg = RegVal;
}
*/

#endif /*(AC49X_NUMBER_OF_DEVICES>1)*/

	#if	  (AC49X_OS_CONFIG == AC49X_OS__WINDOWS)
		void Ac49xUserDef_DelayForHostResponse(void){Sleep(10);}
		/*************************************************************************************/
		void Ac49xUserDef_DelayForDeviceResponse(void){theApp.MyWait(DELAY_FOR_DEVICE_RESPONSE*10);}
		/*************************************************************************************/
		void Ac49xUserDef_VoppReset(int Device, int ResetOnOff){}
		/*************************************************************************************/
		int Ac49xUserDef_GetGpio0(int Device){return (AEBGetStatusRegister(Device/NUMBER_OF_DEVICES_PER_DSP)&0x80) ? 1 : 0;}
		/*************************************************************************************/
		void Ac49xUserDef_RaffleAndTrashPacketBits(int Device, int Channel, char *_pPacket, int _PacketSize, Tac49xTransferMedium TransferMedium)
		{
			theApp.m_CrashTestApp.RaffleAndTrashPacketBits(Device, Channel, _pPacket, _PacketSize, TransferMedium);
		}
		/*************************************************************************************/
		void Ac49xUserDef_EMIF_MEMCPY(unsigned long TargetAddress, char *pSourceBuffer, unsigned long BlockSize)
		{
			memcpy((void*)(TargetAddress+EMIF_PHYSICAL_ADDRESS_OFFSET+EMIF_VIRTUAL_ADDRESS_OFFSET), (void*)pSourceBuffer, BlockSize);
		}
		/**************************************************************************************/
		void Ac49xUserDef_HardResetAssertFor16Cycles(void)
		{
			/* At revision 1.0 there is no software reset */
			volatile U32 RegVal = (volatile U32 )0x00000001; /*Reset internal AIC20 Voice Codec*/
			volatile U32 *pReg  = (volatile U32*)0xa50803F0;
			*pReg = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_HardResetReleaseAndWait150Cycles(void)
		{
			volatile U32 RegVal = (volatile U32 )0xFFFFFFFF;/*Unreset all*/
			volatile U32 *pReg  = (volatile U32*)0xa8611600;
			*pReg = RegVal;
			/* TBD 150 Cycles at 25MHz delay*/
		}
		/**************************************************************************************/
		void Ac49xUserDef_PllProgramTo125_MHz(void)
		{
			volatile U32 PllRegVal;
			volatile U32 *pPllReg;

			pPllReg   = (volatile U32*)0xa5080104;
			PllRegVal = (volatile U32 )(*pPllReg);
			PllRegVal = PllRegVal&0xFFFD;
			*pPllReg  = PllRegVal;

			while((*pPllReg)&0x0001)
				Ac49xUserDef_DelayForDeviceResponse();

		#if PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL
			#if   (AC49X_DEVICE_TYPE == AC494_DEVICE)
				PllRegVal = (volatile U32 )0x00004327;
			#elif (AC49X_DEVICE_TYPE == AC495_DEVICE)
				PllRegVal = (volatile U32 )0x00003327;
			#elif (AC49X_DEVICE_TYPE == AC496_DEVICE)
				PllRegVal = (volatile U32 )0x00004327;
			#elif (AC49X_DEVICE_TYPE == AC497_DEVICE)
				PllRegVal = (volatile U32 )0x00003327;
			#endif
		#else
			#if   (AC49X_DEVICE_TYPE == AC494_DEVICE)
				PllRegVal = (volatile U32 )0x0000EB27;
			#elif (AC49X_DEVICE_TYPE == AC495_DEVICE)
				PllRegVal = (volatile U32 )0x00005327;
			#elif (AC49X_DEVICE_TYPE == AC496_DEVICE)
				PllRegVal = (volatile U32 )0x0000EB27;
			#elif (AC49X_DEVICE_TYPE == AC497_DEVICE)
				PllRegVal = (volatile U32 )0x00005327;
			#endif
		#endif

			*pPllReg  = PllRegVal; /*DSP_CLK~DSP_CLK_PLL*/

			while(!((*pPllReg)&0x0001))
				Ac49xUserDef_DelayForDeviceResponse();
		}
		/**************************************************************************************/
		void Ac49xUserDef_MifAndPeripheralsResetRelease(void)
		{
			volatile U32 RegVal;
			volatile U32 *pReg = (volatile U32*)0xa5080000;
			RegVal    = *pReg;
			RegVal   |= ((volatile U32)0x00000763);
			*pReg     = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_CodecResetRelease(void)
		{
			/* At revision 1.0 there is no software reset */
			volatile U32 RegVal = (volatile U32 )0x00000000; /*Reset internal AIC20 Voice Codec*/
			volatile U32 *pReg  = (volatile U32*)0xa50803F0;
			*pReg = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_DspResetRelease(void)
		{
			volatile U32 RegVal;
			volatile U32 *pReg = (volatile U32*)0xa5080000;
			RegVal    = *pReg;
			RegVal   |= 0x00000080;
			*pReg     = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_InitCache(void)
		{
			volatile U32 RegVal = 0x00000050;	/* Map DSP EMIF(sdram) to start at address 0x14000000*/
			volatile U32 *pReg  = (volatile U32*)0xa5080014;
			*pReg = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_InterruptMips(void)
		{
			volatile U32 RegVal = 0x00000001;
			volatile U32 *pReg  = (volatile U32*)0xa5080008;
			*pReg = RegVal;
		}
		/**************************************************************************************/
		Tac49xProgramDownloadStatus  Ac49xUserDef_Boot(int Device, char *pProgram, int ApplicationParam, char *pKernel)
		{
			Tac49xHcrfStatusRegisterIndex ResponseValue;
			int RetVal;
			volatile U32 RegVal;
			volatile U32 *pReg  = (volatile U32*)0xa8611600;
			RegVal = *pReg;

			if(RegVal&0x000100000)
				{
				RetVal = Ac49xHcrfDeviceReload(Device, pProgram, &ResponseValue);
				if((RetVal==0) && (ResponseValue==HCRF_STATUS_REGISTER_INDEX__RELOAD_COMPLETED))
					return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
				else
					return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_FAIL_CHECKSUM;
				}
			else
				{
				return Ac49xBoot(Device, pProgram, ApplicationParam, pKernel);
				}
		}

	#elif (AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
		/*************************************************************************************/
		void Ac49xUserDef_DelayForHostResponse(void){taskDelay(1);}
		/*************************************************************************************/
		void Ac49xUserDef_DelayForDeviceResponse(void){taskDelay(1);}
		/*************************************************************************************/
		void Ac49xUserDef_VoppReset(int Device, int ResetOnOff){}
		/*************************************************************************************/
		int Ac49xUserDef_GetGpio0(int Device){return 0;}
		/*************************************************************************************/
		void Ac49xUserDef_RaffleAndTrashPacketBits(int Device, int Channel, char *_pPacket, int _PacketSize, Tac49xTransferMedium TransferMedium){}
		/*************************************************************************************/
		void Ac49xUserDef_EMIF_MEMCPY(unsigned long TargetAddress, char *pSourceBuffer, unsigned long BlockSize)
		{
			memcpy((void*)(TargetAddress+EMIF_PHYSICAL_ADDRESS_OFFSET+EMIF_VIRTUAL_ADDRESS_OFFSET), (void*)pSourceBuffer, BlockSize);
		}
		/**************************************************************************************/
		void Ac49xUserDef_HardResetAssertFor16Cycles(void)
		{
			/* At revision 1.0 there is no software reset */
			volatile U32 RegVal = (volatile U32 )0x00000001; /*Reset internal AIC20 Voice Codec*/
			volatile U32 *pReg  = (volatile U32*)0xa50803F0;
			*pReg = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_HardResetReleaseAndWait150Cycles(void)
		{
			volatile U32 RegVal = (volatile U32 )0xFFFFFFFF;/*Unreset all*/
			volatile U32 *pReg  = (volatile U32*)0xa8611600;
			*pReg = RegVal;
			/* TBD 150 Cycles at 25MHz delay*/
		}
		/**************************************************************************************/
		void Ac49xUserDef_PllProgramTo125_MHz(void)
		{
			volatile U32 PllRegVal;
			volatile U32 *pPllReg;

			pPllReg   = (volatile U32*)0xa5080104;
			PllRegVal = (volatile U32 )(*pPllReg);
			PllRegVal = PllRegVal&0xFFFD;
			*pPllReg  = PllRegVal;

			while((*pPllReg)&0x0001)
				Ac49xUserDef_DelayForDeviceResponse();

		#if PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL
			#if   (AC49X_DEVICE_TYPE == AC494_DEVICE)
				PllRegVal = (volatile U32 )0x00004327;
			#elif (AC49X_DEVICE_TYPE == AC495_DEVICE)
				PllRegVal = (volatile U32 )0x00003327;
			#elif (AC49X_DEVICE_TYPE == AC496_DEVICE)
				PllRegVal = (volatile U32 )0x00004327;
			#elif (AC49X_DEVICE_TYPE == AC497_DEVICE)
				PllRegVal = (volatile U32 )0x00003327;
			#endif
		#else
			#if   (AC49X_DEVICE_TYPE == AC494_DEVICE)
				PllRegVal = (volatile U32 )0x0000EB27;
			#elif (AC49X_DEVICE_TYPE == AC495_DEVICE)
				PllRegVal = (volatile U32 )0x00005327;
			#elif (AC49X_DEVICE_TYPE == AC496_DEVICE)
				PllRegVal = (volatile U32 )0x0000EB27;
			#elif (AC49X_DEVICE_TYPE == AC497_DEVICE)
				PllRegVal = (volatile U32 )0x00005327;
			#endif
		#endif

			*pPllReg  = PllRegVal; /*DSP_CLK~DSP_CLK_PLL*/

			while(!((*pPllReg)&0x0001))
				Ac49xUserDef_DelayForDeviceResponse();
		}
		/**************************************************************************************/
		void Ac49xUserDef_MifAndPeripheralsResetRelease(void)
		{
			volatile U32 RegVal;
			volatile U32 *pReg = (volatile U32*)0xa5080000;
			RegVal    = *pReg;
			RegVal   |= ((volatile U32)0x00000763);
			*pReg     = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_CodecResetRelease(void)
		{
			/* At revision 1.0 there is no software reset */
			volatile U32 RegVal = (volatile U32 )0x00000000; /*Reset internal AIC20 Voice Codec*/
			volatile U32 *pReg  = (volatile U32*)0xa50803F0;
			*pReg = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_DspResetRelease(void)
		{
			volatile U32 RegVal;
			volatile U32 *pReg = (volatile U32*)0xa5080000;
			RegVal    = *pReg;
			RegVal   |= 0x00000080;
			*pReg     = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_InitCache(void)
		{
			volatile U32 RegVal = CASH_SEG_B;	/* Map DSP EMIF(sdram) to start at address of cash segment*/
			volatile U32 *pReg  = (volatile U32*)0xa5080014;
			*pReg = RegVal;
		}
		/**************************************************************************************/
		void Ac49xUserDef_InterruptMips(void)
		{
			volatile U32 RegVal = 0x00000001;
			volatile U32 *pReg  = (volatile U32*)0xa5080008;
			*pReg = RegVal;
		}
		/**************************************************************************************/
		Tac49xProgramDownloadStatus  Ac49xUserDef_Boot(int Device, char *pProgram, int ApplicationParam, char *pKernel)
		{
			Tac49xHcrfStatusRegisterIndex ResponseValue;
			volatile U32 RegVal;
			volatile U32 *pReg  = (volatile U32*)0xa8611600;
			RegVal = *pReg;

			if(RegVal&0x000100000)
				{
				int RetVal = Ac49xHcrfDeviceReload(Device, pProgram, &ResponseValue);
				if((RetVal==0) && (ResponseValue==HCRF_STATUS_REGISTER_INDEX__RELOAD_COMPLETED))
					return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
				else
					return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_FAIL_CHECKSUM;
				}
			else
				return Ac49xBoot(Device, pProgram, ApplicationParam, pKernel);
		}


	#elif (AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
		/*********************************pTempProgramPointer****************************************************/
		void Ac49xUserDef_DelayForHostResponse(void){mdelay(1);}
		/*************************************************************************************/
		void Ac49xUserDef_DelayForDeviceResponse(void){mdelay(1);}
		/*************************************************************************************/
		void Ac49xUserDef_VoppReset(int Device, int ResetOnOff){}
		/*************************************************************************************/
		int Ac49xUserDef_GetGpio0(int Device){return 0;}
		/*************************************************************************************/
		void Ac49xUserDef_RaffleAndTrashPacketBits(int Device, int Channel, char *_pPacket, int _PacketSize, Tac49xTransferMedium TransferMedium){}
		/*************************************************************************************/
		void Ac49xUserDef_EMIF_MEMCPY(unsigned long TargetAddress, char *pSourceBuffer, unsigned long BlockSize)
		{
			int i;
			for(i=0 ; i < BlockSize ; i+=4)
				*(unsigned int *)(TargetAddress + KSEG1ADDR(EMIF_PHYSICAL_ADDRESS_OFFSET) + i) = __cpu_to_be32((*(unsigned int *)(pSourceBuffer + i)));
		}
		/**************************************************************************************/
		void Ac49xUserDef_HardResetAssertFor16Cycles(void)
		{
			/* At revision 1.0 there is no software reset */
			volatile unsigned int RegVal;
			volatile unsigned int *pReg;

			RegVal = (volatile unsigned int)0x00000001; /*Reset internal AIC20 Voice Codec*/
			pReg  = (volatile unsigned int *)(KSEG1ADDR(0x050803F0));
			*pReg = RegVal;
			mb();
		}
		/**************************************************************************************/
		void Ac49xUserDef_HardResetReleaseAndWait150Cycles(void)
		{

			volatile unsigned int RegVal;
			volatile unsigned int *pReg;

			RegVal = (volatile unsigned int)0xFFFFFFFF;/*Unreset all*/
			pReg  = (volatile unsigned int *)(KSEG1ADDR(0x08611600));
			*pReg = RegVal;
			mb();
			/* TBD 150 Cycles at 25MHz delay*/
		}
		/**************************************************************************************/
		void Ac49xUserDef_PllProgramTo125_MHz(void)
		{
			volatile unsigned int 	PllRegVal;
			volatile unsigned int 	*pPllReg;
			volatile unsigned int	flag;

			pPllReg   = (volatile unsigned int*)(KSEG1ADDR(0x05080104));
			PllRegVal = (volatile unsigned int)(*pPllReg);
			mb();
			PllRegVal = PllRegVal&0xFFFD;
			*pPllReg  = PllRegVal;

			mb();
			flag = (*pPllReg) & (volatile unsigned int)1;
			mb();
			while(flag) {
				Ac49xUserDef_DelayForDeviceResponse();
				flag = (*pPllReg) & (volatile unsigned int)1;
				mb();
			}

    #if PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL
	    #if   (AC49X_DEVICE_TYPE == AC494_DEVICE)
		    PllRegVal = (volatile U32 )0x00004327;
	    #elif (AC49X_DEVICE_TYPE == AC495_DEVICE)
		    PllRegVal = (volatile U32 )0x00003327;
	    #elif (AC49X_DEVICE_TYPE == AC496_DEVICE)
		    PllRegVal = (volatile U32 )0x00004327;
	    #elif (AC49X_DEVICE_TYPE == AC497_DEVICE)
		    PllRegVal = (volatile U32 )0x00003327;
	    #endif
    #else
	    #if   (AC49X_DEVICE_TYPE == AC494_DEVICE)
		    PllRegVal = (volatile U32 )0x0000EB27;
	    #elif (AC49X_DEVICE_TYPE == AC495_DEVICE)
		    PllRegVal = (volatile U32 )0x00005327;
	    #elif (AC49X_DEVICE_TYPE == AC496_DEVICE)
		    PllRegVal = (volatile U32 )0x0000EB27;
	    #elif (AC49X_DEVICE_TYPE == AC497_DEVICE)
		    PllRegVal = (volatile U32 )0x00005327;
	    #endif
    #endif
			*pPllReg  = PllRegVal; /*DSP_CLK~DSP_CLK_PLL*/
			mb();
			flag = (*pPllReg) & (volatile unsigned int)1;
			mb();

			while(!flag) {
				Ac49xUserDef_DelayForDeviceResponse();
				flag = (*pPllReg) & (volatile unsigned int)1;
				mb();
			}
		}
		/**************************************************************************************/
		void Ac49xUserDef_MifAndPeripheralsResetRelease(void)
		{
			volatile unsigned int RegVal;
			volatile unsigned int *pReg;

			pReg = (volatile unsigned int*)(KSEG1ADDR(0x05080000));
			RegVal    = *pReg;
			mb();
			RegVal   |= ((volatile unsigned int)0x00000763);
			*pReg     = RegVal;
			mb();

		}
		/**************************************************************************************/
		void Ac49xUserDef_CodecResetRelease(void)
		{
			/* At revision 1.0 there is no software reset */
			volatile unsigned int RegVal;
			volatile unsigned int *pReg;

			RegVal = (volatile unsigned int )0x00000000; /*Reset internal AIC20 Voice Codec*/
			pReg  = (volatile unsigned int*)(KSEG1ADDR(0x050803F0));
			*pReg = RegVal;
			mb();
		}
		/**************************************************************************************/
		void Ac49xUserDef_DspResetRelease(void)
		{
			volatile unsigned int RegVal;
			volatile unsigned int *pReg;

			pReg = (volatile unsigned int*)(KSEG1ADDR(0x05080000));
			RegVal    = *pReg;
			mb();
			RegVal   |= (volatile unsigned int)0x00000080;
			*pReg     = RegVal;
			mb();
		}
		/**************************************************************************************/
		void Ac49xUserDef_InitCache(void)
		{
			volatile unsigned int RegVal;
			volatile unsigned int *pReg;

			RegVal = CASH_SEG_B;
			pReg  = (volatile unsigned int*)(KSEG1ADDR(0x05080014));
			*pReg = RegVal;
			mb();
		}
		/**************************************************************************************/
		void Ac49xUserDef_InterruptMips(void)
		{
			volatile U32 RegVal = 0x00000001;
			volatile U32 *pReg  = (volatile unsigned int*)(KSEG1ADDR(0x05080008));
			*pReg = RegVal;
			mb();
		}
		/**************************************************************************************/
		Tac49xProgramDownloadStatus  Ac49xUserDef_Boot(int Device, char *pProgram, int ApplicationParam, char *pKernel)
		{
			Tac49xHcrfStatusRegisterIndex ResponseValue;
			volatile U32 RegVal;
			volatile U32 *pReg  = (volatile unsigned int*)(KSEG1ADDR(0xa8611600));
			RegVal = *pReg;

			if(RegVal&0x000100000)
				{
				int RetVal = Ac49xHcrfDeviceReload(Device, pProgram, &ResponseValue);
				if((RetVal==0) && (ResponseValue==HCRF_STATUS_REGISTER_INDEX__RELOAD_COMPLETED))
					return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
				else
					return PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_FAIL_CHECKSUM;
				}
			else
				return Ac49xBoot(Device, pProgram, ApplicationParam, pKernel);
		}
	#endif
#endif


/*************************************************************************************/
/*************************************************************************************/
/*                                   CRITICAL SECTIONS                               */
/*************************************************************************************/
/*************************************************************************************/
#if     (AC49X_OS_CONFIG == AC49X_OS__WINDOWS) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
static CRITICAL_SECTION gCriticalSection_Command;
    static CRITICAL_SECTION gCriticalSection_Hpi[AC49X_NUMBER_OF_DEVICES/NUMBER_OF_DEVICES_PER_DSP]; /*2 for each transfer medium*/
    #if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__HOST)
        static CRITICAL_SECTION gCriticalSection_Fifo[NUMBER_OF_DEVICES_PER_DSP][2];/*2 for each transfer medium*/
#endif

#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
    static SEM_ID gCriticalSection_Command;
    static SEM_ID gCriticalSection_Hpi[AC49X_NUMBER_OF_DEVICES/NUMBER_OF_DEVICES_PER_DSP];
    #if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__HOST)
        static SEM_ID gCriticalSection_Fifo[AC49X_NUMBER_OF_DEVICES][2];
#endif

#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)

#endif


/* C O M M A N D */
/**************************************************************************************/
void Ac49xUserDef_CriticalSection_EnterCommand(int Device)	
{
#if     (AC49X_OS_CONFIG == AC49X_OS__WINDOWS) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
    EnterCriticalSection(&gCriticalSection_Command);
#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
    semTake(gCriticalSection_Command, WAIT_FOREVER);
#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
#endif
}
/**************************************************************************************/
void Ac49xUserDef_CriticalSection_LeaveCommand(int Device)
{
#if     (AC49X_OS_CONFIG == AC49X_OS__WINDOWS) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
    LeaveCriticalSection(&gCriticalSection_Command);
#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
    semGive(gCriticalSection_Command);
#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
#endif
}


/* H P I */
/**************************************************************************************/
void Ac49xUserDef_CriticalSection_EnterHpi(int Device)
{
#if     (AC49X_OS_CONFIG == AC49X_OS__WINDOWS) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
    EnterCriticalSection(&gCriticalSection_Hpi[Device/NUMBER_OF_DEVICES_PER_DSP]);
#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
    semTake(gCriticalSection_Hpi[Device], WAIT_FOREVER);
#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
#endif
}
/**************************************************************************************/
void Ac49xUserDef_CriticalSection_LeaveHpi(int Device)
{
#if     (AC49X_OS_CONFIG == AC49X_OS__WINDOWS) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
    LeaveCriticalSection(&gCriticalSection_Hpi[Device/NUMBER_OF_DEVICES_PER_DSP]);
#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
    semGive(gCriticalSection_Hpi[Device]);
#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
#endif
}


/* F I F O */
/**************************************************************************************/
void Ac49xUserDef_CriticalSection_EnterFifo(int Device, Tac49xTransferMedium TransferMedium)
{
#if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__HOST)
    if(  (FIFO_USAGE_ENABLE__HOST    && TransferMedium==TRANSFER_MEDIUM__HOST)
      || (FIFO_USAGE_ENABLE__NETWORK && TransferMedium==TRANSFER_MEDIUM__NETWORK)
      )
        {
#if     (AC49X_OS_CONFIG == AC49X_OS__WINDOWS) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
        EnterCriticalSection(&gCriticalSection_Fifo[Device][TransferMedium]);
#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
        semTake(&gCriticalSection_Fifo[Device][TransferMedium], WAIT_FOREVER);
#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)

#endif
        }


#endif /*(FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__HOST)*/
}
/**************************************************************************************/
void Ac49xUserDef_CriticalSection_LeaveFifo(int Device, Tac49xTransferMedium TransferMedium)
{
#if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__HOST)
    if(  (FIFO_USAGE_ENABLE__HOST    && TransferMedium==TRANSFER_MEDIUM__HOST)
      || (FIFO_USAGE_ENABLE__NETWORK && TransferMedium==TRANSFER_MEDIUM__NETWORK)
      )
        {
#if     (AC49X_OS_CONFIG == AC49X_OS__WINDOWS) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
    LeaveCriticalSection(&gCriticalSection_Fifo[Device][TransferMedium]);
#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
    semGive(&gCriticalSection_Fifo[Device][TransferMedium]);
#elif (AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)

#endif
        }

#endif /*(FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__HOST)*/
}

/**************************************************************************************/
void  Ac49xUserDef_InitUserDefSection(void)
{
	int Device;
    int TransferMedium;
    for(Device=0; Device<AC49X_NUMBER_OF_DEVICES; Device++)
        {
#if (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB)
        ac49xUserDef_HpiPhysicalAddress[Device] = (Tac49xHpiPort*)(void*)(AC49XIF_BASE_ADDRESS+AC49XIF_HCNTL0_OFFSET+(Device%NUMBER_OF_DEVICES_PER_DSP)*AC49XIF_DEVICE_OFFSET);
#elif (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
        ac49xUserDef_HpiPhysicalAddress[Device] = (Tac49xHpiPort*)(void*)(AC49XIF_BASE_ADDRESS+(Device%NUMBER_OF_DEVICES_PER_DSP)*AC49XIF_DEVICE_OFFSET);
#elif	(  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC495_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC496_DEVICE)\
		|| (AC49X_DEVICE_TYPE == AC497_DEVICE)\
		)
        if(Device==0)
            ac49xUserDef_HpiPhysicalAddress[Device] = (Tac49xHpiPort*)(void*)(DSP_PHYSICAL_ADDRESS_OFFSET|DSP_VIRTUAL_ADDRESS_OFFSET);
#if(AC49X_NUMBER_OF_DEVICES>1)
        else
            ac49xUserDef_HpiPhysicalAddress[Device] = (Tac49xHpiPort*)(void*)AcVlynq_GetAc498BaseAddress(Device-1);
#endif
#endif
        }



#if     (AC49X_OS_CONFIG == AC49X_OS__WINDOWS) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB) || (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
	for(Device=0; Device<AC49X_NUMBER_OF_DEVICES; Device++)
        {
        InitializeCriticalSection(&gCriticalSection_Hpi[Device/NUMBER_OF_DEVICES_PER_DSP]);
	    for(TransferMedium=TRANSFER_MEDIUM__HOST; TransferMedium<=TRANSFER_MEDIUM__NETWORK; TransferMedium++)
            {
            #if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)
            if(  (FIFO_USAGE_ENABLE__HOST    && TransferMedium==TRANSFER_MEDIUM__HOST)
              || (FIFO_USAGE_ENABLE__NETWORK && TransferMedium==TRANSFER_MEDIUM__NETWORK)
              )
    		    InitializeCriticalSection(&gCriticalSection_Fifo[Device][TransferMedium]);
            #endif
            }
        }
    InitializeCriticalSection(&gCriticalSection_Command);

#elif	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
	for(Device=0; Device<AC49X_NUMBER_OF_DEVICES; Device++)
        {
        gCriticalSection_Hpi[Device/NUMBER_OF_DEVICES_PER_DSP] = semBCreate(SEM_Q_FIFO, SEM_FULL);
	    for(TransferMedium=TRANSFER_MEDIUM__HOST; TransferMedium<=TRANSFER_MEDIUM__NETWORK; TransferMedium++)
            {
            #if (FIFO_USAGE_ENABLE__HOST || FIFO_USAGE_ENABLE__NETWORK)
            if(  (FIFO_USAGE_ENABLE__HOST    && TransferMedium==TRANSFER_MEDIUM__HOST)
              || (FIFO_USAGE_ENABLE__NETWORK && TransferMedium==TRANSFER_MEDIUM__NETWORK)
              )
    		    gCriticalSection_Fifo[Device][TransferMedium] = semBCreate(SEM_Q_FIFO, SEM_FULL);
            #endif
            }
        }
    gCriticalSection_Command = semBCreate(SEM_Q_FIFO, SEM_FULL);

#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)

#endif

}

#if		(AC49X_DEVICE_TYPE == ACLX5280_DEVICE)
#ifndef ACL_SIMPLE_KERNEL
/**************************************************************************************/
__ac49xdrv unsigned long Ac49xUserDef_DisableInterrupts(void)
{
	unsigned long flags;
	save_flags(flags);
	cli();
	return flags;
}

/**************************************************************************************/
__ac49xdrv void Ac49xUserDef_RestoreInterrupts(unsigned long flags)
{
	restore_flags(flags);
}
#else
/**************************************************************************************/
__ac49xdrv  unsigned long Ac49xUserDef_DisableInterrupts(void)	{}

/**************************************************************************************/
__ac49xdrv void Ac49xUserDef_RestoreInterrupts(unsigned long flags) {}
#endif



/**************************************************************************************/
static unsigned long _pollTime100msec, cnt=0;
__ac49xdrv void Ac49xUserDef_SetPollingIntervalTime100msec(unsigned long poll100msec)	{_pollTime100msec = poll100msec;}
/**************************************************************************************/
__ac49xdrv unsigned long Ac49xUserDef_GetPollingIntervalTime100msec()
{
#if 0 //100ms
	cnt = (cnt + 1)%10;
	
	if (cnt == 0)
		return 1;
	else
		return 0;
#else //10ms
		return 1;
		//return _pollTime100msec;
#endif
}
#endif
/*************************************************************************************/
/*************************************************************************************/
/*                          END OF CRITICAL SECTIONS                                 */
/*************************************************************************************/
/*************************************************************************************/

#if __cplusplus
			}
#endif

