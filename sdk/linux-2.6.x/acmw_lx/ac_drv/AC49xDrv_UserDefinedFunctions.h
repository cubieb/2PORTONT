/** @file
 *  @brief This file contains 'User-defined Function Prototypes' and User Defined Read / Write Macro Definitions,
 *  in other words, the user's profiling of the AC49x driver stack.
 *  @n Users must set the definitions contained in this file according to their application and hardware environment.
 */

/************************************************************************************/
/* AC49x Device Drivers / User-defined Function Prototypes							*/
/*																					*/
/* Copyright (C) 2000 AudioCodes Ltd.												*/
/*																					*/
/* Modifications :																	*/
/*																					*/
/* 1/7/01 - Coding started.															*/
/*																					*/
/************************************************************************************/

#ifndef AC49XUFN_H
#define AC49XUFN_H

#if (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
	)
	#if AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA
		#include <linux/module.h>
		#include <linux/types.h>
		#include <linux/byteorder/little_endian.h>
		#include <asm/avalanche/titan/titan.h>
	#endif
#elif (AC49X_DEVICE_TYPE == ACLX5280_DEVICE)
#ifndef ACL_SIMPLE_KERNEL
#if defined (ACL_LINUX_2_4_18)
		#include <linux/module.h>
		#include <linux/types.h>
#else
#ifndef NULL
#define NULL 0
#endif
#endif //ACL_LINUX_2_4_18
#else
#ifndef NULL
#define NULL 0
#endif
#endif
#endif /*((AC49X_DEVICE_TYPE == AC494_DEVICE) ||...*/

/*******************************************/
/*******************************************/
/**   User Defined Function Prototypes    **/
/*******************************************/
/*******************************************/

