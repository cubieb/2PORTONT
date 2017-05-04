/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI driver define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI driver define
 */

#ifndef __OMCI_DRIVER_H__
#define __OMCI_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <omci_mib.h>
#include "omci_dm_sd.h"
#include "hal/chipdef/chip.h"
#include "omci_customize.h"
#include "omci_vlan_rule.h"

#define OMCI_VEIP_PORT 0x601

#define OMCI_DSCP_NUM 64

#define OMCI_DRV_INVALID_TCONT_ID   (0xFFFF)
#define OMCI_DRV_INVALID_QUEUE_ID   (0xFFFF)

#define MMT_BUF_SIZE                (8192)
#define DEFAULT_AGING_TIME_IN_SEC   (300)
#define DEFAULT_AGING_TIME_UNIT     (7)

enum {
	OMCI_LOG_LEVEL_OFF = 0,  /*For disable log*/
	OMCI_LOG_LEVEL_DRIVER,
	OMCI_LOG_LEVEL_HIGH,
	OMCI_LOG_LEVEL_NORMAL,
	OMCI_LOG_LEVEL_LOW,
};


typedef enum
{
    PON_ONU_STATE_INITIAL            = 0x01,      /* O1 */
    PON_ONU_STATE_STANDBY            = 0x02,      /* O2 */
    PON_ONU_STATE_SERIAL_NUMBER      = 0x03,      /* O3 */
    PON_ONU_STATE_RANGING            = 0x04,      /* O4 */
    PON_ONU_STATE_OPERATION          = 0x05,      /* O5 */
    PON_ONU_STATE_POPUP              = 0x06,      /* O6 */
    PON_ONU_STATE_EMERGENCY_STOP     = 0x07,      /* O7 */
    PON_ONU_STATE_UNKNOWN            = 0x08,
} PON_ONU_STATE;


typedef enum {
    PLOAM_DS_OVERHEAD        = 0x01,
    PLOAM_DS_SNMASK          = 0x02,
    PLOAM_DS_ASSIGNONUID     = 0x03,
    PLOAM_DS_RANGINGTIME     = 0x04,
    PLOAM_DS_DEACTIVEONU     = 0x05,
    PLOAM_DS_DISABLESN       = 0x06,
    PLOAM_DS_CFG_VPVC        = 0x07,
    PLOAM_DS_ENCRYPTPORT     = 0x08,
    PLOAM_DS_REQUESTPASSWORD = 0x09,
    PLOAM_DS_ASSIGNEDALLOCID = 0x0A,
    PLOAM_DS_NOMESSAGE       = 0x0B,
    PLOAM_DS_POPUP           = 0x0C,
    PLOAM_DS_REQUESTKEY      = 0x0D,
    PLOAM_DS_CONFIGPORT      = 0x0E,
    PLOAM_DS_PEE             = 0x0F,
    PLOAM_DS_POWERLEVEL      = 0x10,
    PLOAM_DS_PST             = 0x11,
    PLOAM_DS_BER_INTERVAL    = 0x12,
    PLOAM_DS_SWITCHINGKEY    = 0x13,
    PLOAM_DS_EXT_BURSTLENGTH = 0x14,
}GMac_Ploam_Ds_te;


#define DEFAULT_LOGGING_ACTMASK (OMCI_ME_ACTION_CREATE | OMCI_ME_ACTION_DELETE | OMCI_ME_ACTION_SET | OMCI_ME_ACTION_MIB_RESET)


typedef enum
{
	OMCI_ERR_OK,
	OMCI_ERR_FAILED,
	OMCI_ERR_ENTRY_EXIST,
	OMCI_ERR_END
}OMCI_CHIP_ERROR_CODE;




typedef struct {
	int tcontId;
	int allocId;
} OMCI_TCONT_ts;

