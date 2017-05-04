#ifndef __RTK_API_ACL_H__
#define __RTK_API_ACL_H__

#include "rtk_api_common.h"
#include "rtk_types_common.h"
#include "rtk_error.h"



#define RTK_FILTER_RAW_FIELD_NUMBER                8

#define ACL_DEFAULT_ABILITY                         0
#define ACL_DEFAULT_UNMATCH_PERMIT                  1

#define ACL_RULE_FREE                               0
#define ACL_RULE_INAVAILABLE                        1
#define ACL_RULE_CARETAG_MASK						0x1F
#define FILTER_POLICING_MAX                         4
#define FILTER_LOGGING_MAX                          8
#define FILTER_PATTERN_MAX                          4

#define RTK_IPV6_ADDR_WORD_LENGTH                   4UL
#define RTK_DOT_1AS_TIMESTAMP_UNIT_IN_WORD_LENGTH   3UL


#define RTK_FILTER_FIELD_USED_MAX					8
#define RTK_FILTER_FIELD_INDEX(template, index)  	((template << 4) + index)


typedef struct rtk_filter_field rtk_filter_field_t;
typedef rtk_uint32 rtk_filter_field_raw_t;
typedef rtk_uint32  rtk_filter_id_t;    /* filter id type */
typedef rtk_uint32 rtk_filter_number_t;


typedef enum rtk_filter_field_data_type_e
{
    FILTER_FIELD_DATA_MASK = 0,
    FILTER_FIELD_DATA_RANGE,
    FILTER_FIELD_DATA_END ,
} rtk_filter_field_data_type_t;

typedef enum rtk_filter_care_tag_index_e
{
    CARE_TAG_CTAG = 0,
    CARE_TAG_STAG,
    CARE_TAG_PPPOE,
    CARE_TAG_IPV4,
    CARE_TAG_IPV6,
    CARE_TAG_TCP,
    CARE_TAG_UDP,
    CARE_TAG_ARP,
    CARE_TAG_RSV1,
    CARE_TAG_RSV2,
    CARE_TAG_ICMP,
    CARE_TAG_IGMP,
    CARE_TAG_LLC,
    CARE_TAG_RSV3,
    CARE_TAG_HTTP,
    CARE_TAG_RSV4,
    CARE_TAG_RSV5,
    CARE_TAG_DHCP,
    CARE_TAG_DHCPV6,
    CARE_TAG_SNMP,
    CARE_TAG_OAM,
    CARE_TAG_END,
} rtk_filter_care_tag_index_t;

typedef struct rtk_filter_flag_s
{
    rtk_uint32 value;
    rtk_uint32 mask;
} rtk_filter_flag_t;

typedef struct rtk_filter_care_tag_s
{
    rtk_filter_flag_t tagType[CARE_TAG_END];
} rtk_filter_care_tag_t;

typedef struct rtk_filter_value_s
{
    rtk_uint32 dataType;
    rtk_uint32 value;
    rtk_uint32 mask;
    rtk_uint32 rangeStart;
    rtk_uint32 rangeEnd;
} rtk_filter_value_t;

typedef enum rtk_filter_invert_e
{
    FILTER_INVERT_DISABLE = 0,
    FILTER_INVERT_ENABLE,
    FILTER_INVERT_END,
} rtk_filter_invert_t;


typedef struct
{
    rtk_filter_field_t      *fieldHead;
    rtk_filter_care_tag_t   careTag;
    rtk_filter_value_t      activeport;

    rtk_filter_invert_t     invert;
} rtk_filter_cfg_t;