#if __cplusplus
extern "C" {
#endif

#define PACKETS_RECORDING__MAX_PACKET_SIZE 6000
#if AC49X_ENDIAN_MODE == BIG_ENDIAN

typedef volatile struct
{
	U8 Ver					:8;  /*!< Driver version (e.g 107).*/
	U8 SubVersion			:8;  /*!< Driver sub version (e.g 7).*/
	U8 Direction			:1;  /*!< @li 0 = Packet transmitted to AC49x. @li 1 = Packet received from AC49x.*/
	U8						:7;
	U8 Device				:8;  /*!< The Device ID that the packet was received or transmitted from/to.*/

	U8 SequenceNumber_Msb	:8;  /*!< The packet's sequence number.*/
	U8 SequenceNumber_Lsb	:8;
	U8						:8;
	U8						:8;

	U8 TimeStamp_MswMsb		:8;  /*!< Time stamp, may be the system's time.*/
	U8 TimeStamp_MswLsb		:8;
	U8 TimeStamp_LswMsb		:8;
	U8 TimeStamp_LswLsb		:8;

	U8						:8;
	U8						:8;
	U8						:8;
	U8						:8;
} Tac49xAnalysisPacketHeader;
#else
typedef volatile struct
{
	U8 Ver					:8;
	U8 SubVersion			:8;
	U8						:7;
	U8 Direction			:1;
	U8 Device				:8;

	U8 SequenceNumber_Msb	:8;
	U8 SequenceNumber_Lsb	:8;
	U8						:8;
	U8						:8;

	U8 TimeStamp_MswMsb		:8;
	U8 TimeStamp_MswLsb		:8;
	U8 TimeStamp_LswMsb		:8;
	U8 TimeStamp_LswLsb		:8;

	U8						:8;
	U8						:8;
	U8						:8;
	U8						:8;
} Tac49xAnalysisPacketHeader;
#endif

    /**
     * @defgroup Tac49xAnalysisPacket Tac49xAnalysisPacket
     * @ingroup PACKET_RECORDING_STRUCTURES
     * @brief
     * This is the structure of the recorded packet.
     * @{
     */

    /**
     * <BR>
     * This is the structure of the recorded packet. The callback function ::TFlushPacketFunction
     * should prepare the recorded packets with this structure and fill its header parameters before sending it
     * to the network.
     */

typedef volatile struct
{
	Tac49xAnalysisPacketHeader Header;                /*!< Packet header.*/
	U8	Payload[PACKETS_RECORDING__MAX_PACKET_SIZE];  /*!< The AC49x packet.*/
} Tac49xAnalysisPacket;

    /** @} */

#ifndef NDOC
extern void	Ac49xUserDef_RaffleAndTrashPacketBits(int Device, int Channel, char *_pPacket, int _PacketSize, Tac49xTransferMedium TransferMedium);
#endif /* NDOC */

    /**
     * @defgroup Ac49xUserDef_VoppReset Ac49xUserDef_VoppReset
     * @ingroup FUNCTION_PROTOTYPES_AC491
     * @ingroup FUNCTION_PROTOTYPES_AC490
     * @brief
     * Resets the AC49x device.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_VoppReset function resets the AC49x device. It is invoked by the :: Ac49xProgramDownLoad
     * and ::Ac49xKernelDownLoad functions.
     *
     * @param Device      [in] .
     * @param ResetOnOff  [in] 1 = reset activate, 0 = reset release.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void Ac49xUserDef_VoppReset(int Device, int ResetOnOff);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_DelayForDeviceResponse Ac49xUserDef_DelayForDeviceResponse
     * @ingroup FUNCTION_PROTOTYPES_COMMON
     * @brief
     * Performs a constant delay. Used when waiting for the AC49x VoPP to respond.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_DelayForDeviceResponse function performs a constant delay.
     * Used when waiting for the AC49x VoPP to respond. The delay should be set to 10 ms.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void Ac49xUserDef_DelayForDeviceResponse(void);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_DelayForHostResponse Ac49xUserDef_DelayForHostResponse
     * @ingroup FUNCTION_PROTOTYPES_COMMON
     * @brief
     * Performs a constnat delay. Used when waiting for the Host to respond
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_DelayForHostResponse function performs a constnat delay.
     * Used when waiting for the Host to respond. This function is used in the FIFO mechanisim. The delay time
     * is up to the user. During the delay other host tusks (in a multi-task environment) can be served.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void Ac49xUserDef_DelayForHostResponse(void);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_GetGpio0 Ac49xUserDef_GetGpio0
     * @ingroup FUNCTION_PROTOTYPES_AC491
     * @brief
     * Returns the Gpio0 output pin value.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_GetGpio0 function returns the Gpio0 output pin value. It is used in the program's
     * download process.
     *
     * @param DSP  [in] .
     *
     * @par Return values:
     * @li 0 if the Gpio0 output pin = 0.
     * @li 1 if the Gpio0 output pin = 1.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern int Ac49xUserDef_GetGpio0(int DSP);

    /** @} */
#if  (AC49X_HPI_TYPE == AC49X_HPI_PORT)
extern Tac49xHpiPort *ac49xUserDef_HpiPhysicalAddress[AC49X_NUMBER_OF_DEVICES];
#endif
    /**
     * @defgroup Ac49xUserDef_InitUserDefSection Ac49xUserDef_InitUserDefSection
     * @ingroup FUNCTION_PROTOTYPES_COMMON
     * @brief
     * Initializes the user defined section.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_InitUserDefSection function initializes the user defined section.
     * It is called by the ::Ac49xInitDriver function. For example, in the AC490 it initializes the physical
     * address of the HPI in order to read/write to the VoPP. When using several AC490 devices on a card/board,
     * this function sets an array of HPI physical addresses for each device.
     * @n Example of an array:
     * @n ac49xUserDef_HpiPhysicalAddress[0] = ADDRESS_0;
     * @n ac49xUserDef_HpiPhysicalAddress[1] = ADDRESS_1;
     * @n ac49xUserDef_HpiPhysicalAddress[2] = ADDRESS_2;
     * @n @b Note: This function is in use only with the AC490 device.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void  Ac49xUserDef_InitUserDefSection(void);

    /** @} */

#if(AC49X_DEVICE_TYPE==AC491_DEVICE)
#ifndef NDOC
    extern int Ac49xUserDef_TransmitViaUtopia(int Device,int Channel,char * pPacket,int size) ;
#endif /* NDOC */
#endif

/*              D E F I N I T I O N   E X A M P L E
Tac49xHpiPort *ac49xUserDef_HpiPhysicalAddress[AC49X_NUMBER_OF_DEVICES];
**************************************************************************************
void  Ac49xUserDef_InitUserDefSection(void)
{
	int Device;
    for(Device=0; Device<AC49X_NUMBER_OF_DEVICES; Device++)
        {
#if (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB)
        ac49xUserDef_HpiPhysicalAddress[Device] = (Tac49xHpiPort*)(void*)(AC49XIF_BASE_ADDRESS+AC49XIF_HCNTL0_OFFSET+(Device%NUMBER_OF_DEVICES_PER_DSP)*AC49XIF_DEVICE_OFFSET);
#elif (AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB)
        ac49xUserDef_HpiPhysicalAddress[Device] = (Tac49xHpiPort*)(void*)(AC49XIF_BASE_ADDRESS+(Device%NUMBER_OF_DEVICES_PER_DSP)*AC49XIF_DEVICE_OFFSET);
#endif
        }
#if FIFO_USAGE_ENABLE == 1
	for(Device=0; Device<NUMBER_OF_DEVICES_PER_DSP; Device++)
		InitializeCriticalSection(&gCriticalSection_Fifo[Device]);
#endif
    InitializeCriticalSection(&gCriticalSection_Command);
#if(AC49X_DEVICE_TYPE==AC491_DEVICE)
    InitializeCriticalSection(&gCriticalSection_Hpi);
#else
    InitializeCriticalSection(&gCriticalSection_HpiRx);
    InitializeCriticalSection(&gCriticalSection_HpiTx);
#endif
}
*/


    /**
     * @defgroup Ac49xUserDef_CriticalSection_EnterCommand Ac49xUserDef_CriticalSection_EnterCommand
     * @ingroup FUNCTION_MULTITHREADING_PROTECTION
     * @brief
     * Protects the Host port driver command functions against re-entrance.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_CriticalSection_EnterCommand function is invoked at the beginning.of all command
     * functions. In this function, users must implement a critical section entrance. If the section is available,
     * the function returns; if it is occupied, the function should put the current thread into sleep slate.
     *
     * @param Device      [in] .
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */


extern void Ac49xUserDef_CriticalSection_EnterCommand(int Device);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_CriticalSection_LeaveCommand Ac49xUserDef_CriticalSection_LeaveCommand
     * @ingroup FUNCTION_MULTITHREADING_PROTECTION
     * @brief
     * Protects the Host port driver command functions against re-entrance.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_CriticalSection_LeaveCommand function is invoked at the end.of all command functions.
     * In this function, users must implement a critical section release.
     *
     * @param Device      [in] .
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void Ac49xUserDef_CriticalSection_LeaveCommand(int Device);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_CriticalSection_EnterHpi Ac49xUserDef_CriticalSection_EnterHpi
     * @ingroup FUNCTION_MULTITHREADING_PROTECTION
     * @brief
     * Protects the Host Port Interface against re-entrance.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_CriticalSection_EnterHpi function is invoked at the beginning.of the
     * functions ::Ac49xTransmitPacket and ::Ac49xReceivePacket. In this function, users must implement a critical
     * section entrance. If the section is available, the function returns; if it is occupied, the function should put
     * the current thread into sleep slate.
     *
     * @param Device      [in] .
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void Ac49xUserDef_CriticalSection_EnterHpi(int Device);

    /** @} */
    /**
     * @defgroup Ac49xUserDef_CriticalSection_LeaveHpi Ac49xUserDef_CriticalSection_LeaveHpi
     * @ingroup FUNCTION_MULTITHREADING_PROTECTION
     * @brief
     * Protects the Host Port Interface against re-entrance.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_CriticalSection_LeaveHpi function is invoked at the end.of the
     * functions ::Ac49xTransmitPacket and ::Ac49xReceivePacket. In this function, users must implement a critical
     * section release.
     *
     * @param Device      [in] .
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void Ac49xUserDef_CriticalSection_LeaveHpi(int Device);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_CriticalSection_EnterFifo Ac49xUserDef_CriticalSection_EnterFifo
     * @ingroup FUNCTION_MULTITHREADING_PROTECTION
     * @brief
     * Protects the FIFO functions against re-entrance.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_CriticalSection_EnterFifo function is invoked at the beginning.of the functions
     * ::Ac49xDequeueAndTransmitCell and ::Ac49xTransmitPacket. Users must implement a critical section entrance.
     * If the section is available, the function returns; if it is occupied, the function should put the current
     * thread into sleep slate. This function is available only if the FIFO mechanism is enabled.
     *
     * @param Device            [in]
     * @param TransferMedium    [in] ::Tac49xTransferMedium
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void Ac49xUserDef_CriticalSection_EnterFifo(int Device, Tac49xTransferMedium TransferMedium);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_CriticalSection_LeaveFifo Ac49xUserDef_CriticalSection_LeaveFifo
     * @ingroup FUNCTION_MULTITHREADING_PROTECTION
     * @brief
     * Protects the FIFO functions against re-entrance.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_CriticalSection_LeaveFifo function is invoked at the end.of the functions
     * ::Ac49xDequeueAndTransmitCell and ::Ac49xTransmitPacket. Users must implement a critical section release.
     * This function is available only if the FIFO mechanism is enabled.
     *
     * @param Device            [in]
     * @param TransferMedium    [in] ::Tac49xTransferMedium
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

extern void Ac49xUserDef_CriticalSection_LeaveFifo(int Device, Tac49xTransferMedium TransferMedium);

    /** @} */


#if (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
    )

    #if (AC49X_NUMBER_OF_DEVICES>1)

    /**
     * @defgroup Ac49xUserDef_498SetPeripheralResetRegister Ac49xUserDef_498SetPeripheralResetRegister
     * @ingroup FUNCTION_PROTOTYPES_AC498
     * @brief 
     * Writes a value to the AC498 VoPP PRCR Register.
     * @{
     */

    /**
     * <BR>
     * The function @b Ac49xUserDef_498SetPeripheralResetRegister writes a value to the AC498 VoPP PRCR Register. 
     * The function is accessed by the program download. Refer to <A HREF="group__AC498__VOPP.html"> AC498 Low-Density VoPP</A> 
     * for details of the Register's contents.
     * 
     * @param Device            [in] The AC498 device. The range is 1-3 (Device 0 is always the AC494 DSP). 
     * @param Value             [in] The value that is written to the PRCR Register.
     * 
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_498SetPeripheralResetRegister(int Device, int Value);

    /** @} */

    #endif

    /**
     * @defgroup Ac49xUserDef_HardResetAssertFor16Cycles Ac49xUserDef_HardResetAssertFor16Cycles
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief
     * Activates the DSP hard reset and waits 16 MIPS cycles
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_HardResetAssertFor16Cycles function activates the DSP hard reset and waits 16
     * MIPS cycles. This function is invoked by the ::Ac49xProgramDownLoad function.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_HardResetAssertFor16Cycles(void);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_HardResetReleaseAndWait150Cycles Ac49xUserDef_HardResetReleaseAndWait150Cycles
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief
     * Releases the DSP's hard reset and waits 150 MIPS cycles.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_HardResetReleaseAndWait150Cycles function releases the DSP's hard reset and waits 150
     * MIPS cycles. This function is invoked by the ::Ac49xProgramDownLoad function.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_HardResetReleaseAndWait150Cycles(void);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_PllProgramTo125_MHz Ac49xUserDef_PllProgramTo125_MHz
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief
     * Sets the DSP's frequency to 125 MHz (AC494) and 100 MHz (AC495).
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_PllProgramTo125_MHz function sets the DSP's frequency to 125 MHz (AC494) and 100 MHz (AC495).
     * This function is invoked by the ::Ac49xProgramDownLoad function. If the PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL
     * definition is set to zero, this function sets the DSP's frequency to 122.88 MHz (AC494) and 98.304 MHz (AC495).
     * The DSP uses this frequency to derive the PCM clock frequency.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_PllProgramTo125_MHz(void);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_MifAndPeripheralsResetRelease Ac49xUserDef_MifAndPeripheralsResetRelease
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief
     * Releases the internal DSP's memory and peripherals from reset.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_MifAndPeripheralsResetRelease function releases the internal DSP's memory and peripherals
     * from reset. This function is invoked by the ::Ac49xProgramDownLoad function.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_MifAndPeripheralsResetRelease(void);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_CodecResetRelease Ac49xUserDef_CodecResetRelease
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief
     * Releales the internal codec's reset.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_CodecResetRelease function releales the internal codec's reset.
     * This function is invoked by the ::Ac49xProgramDownLoad function.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_CodecResetRelease(void);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_DspResetRelease Ac49xUserDef_DspResetRelease
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief
     * Releales the DSP's reset
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_DspResetRelease function releales the DSP's reset. After this command the DSP starts running.
     * This function is invoked by the ::Ac49xProgramDownLoad function.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_DspResetRelease(void);

    /** @} */

    /**
     * @defgroup Ac49xUserDef_InitCache Ac49xUserDef_InitCache
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief
     * Initializes the DSP's cash
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_InitCache function initializes the DSP's cash. It maps the DSP's code to address 0x14000000
     * on the external memory (the DSP uses up to 256 kbytes of external memory). This function
     * is invoked by the ::Ac49xProgramDownLoad function.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_InitCache(void);

    /** @} */

