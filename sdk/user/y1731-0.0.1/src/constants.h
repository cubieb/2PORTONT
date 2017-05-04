/**
 * Copyright (c) 2010-2012 Helsinki Institute for Information Technology
 * and University of Helsinki.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file
 *
 * @author Samu Varjonen
 */ 

#ifndef OAM_CONSTANTS_H
#define OAM_CONSTANTS_H

#include <sys/time.h>
#include <netinet/in.h>

#include "pdu.h"

/**
 * Loopback address to be used with mepd and oamstatd */
#define OAM_LOOPBACK "127.0.0.1" 

/** 
 * The default usec for ccm in fault management 
 */
#define OAM_CCM_DEFAULT_USEC OAM_USEC_IN_S * 10   

/**
 * Select ticks at this pace.
 *
 * @see Ticket #74 
 */
#define OAM_SELECT_TIMEOUT_TICKER_USECS  1000 //3340

#define OAM_SELECT_TIMEOUT 1 /**< What timeout is used with select */

#define OAM_ETH_HEADER_LENGTH 14 /**< Ethernet header lenght */
#define OAM_ETH_MAC_LENGTH 6 /**< Ethernet header MAC length */

#define OAM_ETH_P_CFM 0x8902 /**< Y.1731 / 802.1ag Ethernet type */

#define OAM_MAX_MEG_ID_ICC  14 /**< Maximum length of the MEG ID+terminator */ 
#define OAM_MEG_ID_RESERVED  1 /**< Reserved field must be set to 01 */
#define OAM_MEG_ID_FORMAT   32 /**< Format type for MEG ID */
#define OAM_MEG_ID_LENGTH   13 /**< Length for MEG ID in CCM */

#define OAM_MAX_IPV4_LENGTH 17 /**< longest IPv4 255.255.255.255 + delimiter */

#define OAM_MEPD_LOCAL_PORT 1500 /**< Local port to bind */

#define OAM_NANOSEC_IN_S    1000000000 /**< nanoseconds in s */

#define OAM_USEC_IN_S       1000000  /**< usec's in s */

#define OAM_USEC_IN_MS      1000  /**< usec's in ms */

#define OAM_USEC_IN_MIN     60000000 /**< usec's in min */

#define OAM_USECS_TO_NANO   1000 /**< the multiplier from usec to nanosecs */

/**
 * Longest interval to reveive LBR befor it is discarded 
 */
#define OAM_LBR_TIMEOUT_SEC 5 

/**
 * The multiplier for too long receival of e.g. CCM
 */
#define OAM_ERROR_INTERVAL_MULTIPLIER 3.5 

/**
 * What is the name of the loopback device
 */
#define OAM_LOOPBACK_NAME "lo"

/**
 * The maximum lenght of the ifname
 */
#define OAM_IFNAME_MAX 6

/**
 * Length of the debug print time
 */
#define OAM_TIME_STRING_LEN 22

/**
 * Length needed for the internal time in str format.
 */
#define OAM_INTERNAL_TIME_TXT_LEN 22

/**
 * Enumeration of alarm states, used in alarm functions. 
 */
enum oam_alarms { OAM_ALARM_OFF = 0, OAM_ALARM_ON = 1 };

/**
 * The length of the used shift register for PRBS
 */
#define OAM_PRBS_SHIFT_REGISTER_LENGTH 31
/**
 * The first index of the feedback tap for PRBS 
 * (Must be inside the shift register!! Remember if the above is changed)
 */
#define OAM_PRBS_FIRST_TAP 27
/**
 * The second index of the feedback tap for PRBS
 * (Must be inside the shift register!! Remember if the above is changed)
 */
#define OAM_PRBS_SECOND_TAP 30
/**
 * The byte length in bits for PRBS
 */
#define OAM_PRBS_BYTE 8
/**
 * The bit one for PRBS
 */
#define OAM_PRBS_BIT_ONE 1
/**
 * The bit zero for PRBS
 */
