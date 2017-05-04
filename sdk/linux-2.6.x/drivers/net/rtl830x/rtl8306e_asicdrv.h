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
* $Date: 2011/03/03 08:39:16 $
*
* Purpose : asic-level driver implementation for RTL8306E switch
*
*  Feature :  This file consists of following modules:
*                1) 
*
*/


#ifndef __RTL8306E_ASICDRV_H__
#define __RTL8306E_ASICDRV_H__

/*save time of reading LUT*/
/*#define RTL8306_LUT_CACHE */

/*if you need backup asic info in cpu memroy in order to 
 *accellerate CPU process, please define this macro. If 
 *support IGMP snooping, this macro is required.
 */
#define RTL8306_TBLBAK  

#define RTL8306_PHY_NUMBER        7
#define RTL8306_PAGE_NUMBER      5
#define RTL8306_PORT_NUMBER      6
#define RTL8306_VLAN_ENTRYS      16            /*Vlan entry number*/ 
#define RTL8306_ACL_ENTRYNUM    16           /*ACL entry number*/

#define RTL8306_IDLE_TIMEOUT   100
#define RTL8306_QOS_RATE_INPUT_MAX 0x5F6
#define RTL8306_VIDMAX                       0XFFF
#define RTL8306_MAX_PORTMASK           0X3F


enum RTL8306E_REGPAGE
{
    RTL8306_REGPAGE0 = 0,
    RTL8306_REGPAGE1,
    RTL8306_REGPAGE2,
    RTL8306_REGPAGE3,
    RTL8306_REGPAGE4,    
    RTL8306_REGPAGE_END
    
};

enum RTL8306E_PORTNUM
{
    RTL8306_PORT0 = 0,
    RTL8306_PORT1,        
    RTL8306_PORT2,
    RTL8306_PORT3,  
    RTL8306_PORT4,
    RTL8306_PORT5,
    RTL8306_NOCPUPORT = 7    
};

enum RTL8306E_PHYMODE
{
    RTL8306_ETHER_AUTO_100FULL = 1,
    RTL8306_ETHER_AUTO_100HALF,
    RTL8306_ETHER_AUTO_10FULL,
    RTL8306_ETHER_AUTO_10HALF
};

enum RTL8306E_PHYSPD
{
    RTL8306_ETHER_SPEED_100 = 100,
    RTL8306_ETHER_SPEED_10 = 10        
};

enum RTL8306E_TAGMOD
{
    RTL8306_VLAN_IRTAG = 0,             /*The switch will remove VLAN tags and add new tags */
    RTL8306_VLAN_RTAG,                   /*The switch will remove VLAN tags */
    RTL8306_VLAN_ITAG,                   /*The switch will  add new VLANtag */
    RTL8306_VLAN_UNDOTAG            /*Do not insert or remove  VLAN tag */
};

enum RTL8306E_LEAKYVLAN
{
    RTL8306_VALN_LEAKY_UNICAST = 0,
    RTL8306_VALN_LEAKY_MULTICAST,
    RTL8306_VALN_LEAKY_ARP,
    RTL8306_VALN_LEAKY_MIRROR,
    RTL8306_VALN_LEAKY_END
};

/*ACL Packet processing method*/
enum RTL8306E_ACTION
{
    RTL8306_ACT_DROP = 0,    /*drop the packet*/    
    RTL8306_ACT_PERMIT,      /*permit the packet*/        
    RTL8306_ACT_TRAPCPU,   /*trap the packet to cpu*/
    RTL8306_ACT_MIRROR,     /*mirror the packet */
    RTL8306_ACT_FLOOD      /*flood the packet  */    
};

/*
  * PHY control register field definitions 
  */
#define RTL8306_SPEED_SELECT_100M                       (1 << 13)  
#define RTL8306_ENABLE_AUTONEGO                         (1 << 12)
#define RTL8306_RESTART_AUTONEGO                        (1 << 9)
#define RTL8306_SELECT_FULL_DUPLEX                      (1 << 8)

/* 
  *PHY auto-negotiation advertisement and link partner 
  *ability registers field definitions
  */
#define RTL8306_NEXT_PAGE_ENABLED                       (1 << 15)
#define RTL8306_ACKNOWLEDGE                                 (1 << 14)
#define RTL8306_REMOTE_FAULT                                 (1 << 13)
#define RTL8306_CAPABLE_PAUSE                               (1 << 10)
#define RTL8306_CAPABLE_100BASE_T4                      (1 << 9)
#define RTL8306_CAPABLE_100BASE_TX_FD                (1 << 8)
#define RTL8306_CAPABLE_100BASE_TX_HD                (1 << 7)
#define RTL8306_CAPABLE_10BASE_TX_FD                  (1 << 6)
#define RTL8306_CAPABLE_10BASE_TX_HD                  (1 << 5)
#define RTL8306_SELECTOR_MASK                               0x1F
#define RTL8306_SELECTOR_OFFSET                             0

enum RTL8306E_IGMPCTL
{
    RTL8306_IGMP = 0,
    RTL8306_MLD, 
    RTL8306_PPPOE
};

enum RTL8306E_IPMULTICAST
{
    RTL8306_IPV4_MULTICAST = 0,
    RTL8306_IPV6_MULTICAST
};

#define RTL8306_PORT_RX  0
#define RTL8306_PORT_TX  1

enum RTL8306E_QUENUM
{
    RTL8306_QUEUE0 = 0,
    RTL8306_QUEUE1,
    RTL8306_QUEUE2,
    RTL8306_QUEUE3
};

enum RTL8306E_PRISRC
{
    RTL8306_ACL_PRIO = 0,           /*ACL-based priority*/
    RTL8306_DSCP_PRIO,              /*DSCP-based priority*/
    RTL8306_1QBP_PRIO,              /*802.1Q-based priority*/ 
    RTL8306_PBP_PRIO,                /*port- based priority */
    RTL8306_CPUTAG_PRIO,          /*cpu tag priority*/
    RTL8306_IP_PRIO,                  /* ip address priority*/
    RTL8306_1QTAG_PRIO,           /* VLAN tag priority */
    RTL8306_PRI_1QDEFAULT      /* 802.1Q priority for untagged packet*/     
};


enum RTL8306E_1QPRI
{
    RTL8306_1QTAG_PRIO0 = 0,
    RTL8306_1QTAG_PRIO1,
    RTL8306_1QTAG_PRIO2,
    RTL8306_1QTAG_PRIO3,
    RTL8306_1QTAG_PRIO4,
    RTL8306_1QTAG_PRIO5,
    RTL8306_1QTAG_PRIO6,
    RTL8306_1QTAG_PRIO7,
    RTL8306_1QTAG_END
};

enum RTL8306E_PRI
{
    RTL8306_PRIO0 = 0,
    RTL8306_PRIO1,
    RTL8306_PRIO2,
    RTL8306_PRIO3,    
    RTL8306_PRI_END
};

enum RTL8306E_SCHSET
{
    RTL8306_QOS_SET0 = 0, 
    RTL8306_QOS_SET1        
};

enum RTL8306E_DSCPCODE
{
    RTL8306_DSCP_EF = 0,
    RTL8306_DSCP_AFL1,
    RTL8306_DSCP_AFM1,    
    RTL8306_DSCP_AFH1,
    RTL8306_DSCP_AFL2,
    RTL8306_DSCP_AFM2,
    RTL8306_DSCP_AFH2,
    RTL8306_DSCP_AFL3,
    RTL8306_DSCP_AFM3,
    RTL8306_DSCP_AFH3,
    RTL8306_DSCP_AFL4,
    RTL8306_DSCP_AFM4,
    RTL8306_DSCP_AFH4,
    RTL8306_DSCP_NC,
    RTL8306_DSCP_REG_PRI,
    RTL8306_DSCP_BF       
};

#define RTL8306_DSCP_USERA        0
#define RTL8306_DSCP_USERB        1
#define RTL8306_IPADD_A 0
#define RTL8306_IPADD_B 1

#define RTL8306_FCO_SET0            0x0
#define RTL8306_FCO_SET1            0x1
#define RTL8306_FCOFF                 0x0
#define RTL8306_FCON                   0x1
#define RTL8306_FCO_DSC             0x0
#define RTL8306_FCO_QLEN           0x1
#define RTL8306_FCO_FULLTHR      0x0
#define RTL8306_FCO_OVERTHR     0x1


#define RTL8306_ACL_INVALID       0x6
#define RTL8306_ACL_ANYPORT     0x7

enum RTL8306E_ACLPRO
{
    RTL8306_ACL_ETHER = 0,
    RTL8306_ACL_TCP,
    RTL8306_ACL_UDP,
    RTL8306_ACL_TCPUDP    
};

enum RTL8306E_MIBCNT
{
    RTL8306_MIB_CNT1 = 0,
    RTL8306_MIB_CNT2,
    RTL8306_MIB_CNT3,
    RTL8306_MIB_CNT4,
    RTL8306_MIB_CNT5 
};

enum RTL8306E_MIBOP
{
    RTL8306_MIB_RESET = 0, 
    RTL8306_MIB_START        
};

#define RTL8306_MIB_BYTE            0
#define RTL8306_MIB_PKT              1

#define RTL8306_MIR_INVALID       0x6

enum RTL8306E_LUT4WAY
{
    RTL8306_LUT_ENTRY0 = 0,
    RTL8306_LUT_ENTRY1,
    RTL8306_LUT_ENTRY2,
    RTL8306_LUT_ENTRY3    
};    

#define RTL8306_LUT_FULL            -2  /*Four way of the same entry are all written by cpu*/
#define RTL8306_LUT_NOTEXIST     -3

enum RTL8306E_LUTAGE
{
    RTL8306_LUT_AGEOUT = 0,
    RTL8306_LUT_AGE100 = 100,
    RTL8306_LUT_AGE200 = 200,
    RTL8306_LUT_AGE300 = 300   
};

#define RTL8306_LUT_DYNAMIC      0
#define RTL8306_LUT_STATIC         1
#define RTL8306_LUT_UNAUTH       0
#define RTL8306_LUT_AUTH           1

enum RTL8306E_SPAN_STATE
{
    RTL8306_SPAN_DISABLE = 0,
    RTL8306_SPAN_BLOCK,
    RTL8306_SPAN_LEARN,
    RTL8306_SPAN_FORWARD  
};

enum RTL8306E_DOT1X_STATE
{
    RTL8306_PORT_UNAUTH = 0, 
    RTL8306_PORT_AUTH
};

enum RTL8306E_DOT1X_PORT_DIR
{
    RTL8306_PORT_BOTHDIR = 0,
    RTL8306_PORT_INDIR      
};

enum RTL8306E_DOT1X_MAC_DIR
{
    RTL8306_MAC_BOTHDIR = 0,
    RTL8306_MAC_INDIR      
};

enum RTL8306E_ABNORMAL_PKT
{
    RTL8306_UNMATCHVID =0,
    RTL8306_DOT1XUNAUTH
};

enum RTL8306E_RMA
{
    RTL8306_RESADDRXX = 0, /*reserved address 01-80-c2-00-00-xx (exclude 00, 01, 02, 03, 10, 20, 21) */
    RTL8306_RESADDR21,       /*reserved address 01-80-c2-00-00-21*/        
    RTL8306_RESADDR20,       /*reserved address 01-80-c2-00-00-20*/
    RTL8306_RESADDR10,       /*reserved address 01-80-c2-00-00-10*/
    RTL8306_RESADDR03,      /*reserved address 01-80-c2-00-00-03*/
    RTL8306_RESADDR02,      /*reserved address 01-80-c2-00-00-02*/
    RTL8306_RESADDR00,      /*reserved address 01-80-c2-00-00-00*/
    RTL8306_RESADDR01     /*reserved address 01-80-c2-00-00-01*/
};

enum RTL8306E_PKT_TYPE
{
    RTL8306_UNICASTPKT = 0,  /*Unicast packet, but not include unknown DA unicast packet*/
    RTL8306_BROADCASTPKT,   /*Broadcast packet*/
    RTL8306_MULTICASTPKT,    /*Multicast packet*/
    RTL8306_UDAPKT             /*Unknown DA unicast packet*/
};


/*Max packet length*/
enum RTL8306E_PKT_LEN
{
    RTL8306_MAX_PKT_LEN_1518 = 0,  /*1518 bytes without any tag; 1522 bytes: with VLAN tag or CPU tag*/
    RTL8306_MAX_PKT_LEN_1536,       /*1536 bytes*/
    RTL8306_MAX_PKT_LEN_1552,       /*1552 bytes*/
    RTL8306_MAX_PKT_LEN_2000       /*1552 bytes*/
};

enum RTL8306E_STORM_THR
{
    RTL8306_STM_FILNUM64  =  0,     /*64 pkts will trigger storm fileter*/
    RTL8306_STM_FILNUM32,            /*32 pkts will trigger storm fileter*/
    RTL8306_STM_FILNUM16,            /*16 pkts will trigger storm fileter*/
    RTL8306_STM_FILNUM8,              /*8 pkts will trigger storm fileter*/
    RTL8306_STM_FILNUM128,          /*128 pkts will trigger storm fileter*/
    RTL8306_STM_FILNUM256,          /*256 pkts will trigger storm fileter*/
    RTL8306_STM_FILNUM_END    
};