#ifndef NDOC
    Tac49xProgramDownloadStatus  Ac49xUserDef_Boot(int Device, char *pProgram, int ApplicationParam, char *pKernel);
#endif /* NDOC */

    /**
     * @defgroup Ac49xUserDef_InterruptMips Ac49xUserDef_InterruptMips
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief
     * Interrupts the DSP
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_InterruptMips function interrupts the DSP. This function is invoked by @link DEBUG_AND_RECOVERY Debug and Recovery @endlink functions.
     *
     * @param
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    void Ac49xUserDef_InterruptMips(void);

    /** @} */

#endif /*((AC49X_DEVICE_TYPE == AC494_DEVICE) ||...*/

/*----------------------------------------------------------------------------------*/
/* ROUTINE:   IO_ReadByte / IO_ReadShort / IO_ReadLong								*/
/*																					*/
/* DESCRIPTION:																		*/
/*   Reads a byte (8 bits) / short (16 bits) / long (32 bits) from the DSP when		*/
/*   using PC based applications (not relevant in embedded applications when using	*/
/*   direct memory access).															*/
/*																					*/
/* ARGUMENTS:																		*/
/* [i]int BoardID                  - Board ID										*/
/* [i]unsigned long dwIOAddr       - The address on the board						*/
/*																					*/
/* RESULTS:																			*/
/* [o]unsigned char                - The char read									*/
/*----------------------------------------------------------------------------------*/
extern unsigned char  IO_ReadByte(unsigned  long dwIOAddr,int Board);
extern unsigned short IO_ReadShort(unsigned long dwIOAddr,int Board);
extern unsigned long  IO_ReadLong(unsigned  long dwIOAddr,int Board);