#define OAM_PRBS_BIT_ZERO 0

/**
 * EP2 M
 */
#define OAM_EP2_M 3

/** 
 * Intervals used in sending the packets.
 *
 * @see ccm_period
 *
 * @note ARE THESE OF ANY USE AT ALL?
 */
enum oam_intervals
    {
        OAM_INTERVALS_IN_USEC_NONE  = 0, /**< Turn of sending e.g. LMM */
        OAM_INTERVALS_IN_USEC_333MS = 3330, /**< 3.33 ms in usecs */
        OAM_INTERVALS_IN_USEC_10MS  = 10000, /**< 10 ms in usecs */
        OAM_INTERVALS_IN_USEC_100MS = 100000, /**< 100 ms in usecs */
        OAM_INTERVALS_IN_USEC_1S    = 1000000, /**< 1 s in usecs */
        OAM_INTERVALS_IN_USEC_10S   = 10000000, /**< 10 s in usecs */
        OAM_INTERVALS_IN_USEC_1MIN  = 60000000, /**< 1 min in usecs */
        OAM_INTERVALS_IN_USEC_10MIN = 600000000 /**< 10 min in usecs */
    };

/**
 * On/Off states for use through out the program.
 */
enum oam_onoff
{
    OFF = 0,
    ON = 1
};

/**
 * Head of the control information db
 */
struct oam_db {
    /**
     * How many oam_entity_stat structs there are, does not include this entity
     */ 
    int      participants;
    enum oam_alarms rdi; /**< We have seen an alarm */
} __attribute__ ((packed));

/**
 * Internal if table line
 */
struct oam_if {
    unsigned char       mac[OAM_ETH_MAC_LENGTH]; /**< IF MAC address */
    struct sockaddr_in6 ip;          /**< IF IPv6 or mapped IPv4*/
    int                 ifindex;         /**< Interface index */
    char                ifindex_name[6]; /**< Interface name */
} __attribute__ ((packed));

/**
 * Time conforming to IEC 61588
 */
struct oam_internal_time {
    uint32_t seconds; /**< Seconds in internal time representation */
    uint32_t nanoseconds; /**< Nano seconds in internal time representation*/
} __attribute__ ((packed));

/**
 * Receive information 
 */
struct oam_recv_information {
    enum oam_onoff           multicast; /**< Was this received as multicast */
    int                      family;       /**< PF_INET, PF_INET6 or PF_PACKET*/
    int                      from_ifindex; /**< Receiving interface */ 
    int                      rcv_len; /**< length of the received frame.*/
    unsigned char            from_mac[OAM_ETH_MAC_LENGTH];  /**< From mac */
    struct oam_internal_time hw_time; /**< hardware timestamp SIOCGSTAMP */
    struct oam_internal_time sw_time; /**< userspace timestamp */

} __attribute__ ((packed));

/**
 * Tool information used to carry command line provided data
 */
struct oam_tool_information {
    uint16_t               mepid; /**< MEP ID */
    uint8_t                interval; /**< What is the used interval for send */
    uint8_t                count; /**< How many PDUs */
    uint8_t                loop; /**< Continuous or not */
    uint8_t                pattern; /**< used test pattern */
    uint16_t               pattern_len; /**< length of the used test pattern */
    uint32_t               interval_us; /**< used with SLM to give more flexibility to the interval */
} __attribute__ ((packed));

/**
 * Structure defining the oam entity.
 */
struct oam_entity { 
    char            id_icc[OAM_MAX_MEG_ID_ICC]; /**< Entity's MEG ID */
    uint16_t        mepid;           /**< Entity's MEP ID */
    int             meg_level;       /**< MEG level */
    enum oam_onoff  accuracy_correction; /**< do we try to correct the accuracy of the clock */
    enum oam_onoff  ccd; /**< do we send to central server or not */
    /** Send function that *Must* be used to send packets because the send
     * function is decided on the initialization and the pointer stored here */
    char            srv_ip[OAM_MAX_IPV4_LENGTH]; /**< CCD server ip */
    int             srv_port; /**< CCD server port */
    int (*send)(const int, 
                const void *const, 
                const struct oam_pdu_common *const,
                int,
                int); /**< Send function pointer. */ 
} __attribute__ ((packed));

