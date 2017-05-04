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

#include <rtl_multicast_glue.h>
#include <rtl_multicast_snooping_local.h>

#ifndef RTL_MULTICAST_SNOOPING_TEST
  #include <asicRegs.h>
  #include <rtl865x/mbuf.h>
  #include <swNic2.h>
  
  #include <rtl865x/assert.h>
  #include <rtl8651_tblDrv.h>
#endif

#ifdef __linux__
  #include <linux/mm.h>
#endif


#ifdef RTL_MULTICAST_SNOOPING_TEST
  static uint8 portMaskArray[2048];
  static uint8 macAddrArray[2048][6];
  uint32 cpuPortNumer;
  #if defined(CONFIG_RTL8306E)
    #include <rtl8306e_asicdrv.h>
    #ifdef RTL8306_TBLBAK
      rtl8306_ConfigBakPara_t rtl8306_TblBak;
    #endif
  #elif defined(CONFIG_RTL8306SDM)
    #include <Rtl8306_Driver_s.h>
    #include <Rtl8306_AsicDrv.h>
    #ifdef RTL8306_TBLBAK
      rtl8306_ConfigBakPara_t rtl8306_TblBak;
    #endif
  #endif
#endif

extern uint32 prevTC1CNT;
extern uint32 tuRemind;   /* unit is the same as TC0DATA, say "timer unit". */
extern  uint32 accJiffies; /* accumulated jiffies, unit is the same as jiffies, 1/HZ. */
extern uint32 timer1Sec;  /* sysUpTime of Timer1 */
extern uint32 _sysUpSeconds;


#ifndef RTL_MULTICAST_SNOOPING_TEST
int intcc = 0;
uint32 savedGimr = 0;

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
__IRAM_GEN int32 rtl_glueMutexLock(void)
{
    spinlock_t lockForCC = SPIN_LOCK_UNLOCKED;
    int flagsForCC; 
    int needLock;
    spin_lock_irqsave(&lockForCC,flagsForCC);   
    needLock = (intcc >= 1) ? FALSE : TRUE;
    intcc ++;
    if ( needLock == TRUE ) 
    {       
        savedGimr = READ_MEM32( GIMR );
        WRITE_MEM32( GIMR, 0 );
    }       
    spin_unlock_irqrestore(&lockForCC, flagsForCC); 
    return 0;
}

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
__IRAM_GEN int32 rtl_glueMutexUnlock(void)
{
    spinlock_t lockForCC = SPIN_LOCK_UNLOCKED;
    int flagsForCC; 
    int needUnlock;     
    spin_lock_irqsave(&lockForCC,flagsForCC);       
    intcc--;
    needUnlock = ( intcc == 0 )? TRUE : FALSE;
    if ( needUnlock == TRUE )   
    {       
        WRITE_MEM32( GIMR, savedGimr );
    }   
    spin_unlock_irqrestore(&lockForCC, flagsForCC); 
    return 0;
}
#else

int testdrvMutex = 0;

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
void *rtl_glueMalloc(uint32 NBYTES)
{
#ifndef RTL_MULTICAST_SNOOPING_TEST
    if(0 == NBYTES)
        return NULL;
    return (void *)kmalloc(NBYTES, GFP_ATOMIC);      
#else
    return malloc(NBYTES);  
#endif  
}

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
void rtl_glueFree(void *APTR)
{
#ifndef RTL_MULTICAST_SNOOPING_TEST
    kfree(APTR);    
#else
    free(APTR);
#endif
}