enum RTL8306E_STORM_TIMEWIN
{
    RTL8306_STM_FIL800MS = 0,       /*filter 800ms after trigger storm filter*/
    RTL8306_STM_FIL400MS,            /*filter 400ms after trigger storm filter*/
    RTL8306_STM_FIL200MS,            /*filter 200ms after trigger storm filter*/
    RTL8306_STM_FIL100MS,            /*filter 100ms after trigger storm filter*/
    RTL8306_STM_FILTIM_END
};

typedef enum rtl8306e_acceptFrameType_e
{
    RTL8306E_ACCEPT_ALL = 0,                   /* untagged, priority-tagged and tagged */
    RTL8306E_ACCEPT_TAG_ONLY = 2,         /* tagged */
    RTL8306E_ACCEPT_UNTAG_ONLY = 3 ,    /* untagged and priority-tagged */
    RTL8306E_ACCEPT_TYPE_END
} rtl8306e_acceptFrameType_t;

typedef enum rtl8306e_vidSrc_e
{
    RTL8306E_VIDSRC_POVID = 0,              /*port-based outer tag vid*/
    RTL8306E_VIDSRC_NVID,                     /* new vid translated*/
    RTL8306E_VIDSRC_END    

} rtl8306e_vidSrc_t;

typedef enum rtl8306e_priSrc_e
{
    RTL8306E_PRISRC_PPRI = 0,              /*port-based outer tag priority*/
    RTL8306E_PRISRC_1PRMK,                /* 1p remarking priority*/
    RTL8306E_PRISRC_END
} rtl8306e_priSrc_t;


typedef struct rtl8306e_qosPriArbitPara_s 
{
    uint32 acl_pri_lev;
    uint32 dscp_pri_lev;
    uint32 dot1q_pri_lev;
    uint32 port_pri_lev;
    uint32 vid_pri_lev;
} rtl8306e_qosPriArbitPara_t;

typedef struct rtl8306e_qosSchPara_s
{
    uint8   q0_wt;
    uint8   q1_wt;
    uint8   q2_wt;
    uint8   q3_wt;
    uint16 q2_n64Kbps;
    uint16 q3_n64Kbps;    

} rtl8306e_qosSchPara_t;

#ifdef RTL8306_LUT_CACHE
#define RTL8306_MAX_LUT_NUM    2048
typedef struct RTL8306_LUT_s
{
    uint8   mac[6];
    union
    {
        struct 
        {
            uint8 auth;
            uint8 isStatic;
            uint8 spa;
            uint8 age;
            uint8 reserved;
        } unicast;
        struct 
        {
            uint8 auth;
            uint8 portMask;
            uint8 reserved;
        } multicast;
    }un;
}RTL8306_LUT;
#endif

#ifdef RTL8306_TBLBAK
typedef struct rtl8306_vlanConfigBakPara_s 
{
    uint8 enVlan;
    uint8 enArpVlan;
    uint8 enLeakVlan;
    uint8 enVlanTagOnly;
    uint8 enIngress;
    uint8 enTagAware;
    uint8 enIPMleaky;
    uint8 enMirLeaky;
} rtl8306_vlanConfigBakPara_t;

typedef struct rtl8306_vlanConfigPerPortBakPara_s 
{
    uint8 vlantagInserRm;
    uint8 en1PRemark;
    uint8 enNulPvidRep;         
} rtl8306_vlanConfigPerPortBakPara_t;

typedef struct  rtl8306_vlanTblBakPara_s 
{
    uint16 vid;
    uint8 memberPortMask;        
} rtl8306_vlanTblBakPara_t;

typedef struct rtl8306_aclTblBakPara_s 
{
    uint8 phy_port;
    uint8 proto;
    uint16 data;
    uint8 action;
    uint8 pri;
} rtl8306_aclTblBakPara_t;

typedef struct rtl8306_mirConfigBakPara_s
{
    uint8 mirPort;
    uint8 mirRxPortMask;
    uint8 mirTxPortMask;
    uint8 enMirself;
    uint8 enMirMac;
    uint8 mir_mac[6];
} rtl8306_mirConfigBakPara_t;
    

typedef struct rtl8306_ConfigBakPara_s  
{
    rtl8306_vlanConfigBakPara_t vlanConfig;                    /*VLAN global configuration*/
    rtl8306_vlanConfigPerPortBakPara_t vlanConfig_perport[6];   /*VLAN per-port configuration*/
    rtl8306_vlanTblBakPara_t vlanTable[16]; /*It backups VLAN table in cpu memory*/
    uint8 vlanPvidIdx[6];   /*per-port PVID index*/                  
    uint8 En1PremarkPortMask; /*Enable/disable 802.1P remarking  port mask */
    uint8 dot1PremarkCtl[4]; /*802.1p remarking table*/
    uint8 dot1DportCtl[6]; /*Spanning tree port state*/
    rtl8306_aclTblBakPara_t aclTbl[16];         /*ACL table*/
    rtl8306_mirConfigBakPara_t mir; /*mirror configuration*/                                                                         
} rtl8306_ConfigBakPara_t;

extern rtl8306_ConfigBakPara_t rtl8306_TblBak; 

#endif


#define RTL8306_GET_REG_ADDR(x, page, phy, reg) \
    do { (page) = ((x) & 0xFF0000) >> 16; (phy) = ((x) & 0x00FF00) >> 8; (reg) = ((x) & 0x0000FF);\
    } while(0) \

/*compute look up table index of a mac addrees, LUT index : MAC[13:15] + MAC[0:5]*/
#define RTL8306_MAC_INDEX(mac, index)    rtl8306e_l2_MacToIdx_get(mac, &index)


/* Function Name:
 *      rtl8306e_reg_set
 * Description:
 *      Write Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~3)
 *      value    - Value to be write into the register
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable registers of RTL8306, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
extern int32 rtl8306e_reg_set(uint32 phyad, uint32 regad, uint32 npage, uint32 value) ;

/* Function Name:
 *      rtl8306e_reg_get
 * Description:
 *      Read Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~3)
 * Output:
 *      pvalue    - The pointer of value read back from register
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable registers of RTL8306, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
extern int32 rtl8306e_reg_get(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue);

/* Function Name:
 *      rtl8306e_regbit_set
 * Description:
 *      Write one bit of Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      bit        - Specify bit position(0 ~ 15)
 *      npage   - Specify page number (0 ~3)
 *      value    - Value to be write(0, 1)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function  you could write each bit of  all configurable registers of RTL8306.
 */
extern int32 rtl8306e_regbit_set(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 value);

/* Function Name:
 *      rtl8306e_regbit_get
 * Description:
 *      Read one bit of Asic  PHY Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      bit        - Specify bit position(0 ~ 15)
 *      npage   - Specify page number (0 ~3)
 * Output:
 *      pvalue  - The pointer of value read back
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could read each bit of  all configurable registers of RTL8306
 */
extern int32 rtl8306e_regbit_get(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 * pvalue) ;

/* Function Name:
 *      rtl8306e_phyReg_set
 * Description:
 *      Write PCS page register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~5)
 *      value    - Value to be write into the register
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable pcs registers of RTL8306, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
extern int32 rtl8306e_phyReg_set(uint32 phyad, uint32 regad, uint32 npage, uint32 value);

/* Function Name:
 *      rtl8306e_phyReg_get
 * Description:
 *      Read PCS page register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~5)
 * Output:
 *      pvalue    - The pointer of value read back from register
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable pcs registers of RTL8306, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
extern int32 rtl8306e_phyReg_get(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue);

extern int32 rtl8306e_asic_init(void);

/* Function Name:
 *      rtl8306e_phy_reset
 * Description:
 *      Reset the phy
 * Input:
 *      phy   - Specify Phy address (0 ~6)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
extern int32 rtl8306e_phy_reset(uint32 phy);

/* Function Name:
 *      rtl8306e_switch_maxPktLen_set
 * Description:
 *      set Max packet length which could be forwarded by
 * Input:
 *      maxLen         -  max packet length
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      maxLen could be set : 
 *      RTL8306_MAX_PKT_LEN_1518 -1518 bytes without any tag; 1522 bytes: 
 *              with VLAN tag or CPU tag, 1526 bytes with CPU and VLAN tag;
 *      RTL8306_MAX_PKT_LEN_1536 - 1536 bytes (all tags counted);
 *      RTL8306_MAX_PKT_LEN_1552 - 1552 bytes (all tags counted); 
 *      RTL8306_MAX_PKT_LEN_2000 - 2000 bytes (all tags counted) 
 *              
 */ 
extern int32 rtl8306e_switch_maxPktLen_set(uint32 maxLen);

/* Function Name:
 *      rtl8306e_switch_maxPktLen_get
 * Description:
 *      set Max packet length which could be forwarded by
 * Input:
 *      none
 * Output:
 *      maxLen         -  max packet length
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      maxLen could be set : 
 *      RTL8306_MAX_PKT_LEN_1518 -1518 bytes without any tag; 1522 bytes: 
 *              with VLAN tag or CPU tag, 1526 bytes with CPU and VLAN tag;
 *      RTL8306_MAX_PKT_LEN_1536 - 1536 bytes (all tags counted);
 *      RTL8306_MAX_PKT_LEN_1552 - 1552 bytes (all tags counted); 
 *      RTL8306_MAX_PKT_LEN_2000 - 2000 bytes (all tags counted) 
 *              
 */ 
extern int32 rtl8306e_switch_maxPktLen_get(uint32 *pMaxLen);

/*
  *  physical port function
  */

/* Function Name:
 *      rtl8306e_port_etherPhy_set
 * Description:
 *      Configure PHY setting
 * Input:
 *      phy                    - Specify the phy to configure
 *      autoNegotiation    - Specify whether enable auto-negotiation
 *      advCapability       - When auto-negotiation is enabled, specify the advertised capability
 *      speed                 - When auto-negotiation is disabled, specify the force mode speed
 *      fullDuplex            - When auto-negotiatoin is disabled, specify the force mode duplex mode
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
 *      When auto-negotiation is disabled, the phy is configured into force mode and the speed and duplex mode 
 *      setting is based on speed and fullDuplex setting.Port number should be smaller than RTL8306_PHY_NUMBER.
 *      AdverCapability should be ranged between RTL8306_ETHER_AUTO_100FULL and RTL8306_ETHER_AUTO_10HALF.
 *      Speed should be either RTL8306_ETHER_SPEED_100 or RTL8306_ETHER_SPEED_10.
 */

extern int32 rtl8306e_port_etherPhy_set(uint32 phy, uint32 autoNegotiation, uint32 advCapability, uint32 speed, uint32 fullDuplex) ;

/* Function Name:
 *      rtl8306e_port_etherPhy_get
 * Description:
 *       Get PHY setting
 * Input:
 *      phy                    - Specify the phy to configure
 * Output:
 *      pAutoNegotiation    - Get whether auto-negotiation is enabled
 *      pAdvCapability       - When auto-negotiation is enabled, Get the advertised capability
 *      pSpeed                 - When auto-negotiation is disabled, Get the force mode speed
 *      pFullDuplex            - When auto-negotiatoin is disabled, Get the force mode duplex mode

 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
 *      When auto-negotiation is disabled, the phy is configured into force mode and the speed and duplex mode 
 *      setting is based on speed and fullDuplex setting.Port number should be smaller than RTL8306_PHY_NUMBER.
 *      AdverCapability should be ranged between RTL8306_ETHER_AUTO_100FULL and RTL8306_ETHER_AUTO_10HALF.
 *      Speed should be either RTL8306_ETHER_SPEED_100 or RTL8306_ETHER_SPEED_10.
 */
extern int32 rtl8306e_port_etherPhy_get(uint32 phy, uint32 *pAutoNegotiation, uint32 *pAdvCapability, uint32 *pSpeed, uint32 *pFullDuplex);

/* Function Name:
 *      rtl8306e_port_port5LinkStatus_set
 * Description:
 *      Force port 5 link up or link down
 * Input:
 *      enabled   - true or false
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Port 5 should be manully enable / disable
 */
extern int32 rtl8306e_port_port5LinkStatus_set(uint32 enabled) ;

/* Function Name:
 *      rtl8306e_port_port5LinkStatus_get
 * Description:
 *      get port 5 link status
 * Input:
 *      none
 * Output:
*      enabled   - true or false
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Port 5 should be manully enable / disable
 */
extern int32 rtl8306e_port_port5LinkStatus_get(uint32 *pEnabled);

/* Function Name:
 *      rtl8306e_port_phyLinkStatus_get
 * Description:
 *      Get PHY Link Status
 * Input:
*      phy        - Specify the phy 
 * Output:
*      plinkUp   - Describe whether link status is up or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       Read the link status of PHY register 1
 */
extern int32 rtl8306e_port_phyLinkStatus_get(uint32 phy, uint32 *plinkUp);

