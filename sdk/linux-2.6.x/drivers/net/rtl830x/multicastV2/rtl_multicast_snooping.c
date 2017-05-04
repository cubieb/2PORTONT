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

#include <rtl_multicast_snooping.h>
#include <rtl_multicast_snooping_local.h>
#include <rtl_multicast_glue.h>

#if defined(RTL_MULTICAST_SNOOPING_TEST) 
  #include<string.h>
  #include<stdio.h>
#endif

#if defined(CONFIG_RTL8306E)
  #include <rtl8306e_asicdrv.h>
  #include <rtk_api.h>
  #include <rtk_api_ext.h>

#elif defined(CONFIG_RTL8306SDM)
  #include <Rtl8306_Driver_s.h>
  #include <Rtl8306_AsicDrv.h>

#elif defined(CONFIG_RTL8366S)
  #include <rtl8366s_asicdrv.h>
  #include <rtl8366s_api.h>
#endif

#if defined(RTL_MULTICAST_SNOOPING_TEST) 
  #ifdef RTL8306_TBLBAK
    extern rtl8306_ConfigBakPara_t rtl8306_TblBak;
  #endif
#endif

/*#define IGMP_SNOOPING_DEBUG*/
     
/*system settings*/
static uint8 rtl_cpuPortMask;
static uint16 rtl_etherType;
static struct rtl_mCastTimerParameters rtl_mCastTimerParas;  /*IGMP snooping parameters */
static struct rtl_multicastRouters rtl_ipv4MulticastRouters;
static struct rtl_multicastRouters rtl_ipv6MulticastRouters;
static uint32 rtl_enableSourceList;
static uint32 rtl_delPortMaskRevLeave;   

#if defined(RTL_MULTICAST_SNOOPING_TEST) 
uint32 cpuPortNumer;
#endif

/*gateway infomation*/
uint8 rtl_gatewayMac[4][6] = {{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0}};
static uint32 rtl_gatewayIpv4Addr[4] = {0,0,0,0};
static uint32 rtl_gatewayIpv6Addr[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
uint32 rtl_multicastStatus=DISABLE;

/*global system resources declaration*/
static uint32 rtl_totalMaxGroupCnt;    /*maximum total group entry count,  default is 100*/
static uint32 rtl_totalMaxSourceCnt;   /*maximum total group entry count,  default is 3000*/
void *rtl_groupMemory = NULL;
void *rtl_sourceMemory = NULL;
static struct rtl_groupEntry *rtl_groupEntryPool = NULL;
static struct rtl_sourceEntry *rtl_sourceEntryPool = NULL;

/*the system up time*/
static uint32 rtl_startTime;
static uint32 rtl_previousSysTime;
static uint32 rtl_sysUpSeconds;       

/*hash table definition*/
static struct rtl_groupEntry **rtl_ipv4HashTable = NULL;
static struct rtl_groupEntry **rtl_ipv6HashTable = NULL;
static uint32 rtl_hashTableSize = 0;
static uint32 rtl_hashMask = 0;


/*******************************internal function declaration*****************************/


/**************************
    resource managment
**************************/
static struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize);
static struct rtl_groupEntry* rtl_allocateGroupEntry(void);
static void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) ;


static struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize);
static struct rtl_sourceEntry* rtl_allocateSourceEntry(void);
static void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr) ;


/**********************************Structure Maintenance*************************/

static struct rtl_groupEntry* rtl_searchGroupEntry(uint32 ipVersion,uint32 *multicastAddr);
static void rtl_linkGroupEntry(struct rtl_groupEntry* entryNode ,  struct rtl_groupEntry ** hashTable, uint32 hashIndex);
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* entryNode,  struct rtl_groupEntry ** hashTable, uint32 hashIndex);
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntryPtr);

static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_groupEntry *groupEntry);
static void rtl_linkSourceEntry(struct rtl_groupEntry *groupEntry,  struct rtl_sourceEntry* entryNode);
static void rtl_unlinkSourceEntry(struct rtl_groupEntry *groupEntry, struct rtl_sourceEntry* entryNode);
static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr);
static void rtl_deleteSourceEntry(struct rtl_groupEntry *groupEntry, struct rtl_sourceEntry* sourceEntry);

static int32 rtl_checkPortMask(uint8 pktPortMask);
static uint8 rtl_mapPortMaskToPortNum(uint8 pormask);
static uint8 rtl_mapPortNumToPortMask(uint8 portNum);


static int32 rtl_mapMCastIPToMAC(uint32 ipVersion, uint32 *ipAddr, uint8 *macAddr );
static int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr);
static int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2);
static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2);
static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen);
static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr);

static uint8 rtl_getGroupFwdPortMask(struct rtl_groupEntry * groupEntry, uint32 sysTime);
static void rtl_checkSourceTimer(struct rtl_groupEntry * groupEntry , struct rtl_sourceEntry * sourceEntry);
static uint8 rtl_getSourceFwdPortMask(struct rtl_groupEntry * groupEntry,uint32 *sourceAddr, uint32 sysTime);
//static void rtl_checkGroupFilterTimer(struct rtl_groupEntry *groupEntry);             
static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry);

static uint8  rtl_getMulticastRouterPortMask(uint32 ipVersion, uint32 sysTime);

static void rtl_checkAggregator(uint32 ipVersion, uint32 *ipAddr,  uint8 *portMask, uint8* lookupTableFlag);
static void rtl_setAggregator(uint32 ipVersion, uint32 *ipAddr,  int32 flag);
static int32 rtl_setLookupTable(struct rtl_groupEntry *groupEntry);
static void rtl_updateAllGroupEntry(struct rtl_groupEntry** hashTable);

/*hash table operation*/
static int32 rtl_initHashTable(uint32 hashTableSize);


/************************************Pkt Process**********************************/
/*MAC frame analyze function*/
static void  rtl_parseMacFrame(uint8* MacFrame, struct rtl_macFrameInfo* macInfo);

/*Process Query Packet*/
static void rtl_snoopQuerier(uint32 ipVersion, uint8 pktPortMask);
static uint8 rtl_processQueries(uint32 ipVersion, uint8 pktPortMask, uint8* pktBuf, uint32 pktLen);
/*Process Report Packet*/
static uint8 rtl_processJoin(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf); // process join report packet 
static uint8 rtl_processLeave(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf); //process leave/done report packet
static int32 rtl_processIsInclude(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf); //process MODE_IS_INCLUDE report packet 
static int32 rtl_processIsExclude(uint32 ipVersion,uint8 pktPortMask, uint8 *pktBuf); //process MODE_IS_EXCLUDE report packet
static int32 rtl_processToInclude(uint32 ipVersion,  uint8 pktPortMask, uint8 *pktBuf); //process CHANGE_TO_INCLUDE_MODE report packet
static int32 rtl_processToExclude(uint32 ipVersion,uint8 pktPortMask, uint8 *pktBuf); //process CHANGE_TO_EXCLUDE_MODE report packet
static int32 rtl_processAllow(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf); //process ALLOW_NEW_SOURCES report packet 
static int32 rtl_processBlock(uint32 ipVersion,uint8 pktPortMask, uint8 *pktBuf); //process BLOCK_OLD_SOURCES report packet
static uint8 rtl_processIgmpv3Mldv2Reports(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf);

/*******************different protocol process function**********************************/
static uint8 rtl_processIgmpMld(uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint8 pktPortMask);
static uint8 rtl_processDvmrp(uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint8 pktPortMask);
static void rtl_processMospf(uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint8 pktPortMask);
static void rtl_processPim(uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint8 pktPortMask);


/*******************Switch chip dependent function**********************************/
static int32 rtl_getCPUPort(uint32 *port);
#if defined(CONFIG_RTL8306SDM)
static void  rtl_config8306(uint32 cpuPortNum, uint32 realtekEtherType);
static void  rtl8306_CPUTagXmit(uint8 *macFrame, struct rtl_macFrameInfo macFrameInfo, uint8 fwdPortMask);
#elif defined(CONFIG_RTL8306E)
static void  rtl_config8306e(uint32 cpuPortNum, uint32 realtekEtherType);
static void  rtl8306e_CPUTagXmit(uint8 *macFrame, struct rtl_macFrameInfo macFrameInfo, uint8 fwdPortMask);
#elif defined(CONFIG_RTL8366S)
static void  rtl_config8366S(uint32 cpuPortNum, uint32 realtekEtherType);
static void  rtl8366S_CPUTagXmit(uint8 *macFrame, struct rtl_macFrameInfo macFrameInfo, uint8 fwdPortMask);
#endif
static void  rtl_configSwitch(uint32 cpuPortNum, uint32 realtekEtherType);
static int32 rtl_addLUTUnicast(uint8 *mac, uint8 port);
static int32 rtl_addLUTMulticast(uint8 *mac, uint32 portmask);
static int32 rtl_delLUTMACAddress(uint8 *mac);
static void  rtl_CPUTagXmit(uint8 *macFrame, struct rtl_macFrameInfo macFrameInfo, uint8 fwdPortMask);
static void  rtl_GetPVID(uint8 port, uint32 *vid);
static void  rtl_disableSnooping(void);
static void  rtl_getCPUTagRXPort(uint8 *portNum, uint8 *portMask);

/************************************************
            Implementation
  ************************************************/
  
/**************************
    Initialize
**************************/
static struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize)
{  
    uint32 idx = 0;
    struct rtl_groupEntry *poolHead = NULL;
    struct rtl_groupEntry *entryPtr = NULL;
    rtl_glueMutexLock();    /* Lock resource */
    if(0 == poolSize)
    {
        goto out;
    }

    /* Allocate memory */
    poolHead = (struct rtl_groupEntry *)rtl_glueMalloc(sizeof(struct rtl_groupEntry) * rtl_totalMaxGroupCnt);
    rtl_groupMemory = (void *)poolHead;
    
    if (poolHead != NULL)
    {
        memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_groupEntry)));
        entryPtr = poolHead;

        /* link the whole group entry pool */
        for (idx = 0; idx < poolSize; idx++, entryPtr++)
        {   
            if(0 == idx)
            {
                entryPtr->previous = NULL;
                if(idx == (poolSize - 1))
                {
                    entryPtr->next = NULL;
                }
                else
                {
                    entryPtr->next = entryPtr + 1;
                }
            }
            else
            {
                entryPtr->previous = entryPtr - 1;
                if (idx == (poolSize - 1))
                {
                    entryPtr->next = NULL;
                }
                else
                {
                    entryPtr->next = entryPtr + 1;
                }
            }
        }
    }
    
out:

    rtl_glueMutexUnlock();  /* UnLock resource */
    return poolHead;
    
}



/**************************
    Resource Managment
**************************/

/* allocate a group entry pool from the group entry pool*/
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void)
{
    struct rtl_groupEntry *ret = NULL;

    rtl_glueMutexLock();    
    if (rtl_groupEntryPool != NULL)
    {
        ret = rtl_groupEntryPool;
        if(rtl_groupEntryPool->next != NULL)
        {
            rtl_groupEntryPool->next->previous = NULL;
        }
        rtl_groupEntryPool = rtl_groupEntryPool->next;
        memset(ret, 0, sizeof(struct rtl_groupEntry));
    }

    rtl_glueMutexUnlock();  

    return ret;
}

/* free a group entry and link it back to the group entry pool, default is link to the pool head*/
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) 
{
    if (!groupEntryPtr)
    {
        return;
    }

    rtl_glueMutexLock();    
    groupEntryPtr->next = rtl_groupEntryPool;
    if(rtl_groupEntryPool != NULL)
    {
        rtl_groupEntryPool->previous = groupEntryPtr;
    }
    rtl_groupEntryPool = groupEntryPtr;   
    rtl_glueMutexUnlock();  
}

static  struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize)
{

    uint32 idx = 0;
    struct rtl_sourceEntry *poolHead = NULL;
    struct rtl_sourceEntry *entryPtr = NULL;
    rtl_glueMutexLock();    /* Lock resource */
    if(0 == poolSize)
    {
        goto out;
    }

    /* Allocate memory */
    poolHead = (struct rtl_sourceEntry *)rtl_glueMalloc(sizeof(struct rtl_sourceEntry) * rtl_totalMaxSourceCnt);
    rtl_sourceMemory = (void *)poolHead;
    if (poolHead != NULL)
    {
        memset(poolHead, 0, (poolSize  * sizeof(struct rtl_sourceEntry)));
        entryPtr = poolHead;

        /* link the whole source entry pool */
        for (idx = 0; idx < poolSize; idx++, entryPtr++)
        {   
            if(0 == idx)
            {
                entryPtr->previous = NULL;
                if(idx == (poolSize - 1))
                {
                    entryPtr->next = NULL;
                }
                else
                {
                    entryPtr->next = entryPtr + 1;
                }
            }
            else
            {
                entryPtr->previous = entryPtr - 1;
                if (idx == (poolSize - 1))
                {
                    entryPtr->next = NULL;
                }
                else
                {
                    entryPtr->next = entryPtr + 1;
                }
            }
            
        }
    }
    
out:
    rtl_glueMutexUnlock();  /* UnLock resource */
    return poolHead;

}



/**************************
    Resource Managment
**************************/

/* allocate a group entry pool from the group entry pool*/
static struct rtl_sourceEntry* rtl_allocateSourceEntry(void)
{
    struct rtl_sourceEntry *ret = NULL;

    rtl_glueMutexLock();    
    if (rtl_sourceEntryPool != NULL)
    {   
        ret = rtl_sourceEntryPool;
        if(rtl_sourceEntryPool->next != NULL)
        {
            rtl_sourceEntryPool->next->previous = NULL;
        }
        rtl_sourceEntryPool = rtl_sourceEntryPool->next;
        memset(ret, 0, sizeof(struct rtl_sourceEntry));
    }

    rtl_glueMutexUnlock();  

    return ret;
}

/*free a group entry and link it back to the group entry pool, default is link to the pool head*/
static void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr) 
{
    if (!sourceEntryPtr)
    {
        return;
    }

    rtl_glueMutexLock();    
    sourceEntryPtr->next = rtl_sourceEntryPool;
    if(rtl_sourceEntryPool != NULL)
    {
        rtl_sourceEntryPool->previous = sourceEntryPtr;
    }

    rtl_sourceEntryPool = sourceEntryPtr; 

    rtl_glueMutexUnlock();  
}



/*********************************************
            Group list operation
 *********************************************/

/*       find a group address in a group list    */

struct rtl_groupEntry* rtl_searchGroupEntry(uint32 ipVersion,uint32 *multicastAddr)
{
    struct rtl_groupEntry* groupPtr = NULL;
    int32 hashIndex;
    
    if(IP_VERSION4 == ipVersion)
    {
        hashIndex = rtl_hashMask&multicastAddr[0];
        groupPtr = rtl_ipv4HashTable[hashIndex];
    }
    else
    {
        hashIndex = rtl_hashMask&multicastAddr[3];
        groupPtr = rtl_ipv6HashTable[hashIndex];
    }
    
    while (groupPtr != NULL)
    {   
        if(IP_VERSION4 == ipVersion)
        {
            if(multicastAddr[0] == groupPtr->groupAddr[0])
            {
                return groupPtr;
            }
        }

        if(IP_VERSION6 == ipVersion)
        {
            if(multicastAddr[0] == groupPtr->groupAddr[0])
            {
                if(multicastAddr[1] == groupPtr->groupAddr[1])
                {
                    if(multicastAddr[2] == groupPtr->groupAddr[2])
                    {
                        if(multicastAddr[3] == groupPtr->groupAddr[3])
                        {
                            return groupPtr;
                        }
                    }
                
                } 
            }
        }

        groupPtr = groupPtr->next;

    }

    return NULL;
}


/* link group Entry in the front of a group list */
static void  rtl_linkGroupEntry(struct rtl_groupEntry* groupEntry ,  struct rtl_groupEntry ** hashTable, uint32 hashIndex)
{
    rtl_glueMutexLock();/*Lock resource*/
    if(NULL == groupEntry)
    {
        return;
    }
    else
    {
        if(hashTable[hashIndex] != NULL)
        {
                hashTable[hashIndex]->previous = groupEntry;
        }
        groupEntry->next = hashTable[hashIndex];
        hashTable[hashIndex] = groupEntry;
        hashTable[hashIndex]->previous = NULL;

    }
    rtl_glueMutexUnlock();/*UnLock resource*/

}


/* unlink a group entry from group list */
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* groupEntry,  struct rtl_groupEntry ** hashTable, uint32 hashIndex)
{   
    if(NULL == groupEntry)
    {
        return;
    }
    else
    {
        rtl_glueMutexLock();  /* lock resource*/    
        /* unlink entry node*/
        if(groupEntry == hashTable[hashIndex]) /*unlink group list head*/
        {
            hashTable[hashIndex] = groupEntry->next;
            if(hashTable[hashIndex] != NULL)
            {
                hashTable[hashIndex]->previous = NULL;
            }

            groupEntry->previous = NULL;
            groupEntry->next = NULL;
        }
        else
        {
            if(groupEntry->previous != NULL)
            {
                groupEntry->previous->next = groupEntry->next;
            }

            if(groupEntry->next != NULL)
            {
                groupEntry->next->previous = groupEntry->previous;
            }
            groupEntry->previous = NULL;
            groupEntry->next = NULL;
        }

        rtl_glueMutexUnlock();/*UnLock resource*/
    }
}


/* clear the content of group entry */
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntry)
{
    rtl_glueMutexLock();
    if (NULL != groupEntry)
    {
        memset(groupEntry, 0, sizeof(struct rtl_groupEntry));
    }
    rtl_glueMutexUnlock();
}

static void rtl_deleteSourceList(struct rtl_groupEntry* groupEntry)
{
    struct rtl_sourceEntry *sourceEntry = groupEntry->sourceList;
    struct rtl_sourceEntry *nextSourceEntry = NULL;
    while(sourceEntry != NULL)
    {
        nextSourceEntry = sourceEntry->next;
        rtl_deleteSourceEntry(groupEntry,sourceEntry);
        sourceEntry = nextSourceEntry;
    }
}

static void rtl_deleteGroupEntry(struct rtl_groupEntry* groupEntry)
{   
    if(groupEntry != NULL)
    {
        if(IP_VERSION4 == groupEntry->ipVersion)
        {   
            rtl_deleteSourceList(groupEntry);
            rtl_unlinkGroupEntry(groupEntry, rtl_ipv4HashTable,(groupEntry->groupAddr[0] & rtl_hashMask));
            rtl_clearGroupEntry(groupEntry);
            rtl_freeGroupEntry(groupEntry);
        }

        if(IP_VERSION6 == groupEntry->ipVersion)
        {
            rtl_deleteSourceList(groupEntry);
            rtl_unlinkGroupEntry(groupEntry, rtl_ipv6HashTable,(groupEntry->groupAddr[3] & rtl_hashMask));
            rtl_clearGroupEntry(groupEntry);
            rtl_freeGroupEntry(groupEntry);
        }
    }
        
}

static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_groupEntry *groupEntry)
{
    struct rtl_sourceEntry *sourcePtr=groupEntry->sourceList;
    while(sourcePtr != NULL)
    {
        if(4 == ipVersion)
        {
            if(sourceAddr[0] == sourcePtr->sourceAddr[0])
            {
                return sourcePtr;
            }
        }
        else
        {
            if(sourceAddr[0] == sourcePtr->sourceAddr[0])
            {
                if(sourceAddr[1] == sourcePtr->sourceAddr[1])
                {
                    if(sourceAddr[2] == sourcePtr->sourceAddr[2])
                    {
                        if(sourceAddr[3] == sourcePtr->sourceAddr[3])
                        {
                            return sourcePtr;
                        }
                    }
                }
            }
        }
        sourcePtr = sourcePtr->next;
    }

    return NULL;
}

static int32 rtl_searchSourceAddr(uint32 ipVersion, uint32 *sourceAddr, uint32 *sourceArray, uint32 elementCount)
{
    uint32 i = 0;
    uint32 *srcPtr = sourceArray;

    for(i = 0; i < elementCount; i++)
    {
        if(IP_VERSION4 == ipVersion)
        {
            if(sourceAddr[0] == srcPtr[0])
            {
                return TRUE;
            }
            srcPtr++;
        }

        if(IP_VERSION6 == ipVersion)
        {
            if((sourceAddr[0] == srcPtr[0])&&\
                (sourceAddr[1] == srcPtr[1])&&\
                (sourceAddr[2] == srcPtr[2])&&\
                (sourceAddr[3] == srcPtr[3]))
            { 
                return TRUE;
            }
            
            srcPtr = srcPtr + 4;
        }
    }
    
    return FALSE;
}

static void rtl_linkSourceEntry(struct rtl_groupEntry *groupEntry,  struct rtl_sourceEntry* entryNode)
{
    if((NULL == entryNode) || (NULL == groupEntry))
    {
        return;
    }
    else
    {
        rtl_glueMutexLock();  /* lock resource*/    

        if(groupEntry->sourceList != NULL)
        {
            groupEntry->sourceList->previous = entryNode;
        }
        entryNode->next = groupEntry->sourceList;
        groupEntry->sourceList = entryNode;
        groupEntry->sourceList->previous = NULL;

        rtl_glueMutexUnlock();  /* lock resource*/  
    }
}

