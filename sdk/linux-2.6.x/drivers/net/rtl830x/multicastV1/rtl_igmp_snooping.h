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
* Purpose : IGMP snooping API support IGMPv1/v2/v3
*
* Feature : 
*
*/

#ifndef __RTL_IGMP_SNOOPING_H__
#define __RTL_IGMP_SNOOPING_H__

#include <rtl_igmp_glue.h>

/* IGMP parameters; */
struct rtl_igmpSnoopingParameter
{   
    uint16 groupMemberAgingTime;                  /* expired time of group membership, default: 260 seconds*/
    uint16 lastMemberAgingTime;              /* IGMPv2 fast leave latency*/

    uint16 querierPresentInterval;                   

    uint16 dvmrpRouterAgingTime;                  /*DVMRP multicast router aging time*/
    uint16 mospfRouterAgingTime;                           /*MOSPF multicast router aging time*/
    uint16 pimDmRouterAgingTime;                          /*PIM-DM multicast router aging time*/
};


/******************************************************
    Function called in the system initialization 
******************************************************/

/* Function Name:
 *      rtl_initIgmpSnoopingV1
 * Description:
 *      IGMP snooping initialization
 * Input:
 *      mCastDataToCpu        - TRUE to enable multicast data forwarding to cpu, FALSE to disable it.
 *      delPortMaskRevLeave  - TRUE to delete group member port which receiving igmp leave message, FALSE to lower group member port timer 
 *      maxGroupNum     -  Specifies maximum group number to be supported in Igmp snooping module, default is 100
 *      hashTableSize     -  Specifies hash table size, default is 32.
 *      currentSystemTime  - Specifies the current system time for IGMP snooping timer list initialization(unit: seconds).
 * Output:
 *      none
 * Return:
 *      SUCCESS                   -  Igmp snooping  initialization success
 *      FAILED                     -  Igmp snooping  initialization fail
 * Note:
 *     If realtekEtherType or  maxGroupNum or hashTableSize has been set to 0, 
 *     it means not to change this value.
 */
extern int32 rtl_initIgmpSnoopingV1( uint32 mCastDataToCpu, uint32 igmpProxyAvailable,  uint32 maxGroupNum, uint32 hashTableSize, uint32 currentSystemTime);


/* Function Name:
 *      rtl_setIgmpSnoopingV1Parameter
 * Description:
 *      Config IGMP snooping parameters.
 * Input:
 *      igmpSnoopingParas        - Specifies IGMP snooping time parameters
 *      pGatewayMac    -      Specifies gateway mac address
 *      gatewayIp      -      Specifies gateway ip address
 * Output:
 *      none
 * Return:
 *      SUCCESS           -  success
 * Note:
 *     (1)if the gatewayMac or gatewayIp has been set, the unicast igmp packet whose destination MAC equals to
 *          gatewayMac and destination IP equals to gatewayIp, will be accepted too.
 *     (2)If the member of igmpSnoopingParas has been set to 0, it means not to change its value.
 */
extern int32 rtl_setIgmpSnoopingV1Parameter(struct rtl_igmpSnoopingParameter igmpSnoopingParameters, uint8* pGatewayMac, uint32 gatewayIp);

/* Function Name:
 *      rtl_igmpSnoopingV1Receive
 * Description:
 *      IGMP packet snooping function.
 * Input:
 *      pMacFrame        - Pointer of MAC frame
 *      pPort             -    Pointer of source port
 * Output:
 *      none
 * Return:
 *      none
 * Note:
 *      (1)CPU should have the ability to receive/transmit mac frame with cpu tag attached.
 *      (2)If pPort == NULL, the source portof this igmp frame is retreaved by parsing the frame; 
 *          otherwise the source port is decided by pPort.
 */
extern void rtl_igmpSnoopingV1Receive(uint8 * pMacFrame , uint8 *pPort);

/* Function Name:
 *      rtl_maintainIgmpSnoopingV1TimerList
 * Description:
 *      IGMP snooping timer list maintenance function.
 * Input:
 *      currentSystemTime        - The current system time (unit: seconds).
 * Output:
 *      none
 * Return:
 *      SUCCESS            -  Always return SUCCESS
 * Note:
 *     This function should be called once a second to maintain IGMP timer list.
 */
extern int32 rtl_maintainIgmpSnoopingV1TimerList(uint32 currentSystemTime);

/* Function Name:
 *     rtl_igmpSnoopingV1Send
 * Description:
 *      IGMP snooping packet forwarding function.
 * Input:
 *      pMacFrame               -    The pointer of MAC frame to be transmitted.
 *      priorFreeBytes           -    Free space ahead MAC Frame.
 *      posteriorFreeSpace     -    Free space after MAC Frame.
 *      ppNewMacFrame        -    New pointer of MAC frame to be forwarded(maybe insert cpu tag or not).
 * Output:
 *      none
 * Return:
 *      SUCCESS        -   This packet can be handled by igmp snooping module
 *      FAILED           -   Not enough space to insert cpu tag, or pppoe header present, or ip header not found
 * Note:
 *     This function is called when forward a multicast MAC frame. Since there is an aggregator issues 
 *     when multicast IP maps to MAC address, besides, RTL8305SD may also run out of multicast entry. 
 *     CPU has to solve these problems, and give appropriate direction when forward a multicast data.
 */
extern int32 rtl_igmpSnoopingV1Send(uint8 *pMacFrame, uint32 priorFreeBytes,  uint32 posteriorFreeSpace, uint8 **ppNewMacFrame);

/*
@func int32 | rtl_disableIgmpSnoopingV1 |   IGMP snooping invalidation function
@parm void  | void
@rvalue SUCCESS | IGMP snooping module is disabled.
@rvalue FAILED  | IGMP snooping invalidation function is failed.
@comm 
Disable IGMP snooping module.
*/
extern int32 rtl_disableIgmpSnoopingV1(void);

#ifdef RTL_CPU_HW_FWD
/* Function Name:
 *      rtl_checkGroupStatus
 * Description:
 *       To check if a group address has been set into lookup table, or an aggregator address.
 * Input:
 *      groupAddress        - Specifies the group address to be checked
 *      pLutFlag                 - TRUE: if groupAddress has been set into lookup table.
 *      pAggregatorFlag      - TRUE: if groupAddress is an aggregator address.
 * Output:
 *      none
 * Return:
 *      SUCCESS           -  groupAddress has been found.
 *      FAILED              -  p groupAddress is not found.
 * Note:
 *     This function is enable when define the RTL_CPU_HW_FWD macro.
 *     It's used to determine whether a group address can be set into CPU acceleration hardware 
 */
extern int32 rtl_checkGroupStatus(uint32 groupAddress, uint32 *pLutFlag, uint32 *pAggregatorFlag);

#endif

#endif /* __RTL_IGMP_SNOOPING_H__ */