typedef struct
{
	unsigned int	flowId;
	unsigned int	portId;
	unsigned int	tcontId; /*OMCI entity index*/
	unsigned int	allocId;  /*Tcont Physical allocated Id*/
	unsigned int 	queueId; /*queue entity index*/
	unsigned int	cir;
	unsigned int	pir;
	int             isOmcc;
	int				isFilterMcast;
	int             ena;
	PON_GEMPORT_DIRECTION dir;

} OMCI_GEM_FLOW_ts;


typedef enum
{
	TPID_8100,
	TPID_88A8
}OMCI_TPID_VALUE_e;

typedef enum
{
	MAC_FILTER_DIR_US,
	MAC_FILTER_DIR_DS
}OMCI_MAC_FILTER_DIR;

typedef enum
{
	MAC_FILTER_TYPE_DA,
	MAC_FILTER_TYPE_SA
}OMCI_MAC_FILTER_TYPE;

typedef enum
{
	MAC_FILTER_ACT_FORWARD,
	MAC_FILTER_ACT_FILTER
}OMCI_MAC_FILTER_ACT;

typedef enum
{
	MAC_FILTER_ENTRY_ACT_REMOVE,
	MAC_FILTER_ENTRY_ACT_CLEAR_ALL,
	MAC_FILTER_ENTRY_ACT_ADD
}OMCI_MAC_FILTER_ENTRY_ACT;


typedef enum
{
	GROUP_MAC_PROTOCOL_IPV4_MCAST = 0,
	GROUP_MAC_PROTOCOL_IPV6_MCAST,
	GROUP_MAC_PROTOCOL_IPV4_BCAST,
	GROUP_MAC_PROTOCOL_RARP,
	GROUP_MAC_PROTOCOL_IPX,
	GROUP_MAC_PROTOCOL_NET_BEUI,
	GROUP_MAC_PROTOCOL_APPLE_TALK,
	GROUP_MAC_PROTOCOL_BPDU,
	GROUP_MAC_PROTOCOL_ARP,
	GROUP_MAC_PROTOCOL_PPPOE_BCAST,
	GROUP_MAC_PROTOCOL_END
}OMCI_GROUP_MAC_PROTOCOL;

#define GROUP_MAC_PROTOCOL_NUM GROUP_MAC_PROTOCOL_END

typedef struct omci_bridge_rule_s
{
	int 					isUsed;		/*to specify this rule is in used or not*/
	int						isLatch;		/* used for acl latch cf rule */
	int						servId;		/*used for omci to mapping chip cf rule*/
	int						uniMask;    /*ingress uni port mask for upstream direction*/
	int						usFlowId;	/* US: stream Id = flow Id belong to logical. It is used for mapping to gem port */
    int                     usDpFlowId;
    int                     usDpMarking;
	int						dsFlowId;	/* DS: stream Id = flow Id belong to logical. It is used for mapping to gem port */
	PON_GEMPORT_DIRECTION	dir;		/*direction for rule*/
	OMCI_VLAN_OPER_ts		vlanRule;	/*vlan related setting*/
} OMCI_BRIDGE_RULE_ts;


typedef enum
{
    PQ_POLICY_STRICT_PRIORITY,
    PQ_POLICY_WEIGHTED_ROUND_ROBIN
} OMCI_PQ_POLICY_e;

typedef struct
{
	unsigned short queueId;
	unsigned short tcontId;
	unsigned int cir;
	unsigned int pir;
	unsigned short weight;
	OMCI_PQ_POLICY_e scheduleType;
    unsigned char dpMarking;
    PON_GEMPORT_DIRECTION dir;

} OMCI_PRIQ_ts;


typedef struct {
	int logical;
	int physical;
} OMCI_PORT_ts;

typedef struct{
	unsigned char pbit[OMCI_DSCP_NUM];
}OMCI_DSCP2PBIT_ts;

typedef struct{
	unsigned int portIdx;
	unsigned int macLimitNum;
}OMCI_MACLIMIT_ts;

