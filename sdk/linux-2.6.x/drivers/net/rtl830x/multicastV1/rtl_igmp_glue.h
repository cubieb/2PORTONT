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
* $Date: 2011/03/03 08:36:32 $
*
* Purpose : IGMP snooping cleshell glue file
*
* Feature : 
*
*/

/* supported chip: CONFIG_RTL8366S/CONFIG_RTL8306SDM/CONFIG_RTL8306E */
/*#define CONFIG_RTL8306SDM 1 */
/*#define CONFIG_RTL8306E   1 */
/*#define CONFIG_RTL8366S   1 */

#ifndef __RTL_IGMP_GLUE_H__
#define __RTL_IGMP_GLUE_H__

#include <rtl_igmp_types.h>

#ifdef RTL_IGMP_SNOOPING_TEST
  #include <stdio.h>
  #if defined(CONFIG_RTL8306E)
    #include <rtl8306e_types.h>
    #include <rtk_api.h>
  #endif
  uint32 cpuPortNumer;
#else
  #define IGMP_DEBUG    /*should be removed when release code*/
  #define RTL8651B_SDK  /*should be removed when release code*/
  #define  RTL_CPU_HW_FWD /*should be removed when release code*/
#endif

#define MII_PORT_MASK 0x20 /*should be modified by user*/

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
  #define rtl_igmpGluePrintf printk
#else
  #define rtl_igmpGluePrintf printf 
#endif


#ifndef RTL_IGMP_SNOOPING_TEST

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
  extern int32 rtl_igmpGlueMutexLock(void);

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
    extern int32 rtl_igmpGlueMutexUnlock(void);

#else
  extern int testdrvMutex;
  
  #define rtl_igmpGlueMutexLock()\
    do { \
        testdrvMutex ++;\
    } while (0)
    
  #define rtl_igmpGlueMutexUnlock()\
    do {\
        testdrvMutex --;\
        if (testdrvMutex < 0)\
        {\
            rtl_igmpGluePrintf("%s (%d) Error: Driver Mutex Lock/Unlcok is not balance (%d).\n", __FUNCTION__, __LINE__, testdrvMutex);\
        }\
    } while (0)

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
extern void *rtl_igmpGlueMalloc(uint32 NBYTES);

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
extern void rtl_igmpGlueFree(void *APTR);

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

#ifndef RTL_IGMP_SNOOPING_TEST

/* Function Name:
 *      rtl_igmpGlueNicSend
 * Description:
 *       Glue function for MAC frame send
 * Input:
 *      pMacFrame       -   Specifies the MAC frame position
 *      macFrameLen    -  Specifies the length of the MAC frame
 * Output:
 *      none
 * Return:
 *      none
 * Note:
 *      User should modify this function according to different NIC driver.
 */
extern void rtl_igmpGlueNicSend(uint8 *pMacFrame, uint32 macFrameLen);

/* Function Name:
 *      rtl_igmpGlueNicSend
 * Description:
 *       Glue function for MAC frame send
 * Input:
 *      pMacFrame       -   Specifies the MAC frame position
 *      macFrameLen  -   Specifies the length of the MAC frame
 * Output:
 *      none
 * Return:
 *      none
 * Note:
 *      User should modify this function according to different NIC driver.
 */
extern void rtl_multicastSnoopingV1TimeUpdate(void);

#endif

#ifdef RTL_CPU_HW_FWD
/* Function Name:
 *      tl_igmpDisCpuHwFwd
 * Description:
 *      Glue function for getting source port mask of the mac frame
 * Input:
 *      groupAddress  -   Specifies the group address to be stopped.
 * Output:
 *      none
 * Return:
 *      one
 * Note:
 *      User should modify this function according to his own system.
 */
extern void rtl_igmpDisCpuHwFwd(uint32 groupAddress);
#endif

#ifdef RTL_IGMP_SNOOPING_TEST
  extern void   rtl_igmpGlueInit(void);
  #if defined(CONFIG_RTL8306E)
    extern int32  rtl8306e_l2_multicastMac_add(uint8 *macAddr,uint32 isAuth, uint32 portMask, uint32 *entryaddr);
    extern int32  rtl8306e_l2_mac_del(uint8 * macAddr, uint32 *entryaddr);
    extern uint8  rtl8306_forward(uint8* macFrame);
    extern rtk_api_ret_t rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t portmask);
    extern rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid);

  #elif defined(CONFIG_RTL8306SDM)
    extern int32  rtl8306_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask, uint32 *entryaddr);
    extern int32  rtl8306_deleteMacAddress(uint8 * macAddr, uint32 *entryaddr);
    extern uint8  rtl8306_forward(uint8* macFrame);
  #endif
#endif

#endif /* __RTL_IGMP_GLUE_H__ */