typedef enum rtk_filter_field_type_e
{
    FILTER_FIELD_DMAC = 0,
    FILTER_FIELD_SMAC,
    FILTER_FIELD_ETHERTYPE,
    FILTER_FIELD_CTAG,
    FILTER_FIELD_STAG,

    FILTER_FIELD_IPV4_SIP,
    FILTER_FIELD_IPV4_DIP,
    FILTER_FIELD_IPV4_TOS,
    FILTER_FIELD_IPV4_PROTOCOL,
    FILTER_FIELD_IPV4_FLAG,
    FILTER_FIELD_IPV4_OFFSET,
    FILTER_FIELD_IPV6_SIPV6,
    FILTER_FIELD_IPV6_DIPV6,
    FILTER_FIELD_IPV6_TRAFFIC_CLASS,
    FILTER_FIELD_IPV6_NEXT_HEADER,

    FILTER_FIELD_TCP_SPORT,
    FILTER_FIELD_TCP_DPORT,
    FILTER_FIELD_TCP_FLAG,
    FILTER_FIELD_UDP_SPORT,
    FILTER_FIELD_UDP_DPORT,
    FILTER_FIELD_ICMP_CODE,
    FILTER_FIELD_ICMP_TYPE,
    FILTER_FIELD_IGMP_TYPE,

    FILTER_FIELD_VID_RANGE,
    FILTER_FIELD_IP_RANGE,
    FILTER_FIELD_PORT_RANGE,

    FILTER_FIELD_USER_DEFINED00,
    FILTER_FIELD_USER_DEFINED01,
    FILTER_FIELD_USER_DEFINED02,
    FILTER_FIELD_USER_DEFINED03,
    FILTER_FIELD_USER_DEFINED04,
    FILTER_FIELD_USER_DEFINED05,
    FILTER_FIELD_USER_DEFINED06,
    FILTER_FIELD_USER_DEFINED07,
    FILTER_FIELD_USER_DEFINED08,
    FILTER_FIELD_USER_DEFINED09,
    FILTER_FIELD_USER_DEFINED10,
    FILTER_FIELD_USER_DEFINED11,
    FILTER_FIELD_USER_DEFINED12,
    FILTER_FIELD_USER_DEFINED13,
    FILTER_FIELD_USER_DEFINED14,
    FILTER_FIELD_USER_DEFINED15,

    FILTER_FIELD_PATTERN_MATCH,

    FILTER_FIELD_END,
} rtk_filter_field_type_t;

typedef enum rtk_filter_field_type_raw_e
{
    FILTER_FIELD_RAW_UNUSED = 0,
    FILTER_FIELD_RAW_DMAC_15_0,
    FILTER_FIELD_RAW_DMAC_31_16,
	FILTER_FIELD_RAW_DMAC_47_32,
    FILTER_FIELD_RAW_SMAC_15_0,
    FILTER_FIELD_RAW_SMAC_31_16,
    FILTER_FIELD_RAW_SMAC_47_32,
    FILTER_FIELD_RAW_ETHERTYPE,
    FILTER_FIELD_RAW_STAG,
    FILTER_FIELD_RAW_CTAG,

    FILTER_FIELD_RAW_IPV4_SIP_15_0 = 0x10,
    FILTER_FIELD_RAW_IPV4_SIP_31_16,
    FILTER_FIELD_RAW_IPV4_DIP_15_0,
    FILTER_FIELD_RAW_IPV4_DIP_31_16,


    FILTER_FIELD_RAW_IPV6_SIP_15_0 = 0x20,
    FILTER_FIELD_RAW_IPV6_SIP_31_16,
    FILTER_FIELD_RAW_IPV6_DIP_15_0,
    FILTER_FIELD_RAW_IPV6_DIP_31_16,

	FILTER_FIELD_RAW_VIDRANGE = 0x30,
	FILTER_FIELD_RAW_IPRANGE,
	FILTER_FIELD_RAW_PORTRANGE,

	FILTER_FIELD_RAW_FIELD_SELECT00 = 0x40,
	FILTER_FIELD_RAW_FIELD_SELECT01,
	FILTER_FIELD_RAW_FIELD_SELECT02,
	FILTER_FIELD_RAW_FIELD_SELECT03,
	FILTER_FIELD_RAW_FIELD_SELECT04,
	FILTER_FIELD_RAW_FIELD_SELECT05,
	FILTER_FIELD_RAW_FIELD_SELECT06,
	FILTER_FIELD_RAW_FIELD_SELECT07,
	FILTER_FIELD_RAW_FIELD_SELECT08,
	FILTER_FIELD_RAW_FIELD_SELECT09,
	FILTER_FIELD_RAW_FIELD_SELECT10,
	FILTER_FIELD_RAW_FIELD_SELECT11,
	FILTER_FIELD_RAW_FIELD_SELECT12,
	FILTER_FIELD_RAW_FIELD_SELECT13,
	FILTER_FIELD_RAW_FIELD_SELECT14,
	FILTER_FIELD_RAW_FIELD_SELECT15,

    FILTER_FIELD_RAW_END,
} rtk_filter_field_type_raw_t;