typedef struct{
	unsigned int id;
	unsigned int portMask; //TBD for 9607
	unsigned char mac[ETHER_ADDR_LEN];
	unsigned char entryAct;
	OMCI_MAC_FILTER_TYPE macType;
	OMCI_MAC_FILTER_DIR dir;
	OMCI_MAC_FILTER_ACT filter;
	/* TBD vlanId for different OMCI model */
}OMCI_MACFILTER_ts;

typedef struct{
	unsigned int portIdx;
	unsigned char protocol_act[GROUP_MAC_PROTOCOL_NUM];
	unsigned char protocol_mask[GROUP_MAC_PROTOCOL_NUM];
}OMCI_GROUPMACFILTER_ts;

typedef enum
{
	SIG_TYPE_LOS = 1,
	SIG_TYPE_LOF,
	SIG_TYPE_LOM,
	SIG_TYPE_SF,
	SIG_TYPE_SD
}OMCI_SIG_TYPE;

typedef struct
{
	OMCI_SIG_TYPE	signal_type;
	unsigned int	signal_threshold;
}OMCI_SIGNAL_PARA_ts;

typedef enum
{
    OMCI_TRANSCEIVER_STATUS_TYPE_VENDOR_NAME = 0,
    OMCI_TRANSCEIVER_STATUS_TYPE_VENDOR_PART_NUM,
    OMCI_TRANSCEIVER_STATUS_TYPE_TEMPERATURE,
    OMCI_TRANSCEIVER_STATUS_TYPE_VOLTAGE,
    OMCI_TRANSCEIVER_STATUS_TYPE_BIAS_CURRENT,
    OMCI_TRANSCEIVER_STATUS_TYPE_TX_POWER,
    OMCI_TRANSCEIVER_STATUS_TYPE_RX_POWER,
    OMCI_TRANSCEIVER_STATUS_TYPE_END
} omci_transceiver_status_type_t;

#define OMCI_DRV_TRANSCEIVER_DATA_LEN   (32)

typedef struct
{
    omci_transceiver_status_type_t  type;
    unsigned char                   data[OMCI_DRV_TRANSCEIVER_DATA_LEN];
} omci_transceiver_status_t;

typedef struct
{
    unsigned int port;
    unsigned int state;
} omci_port_state_t;

typedef struct
{
    unsigned int port;
    unsigned int status;
} omci_port_link_status_t;

typedef enum
{
    OMCI_PORT_SPEED_10M,
    OMCI_PORT_SPEED_100M,
    OMCI_PORT_SPEED_1000M,
    OMCI_PORT_SPEED_END
} omci_port_speed_t;

typedef enum
{
    OMCI_PORT_HALF_DUPLEX,
    OMCI_PORT_FULL_DUPLEX,
    OMCI_PORT_DUPLEX_END
} omci_port_duplex_t;

typedef struct
{
    unsigned int        port;
    omci_port_speed_t   speed;
    omci_port_duplex_t  duplex;
} omci_port_speed_duplex_status_t;

typedef struct
{
    unsigned int    port;
    unsigned int    half_10:1;
    unsigned int    full_10:1;
    unsigned int    half_100:1;
    unsigned int    full_100:1;
    unsigned int    half_1000:1;
    unsigned int    full_1000:1;
} omci_port_auto_nego_ability_t;

typedef struct
{
    unsigned int port;
    unsigned int size;
} omci_port_max_frame_size_t;

typedef struct
{
    unsigned int port;
    unsigned int loopback;
} omci_port_loopback_t;

typedef struct
{
    unsigned int port;
    unsigned int state;
} omci_port_pwr_down_t;

typedef struct
{
    unsigned int port;
    unsigned int pause_time;
} omci_port_pause_ctrl_t;