/**
 * PDU send control information
 */
struct oam_send_control {
    enum oam_onoff           onoff; /**< Continuous sending of the PDU */
    int                      count; /**< How many PDUs have to be sent */
    struct oam_internal_time next_send; /**< If exceeded then send */
    uint32_t                 interval; /**< What is the PDU send interval */
    int                      sent; /**< How many PDUs succesfully sent */
    int                      received; /**< How many PDUs succesfully received */
} __attribute__ ((packed));

/**
 * Structure defining the oam entity statistics.
 */
struct oam_entity_stat{
    char                id_icc[OAM_MAX_MEG_ID_ICC]; /**< Entity's MEG ID */
    uint16_t            mepid;           /**< Entity's MEP ID */
    int                 meg_level;       /**< MEG level */
    unsigned char       mac[OAM_ETH_MAC_LENGTH]; /**< Entity's MAC address */
//    struct sockaddr_in6 ip;          /**< IPv6 or mapped IPv4*/
    int                 ifindex;         /**< Interface index for the MAC */
    char                ifindex_name[6]; /**< Interface name for the MAC */

    int rdi;     /**< I have seen an error from this entity */

    struct oam_ep2 *delay_results; /**< Delay results in ep2 struct */

    /****** CCM *******/
    uint32_t        ccm_seq; /**< CCM sequence number */
    struct timeval  ccm_stamp; /**< ccm received at this time, 0 if not used */
    enum oam_alarms alarm_ccm; /**< 1 if we did not receive CCM as expected */
    uint32_t        ccm_txfcf_tp; /**< previous TxFCf */
    uint32_t        ccm_txfcb_tp; /**< previous TxFCb  */
    uint32_t        ccm_rxfcb_tp; /**< previous rxfcb */
    uint32_t        ccm_rx; /**< RX upon receiving the CCM frame */
    uint32_t        ccm_my_txfcf; /**< TxFCf we sent to this MEP ID */    

    /****** LB *******/    
    struct oam_send_control  lb_send_control; /**< PDU send control info */
    long                     lb_usecs_rtt; /**< RTT for tool */
    uint32_t                 lb_seq; /**< Loopback sequence number */
    uint32_t                 lb_seq_in; /**< Loopback seq number we saw last */
    struct timeval           lb_stamp; /**< lbm send time, 0 if not used */
    struct oam_internal_time lb_stamp_int; /**< internal time */
    enum oam_alarms          alarm_lb; /**< ON if LBR not received in time */ 

    /****** LT *******/
    struct oam_send_control lt_send_control; /**< PDU send control info */
    uint32_t                lt_seq; /**< Linktrace sequence number */
    uint32_t                lt_seq_in; /**< Inbound linktrace seq s*/
    
    /****** AIS *******/
    int      ais;     /**< Alarm indication 0 = off, 1 = on */
    uint8_t  ais_recv_period; /**< last seen period */
    
    /****** LCK *******/
    struct oam_send_control lck_send_control; /**< PDU send control info */
    uint8_t                 lck_recv_period; /**< What was the last seen one */

    /****** TST *******/
    struct oam_send_control tst_send_control; /**< PDU send control info */
    uint32_t                tst_seq; /**< test sequence number */
    uint32_t                tst_seq_in; /**< test seq number we saw last */
    uint8_t                 tst_pattern; /**< Pattern type to use */
    uint16_t                tst_pattern_len; /**< length of the used pattern */

