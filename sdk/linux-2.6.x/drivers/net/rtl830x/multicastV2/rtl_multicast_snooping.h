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
 * Purpose : Multicast Snooping Function fully support IGMPV1/V2/V3 and MLDV1/V2
 *
 * Feature : 
 *
 */

#ifndef __RTL_MULTICAST_SNOOPING_H__
#define __RTL_MULTICAST_SNOOPING_H__

#include <rtl_multicast_types.h>

#define DROP_PACKET     0
#define FORWARD_PACKET  1
#define ERROR_NO_SPACE  -1

/* multicast configuration*/
struct rtl_multicastConfig
{
    uint32 enableSourceList;
    uint32 maxGroupNum;
    uint32 maxSourceNum;
    uint32 hashTableSize;

    uint32 groupMemberAgingTime;                
    uint32 lastMemberAgingTime;         
    uint32 querierPresentInterval;                   

    uint32 dvmrpRouterAgingTime;            
    uint32 mospfRouterAgingTime;                     
    uint32 pimRouterAgingTime;     

};

struct rtl_mCastTimerParameters
{
    uint32 groupMemberAgingTime;              
    uint32 lastMemberAgingTime;            
    uint32 querierPresentInterval;                   

    uint32 dvmrpRouterAgingTime;                  /*DVMRP multicast router aging time*/
    uint32 mospfRouterAgingTime;                           /*MOSPF multicast router aging time*/
    uint32 pimRouterAgingTime;                          /*PIM-DM multicast router aging time*/

};

extern uint8 rtl_gatewayMac[4][6];
extern uint32 rtl_multicastStatus;

/******************************************************
    Function called in the system initialization 
******************************************************/

/* Function Name:
 *      rtl_initMulticastSnooping
 * Description:
 *      Multicast snooping initialization
 * Input:
 *      multicastConfiguration     - Specifies Multicast snooping parameters.
 *      currentSystemTime       -  TSpecifies the current system time for igmp snooping timer list initialization(unit: seconds)
 *      delPortMaskRevLeave    -  TRUE to delete group member port while receiving a leave report or TO_IN{} records.  FALSE to lower group member port timer.
 * Output:
 *      none
 * Return:
 *      SUCCESS                   -  Multicast snooping initialization success
 *      FAILED                     -  Multicast snooping initialization fail
 * Note:
 *     If the members of  multicastConfiguration have been set to 0, it means to use the default values.
 */
extern int32 rtl_initMulticastSnooping(struct rtl_multicastConfig multicastConfiguration, uint32 currentSystemTime, uint32 cpuAsRouter);

/* Function Name:
 *      rtl_setMulticastParameters
 * Description:
 *      Config multicast snooping parameters
 * Input:
 *      pGatewayMac            -  Specifies gateway mac address.
 *      gatewayIpv4Addr       -  Specifies gateway ipv4 address
 *      pGatewayIpv6Addr     -  Specifies gateway ipv6 address
 * Output:
 *      none
 * Return:
 *      none
 * Note:
 *     (1)if the gatewayMac or gatewayIpv4Addr or gatewayIpv6Addr has been set, the unicast IGMP or MLD packet
 *         whose destination MAC equals to gatewayMac and destination IP equals to gatewayIpv4Addr or gatewayIpv6Addrwill be accepted too.
 *     (2)If the member of mCastTimerParameters has been set to 0, it means to use the default value.
 */
extern void rtl_setMulticastParameters(struct rtl_mCastTimerParameters mCastTimerParameters, uint8 *pGatewayMac, uint32 gatewayIpv4Addr, uint32 *pGatewayIpv6Addr);

/* Function Name:
 *      rtl_multicastSnoopingReceive
 * Description:
 *      IGMP or MLD packet snooping function
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
extern void rtl_multicastSnoopingReceive(uint8 * macFrame,  uint8 *port);

/* Function Name:
 *      rtl_maintainMulticastSnoopingTimerList
 * Description:
 *      Multicast snooping timer list maintenance function
 * Input:
 *      currentSystemTime       - The current system time (unit: seconds)
 * Output:
 *      none
 * Return:
 *      SUCCESS                  -  Always return SUCCESS
 * Note:
 *      This function should be called once a second to maintain multicast timer list.
 */
extern int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime);

/* Function Name:
 *      rtl_multicastSnoopingSend
 * Description:
 *      Multicast snooping packet forwarding function
 * Input:
 *      pMacFrame              -  The pointer of MAC frame to be transmitted
 *      priorFreeBytes         -  Free space ahead MAC Frame
 *      posteriorFreeSpace   -  Free space after MAC Frame
 *      ppNewMacFrame      -  New pointer of MAC frame to be forwarded(maybe insert cpu tag or not).
 * Output:
 *      none
 * Return:
 *      FORWARD_PACKET   -  This packet can be forwarded by NIC
 *      DROP_PACKET         -   This packet should be dropped
 *      ERROR_NO_SPACE   -    There is no enough space to insert CPU tag
 * Note:
 *      This function is called when forward a multicast MAC frame. Since there is an aggregator issues 
 *      when multicast IP maps to MAC address, besides, RTL8306SDM may also run out of multicast entry. 
 *      CPU has to solve these problems, and give appropriate direction when forward a multicast data.
 */
extern int32 rtl_multicastSnoopingSend(uint8 *pMacFrame, uint32 priorFreeBytes,  uint32 posteriorFreeSpace, uint8  **ppNewMacFrame);

/* Function Name:
 *      rtl_disableMulticastSnooping 
 * Description:
 *      Multicast snooping invalidation function
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS    -  Multicast snooping module is disabled.
 *      FAILED       -  Multicast snooping invalidation function is failed
 * Note:
 *      This function is uesed to disable multicast snooping module.
 */
extern int32 rtl_disableMulticastSnooping(void);

#endif /* __RTL_MULTICAST_SNOOPING_H__ */