typedef struct
{
    unsigned int        port;
    unsigned long long  ifInOctets;
    unsigned int        ifInUcastPkts;
    unsigned int        ifInMulticastPkts;
    unsigned int        ifInBroadcastPkts;
    unsigned long long  ifOutOctets;
    unsigned int        ifOutUcastPkts;
    unsigned int        ifOutMulticastPkts;
    unsigned int        ifOutBrocastPkts;
    unsigned int        dot1dTpPortInDiscards;
    unsigned int        dot3StatsSingleCollisionFrames;
    unsigned int        dot3StatsMultipleCollisionFrames;
    unsigned int        dot3StatsDeferredTransmissions;
    unsigned int        dot3StatsLateCollisions;
    unsigned int        dot3StatsExcessiveCollisions;
    unsigned int        etherStatsDropEvents;
    unsigned int        etherStatsFragments;
    unsigned int        etherStatsJabbers;
    unsigned int        etherStatsCRCAlignErrors;
    unsigned int        etherStatsTxUndersizePkts;
    unsigned int        etherStatsTxOversizePkts;
    unsigned int        etherStatsTxPkts64Octets;
    unsigned int        etherStatsTxPkts65to127Octets;
    unsigned int        etherStatsTxPkts128to255Octets;
    unsigned int        etherStatsTxPkts256to511Octets;
    unsigned int        etherStatsTxPkts512to1023Octets;
    unsigned int        etherStatsTxPkts1024to1518Octets;
    unsigned int        etherStatsTxPkts1519toMaxOctets;
    unsigned int        etherStatsTxCRCAlignErrors;
    unsigned int        etherStatsRxUndersizePkts;
    unsigned int        etherStatsRxOversizePkts;
    unsigned int        etherStatsRxPkts64Octets;
    unsigned int        etherStatsRxPkts65to127Octets;
    unsigned int        etherStatsRxPkts128to255Octets;
    unsigned int        etherStatsRxPkts256to511Octets;
    unsigned int        etherStatsRxPkts512to1023Octets;
    unsigned int        etherStatsRxPkts1024to1518Octets;
    unsigned int        etherStatsRxPkts1519toMaxOctets;
} omci_port_stat_t;

typedef struct
{
    unsigned int        flow;
    unsigned int        gemBlock;
    unsigned long long  gemByte;
} omci_flow_stat_t;

typedef struct
{
    unsigned int    corByte;
    unsigned int    corCodeword;
    unsigned int    uncorCodeword;
} omci_ds_fec_stat_t;

typedef struct
{
    unsigned int    index;
    unsigned int    tpid;
} omci_svlan_tpid_t;

typedef struct
{
    unsigned int    bwThreshold;
    unsigned int    reqBwThreshold;
} omci_pon_bw_threshold_t;

typedef struct
{
    unsigned int    gemPortId;
    unsigned int    tcontId;
    unsigned int    flowId[WAN_PONMAC_QUEUE_MAX];
    unsigned int    tcQueueId[WAN_PONMAC_QUEUE_MAX];
} veipGemFlow_t;

#define MAX_BW_RATE (1048568)
typedef struct
{
    unsigned int port;
    unsigned int dir;
    unsigned int rate;
} omci_port_rate_t;

typedef enum
{
    OMCI_DOT1_RATE_UNICAST_FLOOD,
    OMCI_DOT1_RATE_BROADCAST,
    OMCI_DOT1_RATE_MULTICAST_PAYLOAD,
    OMCI_DOT1_RATE_TYPE_END,
} omci_dot1_rate_type_t;

typedef struct
{
    unsigned int            portMask;
    omci_dot1_rate_type_t   type;
    unsigned int            cir;
    unsigned int            cbs;
} omci_dot1_rate_t;

typedef struct {
    unsigned int        meterId;
    omci_dot1_rate_t    dot1Rate;
} omci_dot1_rate_meter_t;