/*----------------------------------------------------------------------------------*/
/* ROUTINE:   IO_WriteByte / IO_WriteShort / IO_WriteLong							*/
/*																					*/
/* DESCRIPTION:																		*/
/*   Writes byte from DSP device when using PC based applications					*/
/*    (not relevant in embedded applications when using direct memory access).		*/
/*																					*/
/* ARGUMENTS:																		*/
/* [i]int BoardID                  - Board ID										*/
/* [i]unsigned long dwIOAddr       - The address on the board						*/
/*																					*/
/* RESULTS:																			*/
/* [o]unsigned char                - The char read									*/
/*----------------------------------------------------------------------------------*/
extern void IO_WriteByte( unsigned long dwIOAddr, unsigned char  bData, int Board);
extern void IO_WriteShort(unsigned long dwIOAddr, unsigned short wData, int Board);
extern void IO_WriteLong( unsigned long dwIOAddr, unsigned long dwData, int Board);

/******************************************************/
/******************************************************/
/**  User Defined Read / Write Macro Definitions     **/
/******************************************************/
/******************************************************/

                    /* The following definitions are used when accessing the HPI via I/O  */
                    /* Each word is associated with its offset from the HPI base address. */
                    /* For detailed HPI structure, look for 'Tac49xHpiPort' structure.    */