    /****** LM *******/
    struct oam_send_control lm_send_control; /**< PDU send control info */
    struct timeval          lm_stamp; /**< lbm send time, 0 if not used */   
    uint32_t                lmm_txfcf;     /**< The last sent TxFCf by MEP ID IS THIS USED ANYMORE */
    uint32_t                lmm_rxfcb;     /**< The last sent RxFCf by MEP ID */
    uint32_t                lmm_my_txfcf; /**< Our TX when last sent LMM */
    uint32_t                lm_num_of_errors; /**< how many errors we have seen */
    uint32_t                lm_num_of_warnings; /**< how many warnings we have seen */

    /****** DM *******/
    struct oam_send_control dm_send_control; /**< PDU send control info */
    time_t                  dm_stamp; /**< lbm sent at time, 0 if not used */   
    uint32_t                dm_num_of_errors; /**< how many errors we have seen */
    uint32_t                dm_num_of_warnings; /**< how many warnings we have seen */

    /****** 1DM *******/
    struct oam_send_control dm1_send_control; /**< PDU send control info */
    time_t                  dm1_stamp; /**< lbm sent at time, 0 if not used */   

    /****** SLM *******/
    struct oam_send_control sl_send_control; /**< PDU send control info */
    uint32_t                slm_test_id;   /**< test id */
    uint32_t                slm_txfcl_slm; /**< Sent SLM frames */
    uint32_t                slm_txfcl_slr; /**< Sent SLR frames */

} __attribute__ ((packed));

#define OAM_RUNNING 0 /**< Daemon is running */
#define OAM_STOPPED 1 /**< Daemon is stopped */

/**
* Maximum random delay that the MEPs/MIPs can use to delay the replies. 
*/
#define OAM_RANDOM_DELAY_MAX_FOR_REPLIES_IN_MS 1000

/* OAM_OPCODES pages 30-31 */

/**
 * \defgroup op_codes Constants for the frame types.
 */
/*@{*/

#define OAM_OPCODE_CCM   1 /**< Relevant to MEPs */ 
#define OAM_OPCODE_LBM   3 /**< Relevant to MEPs and MIPs (connectivity verification) */ 
#define OAM_OPCODE_LBR   2 /**< Relevant to MEPs and MIPs (connectivity verification) */ 
#define OAM_OPCODE_LTM   5 /**< Relevant to MEPs and MIPs */ 
#define OAM_OPCODE_LTR   4 /**< Relevant to MEPs and MIPs */
#define OAM_OPCODE_AIS  33 /**< Relevant to MEPs */
#define OAM_OPCODE_LCK  35 /**< Relevant to MEPs */
#define OAM_OPCODE_TST  37 /**< Relevant to MEPs */
#define OAM_OPCODE_LAPS 39 /**< Relevant to MEPs */
#define OAM_OPCODE_RAPS 40 /**< Relevant to MEPs */
#define OAM_OPCODE_MCC  41 /**< Relevant to MEPs */
#define OAM_OPCODE_LMM  43 /**< Relevant to MEPs */
#define OAM_OPCODE_LMR  42 /**< Relevant to MEPs */
#define OAM_OPCODE_1DM  45 /**< Relevant to MEPs */
#define OAM_OPCODE_DMM  47 /**< Relevant to MEPs */
#define OAM_OPCODE_DMR  46 /**< Relevant to MEPs */
#define OAM_OPCODE_EXM  49 /**< Outside of scope */
#define OAM_OPCODE_EXR  48 /**< Outside of scope */
#define OAM_OPCODE_VSR  50 /**< Outside of scope */
#define OAM_OPCODE_VSM  51 /**< Outside of scope */
#define OAM_OPCODE_CSF  52 /**< Relevant to MEPs */
#define OAM_OPCODE_SLR  54 /**< Relevant to MEPs */
#define OAM_OPCODE_SLM  55 /**< Relevant to MEPs */

/*@}*/

/**
 * \defgroup sub_op_codes Constants for the TLV types used to communicate between oamstatd and mepd.
 */