static void rtl_unlinkSourceEntry(struct rtl_groupEntry *groupEntry, struct rtl_sourceEntry* sourceEntry)
{
    if((NULL == sourceEntry) || (NULL == groupEntry))
    {
        return;
    }
    else
    {
        rtl_glueMutexLock();  /* lock resource*/    
        /* unlink entry node*/ 
        if(sourceEntry == groupEntry->sourceList) /*unlink group list head*/
        {

            groupEntry->sourceList = sourceEntry->next;
            if(groupEntry->sourceList != NULL)
            {
                groupEntry->sourceList ->previous = NULL;
            }
            
            sourceEntry->previous = NULL;
            sourceEntry->next = NULL;
        }
        else
        {   
            if(sourceEntry->previous != NULL)
            {
                sourceEntry->previous->next = sourceEntry->next;
            }

            if(sourceEntry->next != NULL)
            {
                sourceEntry->next->previous = sourceEntry->previous;
            }
            sourceEntry->previous = NULL;
            sourceEntry->next = NULL;
        }
        
        rtl_glueMutexUnlock();/*UnLock resource*/
    }

}

static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr)
{
    rtl_glueMutexLock();
    if (NULL != sourceEntryPtr)
    {
        memset(sourceEntryPtr, 0, sizeof(struct rtl_sourceEntry));
    }
    rtl_glueMutexUnlock();
}


static void rtl_deleteSourceEntry(struct rtl_groupEntry *groupEntry, struct rtl_sourceEntry* sourceEntry)
{
    if(sourceEntry != NULL)
    {
        rtl_unlinkSourceEntry(groupEntry,sourceEntry);
        rtl_clearSourceEntry(sourceEntry);
        rtl_freeSourceEntry(sourceEntry);
    }
}
    
    
static int32 rtl_checkPortMask(uint8 pktPortMask)
{
    int32 i = 0;
    uint8 portMaskn = PORT0_MASK;
    uint8 count = 0;
    for(i = 0; i < 6; i++)
    {
        if(portMaskn & pktPortMask)
        {
            count++;
        }
        portMaskn = portMaskn << 1;
    }

    if(1 == count)
    {
        return GOOD;
    }
    else
    {
        return WRONG;
    }
}


static uint8 rtl_mapPortMaskToPortNum(uint8 portMask)
{   
    switch(portMask)
    {
        
        case PORT0_MASK: return 0;
                
        case PORT1_MASK: return 1;

        case PORT2_MASK: return 2;

        case PORT3_MASK: return 3;

        case PORT4_MASK: return 4;

        case PORT5_MASK: return 5;

        default:return 255;
        
    }

}


static uint8 rtl_mapPortNumToPortMask(uint8 portNum)
{   
    switch(portNum)
    {
    
        case 0: return PORT0_MASK;
                
        case 1: return PORT1_MASK;

        case 2: return PORT2_MASK;

        case 3: return PORT3_MASK;

        case 4: return PORT4_MASK;
                
        case 5: return PORT5_MASK;

        default:return NON_PORT_MASK;
        
    }

}


static int32 rtl_mapMCastIPToMAC(uint32 ipVersion, uint32 *ipAddr, uint8 *macAddr )
{
    if(IP_VERSION6 == ipVersion)
    {
        if(IS_IPV6_MULTICAST_ADDRESS(ipAddr))
        {
            macAddr[0] = 0x33;
            macAddr[1] = 0x33;
            macAddr[2] = (ipAddr[3] & 0xff000000) >> 24;
            macAddr[3] = (ipAddr[3] & 0x00ff0000) >> 16;
            macAddr[4] = (ipAddr[3] & 0x0000ff00) >> 8;
            macAddr[5] = ipAddr[3] & 0x000000ff;
            return SUCCESS;
        }
        else
        {
            return FAILED;
        }
    }

    if(IP_VERSION4 == ipVersion)
    {
        if(IS_IPV4_MULTICAST_ADDRESS(ipAddr))
        {
            macAddr[0] = 0x01;
            macAddr[1] = 0x00;
            macAddr[2] = 0x5e;
            macAddr[3] = (ipAddr[0] & 0x007f0000) >> 16;
            macAddr[4] = (ipAddr[0] & 0x0000ff00) >> 8;
            macAddr[5] = ipAddr[0] & 0x000000ff;
            return SUCCESS;
        }
        else
        {
            return FAILED;
        }
    }

    return FAILED;
}

static int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr)
{
    if(IP_VERSION4 == ipVersion)
    {
        if(macAddr[0] != 0x01)
        {
            return FAILED;
        }

        if((macAddr[3] & 0x7f) != (uint8)((ipAddr[0] & 0x007f0000) >> 16))
        {
            return FAILED;
        }

        if(macAddr[4] != (uint8)((ipAddr[0]&0x0000ff00) >> 8))
        {
            return FAILED;
        }

        if(macAddr[5] != (uint8)(ipAddr[0] & 0x000000ff))
        {
            return FAILED;
        }

        return SUCCESS;
    }

    if(IP_VERSION6 == ipVersion)
    {
        if(macAddr[0] != 0x33)
        {
            return FAILED;
        }

        if(macAddr[1] != 0x33)
        {
            return FAILED;
        }

        if(macAddr[2] != (uint8)((ipAddr[3] & 0xff000000) >> 24))
        {
            return FAILED;
        }
        
        if(macAddr[3] != (uint8)((ipAddr[3] & 0x00ff0000) >> 16))
        {
            return FAILED;
        }

        if(macAddr[4] != (uint8)((ipAddr[3] & 0x0000ff00) >> 8))
        {
            return FAILED;
        }
        
        if(macAddr[5] != (uint8)(ipAddr[3] & 0x000000ff))
        {
            return FAILED;
        }
        
        return SUCCESS;
    }

    return FAILED;
}

static int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2)
{
    int i;
    for(i = 0; i < 4; i++)
    {
        if(ipv6Addr1[i] != ipv6Addr2[i])
        {
            return FAILED;
        }
    }

    return SUCCESS;
}

static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2)
{
    int i;
    for(i = 0; i < 6; i++)
    {
        if(macAddr1[i] != macAddr2[i])
        {
            return FAILED;
        }
    }
    return SUCCESS;
}

static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen)
{
    /*note: the first bytes of  packetBuf should be two bytes aligned*/
    uint32  checksum = 0;
    uint32 count = packetLen;
    uint16 *ptr = (uint16 *)(packetBuf);  

    while(count > 1)
    {
        checksum += ntohs(*ptr);
        ptr++;
        count -= 2;
    }
     
    if(count > 0)
    {
        checksum += *(packetBuf+packetLen - 1) << 8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
    }

    /* Roll over carry bits */
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);

    /* Return checksum */
    return ((uint16)~checksum);

}

static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr)
{
    uint32  checksum = 0;
    uint32 count = pktLen;
    uint16 *ptr;

    /*compute ipv6 pseudo-header checksum*/
    ptr= (uint16 *)(ipv6PseudoHdr);    
    for(count = 0; count < 20; count++) /*the pseudo header is 40 bytes long*/
    {
        checksum += ntohs(*ptr);
        ptr++;
    }
    
    /*compute the checksum of mld buffer*/
    count = pktLen;
    ptr = (uint16 *)(pktBuf);   
    while(count > 1)
    {
        checksum += ntohs(*ptr);
        ptr++;
        count -= 2;
    }

    if(count > 0)
    {
        checksum += *(pktBuf + pktLen - 1) << 8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
    }

    /* Roll over carry bits */
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);

    /* Return checksum */
    return ((uint16)~checksum);

}


static uint8 rtl_getGroupFwdPortMask(struct rtl_groupEntry * groupEntry, uint32 sysTime)
{
    int i;
    uint8 portMaskn = PORT0_MASK;
    uint8 fwdPortMask = 0;
    struct rtl_sourceEntry *sourcePtr = NULL;;

    for(i = 0; i < 6; i++)
    {
        if(TRUE == rtl_enableSourceList)
        {
            if(groupEntry->groupFilterTimer[i] > sysTime) /*exclude mode never expired*/
            {
                fwdPortMask |= portMaskn;
            }
            else/*include mode*/
            {
                sourcePtr = groupEntry->sourceList;
                while(sourcePtr != NULL)
                {
                    if(sourcePtr->portTimer[i] > sysTime)
                    {
                        fwdPortMask |= portMaskn;
                        break;
                    }
                    else
                    {
                        sourcePtr = sourcePtr->next;
                    }
                }
            }
        }
        else
        {
            if(groupEntry->groupFilterTimer[i] > sysTime) /*exclude mode never expired*/
            {
                fwdPortMask |= portMaskn;
            }
        }
        portMaskn = portMaskn << 1;
    }

    return fwdPortMask;
}



static void rtl_checkSourceTimer(struct rtl_groupEntry * groupEntry , struct rtl_sourceEntry * sourceEntry)
{
    int i = 0;

    uint8 portMaskn = PORT0_MASK;
    uint8 deletePortMask = 0;
    for(i = 0; i < 6; i++)
    {   
        if(0 == sourceEntry->portTimer[i])/*means not exist*/
        {
            deletePortMask |= portMaskn;
        }
        else
        {
            if(sourceEntry->portTimer[i] <= rtl_sysUpSeconds) /*means time out*/
            {
                if(groupEntry->groupFilterTimer[i] <= rtl_sysUpSeconds) /*means include mode*/
                {
                    sourceEntry->portTimer[i] = 0;
                    deletePortMask |= portMaskn;
                }  
            }
        }

        portMaskn = portMaskn << 1;
    }

    if(0x3f == deletePortMask) /*means all port  are INCLUDE mode and expired*/
    {
        rtl_deleteSourceEntry(groupEntry, sourceEntry);
    }
    
}

static uint8 rtl_getSourceFwdPortMask(struct rtl_groupEntry * groupEntry,uint32 *sourceAddr, uint32 sysTime)
{
    int i;
    uint8 portMaskn = PORT0_MASK;
    uint8 fwdPortMask = 0;
    struct rtl_sourceEntry *sourceEntry = NULL;
    if(NULL == groupEntry)
    {
        return 0x3f; /*broadcast*/
    }
    else
    {
        sourceEntry = rtl_searchSourceEntry((uint32)(groupEntry->ipVersion), sourceAddr, groupEntry);
        for(i = 0; i < 6; i++)
        {
            
            if(groupEntry->groupFilterTimer[i] <= sysTime)    /*include mode*/
            {   
                if(sourceEntry != NULL)
                {
                    if( sourceEntry->portTimer[i] > sysTime)
                    {
                        fwdPortMask |= portMaskn;
                    }
                }
            }
            else/*exclude mode*/
            {   
                if(NULL == sourceEntry)
                {
                    fwdPortMask |= portMaskn;
                }
                else
                {
                    if((sourceEntry->portTimer[i] > sysTime) || (sourceEntry->portTimer[i] == 0))
                    {
                        fwdPortMask |= portMaskn;
                    }
                }
            }            
            portMaskn = portMaskn << 1;
        }
        
        return fwdPortMask;       
    }   
}


static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry)
{
    uint8 oldFwdPortMask = 0;
    uint8 newFwdPortMask = 0;
    struct rtl_sourceEntry *sourceEntry = groupEntry->sourceList;
    struct rtl_sourceEntry *nextSourceEntry = NULL;
    oldFwdPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_previousSysTime);

    while(sourceEntry != NULL)
    {
        nextSourceEntry = sourceEntry->next;
        rtl_checkSourceTimer(groupEntry, sourceEntry);
        sourceEntry = nextSourceEntry;
    }

    newFwdPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);

    if(oldFwdPortMask != newFwdPortMask)
    {
        if(TRUE == groupEntry->lookupTableFlag)
        {
            rtl_setLookupTable(groupEntry);  /*set rtl8306sdm lookup table*/
        }
    }

    if(0 == newFwdPortMask) /*none active port*/
    {
        rtl_deleteGroupEntry(groupEntry);
    }
    
}

static void rtl_checkAggregator(uint32 ipVersion, uint32 *ipAddr,  uint8 *portMask, uint8 *lookupTableFlag)
{   
    uint32 hashIndex = 0;
    struct rtl_groupEntry *entryPtr = NULL;
    uint32 lowIpAddr = 0;
    uint32 count = 0;
    uint8 fwdPortMask = 0;

    *portMask = 0;
    *lookupTableFlag = 0;

    if(IP_VERSION6 == ipVersion)
    {
        hashIndex = ipAddr[3] & rtl_hashMask;
        entryPtr = rtl_ipv6HashTable[hashIndex];
        lowIpAddr = ipAddr[3] & IPV6_MCAST_MAC_MASK;
    }
    else
    {
        hashIndex = ipAddr[0]&rtl_hashMask;
        entryPtr = rtl_ipv4HashTable[hashIndex];
        lowIpAddr = ipAddr[0] & IPV4_MCAST_MAC_MASK;
    }

    
    while(entryPtr != NULL)
    {
        fwdPortMask = rtl_getGroupFwdPortMask(entryPtr, rtl_sysUpSeconds);
        if(IP_VERSION6 == ipVersion)
        {
            if((entryPtr->groupAddr[3] & IPV6_MCAST_MAC_MASK) == lowIpAddr)
            {
                *portMask |= fwdPortMask;
                if(TRUE == entryPtr->lookupTableFlag)
                {
                    *lookupTableFlag = TRUE;
                }

                if(fwdPortMask != 0)/*exclude zero port mask entry*/
                {
                    count = count + 1;
                }
                
            }
        }
        else
        {
            if((entryPtr->groupAddr[0] & IPV4_MCAST_MAC_MASK) == lowIpAddr)
            {
                *portMask |= fwdPortMask;
                if(TRUE == entryPtr->lookupTableFlag)
                {
                    *lookupTableFlag = TRUE;
                }

                if(fwdPortMask != 0)/*exclude zero port mask entry*/
                {
                    count = count + 1;
                }
            
            }
        }
    
        entryPtr=entryPtr->next;
    }

     /*check whether need  change to non-aggregator status*/
    if(IP_VERSION6 == ipVersion)
    {
        entryPtr = rtl_ipv6HashTable[hashIndex];
    }
    else
    {
        entryPtr = rtl_ipv4HashTable[hashIndex];
    }
    
    while(entryPtr != NULL)
    {
        if(IP_VERSION6 == ipVersion)
        {
            if((entryPtr->groupAddr[3] & IPV6_MCAST_MAC_MASK) == lowIpAddr)
            {
        
                if(count > 1)
                {   
                    entryPtr->aggregatorFlag = TRUE;
                }
                else
                {
                    entryPtr->aggregatorFlag = FALSE;
                }
            }
        }
        else
        {
            if((entryPtr->groupAddr[0] & IPV4_MCAST_MAC_MASK) == lowIpAddr)
            {
        
                if(count > 1)
                {   
                    entryPtr->aggregatorFlag = TRUE;
                }
                else
                {
                    entryPtr->aggregatorFlag = FALSE;
                }
            }
        }
        
        entryPtr = entryPtr->next;
    }
    
}

static void rtl_setAggregator(uint32 ipVersion, uint32 *ipAddr, int32 flag)
{   
    uint32 hashIndex = 0;
    struct rtl_groupEntry *entryPtr = NULL;
    uint32 lowIpAddr = 0;
    uint8 fwdPortMask = 0;
    if(IP_VERSION6 == ipVersion)
    {
        hashIndex = ipAddr[3] & rtl_hashMask;
        entryPtr = rtl_ipv6HashTable[hashIndex];
        lowIpAddr = ipAddr[3] & IPV6_MCAST_MAC_MASK;
    }
    else
    {
        hashIndex = ipAddr[0] & rtl_hashMask;
        entryPtr = rtl_ipv4HashTable[hashIndex];
        lowIpAddr = ipAddr[0] & IPV4_MCAST_MAC_MASK;
    }
    
    while(entryPtr != NULL)
    {
        fwdPortMask = rtl_getGroupFwdPortMask(entryPtr,rtl_sysUpSeconds);
        if(IP_VERSION6 == ipVersion)
        {
            if((entryPtr->groupAddr[3] & IPV6_MCAST_MAC_MASK) == lowIpAddr)
            {
        
                if((TRUE == flag) && (fwdPortMask != 0))
                {   
                    entryPtr->lookupTableFlag=TRUE;
                }
                else
                {
                    entryPtr->lookupTableFlag=FALSE;
                }
            }
        }
        else
        {
            if((entryPtr->groupAddr[0] & IPV4_MCAST_MAC_MASK) == lowIpAddr)
            {

                if((TRUE == flag) && (fwdPortMask != 0))
                {   
                    entryPtr->lookupTableFlag = TRUE;
                }
                else
                {
               
                    entryPtr->lookupTableFlag = FALSE;
                }
            }
        }        
        entryPtr = entryPtr->next;
    }

}



static int32 rtl_setLookupTable(struct rtl_groupEntry *groupEntry)
{ 
    uint8 multicastMacAddress[6];
    uint8 aggregatorPortMask = 0;
    uint8 aggregatorLookupTableFlag = 0;
    uint32 LookupTablePortMask = 0;
    uint32 groupPortMask;
    uint8 multicastRouterPortMask = rtl_getMulticastRouterPortMask(groupEntry->ipVersion, rtl_sysUpSeconds);
    
    rtl_mapMCastIPToMAC(groupEntry->ipVersion, groupEntry->groupAddr, multicastMacAddress);
    
    if(TRUE == groupEntry->aggregatorFlag)
    {
        /*get aggreagtor count and aggregator forward port mask*/
    
        rtl_checkAggregator(groupEntry->ipVersion, groupEntry->groupAddr, &aggregatorPortMask,&aggregatorLookupTableFlag);

        LookupTablePortMask = (uint32)(aggregatorPortMask|multicastRouterPortMask|rtl_cpuPortMask);
        
        if(aggregatorPortMask != 0) /*need to modify aggregator multicast entry*/
        {
            if(TRUE == groupEntry->lookupTableFlag) /*means in the lookup table*/
            {
                if(rtl_addLUTMulticast(multicastMacAddress, LookupTablePortMask) == SUCCESS) /*then modify existing entry*/
                {
                    rtl_setAggregator(groupEntry->ipVersion, groupEntry->groupAddr, TRUE);

                }
                else
                {
                    rtl_gluePrintf("hareware failure 1\n");
                    return FAILED;
                }
            }
            else /*means not in the lookup table*/
            {     
                if(rtl_addLUTMulticast(multicastMacAddress, LookupTablePortMask) == SUCCESS) /*then modify existing entry*/
                {
                    rtl_setAggregator(groupEntry->ipVersion, groupEntry->groupAddr, TRUE); /* update aggragator entries' flag*/
                }
                else /* maybe lookup table collision, or run out of multicast entry*/
                {
                    rtl_setAggregator(groupEntry->ipVersion, groupEntry->groupAddr, FALSE);
                    return FAILED;
                }
            }           
        }
        else /*means all aggregator entries expired at the same time*/
        {
            if(aggregatorLookupTableFlag != 0)
            {                           
                if(rtl_delLUTMACAddress(multicastMacAddress) == SUCCESS)
                {
                    rtl_setAggregator(groupEntry->ipVersion, groupEntry->groupAddr, FALSE);  /* update aggragator entries' flag*/
                }
                else
                {
                        rtl_gluePrintf("hareware failure 2\n");
                        return FAILED;
                }
            }
            
        }   
    }
    else /*here means non-aggregator entry*/
    {
        groupPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        
        if(groupPortMask != 0) /*modify look up table*/
        {   
            LookupTablePortMask = (uint32)(groupPortMask | multicastRouterPortMask | rtl_cpuPortMask);
            /*forward port mask is the combination of member ports,multicast router ports and cpu port*/
            if(TRUE == groupEntry->lookupTableFlag)
            {
                if(rtl_addLUTMulticast(multicastMacAddress, LookupTablePortMask) != SUCCESS)
                {
                    rtl_gluePrintf("hareware failure 3\n");
                    return FAILED;
                }               
            }
            else  /*means not in the look up table, alwarys try to write lookup table*/
            {
                if(rtl_addLUTMulticast(multicastMacAddress, LookupTablePortMask) == SUCCESS)
                {
                    groupEntry->lookupTableFlag = TRUE;
                }
                else /* maybe lookup table collision, or run out of multicast entry*/
                {
                    return FAILED;
                }
            }
        }
        else
        {
            if(TRUE == groupEntry->lookupTableFlag)
            {
                if(rtl_delLUTMACAddress(multicastMacAddress) == SUCCESS)
                {
                    groupEntry->lookupTableFlag = FALSE;
                }
                else
                {
                    rtl_gluePrintf("hareware failure 4\n");
                    return FAILED;
                }
            }
        }       
    }

    return SUCCESS;
}

static void rtl_updateAllGroupEntry(struct rtl_groupEntry** hashTable)
{
    uint32 i = 0;
    struct rtl_groupEntry* groupEntryPtr = NULL;
    for(i = 0; i < rtl_hashTableSize; i++)
    {
        groupEntryPtr = hashTable[i];
        while(groupEntryPtr != NULL)
        {   
            if(TRUE == groupEntryPtr->lookupTableFlag)
            {
                rtl_setLookupTable(groupEntryPtr);
            }
            groupEntryPtr = groupEntryPtr->next;
        }
    }   
}


