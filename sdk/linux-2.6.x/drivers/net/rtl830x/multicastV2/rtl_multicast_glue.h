/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 * 
 * $Revision: 1.1 $ 
 * $Date: 2011/03/03 08:36:38 $
 *
 * Purpose : IGMP snooping API Glue File
 *
 * Feature : 
 *
 */

#ifndef __RTL_MULTICAST_GLUE_H__
#define __RTL_MULTICAST_GLUE_H__

#include <rtl_multicast_types.h>


/* supported chip: CONFIG_RTL8366S/CONFIG_RTL8306SDM/CONFIG_RTL8306E */
/*#define CONFIG_RTL8306SDM 1 */
/*#define CONFIG_RTL8306E   1 */
/*#define CONFIG_RTL8366S   1 */

#ifdef RTL865X_OVER_LINUX
  #include <linux/config.h>
  #if defined(CONFIG_RTL8306SDM) || defined(CONFIG_RTL8306E)
    #define MII_PORT_MASK       0x20
  #elif defined(CONFIG_RTL8366S)
    #include "../rtl8366s/rtl8366s_cpu.h"
  #endif
#endif

#if defined(RTL_MULTICAST_SNOOPING_TEST)
extern uint32 cpuPortNumer;
#endif


#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))

#ifdef _LITTLE_ENDIAN

#ifndef ntohs
    #define ntohs(x)   (swaps16(x))
#endif 

#ifndef ntohl
    #define ntohl(x)   (swapl32(x))
#endif

#ifndef htons
    #define htons(x)   (swaps16(x))
#endif

#ifndef htonl
    #define htonl(x)   (swapl32(x))
#endif

#else

#ifndef ntohs
    #define ntohs(x)    (x)
#endif 

#ifndef ntohl
    #define ntohl(x)    (x)
#endif

#ifndef htons
    #define htons(x)    (x)
#endif

#ifndef htonl
    #define htonl(x)    (x)
#endif

#endif

#ifdef __KERNEL__
    #define rtl_gluePrintf printk
#else
    #define rtl_gluePrintf printf 
#endif


#ifndef RTL_MULTICAST_SNOOPING_TEST
  /* Function Name:
   *      rtl_igmpGlueMutexLock
   * Description:
   *       Glue function for system mutex lock.
   * Input:
   *      none
   * Output:
   *      none
   * Return:
   *      0     -    always return 0
   * Note:
   *      User should modify this function to glue different OS.
   */
  extern int32 rtl_glueMutexLock(void);

  /* Function Name:
   *      rtl_igmpGlueMutexUnlock
   * Description:
   *       Glue function for system mutex unlock.
   * Input:
   *      none
   * Output:
   *      none
   * Return:
   *      0     -    always return 0
   * Note:
   *      User should modify this function to glue different OS.
   */
  extern int32 rtl_glueMutexUnlock(void);
#else
extern int testdrvMutex;

#define rtl_glueMutexLock()\
    do { \
        testdrvMutex ++;\
    } while (0)
    
#define rtl_glueMutexUnlock()\
    do{\
        testdrvMutex --;\
        if (testdrvMutex < 0)\
        {\
        printf("Error: Driver Mutex Lock/Unlcok is not balance");\
        }\
    }while (0)


#endif


/* Function Name:
 *      rtl_igmpGlueMalloc
 * Description:
 *       Glue function for memory allocation.
 * Input:
 *      NBYTES  -   Specifies the number of memory bytes to be allocated
 * Output:
 *      none
 * Return:
 *      void*    -   The memory pointer after allocation.
 * Note:
 *      User should modify this function according to different OS.
 */
extern void *rtl_glueMalloc(uint32 NBYTES);

/* Function Name:
 *      rtl_igmpGlueFree
 * Description:
 *       Glue function for memory free
 * Input:
 *      APTR  -   Specifies the memory buffer to be freed
 * Output:
 *      none
 * Return:
 *      none
 * Note:
 *      User should modify this function according to different OS.
 */
extern void rtl_glueFree(void *memblock);

/* Function Name:
 *      rtl_glueGetSrcPortMask
 * Description:
 *      Glue function for getting source port mask of the mac frame
 * Input:
 *      pMacFrame   -   pointer of the mac frame
 * Output:
 *      none
 * Return:
 *      NON_PORT_MASK   - source port mask is not found
 * Note:
 *      User should modify this function according to different NIC driver.
 */
extern uint8 rtl_glueGetSrcPortMask(uint8 *pMacFrame);


#ifdef RTL_MULTICAST_SNOOPING_TEST
extern void   rtl_glueInit(void);
extern int32  Software_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask);
extern int32  Software_deleteMacAddress(uint8 *macAddr);
extern uint8  Software_forward(uint8 *macFrame);
#else

/* Function Name:
 *      rtl_igmpGlueNicSend
 * Description:
 *       Glue function for sending MAC frame by NIC driver
 * Input:
 *      pMacFrame       -   Specifies the MAC frame position
 *      macFrameLen  -   Specifies the length of the MAC frame
 * Output:
 *      none
 * Return:
 *      none
 * Note:
 *      User should modify this function according to different NIC driver.
 *      For RTL8366S: The minimum frame size with CPU tag accepted by RTL8366S is 68 bytes.
 *      NIC driver has to padding the frame if its size is less than 68 bytes or the frame will be
 *      dropped by RTL8366S. 68 = data(60) + CPU tag(4) + CRC checksum(4).
 */
extern void rtl_glueNicSend(uint8 *macFrame, uint32 macFrameLen);

/* Function Name:
 *      rtl_multicastSnoopingV2TimeUpdate
 * Description:
 *      Glue function for multicast time update
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      none
 * Note:
 *      User should modify this function according to different NIC driver.
 */
extern void rtl_multicastSnoopingV2TimeUpdate(void);
#endif

#endif /* __RTL_MULTICAST_GLUE_H__ */