/* Function Name:
 *      rtl8306e_port_phyAutoNegotiationDone_get
 * Description:
 *      Get PHY auto-negotiation result status
 * Input:
 *      phy      - Specify the phy to get status
 * Output:
*      pDone   -  Describe whether auto-negotiation is done or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Read the auto-negotiation complete of PHY register 1.
 */

extern int32 rtl8306e_port_phyAutoNegotiationDone_get(uint32 phy, uint32 *pDone) ;

/* Function Name:
 *      rtl8306e_port_phyLoopback_set
 * Description:
 *       Set PHY loopback
 * Input:
 *      phy         - Specify the phy to configure
 *      enabled   - Enable phy loopback
 * Output:
 *      none      
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Only phy 0~4 could be configured the phy loopback
 */
extern int32 rtl8306e_port_phyLoopback_set(uint32 phy, uint32 enabled);

/* Function Name:
 *      rtl8306e_port_phyLoopback_get
 * Description:
 *      Get PHY loopback setting
 * Input:
 *      phy         - Specify the phy to get status
 * Output:
 *      pEnabled  -  phy loopback setting
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
extern int32 rtl8306e_port_phyLoopback_get(uint32 phy, uint32 *pEnabled) ;

/* Function Name:
 *      rtl8306e_portLearningAbility_set
 * Description:
 *      Enable/disable physical port learning ability
 * Input:
 *      port        - Specify port number (0 ~ 5)
 * Output:
 *      enabled -  TRUE or FALSE
 * Return:
 *      SUCCESS
 * Note:
 *
 */
extern int32 rtl8306e_portLearningAbility_set(uint32 port, uint32 enabled);


/* Function Name:
 *      rtl8306e_port_isolation_set
 * Description:
 *      set port isolation 
 * Input:
 *      isomsk    - port isolation port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      each bit of isomsk determine two port's isolation,
 *      1 means two port could not forward packet between
 *      each other.
 *      bit 0  - Port0 & Port1 
 *      bit 1  - Port0 & Port2 
 *      bit 2  - Port0 & Port3
 *      bit 3  - Port0 & Port4
 *      bit 4  - Port0 & Port5
 *      bit 5  - Port1 & Port2
 *      bit 6  - Port1 & Port3
 *      bit 7  - Port1 & Port4
 *      bit 8  - Port1 & Port5
 *      bit 9  - Port2 & Port3
 *      bit 10 - Port2 & Port4
 *      bit 11 - Port2 & Port5
 *      bit 12 - Port3 & Port4
 *      bit 13 - Port3 & Port5
 *      bit 14 - Port4 & Port5
 */
extern int32 rtl8306e_port_isolation_set(uint32 isomsk);

/* Function Name:
 *      rtl8306e_port_isolation_set
 * Description:
 *      set port isolation 
 * Input:
 *      none
 * Output:
 *      pIsomsk    -  the pointer of port isolation port mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      each bit of isomsk determine two port's isolation,
 *      1 means two port could not forward packet between
 *      each other.
 *      bit 0  - Port0 & Port1 
 *      bit 1  - Port0 & Port2 
 *      bit 2  - Port0 & Port3
 *      bit 3  - Port0 & Port4
 *      bit 4  - Port0 & Port5
 *      bit 5  - Port1 & Port2
 *      bit 6  - Port1 & Port3
 *      bit 7  - Port1 & Port4
 *      bit 8  - Port1 & Port5
 *      bit 9  - Port2 & Port3
 *      bit 10 - Port2 & Port4
 *      bit 11 - Port2 & Port5
 *      bit 12 - Port3 & Port4
 *      bit 13 - Port3 & Port5
 *      bit 14 - Port4 & Port5
 */
extern int32 rtl8306e_port_isolation_get(uint32 *pIsomsk);

/*
  *  VLAN function
  */

/* Function Name:
 *      rtl8306e_vlan_tagAware_set
 * Description:
 *      Configure switch to be VLAN tag awared
 * Input:
 *      enabled  - Configure RTL8306 VLAN tag awared
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If switch is unawared VLAN tag, packet with vlan tag is treated as untagged pkt
 *      and assigned PVID as VID.
 */
extern int32 rtl8306e_vlan_tagAware_set(uint32 enabled);

/* Function Name:
 *      rtl8306e_vlan_tagAware_set
 * Description:
 *      Get switch to be VLAN tag awared  or not
 * Input:
 *      none
 * Output:
 *      pEnabled  - the pointer of RTL8306 VLAN tag awared status
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If switch is unawared VLAN tag, packet with vlan tag is treated as untagged pkt
 *      and assigned PVID as VID.
 */
extern int32 rtl8306e_vlan_tagAware_get(uint32 * pEnabled);

/* Function Name:
 *      rtl8306e_vlan_IgrFilterEnable_set
 * Description:
 *      Configure VLAN ingress filter
 * Input:
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
extern int32 rtl8306e_vlan_IgrFilterEnable_set(uint32 enabled); 

/* Function Name:
 *      rtl8306e_vlan_IgrFilterEnable_get
 * Description:
 *      Get VLAN ingress filter enabled or disabled
 * Input:
 *      none
 * Output:
 *      pEnabled  - enable or disable
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
extern int32 rtl8306e_vlan_IgrFilterEnable_get(uint32 *pEnabled); 

/* Function Name:
 *      rtl8306e_vlan_leaky_set
 * Description:
 *      Configure switch to forward frames to other VLANs ignoring the egress rule.
 * Input:
 *      type   -  vlan leaky type
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_VALN_LEAKY_UNICAST - Vlan leaky for unicast pkt
 *          RTL8306_VALN_LEAKY_MULTICAST - Vlan leaky for multicast pkt
 *          RTL8306_VALN_LEAKY_ARP - Vlan leaky for ARP brodcast pkt 
 *          RTL8306_VALN_LEAKY_MIRROR - Vlan leaky for mirror function
 *    1.When the Vlan leaky for unicast pkt is enabled, it enables the inter-VLANs unicast packet forwarding. 
 *    That is, if the L2 look up MAC table search hit, then the unicast packet will be forwarded
 *    to the egress port ignoring the egress rule.
 *    2.When Vlan leaky for multicast pkt is enabled, multicast packet may be flood to all multicast address
 *    group member set, ignoring the VLAN member set domain limitation.
 *    3.When Vlan leaky for ARP pkt is enabled, the ARP broadcast packets will be forward to all the other
 *    ports ignoring the egress rule.
 *    4.When Vlan leaky for mirror function is enabled, it enables the inter-VLANs mirror function, 
 *    ignoring the VLAN member set domain limitation.
 */
extern int32 rtl8306e_vlan_leaky_set(uint32 type, uint32 enabled);

/* Function Name:
 *      rtl8306e_vlan_leaky_get
 * Description:
 *      Get switch whether forwards unicast frames to other VLANs
 * Input:
 *      type   -  vlan leaky type
 * Output:
 *      pEnabled  - the pointer of Vlan Leaky status(Dsiabled or Enabled) 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *   type coulde be:
 *          RTL8306_VALN_LEAKY_UNICAST - Vlan leaky for unicast pkt
 *          RTL8306_VALN_LEAKY_MULTICAST - Vlan leaky for multicast pkt
 *          RTL8306_VALN_LEAKY_ARP - Vlan leaky for ARP brodcast pkt 
 *    1.When the Vlan leaky for unicast pkt is enabled, it enables the inter-VLANs unicast packet forwarding. 
 *    That is, if the L2 look up MAC table search hit, then the unicast packet will be forwarded
 *    to the egress port ignoring the egress rule.
 *    2.When Vlan leaky for multicast pkt is enabled, multicast packet may be flood to all multicast address
 *    group member set, ignoring the VLAN member set domain limitation.
 *    3.When Vlan leaky for ARP pkt is enabled, the ARP broadcast packets will be forward to all the other
 *    ports ignoring the egress rule.
 *    4.When Vlan leaky for mirror function is enabled, it enables the inter-VLANs mirror function, 
 *    ignoring the VLAN member set domain limitation.
 */
extern int32 rtl8306e_vlan_leaky_get(uint32 type, uint32 *pEnabled);

/* Function Name:
 *      rtl8306e_vlan_nullVidReplace_set
 * Description:
 *      Configure switch to replace Null VID tagged frame by PVID if it is tag aware
 * Input:
 *      port   -  port number
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.When Null VID replacement is enabled, 8306E only captures tagged packet with VID=0,
 *      then replace VID with input port's PVID. If switch received a packet that is not tagged, 
 *      it will not insert a tag with PVID to this packet.
 *      2. When Null VID replacement is disabled, switch will drop or deal the null VID tagged 
 *      frame depends on the configuration.
 */
extern int32 rtl8306e_vlan_nullVidReplace_set(uint32 port, uint32 enabled);

/* Function Name:
 *      rtl8306e_vlan_nullVidReplace_get
 * Description:
 *      Configure switch to forward frames to other VLANs ignoring the egress rule.
 * Input:
 *      port   -  port number
 * Output:
 *      pEnabled  - the pointer of Null VID replacement ability(Dsiabled or Enabled) 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.When Null VID replacement is enabled, 8306E only captures tagged packet with VID=0,
 *      then replace VID with input port's PVID. If switch received a packet that is not tagged, 
 *      it will not insert a tag with PVID to this packet.
 *      2. When Null VID replacement is disabled, switch will drop or deal the null VID tagged 
 *      frame depends on the configuration.
 */
extern int32 rtl8306e_vlan_nullVidReplace_get(uint32 port, uint32 *pEnabled);

/* Function Name:
 *      rtl8306e_vlan_portPvidIndex_set
 * Description:
 *      Configure switch port PVID index 
 * Input:
 *      port           -   Specify the port(port 0 ~ port 5) to configure VLAN index
 *      vlanIndex    -   Specify the VLAN index
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 16 vlan entry, VID of vlan entry pointed by PVID index  is the PVID 
 */

extern int32 rtl8306e_vlan_portPvidIndex_set(uint32 port, uint32 vlanIndex);

/* Function Name:
 *      rtl8306e_vlan_portPvidIndex_get
 * Description:
 *      Get switch port PVID index 
 * Input:
 *      port            -   Specify the port(port 0 ~ port 5) to configure VLAN index
 * Output:
 *      pVlanIndex   -   pointer of VLAN index number
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 16 vlan entry, VID of vlan entry pointed by PVID index  is the PVID 
 */

extern int32 rtl8306e_vlan_portPvidIndex_get(uint32 port, uint32 *pVlanIndex);

/* Function Name:
 *      rtl8306e_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    RTL8306E_ACCEPT_ALL
 *    RTL8306E_ACCEPT_TAG_ONLY
 *    RTL8306E_ACCEPT_UNTAG_ONLY
 */