static int32 rtl_initHashTable(uint32 hashTableSize)
{
    uint32 i = 0;
    rtl_ipv4HashTable = NULL;
    rtl_ipv6HashTable = NULL;
    rtl_glueMutexLock();    /* Lock resource */

    /* Allocate memory */
    rtl_ipv4HashTable = (struct rtl_groupEntry **)rtl_glueMalloc(4 * hashTableSize);
    rtl_ipv6HashTable = (struct rtl_groupEntry **)rtl_glueMalloc(4 * hashTableSize);
    if((rtl_ipv4HashTable != NULL) && (rtl_ipv6HashTable != NULL))
    {
        for (i = 0; i < hashTableSize; i++)
        {   
            rtl_ipv4HashTable[i] = NULL;
            rtl_ipv6HashTable[i] = NULL;
        }

        rtl_glueMutexUnlock();  /* UnLock resource */
        return SUCCESS;
    }
    else
    {
        if(rtl_ipv4HashTable != NULL)
        {
            rtl_glueFree(rtl_ipv4HashTable);
        }

        if(rtl_ipv6HashTable != NULL)
        {
            rtl_glueFree(rtl_ipv6HashTable);
        }
        rtl_glueMutexUnlock();  /* UnLock resource */
        return FAILED;
    }
}



/**************************
    Utility
**************************/
static void rtl_parseMacFrame(uint8* macFrame, struct rtl_macFrameInfo* macInfo) 
{

/*MAC Frame :DA(6 bytes)+SA(6 bytes) + CPU tag(4 bytes) + VlAN tag(Optional, 4 bytes)
 *                 + Type(IPv4:0x0800, IPV6:0x86DD, PPPOE:0x8864, 2 bytes ) + Data(46~1500 bytes) + CRC(4 bytes)
 *CPU tag: Realtek Ethertype==0x8899(2 bytes) + protocol==0x9(4 MSB) + priority(2 bits) + reserved(4 bits) + portmask(6 LSB)
 */
    uint8 *ptr = macFrame;
    int i = 0;
    int j = 0;
    uint8 nextHeader = 0;
    uint16 extensionHdrLen = 0;
    uint8 routerhead = FALSE;
    uint8 needchecksum = FALSE;

    uint8 optionDataLen = 0;
    uint8 optionType = 0;
    uint32 ipv6RAO = 0;
    uint32 ipAddr[4] = {0, 0, 0, 0};
    union pseudoHeader pHeader;

    uint8 portNum = 0xff;
    uint32 PVid;
    
    
    memset(&pHeader, 0, sizeof(union pseudoHeader));

    /*check the presence of CPU tag*/
    ptr = ptr + 12;
    if((*((uint16 *)ptr) == htons(rtl_etherType)) && (((*(ptr + 2) & 0xf0) >> 4) == CPUTAGPROTOCOL))
    {
        macInfo->cpuTagFlag = 1;
        macInfo->cpuPriority = (*(ptr + 2) & 0x0c) >> 2;
        macInfo->pktPortMask = *(ptr + 3) & 0x3f; 
        rtl_getCPUTagRXPort(&portNum, &macInfo->pktPortMask);
        ptr = ptr + 4;
    }
    else
    {
        macInfo->pktPortMask = NON_PORT_MASK;
    }

    /*check the presence of VLAN tag*/  
    if(*(int16 *)(ptr) == (int16)htons(VLAN_PROTOCOL_ID))
    {
        if((macInfo->vlanTagID = (ntohs(*(int16 *)(ptr+2)) & 0x0fff))!= 0)
        {
            macInfo->vlanTagFlag = VLANTAGGED;
        }
        else
        {
            macInfo->vlanTagFlag = PRITAGGED;
        }

        ptr = ptr + 4;
    }
    else
    {
        if(portNum != 0xff)
        {
            rtl_GetPVID(portNum, &PVid);
            macInfo->vlanTagID = PVid;
        }
        else
        {
            macInfo->vlanTagID = 0x00;
        }

       macInfo->vlanTagFlag = UNTAGGED;
    }
/*
#ifndef RTL8306_TBLBAK
        if(*(int16 *)(ptr)==(int16)htons(VLAN_PROTOCOL_ID))
        {
            ptr=ptr+4;
        }
#endif
*/
    
    /*ignore packet with PPPOE header*/ 
    if(*(int16 *)(ptr) == (int16)htons(PPPOE_ETHER_TYPE))
    {
        return; 
    }

    
    /*check the presence of ipv4 type*/
    if(*(int16 *)(ptr) == (int16)htons(IPV4_ETHER_TYPE))
    {
        ptr = ptr + 2;
        macInfo->ipBuf = ptr;
        macInfo->ipVersion = IP_VERSION4;
    }
    else
    {
        /*check the presence of ipv4 type*/
        if(*(int16 *)(ptr) == (int16)htons(IPV6_ETHER_TYPE))
        {
            ptr = ptr + 2;
            macInfo->ipBuf = ptr;
            macInfo->ipVersion = IP_VERSION6;
        }
    }

    if((macInfo->ipVersion != IP_VERSION4) && (macInfo->ipVersion != IP_VERSION6))
    {
        return;
    }

    if(IP_VERSION4 == macInfo->ipVersion)
    {
        macInfo->ipHdrLen = (uint16)((((struct ipv4Pkt *)(macInfo->ipBuf))->vhl & 0x0f) << 2);
        macInfo->l3PktLen = ntohs(((struct ipv4Pkt *)(macInfo->ipBuf))->length) - macInfo->ipHdrLen;
        ptr = ptr + macInfo->ipHdrLen;
        macInfo->l3PktBuf = ptr;
        macInfo->macFrameLen=(uint16)((ptr - macFrame) + macInfo->l3PktLen);
    
/*distinguish different IGMP packet:
                                                    ip_header_length      destination_ip      igmp_packet_length   igmp_type   group_address            
 *IGMPv1_general_query:                            20                   224.0.0.1                       8                    0x11                 0
 *IGMPv2_general_query:                            24                   224.0.0.1                       8                    0x11                 0                     
 *IGMPv2_group_specific_query:                   24                   224.0.0.1                       8                    0x11               !=0  
 *IGMPv3 _query:                                       24                   224.0.0.1                   >=12                  0x11        according_to_different_situation 

 *IGMPv1_join:                                           20          actual_multicast_address         8                    0x12           actual_multicast_address
 *IGMPv2_join:                                           24          actual_multicast_address         8                    0x16           actual_multicast_address
 *IGMPv2_leave:                                         24          actual_multicast_address         8                    0x17           actual_multicast_address
 *IGMPv3_report:                                        24          actual_multicast_address       >=12                0x22           actual_multicast_address
 */

        /* parse IGMP type and version*/    
        if(IGMP_PROTOCOL == ((struct ipv4Pkt *)(macInfo->ipBuf))->protocol)
        {   
            /*check DVMRP*/
            if((DVMRP_TYPE == macInfo->l3PktBuf[0]) && (((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp == htonl(DVMRP_ADDR)))
            {
                macInfo->l3Protocol = DVMRP_PROTOCOL;
            }
            else
            {
                /*means unicast*/
                if(0 == (macFrame[0] & 0x01))
                {
                    for(i = 0; i < 4; i++)
                    {
                        if(rtl_compareMacAddr(macFrame, rtl_gatewayMac[i]) == SUCCESS)
                        {
                            for(j = 0; j < 4; j++)
                            {
                                if(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp == htonl(rtl_gatewayIpv4Addr[j]))
                                {
                                    macInfo->l3Protocol = IGMP_PROTOCOL;
                                        goto otherpro;
                                }
                            }
                        }
                    }                           
                }
                else /*means multicast*/
                {   
                    ipAddr[0] = ntohl(((struct ipv4Pkt*)(macInfo->ipBuf))->destinationIp);
                    if(rtl_checkMCastAddrMapping(IP_VERSION4, ipAddr, macFrame) == SUCCESS)
                    {
                        macInfo->l3Protocol = IGMP_PROTOCOL;
                    }
                    else
                    {
                        return;
                    }
                }
            }
            
        }

otherpro:   
        if((((struct ipv4Pkt*)(macInfo->ipBuf))->protocol == MOSPF_PROTOCOL) &&\
        ((((struct ipv4Pkt*)(macInfo->ipBuf))->destinationIp == htonl(IPV4_MOSPF_ADDR1)) ||\
        (((struct ipv4Pkt*)(macInfo->ipBuf))->destinationIp == htonl(IPV4_MOSPF_ADDR2))))
        {
            macInfo->l3Protocol = MOSPF_PROTOCOL;
        }

        if(((struct ipv4Pkt*)(macInfo->ipBuf))->protocol == PIM_PROTOCOL && (((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp == htonl(IPV4_PIM_ADDR)))
        {
            macInfo->l3Protocol = PIM_PROTOCOL;
        }
        
        if(rtl_checksum(macInfo->l3PktBuf, macInfo->l3PktLen) != 0)
        {
            macInfo->checksumFlag = FAILED;
        }
        else
        {
            macInfo->checksumFlag = SUCCESS;
        }
    }


    if(IP_VERSION6 == macInfo->ipVersion)
    {
        macInfo->macFrameLen = (uint16)(ptr - macFrame + IPV6_HEADER_LENGTH + ntohs(((struct ipv6Pkt*)(macInfo->ipBuf))->payloadLenth));
        macInfo->ipHdrLen = IPV6_HEADER_LENGTH;
        
        nextHeader = ((struct ipv6Pkt*)(macInfo->ipBuf))->nextHeader;
        ptr = ptr + IPV6_HEADER_LENGTH;
        while((ptr - macInfo->ipBuf) < (ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth) + IPV6_HEADER_LENGTH))
        {
            switch(nextHeader) 
            {
                case HOP_BY_HOP_OPTIONS_HEADER:
                    /*parse hop-by-hop option*/
                    nextHeader = ptr[0];
                    extensionHdrLen = ((uint16)(ptr[1]) + 1) * 8;
                    ptr = ptr + 2;
                    
                    while((ptr - macInfo->ipBuf - 40) < extensionHdrLen)
                    {
                        optionType = ptr[0];
                        /*pad1 option*/
                        if(0 == optionType)
                        {
                            ptr = ptr + 1;
                            continue;
                        }

                        /*padN option*/
                        if(1 == optionType)
                        {
                            optionDataLen = ptr[1];
                            ptr = ptr + optionDataLen + 2;
                            continue;
                        }

                        /*router alter option*/
                        if(IPV6_ROUTER_ALTER_OPTION == ntohl(*(uint32 *)(ptr)))
                        {
                            ipv6RAO = IPV6_ROUTER_ALTER_OPTION;
                            ptr = ptr + 4;  
                            continue;
                        }

                        /*other TLV option*/
                        if((optionType != 0) && (optionType != 1))
                        {
                            optionDataLen = ptr[1];
                            ptr = ptr+optionDataLen + 2;
                            continue;
                        }
                    

                    }
                   /*
                    if((ptr-macInfo->ipBuf-40)!=extensionHdrLen)
                    {
                        rtl_gluePrintf("ipv6 packet parse error\n");
                    }
                    */
                    
                    break;
                
                case ROUTING_HEADER:
                    nextHeader = ptr[0];
                    extensionHdrLen = ((uint16)(ptr[1]) + 1) * 8;

                    if (ptr[3] > 0)
                    {
                        ptr = ptr + extensionHdrLen;
                        for(i = 0; i < 4; i++)
                        {
                        pHeader.ipv6_pHdr.destinationAddr[i] = *((uint32 *)(ptr) - 4 + i);

                        }
                        routerhead=TRUE;                                                 
                    }
                    else
                    {
                        ptr=ptr+extensionHdrLen;
                    }
                                                            
                    break;
                
                case FRAGMENT_HEADER:
                    nextHeader = ptr[0];
                    ptr = ptr + 8;
                    break;
                
                case DESTINATION_OPTION_HEADER:
                    nextHeader = ptr[0];
                    extensionHdrLen = ((uint16)(ptr[1]) + 1) * 8;
                    ptr = ptr + extensionHdrLen;
                    break;
                
                case ICMP_PROTOCOL:
                    nextHeader = NO_NEXT_HEADER;
                    macInfo->l3PktLen = ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth) - (uint16)(ptr - macInfo->ipBuf - IPV6_HEADER_LENGTH);
                    macInfo->l3PktBuf = ptr;
                    if((MLD_QUERY == ptr[0]) ||(MLDV1_REPORT == ptr[0]) ||(MLDV1_DONE == ptr[0]) ||(MLDV2_REPORT == ptr[0]))
                    {
                        /*means multicast*/
                        if((0x33 == macFrame[0])&&(0x33 == macFrame[1]))
                        {
                            ipAddr[0] = ntohl(((struct ipv6Pkt*)(macInfo->ipBuf))->destinationAddr[0]);
                            ipAddr[1] = ntohl(((struct ipv6Pkt*)(macInfo->ipBuf))->destinationAddr[1]);
                            ipAddr[2] = ntohl(((struct ipv6Pkt*)(macInfo->ipBuf))->destinationAddr[2]);
                            ipAddr[3] = ntohl(((struct ipv6Pkt*)(macInfo->ipBuf))->destinationAddr[3]);
                            
                            if(rtl_checkMCastAddrMapping(IP_VERSION6, ipAddr, macFrame) == SUCCESS)
                            {
                                macInfo->l3Protocol = ICMP_PROTOCOL;
                            }
                            
                        }
                        else /*means multicast*/
                        {   
                            for(i = 0; i < 4; i++)
                            {
                                ipAddr[0] = htonl(rtl_gatewayIpv6Addr[i][0]);
                                ipAddr[1] = htonl(rtl_gatewayIpv6Addr[i][1]);
                                ipAddr[2] = htonl(rtl_gatewayIpv6Addr[i][2]);
                                ipAddr[3] = htonl(rtl_gatewayIpv6Addr[i][3]);
                                if((rtl_compareMacAddr(macFrame, rtl_gatewayMac[i]) == SUCCESS) &&\
                                (rtl_compareIpv6Addr(((struct ipv6Pkt *)macInfo->ipBuf)->destinationAddr, ipAddr) == SUCCESS))
                                {
                                    macInfo->l3Protocol=ICMP_PROTOCOL;
                                }
                            }
                        }

                        needchecksum = TRUE;
                        /*
                         if(ipv6RAO!=IPV6_ROUTER_ALTER_OPTION)
                         {
                             rtl_gluePrintf("router alter option error\n");
                         }*/
                    }                                   
                    break;
                
                case PIM_PROTOCOL:
                    nextHeader = NO_NEXT_HEADER;
                    macInfo->l3PktLen = ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth) - (uint16)(ptr - macInfo->ipBuf - IPV6_HEADER_LENGTH);
                    macInfo->l3PktBuf = ptr;
                    
                    ipAddr[0] = ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
                    ipAddr[1] = ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
                    ipAddr[2] = ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
                    ipAddr[3] = ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
                    if(IS_IPV6_PIM_ADDR(ipAddr))
                    {
                        macInfo->l3Protocol = PIM_PROTOCOL;
                    }
                    needchecksum = TRUE;                
                    break;
                
                case MOSPF_PROTOCOL:
                    nextHeader = NO_NEXT_HEADER;
                    macInfo->l3PktLen = ntohs(((struct ipv6Pkt*)(macInfo->ipBuf))->payloadLenth) - (uint16)(ptr - macInfo->ipBuf - IPV6_HEADER_LENGTH);
                    macInfo->l3PktBuf = ptr;
                    
                    ipAddr[0] = ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
                    ipAddr[1] = ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
                    ipAddr[2] = ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
                    ipAddr[3] = ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
                    
                    if(IS_IPV6_MOSPF_ADDR1(ipAddr) || IS_IPV6_MOSPF_ADDR2(ipAddr))
                    {
                        macInfo->l3Protocol = MOSPF_PROTOCOL;
                    }
                    needchecksum = TRUE;
                    break;
                
                default:        
                    goto out;
                    break;
            }
        
        }

out:
        /*compute pseudo header*/
        if(TRUE == needchecksum)
        {
            for(i = 0; i < 4; i++)
            {
                pHeader.ipv6_pHdr.sourceAddr[i] = ((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[i];
            }

            if(FALSE == routerhead)
            {
                for(i = 0; i < 4; i++)
                {
                    pHeader.ipv6_pHdr.destinationAddr[i] = ((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[i];
                }
            }
            
            pHeader.ipv6_pHdr.nextHeader = macInfo->l3Protocol;
            pHeader.ipv6_pHdr.upperLayerPacketLength = htonl((uint32)(macInfo->l3PktLen));
            pHeader.ipv6_pHdr.zeroData[0] = 0;
            pHeader.ipv6_pHdr.zeroData[1] = 0;
            pHeader.ipv6_pHdr.zeroData[2] = 0;

            if(macInfo->l3PktBuf != NULL)
            {
                if(rtl_ipv6L3Checksum(macInfo->l3PktBuf, macInfo->l3PktLen, &pHeader) != 0)
                {
                    macInfo->checksumFlag = FAILED;
                }
                else
                {
                    macInfo->checksumFlag = SUCCESS;
                }
            }
        }
    }

}


static uint8  rtl_getMulticastRouterPortMask(uint32 ipVersion, uint32 sysTime)
{
    uint32 portIndex = 0;
    uint8 portMaskn = PORT0_MASK;
    uint8 routerPortmask = 0;

    if(IP_VERSION4 == ipVersion)
    {
        for(portIndex = 0; portIndex < 6; portIndex++)
        {
            if(rtl_ipv4MulticastRouters.querier.portTimer[portIndex] > sysTime)
            {
                routerPortmask = routerPortmask | portMaskn;
            }
            
            if(rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portIndex] > sysTime)
            {   
                routerPortmask = routerPortmask | portMaskn;
            }
            
            if(rtl_ipv4MulticastRouters.mospfRouter.portTimer[portIndex] > sysTime)
            {   
                routerPortmask = routerPortmask | portMaskn;
            }       

            if(rtl_ipv4MulticastRouters.pimRouter.portTimer[portIndex] > sysTime)
            {   
                routerPortmask = routerPortmask|portMaskn;
            }   
        
            portMaskn = portMaskn << 1;  /*shift to next port mask*/           
        }

    }

    if(IP_VERSION6 == ipVersion)
    {
        for(portIndex = 0; portIndex < 6; portIndex++)
        {
            if(rtl_ipv6MulticastRouters.querier.portTimer[portIndex] > sysTime)
            {
                routerPortmask = routerPortmask | portMaskn;
            }       

            if(rtl_ipv6MulticastRouters.mospfRouter.portTimer[portIndex] > sysTime)
            {   
                routerPortmask = routerPortmask | portMaskn;
            }   
            
            if(rtl_ipv6MulticastRouters.pimRouter.portTimer[portIndex] > sysTime)
            {   
                routerPortmask = routerPortmask | portMaskn;
            }   
            
            portMaskn = portMaskn << 1;  /*shift to next port mask*/           
        }
    }
    
    return routerPortmask;
}

static uint8 rtl_processQueries(uint32 ipVersion, uint8 pktPortMask, uint8* pktBuf, uint32 pktLen)
{
    struct rtl_groupEntry *groupEntry = NULL;
    struct rtl_sourceEntry*sourceEntry = NULL;
    uint32 timerIndex = 0;
    uint32 hashIndex = 0; 
    uint32 groupAddress[4] = {0, 0, 0, 0};
    uint32 suppressFlag = 0;
    uint32 *sourceAddr = NULL;
    uint32 numOfSrc = 0;
    uint32 i = 0;
    
    /*querier timer update and election process*/
    rtl_snoopQuerier(ipVersion, pktPortMask);
    
    if(IP_VERSION4 == ipVersion)
    {   
        if(pktLen >= 12) /*means igmpv3 query*/
        {
            groupAddress[0] = ntohl(((struct igmpv3Query*)pktBuf)->groupAddr);
            suppressFlag = ((struct igmpv3Query*)pktBuf)->rsq & S_FLAG_MASK;
            sourceAddr =& (((struct igmpv3Query*)pktBuf)->srcList);
            numOfSrc = (uint32)ntohs(((struct igmpv3Query*)pktBuf)->numOfSrc);
        }
        else
        {
            groupAddress[0] = ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
        }
        
        if(groupAddress[0] == 0) /*means general query*/
        {
            goto out;
        }
        else
        {
            hashIndex = groupAddress[0] & rtl_hashMask;
        }       
    }

    if(IP_VERSION6 == ipVersion)
    {
        if(pktLen >= 28) /*means mldv2 query*/
        {
            groupAddress[0] = ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[0]);
            groupAddress[1] = ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[1]);
            groupAddress[2] = ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[2]);
            groupAddress[3] = ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[3]);

            suppressFlag = ((struct mldv2Query*)pktBuf)->rsq & S_FLAG_MASK;
            sourceAddr =& (((struct mldv2Query*)pktBuf)->srcList);
            numOfSrc = (uint32)ntohs(((struct mldv2Query*)pktBuf)->numOfSrc);
        }
        else /*means mldv1 query*/
        {
            groupAddress[0] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
            groupAddress[1] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
            groupAddress[2] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
            groupAddress[3] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);            
        }
        
        if(0 == groupAddress[0]) /*means general query*/
        {
            goto out;
        }
        else
        {
            hashIndex = groupAddress[3] & rtl_hashMask;
        }
    }
    
    if(0 == suppressFlag)
    {
        groupEntry = rtl_searchGroupEntry(ipVersion, groupAddress);
        if((groupEntry != NULL))
        {   
            if(0 == numOfSrc) /*means group specific query*/
            {
                for(timerIndex = 0; timerIndex < 6; timerIndex++)
                {
                    if(groupEntry->groupFilterTimer[timerIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                    {
                        groupEntry->groupFilterTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                    }
                }
            }
            else /*means group and source specific query*/
            {
                if(TRUE == rtl_enableSourceList)
                {
                    for(i = 0; i < numOfSrc; i++)
                    {   
                        
                        sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
                        
                        if(sourceEntry != NULL)
                        {
                            for(timerIndex = 0; timerIndex < 6; timerIndex++)
                            {
                                if(sourceEntry->portTimer[timerIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                                {
                                    sourceEntry->portTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                                }
                            }
                        }

                        if(IP_VERSION4 == ipVersion)
                        {
                            sourceAddr++;
                        }

                        if(IP_VERSION6 == ipVersion)
                        {
                            sourceAddr = sourceAddr + 4;
                        }
                    }
                }
            }
        }
     
    }
        
out:    
    return ((~(pktPortMask | rtl_cpuPortMask)) & 0x3f);
}


static void rtl_snoopQuerier(uint32 ipVersion, uint8 pktPortMask)
{
    uint8 oldPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
    uint8 newPortMask = 0;
    uint32 portNum = rtl_mapPortMaskToPortNum(pktPortMask);
    
    if(IP_VERSION4 == ipVersion)
    {
        rtl_ipv4MulticastRouters.querier.portTimer[portNum] = rtl_sysUpSeconds + rtl_mCastTimerParas.querierPresentInterval;/*update timer value*/
        newPortMask = rtl_getMulticastRouterPortMask(IP_VERSION4, rtl_sysUpSeconds);
        if(oldPortMask != newPortMask)
        {
            rtl_updateAllGroupEntry(rtl_ipv4HashTable);
        }
    }
        
    if(IP_VERSION6 == ipVersion)
    {
        rtl_ipv6MulticastRouters.querier.portTimer[portNum] = rtl_sysUpSeconds + rtl_mCastTimerParas.querierPresentInterval;/*update timer value*/
        newPortMask = rtl_getMulticastRouterPortMask(IP_VERSION6, rtl_sysUpSeconds);
        if(oldPortMask != newPortMask)
        {
            rtl_updateAllGroupEntry(rtl_ipv6HashTable);
        }
    }
}


/*Process Report Packet*/
static uint8 rtl_processJoin(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf)
{
    uint32 groupAddress[4] = {0, 0, 0, 0};
    struct rtl_groupEntry* groupEntry = NULL;
    struct rtl_groupEntry* newGroupEntry = NULL;
    struct rtl_sourceEntry *sourceEntry = NULL;

    uint32 hashIndex = 0;
    uint32 portIndex = rtl_mapPortMaskToPortNum(pktPortMask);
    uint8 aggregatorPortMask = 0;
    uint8 aggregatorLookupTableFlag = 0;

    uint8 oldPortMask = 0;
    uint8 newPortMask = 0;
    uint8 multicastRouterPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);

    if(IP_VERSION4 == ipVersion)
    {
        if(0x12 == pktBuf[0])
        { 
            groupAddress[0] = ntohl(((struct igmpv1Pkt *)pktBuf)->groupAddr);
        }

        if(0x16 == pktBuf[0])
        {
            groupAddress[0] = ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
        }
        
        hashIndex = groupAddress[0] & rtl_hashMask;

    }

    if(IP_VERSION6 == ipVersion)
    {
        
        groupAddress[0] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
        groupAddress[1] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
        groupAddress[2] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
        groupAddress[3] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);

        hashIndex = groupAddress[3] & rtl_hashMask;
    }
    

    groupEntry = rtl_searchGroupEntry(ipVersion, groupAddress);
    if(NULL == groupEntry)   /*means new group address, create new group entry*/
    {
        newGroupEntry = rtl_allocateGroupEntry();
        if(NULL == newGroupEntry)
        {
            rtl_gluePrintf("run out of group entry!\n");
            goto out;
        }
        else
        {
            /*set new multicast entry*/ 
            newGroupEntry->groupAddr[0] = groupAddress[0];
            newGroupEntry->groupAddr[1] = groupAddress[1];
            newGroupEntry->groupAddr[2] = groupAddress[2];
            newGroupEntry->groupAddr[3] = groupAddress[3];
            
            newGroupEntry->sourceList = NULL;
            newGroupEntry->ipVersion = ipVersion;

            newGroupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
            newGroupEntry->lookupTableFlag = FALSE;
            
            /*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
            if(IP_VERSION4 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv4HashTable, hashIndex);
            }

            if(IP_VERSION6 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv6HashTable, hashIndex);
            }
            
            rtl_checkAggregator(ipVersion, groupAddress, &aggregatorPortMask, &aggregatorLookupTableFlag);

            rtl_setLookupTable(newGroupEntry); /*always try to add new group entry into lookup table*/
        }     
    }
    else
    {  
        oldPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        sourceEntry = groupEntry->sourceList;

        /*delete all the source list*/
        while(sourceEntry != NULL)
        {
            sourceEntry->portTimer[portIndex] = 0;
            sourceEntry = sourceEntry->next;
        }
        
        groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
        newPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);

        /*always try to add new portmask and rewrite those that were not in lookup table*/
        if((oldPortMask != newPortMask) || (FALSE == groupEntry->lookupTableFlag))
        {   
            rtl_setLookupTable(groupEntry);
        }
    }
    
out:
    return (multicastRouterPortMask & (~pktPortMask) & 0x3f);
}