#if (AC49X_DEVICE_TYPE == AC491_DEVICE)
	#define HPIC1st_Offset	0
	#define HPIC2nd_Offset	8
	#define HPIDA1st_Offset	16
	#define HPIDA2nd_Offset	24
	#define HPIA1st_Offset	32
	#define HPIA2nd_Offset	40
	#define HPID1st_Offset	48
	#define HPID2nd_Offset	56
#elif (AC49X_DEVICE_TYPE == AC490_DEVICE)
	#define HPIC_Offset (-AC49XIF_HCNTL0_OFFSET)
#endif
/* The following definitions are for Write and Read macros to and from the DSP memory. */
/* The macros are defined differently for varios memory access types. */
/* The conditions relate to the way the DSP is mapped in the host's memory (either */
/* directly or indirectly (via I/O) and to the DSP's host port interface type (either */
/* random access or auto-increment (multiplexed). */

            /*********************************/
            /* PCI driver for win95/winNT/XP */
            /*********************************/

                /* For Single Vopp PCI Boards */
                /* For Multi Device Vopp */

#if (AC49X_DEVICE_TYPE == AC491_DEVICE)
                /* For Multi Device Vopp */

    /**
     * @defgroup Ac49xUserDef_WriteShort Ac49xUserDef_WriteShort
     * @ingroup READ_WRITE_MACROS_AC490
     * @ingroup READ_WRITE_MACROS_AC491
     * @brief
     * Writes a 16-bit word to the AC491/AC490 VoPP host port.
     * @{
     */

    /**
     * <BR>
     * @b AC490:
     * @n The @b Ac49xUserDef_WriteShort macro
     * writes a 16-bit word to the AC490 VoPP host port. It is used by the ::Ac49xWriteBlock function.
     * This function writes the 16-bit word directly to the device location defined by the Addr input field.
     * The AC490 device is a Big Endian machine. In Little Endian hosts the two 8-bit sections of the 16 bit
     * data fields are inverted. This incompatibility can be resolved in two methods:
     * @li Software: Invert the data field bytes in this macro. @li Hardware: Invert the data pins in the hardware.
     *
     * @n Big Endian
     * @n ----------------------
     * @n | 1st byte | 2nd Byte |
     * @n ----------------------
     * @n Little Endian
     * @n ----------------------
     * @n | 2nd byte | 1st Byte |
     * @n ----------------------
     *
     * @param Device  [in] The device in which the word is written to.
     * @param Addr    [in] Data address.
     * @param Data    [in] 16-bit data word.
     *
     * @n @b AC491:
     * @n The @b Ac49xUserDef_WriteShort macro writes a 16-bit word to the AC491 VoPP host port. It is used
     * by the ::Ac49xWriteBlock function.
     * It writes a 16-bit word to one of the following registers: HPIA, HPID, HPIC, HPID (with post increment).
     * @n @b Note: In the AC491 VoPP, every consecutive 6 devices (cores) share the same registers.
     * The AC491 device is a Big Endian machine. In Little Endian hosts the two 8-bit sections of the 16 bit
     * data fields are inverted. This incompatibility can be resolved in two methods:
     * @li Software: Invert the data field bytes in this macro. @li Hardware: Invert the data pins in the hardware.
     *
     * @n Big Endian
     * @n ----------------------
     * @n | 1st byte | 2nd Byte |
     * @n ----------------------
     * @n Little Endian
     * @n ----------------------
     * @n | 2nd byte | 1st Byte |
     * @n ----------------------
     *
     * @param Device  [in] The device in which the word is written to.
     * @param Addr    [in] Register address. 0 = HPIC. 1 = HPID, with HPIA post-increment. 2 = HPIA. 3 = HPID, HPIA not affected.
     * @param Data    [in] 16-bit data word.
     *
     * @par Return values:
     * @e None.
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

	#define Ac49xUserDef_WriteShort(Device, Addr, Data) IO_WriteShort((unsigned long)(Addr##_Offset), Data, (Device/NUMBER_OF_DEVICES_PER_DSP))

    /** @} */

    /**
     * @defgroup Ac49xUserDef_ReadShort Ac49xUserDef_ReadShort
     * @ingroup READ_WRITE_MACROS_AC490
     * @ingroup READ_WRITE_MACROS_AC491
     * @brief
     * Reads a 16-bit word from the AC490/AC491 VoPP host port.
     * @{
     */

    /**
     * <BR>
     * @b AC490
     * @n The @b Ac49xUserDef_ReadShort macro
     * reads a 16-bit word from the AC490 VoPP host port. It is used by the ::Ac49xReadBlock function.
     * This function reads the 16-bit word directly from the device location defined by the Addr input field.
     *
     * @param Device  [in] The device in which the word is read from.
     * @param Addr    [in] Data address.
     *
     * @b AC491
     * @n The @b Ac49xUserDef_ReadShort macro
     * reads a 16-bit word from the AC491 VoPP host port. It is used by the ::Ac49xReadBlock function.
     * It reads a 16-bit word from one of the following registers: HPIA, HPID, HPIC, HPID (with post increment).
     * @n @b Note: In the AC491 VoPP, every consecutive 6 devices (cores) share the same registers.
     *
     * @param Device  [in] The device in which the word is read from.
     * @param Addr    [in] Register address. 0 = HPIC. 1 = HPID, with HPIA post-increment. 2 = HPIA. 3 = HPID, HPIA not affected.
     *
     * @par Return values:
     * @e Data (16-bit short).
     *
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

	#define Ac49xUserDef_ReadShort( Device, Addr)       IO_ReadShort( (unsigned long)(Addr##_Offset),       (Device/NUMBER_OF_DEVICES_PER_DSP))

    /** @} */