/* for priority Queue and real tcont/tcont queue mapping */
typedef struct {
	UINT16				tcontId;
	UINT16				tcQueueId;
	OMCI_PQ_POLICY_e	policy;
	union
	{
		UINT16			priority;
		UINT16			weight;
	} pw;
	UINT16				dpQueueId;
	UINT8				dpMarking;
	UINT32				cir;
	UINT32				pir;
    UINT8				mulQueue; /* for multiQ per gem */
} omci_pq_info_t;

typedef struct {
	UINT16	entityId;
	UINT16	allocId;
	UINT16	gemCount;
} omci_tcont_info_t;

typedef struct {
	UINT16	portId;
	UINT16	pqEntityId;
	BOOL	inUse;
} omci_gem_flow_info_t;

typedef struct {
    UINT32                  dot1PortMask;
    omci_dot1_rate_type_t   dot1Type;
    BOOL                    inUse;
} omci_meter_info_t;

typedef struct {
    unsigned short  portId;
    unsigned short  cnt;
} omci_bridge_tbl_per_port_t;

typedef enum {
    // driver control
	OMCI_IOCTL_MIB_RESET,
	OMCI_IOCTL_LOG_SET,
	OMCI_IOCTL_DEVMODE_SET,

    // device info
    OMCI_IOCTL_DEV_CAPABILITIES_GET,
    OMCI_IOCTL_DEV_ID_VERSION_GET,
    OMCI_IOCTL_DUAL_MGMT_MODE_SET,
    OMCI_IOCTL_DRV_VERSION_GET,

    // optical info and control
    OMCI_IOCTL_US_DBRU_STATUS_GET,
    OMCI_IOCTL_TRANSCEIVER_STATUS_GET,
    OMCI_IOCTL_SIGPARAMETER_SET,
    OMCI_IOCTL_ONUSTATE_GET,

    // pon/ani control
    OMCI_IOCTL_SN_SET,
    OMCI_IOCTL_SN_GET,
    OMCI_IOCTL_GPONPWD_SET,
    OMCI_IOCTL_GPON_ACTIVATE,
    OMCI_IOCTL_GEMBLKLEN_SET,
    OMCI_IOCTL_GEMBLKLEN_GET,
    OMCI_IOCTL_PON_BWTHRESHOLD_SET,
    OMCI_IOCTL_TCONT_GET,
    OMCI_IOCTL_TCONT_UPDATE,
    OMCI_IOCTL_PRIQ_SET,
    OMCI_IOCTL_PRIQ_DEL,
    OMCI_IOCTL_GEMPORT_SET,
    OMCI_IOCTL_DS_BC_GEM_FLOW_SET,

    // uni info and control
    OMCI_IOCTL_PORT_LINK_STATUS_GET,
    OMCI_IOCTL_PORT_SPEED_DUPLEX_STATUS_GET,
    OMCI_IOCTL_PORT_AUTO_NEGO_ABILITY_SET,
    OMCI_IOCTL_PORT_AUTO_NEGO_ABILITY_GET,
    OMCI_IOCTL_PORT_STATE_SET,
    OMCI_IOCTL_PORT_STATE_GET,
    OMCI_IOCTL_PORT_MAX_FRAME_SIZE_SET,
    OMCI_IOCTL_PORT_MAX_FRAME_SIZE_GET,
    OMCI_IOCTL_PORT_PHY_LOOPBACK_SET,
    OMCI_IOCTL_PORT_PHY_LOOPBACK_GET,
    OMCI_IOCTL_PORT_PHY_PWR_DOWN_SET,
    OMCI_IOCTL_PORT_PHY_PWR_DOWN_GET,
    OMCI_IOCTL_PORT_PAUSE_CONTROL_SET,
    OMCI_IOCTL_PORT_PAUSE_CONTROL_GET,

    // statistics
    OMCI_IOCTL_CNTR_PORT_GET,
    OMCI_IOCTL_CNTR_PORT_CLEAR,
    OMCI_IOCTL_CNTR_US_FLOW_GET,
    OMCI_IOCTL_CNTR_US_FLOW_CLEAR,
    OMCI_IOCTL_CNTR_DS_FLOW_GET,
    OMCI_IOCTL_CNTR_DS_FLOW_CLEAR,
    OMCI_IOCTL_CNTR_DS_FEC_GET,
    OMCI_IOCTL_CNTR_DS_FEC_CLEAR,

    // bridge/vlan control
    OMCI_IOCTL_CF_DEL,
    OMCI_IOCTL_CF_ADD,
    OMCI_IOCTL_DSCPREMAP_SET,
    OMCI_IOCTL_MACLIMIT_SET,
    OMCI_IOCTL_MACFILTER_SET,
    OMCI_IOCTL_GROUPMACFILTER_SET,
    OMCI_IOCTL_SVLAN_TPID_SET,
    OMCI_IOCTL_SVLAN_TPID_GET,
    OMCI_IOCTL_CVLAN_STATE_GET,
    OMCI_IOCTL_DOT1_RATE_LIMITER_SET,
    OMCI_IOCTL_DOT1_RATE_LIMITER_DEL,
    OMCI_IOCTL_BG_TBL_PER_PORT_GET,
    OMCI_IOCTL_MAC_AGEING_TIME_SET,

    // veip control
    OMCI_IOCTL_VEIP_GEM_FLOW_SET,
    OMCI_IOCTL_VEIP_GEM_FLOW_DEL,

    // uni rate
    OMCI_IOCTL_UNI_PORT_RATE,

	OMCI_IOCTL_END
}OMCI_IOCTL_t;