typedef struct
{
    rtk_filter_field_raw_t      dataFieldRaw[RTK_FILTER_RAW_FIELD_NUMBER];
    rtk_filter_field_raw_t      careFieldRaw[RTK_FILTER_RAW_FIELD_NUMBER];
	rtk_filter_field_type_raw_t fieldRawType[RTK_FILTER_RAW_FIELD_NUMBER];
    rtk_filter_care_tag_t       careTag;
    rtk_filter_value_t          activeport;

    rtk_filter_invert_t         invert;
	rtk_enable_t                valid;
} rtk_filter_cfg_raw_t;

typedef enum rtk_filter_act_enable_e
{
    /* CVLAN */
    FILTER_ENACT_INGRESS_CVLAN_INDEX = 0,
    FILTER_ENACT_INGRESS_CVLAN_VID,
    FILTER_ENACT_CVLAN_INGRESS,
    FILTER_ENACT_CVLAN_EGRESS,
    FILTER_ENACT_CVLAN_SVID,
    FILTER_ENACT_POLICING_1,

    /* SVLAN */
    FILTER_ENACT_EGRESS_SVLAN_INDEX,
    FILTER_ENACT_SVLAN_INGRESS,
    FILTER_ENACT_SVLAN_EGRESS,
    FILTER_ENACT_SVLAN_CVID,
    FILTER_ENACT_POLICING_2,

    /* Policing and Logging */
    FILTER_ENACT_POLICING_0,

    /* Forward */
    FILTER_ENACT_COPY_CPU,
    FILTER_ENACT_DROP,
    FILTER_ENACT_ADD_DSTPORT,
    FILTER_ENACT_REDIRECT,
    FILTER_ENACT_MIRROR,
    FILTER_ENACT_TRAP_CPU,

    /* QoS */
    FILTER_ENACT_PRIORITY,
    FILTER_ENACT_DSCP_REMARK,
    FILTER_ENACT_1P_REMARK,
    FILTER_ENACT_POLICING_3,

    /* Interrutp and GPO */
    FILTER_ENACT_INTERRUPT,
    FILTER_ENACT_GPO,

    FILTER_ENACT_END,
} rtk_filter_act_enable_t;


#define FILTER_ENACT_CVLAN_MASK         0x01
#define FILTER_ENACT_SVLAN_MASK         0x02
#define FILTER_ENACT_PRIORITY_MASK    	0x04
#define FILTER_ENACT_POLICING_MASK    	0x08
#define FILTER_ENACT_FWD_MASK    		0x10
#define FILTER_ENACT_INTGPIO_MASK    	0x20
#define FILTER_ENACT_INIT_MASK			0x3F

#define FILTER_ENACT_CVLAN_TYPE(type)	(type - FILTER_ENACT_CVLAN_INGRESS)
#define FILTER_ENACT_SVLAN_TYPE(type)	(type - FILTER_ENACT_SVLAN_INGRESS)
#define FILTER_ENACT_FWD_TYPE(type)		(type - FILTER_ENACT_ADD_DSTPORT)
#define FILTER_ENACT_PRI_TYPE(type)		(type - FILTER_ENACT_PRIORITY)



typedef struct
{
    rtk_filter_act_enable_t actEnable[FILTER_ENACT_END];

	/* CVLAN acton */
	rtk_uint32 		filterIngressCvlanIdx;
	rtk_uint32     	filterIngressCvlanVid;
	rtk_uint32 		filterCvlanIdx;
	/* SVLAN action */
	rtk_uint32     	filterEgressSvlanIdx;
    rtk_uint32      filterSvlanVid;
	rtk_uint32 		filterSvlanIdx;

	/* Policing action */
	rtk_uint32     	filterPolicingIdx[FILTER_POLICING_MAX];

	/* Forwarding action */
    rtk_uint32      filterRedirectPortmask;
    rtk_uint32      filterAddDstPortmask;
	rtk_uint32 		filterPortmask;

	/* QOS action */
    rtk_uint32      filterPriority;

	/*GPO*/
    rtk_uint32      filterPin;

} rtk_filter_action_t;

typedef struct rtk_filter_ip_s
{
    rtk_uint32 dataType;
    rtk_uint32 rangeStart;
    rtk_uint32 rangeEnd;
    rtk_uint32 value;
    rtk_uint32 mask;
} rtk_filter_ip_t;

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif

/* ethernet address type */
typedef struct  rtk_mac_s
{
    rtk_uint8 octet[ETHER_ADDR_LEN];
} rtk_mac_t;


typedef struct rtk_filter_mac_s
{
    rtk_uint32 dataType;
    rtk_mac_t value;
    rtk_mac_t mask;
    rtk_mac_t rangeStart;
    rtk_mac_t rangeEnd;
} rtk_filter_mac_t;

typedef rtk_uint32 rtk_filter_op_t;



typedef struct rtk_filter_tag_s
{
    rtk_filter_value_t pri;
    rtk_filter_flag_t cfi;
    rtk_filter_value_t vid;
} rtk_filter_tag_t;

typedef struct rtk_filter_ipFlag_s
{
    rtk_filter_flag_t xf;
    rtk_filter_flag_t mf;
    rtk_filter_flag_t df;
} rtk_filter_ipFlag_t;

typedef struct
{
    rtk_uint32 addr[RTK_IPV6_ADDR_WORD_LENGTH];
} rtk_filter_ip6_addr_t;

typedef struct
{
    rtk_uint32 dataType;
    rtk_filter_ip6_addr_t value;
    rtk_filter_ip6_addr_t mask;
    rtk_filter_ip6_addr_t rangeStart;
    rtk_filter_ip6_addr_t rangeEnd;
} rtk_filter_ip6_t;



typedef struct rtk_filter_pattern_s
{
    rtk_uint32 value[FILTER_PATTERN_MAX];
    rtk_uint32 mask[FILTER_PATTERN_MAX];
} rtk_filter_pattern_t;

typedef struct rtk_filter_tcpFlag_s
{
    rtk_filter_flag_t urg;
    rtk_filter_flag_t ack;
    rtk_filter_flag_t psh;
    rtk_filter_flag_t rst;
    rtk_filter_flag_t syn;
    rtk_filter_flag_t fin;
    rtk_filter_flag_t ns;
    rtk_filter_flag_t cwr;
    rtk_filter_flag_t ece;
} rtk_filter_tcpFlag_t;



typedef enum rtk_filter_field_temple_input_e
{
    FILTER_FIELD_TEMPLE_INPUT_TYPE = 0,
    FILTER_FIELD_TEMPLE_INPUT_INDEX,
    FILTER_FIELD_TEMPLE_INPUT_MAX ,
} rtk_filter_field_temple_input_t;

typedef struct
{
    rtk_uint32 value[RTK_DOT_1AS_TIMESTAMP_UNIT_IN_WORD_LENGTH];
} rtk_filter_dot1as_timestamp_t;


struct rtk_filter_field
{
    rtk_uint32 fieldType;

    union
    {
        /* L2 struct */
        rtk_filter_mac_t       dmac;
        rtk_filter_mac_t       smac;
        rtk_filter_value_t     etherType;
        rtk_filter_tag_t       ctag;
        rtk_filter_tag_t       relayCtag;
        rtk_filter_tag_t       stag;
        rtk_filter_tag_t       l2tag;
        rtk_filter_dot1as_timestamp_t dot1asTimeStamp;
        rtk_filter_mac_t       mac;

        /* L3 struct */
	    rtk_filter_ip_t      sip;
        rtk_filter_ip_t      dip;
        rtk_filter_ip_t      ip;
        rtk_filter_value_t   protocol;
        rtk_filter_value_t   ipTos;
        rtk_filter_ipFlag_t  ipFlag;
        rtk_filter_value_t   ipOffset;
	    rtk_filter_ip6_t     sipv6;
        rtk_filter_ip6_t     dipv6;
        rtk_filter_ip6_t     ipv6;
        rtk_filter_value_t   ipv6TrafficClass;
        rtk_filter_value_t   ipv6NextHeader;
        rtk_filter_value_t   flowLabel;

        /* L4 struct */
        rtk_filter_value_t   tcpSrcPort;
        rtk_filter_value_t   tcpDstPort;
        rtk_filter_tcpFlag_t tcpFlag;
        rtk_filter_value_t   tcpSeqNumber;
        rtk_filter_value_t   tcpAckNumber;
        rtk_filter_value_t   udpSrcPort;
        rtk_filter_value_t   udpDstPort;
        rtk_filter_value_t   icmpCode;
        rtk_filter_value_t   icmpType;
        rtk_filter_value_t   igmpType;

