/** @file
 *  @brief This file includes definitions and compilation switches. Users should configure these definitions
 *  according to their operation system and processor.
 */


/************************************************************************************/
/* AC49xDrv_Cfg.h - AC49x Device Driver / User Configuration Selection Header File	*/
/*																					*/
/* Copyright (C) 2000 AudioCodes Ltd.												*/
/*																					*/
/* Modifications :																	*/
/*																					*/
/* 1/7/01 - Coding started.															*/
/*																					*/
/************************************************************************************/
#ifndef AC49XCFG_H
#define AC49XCFG_H

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC
#define ACMW_MAX_NUM_CH 4
#elif defined (CONFIG_RTK_VOIP_DRIVERS_IP_PHONE)
#define ACMW_MAX_NUM_CH 2
#else
#error "ACMW_MAX_NUM_CH is not defined" 
#endif
/*******************************************/
/*******************************************/
/**    Current Configuration Selection    **/
/*******************************************/
/*******************************************/

/* The follwoing are saved user configurations: */
/* The user can modify/add configurations.      */

/*-----------------------------------------------------------------------------------------*/
#define AC49X_CONFIG_NONE					0
#define AC49X_CONFIG_AC490_AEB				1
#define AC49X_CONFIG_AC491xxx_AEB			2
#define AC49X_CONFIG_AC494xxx_AEB			3
#define AC49X_CONFIG_AC495xxx_AEB			4
#define AC49X_CONFIG_AC496xxx_AEB			5
#define AC49X_CONFIG_AC497xxx_AEB			6

/**  Lexra Device baseed configurations   **/
#define ACLX_CONFIG_ACLX5280_A              100

#define AC49X_CURRENT_CONFIG  ACLX_CONFIG_ACLX5280_A

#ifndef AC49X_CURRENT_CONFIG
	#define AC49X_CURRENT_CONFIG	AC49X_CONFIG_NONE
#endif /* AC49X_CURRENT_CONFIG */
#if	(AC49X_CURRENT_CONFIG == AC49X_CONFIG_NONE)
	#error The selected configuration is not implemented, please set "AC49X_CURRENT_CONFIG" to one of the supported options.
#endif

#define AC49X_OS__NONE						0
#define AC49X_OS__WINDOWS					1
#define AC49X_OS__VXWORKS					2
#define AC49X_OS__LINUX_MONTAVISTA			3


/*-----------------------------------------------------------------------------------------*/

/*******************************************/
/*******************************************/
/**     Configuration Implementation      **/
/*******************************************/
/*******************************************/

/* This section implements the various saved configurations by setting relevant parameters. */
/* ---------------------------------------------------------------------------------------- */

/* AC49X_DEVICE_TYPE selects the AudioCodes Packet Processor :
 *
 * AC49X_ENDIAN_MODE selects the host's CPU byte-order type :
 *     LITTLE_ENDIAN		- (LSB first) should be used for Intel CPUs,
 *     BIG_ENDIAN			- (MSB first) should be used for Motoral/RISC CPUs
 *
 * DSP_TO_HOST_ADDRESS_SHIFT defines the difference between the DSP and the
 *     Host addressing modes. Since the DSP addresses 16-bit words, and the
 *     Host usually addresses 8-bit words, the value should be 1 to indicate that
 *     a single shift operation is required for address translations.
 *     For hosts that address 16-bit words, the value should be set to 0.
 *
 * AC49X_NUMBER_OF_DEVICES selects the number of DSP cores on the user's boards.
 *
 *
 * AC48XIF_BASE_ADDRESS defines the base address in the host's memory of the array of DSPs
 *
 *
 * AC48XIF_DEVICES_ADDRESS_OFFSET defines the offset in bytes between two consecutive DSPs.
 *     If the user does not define this constant, it is assumed that the DSPs are continously
 *     mapped in the host's memory.
 *
 *
 *     The options are listed in the table obove.
 *     This constant selects the appropriate hardware access macro in AC48xHW.h.
 *
 */