#ifndef RTL_MULTICAST_SNOOPING_TEST

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
void rtl_glueNicSend(uint8 *pMacFrame, uint32 macFrameLen)
{
    int i = 0;
    struct rtl_mBuf *mbuf = NULL;
    struct rtl_pktHdr *pkt = NULL;
    if((mbuf = mBuf_get(MBUF_DONTWAIT, MBUFTYPE_DATA, 1)) == NULL)
        return;
    if(mBuf_getPkthdr(mbuf, MBUF_DONTWAIT) == (struct rtl_mBuf *) NULL)
    {
        mBuf_freeMbufChain(mbuf);
        return;
    }
    pkt = mbuf->m_pkthdr;
    
    for(i = 0; i < macFrameLen; i++) /*copy mac frame to mbuf*/
    {
        *(mbuf->m_data+i) = *(pMacFrame + i);
    }
    
    pkt->ph_len = macFrameLen;
    pkt->ph_mbuf->m_len = macFrameLen;

    pkt->ph_flags &= ~(CSUM_IP | CSUM_L4); /*STOP IP / L4 checksum.*/ 
    pkt->ph_flags &= ~(PKT_BCAST | PKT_MCAST); 

#ifdef CONFIG_RTL865XC
    pkt->ph_flags &= ~(PKTHDR_PPPOE_AUTOADD); 
    pkt->ph_flags2 &= ~(PKTHDR_VLAN_AUTOADD); 
#elif CONFIG_RTL865XB
    pkt->ph_flags &= ~(PKTHDR_PPPOE_AUTOADD | PKTHDR_VLAN_AUTOADD); 
#endif

    pkt->ph_proto = PKTHDR_ETHERNET;
    pkt->ph_pppoeIdx = 0; 
    pkt->ph_pppeTagged = 0; 
    pkt->ph_vlanTagged = 0;
    pkt->ph_LLCTagged = 0;
    pkt->ph_srcExtPortNum = 0; 
    pkt->ph_extPortList = 0;    
    pkt->ph_portlist = MII_PORT_MASK; /*send to external switch*/

#ifdef CONFIG_RTL8366S
    /* padding to the minimum frame size(without checksum), checksum will be 
      * appended by RTL865X HW 
      */
    if(macFrameLen < 64 && mBuf_padding(pkt->ph_mbuf, 64 - macFrameLen, MBUF_DONTWAIT) == NULL)
    {
        mBuf_freeMbufChain(pkt->ph_mbuf);
        return;
    }
#endif
    
    if (FAILED == swNic_write((void*)pkt))
    {
        mBuf_freeMbufChain(pkt->ph_mbuf);
    }
}

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
void rtl_multicastSnoopingV2TimeUpdate(void)
{
    uint32 jifPassed;
    uint32 secPassed;
    uint32 currTC1CNT;
    uint32 systemHZ;
    uint32 tc0data;
    uint32 prevTC1CNTtemp = prevTC1CNT;
    uint32 tuRemindtemp = tuRemind;  
    uint32 accJiffiestemp = accJiffies; /* accumulated jiffies, unit is the same as jiffies, 1/HZ. */
    uint32 _sysUpSecondstemp = _sysUpSeconds;


    systemHZ = rtl865x_getHZ();
    tc0data = READ_MEM32(TC0DATA) >> TCD_OFFSET;


    /* compute passed time since last time executed this function */
    currTC1CNT = READ_MEM32(TC1CNT);

#ifdef CONFIG_RTL865XC
    /*
        In RTL865xC, timer / counter is incremental
    */
    if ( prevTC1CNTtemp <= currTC1CNT )
    {
        /* No wrap happend. */
        tuRemindtemp += (currTC1CNT - prevTC1CNTtemp) >> TCD_OFFSET; /* how many units are passed since last check? */
    }
    else
    {
        /* Timer1 wrapped!! */
        tuRemindtemp += (currTC1CNT + (0xffffff00 - prevTC1CNTtemp) + (0x1 << TCD_OFFSET)) >> TCD_OFFSET; /* how many units are passed since last check? */
    }
#elif defined CONFIG_RTL865XB
    /*
        In RTL865xB, timer / counter is decremental
    */
    if ( prevTC1CNTtemp >= currTC1CNT )
    {
        /* No wrap happened. */
        tuRemindtemp += (prevTC1CNTtemp-currTC1CNT) >> TCD_OFFSET; /* how many units are passed since last check? */
    }
    else
    {
        /* Timer1 wrapped!! */
        tuRemindtemp += (prevTC1CNTtemp + (0xffffff00 - currTC1CNT) + (0x1 << TCD_OFFSET)) >> TCD_OFFSET; /* how many units are passed since last check? */
    }
#endif
    prevTC1CNTtemp = currTC1CNT; /* keep TC1CNT value for next time check */

    /* If tc0data is zero, it means 'time is frozen.' */
    if (0 == tc0data)
    {
        jifPassed = 0;
    }
    else
    {
        jifPassed = tuRemindtemp / tc0data;
        tuRemindtemp = tuRemindtemp % tc0data;
    }
 

    /* check second-routine list, said _rtl8651_timeUpdate() */
    secPassed = ((accJiffiestemp + jifPassed) / systemHZ) - (accJiffiestemp / systemHZ);/* Count up only when acroosing HZ. */
    if ( secPassed > 0 )
    {
        _sysUpSecondstemp += secPassed;
        rtl_maintainMulticastSnoopingTimerList(_sysUpSecondstemp);
    }
}
#endif

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
uint8 rtl_glueGetSrcPortMask(uint8 *pMacFrame)
{
    uint8 *ptr = pMacFrame;

    ptr = ptr + 12;
    if((*((uint16 *)ptr) == htons(REALTEKETHERTYPE)) && (((*(ptr + 2) & 0xf0) >> 4) == CPUTAGPROTOCOL))
    {         
        return (*(ptr + 3) & 0x3f);
    }
    else
    {
        return NON_PORT_MASK;
    }
}