extern int32 rtl8306e_vlan_portAcceptFrameType_set(uint32 port, rtl8306e_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtl8306e_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                                 - Port id
 * Output:
 *      pAccept_frame_type             - accept frame type pointer
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    RTL8306E_ACCEPT_ALL
 *    RTL8306E_ACCEPT_TAG_ONLY
 *    RTL8306E_ACCEPT_UNTAG_ONLY
 */
extern int32 rtl8306e_vlan_portAcceptFrameType_get(uint32 port, rtl8306e_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtl8306e_vlan_tagInsert_set
 * Description:
 *      Insert VLAN tag by ingress port
 * Input:
 *      egPort               - egress port number 0~5
 *      igPortMsk           - ingress port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      egPort is packet egress port, if the packet is untagged and its igress port
 *      is in the igPortMsk, it will be inserted with an VLAN tag.
 */
extern int32 rtl8306e_vlan_tagInsert_set(uint32 egPort, uint32 igPortMsk);

/* Function Name:
 *      rtl8306e_vlan_tagInsert_get
 * Description:
 *      get  ingress port mask of VLAN tag insertion for untagged packet
 * Input:
 *      egPort               - egress port number 0~5
 *      igPortMsk           - ingress port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      egPort is packet egress port, if the packet is untagged and its igress port
 *      is in the igPortMsk, it will be inserted with an VLAN tag.
 */
extern int32 rtl8306e_vlan_tagInsert_get(uint32 egPort, uint32 * pIgPortMsk);

/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry
 * Input:
 *      vlanIndex  - VLAN entry index
 *      vid           - VLAN ID to configure
 *      mbrmsk     - VLAN member set portmask
 *      untagmsk  - VLAN untag set portmask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 */
extern int32 rtl8306e_vlan_entry_set(uint32 vlanIndex, uint32 vid, uint32 mbrmsk, uint32 untagmsk );

/* Function Name:
 *      rtl8306e_vlan_entry_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vlanIndex  - VLAN entry index
 * Output:
 *      pVid           -  the pointer of VLAN ID 
 *      pMbrmsk     -  the pointer of VLAN member set portmask
 *      pUntagmsk  -  the pointer of VLAN untag set portmask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 */
extern int32 rtl8306e_vlan_entry_get(uint32 vlanIndex, uint32 *pVid, uint32 *pMbrmsk, uint32 *pUntagmsk);

/* Function Name:
 *      rtl8306e_vlan_vlanBasedPriority_set
 * Description:
 *       Set VID based priority
 * Input:
 *      vlanIndex   -   Specify VLAN entry index
 *      pri            -   the specified VLAN priority  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      each VLAN could be assigned an priority. if vlanIndex > 15, it means the VID
 *      is not in VLAN entries, unmatched VID could also be assigned an priority.
 *       
 */
extern int32 rtl8306e_vlan_vlanBasedPriority_set(uint32 vlanIndex, uint32 pri);

/* Function Name:
 *      rtl8306e_vlan_vlanBasedPriority_get
 * Description:
 *       Get VID based priority
 * Input:
 *      vlanIndex   -   Specify VLAN entry index
 * Output:
 *      pPri           -   the pointer of specified VLAN priority  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      each VLAN could be assigned an priority. if vlanIndex > 15, it means the VID
 *      is not in VLAN entries, unmatched VID could also be assigned an priority.
 *       
 */
extern int32 rtl8306e_vlan_vlanBasedPriority_get(uint32 vlanIndex, uint32 *pPri);

/* Function Name:
 *      rtl8306e_vlan_transEnable_set
 * Description:
 *      Enable VLAN translation 
 * Input:
 *      enable       -   enable or disable VLAN translation
 *      portmask    -   NNI port is set 1 and UNI port is set 0
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Only the traffic between NNI port and UNI port will be change with VID,
 *      the VLAN tranlation function and Q-in-Q(SVLAN) could not be enabled  
 *      at the same time, user should choose one of them. 
 */
extern int32 rtl8306e_vlan_transEnable_set(uint32 enable, uint32 portmask);

/* Function Name:
 *      rtl8306e_vlan_transEnable_get
 * Description:
 *      Get VLAN translation setting
 * Input:
 *      none
 * Output:
 *      pEnable       -   the pointer of enable or disable VLAN translation
 *      pPortMask    -   the pointer of VLAN translation port mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Only the traffic between NNI port and UNI port will be change with VID,
 *      the VLAN tranlation function and Q-in-Q(SVLAN) could not be enabled  
 *      at the same time, user should choose one of them. 
 */
extern int32 rtl8306e_vlan_transEnable_get(uint32 *pEnable, uint32 *pPortMask);

/* Function Name:
 *      rtl8306e_vlan_transparentEnable_set
 * Description:
 *      Enable VLAN transparent 
 * Input:
 *      enable       -   enable or disable VLAN transparent
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
extern int32 rtl8306e_vlan_transparentEnable_set(uint32 enable);

/* Function Name:
 *      rtl8306e_vlan_transVid_set
 * Description:
 *      Set the translated VID
 * Input:
 *      vlanIndex   -   the VLAN entry index
 *      transVid     -   the new VID
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The VID in the entry indexed by vlanIndex will be replaced with
 *      new VID.
 */ 
extern int32 rtl8306e_vlan_transVid_set(uint32 vlanIndex, uint32 transVid);


/* Function Name:
 *      rtl8306e_vlan_transVid_get
 * Description:
 *      Get the translated VID
 * Input:
 *      vlanIndex   -   the VLAN entry index
 * Output:
 *      pTransVid   -   the pointer of the new VID
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The VID in the entry indexed by vlanIndex will be replaced with
 *      new VID.
 */ 
extern int32 rtl8306e_vlan_transVid_get(uint32 vlanIndex, uint32 *pTransVid);


/* Function Name:
 *      rtl8306e_svlan_tagAdmit_set
 * Description:
 *      Set Q-in-Q tag admit control
 * Input:
 *      port          -   port id
 *      enable       -   enable tag admit control 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       
 */
extern int32 rtl8306e_svlan_tagAdmit_set(uint32 port, uint32 enable);

/* Function Name:
 *      rtl8306e_svlan_tagAdmit_get
 * Description:
 *      Get Q-in-Q tag admit control
 * Input:
 *      port          -   port id
 * Output:
 *      pEnable     -   the pointer of enable tag admit control 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       
 */
extern int32 rtl8306e_svlan_tagAdmit_get(uint32 port, uint32 *pEnable);

/* Function Name:
 *      rtl8306e_svlan_otagSrc_set
 * Description:
 *      Set how to decide outer tag vid and priority 
 * Input:
 *      port          -   port id
 *      ovidSrc      -   ovid comes from
 *      opriSrc      -    opri comes from 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       ovidSrc RTL8306E_VIDSRC_POVID : ovid is port-based ovid,    RTL8306E_VIDSRC_NVID:ovid is new vid(translated vid)
 *       opriSrc  RTL8306E_PRISRC_PPRI   : opri is port-based priority, RTL8306E_PRISRC_1PRMK: opri is 1p remarking value
 */
extern int32 rtl8306e_svlan_otagSrc_set(uint32 port, uint32 ovidSrc, uint32 opriSrc);

/* Function Name:
 *      rtl8306e_svlan_otagSrc_get
 * Description:
 *      Get how to decide outer tag vid and priority 
 * Input:
 *      port            -   port id
 * Output:
 *      pOvidsrc      -   the pointer of ovid comes from
 *      pOpriSrc      -   the pointer of opri comes from 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       ovidSrc RTL8306E_VIDSRC_POVID : ovid is port-based ovid,    RTL8306E_VIDSRC_NVID:ovid is new vid(translated vid)
 *       opriSrc  RTL8306E_PRISRC_PPRI   : opri is port-based priority, RTL8306E_PRISRC_1PRMK: opri is 1p remarking value
 */
extern int32 rtl8306e_svlan_otagSrc_get(uint32 port, uint32 *pOvidsrc, uint32 *pOpriSrc);

/* Function Name:
 *      rtl8306e_cpu_set
 * Description:
 *       Specify Asic CPU port 
 * Input:
 *      port       -   Specify the port
 *      enTag    -    CPU tag insert or not
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If the port is specified RTL8306_NOCPUPORT, it means
 *      that no port is assigned as cpu port
 */
extern int32 rtl8306e_cpu_set(uint32 port, uint32 enTag) ;

/* Function Name:
 *      rtl8306e_cpu_set
 * Description:
 *       Get Asic CPU port number
 * Input:
 *      none
 * Output:
 *      pPort     - the pointer of CPU port number
 *      pEnTag  - the pointer of CPU tag insert or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If the port is specified RTL8306_NOCPUPORT, it means
 *      that no port is assigned as cpu port
 */

extern int32 rtl8306e_cpu_get(uint32 *pPort, uint32 *pEnTag) ;

/*
  *  QOS  function
  */

/* Function Name:
 *      rtl8306e_qos_softReset_set
 * Description:
 *      Software reset the asic
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      reset packet buffer.
 */
extern int32 rtl8306e_qos_softReset_set(void) ;

/* Function Name:
 *      rtl8306e_qos_queueNum_set
 * Description:
 *      Set egress port queue number (1 ~4)
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Queue number is global configuration for switch.
 */
extern int32 rtl8306e_qos_queueNum_set(uint32 num);

/* Function Name:
 *      rtl8306e_qos_queueNum_set
 * Description:
 *      Set egress port queue number (1 ~4)
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Queue number is global configuration for switch.
 */
extern int32 rtl8306e_qos_queueNum_get(uint32 *pNum) ;

/* Function Name:
 *      rtl8306e_qos_priToQueMap_set
 * Description:
 *     Set priority to Queue ID mapping
 * Input:
 *      priority   -  priority value (0 ~ 3)
 *      qid        -  Queue id (0~3)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Packets could be classified into specified queue through their priority. 
 *      we can use this function to set pkt priority with queue id mapping
 */
extern int32 rtl8306e_qos_priToQueMap_set(uint32 priority, uint32 qid) ;

/* Function Name:
 *      rtl8306e_qos_priToQueMap_get
 * Description:
 *      Get priority to Queue ID mapping
 * Input:
 *      priority   -  priority value (0 ~ 3)
 * Output:
 *      pQid      -  pointer of Queue id (0~3)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Packets could be classified into specified queue through their priority. 
 *      we can use this function to set pkt priority with queue id mapping
 */
extern int32 rtl8306e_qos_priToQueMap_get(uint32 priority, uint32 *pQid) ;

/* Function Name:
 *      rtl8306e_qos_portRate_set
 * Description:
 *      Set port bandwidth control
 * Input:
 *      port            -  port number (0~5)
 *      n64Kbps       -  Port rate (0~1526), unit 64Kbps
 *      direction      -  Ingress or Egress bandwidth control
 *      enabled       -  enable bandwidth control
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For each port, both input and output bandwidth could be configured, 
 *      RTL8306_PORT_RX represents port input bandwidth control, 
 *      RTL8306_PORT_TX represents port output bandwidth control.
 *      port rate unit is 64Kbps. For output rate control, enable/disable 
 *      is configured per port, but for input rate control, it is for all port.
 */
extern int32 rtl8306e_qos_portRate_set(uint32 port, uint32 n64Kbps, uint32 direction, uint32 enabled);

/* Function Name:
 *      rtl8306e_qos_portRate_get
 * Description:
 *      Get port bandwidth control rate
 * Input:
 *      port                 -  Port number (0~5)
 * Output:
 *      *pN64Kbps        -  Port rate (0~1526), unit 64Kbps
 *      direction           -  Input or output bandwidth control
 *      *enabled           -  enabled or disabled bandwidth control
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For each port, both input and output bandwidth could be configured, 
 *      RTL8306_PORT_RX represents port input bandwidth control, 
 *      RTL8306_PORT_TX represents port output bandwidth control.
 *      port rate unit is 64Kbps. For output rate control, enable/disable 
 *      is configured per port, but for input rate control, it is for all port.
 */
extern int32 rtl8306e_qos_portRate_get(uint32 port, uint32 *pN64Kbps, uint32 direction, uint32 *pEnabled);

/* Function Name:
 *      rtl8306e_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1P remarking ability
 * Input:
 *      port       -  port number (0~5)
 *      enabled  -  TRUE or FALSE
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      
 */
extern int32 rtl8306e_qos_1pRemarkEnable_set(uint32 port, uint32 enabled);

/* Function Name:
 *      rtl8306e_qos_1pRemarkEnable_set
 * Description:
 *      Get 802.1P remarking ability
 * Input:
 *      port        -  port number (0~5)
 * Output:
 *      pEnabled  -  pointer of the ability status
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      
 */
extern int32 rtl8306e_qos_1pRemarkEnable_get(uint32 port, uint32 *pEnabled) ;

/* Function Name:
 *      rtl8306e_qos_1pRemark_set
 * Description:
 *      Set 802.1P remarking priority
 * Input:
 *      priority       -  Packet priority(0~4)
 *      priority1p    -  802.1P priority(0~7)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch determines packet priority, the priority souce could
 *      be port-based, 1Q-based, dscp-based, vid-based, ip address,
 *      cpu tag.
 */
extern int32 rtl8306e_qos_1pRemark_set(uint32 priority, uint32 priority1p);

/* Function Name:
 *      rtl8306_getAsic1pRemarkingPriority
 * Description:
 *      Get 802.1P remarking priority
 * Input:
 *      priority       -  Packet priority(0~4)
 * Output:
 *      pPriority1p  -  the pointer of 802.1P priority(0~7)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch determines packet priority, the priority souce could
 *      be port-based, 1Q-based, dscp-based, vid-based, ip address,
 *      cpu tag.
 */
extern int32 rtl8306e_qos_1pRemark_get(uint32 priority, uint32 *pPriority1p);

/* Function Name:
 *      rtl8306e_qos_portPri_set
 * Description:
 *      Set port-based priority
 * Input:
 *      port          -  port number (0~5)
 *      priority      -  Packet port-based priority(0~4)
 * Output:
*       none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      packet will be assigned a port-based priority correspond to the ingress port.
 */
extern int32 rtl8306e_qos_portPri_set(uint32 port, uint32 priority);

/* Function Name:
 *      rtl8306e_qos_portPri_get
 * Description:
 *      Get port-based priority
 * Input:
 *      port          -  port number (0~5)
 * Output:
 *      pPriority    -   pointer of packet port-based priority(0~4)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      packet will be assigned a port-based priority correspond to the ingress port.
 */ 
extern int32 rtl8306e_qos_portPri_get(uint32 port, uint32 *pPriority);

/* Function Name:
 *      rtl8306e_qos_1pPriRemap_set
 * Description:
 *      Set Asic 1Q-tag priority mapping to 2-bit priority
 * Input:
 *      tagprio  -  1Q-tag proirty (0~7, 3 bit value)
 *      prio      -   internal use 2-bit priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch internal use 2-bit priority, so it should map 3-bit 1Q-tag priority
 *      to 2-bit priority
 */ 
extern int32 rtl8306e_qos_1pPriRemap_set(uint32 tagprio, uint32 prio) ;

/* Function Name:
 *      rtl8306e_qos_1pPriRemap_get
 * Description:
 *      Get Asic 1Q-tag priority mapping to 2-bit priority
 * Input:
 *      tagprio  -  1Q-tag proirty (0~7, 3 bit value)
 * Output:
 *      pPrio     -  pointer of  internal use 2-bit priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch internal use 2-bit priority, so it should map 3-bit 1Q-tag priority
 *      to 2-bit priority
 */ 
extern int32 rtl8306e_qos_1pPriRemap_get(uint32 tagprio, uint32 *pPrio);

/* Function Name:
 *      rtl8306e_dscpPriRemap_set
 * Description:
 *      Set DSCP-based priority
 * Input:
 *      code      -  dscp code
 *      priority   -  dscp-based priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch support 16 kinds of dscp code:
 *          RTL8306_DSCP_EF          
 *                 - DSCP for the Expedited forwarding PHB, 101110   
 *          RTL8306_DSCP_AFL1         
 *                 - DSCP for AF PHB Class 1 low drop, 001010
 *          RTL8306_DSCP_AFM1     
 *                 - DSCP for AF PHB Class 1 medium drop, 001100
 *          RTL8306_DSCP_AFH1      
 *                 - DSCP for AF PHB Class 1 high drop, 001110
 *          RTL8306_DSCP_AFL2       
 *                 - DSCP for AF PHB Class 2 low drop, 01001
 *          RTL8306_DSCP_AFM2       
 *                 - DSCP for AF PHB Class 2 medium drop, 010100
 *          RTL8306_DSCP_AFH2   
 *                 - DSCP for AF PHB Class 2 high drop, 010110
 *          RTL8306_DSCP_AFL3    
 *                 - DSCP for AF PHB Class 3 low drop, 011010
 *          RTL8306_DSCP_AFM3      
 *                 - DSCP for AF PHB Class 3 medium drop, 011100
 *          RTL8306_DSCP_AFH3    
 *                 - DSCP for AF PHB Class 3 high drop, 0111
 *          RTL8306_DSCP_AFL4     
 *                 - DSCP for AF PHB Class 4 low drop, 100010
 *          RTL8306_DSCP_AFM4    
 *                 - DSCP for AF PHB Class 4 medium drop, 100100
 *          RTL8306_DSCP_AFH4     
 *                 - DSCP for AF PHB Class 4 high drop, 100110
 *          RTL8306_DSCP_NC        
 *                 - DSCP for network control, 110000 or 111000
 *          RTL8306_DSCP_REG_PRI 
 *                 - DSCP Register match priority, user could define two dscp code
 *          RTL8306_DSCP_BF        
 *                 - DSCP Default PHB
 *     
 */ 
extern int32 rtl8306e_qos_dscpPriRemap_set(uint32 code, uint32 priority) ;

/* Function Name:
 *      rtl8306e_dscpPriRemap_set
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      code      -  dscp code
 * Output:
 *      pPriority  -  the pointer of dscp-based priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch support 16 kinds of dscp code:
 *          RTL8306_DSCP_EF          
 *                 - DSCP for the Expedited forwarding PHB, 101110   
 *          RTL8306_DSCP_AFL1         
 *                 - DSCP for AF PHB Class 1 low drop, 001010
 *          RTL8306_DSCP_AFM1     
 *                 - DSCP for AF PHB Class 1 medium drop, 001100
 *          RTL8306_DSCP_AFH1      
 *                 - DSCP for AF PHB Class 1 high drop, 001110
 *          RTL8306_DSCP_AFL2       
 *                 - DSCP for AF PHB Class 2 low drop, 01001
 *          RTL8306_DSCP_AFM2       
 *                 - DSCP for AF PHB Class 2 medium drop, 010100
 *          RTL8306_DSCP_AFH2   
 *                 - DSCP for AF PHB Class 2 high drop, 010110
 *          RTL8306_DSCP_AFL3    
 *                 - DSCP for AF PHB Class 3 low drop, 011010
 *          RTL8306_DSCP_AFM3      
 *                 - DSCP for AF PHB Class 3 medium drop, 011100
 *          RTL8306_DSCP_AFH3    
 *                 - DSCP for AF PHB Class 3 high drop, 0111
 *          RTL8306_DSCP_AFL4     
 *                 - DSCP for AF PHB Class 4 low drop, 100010
 *          RTL8306_DSCP_AFM4    
 *                 - DSCP for AF PHB Class 4 medium drop, 100100
 *          RTL8306_DSCP_AFH4     
 *                 - DSCP for AF PHB Class 4 high drop, 100110
 *          RTL8306_DSCP_NC        
 *                 - DSCP for network control, 110000 or 111000
 *          RTL8306_DSCP_REG_PRI 
 *                 - DSCP Register match priority, user could define two dscp code
 *          RTL8306_DSCP_BF        
 *                 - DSCP Default PHB
 *     
 */ 
extern int32 rtl8306e_qos_dscpPriRemap_get(uint32 code, uint32 *pPriority);

/* Function Name:
 *      rtl8306e_qos_priSrcArbit_set
 * Description:
 *      Set priority source arbitration level
 * Input:
 *      priArbit  - The structure describe levels of 5 kinds of priority 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch could recognize 7 types of priority source at most, 
 *      and a packet properly has all of them. among them, there 
 *      are 5 type priorities could be set priority level, they are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,
 *      Port-based priority, VID- based priority.each one could be 
 *      set level from 0 to 5, arbitration module will decide their sequece 
 *      to take, the highest level priority will be adopted at first, 
 *      then  priority type of the sencond highest level. priority with level 0
 *      will not be recognized any more. 
 */
extern int32 rtl8306e_qos_priSrcArbit_set(rtl8306e_qosPriArbitPara_t priArbit);

/* Function Name:
 *      rtl8306e_qos_priSrcArbit_get
 * Description:
 *      Get priority source arbitration level
 * Input:
 *      none 
 * Output:
 *      pPriArbit  - The structure describe levels of 5 kinds of priority 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch could recognize 7 types of priority source at most, 
 *      and a packet properly has all of them. among them, there 
 *      are 5 type priorities could be set priority level, they are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,
 *      Port-based priority, VID- based priority.each one could be 
 *      set level from 0 to 5, arbitration module will decide their sequece 
 *      to take, the highest level priority will be adopted at first, 
 *      then  priority type of the sencond highest level. priority with level 0
 *      will not be recognized any more. 
 */
extern int32 rtl8306e_qos_priSrcArbit_get(rtl8306e_qosPriArbitPara_t *pPriArbit);

/* Function Name:
 *      rtl8306e_qos_priSrcEnable_set
 * Description:
 *      enable/disable Qos priority source for  ingress port
 * Input:
 *      port      -  Specify port number (0 ~5)
 *      priSrc    -  Specify priority source  
 *      enabled -   TRUE of FALSE  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 kind of priority source for each port which could
 *     be enabled ordisabled:
 *          RTL8306_DSCP_PRIO     - DSCP-based priority
 *          RTL8306_1QBP_PRIO     - 1Q-based priority
 *          RTL8306_PBP_PRIO        - port-based priority
 *          RTL8306_CPUTAG_PRIO  - cpu tag priority
 */
extern int32 rtl8306e_qos_priSrcEnable_set(uint32 port, uint32 priSrc, uint32 enabled);

/* Function Name:
 *      rtl8306e_qos_priSrcEnable_set
 * Description:
 *      enable/disable Qos priority source for  ingress port
 * Input:
 *      port       -  Specify port number (0 ~5)
 *      priSrc     -  Specify priority source  
 * Output:
 *      pEnabled -  the pointer of priority source status  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 kind of priority source for each port which could
 *     be enabled ordisabled:
 *          RTL8306_DSCP_PRIO     - DSCP-based priority
 *          RTL8306_1QBP_PRIO     - 1Q-based priority
 *          RTL8306_PBP_PRIO        - port-based priority
 *          RTL8306_CPUTAG_PRIO  - cpu tag priority
 */
extern int32 rtl8306e_qos_priSrcEnable_get(uint32 port, uint32 priSrc, uint32 *pEnabled);

/* Function Name:
 *      rtl8306e_qos_ipAddrPri_set
 * Description:
 *      Set IP address priority
 * Input:
 *      priority  -  internal use 2-bit priority value (0~3)  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
extern int32 rtl8306e_qos_ipAddrPri_set(uint32 priority); 

/* Function Name:
 *      rtl8306e_qos_ipAddrPri_get
 * Description:
 *      Get IP address priority
 * Input:
 *      priority  -  internal use 2-bit priority value (0~3)  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
extern int32 rtl8306e_qos_ipAddrPri_get(uint32 *priority);

/* Function Name:
 *      rtl8306e_qos_ipAddr_set
 * Description:
 *      Set IP address
 * Input:
 *      entry        -   specify entry
         ip            -   ip address
         mask        -  ip mask
         enabled    -   enable the entry
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are two entries RTL8306_IPADD_A and RTL8306_IPADD_B
 *      for user setting ip address, if ip address of packet matches
 *      the entry, the packet will be assign the priority of ip address
 *      priority which is configured by rtl8306e_qos_ipAddrPri_set.
 */
extern int32 rtl8306e_qos_ipAddr_set(uint32 entry, uint32 ip, uint32 mask, uint32 enabled);

/* Function Name:
 *      rtl8306e_qos_ipAddr_get
 * Description:
 *      Get IP address user seting
 * Input:
 *      entry       -   specify entry
 * Output:
 *      pIp            -   ip address
         pMask        -   ip mask
         pEnabled    -  enabled or disabled the entry for IP Priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
extern int32 rtl8306e_qos_ipAddr_get(uint32 entry, uint32 *pIp, uint32 *pMask , uint32 *pEnabled); 

/* Function Name:
 *      rtl8306e_qos_schedulingPara_set
 * Description:
 *      Set qos scheduling parameter
 * Input:
 *      set          -  RTL8306_QOS_SET0 or RTL8306_QOS_SET1
 *      sch_para  -  The structure describe schedule parameter
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch supports 4 queue per egress port, scheduling algorithm could be WRR(Weight Round Robin) or 
 *      SP(Strict Priority). Only high queue Q3 and Q2 could be set as strict priority queue. There two set of 
 *      scheduling parameter in whole system(RTL8306_QOS_SET0/RTL8306_QOS_SET1), each egress port select
 *      one of them. The parameter defined by rtl8306e_qosSchPara_t, q0_wt and q1_wt should between 1~127,
 *      that means Q0 and Q1 WRR weight, q2_wt and  q3_wt could be 0~127, 0 means strict priority. q2_n64Kbps 
 *      and q3_n64Kbps means Q2/Q3 queue bandwidth control, unit is 64Kbps.
 */
extern int32 rtl8306e_qos_schedulingPara_set(uint32 set, rtl8306e_qosSchPara_t sch_para);

/* Function Name:
 *      rtl8306e_qos_schedulingPara_get
 * Description:
 *      Set qos scheduling parameter
 * Input:
 *      set           -  RTL8306_QOS_SET0 or RTL8306_QOS_SET1
 * Output:
 *      pSch_para  - the pointer of schedule parameter
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch supports 4 queue per egress port, scheduling algorithm could be WRR(Weight Round Robin) or 
 *      SP(Strict Priority). Only high queue Q3 and Q3 could be set as strict priority queue. There two set of 
 *      scheduling parameter in whole system(RTL8306_QOS_SET0/RTL8306_QOS_SET1), each egress port select
 *      one of them. The parameter defined by rtl8306e_qosSchPara_t, q0_wt and q1_wt should between 1~127,
 *      that means Q0 and Q1 WRR weight, q2_wt and  q3_wt could be 0~127, 0 means strict priority. q2_n64Kbps 
 *      and q3_n64Kbps means Q2/Q3 queue bandwidth control, unit is 64Kbps.
 */
extern int32 rtl8306e_qos_schedulingPara_get(uint32 set, rtl8306e_qosSchPara_t *pSch_para);

/* Function Name:
 *      rtl8306e_qos_portSchedulingMode_set
 * Description:
 *      Select port schedule algorithm  from two sets.
 * Input:
 *      port    -   port number (0 ~ 5)
 *      set     -   RTL8306_QOS_SET0 or RTL8306_QOS_SET1
 *      lbmsk  -   Queue mask for enable queue leaky buckt
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are two sets configuration for schedule mode including strict priority 
 *      enable/disable, queue weight and queue leaky bucket, every port could select
 *      one of them. Queue leaky bucket of each port could be enable separately, so 
 *      you can set queue mask to enable/disable them, because only queue 3 and queue 2
 *      have leaky bucket, only bit 3 and bit 2 of quemask have effect, bit 3 represents
 *      queue 3 and set 1 to enable it.
 */
extern int32 rtl8306e_qos_portSchedulingMode_set(uint32 port, uint32 set, uint32 lbmsk);

/* Function Name:
 *      rtl8306e_qos_portSchedulingMode_get
 * Description:
 *      Get which set of schedule algorithm  for the specified port 
 * Input:
 *      port    -   port number (0 ~ 5)
 *      set     -   RTL8306_QOS_SET0 or RTL8306_QOS_SET1
 *      lbmsk  -   Queue mask for enable queue leaky buckt
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are two sets configuration for schedule mode including strict priority 
 *      enable/disable, queue weight and queue leaky bucket, every port could select
 *      one of them. Queue leaky bucket of each port could be enable separately, so 
 *      you can set queue mask to enable/disable them, because only queue 3 and queue 2
 *      have leaky bucket, only bit 3 and bit 2 of quemask have effect, bit 3 represents
 *      queue 3 and set 1 to enable it.
 */
extern int32 rtl8306e_qos_portSchedulingMode_get(uint32 port, uint32 *pSet, uint32 *pLbmsk);
extern int32 rtl8306e_qos_queFlcThr_set(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32 value);
extern int32 rtl8306e_qos_queFlcThr_get(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32* pValue);
extern int32 rtl8306e_qos_portFlcThr_set(uint32 port, uint32 onthr, uint32 offthr, uint32 direction );
extern int32 rtl8306e_qos_portFlcThr_get(uint32 port, uint32 *pOnthr, uint32 *pOffthr, uint32 direction);
extern int32 rtl8306e_qos_queFlcEnable_set( uint32 port, uint32 queue, uint32 enabled);
extern int32 rtl8306e_qos_queFlcEnable_get(uint32 port, uint32 queue, uint32 *pEnabled);


/*
  *  ACL  function
  */

/* Function Name:
 *      rtl8306e_acl_entry_set
 * Description:
 *      Set Asic ACL table
 * Input:
 *      entryadd   - Acl entry address (0~15)
 *      phyport    -  Acl physical port
 *      action      -  Acl action 
 *      protocol   -  Acl protocol
 *      data        -  ether type value or TCP/UDP port
 *      priority     -  Acl priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      phyport could be 
 *            0~5:       port number, 
 *            RTL8306_ACL_INVALID: invalid entry,
 *            RTL8306_ACL_ANYPORT: any port.
 *      Acl action could be
 *          RTL8306_ACT_DROP,
 *          RTL8306_ACT_PERMIT, 
 *          RTL8306_ACT_TRAPCPU, 
 *          RTL8306_ACT_MIRROR
 *      Acl protocol could be
 *          RTL8306_ACL_ETHER(ether type), 
 *          RTL8306_ACL_TCP(TCP), 
 *          RTL8306_ACL_UDP(UDP),
 *          RTL8306_ACL_TCPUDP(TCP or UDP)
 *
 *      Acl priority:
 *          RTL8306_PRIO0~RTL8306_PRIO3
 *       
 */
extern int32 rtl8306e_acl_entry_set(uint32 entryadd, uint32 phyport, uint32 action, uint32 protocol, uint32 data, uint32 priority) ;

/* Function Name:
 *      rtl8306e_acl_entry_get
 * Description:
 *      Get Asic ACL entry
 * Input:
 *      entryadd   - Acl entry address (0~15)
 * Output:
 *      pPhyport    -  Acl physical port 
 *      pAction      -  Acl action 
 *      pProtocol   -  Acl protocol
 *      pData        -  ether type value or TCP/UDP port
 *      pPriority     -  Acl priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      phyport could be 
 *            0~5:       port number, 
 *            RTL8306_ACL_INVALID: invalid entry,
 *            RTL8306_ACL_ANYPORT: any port.
 *      Acl action could be
 *          RTL8306_ACT_DROP,
 *          RTL8306_ACT_PERMIT, 
 *          RTL8306_ACT_TRAPCPU, 
 *          RTL8306_ACT_MIRROR
 *      Acl protocol could be
 *          RTL8306_ACL_ETHER(ether type), 
 *          RTL8306_ACL_TCP(TCP), 
 *          RTL8306_ACL_UDP(UDP),
 *          RTL8306_ACL_TCPUDP(TCP or UDP)
 *
 *      Acl priority:
 *          RTL8306_PRIO0~RTL8306_PRIO3
 *       
 */
extern int32 rtl8306e_acl_entry_get(uint32 entryadd, uint32 *pPhyport, uint32 *pAction, uint32 *pProtocol, uint32  *pData, uint32 *pPriority);

/*
  *  Mib  function
  */

/* Function Name:
 *      rtl8306e_mib_get
 * Description:
 *      enable/disable Qos priority source for  ingress port
 * Input:
 *      port         -  port number (0 ~ 5)
 *      counter    -  Specify counter type  
 * Output:
 *      pEnabled -  the pointer of priority source status  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are five MIB counter for each port, they are:
 *      RTL8306_MIB_CNT1 - TX count
 *      RTL8306_MIB_CNT2 - RX count
 *      RTL8306_MIB_CNT3 - RX Drop Count<nl>
 *      RTL8306_MIB_CNT4 - RX CRC error Count
 *      RTL8306_MIB_CNT5 - RX Fragment Count<nl>
 */
extern int32 rtl8306e_mib_get(uint32 port, uint32 counter, uint32 *pValue); 

/* Function Name:
 *      rtl8306e_mibUnit_set
 * Description:
 *      Set RX/Tx Mib counting unit: byte or packet
 * Input:
 *      port         -  port number (0 ~ 5)
 *      counter    -  Specify counter type  
 *      uint         -  Specify counting unit
 * Output:
 *      none  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.There are five MIB counter for each port, they are:
 *      RTL8306_MIB_CNT1 - TX count
 *      RTL8306_MIB_CNT2 - RX count
 *      RTL8306_MIB_CNT3 - RX Drop Count<nl>
 *      RTL8306_MIB_CNT4 - RX CRC error Count
 *      RTL8306_MIB_CNT5 - RX Fragment Count<nl>
 *      2.Only RTL8306_MIB_CNT1 and RTL8306_MIB_CNT2 could set counting unit  
 *      RTL8306_MIB_PKT or RTL8306_MIB_BYTE, default is RTL8306_MIB_PKT.
 *      the other counters' counting uint is RTL8306_MIB_PKT
 */
extern int32 rtl8306e_mibUnit_set(uint32 port, uint32 counter, uint32 unit);

/* Function Name:
 *      rtl8306e_mibUnit_get
 * Description:
 *      Get Rx/Tx Mib counting unit
 * Input:
 *      port         -  port number (0 ~ 5)
 *      counter    -  Specify counter type  
 * Output:
 *      pUnit         -  the pointer of counting unit
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.There are five MIB counter for each port, they are:
 *      RTL8306_MIB_CNT1 - TX count
 *      RTL8306_MIB_CNT2 - RX count
 *      RTL8306_MIB_CNT3 - RX Drop Count<nl>
 *      RTL8306_MIB_CNT4 - RX CRC error Count
 *      RTL8306_MIB_CNT5 - RX Fragment Count<nl>
 *      2.Only RTL8306_MIB_CNT1 and RTL8306_MIB_CNT2 could set counting unit  
 *      RTL8306_MIB_PKT or RTL8306_MIB_BYTE, default is RTL8306_MIB_PKT.
 *      the other counters' counting uint is RTL8306_MIB_PKT
 */
extern int32 rtl8306e_mibUnit_get(uint32 port, uint32 counter, uint32 *pUnit);

/* Function Name:
 *      rtl8306e_mib_reset
 * Description:
 *      reset MIB counter
 * Input:
 *      port         -  port number (0 ~ 5)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
extern int32 rtl8306e_mib_reset(uint32 port);

/*
  *  mirror  function
  */

/* Function Name:
 *      rtl8306e_mirror_portBased_set
 * Description:
 *      Set asic Mirror port
 * Input:
 *      mirport         -  Specify mirror port 
 *      rxmbr           -  Specify Rx mirror port mask
 *      txmbr           -  Specify Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      mirport could be 0 ~ 5, represent physical port number, 
 *      7 means that no port has mirror ability. rxmbr and txmbr
 *      is 6 bit value, each bit corresponds one port. ingress packet
 *      of port in rxmbr will be mirrored to mirport, egress packet 
 *      of port in txmbr will be mirrored to mirport.
 */
extern int32 rtl8306e_mirror_portBased_set(uint32 mirport, uint32 rxmbr, uint32 txmbr) ;

/* Function Name:
 *      rtl8306e_mirror_portBased_get
 * Description:
 *      Get asic Mirror port
 * Input:
 *      none 
 * Output:
 *      pMirport     -  the pointer of mirror port
 *      pRxmbr       -  the pointer of  Rx mirror port mask
 *      pTxmbr       -  the pointer of Tx mirror port mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      mirport could be 0 ~ 5, represent physical port number, 
 *      7 means that no port has mirror ability. rxmbr and txmbr
 *      is 6 bit value, each bit corresponds one port. ingress packet
 *      of port in rxmbr will be mirrored to mirport, egress packet 
 *      of port in txmbr will be mirrored to mirport.
 */

extern int32 rtl8306e_mirror_portBased_get(uint32 *pMirport, uint32 *pRxmbr, uint32* pTxmbr);

/*
  *  L2 table  function
  */

/* Function Name:
 *      rtl8306e_l2_MacToIdx_get
 * Description:
 *      get L2 table hash value from mac address
 * Input:
 *      macAddr        -  mac address
 * Output:
 *      pIndex           -  mac address table index   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      when a mac address is learned into mac address table, 
 *      9 bit index value is got from the mac address by hashing 
 *      algorithm, each index corresponds to 4 entry, it means
 *      the table could save 4 mac addresses at the same time
 *      whose index value is equal, so switch mac address table 
 *      has 2048 entry. the API could get hash index from 
 *      a specified mac address.
 */
extern int32 rtl8306e_l2_MacToIdx_get(uint8 *macAddr, uint32* pIndex);

/* Function Name:
 *      rtl8306e_l2_unicastEntry_set
 * Description:
 *      write an unicast mac address into L2 table
 * Input:
 *      macAddress        -  Specify the unicast Mac address(6 bytes) to be written into LUT
 *      entry                 -  Specify the 4-way entry to be written (0~3)
 *      age                   -  Specify age time
 *      isStatic              -  TRUE(static entry), FALSE(dynamic entry)
 *      isAuth                -  Whether the mac address is authorized by IEEE 802.1x
 *      port                  -   Specify the port number to be forwarded to  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8306_LUT_AGEOUT, 
 *          RTL8306_LUT_AGE100(100s)
 *          RTL8306_LUT_AGE200(200s)
 *          RTL8306_LUT_AGE300(300s)
 */ 
extern int32 rtl8306e_l2_unicastEntry_set(uint8 *macAddress, uint32 entry, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port); 

/* Function Name:
 *      rtl8306e_l2_unicastEntry_get
 * Description:
 *      read an unicast mac address into L2 table
 * Input:
 *      entry               -  Specify the entry address to be read (0 ~ 2047), not four-way entry
 * Output:
 *      macAddress      -  the mac to be saved in the entry  
 *      pAge               -  the pointer of the age time
 *      pIsStatic         -   the pointer of static or dynamic entry
 *      pIsAuth           -   the pointer of IEEE 802.1x authorized status
 *      pPort              -   the pointer of the port the mac belongs to   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8306_LUT_AGEOUT
 *          RTL8306_LUT_AGE100(100s)
 *          RTL8306_LUT_AGE200(200s)
 *          RTL8306_LUT_AGE300(300s)
 */ 
extern int32 rtl8306e_l2_unicastEntry_get(uint8 *macAddress, uint32 entryAddr, uint32 *pAge, uint32 *pIsStatic, uint32 *pIsAuth, uint32 *pPort) ;

/* Function Name:
 *      rtl8306e_l2_multicastEntry_set
 * Description:
 *      write an multicast mac address into L2 table
 * Input:
 *      macAddress      -  the mac to be saved in the entry  
 *      entry               -  Specify the 4-way entry to be written (0~3)
 *      isAuth              -  IEEE 802.1x authorized status
 *      portMask          -  switch received thepacket with the specified macAddress, 
 *                                and forward it to the member port of portMask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      get 9-bit as index value of lookup table by hashing the mac address , for each index value, 
 *      there are 4-entry to save hash collision mac address, thus there are 2048 entries all together.
 *      the entry could save both unicast multicast mac address.  multicast entry has no age time and
 *      static bit, which has been set by software, unicast entry could be both auto learned and set by 
 *      software. you had better set isAuth TRUE for general application because  IEEE 802.1x is usually
 *      for unicast packet.portMask is 6-bit value, each bit represents one port, bit 0 corresponds to port 0
 *      and bit 5 corresponds port 5.
 */ 

extern int32 rtl8306e_l2_multicastEntry_set(uint8 *macAddress, uint32 entry, uint32 isAuth, uint32 portMask);

/* Function Name:
 *      rtl8306_getAsicLUTMulticastEntry
 * Description:
 *      Get LUT multicast entry
 * Input:
 *      entryAddr         -  Specify the LUT entry address(0~2047)

 * Output:
 *      macAddress      -  The read out multicast Mac address  
 *      pIsAuth            -  the pointer of IEEE 802.1x authorized status
 *      portMask          -  port mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Get multicast entry
 */ 
extern int32 rtl8306e_l2_multicastEntry_get(uint8 *macAddress, uint32 entryAddr, uint32 *pIsAuth, uint32 *pPortMask);

/* Function Name:
 *      rtl8306e_l2_unicastMac_add
 * Description:
 *     Add an unicast mac address, software will detect empty entry
 * Input:
 *      macAddress      -  the mac to be saved in the entry  
 *      age                 -  Specify age time
 *      isStatic            -  TRUE(static entry), FALSE(dynamic entry)
 *      isAuth              -  IEEE 802.1x authorized status
 *      port                 -  the port which the mac address belongs to  
 * Output:
 *      pEntryaddr        -   the entry address (0 ~2047) which the unicast mac address is written into
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :RTL8306_LUT_AGEOUT, RTL8306_LUT_AGE100(100s), 
 *      RTL8306_LUT_AGE200(200s), RTL8306_LUT_AGE300(300s)
 *      The lut has a 4-way entry of an index. If the macAddress has existed in the lut, it will update the entry,
 *      otherwise the function will find an empty entry to put it.
 *      When the index is full, it will find a dynamic & unauth unicast macAddress entry to replace with it. 
 *      If the mac address has been written into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *      entry address of the Mac address stored.
 *      If all the four entries can not be replaced, it will return a  RTL8306_LUT_FULL error, you can delete one of them 
 *      and rewrite the unicast address.  
 */ 
extern int32 rtl8306e_l2_unicastMac_add(uint8 *macAddress, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port, uint32 *pEntryaddr);

/* Function Name:
 *      rtl8306e_l2_multicastMac_add
 * Description:
 *     Add an multicast mac address, software will detect empty entry
 * Input:
 *      macAddress      -  the mac to be saved in the entry  
 *      isAuth              -  IEEE 802.1x authorized status
 *      portMask          -  switch received thepacket with the specified macAddress, 
 *                                and forward it to the member port of portMask
 * Output:
 *      pEntryaddr        -   the entry address (0 ~2047) which the multicast mac address is written into
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      add an multicast entry, it need not specify the 4-way like function rtl8306e_l2_multicastEntry_set,
 *      if the mac address has written into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *      entry address of the Mac address stored, if  4-way entries are all written by cpu, this mac address could 
 *      not written into LUT and the function return value is  RTL8306_LUT_FULL, but if the Mac address has exist, 
 *      the port mask will be updated. When function return value is RTL8306_LUT_FULL, you can delete one of them 
 *      and rewrite the multicast address. 
 */ 
extern int32 rtl8306e_l2_multicastMac_add(uint8 *macAddr,uint32 isAuth, uint32 portMask, uint32 *pEntryaddr);

/* Function Name:
 *      rtl8306e_l2_mac_get
 * Description:
 *      Get an mac address information
 * Input:
 *      macAddress         -   the mac to be find in LUT  
 * Output:
 *      pIsStatic             -   the pointer of static or dynamic entry, for unicast mac address
 *      pIsAuth               -   the pointer of IEEE 802.1x authorized status
 *      pPortInfo             -   for unicast mac, it is the pointer of the port the mac belongs to;
 *                                    for multicast mac, it is the pointer of portmask the mac forwarded to;
 *      pEntryaddr           -   the entry address (0 ~2047) which the mac address is written into
 * Return:
 *      SUCCESS
 *      FAILED
 *      RTL8306_LUT_NOTEXIST
 * Note:
 *      use this API to get a mac address information in LUT.
 */ 
extern int32 rtl8306e_l2_mac_get(uint8 *macAddr, uint32 *pIsStatic, uint32 *pIsAuth, uint32 *pPortInfo, uint32 *pEntryaddr); 

/* Function Name:
 *      rtl8306e_l2_mac_del
 * Description:
 *     Delete the specified Mac address, could be both unicast and multicast 
 * Input:
 *      macAddress      -  the Mac address(unicast or multicast) to be delete  
 *                                and forward it to the member port of portMask
 * Output:
 *      pEntryaddr        -  entry address from which the Mac address is deleted
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function to delete a Mac address, it does not require to specify the 
 *      entry address, if the Mac has existed in the LUT, it will be deleted and function
 *      return value is SUCCESS, *pEntryaddr is recorded the entry address of the Mac 
 *      address stored. if the Mac does not existed in the LUT, function return value is
 *      RTL8306_LUT_NOTEXIST, and *pEntryaddr equals 10000.
 */ 

extern int32 rtl8306e_l2_mac_del(uint8 *macAddr, uint32 *pEntryaddr);
extern int32 rtl8306e_l2_portMacLimit_set(uint32 port, uint32 enabled, uint32 macCnt) ;
extern int32 rtl8306e_l2_portMacLimit_get(uint32 port, uint32 *pEnabled, uint32 *pMacCnt);
extern int32 rtl8306e_l2_systemMacLimit_set(uint32 enabled, uint32 macCnt, uint32 mergMask);
extern int32 rtl8306e_l2_systemMacLimit_get(uint32 *pEnabled, uint32 *pMacCnt, uint32 *pMergMask); 
extern int32 rtl8306e_l2_macLimitAction_set(uint32 action); 
extern int32 rtl8306e_l2_macLimitAction_get(uint32 *pAction);

#ifdef RTL8306_LUT_CACHE
/* Function Name:
 *      rtl8306e_l2_unicastEntry_get
 * Description:
 *      Get LUT unicast entry from software
 * Input:
 *      entryAddr         -  Specify the entry address to be read (0 ~ 2047), not four-way entry
 * Output:
 *      macAddress      -  the mac to be saved in the entry  
 *      pAge               -  the pointer of the age time
 *      pIsStatic         -   the pointer of static or dynamic entry
 *      pIsAuth           -   the pointer of IEEE 802.1x authorized status
 *      pPort              -   the pointer of the port the mac belongs to   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8306_LUT_AGEOUT
 *          RTL8306_LUT_AGE100(100s)
 *          RTL8306_LUT_AGE200(200s)
 *          RTL8306_LUT_AGE300(300s)
 */ 
extern int32 rtl8306e_fastGetAsicLUTUnicastEntry(uint8 *macAddress, uint32 entryAddr, uint32 *age, uint32 *isStatic, uint32 *isAuth, uint32 *port);

/* Function Name:
 *      rtl8306e_fastGetAsicLUTMulticastEntry
 * Description:
 *      Get LUT multicast entry from software
 * Input:
 *      entryAddr         -  Specify the entry address to be read (0 ~ 2047), not four-way entry
 * Output:
 *      macAddress      -  the mac to be saved in the entry  
 *      pIsAuth           -   the pointer of IEEE 802.1x authorized status
 *      pPort              -   the pointer of the port the mac belongs to   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      portMask is 6-bit value, each bit represents one port, bit 0 corresponds to port 0 
 *      and bit 5 corresponds port 5.    
 */ 
extern int32 rtl8306e_fastGetAsicLUTMulticastEntry(uint8 *macAddress, uint32 entryAddr, uint32 *isAuth, uint32 *portMask);

#endif
/*
  *  Spanning Tree  function
  */

/* Function Name:
 *      rtl8306e_stp_set
 * Description:
 *      Set IEEE 802.1d port state
 * Input:
 *      port   -  Specify port number (0 ~ 5)
 *      state -   Specify port state
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 port state:
 *         RTL8306_SPAN_DISABLE   - Disable state
 *         RTL8306_SPAN_BLOCK      - Blocking state
 *         RTL8306_SPAN_LEARN      - Learning state
 *         RTL8306_SPAN_FORWARD - Forwarding state
 */ 
extern int32 rtl8306e_stp_set(uint32 port, uint32 state) ;

/* Function Name:
 *      rtl8306e_stp_get
 * Description:
 *      Get IEEE 802.1d port state
 * Input:
 *      port    -  Specify port number (0 ~ 5)
 * Output:
 *      pState -  get port state
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 port state:
 *         RTL8306_SPAN_DISABLE   - Disable state
 *         RTL8306_SPAN_BLOCK      - Blocking state
 *         RTL8306_SPAN_LEARN      - Learning state
 *         RTL8306_SPAN_FORWARD - Forwarding state
 */ 
extern int32 rtl8306e_stp_get(uint32 port, uint32 *pState) ;

/*
  *  802.1x  function
  */

/* Function Name:
 *      rtl8306e_dot1x_portBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 5)
 *      enabled    -   enable port-based access control
 *      isAuth      -   Authorized or unauthorized state 
 *      direction   -    set IEEE802.1x port-based control direction
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are two IEEE802.1x port state:
 *         RTL8306_PORT_AUTH      - authorized
 *         RTL8306_PORT_UNAUTH  - unauthorized
 *
 *     There are also two 802.1x port-based control direction:
 *         RTL8306_PORT_BOTHDIR - if port-base access control is enabled, 
 *                                              forbid forwarding this port's traffic to unauthorized port
 *         RTL8306_PORT_INDIR     - if port-base access control is enabled, permit forwarding this
 *                                              port's traffic to unauthorized port
 */ 
extern int32 rtl8306e_dot1x_portBased_set(uint32 port, uint32 enabled, uint32 isAuth, uint32 direction);

/* Function Name:
 *      rtl8306e_dot1x_portBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 5)
 * Output:
 *      pEnabled    - the pointer of port-based access control status
 *      pIsAuth      - the pointer of authorized or unauthorized state 
 *      pDirection   - the pointer of IEEE802.1x port-based control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are two IEEE802.1x port state:
 *         RTL8306_PORT_AUTH      - authorized
 *         RTL8306_PORT_UNAUTH  - unauthorized
 *
 *     There are also two 802.1x port-based control direction:
 *         RTL8306_PORT_BOTHDIR - if port-base access control is enabled, 
 *                                              forbid forwarding this port's traffic to unauthorized port
 *         RTL8306_PORT_INDIR     - if port-base access control is enabled, permit forwarding this
 *                                              port's traffic to unauthorized port
 */ 
extern int32 rtl8306e_dot1x_portBased_get(uint32 port, uint32 *pEnabled, uint32 *pIsAuth, uint32 *pDirection) ;

/* Function Name:
 *      rtl8306e_dot1x_macBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 5)
 * Output:
 *      enabled    - Enable the port Mac-based access control ability
 *      direction   -  IEEE802.1x mac-based access control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      the mac address authentication status is saved in L2 table entry,
 *      it should be set by software.
 *      there are also two mac-based control directions which are not per 
 *      port but global configurtion:
 *      RTL8306_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
 *      unauthorized DA will be dropped.
 *      RTL8306_MAC_INDIR   - if Mac-based access control is enabled, packet with 
 *      unauthorized DA will pass mac-based access control igress rule.
 */ 

extern int32 rtl8306e_dot1x_macBased_set(uint32 port, uint32 enabled, uint32 direction);

/* Function Name:
 *      rtl8306e_dot1x_macBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 5)
 * Output:
 *      enabled    - Enable the port Mac-based access control ability
 *      direction   -  IEEE802.1x mac-based access control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      the mac address authentication status is saved in L2 table entry,
 *      it should be set by software.
 *      there are also two mac-based control directions which are not per 
 *      port but global configurtion:
 *      RTL8306_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
 *      unauthorized DA will be dropped.
 *      RTL8306_MAC_INDIR   - if Mac-based access control is enabled, packet with 
 *      unauthorized DA will pass mac-based access control igress rule.
 */ 
extern int32 rtl8306e_dot1x_macBased_get(uint32 port, uint32 *pEnabled, uint32 *pDirection); 

/* Function Name:
 *      rtl8306e_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type         -  Specify IGMP/MLD or PPPOE
 *      action       -  Action could be normal forward or trap
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     type could be:
 *          RTL8306_IGMP   - igmp packet without pppoe header 
 *          RTL8306_MLD    - mld packet  without pppoe header
 *          RTL8306_PPPOE - pppoe packet if enable trap RTL8306_IGMP 
 *                                   or RTL8306_MLD whether pppoe packet 
 *                                   should be trapped. In some application,
 *                                   igmp and mld message is encapsulated in pppoed
 *                                   packet.
 *         
 *      action could be:
 *          RTL8306_ACT_PERMIT    - normal forward
 *          RTL8306_ACT_TRAPCPU  - trap to cpu
 */ 
extern int32 rtl8306e_trap_igmpCtrlPktAction_set(uint32 type, uint32 action);

/* Function Name:
 *      rtl8306e_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap setting
 * Input:
 *      type         -  Specify IGMP/MLD or PPPOE
 * Output:
 *      pAction     -  the pointer of action could be normal forward or trap
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     type could be:
 *          RTL8306_IGMP   - igmp packet without pppoe header 
 *          RTL8306_MLD    - mld packet  without pppoe header
 *          RTL8306_PPPOE - pppoe packet if enable trap RTL8306_IGMP 
 *                                   or RTL8306_MLD whether pppoe packet 
 *                                   should be trapped. In some application,
 *                                   igmp and mld message is encapsulated in pppoed
 *                                   packet.
 *      action could be:
 *          RTL8306_ACT_PERMIT    - normal forward
 *          RTL8306_ACT_TRAPCPU  - trap to cpu
 */ 
extern int32 rtl8306e_trap_igmpCtrlPktAction_get(uint32 type, uint32 *pAction);

/* Function Name:
 *      rtl8306e_trap_unknownIPMcastPktAction_set
 * Description:
 *      Set unknown ip multicast drop or normal forward
 * Input:
 *      type         -  Specify ipv4 or ipv6 unkown multicast
 *      action       -  drop or normal forward
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_IPV4_MULTICAST - ipv4 unknown multicast
 *          RTL8306_IPV6_MULTICAST - ipv6 unknown multicast
 *      action could be:
 *          RTL8306_ACT_DROP      - trap to cpu 
 *          RTL8306_ACT_PERMIT   - normal forward
 */  
extern int32 rtl8306e_trap_unknownIPMcastPktAction_set(uint32 type, uint32 action);

/* Function Name:
 *      rtl8306e_trap_unknownIPMcastPktAction_get
 * Description:
 *      Get unknown ip multicast drop or normal forward
 * Input:
 *      type         -  Specify ipv4 or ipv6 unkown multicast
 * Output:
 *      pAction     -  the pointer of drop or normal forward
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_IPV4_MULTICAST - ipv4 unknown multicast
 *          RTL8306_IPV6_MULTICAST - ipv6 unknown multicast
 *      action could be:
 *          RTL8306_ACT_DROP      - trap to cpu 
 *          RTL8306_ACT_PERMIT   - normal forward
 */ 
extern int32 rtl8306e_trap_unknownIPMcastPktAction_get(uint32 type, uint32 *pAction);

/* Function Name:
 *      rtl8306e_trap_abnormalPktAction_set
 * Description:
 *      set abnormal packet action 
 * Input:
 *      type         -  abnormal packet type
 *      action       -  drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_UNMATCHVID   - vlan-tagged packet, vid dismatch vlan table 
 *          RTL8306_DOT1XUNAUTH - 802.1x authentication fail packet
 *      action could be:
 *          RTL8306_ACT_DROP       - drop 
 *          RTL8306_ACT_TRAPCPU  - trap to cpu
 */ 
extern int32 rtl8306e_trap_abnormalPktAction_set(uint32 type,  uint32 action);

/* Function Name:
 *      rtl8306e_trap_abnormalPktAction_get
 * Description:
 *      get abnormal packet action 
 * Input:
 *      type         -  abnormal packet type
 * Output:
 *      pAction     -  the pointer of action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8306_UNMATCHVID   - vlan-tagged packet, vid dismatch vlan table 
 *          RTL8306_DOT1XUNAUTH - 802.1x authentication fail packet
 *      action could be:
 *          RTL8306_ACT_DROP       - drop 
 *          RTL8306_ACT_TRAPCPU  - trap to cpu
 */ 

extern int32 rtl8306e_trap_abnormalPktAction_get(uint32 type,  uint32 *pAction);

/* Function Name:
 *      rtl8306e_trap_rmaAction_set
 * Description:
 *      Set reserved multicast Mac address forwarding behavior
 * Input:
 *      type         -  reserved Mac address type
 *      action       -  forwarding behavior for the specified mac address
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are eight types  reserved addresses which user can set asic to determine 
 *       how to forwarding them:
 *            RTL8306_RESADDRXX   - reserved address 01-80-c2-00-00-xx 
 *                                              (exclude 00, 01, 02, 03, 10, 20, 21)
 *            RTL8306_RESADDR21   -  reserved address 01-80-c2-00-00-21(GVRP address)
 *            RTL8306_RESADDR20   -  reserved address 01-80-c2-00-00-20(GMRP Address)
 *            RTL8306_RESADDR10   -  reserved address 01-80-c2-00-00-10(All LANs Bridge Management Group Address)
 *            RTL8306_RESADDR03   -  reserved address 01-80-c2-00-00-03(IEEE Std 802.1X PAE address)
 *            RTL8306_RESADDR02   -  reserved address 01-80-c2-00-00-02(IEEE Std 802.3ad Slow_Protocols-Multicast address)
 *            RTL8306_RESADDR00   -  reserved address 01-80-c2-00-00-00(Bridge Group Address)
 *            RTL8306_RESADDR01   -  reserved address 01-80-c2-00-00-01(Pause frame)
 *       Actions are :
 *            TL8306_ACT_DROP      - Drop the packet
 *            TL8306_ACT_TRAPCPU - Trap the packet to cpu
 *            RTL8306_ACT_FLOOD   - Flood the packet
 */ 
extern int32 rtl8306e_trap_rmaAction_set(uint32 type, uint32 action);

/* Function Name:
 *      rtl8306e_trap_rmaAction_get
 * Description:
 *      Get reserved multicast Mac address forwarding behavior
 * Input:
 *      type         -  reserved Mac address type
 * Output:
 *      pAction     -  the pointer of action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are eight types  reserved addresses which user can set asic to determine 
 *       how to forwarding them:
 *            RTL8306_RESADDRXX   - reserved address 01-80-c2-00-00-xx 
 *                                              (exclude 00, 01, 02, 03, 10, 20, 21)
 *            RTL8306_RESADDR21   -  reserved address 01-80-c2-00-00-21(GVRP address)
 *            RTL8306_RESADDR20   -  reserved address 01-80-c2-00-00-20(GMRP Address)
 *            RTL8306_RESADDR10   -  reserved address 01-80-c2-00-00-10(All LANs Bridge Management Group Address)
 *            RTL8306_RESADDR03   -  reserved address 01-80-c2-00-00-03(IEEE Std 802.1X PAE address)
 *            RTL8306_RESADDR02   -  reserved address 01-80-c2-00-00-02(IEEE Std 802.3ad Slow_Protocols-Multicast address)
 *            RTL8306_RESADDR00   -  reserved address 01-80-c2-00-00-00(Bridge Group Address)
 *            RTL8306_RESADDR01   -  reserved address 01-80-c2-00-00-01(Pause frame)
 *       Actions are :
 *            TL8306_ACT_DROP      - Drop the packet
 *            TL8306_ACT_TRAPCPU - Trap the packet to cpu
 *            RTL8306_ACT_FLOOD   - Flood the packet
 */ 
extern int32 rtl8306e_trap_rmaAction_get(uint32 type, uint32 *pAction);


/* Function Name:
 *      rtl8306e_int_control_set
 * Description:
 *      Set asic interrupt 
 * Input:
 *      enInt        -  Enable interrupt cpu
 *      intmask     -  interrupt event  mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      enInt is global setting and  intmask  has 12 bits totally, each bit 
 *      represents one interrupt event, 
 *      - bit0 ~bit4 represent port 0 ~ port 4 link change, 
 *      - bit 5 represents port 4 MAC link change
 *      - bit 6 represents port 5 link change, 
 *      - bit 7 represents storm filter interrupt,
 *      - bit 8 represents loop event 
 *      - bit 9 represents wake up frame interrupt
 *      - bit 10 represents unmatched SA interrupt
 *      - bit 11 represents Tx meter interrupt
 *      write 1 to the bit to enable the interrupt and 0 will disable the interrupt. 
 *
 */ 
extern int32 rtl8306e_int_control_set(uint32 enInt, uint32 intmask);

/* Function Name:
 *      rtl8306e_int_control_get
 * Description:
 *      Get Asic interrupt
 * Input:
 *      none 
 * Output:
 *      pEnInt       -  the pointer of  interrupt global enable bit
 *      pIntmask    -  the pointer of interrupt event  mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      enInt is global setting and  intmask  has 12 bits totally, each bit 
 *      represents one interrupt event, 
 *      - bit0 ~bit4 represent port 0 ~ port 4 link change, 
 *      - bit 5 represents port 4 MAC link change
 *      - bit 6 represents port 5 link change, 
 *      - bit 7 represents storm filter interrupt,
 *      - bit 8 represents loop event 
 *      - bit 9 represents wake up frame interrupt
 *      - bit 10 represents unmatched SA interrupt
 *      - bit 11 represents Tx meter interrupt
 *      write 1 to the bit to enable the interrupt and 0 will disable the interrupt. 
 *
 */ 
extern int32 rtl8306e_int_control_get(uint32 *pEnInt, uint32 *pIntmask);



/* Function Name:
 *      rtl8306e_int_control_get
 * Description:
 *      Get Asic interrupt
 * Input:
 *      none 
 * Output:
 *      pEnInt       -  the pointer of  interrupt global enable bit
 *      pIntmask    -  the pointer of interrupt event  mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      enInt is global setting and  intmask  has 12 bits totally, each bit 
 *      represents one interrupt event, 
 *      - bit0 ~bit4 represent port 0 ~ port 4 link change, 
 *      - bit 5 represents port 4 MAC link change
 *      - bit 6 represents port 5 link change, 
 *      - bit 7 represents storm filter interrupt,
 *      - bit 8 represents loop event 
 *      - bit 9 represents wake up frame interrupt
 *      - bit 10 represents unmatched SA interrupt
 *      - bit 11 represents Tx meter interrupt
 *      write 1 to the bit to enable the interrupt and 0 will disable the interrupt. 
 *
 */ 
extern int32 rtl8306e_int_status_get(uint32 *pStatusMask);


/* Function Name:
 *      rtl8306e_storm_filterEnable_set
 * Description:
 *      Enable Asic storm filter 
 * Input:
 *      type      -  specify storm filter type
 *      enabled  -  TRUE or FALSE
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 3 kinds of storm filter:
 *      (1) RTL8306_BROADCASTPKT - storm filter for broadcast packet
 *      (2) RTL8306_MULTICASTPKT - storm filter for multicast packet
 *      (3) RTL8306_UDAPKT           - storm filter for unknown DA packet
 */ 
extern int32 rtl8306e_storm_filterEnable_set(uint32 type, uint32 enabled);


/* Function Name:
 *      rtl8306e_storm_filterEnable_get
 * Description:
 *      Get Asic storm filter enabled or disabled 
 * Input:
 *      type        -  specify storm filter type
 * Output:
 *      pEnabled  -  the pointer of enabled or disabled
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 3 kinds of storm filter:
 *      (1) RTL8306_BROADCASTPKT - storm filter for broadcast packet
 *      (2) RTL8306_MULTICASTPKT - storm filter for multicast packet
 *      (3) RTL8306_UDAPKT           - storm filter for unknown DA packet
 */ 

extern int32 rtl8306e_storm_filterEnable_get(uint32 type, uint32 *pEnabled);


/* Function Name:
 *      rtl8306e_storm_filter_set
 * Description:
 *      Set storm filter parameter
 * Input:
 *      trigNum        -  set packet threshold which trigger storm filter
 *      filTime         -   set time window 
 *      enStmInt     -   enable storm filter to interrupt cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      there are 6 value for trigNum:
 *      - RTL8306_STM_FILNUM64, 64 pkts
 *      - RTL8306_STM_FILNUM32, 32 pkts           
 *      - RTL8306_STM_FILNUM16, 16 pkts           
 *      - RTL8306_STM_FILNUM8,   8 pkts              
 *      - RTL8306_STM_FILNUM128, 128 pkts          
 *      - RTL8306_STM_FILNUM256, 256 pkts     
 *      there are 4 value for filTime:
 *      - RTL8306_STM_FIL800MS,  800ms 
 *      - RTL8306_STM_FIL400MS,  400ms 
 *      - RTL8306_STM_FIL200MS,  200ms
 *      - RTL8306_STM_FIL100MS,  100ms
 */ 

extern int32 rtl8306e_storm_filter_set(uint32 trigNum, uint32 filTime, uint32 enStmInt);

/* Function Name:
 *      rtl8306e_storm_filter_get
 * Description:
 *      Get storm filter parameter
 * Input:
 *      none 
 * Output:
 *      pTrigNum        -  the pointer of packet threshold which trigger storm filter
 *      pFilTime          -  the pointer of filter time window 
 *      pEnStmInt       -  the pointer of enable storm filter to interrupt cpu
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      there are 6 value for trigNum:
 *      - RTL8306_STM_FILNUM64, 64 pkts
 *      - RTL8306_STM_FILNUM32, 32 pkts           
 *      - RTL8306_STM_FILNUM16, 16 pkts           
 *      - RTL8306_STM_FILNUM8,   8 pkts              
 *      - RTL8306_STM_FILNUM128, 128 pkts          
 *      - RTL8306_STM_FILNUM256, 256 pkts     
 *      there are 4 value for filTime:
 *      - RTL8306_STM_FIL800MS,  800ms 
 *      - RTL8306_STM_FIL400MS,  400ms 
 *      - RTL8306_STM_FIL200MS,  200ms
 *      - RTL8306_STM_FIL100MS,  100ms
 */ 

extern int32 rtl8306e_storm_filter_get(uint32 *pTrigNum, uint32 *pFilTime, uint32 *pEnStmInt);
int32 rtl8306e_qos_portFlcMode_set(uint32 port, uint32 set);
int32 rtl8306_getAsicPhyReg(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue) ;
int32 rtl8306_getAsicVlanEnable(uint32 *enabled);

#endif