#endif /* (AC49X_DEVICE_TYPE == AC491_DEVICE) */

#if (AC49X_DEVICE_TYPE == AC490_DEVICE)

                /* For Single Device Vopp */

    #define Ac49xUserDef_WriteShort(Device, Addr, Data) IO_WriteShort((unsigned long)ac49xUserDef_HpiPhysicalAddress[Device]+Addr, Data, Device)
    #define Ac49xUserDef_ReadShort( Device, Addr)       IO_ReadShort( (unsigned long)ac49xUserDef_HpiPhysicalAddress[Device]+Addr,       Device)

#endif  /* (AC49X_DEVICE_TYPE == AC490_DEVICE) */

#if (  (AC49X_DEVICE_TYPE == AC494_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC495_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC496_DEVICE)\
    || (AC49X_DEVICE_TYPE == AC497_DEVICE)\
	)

#if AC49X_ENDIAN_MODE == LITTLE_ENDIAN
#define UlongBigEndian2LittleEndian(x)((((x) & 0x000000FF) << 24) |\
									   (((x) & 0x0000FF00) <<  8) |\
									   (((x) & 0x00FF0000) >>  8) |\
									   (((x) & 0xFF000000) >> 24))
#define UlongLittleEndian2BigEndian(x) UlongBigEndian2LittleEndian(x)
#endif