#ifdef RTL_MULTICAST_SNOOPING_TEST
void rtl_glueInit(void)
{
    int i = 0;
    int j = 0; 
    uint32 cnt = 0;
    
    for(i = 0; i < 2048; i++)
    {
        portMaskArray[i] = 0;
        for(j = 0; j < 6; j++)
        {
            macAddrArray[i][j] = 0;
        }
    }

#ifdef RTL8306_TBLBAK

        /*Vlan default value*/
        rtl8306_TblBak.vlanConfig.enVlan = FALSE;
        rtl8306_TblBak.vlanConfig.enArpVlan = FALSE;
        rtl8306_TblBak.vlanConfig.enLeakVlan = FALSE;
        rtl8306_TblBak.vlanConfig.enVlanTagOnly = FALSE;
        rtl8306_TblBak.vlanConfig.enIngress =  FALSE;
        rtl8306_TblBak.vlanConfig.enTagAware = FALSE;
        rtl8306_TblBak.vlanConfig.enIPMleaky = FALSE;
        rtl8306_TblBak.vlanConfig.enMirLeaky = FALSE;
        for (cnt = 0; cnt < 6; cnt++) 
        {
            rtl8306_TblBak.vlanConfig_perport[cnt].vlantagInserRm = RTL8306_VLAN_UNDOTAG;
            rtl8306_TblBak.vlanConfig_perport[cnt].en1PRemark = FALSE;
            rtl8306_TblBak.vlanConfig_perport[cnt].enNulPvidRep =  FALSE;
        }
        for (cnt = 0; cnt < 16; cnt++) 
        {
            rtl8306_TblBak.vlanTable[cnt].vid = cnt;
            if ((cnt % 5) == 4 )
            {            
                rtl8306_TblBak.vlanTable[cnt].memberPortMask = 0x1F;                
            } 
            else  
            {
                rtl8306_TblBak.vlanTable[cnt].memberPortMask = (0x1<<4) | (0x1 << (cnt % 5));
            }
        }
        for (cnt = 0; cnt < 6; cnt++)
        {
            rtl8306_TblBak.vlanPvidIdx[cnt] = (uint8)cnt;
            rtl8306_TblBak.dot1DportCtl[cnt] = RTL8306_SPAN_FORWARD;
        }
        rtl8306_TblBak.En1PremarkPortMask = 0;
        rtl8306_TblBak.dot1PremarkCtl[0] = 0x3;
        rtl8306_TblBak.dot1PremarkCtl[1] = 0x4;
        rtl8306_TblBak.dot1PremarkCtl[2] = 0x5;
        rtl8306_TblBak.dot1PremarkCtl[3] = 0x6;
        
        for (cnt = 0; cnt < RTL8306_ACL_ENTRYNUM; cnt++) 
        {
            rtl8306_TblBak.aclTbl[cnt].phy_port = RTL8306_ACL_INVALID;
            rtl8306_TblBak.aclTbl[cnt].proto = RTL8306_ACL_ETHER;
            rtl8306_TblBak.aclTbl[cnt].data = 0;
            rtl8306_TblBak.aclTbl[cnt].action = RTL8306_ACT_PERMIT;
            rtl8306_TblBak.aclTbl[cnt].pri = RTL8306_PRIO0;            
        }
        rtl8306_TblBak.mir.mirPort = 0x7;
        rtl8306_TblBak.mir.mirRxPortMask = 0;
        rtl8306_TblBak.mir.mirTxPortMask = 0;
        rtl8306_TblBak.mir.enMirself = FALSE;
        rtl8306_TblBak.mir.enMirMac = FALSE;
        rtl8306_TblBak.mir.mir_mac[0] = 0x0;
        rtl8306_TblBak.mir.mir_mac[1] = 0x0;
        rtl8306_TblBak.mir.mir_mac[2] = 0x0;
        rtl8306_TblBak.mir.mir_mac[3] = 0x0;
        rtl8306_TblBak.mir.mir_mac[4] = 0x0;
        rtl8306_TblBak.mir.mir_mac[5] = 0x0;
#endif /* RTL8306_TBLBAK */

}