#if	AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC490_AEB
	#define AC49X_DEVICE_TYPE		        AC490_DEVICE			/* The DSP device type */
	#define AC49X_ENDIAN_MODE		        LITTLE_ENDIAN			/* The Host endian mode */
	#define DSP_TO_HOST_ADDRESS_SHIFT       1						/* Addressed word size shift (DSP->Host) */
	#define NUMBER_OF_DEVICES_PER_DSP	    1 						/* Number of devices (DSP cores) available */
    #define AC49X_NUMBER_OF_DEVICES		    (NUMBER_OF_DEVICES_PER_DSP*1)
	#define AC49XIF_BASE_ADDRESS	        0x00000000				/* The base address of the HPI multiplexed mode */
	#define AC49XIF_HCNTL0_OFFSET	        0x00800000				/* The HCNTL0 address */
	#define AC49XIF_DEVICE_OFFSET           0x00000000				/* Since the devices (cores) are not continously mapped, */
	#define AC49X_DRIVRES_VERSION			AC49X_DRIVER_VERSION_540
	#define FIFO_USAGE_ENABLE__HOST				1
	#define FIFO_USAGE_ENABLE__NETWORK			1

#elif	AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC491xxx_AEB
	#define AC49X_DEVICE_TYPE		        AC491_DEVICE			/* The DSP device type */
	#define AC49X_ENDIAN_MODE		        LITTLE_ENDIAN			/* The Host endian mode */
	#define DSP_TO_HOST_ADDRESS_SHIFT       1						/* Addressed word size shift (DSP->Host) */
	#define NUMBER_OF_DEVICES_PER_DSP	    6 						/* Number of devices (DSP cores) available */
    #define AC49X_NUMBER_OF_DEVICES		    (NUMBER_OF_DEVICES_PER_DSP*1)
	#define AC49XIF_BASE_ADDRESS	        0x00000000				/* The base address of the HPI multiplexed mode */
	#define AC49XIF_DEVICE_OFFSET           0x00800000				/* The HCNTL0 address */
	#define AC49XIF_ENTRY_ADDRESS           0x00000100				/* Since the devices (cores) are not continously mapped */
	#define UTOPIA_ROUTING 
	#define AC49X_DRIVRES_VERSION			AC49X_DRIVER_VERSION_540

	#define FIFO_USAGE_ENABLE__HOST				1
	#define FIFO_USAGE_ENABLE__NETWORK			1
#elif	AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC494xxx_AEB
	#define AC49X_DEVICE_TYPE		        AC494_DEVICE			/* The DSP device type */

	#define AC49X_OS_CONFIG					AC49X_OS__WINDOWS  
	#ifndef AC49X_OS_CONFIG
		#error The selected Operationg System is not implemented, please set "AC49X_OS_CONFIG" to one of the supported options.
	#endif

	#if		AC49X_OS_CONFIG == AC49X_OS__VXWORKS
		#define AC49X_ENDIAN_MODE		    BIG_ENDIAN			/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__WINDOWS
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#endif	/* endif AC49X_OS_CONFIG types */

	#define DSP_TO_HOST_ADDRESS_SHIFT       1						/* Addressed word size shift (DSP->Host) */
	#define NUMBER_OF_DEVICES_PER_DSP	    1 						/* Number of devices (DSP cores) available */
    #define AC49X_NUMBER_OF_DEVICES		    (NUMBER_OF_DEVICES_PER_DSP*4)


	#define DSP_PHYSICAL_ADDRESS_OFFSET     0x04000000				/* translate address From Program download address to physical address*/
	#define DSP_VIRTUAL_ADDRESS_OFFSET      0xA0000000				/* translate from physical address to virtual address.*/

	#define EMIF_VIRTUAL_ADDRESS_OFFSET		0xA0000000				/* translate from physical address to virtual address.*/
	#define AC49X_SDRAM_SIZE				0x4000000				/* must be a multipile of 4M */
	#define CASH_SEG_B_ALIGNED				0x400000				/* 4M */
	#define	DSP_EXTERNAL_MEMORY_OFFSET		0x400000				/* 4M */
	#define DSP_PROGRAM_SIZE				0x80000
	#define EMIF_PHYSICAL_BASE_ADDRESS		0x14000000
	#define EMIF_MAX_ADRESS					(EMIF_PHYSICAL_BASE_ADDRESS + AC49X_SDRAM_SIZE)
	#define EMIF_4M_DSP_ALIGEND				(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED + EMIF_VIRTUAL_ADDRESS_OFFSET)
	#define CASH_SEG_B						(((EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED) >> 22 ) & 0x3FF)
	#define	EMIF_PHYSICAL_ADDRESS_OFFSET	(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED - DSP_EXTERNAL_MEMORY_OFFSET)  

	#define PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL 1
	#define AC49X_DRIVRES_VERSION			AC49X_DRIVER_VERSION_540


	#define FIFO_USAGE_ENABLE__HOST				0
	#define FIFO_USAGE_ENABLE__NETWORK			0