#if(AC49X_NUMBER_OF_DEVICES>1)

    /**
     * @defgroup AcVlynq_MapToAc498Registers AcVlynq_MapToAc498Registers
     * @ingroup FUNCTION_PROTOTYPES_AC498
     * @brief 
     * Maps the Vlynq portal of all AC498 VoPPs in the chain to the internal registers.
     * @{
     */

    /**
     * <BR>
     * The Vlynq portal can map either the AC498 VoPP's internal memory or its internal registers. During program download, 
     * both should be accessed. During run time, only the internal memory should be accessed. The function @b AcVlynq_MapToAc498Registers 
     * maps the Vlynq portal of all AC498 VoPPs in the chain to the internal registers.
     * 
     * @param 
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    extern void AcVlynq_MapToAc498Registers();

    /** @} */


    /**
     * @defgroup AcVlynq_MapToAc498LocalMemory AcVlynq_MapToAc498LocalMemory
     * @ingroup FUNCTION_PROTOTYPES_AC498
     * @brief 
     * Maps the Vlynq portal of all AC498 VoPPs in the chain to the internal memory.
     * @{
     */

    /**
     * <BR>
     * The Vlynq portal can map either the AC498 VoPP's internal memory or its internal registers. During program download, 
     * both should be accessed. During run time, only the internal memory should be accessed. The function @b AcVlynq_MapToAc498LocalMemory 
     * maps the Vlynq portal of all AC498 VoPPs in the chain to the internal memory.
     * 
     * @param 
     * This function has no parameters.
     *
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    extern void AcVlynq_MapToAc498LocalMemory();

    /** @} */

    /**
     * @defgroup AcVlynq_GetAc498BaseAddress AcVlynq_GetAc498BaseAddress
     * @ingroup FUNCTION_PROTOTYPES_AC498
     * @brief 
     * Returns the base address in the Vlynq portal of the specific AC498 device.
     * @{
     */

    /**
     * <BR>
     * The function @b AcVlynq_GetAc498BaseAddress returns the base address in the Vlynq portal of the specific AC498 device.
     * The base address is later used by the driver to access the AC498 device.
     * 
     * @param Device            [in] The AC498 device. The range is 1-3 (Device 0 is always the AC494 DSP). 
     * 
     * @par Return values:
     * @e U32 - the virtual address on the Vlynq portal of the specific AC498 device.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    extern U32  AcVlynq_GetAc498BaseAddress(int Device);

    /** @} */
    
    /**
     * @defgroup AcVlynq_WaitForVlynqLinkStatus AcVlynq_WaitForVlynqLinkStatus
     * @ingroup FUNCTION_PROTOTYPES_AC498
     * @brief 
     * Waits 1 msec before rechecking the Vlynq link to all AC498 VoPPs in the chain.
     * @{
     */

    /**
     * <BR>
     * During program download, the chained AC498 VoPPs change their CPU clock from 25 MHz to 150 MHz. 
     * During this time, the Vlynq link is lost and restored. The function @b AcVlynq_WaitForVlynqLinkStatus waits
     * 1 msec before rechecking the Vlynq link to all AC498 VoPPs in the chain.
     * 
     * @param 
     * This function has no parameters.
     * 
     * @par Return values:
     * @e U32 : @li 0 = Successfull links @li -1 = Links failed
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    extern U32  AcVlynq_WaitForVlynqLinkStatus();

    /** @} */

    /**
     * @defgroup AcVlynq_494UpadateVlynqClk AcVlynq_494UpadateVlynqClk
     * @ingroup FUNCTION_PROTOTYPES_AC498
     * @brief 
     * Changes the Vlynq clock to the maximum value <= 75 MHz.
     * @{
     */

    /**
     * <BR>
     * After resetting, the CPU clock of the AC498 is 25 MHz. The AC494 is the master of the Vlynq clock. 
     * During this time, the Vlynq clock cannot exceed 25 Mhz. During program download, the AC498 changes the CPU
     * clock to 150 MHz. From this moment on, the Vlynq clock can be set to up to 75 MHz. The function @b AcVlynq_494UpadateVlynqClk 
     * changes the Vlynq clock to the maximum value <= 75 MHz.
     * 
     * @param 
     * This function has no parameters.
     * 
     * @par Return values:
     * @e int - @li 0 = Successful link @li -1 = Link failed
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

    extern int  AcVlynq_494UpadateVlynqClk();

    /** @} */

#endif

    /**
     * @defgroup Ac49xUserDef_WriteLong Ac49xUserDef_WriteLong
     * @ingroup READ_WRITE_MACROS_AC494
     * @brief 
     * Writes a 32-bit word to the DSP's internal RAM.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_WriteLong macro
     * writes a 32-bit word to the DSP's internal RAM. It is used by the ::Ac49xWriteBlock function.
     * This function writes the 32-bit word directly to the device location defined by the Addr input field.
     * @n @b Note: The data must be placed at a long aligned address.
     * 
     * @param Device  [in] @li 0 = AC494 DSP @li 1-3 = AC498 VoPP
     * @param Addr    [in] Data address.
     * @param Data    [in] 32-bit data word.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */
#if	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
	#if AC49X_ENDIAN_MODE == LITTLE_ENDIAN
		#define Ac49xUserDef_WriteLong(Device, Addr, Data) (*(volatile unsigned int* )(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device])) = UlongLittleEndian2BigEndian(Data)
	#else
		#define Ac49xUserDef_WriteLong(Device, Addr, Data) (*(volatile unsigned int* )(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device])) = Data
	#endif

#elif (AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
	#define Ac49xUserDef_WriteLong(Device, Addr, Data)	   (*(volatile unsigned int*)(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device])) = __cpu_to_be32(Data)