int32  Software_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask)
{
    int i = 0;
    uint8 EqualFlag = 0;
    int tableIndex = (((int)(macAddr[4] & 0x7)) << 8) |(macAddr[5]) ;
    for(i = 0; i < 6; i++)
    {
        if(macAddr[i] != macAddrArray[tableIndex][i])
        {
            EqualFlag = 0;
            break;
        }
        else
        {
            EqualFlag = 1;
        }
    }

    if(((isAuth == 1) && (portMaskArray[tableIndex] == 0)) || EqualFlag)
    {
        portMaskArray[tableIndex] = portMask;
        for(i = 0; i < 6; i++)
        {
            macAddrArray[tableIndex][i] = macAddr[i];
        }
        return SUCCESS;
    }
    else
    {   
        return FAILED;
    }
    
}

int32 Software_deleteMacAddress(uint8 *macAddr)
{
    int i = 0;
    int tableIndex = (((int)(macAddr[4] & 0x7)) << 8) |(macAddr[5]) ;
    portMaskArray[tableIndex] = 0;
    for(i = 0; i < 6; i++)
    {
        macAddrArray[tableIndex][i] = 0;
    }

    return SUCCESS;
}

uint8 Software_forward(uint8 *macFrame)
{
    int tableIndex=(((int)(macFrame[4] & 0x7)) << 8) |(macFrame[5]) ;
    if(macAddrArray[tableIndex][0] == macFrame[0]\
       && macAddrArray[tableIndex][1] == macFrame[1]\
       && macAddrArray[tableIndex][2] == macFrame[2]\
       && macAddrArray[tableIndex][3] == macFrame[3]\
       && macAddrArray[tableIndex][4] == macFrame[4]\
       && macAddrArray[tableIndex][5] == macFrame[5])
    {
        return portMaskArray[tableIndex];
    }
    else
    {
        return 0xff; /*means unknown DA*/
    }
}
#endif/*RTL_MULTICAST_SNOOPING_TEST*/