/*@{*/

/* OAM_SUB_OPCODES */
#define OAM_SUB_OPCODE_LMM  1 /**< Start LM */
#define OAM_SUB_OPCODE_1DM  2 /**< Start One-way DM */
#define OAM_SUB_OPCODE_DMM  3 /**< Start Two-way DM */
#define OAM_SUB_OPCODE_LBM  4 /**< Start LBM */
#define OAM_SUB_OPCODE_LTM  5 /**< Start LTM */
#define OAM_SUB_OPCODE_TST  6 /**< Start TST */
#define OAM_SUB_OPCODE_LCK  7 /**< Start LCK */
#define OAM_SUB_OPCODE_SLM  8 /**< Start SLM */

/*@}*/

/**
 * \defgroup tlv_types_local TLV types used for between peers and in IPC.
 */
/*@{*/

/* OAM_SUB_OPCODES */
#define OAM_TLV_TYPE_TOOL    10 /**< Local use: MEP ID sent to the mepd */

/*@}*/

/**
 * \defgroup tst_pattern_types pattern types used for between peers for testing.
 */
/*@{*/

/* OAM_PATTERN_TYPES */
#define OAM_PATTERN_TYPE_ZERO_NO_CRC 0 /**< All zeroes */
#define OAM_PATTERN_TYPE_ZERO_CRC    1 /**< All zeroes With CRC*/
#define OAM_PATTERN_TYPE_PRBS_NO_CRC 2 /**< PRBS (2^31-1) Without CRC */
#define OAM_PATTERN_TYPE_PRBS_CRC    3 /**< PRBS (2^31-1) With CRC*/

/*@}*/

/* OAM_TLVOFFSETs */

/**
 * \defgroup op_codes Constants for the frame types.
 */
/*@{*/

#define OAM_TLVOFFSET_CCM  70 /**< Fixed content, end-TLV */
#define OAM_TLVOFFSET_LBM   4 /**< Start of optional data or end-TLV */
#define OAM_TLVOFFSET_LBR   4 /**< Start of optional data or end-TLV */
#define OAM_TLVOFFSET_LTM  17 /**< Start of additional TLVs*/
#define OAM_TLVOFFSET_LTR   6 /**< Start of additional TLVs*/
#define OAM_TLVOFFSET_AIS   0 /**< Just the end-TLV */
#define OAM_TLVOFFSET_LCK   0 /**< Just the end-TLV */
#define OAM_TLVOFFSET_TST   4 /**< Start of additional Test TLVs or end-TLV */
/* #define OAM_TLVOFFSET_LAPS  ? out-of-scope */
/* #define OAM_TLVOFFSET_RAPS  ? out-of-scope */
#define OAM_TLVOFFSET_MCC   4 /**< Start of MCC data or end-TLV*/
#define OAM_TLVOFFSET_LMM  12 /**< Fixed content, end-TLV */
#define OAM_TLVOFFSET_LMR  12 /**< Fixed content, end-TLV */
#define OAM_TLVOFFSET_1DM  16 /**< Fixed content, end-TLV */
#define OAM_TLVOFFSET_DMM  32 /**< Fixed content, end-TLV */
#define OAM_TLVOFFSET_DMR  32 /**< Fixed content, end-TLV */
#define OAM_TLVOFFSET_EXM   4 /**< Start of optional data or end-TLV */
#define OAM_TLVOFFSET_EXR   4 /**< Start of optional data or end-TLV */
#define OAM_TLVOFFSET_VSR   4 /**< Start of optional data or end-TLV */
#define OAM_TLVOFFSET_VSM   4 /**< Start of optional data or end-TLV */
#define OAM_TLVOFFSET_SLM  16 /**< Start of additional Test TLVs or end-TLV */
#define OAM_TLVOFFSET_SLR  16 /**< Start of additional Test TLVs or end-TLV */

/*@}*/

