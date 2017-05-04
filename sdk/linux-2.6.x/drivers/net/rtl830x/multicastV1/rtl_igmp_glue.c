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

#include <rtl_igmp_glue.h>
#include <rtl_igmp_snooping_local.h>
#ifndef RTL_IGMP_SNOOPING_TEST
  #ifdef RTL8651B_SDK
    #include <asicRegs.h>
    #include <mbuf.h>
    #include <swNic2.h>
    #include <assert.h>
    #include <rtl8651_tblDrv.h>
  #endif
#endif

#ifdef RTL_IGMP_SNOOPING_TEST
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif

#ifdef __linux__
  #include <linux/mm.h>
#endif

#ifdef RTL_IGMP_SNOOPING_TEST
  #if defined(CONFIG_RTL8306E)
    #include <rtl8306e_asicdrv.h>
    rtl8306_ConfigBakPara_t rtl8306_TblBak;

  #elif defined(CONFIG_RTL8306SDM)
    #include <Rtl8306_Driver_s.h>
    #include <Rtl8306_AsicDrv.h>
    rtl8306_ConfigBakPara_t rtl8306_TblBak;
  #endif

  static uint8 portMaskArray[2048];
  static uint8 macAddrArray[2048][6];
#endif

extern uint32 prevTC1CNT;
extern uint32 tuRemind;   /* unit is the same as TC0DATA, say "timer unit". */
extern uint32 accJiffies; /* accumulated jiffies, unit is the same as jiffies, 1/HZ. */
extern uint32 timer1Sec;  /* sysUpTime of Timer1 */
extern uint32 _sysUpSeconds;