#elif	AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC495xxx_AEB
	#define AC49X_DEVICE_TYPE		        AC495_DEVICE			/* The DSP device type */

	#define AC49X_OS_CONFIG					AC49X_OS__WINDOWS  
	#ifndef AC49X_OS_CONFIG
		#error The selected Operationg System is not implemented, please set "AC49X_OS_CONFIG" to one of the supported options.
	#endif

	#if		AC49X_OS_CONFIG == AC49X_OS__VXWORKS
		#define AC49X_ENDIAN_MODE		    BIG_ENDIAN				/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__WINDOWS
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#endif	/* endif AC49X_EVM_OS_CONFIG types */

	#define DSP_TO_HOST_ADDRESS_SHIFT       1						/* Addressed word size shift (DSP->Host) */
	#define NUMBER_OF_DEVICES_PER_DSP	    1 						/* Number of devices (DSP cores) available */
    #define AC49X_NUMBER_OF_DEVICES		    (NUMBER_OF_DEVICES_PER_DSP*4)

	#define DSP_PHYSICAL_ADDRESS_OFFSET     0x04000000				/* translate address From Program download address to physical address*/
	#define DSP_VIRTUAL_ADDRESS_OFFSET      0xA0000000				/* translate from physical address to virtual address.*/

	#define EMIF_VIRTUAL_ADDRESS_OFFSET		0xA0000000				/* translate from physical address to virtual address.*/
	#define AC49X_SDRAM_SIZE				0x4000000				/* must be a multipile of 4M */
	#define CASH_SEG_B_ALIGNED				0x400000				/* 4M */
	#define	DSP_EXTERNAL_MEMORY_OFFSET		0x400000				/* 4M */
	#define DSP_PROGRAM_SIZE				0x80000
	#define EMIF_PHYSICAL_BASE_ADDRESS		0x14000000
	#define EMIF_MAX_ADRESS					(EMIF_PHYSICAL_BASE_ADDRESS + AC49X_SDRAM_SIZE)
	#define EMIF_4M_DSP_ALIGEND				(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED + EMIF_VIRTUAL_ADDRESS_OFFSET)
	#define CASH_SEG_B						(((EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED) >> 22 ) & 0x3FF)
	#define	EMIF_PHYSICAL_ADDRESS_OFFSET	(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED - DSP_EXTERNAL_MEMORY_OFFSET)  

	#define PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL 1
	#define AC49X_DRIVRES_VERSION			AC49X_DRIVER_VERSION_540
	#define FIFO_USAGE_ENABLE__HOST				0
	#define FIFO_USAGE_ENABLE__NETWORK			0

#elif	AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC496xxx_AEB
	#define AC49X_DEVICE_TYPE		        AC496_DEVICE			/* The DSP device type */

	#define AC49X_OS_CONFIG					AC49X_OS__WINDOWS  
	#ifndef AC49X_OS_CONFIG
		#error The selected Operationg System is not implemented, please set "AC49X_OS_CONFIG" to one of the supported options.
	#endif

	#if		AC49X_OS_CONFIG == AC49X_OS__VXWORKS
		#define AC49X_ENDIAN_MODE		    BIG_ENDIAN				/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__WINDOWS
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#endif	/* endif AC49X_EVM_OS_CONFIG types */

	#define DSP_TO_HOST_ADDRESS_SHIFT       1						/* Addressed word size shift (DSP->Host) */
	#define NUMBER_OF_DEVICES_PER_DSP	    1 						/* Number of devices (DSP cores) available */
    #define AC49X_NUMBER_OF_DEVICES		    (NUMBER_OF_DEVICES_PER_DSP*4)

	#define DSP_PHYSICAL_ADDRESS_OFFSET     0x04000000				/* translate address From Program download address to physical address*/
	#define DSP_VIRTUAL_ADDRESS_OFFSET      0xA0000000				/* translate from physical address to virtual address.*/

	#define EMIF_VIRTUAL_ADDRESS_OFFSET		0xA0000000				/* translate from physical address to virtual address.*/
	#define AC49X_SDRAM_SIZE				0x4000000				/* must be a multipile of 4M */
	#define CASH_SEG_B_ALIGNED				0x400000				/* 4M */
	#define	DSP_EXTERNAL_MEMORY_OFFSET		0x400000				/* 4M */
	#define DSP_PROGRAM_SIZE				0x80000
	#define EMIF_PHYSICAL_BASE_ADDRESS		0x14000000
	#define EMIF_MAX_ADRESS					(EMIF_PHYSICAL_BASE_ADDRESS + AC49X_SDRAM_SIZE)
	#define EMIF_4M_DSP_ALIGEND				(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED + EMIF_VIRTUAL_ADDRESS_OFFSET)
	#define CASH_SEG_B						(((EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED) >> 22 ) & 0x3FF)
	#define	EMIF_PHYSICAL_ADDRESS_OFFSET	(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED - DSP_EXTERNAL_MEMORY_OFFSET)  

	#define PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL 1
	#define AC49X_DRIVRES_VERSION			AC49X_DRIVER_VERSION_540
	#define FIFO_USAGE_ENABLE__HOST				0
	#define FIFO_USAGE_ENABLE__NETWORK			0