#elif (AC49X_OS_CONFIG == AC49X_OS__WINDOWS)
	#if AC49X_ENDIAN_MODE == LITTLE_ENDIAN
		#define Ac49xUserDef_WriteLong(Device, Addr, Data) (*(volatile unsigned int* )(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device])) = UlongLittleEndian2BigEndian(Data)
	#else
		#define Ac49xUserDef_WriteLong(Device, Addr, Data) (*(volatile unsigned int* )(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device])) = Data
	#endif

#else /*AC49X_OS__NONE*/
	#error Ac49xUserDef_WriteLong(Device, Addr, Data)  Not Defined!!!

#endif

    /** @} */





    /**
     * @defgroup Ac49xUserDef_ReadLong Ac49xUserDef_ReadLong
     * @ingroup READ_WRITE_MACROS_AC494
     * @brief 
     * Reads a 32-bit word from the DSP's internal RAM.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_ReadLong macro
     * reads a 32-bit word from the DSP's internal RAM.. It is used by the ::Ac49xReadBlock function. 
     * This function reads the 32-bit word directly from the device location defined by the Addr input field.
     * @n @b Note: The data must be placed at a long aligned address.
     * 
     * @param Device  [in] @li 0 = AC494 DSP @li 1-3 = AC498 VoPP
     * @param Addr    [in] Data address.
     *    
     * @par Return values:
     * @e Data (32-bit long).
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

#if	(AC49X_OS_CONFIG == AC49X_OS__VXWORKS)
	#if AC49X_ENDIAN_MODE == LITTLE_ENDIAN
		#define Ac49xUserDef_ReadLong(Device, Addr)       UlongLittleEndian2BigEndian((*(volatile unsigned int* )(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device])))
	#else
		#define Ac49xUserDef_ReadLong(Device, Addr)       (*(volatile unsigned int* )(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device]))
	#endif

#elif	(AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA)
	#define Ac49xUserDef_ReadLong(Device, Addr)			  __be32_to_cpu((*(volatile unsigned int*)(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device])))

#elif (AC49X_OS_CONFIG == AC49X_OS__WINDOWS)
	#if AC49X_ENDIAN_MODE == LITTLE_ENDIAN
		#define Ac49xUserDef_ReadLong(Device, Addr)       UlongLittleEndian2BigEndian((*(volatile unsigned int* )(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device])))
	#else
		#define Ac49xUserDef_ReadLong(Device, Addr)       (*(volatile unsigned int* )(Addr|(volatile unsigned int)ac49xUserDef_HpiPhysicalAddress[Device]))
	#endif

#else /*AC49X_OS__NONE*/
	#error Ac49xUserDef_ReadLong(Device, Addr) Not Defined !!!.

#endif

    /** @} */

    /**
     * @defgroup Ac49xUserDef_EMIF_MEMCPY Ac49xUserDef_EMIF_MEMCPY
     * @ingroup FUNCTION_PROTOTYPES_AC494
     * @brief 
     * Writes an array to the external memory.
     * @{
     */

    /**
     * <BR>
     * The @b Ac49xUserDef_EMIF_MEMCPY function
     * writes an array to the external memory. It is used by the ::Ac49xProgramDownLoad function.
     * 
     * @param TargetAddress   [out] Target address on the external memory.
     * @param pSourceBuffer   [in]  Source address.
     * @param BlockSize       [in]  Array size in bytes.
     *    
     * @par Return values:
     * @e None.
     * 
     * @req
     * @b Header: Declared in AC49xDrv_UserDefinedFunctions.h.
     */

void Ac49xUserDef_EMIF_MEMCPY(unsigned long TargetAddress, char *pSourceBuffer, unsigned long BlockSize);

    /** @} */

#endif /* ((AC49X_DEVICE_TYPE == AC494_DEVICE) ||... */

#if (  (AC49X_DEVICE_TYPE != AC490_DEVICE)\
    && (AC49X_DEVICE_TYPE != AC491_DEVICE)\
    && (AC49X_DEVICE_TYPE != AC494_DEVICE)\
    && (AC49X_DEVICE_TYPE != AC495_DEVICE)\
    && (AC49X_DEVICE_TYPE != AC496_DEVICE)\
    && (AC49X_DEVICE_TYPE != AC497_DEVICE)\
	&& (AC49X_DEVICE_TYPE != ACLX5280_DEVICE)\
	)
    #error Unsupported AC49x Device Type (see AC49X_DEVICE_TYPE)
#endif /* ((AC49X_DEVICE_TYPE != AC494_DEVICE) && (AC49X_DEVICE_TYPE != AC495_DEVICE) && (AC49X_DEVICE_TYPE != AC490_DEVICE) && (AC49X_DEVICE_TYPE != AC491_DEVICE)) */


#if __cplusplus
}
#endif

#endif /* ifndef AC49XUFN_H */