        /* pattern match */
        rtk_filter_pattern_t pattern;

        rtk_filter_value_t   inData;

	} filter_pattern_union;

    rtk_uint32 fieldTemplateNo;
    rtk_uint32 fieldTemplateIdx[RTK_FILTER_FIELD_USED_MAX];

    struct rtk_filter_field *next;
};





typedef enum rtk_filter_flag_care_type_e
{
    FILTER_FLAG_CARE_DONT_CARE = 0,
    FILTER_FLAG_CARE_1,
    FILTER_FLAG_CARE_0,
    FILTER_FLAG_END
} rtk_filter_flag_care_type_t;





typedef rtk_uint32 rtk_filter_state_t;

typedef rtk_uint32 rtk_filter_unmatch_action_t;

typedef enum rtk_filter_unmatch_action_e
{
    FILTER_UNMATCH_DROP = 0,
    FILTER_UNMATCH_PERMIT,
    FILTER_UNMATCH_END,
} rtk_filter_unmatch_action_type_t;



typedef struct
{
    rtk_uint32 index;
    rtk_filter_field_type_raw_t fieldType[RTK_FILTER_RAW_FIELD_NUMBER];
} rtk_filter_template_t;

typedef enum rtk_field_sel_e
{
    FORMAT_DEFAULT = 0,
    FORMAT_RAW,
	FORMAT_LLC,
	FORMAT_IPV4,
	FORMAT_ARP,
	FORMAT_IPV6,
	FORMAT_IPPAYLOAD,
	FORMAT_L4PAYLOAD,
    FORMAT_END
}rtk_field_sel_t;

typedef enum rtk_filter_iprange_e
{
    IPRANGE_UNUSED = 0,
    IPRANGE_IPV4_SIP,
    IPRANGE_IPV4_DIP,
    IPRANGE_IPV6_SIP,
    IPRANGE_IPV6_DIP,
    IPRANGE_END
}rtk_filter_iprange_t;

typedef enum rtk_filter_vidrange_e
{
    VIDRANGE_UNUSED = 0,
    VIDRANGE_CVID,
    VIDRANGE_SVID,
    VIDRANGE_END
}rtk_filter_vidrange_t;

typedef enum rtk_filter_portrange_e
{
    PORTRANGE_UNUSED = 0,
    PORTRANGE_SPORT,
    PORTRANGE_DPORT,
    PORTRANGE_END
}rtk_filter_portrange_t;

rtk_api_ret_t rtk_filter_igrAcl_init(void);
rtk_api_ret_t rtk_filter_igrAcl_cfg_delAll(void);
rtk_api_ret_t rtk_filter_igrAcl_cfg_add(rtk_filter_id_t filter_id, rtk_filter_cfg_t* pFilter_cfg, rtk_filter_action_t* pFilter_action, rtk_filter_number_t *ruleNum);
rtk_api_ret_t rtk_filter_igrAcl_field_sel_set(rtk_uint32 index, rtk_field_sel_t format, rtk_uint32 offset);
rtk_api_ret_t rtk_filter_igrAcl_template_set(rtk_filter_template_t *aclTemplate);
rtk_api_ret_t rtk_filter_iprange_set(rtk_uint32 index, rtk_filter_iprange_t type, rtk_uint32 upperIp, rtk_uint32 lowerIp);
rtk_api_ret_t rtk_filter_iprange_get(rtk_uint32 index, rtk_filter_iprange_t *pType, rtk_uint32 *pUpperIp, rtk_uint32 *pLowerIp);
rtk_api_ret_t rtk_filter_portrange_set(rtk_uint32 index, rtk_filter_portrange_t type, rtk_uint32 upperPort, rtk_uint32 lowerPort);
rtk_api_ret_t rtk_filter_portrange_get(rtk_uint32 index, rtk_filter_portrange_t *pType, rtk_uint32 *pUpperPort, rtk_uint32 *pLowerPort);
rtk_api_ret_t rtk_filter_igrAcl_field_add(rtk_filter_cfg_t* pFilter_cfg, rtk_filter_field_t* pFilter_field);
rtk_api_ret_t rtk_filter_igrAcl_cfg_get(rtk_filter_id_t filter_id, rtk_filter_cfg_raw_t *pFilter_cfg, rtk_filter_action_t *pAction);

#endif