#elif	AC49X_CURRENT_CONFIG == AC49X_CONFIG_AC497xxx_AEB
	#define AC49X_DEVICE_TYPE		        AC497_DEVICE			/* The DSP device type */

	#define AC49X_OS_CONFIG					AC49X_OS__WINDOWS  
	#ifndef AC49X_OS_CONFIG
		#error The selected Operationg System is not implemented, please set "AC49X_OS_CONFIG" to one of the supported options.
	#endif

	#if		AC49X_OS_CONFIG == AC49X_OS__VXWORKS
		#define AC49X_ENDIAN_MODE		    BIG_ENDIAN				/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__WINDOWS
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#endif	/* endif AC49X_EVM_OS_CONFIG types */

	#define DSP_TO_HOST_ADDRESS_SHIFT       1						/* Addressed word size shift (DSP->Host) */
	#define NUMBER_OF_DEVICES_PER_DSP	    1 						/* Number of devices (DSP cores) available */
    #define AC49X_NUMBER_OF_DEVICES		    (NUMBER_OF_DEVICES_PER_DSP*4)

	#define DSP_PHYSICAL_ADDRESS_OFFSET     0x04000000				/* translate address From Program download address to physical address*/
	#define DSP_VIRTUAL_ADDRESS_OFFSET      0xA0000000				/* translate from physical address to virtual address.*/

	#define EMIF_VIRTUAL_ADDRESS_OFFSET		0xA0000000				/* translate from physical address to virtual address.*/
	#define AC49X_SDRAM_SIZE				0x4000000				/* must be a multipile of 4M */
	#define CASH_SEG_B_ALIGNED				0x400000				/* 4M */
	#define	DSP_EXTERNAL_MEMORY_OFFSET		0x400000				/* 4M */
	#define DSP_PROGRAM_SIZE				0x80000
	#define EMIF_PHYSICAL_BASE_ADDRESS		0x14000000
	#define EMIF_MAX_ADRESS					(EMIF_PHYSICAL_BASE_ADDRESS + AC49X_SDRAM_SIZE)
	#define EMIF_4M_DSP_ALIGEND				(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED + EMIF_VIRTUAL_ADDRESS_OFFSET)
	#define CASH_SEG_B						(((EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED) >> 22 ) & 0x3FF)
	#define	EMIF_PHYSICAL_ADDRESS_OFFSET	(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED - DSP_EXTERNAL_MEMORY_OFFSET)  

	#define PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL 1
	#define AC49X_DRIVRES_VERSION			AC49X_DRIVER_VERSION_540