#ifndef RTL_IGMP_SNOOPING_TEST
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
__IRAM_GEN int32 rtl_igmpGlueMutexLock(void)
{
    spinlock_t lockForCC = SPIN_LOCK_UNLOCKED;
    int flagsForCC; 
    int needLock;

    spin_lock_irqsave(&lockForCC,flagsForCC);   
    needLock = (intcc >= 1) ? FALSE : TRUE;
    intcc ++;
    if (TRUE == needLock)
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
__IRAM_GEN int32 rtl_igmpGlueMutexUnlock(void)
{
    spinlock_t lockForCC = SPIN_LOCK_UNLOCKED;
    int flagsForCC; 
    int needUnlock;

    spin_lock_irqsave(&lockForCC,flagsForCC);       
    intcc--;
    needUnlock = ( intcc==0 ) ? TRUE : FALSE;
    if (TRUE == needUnlock) 
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
void *rtl_igmpGlueMalloc(uint32 NBYTES)
{
#ifndef RTL_IGMP_SNOOPING_TEST
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
void rtl_igmpGlueFree(void *APTR)
{
#ifndef RTL_IGMP_SNOOPING_TEST
    kfree(APTR);    
#else
    free(APTR);
#endif

}


/*send a mac frame, suppose the cpu tag already exist */
#ifndef RTL_IGMP_SNOOPING_TEST

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
void rtl_igmpGlueNicSend(uint8 *pMacFrame, uint32 macFrameLen)
{
    int i=0;
    struct rtl_mBuf *mbuf = NULL;
    struct rtl_pktHdr *pkt=NULL;
    
    if(NULL == (mbuf = mBuf_get(MBUF_DONTWAIT, MBUFTYPE_DATA, 1)))
        return;
    if(mBuf_getPkthdr(mbuf, MBUF_DONTWAIT) == (struct rtl_mBuf *)NULL)
    {
        mBuf_freeMbufChain(mbuf);
        return;
    }
    pkt = mbuf->m_pkthdr;


    for(i = 0; i < macFrameLen; i++) /*copy mac frame to mbuf*/
    {
      *(mbuf->m_data + i) = *(pMacFrame + i);
    }

    pkt->ph_len = macFrameLen;
    pkt->ph_mbuf->m_len = macFrameLen;

    pkt->ph_flags &= ~(CSUM_IP|CSUM_L4); /*STOP IP / L4 checksum.*/ 
    pkt->ph_flags &= ~(PKT_BCAST|PKT_MCAST); 

#ifdef CONFIG_RTL865XC
    pkt->ph_flags &= ~(PKTHDR_PPPOE_AUTOADD); 
    pkt->ph_flags2 &= ~(PKTHDR_VLAN_AUTOADD); 
#elif defined CONFIG_RTL865XB
    pkt->ph_flags &= ~(PKTHDR_PPPOE_AUTOADD|PKTHDR_VLAN_AUTOADD); 
#endif

    pkt->ph_proto = PKTHDR_ETHERNET;
    pkt->ph_pppoeIdx = 0; 
    pkt->ph_pppeTagged = 0; 
    pkt->ph_vlanTagged = 0;
    pkt->ph_LLCTagged = 0;
    pkt->ph_srcExtPortNum = 0; 
    pkt->ph_extPortList = 0;    
    pkt->ph_portlist = MII_PORT_MASK; /*send to port 5*/
    if (FAILED == swNic_write((void*)pkt))
    {
        mBuf_freeMbufChain(pkt->ph_mbuf);
    }
}
  
 
/* Function Name:
 *      rtl_multicastSnoopingV1TimeUpdate
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
void rtl_multicastSnoopingV1TimeUpdate(void)
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
      tuRemindtemp += (currTC1CNT-prevTC1CNTtemp) >> TCD_OFFSET; /* how many units are passed since last check? */
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
      tuRemindtemp += (prevTC1CNTtemp - currTC1CNT) >> TCD_OFFSET; /* how many units are passed since last check? */
  }
  else
  {
      /* Timer1 wrapped!! */
      tuRemindtemp += (prevTC1CNTtemp + (0xffffff00 - currTC1CNT) + (0x1 << TCD_OFFSET)) >> TCD_OFFSET; /* how many units are passed since last check? */
  }
#endif
  prevTC1CNTtemp = currTC1CNT; /* keep TC1CNT value for next time check */

  /* If tc0data is zero, it means 'time is frozen.' */
  if (0 == tc0data )
  {
    jifPassed = 0;
  }
  else
  {
    jifPassed = tuRemindtemp / tc0data;
    tuRemindtemp = tuRemindtemp % tc0data;
  }


  /* check second-routine list, said _rtl8651_timeUpdate() */
  secPassed = ((accJiffiestemp+jifPassed)/systemHZ)-(accJiffiestemp/systemHZ);/* Count up only when acroosing HZ. */
  if ( secPassed > 0 )
  {
      _sysUpSecondstemp+=secPassed;
      rtl_maintainIgmpSnoopingV1TimerList(_sysUpSecondstemp);
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
    if((*((uint16 *)ptr) == htons(REALTEKETHERTYPE)) && (((*(ptr+2)&0xf0)>>4) == CPUTAGPROTOCOL))
    {
        return (*(ptr + 3) & 0x3f);
    }
    else
    {
        return NON_PORT_MASK;
    }

}

#ifdef RTL_CPU_HW_FWD

/* Function Name:
 *      tl_igmpDisCpuHwFwd
 * Description:
 *      Prevent CPU hardware from forwarding multicast data which is specified by groupAddress
 * Input:
 *      groupAddress  -   Specifies the group address to be stopped.
 * Output:
 *      none
 * Return:
 *      one
 * Note:
 *      User should modify this function according to his own system.
 */
void rtl_igmpDisCpuHwFwd(uint32 groupAddress)
{
#ifndef RTL_IGMP_SNOOPING_TEST
  #ifdef RTL8651B_SDK
    _rtl8651_setMGroupAttribute(groupAddress, TRUE);
  #endif
#endif
}
#endif

#ifdef RTL_IGMP_SNOOPING_TEST
void rtl_igmpGlueInit(void)
{
    int i = 0;
    int j = 0;
    uint32 cnt;

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
  #endif
}

uint8 rtl8306_forward(uint8* macFrame)
{
    int tableIndex=(((int)(macFrame[4] & 0x7)) << 8) |(macFrame[5]);

    if(macAddrArray[tableIndex][0]==macFrame[0]\
     && macAddrArray[tableIndex][1]==macFrame[1]\
     && macAddrArray[tableIndex][2]==macFrame[2]\
     && macAddrArray[tableIndex][3]==macFrame[3]\
     && macAddrArray[tableIndex][4]==macFrame[4]\
     && macAddrArray[tableIndex][5]==macFrame[5])
    {
        return portMaskArray[tableIndex];
    }
    else
    {
        return 0xff; /*means unknown DA*/
    }   
}

#if defined(CONFIG_RTL8306E)
int32  rtl8306e_l2_multicastMac_add(uint8 *macAddr,uint32 isAuth, uint32 portMask,uint32 *entryaddr)
{
    int i = 0;
    uint8 EqualFlag = 0;
    int tableIndex = (((int)(macAddr[4] & 0x7)) << 8) |(macAddr[5]) ;
    *entryaddr = tableIndex;
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

    if(((isAuth == 1) && (portMaskArray[tableIndex] == 0))||EqualFlag)
    {
        portMaskArray[tableIndex]=portMask;
        for(i = 0; i < 6; i++)
        {
            macAddrArray[tableIndex][i]=macAddr[i];
        }
        return SUCCESS;
    }
    else
    {   
        return FAILED;
    }
}

int32 rtl8306e_addLUTUnicastMacAddress(uint8* macAddr, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port)
{
    int i = 0;
    uint8 EqualFlag = 0;
    int tableIndex=(((int)(macAddr[4] & 0x7)) << 8) |(macAddr[5]) ;
    for(i = 0; i < 6; i++)
    {
        if(macAddr[i]!=macAddrArray[tableIndex][i])
        {
            EqualFlag=0;
            break;
        }
        else
        {
            EqualFlag = 1;
        }
    }

    if(((1 == isAuth) && (0 == portMaskArray[tableIndex])) || EqualFlag)
    {
        portMaskArray[tableIndex]=(1<<port);
        for(i=0; i<6; i++)
        {
            macAddrArray[tableIndex][i]=macAddr[i];
        }
        return SUCCESS;
    }
    else
    {   
        return FAILED;
    }

}

int32 rtl8306e_l2_mac_del(uint8 * macAddr, uint32 *entryaddr)
{
    int i = 0;
    int tableIndex = (((int)(macAddr[4] & 0x7)) << 8) |(macAddr[5]) ;
    *entryaddr = tableIndex;
    portMaskArray[tableIndex] = 0;
    for(i = 0; i < 6; i++)
    {
    macAddrArray[tableIndex][i] = 0;
    }

    return SUCCESS;
}

rtk_api_ret_t rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t portmask)
{
    rtk_api_ret_t retVal;
    uint32 entryAddr;

    /* must be L2 multicast address */
    if((NULL == pMac) || (!(pMac->octet[0] & 0x1)))
        return RT_ERR_MAC;

    if(portmask.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK;    

    retVal = rtl8306e_l2_multicastMac_add(pMac->octet, TRUE, portmask.bits[0], &entryAddr);

    return retVal;  
}

rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    uint32 entryAddr;

    /* must be multicast address */
    if((NULL == pMac) || !(pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    retVal = rtl8306e_l2_mac_del(pMac->octet, &entryAddr);

    return retVal;
}

#elif defined(CONFIG_RTL8306SDM)
int32  rtl8306_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask,uint32 *entryaddr)
{
    int i = 0;
    uint8 EqualFlag = 0;
    int tableIndex=(((int)(macAddr[4] & 0x7)) << 8) |(macAddr[5]) ;
    *entryaddr=tableIndex;
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

    if(((1 == isAuth) && (0 == portMaskArray[tableIndex])) || EqualFlag)
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

int32 rtl8306_addLUTUnicastMacAddress(uint8* macAddr, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port)
{
    int i = 0;
    uint8 EqualFlag = 0;
    int tableIndex = (((int)(macAddr[4] & 0x7)) << 8) |(macAddr[5]) ;
    for(i = 0; i < 6; i++)
    {
        if(macAddr[i]!=macAddrArray[tableIndex][i])
        {
            EqualFlag=0;
            break;
        }
        else
        {
            EqualFlag=1;
        }
    }

    if(((1 == isAuth) && (0 == portMaskArray[tableIndex])) || EqualFlag)
    {
        portMaskArray[tableIndex] = (1 << port);
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

int32 rtl8306_deleteMacAddress(uint8 * macAddr, uint32 *entryaddr)
{
    int i = 0;
    int tableIndex = (((int)(macAddr[4] & 0x7)) << 8) |(macAddr[5]) ;
    *entryaddr = tableIndex;
    portMaskArray[tableIndex] = 0;
    for(i = 0; i < 6; i++)
    {
        macAddrArray[tableIndex][i] = 0;
    }

    return SUCCESS;
}
#endif

#endif /* RTL_IGMP_SNOOPING_TEST */