enum {
	OMCI_DEV_MODE_BRIDGE=0,
	OMCI_DEV_MODE_ROUTER,
	OMCI_DEV_MODE_HYBRID
};


typedef struct
{
    // portType should be rt_port_type_t
    // defined as uint8 just to save storage
    uint8   portType;
    uint8   portIdInType;
} omci_uni_port_t;

typedef struct
{
    omci_uni_port_t     uniPort[RTK_MAX_NUM_OF_PORTS];
    uint32              fePortNum;
    uint32              gePortNum;
    int32               cpuPort;
    int32               ponPort;
    int32               rgmiiPort;
    uint32              totalTContNum;
    uint32              totalGEMPortNum;
    uint32              totalTContQueueNum;
    uint32              perUNIQueueNum;
    uint8               perTContQueueDp;
    uint8               perUNIQueueDp;
    uint32              meterNum;
    uint32              totalL2Num;
} omci_dev_capability_t;

#define OMCI_DRV_STR_NL_LEN         (1)
#define OMCI_DRV_DEV_ID_LEN         (14 + OMCI_DRV_STR_NL_LEN)
#define OMCI_DRV_DEV_VERSION_LEN    (20 + OMCI_DRV_STR_NL_LEN)

typedef struct
{
    char    id[OMCI_DRV_DEV_ID_LEN];
    char    version[OMCI_DRV_DEV_VERSION_LEN];
	uint32	chipId;
} omci_dev_id_version_t;

/*bit mask*/
typedef enum {
	OMCI_LOGFILE_MODE_DISABLE = 0x0,
	OMCI_LOGFILE_MODE_RAW     = 0x1,
	OMCI_LOGFILE_MODE_PARSED  = 0x2,
	OMCI_LOGFILE_MODE_WITH_TIMESTAMP = 0x4,
}OMCI_LOGFILE_MODE;

typedef struct
{
    unsigned char   vlan_cfg_type;
    unsigned char   vlan_cfg_manual_mode;
    unsigned char   vlan_cfg_manual_pri;
    unsigned short  vlan_cfg_manual_vid;
}omci_iot_vlan_cfg_t;

typedef struct
{
	unsigned char loid[24];
	unsigned char loidPwd[12];
	unsigned char loidAuthStatus; /*Store the AuthStatus to avoid remove by mib reset*/
	unsigned char lastLoidAuthStatus; /*Store the Last AuthStatus set by OLT for user get auth result and not be set to 0(init) by non-O5*/
}omci_loid_cfg_t;