#elif	AC49X_CURRENT_CONFIG == ACLX_CONFIG_ACLX5280_A
	#define AC49X_DEVICE_TYPE		        ACLX5280_DEVICE			/* The DSP device type */
	#define AC49X_HPI_TYPE				AC49X_HPI_NONE

	#define AC49X_OS_CONFIG					AC49X_OS__LINUX_MONTAVISTA
	#ifndef AC49X_OS_CONFIG
		#error The selected Operationg System is not implemented, please set "AC49X_OS_CONFIG" to one of the supported options.
	#endif

	#if		AC49X_OS_CONFIG == AC49X_OS__VXWORKS
		#define AC49X_ENDIAN_MODE		    BIG_ENDIAN			/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__LINUX_MONTAVISTA
		#define AC49X_ENDIAN_MODE		    BIG_ENDIAN			/* The Host endian mode */
	#elif	AC49X_OS_CONFIG == AC49X_OS__WINDOWS
		#define AC49X_ENDIAN_MODE		    LITTLE_ENDIAN			/* The Host endian mode */
	#endif	/* endif AC49X_OS_CONFIG types */

	#define DSP_TO_HOST_ADDRESS_SHIFT       1						/* Addressed word size shift (DSP->Host) */
	#define NUMBER_OF_DEVICES_PER_DSP	    1 						/* Number of devices (DSP cores) available */

	#define DSP_PHYSICAL_ADDRESS_OFFSET     0x04000000				/* translate address From Program download address to physical address*/
	#define DSP_VIRTUAL_ADDRESS_OFFSET      0xA0000000				/* translate from physical address to virtual address.*/

	#define EMIF_VIRTUAL_ADDRESS_OFFSET		0xA0000000				/* translate from physical address to virtual address.*/
	#define AC49X_SDRAM_SIZE				0x4000000				/* must be a multipile of 4M */
	#define CASH_SEG_B_ALIGNED				0x400000				/* 4M */
	#define	DSP_EXTERNAL_MEMORY_OFFSET		0x400000				/* 4M */
	#define DSP_PROGRAM_SIZE				0x80000
	#define EMIF_PHYSICAL_BASE_ADDRESS		0x14000000
	#define EMIF_MAX_ADRESS					(EMIF_PHYSICAL_BASE_ADDRESS + AC49X_SDRAM_SIZE)
	#define EMIF_4M_DSP_ALIGEND				(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED + EMIF_VIRTUAL_ADDRESS_OFFSET)
	#define CASH_SEG_B						(((EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED) >> 22 ) & 0x3FF)
	#define	EMIF_PHYSICAL_ADDRESS_OFFSET	(EMIF_MAX_ADRESS - CASH_SEG_B_ALIGNED - DSP_EXTERNAL_MEMORY_OFFSET)

	#define PCM_CLK_AND_FRAME_SYNC_SOURCE_EXTERNAL 1
	#define AC49X_DRIVRES_VERSION			AC49X_DRIVER_VERSION_210
	#define FIFO_USAGE_ENABLE__NETWORK			0
	#define FIFO_USAGE_ENABLE__HOST				0
	#define NUMBER_OF_DEVICES_PER_DSP	1						/* Number of devices (DSP cores) available */
    #define AC49X_NUMBER_OF_DEVICES		    (1)

#endif /*~~AC49X_CURRENT_CONFIG*/

/*******************************************/
/*******************************************/
/**  Miscellaneous User Configurations    **/
/*******************************************/
/*******************************************/


/* The max number of Ac49xUserDef_DelayForDeviceResponse iteration allowed */
/* WaitForDeviceResponse is a user-defined function that performs a */
/* constant delay (e.g. by calling sleep(1)). */
/* A value of 10000 is equal to 10 sec if Ac49xUserDef_DelayForDeviceResponse waits for 1msec */

/* The user-implemented Ac49xUserDef_DelayForDeviceResponse delay time in micro sec : */
#define DELAY_FOR_DEVICE_RESPONSE	200

/* The default maximum delay time (before declaring an error) in msec: */
/* Note that the actual time will be longer because of the device polling */
/* time which is added to the user delay time at each iteration. */
#define MAX_DELAY_TIME			    5000

/* Set CHECK_COMMAND_ECHO to enable checking of the echoed host-to-dsp */
/* command. The check is done by comparing the echo to the original command. */
/* Set to 0 to diable the check. */
#define CHECK_COMMAND_ECHO		    1

/* Debugging control - set to 1 to  enable Tx/Rx debug option (depend on 'ac49xDebugMode'). */
/* Debugging control - set to 0 to disable Tx/Rx debug option (save processing time).       */
#define ALLOW_DEBUG_MODE					0

#define PACKET_RECORDING_ENABLE				0

#define CRASH_TEST_ENABLE					0

#define FIFO_CELL_SIZE__HOST				44 /*This value must be Greater or equal to HostCellSize at Tac49xDeviceControlRegisterAttr     */
#define FIFO_CELL_SIZE__NETWORK				56 /*This value must be Greater or equal to NetworkCellSize at Tac49xDeviceControlRegisterAttr  */

#define FIFO_SIZE__HOST                     ((U16)(1<<10)) /* have to be power of 2 (range: 0 to 15)*/  
#define FIFO_SIZE__NETWORK                  ((U16)(1<<10)) /* have to be power of 2 (range: 0 to 15)*/  

#endif /* ifndef AC49XCFG_H */