static uint8 rtl_processLeave(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf)
{
    /* do nothing, just forward this packet to multicast router*/ 
    uint32 timerIndex = 0;
    uint32 groupAddress[4] = {0, 0, 0, 0};
    struct rtl_groupEntry* groupEntry = NULL;
    struct rtl_sourceEntry *sourceEntry = NULL;
    uint8 oldFwdPortMask = 0;
    uint8 newFwdPortMask = 0;

    uint32 hashIndex = 0;
    uint32 portIndex = rtl_mapPortMaskToPortNum(pktPortMask);

    uint8 multicastRouterPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
    
    if(IP_VERSION4 == ipVersion)
    {
        groupAddress[0] = ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
        hashIndex = groupAddress[0] & rtl_hashMask;       
    }
        
    if(IP_VERSION6 == ipVersion)
    {
        groupAddress[0] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
        groupAddress[1] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
        groupAddress[2] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
        groupAddress[3] = ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);
        
        hashIndex = groupAddress[3] & rtl_hashMask;
    }

    groupEntry = rtl_searchGroupEntry(ipVersion, groupAddress);
    /*lower the timer of the group*/
    if(groupEntry != NULL)
    {   
        oldFwdPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);

        if(groupEntry->groupFilterTimer[portIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
        {
            for(timerIndex = 0; timerIndex < 6; timerIndex++)
            { 
                if(groupEntry->groupFilterTimer[timerIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                    groupEntry->groupFilterTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;                     
            }

            if(TRUE == rtl_delPortMaskRevLeave)
            {
                groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds;
            }     
        }

        sourceEntry = groupEntry->sourceList;
        while(sourceEntry)
        {
            if(sourceEntry->portTimer[portIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
            {
                for(timerIndex = 0; timerIndex < 6; timerIndex++)
                {
                    if(sourceEntry->portTimer[timerIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                        sourceEntry->portTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;                            
                }

                if(TRUE == rtl_delPortMaskRevLeave)
                {
                    sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds;
                }                   
            }
            sourceEntry = sourceEntry->next;
        }

        if(TRUE == rtl_delPortMaskRevLeave)
        {    
            newFwdPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);

            if(((oldFwdPortMask != newFwdPortMask) || (FALSE == groupEntry->lookupTableFlag)))
            {   
                rtl_setLookupTable(groupEntry);  /*set rtl8306 lookup table*/
            }
        }    
    }   
            
    return (multicastRouterPortMask & (~pktPortMask) & 0x3f);
}

static  int32 rtl_processIsInclude(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf)
{

    uint32 j = 0;
    uint32 groupAddress[4] = {0, 0, 0, 0};
    struct rtl_groupEntry* groupEntry = NULL;
    struct rtl_groupEntry* newGroupEntry = NULL;
    struct rtl_sourceEntry *sourceEntry = NULL;
    struct rtl_sourceEntry *newSourceEntry = NULL;
    
    uint32 hashIndex = 0;
    uint32 portIndex = rtl_mapPortMaskToPortNum(pktPortMask);
    uint8 aggregatorPortMask = 0;
    uint8 aggregatorLookupTableFlag = 0;

    uint8 oldPortMask = 0;
    uint8 newPortMask = 0;

    uint16 numOfSrc = 0;
    uint32 *sourceAddr = NULL;

    if(IP_VERSION4 == ipVersion)
    {
        groupAddress[0] = ntohl(((struct groupRecord *)pktBuf)->groupAddr);
        numOfSrc = ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
        sourceAddr =& (((struct groupRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[0] & rtl_hashMask;
    }
        
    if(IP_VERSION6 == ipVersion)
    {
        groupAddress[0] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
        groupAddress[1] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
        groupAddress[2] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
        groupAddress[3] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
        
        numOfSrc = ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
        sourceAddr =& (((struct mCastAddrRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[3] & rtl_hashMask;
    }

    groupEntry = rtl_searchGroupEntry(ipVersion, groupAddress);
    if(NULL == groupEntry)   /*means new group address, create new group entry*/
    {
        newGroupEntry = rtl_allocateGroupEntry();
        if(NULL == newGroupEntry)
        {
            rtl_gluePrintf("run out of group entry!\n");
            return FAILED;
        }
        else
        {   
            /*set new multicast entry*/
            newGroupEntry->groupAddr[0] = groupAddress[0];
            newGroupEntry->groupAddr[1] = groupAddress[1];
            newGroupEntry->groupAddr[2] = groupAddress[2];
            newGroupEntry->groupAddr[3] = groupAddress[3];

            newGroupEntry->ipVersion = ipVersion;
            newGroupEntry->sourceList = NULL;
            newGroupEntry->lookupTableFlag = FALSE;
            
            /*end of set group entry*/
            
            /*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
            if(IP_VERSION4 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv4HashTable, hashIndex);
            }

            if(IP_VERSION6 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv6HashTable, hashIndex);
            }

            if(TRUE == rtl_enableSourceList)
            {
                newGroupEntry->groupFilterTimer[portIndex] = 0;
                /*link the new source list*/
                for(j = 0; j < numOfSrc; j++)
                { 
                    newSourceEntry = rtl_allocateSourceEntry();
                    if(NULL == newSourceEntry)
                    {
                        rtl_gluePrintf("run out of source entry!\n");
                        return FAILED;
                    }
                    else
                    {   
                    
                        if(IP_VERSION4 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                        }

                        if(IP_VERSION6 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            newSourceEntry->sourceAddr[1] = sourceAddr[1];
                            newSourceEntry->sourceAddr[2] = sourceAddr[2];
                            newSourceEntry->sourceAddr[3] = sourceAddr[3];
                        }
                        newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                        rtl_linkSourceEntry(newGroupEntry, newSourceEntry);
                    }

                    if(IP_VERSION4 == ipVersion)
                    {   
                        sourceAddr++;
                    }

                    if(IP_VERSION6 == ipVersion)
                    {
                        sourceAddr = sourceAddr + 4;
                    }                  
                }
            
            }
            else
            {   
                newGroupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
            }                   
        }

        /*always try to set new group entry into look up table*/ 
        /*only the new group entry should be checked the aggregator status */
        rtl_checkAggregator(ipVersion, groupAddress, &aggregatorPortMask, &aggregatorLookupTableFlag); 

        rtl_setLookupTable(newGroupEntry);  /*always try to add new group entry into lookup table*/
        
    }
    else /*means it can be found in the forward hash table*/
    {  
        oldPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        if(TRUE == rtl_enableSourceList)
        {
            /*here to handle the source list*/
            for(j = 0; j < numOfSrc; j++)
            {                
                sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
                if(NULL == sourceEntry)
                {
                    newSourceEntry = rtl_allocateSourceEntry();
                    if(NULL == newSourceEntry)
                    {
                        rtl_gluePrintf("run out of source entry!\n");
                        return FAILED;
                    }
                    else
                    {   
                    
                        if(IP_VERSION4 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0]=sourceAddr[0];
                        
                        }
                        if(IP_VERSION6 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0]=sourceAddr[0];
                            newSourceEntry->sourceAddr[1]=sourceAddr[1];
                            newSourceEntry->sourceAddr[2]=sourceAddr[2];
                            newSourceEntry->sourceAddr[3]=sourceAddr[3];
                        }
                        
                        newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                        rtl_linkSourceEntry(groupEntry, newSourceEntry);
                    }

                }
                else
                {       
                    /*just update source timer*/
                    sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                }
                    
                if(IP_VERSION4 == ipVersion)
                {   
                    sourceAddr++;
                }

                if(IP_VERSION6 == ipVersion)
                {
                    sourceAddr = sourceAddr + 4;
                }
                
            }

        }
        else
        {
            groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
        }
        
        /*check whether need to set lookup table*/  
        newPortMask=rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        /*always try to add new portmask and rewrite those that were not in lookup table*/
        if((groupEntry != NULL) && ((oldPortMask != newPortMask) || (FALSE == groupEntry->lookupTableFlag)))
        {
            rtl_setLookupTable(groupEntry);  
        }

    }
    
    return SUCCESS;
}

static  int32 rtl_processIsExclude(uint32 ipVersion,uint8 pktPortMask, uint8 *pktBuf)
{
    uint32 j = 0;
    uint32 groupAddress[4] = {0, 0, 0, 0};
    struct rtl_groupEntry* groupEntry = NULL;
    struct rtl_groupEntry* newGroupEntry = NULL;
    struct rtl_sourceEntry *sourceEntry = NULL;
    struct rtl_sourceEntry *newSourceEntry = NULL;
    
    uint32 hashIndex = 0;
    uint32 portIndex = rtl_mapPortMaskToPortNum(pktPortMask);
    uint8 aggregatorPortMask = 0;
    uint8 aggregatorLookupTableFlag = 0;

    uint8 oldPortMask = 0;
    uint8 newPortMask = 0;
    
    uint16 numOfSrc = 0;
    uint32 *sourceArray = NULL;
    uint32 *sourceAddr = NULL;

    if(IP_VERSION4 == ipVersion)
    {
        groupAddress[0] = ntohl(((struct groupRecord *)pktBuf)->groupAddr);
        numOfSrc = ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
        sourceArray =& (((struct groupRecord *)pktBuf)->srcList);
        sourceAddr =& (((struct groupRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[0] & rtl_hashMask;
    }
        
    if(IP_VERSION6 == ipVersion)
    {
        
        groupAddress[0] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
        groupAddress[1] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
        groupAddress[2] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
        groupAddress[3] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
        
        numOfSrc = ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
        sourceArray =& (((struct mCastAddrRecord *)pktBuf)->srcList);
        sourceAddr =& (((struct mCastAddrRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[3] & rtl_hashMask;    
    }

    groupEntry = rtl_searchGroupEntry(ipVersion, groupAddress);
    if(NULL == groupEntry)   /*means new group address, create new group entry*/
    {
        newGroupEntry = rtl_allocateGroupEntry();
        if(NULL == newGroupEntry)
        {
            rtl_gluePrintf("run out of group entry!\n");
            return FAILED;
        }
        else
        {   
            /*set new multicast entry*/     
            newGroupEntry->groupAddr[0] = groupAddress[0];
            newGroupEntry->groupAddr[1] = groupAddress[1];
            newGroupEntry->groupAddr[2] = groupAddress[2];
            newGroupEntry->groupAddr[3] = groupAddress[3];

            newGroupEntry->ipVersion = ipVersion;
            newGroupEntry->sourceList = NULL;
            /*means the filter mode is exclude*/
            newGroupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;

            newGroupEntry->lookupTableFlag = FALSE;
            /*end of set group entry*/
            
            /*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
            if(IP_VERSION4 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv4HashTable, hashIndex);
            }

            if(IP_VERSION6 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv6HashTable, hashIndex);
            }

            if(TRUE == rtl_enableSourceList)
            {
                /*link the new source list*/
                for(j = 0; j < numOfSrc; j++)
                {
                    newSourceEntry = rtl_allocateSourceEntry();
                    if(NULL == newSourceEntry)
                    {
                        rtl_gluePrintf("run out of source entry!\n");
                        return FAILED;
                    }
                    else
                    {                       
                        if(IP_VERSION4 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0]=sourceAddr[0];
                        }

                        if(IP_VERSION6 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0]=sourceAddr[0];
                            newSourceEntry->sourceAddr[1]=sourceAddr[1];
                            newSourceEntry->sourceAddr[2]=sourceAddr[2];
                            newSourceEntry->sourceAddr[3]=sourceAddr[3];
                        }
                        
                        /*time out the sources included in the MODE_IS_EXCLUDE report*/
                        newSourceEntry->portTimer[portIndex]=rtl_sysUpSeconds;
                        rtl_linkSourceEntry(newGroupEntry,newSourceEntry);
                    }

                    if(IP_VERSION4 == ipVersion)
                    {   
                        sourceAddr++;
                    }

                    if(IP_VERSION6 == ipVersion)
                    {
                        sourceAddr = sourceAddr + 4;
                    }                 
                }
            }                
        }

       /*Always try to set new group entry into look up table,
        *only the new group entry should be checked the aggregator status 
        */
        rtl_checkAggregator(ipVersion,groupAddress,&aggregatorPortMask,&aggregatorLookupTableFlag);
        rtl_setLookupTable(newGroupEntry);  /*always try to add new group entry into lookup table*/
    }
    else /*means group address can be found in the forward hash table*/
    {  
        oldPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        if(groupEntry->groupFilterTimer[portIndex] <= rtl_sysUpSeconds) /*means include mode*/
        {
            if(TRUE == rtl_enableSourceList)
            {
                /*here to handle the source list*/

                /*delete (A-B)*/
                sourceEntry = groupEntry->sourceList;
                while(sourceEntry)
                {
                    if(rtl_searchSourceAddr(ipVersion,sourceEntry->sourceAddr, sourceArray, numOfSrc) == FALSE)
                    {
                        sourceEntry->portTimer[portIndex] = 0;
                    }
                    sourceEntry = sourceEntry->next;
                }

                /*(B-A) time out*/
                for(j = 0; j < numOfSrc; j++)
                {                    
                    sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
                    
                    if(NULL == sourceEntry)
                    {
                        newSourceEntry = rtl_allocateSourceEntry();
                        if(NULL == newSourceEntry)
                        {
                            rtl_gluePrintf("run out of source entry!\n");
                            return FAILED;
                        }
                        else
                        {                           
                            if(IP_VERSION4 == ipVersion)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            
                            }

                            if(IP_VERSION6 == ipVersion)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];
                                newSourceEntry->sourceAddr[1] = sourceAddr[1];
                                newSourceEntry->sourceAddr[2] = sourceAddr[2];
                                newSourceEntry->sourceAddr[3] = sourceAddr[3];
                            }
                            newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds;
                            rtl_linkSourceEntry(groupEntry, newSourceEntry);
                        }
                    }
                    else
                    {
                        if(0 == sourceEntry->portTimer[portIndex] )
                        {
                            sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds;
                        }
                    }
                
                    if(IP_VERSION4 == ipVersion)
                    {   
                        sourceAddr++;
                    }

                    if(IP_VERSION6 == ipVersion)
                    {
                        sourceAddr = sourceAddr + 4;
                    }                    
                }
            }            
        }
        else/*means exclude mode*/
        {
            if(TRUE == rtl_enableSourceList)
            {
               /*here to handle the source list
                *delete (X-A) and delete (Y-A)
                */
                sourceEntry = groupEntry->sourceList;
                while(sourceEntry)
                {
                    if(rtl_searchSourceAddr(ipVersion,sourceEntry->sourceAddr, sourceArray, numOfSrc) == FALSE)
                    {
                        sourceEntry->portTimer[portIndex] = 0;
                    }
                    sourceEntry = sourceEntry->next;
                }

                /*A-X-Y=GMI*/
                for(j = 0; j < numOfSrc; j++)
                {
                    sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
                    
                    if(NULL == sourceEntry)
                    {
                        newSourceEntry = rtl_allocateSourceEntry();
                        if(NULL == newSourceEntry)
                        {
                            rtl_gluePrintf("run out of source entry!\n");
                            return FAILED;
                        }
                        else
                        {                        
                            if(IP_VERSION4 == ipVersion)
                            {   
                                newSourceEntry->sourceAddr[0]= sourceAddr[0];                            
                            }

                            if(IP_VERSION6 == ipVersion)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];
                                newSourceEntry->sourceAddr[1] = sourceAddr[1];
                                newSourceEntry->sourceAddr[2] = sourceAddr[2];
                                newSourceEntry->sourceAddr[3] = sourceAddr[3];
                            }
                                                    
                            newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                            rtl_linkSourceEntry(groupEntry, newSourceEntry);
                        }

                    }
                    else
                    {
                        if(0 == sourceEntry->portTimer[portIndex])
                        {
                            sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                        }
                    }
                    
                    if(IP_VERSION4 == ipVersion)
                    {   
                        sourceAddr++;
                    }

                    if(IP_VERSION6 == ipVersion)
                    {
                        sourceAddr = sourceAddr + 4;
                    }
                    
                }
            }
        }

        /*switch to exclude mode or update exclude mode filter timer*/
        groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
    
        /*check whether need to set lookup table*/  
        newPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        /*always try to add new portmask and rewrite those that were not in lookup table*/
        if((groupEntry != NULL) && ((oldPortMask != newPortMask) || (FALSE == groupEntry->lookupTableFlag)))
        {
            rtl_setLookupTable(groupEntry);  
        }

    }

    return SUCCESS;

}

static int32 rtl_processToInclude(uint32 ipVersion,  uint8 pktPortMask, uint8 *pktBuf)
{
    uint32 timerIndex;  
    uint32 j = 0;
    uint32 groupAddress[4] = {0, 0, 0, 0};
    struct rtl_groupEntry* groupEntry = NULL;
    struct rtl_groupEntry* newGroupEntry = NULL;
    struct rtl_sourceEntry *sourceEntry = NULL;
    struct rtl_sourceEntry *newSourceEntry = NULL;

    uint32 hashIndex = 0;
    uint32 portIndex = rtl_mapPortMaskToPortNum(pktPortMask);
    uint8 aggregatorPortMask = 0;
    uint8 aggregatorLookupTableFlag = 0;

    uint8 oldPortMask = 0;
    uint8 newPortMask = 0;

    uint16 numOfSrc = 0;
    uint32 *sourceAddr = NULL;

    if(IP_VERSION4 == ipVersion)
    {
        groupAddress[0] = ntohl(((struct groupRecord *)pktBuf)->groupAddr);
        numOfSrc = ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
        sourceAddr =& (((struct groupRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[0] & rtl_hashMask;
    }
        
    if(IP_VERSION6 == ipVersion)
    {        
        groupAddress[0] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
        groupAddress[1] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
        groupAddress[2] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
        groupAddress[3] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
        
        numOfSrc = ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
        sourceAddr =& (((struct mCastAddrRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[3] & rtl_hashMask;
    
    }

    groupEntry = rtl_searchGroupEntry(ipVersion, groupAddress);
    if(NULL == groupEntry)   /*means new group address, create new group entry*/
    {
        newGroupEntry = rtl_allocateGroupEntry();
        if(NULL == newGroupEntry)
        {
            rtl_gluePrintf("run out of group entry!\n");
            return FAILED;
        }
        else
        {   
            /*set new multicast entry*/            
            newGroupEntry->groupAddr[0] = groupAddress[0];
            newGroupEntry->groupAddr[1] = groupAddress[1];
            newGroupEntry->groupAddr[2] = groupAddress[2];
            newGroupEntry->groupAddr[3] = groupAddress[3];

            newGroupEntry->ipVersion = ipVersion;
            newGroupEntry->sourceList = NULL;
                        
            newGroupEntry->lookupTableFlag = FALSE;
            
            /*end of set group entry*/
            
            /*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
            if(IP_VERSION4 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv4HashTable, hashIndex);
            }

            if(IP_VERSION6 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv6HashTable, hashIndex);
            }

            if(TRUE == rtl_enableSourceList)
            {
                newGroupEntry->groupFilterTimer[portIndex] = 0;
                /*link the new source list*/
                for(j = 0; j < numOfSrc; j++)
                {                   
                    newSourceEntry = rtl_allocateSourceEntry();
                    if(NULL == newSourceEntry)
                    {
                        rtl_gluePrintf("run out of source entry!\n");
                        return FAILED;
                    }
                    else
                    {                    
                        if(IP_VERSION4 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            
                        }

                        if(IP_VERSION6 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            newSourceEntry->sourceAddr[1] = sourceAddr[1];
                            newSourceEntry->sourceAddr[2] = sourceAddr[2];
                            newSourceEntry->sourceAddr[3] = sourceAddr[3];
                        }
                        newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                        rtl_linkSourceEntry(newGroupEntry, newSourceEntry);
                    }
                        
                    if(IP_VERSION4 == ipVersion)
                    {   
                        sourceAddr++;
                    }

                    if(IP_VERSION6 == ipVersion)
                    {
                        sourceAddr = sourceAddr + 4;
                    }
                    
                }
            }
            else
            {   
                newGroupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
            }                       
        }

        /*always try to set new group entry into look up table*/
        
        /*only the new group entry should be checked the aggregator status */
        rtl_checkAggregator(ipVersion, groupAddress, &aggregatorPortMask, &aggregatorLookupTableFlag);
        rtl_setLookupTable(newGroupEntry);  /*always try to add new group entry into lookup table*/
        
    }
    else /*means it can be found in the forward hash table*/
    {  
        oldPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        if(TRUE == rtl_enableSourceList)
        {
            /*here to handle the source list*/           
            for(j = 0; j < numOfSrc; j++)
            {
                sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
        
                if(NULL == sourceEntry)
                {
                    newSourceEntry = rtl_allocateSourceEntry();
                    if(NULL == newSourceEntry)
                    {
                        rtl_gluePrintf("run out of source entry!\n");
                        return FAILED;
                    }
                    else
                    {   
                    
                        if(IP_VERSION4 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0]=sourceAddr[0];
                        
                        }

                        if(IP_VERSION6 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            newSourceEntry->sourceAddr[1] = sourceAddr[1];
                            newSourceEntry->sourceAddr[2] = sourceAddr[2];
                            newSourceEntry->sourceAddr[3] = sourceAddr[3];
                        }
                        
                        newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                        rtl_linkSourceEntry(groupEntry, newSourceEntry);
                    }

                }
                else
                {       
                    /*just update source timer*/
                    sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;        
                }
                    
                if(IP_VERSION4 == ipVersion)
                {   
                    sourceAddr++;
                }

                if(IP_VERSION6 == ipVersion)
                {
                    sourceAddr=sourceAddr+4;
                }
                
            }

            sourceEntry = groupEntry->sourceList;

            if(IP_VERSION4 == ipVersion)
            {
                sourceAddr =& (((struct groupRecord *)pktBuf)->srcList);     
            }

            if(IP_VERSION6 == ipVersion)
            {         
                sourceAddr =& (((struct mCastAddrRecord *)pktBuf)->srcList);
            }

            while(sourceEntry)
            {                 
                if((rtl_searchSourceAddr(ipVersion, sourceEntry->sourceAddr, sourceAddr, numOfSrc)==FALSE)&&(sourceEntry->portTimer[portIndex]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)))
                {
                    for(timerIndex = 0; timerIndex < 6; timerIndex++)
                    {
                        if(sourceEntry->portTimer[timerIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                        {
                            sourceEntry->portTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;                        
                        }
                    } 
                }
                sourceEntry = sourceEntry->next;
            }
                
            if(groupEntry->groupFilterTimer[portIndex] > rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime)   /*lower the group timer if in exclude mode*/
            {
                for(timerIndex = 0; timerIndex < 6; timerIndex++)
                {
                    if(groupEntry->groupFilterTimer[timerIndex] > rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime)   
                    {
                        groupEntry->groupFilterTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                    }
                }
            }

            if((0 == numOfSrc) && (TRUE == rtl_delPortMaskRevLeave))
            {
                if(groupEntry->groupFilterTimer[portIndex] > rtl_sysUpSeconds)
                {
                    groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds;
                }

                sourceEntry = groupEntry->sourceList;

                while(sourceEntry)
                {
                    if(sourceEntry->portTimer[portIndex] > rtl_sysUpSeconds)
                    {
                        sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds;
                    }
                    sourceEntry = sourceEntry->next;
                }
            }
        }
        else
        {   
            if((0 == numOfSrc) && (groupEntry->groupFilterTimer[portIndex] > rtl_sysUpSeconds))
            {
                if(TRUE == rtl_delPortMaskRevLeave)
                {
                    groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds;
                }
                else
                {
                    groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                }           
            }
            else
            {
                groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
            }
        }
        
        /*check whether need to set lookup table*/  
        newPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        
        /*always try to add new portmask and rewrite those that were not in lookup table*/
        if((groupEntry != NULL) && ((oldPortMask!=newPortMask) || (groupEntry->lookupTableFlag == FALSE)))
        {
            rtl_setLookupTable(groupEntry);
        }
    }

    return SUCCESS;
    
}

static  int32 rtl_processToExclude(uint32 ipVersion,uint8 pktPortMask, uint8 *pktBuf)
{
    uint32 j = 0;
    uint32 timerIndex;
    uint32 groupAddress[4]= {0, 0, 0, 0};
    struct rtl_groupEntry* groupEntry = NULL;
    struct rtl_groupEntry* newGroupEntry = NULL;
    struct rtl_sourceEntry *sourceEntry = NULL;
    struct rtl_sourceEntry *newSourceEntry = NULL;
    
    uint32 hashIndex = 0;
    uint32 portIndex = rtl_mapPortMaskToPortNum(pktPortMask);
    uint8 aggregatorPortMask = 0;
    uint8 aggregatorLookupTableFlag = 0;

    uint8 oldPortMask = 0;
    uint8 newPortMask = 0;
    
    uint16 numOfSrc = 0;
    uint32 *sourceArray = NULL;
    uint32 *sourceAddr = NULL;

    if(ipVersion == IP_VERSION4)
    {
        groupAddress[0] = ntohl(((struct groupRecord *)pktBuf)->groupAddr);
        numOfSrc = ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
        sourceArray =& (((struct groupRecord *)pktBuf)->srcList);
        sourceAddr =& (((struct groupRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[0] & rtl_hashMask;
    }
        
    if(ipVersion == IP_VERSION6)
    {        
        groupAddress[0] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
        groupAddress[1] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
        groupAddress[2] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
        groupAddress[3] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
        
        numOfSrc = ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
        sourceArray =& (((struct mCastAddrRecord *)pktBuf)->srcList);
        sourceAddr =& (((struct mCastAddrRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[3] & rtl_hashMask;    
    }

    groupEntry = rtl_searchGroupEntry(ipVersion, groupAddress);
    if(groupEntry == NULL)   /*means new group address, create new group entry*/
    {
        newGroupEntry = rtl_allocateGroupEntry();
        if(newGroupEntry == NULL)
        {
            rtl_gluePrintf("run out of group entry!\n");
            return FAILED;
        }
        else
        {   
            /*set new multicast entry*/
            newGroupEntry->groupAddr[0] = groupAddress[0];
            newGroupEntry->groupAddr[1] = groupAddress[1];
            newGroupEntry->groupAddr[2] = groupAddress[2];
            newGroupEntry->groupAddr[3] = groupAddress[3];

            newGroupEntry->ipVersion = ipVersion;
            newGroupEntry->sourceList = NULL;
            
            /*means the filter mode is exclude*/
            newGroupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
        
            newGroupEntry->lookupTableFlag = FALSE;
            /*end of set group entry*/
            
            /*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
            if(ipVersion == IP_VERSION4)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv4HashTable, hashIndex);
            }

            if(ipVersion == IP_VERSION6)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv6HashTable, hashIndex);
            }

            if(rtl_enableSourceList == TRUE)
            {
                /*link the new source list*/
                for(j = 0; j < numOfSrc; j++)
                {
                    newSourceEntry = rtl_allocateSourceEntry();
                    if(newSourceEntry == NULL)
                    {
                        rtl_gluePrintf("run out of source entry!\n");
                        return FAILED;
                    }
                    else
                    {   
                    
                        if(ipVersion == IP_VERSION4)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                        }

                        if(ipVersion == IP_VERSION6)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            newSourceEntry->sourceAddr[1] = sourceAddr[1];
                            newSourceEntry->sourceAddr[2] = sourceAddr[2];
                            newSourceEntry->sourceAddr[3] = sourceAddr[3];
                        }
                        
                        /*time out the sources included  in the TO_EXCLUDE report */
                        newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds;
                        rtl_linkSourceEntry(newGroupEntry, newSourceEntry);
                    }
                        
                    if(ipVersion == IP_VERSION4)
                    {   
                        sourceAddr++;
                    }

                    if(ipVersion == IP_VERSION6)
                    {
                        sourceAddr = sourceAddr + 4;
                    }
                    
                }

            }
            
            
        }

        /*always try to set new group entry into look up table*/
        
        /*only the new group entry should be checked the aggregator status */
        rtl_checkAggregator(ipVersion, groupAddress, &aggregatorPortMask, &aggregatorLookupTableFlag);
        rtl_setLookupTable(newGroupEntry);  /*always try to add new group entry into lookup table*/    
    }
    else /*means group address can be found in the forward hash table*/
    {  
        oldPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        
        if(groupEntry->groupFilterTimer[portIndex] <= rtl_sysUpSeconds) /*means include mode*/
        {
            if(rtl_enableSourceList == TRUE)
            {
                /*here to handle the source list
                 *delete (A-B)
                 */
                sourceEntry = groupEntry->sourceList;
                while(sourceEntry)
                {
                    if(rtl_searchSourceAddr(ipVersion, sourceEntry->sourceAddr, sourceArray, numOfSrc) ==  FALSE)
                    {
                        sourceEntry->portTimer[portIndex] = 0;
                    }
                    sourceEntry = sourceEntry->next;
                }

                /*(B-A) time out*/
                for(j = 0; j < numOfSrc; j++)
                {
                    sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
                    
                    if(sourceEntry == NULL)
                    {
                        newSourceEntry = rtl_allocateSourceEntry();
                        if(newSourceEntry == NULL)
                        {
                            rtl_gluePrintf("run out of source entry!\n");
                            return FAILED;
                        }
                        else
                        {                           
                            if(ipVersion == IP_VERSION4)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            
                            }

                            if(ipVersion == IP_VERSION6)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];
                                newSourceEntry->sourceAddr[1] = sourceAddr[1];
                                newSourceEntry->sourceAddr[2] = sourceAddr[2];
                                newSourceEntry->sourceAddr[3] = sourceAddr[3];
                            }
                        
                            /*B-A time out*/
                            newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds;
                            rtl_linkSourceEntry(groupEntry, newSourceEntry);
                        }

                    }
                    else/*maybe include redundant sources*/
                    {
                        /*B-A time out*/
                        if(sourceEntry->portTimer[portIndex] == 0)
                        {
                            sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds;
                        }
                        if(sourceEntry->portTimer[portIndex] > rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime)  /*lower the A*B timer if the cpu is router*/
                             {
                            for(timerIndex = 0; timerIndex < 6; timerIndex++)
                            {
                                if(sourceEntry->portTimer[timerIndex] >( rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                                {
                                    sourceEntry->portTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                                }
                            }
                        }
                    }

                    if(ipVersion == IP_VERSION4)
                    {   
                        sourceAddr++;
                    }

                    if(ipVersion == IP_VERSION6)
                    {
                        sourceAddr = sourceAddr + 4;
                    }                    
                }
            }               
        }
        else/*means exclude mode*/
        {
            if(rtl_enableSourceList == TRUE)
            {
                /*here to handle the source list
                 *delete (X-A) and delete (Y-A)
                 */
                sourceEntry = groupEntry->sourceList;
                while(sourceEntry)
                {
                    if(rtl_searchSourceAddr(ipVersion, sourceEntry->sourceAddr, sourceArray, numOfSrc) == FALSE)
                    {
                        sourceEntry->portTimer[portIndex] = 0;
                    }
                    sourceEntry = sourceEntry->next;
                }

                /*A-X-Y=filter timer*/
                for(j = 0; j < numOfSrc; j++)
                {
                    sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
                    
                    if(sourceEntry == NULL)
                    {
                        newSourceEntry = rtl_allocateSourceEntry();
                        if(newSourceEntry == NULL)
                        {
                            rtl_gluePrintf("run out of source entry!\n");
                            return FAILED;
                        }
                        else
                        {                       
                            if(ipVersion == IP_VERSION4)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            }

                            if(ipVersion == IP_VERSION6)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];
                                newSourceEntry->sourceAddr[1] = sourceAddr[1];
                                newSourceEntry->sourceAddr[2] = sourceAddr[2];
                                newSourceEntry->sourceAddr[3] = sourceAddr[3];    
                            }                           
                            newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                            rtl_linkSourceEntry(groupEntry, newSourceEntry);                                      
                        }
                    }
                    else
                    {   
                        if((sourceEntry->portTimer[portIndex] == 0) || sourceEntry->portTimer[portIndex] > rtl_sysUpSeconds)                                                          
                        {
                            sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                            for(timerIndex = 0; timerIndex < 6; timerIndex++)
                            {
                                if(sourceEntry->portTimer[timerIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                                {
                                    sourceEntry->portTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                                }
                            }
                        }                           
                    }
                
                    if(ipVersion == IP_VERSION4)
                    {   
                        sourceAddr++;
                    }

                    if(ipVersion == IP_VERSION6)
                    {
                        sourceAddr = sourceAddr + 4;
                    }                    
                }
            }                    
        }

        /*switch to exclude mode or update exclude mode filter timer*/
        groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
        
        /*check whether need to set lookup table*/  
        newPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        /*always try to add new portmask and rewrite those that were not in lookup table*/
        if((groupEntry != NULL) && ((oldPortMask != newPortMask) || (groupEntry->lookupTableFlag == FALSE)))
        {
            rtl_setLookupTable(groupEntry);  
        }

    }

    return SUCCESS;
}

static  int32 rtl_processAllow(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf)
{
    uint32 j = 0;
    uint32 groupAddress[4] = {0, 0, 0, 0};
    struct rtl_groupEntry* groupEntry = NULL;
    struct rtl_groupEntry* newGroupEntry = NULL;
    struct rtl_sourceEntry *sourceEntry = NULL;
    struct rtl_sourceEntry *newSourceEntry = NULL;
    
    uint32 hashIndex = 0;
    uint32 portIndex = rtl_mapPortMaskToPortNum(pktPortMask);
    uint8 aggregatorPortMask = 0;
    uint8 aggregatorLookupTableFlag = 0;

    uint8 oldPortMask = 0;
    uint8 newPortMask = 0;

    uint16 numOfSrc = 0;
    uint32 *sourceAddr = NULL;
    
    if(ipVersion == IP_VERSION4)
    {
        groupAddress[0] = ntohl(((struct groupRecord *)pktBuf)->groupAddr);
        numOfSrc = ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
        sourceAddr =& (((struct groupRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[0] & rtl_hashMask;
    }
        
    if(ipVersion == IP_VERSION6)
    {        
        groupAddress[0] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
        groupAddress[1] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
        groupAddress[2] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
        groupAddress[3] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
        
        numOfSrc = ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
        sourceAddr =& (((struct mCastAddrRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[3] & rtl_hashMask;    
    }

    groupEntry=rtl_searchGroupEntry(ipVersion, groupAddress);
    if(groupEntry == NULL)   /*means new group address, create new group entry*/
    {
        newGroupEntry = rtl_allocateGroupEntry();
        if(newGroupEntry == NULL)
        {
            rtl_gluePrintf("run out of group entry!\n");
            return FAILED;
        }
        else
        {   
            /*set new multicast entry*/
            newGroupEntry->groupAddr[0] = groupAddress[0];
            newGroupEntry->groupAddr[1] = groupAddress[1];
            newGroupEntry->groupAddr[2] = groupAddress[2];
            newGroupEntry->groupAddr[3] = groupAddress[3];

            newGroupEntry->ipVersion = ipVersion;
            newGroupEntry->sourceList = NULL;
               
            newGroupEntry->lookupTableFlag = FALSE;
            
            /*end of set group entry*/
            
            /*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
            if(IP_VERSION4 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv4HashTable, hashIndex);
            }

            if(IP_VERSION6 == ipVersion)
            {
                rtl_linkGroupEntry(newGroupEntry, rtl_ipv6HashTable, hashIndex);
            }

            if(TRUE == rtl_enableSourceList)
            {
                newGroupEntry->groupFilterTimer[portIndex] = 0;
                /*link the new source list*/
                for(j = 0; j < numOfSrc; j++)
                {                    
                    newSourceEntry = rtl_allocateSourceEntry();
                    if(NULL == newSourceEntry)
                    {
                        rtl_gluePrintf("run out of source entry!\n");
                        return FAILED;
                    }
                    else
                    {                     
                        if(IP_VERSION4 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                        }

                        if(IP_VERSION6 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            newSourceEntry->sourceAddr[1] = sourceAddr[1];
                            newSourceEntry->sourceAddr[2] = sourceAddr[2];
                            newSourceEntry->sourceAddr[3] = sourceAddr[3];
                        }
                        newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                        rtl_linkSourceEntry(newGroupEntry, newSourceEntry);
                    }
                    
                    if(IP_VERSION4 == ipVersion)
                    {   
                        sourceAddr++;
                    }

                    if(IP_VERSION6 == ipVersion)
                    {
                        sourceAddr = sourceAddr + 4;
                    }
                    
                }
            }
            else
            {
                newGroupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;;
            }
    
        }

        /*always try to set new group entry into look up table,
         *only the new group entry should be checked the aggregator status 
         */
        rtl_checkAggregator(ipVersion,groupAddress, &aggregatorPortMask, &aggregatorLookupTableFlag);
        rtl_setLookupTable(newGroupEntry);  /*always try to add new group entry into lookup table*/
        
    }
    else /*means it can be found in the forward hash table*/
    {  
        oldPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        if(TRUE == rtl_enableSourceList)
        {
            /*here to handle the source list*/
            for(j = 0; j < numOfSrc; j++)
            {         
                sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
            
                if(NULL == sourceEntry )
                {
                    newSourceEntry = rtl_allocateSourceEntry();
                    if(NULL == newSourceEntry)
                    {
                        rtl_gluePrintf("run out of source entry!\n");
                        return FAILED;
                    }
                    else
                    {                   
                        if(IP_VERSION4 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                        }

                        if(IP_VERSION6 == ipVersion)
                        {   
                            newSourceEntry->sourceAddr[0] = sourceAddr[0];
                            newSourceEntry->sourceAddr[1] = sourceAddr[1];
                            newSourceEntry->sourceAddr[2] = sourceAddr[2];
                            newSourceEntry->sourceAddr[3] = sourceAddr[3];
                        }
                        newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;
                        rtl_linkSourceEntry(groupEntry, newSourceEntry);
                    }

                }
                else
                {       
                    /*just update source timer*/
                    sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;        
                }
                    
                if(IP_VERSION4 == ipVersion)
                {   
                    sourceAddr++;
                }

                if(IP_VERSION6 == ipVersion)
                {
                    sourceAddr = sourceAddr + 4;
                }
                
            }
        }
        else
        {
            groupEntry->groupFilterTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.groupMemberAgingTime;;
        }

        /*check whether need to set lookup table*/  
        newPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        /*always try to add new portmask and rewrite those that were not in lookup table*/
        if((groupEntry != NULL) && ((oldPortMask != newPortMask) || (groupEntry->lookupTableFlag == FALSE)))
        {
            rtl_setLookupTable(groupEntry);  
        }

    }

    return SUCCESS;
}

static int32 rtl_processBlock(uint32 ipVersion,uint8 pktPortMask, uint8 *pktBuf)
{
    uint32 j = 0;
    uint32 timerIndex=0;
    uint32 groupAddress[4]= {0, 0, 0, 0};

    struct rtl_groupEntry* groupEntry = NULL;
    struct rtl_sourceEntry *sourceEntry = NULL;
    struct rtl_sourceEntry *newSourceEntry = NULL;
    
    uint32 hashIndex = 0;
    uint32 portIndex = rtl_mapPortMaskToPortNum(pktPortMask);

    uint8 oldPortMask = 0;
    uint8 newPortMask = 0;
    
    uint16 numOfSrc = 0;
    uint32 *sourceAddr = NULL;

    if(IP_VERSION4 == ipVersion)
    {
        groupAddress[0] = ntohl(((struct groupRecord *)pktBuf)->groupAddr);
        numOfSrc = ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
        sourceAddr = &(((struct groupRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[0] & rtl_hashMask;
    }
        
    if(IP_VERSION6 == ipVersion)
    {        
        groupAddress[0] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
        groupAddress[1] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
        groupAddress[2] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
        groupAddress[3] = ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);

        numOfSrc = ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
        sourceAddr =& (((struct mCastAddrRecord *)pktBuf)->srcList);
        hashIndex = groupAddress[3] & rtl_hashMask;    
    }

    groupEntry=rtl_searchGroupEntry(ipVersion, groupAddress);
    if(groupEntry != NULL)
    {  
        oldPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
                
        if(groupEntry->groupFilterTimer[portIndex] > rtl_sysUpSeconds) /*means exclude mode*/
        {
            if(TRUE == rtl_enableSourceList)
            {
                /*here to handle the source list*/
                /*A-X-Y=filter timer*/
                for(j = 0; j < numOfSrc; j++)
                {                   
                    sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
                
                    if(NULL == sourceEntry)
                    {
                        newSourceEntry = rtl_allocateSourceEntry();
                        if(NULL == newSourceEntry)
                        {
                            rtl_gluePrintf("run out of source entry!\n");
                            return FAILED;
                        }
                        else
                        {                       
                            if(IP_VERSION4 == ipVersion)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];                            
                            }

                            if(IP_VERSION6 == ipVersion)
                            {   
                                newSourceEntry->sourceAddr[0] = sourceAddr[0];
                                newSourceEntry->sourceAddr[1] = sourceAddr[1];
                                newSourceEntry->sourceAddr[2] = sourceAddr[2];
                                newSourceEntry->sourceAddr[3] = sourceAddr[3];
                            }
                                                                        
                            if(groupEntry->groupFilterTimer[portIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                            {
                                newSourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                            }
                            else
                            {
                                newSourceEntry->portTimer[portIndex] = groupEntry->groupFilterTimer[portIndex];
                            }
                            rtl_linkSourceEntry(groupEntry,newSourceEntry);
                            
                        }

                    }
                    else
                    {
                        if((sourceEntry->portTimer[portIndex] == 0) || (sourceEntry->portTimer[portIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime)))
                        {
                            sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                            for(timerIndex = 0; timerIndex < 6; timerIndex++)
                            {
                                if(sourceEntry->portTimer[timerIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                                {
                                    sourceEntry->portTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                                }
                            }
                        }                    
                    }
                
                    if(IP_VERSION4 == ipVersion)
                    {   
                        sourceAddr++;
                    }

                    if(IP_VERSION6 == ipVersion)
                    {
                        sourceAddr = sourceAddr + 4;
                    }                    
                }                           
            }
        }
        else    /*means include mode*/
        {
            if(TRUE == rtl_enableSourceList)     /*process the special-group query if the cpu is router*/
            {
                for(j = 0; j < numOfSrc; j++)
                {
                    sourceEntry = rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
                    if((sourceEntry != NULL) && (sourceEntry->portTimer[portIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime)))
                    {
                        for(timerIndex = 0; timerIndex < 6; timerIndex++)
                        {
                            if(sourceEntry->portTimer[timerIndex] > (rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime))
                            {
                                sourceEntry->portTimer[timerIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.lastMemberAgingTime;
                            }
                        }
                        if(TRUE == rtl_delPortMaskRevLeave)
                        {
                            sourceEntry->portTimer[portIndex] = rtl_sysUpSeconds;
                        }
                    }   

                    if(IP_VERSION4 == ipVersion)
                    {   
                        sourceAddr++;
                    }

                    if(IP_VERSION6 == ipVersion)
                    {
                        sourceAddr = sourceAddr + 4;
                    }
                }
            }
        }
    
        /*check whether need to set lookup table*/  
        newPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);
        /*always try to add new portmask and rewrite those that were not in lookup table*/
        if((groupEntry != NULL) && ((oldPortMask != newPortMask) || (groupEntry->lookupTableFlag == FALSE)))
        {
            rtl_setLookupTable(groupEntry);  
        }
    }

    return SUCCESS;

}


static uint8 rtl_processIgmpv3Mldv2Reports(uint32 ipVersion, uint8 pktPortMask, uint8 *pktBuf)
{
    uint32 i = 0;
    uint16 numOfRecords = 0;
    uint8 *groupRecords = NULL;
    uint8 recordType = 0xff;
    uint16 numOfSrc = 0;
    int32 returnVal = 0;
    uint8 multicastRouterPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);

    if(IP_VERSION4 == ipVersion)
    {
        numOfRecords = ntohs(((struct igmpv3Report *)pktBuf)->numOfRecords);
        if(numOfRecords != 0)
        {
            groupRecords = (uint8 *)(&(((struct igmpv3Report *)pktBuf)->recordList));
        }
    }
    
    if(IP_VERSION6 == ipVersion)
    {   
        numOfRecords = ntohs(((struct mldv2Report *)pktBuf)->numOfRecords);
        if(numOfRecords != 0)
        {
            groupRecords = (uint8 *)(&(((struct mldv2Report *)pktBuf)->recordList));
        }
    }

    
    for(i = 0; i < numOfRecords;  i++)
    {
        if(IP_VERSION4 == ipVersion)
        {
            recordType = ((struct groupRecord *)groupRecords)->type;
        }
        
        if(IP_VERSION6 == ipVersion)
        {
            recordType = ((struct mCastAddrRecord *)groupRecords)->type;
        }
          
        switch(recordType)
        {
            case MODE_IS_INCLUDE:
                returnVal = rtl_processIsInclude(ipVersion, pktPortMask, groupRecords);
                break;
            
            case MODE_IS_EXCLUDE:
                returnVal = rtl_processIsExclude(ipVersion, pktPortMask, groupRecords);
                break;
            
            case CHANGE_TO_INCLUDE_MODE:
                returnVal = rtl_processToInclude(ipVersion, pktPortMask, groupRecords);
                break;
            
            case CHANGE_TO_EXCLUDE_MODE:
                returnVal = rtl_processToExclude(ipVersion, pktPortMask, groupRecords);
                break;
            
            case ALLOW_NEW_SOURCES:
                returnVal = rtl_processAllow(ipVersion, pktPortMask, groupRecords);
                break;
            
            case BLOCK_OLD_SOURCES:
                returnVal = rtl_processBlock(ipVersion, pktPortMask, groupRecords);
                break;
            
            default:
                break;      
        }

        if(IP_VERSION4 == ipVersion)
        {
            numOfSrc = ntohs(((struct groupRecord *)groupRecords)->numOfSrc);
            /*shift pointer to another group record*/
            groupRecords = groupRecords + 8 + numOfSrc * 4 + (((struct groupRecord *)(groupRecords))->auxLen) * 4;
        }
        
        if(IP_VERSION6 == ipVersion)
        {
            numOfSrc = ntohs(((struct mCastAddrRecord *)groupRecords)->numOfSrc);
            /*shift pointer to another group record*/
            groupRecords = groupRecords + 20 + numOfSrc * 16 + (((struct mCastAddrRecord *)(groupRecords))->auxLen) * 4;
        }
    }

    return (multicastRouterPortMask & (~pktPortMask) & 0x3f);
    
}

static uint8 rtl_processIgmpMld(uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint8 pktPortMask)
{   
    uint8 fwdPortMask = 0;
    
    switch(pktBuf[0])
    {
        case IGMP_QUERY:
#ifdef IGMP_SNOOPING_DEBUG          
            rtl_gluePrintf("rtl_processIgmpMld: received IGMP_QUERY\n");
#endif
            fwdPortMask = rtl_processQueries(ipVersion, pktPortMask, pktBuf, pktLen);
            break;
            
        case IGMPV1_REPORT:
#ifdef IGMP_SNOOPING_DEBUG          
            rtl_gluePrintf("rtl_processIgmpMld: received IGMPV1_REPORT\n");
#endif          
            fwdPortMask = rtl_processJoin(ipVersion, pktPortMask, pktBuf);
            break;
            
        case IGMPV2_REPORT: 
#ifdef IGMP_SNOOPING_DEBUG      
            rtl_gluePrintf("rtl_processIgmpMld: received IGMPV2_REPORT\n");
#endif          
            fwdPortMask = rtl_processJoin(ipVersion, pktPortMask, pktBuf);
            break;
            
        case IGMPV2_LEAVE:
#ifdef IGMP_SNOOPING_DEBUG          
            rtl_gluePrintf("rtl_processIgmpMld: received IGMPV2_LEAVE\n");
#endif          
            fwdPortMask = rtl_processLeave(ipVersion, pktPortMask, pktBuf);
            break;

        case IGMPV3_REPORT:
#ifdef IGMP_SNOOPING_DEBUG          
            rtl_gluePrintf("rtl_processIgmpMld: received IGMPV3_REPORT\n");
#endif          
            fwdPortMask = rtl_processIgmpv3Mldv2Reports(ipVersion, pktPortMask, pktBuf);
            break;

        case MLD_QUERY:
            fwdPortMask = rtl_processQueries(ipVersion, pktPortMask, pktBuf, pktLen);
            break;
            
        case MLDV1_REPORT:
            fwdPortMask = rtl_processJoin(ipVersion, pktPortMask, pktBuf);
            break;
            
        case MLDV1_DONE:    
            fwdPortMask = rtl_processLeave(ipVersion, pktPortMask, pktBuf);
            break;
            
        case MLDV2_REPORT:
            fwdPortMask = rtl_processIgmpv3Mldv2Reports(ipVersion, pktPortMask, pktBuf);
            break;

        default:
            fwdPortMask = ((~(pktPortMask | rtl_cpuPortMask)) & 0x3f);
            break;
    }                       

#ifdef IGMP_SNOOPING_DEBUG          
    rtl_gluePrintf("rtl_processIgmpMld: fwdMask is 0x %x\n", fwdPortMask);
#endif          

    return fwdPortMask;
            
}



static uint8 rtl_processDvmrp(uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint8 pktPortMask)
{
    uint8 portIndex = 0;
    uint8 oldPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
    uint8 newPortMask = 0;

    portIndex = rtl_mapPortMaskToPortNum(pktPortMask);
    if(IP_VERSION4 == ipVersion)
    {
        rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.dvmrpRouterAgingTime; /*update timer*/
        if(pktPortMask != 0) 
        {   
            newPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
            if(oldPortMask != newPortMask)
            {
                rtl_updateAllGroupEntry(rtl_ipv4HashTable);
            }
        }
    }
    return ((~(pktPortMask | rtl_cpuPortMask)) & 0x3f);

}

static void rtl_processMospf(uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint8 pktPortMask)
{ 
    uint8 portIndex = 0;
    struct ipv4MospfHdr *ipv4MospfHeader = (struct ipv4MospfHdr*)pktBuf;
    struct ipv4MospfHello *ipv4HelloPkt = (struct ipv4MospfHello*)pktBuf;
    
    struct ipv6MospfHdr *ipv6MospfHeader = (struct ipv6MospfHdr*)pktBuf;
    struct ipv6MospfHello *ipv6HelloPkt = (struct ipv6MospfHello*)pktBuf;
    uint8 oldPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
    uint8 newPortMask = 0;

    portIndex = rtl_mapPortMaskToPortNum(pktPortMask);

    if(IP_VERSION4 == ipVersion)
    {   
        /*mospf is built based on ospfv2*/
        if((2 == ipv4MospfHeader->version) && (MOSPF_HELLO_TYPE == ipv4MospfHeader->type))
        {
            if((ipv4HelloPkt->options & 0x04) != 0)
            {
                rtl_ipv4MulticastRouters.mospfRouter.portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.mospfRouterAgingTime; /*update timer*/
                if(pktPortMask != 0) 
                {   
                    newPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
                    if(oldPortMask != newPortMask)
                    {
                        rtl_updateAllGroupEntry(rtl_ipv4HashTable);
                    }
                }
            }
        }
    }
    
    if(IP_VERSION6 == ipVersion)
    {   
        if((3 == ipv6MospfHeader->version) && (MOSPF_HELLO_TYPE == ipv6MospfHeader->type))
        {
            if((ipv6HelloPkt->options[2] & 0x04) != 0)
            {
                rtl_ipv6MulticastRouters.mospfRouter.portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.mospfRouterAgingTime; /*update timer*/
                if(pktPortMask != 0)
                {   
                    newPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
                    if(oldPortMask != newPortMask)
                    {
                        rtl_updateAllGroupEntry(rtl_ipv6HashTable);
                    }
                }
            }
        }
    }    
}

static void rtl_processPim(uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint8 pktPortMask)
{
    uint8 portIndex = 0;
    uint8 oldPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
    uint8 newPortMask = 0;
    
    portIndex = rtl_mapPortMaskToPortNum(pktPortMask);
    if(IP_VERSION4 == ipVersion)
    {   
        rtl_ipv4MulticastRouters.pimRouter.portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
        if(pktPortMask != 0)
        {
            newPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
            if(oldPortMask != newPortMask)
            {
                rtl_updateAllGroupEntry(rtl_ipv4HashTable);
            }
        }
    }
    
    if(IP_VERSION6 == ipVersion)
    {
        rtl_ipv6MulticastRouters.pimRouter.portTimer[portIndex] = rtl_sysUpSeconds + rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
        if(pktPortMask != 0)
        {
            newPortMask = rtl_getMulticastRouterPortMask(ipVersion, rtl_sysUpSeconds);
            if(oldPortMask != newPortMask)
            {
                rtl_updateAllGroupEntry(rtl_ipv6HashTable);
            }
        }
    }     
}


/*********************************************
                External Function
  *********************************************/


/*External called function by high level program*/


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
int32 rtl_initMulticastSnooping(struct rtl_multicastConfig multicastConfiguration, uint32 currentSystemTime, uint32 delPortMaskRevLeave)
{
    int32 i = 0;
    int32 j = 0;
    uint32 maxHashTableSize = 1024;
    uint32 cpuPortNum = 0;

    if(DISABLE == rtl_multicastStatus)
    {
        rtl_startTime = currentSystemTime;
        rtl_sysUpSeconds = 0;         

        /*specify the  realtekEtherType of the cpu tag*/    
        rtl_etherType = REALTEKETHERTYPE ;        

        /*specify the cpuPortMask*/
        rtl_getCPUPort(&cpuPortNum);
        
        if(cpuPortNum < 6)
        {       
            rtl_cpuPortMask = rtl_mapPortNumToPortMask((uint8)cpuPortNum);
        }
        else
        {
            return FAILED;
        }
    

        if(TRUE == multicastConfiguration.enableSourceList)
        {
            rtl_enableSourceList = TRUE;
        }
        else
        {
            rtl_enableSourceList = FALSE;
        }

        if(0 == multicastConfiguration.maxGroupNum)
        {
            rtl_totalMaxGroupCnt = RTL_DefaultMaxGroupEntryCnt;
        } 
        else
        {
            rtl_totalMaxGroupCnt = multicastConfiguration.maxGroupNum;
        }

        if(TRUE == rtl_enableSourceList)
        {
            if(0 == multicastConfiguration.maxSourceNum)
            {
                rtl_totalMaxSourceCnt = RTL_DefaultMaxSourceEntryCnt;
            } 
            else
            {
                rtl_totalMaxSourceCnt = multicastConfiguration.maxSourceNum;
            }
        }
        else
        {
            rtl_totalMaxSourceCnt = 0;
        }

        /* set hash table size and hash mask*/
        if(multicastConfiguration.hashTableSize == 0)
        {
            rtl_hashTableSize = 32;   /*default hash table size*/
        }
        else
        {     
            for(i = 0; i < 11; i++)
            {
                if(multicastConfiguration.hashTableSize >= maxHashTableSize)
                {
                    rtl_hashTableSize = maxHashTableSize;
                    break;
                }
                maxHashTableSize = maxHashTableSize >> 1;
            }
        }

        rtl_hashMask = rtl_hashTableSize - 1;


        /*set multicast snooping parameters, use default value*/
        if(0 == multicastConfiguration.groupMemberAgingTime)
        {
            rtl_mCastTimerParas.groupMemberAgingTime = DEFAULT_GROUP_MEMBER_INTERVAL;
        }
        else
        {
            rtl_mCastTimerParas.groupMemberAgingTime = multicastConfiguration.groupMemberAgingTime;
        }

        if(0 == multicastConfiguration.lastMemberAgingTime)
        {
            rtl_mCastTimerParas.lastMemberAgingTime = DEFAULT_LAST_MEMBER_INTERVAL;
        }
        else
        {
            rtl_mCastTimerParas.lastMemberAgingTime = multicastConfiguration.lastMemberAgingTime;
        }

        if(0 == multicastConfiguration.querierPresentInterval)
        {
            rtl_mCastTimerParas.querierPresentInterval = DEFAULT_QUERIER_PRESENT_TIMEOUT;
        }
        else
        {
            rtl_mCastTimerParas.querierPresentInterval = multicastConfiguration.querierPresentInterval;
        }


        if(0 == multicastConfiguration.dvmrpRouterAgingTime)
        {
            rtl_mCastTimerParas.dvmrpRouterAgingTime = DEFAULT_DVMRP_AGING_TIME;
        }
        else
        {
            rtl_mCastTimerParas.dvmrpRouterAgingTime = multicastConfiguration.dvmrpRouterAgingTime;
        }

        if(0 == multicastConfiguration.mospfRouterAgingTime)
        {
            rtl_mCastTimerParas.mospfRouterAgingTime = DEFAULT_MOSPF_AGING_TIME;
        }
        else
        {
            rtl_mCastTimerParas.mospfRouterAgingTime = multicastConfiguration.mospfRouterAgingTime;
        }

        if(0 == multicastConfiguration.pimRouterAgingTime)
        {
            rtl_mCastTimerParas.pimRouterAgingTime = DEFAULT_PIM_AGING_TIME;
        }
        else
        {
            rtl_mCastTimerParas.pimRouterAgingTime = multicastConfiguration.pimRouterAgingTime;
        }

        /*initialize multicast Routers information*/
        for(i = 0; i < 6; i++)
        {
            rtl_ipv4MulticastRouters.querier.portTimer[i] = 0;
            rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[i] = 0;
            rtl_ipv4MulticastRouters.pimRouter.portTimer[i] = 0;
            rtl_ipv4MulticastRouters.mospfRouter.portTimer[i] = 0;

            rtl_ipv6MulticastRouters.querier.portTimer[i] = 0;
            rtl_ipv6MulticastRouters.dvmrpRouter.portTimer[i] = 0;
            rtl_ipv6MulticastRouters.pimRouter.portTimer[i] = 0;
            rtl_ipv6MulticastRouters.mospfRouter.portTimer[i] = 0;
        }

    
        /*initialize hash table*/
        rtl_initHashTable(rtl_hashTableSize);
        if((NULL == rtl_ipv4HashTable)  || (NULL == rtl_ipv6HashTable))
        {
            return FAILED;
        }

        /*initialize group entry pool*/
        rtl_groupMemory = NULL;
        rtl_sourceMemory = NULL;

        rtl_groupEntryPool = rtl_initGroupEntryPool(rtl_totalMaxGroupCnt); 
        if(NULL == rtl_groupEntryPool)
        {
            return FAILED;
        }

        if(TRUE == rtl_enableSourceList)
        {
            rtl_sourceEntryPool = rtl_initSourceEntryPool(rtl_totalMaxSourceCnt); 

            if(NULL == rtl_sourceEntryPool)
            {
                rtl_totalMaxSourceCnt = 0;
                rtl_enableSourceList = FALSE;
                return FAILED;
            }
        }
        else
        {
            rtl_sourceEntryPool = NULL;
        }
    
        for(j = 1; j < 4; j++)
        {
            for(i = 0; i < 6; i++)
            {
                rtl_gatewayMac[j][i] = 0;
            }
        }

        rtl_gatewayMac[0][0] = 0x00;
        rtl_gatewayMac[0][1] = 0x0c;
        rtl_gatewayMac[0][2] = 0x29;
        rtl_gatewayMac[0][3] = 0x1a;
        rtl_gatewayMac[0][4] = 0xa0;
        rtl_gatewayMac[0][5] = 0xe7;

        rtl_gatewayIpv4Addr[0] = 0xc0a801fe;

        for(i = 1; i < 4; i++)
        {
            rtl_gatewayIpv4Addr[i] = 0;
            for(j = 0; j < 4; j++)
            {
                rtl_gatewayIpv6Addr[i][j] = 0;
            }
        }
             
        /*specify the cpu is a router or not */
        rtl_delPortMaskRevLeave = delPortMaskRevLeave;
     
        /*at last, config switch*/
        rtl_configSwitch(cpuPortNum, rtl_etherType);
        rtl_multicastStatus = ENABLE;
        
#ifdef RTL_MULTICAST_SNOOPING_DEBUG
        rtl_gluePrintf("Multicast Snooping is enabled!\n");
#endif
        return SUCCESS;
    }
    else
    {
#ifdef RTL_MULTICAST_SNOOPING_DEBUG 
        rtl_gluePrintf("Multicast Snooping is already enabled!\n");
#endif
        return SUCCESS;
    }
    
}

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
void rtl_setMulticastParameters(struct rtl_mCastTimerParameters mCastTimerParameters, uint8 *pGatewayMac, uint32 gatewayIpv4Addr, uint32 *pGatewayIpv6Addr)
{
    uint8 cpuPortNum;
    int32 i;

    if(mCastTimerParameters.groupMemberAgingTime != 0)
    {
        rtl_mCastTimerParas.groupMemberAgingTime = mCastTimerParameters.groupMemberAgingTime;
    }

    if(mCastTimerParameters.lastMemberAgingTime != 0)
    {
        rtl_mCastTimerParas.lastMemberAgingTime = mCastTimerParameters.lastMemberAgingTime;
    }

    if(mCastTimerParameters.querierPresentInterval != 0)
    {
    
        rtl_mCastTimerParas.querierPresentInterval = mCastTimerParameters.querierPresentInterval;
    }


    if(mCastTimerParameters.dvmrpRouterAgingTime != 0)
    {
    
        rtl_mCastTimerParas.dvmrpRouterAgingTime = mCastTimerParameters.dvmrpRouterAgingTime;
    }

    if(mCastTimerParameters.mospfRouterAgingTime != 0)
    {
    
        rtl_mCastTimerParas.mospfRouterAgingTime=mCastTimerParameters.mospfRouterAgingTime;
    }

    if(mCastTimerParameters.pimRouterAgingTime != 0)
    {
    
        rtl_mCastTimerParas.pimRouterAgingTime = mCastTimerParameters.pimRouterAgingTime;
    }


    if(pGatewayMac != NULL)
    {
        for(i = 0; i < 4; i++)
        {
            if((rtl_gatewayMac[i][0] | rtl_gatewayMac[i][1] | rtl_gatewayMac[i][2] | rtl_gatewayMac[i][3] | rtl_gatewayMac[i][4] | rtl_gatewayMac[i][5]) == 0)
            {
                break;
            }
        }
        rtl_gatewayMac[i][0] = pGatewayMac[0];
        rtl_gatewayMac[i][1] = pGatewayMac[1];
        rtl_gatewayMac[i][2] = pGatewayMac[2];
        rtl_gatewayMac[i][3] = pGatewayMac[3];
        rtl_gatewayMac[i][4] = pGatewayMac[4];
        rtl_gatewayMac[i][5] = pGatewayMac[5];
        rtl_gluePrintf("set gateway mac %d : %x %x %x %x %x %x\n", i,  rtl_gatewayMac[i][0], rtl_gatewayMac[i][1], rtl_gatewayMac[i][2], rtl_gatewayMac[i][3], rtl_gatewayMac[i][4], rtl_gatewayMac[i][5]);

        /*set CPU mac to LUT*/
#ifdef RTL_MULTICAST_SNOOPING_TEST
#else
        cpuPortNum = rtl_mapPortMaskToPortNum(rtl_cpuPortMask);
        if(rtl_addLUTUnicast(rtl_gatewayMac[i], cpuPortNum) == FAILED)
        {
            rtl_gluePrintf("set CPU mac error!\n");
        }
#endif
    }

    if(gatewayIpv4Addr != 0)
    {
        for(i = 0; i < 4; i++)
        {
            if(0 == rtl_gatewayIpv4Addr[i])
            {
                rtl_gatewayIpv4Addr[i] = gatewayIpv4Addr;
            }
        }       
    }
    
    if(pGatewayIpv6Addr != NULL)
    {
        for(i = 0; i < 4; i++)
        {
            if((rtl_gatewayIpv6Addr[i][0] | rtl_gatewayIpv6Addr[i][1] | rtl_gatewayIpv6Addr[i][2] | rtl_gatewayIpv6Addr[i][3]) == 0)
            {
                rtl_gatewayIpv6Addr[i][0] = pGatewayIpv6Addr[0];
                rtl_gatewayIpv6Addr[i][1] = pGatewayIpv6Addr[1];
                rtl_gatewayIpv6Addr[i][2] = pGatewayIpv6Addr[2];
                rtl_gatewayIpv6Addr[i][3] = pGatewayIpv6Addr[3];
            }        
        }       
    }
    
}

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
int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime )
{
    /* maintain current time */
    uint32 i = 0;
    uint32 maxTime = 0xffffffff;

    struct rtl_groupEntry* groupEntryPtr = NULL;
    struct rtl_groupEntry* nextEntry = NULL;
    uint8 oldIpv4RouterPortMask = 0;
    uint8 oldIpv6RouterPortMask = 0;
    uint8 newIpv4RouterPortMask = 0;
    uint8 newIpv6RouterPortMask = 0;

    if(ENABLE == rtl_multicastStatus)
    {
        rtl_previousSysTime = rtl_sysUpSeconds;
    
        oldIpv4RouterPortMask = rtl_getMulticastRouterPortMask(IP_VERSION4, rtl_previousSysTime);
        oldIpv6RouterPortMask = rtl_getMulticastRouterPortMask(IP_VERSION6, rtl_previousSysTime);
    
        /*handle timer conter overflow*/
        if(currentSystemTime > rtl_startTime)
        {
            rtl_sysUpSeconds = currentSystemTime - rtl_startTime;
        }
        else
        {
            rtl_sysUpSeconds = (maxTime - rtl_startTime) + currentSystemTime + 1;
        }

        /*maintain ipv4 group entry  timer */
        for(i = 0; i < rtl_hashTableSize; i++)
        {
              /*scan the hash table*/
            groupEntryPtr = rtl_ipv4HashTable[i];
            while(groupEntryPtr)              /*traverse each group list*/
            {   
                nextEntry = groupEntryPtr->next; 
                rtl_checkGroupEntryTimer(groupEntryPtr);
                groupEntryPtr = nextEntry;/*because expired group entry  will be cleared*/
            }
        }
    
        /*maintain ipv6 group entry  timer */
        for(i = 0; i < rtl_hashTableSize; i++)
        {
              /*scan the hash table*/
            groupEntryPtr = rtl_ipv6HashTable[i];
            while(groupEntryPtr)              /*traverse each group list*/
            {   
                nextEntry = groupEntryPtr->next; 
                rtl_checkGroupEntryTimer(groupEntryPtr);
                groupEntryPtr = nextEntry;/*because expired group entry  will be cleared*/
            }
        }
        /*maintain multicast router position timer */
    
        newIpv4RouterPortMask = rtl_getMulticastRouterPortMask(IP_VERSION4, rtl_sysUpSeconds);
        newIpv6RouterPortMask = rtl_getMulticastRouterPortMask(IP_VERSION6, rtl_sysUpSeconds);
    
        if(oldIpv4RouterPortMask != newIpv4RouterPortMask)
        {
            rtl_updateAllGroupEntry(rtl_ipv4HashTable);
        }

        if(oldIpv6RouterPortMask != newIpv6RouterPortMask)
        {
            rtl_updateAllGroupEntry(rtl_ipv6HashTable);
        }
    
    }
    
    return SUCCESS;
    
}


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
 *       otherwise the source port is decided by pPort.
 */
void rtl_multicastSnoopingReceive(uint8 *pMacFrame, uint8 *pPort)
{
    struct rtl_macFrameInfo macFrameInfo;
    uint8 fwdPortMask = 0; 

    memset(&macFrameInfo, 0, sizeof(struct rtl_macFrameInfo));

    if(NULL == pPort)
        macFrameInfo.pktPortMask = rtl_glueGetSrcPortMask(pMacFrame);
    else
    {
#if defined(CONFIG_RTL8306SDM) || defined(CONFIG_RTL8306E)
        if(*pPort > RTL8306_PORT5)
        {
            return;
        }
        else
            macFrameInfo.pktPortMask |= 1 << *pPort;
#endif
    }


    rtl_parseMacFrame(pMacFrame, &macFrameInfo);

    if(rtl_multicastStatus == ENABLE)
    {
        if(macFrameInfo.ipBuf == NULL)
        {
            /* goto out;*/
            return;
        }

        if((macFrameInfo.ipVersion != IP_VERSION4) && (macFrameInfo.ipVersion != IP_VERSION6))
        {
            /* goto out;*/
            return;
        }

        if(rtl_checkPortMask(macFrameInfo.pktPortMask) == WRONG)
        {
            /* goto out;*/
            return;
        }

        if(macFrameInfo.checksumFlag != SUCCESS)
        {
            /* goto out;*/
            return;
        }

        switch(macFrameInfo.l3Protocol)
        {
            case IGMP_PROTOCOL:
                fwdPortMask = rtl_processIgmpMld((uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
                break;

            case ICMP_PROTOCOL:
                fwdPortMask = rtl_processIgmpMld((uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
                break;


            case DVMRP_PROTOCOL:
                fwdPortMask = rtl_processDvmrp((uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
                break;

            case MOSPF_PROTOCOL:
                rtl_processMospf((uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
                break;

            case PIM_PROTOCOL:
                rtl_processPim((uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
                break;

            default:
                break;
        }

#ifdef IGMP_SNOOPING_DEBUG          
        rtl_gluePrintf("rtl_multicastSnoopingReceive: fwdPortMask 0x%x\n", fwdPortMask);
#endif  
        rtl_CPUTagXmit(pMacFrame, macFrameInfo, fwdPortMask);
    }
#if 0   
out:
    if((removeCpuTag==TRUE) && (macFrameInfo.cpuTagFlag!=0))
    {
        for(i=11; i>=0; i--)
        {
            macFrame[i+4]=macFrame[i];
        }
        *newMacFrame= macFrame+4;
    }
#endif
    
}


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
int32 rtl_multicastSnoopingSend(uint8 *pMacFrame, uint32 priorFreeBytes, uint32 posteriorFreeSpace, uint8 **ppNewMacFrame)
{
    /*only the multicast address, which can be found in the  forward list but not set in the look up table, will be insert cpu tag*/
    struct rtl_groupEntry * groupEntry = NULL;
    int32 i = 0;
    uint32 pktLen = 0;
    uint32 groupAddress[4] = {0, 0, 0, 0};
    uint8 *tempPtr1 = NULL;
    uint8 *tempPtr2 = NULL;
    uint8 multicastRouterPortMask = 0;
    uint32 fwdPortMask = 0;

    struct rtl_macFrameInfo macInfo;
    struct ipv4Pkt *ipv4Buf = NULL;
    struct ipv6Pkt *ipv6Buf = NULL;

    memset(&macInfo, 0, sizeof(struct rtl_macFrameInfo));

    if(ENABLE == rtl_multicastStatus)
    {

        *ppNewMacFrame = pMacFrame;
        rtl_parseMacFrame(pMacFrame, &macInfo);
        if((macInfo.ipVersion != IP_VERSION4) && (macInfo.ipVersion != IP_VERSION6))
        {
            return FORWARD_PACKET;
        }
        else
        {
            if(IP_VERSION4 == macInfo.ipVersion)
            {
                ipv4Buf = (struct ipv4Pkt *)(macInfo.ipBuf);
                groupAddress[0] = ntohl(ipv4Buf->destinationIp);
            
                if((!IS_IPV4_MULTICAST_ADDRESS(groupAddress)) || (macInfo.cpuTagFlag != 0))    /*check whether destination ip is a multicast address*/
                {
                    return FORWARD_PACKET;
                }            
            }
            else
            {
                ipv6Buf = (struct ipv6Pkt *)(macInfo.ipBuf);
                groupAddress[0] = ntohl(ipv6Buf->destinationAddr[0]);
                groupAddress[1] = ntohl(ipv6Buf->destinationAddr[1]);
                groupAddress[2] = ntohl(ipv6Buf->destinationAddr[2]);
                groupAddress[3] = ntohl(ipv6Buf->destinationAddr[3]);
            
                if((!IS_IPV6_MULTICAST_ADDRESS(groupAddress)) || (macInfo.cpuTagFlag != 0))     /*check whether destination ip is a multicast address*/
                {
                    return FORWARD_PACKET;
                }   
            }
            
        }
    
        groupEntry = rtl_searchGroupEntry((uint32)(macInfo.ipVersion), groupAddress); 
        if(NULL == groupEntry)
        {
            return FORWARD_PACKET;
        }
        else
        {
            /*here to get multicast router port mask and forward port mask*/
            multicastRouterPortMask = rtl_getMulticastRouterPortMask((uint32)(macInfo.ipVersion), rtl_sysUpSeconds);
            if(TRUE == rtl_enableSourceList)
            {
                if(IP_VERSION4 == macInfo.ipVersion)
                {
                    fwdPortMask = rtl_getSourceFwdPortMask(groupEntry, &(ipv4Buf->sourceIp), rtl_sysUpSeconds);
                }
                else
                {
                    fwdPortMask = rtl_getSourceFwdPortMask(groupEntry, ipv6Buf->sourceAddr, rtl_sysUpSeconds);
                }
            
                if((0 == fwdPortMask) && (0 == multicastRouterPortMask)) /*here to drop unrelevant multicast packet*/
                {
                    return DROP_PACKET;
                }
            }
            else
            {
                if((groupEntry != NULL) && (FALSE == groupEntry->lookupTableFlag))
                {
                    fwdPortMask = rtl_getGroupFwdPortMask(groupEntry, rtl_sysUpSeconds);  
                }

                /*if group entry not found or multicast entry already in look up table then forward portmask*/
                if(0 == fwdPortMask)
                {
                    return FORWARD_PACKET;
                }
            }
        }

        /*here to insert cpu tag*/
        if((priorFreeBytes < 4) && (posteriorFreeSpace < 4))  /*not enough space to insert cpu tag*/
        {
            return ERROR_NO_SPACE;
        }
        else
        {   
        
            if(priorFreeBytes >= 4) /*move forwardly*/
            {
                tempPtr1 = pMacFrame - 4;
                for(i = 0; i < 12; i++)  /*move DA,SA forwardly*/
                {
                    tempPtr1[i] = pMacFrame[i];
                }

                *ppNewMacFrame = tempPtr1;
            }
            else  /*posteriorFreeSpace>=4bytes, move backforwardly*/
            {
                if(IP_VERSION4 == macInfo.ipVersion)
                {
                    /*move until one bytes before cpu tag*/
                    pktLen = ntohs(ipv4Buf->length);
                
                    /*locate last byte of ipv4 packet*/
                    tempPtr1 = macInfo.ipBuf + pktLen - 1;            
                }
                else
                {
                    /*move until one bytes before cpu tag*/
                    pktLen = ntohs(ipv6Buf->payloadLenth);
                
                    /*locate last byte of ipv6 packet*/
                    tempPtr1 = macInfo.ipBuf + IPV6_HEADER_LENGTH + pktLen - 1;            
                }
            
                /*move until one bytes before cpu tag*/
                tempPtr2 = pMacFrame + 11;   
                while(tempPtr1 != tempPtr2)
                {
                    *(tempPtr1 + 4) = *tempPtr1;
                     tempPtr1--;
                }
                *ppNewMacFrame = pMacFrame;
            }

            /*insert cpu tag, add multicast router port and clear cpu port*/
            *(*ppNewMacFrame + 12) = (uint8)((rtl_etherType & 0xff00) >> 8);
            *(*ppNewMacFrame + 13) = (uint8)(rtl_etherType & 0x00ff);
            *(*ppNewMacFrame + 14) = CPUTAGPROTOCOL << 4;
            *(*ppNewMacFrame + 15) = (fwdPortMask | multicastRouterPortMask) & (~rtl_cpuPortMask);                        
        }
    }
    
    return FORWARD_PACKET;

}


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
int32 rtl_disableMulticastSnooping(void)
{
    uint32 i = 0;
    uint32 j = 0;
    struct rtl_groupEntry *groupEntryPtr = NULL;
    struct rtl_sourceEntry *sourceEntryPtr = NULL;
    uint8 multicastMacAddress[6];

    if(ENABLE == rtl_multicastStatus)
    {
        rtl_cpuPortMask = 0x00;
        rtl_startTime = 0;
        rtl_sysUpSeconds = 0; 
        rtl_previousSysTime = 0;

        rtl_disableSnooping();

        for(j = 0; j < 4; j++)
        {
            rtl_delLUTMACAddress(rtl_gatewayMac[j]);
            
#ifdef RTL_MULTICAST_SNOOPING_TEST
            rtl_delLUTMACAddress(rtl_gatewayMac[j]);
#else
            if((rtl_gatewayMac[i][0] | rtl_gatewayMac[i][1] | rtl_gatewayMac[i][2] | rtl_gatewayMac[i][3] | rtl_gatewayMac[i][4] | rtl_gatewayMac[i][5]) != 0)
            {
                rtl_delLUTMACAddress(rtl_gatewayMac[j]);
            }   
#endif

            rtl_gatewayIpv4Addr[j] = 0;
            for(i = 0; i < 6; i++)
            {
             rtl_gatewayMac[j][i] = 0;
            }
                
                 
            for(i = 0; i < 4; i++)
            {
                rtl_gatewayIpv6Addr[j][i] = 0;
            }
        }

        /*first delete multicast entry in lookuptable*/
        /*delete ipv4 multicast entry*/
        for(i = 0; i < rtl_hashTableSize; i++)
        {
            groupEntryPtr = rtl_ipv4HashTable[i];
                    
            while(groupEntryPtr != NULL)
            {
                rtl_mapMCastIPToMAC(groupEntryPtr->ipVersion, groupEntryPtr->groupAddr, multicastMacAddress);
                if(TRUE == groupEntryPtr->lookupTableFlag)
                {
                    if(rtl_delLUTMACAddress(multicastMacAddress) == SUCCESS)
                    {
                        if(TRUE == groupEntryPtr->aggregatorFlag)
                        {
                            rtl_setAggregator(groupEntryPtr->ipVersion, groupEntryPtr->groupAddr, FALSE); 
                        }                         
                    }
                    else
                    {
                        rtl_gluePrintf("hardware is failure\n");
                        return FAILED;
                    }
                }
                     
                rtl_deleteGroupEntry(groupEntryPtr);
                groupEntryPtr = rtl_ipv4HashTable[i];
            }
        }
           
        /*delete ipv6 multicast entry*/
        for(i = 0; i < rtl_hashTableSize; i++)
        {       
            groupEntryPtr = rtl_ipv6HashTable[i];
            while(groupEntryPtr != NULL)
            {
                rtl_mapMCastIPToMAC(groupEntryPtr->ipVersion, groupEntryPtr->groupAddr, multicastMacAddress);
                if(groupEntryPtr->lookupTableFlag == TRUE)
                {
                    if(rtl_delLUTMACAddress(multicastMacAddress) == SUCCESS)
                    {
                        if(groupEntryPtr->aggregatorFlag == TRUE)
                        {
                            rtl_setAggregator(groupEntryPtr->ipVersion, groupEntryPtr->groupAddr, FALSE); 
                        }                         
                    }
                    else
                    {
                        rtl_gluePrintf("hardware is failure\n");
                        return FAILED;
                    }
                }
                rtl_deleteGroupEntry(groupEntryPtr);
                groupEntryPtr = rtl_ipv6HashTable[i];
            }
        }

        rtl_glueFree(rtl_ipv4HashTable);
        rtl_glueFree(rtl_ipv6HashTable);

        rtl_ipv4HashTable = NULL;
        rtl_ipv6HashTable = NULL;
        rtl_hashTableSize = 0;
        rtl_hashMask = 0;

        /*free group entry pool*/
        i = 0;
        groupEntryPtr = rtl_groupEntryPool;
        while(groupEntryPtr != NULL)
        {   
            i++;
            groupEntryPtr = groupEntryPtr->next;
        }

        if(i != rtl_totalMaxGroupCnt)
        {
            rtl_gluePrintf(" free group entry pool failed\n");
            return FAILED;
        }

        if(rtl_groupMemory != NULL)
        {
            rtl_glueFree(rtl_groupMemory);  
        }

        rtl_totalMaxGroupCnt = 0;
        rtl_groupMemory = NULL;
        rtl_groupEntryPool = NULL;

        /*free source entry pool*/
        i = 0;
        sourceEntryPtr = rtl_sourceEntryPool;
        while(sourceEntryPtr != NULL)
        {
            i++;
            sourceEntryPtr = sourceEntryPtr->next;
        }

        if(i != rtl_totalMaxSourceCnt)
        {
            rtl_gluePrintf("free source entry pool failed\n");
            return FAILED;
        }

        if(rtl_sourceMemory != NULL)
        {
            rtl_glueFree(rtl_sourceMemory);
        }
        rtl_totalMaxSourceCnt = 0;
        rtl_sourceMemory = NULL;
        rtl_sourceEntryPool = NULL;

        memset(&rtl_mCastTimerParas, 0, sizeof(struct rtl_mCastTimerParameters));
        memset(&rtl_ipv4MulticastRouters, 0, sizeof(struct rtl_multicastRouters));
        memset(&rtl_ipv6MulticastRouters, 0, sizeof(struct rtl_multicastRouters));

        rtl_enableSourceList = FALSE;
        rtl_multicastStatus = DISABLE;
               
#ifdef RTL_MULTICAST_SNOOPING_DEBUG 
        rtl_gluePrintf("Multicast Snooping is disabled\n");
#endif
        return SUCCESS;
    }
       
    return SUCCESS;
    
}

#if defined(CONFIG_RTL8306E)
static void rtl_config8306e(uint32 cpuPortNum, uint32 realtekEtherType)
{
#ifndef RTL_MULTICAST_SNOOPING_TEST
    rtl8306e_reg_set(4, 24, RTL8306_REGPAGE3, realtekEtherType);
    rtk_cpu_tagPort_set(5, CPU_INSERT_TO_ALL);
    rtl8306e_vlan_leaky_set(RTL8306_VALN_LEAKY_MULTICAST, TRUE);
    rtl8306e_storm_filterEnable_set(RTL8306_MULTICASTPKT, FALSE);
    rtk_trap_igmpCtrlPktAction_set(IGMP_IPV4, IGMP_ACTION_TRAP2CPU);
    rtk_trap_igmpCtrlPktAction_set(IGMP_MLD, IGMP_ACTION_TRAP2CPU);
    rtk_trap_unknownMcastPktAction_set(1, MCAST_IPV4, MCAST_ACTION_DROP);
    rtl8306e_portLearningAbility_set(cpuPortNum, FALSE); 
#endif
}

static void rtl8306e_CPUTagXmit(uint8 *macFrame, struct rtl_macFrameInfo macFrameInfo, uint8 fwdPortMask)
{
#ifndef RTL_MULTICAST_SNOOPING_TEST
  #ifdef RTL8306_TBLBAK
    uint32 i=0;
    uint8 portNum;
    uint8 portIndex;
    uint8 vlanEntryExist=FALSE;
    uint8 enablePortMask=0;
    
    for(i = 0; i < 6; i++)
    {
        if(rtl8306_TblBak.dot1DportCtl[i]==RTL8306_SPAN_FORWARD)
        {
            enablePortMask|=(0x1<<i);
        }
    }
        
    fwdPortMask &= enablePortMask;

    if(TRUE == rtl8306_TblBak.vlanConfig.enVlan)/*vlan function is enable*/
    {
        if((FALSE == rtl8306_TblBak.vlanConfig.enTagAware) || (macFrameInfo.vlanTagFlag != VLANTAGGED))  /*enable port-base vlan, untagged pkt use port-base vlan*/
        {
            portNum = rtl_mapPortMaskToPortNum(macFrameInfo.pktPortMask);
            portIndex = rtl8306_TblBak.vlanPvidIdx[portNum];
            fwdPortMask &= rtl8306_TblBak.vlanTable[portIndex].memberPortMask;                 
        }
        else/*enable 802.1q tag aware vlan only when it is a tagged pkt*/
        {
            for(i = 0; i < 16; i++)
            {
                if(rtl8306_TblBak.vlanTable[i].vid == macFrameInfo.vlanTagID)
                {
                    vlanEntryExist = TRUE;
                    fwdPortMask &= rtl8306_TblBak.vlanTable[i].memberPortMask;
                    break;
                }
            }
            if(FALSE == vlanEntryExist)
            {
                fwdPortMask = 0;   
            }                            
        }
    }
  #endif/* RTL8306_TBLBAK */
    macFrame[15] = fwdPortMask;/*set forward packet mask*/
    rtl_glueNicSend(macFrame, (uint32)macFrameInfo.macFrameLen);
    macFrame[15] = macFrameInfo.pktPortMask; /*restore received packet mask*/
#endif /* RTL_MULTICAST_SNOOPING_TEST */

}

#elif defined(CONFIG_RTL8306SDM)
static void rtl_config8306(uint32 cpuPortNum, uint32 realtekEtherType)
{
#ifndef RTL_MULTICAST_SNOOPING_TEST
    rtl8306_setAsicPhyReg(4, 24, RTL8306_REGPAGE3, realtekEtherType);
    rtl8306_setAsicCPUPort(cpuPortNum,TRUE) ;
    rtl8306_setAsicMulticastVlan(TRUE);
    rtl8306_setAsicStormFilterEnable(RTL8306_MULTICASTPKT, FALSE);
    rtl8306_setAsicIGMPMLDSnooping(RTL8306_IGMP, TRUE);
    rtl8306_setAsicIGMPMLDSnooping(RTL8306_MLD, TRUE);   

    /*disable CPU learning*/
    rtl8306_setAsicPortLearningAbility(cpuPortNum, FALSE);
#endif
}

static void rtl8306_CPUTagXmit(uint8 *macFrame, struct rtl_macFrameInfo macFrameInfo, uint8 fwdPortMask)
{
#ifndef RTL_MULTICAST_SNOOPING_TEST
  #ifdef RTL8306_TBLBAK
    uint32 i = 0;
    uint8 portNum;
    uint8 portIndex;
    uint8 vlanEntryExist = FALSE;
    uint8 enablePortMask = 0;
    
    for(i = 0; i < 6; i++)
    {
        if(rtl8306_TblBak.dot1DportCtl[i] == RTL8306_SPAN_FORWARD)
        {
            enablePortMask |= (0x1 << i);
        }
    }
        
    fwdPortMask &= enablePortMask;

    if(TRUE == rtl8306_TblBak.vlanConfig.enVlan)/*vlan function is enable*/
    {
        if((FALSE == rtl8306_TblBak.vlanConfig.enTagAware) || (macFrameInfo.vlanTagFlag != VLANTAGGED))  /*enable port-base vlan, untagged pkt use port-base vlan*/
        {
            portNum = rtl_mapPortMaskToPortNum(macFrameInfo.pktPortMask);
            portIndex = rtl8306_TblBak.vlanPvidIdx[portNum];
            fwdPortMask &= rtl8306_TblBak.vlanTable[portIndex].memberPortMask;                 
        }
        else/*enable 802.1q tag aware vlan only when it is a tagged pkt*/
        {
            for(i = 0; i < 16; i++)
            {
                if(rtl8306_TblBak.vlanTable[i].vid == macFrameInfo.vlanTagID)
                {
                    vlanEntryExist= TRUE;
                    fwdPortMask &= rtl8306_TblBak.vlanTable[i].memberPortMask;
                    break;
                }
            }
            
            if(FALSE == vlanEntryExist)
            {
                fwdPortMask = 0;   
            }                            
        }
    }
  #endif/*RTL8306_TBLBAK*/

    macFrame[15] = fwdPortMask;/*set forward packet mask*/
    rtl_glueNicSend(macFrame, (uint32)macFrameInfo.macFrameLen);
    macFrame[15] = macFrameInfo.pktPortMask; /*restore received packet mask*/
#endif/* RTL_MULTICAST_SNOOPING_TEST */

}


#elif defined(CONFIG_RTL8366S)

static void rtl_config8366S(uint32 cpuPortNum, uint32 realtekEtherType)
{
    uint32 i;
    uint32 bcstorm;
    uint32 mcstorm;
    uint32 undastorm;
        
    /* insert CPU tag and forward unknown DA to CPU port */
    rtl8366s_setCPUPort(cpuPortNum, FALSE, FALSE);      
    
    /*disable VLAN egress check on multicast frame
     *not supported in 8366S
     */
    
    /* turn off multicast storm filter */
    for(i = 0; i < PORT_MAX; i++)
    {
        rtl8366s_getAsicStormFiltering(i, &bcstorm, &mcstorm, &undastorm);
        rtl8366s_setAsicStormFiltering(i, bcstorm, FALSE, undastorm);
    }   
    
    /* trap IGMP/MLD packet to CPU */
    rtl8366s_setAsicRma(RMA_IGMP_MLD_PPPOE, TRUE);
    rtl8366s_setAsicRma(RMA_IGMP, TRUE);
    rtl8366s_setAsicRma(RMA_MLD, TRUE); 
    
    /* add CPU mac to LUT and disable CPU port learning */  
    /*rtl8366s_addLUTUnicast(uint8 *mac, cpuPortNum);*/
    rtl8366s_setAsicPortLearnDisable(1 << cpuPortNum);
}

static void rtl8366S_CPUTagXmit(uint8 *macFrame, struct rtl_macFrameInfo macFrameInfo, uint8 fwdPortMask)
{
    enum SPTSTATE sts;
    uint32 i = 0;
    uint32 enVlan;  
    uint32 PVid;
    uint32 priority;
    uint32 mbrmsk;  
    uint32 untagmsk;
    uint8 enablePortMask = 0;
    uint8 portNum = 0;
    
    for(i = 0;i < PORT_MAX; i++)
    {
        /*only support SVL so far*/
        rtl8366s_getAsicSpanningTreeStatus(i, 0, &sts);
        
        if(sts == FORWARDING)
        {
            enablePortMask |= (0x1 << i);
        }
    }

    fwdPortMask &= enablePortMask;

    rtl8366s_getAsicVlan(&enVlan);
    
    if(enVlan == TRUE)
    {   
        if(macFrameInfo.vlanTagFlag != VLANTAGGED)/*untagged pkt use port-based vlan*/
        {
            portNum=rtl_mapPortMaskToPortNum(macFrameInfo.pktPortMask);
            rtl8366s_getVlanPVID(portNum, &PVid, &priority);            
            rtl8366s_getVlan(PVid, &mbrmsk, &untagmsk);
            fwdPortMask&=mbrmsk;
        }
        else/*tagged pkt use 1Q-based vlan*/
        {
            rtl8366s_getVlan(macFrameInfo.vlanTagID, &mbrmsk, &untagmsk);
            fwdPortMask &= mbrmsk;
        }
    }

#ifdef IGMP_SNOOPING_DEBUG          
    rtl_gluePrintf("rtl8366S_CPUTagXmit: Rx 0x%x, fwdPortMask 0x%x\n\n",portNum,fwdPortMask);
#endif

    /*While CPU tag TX portmask is 0, 8306 will drop the packet
     *but 8366S will forward the packet by its own forward decision        
     */
    if(fwdPortMask != 0)
    {
        macFrame[15] = fwdPortMask;/*set forward packet mask*/
        rtl_glueNicSend(macFrame, (uint32)macFrameInfo.macFrameLen);
        macFrame[15] = macFrameInfo.pktPortMask; /*restore received packet mask*/ 
    }   
}
#endif

static int32 rtl_getCPUPort(uint32 *port)
{
    int32 ret = SUCCESS;
#if defined(CONFIG_RTL8306SDM)  
    uint32 enTag;
#elif defined(CONFIG_RTL8306E)
     rtk_cpu_insert_t mode;
#elif defined(CONFIG_RTL8366S)
    uint32 noTag;
    uint32 dropUnda;    
#endif  
    
#if defined(RTL_MULTICAST_SNOOPING_TEST)

    *port = cpuPortNumer;

#elif defined(CONFIG_RTL8306E)
    ret = rtk_cpu_tagPort_get(port, &mode);

#elif defined(CONFIG_RTL8306SDM)
    ret = rtl8306_getAsicCPUPort(port, &enTag);

#elif defined(CONFIG_RTL8366S)

    ret = rtl8366s_getCPUPort(port, &noTag, &dropUnda);

#endif

    return ret;
}

static void rtl_configSwitch(uint32 cpuPortNum, uint32 realtekEtherType)
{
#if defined(RTL_MULTICAST_SNOOPING_TEST)

#elif defined(CONFIG_RTL8306SDM)

    rtl_config8306(cpuPortNum, rtl_etherType);

#elif defined(CONFIG_RTL8306E)

    rtl_config8306e(cpuPortNum, rtl_etherType);

#elif defined(CONFIG_RTL8366S)

    rtl_config8366S(cpuPortNum, rtl_etherType);

#endif

    return;
}

static int32 rtl_addLUTUnicast(uint8 *mac, uint8 port)
{
#if defined(CONFIG_RTL8306E)
    rtk_l2_ucastAddr_t  L2_data;
#endif
    int32 ret = SUCCESS;

#if defined(RTL_MULTICAST_SNOOPING_TEST)

#elif defined(CONFIG_RTL8306SDM)
    ret = rtl8306_addLUTUnicastMacAddress(mac, 0x02, TRUE, TRUE, port);

#elif defined(CONFIG_RTL8306E)
    memset(&L2_data, 0, sizeof (rtk_l2_ucastAddr_t));
    L2_data.auth = TRUE;
    L2_data.is_static = TRUE;
    L2_data.port = port;
    ret = rtk_l2_addr_add((rtk_mac_t *)mac, &L2_data);

#elif defined(CONFIG_RTL8366S)

    ret = rtl8366s_addLUTUnicast(mac, port);
  #ifdef IGMP_SNOOPING_DEBUG          
    rtl_gluePrintf("rtl_addLUTUnicast: ret %d, mac %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x, port %d\n",\
                ret, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], port);
  #endif

#endif

    return ret;
}


static int32 rtl_addLUTMulticast(uint8 *mac, uint32 portmask)
{
#if defined(CONFIG_RTL8306SDM)
    uint32 entryaddr = 0;
#elif defined(CONFIG_RTL8306E)
    rtk_portmask_t portMaskTmp;
#endif
    int32 ret = SUCCESS;
    
#if defined(RTL_MULTICAST_SNOOPING_TEST)

    ret = Software_addMuticastMacAddress(mac, 1, portmask);

#elif defined(CONFIG_RTL8306SDM)

    ret = rtl8306_addMuticastMacAddress(mac, 1, portmask, &entryaddr);

#elif defined(CONFIG_RTL8306E)

    portMaskTmp.bits[0] = portmask;
    ret = rtk_l2_mcastAddr_add((rtk_mac_t *)mac, 1, portMaskTmp);

#elif defined(CONFIG_RTL8366S)

    ret = rtl8366s_addLUTMulticast(mac, portmask);

#ifdef IGMP_SNOOPING_DEBUG          
    rtl_gluePrintf("rtl_addLUTMulticast: ret %d, mac %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x, portmask 0x%x\n",\
                ret, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], portmask);
#endif

#endif

    return ret;
}

static int32 rtl_delLUTMACAddress(uint8 *mac)
{
    int32 ret = SUCCESS;
#if defined(CONFIG_RTL8306SDM)  
    uint32 entryaddr = 0;
#endif
    
#if defined(RTL_MULTICAST_SNOOPING_TEST)

    ret = Software_deleteMacAddress(mac);

#elif defined(CONFIG_RTL8306SDM)
    
    ret = rtl8306_deleteMacAddress(mac, &entryaddr);

#elif defined(CONFIG_RTL8306E)
    ret = rtk_l2_mcastAddr_del((rtk_mac_t *)mac, 1);

#elif defined(CONFIG_RTL8366S)

    ret = rtl8366s_delLUTMACAddress(mac);
#ifdef IGMP_SNOOPING_DEBUG         
    rtl_gluePrintf("rtl_delLUTMACAddress: ret %d, mac %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",\
                    ret, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
 
#endif

    return ret;
}

static void rtl_CPUTagXmit(uint8 *macFrame, struct rtl_macFrameInfo macFrameInfo, uint8 fwdPortMask)
{
#if defined(RTL_MULTICAST_SNOOPING_TEST)

    macFrame[15] = fwdPortMask;/*set forward packet mask*/

#elif defined(CONFIG_RTL8306SDM)

    if(fwdPortMask != 0)
        rtl8306_CPUTagXmit(macFrame, macFrameInfo, fwdPortMask);
#elif defined(CONFIG_RTL8306E)

    if(fwdPortMask != 0)
        rtl8306e_CPUTagXmit(macFrame, macFrameInfo, fwdPortMask);

#elif defined(CONFIG_RTL8366S)

    if(fwdPortMask != 0)
        rtl8366S_CPUTagXmit(macFrame, macFrameInfo, fwdPortMask);

#endif
}

static void rtl_GetPVID(uint8 port, uint32 *vid)
{
#if defined(RTL8306_TBLBAK)
    uint8 portIndex = 0xff;
#elif defined(CONFIG_RTL8366S)
    uint32 priority;
#endif
    
#if defined(RTL8306_TBLBAK)

    portIndex = rtl8306_TblBak.vlanPvidIdx[port];
    *vid = rtl8306_TblBak.vlanTable[portIndex].vid;

#elif defined(CONFIG_RTL8366S)

    rtl8366s_getVlanPVID(port, vid, &priority);

#endif

    return;
}

static void rtl_disableSnooping(void)
{
#if defined(RTL_MULTICAST_SNOOPING_TEST)

#elif defined(CONFIG_RTL8306SDM)
    rtl8306_setAsicIGMPMLDSnooping(RTL8306_MLD, FALSE);
    rtl8306_setAsicIGMPMLDSnooping(RTL8306_IGMP, FALSE);
    /*enable CPU learning*/
    rtl8306_setAsicPortLearningAbility(rtl_mapPortMaskToPortNum(rtl_cpuPortMask), TRUE);

#elif defined(CONFIG_RTL8306E)
    rtl8306e_vlan_leaky_set(RTL8306_VALN_LEAKY_MULTICAST, FALSE);
    rtl8306e_storm_filterEnable_set(RTL8306_MULTICASTPKT, TRUE);
    rtk_trap_igmpCtrlPktAction_set(IGMP_IPV4, IGMP_ACTION_FORWARD);
    rtk_trap_igmpCtrlPktAction_set(IGMP_MLD, IGMP_ACTION_FORWARD);
    rtk_trap_unknownMcastPktAction_set(1, MCAST_IPV4,MCAST_ACTION_FORWARD);
    /*enable CPU learning*/
    rtl8306e_portLearningAbility_set(rtl_mapPortMaskToPortNum(rtl_cpuPortMask), TRUE);

#elif defined(CONFIG_RTL8366S)

    rtl8366s_setAsicRma(RMA_IGMP_MLD_PPPOE, FALSE);
    rtl8366s_setAsicRma(RMA_IGMP, FALSE);
    rtl8366s_setAsicRma(RMA_MLD, FALSE);
    rtl8366s_setAsicPortLearnDisable(0);   
#endif

    return;
}

static void rtl_getCPUTagRXPort(uint8 *portNum, uint8 *portMask)
{
#ifdef CONFIG_RTL8366S

    *portNum = *portMask;

   /* 6-bit RX of CPU tag in RTL8366s indicate port number while in RTL8306 
    * is port mask, so map port number to port mask here 
    */
    *portMask = rtl_mapPortNumToPortMask(*portNum);     

#else

    *portNum = rtl_mapPortMaskToPortNum(*portMask);

#endif

    return;
}