typedef struct
{
	int devMode;
    int dmMode;
	int receiveState;
	int logLevel;
	unsigned char sn[16];
	unsigned char gponPwd[10];
	omci_loid_cfg_t loidCfg;
	unsigned int gLoggingActMask; /* Used for logging omcilog with the specefic omci action */
	unsigned int changeSvlanTpidB;
	omci_customer_feature_flag_t customize;
    omci_dev_capability_t devCapabilities;
    omci_dev_id_version_t devIdVersion;
    omci_iot_vlan_cfg_t iotVlanCfg;
	unsigned int tmTimerRunningB;
	unsigned int logFileMode;
    void  *pMCwrapper;
}OMCI_STATUS_INFO_ts;

extern OMCI_STATUS_INFO_ts gInfo;


typedef enum
{
	TRANSPARENT		=(1 << 0),
	ADD_VID			=(1 << 1),
	ADD_PRI			=(1 << 2),
	COPY_INNER_VID	=(1 << 3),
	COPY_OUTER_VID	=(1 << 4),
	COPY_INNER_PRI	=(1 << 5),
	COPY_OUTER_PRI	=(1 << 6),
	REMOVE_VLAN		=(1 << 7),
	MODIFY_VID		=(1 << 8),
	MODIFY_PRI		=(1 << 9)
}TAG_ACT_MODE_e;

typedef enum
{
	NO_FILTER		=(1 << 0),	// = 0xFFFF
	FILTER_UNTAG	=(1 << 1),  // = 4096
	FILTER_1TAG		=(1 << 2),	// = 4097
	FILTER_PRI		=(1 << 3),  // = 4098 only priority		-- 4100 PRI+VID
	FILTER_VID		=(1 << 4),	// = 4099 only vid			-- 4100 PRI+VID
	FILTER_ETYPE	=(1 << 6)
}TAG_FILTER_MODE_e;


typedef struct tag_s
{
	unsigned short tpid;
	union
    {
        struct
        {
            unsigned short pri : 3;
            unsigned short dei : 1;
            unsigned short vid : 12;
        } bit;
        unsigned short val;
    } tci;

	union
	{
		TAG_ACT_MODE_e		tagAct;
		TAG_FILTER_MODE_e	tagFilter;
	} tagOp;

} __attribute__((packed)) TAG_t;

typedef struct omci_mcast_port_info_s
{
	unsigned short			mopId;
	unsigned short			vidUni;
	unsigned int			uniPort;
	TAG_t					inTag1; /* stag */
	TAG_t					inTag2; /* ctag */
	TAG_t					outTag1; /* stag */
	TAG_t					outTag2; /* ctag */
	int						usFlowId;
} __attribute__((packed)) OMCI_MCAST_PORT_INFO_t;

enum
{
	CTRL_PASS,
	CTRL_ADD,
	CTRL_REPLACE,
	CTRL_REPLACE_VID,
};

typedef enum
{
	TPID_FILTER,
	TPID_ACT,
}TPID_OP_e;

enum
{
    SET_CTRL_RSV        = 0,
    SET_CTRL_WRITE      = (1 << 0),
    SET_CTRL_DELETE     = (1 << 1),
    SET_CTRL_DELETE_ALL = (SET_CTRL_WRITE | SET_CTRL_DELETE),
};

enum
{
    IP_TYPE_UNKNOWN,
    IP_TYPE_IPV4,
    IP_TYPE_IPV6,
};

typedef INT32 (*OMCI_IOCTL_SEND_PTR)(int, void *, int);
typedef UINT16 (*OMCI_GET_AVAIL_USFLOWID_PTR)(void);
typedef GOS_ERROR_CODE (*OMCI_DEL_PRIQ_BY_QID_PTR)(UINT16);

#ifdef __cplusplus
}
#endif

#endif