/* TLV TYPES page 32 */

/**
 * \defgroup tlv_types_out Constants for the TLV types.
 */
/*@{*/

#define OAM_TYPE_END_TLV                    0 /**< End TLV */
#define OAM_TYPE_DATA_TLV                   3 /**< Data TLV */
#define OAM_TYPE_REPLY_INGRESS_TLV          5 /**< Reply ingress TLV */
#define OAM_TYPE_REPLY_EGRESS_TLV           6 /**< Reply egress TLV */
#define OAM_TYPE_LTM_EGRESS_IDENTIFIER_TLV  7 /**< LTM egress identifier TLV */
#define OAM_TYPE_LTR_EGRESS_IDENTIFIER_TLV  8 /**< LTR egress identifier TLV */
#define OAM_TYPE_TEST_TLV                  32 /**< Test TVL */
/*@}*/

/* CCM Period values page 34 */

/**
 * \defgroup ccm_period Constants for the CCM periods.
 */
/*@{*/

#define OAM_CCM_PERIOD_THREEISH_MS 0x1 /**< 001 3.33 ms 300 frames per second */
#define OAM_CCM_PERIOD_10MS        0x2 /**< 010 10 ms 100 frames per second */
#define OAM_CCM_PERIOD_100MS       0x3 /**< 011 100 ms 10 frames per second */
#define OAM_CCM_PERIOD_1S          0x4 /**< 100 1 s 1 frame per second */
#define OAM_CCM_PERIOD_10S         0x5 /**< 101 10 s 6 frames per minute */
#define OAM_CCM_PERIOD_1MIN        0x6 /**< 110 1 min 1 frame per minute */
#define OAM_CCM_PERIOD_10MIN       0x7 /**< 111 10 min 6 frames per hour */

/*@}*/
 
/*AIS/LCK period values page 42*/

/**
 * \defgroup ais_lck_period Constants for the AIS/LCK period.
 */
/*@{*/

#define OAM_AIS_LCK_PERIOD_1S   0x4 /**< 100 1 s 1 frame per second */
#define OAM_AIS_LCK_PERIOD_1MIN 0x6 /**< 110 1 min 1 frame per minute */

/*@}*/

/* MEG ID format types */

/**
 * \defgroup meg_id_type Constants for the MEG ID types.
 */
/*@{*/

#define OAM_MEG_ID_TYPE 32 /**< ICC-based format */

/*@}*/

/* MEG CSF types */

/**
 * \defgroup meg_csf_type Constants for the MEG CSF types.
 */
/*@{*/

#define OAM_MEG_CSF_TYPE_LOS    0x0 /**< Client Loss of Signal */
#define OAM_MEG_CSF_TYPE_FDIAIS 0x1 /**< Client forward defect indication */
#define OAM_MEG_CSF_TYPE_RDI    0x2 /**< Client Reverse defect indication */
#define OAM_MEG_CSF_TYPE_DCI    0x3 /**< Client Defect clear indication */

/*@}*/

/**
 * \defgroup defect_conditions Defect condition constants for receiving OAM PDUs.
 */
/*@{*/

#define OAM_DEFCON_UNUSED               0 /**< Unused */
#define OAM_DEFCON_CCM_NO_FRAMES        1 /**< No Frames inside 3.5 times the period */
#define OAM_DEFCON_CCM_UNEXPECTED_LEVEL 2 /**< MEG level lower than our own */
#define OAM_DEFCON_CCM_MISMERGE         3 /**< Correct MEG level but MEG ID different to that we belong to */
#define OAM_DEFCON_CCM_UNEXPECTED_MEP   4 /**< Correct MEG level and MEG ID but incorrect MEP ID */
#define OAM_DEFCON_CCMUNEXPECTED_PERIOD 5 /**< Correct MEG level, MEG ID, and MEP ID but different period than ours */

/*@}*/

#endif /* OAM_CONSTANTS_H */
